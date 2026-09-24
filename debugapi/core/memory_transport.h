//========= Copyright Valve Corporation, All rights reserved. ============//
//
// In-process debugapi.transport.v1 provider. Tests and in-process tools use a
// MemoryPeer as the client end. The channel is shared by the transport and the
// peer (std::shared_ptr): either side may be destroyed first, and a destroyed
// peer reads as a peer close. Single sequence, like every transport.
//
//=============================================================================//

#ifndef DEBUGAPI_CORE_MEMORY_TRANSPORT_H
#define DEBUGAPI_CORE_MEMORY_TRANSPORT_H

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "debugapi/debugapi_transport.h"

namespace debugapi
{

struct MemoryChannel;

// Client end of one in-memory connection.
class MemoryPeer
{
public:
	MemoryPeer() = default;
	MemoryPeer( MemoryPeer && ) noexcept = default;
	MemoryPeer &operator=( MemoryPeer && ) noexcept;
	~MemoryPeer();

	bool IsValid() const { return m_Channel != nullptr; }
	// Queues bytes for the server; false after either side closed.
	bool Write( std::string_view bytes );
	// Takes every byte the transport has delivered so far.
	std::string Read();
	// The server closed the connection (after delivering everything it wrote).
	bool ServerClosed() const;
	void Close();
	// Caps the bytes the transport may deliver to this peer per Poll(), to
	// model a slow reader. 0 blocks delivery entirely.
	void SetReceiveLimit( size_t bytesPerPoll );

private:
	friend class MemoryTransport;
	explicit MemoryPeer( std::shared_ptr<MemoryChannel> channel );

	std::shared_ptr<MemoryChannel> m_Channel;
};

class MemoryTransport final : public ITransport
{
public:
	explicit MemoryTransport( std::string description = "memory:" );
	~MemoryTransport() override;

	// Creates a connection; OnConnected is delivered on the next Poll().
	// Fails (invalid peer) after Shutdown().
	MemoryPeer Connect();

	std::string_view Describe() const override { return m_Description; }
	void Poll( ITransportSink &sink ) override;
	bool Send( ConnectionId id, std::string_view bytes ) override;
	size_t PendingOutputBytes( ConnectionId id ) const override;
	void Close( ConnectionId id, CloseMode mode ) override;
	void Shutdown() override;

private:
	enum class State
	{
		Open,
		Flushing,
		Closed,
	};

	struct Connection
	{
		std::shared_ptr<MemoryChannel> channel;
		std::string output;
		State state = State::Open;
	};

	void Flush( Connection &connection, bool ignoreLimit );

	std::string m_Description;
	std::vector<std::shared_ptr<MemoryChannel>> m_Incoming;
	std::map<uint32_t, Connection> m_Connections;
	uint32_t m_NextId = 1;
	bool m_Shutdown = false;
};

} // namespace debugapi

#endif // DEBUGAPI_CORE_MEMORY_TRANSPORT_H
