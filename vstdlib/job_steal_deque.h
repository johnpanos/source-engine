//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded per-worker work-stealing deque for CThreadPool.
//
//          Topology: one owner, the worker thread whose deque it is, pushes
//          and pops at the bottom (LIFO: cache-warm, depth-first for
//          recursive spawns). Any other worker of the same pool steals from
//          the top (FIFO: the oldest entry). No other thread pushes or pops.
//
//          Capacity: fixed at CAPACITY entries. PushBottom fails when full and
//          the caller spills the entry to the pool's shared injection queue,
//          so admitted work is never dropped, blocked on, or run early.
//
//          Payload lifetime: entries are opaque values. CThreadPool stores one
//          owned job reference per entry and releases it after servicing,
//          aborting or spilling that entry.
//
//          Ordering: none between entries. JF_SERIAL and thread-affine jobs
//          never enter a steal deque.
//
//          Close/drain: the pool pops every entry while it is quiescent
//          (suspended or stopped) and aborts or requeues it.
//
//          Synchronization: one short critical section per operation. A thief
//          touches a deque only after its own queues are empty, so contention
//          is rare; a lock-free (Chase-Lev) deque needs stress, sanitizer and
//          performance evidence before it replaces this.
//
//=============================================================================

#ifndef JOB_STEAL_DEQUE_H
#define JOB_STEAL_DEQUE_H

#if defined( _WIN32 )
#pragma once
#endif

#include "tier0/threadtools.h"

template <typename T, unsigned CAPACITY> class CJobStealDeque
{
public:
	CJobStealDeque() : m_nTop( 0 ), m_nBottom( 0 ), m_nCount( 0 )
	{
		COMPILE_TIME_ASSERT( CAPACITY > 0 && ( CAPACITY & ( CAPACITY - 1 ) ) == 0 );
	}

	// Owner only. False when full; the entry was not stored.
	bool PushBottom( const T &item )
	{
		AUTO_LOCK( m_Mutex );
		if ( m_nBottom - m_nTop == CAPACITY )
			return false;
		m_Items[m_nBottom % CAPACITY] = item;
		++m_nBottom;
		PublishCountLocked();
		return true;
	}

	// Owner only. Most recently pushed entry.
	bool PopBottom( T *pItem )
	{
		AUTO_LOCK( m_Mutex );
		if ( m_nBottom == m_nTop )
			return false;
		--m_nBottom;
		*pItem = m_Items[m_nBottom % CAPACITY];
		PublishCountLocked();
		return true;
	}

	// Any other worker. Oldest entry.
	bool StealTop( T *pItem )
	{
		AUTO_LOCK( m_Mutex );
		if ( m_nBottom == m_nTop )
			return false;
		*pItem = m_Items[m_nTop % CAPACITY];
		++m_nTop;
		PublishCountLocked();
		return true;
	}

	// Atomic observation; may be stale when it returns. Used only to decide
	// whether to look, never as proof of ownership.
	int Count() const { return ThreadAtomicLoad( &m_nCount ); }

	static unsigned Capacity() { return CAPACITY; }

private:
	void PublishCountLocked() { ThreadAtomicStore( &m_nCount, (int)( m_nBottom - m_nTop ) ); }

	CThreadFastMutex m_Mutex;
	unsigned m_nTop;
	unsigned m_nBottom;
	volatile int m_nCount;
	T m_Items[CAPACITY];
};

#endif // JOB_STEAL_DEQUE_H
