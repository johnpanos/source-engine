//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX process entry (see process_exec.h).
//
//=============================================================================//

#include "process_exec.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

namespace platform
{

void ExecReplacingProcess( const std::vector<std::string> &argv,
    const std::vector<ToolProcessEnvironmentOverride> &environment,
    const std::string &workingDirectory, std::string &error )
{
	if ( argv.empty() )
	{
		error = "no program";
		return;
	}
	for ( const ToolProcessEnvironmentOverride &entry : environment )
	{
		if ( !entry.value )
		{
			unsetenv( entry.name.c_str() );
			continue;
		}
		std::string value = *entry.value;
		const char *inherited = std::getenv( entry.name.c_str() );
		const std::string marker = "{inherit}";
		for ( size_t at = value.find( marker ); at != std::string::npos;
		    at = value.find( marker, at ) )
		{
			const std::string replacement = inherited ? inherited : "";
			value.replace( at, marker.size(), replacement );
			at += replacement.size();
		}
		setenv( entry.name.c_str(), value.c_str(), 1 );
	}
	if ( chdir( workingDirectory.c_str() ) != 0 )
	{
		error = "cannot enter " + workingDirectory + ": " + std::strerror( errno );
		return;
	}
	std::vector<char *> args;
	for ( const std::string &argument : argv )
		args.push_back( const_cast<char *>( argument.c_str() ) );
	args.push_back( nullptr );
	execv( args[0], args.data() );
	error =
	    "cannot start " + argv.front() + " in " + workingDirectory + ": " + std::strerror( errno );
}

} // namespace platform
