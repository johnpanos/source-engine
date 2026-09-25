//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Per-thread batch nesting depth, shared by the batch executors
//          (parallel_batch.cpp, declared_frame_graph.cpp). A batch started from
//          inside a batch item runs inline: its thread may be one of the
//          backend's workers, which must not wait on its own pool.
//
//=============================================================================//

#ifndef JOBSYSTEM_BATCH_DEPTH_H
#define JOBSYSTEM_BATCH_DEPTH_H

namespace jobsystem
{
namespace detail
{

// Nonzero while this thread runs batch items. Execution context only, not a
// worker pool or a service locator.
unsigned &BatchDepth();

class BatchDepthScope
{
public:
	BatchDepthScope() { ++BatchDepth(); }
	~BatchDepthScope() { --BatchDepth(); }

private:
	BatchDepthScope( const BatchDepthScope & ) = delete;
	BatchDepthScope &operator=( const BatchDepthScope & ) = delete;
};

} // namespace detail
} // namespace jobsystem

#endif // JOBSYSTEM_BATCH_DEPTH_H
