//========= Copyright Valve Corporation, All rights reserved. ============//
//
// POSIX stream-socket debugapi.transport.v1 provider.
//
//=============================================================================//

#include "debugapi/debugapi_unix_socket.h"

#include <cerrno>
#include <cstring>
#include <map>
#include <string>
#include <utility>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

namespace debugapi
{
namespace
{

#ifdef MSG_NOSIGNAL
constexpr int kSendFlags = MSG_NOSIGNAL;
#else
constexpr int kSendFlags = 0;
#endif

std::string ErrnoText( const char *what, int error )
{
	return std::string( what ) + ": " + std::strerror( error );
}

TransportError Failure( TransportErrorCode code, std::string message )
{
	return TransportError{ code, std::move( message ) };
}

class ScopedFd
{
public:
	ScopedFd() = default;
	explicit ScopedFd( int fd ) : m_Fd( fd ) {}
	ScopedFd( ScopedFd &&other ) noexcept : m_Fd( std::exchange( other.m_Fd, -1 ) ) {}
	ScopedFd &operator=( ScopedFd &&other ) noexcept
	{
		if ( this != &other )
		{
			Reset();
			m_Fd = std::exchange( other.m_Fd, -1 );
		}
		return *this;
	}
	~ScopedFd() { Reset(); }

	int Get() const { return m_Fd; }
	void Reset()
	{
		if ( m_Fd >= 0 )
			::close( m_Fd );
		m_Fd = -1;
	}

private:
	int m_Fd = -1;
};

bool ConfigureSocket( int fd )
{
	const int flags = ::fcntl( fd, F_GETFL, 0 );
	if ( flags < 0 || ::fcntl( fd, F_SETFL, flags | O_NONBLOCK ) < 0 )
		return false;
	if ( ::fcntl( fd, F_SETFD, FD_CLOEXEC ) < 0 )
		return false;
#if !defined( MSG_NOSIGNAL ) && defined( SO_NOSIGPIPE )
	int one = 1;
	if ( ::setsockopt( fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof( one ) ) < 0 )
		return false;
#endif
	return true;
}

class UnixSocketTransport final : public ITransport
{
public:
	UnixSocketTransport( std::string path, ScopedFd listener, dev_t device, ino_t inode,
	    UnixSocketLimits limits )
	    : m_Path( std::move( path ) ), m_Description( "unix:" + m_Path ),
	      m_Listener( std::move( listener ) ), m_Device( device ), m_Inode( inode ),
	      m_Limits( limits )
	{
	}

	~UnixSocketTransport() override { Shutdown(); }

	std::string_view Describe() const override { return m_Description; }

	void Poll( ITransportSink &sink ) override
	{
		if ( m_Shutdown )
			return;
		AcceptPending( sink );
		if ( m_Shutdown )
			return;

		for ( auto it = m_Connections.begin(); it != m_Connections.end(); )
		{
			const ConnectionId id{ it->first };
			Connection &connection = it->second;

			if ( connection.state == State::Open )
			{
				std::string bytes = ReadAvailable( connection );
				if ( !bytes.empty() )
				{
					sink.OnData( id, bytes );
					if ( m_Shutdown )
						return;
				}
			}

			if ( connection.state != State::Closed && !connection.peerClosed )
				Flush( connection );

			bool finished = connection.state == State::Closed || connection.peerClosed;
			if ( connection.state == State::Flushing && connection.output.empty() )
				finished = true;
			if ( !finished )
			{
				++it;
				continue;
			}
			it = m_Connections.erase( it );
			sink.OnDisconnected( id );
			if ( m_Shutdown )
				return;
		}
	}

	bool Send( ConnectionId id, std::string_view bytes ) override
	{
		auto it = m_Connections.find( id.value );
		if ( m_Shutdown || it == m_Connections.end() || it->second.state != State::Open ||
		     it->second.peerClosed )
			return false;
		it->second.output.append( bytes );
		Flush( it->second );
		return true;
	}

