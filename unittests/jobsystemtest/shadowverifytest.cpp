//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Batch shadow verifier fixture (RFC 0003 R10/R21, RFC 0005 Q-JOBS).
//
//          public/jobsystem/batch_shadow_verify.h is the comparison and
//          restore protocol the client's bone-setup verifier uses
//          (game/client/bone_setup_shadow_verify.cpp). Here it runs against a
//          stand-in cohort whose items read and write per-item state, run as a
//          real pooled batch on three test threads. Checked:
//
//          - a correct pooled batch verifies with 0 mismatches, every
//            verifiable item is rerun exactly once, and the state after
//            the comparison is byte-identical to the pooled state (no side effects);
//          - skipped items are neither restored nor rerun;
//          - seeded pooled faults are reported as mismatches at the right
//            item and field (an epsilon on one output, a dropped item, two
//            items' outputs swapped), and the verifier does not repair them;
//          - an item reading another item's output: the serial rerun sees
//            what a serial loop would see, so a pooled run in another order
//            is reported;
//          - negative cohorts: a snapshot that does not restore state the
//            item reads is reported; a lossy restore is reported;
//          - record comparison: layout and byte divergences, -0 vs +0, NaN
//            payloads, Read.
//
//          The test threads stand in for the engine pool; the verifier itself
//          runs on the calling thread and starts none.
//
//=============================================================================//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>
#include <thread>
#include <vector>

#include "jobsystem/batch_shadow_verify.h"
#include "testing/conformance_result.h"

static int s_checks = 0;
static int s_failures = 0;

static void Check( bool value, const char *expression, int line )
{
	++s_checks;
	if ( !value )
	{
		++s_failures;
		printf( "FAIL line %d: %s\n", line, expression );
		fflush( stdout );
	}
}
#define CHECK( expression ) Check( ( expression ), #expression, __LINE__ )

using jobsystem::BatchShadowVerifier;
using jobsystem::FirstShadowDivergence;
using jobsystem::ShadowBatchCounts;
using jobsystem::ShadowDivergence;
using jobsystem::ShadowRecord;

//=============================================================================
// Stand-in cohort
//=============================================================================

struct Item
{
	int input;
	int reads;        // index of an item whose output this item reads, or -1
	int skip;         // nonzero: the cohort cannot verify this item
	float out[4];     // the item's output
	unsigned history; // state the item reads and writes (like an IK target)
	int runs;         // how often the item's work ran
};

struct World
{
	std::vector<Item> items;
	std::vector<int> reruns; // per item, counted by the cohort's Rerun
};

static void Work( World &world, unsigned iItem )
{
	Item &item = world.items[iItem];
	const float neighbour = item.reads >= 0 ? world.items[item.reads].out[0] : 0.0f;
	item.history = item.history * 1664525u + 1013904223u + (unsigned)item.input;
	for ( int k = 0; k < 4; ++k )
	{
		item.out[k] = (float)item.input * 0.25f + (float)k + neighbour * 0.5f +
		              (float)( item.history >> 24 ) / 256.0f;
	}
	++item.runs;
}

// Which state the snapshot covers: the correct cohort covers everything an item
// writes; the bad ones do not.
enum class RestorePolicy
{
	Complete,
	ForgetsHistory, // captures history but does not restore it
	Lossy           // restores out[0] rounded
};

class StandInCohort
{
public:
	struct Snapshot
	{
		ShadowRecord record;
	};

	StandInCohort( World &world, RestorePolicy policy ) : m_world( world ), m_policy( policy ) {}

	void Capture( unsigned iItem, Snapshot &out )
	{
		const Item &item = m_world.items[iItem];
		out.record.Clear();
		out.record.Append( "out", item.out, sizeof( item.out ) );
		out.record.AppendValue( "history", item.history );
		out.record.AppendValue( "runs", item.runs );
	}

