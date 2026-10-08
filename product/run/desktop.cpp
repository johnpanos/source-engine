//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.run.desktop: single, external-install, coop-pair and
//			browser-page.
//
//=============================================================================//

#include "product/run_desktop.h"

#include <chrono>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace product
{

namespace
{

namespace fs = std::filesystem;

ProviderError Fail( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

bool Cancelled( const RunRequest &request )
{
	return request.cancel && request.cancel->IsCancelled();
}

std::string Fact( const RunRequest &request, const char *name, std::string fallback )
{
	auto it = request.facts.find( name );
	return it == request.facts.end() || it->second.empty() ? fallback : it->second;
}

// The display's prefix, then the program; the launch's environment, then
// the display's: the display session owns its variables (a headless session's
// SDL_VIDEODRIVER wins over the profile's player default).
platform::SpawnRequest SpawnFor( const RunRequest &request, const LaunchSpec &launch )
{
	platform::SpawnRequest spawn;
	spawn.argv = request.display.commandPrefix;
	spawn.argv.insert( spawn.argv.end(), launch.argv.begin(), launch.argv.end() );
	spawn.workingDirectory = launch.workingDirectory.string();
	spawn.environment = launch.environment;
	spawn.environment.insert( spawn.environment.end(), request.display.environment.begin(),
	    request.display.environment.end() );
	spawn.outputFile = launch.outputFile.string();
	return spawn;
}

foundation::Expected<platform::SpawnedProcess, ProviderError> Start(
    const RunRequest &request, const LaunchSpec &launch )
{
	std::string error;
	const platform::SpawnedProcess process =
	    request.spawner->Spawn( SpawnFor( request, launch ), error );
	if ( process.id < 0 )
		return foundation::MakeUnexpected( Fail( "spawn-failed", launch.name + ": " + error ) );
	if ( request.diagnostics )
		request.diagnostics->Report( Severity::kInfo, "run", "started " + launch.name );
	if ( request.started )
		request.started( launch.name, process );
	return process;
}

// Waits for every process; stops all of them on cancellation. The status is
// the first nonzero one.
int WaitAll( const RunRequest &request, const std::vector<platform::SpawnedProcess> &processes )
{
	std::vector<std::optional<int>> done( processes.size() );
	while ( true )
	{
		bool running = false;
		for ( size_t i = 0; i < processes.size(); ++i )
		{
			if ( !done[i] )
				done[i] = request.spawner->Poll( processes[i] );
			running |= !done[i];
		}
		if ( !running )
			break;
		if ( Cancelled( request ) )
		{
			for ( size_t i = 0; i < processes.size(); ++i )
			{
				if ( !done[i] )
				{
					request.spawner->Terminate( processes[i], 3000 );
					done[i] = 130;
				}
			}
			break;
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	}
	for ( const auto &status : done )
	{
		if ( status && *status != 0 )
			return *status;
	}
	return 0;
}

class SingleRun final : public IRunProvider
{
public:
	explicit SingleRun( const char *name ) : m_Name( name ) {}
	std::string_view Name() const noexcept override { return m_Name; }
	foundation::Expected<int, ProviderError> Run( const RunRequest &request ) override
	{
		if ( Cancelled( request ) )
			return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "" ) );
		if ( !request.spawner || request.launches.size() != 1 )
			return foundation::MakeUnexpected(
			    Fail( "invalid-request", std::string( m_Name ) + " runs one launch" ) );
		auto process = Start( request, request.launches.front() );
		if ( !process )
			return foundation::MakeUnexpected( process.Error() );
		return WaitAll( request, { process.Value() } );
	}

private:
	const char *m_Name;
};

// The address this host uses toward the LAN: the source of the default
// route, found by connecting a UDP socket (no packet is sent).
std::string LanAddress()
{
	const int fd = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 )
		return {};
	sockaddr_in remote{};
	remote.sin_family = AF_INET;
	remote.sin_port = htons( 80 );
	inet_pton( AF_INET, "1.1.1.1", &remote.sin_addr );
	std::string address;
	if ( connect( fd, reinterpret_cast<sockaddr *>( &remote ), sizeof( remote ) ) == 0 )
	{
		sockaddr_in local{};
		socklen_t length = sizeof( local );
		char text[INET_ADDRSTRLEN] = {};
		if ( getsockname( fd, reinterpret_cast<sockaddr *>( &local ), &length ) == 0 &&
		     inet_ntop( AF_INET, &local.sin_addr, text, sizeof( text ) ) )
			address = text;
	}
	close( fd );
	return address;
}

// The engine answers a server-challenge request ('W' -> 'A') once its server
// runs (engine/baseserver.cpp); without Steam it does not answer A2S_INFO.
bool ServerAnswers( const std::string &address, int port )
{
	const int fd = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 )
		return false;
	timeval timeout{ 1, 0 };
	setsockopt( fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );
	sockaddr_in server{};
	server.sin_family = AF_INET;
	server.sin_port = htons( static_cast<std::uint16_t>( port ) );
	inet_pton( AF_INET, address.c_str(), &server.sin_addr );
	const char request[] = "\xff\xff\xff\xffW";
	char reply[64] = {};
	bool answered = false;
	if ( sendto( fd, request, 5, 0, reinterpret_cast<sockaddr *>( &server ), sizeof( server ) ) ==
	     5 )
	{
		const ssize_t got = recv( fd, reply, sizeof( reply ), 0 );
		answered = got >= 5 && std::memcmp( reply,
		                           "\xff\xff\xff\xff"
		                           "A",
		                           5 ) == 0;
	}
	close( fd );
	return answered;
}

