//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: JobGraphBuilder validation and sealing (RFC 0003, Phase B).
//
//=============================================================================//

#include "jobsystem/job_graph.h"

#include <algorithm>
#include <cstring>
#include <functional>
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

namespace
{

// Compressed adjacency: the successors of node i are
// targets[offsets[i] .. offsets[i + 1]), in unique-edge order.
struct Csr
{
	uint32_t *offsets = nullptr;
	uint32_t *targets = nullptr;

	const uint32_t *begin( uint32_t i ) const { return targets + offsets[i]; }
	const uint32_t *end( uint32_t i ) const { return targets + offsets[i + 1]; }
};

// Answers "is b reachable from a" for a validated DAG. A path can only move
// forward in topological position, so the search prunes every node placed at or
// after b and stops as soon as b is found. Stamps avoid clearing per query.
class ReachQuery
{
public:
	ReachQuery( const Csr &succ, const std::vector<uint32_t> &pos )
	    : m_succ( succ ), m_pos( pos ), m_stamp( pos.size(), 0 )
	{
	}

	bool Reaches( uint32_t a, uint32_t b )
	{
		if ( m_pos[a] >= m_pos[b] )
			return false;
		if ( ++m_epoch == 0 )
		{
			std::fill( m_stamp.begin(), m_stamp.end(), 0u );
			m_epoch = 1;
		}
		const uint32_t limit = m_pos[b];
		m_stack.clear();
		m_stack.push_back( a );
		m_stamp[a] = m_epoch;
		while ( !m_stack.empty() )
		{
			const uint32_t i = m_stack.back();
			m_stack.pop_back();
			for ( const uint32_t *c = m_succ.begin( i ); c != m_succ.end( i ); ++c )
			{
				if ( *c == b )
					return true;
				if ( m_stamp[*c] == m_epoch || m_pos[*c] >= limit )
					continue;
				m_stamp[*c] = m_epoch;
				m_stack.push_back( *c );
			}
		}
		return false;
	}

