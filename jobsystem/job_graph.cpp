//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: JobGraphBuilder validation and sealing (RFC 0003, Phase B).
//
//=============================================================================//

#include "jobsystem/job_graph.h"

#include <algorithm>
#include <cstring>
#include <utility>

namespace jobsystem
{

//-----------------------------------------------------------------------------

JobHandle JobGraphBuilder::AddJob( const JobDesc &desc )
{
	const uint32_t id = (uint32_t)m_jobs.size();
	m_jobs.push_back( BuildJob{ desc } );
	if ( desc.executor.kind == ExecutorKind::Sequence )
	{
		m_sequenceMembers.push_back( { desc.executor.lane, id } );
	}
	return JobHandle{ id };
}

void JobGraphBuilder::AddDependency( JobHandle producer, JobHandle consumer, DependencyKind kind )
{
	m_edges.push_back( Edge{ producer.id, consumer.id, kind } );
}

void JobGraphBuilder::Read( JobHandle job, const ResourceVersion &resource )
{
	m_resources.push_back( ResourceDecl{ job.id, resource, /*write=*/false } );
}

void JobGraphBuilder::Write( JobHandle job, const ResourceVersion &resource )
{
	m_resources.push_back( ResourceDecl{ job.id, resource, /*write=*/true } );
}

//-----------------------------------------------------------------------------

Expected<SealedGraph, GraphError> JobGraphBuilder::Seal()
{
	const uint32_t n = (uint32_t)m_jobs.size();

	// (1) Every job must carry a stable diagnostic name.
	for ( uint32_t i = 0; i < n; ++i )
	{
		const char *nm = m_jobs[i].desc.name;
		if ( nm == nullptr || nm[0] == '\0' )
		{
			return MakeUnexpected( GraphError{ GraphErrorCode::EmptyName, "job has empty name", i, JobHandle::kInvalid } );
		}
	}

	// (2) Combine explicit edges with sequence-induced ordering edges. Sequence
	//     edges are Terminal (ordering only): the next job on a lane runs after
	//     the previous reaches a terminal state, regardless of its outcome.
	std::vector<Edge> edges = m_edges;
	{
		// Group sequence members by lane, preserving registration order.
		std::vector<std::pair<uint16_t, uint32_t>> members = m_sequenceMembers;
		std::stable_sort( members.begin(), members.end(),
			[]( const auto &a, const auto &b ){ return a.first < b.first; } );
		for ( size_t i = 1; i < members.size(); ++i )
		{
			if ( members[i].first == members[i - 1].first )
			{
				edges.push_back( Edge{ members[i - 1].second, members[i].second, DependencyKind::Terminal } );
			}
		}
	}

	// (3) Validate edge endpoints and reject self-dependencies. Deduplicate,
	//     keeping the stronger kind (Success outranks Terminal) so indegree and
	//     prerequisite counts are exact at execution time.
	auto keyOf = []( uint32_t p, uint32_t c ) -> uint64_t { return ( (uint64_t)p << 32 ) | c; };
	std::vector<std::pair<uint64_t, DependencyKind>> uniqueEdges;
	uniqueEdges.reserve( edges.size() );
	for ( const Edge &e : edges )
	{
		if ( e.producer >= n || e.consumer >= n )
		{
			return MakeUnexpected( GraphError{ GraphErrorCode::InvalidHandle, "dependency references unknown job", e.producer, e.consumer } );
		}
		if ( e.producer == e.consumer )
		{
			return MakeUnexpected( GraphError{ GraphErrorCode::SelfDependency, "job depends on itself", e.producer, e.consumer } );
		}
		const uint64_t k = keyOf( e.producer, e.consumer );
		bool merged = false;
		for ( auto &ue : uniqueEdges )
		{
			if ( ue.first == k )
			{
				if ( e.kind == DependencyKind::Success ) ue.second = DependencyKind::Success;
				merged = true;
				break;
			}
		}
		if ( !merged ) uniqueEdges.push_back( { k, e.kind } );
	}

	// Resource decls must reference known jobs.
	for ( const ResourceDecl &r : m_resources )
	{
		if ( r.job >= n )
		{
			return MakeUnexpected( GraphError{ GraphErrorCode::InvalidHandle, "resource decl references unknown job", r.job, JobHandle::kInvalid } );
		}
	}

	// (4) Build adjacency + indegree.
	std::vector<std::vector<uint32_t>> succ( n );
	std::vector<uint32_t> indeg( n, 0 );
	for ( const auto &ue : uniqueEdges )
	{
		const uint32_t p = (uint32_t)( ue.first >> 32 );
		const uint32_t c = (uint32_t)( ue.first & 0xFFFFFFFFu );
		succ[p].push_back( c );
		indeg[c]++;
	}

	// (5) Kahn topological sort, tie-broken by lowest index for stability.
	//     A cycle leaves some node with indegree > 0.
	std::vector<uint32_t> topo;
	topo.reserve( n );
	{
		std::vector<uint32_t> work = indeg;
		// Simple stable ready-set: scan for lowest-index ready node each step.
		std::vector<char> done( n, 0 );
		uint32_t placed = 0;
		while ( placed < n )
		{
			uint32_t pick = JobHandle::kInvalid;
			for ( uint32_t i = 0; i < n; ++i )
			{
				if ( !done[i] && work[i] == 0 ) { pick = i; break; }
			}
			if ( pick == JobHandle::kInvalid ) break; // remaining nodes form a cycle
			done[pick] = 1;
			topo.push_back( pick );
			++placed;
			for ( uint32_t c : succ[pick] ) --work[c];
		}
		if ( placed < n )
		{
			uint32_t offender = JobHandle::kInvalid;
			for ( uint32_t i = 0; i < n; ++i ) if ( !done[i] ) { offender = i; break; }
			return MakeUnexpected( GraphError{ GraphErrorCode::Cycle, "graph contains an ordering cycle", offender, JobHandle::kInvalid } );
		}
	}

	// (6) Resource-conflict validation. Compute descendant reachability so we can
	//     tell whether two conflicting accesses are ordered. Reject unordered
	//     write/write or read/write on the same version rather than invent order.
	if ( !m_resources.empty() && n > 0 )
	{
		// reach[i] : set of nodes reachable *from* i (i is an ancestor). Filled
		// in reverse topo order: reach[i] = union over successors of {c} + reach[c].
		std::vector<std::vector<char>> reach( n, std::vector<char>( n, 0 ) );
		for ( auto it = topo.rbegin(); it != topo.rend(); ++it )
		{
			const uint32_t i = *it;
			for ( uint32_t c : succ[i] )
			{
				reach[i][c] = 1;
				const std::vector<char> &rc = reach[c];
				for ( uint32_t j = 0; j < n; ++j ) if ( rc[j] ) reach[i][j] = 1;
			}
		}
		auto ordered = [&]( uint32_t a, uint32_t b ) { return reach[a][b] || reach[b][a]; };

		for ( size_t a = 0; a < m_resources.size(); ++a )
		{
			for ( size_t b = a + 1; b < m_resources.size(); ++b )
			{
				const ResourceDecl &ra = m_resources[a];
				const ResourceDecl &rb = m_resources[b];
				if ( ra.job == rb.job ) continue;
				if ( !( ra.resource == rb.resource ) ) continue;
				if ( !ra.write && !rb.write ) continue; // read/read on same version is fine
				if ( !ordered( ra.job, rb.job ) )
				{
					return MakeUnexpected( GraphError{ GraphErrorCode::ResourceConflict, "unordered conflicting resource access", ra.job, rb.job } );
				}
			}
		}
	}

	// (7) Materialize the immutable SealedGraph.
	SealedGraph sealed;
	sealed.m_jobs.resize( n );
	for ( uint32_t i = 0; i < n; ++i )
	{
		SealedGraph::Job &j = sealed.m_jobs[i];
		j.name     = m_jobs[i].desc.name;
		j.executor = m_jobs[i].desc.executor;
		j.priority = m_jobs[i].desc.priority;
		j.function = std::move( m_jobs[i].desc.function );
	}
	for ( const auto &ue : uniqueEdges )
	{
		const uint32_t p = (uint32_t)( ue.first >> 32 );
		const uint32_t c = (uint32_t)( ue.first & 0xFFFFFFFFu );
		sealed.m_jobs[c].prereqs.push_back( SealedGraph::Prereq{ p, ue.second } );
		sealed.m_jobs[p].dependents.push_back( c );
	}
	sealed.m_topoOrder = std::move( topo );

	// The builder is now spent; leave it empty so it cannot be reused.
	m_jobs.clear();
	m_edges.clear();
	m_resources.clear();
	m_sequenceMembers.clear();

	return Expected<SealedGraph, GraphError>( std::move( sealed ) );
}

} // namespace jobsystem
