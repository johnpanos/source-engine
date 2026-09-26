//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A platform.task-runner.v1 single-thread runner on a GLib main
//			context (RFC 0002 GTK host; R08-ASYNC-BUILD). Tasks run from idle
//			sources (timeout sources for delays) on the thread that iterates the
//			context, which for the GTK shell is the UI thread. Posting is safe
//			from any thread.
//
//			The owner constructs the runner on the context's thread and shuts
//			it down (or destroys it) there: pending sources are destroyed and
//			their tasks are dropped without running.
//
//=============================================================================//

#ifndef HAMMER_GTK_GLIB_TASK_RUNNER_H
#define HAMMER_GTK_GLIB_TASK_RUNNER_H

#include "platform/contracts/task_runner.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_set>

typedef struct _GMainContext GMainContext;

namespace hammer::gtk
{

class GlibTaskRunner final : public platform::ISingleThreadTaskRunner
{
public:
	// `context` null means the default main context. The calling thread is the
	// one that iterates it.
	explicit GlibTaskRunner( GMainContext *context = nullptr );
	~GlibTaskRunner() override;

	[[nodiscard]] platform::PostResult PostTask( platform::Task task ) override;
	[[nodiscard]] platform::PostResult PostDelayedTask(
	    platform::Task task, std::uint64_t delayNanoseconds ) override;
	bool RunsTasksInCurrentSequence() const override;
	bool BelongsToCurrentThread() const override;

	// On the context's thread only.
	void Shutdown();

	struct Pending;

private:
	static int Dispatch( void *data );

	GMainContext *m_context;
	std::mutex m_mutex;
	std::unordered_set<Pending *> m_pending;
	bool m_shutDown = false;
	std::atomic<std::thread::id> m_owner;
};

} // namespace hammer::gtk

#endif // HAMMER_GTK_GLIB_TASK_RUNNER_H