	void Restore( unsigned iItem, const Snapshot &in )
	{
		Item &item = m_world.items[iItem];
		in.record.Read( "out", item.out, sizeof( item.out ) );
		if ( m_policy == RestorePolicy::Lossy )
			item.out[0] = floorf( item.out[0] );
		if ( m_policy != RestorePolicy::ForgetsHistory )
			in.record.Read( "history", &item.history, sizeof( item.history ) );
		in.record.Read( "runs", &item.runs, sizeof( item.runs ) );
	}

	int SkipReason( unsigned iItem, const Snapshot & ) { return m_world.items[iItem].skip; }
	void RerunBegin() { ++rerunBrackets; }
	void RerunEnd() { ++rerunBrackets; }

	void Rerun( unsigned iItem )
	{
		++m_world.reruns[iItem];
		Work( m_world, iItem );
	}

	void OnMismatch(
	    unsigned iItem, const Snapshot &, const Snapshot &, const ShadowDivergence &divergence )
	{
		mismatchItems.push_back( iItem );
		mismatchFields.push_back( divergence.pField ? divergence.pField : "(end)" );
		mismatchOffsets.push_back( divergence.nByteOffset );
	}

	void OnRestoreMismatch( unsigned iItem, const ShadowDivergence & )
	{
		restoreMismatchItems.push_back( iItem );
	}

	int rerunBrackets = 0;
	std::vector<unsigned> mismatchItems;
	std::vector<const char *> mismatchFields;
	std::vector<size_t> mismatchOffsets;
	std::vector<unsigned> restoreMismatchItems;

private:
	World &m_world;
	RestorePolicy m_policy;
};

//=============================================================================
// Pooled batch (three test threads) and seeded faults
//=============================================================================

enum class Fault
{
	None,
	Epsilon, // one item's out[2] nudged after its work
	Drop,    // one item's work never runs
	Swap,    // two items' outputs exchanged after the batch
	Reverse  // items run serially in reverse order
};

static void RunPooled( World &world, Fault fault, unsigned iFaultItem )
{
	const unsigned nItems = (unsigned)world.items.size();
	if ( fault == Fault::Reverse )
	{
		for ( unsigned i = nItems; i-- > 0; )
			Work( world, i );
		return;
	}
	std::atomic<unsigned> next( 0 );
	auto runner = [&]()
	{
		for ( ;; )
		{
			const unsigned i = next.fetch_add( 1, std::memory_order_relaxed );
			if ( i >= nItems )
				return;
			if ( fault == Fault::Drop && i == iFaultItem )
				continue;
			Work( world, i );
			if ( fault == Fault::Epsilon && i == iFaultItem )
				world.items[i].out[2] = nextafterf( world.items[i].out[2], 1e30f );
		}
	};
	std::thread a( runner ), b( runner ), c( runner );
	runner();
	a.join();
	b.join();
	c.join();
	if ( fault == Fault::Swap && nItems > iFaultItem + 1 )
	{
		Item &x = world.items[iFaultItem];
		Item &y = world.items[iFaultItem + 1];
		for ( int k = 0; k < 4; ++k )
		{
			const float t = x.out[k];
			x.out[k] = y.out[k];
			y.out[k] = t;
		}
	}
}

static unsigned s_seed = 12345u;
static unsigned NextRandom()
{
	s_seed = s_seed * 1103515245u + 12345u;
	return s_seed >> 8;
}

static World MakeWorld( unsigned nItems, bool bCrossReads, unsigned skipEvery )
{
	World world;
	world.items.resize( nItems );
	world.reruns.assign( nItems, 0 );
	for ( unsigned i = 0; i < nItems; ++i )
	{
		Item &item = world.items[i];
		memset( &item, 0, sizeof( item ) );
		item.input = (int)( NextRandom() % 1000 ) - 500;
		item.reads = ( bCrossReads && i > 0 ) ? (int)( i - 1 ) : -1;
		item.skip = ( skipEvery && i % skipEvery == 1 ) ? 1 : 0;
		item.history = NextRandom();
		for ( int k = 0; k < 4; ++k )
			item.out[k] = (float)( NextRandom() % 100 );
	}
	return world;
}

