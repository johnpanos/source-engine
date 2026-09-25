//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Opt-in parallel stepping capability (RFC 0013, vphysics.parallel-step.v1).
//
//          A provider that can step one environment on several workers exposes
//          this interface through IPhysics::QueryInterface. A provider without
//          it (IVP) returns NULL, and callers keep using
//          IPhysics::CreateEnvironment. Nothing in IPhysics or
//          IPhysicsEnvironment changes, so the legacy ABI is untouched.
//
//          The worker count is fixed when the environment is created: it is a
//          composition choice, not a runtime toggle. Destroy these
//          environments with IPhysics::DestroyEnvironment as usual.
//
//          Worker tasks run on a thread pool the caller owns and passes in
//          (the engine's compute pool in products), so the pool's size is the
//          physics worker budget. The provider starts no threads of its own.
//          The pool must outlive the environment. A pool that has been
//          stopped still works: its jobs then run on the waiting thread.
//
//          Contract (every claiming provider):
//          - CreateParallelEnvironment returns NULL and creates nothing when
//            workerCount < 1 or > GetMaxWorkerCount(), when workerCount > 1
//            and pThreadPool is NULL, or when workerCount exceeds the pool's
//            threads plus the calling thread.
//          - Stepping creates no threads; worker tasks run on pThreadPool or
//            on the thread that called Simulate. Waiting for a task runs only
//            that task, never unrelated pool work, so Simulate may be called
//            from inside a pool job without deadlock.
//          - GetWorkerCount returns the created count for an environment this
//            provider created, and 0 for any other pointer (including NULL).
//          - Game callbacks (IPhysicsCollisionSolver, event handlers,
//            controllers) are never entered concurrently. Event handlers and
//            controllers run on the thread that called Simulate. The collision
//            solver may be entered from a worker thread, one call at a time,
//            while that thread is blocked in Simulate.
//          - When IsWorkerCountInvariant() is true, a fixed call sequence
//            produces bitwise-identical object state for every accepted
//            worker count.
//
//=============================================================================//

#ifndef VPHYSICS_PARALLEL_STEP_H
#define VPHYSICS_PARALLEL_STEP_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class IPhysicsEnvironment;
class IThreadPool;

#define VPHYSICS_PARALLEL_STEP_INTERFACE_VERSION "VPhysicsParallelStep001"

struct physics_parallelparams_t
{
	int workerCount;          // workers stepping this environment; 1 is the serial step
	IThreadPool *pThreadPool; // runs worker tasks; required when workerCount > 1

	void Defaults()
	{
		workerCount = 1;
		pThreadPool = NULL;
	}
};

abstract_class IPhysicsParallelStep
{
public:
	// Largest worker count accepted for one environment (at least 1).
	virtual int GetMaxWorkerCount() const = 0;

	virtual IPhysicsEnvironment *CreateParallelEnvironment(
	    const physics_parallelparams_t &params ) = 0;

	virtual int GetWorkerCount( const IPhysicsEnvironment *pEnvironment ) const = 0;

	virtual bool IsWorkerCountInvariant() const = 0;
};

#endif // VPHYSICS_PARALLEL_STEP_H
