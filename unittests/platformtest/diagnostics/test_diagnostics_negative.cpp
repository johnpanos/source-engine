//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the RFC 0001 diagnostics conformance suites
//			(PLAT-DIAG-001, Q-FOUNDATION). Feeds the SAME shared predicates broken
//			providers, each violating one clause, and asserts every one is caught
//			while the conforming backends pass.
//
//			Build/run: tools/quality/conformance.py check --suite platform.diagnostics.sensitivity
//
//=============================================================================//

#include "diagnostics_conformance.h"
#include "fake_diagnostics.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

namespace
{

using platform::CrashReportResult;
using platform::DiagnosticSeverity;
using platformtest::CFakeCrashReporter;
using platformtest::CFakeDebugOutput;

// DEFECT: appends a newline to every message.
class CAppendsNewline : public CFakeDebugOutput
{
public:
	void Write( DiagnosticSeverity s, const char *m ) override
	{
		if ( m != nullptr )
		{
			CFakeDebugOutput::Write( s, ( std::string( m ) + "\n" ).c_str() );
		}
	}
};

// DEFECT: splits multi-line messages into one entry per line.
class CSplitsLines : public CFakeDebugOutput
{
public:
	void Write( DiagnosticSeverity s, const char *m ) override
	{
		if ( m == nullptr )
		{
			return;
		}
		std::string rest( m );
		std::size_t nl;
		while ( ( nl = rest.find( '\n' ) ) != std::string::npos )
		{
			CFakeDebugOutput::Write( s, rest.substr( 0, nl ).c_str() );
			rest.erase( 0, nl + 1 );
		}
		CFakeDebugOutput::Write( s, rest.c_str() );
	}
};

// DEFECT: truncates long messages to a fixed buffer.
class CTruncates : public CFakeDebugOutput
{
public:
	void Write( DiagnosticSeverity s, const char *m ) override
	{
		if ( m != nullptr )
		{
			CFakeDebugOutput::Write( s, std::string( m ).substr( 0, 1023 ).c_str() );
		}
	}
};

// DEFECT: every message is reported as kInfo.
class CDropsSeverity : public CFakeDebugOutput
{
public:
	void Write( DiagnosticSeverity, const char *m ) override
	{
		CFakeDebugOutput::Write( DiagnosticSeverity::kInfo, m );
	}
};

// DEFECT: a write is delivered as two halves, so another thread's message can
// land between them (not whole).
class CSplitsWrites : public CFakeDebugOutput
{
public:
	void Write( DiagnosticSeverity s, const char *m ) override
	{
		if ( m == nullptr )
		{
			return;
		}
		const std::string text( m );
		if ( text.size() < 2 )
		{
			CFakeDebugOutput::Write( s, m );
			return;
		}
		CFakeDebugOutput::Write( s, text.substr( 0, text.size() / 2 ).c_str() );
		CFakeDebugOutput::Write( s, text.substr( text.size() / 2 ).c_str() );
	}
};

// DEFECT: an unavailable reporter accepts annotations.
class CUnavailableAccepts : public CFakeCrashReporter
{
public:
	CUnavailableAccepts() : CFakeCrashReporter( false ) {}
	CrashReportResult SetAnnotation( const char *, const char * ) override
	{
		return CrashReportResult::kOk;
	}
};

// DEFECT: keys are not validated.
class CAnyKey : public CFakeCrashReporter
{
public:
	CrashReportResult SetAnnotation( const char *key, const char *value ) override
	{
		if ( key != nullptr && !platformtest::IsValidCrashKey( key ) )
		{
			return CrashReportResult::kOk;
		}
		return CFakeCrashReporter::SetAnnotation( key, value );
	}
};

// DEFECT: an oversized value is truncated and stored instead of refused.
class CTruncatesValue : public CFakeCrashReporter
{
public:
	CrashReportResult SetAnnotation( const char *key, const char *value ) override
	{
		if ( value != nullptr &&
		     std::strlen( value ) > static_cast<std::size_t>( platform::kCrashValueMaxBytes ) )
		{
			const std::string cut( value, platform::kCrashValueMaxBytes - 1 );
			return CFakeCrashReporter::SetAnnotation( key, cut.c_str() );
		}
		return CFakeCrashReporter::SetAnnotation( key, value );
	}
};

// DEFECT: removing a key with a null value is refused.
class CNullValueRefused : public CFakeCrashReporter
{
public:
	CrashReportResult SetAnnotation( const char *key, const char *value ) override
	{
		return value == nullptr ? CrashReportResult::kInvalidArgument
		                        : CFakeCrashReporter::SetAnnotation( key, value );
	}
};

// DEFECT: every report gets the same id.
class CReusedId : public CFakeCrashReporter
{
public:
	CrashReportResult WriteReport( const char *reason, char *id, int size ) override
	{
		const CrashReportResult result = CFakeCrashReporter::WriteReport( reason, id, size );
		if ( result == CrashReportResult::kOk )
		{
			std::strcpy( id, "report" );
		}
		return result;
	}
};

// DEFECT: no capacity limit.
class CUnbounded : public CFakeCrashReporter
{
public:
	CrashReportResult SetAnnotation( const char *key, const char *value ) override
	{
		if ( key != nullptr && std::strcmp( key, "one.too.many" ) == 0 && value != nullptr )
		{
			return CrashReportResult::kOk;
		}
		return CFakeCrashReporter::SetAnnotation( key, value );
	}
};

// --- stack capture defects ---------------------------------------------

// DEFECT: includes its own frame first.
class COwnFrame : public platformtest::CFakeStackCapture
{
public:
	int CaptureStack( void **frames, int maxFrames ) override
	{
		if ( frames == nullptr || maxFrames <= 0 )
		{
			return 0;
		}
		return Fill( frames, maxFrames, reinterpret_cast<void *>( &COwnFrame::Marker ) );
	}
	static void Marker() {}
};

// DEFECT: outermost first.
class CReversed : public platformtest::CFakeStackCapture
{
public:
	int CaptureStack( void **frames, int maxFrames ) override
	{
		if ( frames == nullptr || maxFrames <= 0 )
		{
			return 0;
		}
		void *full[8] = {};
		const int n = CFakeStackCapture::CaptureStack( full, 8 );
		int out = 0;
		for ( int i = n - 1; i >= 0 && out < maxFrames; --i )
		{
			frames[out++] = full[i];
		}
		return out;
	}
};

// DEFECT: reports more frames than it may write.
class COverrun : public platformtest::CFakeStackCapture
{
public:
	int CaptureStack( void **frames, int maxFrames ) override
	{
		const int n = CFakeStackCapture::CaptureStack( frames, maxFrames );
		return n > 0 ? n + 2 : n;
	}
};

// DEFECT: a null buffer is reported as frames captured.
class CNullFrames : public platformtest::CFakeStackCapture
{
public:
	int CaptureStack( void **frames, int maxFrames ) override
	{
		return frames == nullptr ? 3 : CFakeStackCapture::CaptureStack( frames, maxFrames );
	}
};

// DEFECT: a different stack each time from the same site.
class CUnstable : public platformtest::CFakeStackCapture
{
public:
	int CaptureStack( void **frames, int maxFrames ) override
	{
		const int n = CFakeStackCapture::CaptureStack( frames, maxFrames );
		static char salt[64];
		if ( n > 1 )
		{
			frames[1] = &salt[( m_calls++ ) % 64];
		}
		return n;
	}

private:
	int m_calls = 0;
};

// --- watchdog defects --------------------------------------------------

// DEFECT: fires again on the tick after it fired.
class CFiresTwice : public platformtest::CFakeWatchdog
{
public:
	bool Arm( unsigned seconds, void ( *fire )( void * ), void *context ) override
	{
		m_fire = fire;
		m_context = context;
		return CFakeWatchdog::Arm( seconds, &CFiresTwice::Wrapped, this );
	}
	void Tick( unsigned ms )
	{
		if ( m_again )
		{
			m_again = false;
			m_fire( m_context ); // the defect: a second fire
		}
		Advance( ms );
	}

private:
	static void Wrapped( void *self )
	{
		CFiresTwice *me = static_cast<CFiresTwice *>( self );
		me->m_fire( me->m_context );
		me->m_again = true;
	}
	void ( *m_fire )( void * ) = nullptr;
	void *m_context = nullptr;
	bool m_again = false;
};

// DEFECT: Disarm does nothing.
class CIgnoresDisarm : public platformtest::CFakeWatchdog
{
public:
	void Disarm() override {}
};

// DEFECT: re-arming keeps the first callback.
class CKeepsFirstCallback : public platformtest::CFakeWatchdog
{
public:
	bool Arm( unsigned seconds, void ( *fire )( void * ), void *context ) override
	{
		if ( m_first == nullptr && fire != nullptr && seconds != 0 )
		{
			m_first = fire;
			m_firstContext = context;
		}
		return CFakeWatchdog::Arm( seconds, m_first != nullptr ? m_first : fire,
		    m_first != nullptr ? m_firstContext : context );
	}
	void ( *m_first )( void * ) = nullptr;
	void *m_firstContext = nullptr;
};

// DEFECT: time runs twice as fast, so it fires early.
class CFiresEarly : public platformtest::CFakeWatchdog
{
public:
	void Tick( unsigned ms ) { Advance( ms * 2 ); }
};

// DEFECT: unsupported, yet Arm reports success and fires.
class CClaimsWhenUnsupported : public platformtest::CFakeWatchdog
{
public:
	CClaimsWhenUnsupported() : CFakeWatchdog( false ) {}
	bool Arm( unsigned, void ( *fire )( void * ), void *context ) override
	{
		if ( fire != nullptr )
		{
			fire( context );
		}
		return true;
	}
};

template <typename T> bool StackCaught()
{
	T capture;
	return platformtest::RunStackCaptureConformance( capture ).failures > 0;
}

template <typename T> bool WatchdogCaught()
{
	T watchdog;
	return platformtest::RunWatchdogConformance( watchdog, [&]( unsigned ms ) { watchdog.Advance( ms ); } )
	           .failures > 0;
}

template <typename T> bool WatchdogTickCaught()
{
	T watchdog;
	return platformtest::RunWatchdogConformance( watchdog, [&]( unsigned ms ) { watchdog.Tick( ms ); } )
	           .failures > 0;
}

template <typename T> bool OutputCaught()
{
	T output;
	return platformtest::RunDebugOutputConformance( output, output ).failures > 0;
}

template <typename T> bool ReporterCaught()
{
	T reporter;
	return platformtest::RunCrashReporterConformance( reporter ).failures > 0;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	{
		CFakeDebugOutput output;
		CFakeCrashReporter reporter;
		CFakeCrashReporter unavailable( false );
		const bool ok = platformtest::RunDebugOutputConformance( output, output ).failures == 0 &&
		                platformtest::RunCrashReporterConformance( reporter ).failures == 0 &&
		                platformtest::RunCrashReporterConformance( unavailable ).failures == 0;
		++checks;
		if ( !ok )
		{
			std::printf( "FAIL: conforming providers rejected\n" );
			++failures;
		}
	}
	struct Case
	{
		bool ( *caught )();
		const char *name;
	};
	const Case cases[] = {
	    { OutputCaught<CAppendsNewline>, "appends-newline" },
	    { OutputCaught<CSplitsLines>, "splits-lines" },
	    { OutputCaught<CTruncates>, "truncates-long-message" },
	    { OutputCaught<CDropsSeverity>, "drops-severity" },
	    { OutputCaught<CSplitsWrites>, "write-not-whole" },
	    { ReporterCaught<CUnavailableAccepts>, "unavailable-accepts" },
	    { ReporterCaught<CAnyKey>, "accepts-malformed-key" },
	    { ReporterCaught<CTruncatesValue>, "truncates-oversized-value" },
	    { ReporterCaught<CNullValueRefused>, "null-value-refused" },
	    { ReporterCaught<CReusedId>, "reused-report-id" },
	    { ReporterCaught<CUnbounded>, "no-annotation-capacity" },
	    { StackCaught<COwnFrame>, "stack-includes-own-frame" },
	    { StackCaught<CReversed>, "stack-outermost-first" },
	    { StackCaught<COverrun>, "stack-ignores-max" },
	    { StackCaught<CNullFrames>, "stack-null-buffer" },
	    { StackCaught<CUnstable>, "stack-unstable" },
	    { WatchdogTickCaught<CFiresTwice>, "watchdog-fires-twice" },
	    { WatchdogCaught<CIgnoresDisarm>, "watchdog-ignores-disarm" },
	    { WatchdogCaught<CKeepsFirstCallback>, "watchdog-rearm-keeps-old" },
	    { WatchdogTickCaught<CFiresEarly>, "watchdog-fires-early" },
	    { WatchdogCaught<CClaimsWhenUnsupported>, "watchdog-claims-when-unsupported" },
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
		std::printf( "ok test_diagnostics_negative: all %zu broken providers caught\n",
		    sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
