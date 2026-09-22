//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Scoped batch migration from legacy ParallelProcess to job graphs.
//          No new worker pool, no C++20 requirements in game/engine headers.
//
//=============================================================================//

#ifndef VSTDLIB_JOBGRAPH_PARALLEL_H
#define VSTDLIB_JOBGRAPH_PARALLEL_H

#include "jobsystem/parallel_batch.h"
#include "vstdlib/jobthread.h"

// Borrows the already composed pool for this call only; never starts/stops it.
// NULL means inline here. The templates below resolve the legacy default pool.
// No YieldWait or shared-queue helping is used. See parallel_batch.h for the
// callback, lifetime and nested-execution contract. Invalid input runs no work.
VSTDLIB_INTERFACE bool RunThreadPoolJobBatch(
    IThreadPool *pool, const jobsystem::BatchDesc &desc, jobsystem::BatchMode mode );

namespace jobgraph_detail
{
template <typename T, typename Processor> struct BatchBinding
{
	T *items;
	Processor processor;

	static void Process( void *context, unsigned index )
	{
		BatchBinding &binding = *static_cast<BatchBinding *>( context );
		binding.processor.Process( binding.items[index] );
	}
	static void Begin( void *context )
	{
		static_cast<BatchBinding *>( context )->processor.Begin();
	}
	static void End( void *context ) { static_cast<BatchBinding *>( context )->processor.End(); }

	bool Run( const char *name, unsigned count, int maxParallel, IThreadPool *pool,
	    jobsystem::BatchMode mode )
	{
		if ( count && !items )
			return false;
		jobsystem::BatchDesc desc;
		desc.name = name;
		desc.context = this;
		desc.count = count;
		desc.process = &Process;
		desc.begin = &Begin;
		desc.end = &End;
		desc.maxParticipants = maxParallel > 0 ? static_cast<unsigned>( maxParallel ) + 1u : 1u;
		return RunThreadPoolJobBatch( pool ? pool : g_pThreadPool, desc, mode );
	}
};
} // namespace jobgraph_detail

template <typename T>
bool JobGraphParallelProcess( const char *name, T *items, unsigned count, void ( *process )( T & ),
    void ( *begin )() = NULL, void ( *end )() = NULL, int maxParallel = INT_MAX,
    IThreadPool *pool = NULL, jobsystem::BatchMode mode = jobsystem::BatchMode::Parallel )
{
	if ( count && !process )
		return false;
	jobgraph_detail::BatchBinding<T, CFuncJobItemProcessor<T>> binding;
	binding.items = items;
	binding.processor.Init( process, begin, end );
	return binding.Run( name, count, maxParallel, pool, mode );
}

template <typename T, class Object, class FunctionClass>
bool JobGraphParallelProcess( const char *name, T *items, unsigned count, Object *object,
    void ( FunctionClass::*process )( T & ), void ( FunctionClass::*begin )() = NULL,
    void ( FunctionClass::*end )() = NULL, int maxParallel = INT_MAX, IThreadPool *pool = NULL,
    jobsystem::BatchMode mode = jobsystem::BatchMode::Parallel )
{
	if ( count && ( !object || !process ) )
		return false;
	jobgraph_detail::BatchBinding<T, CMemberFuncJobItemProcessor<T, Object, FunctionClass>> binding;
	binding.items = items;
	binding.processor.Init( object, process, begin, end );
	return binding.Run( name, count, maxParallel, pool, mode );
}

#endif // VSTDLIB_JOBGRAPH_PARALLEL_H