	size_t PendingOutputBytes( ConnectionId id ) const override
	{
		auto it = m_Connections.find( id.value );
		return it == m_Connections.end() ? 0 : it->second.output.size();
	}

	void Close( ConnectionId id, CloseMode mode ) override
	{
		auto it = m_Connections.find( id.value );
		if ( it == m_Connections.end() || it->second.state == State::Closed )
			return;
		if ( mode == CloseMode::Immediate )
		{
			it->second.output.clear();
			it->second.socket.Reset();
			it->second.state = State::Closed;
		}
		else
		{
			::shutdown( it->second.socket.Get(), SHUT_RD );
			it->second.state = State::Flushing;
		}
	}

	void Shutdown() override
	{
		if ( m_Shutdown )
			return;
		m_Shutdown = true;
		for ( auto &[id, connection] : m_Connections )
		{
			if ( connection.state != State::Closed && !connection.peerClosed )
				Flush( connection );
		}
		m_Connections.clear();
		m_Listener.Reset();

		struct stat current;
		if ( ::lstat( m_Path.c_str(), &current ) == 0 && S_ISSOCK( current.st_mode ) &&
		     current.st_dev == m_Device && current.st_ino == m_Inode )
			::unlink( m_Path.c_str() );
	}

private:
	enum class State
	{
		Open,
		Flushing,
		Closed,
	};

	struct Connection
	{
		ScopedFd socket;
		std::string output;
		State state = State::Open;
		bool peerClosed = false;
	};

	void AcceptPending( ITransportSink &sink )
	{
		for ( ;; )
		{
			ScopedFd accepted( ::accept( m_Listener.Get(), nullptr, nullptr ) );
			if ( accepted.Get() < 0 )
				return; // EAGAIN, or a transient accept failure retried next Poll.
			if ( m_Connections.size() >= m_Limits.maxConnections ||
			     !ConfigureSocket( accepted.Get() ) )
				continue; // closed by ScopedFd
			const uint32_t id = m_NextId++;
			m_Connections.emplace( id, Connection{ std::move( accepted ), std::string(),
			                               State::Open, false } );
			sink.OnConnected( ConnectionId{ id } );
			if ( m_Shutdown )
				return;
		}
	}

	std::string ReadAvailable( Connection &connection )
	{
		std::string bytes;
		char buffer[64 * 1024];
		while ( bytes.size() < m_Limits.maxReadBytesPerPoll )
		{
			const ssize_t count = ::recv( connection.socket.Get(), buffer, sizeof( buffer ), 0 );
			if ( count > 0 )
			{
				bytes.append( buffer, static_cast<size_t>( count ) );
				continue;
			}
			if ( count == 0 )
				connection.peerClosed = true;
			else if ( errno == EINTR )
				continue;
			else if ( errno != EAGAIN && errno != EWOULDBLOCK )
				connection.peerClosed = true;
			break;
		}
		return bytes;
	}

	void Flush( Connection &connection )
	{
		size_t written = 0;
		while ( written < connection.output.size() )
		{
			const ssize_t count = ::send( connection.socket.Get(), connection.output.data() + written,
			    connection.output.size() - written, kSendFlags );
			if ( count > 0 )
			{
				written += static_cast<size_t>( count );
				continue;
			}
			if ( count < 0 && errno == EINTR )
				continue;
			if ( count < 0 && errno != EAGAIN && errno != EWOULDBLOCK )
			{
				connection.peerClosed = true;
				connection.output.clear();
				return;
			}
			break;
		}
		connection.output.erase( 0, written );
	}

