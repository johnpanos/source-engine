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

// Two owners: the provider (until join or detach) and the running thread
// (until its entry returns). A plain count keeps std's shared pointer out of
// the shared modules that link this.
struct ThreadRecord
{
	std::atomic<int> refs{ 2 };
	HANDLE handle = nullptr; // the provider's own handle
	unsigned long nativeId = 0;
	std::uint64_t id = 0;
	ThreadEntry entry = nullptr;			   // a platform.thread.v1 thread, or
	IWin32Threads::NativeProc proc = nullptr; // a native one, whose result is the exit code
	void *context = nullptr;
	char name[kThreadNameMaxBytes + 1] = {};
	ThreadPriority priority = ThreadPriority::kNormal;
};

void Release( ThreadRecord *record )
{
	if ( record != nullptr && record->refs.fetch_sub( 1, std::memory_order_acq_rel ) == 1 )
	{
		if ( record->handle != nullptr )
		{
			CloseHandle( record->handle );
		}
		delete record;
	}
}

unsigned __stdcall ThreadTrampoline( void *arg )
{
	ThreadRecord *record = static_cast<ThreadRecord *>( arg );
	t_threadId = record->id;
	if ( record->name[0] != '\0' )
	{
		SetNativeName( record->name );
	}
	if ( record->priority != ThreadPriority::kNormal )
	{
		ApplyPriority( record->priority );
	}
	unsigned result = 0;
	if ( record->proc != nullptr )
	{
		IWin32Threads::NativeProc proc = record->proc;
		void *context = record->context;
		Release( record );
		return proc( context );
	}
	record->entry( record->context );
	Release( record );
	return result;
}

