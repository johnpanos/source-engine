//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic backends for platform::IDebugOutput and
//			platform::ICrashReporter. The debug output records each Write; the
//			crash reporter keeps annotations in memory and returns sequential
//			report ids with a snapshot of the annotations. Both are CONFORMING
//			providers: the positive subjects of the shared suite.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_DIAGNOSTICS_H
#define PLATFORMTEST_FAKE_DIAGNOSTICS_H

#include "platform/contracts/diagnostics.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace platformtest
{

// What the suite reads back from a debug-output sink. A native run implements
// it over a redirected stderr or a platform log reader.
class IDebugOutputCapture
{
public:
	virtual ~IDebugOutputCapture() = default;
	virtual int Count() const = 0;
	virtual bool Entry(
	    int index, platform::DiagnosticSeverity &severity, std::string &message ) const = 0;
};

class CFakeDebugOutput : public platform::IDebugOutput, public IDebugOutputCapture
{
public:
	void Write( platform::DiagnosticSeverity severity, const char *message ) override
	{
		if ( message == nullptr )
		{
			return;
		}
		std::lock_guard<std::mutex> lock( m_mutex );
		m_entries.push_back( { severity, message } );
	}

	int Count() const override
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		return static_cast<int>( m_entries.size() );
	}

	bool Entry(
	    int index, platform::DiagnosticSeverity &severity, std::string &message ) const override
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		if ( index < 0 || index >= static_cast<int>( m_entries.size() ) )
		{
			return false;
		}
		severity = m_entries[index].first;
		message = m_entries[index].second;
		return true;
	}

private:
	mutable std::mutex m_mutex;
	std::vector<std::pair<platform::DiagnosticSeverity, std::string>> m_entries;
};

inline bool IsValidCrashKey( const char *key )
{
	if ( key == nullptr )
	{
		return false;
	}
	const std::size_t n = std::strlen( key );
	if ( n == 0 || n > static_cast<std::size_t>( platform::kCrashKeyMaxBytes ) )
	{
		return false;
	}
	for ( std::size_t i = 0; i < n; ++i )
	{
		const char c = key[i];
		const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
		                ( c >= '0' && c <= '9' ) || c == '_' || c == '.' || c == '-';
		if ( !ok )
		{
			return false;
		}
	}
	return true;
}

class CFakeCrashReporter : public platform::ICrashReporter
{
public:
	explicit CFakeCrashReporter( bool available = true ) : m_available( available ) {}

	bool IsAvailable() const override { return m_available; }

	platform::CrashReportResult SetAnnotation( const char *key, const char *value ) override
	{
		if ( !m_available )
		{
			return platform::CrashReportResult::kUnsupported;
		}
		if ( !IsValidCrashKey( key ) ||
		     ( value != nullptr && std::strlen( value ) >
		                               static_cast<std::size_t>( platform::kCrashValueMaxBytes ) ) )
		{
			return platform::CrashReportResult::kInvalidArgument;
		}
		if ( value != nullptr && m_annotations.count( key ) == 0 &&
		     static_cast<int>( m_annotations.size() ) >= platform::kCrashMaxAnnotations )
		{
			return platform::CrashReportResult::kInvalidArgument;
		}
		if ( value == nullptr )
		{
			m_annotations.erase( key );
		}
		else
		{
			m_annotations[key] = value;
		}
		return platform::CrashReportResult::kOk;
	}

	int GetAnnotation( const char *key, char *buffer, int bufferSize ) const override
	{
		if ( !m_available || !IsValidCrashKey( key ) )
		{
			return -1;
		}
		auto it = m_annotations.find( key );
		if ( it == m_annotations.end() || buffer == nullptr ||
		     bufferSize <= static_cast<int>( it->second.size() ) )
		{
			return -1;
		}
		std::memcpy( buffer, it->second.c_str(), it->second.size() + 1 );
		return static_cast<int>( it->second.size() );
	}

	platform::CrashReportResult WriteReport(
	    const char *reason, char *idBuffer, int idBufferSize ) override
	{
		if ( !m_available )
		{
			return platform::CrashReportResult::kUnsupported;
		}
		char id[32];
		const int n = std::snprintf( id, sizeof( id ), "report-%d", m_nextReport );
		if ( reason == nullptr || idBuffer == nullptr || idBufferSize <= n )
		{
			return platform::CrashReportResult::kInvalidArgument;
		}
		++m_nextReport;
		m_reports.push_back( m_annotations );
		std::memcpy( idBuffer, id, n + 1 );
		return platform::CrashReportResult::kOk;
	}

	// Test access: the annotations captured by each report, in order.
	const std::vector<std::map<std::string, std::string>> &Reports() const { return m_reports; }

private:
	bool m_available;
	int m_nextReport = 1;
	std::map<std::string, std::string> m_annotations;
	std::vector<std::map<std::string, std::string>> m_reports;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_DIAGNOSTICS_H
