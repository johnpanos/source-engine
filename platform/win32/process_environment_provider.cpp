//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 process environment (platform.process-environment.v1).
//
//=============================================================================//

#include "foundation_providers.h"

#include "win32_text.h"

#include <cstring>
#include <map>
#include <shellapi.h>
#include <string>
#include <vector>

namespace platform
{
namespace
{

int CopyOut( const std::string *s, char *buffer, int bufferSize )
{
	if ( s == nullptr || buffer == nullptr || bufferSize <= static_cast<int>( s->size() ) )
	{
		return -1;
	}
	std::memcpy( buffer, s->c_str(), s->size() + 1 );
	return static_cast<int>( s->size() );
}

class CWin32ProcessEnvironment final : public IWin32ProcessEnvironment
{
public:
	CWin32ProcessEnvironment()
	{
		int argc = 0;
		LPWSTR *argv = CommandLineToArgvW( GetCommandLineW(), &argc );
		for ( int i = 0; argv != nullptr && i < argc; ++i )
		{
			std::string arg;
			win32::ToUtf8( argv[i], arg ); // an unconvertible argument becomes ""
			m_arguments.push_back( std::move( arg ) );
		}
		if ( argv != nullptr )
		{
			LocalFree( argv );
		}

		LPWCH block = GetEnvironmentStringsW();
		for ( const wchar_t *e = block; e != nullptr && *e != L'\0'; e += wcslen( e ) + 1 )
		{
			const wchar_t *eq = wcschr( e, L'=' );
			if ( eq == nullptr || eq == e )
			{
				continue; // "=C:=C:\dir" drive pseudo-variables
			}
			std::string name, value;
			if ( win32::ToUtf8( e, static_cast<int>( eq - e ), name ) &&
			     win32::ToUtf8( eq + 1, value ) )
			{
				m_variables.emplace( std::move( name ), std::move( value ) );
			}
		}
		if ( block != nullptr )
		{
			FreeEnvironmentStringsW( block );
		}
		m_pid = GetCurrentProcessId();
	}

	int ArgumentCount() const override { return static_cast<int>( m_arguments.size() ); }

	int ArgumentLength( int index ) const override
	{
		const std::string *s = Argument( index );
		return s != nullptr ? static_cast<int>( s->size() ) : -1;
	}

	int GetArgument( int index, char *buffer, int bufferSize ) const override
	{
		return CopyOut( Argument( index ), buffer, bufferSize );
	}

	int VariableLength( const char *name ) const override
	{
		const std::string *s = Variable( name );
		return s != nullptr ? static_cast<int>( s->size() ) : -1;
	}

	int GetVariable( const char *name, char *buffer, int bufferSize ) const override
	{
		return CopyOut( Variable( name ), buffer, bufferSize );
	}

	std::uint64_t ProcessId() const override { return m_pid; }

	DebuggerState GetDebuggerState() const override
	{
		return IsDebuggerPresent() ? DebuggerState::kAttached : DebuggerState::kNotAttached;
	}

	const wchar_t *RawCommandLineW() const override { return GetCommandLineW(); }
	const char *RawCommandLineA() const override { return GetCommandLineA(); }

private:
	const std::string *Argument( int index ) const
	{
		return index >= 0 && index < ArgumentCount() ? &m_arguments[index] : nullptr;
	}

	const std::string *Variable( const char *name ) const
	{
		if ( name == nullptr || name[0] == '\0' || std::strchr( name, '=' ) != nullptr )
		{
			return nullptr;
		}
		auto it = m_variables.find( name );
		return it != m_variables.end() ? &it->second : nullptr;
	}

	std::vector<std::string> m_arguments;
	std::map<std::string, std::string> m_variables;
	std::uint64_t m_pid = 0;
};

} // namespace

std::unique_ptr<IWin32ProcessEnvironment> CreateWin32ProcessEnvironment()
{
	return std::make_unique<CWin32ProcessEnvironment>();
}

} // namespace platform
