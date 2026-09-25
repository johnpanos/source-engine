//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Frame graph ordered by declared resource access (RFC 0003).
//
//          Each host node is one MainThread job; the host nodes form a chain.
//          Each batch node is a fixed set of Compute runner jobs, the first
//          of which reads the item count, that claim items from a shared
//          cursor, and an empty join job. Node j depends on node i < j when
//          their declarations conflict: the edges run from i's last job (host
//          job or join) to each of j's first jobs (host job or runners). The
//          join also carries the batch's declarations to the builder, so
//          Seal() independently checks that the batch is ordered against
//          every conflicting node; the runners, which are mutually
//          unordered, declare nothing.
//
//          The pooled executor runs in waves, so a batch overlaps the host
//          nodes that become ready in the same wave as its runners.
//
//=============================================================================//

#include "jobsystem/declared_frame_graph.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include "jobsystem/graph_executor.h"
#include "jobsystem/job_graph.h"
#include "jobsystem/pooled_executor.h"
#include "jobsystem/worker_backend.h"
#include "batch_depth.h"

namespace jobsystem
{

namespace
{
bool Conflicts( const FrameNodeDesc &a, const FrameNodeDesc &b )
{
	for ( unsigned i = 0; i < a.accessCount; ++i )
	{
		for ( unsigned j = 0; j < b.accessCount; ++j )
		{
			const FrameAccess &x = a.access[i];
			const FrameAccess &y = b.access[j];
			const bool sameDomain = x.domain == y.domain || x.domain == FRAME_DOMAIN_ALL ||
			                        y.domain == FRAME_DOMAIN_ALL;
			if ( sameDomain && ( x.write || y.write ) )
				return true;
		}
	}
	return false;
}

bool ValidNode( const FrameNodeDesc &node )
{
	if ( !node.name || !node.name[0] || ( node.accessCount && !node.access ) )
		return false;
	if ( node.kind == FRAME_NODE_HOST )
		return node.run != nullptr;
	if ( node.kind == FRAME_NODE_BATCH )
		return node.count && node.process;
	return false;
}
} // namespace

struct DeclaredFrameGraph::Impl
{
	// Per-run state of one batch node. The cursor only reserves indices; the
	// executor's wave barriers publish the item outputs.
	struct BatchState
	{
		std::mutex prepare; // the first runner reads the count under it
		bool prepared = false;
		unsigned count = 0;
		std::atomic<unsigned> next{ 0 };
		std::atomic<unsigned> processed{ 0 };
	};

	struct ShapeNode
	{
		const char *name;
		FrameNodeKind kind;
		void ( *run )( void * );
		unsigned ( *count )( void * );
		void ( *process )( void *, unsigned );
		void ( *begin )( void * );
		void ( *end )( void * );
		std::vector<FrameAccess> access;
	};

	bool SameShape( const FrameNodeDesc *nodes, unsigned n, FrameGraphMode m, unsigned runners ) const
	{
		if ( !sealed || shape.size() != n || mode != m || batchRunners != runners )
			return false;
		for ( unsigned i = 0; i < n; ++i )
		{
			const ShapeNode &s = shape[i];
			const FrameNodeDesc &d = nodes[i];
			if ( std::strcmp( s.name, d.name ) != 0 || s.kind != d.kind || s.run != d.run ||
			     s.count != d.count || s.process != d.process || s.begin != d.begin ||
			     s.end != d.end || s.access.size() != d.accessCount )
				return false;
			for ( unsigned a = 0; a < d.accessCount; ++a )
			{
				if ( s.access[a].domain != d.access[a].domain ||
				     s.access[a].write != d.access[a].write )
					return false;
			}
		}
		return true;
	}

	void Declare( JobGraphBuilder &builder, JobHandle job, const FrameNodeDesc &node,
	    const std::vector<unsigned> &domains )
	{
		for ( unsigned a = 0; a < node.accessCount; ++a )
		{
			const FrameAccess &access = node.access[a];
			auto declare = [&]( unsigned domain )
			{
				const ResourceVersion resource{ domain, 0, 0 };
				if ( access.write )
					builder.Write( job, resource );
				else
					builder.Read( job, resource );
			};
			if ( access.domain == FRAME_DOMAIN_ALL )
			{
				for ( unsigned domain : domains )
					declare( domain );
			}
			else
			{
				declare( access.domain );
			}
		}
	}

