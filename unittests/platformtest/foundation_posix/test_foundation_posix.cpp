//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native conformance for the RFC 0001 foundation providers on POSIX
//			(R26). Runs every shared suite against the real providers in
//			platform/posix, then the clauses only a native provider can be
//			judged on: real faults on protected pages, the crash handler's report
//			and re-raise, teardown (unjoined threads abort, handlers restored),
//			and the failure paths (address space refused, report directory gone).
//
//			The process re-executes itself first with fixed arguments and
//			environment, so the process-environment fixture is exact.
//
//			Build/run: tools/quality/conformance.py check --suite platform.foundation.posix
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
#include "../../../platform/posix/foundation_providers.h"
#include "../../../platform/resolver/module_resolver.h"
#include "testing/conformance_result.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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

// Runs `body` in a forked child and returns its wait status.
template <typename Body> int InChild( Body body )
{
	std::fflush( stdout );
	const pid_t pid = fork();
	if ( pid == 0 )
	{
		body();
		_exit( 0 );
	}
	int status = 0;
	waitpid( pid, &status, 0 );
	return status;
}

const char *const kChildArgs[] = { nullptr, "-game", "portal", "", "+map x" };
const char kChildMarker[] = "SOURCE_R26_CHILD";

// ---------------------------------------------------------------------------

void ClockSuites()
{
	auto clock = platform::CreatePosixMonotonicClock();
	Tally( "posix.monotonic-clock", platformtest::RunMonotonicClockConformance( *clock, 16 ) );

	auto wall = platform::CreatePosixWallClock();
	Tally( "posix.wall-clock[TZ=Asia/Kolkata]", platformtest::RunWallClockConformance( *wall ) );

	// The wall clock agrees with time(2) and moves with the monotonic clock.
	const std::int64_t before = static_cast<std::int64_t>( time( nullptr ) );
	const std::int64_t wallSeconds = wall->Now().unixNanoseconds / 1000000000LL;
	NATIVE_CHECK( wallSeconds >= before && wallSeconds <= before + 1 );

	// A real local zone: Asia/Kolkata is UTC+5:30 with no daylight saving.
	platform::CivilTime local;
	platform::WallTime instant;
	instant.unixNanoseconds = 1759926896LL * 1000000000LL; // 2025-10-08T12:34:56Z
	const bool ok = wall->ToCivil( instant, platform::CivilZone::kLocal, local );
	NATIVE_CHECK( ok && local.utcOffsetSeconds == 5 * 3600 + 1800 );
	NATIVE_CHECK( ok && local.hour == 18 && local.minute == 4 && local.second == 56 );
}

