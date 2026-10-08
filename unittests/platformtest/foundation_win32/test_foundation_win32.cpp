//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native conformance for the RFC 0001 foundation providers on Win32
//			(R26), built as a Windows PE and run under Wine by parity_wine.py.
//			Runs every shared suite against platform/win32, then the native
//			clauses: real access faults, the crash filter's report and the
//			original exit code, teardown (unjoined threads abort, the filter is
//			restored) and the failure paths. Crash and teardown cases run in
//			child processes of this executable.
//
//			Build/run: tools/quality/parity_wine.py check --rfc 0001 --domain Q-FOUNDATION
//
//=============================================================================//

#include "../clock/clock_conformance.h"
#include "../diagnostics/diagnostics_conformance.h"
#include "../file_probe/file_probe_conformance.h"
#include "../module_resolver/module_resolver_conformance.h"
#include "../paths/paths_conformance.h"
#include "../process_environment/process_environment_conformance.h"
#include "../thread/thread_conformance.h"
#include "../virtual_memory/virtual_memory_conformance.h"
#include "../wall_clock/wall_clock_conformance.h"
#include "../../../platform/resolver/module_resolver.h"
#include "../../../platform/win32/foundation_providers.h"
#include "testing/conformance_result.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace
{

int g_checks = 0;
int g_failures = 0;

void Check( bool ok, const char *what, int line )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::printf( "FAIL line %d: %s\n", line, what );
	}
}

#define NATIVE_CHECK( cond ) Check( ( cond ), #cond, __LINE__ )

template <typename Report> void Tally( const char *name, const Report &r )
{
	g_checks += r.checks;
	g_failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", name, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
	}
	else
	{
		std::printf( "ok %s: %d checks passed\n", name, r.checks );
	}
}

std::wstring SelfPath()
{
	wchar_t self[MAX_PATH];
	GetModuleFileNameW( nullptr, self, MAX_PATH );
	return self;
}

// Starts this executable with `commandLine` and `environment` (null = inherit)
// and returns its exit code.
DWORD RunSelf( const std::wstring &commandLine, const wchar_t *environment )
{
	std::fflush( stdout );
	std::wstring line = commandLine;
	STARTUPINFOW si{};
	si.cb = sizeof( si );
	PROCESS_INFORMATION pi{};
	const std::wstring self = SelfPath();
	if ( !CreateProcessW( self.c_str(), line.data(), nullptr, nullptr, TRUE,
	         environment != nullptr ? CREATE_UNICODE_ENVIRONMENT : 0,
	         const_cast<wchar_t *>( environment ), nullptr, &si, &pi ) )
	{
		return 0xffffffff;
	}
	WaitForSingleObject( pi.hProcess, INFINITE );
	DWORD code = 0;
	GetExitCodeProcess( pi.hProcess, &code );
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	return code;
}

DWORD RunMode( const wchar_t *mode, const std::wstring &arg = L"" )
{
	return RunSelf(
	    L"\"" + SelfPath() + L"\" " + mode + ( arg.empty() ? L"" : L" \"" + arg + L"\"" ),
	    nullptr );
}

std::string g_tempDir;

// ---------------------------------------------------------------------------
// Child modes: each exits abnormally on purpose.

