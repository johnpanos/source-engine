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
