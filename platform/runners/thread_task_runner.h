//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native single-thread task runner (platform.runners): one owned
//			thread runs the tasks, in (due time, post order) order, with delays
//			measured on the steady clock. For APIs with real thread affinity and
//			for blocking work that must stay off a caller's thread; ordered work
//			without affinity should use SequencedTaskRunner over a shared runner.
//
//			The owner shuts it down (or destroys it) from any thread but its
//			own: later posts are refused, pending tasks are destroyed without
//			running, and Shutdown returns after the running task finishes and
//			the thread has exited.
//
//=============================================================================//

#ifndef PLATFORM_RUNNERS_THREAD_TASK_RUNNER_H
#define PLATFORM_RUNNERS_THREAD_TASK_RUNNER_H

#include "platform/contracts/task_runner.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace platform
{

class ThreadTaskRunner final : public ISingleThreadTaskRunner
{
public:
	// Starts the thread. `name` is diagnostic only.
	explicit ThreadTaskRunner( std::string name );
	~ThreadTaskRunner() override;

	[[nodiscard]] PostResult PostTask( Task task ) override;
	[[nodiscard]] PostResult PostDelayedTask( Task task, std::uint64_t delayNanoseconds ) override;
	bool RunsTasksInCurrentSequence() const override;
	bool BelongsToCurrentThread() const override;

	const std::string &Name() const { return m_name; }

	// See the file comment. Calling it from the runner's own thread aborts.
	void Shutdown();

private:
	using Clock = std::chrono::steady_clock;
	struct Entry
	{
		Clock::time_point due;
		std::uint64_t sequence = 0;
		Task task;
	};

	PostResult Post( Task task, std::uint64_t delayNanoseconds );
	void Run();

	const std::string m_name;
	std::mutex m_mutex;
	std::condition_variable m_wake;
	std::vector<Entry> m_pending; // a min-heap on (due, sequence)
	std::uint64_t m_nextSequence = 0;
	bool m_shutDown = false;
	std::once_flag m_joinOnce;
	std::thread m_thread;
	// The running thread's id, cleared once it has exited: thread ids are
	// reused, so a later thread must not appear to belong to this runner.
	std::atomic<std::thread::id> m_threadId{};
};

} // namespace platform

#endif // PLATFORM_RUNNERS_THREAD_TASK_RUNNER_H
