//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: the shared debugapi.transport.v1 suite.
//
//===========================================================================//

#include "transport_conformance.h"

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <vector>

namespace debugapitest
{
namespace
{

using debugapi::CloseMode;
using debugapi::ConnectionId;
using debugapi::ITransport;

constexpr int kMaxPolls = 20000;

std::string Pattern( size_t size, unsigned seed )
{
	std::string text( size, '\0' );
	unsigned state = seed * 2654435761u + 1;
	for ( char &c : text )
	{
		state = state * 1664525u + 1013904223u;
		c = char( state >> 24 );
	}
	return text;
}

// Records every callback and flags any delivered outside Poll().
class RecordingSink final : public debugapi::ITransportSink
{
public:
	struct Connection
	{
		int connected = 0;
		int disconnected = 0;
		std::string data;
		bool dataAfterDisconnect = false;
	};

	void OnConnected( ConnectionId id ) override
	{
		Note();
		order.push_back( id );
		connections[id.value].connected++;
		if ( id.value == 0 )
			zeroId = true;
	}

	void OnData( ConnectionId id, std::string_view bytes ) override
	{
		Note();
		Connection &connection = connections[id.value];
		if ( connection.disconnected != 0 || connection.connected == 0 )
			connection.dataAfterDisconnect = true;
		connection.data.append( bytes );
		if ( onData )
			onData( id, bytes );
	}

	void OnDisconnected( ConnectionId id ) override
	{
		Note();
		connections[id.value].disconnected++;
	}

	void Poll( ITransport &transport )
	{
		inPoll = true;
		transport.Poll( *this );
		inPoll = false;
	}

	std::map<uint32_t, Connection> connections;
	std::vector<ConnectionId> order;
	std::function<void( ConnectionId, std::string_view )> onData;
	bool callbackOutsidePoll = false;
	bool zeroId = false;
	int callbacks = 0;

private:
	void Note()
	{
		++callbacks;
		if ( !inPoll )
			callbackOutsidePoll = true;
	}