int ChildMode( const char *mode, const char *arg )
{
	SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );
	if ( std::strcmp( mode, "--fault-readonly" ) == 0 ||
	     std::strcmp( mode, "--fault-reserved" ) == 0 )
	{
		auto vm = platform::CreateWin32VirtualMemory();
		platform::MemoryRegion region;
		vm->Reserve( vm->PageSize(), region );
		if ( std::strcmp( mode, "--fault-readonly" ) == 0 )
		{
			vm->Commit( region.base, vm->PageSize(), platform::PageAccess::kRead );
		}
		*static_cast<volatile char *>( region.base ) = 1;
		return 0;
	}
	if ( std::strcmp( mode, "--unjoined" ) == 0 )
	{
		auto threads = platform::CreateWin32Threads();
		platform::ThreadHandle h;
		threads->Start( {}, []( void * ) {}, nullptr, h );
		threads.reset();
		return 0;
	}
	if ( std::strcmp( mode, "--exception-callback" ) == 0 )
	{
		static std::string target;
		target = std::string( arg ) + "/callback.txt";
		platform::SetUnhandledExceptionCallback( []( unsigned long code, void *pointers ) {
			FILE *f = std::fopen( target.c_str(), "w" );
			if ( f != nullptr )
			{
				std::fprintf( f, "%08lx %d\n", code, pointers != nullptr );
				std::fclose( f );
			}
		} );
		volatile int *nowhere = nullptr;
		*nowhere = 1;
		return 0;
	}
	if ( std::strcmp( mode, "--crash" ) == 0 )
	{
		auto reporter = platform::CreateWin32CrashReporter( arg, true );
		if ( reporter == nullptr )
		{
			return 3;
		}
		reporter->SetAnnotation( "build", "r26-native" );
		volatile int *nowhere = nullptr;
		*nowhere = 1;
		return 0;
	}
	return 2;
}

// ---------------------------------------------------------------------------

void ClockSuites()
{
	auto clock = platform::CreateWin32MonotonicClock();
	Tally( "win32.monotonic-clock", platformtest::RunMonotonicClockConformance( *clock, 16 ) );
	auto wall = platform::CreateWin32WallClock();
	Tally( "win32.wall-clock", platformtest::RunWallClockConformance( *wall ) );

	FILETIME ft;
	GetSystemTimeAsFileTime( &ft );
	const long long nowSeconds =
	    ( ( ( static_cast<long long>( ft.dwHighDateTime ) << 32 ) | ft.dwLowDateTime ) -
	        116444736000000000LL ) /
	    10000000LL;
	const long long wallSeconds = wall->Now().unixNanoseconds / 1000000000LL;
	NATIVE_CHECK( wallSeconds >= nowSeconds - 1 && wallSeconds <= nowSeconds + 1 );
}

void ThreadSuites()
{
	auto clock = platform::CreateWin32MonotonicClock();
	{
		auto threads = platform::CreateWin32Threads();
		Tally( "win32.threads", platformtest::RunThreadConformance( *threads, *clock ) );
	}
	{
		auto threads = platform::CreateWin32Threads();
		struct Meet
		{
			std::atomic<int> arrived{ 0 };
			std::atomic<int> passed{ 0 };
			platform::IThreads *threads;
		} meet;
		meet.threads = threads.get();
		auto entry = []( void *p )
		{
			Meet &m = *static_cast<Meet *>( p );
			m.arrived.fetch_add( 1 );
			for ( int i = 0; i < 2000000 && m.arrived.load() < 2; ++i )
			{
				m.threads->SleepFor( 0 );
			}
			if ( m.arrived.load() == 2 )
			{
				m.passed.fetch_add( 1 );
			}
		};
		platform::ThreadHandle a, b;
		NATIVE_CHECK( threads->Start( {}, entry, &meet, a ) == platform::ThreadResult::kOk );
		NATIVE_CHECK( threads->Start( {}, entry, &meet, b ) == platform::ThreadResult::kOk );
		threads->Join( a );
		threads->Join( b );
		NATIVE_CHECK( meet.passed.load() == 2 );
	}
	// abort() exits with code 3 on the Windows CRT.
	NATIVE_CHECK( RunMode( L"--unjoined" ) == 3 );
}

// The backend-only extension Tier 0's exports use (R103).
struct NativeProbe
{
	std::atomic<unsigned long> id{ 0 };
	std::atomic<int> go{ 0 };
};

