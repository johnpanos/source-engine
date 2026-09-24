//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The debug API server: JSON-RPC 2.0 over a selected framing and transport,
// executing against an IDebugApiHost. Everything runs on the host's main
// sequence from Pump(); console text may arrive from any thread through the
// internal LogCollector.
//
// Ownership and lifetime: the composition root owns the server; the host must
// outlive it. Shutdown() (implied by the destructor) answers every deferred
// call with CANCELLED, removes the log sink and shuts the transport down.
//
// Bounds: maxConnections (excess connections get one error and are closed),
// maxPendingPerConnection deferred calls, maxOutputBytesPerConnection queued
// bytes. Log notifications over the output bound are dropped and counted;
// a response over the bound closes that connection instead.
//
//=============================================================================//

#ifndef DEBUGAPI_CORE_DEBUGAPI_SERVER_H
#define DEBUGAPI_CORE_DEBUGAPI_SERVER_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "debugapi/debugapi_framing.h"
#include "debugapi/debugapi_host.h"
#include "debugapi/debugapi_service.h"
#include "debugapi/debugapi_transport.h"
#include "dispatcher.h"
#include "jsonrpc.h"
#include "log_collector.h"

namespace debugapi
{

struct ServerLimits
{
	size_t maxConnections = 4;
	size_t maxPendingPerConnection = 64;
	size_t maxOutputBytesPerConnection = 64 * 1024 * 1024;
	size_t maxFrameBytes = 1024 * 1024;
	size_t maxBatch = 64;
	size_t logCollectorBytes = 4 * 1024 * 1024;
	DispatcherLimits dispatcher;
};

class Server final : public IDebugApiService, private ITransportSink
{
public:
	static foundation::Expected<std::unique_ptr<Server>, std::string> Create(
	    std::unique_ptr<ITransport> transport, const FramingProvider &framing, IDebugApiHost &host,
	    ServerLimits limits = {} );
	~Server() override;

	Server( const Server & ) = delete;
	Server &operator=( const Server & ) = delete;

	// One server step: advances deferred calls, services the transport and
	// forwards collected console text. Re-entrant calls return immediately.
	void Pump() override;
	void Shutdown() override;
	size_t ConnectionCount() const { return m_Connections.size(); }

private:
	struct Batch
	{
		size_t outstanding = 0;
		std::vector<std::string> members;
	};

	struct Connection
	{
		std::unique_ptr<IFrameDecoder> decoder;
		bool closing = false;
		bool subscribed = false;
		uint32_t nextSeq = 1;
		uint32_t dropped = 0;
		size_t pending = 0;
		std::map<uint64_t, Batch> batches;
	};

	struct Pending
	{
		ConnectionId connection;
		RequestId id;
		uint64_t batch = 0;
	};

	Server( std::unique_ptr<ITransport> transport, const FramingProvider &framing,
	    IDebugApiHost &host, ServerLimits limits );

	void OnConnected( ConnectionId id ) override;
	void OnData( ConnectionId id, std::string_view bytes ) override;
	void OnDisconnected( ConnectionId id ) override;

	void HandleFrame( ConnectionId id, Connection &connection, std::string_view frame );
	// Returns true when deferred; otherwise `response` holds the reply ("" for
	// a notification).
	bool HandleCall( ConnectionId id, Connection &connection, DecodedCall &call, uint64_t batch,
	    std::string &response );
	std::string Encode( const RequestId &id, const CallOutcome &outcome ) const;
	void Complete( std::vector<Completion> &completions );
	void SendPayload( ConnectionId id, std::string_view payload );
	void DrainLogs();
	void UpdateLogSink();

	std::unique_ptr<ITransport> m_Transport;
	const FramingProvider &m_Framing;
	IDebugApiHost &m_Host;
	ServerLimits m_Limits;
	Dispatcher m_Dispatcher;
	LogCollector m_Logs;
	std::map<ConnectionId, Connection> m_Connections;
	std::map<CallTag, Pending> m_Pending;
	CallTag m_NextTag = 1;
	uint64_t m_NextBatch = 1;
	bool m_LogSinkInstalled = false;
	bool m_InPump = false;
	bool m_Shutdown = false;
};

} // namespace debugapi

#endif // DEBUGAPI_CORE_DEBUGAPI_SERVER_H
