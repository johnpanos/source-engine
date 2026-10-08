//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX process environment (platform.process-environment.v1): a
//			snapshot of argv and environ, getpid and the debugger check.
//
//=============================================================================//

#include "foundation_providers.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <unistd.h>

#if defined( __APPLE__ )
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

extern char **environ;

namespace platform
{
namespace
{

bool ValidName( const char *name )
{
	return name != nullptr && name[0] != '\0' && std::strchr( name, '=' ) == nullptr;
}

int CopyOut( const std::string *s, char *buffer, int bufferSize )
{
	if ( s == nullptr || buffer == nullptr || bufferSize <= static_cast<int>( s->size() ) )
	{
		return -1;
	}
	std::memcpy( buffer, s->c_str(), s->size() + 1 );
	return static_cast<int>( s->size() );
}

class CPosixProcessEnvironment final : public IProcessEnvironment
{
public:
	CPosixProcessEnvironment( int argc, const char *const *argv )
	{
		for ( int i = 0; argv != nullptr && i < argc; ++i )
		{
			m_arguments.emplace_back( argv[i] != nullptr ? argv[i] : "" );
		}
		for ( char **e = environ; e != nullptr && *e != nullptr; ++e )
		{
			const char *eq = std::strchr( *e, '=' );
			if ( eq == nullptr || eq == *e )
			{
				continue;
			}
			// First definition wins, as getenv does.
			m_variables.emplace(
			    std::string( *e, static_cast<std::size_t>( eq - *e ) ), std::string( eq + 1 ) );
		}
		m_pid = static_cast<std::uint64_t>( getpid() );
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
#if defined( __linux__ )
		FILE *f = std::fopen( "/proc/self/status", "r" );
		if ( f == nullptr )
		{
			return DebuggerState::kUnknown;
		}
		char line[256];
		DebuggerState state = DebuggerState::kUnknown;
		while ( std::fgets( line, sizeof( line ), f ) != nullptr )
		{
			if ( std::strncmp( line, "TracerPid:", 10 ) == 0 )
			{
				state = std::atol( line + 10 ) != 0 ? DebuggerState::kAttached
				                                    : DebuggerState::kNotAttached;
				break;
			}
		}
		std::fclose( f );
		return state;
#elif defined( __APPLE__ )
		int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
		kinfo_proc info{};
		size_t size = sizeof( info );
		if ( sysctl( mib, 4, &info, &size, nullptr, 0 ) != 0 )
		{
			return DebuggerState::kUnknown;
		}
		return ( info.kp_proc.p_flag & P_TRACED ) != 0 ? DebuggerState::kAttached
		                                               : DebuggerState::kNotAttached;
#else
		return DebuggerState::kUnknown;
#endif
	}

private:
	const std::string *Argument( int index ) const
	{
		if ( index < 0 || index >= ArgumentCount() )
		{
			return nullptr;
		}
		return &m_arguments[index];
	}

	const std::string *Variable( const char *name ) const
	{
		if ( !ValidName( name ) )
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

std::unique_ptr<IProcessEnvironment> CreatePosixProcessEnvironment(
    int argc, const char *const *argv )
{
	return std::make_unique<CPosixProcessEnvironment>( argc, argv );
}

} // namespace platform
