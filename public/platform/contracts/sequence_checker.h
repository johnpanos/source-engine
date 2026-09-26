//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Diagnostic sequence-affinity checks (RFC 0001: "Thread-affinity
//			and sequence-affinity checks are enabled in diagnostic builds").
//
//			A SequenceChecker binds, on its first check, to the sequence whose
//			task is running (platform::CurrentSequence()), or to the calling
//			thread when no sequence is running. Later checks pass only on that
//			sequence, from whichever thread runs it, or on that thread. A class
//			whose state belongs to one sequence holds a checker and states the
//			rule with PLATFORM_CHECK_SEQUENCE, which aborts on a violation in
//			diagnostic builds and compiles to nothing otherwise.
//
//			Diagnostic builds: PLATFORM_SEQUENCE_CHECKS defaults to 1 unless
//			NDEBUG is defined; define it to 0 or 1 to override.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_SEQUENCE_CHECKER_H
#define PLATFORM_CONTRACTS_SEQUENCE_CHECKER_H

#include "platform/contracts/task_runner.h"

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>

#ifndef PLATFORM_SEQUENCE_CHECKS
#ifdef NDEBUG
#define PLATFORM_SEQUENCE_CHECKS 0
#else
#define PLATFORM_SEQUENCE_CHECKS 1
#endif
#endif

namespace platform
{

class SequenceChecker
{
public:
	SequenceChecker() = default;
	SequenceChecker( const SequenceChecker & ) = delete;
	SequenceChecker &operator=( const SequenceChecker & ) = delete;

	// Binds on the first call; true while called on the bound sequence (or
	// thread). Safe to call from any thread.
	bool CalledOnValidSequence() const
	{
		const void *sequence = CurrentSequence();
		const std::thread::id thread = std::this_thread::get_id();
		std::lock_guard lock( m_mutex );
		if ( !m_bound )
		{
			m_bound = true;
			m_sequence = sequence;
			m_thread = thread;
			return true;
		}
		return m_sequence ? sequence == m_sequence : ( sequence == nullptr && thread == m_thread );
	}

	// Unbinds: the next check binds again (for state handed to a new owner).
	void Detach()
	{
		std::lock_guard lock( m_mutex );
		m_bound = false;
	}

private:
	mutable std::mutex m_mutex;
	mutable bool m_bound = false;
	mutable const void *m_sequence = nullptr;
	mutable std::thread::id m_thread;
};

[[noreturn]] inline void SequenceCheckFailed( const char *expression, const char *file, int line )
{
	std::fprintf( stderr, "%s:%d: sequence check failed: %s is used off its sequence\n", file, line,
	    expression );
	std::abort();
}

} // namespace platform

#if PLATFORM_SEQUENCE_CHECKS
#define PLATFORM_CHECK_SEQUENCE( checker )                                                         \
	( ( checker ).CalledOnValidSequence()                                                          \
	        ? (void)0                                                                              \
	        : ::platform::SequenceCheckFailed( #checker, __FILE__, __LINE__ ) )
#else
#define PLATFORM_CHECK_SEQUENCE( checker ) ( (void)0 )
#endif

#endif // PLATFORM_CONTRACTS_SEQUENCE_CHECKER_H
