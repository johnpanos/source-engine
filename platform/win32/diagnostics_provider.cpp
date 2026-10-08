//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 diagnostics (platform.diagnostics.v1): a framed sink over a
//			file handle, OutputDebugStringW, and a crash reporter with an
//			unhandled-exception filter.
//
//=============================================================================//

#include "foundation_providers.h"

#include "win32_text.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

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

std::string Frame( DiagnosticSeverity severity, const char *message )
{
	return std::string( SeverityTag( severity ) ) + message + "\n";
}

class CHandleDebugOutput final : public IDebugOutput
{
public:
	explicit CHandleDebugOutput( HANDLE file ) : m_file( file ) {}

	void Write( DiagnosticSeverity severity, const char *message ) override
	{
		if ( message == nullptr )
		{
			return;
		}
		const std::string record = Frame( severity, message );
		std::lock_guard<std::mutex> lock( m_mutex );
		// One WriteFile per record (a message-mode pipe keeps it as one message);
		// a short write to a byte stream is finished.
		std::size_t done = 0;
		while ( done < record.size() )
		{
			DWORD written = 0;
			if ( !WriteFile( m_file, record.data() + done,
			         static_cast<DWORD>( record.size() - done ), &written, nullptr ) ||
			     written == 0 )
			{
				return;
			}
			done += written;
		}
	}

private:
	HANDLE m_file;
	std::mutex m_mutex;
};

class CDebuggerOutput final : public IDebugOutput
{
public:
	void Write( DiagnosticSeverity severity, const char *message ) override
	{
		if ( message != nullptr )
		{
			OutputDebugStringW( win32::ToWide( Frame( severity, message ).c_str() ).c_str() );
		}
	}
};

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

struct Annotation
{
	std::atomic<bool> used{ false };
	char key[kCrashKeyMaxBytes + 1] = {};
	char value[kCrashValueMaxBytes + 1] = {};
};

class CWin32CrashReporter;
std::atomic<CWin32CrashReporter *> g_handlerOwner{ nullptr };
LONG WINAPI UnhandledFilter( EXCEPTION_POINTERS *info );

class CWin32CrashReporter final : public ICrashReporter
{
public:
	explicit CWin32CrashReporter( const char *dir ) : m_dir( win32::ToWide( dir ) ) {}

	~CWin32CrashReporter() override
	{
		if ( m_installed )
		{
			SetUnhandledExceptionFilter( m_previous );
			g_handlerOwner.store( nullptr );
		}
	}

