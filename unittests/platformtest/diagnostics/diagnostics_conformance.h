//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suites for the RFC 0001 diagnostics capability
//			(platform::IDebugOutput, platform::ICrashReporter). Every provider
//			that claims a contract runs THIS predicate. The debug-output suite
//			reads messages back through an IDebugOutputCapture the test supplies;
//			the crash-reporter suite runs the available or the refusal branch
//			according to IsAvailable().
//
//=============================================================================//

#ifndef PLATFORMTEST_DIAGNOSTICS_CONFORMANCE_H
#define PLATFORMTEST_DIAGNOSTICS_CONFORMANCE_H

#include "fake_diagnostics.h"
#include "platform/contracts/diagnostics.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>

namespace platformtest
{

struct DiagnosticsReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define DG_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

inline DiagnosticsReport RunDebugOutputConformance(
    platform::IDebugOutput &output, const IDebugOutputCapture &capture )
{
	using platform::DiagnosticSeverity;
	DiagnosticsReport r;
	const int start = capture.Count();

	// Messages arrive byte for byte with their severity, in order; null is ignored.
	const std::string longMessage( 8192, 'x' );
	struct Sent
	{
		DiagnosticSeverity severity;
		const char *message;
	};
	const Sent sent[] = {
	    { DiagnosticSeverity::kInfo, "hello" },
	    { DiagnosticSeverity::kWarning, "two\nlines" },
	    { DiagnosticSeverity::kError, "caf\xc3\xa9" },
	    { DiagnosticSeverity::kInfo, "" },
	    { DiagnosticSeverity::kInfo, longMessage.c_str() },
	};
	for ( const Sent &s : sent )
	{
		output.Write( s.severity, s.message );
		output.Write( DiagnosticSeverity::kError, nullptr );
	}
	const int count = static_cast<int>( sizeof( sent ) / sizeof( sent[0] ) );
	DG_CHECK( r, capture.Count() - start == count );
	for ( int i = 0; i < count; ++i )
	{
		DiagnosticSeverity severity = DiagnosticSeverity::kInfo;
		std::string message;
		const bool ok = capture.Entry( start + i, severity, message );
		DG_CHECK( r, ok && severity == sent[i].severity );
		DG_CHECK( r, ok && message == sent[i].message );
	}

	// Two threads writing at once: every message whole, each thread's in order.
	const int kPerThread = 200;
	const int before = capture.Count();
	auto writer = [&output]( char tag )
	{
		for ( int i = 0; i < kPerThread; ++i )
		{
			char message[64];
			std::snprintf( message, sizeof( message ), "%c:%04d:%c%c%c%c%c%c%c%c", tag, i, tag, tag,
			    tag, tag, tag, tag, tag, tag );
			output.Write( platform::DiagnosticSeverity::kInfo, message );
		}
	};
	std::thread a( writer, 'a' );
	std::thread b( writer, 'b' );
	a.join();
	b.join();
	DG_CHECK( r, capture.Count() - before == 2 * kPerThread );
	int nextA = 0;
	int nextB = 0;
	bool whole = true;
	bool ordered = true;
	for ( int i = before; i < capture.Count(); ++i )
	{
		DiagnosticSeverity severity;
		std::string message;
		if ( !capture.Entry( i, severity, message ) || message.size() != 15 )
		{
			whole = false;
			continue;
		}
		const char tag = message[0];
		const int n = std::atoi( message.c_str() + 2 );
		whole = whole && message.find_first_not_of( tag, 7 ) == std::string::npos;
		int &next = tag == 'a' ? nextA : nextB;
		ordered = ordered && n == next;
		++next;
	}
	DG_CHECK( r, whole );
	DG_CHECK( r, ordered && nextA == kPerThread && nextB == kPerThread );
	return r;
}

inline DiagnosticsReport RunCrashReporterConformance( platform::ICrashReporter &reporter )
{
	using platform::CrashReportResult;
	DiagnosticsReport r;
	char buffer[2048];

	if ( !reporter.IsAvailable() )
	{
		DG_CHECK( r, reporter.SetAnnotation( "map", "x" ) == CrashReportResult::kUnsupported );
		DG_CHECK( r, reporter.GetAnnotation( "map", buffer, sizeof( buffer ) ) == -1 );
		std::strcpy( buffer, "untouched" );
		DG_CHECK( r, reporter.WriteReport( "probe", buffer, sizeof( buffer ) ) ==
		                 CrashReportResult::kUnsupported );
		DG_CHECK( r, std::strcmp( buffer, "untouched" ) == 0 );
		return r;
	}

	// Set, read back, replace, remove.
	DG_CHECK( r, reporter.SetAnnotation( "map.name", "sp_a1_intro4" ) == CrashReportResult::kOk );
	DG_CHECK( r, reporter.GetAnnotation( "map.name", buffer, sizeof( buffer ) ) == 12 &&
	                 std::strcmp( buffer, "sp_a1_intro4" ) == 0 );
	DG_CHECK( r, reporter.SetAnnotation( "map.name", "testchmb" ) == CrashReportResult::kOk );
	DG_CHECK( r, reporter.GetAnnotation( "map.name", buffer, sizeof( buffer ) ) == 8 &&
	                 std::strcmp( buffer, "testchmb" ) == 0 );
	{
		char tight[8];
		std::strcpy( tight, "keep" );
		DG_CHECK( r, reporter.GetAnnotation( "map.name", tight, 8 ) == -1 );
		DG_CHECK( r, std::strcmp( tight, "keep" ) == 0 );
	}
	DG_CHECK( r, reporter.SetAnnotation( "map.name", nullptr ) == CrashReportResult::kOk );
	DG_CHECK( r, reporter.GetAnnotation( "map.name", buffer, sizeof( buffer ) ) == -1 );
	DG_CHECK( r, reporter.SetAnnotation( "never-set", nullptr ) == CrashReportResult::kOk );
	DG_CHECK( r, reporter.SetAnnotation( "empty", "" ) == CrashReportResult::kOk );
	DG_CHECK( r, reporter.GetAnnotation( "empty", buffer, sizeof( buffer ) ) == 0 );

	// Keys and values at and past their limits.
	const std::string maxKey( platform::kCrashKeyMaxBytes, 'k' );
	const std::string longKey( platform::kCrashKeyMaxBytes + 1, 'k' );
	const std::string maxValue( platform::kCrashValueMaxBytes, 'v' );
	const std::string longValue( platform::kCrashValueMaxBytes + 1, 'v' );
	DG_CHECK( r, reporter.SetAnnotation( maxKey.c_str(), "1" ) == CrashReportResult::kOk );
	DG_CHECK( r, reporter.SetAnnotation( "big", maxValue.c_str() ) == CrashReportResult::kOk );
	DG_CHECK( r, reporter.GetAnnotation( "big", buffer, sizeof( buffer ) ) ==
	                 platform::kCrashValueMaxBytes );
	const char *badKeys[] = {
	    nullptr, "", "has space", "slash/key", "caf\xc3\xa9", longKey.c_str() };
	for ( const char *key : badKeys )
	{
		DG_CHECK( r, reporter.SetAnnotation( key, "x" ) == CrashReportResult::kInvalidArgument );
	}
	DG_CHECK( r,
	    reporter.SetAnnotation( "big", longValue.c_str() ) == CrashReportResult::kInvalidArgument );
	DG_CHECK( r, reporter.GetAnnotation( "big", buffer, sizeof( buffer ) ) ==
	                 platform::kCrashValueMaxBytes ); // refused set changed nothing

	// Capacity: fill to kCrashMaxAnnotations, refuse one more, still replace.
	{
		const int held = 3; // maxKey, big and empty are set above
		char key[32];
		for ( int i = 0; i < platform::kCrashMaxAnnotations - held; ++i )
		{
			std::snprintf( key, sizeof( key ), "slot.%d", i );
			r.Record( reporter.SetAnnotation( key, "1" ) == CrashReportResult::kOk,
			    "fill to capacity", __LINE__ );
		}
		DG_CHECK( r,
		    reporter.SetAnnotation( "one.too.many", "1" ) == CrashReportResult::kInvalidArgument );
		DG_CHECK( r, reporter.GetAnnotation( "one.too.many", buffer, sizeof( buffer ) ) == -1 );
		DG_CHECK( r, reporter.SetAnnotation( "slot.0", "2" ) == CrashReportResult::kOk );
		DG_CHECK( r, reporter.SetAnnotation( "slot.0", nullptr ) == CrashReportResult::kOk );
		DG_CHECK( r, reporter.SetAnnotation( "one.too.many", "1" ) == CrashReportResult::kOk );
		DG_CHECK( r, reporter.SetAnnotation( "one.too.many", nullptr ) == CrashReportResult::kOk );
		for ( int i = 1; i < platform::kCrashMaxAnnotations - held; ++i )
		{
			std::snprintf( key, sizeof( key ), "slot.%d", i );
			reporter.SetAnnotation( key, nullptr );
		}
	}

	// Reports: non-empty unique ids; refusals leave the id buffer alone.
	char first[128] = {};
	char second[128] = {};
	DG_CHECK(
	    r, reporter.WriteReport( "probe one", first, sizeof( first ) ) == CrashReportResult::kOk );
	DG_CHECK( r,
	    reporter.WriteReport( "probe two", second, sizeof( second ) ) == CrashReportResult::kOk );
	DG_CHECK( r, first[0] != '\0' && second[0] != '\0' && std::strcmp( first, second ) != 0 );
	std::strcpy( buffer, "untouched" );
	DG_CHECK( r, reporter.WriteReport( nullptr, buffer, sizeof( buffer ) ) ==
	                 CrashReportResult::kInvalidArgument );
	DG_CHECK( r, std::strcmp( buffer, "untouched" ) == 0 );
	DG_CHECK(
	    r, reporter.WriteReport( "probe", nullptr, 64 ) == CrashReportResult::kInvalidArgument );
	char tiny[1] = { 'z' };
	DG_CHECK( r, reporter.WriteReport( "probe", tiny, 1 ) == CrashReportResult::kInvalidArgument );
	DG_CHECK( r, tiny[0] == 'z' );

	reporter.SetAnnotation( maxKey.c_str(), nullptr );
	reporter.SetAnnotation( "big", nullptr );
	reporter.SetAnnotation( "empty", nullptr );
	return r;
}

// ---------------------------------------------------------------------------
// Stack capture (R103)

#if defined( _MSC_VER )
#define DG_NOINLINE __declspec( noinline )
#else
#define DG_NOINLINE __attribute__( ( noinline ) )
#endif

// Captures from a known function, so the innermost frame can be checked.
DG_NOINLINE inline int CaptureAt( platform::IStackCapture &capture, void **frames, int maxFrames )
{
	const int n = capture.CaptureStack( frames, maxFrames );
	// Keep the call from becoming a tail call, which would drop this frame.
	volatile int keep = n;
	return keep;
}

DG_NOINLINE inline int CaptureTwiceFromOneSite( platform::IStackCapture &capture, void **a, void **b,
	int maxFrames, int limited, void **c )
{
	int n[3] = {};
	for ( int i = 0; i < 3; ++i )
	{
		void **out = i == 0 ? a : i == 1 ? b : c;
		n[i] = CaptureAt( capture, out, i == 2 ? limited : maxFrames );
	}
	volatile int keep = n[0] * 10000 + n[1] * 100 + n[2];
	return keep;
}

inline DiagnosticsReport RunStackCaptureConformance( platform::IStackCapture &capture )
{
	DiagnosticsReport r;
	void *dummy[4] = {};
	DG_CHECK( r, capture.CaptureStack( nullptr, 8 ) == 0 );
	DG_CHECK( r, capture.CaptureStack( dummy, 0 ) == 0 );
	DG_CHECK( r, capture.CaptureStack( dummy, -1 ) == 0 );

	void *a[32] = {};
	void *b[32] = {};
	void *c[32] = {};
	const int packed = CaptureTwiceFromOneSite( capture, a, b, 32, 2, c );
	const int na = packed / 10000, nb = packed / 100 % 100, nc = packed % 100;
	DG_CHECK( r, na >= 2 && na <= 32 );
	DG_CHECK( r, nb == na );
	DG_CHECK( r, nc == 2 ); // never more than asked for
	bool nonNull = na > 0;
	bool same = na == nb;
	for ( int i = 0; i < na && i < 32; ++i )
	{
		nonNull = nonNull && a[i] != nullptr;
		same = same && a[i] == b[i];
	}
	DG_CHECK( r, nonNull );
	DG_CHECK( r, same ); // the same site yields the same stack
	DG_CHECK( r, nc == 2 && c[0] == a[0] && c[1] == a[1] ); // a limited capture is a prefix
	// Innermost first, starting at the caller: frame 0 returns into CaptureAt.
	const std::uintptr_t at = reinterpret_cast<std::uintptr_t>( &CaptureAt );
	const std::uintptr_t first = reinterpret_cast<std::uintptr_t>( a[0] );
	DG_CHECK( r, na > 0 && first > at && first - at < 4096 );
	return r;
}

// ---------------------------------------------------------------------------
// Watchdog (R103). `wait( milliseconds )` lets time pass: real sleeps for a
// native provider, virtual time for the fake.

struct WatchdogProbe
{
	std::atomic<int> fired{ 0 };
};

inline void WatchdogFire( void *context )
{
	static_cast<WatchdogProbe *>( context )->fired.fetch_add( 1 );
}

inline DiagnosticsReport RunWatchdogConformance( platform::IWatchdog &watchdog,
	const std::function<void( unsigned )> &wait )
{
	DiagnosticsReport r;
	WatchdogProbe probe;
	if ( !watchdog.IsSupported() )
	{
		DG_CHECK( r, !watchdog.Arm( 1, WatchdogFire, &probe ) );
		watchdog.Disarm();
		wait( 1300 );
		DG_CHECK( r, probe.fired.load() == 0 );
		return r;
	}
	DG_CHECK( r, !watchdog.Arm( 0, WatchdogFire, &probe ) );
	DG_CHECK( r, !watchdog.Arm( 1, nullptr, &probe ) );
	watchdog.Disarm(); // harmless when not armed

	// Fires once, not early.
	DG_CHECK( r, watchdog.Arm( 1, WatchdogFire, &probe ) );
	wait( 600 );
	DG_CHECK( r, probe.fired.load() == 0 );
	wait( 900 );
	DG_CHECK( r, probe.fired.load() == 1 );
	wait( 1200 );
	DG_CHECK( r, probe.fired.load() == 1 );

	// Disarm cancels.
	WatchdogProbe cancelled;
	DG_CHECK( r, watchdog.Arm( 1, WatchdogFire, &cancelled ) );
	wait( 300 );
	watchdog.Disarm();
	wait( 1200 );
	DG_CHECK( r, cancelled.fired.load() == 0 );

	// Re-arming replaces the delay and the callback.
	WatchdogProbe first, second;
	DG_CHECK( r, watchdog.Arm( 1, WatchdogFire, &first ) );
	DG_CHECK( r, watchdog.Arm( 2, WatchdogFire, &second ) );
	wait( 1500 );
	DG_CHECK( r, first.fired.load() == 0 && second.fired.load() == 0 );
	wait( 1000 );
	DG_CHECK( r, first.fired.load() == 0 && second.fired.load() == 1 );
	watchdog.Disarm();
	return r;
}

} // namespace platformtest

#endif // PLATFORMTEST_DIAGNOSTICS_CONFORMANCE_H
