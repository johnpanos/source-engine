//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX diagnostics (platform.diagnostics.v1): a framed debug-output
//			sink over a file descriptor, Android logcat, and a crash reporter
//			that writes text reports on demand and from a fatal-signal handler.
//
//=============================================================================//

#include "foundation_providers.h"

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <mutex>
#include <string>

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <unwind.h>

#if defined( __ANDROID__ )
#include <android/log.h>
#endif

namespace platform
{
namespace
{

const char *SeverityTag( DiagnosticSeverity severity )
{
	switch ( severity )
	{
	case DiagnosticSeverity::kWarning:
		return "[warn] ";
	case DiagnosticSeverity::kError:
		return "[error] ";
	case DiagnosticSeverity::kInfo:
	default:
		return "[info] ";
	}
}

class CFdDebugOutput final : public IDebugOutput
{
public:
	explicit CFdDebugOutput( int fd ) : m_fd( fd ) {}

	void Write( DiagnosticSeverity severity, const char *message ) override
	{
		if ( message == nullptr )
		{
			return;
		}
		const char *tag = SeverityTag( severity );
		iovec parts[3];
		parts[0].iov_base = const_cast<char *>( tag );
		parts[0].iov_len = std::strlen( tag );
		parts[1].iov_base = const_cast<char *>( message );
		parts[1].iov_len = std::strlen( message );
		parts[2].iov_base = const_cast<char *>( "\n" );
		parts[2].iov_len = 1;
		// One writev per message under a lock: whole within the process, and one
		// record on a packet socket. Short writes to a stream are finished.
		std::lock_guard<std::mutex> lock( m_mutex );
		std::size_t total = parts[0].iov_len + parts[1].iov_len + 1;
		ssize_t n;
		do
		{
			n = writev( m_fd, parts, 3 );
		} while ( n < 0 && errno == EINTR );
		if ( n <= 0 || static_cast<std::size_t>( n ) >= total )
		{
			return;
		}
		std::string rest = std::string( tag ) + message + "\n";
		std::size_t done = static_cast<std::size_t>( n );
		while ( done < total )
		{
			const ssize_t w = write( m_fd, rest.data() + done, total - done );
			if ( w < 0 && errno == EINTR )
			{
				continue;
			}
			if ( w <= 0 )
			{
				return;
			}
			done += static_cast<std::size_t>( w );
		}
	}

private:
	const int m_fd;
	std::mutex m_mutex;
};

#if defined( __ANDROID__ )
class CAndroidLogDebugOutput final : public IDebugOutput
{
public:
	explicit CAndroidLogDebugOutput( const char *tag ) : m_tag( tag != nullptr ? tag : "source" ) {}

	void Write( DiagnosticSeverity severity, const char *message ) override
	{
		if ( message == nullptr )
		{
			return;
		}
		const int priority = severity == DiagnosticSeverity::kError     ? ANDROID_LOG_ERROR
		                     : severity == DiagnosticSeverity::kWarning ? ANDROID_LOG_WARN
		                                                                : ANDROID_LOG_INFO;
		__android_log_write( priority, m_tag.c_str(), message );
	}

private:
	std::string m_tag;
};
#endif

// ---------------------------------------------------------------------------
// Crash reporter
// ---------------------------------------------------------------------------

bool IsValidKey( const char *key )
{
	if ( key == nullptr )
	{
		return false;
	}
	const std::size_t n = std::strlen( key );
	if ( n == 0 || n > static_cast<std::size_t>( kCrashKeyMaxBytes ) )
	{
		return false;
	}
	for ( std::size_t i = 0; i < n; ++i )
	{
		const char c = key[i];
		if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
		         c == '_' || c == '.' || c == '-' ) )
		{
			return false;
		}
	}
	return true;
}

// Async-signal-safe text building into a fixed buffer.
struct SafeText
{
	char data[8192];
	std::size_t length = 0;

	void Append( const char *s )
	{
		while ( *s != '\0' && length + 1 < sizeof( data ) )
		{
			data[length++] = *s++;
		}
		data[length] = '\0';
	}

	void AppendUnsigned( unsigned long long v, int base = 10 )
	{
		char digits[32];
		int n = 0;
		do
		{
			const int d = static_cast<int>( v % base );
			digits[n++] = static_cast<char>( d < 10 ? '0' + d : 'a' + d - 10 );
			v /= base;
		} while ( v != 0 && n < 32 );
		while ( n > 0 && length + 1 < sizeof( data ) )
		{
			data[length++] = digits[--n];
		}
		data[length] = '\0';
	}
};

