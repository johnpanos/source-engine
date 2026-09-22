//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: C++11-compatible, synchronous migration boundary for independent
//          compute batches. Implementation and graph storage remain C++20.
//
//=============================================================================//

#ifndef JOBSYSTEM_PARALLEL_BATCH_H
#define JOBSYSTEM_PARALLEL_BATCH_H

namespace jobsystem
{

class IWorkerBackend;

enum class BatchMode
{
	Serial,
	Parallel
};

struct BatchDesc
{
	const char *name = nullptr;
	void *context = nullptr;
	unsigned count = 0;
	void ( *process )( void *, unsigned ) = nullptr;
	void ( *begin )( void * ) = nullptr;
	void ( *end )( void * ) = nullptr;
	unsigned maxParticipants = 0;
};

// The caller gathers/pins inputs before entry and commits after return. Items
// must be independent; callbacks may not throw, block on this pool, or mutate
// shared unsynchronized state. context, callbacks and their module code remain
// borrowed until return. No callback is retained. Each index executes once.
//
// begin/end surround a participating runner's items on the same physical thread
// (e.g. model-cache pinning). Empty runners do not invoke either hook. No worker
// is created here: capacity is bounded by the supplied backend and descriptor.
// Nested batches execute inline to avoid a worker waiting on its own pool.
//
// Serial uses the deterministic graph executor, in ascending item order. The
// parallel path has a completion dependency after all compute runners. Return
// publishes all callback writes, including end hooks, to the caller. False means
// invalid input or graph construction failure BEFORE any callback executed;
// successful execution cannot fail halfway and must never be retried.
bool ExecuteParallelBatch( const BatchDesc &desc, IWorkerBackend *backend, BatchMode mode );

} // namespace jobsystem

#endif // JOBSYSTEM_PARALLEL_BATCH_H