static bool SameItems( const World &a, const World &b )
{
	return a.items.size() == b.items.size() &&
	       ( a.items.empty() ||
	           memcmp( a.items.data(), b.items.data(), a.items.size() * sizeof( Item ) ) == 0 );
}

// Gather, pooled batch, compare. The cohort outlives the call.
static ShadowBatchCounts RunVerified(
    World &world, StandInCohort &cohort, Fault fault, unsigned iFaultItem, World *pPooled )
{
	BatchShadowVerifier<StandInCohort> verifier;
	verifier.Gather( cohort, (unsigned)world.items.size() );
	RunPooled( world, fault, iFaultItem );
	if ( pPooled )
		*pPooled = world;
	return verifier.CompareWithSerialRerun( cohort );
}

//=============================================================================
// Tests
//=============================================================================

static void TestEquivalence()
{
	const unsigned sizes[] = { 0, 1, 2, 3, 17, 256 };
	for ( int round = 0; round < 40; ++round )
	{
		for ( unsigned n : sizes )
		{
			World world = MakeWorld( n, false, 0 );
			StandInCohort cohort( world, RestorePolicy::Complete );
			World pooled;
			const ShadowBatchCounts counts = RunVerified( world, cohort, Fault::None, 0, &pooled );
			CHECK( counts.items == n );
			CHECK( counts.verified == n );
			CHECK( counts.skipped == 0 );
			CHECK( counts.mismatches == 0 );
			CHECK( counts.restoreMismatches == 0 );
			CHECK( cohort.rerunBrackets == 2 );
			// No side effects: the frame keeps exactly the pooled state.
			CHECK( SameItems( world, pooled ) );
			bool bOnce = true;
			for ( unsigned i = 0; i < n; ++i )
				bOnce = bOnce && world.reruns[i] == 1 && world.items[i].runs == 1;
			CHECK( bOnce );
		}
	}
}

static void TestSkip()
{
	World world = MakeWorld( 40, false, 3 );
	StandInCohort cohort( world, RestorePolicy::Complete );
	World pooled;
	const ShadowBatchCounts counts = RunVerified( world, cohort, Fault::None, 0, &pooled );
	unsigned nSkip = 0;
	bool bSkippedNotRerun = true;
	bool bOthersRerun = true;
	for ( unsigned i = 0; i < 40; ++i )
	{
		if ( world.items[i].skip )
		{
			++nSkip;
			bSkippedNotRerun = bSkippedNotRerun && world.reruns[i] == 0;
		}
		else
		{
			bOthersRerun = bOthersRerun && world.reruns[i] == 1;
		}
	}
	CHECK( nSkip > 0 );
	CHECK( counts.skipped == nSkip );
	CHECK( counts.verified == 40 - nSkip );
	CHECK( counts.mismatches == 0 );
	CHECK( bSkippedNotRerun );
	CHECK( bOthersRerun );
	CHECK( SameItems( world, pooled ) );
}

static void TestSeededFaults()
{
	for ( int round = 0; round < 20; ++round )
	{
		const unsigned n = 8 + NextRandom() % 64;
		const unsigned iFault = NextRandom() % ( n - 1 );

		{
			World world = MakeWorld( n, false, 0 );
			StandInCohort cohort( world, RestorePolicy::Complete );
			World pooled;
			const ShadowBatchCounts counts =
			    RunVerified( world, cohort, Fault::Epsilon, iFault, &pooled );
			CHECK( counts.mismatches == 1 );
			CHECK( cohort.mismatchItems.size() == 1 && cohort.mismatchItems[0] == iFault );
			CHECK(
			    !cohort.mismatchFields.empty() && strcmp( cohort.mismatchFields[0], "out" ) == 0 );
			// out[2] starts at byte 8; the lowest differing byte is within it.
			CHECK( !cohort.mismatchOffsets.empty() && cohort.mismatchOffsets[0] >= 8 &&
			       cohort.mismatchOffsets[0] < 12 );
			// Reported, not repaired: the frame keeps the pooled (faulted) state.
			CHECK( SameItems( world, pooled ) );
			CHECK( counts.restoreMismatches == 0 );
		}
		{
			World world = MakeWorld( n, false, 0 );
			StandInCohort cohort( world, RestorePolicy::Complete );
			const ShadowBatchCounts counts =
			    RunVerified( world, cohort, Fault::Drop, iFault, NULL );
			CHECK( counts.mismatches == 1 );
			CHECK( cohort.mismatchItems.size() == 1 && cohort.mismatchItems[0] == iFault );
		}
		{
			World world = MakeWorld( n, false, 0 );
			StandInCohort cohort( world, RestorePolicy::Complete );
			const ShadowBatchCounts counts =
			    RunVerified( world, cohort, Fault::Swap, iFault, NULL );
			CHECK( counts.mismatches == 2 );
			CHECK( cohort.mismatchItems.size() == 2 && cohort.mismatchItems[0] == iFault &&
			       cohort.mismatchItems[1] == iFault + 1 );
		}
	}
}