unsigned __stdcall NativeProc( void *arg )
{
	NativeProbe &probe = *static_cast<NativeProbe *>( arg );
	probe.id.store( GetCurrentThreadId() );
	while ( probe.go.load() == 0 )
	{
		SwitchToThread();
	}
	return 0x1234;
}

void Win32ThreadExtension()
{
	auto threads = platform::CreateWin32Threads();
	NativeProbe probe;
	platform::ThreadHandle handle;
	void *caller = nullptr;
	unsigned long id = 0;
	NATIVE_CHECK( threads->StartNative( 0, NativeProc, &probe, /*suspended=*/true, handle, caller, id ) ==
	              platform::ThreadResult::kOk );
	NATIVE_CHECK( caller != nullptr && id != 0 );
	NATIVE_CHECK( threads->WaitNative( caller, 30 ) == platform::IWin32Threads::WaitResult::kTimeout );
	NATIVE_CHECK( probe.id.load() == 0 ); // suspended: has not run
	NATIVE_CHECK( threads->IsNativeHandleRunning( caller ) );
	NATIVE_CHECK( threads->IsNativeIdRunning( id ) );
	NATIVE_CHECK( threads->SetNativePriority( caller, THREAD_PRIORITY_BELOW_NORMAL ) );
	NATIVE_CHECK( threads->GetNativePriority( caller ) == THREAD_PRIORITY_BELOW_NORMAL );
	NATIVE_CHECK( threads->ResumeNative( caller ) );
	while ( probe.id.load() == 0 )
	{
		SwitchToThread();
	}
	NATIVE_CHECK( probe.id.load() == id );
	probe.go.store( 1 );
	NATIVE_CHECK( threads->WaitNative( caller, 0xFFFFFFFF ) == platform::IWin32Threads::WaitResult::kSignaled );
	DWORD code = 0;
	NATIVE_CHECK( GetExitCodeThread( static_cast<HANDLE>( caller ), &code ) && code == 0x1234 );
	NATIVE_CHECK( !threads->IsNativeHandleRunning( caller ) );
	NATIVE_CHECK( threads->DetachNativeId( id ) == platform::ThreadResult::kOk );
	NATIVE_CHECK( threads->DetachNativeId( id ) == platform::ThreadResult::kInvalidArgument );
	NATIVE_CHECK( threads->CloseNative( caller ) ); // the caller's duplicate is its own
	NATIVE_CHECK( threads->CurrentNativeId() == GetCurrentThreadId() );
	NATIVE_CHECK( threads->CurrentPseudoHandle() == GetCurrentThread() );
}

void VirtualMemorySuites()
{
	auto vm = platform::CreateWin32VirtualMemory();
	Tally( "win32.virtual-memory", platformtest::RunVirtualMemoryConformance( *vm ) );
	SYSTEM_INFO info;
	GetSystemInfo( &info );
	NATIVE_CHECK( vm->AllocationGranularity() == info.dwAllocationGranularity );
	platform::MemoryRegion huge;
	NATIVE_CHECK( vm->Reserve( static_cast<std::size_t>( 1ULL << 62 ), huge ) ==
	              platform::MemoryResult::kOutOfMemory );
	NATIVE_CHECK( RunMode( L"--fault-readonly" ) == EXCEPTION_ACCESS_VIOLATION );
	NATIVE_CHECK( RunMode( L"--fault-reserved" ) == EXCEPTION_ACCESS_VIOLATION );
}

void ProcessEnvironmentSuites()
{
	auto env = platform::CreateWin32ProcessEnvironment();
	char self[1024] = {};
	env->GetArgument( 0, self, sizeof( self ) );
	const char *expected[5] = { self, "-game", "portal", "", "+map x" };
	platformtest::ProcessEnvironmentFixture fixture;
	fixture.arguments = expected;
	fixture.argumentCount = 5;
	fixture.presentName = "SOURCE_TEST_VAR";
	fixture.presentValue = "caf\xc3\xa9 = 1";
	fixture.emptyName = "SOURCE_TEST_EMPTY";
	fixture.absentName = "SOURCE_TEST_ABSENT";
	fixture.caseVariantName = "source_test_var";
	Tally( "win32.process-environment",
	    platformtest::RunProcessEnvironmentConformance( *env, fixture ) );
	NATIVE_CHECK( env->ProcessId() == GetCurrentProcessId() );
	NATIVE_CHECK( env->GetDebuggerState() == platform::DebuggerState::kNotAttached );
}

