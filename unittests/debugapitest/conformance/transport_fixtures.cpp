//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: transport fixtures (memory, Unix socket) for the shared suites.
//
//===========================================================================//

#include "transport_fixtures.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <utility>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "debugapi/core/memory_transport.h"
#include "debugapi/debugapi_unix_socket.h"

namespace debugapitest
{
namespace
{

class MemoryTestPeer final : public ITestPeer
{
public:
	explicit MemoryTestPeer( debugapi::MemoryPeer peer ) : m_Peer( std::move( peer ) ) {}

	bool Write( std::string_view bytes ) override { return m_Peer.Write( bytes ); }
	std::string Read() override { return m_Peer.Read(); }
	bool SawClose() override { return m_Peer.ServerClosed(); }
	void Close() override { m_Peer.Close(); }

private:
	debugapi::MemoryPeer m_Peer;
};

class UnixTestPeer final : public ITestPeer
{
public:
	explicit UnixTestPeer( int fd ) : m_Fd( fd ) {}
	~UnixTestPeer() override { Close(); }

	bool Write( std::string_view bytes ) override
	{
		if ( m_Fd < 0 || m_Eof )
			return false;
		m_Outbox.append( bytes );
		Flush();
		return true;
	}

	std::string Read() override
	{
		Flush();
		Drain();
		return std::exchange( m_Inbox, std::string() );
	}

	bool SawClose() override
	{
		Flush();
		Drain();
		return m_Eof;
	}

	void Close() override
	{
		if ( m_Fd >= 0 )
			::close( m_Fd );
		m_Fd = -1;
	}

private:
	void Flush()
	{
		while ( m_Fd >= 0 && !m_Outbox.empty() )
		{
			const ssize_t count = ::send( m_Fd, m_Outbox.data(), m_Outbox.size(), MSG_NOSIGNAL );
			if ( count <= 0 )
				return;
			m_Outbox.erase( 0, static_cast<size_t>( count ) );
		}
	}

	void Drain()
	{
		char buffer[65536];
		while ( m_Fd >= 0 && !m_Eof )
		{
			const ssize_t count = ::recv( m_Fd, buffer, sizeof( buffer ), 0 );
			if ( count > 0 )
				m_Inbox.append( buffer, static_cast<size_t>( count ) );
			else if ( count == 0 || ( errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR ) )
				m_Eof = true;
			else
				return;
		}
	}

	int m_Fd;
	std::string m_Inbox;
	std::string m_Outbox;
	bool m_Eof = false;
};

// Stands in for a connection that could not be made, so cases fail cleanly.
class DeadPeer final : public ITestPeer
{
public:
	bool Write( std::string_view ) override { return false; }
	std::string Read() override { return std::string(); }
	bool SawClose() override { return false; }
	void Close() override {}
};

} // namespace

std::unique_ptr<debugapi::ITransport> MemoryFixture::Create()
{
	return std::make_unique<debugapi::MemoryTransport>();
}

std::unique_ptr<ITestPeer> MemoryFixture::Connect( debugapi::ITransport &transport )
{
	auto &memory = static_cast<debugapi::MemoryTransport &>( transport );
	return std::make_unique<MemoryTestPeer>( memory.Connect() );
}

UnixSocketFixture::UnixSocketFixture()
{
	char pattern[] = "/tmp/debugapi-test-XXXXXX";
	if ( ::mkdtemp( pattern ) != nullptr )
		m_Directory = pattern;
}

UnixSocketFixture::~UnixSocketFixture()
{
	if ( m_Directory.empty() )
		return;
	for ( int i = 1; i <= m_Counter; ++i )
		::unlink( ( m_Directory + "/s" + std::to_string( i ) ).c_str() );
	::unlink( ( m_Directory + "/file" ).c_str() );
	::rmdir( m_Directory.c_str() );
}

std::string UnixSocketFixture::NextPath()
{
	return m_Directory + "/s" + std::to_string( ++m_Counter );
}

std::unique_ptr<debugapi::ITransport> UnixSocketFixture::Create()
{
	m_LastPath = NextPath();
	debugapi::TransportResult created = debugapi::CreateUnixSocketTransport( m_LastPath );
	if ( !created )
		return nullptr;
	return std::move( created ).Value();
}

std::unique_ptr<ITestPeer> UnixSocketFixture::Connect( debugapi::ITransport &transport )
{
	(void)transport;
	return ConnectUnixPeer( m_LastPath );
}

std::unique_ptr<ITestPeer> ConnectUnixPeer( const std::string &path )
{
	const int fd = ::socket( AF_UNIX, SOCK_STREAM, 0 );
	if ( fd < 0 )
		return std::make_unique<DeadPeer>();
	sockaddr_un address{};
	address.sun_family = AF_UNIX;
	std::strncpy( address.sun_path, path.c_str(), sizeof( address.sun_path ) - 1 );
	if ( ::connect( fd, reinterpret_cast<const sockaddr *>( &address ), sizeof( address ) ) != 0 ||
	     ::fcntl( fd, F_SETFL, ::fcntl( fd, F_GETFL, 0 ) | O_NONBLOCK ) != 0 )
	{
		::close( fd );
		return std::make_unique<DeadPeer>();
	}
	return std::make_unique<UnixTestPeer>( fd );
}

} // namespace debugapitest
