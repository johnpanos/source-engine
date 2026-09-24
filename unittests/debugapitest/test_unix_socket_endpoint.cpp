//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: the "unix:" transport's endpoint policy (unix_socket_transport.h):
//          address validation, 0600 mode, refusal to replace live sockets or
//          other files, stale-socket replacement, owned-only unlink and the
//          connection bound.
//
//===========================================================================//

#include "conformance/checks.h"
#include "conformance/transport_fixtures.h"
#include "debugapi/debugapi_unix_socket.h"
#include "testing/conformance_result.h"

#include <cstring>
#include <fstream>
#include <string>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

namespace
{

using namespace debugapitest;
using debugapi::TransportErrorCode;

bool FailsWith( const debugapi::TransportResult &result, TransportErrorCode code )
{
	return !result && result.Error().code == code && !result.Error().message.empty();
}

bool Exists( const std::string &path )
{
	struct stat info;
	return ::lstat( path.c_str(), &info ) == 0;
}

class CountingSink final : public debugapi::ITransportSink
{
public:
	void OnConnected( debugapi::ConnectionId ) override { ++connected; }
	void OnData( debugapi::ConnectionId, std::string_view bytes ) override { data.append( bytes ); }
	void OnDisconnected( debugapi::ConnectionId ) override { ++disconnected; }

	int connected = 0;
	int disconnected = 0;
	std::string data;
};

} // namespace

int main()
{
	Results results;
	results.scope = "unix-endpoint";
	UnixSocketFixture fixture;
	results.Check( !fixture.Directory().empty(), "a private socket directory was created" );
	if ( fixture.Directory().empty() )
		return testing::ReportConformance( results.checks, results.failures );

	results.Check( FailsWith( debugapi::CreateUnixSocketTransport( "relative.sock" ),
	                   TransportErrorCode::InvalidAddress ),
	    "a relative path is rejected" );
	results.Check( FailsWith( debugapi::CreateUnixSocketTransport( "/" + std::string( 200, 'x' ) ),
	                   TransportErrorCode::InvalidAddress ),
	    "a path longer than sun_path is rejected" );
	results.Check( FailsWith( debugapi::CreateUnixSocketTransport(
	                              fixture.NextPath(), debugapi::UnixSocketLimits{ 0, 1 } ),
	                   TransportErrorCode::InvalidAddress ),
	    "zero limits are rejected" );

	// Another file type at the path is never replaced.
	const std::string file = fixture.Directory() + "/file";
	{
		std::ofstream( file ) << "keep";
	}
	results.Check( FailsWith( debugapi::CreateUnixSocketTransport( file ),
	                   TransportErrorCode::InvalidAddress ),
	    "a regular file at the path is refused" );
	std::string kept;
	std::ifstream( file ) >> kept;
	results.Check( kept == "keep", "the refused file is left untouched" );

	// A live endpoint is not stolen; the first server keeps working.
	const std::string live = fixture.NextPath();
	debugapi::TransportResult first = debugapi::CreateUnixSocketTransport( live );
	results.Check( first.HasValue(), "a fresh path is bound" );
	if ( first )
	{
		struct stat info;
		results.Check( ::stat( live.c_str(), &info ) == 0 && ( info.st_mode & 0777 ) == 0600,
		    "the socket file is created with mode 0600" );
		results.Check( FailsWith( debugapi::CreateUnixSocketTransport( live ),
		                   TransportErrorCode::AddressInUse ),
		    "a live socket is refused with AddressInUse" );
		// The refused server's liveness probe shows up here as a connection
		// that closes at once; the real peer must still be served.
		auto peer = ConnectUnixPeer( live );
		peer->Write( "still-live" );
		CountingSink sink;
		for ( int i = 0; i < 100 && sink.data.find( "still-live" ) == std::string::npos; ++i )
			first.Value()->Poll( sink );
		results.Check( sink.data == "still-live", "the original server still serves its peers" );
		first.Value()->Shutdown();
		results.Check( !Exists( live ), "Shutdown unlinks the socket file it created" );
	}

	// A stale socket (nothing accepting) is replaced.
	const std::string stale = fixture.NextPath();
	{
		const int fd = ::socket( AF_UNIX, SOCK_STREAM, 0 );
		sockaddr_un address{};
		address.sun_family = AF_UNIX;
		std::strncpy( address.sun_path, stale.c_str(), sizeof( address.sun_path ) - 1 );
		const bool bound =
		    ::bind( fd, reinterpret_cast<const sockaddr *>( &address ), sizeof( address ) ) == 0;
		::close( fd );
		results.Check( bound && Exists( stale ), "a stale socket file is staged" );
	}
	debugapi::TransportResult replaced = debugapi::CreateUnixSocketTransport( stale );
	results.Check( replaced.HasValue(), "a stale socket is replaced" );

	// Shutdown never removes a path someone else re-created.
	const std::string moved = fixture.NextPath();
	debugapi::TransportResult owner = debugapi::CreateUnixSocketTransport( moved );
	results.Check( owner.HasValue(), "a second fresh path is bound" );
	if ( owner )
	{
		::unlink( moved.c_str() );
		{
			std::ofstream( moved ) << "other";
		}
		owner.Value()->Shutdown();
		results.Check( Exists( moved ), "Shutdown leaves a path it no longer owns" );
		::unlink( moved.c_str() );
	}

	// Connections over the transport's bound are closed without OnConnected.
	const std::string bounded = fixture.NextPath();
	debugapi::TransportResult limited =
	    debugapi::CreateUnixSocketTransport( bounded, debugapi::UnixSocketLimits{ 1, 65536 } );
	results.Check( limited.HasValue(), "a bounded transport is bound" );
	if ( limited )
	{
		auto a = ConnectUnixPeer( bounded );
		auto b = ConnectUnixPeer( bounded );
		CountingSink sink;
		bool excessClosed = false;
		for ( int i = 0; i < 100 && !excessClosed; ++i )
		{
			limited.Value()->Poll( sink );
			excessClosed = b->SawClose();
		}
		results.Check( sink.connected == 1, "only maxConnections connections are announced" );
		results.Check( excessClosed, "the excess connection is closed" );
	}
	return testing::ReportConformance( results.checks, results.failures );
}
