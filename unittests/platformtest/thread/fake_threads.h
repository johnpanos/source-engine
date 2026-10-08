//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic backend for platform::IThreads. Start() records the
//			entry; Join() runs it on the calling OS thread under a virtual thread
//			id, so a test observes exactly-once execution, ids and names without
//			real concurrency. SleepFor() advances a fake monotonic clock. This is
//			a CONFORMING provider: the positive subject of the shared suite.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_THREADS_H
#define PLATFORMTEST_FAKE_THREADS_H

#include "../clock/fake_clock.h"
#include "platform/contracts/thread.h"

#include <cstring>
#include <map>
#include <string>

namespace platformtest
{

class CFakeThreads : public platform::IThreads
{
public:
	explicit CFakeThreads(
	    CFakeMonotonicClock &clock, bool supportsNames = true, bool supportsPriority = true )
	    : m_clock( clock ), m_supportsNames( supportsNames ), m_supportsPriority( supportsPriority )
	{
	}

	platform::ThreadResult Start( const platform::ThreadOptions &options,
	    platform::ThreadEntry entry, void *context, platform::ThreadHandle &out ) override
	{
		if ( entry == nullptr )
		{
			return platform::ThreadResult::kInvalidArgument;
		}
		Pending p;
		p.entry = entry;
		p.context = context;
		p.id = m_nextId++;
		if ( options.name != nullptr && m_supportsNames )
		{
			m_names[p.id] = Truncate( options.name );
		}
		const std::uint64_t handle = m_nextHandle++;
		m_pending[handle] = p;
		out.value = handle;
		return platform::ThreadResult::kOk;
	}

	platform::ThreadResult Join( platform::ThreadHandle thread ) override
	{
		auto it = m_pending.find( thread.value );
		if ( it == m_pending.end() || it->second.id == m_current )
		{
			return platform::ThreadResult::kInvalidArgument;
		}
		Pending &p = it->second;
		if ( !p.ran )
		{
			const std::uint64_t saved = m_current;
			m_current = p.id;
			p.ran = true;
			p.entry( p.context );
			m_current = saved;
		}
		m_names.erase( p.id );
		m_pending.erase( it );
		return platform::ThreadResult::kOk;
	}

	platform::ThreadId CurrentId() const override
	{
		platform::ThreadId id;
		id.value = m_current;
		return id;
	}

	platform::ThreadId IdOf( platform::ThreadHandle thread ) const override
	{
		platform::ThreadId id;
		auto it = m_pending.find( thread.value );
		if ( it != m_pending.end() )
		{
			id.value = it->second.id;
		}
		return id;
	}

	platform::ThreadResult SetCurrentName( const char *name ) override
	{
		if ( !m_supportsNames )
		{
			return platform::ThreadResult::kUnsupported;
		}
		if ( name == nullptr )
		{
			return platform::ThreadResult::kInvalidArgument;
		}
		m_names[m_current] = Truncate( name );
		return platform::ThreadResult::kOk;
	}

	int GetCurrentName( char *buffer, int bufferSize ) const override
	{
		auto it = m_names.find( m_current );
		if ( !m_supportsNames || it == m_names.end() || buffer == nullptr ||
		     bufferSize <= static_cast<int>( it->second.size() ) )
		{
			return -1;
		}
		std::memcpy( buffer, it->second.c_str(), it->second.size() + 1 );
		return static_cast<int>( it->second.size() );
	}

	platform::ThreadResult SetCurrentPriority( platform::ThreadPriority ) override
	{
		return m_supportsPriority ? platform::ThreadResult::kOk
		                          : platform::ThreadResult::kUnsupported;
	}

	void SleepFor( std::uint64_t nanoseconds ) override { m_clock.Advance( nanoseconds ); }

	unsigned HardwareConcurrency() const override { return 4; }

private:
	struct Pending
	{
		platform::ThreadEntry entry = nullptr;
		void *context = nullptr;
		std::uint64_t id = 0;
		bool ran = false;
	};

	static std::string Truncate( const char *name )
	{
		std::string s( name );
		if ( s.size() > static_cast<std::size_t>( platform::kThreadNameMaxBytes ) )
		{
			s.resize( platform::kThreadNameMaxBytes );
		}
		return s;
	}

	CFakeMonotonicClock &m_clock;
	bool m_supportsNames;
	bool m_supportsPriority;
	std::uint64_t m_current = 1; // the test's own thread
	std::uint64_t m_nextId = 2;
	std::uint64_t m_nextHandle = 1;
	std::map<std::uint64_t, Pending> m_pending;
	std::map<std::uint64_t, std::string> m_names;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_THREADS_H