void ThreadSuites()
{
	auto clock = platform::CreatePosixMonotonicClock();
	{
		auto threads = platform::CreatePosixThreads();
		Tally( "posix.threads", platformtest::RunThreadConformance( *threads, *clock ) );
	}

	// Threads really run concurrently: two threads meet at a rendezvous that
	// neither can pass alone.
	{
		auto threads = platform::CreatePosixThreads();
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

	// A requested stack is honoured: a thread with 4 MiB uses 2 MiB of it.
	{
		auto threads = platform::CreatePosixThreads();
		platform::ThreadOptions options;
		options.stackBytes = 4 * 1024 * 1024;
		std::atomic<int> sum{ 0 };
		auto entry = []( void *p )
		{
			volatile char big[2 * 1024 * 1024];
			for ( std::size_t i = 0; i < sizeof( big ); i += 4096 )
			{
				big[i] = 1;
			}
			static_cast<std::atomic<int> *>( p )->store( big[4096] );
		};
		platform::ThreadHandle h;
		NATIVE_CHECK( threads->Start( options, entry, &sum, h ) == platform::ThreadResult::kOk );
		threads->Join( h );
		NATIVE_CHECK( sum.load() == 1 );
	}

	// Teardown: destroying the provider with an unjoined thread aborts.
	const int status = InChild(
	    []
	    {
		    auto threads = platform::CreatePosixThreads();
		    platform::ThreadHandle h;
		    threads->Start( {}, []( void * ) {}, nullptr, h );
		    threads.reset();
	    } );
	NATIVE_CHECK( WIFSIGNALED( status ) && WTERMSIG( status ) == SIGABRT );
}

void VirtualMemorySuites()
{
	{
		auto vm = platform::CreatePosixVirtualMemory();
		Tally( "posix.virtual-memory", platformtest::RunVirtualMemoryConformance( *vm ) );
		NATIVE_CHECK( vm->PageSize() == static_cast<std::size_t>( sysconf( _SC_PAGESIZE ) ) );

		// Failure: the platform refuses absurd address space.
		platform::MemoryRegion huge;
		// 2^62 bytes on 64-bit; on 32-bit, all but 64 pages of the address space.
		const std::size_t absurd = sizeof( void * ) == 8 ? static_cast<std::size_t>( 1ULL << 62 )
		                                                 : SIZE_MAX - 64 * vm->PageSize();
		NATIVE_CHECK( vm->Reserve( absurd, huge ) == platform::MemoryResult::kOutOfMemory );
		NATIVE_CHECK( huge.base == nullptr );
	}

	// Access is enforced: writing a read-only page and touching a reserved
	// page both fault.
	auto faultsOn = []( bool readOnly )
	{
		return InChild(
		    [readOnly]
		    {
			    auto vm = platform::CreatePosixVirtualMemory();
			    platform::MemoryRegion region;
			    vm->Reserve( vm->PageSize(), region );
			    if ( readOnly )
			    {
				    vm->Commit( region.base, vm->PageSize(), platform::PageAccess::kRead );
			    }
			    *static_cast<volatile char *>( region.base ) = 1;
		    } );
	};
	const int readOnly = faultsOn( true );
	const int reserved = faultsOn( false );
	NATIVE_CHECK( WIFSIGNALED( readOnly ) &&
	              ( WTERMSIG( readOnly ) == SIGSEGV || WTERMSIG( readOnly ) == SIGBUS ) );
	NATIVE_CHECK( WIFSIGNALED( reserved ) &&
	              ( WTERMSIG( reserved ) == SIGSEGV || WTERMSIG( reserved ) == SIGBUS ) );

	// Cleanup: destroying the provider unmaps what it still holds.
	void *leaked = nullptr;
	{
		auto vm = platform::CreatePosixVirtualMemory();
		platform::MemoryRegion region;
		vm->Reserve( vm->PageSize(), region );
		leaked = region.base;
	}
	NATIVE_CHECK(
	    msync( leaked, static_cast<std::size_t>( sysconf( _SC_PAGESIZE ) ), MS_ASYNC ) == -1 );
}

void ProcessEnvironmentSuites( int argc, char **argv )
{
	auto env = platform::CreatePosixProcessEnvironment( argc, argv );
	platformtest::ProcessEnvironmentFixture fixture;
	const char *expected[5] = {
	    argv[0], kChildArgs[1], kChildArgs[2], kChildArgs[3], kChildArgs[4] };
	fixture.arguments = expected;
	fixture.argumentCount = 5;
	fixture.presentName = "SOURCE_TEST_VAR";
	fixture.presentValue = "caf\xc3\xa9 = 1";
	fixture.emptyName = "SOURCE_TEST_EMPTY";
	fixture.absentName = "SOURCE_TEST_ABSENT";
	fixture.caseVariantName = "source_test_var";
	Tally( "posix.process-environment",
	    platformtest::RunProcessEnvironmentConformance( *env, fixture ) );
	NATIVE_CHECK( env->ProcessId() == static_cast<std::uint64_t>( getpid() ) );
	NATIVE_CHECK( env->GetDebuggerState() == platform::DebuggerState::kNotAttached );

	// The snapshot does not follow later changes.
	setenv( "SOURCE_TEST_VAR", "changed", 1 );
	char value[64];
	NATIVE_CHECK( env->GetVariable( "SOURCE_TEST_VAR", value, sizeof( value ) ) == 9 );
	NATIVE_CHECK( std::strcmp( value, "caf\xc3\xa9 = 1" ) == 0 );
}

void PathsSuites()
{
	auto paths = platform::CreateLinuxPlatformPaths();
	NATIVE_CHECK( paths != nullptr );
	if ( paths == nullptr )
	{
		return;
	}
	Tally( "posix.paths[linux]", platformtest::RunPlatformPathsConformance( *paths ) );

	char exe[PATH_MAX] = {};
	char reported[PATH_MAX] = {};
	NATIVE_CHECK( realpath( "/proc/self/exe", exe ) != nullptr );
	NATIVE_CHECK( paths->GetPath( platform::PlatformPathId::kExecutableFile, reported,
	                  sizeof( reported ) ) > 0 );
	NATIVE_CHECK( std::strcmp( exe, reported ) == 0 );
	NATIVE_CHECK(
	    paths->GetPath( platform::PlatformPathId::kTemp, reported, sizeof( reported ) ) > 0 );
	NATIVE_CHECK( std::strcmp( reported, "/tmp/r26-tmp" ) == 0 ); // TMPDIR=/tmp//r26-tmp/

	// Supplied paths (app containers): normalized, and refused when relative.
	platform::PlatformPathValues values;
	values.executableFile = "/data/app/x/lib/arm64/libmain.so";
	values.userData = "/data/user/0/com.panos.sourceengine/files/";
	values.temp = "/data/user/0/com.panos.sourceengine//cache";
	values.nativeLibraryDir = "/data/app/x/lib/./arm64";
	auto supplied = platform::CreateSuppliedPlatformPaths( values );
	NATIVE_CHECK( supplied != nullptr );
	if ( supplied != nullptr )
	{
		Tally( "posix.paths[supplied]", platformtest::RunPlatformPathsConformance( *supplied ) );
		NATIVE_CHECK( supplied->GetPath( platform::PlatformPathId::kNativeLibraryDir, reported,
		                  sizeof( reported ) ) > 0 &&
		              std::strcmp( reported, "/data/app/x/lib/arm64" ) == 0 );
	}
	values.temp = "relative/cache";
	NATIVE_CHECK( platform::CreateSuppliedPlatformPaths( values ) == nullptr );
}

// Reads framed records back from a packet socket: one record per Write.
class CPacketCapture : public platformtest::IDebugOutputCapture
{
public:
	explicit CPacketCapture( int fd ) : m_fd( fd ) {}

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

	bool framingValid = true;

private:
	void Drain() const
	{
		static char buffer[65536];
		for ( ;; )
		{
			const ssize_t n = recv( m_fd, buffer, sizeof( buffer ), MSG_DONTWAIT );
			if ( n <= 0 )
			{
				return;
			}
			std::string record( buffer, static_cast<std::size_t>( n ) );
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
				const_cast<CPacketCapture *>( this )->framingValid = false;
			}
			if ( record.empty() || record.back() != '\n' )
			{
				const_cast<CPacketCapture *>( this )->framingValid = false;
			}
			else
			{
				record.pop_back();
			}
			m_entries.push_back( { severity, record.substr( skip ) } );
		}
	}

	int m_fd;
	mutable std::vector<std::pair<platform::DiagnosticSeverity, std::string>> m_entries;
};

