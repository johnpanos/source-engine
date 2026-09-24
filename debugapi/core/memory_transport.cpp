//========= Copyright Valve Corporation, All rights reserved. ============//
//
// In-process debugapi.transport.v1 provider.
//
//=============================================================================//

#include "memory_transport.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace debugapi
{

struct MemoryChannel
{
	std::string toServer;
	std::string toPeer;
	size_t receiveLimit = std::numeric_limits<size_t>::max();
	bool peerClosed = false;
	bool serverClosed = false;
};

MemoryPeer::MemoryPeer( std::shared_ptr<MemoryChannel> channel ) : m_Channel( std::move( channel ) )
{
}

MemoryPeer &MemoryPeer::operator=( MemoryPeer &&other ) noexcept
{
	if ( this != &other )
	{
		Close();
		m_Channel = std::move( other.m_Channel );
	}
	return *this;
}

MemoryPeer::~MemoryPeer()
{
	Close();
}

bool MemoryPeer::Write( std::string_view bytes )
{
	if ( !m_Channel || m_Channel->peerClosed || m_Channel->serverClosed )
		return false;
	m_Channel->toServer.append( bytes );
	return true;
}

std::string MemoryPeer::Read()
{
	if ( !m_Channel )
		return std::string();
	return std::exchange( m_Channel->toPeer, std::string() );
}

bool MemoryPeer::ServerClosed() const
{
	return m_Channel && m_Channel->serverClosed;
}

void MemoryPeer::Close()
{
	if ( m_Channel )
		m_Channel->peerClosed = true;
}

void MemoryPeer::SetReceiveLimit( size_t bytesPerPoll )
{
	if ( m_Channel )
		m_Channel->receiveLimit = bytesPerPoll;
}

MemoryTransport::MemoryTransport( std::string description ) : m_Description( std::move( description ) )
{
}

MemoryTransport::~MemoryTransport()
{
	Shutdown();
}

MemoryPeer MemoryTransport::Connect()
{
	if ( m_Shutdown )
		return MemoryPeer();
	auto channel = std::make_shared<MemoryChannel>();
	m_Incoming.push_back( channel );
	return MemoryPeer( std::move( channel ) );
}

void MemoryTransport::Flush( Connection &connection, bool ignoreLimit )
{
	MemoryChannel &channel = *connection.channel;
	if ( channel.peerClosed )
	{
		connection.output.clear();
		return;
	}
	const size_t budget = ignoreLimit ? connection.output.size() : channel.receiveLimit;
	const size_t count = std::min( budget, connection.output.size() );
	channel.toPeer.append( connection.output, 0, count );
	connection.output.erase( 0, count );
}

void MemoryTransport::Poll( ITransportSink &sink )
{
	if ( m_Shutdown )
		return;

	std::vector<std::shared_ptr<MemoryChannel>> incoming = std::exchange( m_Incoming, {} );
	for ( auto &channel : incoming )
	{
		const uint32_t id = m_NextId++;
		m_Connections.emplace( id, Connection{ std::move( channel ), std::string(), State::Open } );
		sink.OnConnected( ConnectionId{ id } );
		if ( m_Shutdown )
			return;
	}

	// Callbacks may Send/Close any connection but never add or erase entries;
	// erasure happens here, after the entry's final callback.
	for ( auto it = m_Connections.begin(); it != m_Connections.end(); )
	{
		const ConnectionId id{ it->first };
		Connection &connection = it->second;
		MemoryChannel &channel = *connection.channel;

		if ( connection.state == State::Open && !channel.toServer.empty() )
		{
			std::string bytes = std::exchange( channel.toServer, std::string() );
			sink.OnData( id, bytes );
			if ( m_Shutdown )
				return;
		}

		bool finished = false;
		if ( connection.state == State::Closed )
			finished = true;
		else
		{
			Flush( connection, false );
			if ( channel.peerClosed )
				finished = true;
			else if ( connection.state == State::Flushing && connection.output.empty() )
				finished = true;
		}

		if ( !finished )
		{
			++it;
			continue;
		}
		channel.serverClosed = true;
		it = m_Connections.erase( it );
		sink.OnDisconnected( id );
		if ( m_Shutdown )
			return;
	}
}

bool MemoryTransport::Send( ConnectionId id, std::string_view bytes )
{
	auto it = m_Connections.find( id.value );
	if ( m_Shutdown || it == m_Connections.end() || it->second.state != State::Open )
		return false;
	it->second.output.append( bytes );
	return true;
}

size_t MemoryTransport::PendingOutputBytes( ConnectionId id ) const
{
	auto it = m_Connections.find( id.value );
	return it == m_Connections.end() ? 0 : it->second.output.size();
}

void MemoryTransport::Close( ConnectionId id, CloseMode mode )
{
	auto it = m_Connections.find( id.value );
	if ( it == m_Connections.end() || it->second.state == State::Closed )
		return;
	if ( mode == CloseMode::Immediate )
	{
		it->second.output.clear();
		it->second.state = State::Closed;
	}
	else
	{
		it->second.state = State::Flushing;
	}
}

void MemoryTransport::Shutdown()
{
	if ( m_Shutdown )
		return;
	m_Shutdown = true;
	for ( auto &[id, connection] : m_Connections )
	{
		if ( connection.state != State::Closed )
			Flush( connection, true );
		connection.channel->serverClosed = true;
	}
	m_Connections.clear();
	for ( auto &channel : m_Incoming )
		channel->serverClosed = true;
	m_Incoming.clear();
}

} // namespace debugapi
