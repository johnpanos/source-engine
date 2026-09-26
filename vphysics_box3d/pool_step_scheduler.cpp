//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D's worker tasks on the application's thread pool; see
//          pool_step_scheduler.h.
//
//===========================================================================//

#include "pool_step_scheduler.h"

#include <chrono>

#include "tier0/dbg.h"
#include "vstdlib/jobthread.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

namespace
{
// How long an idle helper spins before it sleeps: Box3D publishes a step's
// tasks microseconds apart, so a brief spin keeps a helper on its core
// between them, while a longer idle stretch releases the core.
const auto kHelperSpin = std::chrono::microseconds( 20 );

inline void CpuRelax()
{
#if defined( __aarch64__ ) || defined( __arm__ )
	__asm__ __volatile__( "yield" ::: "memory" );
#elif defined( __x86_64__ ) || defined( __i386__ )
	__builtin_ia32_pause();
#endif
}
} // namespace

CPoolStepScheduler::CPoolStepScheduler( IThreadPool *pPool, int workerCount )
    : m_pPool( pPool ), m_helperCount( workerCount - 1 )
{
	Assert( pPool && workerCount > 1 && workerCount <= B3_MAX_WORKERS );
	for ( CJob *&pHelper : m_helpers )
		pHelper = nullptr;
}

CPoolStepScheduler::~CPoolStepScheduler()
{
	Assert( m_stepDone.load() );
}

void CPoolStepScheduler::BeginStep()
{
	Assert( m_stepDone.load() );
	// Every task of the previous step completed before it returned.
	const int used = m_nextSlot.load();
	for ( int i = 0; i < used; ++i )
		m_slots[i].status.store( kFree, std::memory_order_relaxed );
	m_nextSlot.store( 0 );
	m_stepThread.store( ThreadGetCurrentId() );
	m_stepDone.store( false );
	for ( int i = 0; i < m_helperCount; ++i )
		m_helpers[i] = m_pPool->QueueCall( &CPoolStepScheduler::HelperEntry, this );
}

void CPoolStepScheduler::EndStep()
{
	m_stepDone.store( true );
	m_wake.release( m_helperCount );
	for ( int i = 0; i < m_helperCount; ++i )
	{
		if ( !m_helpers[i] )
			continue;
		// A helper the pool never started runs here and returns at once.
		m_helpers[i]->WaitForFinish( TT_INFINITE, m_pPool );
		m_helpers[i]->Release();
		m_helpers[i] = nullptr;
	}
	// Permits no helper consumed (tasks drained by others, helpers that never
	// waited) do not carry into the next step.
	while ( m_wake.try_acquire() )
	{
	}
}

void *CPoolStepScheduler::Enqueue(
    b3TaskCallback *pTask, void *pTaskContext, void *pUserContext, const char * )
{
	return static_cast<CPoolStepScheduler *>( pUserContext )->Add( pTask, pTaskContext );
}

void CPoolStepScheduler::Finish( void *pUserTask, void *pUserContext )
{
	if ( pUserTask )
		static_cast<CPoolStepScheduler *>( pUserContext )
		    ->Wait( *static_cast<Slot *>( pUserTask ) );
}

void *CPoolStepScheduler::Add( b3TaskCallback *pTask, void *pTaskContext )
{
	// Outside a step no helper exists; Box3D enqueues at most B3_MAX_TASKS per
	// step. Either way the task runs here (NULL tells Box3D it already ran).
	const int index = m_stepDone.load() ? B3_MAX_TASKS : m_nextSlot.fetch_add( 1 );
	if ( index >= B3_MAX_TASKS )
	{
		pTask( pTaskContext );
		return nullptr;
	}
	Slot &slot = m_slots[index];
	slot.pTask = pTask;
	slot.pContext = pTaskContext;
	// Publishes the task and its context to the claiming thread.
	slot.status.store( kPending, std::memory_order_release );
	m_wake.release();
	return &slot;
}

void CPoolStepScheduler::Wait( Slot &slot )
{
	// Help with the step's pending tasks until this one has completed; a task
	// another thread runs is short (a block range or a solver worker).
	while ( slot.status.load( std::memory_order_acquire ) != kComplete )
	{
		if ( !RunOne() )
			CpuRelax();
	}
}

bool CPoolStepScheduler::RunOne()
{
	const int count = std::min( m_nextSlot.load(), int( B3_MAX_TASKS ) );
	for ( int i = 0; i < count; ++i )
	{
		Slot &slot = m_slots[i];
		int expected = kPending;
		if ( slot.status.load( std::memory_order_relaxed ) != kPending ||
		     !slot.status.compare_exchange_strong( expected, kClaimed, std::memory_order_acquire ) )
			continue;
		slot.pTask( slot.pContext );
		slot.status.store( kComplete, std::memory_order_release );
		return true;
	}
	return false;
}

bool CPoolStepScheduler::HasPending()
{
	const int count = std::min( m_nextSlot.load(), int( B3_MAX_TASKS ) );
	for ( int i = 0; i < count; ++i )
	{
		if ( m_slots[i].status.load( std::memory_order_relaxed ) == kPending )
			return true;
	}
	return false;
}

void CPoolStepScheduler::HelperMain()
{
	if ( ThreadGetCurrentId() == m_stepThread.load() )
		return;
	for ( ;; )
	{
		while ( RunOne() )
		{
		}
		if ( m_stepDone.load() )
			return;
		const auto spinUntil = std::chrono::steady_clock::now() + kHelperSpin;
		while (
		    !HasPending() && !m_stepDone.load() && std::chrono::steady_clock::now() < spinUntil )
			CpuRelax();
		if ( !HasPending() && !m_stepDone.load() )
			m_wake.acquire();
	}
}

void CPoolStepScheduler::HelperEntry( CPoolStepScheduler *pScheduler )
{
	pScheduler->HelperMain();
}