	bool Rebuild( const FrameNodeDesc *nodes, unsigned n, FrameGraphMode m, unsigned runners )
	{
		sealed.reset();
		shape.clear();
		batches.reset( new BatchState[n] ); // indexed by node; unused for host nodes
		first.assign( n, std::vector<JobHandle>() );
		last.assign( n, JobHandle{} );

		// Every concrete domain named in the frame, plus one of its own for
		// FRAME_DOMAIN_ALL, so an ALL access conflicts with everything.
		std::vector<unsigned> domains;
		for ( unsigned i = 0; i < n; ++i )
			for ( unsigned a = 0; a < nodes[i].accessCount; ++a )
				if ( nodes[i].access[a].domain != FRAME_DOMAIN_ALL )
					domains.push_back( nodes[i].access[a].domain );
		domains.push_back( FRAME_DOMAIN_ALL );
		std::sort( domains.begin(), domains.end() );
		domains.erase( std::unique( domains.begin(), domains.end() ), domains.end() );

		JobGraphBuilder builder;
		JobHandle previousHost;
		for ( unsigned i = 0; i < n; ++i )
		{
			const FrameNodeDesc &node = nodes[i];
			if ( node.kind == FRAME_NODE_HOST )
			{
				JobDesc job;
				job.name = node.name;
				job.executor = Executor::MainThread();
				job.function = [this, i]( JobRunContext & )
				{
					++hostNodesRun;
					current[i].run( current[i].context );
				};
				const JobHandle handle = builder.AddJob( job );
				Declare( builder, handle, node, domains );
				if ( previousHost.IsValid() )
					builder.AddDependency( previousHost, handle );
				previousHost = last[i] = handle;
				first[i].push_back( handle );
			}
			else
			{
				JobDesc join;
				join.name = node.name;
				const JobHandle joinHandle = builder.AddJob( join );
				Declare( builder, joinHandle, node, domains );

				JobDesc runner;
				runner.name = node.name;
				runner.executor = Executor::Compute();
				runner.function = [this, i]( JobRunContext & )
				{
					Participate( i );
				};
				for ( unsigned r = 0; r < runners; ++r )
				{
					const JobHandle runnerHandle = builder.AddJob( runner );
					builder.AddDependency( runnerHandle, joinHandle );
					first[i].push_back( runnerHandle );
				}
				last[i] = joinHandle;
			}

			for ( unsigned p = 0; p < i; ++p )
			{
				if ( Conflicts( nodes[p], node ) )
				{
					for ( JobHandle entry : first[i] )
						builder.AddDependency( last[p], entry );
				}
			}
		}

		Expected<SealedGraph, GraphError> built = builder.Seal();
		if ( !built )
			return false;
		sealed.emplace( std::move( built ).Value() );

		for ( unsigned i = 0; i < n; ++i )
		{
			const FrameNodeDesc &d = nodes[i];
			shape.push_back( ShapeNode{ d.name, d.kind, d.run, d.count, d.process, d.begin, d.end,
			    std::vector<FrameAccess>( d.access, d.access + d.accessCount ) } );
		}
		mode = m;
		batchRunners = runners;
		ComputeOverlap( nodes, n );
		++seals;
		return true;
	}

	// Node-level ordering is the host chain plus the conflict edges; two nodes
	// may overlap when neither reaches the other.
	void ComputeOverlap( const FrameNodeDesc *nodes, unsigned n )
	{
		std::vector<std::vector<bool>> reach( n, std::vector<bool>( n, false ) );
		int previousHost = -1;
		for ( unsigned j = 0; j < n; ++j )
		{
			std::vector<unsigned> preds;
			if ( nodes[j].kind == FRAME_NODE_HOST )
			{
				if ( previousHost >= 0 )
					preds.push_back( (unsigned)previousHost );
				previousHost = (int)j;
			}
			for ( unsigned p = 0; p < j; ++p )
				if ( Conflicts( nodes[p], nodes[j] ) )
					preds.push_back( p );
			for ( unsigned p : preds )
			{
				reach[p][j] = true;
				for ( unsigned q = 0; q < j; ++q )
					if ( reach[q][p] )
						reach[q][j] = true;
			}
		}
		overlap.assign( n, std::vector<bool>( n, false ) );
		overlapping = 0;
		for ( unsigned a = 0; a < n; ++a )
		{
			for ( unsigned b = a + 1; b < n; ++b )
			{
				if ( !reach[a][b] )
				{
					overlap[a][b] = overlap[b][a] = true;
					++overlapping;
				}
			}
		}
	}

