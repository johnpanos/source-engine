//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: the debug API server's lifecycle and bounds (debugapi_server.h):
//          creation failures, connection/pending/output limits, log drop
//          accounting, disconnect and shutdown cancellation, framing faults,
//          re-entrant pumps, log sink install/removal and a concurrent
//          LogCollector stress (run it under TSan too).
//
//===========================================================================//

#include "conformance/checks.h"
#include "debugapi/core/debugapi_server.h"
#include "debugapi/core/memory_transport.h"
#include "fakes/fake_host.h"
#include "testing/conformance_result.h"

#include <limits>
#include <string>
#include <thread>
#include <vector>

#include <google/protobuf/struct.pb.h>
#include <google/protobuf/util/json_util.h>

namespace
{

namespace gp = ::google::protobuf;
using namespace debugapitest;
using debugapi::MemoryPeer;
using debugapi::MemoryTransport;
using debugapi::Server;
using debugapi::ServerLimits;

struct Harness
{
	FakeHost host;
	MemoryTransport *memory = nullptr;
	std::unique_ptr<Server> server;

	explicit Harness( ServerLimits limits = {} )
	{
		auto transport = std::make_unique<MemoryTransport>();
		memory = transport.get();
		auto created =
		    Server::Create( std::move( transport ), debugapi::NewlineFraming(), host, limits );
		if ( created )
			server = std::move( created ).Value();
	}