struct Annotation
{
	std::atomic<bool> used{ false };
	char key[kCrashKeyMaxBytes + 1] = {};
	char value[kCrashValueMaxBytes + 1] = {};
};

struct BacktraceState
{
	void *frames[64];
	int count = 0;
};

_Unwind_Reason_Code CollectFrame( _Unwind_Context *context, void *arg )
{
	BacktraceState &state = *static_cast<BacktraceState *>( arg );
	const std::uintptr_t ip = static_cast<std::uintptr_t>( _Unwind_GetIP( context ) );
	if ( ip != 0 && state.count < 64 )
	{
		state.frames[state.count++] = reinterpret_cast<void *>( ip );
	}
	return state.count < 64 ? _URC_NO_REASON : _URC_END_OF_STACK;
}

class CPosixCrashReporter;
std::atomic<CPosixCrashReporter *> g_handlerOwner{ nullptr };

const int kFatalSignals[] = { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT };
constexpr int kFatalSignalCount = sizeof( kFatalSignals ) / sizeof( kFatalSignals[0] );

void FatalSignalHandler( int sig, siginfo_t *, void * );

class CPosixCrashReporter final : public ICrashReporter
{
public:
	explicit CPosixCrashReporter( const char *reportDir )
	{
		std::snprintf( m_dir, sizeof( m_dir ), "%s", reportDir );
	}

	~CPosixCrashReporter() override
	{
		if ( m_installed )
		{
			for ( int i = 0; i < kFatalSignalCount; ++i )
			{
				sigaction( kFatalSignals[i], &m_previous[i], nullptr );
			}
			sigaltstack( &m_previousStack, nullptr );
			delete[] static_cast<char *>( m_stack.ss_sp );
			g_handlerOwner.store( nullptr );
		}
	}

	bool InstallHandler()
	{
		CPosixCrashReporter *expected = nullptr;
		if ( !g_handlerOwner.compare_exchange_strong( expected, this ) )
		{
			return false;
		}
		const std::size_t stackBytes = 64 * 1024;
		m_stack.ss_sp = new char[stackBytes];
		m_stack.ss_size = stackBytes;
		m_stack.ss_flags = 0;
		sigaltstack( &m_stack, &m_previousStack );
		struct sigaction action{};
		action.sa_sigaction = FatalSignalHandler;
		action.sa_flags = SA_SIGINFO | SA_ONSTACK;
		sigemptyset( &action.sa_mask );
		for ( int i = 0; i < kFatalSignalCount; ++i )
		{
			sigaction( kFatalSignals[i], &action, &m_previous[i] );
		}
		m_installed = true;
		return true;
	}

	bool IsAvailable() const override { return true; }

	CrashReportResult SetAnnotation( const char *key, const char *value ) override
	{
		if ( !IsValidKey( key ) ||
		     ( value != nullptr &&
		         std::strlen( value ) > static_cast<std::size_t>( kCrashValueMaxBytes ) ) )
		{
			return CrashReportResult::kInvalidArgument;
		}
		std::lock_guard<std::mutex> lock( m_mutex );
		Annotation *slot = Find( key );
		if ( value == nullptr )
		{
			if ( slot != nullptr )
			{
				slot->used.store( false, std::memory_order_release );
			}
			return CrashReportResult::kOk;
		}
		if ( slot == nullptr )
		{
			for ( Annotation &a : m_annotations )
			{
				if ( !a.used.load( std::memory_order_relaxed ) )
				{
					slot = &a;
					std::snprintf( slot->key, sizeof( slot->key ), "%s", key );
					break;
				}
			}
			if ( slot == nullptr )
			{
				return CrashReportResult::kInvalidArgument;
			}
		}
		// Hide the slot from the handler while its value changes.
		slot->used.store( false, std::memory_order_release );
		std::snprintf( slot->value, sizeof( slot->value ), "%s", value );
		slot->used.store( true, std::memory_order_release );
		return CrashReportResult::kOk;
	}

	int GetAnnotation( const char *key, char *buffer, int bufferSize ) const override
	{
		if ( !IsValidKey( key ) || buffer == nullptr || bufferSize <= 0 )
		{
			return -1;
		}
		std::lock_guard<std::mutex> lock( m_mutex );
		const Annotation *slot = const_cast<CPosixCrashReporter *>( this )->Find( key );
		if ( slot == nullptr )
		{
			return -1;
		}
		const int n = static_cast<int>( std::strlen( slot->value ) );
		if ( n >= bufferSize )
		{
			return -1;
		}
		std::memcpy( buffer, slot->value, n + 1 );
		return n;
	}

