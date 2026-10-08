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

#include <cstdint>
#if defined( _MSC_VER )
#include <intrin.h>
#endif
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

// A stack capture whose innermost frame is its true caller (the compiler's
// return address) followed by fixed synthetic frames: a conforming fake.
class CFakeStackCapture : public platform::IStackCapture
{
public:
#if defined( _MSC_VER )
	__declspec( noinline )
#else
	__attribute__( ( noinline ) )
#endif
	int CaptureStack( void **frames, int maxFrames ) override
	{
		if ( frames == nullptr || maxFrames <= 0 )
		{
			return 0;
		}
#if defined( _MSC_VER )
		void *caller = _ReturnAddress();
#else
		void *caller = __builtin_return_address( 0 );
#endif
		return Fill( frames, maxFrames, caller );
	}

protected:
	static int Fill( void **frames, int maxFrames, void *caller )
	{
		static char outer[4];
		const int depth = 4;
		int n = 0;
		for ( ; n < depth && n < maxFrames; ++n )
		{
			frames[n] = n == 0 ? caller : static_cast<void *>( &outer[n] );
		}
		return n;
	}
};

// A watchdog on virtual time: Advance( ms ) fires it when its delay has passed.
class CFakeWatchdog : public platform::IWatchdog
{
public:
	explicit CFakeWatchdog( bool supported = true ) : m_supported( supported ) {}

	bool IsSupported() const override { return m_supported; }

	bool Arm( unsigned seconds, void ( *fire )( void * ), void *context ) override
	{
		if ( !m_supported || seconds == 0 || fire == nullptr )
		{
			return false;
		}
		m_deadline = m_now + static_cast<std::uint64_t>( seconds ) * 1000;
		m_fire = fire;
		m_context = context;
		return true;
	}

	void Disarm() override { m_fire = nullptr; }

	void Advance( unsigned milliseconds )
	{
		m_now += milliseconds;
		if ( m_fire != nullptr && m_now >= m_deadline )
		{
			void ( *fire )( void * ) = m_fire;
			m_fire = nullptr;
			fire( m_context );
		}
	}

private:
	bool m_supported;
	std::uint64_t m_now = 0;
	std::uint64_t m_deadline = 0;
	void ( *m_fire )( void * ) = nullptr;
	void *m_context = nullptr;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_DIAGNOSTICS_H