	void Participate( unsigned node )
	{
		BatchState &state = batches[node];
		const FrameNodeDesc &desc = current[node];
		{
			// The lock orders the count read before every runner's claims.
			std::lock_guard<std::mutex> lock( state.prepare );
			if ( !state.prepared )
			{
				state.count = desc.count( desc.context );
				state.prepared = true;
			}
		}
		unsigned index = state.next.fetch_add( 1, std::memory_order_relaxed );
		if ( index >= state.count )
			return; // an empty runner calls neither begin nor end
		detail::BatchDepthScope depth;
		if ( desc.begin )
			desc.begin( desc.context );
		unsigned processed = 0;
		do
		{
			desc.process( desc.context, index );
			++processed;
			index = state.next.fetch_add( 1, std::memory_order_relaxed );
		} while ( index < state.count );
		if ( desc.end )
			desc.end( desc.context );
		state.processed.fetch_add( processed, std::memory_order_relaxed );
	}

	std::vector<ShapeNode> shape;
	std::optional<SealedGraph> sealed;
	std::unique_ptr<BatchState[]> batches;
	std::vector<std::vector<JobHandle>> first; // a node's entry jobs
	std::vector<JobHandle> last;
	std::vector<std::vector<bool>> overlap;
	unsigned overlapping = 0;
	FrameGraphMode mode = FRAME_GRAPH_SERIAL;
	unsigned batchRunners = 0;
	const FrameNodeDesc *current = nullptr;
	unsigned hostNodesRun = 0;
	unsigned seals = 0;
	bool running = false;
};

DeclaredFrameGraph::DeclaredFrameGraph() : m_impl( new Impl )
{
}

DeclaredFrameGraph::~DeclaredFrameGraph()
{
	delete m_impl;
}

DeclaredFrameRun DeclaredFrameGraph::Run(
    const FrameNodeDesc *nodes, unsigned count, IWorkerBackend *backend, FrameGraphMode mode )
{
	DeclaredFrameRun result = { false, 0u, 0u, 0u };
	if ( m_impl->running || ( count && !nodes ) ||
	     ( mode != FRAME_GRAPH_SERIAL && mode != FRAME_GRAPH_POOLED ) )
		return result;
	for ( unsigned i = 0; i < count; ++i )
	{
		if ( !ValidNode( nodes[i] ) )
			return result;
	}

	// A batch started from inside a batch item runs serially: its thread may
	// be a worker of the backend's pool.
	const bool pooled = mode == FRAME_GRAPH_POOLED && backend && backend->WorkerCount() > 0 &&
	                    detail::BatchDepth() == 0;
	const unsigned runners = pooled ? (unsigned)backend->WorkerCount() + 1u : 1u;
	const FrameGraphMode effective = pooled ? FRAME_GRAPH_POOLED : FRAME_GRAPH_SERIAL;
	if ( !m_impl->SameShape( nodes, count, effective, runners ) &&
	     !m_impl->Rebuild( nodes, count, effective, runners ) )
		return result;

	for ( unsigned i = 0; i < count; ++i )
	{
		Impl::BatchState &state = m_impl->batches[i];
		state.prepared = false;
		state.count = 0;
		state.next.store( 0, std::memory_order_relaxed );
		state.processed.store( 0, std::memory_order_relaxed );
	}
	m_impl->current = nodes;
	m_impl->hostNodesRun = 0;
	m_impl->running = true;

	// The deterministic executor runs in the sealed topological order: the host
	// chain and every conflict edge, ties broken by registration (array) order,
	// so a serial run is array order with batch items ascending.
	RunOptions options;
	options.pumpMainThread = true;
	if ( pooled )
		PooledExecutor( backend ).Execute( *m_impl->sealed, options );
	else
		DeterministicExecutor().Execute( *m_impl->sealed, options );

	m_impl->running = false;
	m_impl->current = nullptr;
	result.valid = true;
	result.hostNodesRun = m_impl->hostNodesRun;
	for ( unsigned i = 0; i < count; ++i )
	{
		if ( nodes[i].kind == FRAME_NODE_BATCH )
			result.batchItemsRun += m_impl->batches[i].processed.load( std::memory_order_relaxed );
	}
	result.batchRunners = runners;
	return result;
}

bool DeclaredFrameGraph::MayOverlap( unsigned a, unsigned b ) const
{
	return a < m_impl->overlap.size() && b < m_impl->overlap.size() && m_impl->overlap[a][b];
}

unsigned DeclaredFrameGraph::OverlappingPairs() const
{
	return m_impl->overlapping;
}

unsigned DeclaredFrameGraph::SealCount() const
{
	return m_impl->seals;
}

} // namespace jobsystem