	CrashReportResult WriteReport( const char *reason, char *idBuffer, int idBufferSize ) override
	{
		if ( reason == nullptr || idBuffer == nullptr || idBufferSize <= 0 )
		{
			return CrashReportResult::kInvalidArgument;
		}
		char id[64];
		const int sequence = m_nextReport.fetch_add( 1 );
		const int n = std::snprintf(
		    id, sizeof( id ), "report-%d-%d", static_cast<int>( getpid() ), sequence );
		if ( n >= idBufferSize )
		{
			m_nextReport.fetch_sub( 1 );
			return CrashReportResult::kInvalidArgument;
		}
		std::lock_guard<std::mutex> lock( m_mutex );
		if ( !Emit( id, reason, /*symbolize=*/true ) )
		{
			return CrashReportResult::kFailed;
		}
		std::memcpy( idBuffer, id, n + 1 );
		return CrashReportResult::kOk;
	}

	// Called from the signal handler: async-signal-safe calls only.
	void OnFatalSignal( int sig )
	{
		char id[64];
		SafeText name;
		name.Append( "crash-" );
		name.AppendUnsigned( static_cast<unsigned long long>( getpid() ) );
		name.Append( "-" );
		name.AppendUnsigned( static_cast<unsigned long long>( sig ) );
		std::memcpy( id, name.data, name.length + 1 );
		SafeText reason;
		reason.Append( "signal " );
		reason.AppendUnsigned( static_cast<unsigned long long>( sig ) );
		Emit( id, reason.data, /*symbolize=*/false );
		for ( int i = 0; i < kFatalSignalCount; ++i )
		{
			if ( kFatalSignals[i] == sig )
			{
				sigaction( sig, &m_previous[i], nullptr );
			}
		}
	}

private:
	Annotation *Find( const char *key )
	{
		for ( Annotation &a : m_annotations )
		{
			if ( a.used.load( std::memory_order_acquire ) && std::strcmp( a.key, key ) == 0 )
			{
				return &a;
			}
		}
		return nullptr;
	}

	// Writes "<dir>/<id>.txt". dladdr is used only off the signal path.
	bool Emit( const char *id, const char *reason, bool symbolize )
	{
		SafeText path;
		path.Append( m_dir );
		path.Append( "/" );
		path.Append( id );
		path.Append( ".txt" );
		const int fd = open( path.data, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644 );
		if ( fd < 0 )
		{
			return false;
		}
		SafeText text;
		text.Append( "reason: " );
		text.Append( reason );
		text.Append( "\npid: " );
		text.AppendUnsigned( static_cast<unsigned long long>( getpid() ) );
		timespec now{};
		clock_gettime( CLOCK_REALTIME, &now );
		text.Append( "\ntime: " );
		text.AppendUnsigned( static_cast<unsigned long long>( now.tv_sec ) );
		text.Append( "\n" );
		for ( const Annotation &a : m_annotations )
		{
			if ( a.used.load( std::memory_order_acquire ) )
			{
				text.Append( "annotation " );
				text.Append( a.key );
				text.Append( "=" );
				text.Append( a.value );
				text.Append( "\n" );
			}
		}
		text.Append( "backtrace:\n" );
		BacktraceState frames;
		_Unwind_Backtrace( CollectFrame, &frames );
		for ( int i = 0; i < frames.count; ++i )
		{
			text.Append( "  0x" );
			text.AppendUnsigned( reinterpret_cast<unsigned long long>( frames.frames[i] ), 16 );
			Dl_info info{};
			if ( symbolize && dladdr( frames.frames[i], &info ) != 0 )
			{
				text.Append( " " );
				text.Append( info.dli_sname != nullptr ? info.dli_sname : "?" );
				text.Append( " (" );
				text.Append( info.dli_fname != nullptr ? info.dli_fname : "?" );
				text.Append( ")" );
			}
			text.Append( "\n" );
		}
		std::size_t done = 0;
		bool ok = true;
		while ( done < text.length )
		{
			const ssize_t w = write( fd, text.data + done, text.length - done );
			if ( w < 0 && errno == EINTR )
			{
				continue;
			}
			if ( w <= 0 )
			{
				ok = false;
				break;
			}
			done += static_cast<std::size_t>( w );
		}
		close( fd );
		return ok;
	}

	char m_dir[512] = {};
	mutable std::mutex m_mutex;
	Annotation m_annotations[kCrashMaxAnnotations];
	std::atomic<int> m_nextReport{ 1 };
	bool m_installed = false;
	struct sigaction m_previous[kFatalSignalCount]{};
	stack_t m_stack{};
	stack_t m_previousStack{};
};