std::vector<std::string> ReportFiles( const char *dir )
{
	std::vector<std::string> names;
	DIR *d = opendir( dir );
	for ( dirent *e = d != nullptr ? readdir( d ) : nullptr; e != nullptr; e = readdir( d ) )
	{
		if ( e->d_name[0] != '.' )
		{
			names.push_back( e->d_name );
		}
	}
	if ( d != nullptr )
	{
		closedir( d );
	}
	return names;
}

std::string ReadFile( const std::string &path )
{
	std::string text;
	FILE *f = std::fopen( path.c_str(), "r" );
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
	// Debug output through a packet socket, so record boundaries are visible.
	{
		int fds[2];
		NATIVE_CHECK( socketpair( AF_UNIX, SOCK_SEQPACKET, 0, fds ) == 0 );
		int big = 4 * 1024 * 1024;
		setsockopt( fds[0], SOL_SOCKET, SO_SNDBUF, &big, sizeof( big ) );
		setsockopt( fds[1], SOL_SOCKET, SO_RCVBUF, &big, sizeof( big ) );
		auto output = platform::CreateFdDebugOutput( fds[0] );
		CPacketCapture capture( fds[1] );
		Tally( "posix.debug-output", platformtest::RunDebugOutputConformance( *output, capture ) );
		NATIVE_CHECK( capture.framingValid );
		close( fds[0] );
		close( fds[1] );
	}
#if defined( __ANDROID__ )
	// logcat has no in-process reader; the provider exists and accepts every
	// severity and an ignored null. Its records are checked with `adb logcat`.
	{
		auto logcat = platform::CreateAndroidLogDebugOutput( "r26" );
		NATIVE_CHECK( logcat != nullptr );
		if ( logcat != nullptr )
		{
			logcat->Write( platform::DiagnosticSeverity::kInfo, "r26 info" );
			logcat->Write( platform::DiagnosticSeverity::kWarning, "r26 warn" );
			logcat->Write( platform::DiagnosticSeverity::kError, "r26 error" );
			logcat->Write( platform::DiagnosticSeverity::kError, nullptr );
		}
	}
	char dir[] = "/data/local/tmp/r26-crash-XXXXXX";
	char missing[] = "/data/local/tmp/r26-gone-XXXXXX";
#else
	NATIVE_CHECK( platform::CreateAndroidLogDebugOutput( "test" ) == nullptr ); // not Android
	char dir[] = "/tmp/r26-crash-XXXXXX";
	char missing[] = "/tmp/r26-gone-XXXXXX";
#endif
	NATIVE_CHECK( mkdtemp( dir ) != nullptr );

	// On-demand reports through the shared suite, then their contents.
	{
		auto reporter = platform::CreatePosixCrashReporter( dir, /*installHandler=*/false );
		NATIVE_CHECK( reporter != nullptr );
		Tally( "posix.crash-reporter", platformtest::RunCrashReporterConformance( *reporter ) );
		reporter->SetAnnotation( "map.name", "sp_a1_intro4" );
		char id[64];
		NATIVE_CHECK( reporter->WriteReport( "on demand", id, sizeof( id ) ) ==
		              platform::CrashReportResult::kOk );
		const std::string text = ReadFile( std::string( dir ) + "/" + id + ".txt" );
		NATIVE_CHECK( text.find( "reason: on demand\n" ) != std::string::npos );
		NATIVE_CHECK( text.find( "annotation map.name=sp_a1_intro4\n" ) != std::string::npos );
		NATIVE_CHECK( text.find( "backtrace:\n  0x" ) != std::string::npos );

		// Failure: the report directory has gone.
		NATIVE_CHECK( mkdtemp( missing ) != nullptr );
		auto orphan = platform::CreatePosixCrashReporter( missing, false );
		rmdir( missing );
		std::strcpy( id, "untouched" );
		NATIVE_CHECK(
		    orphan->WriteReport( "x", id, sizeof( id ) ) == platform::CrashReportResult::kFailed );
		NATIVE_CHECK( std::strcmp( id, "untouched" ) == 0 );
	}
	NATIVE_CHECK( platform::CreatePosixCrashReporter( nullptr, false ) == nullptr );

	// The handler: a crash writes a report with the annotations, then the
	// process still dies by the original signal.
	const std::size_t reportsBefore = ReportFiles( dir ).size();
	const int status = InChild(
	    [&dir]
	    {
		    auto reporter = platform::CreatePosixCrashReporter( dir, /*installHandler=*/true );
		    if ( reporter == nullptr )
		    {
			    _exit( 3 );
		    }
		    reporter->SetAnnotation( "build", "r26-native" );
		    volatile int *nowhere = nullptr;
		    *nowhere = 1;
	    } );
	NATIVE_CHECK( WIFSIGNALED( status ) && WTERMSIG( status ) == SIGSEGV );
	std::string crashReport;
	for ( const std::string &name : ReportFiles( dir ) )
	{
		if ( name.rfind( "crash-", 0 ) == 0 )
		{
			crashReport = ReadFile( std::string( dir ) + "/" + name );
		}
	}
	NATIVE_CHECK( ReportFiles( dir ).size() == reportsBefore + 1 );
	NATIVE_CHECK( crashReport.find( "reason: signal 11\n" ) != std::string::npos );
	NATIVE_CHECK( crashReport.find( "annotation build=r26-native\n" ) != std::string::npos );

	// Teardown: one handler owner at a time, and destruction restores the
	// previous handlers.
	{
		struct sigaction before{};
		sigaction( SIGSEGV, nullptr, &before );
		{
			auto first = platform::CreatePosixCrashReporter( dir, true );
			NATIVE_CHECK( first != nullptr );
			NATIVE_CHECK( platform::CreatePosixCrashReporter( dir, true ) == nullptr );
			struct sigaction during{};
			sigaction( SIGSEGV, nullptr, &during );
			NATIVE_CHECK( during.sa_sigaction != before.sa_sigaction );
		}
		struct sigaction after{};
		sigaction( SIGSEGV, nullptr, &after );
		NATIVE_CHECK( after.sa_handler == before.sa_handler );
		auto again = platform::CreatePosixCrashReporter( dir, true );
		NATIVE_CHECK( again != nullptr );
	}

	auto unavailable = platform::CreateUnavailableCrashReporter();
	Tally( "posix.crash-reporter[unavailable]",
	    platformtest::RunCrashReporterConformance( *unavailable ) );

	for ( const std::string &name : ReportFiles( dir ) )
	{
		unlink( ( std::string( dir ) + "/" + name ).c_str() );
	}
	rmdir( dir );
}

