//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: An IWorkerBackend over a fixed set of its own threads, for test and
//          tool roots that have no engine pool (RFC 0003). Products lend the
//          engine's compute pool instead (vstdlib/jobgraph_pool_bridge.h); a
//          provider never creates one of these for itself (RFC 0003 J3).
//
//          Tasks queue in one synchronized FIFO, the simple queue RFC 0003
//          prefers until a more complex one has evidence. Workers start in the
//          constructor and are joined in the destructor, which first runs or
//          withdraws nothing: every accepted ticket must have been settled.
//
//=============================================================================//

#ifndef JOBSYSTEM_THREAD_WORKER_BACKEND_H
#define JOBSYSTEM_THREAD_WORKER_BACKEND_H

#include <cstdint>
#include <memory>

#include "jobsystem/worker_backend.h"

namespace jobsystem
{

class ThreadWorkerBackend final : public IWorkerBackend
{
public:
	// workers <= 0 starts no thread: WorkerCount() is 0 and every post is
	// refused, so schedulers run inline.
	explicit ThreadWorkerBackend( int workers );
	~ThreadWorkerBackend() override;

	ThreadWorkerBackend( const ThreadWorkerBackend & ) = delete;
	ThreadWorkerBackend &operator=( const ThreadWorkerBackend & ) = delete;

	int WorkerCount() const override;
	void *PostTask( WorkerTaskFn task, void *context ) override;
	bool SettleTask( void *ticket ) override;
	bool ShouldRunInline() override;

	// Counters since construction, for tests.
	uint64_t TasksPosted() const;
	uint64_t TasksRun() const;
	uint64_t TasksWithdrawn() const;

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};

} // namespace jobsystem

#endif // JOBSYSTEM_THREAD_WORKER_BACKEND_H