void FatalSignalHandler( int sig, siginfo_t *, void * )
{
	CPosixCrashReporter *owner = g_handlerOwner.load();
	if ( owner != nullptr )
	{
		owner->OnFatalSignal( sig );
	}
	else
	{
		signal( sig, SIG_DFL );
	}
	raise( sig );
}

class CUnavailableCrashReporter final : public ICrashReporter
{
public:
	bool IsAvailable() const override { return false; }
	CrashReportResult SetAnnotation( const char *, const char * ) override
	{
		return CrashReportResult::kUnsupported;
	}
	int GetAnnotation( const char *, char *, int ) const override { return -1; }
	CrashReportResult WriteReport( const char *, char *, int ) override
	{
		return CrashReportResult::kUnsupported;
	}
};

} // namespace

namespace
{

struct StackState
{
	void **frames;
	int max;
	int count;
	int skip;
};

_Unwind_Reason_Code CollectStackFrame( _Unwind_Context *context, void *arg )
{
	StackState &state = *static_cast<StackState *>( arg );
	const std::uintptr_t ip = static_cast<std::uintptr_t>( _Unwind_GetIP( context ) );
	if ( ip == 0 )
	{
		return _URC_END_OF_STACK;
	}
	if ( state.skip > 0 )
	{
		--state.skip;
		return _URC_NO_REASON;
	}
	state.frames[state.count++] = reinterpret_cast<void *>( ip );
	return state.count < state.max ? _URC_NO_REASON : _URC_END_OF_STACK;
}

class CPosixStackCapture final : public IStackCapture
{
public:
	__attribute__( ( noinline ) ) int CaptureStack( void **frames, int maxFrames ) override
	{
		if ( frames == nullptr || maxFrames <= 0 )
		{
			return 0;
		}
		// The first frame reported is this function's own.
		StackState state{ frames, maxFrames, 0, 1 };
		_Unwind_Backtrace( CollectStackFrame, &state );
		return state.count;
	}
};

std::atomic<void ( * )( void * )> g_watchdogFire{ nullptr };
std::atomic<void *> g_watchdogContext{ nullptr };

void OnWatchdogAlarm( int )
{
	void ( *fire )( void * ) = g_watchdogFire.exchange( nullptr );
	if ( fire != nullptr )
	{
		fire( g_watchdogContext.load() );
	}
}

class CPosixWatchdog final : public IWatchdog
{
public:
	~CPosixWatchdog() override { Disarm(); }

	bool IsSupported() const override { return true; }

	bool Arm( unsigned seconds, void ( *fire )( void * ), void *context ) override
	{
		if ( seconds == 0 || fire == nullptr )
		{
			return false;
		}
		g_watchdogContext.store( context );
		g_watchdogFire.store( fire );
		struct sigaction action{};
		action.sa_handler = OnWatchdogAlarm;
		sigemptyset( &action.sa_mask );
		sigaction( SIGALRM, &action, nullptr );
		alarm( seconds );
		return true;
	}

	void Disarm() override
	{
		alarm( 0 );
		g_watchdogFire.store( nullptr );
		struct sigaction action{};
		action.sa_handler = SIG_DFL;
		sigemptyset( &action.sa_mask );
		sigaction( SIGALRM, &action, nullptr );
	}
};

} // namespace

std::unique_ptr<IStackCapture> CreatePosixStackCapture()
{
	return std::make_unique<CPosixStackCapture>();
}

std::unique_ptr<IWatchdog> CreatePosixWatchdog()
{
	return std::make_unique<CPosixWatchdog>();
}

std::unique_ptr<IDebugOutput> CreateFdDebugOutput( int fd )
{
	return std::make_unique<CFdDebugOutput>( fd );
}

std::unique_ptr<IDebugOutput> CreateAndroidLogDebugOutput( const char *tag )
{
#if defined( __ANDROID__ )
	return std::make_unique<CAndroidLogDebugOutput>( tag );
#else
	(void)tag;
	return nullptr;
#endif
}

std::unique_ptr<ICrashReporter> CreatePosixCrashReporter(
    const char *reportDir, bool installHandler )
{
	if ( reportDir == nullptr || std::strlen( reportDir ) >= 400 )
	{
		return nullptr;
	}
	auto reporter = std::make_unique<CPosixCrashReporter>( reportDir );
	if ( installHandler && !reporter->InstallHandler() )
	{
		return nullptr;
	}
	return reporter;
}

std::unique_ptr<ICrashReporter> CreateUnavailableCrashReporter()
{
	return std::make_unique<CUnavailableCrashReporter>();
}

} // namespace platform
