//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine thread pool as a platform.task-runner.v1 runner (RFC
//			0001 rank 11: "Adapt existing queues; do not replace the job system
//			wholesale"; roadmap R10, R10-POOLRUNNER).
//
//			The runner posts each task to a borrowed CThreadPool as a queued
//			job (JF_QUEUE: never run inside the post), so independent work and,
//			through platform::SequencedTaskRunner, ordered sequences run on the
//			pool the engine already has, without new threads. Delays wait on a
//			timer runner the composition root injects (a thread runner, the
//			host's clock-driven runner, or virtual time in tests); when due,
//			the task is posted to the pool.
//
//			Shutdown (explicit, or by destroying the runner) refuses later
//			posts, destroys queued and delayed tasks without running them, and
//			returns once no task of the runner is running. Jobs already queued
//			in the pool or the timer then find nothing to do. The pool and the
//			timer must outlive the runner, and the pool must be started with at
//			least one thread (a pool without threads would run every job inside
//			the post); otherwise the factory returns null.
//
//			This header only declares the factory, so any engine target can use
//			it without the C++20 contract headers.
//
//=============================================================================//

#ifndef VSTDLIB_TASK_RUNNER_POOL_BRIDGE_H
#define VSTDLIB_TASK_RUNNER_POOL_BRIDGE_H

#ifdef _WIN32
#pragma once
#endif

#include "vstdlib/vstdlib.h"

class IThreadPool;
namespace platform
{
class ITaskRunner;
}

VSTDLIB_INTERFACE platform::ITaskRunner *CreateThreadPoolTaskRunner(
    IThreadPool *pPool, platform::ITaskRunner *pDelayTimer );
// The owner's shutdown, for teardown ordering; the runner stays valid (it
// refuses every post) until destroyed. Not from one of the runner's tasks.
VSTDLIB_INTERFACE void ShutdownThreadPoolTaskRunner( platform::ITaskRunner *pRunner );
VSTDLIB_INTERFACE void DestroyThreadPoolTaskRunner( platform::ITaskRunner *pRunner );

#endif // VSTDLIB_TASK_RUNNER_POOL_BRIDGE_H
