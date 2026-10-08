//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX platform.process-spawn.v1 provider: fork, a new process
//			group, the environment applied in the child, exec.
//
//=============================================================================//

#include "process_spawner.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <thread>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace platform
{

namespace
{

int StatusOf( int raw )
{
	if ( WIFEXITED( raw ) )
		return WEXITSTATUS( raw );
	if ( WIFSIGNALED( raw ) )
		return 128 + WTERMSIG( raw );
	return 1;
}

class PosixProcessSpawner final : public IProcessSpawner
{
public:
	SpawnedProcess Spawn( const SpawnRequest &request, std::string &error ) override
	{
		if ( request.argv.empty() )
		{
			error = "no program";
			return {};
		}
		// Everything the child needs is prepared before fork.
		std::vector<std::pair<std::string, std::optional<std::string>>> environment;
		for ( const ToolProcessEnvironmentOverride &entry : request.environment )
		{
			if ( !entry.value )
			{
				environment.emplace_back( entry.name, std::nullopt );
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
			environment.emplace_back( entry.name, value );
		}
		std::vector<char *> argv;
		for ( const std::string &argument : request.argv )
			argv.push_back( const_cast<char *>( argument.c_str() ) );
		argv.push_back( nullptr );
		int output = -1;
		if ( !request.outputFile.empty() )
		{
			output =
			    open( request.outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644 );
			if ( output < 0 )
			{
				error = "cannot open " + request.outputFile + ": " + std::strerror( errno );
				return {};
			}
		}
		int pipeFds[2];
		if ( pipe2( pipeFds, O_CLOEXEC ) != 0 )
		{
			error = std::strerror( errno );
			if ( output >= 0 )
				close( output );
			return {};
		}
		const pid_t pid = fork();
		if ( pid < 0 )
		{
			error = std::strerror( errno );
			close( pipeFds[0] );
			close( pipeFds[1] );
			if ( output >= 0 )
				close( output );
			return {};
		}
		if ( pid == 0 )
		{
			setpgid( 0, 0 );
			if ( output >= 0 )
			{
				// dup2 clears close-on-exec on the copies.
				dup2( output, STDOUT_FILENO );
				dup2( output, STDERR_FILENO );
			}
			for ( const auto &[name, value] : environment )
			{
				if ( value )
					setenv( name.c_str(), value->c_str(), 1 );
				else
					unsetenv( name.c_str() );
			}
			int failure = 0;
			if ( chdir( request.workingDirectory.c_str() ) != 0 )
				failure = errno;
			else
			{
				execvp( argv[0], argv.data() );
				failure = errno;
			}
			// Report the failure through the close-on-exec pipe.
			[[maybe_unused]] ssize_t written = write( pipeFds[1], &failure, sizeof( failure ) );
			_exit( 127 );
		}
		close( pipeFds[1] );
		if ( output >= 0 )
			close( output );
		int failure = 0;
		const ssize_t got = read( pipeFds[0], &failure, sizeof( failure ) );
		close( pipeFds[0] );
		if ( got == sizeof( failure ) )
		{
			int raw = 0;
			waitpid( pid, &raw, 0 );
			error = "cannot start " + request.argv.front() + " in " + request.workingDirectory +
			        ": " + std::strerror( failure );
			return {};
		}
		return SpawnedProcess{ pid };
	}

	std::optional<int> Poll( SpawnedProcess process ) override
	{
		if ( process.id <= 0 )
			return 1;
		int raw = 0;
		const pid_t done = waitpid( static_cast<pid_t>( process.id ), &raw, WNOHANG );
		if ( done == 0 )
			return std::nullopt;
		return done < 0 ? 1 : StatusOf( raw );
	}

	int Wait( SpawnedProcess process ) override
	{
		if ( process.id <= 0 )
			return 1;
		int raw = 0;
		while ( waitpid( static_cast<pid_t>( process.id ), &raw, 0 ) < 0 )
		{
			if ( errno != EINTR )
				return 1;
		}
		return StatusOf( raw );
	}

	void Terminate( SpawnedProcess process, int graceMs ) override
	{
		if ( process.id <= 0 )
			return;
		const pid_t group = static_cast<pid_t>( process.id );
		kill( -group, SIGTERM );
		const auto deadline =
		    std::chrono::steady_clock::now() + std::chrono::milliseconds( graceMs );
		while ( std::chrono::steady_clock::now() < deadline )
		{
			if ( Poll( process ) )
				return;
			std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
		}
		kill( -group, SIGKILL );
		Wait( process );
	}
};

} // namespace

std::unique_ptr<IProcessSpawner> CreatePosixProcessSpawner()
{
	return std::make_unique<PosixProcessSpawner>();
}

} // namespace platform
