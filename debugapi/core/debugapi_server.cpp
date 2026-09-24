//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The debug API server.
//
//=============================================================================//

#include "debugapi_server.h"

#include <utility>

namespace debugapi
{

foundation::Expected<std::unique_ptr<Server>, std::string> Server::Create(
    std::unique_ptr<ITransport> transport, const FramingProvider &framing, IDebugApiHost &host,
    ServerLimits limits )
{
	if ( !transport )
		return foundation::MakeUnexpected( std::string( "no transport" ) );
	if ( !framing.name || !framing.createDecoder || !framing.encode )
		return foundation::MakeUnexpected( std::string( "incomplete framing provider" ) );
	if ( !SchemaMatchesMethods() )
		return foundation::MakeUnexpected(
		    std::string( "generated schema does not match the dispatcher's methods" ) );
	if ( limits.maxConnections == 0 || limits.maxPendingPerConnection == 0 ||
	     limits.maxFrameBytes == 0 || limits.maxBatch == 0 )
		return foundation::MakeUnexpected( std::string( "server limits must be nonzero" ) );
	return std::unique_ptr<Server>( new Server( std::move( transport ), framing, host, limits ) );
}

Server::Server( std::unique_ptr<ITransport> transport, const FramingProvider &framing,
    IDebugApiHost &host, ServerLimits limits )
    : m_Transport( std::move( transport ) ), m_Framing( framing ), m_Host( host ),
      m_Limits( limits ),
      m_Dispatcher( host, ServerIdentity{ framing.name, std::string( m_Transport->Describe() ) },
          limits.dispatcher ),
      m_Logs( limits.logCollectorBytes )
{
}

Server::~Server()
{
	Shutdown();
}

void Server::Pump()
{
	if ( m_InPump || m_Shutdown )
		return;
	m_InPump = true;
	std::vector<Completion> completions;
	m_Dispatcher.Tick( completions );
	Complete( completions );
	m_Transport->Poll( *this );
	DrainLogs();
	m_InPump = false;
}

void Server::Shutdown()
{
	if ( m_Shutdown )
		return;
	std::vector<Completion> completions;
	m_Dispatcher.CancelAll( completions );
	Complete( completions );
	m_Shutdown = true;
	if ( m_LogSinkInstalled )
	{
		m_Host.SetLogSink( nullptr );
		m_LogSinkInstalled = false;
	}
	m_Transport->Shutdown();
	m_Connections.clear();
	m_Pending.clear();
}

void Server::OnConnected( ConnectionId id )
{
	Connection connection;
	connection.decoder = m_Framing.createDecoder( m_Limits.maxFrameBytes );
	auto [it, inserted] = m_Connections.emplace( id, std::move( connection ) );
	if ( m_Connections.size() > m_Limits.maxConnections )
	{
		SendPayload( id, EncodeError( RequestId{ "null" },
		                     RpcError{ pb::ERROR_CODE_RESOURCE_EXHAUSTED,
		                         "too many debug API connections" } ) );
		it->second.closing = true;
		m_Transport->Close( id, CloseMode::AfterFlush );
	}
}

void Server::OnData( ConnectionId id, std::string_view bytes )
{
	auto it = m_Connections.find( id );
	if ( it == m_Connections.end() || it->second.closing )
		return;
	std::vector<std::string> frames;
	std::string error;
	const FrameStatus status = it->second.decoder->Feed( bytes, frames, error );
	for ( const std::string &frame : frames )
	{
		// A response over the output bound closes the connection mid-loop.
		if ( it->second.closing )
			return;
		HandleFrame( id, it->second, frame );
	}
	if ( status == FrameStatus::Fatal && !it->second.closing )
	{
		SendPayload( id, EncodeError( RequestId{ "null" },
		                     RpcError{ pb::ERROR_CODE_PARSE_ERROR, "framing error: " + error } ) );
		it->second.closing = true;
		m_Transport->Close( id, CloseMode::AfterFlush );
	}
}

void Server::OnDisconnected( ConnectionId id )
{
	for ( auto it = m_Pending.begin(); it != m_Pending.end(); )
	{
		if ( it->second.connection != id )
		{
			++it;
			continue;
		}
		m_Dispatcher.Cancel( it->first );
		it = m_Pending.erase( it );
	}
	m_Connections.erase( id );
	UpdateLogSink();
}

void Server::HandleFrame( ConnectionId id, Connection &connection, std::string_view frame )
{
	DecodedFrame decoded = DecodeFrame( frame, JsonRpcLimits{ m_Limits.maxBatch } );
	if ( decoded.error )
	{
		SendPayload( id, EncodeError( RequestId{ "null" }, *decoded.error ) );
		return;
	}
	if ( !decoded.batch )
	{
		std::string response;
		if ( !HandleCall( id, connection, decoded.calls.front(), 0, response ) && !response.empty() )
			SendPayload( id, response );
		return;
	}

	const uint64_t batchId = m_NextBatch++;
	Batch batch;
	for ( DecodedCall &call : decoded.calls )
	{
		std::string response;
		if ( HandleCall( id, connection, call, batchId, response ) )
			++batch.outstanding;
		else if ( !response.empty() )
			batch.members.push_back( std::move( response ) );
	}
	if ( batch.outstanding != 0 )
		connection.batches.emplace( batchId, std::move( batch ) );
	else if ( !batch.members.empty() )
		SendPayload( id, EncodeBatch( batch.members ) );
}

bool Server::HandleCall( ConnectionId id, Connection &connection, DecodedCall &call,
    uint64_t batch, std::string &response )
{
	auto reply = [&call, &response]( const CallOutcome &outcome, const Server &server ) {
		if ( !call.id.IsNotification() )
			response = server.Encode( call.id, outcome );
	};

	if ( call.error )
	{
		CallOutcome outcome;
		outcome.error = std::move( call.error );
		reply( outcome, *this );
		return false;
	}

	if ( call.method == Method::Subscribe )
	{
		const auto &request = static_cast<const pb::SubscribeRequest &>( *call.params );
		connection.subscribed = request.log();
		UpdateLogSink();
		auto result = std::make_unique<pb::SubscribeResult>();
		result->set_log( connection.subscribed );
		CallOutcome outcome;
		outcome.result = std::move( result );
		reply( outcome, *this );
		return false;
	}

	if ( ( call.method == Method::WaitFor || call.method == Method::Screenshot ) &&
	     connection.pending >= m_Limits.maxPendingPerConnection )
	{
		CallOutcome outcome;
		outcome.error = RpcError{ pb::ERROR_CODE_RESOURCE_EXHAUSTED,
		    "too many pending calls on this connection" };
		reply( outcome, *this );
		return false;
	}

	const CallTag tag = m_NextTag++;
	std::optional<CallOutcome> outcome = m_Dispatcher.Handle( tag, call.method, *call.params );
	if ( !outcome )
	{
		m_Pending.emplace( tag, Pending{ id, std::move( call.id ), batch } );
		++connection.pending;
		return true;
	}
	reply( *outcome, *this );
	return false;
}

std::string Server::Encode( const RequestId &id, const CallOutcome &outcome ) const
{
	if ( outcome.error )
		return EncodeError( id, *outcome.error );
	return EncodeResult( id, *outcome.result );
}

void Server::Complete( std::vector<Completion> &completions )
{
	for ( Completion &completion : completions )
	{
		auto pending = m_Pending.find( completion.tag );
		if ( pending == m_Pending.end() )
			continue;
		const Pending call = std::move( pending->second );
		m_Pending.erase( pending );
		auto connection = m_Connections.find( call.connection );
		if ( connection == m_Connections.end() )
			continue;
		--connection->second.pending;
		std::string response =
		    call.id.IsNotification() ? std::string() : Encode( call.id, completion.outcome );

		if ( call.batch == 0 )
		{
			if ( !response.empty() )
				SendPayload( call.connection, response );
			continue;
		}
		auto batch = connection->second.batches.find( call.batch );
		if ( batch == connection->second.batches.end() )
			continue;
		if ( !response.empty() )
			batch->second.members.push_back( std::move( response ) );
		if ( --batch->second.outstanding != 0 )
			continue;
		std::vector<std::string> members = std::move( batch->second.members );
		connection->second.batches.erase( batch );
		if ( !members.empty() )
			SendPayload( call.connection, EncodeBatch( members ) );
	}
}

void Server::SendPayload( ConnectionId id, std::string_view payload )
{
	auto connection = m_Connections.find( id );
	if ( connection == m_Connections.end() )
		return;
	std::string framed;
	m_Framing.encode( payload, framed );
	if ( m_Transport->PendingOutputBytes( id ) + framed.size() >
	     m_Limits.maxOutputBytesPerConnection )
	{
		// Responses are never dropped: a client that cannot keep up loses the
		// connection (its OnDisconnected cancels the rest of its calls).
		connection->second.closing = true;
		m_Transport->Close( id, CloseMode::Immediate );
		return;
	}
	(void)m_Transport->Send( id, framed );
}

void Server::DrainLogs()
{
	if ( !m_LogSinkInstalled )
		return;
	std::vector<LogLine> lines;
	uint32_t dropped = 0;
	m_Logs.Drain( lines, dropped );
	for ( auto &[id, connection] : m_Connections )
	{
		if ( !connection.subscribed || connection.closing )
			continue;
		connection.dropped += dropped;
		for ( const LogLine &line : lines )
		{
			pb::LogNotification notification;
			notification.set_seq( connection.nextSeq );
			notification.set_dropped_before( connection.dropped );
			notification.set_text( SanitizeUtf8( line.text ) );
			notification.set_developer( line.developer );
			notification.set_rgba( line.rgba );
			std::string framed;
			m_Framing.encode( EncodeLogNotification( notification ), framed );
			if ( m_Transport->PendingOutputBytes( id ) + framed.size() >
			         m_Limits.maxOutputBytesPerConnection ||
			     !m_Transport->Send( id, framed ) )
			{
				++connection.dropped;
				continue;
			}
			++connection.nextSeq;
			connection.dropped = 0;
		}
	}
}

void Server::UpdateLogSink()
{
	bool wanted = false;
	for ( const auto &[id, connection] : m_Connections )
		wanted = wanted || connection.subscribed;
	if ( wanted == m_LogSinkInstalled || m_Shutdown )
		return;
	m_Host.SetLogSink( wanted ? static_cast<ILogSink *>( &m_Logs ) : nullptr );
	m_LogSinkInstalled = wanted;
	if ( !wanted )
	{
		// Discard text collected for subscribers that are gone.
		std::vector<LogLine> lines;
		uint32_t dropped = 0;
		m_Logs.Drain( lines, dropped );
	}
}

ServiceResult StartDebugApiService( const TransportProvider &transport, std::string_view address,
    const FramingProvider &framing, IDebugApiHost &host )
{
	if ( !transport.create )
		return foundation::MakeUnexpected( std::string( "incomplete transport provider" ) );
	TransportResult created = transport.create( address );
	if ( !created )
		return foundation::MakeUnexpected( std::string( transport.scheme ? transport.scheme : "?" ) +
		                                   " transport: " + created.Error().message );
	auto server = Server::Create( std::move( created ).Value(), framing, host );
	if ( !server )
		return foundation::MakeUnexpected( server.Error() );
	return std::unique_ptr<IDebugApiService>( std::move( server ).Value() );
}

} // namespace debugapi