class CWin32Threads final : public IWin32Threads
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
		ThreadRecord *record = new ThreadRecord();
		record->entry = entry;
		record->context = context;
		record->priority = options.priority;
		if ( options.name != nullptr )
		{
			CopyName( options.name, record->name );
		}
		return Launch( record, options.stackBytes, false, out );
	}

	ThreadResult StartNative( std::size_t stackBytes, NativeProc proc, void *arg, bool suspended,
	    ThreadHandle &out, void *&callerHandle, unsigned long &id ) override
	{
		if ( proc == nullptr )
		{
			return ThreadResult::kInvalidArgument;
		}
		ThreadRecord *record = new ThreadRecord();
		record->proc = proc;
		record->context = arg;
		ThreadHandle handle;
		const ThreadResult result = Launch( record, stackBytes, suspended, handle );
		if ( result != ThreadResult::kOk )
		{
			return result;
		}
		HANDLE duplicate = nullptr;
		{
			std::lock_guard<std::mutex> lock( m_mutex );
			ThreadRecord *held = m_threads[handle.value];
			DuplicateHandle( GetCurrentProcess(), held->handle, GetCurrentProcess(), &duplicate, 0,
			    FALSE, DUPLICATE_SAME_ACCESS );
			id = held->nativeId;
		}
		out = handle;
		callerHandle = duplicate;
		return ThreadResult::kOk;
	}

	ThreadResult Join( ThreadHandle thread ) override
	{
		ThreadRecord *record = nullptr;
		{
			std::lock_guard<std::mutex> lock( m_mutex );
			auto it = m_threads.find( thread.value );
			if ( it == m_threads.end() || it->second->id == CurrentThreadIdValue() )
			{
				return ThreadResult::kInvalidArgument;
			}
			record = it->second;
			m_threads.erase( it );
		}
		WaitForSingleObject( record->handle, INFINITE );
		Release( record );
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

	// --- IWin32Threads ---------------------------------------------------

	unsigned long CurrentNativeId() const override { return GetCurrentThreadId(); }
	void *CurrentPseudoHandle() const override { return GetCurrentThread(); }

	WaitResult WaitNative( void *handle, unsigned long timeoutMs ) override
	{
		const DWORD wait = WaitForSingleObject( static_cast<HANDLE>( handle ), timeoutMs );
		return wait == WAIT_OBJECT_0 ? WaitResult::kSignaled
		       : wait == WAIT_TIMEOUT ? WaitResult::kTimeout
		                              : WaitResult::kFailed;
	}

	bool IsNativeHandleRunning( void *handle ) const override
	{
		DWORD code = 0;
		return handle != nullptr && GetExitCodeThread( static_cast<HANDLE>( handle ), &code ) &&
		       code == STILL_ACTIVE;
	}

	bool IsNativeIdRunning( unsigned long id ) const override
	{
		HANDLE thread = OpenThread( THREAD_QUERY_INFORMATION, FALSE, id );
		if ( thread == nullptr )
		{
			return false;
		}
		const bool running = IsNativeHandleRunning( thread );
		CloseHandle( thread );
		return running;
	}

	int GetNativePriority( void *handle ) const override
	{
		return GetThreadPriority( static_cast<HANDLE>( handle ) );
	}

	bool SetNativePriority( void *handle, int priority ) override
	{
		return SetThreadPriority( static_cast<HANDLE>( handle ), priority ) != 0;
	}

	void SetNativeAffinity( void *handle, std::uintptr_t mask ) override
	{
		SetThreadAffinityMask( static_cast<HANDLE>( handle ), static_cast<DWORD_PTR>( mask ) );
	}

	bool TerminateNative( void *handle, unsigned long exitCode ) override
	{
		return TerminateThread( static_cast<HANDLE>( handle ), exitCode ) != 0;
	}

	bool ResumeNative( void *handle ) override
	{
		return ResumeThread( static_cast<HANDLE>( handle ) ) != static_cast<DWORD>( -1 );
	}

	bool SuspendNative( void *handle ) override
	{
		return SuspendThread( static_cast<HANDLE>( handle ) ) != static_cast<DWORD>( -1 );
	}

	void *OpenNative( unsigned long id, unsigned long access ) override
	{
		return OpenThread( access, FALSE, id );
	}

	bool CloseNative( void *handle ) override
	{
		return handle != nullptr && CloseHandle( static_cast<HANDLE>( handle ) ) != 0;
	}

	ThreadResult DetachNativeId( unsigned long id ) override
	{
		ThreadHandle known;
		{
			std::lock_guard<std::mutex> lock( m_mutex );
			for ( const auto &entry : m_threads )
			{
				if ( entry.second->nativeId == id )
				{
					known.value = entry.first;
					break;
				}
			}
		}
		return known.value != 0 ? Detach( known ) : ThreadResult::kInvalidArgument;
	}

	ThreadResult Detach( ThreadHandle thread ) override
	{
		ThreadRecord *record = nullptr;
		{
			std::lock_guard<std::mutex> lock( m_mutex );
			auto it = m_threads.find( thread.value );
			if ( it == m_threads.end() )
			{
				return ThreadResult::kInvalidArgument;
			}
			record = it->second;
			m_threads.erase( it );
		}
		Release( record ); // closes the provider's handle once the thread is done too
		return ThreadResult::kOk;
	}

private:
	ThreadResult Launch( ThreadRecord *record, std::size_t stackBytes, bool suspended, ThreadHandle &out )
	{
		record->id = g_nextThreadId.fetch_add( 1 );
		if ( stackBytes > 0xffffffffULL )
		{
			delete record;
			return ThreadResult::kInvalidArgument;
		}
		std::lock_guard<std::mutex> lock( m_mutex );
		unsigned nativeId = 0;
		const uintptr_t h = _beginthreadex( nullptr, static_cast<unsigned>( stackBytes ), ThreadTrampoline,
		    record, STACK_SIZE_PARAM_IS_A_RESERVATION | ( suspended ? CREATE_SUSPENDED : 0 ), &nativeId );
		if ( h == 0 )
		{
			delete record; // the thread never ran, so it holds no reference
			return errno == EAGAIN ? ThreadResult::kResourceExhausted : ThreadResult::kInvalidArgument;
		}
		record->handle = reinterpret_cast<HANDLE>( h );
		record->nativeId = nativeId;
		const std::uint64_t handle = m_nextHandle++;
		m_threads[handle] = record;
		out.value = handle;
		return ThreadResult::kOk;
	}

	mutable std::mutex m_mutex;
	std::uint64_t m_nextHandle = 1;
	std::map<std::uint64_t, ThreadRecord *> m_threads;
};

} // namespace

std::unique_ptr<IWin32Threads> CreateWin32Threads()
{
	return std::make_unique<CWin32Threads>();
}

} // namespace platform