void PathsSuites()
{
	auto paths = platform::CreateWin32PlatformPaths();
	NATIVE_CHECK( paths != nullptr );
	if ( paths == nullptr )
	{
		return;
	}
	Tally( "win32.paths", platformtest::RunPlatformPathsConformance( *paths ) );
	char temp[1024] = {};
	if ( paths->GetPath( platform::PlatformPathId::kTemp, temp, sizeof( temp ) ) > 0 )
	{
		g_tempDir = temp;
	}
	NATIVE_CHECK( !g_tempDir.empty() );
}

// Reads records from a message-mode pipe: one message per Write.
class CPipeCapture : public platformtest::IDebugOutputCapture
{
public:
	explicit CPipeCapture( HANDLE pipe ) : m_pipe( pipe ) {}

	int Count() const override
	{
		Drain();
		return static_cast<int>( m_entries.size() );
	}

	bool Entry(
	    int index, platform::DiagnosticSeverity &severity, std::string &message ) const override
	{
		Drain();
		if ( index < 0 || index >= static_cast<int>( m_entries.size() ) )
		{
			return false;
		}
		severity = m_entries[index].first;
		message = m_entries[index].second;
		return true;
	}

	mutable bool framingValid = true;

private:
	void Drain() const
	{
		static char buffer[65536];
		for ( ;; )
		{
			DWORD available = 0;
			if ( !PeekNamedPipe( m_pipe, nullptr, 0, nullptr, &available, nullptr ) ||
			     available == 0 )
			{
				return;
			}
			DWORD n = 0;
			if ( !ReadFile( m_pipe, buffer, sizeof( buffer ), &n, nullptr ) )
			{
				framingValid = false; // ERROR_MORE_DATA: a record larger than the buffer
				return;
			}
			std::string record( buffer, n );
			platform::DiagnosticSeverity severity = platform::DiagnosticSeverity::kInfo;
			std::size_t skip = 0;
			if ( record.rfind( "[info] ", 0 ) == 0 )
			{
				skip = 7;
			}
			else if ( record.rfind( "[warn] ", 0 ) == 0 )
			{
				severity = platform::DiagnosticSeverity::kWarning;
				skip = 7;
			}
			else if ( record.rfind( "[error] ", 0 ) == 0 )
			{
				severity = platform::DiagnosticSeverity::kError;
				skip = 8;
			}
			else
			{
				framingValid = false;
			}
			if ( record.empty() || record.back() != '\n' )
			{
				framingValid = false;
			}
			else
			{
				record.pop_back();
			}
			m_entries.push_back( { severity, record.substr( skip ) } );
		}
	}

	HANDLE m_pipe;
	mutable std::vector<std::pair<platform::DiagnosticSeverity, std::string>> m_entries;
};

std::string ReadText( const std::string &path )
{
	std::string text;
	FILE *f = std::fopen( path.c_str(), "rb" );
	char chunk[4096];
	std::size_t n;
	while ( f != nullptr && ( n = std::fread( chunk, 1, sizeof( chunk ), f ) ) > 0 )
	{
		text.append( chunk, n );
	}
	if ( f != nullptr )
	{
		std::fclose( f );
	}
	return text;
}