// A remote player's connect on the host: Client "<name>" connected (<addr>),
// the host's own player connecting over the loopback excluded.
std::optional<std::string> RemoteJoin( const fs::path &log )
{
	std::ifstream stream( log, std::ios::binary );
	std::ostringstream buffer;
	buffer << stream.rdbuf();
	const std::string text = buffer.str();
	const std::string head = "Client \"";
	for ( size_t at = text.find( head ); at != std::string::npos; at = text.find( head, at + 1 ) )
	{
		const size_t nameEnd = text.find( '"', at + head.size() );
		if ( nameEnd == std::string::npos )
			break;
		const std::string tail = "\" connected (";
		if ( text.compare( nameEnd, tail.size(), tail ) != 0 )
			continue;
		const size_t open = nameEnd + tail.size();
		const size_t close = text.find( ')', open );
		if ( close == std::string::npos || text.compare( open, 8, "loopback" ) == 0 )
			continue;
		return text.substr( at + head.size(), nameEnd - at - head.size() ) + " from " +
		       text.substr( open, close - open );
	}
	return std::nullopt;
}

std::string Replace( std::string text, const std::string &from, const std::string &to )
{
	for ( size_t at = text.find( from ); at != std::string::npos;
	    at = text.find( from, at + to.size() ) )
		text.replace( at, from.size(), to );
	return text;
}

class CoopPairRun final : public IRunProvider
{
public:
	std::string_view Name() const noexcept override { return "coop-pair"; }
	foundation::Expected<int, ProviderError> Run( const RunRequest &request ) override
	{
		if ( Cancelled( request ) )
			return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "" ) );
		if ( !request.spawner || request.launches.size() != 2 )
			return foundation::MakeUnexpected(
			    Fail( "invalid-request", "coop-pair runs a host and a client" ) );
		const int port = std::atoi( Fact( request, "port", "27015" ).c_str() );
		const int timeout = std::atoi( Fact( request, "timeout", "300" ).c_str() );
		const std::string address = LanAddress();
		if ( address.empty() )
			return foundation::MakeUnexpected(
			    Fail( std::string( kUnavailable ), "no LAN address" ) );
		if ( ServerAnswers( address, port ) )
			return foundation::MakeUnexpected(
			    Fail( "busy", "a server already answers at " + address + ":" +
			                      std::to_string( port ) + "; stop it or change the port" ) );
		LaunchSpec host = request.launches[0], client = request.launches[1];
		for ( LaunchSpec *launch : { &host, &client } )
		{
			for ( std::string &argument : launch->argv )
				argument = Replace( argument, "{lan_address}", address );
		}
		const fs::path log = host.workingDirectory / Fact( request, "join_log", "engine.log" );
		std::error_code ec;
		fs::remove( log, ec ); // a join line from an earlier run must not count
		auto hostProcess = Start( request, host );
		if ( !hostProcess )
			return foundation::MakeUnexpected( hostProcess.Error() );
		std::vector<platform::SpawnedProcess> processes = { hostProcess.Value() };
		const auto stopAll = [&]
		{
			for ( const platform::SpawnedProcess &process : processes )
				request.spawner->Terminate( process, 3000 );
		};
		const auto waitFor = [&]( auto ready ) -> std::optional<ProviderError>
		{
			const auto deadline =
			    std::chrono::steady_clock::now() + std::chrono::seconds( timeout );
			while ( std::chrono::steady_clock::now() < deadline )
			{
				if ( Cancelled( request ) )
					return Fail( std::string( kCancelled ), "" );
				for ( const platform::SpawnedProcess &process : processes )
				{
					if ( auto status = request.spawner->Poll( process ) )
						return Fail(
						    "exited", "a peer exited with status " + std::to_string( *status ) );
				}
				if ( ready() )
					return std::nullopt;
				std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
			}
			return Fail( "timeout", "no answer within " + std::to_string( timeout ) + " s" );
		};
		if ( auto e = waitFor(
		         [&]
		         {
			         return ServerAnswers( address, port );
		         } ) )
		{
			stopAll();
			return foundation::MakeUnexpected( Fail( e->code, "host: " + e->detail ) );
		}
		if ( request.diagnostics )
			request.diagnostics->Report( Severity::kInfo, "run",
			    "host answers at " + address + ":" + std::to_string( port ) );
		auto clientProcess = Start( request, client );
		if ( !clientProcess )
		{
			stopAll();
			return foundation::MakeUnexpected( clientProcess.Error() );
		}
		processes.push_back( clientProcess.Value() );
		std::optional<std::string> joined;
		if ( auto e = waitFor(
		         [&]
		         {
			         return ( joined = RemoteJoin( log ) ).has_value();
		         } ) )
		{
			stopAll();
			return foundation::MakeUnexpected( Fail( e->code, "client: " + e->detail ) );
		}
		if ( request.diagnostics )
			request.diagnostics->Report( Severity::kInfo, "run", "connected: " + *joined );
		if ( Fact( request, "stop_after_join", "0" ) == "1" )
		{
			stopAll();
			return 0;
		}
		return WaitAll( request, processes );
	}
};

