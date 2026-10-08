//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 threads (platform.thread.v1) over _beginthreadex.
//
//=============================================================================//

#include "foundation_providers.h"

#include "win32_text.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <process.h>

namespace platform
{
namespace
{

std::atomic<std::uint64_t> g_nextThreadId{ 1 };
thread_local std::uint64_t t_threadId = 0;
// Names live beside the native description so reads are exact UTF-8 and do not
// depend on SetThreadDescription being implemented (older Windows, Wine).
thread_local char t_name[kThreadNameMaxBytes + 1] = {};

std::uint64_t CurrentThreadIdValue()
{
	if ( t_threadId == 0 )
	{
		t_threadId = g_nextThreadId.fetch_add( 1 );
	}
	return t_threadId;
}

void CopyName( const char *name, char ( &out )[kThreadNameMaxBytes + 1] )
{
	std::size_t n = std::strlen( name );
	if ( n > static_cast<std::size_t>( kThreadNameMaxBytes ) )
	{
		n = kThreadNameMaxBytes;
	}
	std::memcpy( out, name, n );
	out[n] = '\0';
}

using SetThreadDescriptionFn = HRESULT( WINAPI * )( HANDLE, PCWSTR );

void SetNativeName( const char *name )
{
	CopyName( name, t_name );
	// SetThreadDescription exists from Windows 10 1607; look it up so older
	// systems still run.
	static const SetThreadDescriptionFn set =
	    reinterpret_cast<SetThreadDescriptionFn>( reinterpret_cast<void *>(
	        GetProcAddress( GetModuleHandleW( L"kernel32.dll" ), "SetThreadDescription" ) ) );
	if ( set != nullptr )
	{
		set( GetCurrentThread(), win32::ToWide( t_name ).c_str() );
	}
}

ThreadResult ApplyPriority( ThreadPriority priority )
{
	const int native = priority == ThreadPriority::kLow    ? THREAD_PRIORITY_BELOW_NORMAL
	                   : priority == ThreadPriority::kHigh ? THREAD_PRIORITY_ABOVE_NORMAL
	                                                       : THREAD_PRIORITY_NORMAL;
	return SetThreadPriority( GetCurrentThread(), native ) ? ThreadResult::kOk
	                                                       : ThreadResult::kUnsupported;
}

struct ThreadRecord
{
	HANDLE handle = nullptr;
	std::uint64_t id = 0;
	ThreadEntry entry = nullptr;
	void *context = nullptr;
	char name[kThreadNameMaxBytes + 1] = {};
	ThreadPriority priority = ThreadPriority::kNormal;
};

unsigned __stdcall ThreadTrampoline( void *arg )
{
	ThreadRecord &record = *static_cast<ThreadRecord *>( arg );
	t_threadId = record.id;
	if ( record.name[0] != '\0' )
	{
		SetNativeName( record.name );
	}
	if ( record.priority != ThreadPriority::kNormal )
	{
		ApplyPriority( record.priority );
	}
	record.entry( record.context );
	return 0;
}

class CWin32Threads final : public IThreads
{
public:
	~CWin32Threads() override
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		if ( !m_threads.empty() )
		{
			std::fprintf( stderr,
			    "platform.thread.v1: provider destroyed with %zu unjoined "
			    "thread(s)\n",
			    m_threads.size() );
			std::abort();
		}
	}

	ThreadResult Start(
	    const ThreadOptions &options, ThreadEntry entry, void *context, ThreadHandle &out ) override
	{
		if ( entry == nullptr )
		{
			return ThreadResult::kInvalidArgument;
		}
		auto record = std::make_unique<ThreadRecord>();
		record->id = g_nextThreadId.fetch_add( 1 );
		record->entry = entry;
		record->context = context;
		record->priority = options.priority;
		if ( options.name != nullptr )
		{
			CopyName( options.name, record->name );
		}
		if ( options.stackBytes > 0xffffffffULL )
		{
			return ThreadResult::kInvalidArgument;
		}
		std::lock_guard<std::mutex> lock( m_mutex );
		const uintptr_t h = _beginthreadex( nullptr, static_cast<unsigned>( options.stackBytes ),
		    ThreadTrampoline, record.get(), STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr );
		if ( h == 0 )
		{
			return errno == EAGAIN ? ThreadResult::kResourceExhausted
			                       : ThreadResult::kInvalidArgument;
		}
		record->handle = reinterpret_cast<HANDLE>( h );
		const std::uint64_t handle = m_nextHandle++;
		m_threads[handle] = std::move( record );
		out.value = handle;
		return ThreadResult::kOk;
	}