	void Pump( int times = 1 )
	{
		for ( int i = 0; i < times; ++i )
		{
			host.now += 0.015625;
			server->Pump();
		}
	}
};

std::string Request( int id, const char *method, const char *params = "{}" )
{
	return std::string( "{\"jsonrpc\":\"2.0\",\"id\":" ) + std::to_string( id ) +
	       ",\"method\":\"" + method + "\",\"params\":" + params + "}\n";
}

std::vector<gp::Value> Messages( MemoryPeer &peer )
{
	std::vector<gp::Value> messages;
	std::string text = peer.Read();
	size_t start = 0;
	for ( size_t end; ( end = text.find( '\n', start ) ) != std::string::npos; start = end + 1 )
	{
		gp::Value value;
		if ( gp::util::JsonStringToMessage( text.substr( start, end - start ), &value ).ok() )
			messages.push_back( value );
	}
	return messages;
}

const gp::Value *Get( const gp::Value &value, std::initializer_list<const char *> path )
{
	const gp::Value *current = &value;
	for ( const char *name : path )
	{
		if ( current->kind_case() != gp::Value::kStructValue )
			return nullptr;
		auto it = current->struct_value().fields().find( name );
		if ( it == current->struct_value().fields().end() )
			return nullptr;
		current = &it->second;
	}
	return current;
}

double ErrorCode( const gp::Value &message )
{
	const gp::Value *code = Get( message, { "error", "code" } );
	return code ? code->number_value() : 0.0;
}

void CreationFailures( Results &results )
{
	results.scope = "create";
	FakeHost host;
	results.Check( !Server::Create( nullptr, debugapi::NewlineFraming(), host ),
	    "a missing transport fails creation" );
	const debugapi::FramingProvider incomplete{ "broken", nullptr, nullptr };
	results.Check( !Server::Create( std::make_unique<MemoryTransport>(), incomplete, host ),
	    "an incomplete framing provider fails creation" );
	ServerLimits zero;
	zero.maxConnections = 0;
	results.Check(
	    !Server::Create( std::make_unique<MemoryTransport>(), debugapi::NewlineFraming(), host, zero ),
	    "zero limits fail creation" );
	results.Check( debugapi::SchemaMatchesMethods(), "the generated service matches the dispatcher" );
}

void ConnectionLimit( Results &results )
{
	results.scope = "connection-limit";
	ServerLimits limits;
	limits.maxConnections = 2;
	Harness h( limits );
	MemoryPeer a = h.memory->Connect();
	MemoryPeer b = h.memory->Connect();
	MemoryPeer c = h.memory->Connect();
	h.Pump( 3 );
	const auto excess = Messages( c );
	results.Check( excess.size() == 1 && ErrorCode( excess[0] ) == -32005,
	    "an excess connection gets one RESOURCE_EXHAUSTED error" );
	results.Check( c.ServerClosed(), "the excess connection is closed" );
	a.Write( Request( 1, "status" ) );
	h.Pump();
	results.Check( Messages( a ).size() == 1, "connections within the limit keep working" );
	results.Check( h.server->ConnectionCount() == 2, "the server tracks the admitted connections" );
}

void PendingLimit( Results &results )
{
	results.scope = "pending-limit";
	ServerLimits limits;
	limits.maxPendingPerConnection = 2;
	Harness h( limits );
	MemoryPeer peer = h.memory->Connect();
	const char *wait = "{\"condition\":\"WAIT_CONDITION_FRAMES\",\"frames\":1000,\"timeoutMs\":600000}";
	peer.Write( Request( 1, "waitFor", wait ) + Request( 2, "waitFor", wait ) +
	            Request( 3, "waitFor", wait ) + Request( 4, "status" ) );
	h.Pump();
	const auto replies = Messages( peer );
	results.Check( replies.size() == 2 && ErrorCode( replies[0] ) == -32005 &&
	                   Get( replies[0], { "id" } )->number_value() == 3,
	    "a deferred call over the pending limit is refused" );
	results.Check( replies.size() == 2 && Get( replies[1], { "result" } ) != nullptr,
	    "immediate calls are not limited by pending deferred calls" );
}

void OutputBound( Results &results )
{
	results.scope = "output-bound";
	ServerLimits limits;
	limits.maxOutputBytesPerConnection = 4096;
	Harness h( limits );
	MemoryPeer slow = h.memory->Connect();
	MemoryPeer other = h.memory->Connect();
	slow.SetReceiveLimit( 0 );
	slow.Write( Request( 1, "exec", "{\"command\":\"spam\"}" ) );
	h.Pump( 3 );
	results.Check( slow.ServerClosed(), "a response over the output bound closes the connection" );
	other.Write( Request( 2, "status" ) );
	h.Pump();
	results.Check( Messages( other ).size() == 1, "other connections are unaffected" );
}

void LogDropAccounting( Results &results )
{
	results.scope = "log-drop";
	ServerLimits limits;
	limits.maxOutputBytesPerConnection = 1024;
	Harness h( limits );
	MemoryPeer peer = h.memory->Connect();
	peer.Write( Request( 1, "subscribe", "{\"log\":true}" ) );
	h.Pump( 2 );
	(void)peer.Read();
	results.Check( h.host.logSink != nullptr, "subscribing installs the log sink" );

	peer.SetReceiveLimit( 0 );
	const int lines = 60;
	for ( int i = 0; i < lines; ++i )
		h.host.logSink->Append( "line " + std::to_string( i ) + " " + std::string( 40, 'x' ) + "\n",
		    false, 0 );
	h.Pump();
	peer.SetReceiveLimit( std::numeric_limits<size_t>::max() );
	h.Pump();
	h.host.logSink->Append( "final\n", true, 0x11223344 );
	h.Pump( 2 );

	unsigned delivered = 0, dropped = 0, expectedSeq = 1;
	bool sequential = true, finalSeen = false;
	for ( const gp::Value &message : Messages( peer ) )
	{
		const gp::Value *seq = Get( message, { "params", "seq" } );
		if ( !seq )
			continue;
		sequential = sequential && seq->number_value() == expectedSeq++;
		dropped += static_cast<unsigned>( Get( message, { "params", "droppedBefore" } )->number_value() );
		++delivered;
		if ( Get( message, { "params", "text" } )->string_value() == "final\n" )
			finalSeen = Get( message, { "params", "developer" } )->bool_value() &&
			            Get( message, { "params", "rgba" } )->number_value() == 0x11223344;
	}
	results.Check( dropped > 0, "notifications over the output bound are dropped" );
	results.Check( delivered + dropped == lines + 1, "every line is delivered or counted as dropped" );
	results.Check( sequential, "delivered notifications have consecutive sequence numbers" );
	results.Check( finalSeen, "later notifications resume with their attributes intact" );
}

void DisconnectCancels( Results &results )
{
	results.scope = "disconnect";
	Harness h;
	MemoryPeer peer = h.memory->Connect();
	peer.Write( Request( 1, "waitFor",
	                "{\"condition\":\"WAIT_CONDITION_FRAMES\",\"frames\":1000,\"timeoutMs\":600000}" ) +
	            Request( 2, "screenshot", "{\"mode\":\"SCREENSHOT_MODE_PATH\"}" ) +
	            Request( 3, "subscribe", "{\"log\":true}" ) );
	h.Pump();
	results.Check( h.host.logSink != nullptr, "the subscriber installed the log sink" );
	peer.Close();
	h.Pump( 2 );
	results.Check( h.host.cancelled.count( 1 ) == 1, "a disconnect cancels the screenshot ticket" );
	results.Check( h.server->ConnectionCount() == 0, "the connection is forgotten" );
	results.Check( h.host.logSink == nullptr, "the last subscriber leaving removes the log sink" );
}

void ShutdownCancels( Results &results )
{
	results.scope = "shutdown";
	Harness h;
	MemoryPeer peer = h.memory->Connect();
	peer.Write( Request( 7, "waitFor",
	                "{\"condition\":\"WAIT_CONDITION_FRAMES\",\"frames\":1000,\"timeoutMs\":600000}" ) +
	            Request( 8, "subscribe", "{\"log\":true}" ) );
	h.Pump();
	(void)Messages( peer );
	h.server->Shutdown();
	const auto replies = Messages( peer );
	results.Check( replies.size() == 1 && ErrorCode( replies[0] ) == -32006 &&
	                   Get( replies[0], { "id" } )->number_value() == 7,
	    "Shutdown answers deferred calls with CANCELLED" );
	results.Check( peer.ServerClosed(), "Shutdown closes the transport" );
	results.Check( h.host.logSink == nullptr, "Shutdown removes the log sink" );
	h.server->Pump();
	h.server->Shutdown();
	results.Check( Messages( peer ).empty(), "Pump and Shutdown after Shutdown do nothing" );
	h.server.reset();
}

void FramingFault( Results &results )
{
	results.scope = "framing-fault";
	ServerLimits limits;
	limits.maxFrameBytes = 64;
	Harness h( limits );
	MemoryPeer peer = h.memory->Connect();
	peer.Write( std::string( 200, 'x' ) );
	h.Pump( 2 );
	const auto replies = Messages( peer );
	results.Check( replies.size() == 1 && ErrorCode( replies[0] ) == -32700,
	    "an oversize frame gets one PARSE_ERROR" );
	results.Check( peer.ServerClosed(), "a framing fault closes the connection" );
}

void ReentrantPump( Results &results )
{
	results.scope = "reentrant-pump";
	Harness h;
	h.host.reentry = [&h] { h.server->Pump(); };
	MemoryPeer peer = h.memory->Connect();
	peer.Write( Request( 1, "exec", "{\"command\":\"pump\"}" ) + Request( 2, "status" ) );
	h.Pump();
	results.Check( Messages( peer ).size() == 2, "a re-entrant Pump is ignored safely" );
}

void SubscriptionToggle( Results &results )
{
	results.scope = "subscription";
	Harness h;
	MemoryPeer peer = h.memory->Connect();
	peer.Write( Request( 1, "subscribe", "{\"log\":true}" ) );
	h.Pump();
	const bool installed = h.host.logSink != nullptr;
	peer.Write( Request( 2, "subscribe", "{\"log\":false}" ) );
	h.Pump();
	results.Check( installed && h.host.logSink == nullptr, "unsubscribing removes the log sink" );
	results.Check( h.host.logSinkChanges == 2, "the sink is installed and removed exactly once" );
}

void ExecTruncation( Results &results )
{
	results.scope = "exec-truncation";
	ServerLimits limits;
	limits.dispatcher.maxExecOutputBytes = 100;
	Harness h( limits );
	MemoryPeer peer = h.memory->Connect();
	peer.Write( Request( 1, "exec", "{\"command\":\"spam\"}" ) );
	h.Pump();
	const auto replies = Messages( peer );
	results.Check( replies.size() == 1 &&
	                   Get( replies[0], { "result", "output" } )->string_value().size() == 100 &&
	                   Get( replies[0], { "result", "truncated" } )->bool_value(),
	    "exec output is bounded and flagged as truncated" );
}

void NotificationDeferred( Results &results )
{
	results.scope = "deferred-notification";
	Harness h;
	MemoryPeer peer = h.memory->Connect();
	peer.Write( "{\"jsonrpc\":\"2.0\",\"method\":\"waitFor\",\"params\":{\"condition\":"
	            "\"WAIT_CONDITION_FRAMES\",\"frames\":1,\"timeoutMs\":1000}}\n" );
	peer.Write( Request( 1, "quit" ) );
	h.Pump( 3 );
	const auto replies = Messages( peer );
	results.Check( replies.size() == 1 && Get( replies[0], { "id" } )->number_value() == 1,
	    "a deferred notification completes silently" );
	results.Check( h.host.quitRequested, "quit reaches the host" );
}

void CollectorStress( Results &results )
{
	results.scope = "log-collector";
	debugapi::LogCollector collector( 256 * 1024 );
	constexpr int kThreads = 4;
	constexpr int kLines = 20000;
	std::vector<std::thread> writers;
	for ( int t = 0; t < kThreads; ++t )
		writers.emplace_back( [&collector] {
			for ( int i = 0; i < kLines; ++i )
				collector.Append( "stress line\n", false, 0 );
		} );
	unsigned long delivered = 0, dropped = 0;
	auto drain = [&] {
		std::vector<debugapi::LogLine> lines;
		uint32_t lost = 0;
		collector.Drain( lines, lost );
		delivered += lines.size();
		dropped += lost;
	};
	for ( int i = 0; i < 200; ++i )
		drain();
	for ( std::thread &writer : writers )
		writer.join();
	drain();
	results.Check( delivered + dropped == static_cast<unsigned long>( kThreads ) * kLines,
	    "concurrent appends are delivered or counted, never lost" );
}

} // namespace

int main()
{
	Results results;
	CreationFailures( results );
	ConnectionLimit( results );
	PendingLimit( results );
	OutputBound( results );
	LogDropAccounting( results );
	DisconnectCancels( results );
	ShutdownCancels( results );
	FramingFault( results );
	ReentrantPump( results );
	SubscriptionToggle( results );
	ExecTruncation( results );
	NotificationDeferred( results );
	CollectorStress( results );
	return testing::ReportConformance( results.checks, results.failures );
}