	std::string m_Path;
	std::string m_Description;
	ScopedFd m_Listener;
	dev_t m_Device;
	ino_t m_Inode;
	UnixSocketLimits m_Limits;
	std::map<uint32_t, Connection> m_Connections;
	uint32_t m_NextId = 1;
	bool m_Shutdown = false;
};

// Returns true when something accepts connections on the socket at `address`.
bool SocketIsLive( const sockaddr_un &address )
{
	ScopedFd probe( ::socket( AF_UNIX, SOCK_STREAM, 0 ) );
	if ( probe.Get() < 0 )
		return false;
	return ::connect( probe.Get(), reinterpret_cast<const sockaddr *>( &address ),
	           sizeof( address ) ) == 0;
}

TransportResult CreateUnixSocket( std::string_view address )
{
	return CreateUnixSocketTransport( address );
}

} // namespace

TransportResult CreateUnixSocketTransport( std::string_view path, UnixSocketLimits limits )
{
	sockaddr_un address{};
	address.sun_family = AF_UNIX;
	if ( path.empty() || path.front() != '/' || path.size() >= sizeof( address.sun_path ) ||
	     path.find( '\0' ) != std::string_view::npos )
		return foundation::MakeUnexpected( Failure( TransportErrorCode::InvalidAddress,
		    "unix socket path must be absolute and shorter than " +
		        std::to_string( sizeof( address.sun_path ) ) + " bytes" ) );
	if ( limits.maxConnections == 0 || limits.maxReadBytesPerPoll == 0 )
		return foundation::MakeUnexpected(
		    Failure( TransportErrorCode::InvalidAddress, "unix socket limits must be nonzero" ) );
	std::memcpy( address.sun_path, path.data(), path.size() );
	const std::string pathText( path );

	struct stat existing;
	if ( ::lstat( pathText.c_str(), &existing ) == 0 )
	{
		if ( !S_ISSOCK( existing.st_mode ) )
			return foundation::MakeUnexpected( Failure( TransportErrorCode::InvalidAddress,
			    "refusing to replace non-socket file " + pathText ) );
		if ( SocketIsLive( address ) )
			return foundation::MakeUnexpected( Failure(
			    TransportErrorCode::AddressInUse, "a server already listens on " + pathText ) );
		if ( ::unlink( pathText.c_str() ) != 0 )
			return foundation::MakeUnexpected( Failure( TransportErrorCode::PermissionDenied,
			    ErrnoText( "cannot remove stale socket", errno ) ) );
	}
	else if ( errno != ENOENT )
	{
		return foundation::MakeUnexpected(
		    Failure( TransportErrorCode::SystemError, ErrnoText( "lstat", errno ) ) );
	}

	ScopedFd listener( ::socket( AF_UNIX, SOCK_STREAM, 0 ) );
	if ( listener.Get() < 0 || !ConfigureSocket( listener.Get() ) )
		return foundation::MakeUnexpected(
		    Failure( TransportErrorCode::SystemError, ErrnoText( "socket", errno ) ) );
	if ( ::bind( listener.Get(), reinterpret_cast<const sockaddr *>( &address ),
	         sizeof( address ) ) != 0 )
	{
		const int error = errno;
		const TransportErrorCode code = error == EADDRINUSE ? TransportErrorCode::AddressInUse
		                                : error == EACCES   ? TransportErrorCode::PermissionDenied
		                                                    : TransportErrorCode::SystemError;
		return foundation::MakeUnexpected( Failure( code, ErrnoText( "bind", error ) ) );
	}

	struct stat created;
	if ( ::chmod( pathText.c_str(), S_IRUSR | S_IWUSR ) != 0 ||
	     ::lstat( pathText.c_str(), &created ) != 0 || ::listen( listener.Get(), 8 ) != 0 )
	{
		const int error = errno;
		::unlink( pathText.c_str() );
		return foundation::MakeUnexpected(
		    Failure( TransportErrorCode::SystemError, ErrnoText( "configure listening socket", error ) ) );
	}

	return std::unique_ptr<ITransport>( new UnixSocketTransport(
	    pathText, std::move( listener ), created.st_dev, created.st_ino, limits ) );
}

const TransportProvider &UnixSocketTransportProvider()
{
	static const TransportProvider provider{ "unix", &CreateUnixSocket };
	return provider;
}

} // namespace debugapi