// The probe and the resolver over a real directory: every file kind, links,
// a name that is not UTF-8, and resolution through the legacy patterns from a
// root whose own name is not UTF-8.
class CLstatProbe final : public platform::IFileProbe // DEFECT: does not follow links
{
public:
	platform::FileKind Probe( const platform::NativePath &path ) const override
	{
		struct stat info{};
		const std::string &bytes = platform::NativePathAccess::PosixBytes( path );
		if ( path.Flavor() != platform::NativePathFlavor::kPosixBytes ||
		     lstat( bytes.c_str(), &info ) != 0 )
		{
			return platform::FileKind::kMissing;
		}
		return S_ISREG( info.st_mode )   ? platform::FileKind::kRegularFile
		       : S_ISDIR( info.st_mode ) ? platform::FileKind::kDirectory
		                                 : platform::FileKind::kOther;
	}
};

class CAnythingIsAFile final : public platform::IFileProbe // DEFECT: no kinds
{
public:
	explicit CAnythingIsAFile( const platform::IFileProbe &inner ) : m_inner( inner ) {}
	platform::FileKind Probe( const platform::NativePath &path ) const override
	{
		return m_inner.Probe( path ) == platform::FileKind::kMissing
		           ? platform::FileKind::kMissing
		           : platform::FileKind::kRegularFile;
	}

private:
	const platform::IFileProbe &m_inner;
};