	ThreadResult Join( ThreadHandle thread ) override
	{
		std::unique_ptr<ThreadRecord> record;
		{
			std::lock_guard<std::mutex> lock( m_mutex );
			auto it = m_threads.find( thread.value );
			if ( it == m_threads.end() || it->second->id == CurrentThreadIdValue() )
			{
				return ThreadResult::kInvalidArgument;
			}
			record = std::move( it->second );
			m_threads.erase( it );
		}
		WaitForSingleObject( record->handle, INFINITE );
		CloseHandle( record->handle );
		return ThreadResult::kOk;
	}

	ThreadId CurrentId() const override
	{
		ThreadId id;
		id.value = CurrentThreadIdValue();
		return id;
	}

	ThreadId IdOf( ThreadHandle thread ) const override
	{
		ThreadId id;
		std::lock_guard<std::mutex> lock( m_mutex );
		auto it = m_threads.find( thread.value );
		if ( it != m_threads.end() )
		{
			id.value = it->second->id;
		}
		return id;
	}

	ThreadResult SetCurrentName( const char *name ) override
	{
		if ( name == nullptr )
		{
			return ThreadResult::kInvalidArgument;
		}
		SetNativeName( name );
		return ThreadResult::kOk;
	}

	int GetCurrentName( char *buffer, int bufferSize ) const override
	{
		const int n = static_cast<int>( std::strlen( t_name ) );
		if ( n == 0 || buffer == nullptr || bufferSize <= n )
		{
			return -1;
		}
		std::memcpy( buffer, t_name, n + 1 );
		return n;
	}

	ThreadResult SetCurrentPriority( ThreadPriority priority ) override
	{
		return ApplyPriority( priority );
	}

	void SleepFor( std::uint64_t nanoseconds ) override
	{
		if ( nanoseconds == 0 )
		{
			SwitchToThread();
			return;
		}
		// Sleep() takes milliseconds and may wake early relative to QPC; finish
		// against the counter so the contract's "at least" holds.
		LARGE_INTEGER f, start, now;
		QueryPerformanceFrequency( &f );
		QueryPerformanceCounter( &start );
		const std::uint64_t target = nanoseconds;
		for ( ;; )
		{
			QueryPerformanceCounter( &now );
			const std::uint64_t ticks = static_cast<std::uint64_t>( now.QuadPart - start.QuadPart );
			const std::uint64_t f64 = static_cast<std::uint64_t>( f.QuadPart );
			const std::uint64_t elapsed =
			    ticks / f64 * 1000000000ULL + ticks % f64 * 1000000000ULL / f64;
			if ( elapsed >= target + 1000 ) // past any one-tick conversion rounding
			{
				return;
			}
			const std::uint64_t leftMs =
			    ( target - ( elapsed < target ? elapsed : target ) ) / 1000000ULL;
			Sleep( leftMs > 1 ? static_cast<DWORD>( leftMs - 1 ) : 0 );
		}
	}

	unsigned HardwareConcurrency() const override
	{
		const DWORD n = GetActiveProcessorCount( ALL_PROCESSOR_GROUPS );
		return n > 0 ? static_cast<unsigned>( n ) : 1u;
	}

private:
	mutable std::mutex m_mutex;
	std::uint64_t m_nextHandle = 1;
	std::map<std::uint64_t, std::unique_ptr<ThreadRecord>> m_threads;
};

} // namespace

std::unique_ptr<IThreads> CreateWin32Threads()
{
	return std::make_unique<CWin32Threads>();
}

} // namespace platform