static void TestCrossItemReads()
{
	// Serial rerun from restored pre-states reproduces a forward serial loop.
	{
		World world = MakeWorld( 32, true, 0 );
		StandInCohort cohort( world, RestorePolicy::Complete );
		BatchShadowVerifier<StandInCohort> verifier;
		verifier.Gather( cohort, 32 );
		for ( unsigned i = 0; i < 32; ++i )
			Work( world, i );
		const ShadowBatchCounts counts = verifier.CompareWithSerialRerun( cohort );
		CHECK( counts.verified == 32 );
		CHECK( counts.mismatches == 0 );
	}
	// A pooled run in another order reads other pre-states: reported.
	{
		World world = MakeWorld( 32, true, 0 );
		StandInCohort cohort( world, RestorePolicy::Complete );
		const ShadowBatchCounts counts = RunVerified( world, cohort, Fault::Reverse, 0, NULL );
		CHECK( counts.mismatches > 0 );
		CHECK( !cohort.mismatchItems.empty() && cohort.mismatchItems[0] >= 1 );
	}
}

static void TestNegativeCohorts()
{
	// History is read by the work but not restored before the rerun: the
	// rerun starts from the pooled history and every item disagrees.
	{
		World world = MakeWorld( 24, false, 0 );
		StandInCohort cohort( world, RestorePolicy::ForgetsHistory );
		const ShadowBatchCounts counts = RunVerified( world, cohort, Fault::None, 0, NULL );
		CHECK( counts.verified == 24 );
		CHECK( counts.mismatches == 24 );
	}
	// A restore that loses bits is caught by the post-restore capture.
	{
		World world = MakeWorld( 24, false, 0 );
		for ( Item &item : world.items )
			item.input |= 1; // odd inputs: out[0] is never integral
		StandInCohort cohort( world, RestorePolicy::Lossy );
		const ShadowBatchCounts counts = RunVerified( world, cohort, Fault::None, 0, NULL );
		CHECK( counts.restoreMismatches == 24 );
		CHECK( cohort.restoreMismatchItems.size() == 24 );
	}
}