	bool Ordered( uint32_t a, uint32_t b ) { return Reaches( a, b ) || Reaches( b, a ); }

private:
	const Csr &m_succ;
	const std::vector<uint32_t> &m_pos;
	std::vector<uint32_t> m_stamp;
	std::vector<uint32_t> m_stack;
	uint32_t m_epoch = 0;
};

inline bool ResourceLess( const ResourceVersion &a, const ResourceVersion &b )
{
	if ( a.domain != b.domain )
		return a.domain < b.domain;
	if ( a.epoch != b.epoch )
		return a.epoch < b.epoch;
	return a.partition < b.partition;
}

} // namespace

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
	//     They are appended in place and removed again if sealing fails, so a
	//     rejected builder keeps exactly its declared contents.
	std::vector<Edge> &edges = m_edges;
	struct RestoreOnFailure
	{
		std::vector<Edge> &edges;
		size_t declared;
		bool sealed = false;
		~RestoreOnFailure()
		{
			if ( !sealed )
				edges.resize( declared );
		}
	} restore{ edges, edges.size() };
	if ( !m_sequenceMembers.empty() )
	{
		// Group sequence members by lane, preserving registration order.
		std::stable_sort( m_sequenceMembers.begin(), m_sequenceMembers.end(),
		    []( const auto &a, const auto &b )
		    {
			    return a.first < b.first;
		    } );
		for ( size_t i = 1; i < m_sequenceMembers.size(); ++i )
		{
			if ( m_sequenceMembers[i].first == m_sequenceMembers[i - 1].first )
			{
				edges.push_back( Edge{ m_sequenceMembers[i - 1].second, m_sequenceMembers[i].second,
				    DependencyKind::Terminal } );
			}
		}
	}

	// (3) Validate edge endpoints and reject self-dependencies, in declaration
	//     order. Deduplicate, keeping the first occurrence's position and the
	//     stronger kind (Success outranks Terminal) so indegree and prerequisite
	//     counts are exact at execution time.
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
	}

	// All per-seal scratch indices live in one allocation.
	const uint32_t edgeCount = (uint32_t)edges.size();
	std::vector<uint32_t> arena( 3 * (size_t)edgeCount + 9 * (size_t)n + 2 );
	uint32_t *next = arena.data();
	auto carve = [&]( size_t count )
	{
		uint32_t *p = next;
		next += count;
		return p;
	};
	uint32_t *keep = carve( edgeCount );   // 1 = first occurrence of its (producer, consumer)
	uint32_t *byCons = carve( edgeCount ); // edge indices grouped by consumer, stable
	uint32_t *consOff = carve( n + 1 );
	uint32_t *seenFor = carve( n );   // producer -> last consumer that saw it (+1)
	uint32_t *firstEdge = carve( n ); // producer -> first edge index for that consumer
	uint32_t *indeg = carve( n );
	uint32_t *work = carve( n ); // fill cursor, then Kahn's remaining indegree
	uint32_t *heap = carve( n );
	uint32_t *roots = carve( n );
	Csr succ;
	succ.offsets = carve( n + 1 );
	succ.targets = carve( edgeCount );

	// Group edges by consumer with a stable counting sort. Within one consumer
	// edges are visited in declaration order, so the first occurrence of each
	// producer is kept and absorbs any later duplicate's kind.
	for ( uint32_t k = 0; k < edgeCount; ++k )
		consOff[edges[k].consumer + 1]++;
	for ( uint32_t i = 0; i < n; ++i )
		consOff[i + 1] += consOff[i];
	std::copy( consOff, consOff + n, work );
	for ( uint32_t k = 0; k < edgeCount; ++k )
		byCons[work[edges[k].consumer]++] = k;
	for ( uint32_t c = 0; c < n; ++c )
	{
		for ( uint32_t x = consOff[c]; x < consOff[c + 1]; ++x )
		{
			const uint32_t k = byCons[x];
			const uint32_t p = edges[k].producer;
			if ( seenFor[p] != c + 1 )
			{
				seenFor[p] = c + 1;
				firstEdge[p] = k;
				keep[k] = 1;
			}
			else if ( edges[k].kind == DependencyKind::Success )
			{
				edges[firstEdge[p]].kind = DependencyKind::Success;
			}
		}
	}

	// Resource decls must reference known jobs.
	for ( const ResourceDecl &r : m_resources )
	{
		if ( r.job >= n )
		{
			return MakeUnexpected( GraphError{ GraphErrorCode::InvalidHandle, "resource decl references unknown job", r.job, JobHandle::kInvalid } );
		}
	}

	// (4) Build successor adjacency (unique-edge order) + indegree.
	for ( uint32_t k = 0; k < edgeCount; ++k )
	{
		if ( !keep[k] )
			continue;
		succ.offsets[edges[k].producer + 1]++;
		indeg[edges[k].consumer]++;
	}
	for ( uint32_t i = 0; i < n; ++i )
		succ.offsets[i + 1] += succ.offsets[i];
	std::copy( succ.offsets, succ.offsets + n, work );
	for ( uint32_t k = 0; k < edgeCount; ++k )
	{
		if ( keep[k] )
			succ.targets[work[edges[k].producer]++] = edges[k].consumer;
	}

	// (5) Kahn topological sort, always placing the lowest-index ready node next
	//     for a stable order. Initial roots are already ascending, so only nodes
	//     released later need a min-heap; each step takes the smaller head. A
	//     cycle leaves nodes unplaced.
	std::vector<uint32_t> topo;
	topo.reserve( n );
	{
		std::copy( indeg, indeg + n, work );
		uint32_t rootCount = 0, rootHead = 0, heapSize = 0;
		for ( uint32_t i = 0; i < n; ++i )
		{
			if ( work[i] == 0 )
				roots[rootCount++] = i;
		}
		const auto greater = std::greater<uint32_t>();
		while ( rootHead < rootCount || heapSize > 0 )
		{
			uint32_t pick;
			if ( heapSize == 0 || ( rootHead < rootCount && roots[rootHead] < heap[0] ) )
			{
				pick = roots[rootHead++];
			}
			else
			{
				std::pop_heap( heap, heap + heapSize, greater );
				pick = heap[--heapSize];
			}
			topo.push_back( pick );
			for ( const uint32_t *c = succ.begin( pick ); c != succ.end( pick ); ++c )
			{
				if ( --work[*c] == 0 )
				{
					heap[heapSize++] = *c;
					std::push_heap( heap, heap + heapSize, greater );
				}
			}
		}
		if ( topo.size() < n )
		{
			// The lowest-index unplaced node is on (or behind) a cycle: exactly
			// the unplaced nodes keep a nonzero remaining indegree.
			uint32_t offender = JobHandle::kInvalid;
			for ( uint32_t i = 0; i < n; ++i )
				if ( work[i] != 0 )
				{
					offender = i;
					break;
				}
			return MakeUnexpected( GraphError{ GraphErrorCode::Cycle, "graph contains an ordering cycle", offender, JobHandle::kInvalid } );
		}
	}

	// (6) Resource-conflict validation. Reject unordered write/write or
	//     read/write on the same version rather than invent an order. The
	//     reported pair is the first conflicting (a, b) declaration pair in
	//     declaration order, a < b.
	if ( !m_resources.empty() && n > 0 )
	{
		std::vector<uint32_t> pos( n );
		for ( uint32_t i = 0; i < n; ++i )
			pos[topo[i]] = i;
		ReachQuery reach( succ, pos );

		// Group declarations by version, keeping declaration order in a group.
		const uint32_t declCount = (uint32_t)m_resources.size();
		std::vector<uint32_t> byVersion( declCount );
		for ( uint32_t d = 0; d < declCount; ++d )
			byVersion[d] = d;
		std::sort( byVersion.begin(), byVersion.end(),
		    [&]( uint32_t a, uint32_t b )
		    {
			    const ResourceVersion &ra = m_resources[a].resource;
			    const ResourceVersion &rb = m_resources[b].resource;
			    if ( ResourceLess( ra, rb ) )
				    return true;
			    if ( ResourceLess( rb, ra ) )
				    return false;
			    return a < b;
		    } );

		uint32_t bestA = JobHandle::kInvalid, bestB = JobHandle::kInvalid; // decl indices
		std::vector<uint32_t> writers, members;
		for ( uint32_t g = 0; g < declCount; )
		{
			uint32_t e = g + 1;
			while ( e < declCount &&
			        m_resources[byVersion[e]].resource == m_resources[byVersion[g]].resource )
				++e;

			// Fast exact test: a group is conflict-free iff its distinct writer
			// jobs form a reachability chain in topological order and every
			// other accessor is ordered after its preceding writer and before
			// its following writer. Reachability is transitive, so this covers
			// every pair; any failed link is itself an unordered conflict.
			writers.clear();
			members.clear();
			for ( uint32_t k = g; k < e; ++k )
			{
				const ResourceDecl &r = m_resources[byVersion[k]];
				members.push_back( r.job );
				if ( r.write )
					writers.push_back( r.job );
			}
			bool clean = true;
			if ( !writers.empty() && members.size() > 1 )
			{
				auto byPos = [&]( uint32_t a, uint32_t b )
				{
					return pos[a] < pos[b];
				};
				std::sort( writers.begin(), writers.end(), byPos );
				writers.erase( std::unique( writers.begin(), writers.end() ), writers.end() );
				for ( size_t w = 1; w < writers.size() && clean; ++w )
					clean = reach.Reaches( writers[w - 1], writers[w] );
				for ( size_t m = 0; m < members.size() && clean; ++m )
				{
					const uint32_t job = members[m];
					auto it = std::lower_bound( writers.begin(), writers.end(), job, byPos );
					if ( it != writers.end() && *it == job )
						continue; // a writer job: covered by the chain
					if ( it != writers.begin() && !reach.Reaches( *( it - 1 ), job ) )
						clean = false;
					else if ( it != writers.end() && !reach.Reaches( job, *it ) )
						clean = false;
				}
			}

			if ( !clean )
			{
				// Locate this group's first conflicting pair in declaration
				// order; keep the earliest across groups.
				for ( uint32_t x = g; x < e; ++x )
				{
					const uint32_t a = byVersion[x];
					if ( bestA != JobHandle::kInvalid && a > bestA )
						break;
					bool found = false;
					for ( uint32_t y = x + 1; y < e; ++y )
					{
						const uint32_t b = byVersion[y];
						const ResourceDecl &ra = m_resources[a];
						const ResourceDecl &rb = m_resources[b];
						if ( ra.job == rb.job )
							continue;
						if ( !ra.write && !rb.write )
							continue; // read/read on same version is fine
						if ( !reach.Ordered( ra.job, rb.job ) )
						{
							if ( bestA == JobHandle::kInvalid || a < bestA ||
							     ( a == bestA && b < bestB ) )
							{
								bestA = a;
								bestB = b;
							}
							found = true;
							break;
						}
					}
					if ( found )
						break;
				}
			}
			g = e;
		}
		if ( bestA != JobHandle::kInvalid )
		{
			return MakeUnexpected( GraphError{ GraphErrorCode::ResourceConflict,
			    "unordered conflicting resource access", m_resources[bestA].job,
			    m_resources[bestB].job } );
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
		j.prereqs.reserve( indeg[i] );
		j.dependents.reserve( succ.offsets[i + 1] - succ.offsets[i] );
	}
	for ( uint32_t k = 0; k < edgeCount; ++k )
	{
		if ( !keep[k] )
			continue;
		const uint32_t p = edges[k].producer;
		const uint32_t c = edges[k].consumer;
		sealed.m_jobs[c].prereqs.push_back( SealedGraph::Prereq{ p, edges[k].kind } );
		sealed.m_jobs[p].dependents.push_back( c );
	}
	sealed.m_topoOrder = std::move( topo );
	restore.sealed = true;

	// The builder is now spent; leave it empty so it cannot be reused.
	m_jobs.clear();
	m_edges.clear();
	m_resources.clear();
	m_sequenceMembers.clear();

	return Expected<SealedGraph, GraphError>( std::move( sealed ) );
}

} // namespace jobsystem