class CLossyNames final : public platform::IFileProbe // DEFECT: decodes names as UTF-8
{
public:
	explicit CLossyNames( const platform::IFileProbe &inner ) : m_inner( inner ) {}
	platform::FileKind Probe( const platform::NativePath &path ) const override
	{
		return m_inner.Probe( platform::PosixNativePath( path.ToDisplayString().c_str() ) );
	}

private:
	const platform::IFileProbe &m_inner;
};

void FileProbeAndResolverSuites()
{
#if defined( __ANDROID__ )
	char dir[] = "/data/local/tmp/r11-probe-XXXXXX";
#else
	char dir[] = "/tmp/r11-probe-XXXXXX";
#endif
	char *base = mkdtemp( dir );
	NATIVE_CHECK( base != nullptr );
	if ( base == nullptr )
	{
		return;
	}
	const std::string root = base;
	const std::string odd = root + "/\xff\xfe-mod"; // a directory name that is not UTF-8
	auto touch = []( const std::string &path )
	{
		const int fd = open( path.c_str(), O_CREAT | O_WRONLY, 0644 );
		if ( fd >= 0 )
		{
			close( fd );
		}
	};
	mkdir( ( root + "/bin" ).c_str(), 0755 );
	mkdir( odd.c_str(), 0755 );
	mkdir( ( odd + "/bin" ).c_str(), 0755 );
	touch( root + "/engine.so" );
	touch( root + "/bin/libserver.so" );
	touch( root + "/\xc3\xa9t\xc3\xa9.so" );
	touch( root + "/\xff\xfe.so" );
	touch( odd + "/bin/libclient.so" );
	symlink( "engine.so", ( root + "/link.so" ).c_str() );
	symlink( "nowhere.so", ( root + "/dangling.so" ).c_str() );
	symlink( "bin", ( root + "/bin-link" ).c_str() );
	mkfifo( ( root + "/pipe" ).c_str(), 0644 );

	using platform::FileKind;
	auto P = []( const std::string &s )
	{
		return platform::PosixNativePath( s.c_str() );
	};
	platformtest::FileProbeFixture fixture;
	fixture.cases = {
	    { P( root + "/engine.so" ), FileKind::kRegularFile, "regular file" },
	    { P( root + "/bin" ), FileKind::kDirectory, "directory" },
	    { P( root + "/missing.so" ), FileKind::kMissing, "missing" },
	    { P( root + "/link.so" ), FileKind::kRegularFile, "link to a file" },
	    { P( root + "/bin-link" ), FileKind::kDirectory, "link to a directory" },
	    { P( root + "/dangling.so" ), FileKind::kMissing, "dangling link" },
	    { P( root + "/pipe" ), FileKind::kOther, "fifo" },
	    { P( root + "/\xc3\xa9t\xc3\xa9.so" ), FileKind::kRegularFile, "UTF-8 name" },
	    { P( root + "/\xff\xfe.so" ), FileKind::kRegularFile, "name that is not UTF-8" },
	    { P( root + "/engine.so/x" ), FileKind::kMissing, "below a file" },
	};
	fixture.foreignFlavor = platformtest::Windows( u"C:\\Windows" );
	fixture.countEntries = [root]()
	{
		int n = 0;
		DIR *d = opendir( root.c_str() );
		for ( dirent *e = d ? readdir( d ) : nullptr; e != nullptr; e = readdir( d ) )
		{
			++n;
		}
		if ( d )
		{
			closedir( d );
		}
		return n;
	};
	auto probe = platform::CreatePosixFileProbe();
	Tally( "posix.file-probe", platformtest::RunFileProbeConformance( *probe, fixture ) );

	// The same fixture rejects broken probes.
	CLstatProbe lstatProbe;
	CAnythingIsAFile anything( *probe );
	CLossyNames lossy( *probe );
	NATIVE_CHECK( platformtest::RunFileProbeConformance( lstatProbe, fixture ).failures > 0 );
	NATIVE_CHECK( platformtest::RunFileProbeConformance( anything, fixture ).failures > 0 );
	NATIVE_CHECK( platformtest::RunFileProbeConformance( lossy, fixture ).failures > 0 );

	// Resolution through the native probe, from the cwd-style root and from a
	// root that is not UTF-8.
	auto resolver = platform::CreateModuleResolver( *probe, nullptr );
	auto server = resolver->Resolve( "server", platformtest::LegacyPolicy( { P( root ) } ) );
	NATIVE_CHECK( server && server.Value().path == P( root + "/bin/libserver.so" ) );
	auto engine = resolver->Resolve( "engine.dll", platformtest::LegacyPolicy( { P( root ) } ) );
	NATIVE_CHECK( engine && engine.Value().path == P( root + "/engine.so" ) );
	auto client =
	    resolver->Resolve( "client", platformtest::LegacyPolicy( { P( root ), P( odd ) } ) );
	NATIVE_CHECK( client && client.Value().path == P( odd + "/bin/libclient.so" ) &&
	              client.Value().root == 1 );
	auto none = resolver->Resolve( "dangling", platformtest::LegacyPolicy( { P( root ) } ) );
	NATIVE_CHECK( !none && none.Error().status == platform::ResolveStatus::kNotFound &&
	              none.Error().attempted.size() == 4 );
	NATIVE_CHECK(
	    platform::PosixCurrentDirectory().Flavor() == platform::NativePathFlavor::kPosixBytes );

	for ( const char *name : { "/engine.so", "/bin/libserver.so", "/\xc3\xa9t\xc3\xa9.so",
	          "/\xff\xfe.so", "/link.so", "/dangling.so", "/bin-link", "/pipe" } )
	{
		unlink( ( root + name ).c_str() );
	}
	unlink( ( odd + "/bin/libclient.so" ).c_str() );
	rmdir( ( odd + "/bin" ).c_str() );
	rmdir( odd.c_str() );
	rmdir( ( root + "/bin" ).c_str() );
	rmdir( root.c_str() );
}

} // namespace