	bool InstallHandler()
	{
		CWin32CrashReporter *expected = nullptr;
		if ( !g_handlerOwner.compare_exchange_strong( expected, this ) )
		{
			return false;
		}
		m_previous = SetUnhandledExceptionFilter( UnhandledFilter );
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
				slot->used.store( false );
			}
			return CrashReportResult::kOk;
		}
		if ( slot == nullptr )
		{
			for ( Annotation &a : m_annotations )
			{
				if ( !a.used.load() )
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
		slot->used.store( false );
		std::snprintf( slot->value, sizeof( slot->value ), "%s", value );
		slot->used.store( true );
		return CrashReportResult::kOk;
	}

	int GetAnnotation( const char *key, char *buffer, int bufferSize ) const override
	{
		if ( !IsValidKey( key ) || buffer == nullptr || bufferSize <= 0 )
		{
			return -1;
		}
		std::lock_guard<std::mutex> lock( m_mutex );
		const Annotation *slot = const_cast<CWin32CrashReporter *>( this )->Find( key );
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
		const int n = std::snprintf( id, sizeof( id ), "report-%lu-%d",
		    static_cast<unsigned long>( GetCurrentProcessId() ), m_nextReport.load() );
		if ( n >= idBufferSize )
		{
			return CrashReportResult::kInvalidArgument;
		}
		std::lock_guard<std::mutex> lock( m_mutex );
		if ( !Emit( id, reason ) )
		{
			return CrashReportResult::kFailed;
		}
		m_nextReport.fetch_add( 1 );
		std::memcpy( idBuffer, id, n + 1 );
		return CrashReportResult::kOk;
	}

	void OnUnhandled( DWORD code )
	{
		char id[64];
		std::snprintf(
		    id, sizeof( id ), "crash-%lu", static_cast<unsigned long>( GetCurrentProcessId() ) );
		char reason[64];
		std::snprintf(
		    reason, sizeof( reason ), "exception 0x%08lx", static_cast<unsigned long>( code ) );
		Emit( id, reason );
	}

private:
	Annotation *Find( const char *key )
	{
		for ( Annotation &a : m_annotations )
		{
			if ( a.used.load() && std::strcmp( a.key, key ) == 0 )
			{
				return &a;
			}
		}
		return nullptr;
	}

	bool Emit( const char *id, const char *reason )
	{
		const std::wstring path = m_dir + L"\\" + win32::ToWide( id ) + L".txt";
		HANDLE file = CreateFileW(
		    path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr );
		if ( file == INVALID_HANDLE_VALUE )
		{
			return false;
		}
		static char text[8192];
		int length = std::snprintf( text, sizeof( text ), "reason: %s\npid: %lu\ntime: %llu\n",
		    reason, static_cast<unsigned long>( GetCurrentProcessId() ),
		    static_cast<unsigned long long>( GetTickCount64() ) );
		for ( const Annotation &a : m_annotations )
		{
			if ( a.used.load() && length < static_cast<int>( sizeof( text ) ) )
			{
				length += std::snprintf(
				    text + length, sizeof( text ) - length, "annotation %s=%s\n", a.key, a.value );
			}
		}
		void *frames[62];
		const USHORT count = CaptureStackBackTrace( 0, 62, frames, nullptr );
		if ( length < static_cast<int>( sizeof( text ) ) )
		{
			length += std::snprintf( text + length, sizeof( text ) - length, "backtrace:\n" );
		}
		for ( USHORT i = 0; i < count && length < static_cast<int>( sizeof( text ) ); ++i )
		{
			length += std::snprintf( text + length, sizeof( text ) - length, "  0x%llx\n",
			    static_cast<unsigned long long>( reinterpret_cast<uintptr_t>( frames[i] ) ) );
		}
		if ( length > static_cast<int>( sizeof( text ) ) )
		{
			length = sizeof( text );
		}
		DWORD written = 0;
		const BOOL ok = WriteFile( file, text, static_cast<DWORD>( length ), &written, nullptr );
		CloseHandle( file );
		return ok && written == static_cast<DWORD>( length );
	}

	std::wstring m_dir;
	mutable std::mutex m_mutex;
	Annotation m_annotations[kCrashMaxAnnotations];
	std::atomic<int> m_nextReport{ 1 };
	bool m_installed = false;
	LPTOP_LEVEL_EXCEPTION_FILTER m_previous = nullptr;
};

LONG WINAPI UnhandledFilter( EXCEPTION_POINTERS *info )
{
	if ( CWin32CrashReporter *owner = g_handlerOwner.load() )
	{
		owner->OnUnhandled( info != nullptr && info->ExceptionRecord != nullptr
		                        ? info->ExceptionRecord->ExceptionCode
		                        : 0 );
	}
	return EXCEPTION_CONTINUE_SEARCH; // the process ends with the original code
}

} // namespace

std::unique_ptr<IDebugOutput> CreateWin32HandleDebugOutput( void *file )
{
	return std::make_unique<CHandleDebugOutput>( static_cast<HANDLE>( file ) );
}

std::unique_ptr<IDebugOutput> CreateWin32DebuggerOutput()
{
	return std::make_unique<CDebuggerOutput>();
}

std::unique_ptr<ICrashReporter> CreateWin32CrashReporter(
    const char *reportDir, bool installHandler )
{
	if ( reportDir == nullptr || std::strlen( reportDir ) >= 400 )
	{
		return nullptr;
	}
	auto reporter = std::make_unique<CWin32CrashReporter>( reportDir );
	if ( installHandler && !reporter->InstallHandler() )
	{
		return nullptr;
	}
	return reporter;
}

} // namespace platform
