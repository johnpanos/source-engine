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

	// Shared by every participant for this call; each job captures only a
	// pointer to it, which keeps the job functions allocation-free.
	struct Run
	{
		const BatchDesc &desc;
		unsigned participants;
		// Only index reservation uses this atomic. Input publication and
		// completion visibility come from the backend's fork/join contract, not
		// the cursor. The cursor never passes count, so it cannot wrap.
		std::atomic<uint64_t> next{ 0 };

		// Guided self-scheduling: claim a contiguous range sized to a fraction
		// of the remaining items per participant. Early claims are large (few
		// contended cursor updates, no neighboring-item sharing between
		// runners) and shrink to single items at the end for load balance.
		bool Claim( unsigned &begin, unsigned &end )
		{
			uint64_t cursor = next.load( std::memory_order_relaxed );
			for ( ;; )
			{
				if ( cursor >= desc.count )
					return false;
				const uint64_t remaining = desc.count - cursor;
				const uint64_t chunk = std::max<uint64_t>( 1, remaining / ( 4ull * participants ) );
				if ( next.compare_exchange_weak( cursor, cursor + chunk, std::memory_order_relaxed ) )
				{
					begin = static_cast<unsigned>( cursor );
					end = static_cast<unsigned>( cursor + chunk );
					return true;
				}
			}
		}

		void Participate()
		{
			// The descriptor is immutable for the call; keep its fields local.
			void ( *const process )( void *, unsigned ) = desc.process;
			void *const context = desc.context;
			if ( participants == 1 )
			{
				ParticipantScope scope( desc );
				const unsigned count = desc.count;
				for ( unsigned index = 0; index < count; ++index )
					process( context, index );
				return;
			}
			unsigned begin, end;
			if ( !Claim( begin, end ) )
				return;
			ParticipantScope scope( desc );
			do
			{
				for ( unsigned index = begin; index < end; ++index )
					process( context, index );
			} while ( Claim( begin, end ) );
		}
	} run{ desc, participants };
	Run *shared = &run;

	JobGraphBuilder builder;
	JobDesc complete;
	complete.name = "batch.complete";
	const JobHandle join = builder.AddJob( complete );
	JobDesc compute;
	compute.name = desc.name;
	compute.function = [shared]( JobRunContext & ) { shared->Participate(); };
	for ( unsigned i = 0; i < participants; ++i )
		builder.AddDependency( builder.AddJob( compute ), join );
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