	bool inPoll = false;
};

template <typename Predicate>
bool PollUntil( RecordingSink &sink, ITransport &transport, Predicate done )
{
	for ( int i = 0; i < kMaxPolls; ++i )
	{
		if ( done() )
			return true;
		sink.Poll( transport );
	}
	return done();
}

ConnectionId Accept( RecordingSink &sink, ITransport &transport )
{
	const size_t before = sink.order.size();
	PollUntil( sink, transport, [&] { return sink.order.size() > before; } );
	return sink.order.size() > before ? sink.order.back() : ConnectionId{};
}

void CaseConnect( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	auto peer = fixture.Connect( *transport );
	const ConnectionId id = Accept( sink, *transport );
	results.Check( id.value != 0, "a connection is announced with a nonzero id" );
	sink.Poll( *transport );
	results.Check( sink.connections[id.value].connected == 1, "OnConnected is delivered once" );
	results.Check( transport->PendingOutputBytes( ConnectionId{ 0xFFFFFFF0u } ) == 0,
	    "PendingOutputBytes is 0 for an unknown connection" );
	results.Check( !transport->Send( ConnectionId{ 0xFFFFFFF0u }, "x" ),
	    "Send to an unknown connection fails" );
	results.Check( !transport->Describe().empty(), "Describe() names the endpoint" );
	results.Check( !sink.callbackOutsidePoll, "no callback outside Poll()" );
}

void CaseUniqueIds( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	std::vector<std::unique_ptr<ITestPeer>> peers;
	std::set<uint32_t> seen;
	for ( int i = 0; i < 3; ++i )
	{
		peers.push_back( fixture.Connect( *transport ) );
		seen.insert( Accept( sink, *transport ).value );
	}
	results.Check( seen.size() == 3 && !seen.count( 0 ), "concurrent connections get distinct ids" );

	const uint32_t first = sink.order.front().value;
	peers.front()->Close();
	PollUntil( sink, *transport, [&] { return sink.connections[first].disconnected != 0; } );
	peers.push_back( fixture.Connect( *transport ) );
	const ConnectionId later = Accept( sink, *transport );
	results.Check( later.value != 0 && !seen.count( later.value ), "ids are never reused" );
}

void CaseInboundOrder( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	auto peer = fixture.Connect( *transport );
	const ConnectionId id = Accept( sink, *transport );
	const std::string big = Pattern( 3 * 1024 * 1024 + 17, 1 );
	std::string expected = "hello, " + std::string( "world" ) + big;

	peer->Write( "hello, " );
	peer->Write( "world" );
	size_t offset = 0;
	PollUntil( sink, *transport, [&] {
		// Feed the large payload in slices so a peer's buffer never blocks.
		if ( offset < big.size() )
		{
			const size_t slice = std::min<size_t>( 65536, big.size() - offset );
			if ( peer->Write( std::string_view( big ).substr( offset, slice ) ) )
				offset += slice;
		}
		peer->Read(); // lets a buffering peer flush its own queued writes
		return sink.connections[id.value].data.size() >= expected.size();
	} );
	results.Check( sink.connections[id.value].data == expected,
	    "inbound bytes arrive in order without loss or duplication" );
}

void CaseOutboundOrder( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	auto peer = fixture.Connect( *transport );
	const ConnectionId id = Accept( sink, *transport );
	const std::string big = Pattern( 4 * 1024 * 1024 + 5, 2 );
	bool sent = transport->Send( id, "first|" );
	sent = transport->Send( id, big ) && sent;
	sent = transport->Send( id, "|last" ) && sent;
	results.Check( sent, "Send accepts bytes for an open connection" );

	const std::string expected = "first|" + big + "|last";
	std::string received;
	PollUntil( sink, *transport, [&] {
		received += peer->Read();
		return received.size() >= expected.size();
	} );
	results.Check( received == expected, "outbound bytes arrive in order without loss" );
	results.Check( transport->PendingOutputBytes( id ) == 0, "queued output drains to zero" );
	results.Check( !sink.callbackOutsidePoll, "no callback outside Poll()" );
}

void CasePeerClose( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	auto peer = fixture.Connect( *transport );
	const ConnectionId id = Accept( sink, *transport );
	peer->Write( "tail" );
	peer->Close();
	PollUntil( sink, *transport, [&] { return sink.connections[id.value].disconnected != 0; } );
	for ( int i = 0; i < 5; ++i )
		sink.Poll( *transport );
	const auto &connection = sink.connections[id.value];
	results.Check( connection.data == "tail", "bytes written before a peer close are delivered" );
	results.Check( connection.disconnected == 1, "a peer close is delivered exactly once" );
	results.Check( !connection.dataAfterDisconnect, "no data after OnDisconnected" );
	results.Check( !transport->Send( id, "late" ), "Send after disconnect fails" );
}

void CaseCloseAfterFlush( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	auto peer = fixture.Connect( *transport );
	const ConnectionId id = Accept( sink, *transport );
	const std::string payload = Pattern( 2 * 1024 * 1024 + 3, 3 );
	results.Check( transport->Send( id, payload ), "Send before close succeeds" );
	const int before = sink.callbacks;
	transport->Close( id, CloseMode::AfterFlush );
	results.Check( sink.callbacks == before, "Close() delivers no callback re-entrantly" );
	results.Check( !transport->Send( id, "x" ), "Send after Close(AfterFlush) fails" );

	std::string received;
	PollUntil( sink, *transport, [&] {
		received += peer->Read();
		return peer->SawClose() && sink.connections[id.value].disconnected != 0;
	} );
	received += peer->Read();
	for ( int i = 0; i < 5; ++i )
		sink.Poll( *transport );
	results.Check( received == payload, "Close(AfterFlush) delivers all queued output first" );
	results.Check( peer->SawClose(), "the peer observes the close" );
	results.Check( sink.connections[id.value].disconnected == 1, "OnDisconnected exactly once" );
}

void CaseCloseImmediate( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	auto peer = fixture.Connect( *transport );
	const ConnectionId id = Accept( sink, *transport );
	const int before = sink.callbacks;
	transport->Close( id, CloseMode::Immediate );
	results.Check( sink.callbacks == before, "Close(Immediate) delivers no callback re-entrantly" );
	results.Check( !transport->Send( id, "x" ), "Send after Close(Immediate) fails" );
	transport->Close( id, CloseMode::Immediate );
	PollUntil( sink, *transport, [&] {
		peer->Read();
		return peer->SawClose() && sink.connections[id.value].disconnected != 0;
	} );
	for ( int i = 0; i < 5; ++i )
		sink.Poll( *transport );
	results.Check( sink.connections[id.value].disconnected == 1,
	    "OnDisconnected exactly once, even after a repeated Close" );
	results.Check( peer->SawClose(), "the peer observes an immediate close" );
}

void CaseReentrantSink( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	auto echo = fixture.Connect( *transport );
	const ConnectionId echoId = Accept( sink, *transport );
	auto victim = fixture.Connect( *transport );
	const ConnectionId victimId = Accept( sink, *transport );
	bool sendOk = true;
	sink.onData = [&]( ConnectionId id, std::string_view bytes ) {
		if ( id != echoId )
			return;
		sendOk = transport->Send( id, bytes ) && sendOk;
		if ( bytes.find( "close" ) != std::string_view::npos )
			transport->Close( victimId, CloseMode::Immediate );
	};
	echo->Write( "ping close" );
	std::string received;
	PollUntil( sink, *transport, [&] {
		received += echo->Read();
		return received.size() >= 10 && sink.connections[victimId.value].disconnected != 0;
	} );
	results.Check( sendOk && received == "ping close", "Send from inside OnData works" );
	results.Check( sink.connections[victimId.value].disconnected == 1,
	    "Close of another connection from inside OnData is honoured once" );
	PollUntil( sink, *transport, [&] {
		victim->Read();
		return victim->SawClose();
	} );
	results.Check( victim->SawClose(), "the other peer observes the close" );
}

void CaseShutdown( ITransportFixture &fixture, Results &results )
{
	auto transport = fixture.Create();
	if ( !transport )
	{
		results.Check( false, "the fixture creates a transport" );
		return;
	}
	RecordingSink sink;
	auto first = fixture.Connect( *transport );
	const ConnectionId firstId = Accept( sink, *transport );
	auto second = fixture.Connect( *transport );
	Accept( sink, *transport );
	results.Check( transport->Send( firstId, "bye" ), "Send before Shutdown succeeds" );

	const int before = sink.callbacks;
	transport->Shutdown();
	transport->Shutdown();
	sink.Poll( *transport );
	results.Check( sink.callbacks == before, "Shutdown delivers no callbacks, now or later" );
	results.Check( !transport->Send( firstId, "x" ), "Send after Shutdown fails" );

	std::string received;
	for ( int i = 0; i < 100 && !( first->SawClose() && second->SawClose() ); ++i )
	{
		received += first->Read();
		second->Read();
	}
	received += first->Read();
	results.Check( received == "bye", "Shutdown flushes small queued output once" );
	results.Check( first->SawClose() && second->SawClose(), "every peer observes Shutdown" );
	transport.reset();
	results.Check( !sink.callbackOutsidePoll, "no callback outside Poll()" );
}

} // namespace

void RunTransportConformance( ITransportFixture &fixture, Results &results )
{
	const std::string previous = results.scope;
	results.scope = fixture.Name();
	CaseConnect( fixture, results );
	CaseUniqueIds( fixture, results );
	CaseInboundOrder( fixture, results );
	CaseOutboundOrder( fixture, results );
	CasePeerClose( fixture, results );
	CaseCloseAfterFlush( fixture, results );
	CaseCloseImmediate( fixture, results );
	CaseReentrantSink( fixture, results );
	CaseShutdown( fixture, results );
	results.scope = previous;
}

} // namespace debugapitest