static void TestRecords()
{
	ShadowRecord a;
	ShadowRecord b;
	CHECK( FirstShadowDivergence( a, b ).kind == ShadowDivergence::SAME );

	const float values[3] = { 1.0f, 2.0f, 3.0f };
	a.Append( "values", values, sizeof( values ) );
	a.AppendValue( "count", 3 );
	b.Append( "values", values, sizeof( values ) );
	b.AppendValue( "count", 3 );
	CHECK( FirstShadowDivergence( a, b ).kind == ShadowDivergence::SAME );

	// A byte difference: the field and the first byte.
	float changed[3] = { 1.0f, 2.0f, 3.0f };
	changed[1] = nextafterf( changed[1], 10.0f );
	ShadowRecord c;
	c.Append( "values", changed, sizeof( changed ) );
	c.AppendValue( "count", 3 );
	ShadowDivergence d = FirstShadowDivergence( a, c );
	CHECK( d.kind == ShadowDivergence::BYTES );
	CHECK( d.iField == 0 && strcmp( d.pField, "values" ) == 0 );
	CHECK( d.nByteOffset >= 4 && d.nByteOffset < 8 );

	// Layout: a different name, size or count.
	ShadowRecord e;
	e.Append( "other", values, sizeof( values ) );
	e.AppendValue( "count", 3 );
	CHECK( FirstShadowDivergence( a, e ).kind == ShadowDivergence::LAYOUT );
	ShadowRecord f;
	f.Append( "values", values, sizeof( float ) * 2 );
	f.AppendValue( "count", 3 );
	CHECK( FirstShadowDivergence( a, f ).kind == ShadowDivergence::LAYOUT );
	ShadowRecord g;
	g.Append( "values", values, sizeof( values ) );
	d = FirstShadowDivergence( a, g );
	CHECK(
	    d.kind == ShadowDivergence::LAYOUT && d.iField == 1 && strcmp( d.pField, "count" ) == 0 );
	d = FirstShadowDivergence( g, a );
	CHECK( d.kind == ShadowDivergence::LAYOUT && d.iField == 1 && d.pField == NULL );

	// Bytes, not values: -0 differs from +0; the same NaN equals itself.
	ShadowRecord zeroA;
	ShadowRecord zeroB;
	zeroA.AppendValue( "z", 0.0f );
	zeroB.AppendValue( "z", -0.0f );
	CHECK( FirstShadowDivergence( zeroA, zeroB ).kind == ShadowDivergence::BYTES );
	const float nan = nanf( "" );
	ShadowRecord nanA;
	ShadowRecord nanB;
	nanA.AppendValue( "n", nan );
	nanB.AppendValue( "n", nan );
	CHECK( FirstShadowDivergence( nanA, nanB ).kind == ShadowDivergence::SAME );

	// Read: round trip, absent field, size mismatch.
	float read[3] = { 0, 0, 0 };
	CHECK( a.Read( "values", read, sizeof( read ) ) );
	CHECK( memcmp( read, values, sizeof( read ) ) == 0 );
	int count = 0;
	CHECK( !a.Read( "missing", &count, sizeof( count ) ) );
	CHECK( !a.Read( "values", &count, sizeof( count ) ) );
	CHECK( count == 0 );
	CHECK( a.Find( "count" ) == 1 && a.Find( "missing" ) == a.FieldCount() );

	// Empty fields compare and read.
	ShadowRecord emptyA;
	ShadowRecord emptyB;
	emptyA.Append( "none", NULL, 0 );
	emptyB.Append( "none", NULL, 0 );
	CHECK( FirstShadowDivergence( emptyA, emptyB ).kind == ShadowDivergence::SAME );
	CHECK( emptyA.Read( "none", NULL, 0 ) );
}

// One verifier reused across batches of different sizes (snapshot storage is
// reused; each Capture must overwrite it completely).
static void TestReuse()
{
	BatchShadowVerifier<StandInCohort> verifier;
	const unsigned sizes[] = { 50, 3, 0, 80, 1 };
	for ( unsigned n : sizes )
	{
		World world = MakeWorld( n, true, 4 );
		StandInCohort cohort( world, RestorePolicy::Complete );
		verifier.Gather( cohort, n );
		CHECK( verifier.GatheredItems() == n );
		for ( unsigned i = 0; i < n; ++i )
			Work( world, i );
		const World pooled = world;
		const ShadowBatchCounts counts = verifier.CompareWithSerialRerun( cohort );
		CHECK( counts.mismatches == 0 && counts.restoreMismatches == 0 );
		CHECK( counts.verified + counts.skipped == n );
		CHECK( SameItems( world, pooled ) );
		CHECK( verifier.GatheredItems() == 0 );
	}
}

int main()
{
	TestRecords();
	TestEquivalence();
	TestSkip();
	TestSeededFaults();
	TestCrossItemReads();
	TestNegativeCohorts();
	TestReuse();
	return testing::ReportConformance( s_checks, s_failures );
}
