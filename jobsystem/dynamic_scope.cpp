//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: DynamicScope implementation (RFC 0003, "Dynamic work").
//
//          A live scheduler over a growable child list. The mutex both serializes
//          state and provides the publication edge from a producing child's output
//          writes (before it locks to resolve) to a consuming continuation's reads
//          (after it locks to dequeue), the same happens-before argument the sealed
//          parallel executor uses.
//
//=============================================================================//

#include "jobsystem/dynamic_scope.h"

namespace jobsystem
{

DynamicScope::DynamicScope( int nWorkers, const FrameContext &frame )
    : m_nWorkers( nWorkers < 0 ? 0 : nWorkers ), m_frame( frame )
{
	m_workers.reserve( (size_t)m_nWorkers );
	for ( int i = 0; i < m_nWorkers; ++i )
		m_workers.emplace_back(
		    [this]
		    {
			    WorkerLoop();
		    } );
}

DynamicScope::~DynamicScope()
{
	// If the owner never called Wait(), release the keepalive so workers can exit.
	{
		std::lock_guard<std::mutex> lk( m_mtx );
		if ( !m_waitStarted )
		{
			m_waitStarted = true;
			if ( m_outstanding > 0 )
				--m_outstanding;
			m_cv.notify_all();
		}
	}
	for ( std::thread &t : m_workers )
		if ( t.joinable() )
			t.join();
}

void DynamicScope::ResolveLocked( std::unique_lock<std::mutex> &lk, uint32_t id, JobState terminal )
{
	m_children[id].state = terminal;
	for ( uint32_t d : m_children[id].dependents )
	{
		// A Success dep whose producer did not succeed cancels the dependent.
		if ( terminal != JobState::Succeeded )
			m_children[d].willCancel = true;
		if ( --m_children[d].remaining == 0 )
			m_ready.push_back( d );
	}
	if ( m_outstanding > 0 )
		--m_outstanding;
	(void)lk;
	m_cv.notify_all();
}

DynamicScope::ChildHandle DynamicScope::Spawn(
    const char *name, JobEntry fn, const std::vector<ChildHandle> &deps, ExecutorToken exec )
{
	std::unique_lock<std::mutex> lk( m_mtx );

	// No completion ownership remains: the scope has drained and admits no more.
	if ( m_outstanding == 0 )
		return ChildHandle{ ChildHandle::kInvalid };

	const uint32_t id = (uint32_t)m_children.size();
	Child c;
	c.name = ( name && name[0] ) ? name : "child";
	c.fn = std::move( fn );
	c.exec = exec;
	c.remaining = 0;
	c.willCancel = m_scopeCanceled;
	c.state = JobState::Admitted;
	m_children.push_back( std::move( c ) );

	for ( ChildHandle d : deps )
	{
		// A child may depend only on already-admitted children.
		if ( !d.IsValid() || d.id >= id )
			continue;
		if ( IsTerminal( m_children[d.id].state ) )
		{
			// Already-resolved dependency: account its outcome immediately.
			if ( m_children[d.id].state != JobState::Succeeded )
				m_children[id].willCancel = true;
		}
		else
		{
			m_children[d.id].dependents.push_back( id );
			m_children[id].remaining++;
		}
	}

	++m_outstanding; // this child now holds completion ownership

	if ( m_children[id].remaining == 0 )
		m_ready.push_back( id );

	m_cv.notify_all();
	return ChildHandle{ id };
}

void DynamicScope::CancelPending()
{
	std::lock_guard<std::mutex> lk( m_mtx );
	m_scopeCanceled = true;
	m_cv.notify_all();
}

void DynamicScope::WorkerLoop()
{
	std::unique_lock<std::mutex> lk( m_mtx );
	for ( ;; )
	{
		m_cv.wait( lk,
		    [&]
		    {
			    return !m_ready.empty() || m_outstanding == 0;
		    } );
		if ( m_ready.empty() )
		{
			if ( m_outstanding == 0 )
				return;
			continue;
		}

		const uint32_t id = m_ready.back();
		m_ready.pop_back();

		if ( m_children[id].willCancel || m_scopeCanceled )
		{
			ResolveLocked( lk, id, JobState::Canceled );
			continue;
		}

		JobEntry fn = m_children[id].fn; // copy so the slot can be touched meanwhile
		JobState terminal;
		{
			lk.unlock();
			JobRunContext ctx( m_frame, id );
			if ( fn )
				fn( ctx );
			terminal = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
			lk.lock();
		}
		ResolveLocked( lk, id, terminal );
	}
}

void DynamicScope::Wait()
{
	std::unique_lock<std::mutex> lk( m_mtx );
	if ( m_waitStarted )
		return; // Wait() is single-shot
	m_waitStarted = true;

	// Release the owner keepalive reserved at construction. The scope now completes
	// once every admitted child is terminal.
	if ( m_outstanding > 0 )
		--m_outstanding;
	m_cv.notify_all();

	if ( m_nWorkers == 0 )
	{
		// No worker threads: the owner drains the ready set itself. This still
		// obeys the contract because there are no workers to block; the single
		// owner thread is the executor.
		while ( !m_ready.empty() )
		{
			const uint32_t id = m_ready.back();
			m_ready.pop_back();
			if ( m_children[id].willCancel || m_scopeCanceled )
			{
				ResolveLocked( lk, id, JobState::Canceled );
				continue;
			}
			JobEntry fn = m_children[id].fn;
			JobState terminal;
			{
				lk.unlock();
				JobRunContext ctx( m_frame, id );
				if ( fn )
					fn( ctx );
				terminal = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
				lk.lock();
			}
			ResolveLocked( lk, id, terminal );
		}
	}
	else
	{
		m_cv.wait( lk,
		    [&]
		    {
			    return m_outstanding == 0;
		    } );
	}

	// Publish final counters for lock-free reads.
	m_admittedCount = (uint32_t)m_children.size();
	m_succeeded = m_failed = m_canceled = 0;
	for ( const Child &c : m_children )
	{
		switch ( c.state )
		{
		case JobState::Succeeded:
			++m_succeeded;
			break;
		case JobState::Failed:
			++m_failed;
			break;
		case JobState::Canceled:
			++m_canceled;
			break;
		default:
			break;
		}
	}
}

JobState DynamicScope::StateOf( ChildHandle h ) const
{
	std::lock_guard<std::mutex> lk( m_mtx );
	if ( !h.IsValid() || h.id >= m_children.size() )
		return JobState::Admitted;
	return m_children[h.id].state;
}

} // namespace jobsystem
