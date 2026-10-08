//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the RFC 0001 threading conformance suite
//			(PLAT-THREAD-001, Q-FOUNDATION). Feeds the SAME shared predicate
//			broken providers, each violating one clause, and asserts every one
//			is caught while the conforming backend passes.
//
//			Build/run: tools/quality/conformance.py check --suite platform.thread.sensitivity
//
//=============================================================================//

#include "fake_threads.h"
#include "thread_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <memory>

namespace
{

using platform::ThreadEntry;
using platform::ThreadHandle;
using platform::ThreadId;
using platform::ThreadOptions;
using platform::ThreadResult;
using platformtest::CFakeMonotonicClock;
using platformtest::CFakeThreads;

// Forwards every call to a conforming fake; each defect overrides one.
class CForwarding : public platform::IThreads
{
public:
	explicit CForwarding( CFakeMonotonicClock &clock ) : m_inner( clock ) {}
	ThreadResult Start( const ThreadOptions &o, ThreadEntry e, void *c, ThreadHandle &out ) override
	{
		return m_inner.Start( o, e, c, out );
	}
	ThreadResult Join( ThreadHandle h ) override { return m_inner.Join( h ); }
	ThreadId CurrentId() const override { return m_inner.CurrentId(); }
	ThreadId IdOf( ThreadHandle h ) const override { return m_inner.IdOf( h ); }
	ThreadResult SetCurrentName( const char *n ) override { return m_inner.SetCurrentName( n ); }
	int GetCurrentName( char *b, int s ) const override { return m_inner.GetCurrentName( b, s ); }
	ThreadResult SetCurrentPriority( platform::ThreadPriority p ) override
	{
		return m_inner.SetCurrentPriority( p );
	}
	void SleepFor( std::uint64_t ns ) override { m_inner.SleepFor( ns ); }
	unsigned HardwareConcurrency() const override { return m_inner.HardwareConcurrency(); }

protected:
	CFakeThreads m_inner;
};

// DEFECT: Join returns without running the entry.
class CNeverRuns : public CForwarding
{
public:
	using CForwarding::CForwarding;
	ThreadResult Start( const ThreadOptions &o, ThreadEntry, void *c, ThreadHandle &out ) override
	{
		return m_inner.Start( o, []( void * ) {}, c, out );
	}
};

// DEFECT: the entry runs twice.
class CRunsTwice : public CForwarding
{
public:
	using CForwarding::CForwarding;
	ThreadResult Start( const ThreadOptions &o, ThreadEntry e, void *c, ThreadHandle &out ) override
	{
		m_entry = e;
		m_context = c;
		return m_inner.Start( o, e, c, out );
	}
	ThreadResult Join( ThreadHandle h ) override
	{
		const ThreadResult result = m_inner.Join( h );
		if ( result == ThreadResult::kOk )
		{
			m_entry( m_context );
		}
		return result;
	}

private:
	ThreadEntry m_entry = nullptr;
	void *m_context = nullptr;
};

// DEFECT: a second Join of the same handle succeeds.
class CDoubleJoin : public CForwarding
{
public:
	using CForwarding::CForwarding;
	ThreadResult Join( ThreadHandle h ) override
	{
		m_inner.Join( h );
		return ThreadResult::kOk;
	}
};

// DEFECT: a null entry is accepted.
class CAcceptsNullEntry : public CForwarding
{
public:
	using CForwarding::CForwarding;
	ThreadResult Start( const ThreadOptions &o, ThreadEntry e, void *c, ThreadHandle &out ) override
	{
		if ( e == nullptr )
		{
			out.value = 1;
			return ThreadResult::kOk;
		}
		return m_inner.Start( o, e, c, out );
	}
};

// DEFECT: IdOf reports a different id than the thread sees.
class CWrongIdOf : public CForwarding
{
public:
	using CForwarding::CForwarding;
	ThreadId IdOf( ThreadHandle h ) const override
	{
		ThreadId id = m_inner.IdOf( h );
		if ( id.value != 0 )
		{
			id.value += 100;
		}
		return id;
	}
};

// DEFECT: the options' name is ignored.
class CIgnoresOptionName : public CForwarding
{
public:
	using CForwarding::CForwarding;
	ThreadResult Start( const ThreadOptions &o, ThreadEntry e, void *c, ThreadHandle &out ) override
	{
		ThreadOptions unnamed = o;
		unnamed.name = nullptr;
		return m_inner.Start( unnamed, e, c, out );
	}
};

// DEFECT: a name that does not fit is partially written instead of refused.
class CPartialName : public CForwarding
{
public:
	using CForwarding::CForwarding;
	int GetCurrentName( char *b, int s ) const override
	{
		char full[64];
		const int n = m_inner.GetCurrentName( full, sizeof( full ) );
		if ( n < 0 || b == nullptr || s <= 0 )
		{
			return n;
		}
		const int w = n < s - 1 ? n : s - 1;
		std::memcpy( b, full, w );
		b[w] = '\0';
		return w;
	}
};

// DEFECT: sleeps half the request.
class CShortSleep : public CForwarding
{
public:
	using CForwarding::CForwarding;
	void SleepFor( std::uint64_t ns ) override { m_inner.SleepFor( ns / 2 ); }
};

// DEFECT: reports zero hardware threads.
class CZeroConcurrency : public CForwarding
{
public:
	using CForwarding::CForwarding;
	unsigned HardwareConcurrency() const override { return 0; }
};

template <typename T> bool Caught()
{
	CFakeMonotonicClock clock;
	T threads( clock );
	return platformtest::RunThreadConformance( threads, clock ).failures > 0;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;

	{
		CFakeMonotonicClock clock;
		CFakeThreads good( clock );
		const platformtest::ThreadReport r = platformtest::RunThreadConformance( good, clock );
		++checks;
		if ( r.failures != 0 )
		{
			std::printf( "FAIL: conforming provider rejected (%d/%d); first: %s (line %d)\n",
			    r.failures, r.checks, r.firstFailure, r.firstFailureLine );
			++failures;
		}
	}

	struct Case
	{
		bool ( *caught )();
		const char *name;
	};
	const Case cases[] = {
	    { Caught<CNeverRuns>, "entry-never-runs" },
	    { Caught<CRunsTwice>, "entry-runs-twice" },
	    { Caught<CDoubleJoin>, "double-join-succeeds" },
	    { Caught<CAcceptsNullEntry>, "accepts-null-entry" },
	    { Caught<CWrongIdOf>, "idof-differs-from-current-id" },
	    { Caught<CIgnoresOptionName>, "ignores-option-name" },
	    { Caught<CPartialName>, "partial-name-write" },
	    { Caught<CShortSleep>, "short-sleep" },
	    { Caught<CZeroConcurrency>, "zero-concurrency" },
	};
	for ( const Case &c : cases )
	{
		++checks;
		if ( !c.caught() )
		{
			std::printf( "FAIL: broken provider '%s' was NOT caught\n", c.name );
			++failures;
		}
	}
	if ( failures == 0 )
	{
		std::printf( "ok test_thread_negative: all %zu broken providers caught\n",
		    sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
