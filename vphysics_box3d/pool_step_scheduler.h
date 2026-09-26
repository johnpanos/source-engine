//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D's worker tasks on the application's thread pool (RFC 0013
//          P2), one step at a time.
//
// Queuing each Box3D task as its own pool job and waiting for the jobs in
// turn put a pool wake-up and a sleeping wait on the step's critical path for
// every task: the step barely scaled on a phone (1.0x at four workers on an
// iPhone 16 Pro, where Box3D's own scheduler gives 1.5x). This scheduler
// works the way Box3D's does, but on borrowed pool threads:
//
//   * BeginStep queues workerCount - 1 helper jobs. Each helper drains the
//     step's task slots and, when none is pending, spins briefly and then
//     waits on a semaphore that each new task signals.
//   * Enqueue publishes a Box3D task in a slot and wakes one helper.
//   * Finish claims and runs pending tasks (any of the step's, not only the
//     one waited for) until its task has completed; the caller never sleeps
//     mid-step.
//   * EndStep releases the helpers and waits for their jobs. A helper the
//     pool never started runs on the caller, sees the step done and returns,
//     so a step completes even when no pool thread is free.
//
// The provider starts no thread; helpers are the pool's. Box3D tasks are
// leaves (none enqueues or waits for another), so helpers never wait on other
// pool work. Task order does not affect Box3D's results.
//
//===========================================================================//

#ifndef VPHYSICS_BOX3D_POOL_STEP_SCHEDULER_H
#define VPHYSICS_BOX3D_POOL_STEP_SCHEDULER_H

#include <atomic>
#include <semaphore>

#include "box3d/box3d.h"
#include "tier0/threadtools.h"

class CJob;
class IThreadPool;

class CPoolStepScheduler
{
public:
	// `pPool` outlives this object; `workerCount` counts the stepping thread.
	CPoolStepScheduler( IThreadPool *pPool, int workerCount );
	~CPoolStepScheduler();
	CPoolStepScheduler( const CPoolStepScheduler & ) = delete;
	CPoolStepScheduler &operator=( const CPoolStepScheduler & ) = delete;

	// Around each b3World_Step, on the stepping thread.
	void BeginStep();
	void EndStep();

	// b3WorldDef::enqueueTask and finishTask; the user context is this object.
	static void *Enqueue(
	    b3TaskCallback *pTask, void *pTaskContext, void *pUserContext, const char *pName );
	static void Finish( void *pUserTask, void *pUserContext );

private:
	enum Status
	{
		kFree,
		kPending,
		kClaimed,
		kComplete
	};
	struct Slot
	{
		b3TaskCallback *pTask = nullptr;
		void *pContext = nullptr;
		std::atomic<int> status{ kFree };
	};

	void *Add( b3TaskCallback *pTask, void *pTaskContext );
	void Wait( Slot &slot );
	// Claims and runs one pending task; false when none is pending.
	bool RunOne();
	bool HasPending();
	void HelperMain();
	static void HelperEntry( CPoolStepScheduler *pScheduler );

	IThreadPool *m_pPool;
	int m_helperCount;
	Slot m_slots[B3_MAX_TASKS];
	std::atomic<int> m_nextSlot{ 0 };
	std::atomic<bool> m_stepDone{ true };
	// One permit per published task, plus one per helper at the end of a step.
	std::counting_semaphore<> m_wake{ 0 };
	// The thread stepping the world: a helper the pool runs on it (a pool
	// without threads, or without an idle one, runs a queued job at once)
	// returns at once, since that thread does the step's work itself.
	std::atomic<ThreadId_t> m_stepThread{ 0 };
	CJob *m_helpers[B3_MAX_WORKERS];
};

#endif // VPHYSICS_BOX3D_POOL_STEP_SCHEDULER_H
