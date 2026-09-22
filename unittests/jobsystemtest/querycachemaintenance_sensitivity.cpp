//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity twin for querycachemaintenancetest. Each mutant is a
//          plausible migration defect; the equivalence comparator must reject
//          every one, and the unmutated kernel must still pass the same run.
//
//=============================================================================//

#include "querycache_harness.h"

#include <cstdio>

namespace
{

using qc::Cache;
using qc::DListWithTail;
using qc::Split;

// Refresh boundary moved from >= to >.
void ClassifyStrictRefresh( Split &split )
{
	for ( int i = 0; i < split.m_nItems; ++i )
	{
		qc::Item &item = split.m_pItems[i];
		const qc::Entry &e = *item.m_pEntry;
		const float elapsed = split.m_flCurTime - e.m_flLastUpdateTime;
		const float interval = e.m_QueryParams.m_flMinimumUpdateInterval;
		if ( e.m_bUsedSinceUpdated )
			item.m_Action = elapsed > interval ? QueryCacheMaintenance::ACTION_REFRESH :
												 QueryCacheMaintenance::ACTION_KEEP;
		else
			item.m_Action = QueryCacheMaintenance::Classify( false, e.m_bSpeculativelyDone,
				elapsed, interval );
	}
}

// A worker that skips the last entry of its split (lost write / bad claim).
void ClassifyDropsLast( Split &split )
{
	QueryCacheMaintenance::ClassifySplit( split );
	if ( split.m_nItems > 0 )
		split.m_pItems[split.m_nItems - 1].m_Action = QueryCacheMaintenance::ACTION_KEEP;
}

// Wasted speculation misreported as plain expiry.
void ClassifyNoWasted( Split &split )
{
	QueryCacheMaintenance::ClassifySplit( split );
	for ( int i = 0; i < split.m_nItems; ++i )
	{
		if ( split.m_pItems[i].m_Action == QueryCacheMaintenance::ACTION_EXPIRE_WASTED )
			split.m_pItems[i].m_Action = QueryCacheMaintenance::ACTION_EXPIRE;
	}
}

// Killed lists published in reverse split order (unordered merge).
void CommitReversePublish( Cache &c, Split *splits, int nSplits )
{
	std::vector<DListWithTail> killed( nSplits );
	qc::Refresh refresh = { &c };
	std::vector<qc::DList> ignored( 1 );
	QueryCacheMaintenance::Commit( splits, nSplits, c.chains.data(), killed.data(), ignored[0],
		int( qc::TYPE_INVALID ), c.nWasted, refresh );
	// Commit prepended into a scratch list; republish the killed lists reversed.
	for ( int i = nSplits; --i >= 0; )
		PrependDListWithTailToDList( killed[i], c.victims );
}

// Splits committed in reverse order (callback order follows completion order).
void CommitReverseSplits( Cache &c, Split *splits, int nSplits )
{
	std::vector<Split> reversed( splits, splits + nSplits );
	for ( int i = 0; i < nSplits / 2; ++i )
		std::swap( reversed[i], reversed[nSplits - 1 - i] );
	qc::KernelCommit( c, reversed.data(), nSplits );
}

// Refresh stamped with the time the batch was gathered one frame late.
void CommitStaleTime( Cache &c, Split *splits, int nSplits )
{
	for ( int i = 0; i < nSplits; ++i )
		splits[i].m_flCurTime -= 0.001f;
	qc::KernelCommit( c, splits, nSplits );
}

struct Mutant
{
	const char *name;
	void ( *classify )( Split & );
	void ( *commit )( Cache &, Split *, int );
};

const Mutant kMutants[] = {
	{ "refresh boundary >= becomes >", &ClassifyStrictRefresh, &qc::KernelCommit },
	{ "worker drops its last item", &ClassifyDropsLast, &qc::KernelCommit },
	{ "wasted speculation not counted", &ClassifyNoWasted, &qc::KernelCommit },
	{ "killed lists published in reverse", &qc::KernelClassify, &CommitReversePublish },
	{ "splits committed in reverse", &qc::KernelClassify, &CommitReverseSplits },
	{ "refresh uses a stale time", &qc::KernelClassify, &CommitStaleTime },
};

} // namespace

int main()
{
	const qc::Config cfg = { 64, 128, 8, 48, 300, 90 };
	int checks = 0, failures = 0;
	qc::ThreadBackend backend( 3 );

	auto run = [&]( void ( *classify )( Split & ), void ( *commit )( Cache &, Split *, int ),
				   std::string *why )
	{
		for ( uint32_t seed = 1; seed <= 4; ++seed )
		{
			auto update = [&]( Cache &cache )
			{
				return qc::KernelUpdate( cache, cfg.nSplits, qc::Mode::PooledGraph, &backend,
					classify, commit );
			};
			int frame = -1;
			if ( qc::RunScenario( cfg, seed, update, why, &frame ) != cfg.nFrames )
				return false;
		}
		return true;
	};

	std::string why;
	++checks;
	if ( !run( &qc::KernelClassify, &qc::KernelCommit, &why ) )
	{
		++failures;
		std::printf( "FAIL unmutated kernel diverged: %s\n", why.c_str() );
	}
	for ( const Mutant &mutant : kMutants )
	{
		++checks;
		why.clear();
		if ( run( mutant.classify, mutant.commit, &why ) )
		{
			++failures;
			std::printf( "FAIL mutant not detected: %s\n", mutant.name );
		}
		else
		{
			std::printf( "detected: %s (%s)\n", mutant.name, why.c_str() );
		}
	}
	std::printf( "%d checks, %d failures\n", checks, failures );
	return failures ? 1 : 0;
}
