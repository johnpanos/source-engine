//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX implementation of the structured tool-process contract.
//
//=============================================================================//

#include "tool_process_provider.h"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <poll.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

extern char **environ;

namespace platform
{

namespace
{

using Clock = std::chrono::steady_clock;

enum class ChildFailureStage
{
	kProcessGroup = 1,
	kWorkingDirectory,
	kStdout,
	kStderr,
	kExecute,
};

struct ChildFailure
{
	ChildFailureStage stage;
	int error;
};

struct Pipe
{
	int read = -1;
	int write = -1;
};

void CloseDescriptor( int &descriptor )
{
	if ( descriptor >= 0 )
	{
		while ( close( descriptor ) != 0 && errno == EINTR )
		{
		}
		descriptor = -1;
	}
}

bool SetDescriptorFlag( int descriptor, int command, int flag )
{
	int current = fcntl( descriptor, command );
	if ( current < 0 )
	{
		return false;
	}
	return fcntl( descriptor, command == F_GETFD ? F_SETFD : F_SETFL, current | flag ) == 0;
}

bool OpenPipe( Pipe &pipe )
{
	int descriptors[2] = { -1, -1 };
	if ( ::pipe( descriptors ) != 0 )
	{
		return false;
	}
	pipe.read = descriptors[0];
	pipe.write = descriptors[1];
	if ( !SetDescriptorFlag( pipe.read, F_GETFD, FD_CLOEXEC ) ||
	     !SetDescriptorFlag( pipe.write, F_GETFD, FD_CLOEXEC ) )
	{
		CloseDescriptor( pipe.read );
		CloseDescriptor( pipe.write );
		return false;
	}
	return true;
}

bool ContainsNul( const std::string &value )
{
	return value.find( '\0' ) != std::string::npos;
}

ToolProcessError Validate( const ToolProcessRequest &request )
{
	ToolProcessError error;
	error.requestedExecutable = request.argv.empty() ? std::string() : request.argv[0];
	if ( request.argv.empty() || request.argv[0].empty() || request.workingDirectory.empty() ||
	     request.executionTimeout <= std::chrono::milliseconds::zero() ||
	     request.cancellationTimeout <= std::chrono::milliseconds::zero() )
	{
		error.code = ToolProcessErrorCode::kInvalidArgument;
		error.detail = "missing executable, working directory, or positive timeout";
		return error;
	}
	for ( const std::string &argument : request.argv )
	{
		if ( ContainsNul( argument ) )
		{
			error.code = ToolProcessErrorCode::kInvalidArgument;
			error.detail = "argument contains NUL";
			return error;
		}
	}
	if ( ContainsNul( request.workingDirectory ) )
	{
		error.code = ToolProcessErrorCode::kInvalidArgument;
		error.detail = "working directory contains NUL";
		return error;
	}
	for ( std::size_t i = 0; i < request.environment.size(); ++i )
	{
		const ToolProcessEnvironmentOverride &entry = request.environment[i];
		if ( entry.name.empty() || entry.name.find( '=' ) != std::string::npos ||
		     ContainsNul( entry.name ) || ( entry.value && ContainsNul( *entry.value ) ) )
		{
			error.code = ToolProcessErrorCode::kInvalidArgument;
			error.detail = "invalid environment override";
			return error;
		}
		for ( std::size_t previous = 0; previous < i; ++previous )
		{
			if ( request.environment[previous].name == entry.name )
			{
				error.code = ToolProcessErrorCode::kInvalidArgument;
				error.detail = "duplicate environment override";
				return error;
			}
		}
	}
	return error;
}

ToolProcessErrorCode ErrorCodeForErrno( int value )
{
	switch ( value )
	{
	case ENOENT:
	case ENOTDIR:
		return ToolProcessErrorCode::kExecutableNotFound;
	case EACCES:
	case EPERM:
		return ToolProcessErrorCode::kPermissionDenied;
	case EAGAIN:
	case EMFILE:
	case ENFILE:
	case ENOMEM:
		return ToolProcessErrorCode::kResourceExhausted;
	default:
		return ToolProcessErrorCode::kProviderFailure;
	}
}

const char *FailureStageName( ChildFailureStage stage )
{
	switch ( stage )
	{
	case ChildFailureStage::kProcessGroup:
		return "create process group";
	case ChildFailureStage::kWorkingDirectory:
		return "change working directory";
	case ChildFailureStage::kStdout:
		return "connect stdout";
	case ChildFailureStage::kStderr:
		return "connect stderr";
	case ChildFailureStage::kExecute:
		return "execute child";
	}
	return "start child";
}

ToolProcessResult FailureResult( ToolProcessCompletion completion, ToolProcessErrorCode code,
	const ToolProcessRequest &request, int providerCode, const std::string &detail )
{
	ToolProcessResult result;
	result.completion = completion;
	result.error.code = code;
	result.error.providerCode = providerCode;
	result.error.requestedExecutable = request.argv.empty() ? std::string() : request.argv[0];
	result.error.detail = detail;
	result.cleanupComplete = true;
	return result;
}

std::string EnvironmentName( const std::string &entry )
{
	const std::size_t separator = entry.find( '=' );
	return separator == std::string::npos ? entry : entry.substr( 0, separator );
}

std::vector<std::string> BuildEnvironment( const ToolProcessRequest &request )
{
	std::vector<std::string> result;
	for ( char **entry = ::environ; entry != nullptr && *entry != nullptr; ++entry )
	{
		const std::string inherited( *entry );
		const std::string name = EnvironmentName( inherited );
		bool overridden = false;
		for ( const ToolProcessEnvironmentOverride &overrideEntry : request.environment )
		{
			if ( overrideEntry.name == name )
			{
				overridden = true;
				break;
			}
		}
		if ( !overridden )
		{
			result.push_back( inherited );
		}
	}
	for ( const ToolProcessEnvironmentOverride &entry : request.environment )
	{
		if ( entry.value )
		{
			result.push_back( entry.name + "=" + *entry.value );
		}
	}
	return result;
}

std::string FindEnvironmentValue(
	const std::vector<std::string> &environment, const std::string &name )
{
	const std::string prefix = name + "=";
	for ( auto entry = environment.rbegin(); entry != environment.rend(); ++entry )
	{
		if ( entry->compare( 0, prefix.size(), prefix ) == 0 )
		{
			return entry->substr( prefix.size() );
		}
	}
	return std::string();
}

std::vector<std::string> BuildExecutableCandidates(
	const std::string &executable, const std::vector<std::string> &environment )
{
	if ( executable.find( '/' ) != std::string::npos )
	{
		return { executable };
	}

	std::string path = FindEnvironmentValue( environment, "PATH" );
	if ( path.empty() )
	{
		path = "/bin:/usr/bin";
	}
	std::vector<std::string> candidates;
	std::size_t begin = 0;
	do
	{
		const std::size_t end = path.find( ':', begin );
		const std::string directory = path.substr( begin, end - begin );
		candidates.push_back( directory.empty() ? executable : directory + "/" + executable );
		if ( end == std::string::npos )
		{
			break;
		}
		begin = end + 1;
	} while ( true );
	return candidates;
}

std::vector<char *> StringPointers( std::vector<std::string> &strings )
{
	std::vector<char *> pointers;
	pointers.reserve( strings.size() + 1 );
	for ( std::string &string : strings )
	{
		pointers.push_back( string.data() );
	}
	pointers.push_back( nullptr );
	return pointers;
}

[[noreturn]] void ChildFail( int descriptor, ChildFailureStage stage, int error )
{
	const ChildFailure failure{ stage, error };
	const char *bytes = reinterpret_cast<const char *>( &failure );
	std::size_t remaining = sizeof( failure );
	while ( remaining > 0 )
	{
		const ssize_t written = write( descriptor, bytes, remaining );
		if ( written > 0 )
		{
			bytes += written;
			remaining -= static_cast<std::size_t>( written );
		}
		else if ( written < 0 && errno == EINTR )
		{
			continue;
		}
		else
		{
			break;
		}
	}
	_exit( 127 );
}

[[noreturn]] void RunChild( const ToolProcessRequest &request, Pipe &stdoutPipe, Pipe &stderrPipe,
	Pipe &failurePipe, std::vector<char *> &arguments, std::vector<char *> &environment,
	const std::vector<std::string> &candidates )
{
	CloseDescriptor( stdoutPipe.read );
	CloseDescriptor( stderrPipe.read );
	CloseDescriptor( failurePipe.read );
	if ( setpgid( 0, 0 ) != 0 )
	{
		ChildFail( failurePipe.write, ChildFailureStage::kProcessGroup, errno );
	}
	if ( chdir( request.workingDirectory.c_str() ) != 0 )
	{
		ChildFail( failurePipe.write, ChildFailureStage::kWorkingDirectory, errno );
	}
	if ( dup2( stdoutPipe.write, STDOUT_FILENO ) < 0 )
	{
		ChildFail( failurePipe.write, ChildFailureStage::kStdout, errno );
	}
	if ( dup2( stderrPipe.write, STDERR_FILENO ) < 0 )
	{
		ChildFail( failurePipe.write, ChildFailureStage::kStderr, errno );
	}
	CloseDescriptor( stdoutPipe.write );
	CloseDescriptor( stderrPipe.write );

	int finalError = ENOENT;
	bool sawPermissionDenied = false;
	for ( const std::string &candidate : candidates )
	{
		execve( candidate.c_str(), arguments.data(), environment.data() );
		finalError = errno;
		if ( errno == EACCES )
		{
			sawPermissionDenied = true;
		}
		else if ( errno != ENOENT && errno != ENOTDIR )
		{
			break;
		}
	}
	if ( sawPermissionDenied && ( finalError == ENOENT || finalError == ENOTDIR ) )
	{
		finalError = EACCES;
	}
	ChildFail( failurePipe.write, ChildFailureStage::kExecute, finalError );
}

void DrainPipe( int &descriptor, std::string &output )
{
	char buffer[4096];
	while ( descriptor >= 0 )
	{
		const ssize_t count = read( descriptor, buffer, sizeof( buffer ) );
		if ( count > 0 )
		{
			output.append( buffer, static_cast<std::size_t>( count ) );
		}
		else if ( count == 0 )
		{
			CloseDescriptor( descriptor );
		}
		else if ( errno == EINTR )
		{
			continue;
		}
		else if ( errno == EAGAIN || errno == EWOULDBLOCK )
		{
			break;
		}
		else
		{
			CloseDescriptor( descriptor );
		}
	}
}

void DrainFailurePipe( int &descriptor, std::string &bytes )
{
	DrainPipe( descriptor, bytes );
}

bool ProcessGroupExists( pid_t processGroup )
{
	if ( kill( -processGroup, 0 ) == 0 )
	{
		return true;
	}
	return errno == EPERM;
}

void SignalProcessGroup( pid_t processGroup, int signal )
{
	if ( kill( -processGroup, signal ) != 0 && errno == ESRCH )
	{
		kill( processGroup, signal );
	}
}

class CPosixToolProcessProvider final : public IToolProcessProvider
{
public:
	ToolProcessResult Run( const ToolProcessRequest &request ) override
	{
		std::lock_guard<std::mutex> lock( m_RunMutex );
		const ToolProcessError validation = Validate( request );
		if ( !validation.IsOk() )
		{
			ToolProcessResult result;
			result.completion = ToolProcessCompletion::kInvalidRequest;
			result.error = validation;
			result.cleanupComplete = true;
			return result;
		}

		Pipe stdoutPipe;
		Pipe stderrPipe;
		Pipe failurePipe;
		if ( !OpenPipe( stdoutPipe ) || !OpenPipe( stderrPipe ) || !OpenPipe( failurePipe ) )
		{
			const int error = errno;
			CloseDescriptor( stdoutPipe.read );
			CloseDescriptor( stdoutPipe.write );
			CloseDescriptor( stderrPipe.read );
			CloseDescriptor( stderrPipe.write );
			CloseDescriptor( failurePipe.read );
			CloseDescriptor( failurePipe.write );
			return FailureResult( ToolProcessCompletion::kSpawnFailed, ErrorCodeForErrno( error ),
				request, error, "create child pipes: " + std::string( std::strerror( error ) ) );
		}

		std::vector<std::string> environmentStrings = BuildEnvironment( request );
		std::vector<std::string> argumentStrings = request.argv;
		const std::vector<std::string> candidates =
			BuildExecutableCandidates( request.argv[0], environmentStrings );
		std::vector<char *> environment = StringPointers( environmentStrings );
		std::vector<char *> arguments = StringPointers( argumentStrings );

		const pid_t child = fork();
		if ( child < 0 )
		{
			const int error = errno;
			CloseDescriptor( stdoutPipe.read );
			CloseDescriptor( stdoutPipe.write );
			CloseDescriptor( stderrPipe.read );
			CloseDescriptor( stderrPipe.write );
			CloseDescriptor( failurePipe.read );
			CloseDescriptor( failurePipe.write );
			return FailureResult( ToolProcessCompletion::kSpawnFailed, ErrorCodeForErrno( error ),
				request, error, "fork child: " + std::string( std::strerror( error ) ) );
		}
		if ( child == 0 )
		{
			RunChild( request, stdoutPipe, stderrPipe, failurePipe, arguments, environment,
				candidates );
		}

		m_LiveProcesses.fetch_add( 1, std::memory_order_release );
		CloseDescriptor( stdoutPipe.write );
		CloseDescriptor( stderrPipe.write );
		CloseDescriptor( failurePipe.write );
		SetDescriptorFlag( stdoutPipe.read, F_GETFL, O_NONBLOCK );
		SetDescriptorFlag( stderrPipe.read, F_GETFL, O_NONBLOCK );
		SetDescriptorFlag( failurePipe.read, F_GETFL, O_NONBLOCK );
		if ( setpgid( child, child ) != 0 && errno != EACCES && errno != ESRCH )
		{
			SignalProcessGroup( child, SIGKILL );
		}

		ToolProcessResult result;
		std::string failureBytes;
		int waitStatus = 0;
		bool childReaped = false;
		bool terminating = false;
		bool forced = false;
		bool strayProcessTree = false;
		ToolProcessCompletion requestedCompletion = ToolProcessCompletion::kExited;
		const Clock::time_point started = Clock::now();
		const Clock::time_point executionDeadline = started + request.executionTimeout;
		Clock::time_point terminationDeadline = Clock::time_point::max();

		while ( true )
		{
			DrainPipe( stdoutPipe.read, result.stdoutData );
			DrainPipe( stderrPipe.read, result.stderrData );
			DrainFailurePipe( failurePipe.read, failureBytes );

			if ( !childReaped )
			{
				const pid_t waited = waitpid( child, &waitStatus, WNOHANG );
				if ( waited == child )
				{
					childReaped = true;
				}
				else if ( waited < 0 && errno == ECHILD )
				{
					childReaped = true;
					waitStatus = 0;
				}
			}

			const Clock::time_point now = Clock::now();
			if ( !terminating )
			{
				if ( request.cancellation != nullptr &&
				     request.cancellation->IsCancellationRequested() )
				{
					terminating = true;
					requestedCompletion = ToolProcessCompletion::kCanceled;
					terminationDeadline = now + request.cancellationTimeout;
					SignalProcessGroup( child, SIGTERM );
				}
				else if ( now >= executionDeadline && !childReaped )
				{
					terminating = true;
					requestedCompletion = ToolProcessCompletion::kTimedOut;
					terminationDeadline = now + request.cancellationTimeout;
					SignalProcessGroup( child, SIGTERM );
				}
				else if ( childReaped && ProcessGroupExists( child ) )
				{
					terminating = true;
					strayProcessTree = true;
					terminationDeadline = now + request.cancellationTimeout;
					SignalProcessGroup( child, SIGTERM );
				}
			}
			else if ( !forced && now >= terminationDeadline && ProcessGroupExists( child ) )
			{
				forced = true;
				SignalProcessGroup( child, SIGKILL );
			}

			const bool groupGone = !ProcessGroupExists( child );
			if ( childReaped && stdoutPipe.read < 0 && stderrPipe.read < 0 &&
			     failurePipe.read < 0 && groupGone )
			{
				break;
			}

			pollfd descriptors[3];
			nfds_t count = 0;
			const int openDescriptors[] = { stdoutPipe.read, stderrPipe.read, failurePipe.read };
			for ( int descriptor : openDescriptors )
			{
				if ( descriptor >= 0 )
				{
					descriptors[count++] = { descriptor, POLLIN | POLLHUP, 0 };
				}
			}
			poll( descriptors, count, 10 );
		}

		CloseDescriptor( stdoutPipe.read );
		CloseDescriptor( stderrPipe.read );
		CloseDescriptor( failurePipe.read );
		m_LiveProcesses.fetch_sub( 1, std::memory_order_release );
		result.cleanupComplete = true;

		if ( failureBytes.size() >= sizeof( ChildFailure ) )
		{
			ChildFailure failure;
			std::memcpy( &failure, failureBytes.data(), sizeof( failure ) );
			result.completion = ToolProcessCompletion::kSpawnFailed;
			result.error.code = ErrorCodeForErrno( failure.error );
			result.error.providerCode = failure.error;
			result.error.requestedExecutable = request.argv[0];
			result.error.detail = std::string( FailureStageName( failure.stage ) ) + ": " +
			                      std::strerror( failure.error );
			return result;
		}
		if ( requestedCompletion == ToolProcessCompletion::kTimedOut ||
		     requestedCompletion == ToolProcessCompletion::kCanceled )
		{
			result.completion = requestedCompletion;
			result.cancellationAcknowledged = true;
			return result;
		}
		if ( strayProcessTree )
		{
			result.completion = ToolProcessCompletion::kProviderFailure;
			result.error.code = ToolProcessErrorCode::kCleanupFailed;
			result.error.requestedExecutable = request.argv[0];
			result.error.detail = "child exited while descendants were still running";
			return result;
		}

		result.completion = ToolProcessCompletion::kExited;
		if ( WIFEXITED( waitStatus ) )
		{
			result.exitCode = WEXITSTATUS( waitStatus );
		}
		else if ( WIFSIGNALED( waitStatus ) )
		{
			result.exitCode = 128 + WTERMSIG( waitStatus );
		}
		else
		{
			result.completion = ToolProcessCompletion::kProviderFailure;
			result.error.code = ToolProcessErrorCode::kProviderFailure;
			result.error.requestedExecutable = request.argv[0];
			result.error.detail = "child ended without an exit or signal status";
		}
		return result;
	}

	int LiveProcessCount() const noexcept override
	{
		return m_LiveProcesses.load( std::memory_order_acquire );
	}

private:
	std::mutex m_RunMutex;
	std::atomic<int> m_LiveProcesses{ 0 };
};

} // namespace

std::unique_ptr<IToolProcessProvider> CreatePosixToolProcessProvider()
{
	return std::make_unique<CPosixToolProcessProvider>();
}

} // namespace platform
