//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Virtual-time providers (RFC 0001: "clocks and delayed scheduling
//			are injectable so tests can use virtual time"; platform.runners).
//
//			VirtualClock is an IMonotonicClock whose time moves only when its
//			owner advances it (1 tick = 1 ns). ManualTaskRunner is a sequenced
//			runner on such a clock: nothing runs until the owner calls
//			RunUntilIdle or AdvanceBy on its own thread, so a consumer's tests
//			are deterministic and never sleep. Tasks run on the thread that
//			drives the runner.
//
//=============================================================================//

#ifndef PLATFORM_RUNNERS_MANUAL_TASK_RUNNER_H
#define PLATFORM_RUNNERS_MANUAL_TASK_RUNNER_H

#include "platform/contracts/clock.h"
#include "platform/contracts/task_runner.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

namespace platform
{

class VirtualClock final : public IMonotonicClock
{
public:
	MonotonicTimestamp Now() const override;
	std::uint64_t ResolutionNanoseconds() const override { return 1; }
	std::uint64_t ElapsedNanoseconds(
	    MonotonicTimestamp begin, MonotonicTimestamp end ) const override
	{
		return end.ticks - begin.ticks;
	}

	void AdvanceBy( std::uint64_t nanoseconds );
	// Moves to `ticks` if that is later than now; time never moves back.
	void AdvanceTo( std::uint64_t ticks );

private:
	std::atomic<std::uint64_t> m_ticks{ 0 };
};

class ManualTaskRunner final : public ISequencedTaskRunner
{
public:
	// Borrows the clock, which must outlive the runner.
	explicit ManualTaskRunner( VirtualClock &clock );
	~ManualTaskRunner() override;

	[[nodiscard]] PostResult PostTask( Task task ) override;
	[[nodiscard]] PostResult PostDelayedTask( Task task, std::uint64_t delayNanoseconds ) override;
	bool RunsTasksInCurrentSequence() const override;

	// Runs every task due at the current virtual time, including tasks those
	// tasks post, and returns how many ran. Must not be called from a task.
	int RunUntilIdle();
	// Advances the clock by `nanoseconds`, stopping at each due time on the way
	// to run the tasks due then (a task sees the clock at its due time).
	int AdvanceBy( std::uint64_t nanoseconds );
	// Pending tasks, due or not.
	size_t PendingCount() const;

	// Refuses later posts and destroys pending tasks without running them.
	void Shutdown();

private:
	struct Entry
	{
		std::uint64_t due = 0;
		std::uint64_t sequence = 0;
		Task task;
	};

	PostResult Post( Task task, std::uint64_t delayNanoseconds );
	bool PopDue( std::uint64_t now, Task &out );

	VirtualClock &m_clock;
	mutable std::mutex m_mutex;
	std::vector<Entry> m_pending; // a min-heap on (due, sequence)
	std::uint64_t m_nextSequence = 0;
	bool m_shutDown = false;
	std::atomic<std::thread::id> m_running{};
};

} // namespace platform

#endif // PLATFORM_RUNNERS_MANUAL_TASK_RUNNER_H
