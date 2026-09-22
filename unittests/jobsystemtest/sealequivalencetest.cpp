//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Differential seal oracle (RFC 0003 Phase B, Q-JOBS). An independent
//          and deliberately naive reference model of JobGraphBuilder::Seal()
//          (quadratic dedup, scan-based stable Kahn order, full reachability
//          matrix, pairwise conflict scan) is compared with the real builder on
//          seeded random graphs: success/failure, error code and offending
//          jobs, topological order, and the exact order of every job's
//          prerequisites and dependents. A rejected builder must also reseal
//          identically after further declarations. Negative controls prove the
//          comparator detects a perturbed order and a wrong error pair.
//
//=============================================================================//

#include "jobsystem/job_graph.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using namespace jobsystem;

namespace
{

int g_checks = 0;
int g_failures = 0;

#define CHECK( condition )                                                                         \
	do                                                                                             \
	{                                                                                              \
		++g_checks;                                                                                \
		if ( !( condition ) )                                                                      \
		{                                                                                          \
			++g_failures;                                                                          \
			std::printf( "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition );                 \
		}                                                                                          \
	} while ( 0 )

// Deterministic generator (no std:: distributions: portable sequences).
struct Rng
{
	uint64_t s;
	uint32_t Next()
	{
		s = s * 6364136223846793005ull + 1442695040888963407ull;
		return (uint32_t)( s >> 33 );
	}
	uint32_t Below( uint32_t n ) { return n ? Next() % n : 0; }
	bool Chance( uint32_t percent ) { return Below( 100 ) < percent; }
};

// One recorded builder call, replayed into both the real builder and the model.
struct Op
{
	enum Kind { AddJob, Dep, Read, Write } kind = AddJob;
	bool emptyName = false;
	ExecutorToken exec;
	uint32_t a = 0, b = 0;
	DependencyKind dep = DependencyKind::Success;
	ResourceVersion res;
};

Op MakeOp( Op::Kind kind )
{
	Op op;
	op.kind = kind;
	return op;
}

// Normalized seal outcome for comparison.
struct Outcome
{
	bool ok = false;
	GraphErrorCode code = GraphErrorCode::None;
	uint32_t jobA = JobHandle::kInvalid, jobB = JobHandle::kInvalid;
	std::vector<uint32_t> topo;
	std::vector<std::vector<std::pair<uint32_t, DependencyKind>>> prereqs;
	std::vector<std::vector<uint32_t>> dependents;

	bool operator==( const Outcome &o ) const
	{
		return ok == o.ok && code == o.code && jobA == o.jobA && jobB == o.jobB && topo == o.topo &&
		       prereqs == o.prereqs && dependents == o.dependents;
	}
};

//-----------------------------------------------------------------------------
// Reference model: the literal RFC semantics with no performance concerns.
//-----------------------------------------------------------------------------
Outcome ReferenceSeal( const std::vector<Op> &ops )
{
	struct E { uint32_t p, c; DependencyKind k; };
	struct R { uint32_t job; ResourceVersion res; bool write; };
	std::vector<bool> emptyName;
	std::vector<std::pair<uint16_t, uint32_t>> seq;
	std::vector<E> edges;
	std::vector<R> res;
	for ( const Op &op : ops )
	{
		switch ( op.kind )
		{
		case Op::AddJob:
			if ( op.exec.kind == ExecutorKind::Sequence )
				seq.push_back( { op.exec.lane, (uint32_t)emptyName.size() } );
			emptyName.push_back( op.emptyName );
			break;
		case Op::Dep:
			edges.push_back( { op.a, op.b, op.dep } );
			break;
		case Op::Read:
		case Op::Write:
			res.push_back( { op.a, op.res, op.kind == Op::Write } );
			break;
		}
	}

	Outcome out;
	auto fail = [&]( GraphErrorCode code, uint32_t a, uint32_t b )
	{
		out.topo.clear();
		out.code = code;
		out.jobA = a;
		out.jobB = b;
		return out;
	};
	const uint32_t n = (uint32_t)emptyName.size();
	for ( uint32_t i = 0; i < n; ++i )
		if ( emptyName[i] )
			return fail( GraphErrorCode::EmptyName, i, JobHandle::kInvalid );

	// Sequence edges: consecutive members of each lane in registration order,
	// appended after the explicit edges in ascending lane order.
	std::vector<uint16_t> lanes;
	for ( const auto &m : seq )
		if ( std::find( lanes.begin(), lanes.end(), m.first ) == lanes.end() )
			lanes.push_back( m.first );
	std::sort( lanes.begin(), lanes.end() );
	for ( uint16_t lane : lanes )
	{
		uint32_t prev = JobHandle::kInvalid;
		for ( const auto &m : seq )
		{
			if ( m.first != lane )
				continue;
			if ( prev != JobHandle::kInvalid )
				edges.push_back( { prev, m.second, DependencyKind::Terminal } );
			prev = m.second;
		}
	}

	std::vector<E> unique;
	for ( const E &e : edges )
	{
		if ( e.p >= n || e.c >= n )
			return fail( GraphErrorCode::InvalidHandle, e.p, e.c );
		if ( e.p == e.c )
			return fail( GraphErrorCode::SelfDependency, e.p, e.c );
		bool merged = false;
		for ( E &u : unique )
			if ( u.p == e.p && u.c == e.c )
			{
				if ( e.k == DependencyKind::Success )
					u.k = DependencyKind::Success;
				merged = true;
				break;
			}
		if ( !merged )
			unique.push_back( e );
	}
	for ( const R &r : res )
		if ( r.job >= n )
			return fail( GraphErrorCode::InvalidHandle, r.job, JobHandle::kInvalid );

	std::vector<uint32_t> indeg( n, 0 );
	for ( const E &u : unique )
		indeg[u.c]++;
	std::vector<bool> done( n, false );
	while ( out.topo.size() < n )
	{
		uint32_t pick = JobHandle::kInvalid;
		for ( uint32_t i = 0; i < n && pick == JobHandle::kInvalid; ++i )
			if ( !done[i] && indeg[i] == 0 )
				pick = i;
		if ( pick == JobHandle::kInvalid )
		{
			for ( uint32_t i = 0; i < n; ++i )
				if ( !done[i] )
					return fail( GraphErrorCode::Cycle, i, JobHandle::kInvalid );
		}
		done[pick] = true;
		out.topo.push_back( pick );
		for ( const E &u : unique )
			if ( u.p == pick )
				indeg[u.c]--;
	}

	// Full transitive closure.
	std::vector<std::vector<bool>> reach( n, std::vector<bool>( n, false ) );
	for ( const E &u : unique )
		reach[u.p][u.c] = true;
	for ( uint32_t k = 0; k < n; ++k )
		for ( uint32_t i = 0; i < n; ++i )
			if ( reach[i][k] )
				for ( uint32_t j = 0; j < n; ++j )
					if ( reach[k][j] )
						reach[i][j] = true;
	for ( size_t a = 0; a < res.size(); ++a )
		for ( size_t b = a + 1; b < res.size(); ++b )
		{
			if ( res[a].job == res[b].job || !( res[a].res == res[b].res ) )
				continue;
			if ( !res[a].write && !res[b].write )
				continue;
			if ( !reach[res[a].job][res[b].job] && !reach[res[b].job][res[a].job] )
				return fail( GraphErrorCode::ResourceConflict, res[a].job, res[b].job );
		}

	out.ok = true;
	out.prereqs.resize( n );
	out.dependents.resize( n );
	for ( const E &u : unique )
	{
		out.prereqs[u.c].push_back( { u.p, u.k } );
		out.dependents[u.p].push_back( u.c );
	}
	return out;
}

//-----------------------------------------------------------------------------
// The real builder.
//-----------------------------------------------------------------------------
void Apply( JobGraphBuilder &b, const std::vector<Op> &ops, size_t from )
{
	for ( size_t i = from; i < ops.size(); ++i )
	{
		const Op &op = ops[i];
		switch ( op.kind )
		{
		case Op::AddJob:
		{
			JobDesc d;
			d.name = op.emptyName ? "" : "job";
			d.executor = op.exec;
			b.AddJob( d );
			break;
		}
		case Op::Dep:
			b.AddDependency( JobHandle{ op.a }, JobHandle{ op.b }, op.dep );
			break;
		case Op::Read:
			b.Read( JobHandle{ op.a }, op.res );
			break;
		case Op::Write:
			b.Write( JobHandle{ op.a }, op.res );
			break;
		}
	}
}

Outcome RealSeal( JobGraphBuilder &b )
{
	Outcome out;
	auto r = b.Seal();
	if ( !r.HasValue() )
	{
		out.code = r.Error().code;
		out.jobA = r.Error().jobA;
		out.jobB = r.Error().jobB;
		return out;
	}
	const SealedGraph &g = r.Value();
	out.ok = true;
	out.topo = g.TopoOrder();
	out.prereqs.resize( g.JobCount() );
	out.dependents.resize( g.JobCount() );
	for ( uint32_t i = 0; i < g.JobCount(); ++i )
	{
		for ( const SealedGraph::Prereq &p : g.GetJob( i ).prereqs )
			out.prereqs[i].push_back( { p.producer, p.kind } );
		out.dependents[i] = g.GetJob( i ).dependents;
	}
	return out;
}

std::vector<Op> RandomOps( Rng &rng, uint32_t maxJobs )
{
	std::vector<Op> ops;
	const uint32_t n = rng.Below( maxJobs + 1 );
	const bool forward = rng.Chance( 70 ); // mostly acyclic by construction
	const uint32_t lanes = 1 + rng.Below( 3 );
	for ( uint32_t i = 0; i < n; ++i )
	{
		Op op = MakeOp( Op::AddJob );
		op.emptyName = rng.Chance( 1 );
		const uint32_t e = rng.Below( 10 );
		op.exec = e < 6 ? Executor::Compute()
		          : e < 8 ? Executor::Sequence( (uint16_t)rng.Below( lanes ) )
		          : e < 9 ? Executor::MainThread()
		                  : Executor::BlockingIO();
		ops.push_back( op );
	}
	const uint32_t edges = n ? rng.Below( n * 3 ) : 0;
	for ( uint32_t k = 0; k < edges; ++k )
	{
		Op op = MakeOp( Op::Dep );
		op.a = rng.Below( n );
		op.b = rng.Below( n );
		if ( forward && op.a > op.b )
			std::swap( op.a, op.b );
		if ( op.a == op.b && !rng.Chance( 3 ) )
			continue;
		if ( rng.Chance( 1 ) )
			op.b = n + rng.Below( 3 ); // invalid handle
		op.dep = rng.Chance( 30 ) ? DependencyKind::Terminal : DependencyKind::Success;
		ops.push_back( op );
		if ( rng.Chance( 10 ) )
		{
			op.dep = rng.Chance( 50 ) ? DependencyKind::Terminal : DependencyKind::Success;
			ops.push_back( op ); // duplicate edge, possibly with another kind
		}
	}
	const uint32_t decls = n ? rng.Below( n * 2 ) : 0;
	for ( uint32_t k = 0; k < decls; ++k )
	{
		Op op = MakeOp( rng.Chance( 40 ) ? Op::Write : Op::Read );
		op.a = rng.Chance( 1 ) ? n + 1 : rng.Below( n );
		op.res = ResourceVersion{ rng.Below( 2 ), rng.Below( 2 ), rng.Below( 3 ) };
		ops.push_back( op );
	}
	return ops;
}

void TestRandomEquivalence()
{
	int sealed = 0, rejected[6] = {};
	for ( uint64_t seed = 1; seed <= 4000; ++seed )
	{
		Rng rng{ seed };
		const std::vector<Op> ops = RandomOps( rng, seed % 4 == 0 ? 48 : 14 );
		JobGraphBuilder b;
		Apply( b, ops, 0 );
		const Outcome real = RealSeal( b );
		const Outcome ref = ReferenceSeal( ops );
		CHECK( real == ref );
		if ( !( real == ref ) )
		{
			std::printf( "    seed=%llu real ok=%d code=%d a=%u b=%u / ref ok=%d code=%d a=%u b=%u\n",
			    (unsigned long long)seed, real.ok, (int)real.code, real.jobA, real.jobB, ref.ok,
			    (int)ref.code, ref.jobA, ref.jobB );
			continue;
		}
		if ( real.ok )
		{
			++sealed;
			continue;
		}
		++rejected[(int)real.code];

		// A rejected builder keeps its declarations: add more and reseal.
		std::vector<Op> more = ops;
		const uint32_t jobs = (uint32_t)std::count_if(
		    ops.begin(), ops.end(), []( const Op &op ) { return op.kind == Op::AddJob; } );
		Op job = MakeOp( Op::AddJob );
		job.exec = Executor::Sequence( 0 );
		more.push_back( job );
		if ( jobs )
		{
			Op dep = MakeOp( Op::Dep );
			dep.a = rng.Below( jobs );
			dep.b = jobs;
			more.push_back( dep );
		}
		Apply( b, more, ops.size() );
		CHECK( RealSeal( b ) == ReferenceSeal( more ) );
	}
	std::printf( "  sealed=%d empty=%d invalid=%d self=%d cycle=%d conflict=%d\n", sealed,
	    rejected[(int)GraphErrorCode::EmptyName], rejected[(int)GraphErrorCode::InvalidHandle],
	    rejected[(int)GraphErrorCode::SelfDependency], rejected[(int)GraphErrorCode::Cycle],
	    rejected[(int)GraphErrorCode::ResourceConflict] );
	// The generator must actually reach every outcome, or the oracle is vacuous.
	CHECK( sealed > 500 );
	for ( int code = 1; code < 6; ++code )
		CHECK( rejected[code] > 5 );
}

// Large shapes the naive model can still check: long chains, wide fans and
// many accessors of one version (the fast conflict path's worst cases).
void TestLargeShapes()
{
	for ( uint32_t n : { 64u, 200u } )
	{
		for ( int variant = 0; variant < 6; ++variant )
		{
			std::vector<Op> ops;
			for ( uint32_t i = 0; i < n; ++i )
			{
				Op op = MakeOp( Op::AddJob );
				op.exec = variant == 5 ? Executor::Sequence( (uint16_t)( i % 3 ) ) : Executor::Compute();
				ops.push_back( op );
			}
			for ( uint32_t i = 1; i < n; ++i )
			{
				Op dep = MakeOp( Op::Dep );
				dep.a = variant == 1 ? 0 : i - 1; // chain, or fan-out from job 0
				dep.b = i;
				if ( variant == 3 && i == n / 2 )
					continue; // break the chain: later writers are unordered
				ops.push_back( dep );
			}
			for ( uint32_t i = 0; i < n; ++i )
			{
				Op acc = MakeOp( ( variant == 2 && i % 7 != 0 ) ? Op::Read : Op::Write );
				acc.a = ( i * 37 ) % n; // shuffled declaration order
				acc.res = ResourceVersion{ 9, 0, 0 };
				ops.push_back( acc );
			}
			if ( variant == 4 )
			{
				Op back = MakeOp( Op::Dep );
				back.a = n - 1;
				back.b = n / 3; // cycle
				ops.push_back( back );
			}
			JobGraphBuilder b;
			Apply( b, ops, 0 );
			CHECK( RealSeal( b ) == ReferenceSeal( ops ) );
		}
	}
}

// The comparator must see the differences it exists to detect.
void TestComparatorSensitivity()
{
	Rng rng{ 99 };
	std::vector<Op> ops;
	for ( int attempt = 0; attempt < 1000; ++attempt )
	{
		ops = RandomOps( rng, 20 );
		const Outcome ref = ReferenceSeal( ops );
		if ( ref.ok && ref.topo.size() > 3 )
			break;
	}
	Outcome ref = ReferenceSeal( ops );
	CHECK( ref.ok );
	Outcome perturbed = ref;
	std::swap( perturbed.topo[0], perturbed.topo[1] );
	CHECK( !( perturbed == ref ) );

	Outcome withEdges = ref;
	for ( auto &p : withEdges.prereqs )
		if ( p.size() > 1 )
		{
			std::swap( p[0], p[1] );
			CHECK( !( withEdges == ref ) );
			break;
		}

	Outcome err;
	err.code = GraphErrorCode::ResourceConflict;
	err.jobA = 1;
	err.jobB = 2;
	Outcome other = err;
	other.jobB = 3;
	CHECK( !( err == other ) );
}

} // namespace

int main()
{
	TestRandomEquivalence();
	TestLargeShapes();
	TestComparatorSensitivity();
	std::printf( "%d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
