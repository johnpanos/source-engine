//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A sequence over any task runner (platform.runners; RFC 0001:
//			"Sequences are preferred to dedicated threads"). Tasks posted here
//			run one at a time, in post order, on whatever threads the base
//			runner uses: the sequence keeps at most one base task in flight,
//			and each base task runs one sequence task and re-posts itself while
//			work remains.
//
//			Lifetime: the base runner must outlive this sequence. Base tasks
//			share the sequence's state (the destruction context of that state
//			is the last of the sequence and its in-flight base tasks), so
//			destroying the sequence with base tasks still queued is safe; they
//			find it shut down and do nothing. If the base runner refuses a post,
//			the sequence shuts down.
//
//=============================================================================//

#ifndef PLATFORM_RUNNERS_SEQUENCED_TASK_RUNNER_H
#define PLATFORM_RUNNERS_SEQUENCED_TASK_RUNNER_H

#include "platform/contracts/task_runner.h"

#include <memory>

namespace platform
{

class SequencedTaskRunner final : public ISequencedTaskRunner
{
public:
	// Borrows `base`, which must outlive this object.
	explicit SequencedTaskRunner( ITaskRunner &base );
	~SequencedTaskRunner() override;

	[[nodiscard]] PostResult PostTask( Task task ) override;
	[[nodiscard]] PostResult PostDelayedTask( Task task, std::uint64_t delayNanoseconds ) override;
	bool RunsTasksInCurrentSequence() const override;

	// Refuses later posts, destroys pending tasks without running them, and
	// waits for a running task to finish. Calling it from the sequence aborts.
	void Shutdown();

	struct State;

private:
	std::shared_ptr<State> m_state;
};

} // namespace platform

#endif // PLATFORM_RUNNERS_SEQUENCED_TASK_RUNNER_H
