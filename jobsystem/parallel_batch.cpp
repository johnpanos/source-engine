//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded independent compute batches on dependency-aware job graphs.
//
//=============================================================================//

#include "jobsystem/parallel_batch.h"
#include "jobsystem/pooled_executor.h"

#include <algorithm>
#include <atomic>
#include <climits>

namespace jobsystem
{
namespace
{
// Execution context only, not a worker pool or a service locator. A callback
// cannot start a blocking fork/join on a pool whose workers it may occupy.
thread_local unsigned s_batchDepth = 0;

class ParticipantScope
{
public:
	explicit ParticipantScope( const BatchDesc &desc ) : m_desc( desc )
	{
		++s_batchDepth;
		if ( m_desc.begin )
			m_desc.begin( m_desc.context );
	}

	~ParticipantScope()
	{
		if ( m_desc.end )
			m_desc.end( m_desc.context );
		--s_batchDepth;
	}

private:
	const BatchDesc &m_desc;
};
} // namespace

bool ExecuteParallelBatch( const BatchDesc &desc, IWorkerBackend *backend, BatchMode mode )
{
	if ( !desc.name || !desc.name[0] ||
	     ( mode != BatchMode::Serial && mode != BatchMode::Parallel ) )
		return false;
	if ( desc.count == 0 )
		return true;
	if ( !desc.process || desc.maxParticipants == 0 )
		return false;

	const bool serial = mode == BatchMode::Serial || s_batchDepth != 0;
	unsigned participants = 1;
	if ( !serial && backend && backend->WorkerCount() > 0 )
	{
		const unsigned capacity = static_cast<unsigned>( backend->WorkerCount() ) + 1u;
		participants = std::min(
		    { desc.count, desc.maxParticipants, capacity, static_cast<unsigned>( INT_MAX ) } );
	}

	// Only index reservation uses this atomic. Input publication and completion
	// visibility come from the backend's fork/join contract, not the cursor.
	// Use a wider cursor so the final unsuccessful claims cannot wrap even
	// when the descriptor contains UINT_MAX items. A reservation needs no CAS
	// retry: every value before count belongs to exactly one participant.
	std::atomic<uint64_t> next{ 0 };
	auto claim = [&]( unsigned &index )
	{
		const uint64_t reserved = next.fetch_add( 1, std::memory_order_relaxed );
		index = static_cast<unsigned>( reserved );
		return reserved < desc.count;
	};

	JobGraphBuilder builder;
	JobDesc complete;
	complete.name = "batch.complete";
	const JobHandle join = builder.AddJob( complete );
	for ( unsigned i = 0; i < participants; ++i )
	{
		JobDesc compute;
		compute.name = desc.name;
		compute.function = [&]( JobRunContext & )
		{
			if ( participants == 1 )
			{
				ParticipantScope scope( desc );
				for ( unsigned index = 0; index < desc.count; ++index )
					desc.process( desc.context, index );
				return;
			}
			unsigned index;
			if ( !claim( index ) )
				return;
			ParticipantScope scope( desc );
			do
			{
				desc.process( desc.context, index );
			} while ( claim( index ) );
		};
		builder.AddDependency( builder.AddJob( compute ), join );
	}
	auto graph = builder.Seal();
	if ( !graph.HasValue() )
		return false;

	// A constructed batch contains only infallible Compute nodes and a join.
	// There is no cancellation or affinity-dependent progress requirement.
	if ( serial )
		DeterministicExecutor().Execute( graph.Value(), RunOptions{} );
	else
		PooledExecutor( backend ).Execute( graph.Value(), RunOptions{} );
	return true;
}

} // namespace jobsystem
