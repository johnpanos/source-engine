//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX threads (platform.thread.v1) over pthreads.
//
//=============================================================================//

#include "foundation_providers.h"

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>

#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#if defined( __APPLE__ )
#include <pthread/qos.h>
#else
#include <sys/syscall.h>
#endif

namespace platform
{
namespace
{

// Thread ids come from one process-wide counter so they are distinct among live
// threads whichever provider instance asks. A thread the provider did not start
// takes the next id the first time it asks.
std::atomic<std::uint64_t> g_nextThreadId{ 1 };
thread_local std::uint64_t t_threadId = 0;

std::uint64_t CurrentThreadIdValue()
{
	if ( t_threadId == 0 )
	{
		t_threadId = g_nextThreadId.fetch_add( 1 );
	}
	return t_threadId;
}

struct ThreadRecord
{
	pthread_t thread{};
	std::uint64_t id = 0;
	ThreadEntry entry = nullptr;
	void *context = nullptr;
	char name[kThreadNameMaxBytes + 1] = {};
	ThreadPriority priority = ThreadPriority::kNormal;
};

void SetNativeName( const char *name )
{
#if defined( __APPLE__ )
	pthread_setname_np( name );
#else
	pthread_setname_np( pthread_self(), name );
#endif
}

// Sets the calling thread's priority. kUnsupported when the platform refuses
// (raising priority without privilege), never a failure.
ThreadResult ApplyPriority( ThreadPriority priority )
{
#if defined( __APPLE__ )
	const qos_class_t qos = priority == ThreadPriority::kLow    ? QOS_CLASS_UTILITY
	                        : priority == ThreadPriority::kHigh ? QOS_CLASS_USER_INTERACTIVE
	                                                            : QOS_CLASS_DEFAULT;
	return pthread_set_qos_class_self_np( qos, 0 ) == 0 ? ThreadResult::kOk
	                                                    : ThreadResult::kUnsupported;
#elif defined( __EMSCRIPTEN__ )
	// The browser schedules its workers; there is no per-thread priority.
	(void)priority;
	return ThreadResult::kUnsupported;
#else
	// Linux and Android give each thread its own nice value. Raising priority
	// needs privilege the process may not have; that is kUnsupported, never
	// a failure.
	const int nice = priority == ThreadPriority::kLow    ? 10
	                 : priority == ThreadPriority::kHigh ? -5
	                                                     : 0;
	const pid_t tid = static_cast<pid_t>( syscall( SYS_gettid ) );
	return setpriority( PRIO_PROCESS, static_cast<id_t>( tid ), nice ) == 0
	           ? ThreadResult::kOk
	           : ThreadResult::kUnsupported;
#endif
}

void *ThreadTrampoline( void *arg )
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
	return nullptr;
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

class CPosixThreads final : public IThreads
{
public:
	~CPosixThreads() override
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

		pthread_attr_t attr;
		if ( pthread_attr_init( &attr ) != 0 )
		{
			return ThreadResult::kResourceExhausted;
		}
		if ( options.stackBytes != 0 )
		{
			const long page = sysconf( _SC_PAGESIZE );
			std::size_t size = options.stackBytes;
			if ( size < static_cast<std::size_t>( PTHREAD_STACK_MIN ) )
			{
				size = PTHREAD_STACK_MIN;
			}
			size = ( size + page - 1 ) / page * page;
			if ( pthread_attr_setstacksize( &attr, size ) != 0 )
			{
				pthread_attr_destroy( &attr );
				return ThreadResult::kInvalidArgument;
			}
		}

		// The record is published under the lock before the thread can run, so
		// IdOf and Join see it as soon as Start returns.
		std::lock_guard<std::mutex> lock( m_mutex );
		const int rc = pthread_create( &record->thread, &attr, ThreadTrampoline, record.get() );
		pthread_attr_destroy( &attr );
		if ( rc != 0 )
		{
			return rc == EAGAIN ? ThreadResult::kResourceExhausted : ThreadResult::kInvalidArgument;
		}
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
		pthread_join( record->thread, nullptr );
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
		char copy[kThreadNameMaxBytes + 1];
		CopyName( name, copy );
		SetNativeName( copy );
		return ThreadResult::kOk;
	}

	int GetCurrentName( char *buffer, int bufferSize ) const override
	{
		char name[64] = {};
		if ( buffer == nullptr || bufferSize <= 0 ||
		     pthread_getname_np( pthread_self(), name, sizeof( name ) ) != 0 || name[0] == '\0' )
		{
			return -1;
		}
		const int n = static_cast<int>( std::strlen( name ) );
		if ( n >= bufferSize )
		{
			return -1;
		}
		std::memcpy( buffer, name, n + 1 );
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
			sched_yield();
			return;
		}
		// Sleep to an absolute CLOCK_MONOTONIC deadline so interruptions never
		// shorten the sleep.
		timespec deadline{};
		clock_gettime( CLOCK_MONOTONIC, &deadline );
		const std::uint64_t total = static_cast<std::uint64_t>( deadline.tv_nsec ) + nanoseconds;
		deadline.tv_sec += static_cast<time_t>( total / 1000000000ULL );
		deadline.tv_nsec = static_cast<long>( total % 1000000000ULL );
#if defined( __APPLE__ )
		for ( ;; )
		{
			timespec now{};
			clock_gettime( CLOCK_MONOTONIC, &now );
			if ( now.tv_sec > deadline.tv_sec ||
			     ( now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec ) )
			{
				return;
			}
			std::int64_t left = ( deadline.tv_sec - now.tv_sec ) * 1000000000LL +
			                    ( deadline.tv_nsec - now.tv_nsec );
			timespec step{ static_cast<time_t>( left / 1000000000LL ),
			    static_cast<long>( left % 1000000000LL ) };
			nanosleep( &step, nullptr );
		}
#else
		while ( clock_nanosleep( CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, nullptr ) == EINTR )
		{
		}
#endif
	}

	unsigned HardwareConcurrency() const override
	{
#if defined( __linux__ )
		cpu_set_t set;
		CPU_ZERO( &set );
		if ( sched_getaffinity( 0, sizeof( set ), &set ) == 0 )
		{
			const int n = CPU_COUNT( &set );
			if ( n > 0 )
			{
				return static_cast<unsigned>( n );
			}
		}
#endif
		const long n = sysconf( _SC_NPROCESSORS_ONLN );
		return n > 0 ? static_cast<unsigned>( n ) : 1u;
	}

private:
	mutable std::mutex m_mutex;
	std::uint64_t m_nextHandle = 1;
	std::map<std::uint64_t, std::unique_ptr<ThreadRecord>> m_threads;
};

} // namespace

std::unique_ptr<IThreads> CreatePosixThreads()
{
	return std::make_unique<CPosixThreads>();
}

} // namespace platform