void DiagnosticsSuites()
{
	{
		const wchar_t *name = L"\\\\.\\pipe\\r26-debug-output";
		HANDLE server = CreateNamedPipeW( name, PIPE_ACCESS_INBOUND,
		    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 1 << 20, 0, nullptr );
		HANDLE client = CreateFileW( name, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr );
		NATIVE_CHECK( server != INVALID_HANDLE_VALUE && client != INVALID_HANDLE_VALUE );
		auto output = platform::CreateWin32HandleDebugOutput( client );
		CPipeCapture capture( server );
		Tally( "win32.debug-output", platformtest::RunDebugOutputConformance( *output, capture ) );
		NATIVE_CHECK( capture.framingValid );
		CloseHandle( client );
		CloseHandle( server );
		auto debugger = platform::CreateWin32DebuggerOutput();
		debugger->Write( platform::DiagnosticSeverity::kInfo, "r26 debugger output" );
		debugger->Write( platform::DiagnosticSeverity::kInfo, nullptr );
	}

	const std::string dir = g_tempDir + "/r26-crash-" + std::to_string( GetCurrentProcessId() );
	NATIVE_CHECK( CreateDirectoryA( dir.c_str(), nullptr ) != 0 );
	{
		auto reporter = platform::CreateWin32CrashReporter( dir.c_str(), false );
		Tally( "win32.crash-reporter", platformtest::RunCrashReporterConformance( *reporter ) );
		reporter->SetAnnotation( "map.name", "sp_a1_intro4" );
		char id[64];
		NATIVE_CHECK( reporter->WriteReport( "on demand", id, sizeof( id ) ) ==
		              platform::CrashReportResult::kOk );
		const std::string text = ReadText( dir + "/" + id + ".txt" );
		NATIVE_CHECK( text.find( "reason: on demand\n" ) != std::string::npos );
		NATIVE_CHECK( text.find( "annotation map.name=sp_a1_intro4\n" ) != std::string::npos );
		NATIVE_CHECK( text.find( "backtrace:\n  0x" ) != std::string::npos );

		const std::string gone = dir + "-gone";
		CreateDirectoryA( gone.c_str(), nullptr );
		auto orphan = platform::CreateWin32CrashReporter( gone.c_str(), false );
		RemoveDirectoryA( gone.c_str() );
		std::strcpy( id, "untouched" );
		NATIVE_CHECK(
		    orphan->WriteReport( "x", id, sizeof( id ) ) == platform::CrashReportResult::kFailed );
		NATIVE_CHECK( std::strcmp( id, "untouched" ) == 0 );
	}

	// The filter: a crash writes a report, the process still ends with the
	// original exception code.
	std::wstring wdir( dir.begin(), dir.end() );
	NATIVE_CHECK( RunMode( L"--crash", wdir ) == EXCEPTION_ACCESS_VIOLATION );
	WIN32_FIND_DATAA found;
	std::string crash;
	HANDLE search = FindFirstFileA( ( dir + "/crash-*.txt" ).c_str(), &found );
	if ( search != INVALID_HANDLE_VALUE )
	{
		crash = ReadText( dir + "/" + found.cFileName );
		FindClose( search );
	}
	NATIVE_CHECK( crash.find( "reason: exception 0xc0000005\n" ) != std::string::npos );
	NATIVE_CHECK( crash.find( "annotation build=r26-native\n" ) != std::string::npos );

	// Stack capture, the (unsupported) watchdog, and the exception callback (R103).
	{
		auto capture = platform::CreateWin32StackCapture();
		Tally( "win32.stack-capture", platformtest::RunStackCaptureConformance( *capture ) );
		auto watchdog = platform::CreateWin32Watchdog();
		Tally( "win32.watchdog[unsupported]",
		    platformtest::RunWatchdogConformance( *watchdog, []( unsigned ms ) { Sleep( ms ); } ) );
		NATIVE_CHECK( RunMode( L"--exception-callback", wdir ) == EXCEPTION_ACCESS_VIOLATION );
		NATIVE_CHECK( !ReadText( dir + "/callback.txt" ).empty() );
		NATIVE_CHECK( ReadText( dir + "/callback.txt" ).find( "c0000005" ) != std::string::npos );
		DeleteFileA( ( dir + "/callback.txt" ).c_str() );
	}

	// Teardown: one filter owner at a time, and destruction restores whatever
	// filter was installed before (the C runtime installs its own).
	const LPTOP_LEVEL_EXCEPTION_FILTER before = SetUnhandledExceptionFilter( nullptr );
	SetUnhandledExceptionFilter( before );
	{
		auto first = platform::CreateWin32CrashReporter( dir.c_str(), true );
		NATIVE_CHECK( first != nullptr );
		NATIVE_CHECK( platform::CreateWin32CrashReporter( dir.c_str(), true ) == nullptr );
		const LPTOP_LEVEL_EXCEPTION_FILTER during = SetUnhandledExceptionFilter( nullptr );
		SetUnhandledExceptionFilter( during );
		NATIVE_CHECK( during != before );
	}
	const LPTOP_LEVEL_EXCEPTION_FILTER restored = SetUnhandledExceptionFilter( nullptr );
	SetUnhandledExceptionFilter( restored );
	NATIVE_CHECK( restored == before );
	NATIVE_CHECK( platform::CreateWin32CrashReporter( dir.c_str(), true ) != nullptr );

	// Clean the report directory.
	search = FindFirstFileA( ( dir + "/*.txt" ).c_str(), &found );
	while ( search != INVALID_HANDLE_VALUE )
	{
		DeleteFileA( ( dir + "/" + found.cFileName ).c_str() );
		if ( !FindNextFileA( search, &found ) )
		{
			FindClose( search );
			break;
		}
	}
	RemoveDirectoryA( dir.c_str() );
}

