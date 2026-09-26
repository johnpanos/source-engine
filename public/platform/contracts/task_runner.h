//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Execution contracts (RFC 0001 "Threads, sequences, and injectable
//			scheduling"; RFC 0003 "Sequences and physical affinity"):
//			platform.task-runner.v1. Features depend on how their work runs,
//			not on physical threads:
//
//			- ITaskRunner posts independent work;
//			- ISequencedTaskRunner runs its tasks one at a time, in post order;
//			- ISingleThreadTaskRunner is a sequence bound to one physical thread,
//			  reserved for APIs with real thread affinity.
//
//			Rules every provider obeys (unittests/platformtest/contracts/
//			platform.task-runner.v1.md; shared suite platform.task_runner):
//
//			1. An accepted task runs exactly once, unless its runner is shut down
//			   first; then it is destroyed without running. A refused task
//			   (PostResult::kShutDown) is destroyed before the post returns.
//			2. Posting is thread-safe, never blocks on running tasks, and never
//			   runs the task inside the post, including from inside a task.
//			3. A delayed task runs no earlier than its delay after the post, as
//			   measured by the runner's clock. A delay of 0 is a plain post.
//			4. Sequenced runners run one task at a time; everything a task did
//			   happens-before the next task of the sequence starts. Tasks posted
//			   without delay run in post order (posts from different threads are
//			   ordered by when each post returned).
//			5. RunsTasksInCurrentSequence() is true exactly while the calling
//			   thread runs a task of that sequence; BelongsToCurrentThread() is
//			   true exactly on the runner's thread.
//
//			Shutdown belongs to the owner, not to consumers: it is a provider
//			operation that refuses later posts, destroys pending tasks without
//			running them, and returns only when no task of the runner is running
//			or will run.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_TASK_RUNNER_H
#define PLATFORM_CONTRACTS_TASK_RUNNER_H

// Contract header: standard library only. No tier0/tier1, no native SDK.
#include <concepts>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace platform
{

// A move-only unit of work. Runners invoke it at most once, then destroy it.
class Task
{
public:
	Task() = default;

	template <typename F>
	    requires( std::invocable<F &> && !std::same_as<std::decay_t<F>, Task> )
	Task( F &&function ) : m_impl( std::make_unique<Model<std::decay_t<F>>>( std::forward<F>( function ) ) )
	{
	}

	Task( Task && ) noexcept = default;
	Task &operator=( Task && ) noexcept = default;
	Task( const Task & ) = delete;
	Task &operator=( const Task & ) = delete;

	explicit operator bool() const { return m_impl != nullptr; }
	void operator()() { m_impl->Run(); }

private:
	struct Concept
	{
		virtual ~Concept() = default;
		virtual void Run() = 0;
	};
	template <typename F> struct Model final : Concept
	{
		template <typename G> explicit Model( G &&function ) : function( std::forward<G>( function ) ) {}
		void Run() override { function(); }
		F function;
	};

	std::unique_ptr<Concept> m_impl;
};

enum class PostResult
{
	kAccepted,
	kShutDown, // the runner is shut down; the task was destroyed without running
};

class ITaskRunner
{
public:
	virtual ~ITaskRunner() = default;

	[[nodiscard]] virtual PostResult PostTask( Task task ) = 0;
	[[nodiscard]] virtual PostResult PostDelayedTask( Task task, std::uint64_t delayNanoseconds ) = 0;
};

class ISequencedTaskRunner : public ITaskRunner
{
public:
	virtual bool RunsTasksInCurrentSequence() const = 0;
};

class ISingleThreadTaskRunner : public ISequencedTaskRunner
{
public:
	virtual bool BelongsToCurrentThread() const = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_TASK_RUNNER_H
