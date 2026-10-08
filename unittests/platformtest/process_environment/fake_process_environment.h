//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic backend for platform::IProcessEnvironment, built from
//			fixed argument and variable lists. This is a CONFORMING provider:
//			the positive subject of the shared suite.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_PROCESS_ENVIRONMENT_H
#define PLATFORMTEST_FAKE_PROCESS_ENVIRONMENT_H

#include "platform/contracts/process_environment.h"

#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace platformtest
{

class CFakeProcessEnvironment : public platform::IProcessEnvironment
{
public:
	CFakeProcessEnvironment( std::vector<std::string> arguments,
	    std::map<std::string, std::string> variables, std::uint64_t pid = 4242,
	    platform::DebuggerState debugger = platform::DebuggerState::kNotAttached )
	    : m_arguments( std::move( arguments ) ), m_variables( std::move( variables ) ),
	      m_pid( pid ), m_debugger( debugger )
	{
	}

	int ArgumentCount() const override { return static_cast<int>( m_arguments.size() ); }

	int ArgumentLength( int index ) const override
	{
		const std::string *s = Argument( index );
		return s != nullptr ? static_cast<int>( s->size() ) : -1;
	}

	int GetArgument( int index, char *buffer, int bufferSize ) const override
	{
		return Copy( Argument( index ), buffer, bufferSize );
	}

	int VariableLength( const char *name ) const override
	{
		const std::string *s = Variable( name );
		return s != nullptr ? static_cast<int>( s->size() ) : -1;
	}

	int GetVariable( const char *name, char *buffer, int bufferSize ) const override
	{
		return Copy( Variable( name ), buffer, bufferSize );
	}

	std::uint64_t ProcessId() const override { return m_pid; }
	platform::DebuggerState GetDebuggerState() const override { return m_debugger; }

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
		if ( name == nullptr || name[0] == '\0' || std::strchr( name, '=' ) != nullptr )
		{
			return nullptr;
		}
		auto it = m_variables.find( name );
		return it != m_variables.end() ? &it->second : nullptr;
	}

	static int Copy( const std::string *s, char *buffer, int bufferSize )
	{
		if ( s == nullptr || buffer == nullptr || bufferSize <= static_cast<int>( s->size() ) )
		{
			return -1;
		}
		std::memcpy( buffer, s->c_str(), s->size() + 1 );
		return static_cast<int>( s->size() );
	}

	std::vector<std::string> m_arguments;
	std::map<std::string, std::string> m_variables;
	std::uint64_t m_pid;
	platform::DebuggerState m_debugger;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_PROCESS_ENVIRONMENT_H
