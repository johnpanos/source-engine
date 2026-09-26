//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A single-thread task runner on a GLib main context.
//
//=============================================================================//

#include "glib_task_runner.h"

#include <glib.h>

#include <cstdlib>
#include <vector>

namespace hammer::gtk
{

// One posted task and its source. Owned by the runner's set until it runs
// or the runner shuts down.
struct GlibTaskRunner::Pending
{
	GlibTaskRunner *runner;
	platform::Task task;
	GSource *source = nullptr;
};

GlibTaskRunner::GlibTaskRunner( GMainContext *context )
    : m_context( context ? g_main_context_ref( context )
                         : g_main_context_ref( g_main_context_default() ) ),
      m_owner( std::this_thread::get_id() )
{
}

GlibTaskRunner::~GlibTaskRunner()
{
	Shutdown();
	g_main_context_unref( m_context );
}

platform::PostResult GlibTaskRunner::PostTask( platform::Task task )
{
	return PostDelayedTask( std::move( task ), 0 );
}

platform::PostResult GlibTaskRunner::PostDelayedTask(
    platform::Task task, std::uint64_t delayNanoseconds )
{
	std::unique_lock lock( m_mutex );
	if ( m_shutDown )
	{
		lock.unlock();
		return platform::PostResult::kShutDown;
	}
	// Timeouts round up to whole milliseconds, so a task never runs early.
	const guint delayMs = static_cast<guint>( ( delayNanoseconds + 999999 ) / 1000000 );
	auto *pending = new Pending{ this, std::move( task ) };
	pending->source = delayNanoseconds == 0 ? g_idle_source_new() : g_timeout_source_new( delayMs );
	g_source_set_priority( pending->source, G_PRIORITY_DEFAULT );
	g_source_set_callback( pending->source,
	    reinterpret_cast<GSourceFunc>( &GlibTaskRunner::Dispatch ), pending, nullptr );
	m_pending.insert( pending );
	// Attaching wakes the context's thread. Dispatch takes m_mutex before
	// touching the entry, so attaching under the lock is safe.
	g_source_attach( pending->source, m_context );
	return platform::PostResult::kAccepted;
}

int GlibTaskRunner::Dispatch( void *data )
{
	auto *pending = static_cast<Pending *>( data );
	GlibTaskRunner *runner = pending->runner;
	{
		std::lock_guard lock( runner->m_mutex );
		if ( runner->m_pending.erase( pending ) == 0 )
			return G_SOURCE_REMOVE; // dropped by Shutdown
	}
	pending->task();
	pending->task = platform::Task();
	g_source_unref( pending->source );
	delete pending;
	return G_SOURCE_REMOVE;
}

bool GlibTaskRunner::RunsTasksInCurrentSequence() const
{
	return BelongsToCurrentThread();
}

bool GlibTaskRunner::BelongsToCurrentThread() const
{
	return m_owner.load( std::memory_order_acquire ) == std::this_thread::get_id();
}

void GlibTaskRunner::Shutdown()
{
	if ( !BelongsToCurrentThread() )
	{
		std::lock_guard lock( m_mutex );
		if ( m_shutDown )
			return;   // already shut down
		std::abort(); // the owner shuts down on the context's thread
	}
	std::vector<Pending *> dropped;
	{
		std::lock_guard lock( m_mutex );
		m_shutDown = true;
		dropped.assign( m_pending.begin(), m_pending.end() );
		m_pending.clear();
	}
	for ( Pending *pending : dropped )
	{
		g_source_destroy( pending->source );
		g_source_unref( pending->source );
		delete pending; // the task is destroyed without running
	}
	m_owner.store( std::thread::id(), std::memory_order_release );
}

} // namespace hammer::gtk