void FileProbeAndResolverSuites()
{
	wchar_t tempDir[MAX_PATH];
	GetTempPathW( MAX_PATH, tempDir );
	const std::wstring root =
	    std::wstring( tempDir ) + L"r11-probe-" + std::to_wstring( GetCurrentProcessId() );
	NATIVE_CHECK( CreateDirectoryW( root.c_str(), nullptr ) != 0 );
	auto touch = []( const std::wstring &path )
	{
		HANDLE h = CreateFileW( path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, 0, nullptr );
		if ( h != INVALID_HANDLE_VALUE )
		{
			CloseHandle( h );
			return true;
		}
		return false;
	};
	const std::wstring nonBmp = root + L"\\\U0001F600.dll";
	const std::wstring lone = root + L"\\a" + wchar_t( 0xd800 ) + L".dll";
	CreateDirectoryW( ( root + L"\\bin" ).c_str(), nullptr );
	touch( root + L"\\engine.dll" );
	touch( root + L"\\bin\\server.dll" );
	NATIVE_CHECK( touch( nonBmp ) );
	const bool loneCreated = touch( lone );

	using platform::FileKind;
	auto W = []( const std::wstring &s )
	{
		return platform::Win32NativePath( s.c_str() );
	};
	platformtest::FileProbeFixture fixture;
	fixture.cases = {
	    { W( root + L"\\engine.dll" ), FileKind::kRegularFile, "regular file" },
	    { W( root + L"/engine.dll" ), FileKind::kRegularFile, "forward separator" },
	    { W( root + L"\\bin" ), FileKind::kDirectory, "directory" },
	    { W( root + L"\\missing.dll" ), FileKind::kMissing, "missing" },
	    { W( root + L"\\engine.dll\\x" ), FileKind::kMissing, "below a file" },
	    { W( nonBmp ), FileKind::kRegularFile, "non-BMP name" },
	    { W( L"\\\\.\\NUL" ), FileKind::kOther, "device" },
	};
	// Wine keeps names as UTF-8 on the host and cannot store an unpaired
	// surrogate; Windows can. The case runs where the platform allows it.
	if ( loneCreated )
	{
		fixture.cases.push_back( { W( lone ), FileKind::kRegularFile, "unpaired surrogate name" } );
	}
	else
	{
		std::printf( "note win32.file-probe: this platform refuses unpaired-surrogate names; case "
		             "not applicable\n" );
	}
	fixture.foreignFlavor = platformtest::Posix( "/usr/lib" );
	fixture.countEntries = [root]()
	{
		int n = 0;
		WIN32_FIND_DATAW found;
		HANDLE h = FindFirstFileW( ( root + L"\\*" ).c_str(), &found );
		while ( h != INVALID_HANDLE_VALUE )
		{
			++n;
			if ( !FindNextFileW( h, &found ) )
			{
				FindClose( h );
				break;
			}
		}
		return n;
	};
	auto probe = platform::CreateWin32FileProbe();
	Tally( "win32.file-probe", platformtest::RunFileProbeConformance( *probe, fixture ) );

	auto resolver = platform::CreateModuleResolver( *probe, nullptr );
	platform::ModuleSearchPolicy policy;
	policy.roots = { W( root ) };
	policy.patterns = { { "bin", "" }, { "", "" } };
	policy.extension = ".dll";
	auto server = resolver->Resolve( "server", policy );
	NATIVE_CHECK( server && server.Value().path == W( root + L"\\bin\\server.dll" ) );
	auto engine = resolver->Resolve( "engine.so", policy );
	NATIVE_CHECK( engine && engine.Value().path == W( root + L"\\engine.dll" ) );
	auto smile = resolver->Resolve( "\xf0\x9f\x98\x80", policy );
	NATIVE_CHECK( smile && smile.Value().path == W( nonBmp ) );
	NATIVE_CHECK(
	    platform::Win32CurrentDirectory().Flavor() == platform::NativePathFlavor::kWindowsUtf16 );

	for ( const std::wstring &f :
	    { root + L"\\engine.dll", root + L"\\bin\\server.dll", nonBmp, lone } )
	{
		DeleteFileW( f.c_str() );
	}
	RemoveDirectoryW( ( root + L"\\bin" ).c_str() );
	RemoveDirectoryW( root.c_str() );
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc >= 2 && std::strncmp( argv[1], "--", 2 ) == 0 &&
	     std::strcmp( argv[1], "--fixture" ) != 0 )
	{
		return ChildMode( argv[1], argc >= 3 ? argv[2] : "" );
	}
	if ( std::getenv( "SOURCE_R26_CHILD" ) == nullptr )
	{
		// Re-run with the fixture's command line and environment.
		std::wstring env;
		for ( const wchar_t *e :
		    { L"SOURCE_R26_CHILD=1", L"SOURCE_TEST_VAR=café = 1", L"SOURCE_TEST_EMPTY=" } )
		{
			env += e;
			env += L'\0';
		}
		// Keep what Windows needs to find its folders.
		for ( const wchar_t *keep : { L"SystemRoot", L"TEMP", L"TMP", L"LOCALAPPDATA",
		          L"USERPROFILE", L"WINEPREFIX", L"PATH" } )
		{
			wchar_t value[4096];
			const DWORD n = GetEnvironmentVariableW( keep, value, 4096 );
			if ( n > 0 && n < 4096 )
			{
				env += std::wstring( keep ) + L"=" + value;
				env += L'\0';
			}
		}
		env += L'\0';
		const std::wstring line = L"\"" + SelfPath() + L"\" -game portal \"\" \"+map x\"";
		const DWORD code = RunSelf( line, env.c_str() );
		return static_cast<int>( code );
	}
	(void)argv;

	ClockSuites();
	ThreadSuites();
	Win32ThreadExtension();
	VirtualMemorySuites();
	ProcessEnvironmentSuites();
	PathsSuites();
	DiagnosticsSuites();
	FileProbeAndResolverSuites();
	return testing::ReportConformance( g_checks, g_failures );
}