int main( int argc, char **argv )
{
	// Re-execute with fixed arguments and environment for the exact fixtures.
	if ( std::getenv( kChildMarker ) == nullptr )
	{
		char self[PATH_MAX];
		const ssize_t n = readlink( "/proc/self/exe", self, sizeof( self ) - 1 );
		if ( n <= 0 )
		{
			std::printf( "FAIL: cannot find /proc/self/exe\n" );
			return testing::ReportConformance( 1, 1 );
		}
		self[n] = '\0';
		const char *args[6] = {
		    self, kChildArgs[1], kChildArgs[2], kChildArgs[3], kChildArgs[4], nullptr };
		std::string home =
		    std::string( "HOME=" ) + ( std::getenv( "HOME" ) ? std::getenv( "HOME" ) : "/root" );
		std::vector<std::string> keep = { "SOURCE_R26_CHILD=1", "SOURCE_TEST_VAR=caf\xc3\xa9 = 1",
		    "SOURCE_TEST_EMPTY=", "TZ=Asia/Kolkata", "TMPDIR=/tmp//r26-tmp/", home };
		// Sanitizer runtimes must leave fatal signals to the crash handler under
		// test; the faults this suite causes are deliberate.
		const char *handlerOptions = "handle_segv=0:handle_sigbus=0:handle_sigfpe=0:"
		                             "handle_sigill=0:handle_abort=0:allow_user_segv_handler=1";
		for ( const char *name : { "ASAN_OPTIONS", "TSAN_OPTIONS" } )
		{
			const char *v = std::getenv( name );
			keep.push_back( std::string( name ) + "=" + handlerOptions + ":halt_on_error=1" +
			                ( v != nullptr && v[0] != '\0' ? std::string( ":" ) + v : "" ) );
		}
		keep.push_back( "UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1" );
		std::vector<const char *> env;
		for ( const std::string &e : keep )
		{
			env.push_back( e.c_str() );
		}
		env.push_back( nullptr );
		execve( self, const_cast<char *const *>( args ), const_cast<char *const *>( env.data() ) );
		std::printf( "FAIL: re-exec failed\n" );
		return testing::ReportConformance( 1, 1 );
	}
	(void)argc;

	ClockSuites();
	ThreadSuites();
	VirtualMemorySuites();
	ProcessEnvironmentSuites( argc, argv );
	PathsSuites();
	DiagnosticsSuites();
	FileProbeAndResolverSuites();
	return testing::ReportConformance( g_checks, g_failures );
}