// A TCP server listening on the loopback at the port.
bool LoopbackListens( int port )
{
	const int fd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( fd < 0 )
		return false;
	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_port = htons( static_cast<uint16_t>( port ) );
	address.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	const bool listening =
	    connect( fd, reinterpret_cast<sockaddr *>( &address ), sizeof( address ) ) == 0;
	close( fd );
	return listening;
}

class BrowserPageRun final : public IRunProvider
{
public:
	std::string_view Name() const noexcept override { return "browser-page"; }
	foundation::Expected<int, ProviderError> Run( const RunRequest &request ) override
	{
		if ( Cancelled( request ) )
			return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "" ) );
		if ( !request.spawner || request.launches.size() != 2 )
			return foundation::MakeUnexpected(
			    Fail( "invalid-request", "browser-page runs a server and a browser" ) );
		const int port = std::atoi( Fact( request, "port", "8080" ).c_str() );
		const int timeout = std::atoi( Fact( request, "timeout", "60" ).c_str() );
		if ( LoopbackListens( port ) )
			return foundation::MakeUnexpected(
			    Fail( "busy", "a server already listens on 127.0.0.1:" + std::to_string( port ) +
			                      "; stop it or change the port" ) );
		// The server draws nothing: it runs outside the display session, whose
		// compositor (a private one wraps each program it runs) is the browser's.
		RunRequest outside = request;
		outside.display = {};
		auto server = Start( outside, request.launches[0] );
		if ( !server )
			return foundation::MakeUnexpected( server.Error() );
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( timeout );
		while ( !LoopbackListens( port ) )
		{
			std::optional<ProviderError> failure;
			if ( Cancelled( request ) )
				failure = Fail( std::string( kCancelled ), "" );
			else if ( auto status = request.spawner->Poll( server.Value() ) )
				failure =
				    Fail( "exited", "server: exited with status " + std::to_string( *status ) );
			else if ( std::chrono::steady_clock::now() >= deadline )
				failure = Fail(
				    "timeout", "server: no listener within " + std::to_string( timeout ) + " s" );
			if ( failure )
			{
				request.spawner->Terminate( server.Value(), 3000 );
				return foundation::MakeUnexpected( *failure );
			}
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}
		auto browser = Start( request, request.launches[1] );
		if ( !browser )
		{
			request.spawner->Terminate( server.Value(), 3000 );
			return foundation::MakeUnexpected( browser.Error() );
		}
		// The server ends when the page reports the engine's exit, with that
		// status; a browser closed first ends the run as cancelled.
		while ( true )
		{
			if ( auto status = request.spawner->Poll( server.Value() ) )
			{
				request.spawner->Terminate( browser.Value(), 3000 );
				return *status;
			}
			if ( Cancelled( request ) || request.spawner->Poll( browser.Value() ) )
			{
				request.spawner->Terminate( browser.Value(), 3000 );
				request.spawner->Terminate( server.Value(), 3000 );
				return 130;
			}
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}
	}
};

} // namespace

std::unique_ptr<IRunProvider> CreateSingleRunProvider()
{
	return std::make_unique<SingleRun>( "single" );
}

std::unique_ptr<IRunProvider> CreateExternalInstallRunProvider()
{
	return std::make_unique<SingleRun>( "external-install" );
}

std::unique_ptr<IRunProvider> CreateCoopPairRunProvider()
{
	return std::make_unique<CoopPairRun>();
}

std::unique_ptr<IRunProvider> CreateBrowserPageRunProvider()
{
	return std::make_unique<BrowserPageRun>();
}

} // namespace product
