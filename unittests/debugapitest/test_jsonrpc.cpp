//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: source.debug.v1 over JSON-RPC 2.0 — every shared case in
//          debugapi/fixtures/jsonrpc_cases.json against the real server and
//          the fake host, once per framing provider (the substitutability
//          proof for the framing seam: identical messages either way).
//
//          DEBUGAPI_SEED_FIXTURE_MUTATION: the comparator's negative control.
//          Every expectation is mutated; the build passes only if every case
//          then mismatches.
//
//===========================================================================//

#include "conformance/checks.h"
#include "debugapi/core/debugapi_server.h"
#include "debugapi/core/memory_transport.h"
#include "fakes/fake_host.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <google/protobuf/struct.pb.h>
#include <google/protobuf/util/json_util.h>

namespace
{

namespace gp = ::google::protobuf;
using namespace debugapitest;

std::string RepoPath( const char *relative )
{
	std::string self = __FILE__;
	const std::string suffix = "unittests/debugapitest/test_jsonrpc.cpp";
	if ( self.size() >= suffix.size() &&
	     self.compare( self.size() - suffix.size(), suffix.size(), suffix ) == 0 )
		self.resize( self.size() - suffix.size() );
	else
		self.clear();
	return self + relative;
}

bool Parse( std::string_view text, gp::Value &value )
{
	return gp::util::JsonStringToMessage( text, &value ).ok();
}

std::string Print( const gp::Value &value )
{
	std::string out;
	(void)gp::util::MessageToJsonString( value, &out );
	return out;
}

const gp::Value *Field( const gp::Value &value, const char *name )
{
	if ( value.kind_case() != gp::Value::kStructValue )
		return nullptr;
	auto it = value.struct_value().fields().find( name );
	return it == value.struct_value().fields().end() ? nullptr : &it->second;
}

// Structural equality; "$FRAMING" stands for the active framing, and an
// expected error object without "message" accepts any nonempty message.
bool Matches( const gp::Value &expected, const gp::Value &actual, const std::string &framing,
    bool errorObject = false )
{
	if ( expected.kind_case() != actual.kind_case() )
		return false;
	switch ( expected.kind_case() )
	{
	case gp::Value::kNullValue:
		return true;
	case gp::Value::kNumberValue:
		return expected.number_value() == actual.number_value();
	case gp::Value::kBoolValue:
		return expected.bool_value() == actual.bool_value();
	case gp::Value::kStringValue:
		return ( expected.string_value() == "$FRAMING" ? framing : expected.string_value() ) ==
		       actual.string_value();
	case gp::Value::kListValue:
	{
		const auto &e = expected.list_value().values();
		const auto &a = actual.list_value().values();
		if ( e.size() != a.size() )
			return false;
		for ( int i = 0; i < e.size(); ++i )
		{
			if ( !Matches( e[i], a[i], framing ) )
				return false;
		}
		return true;
	}
	case gp::Value::kStructValue:
	{
		const auto &e = expected.struct_value().fields();
		const auto &a = actual.struct_value().fields();
		for ( const auto &[key, value] : a )
		{
			if ( e.find( key ) != e.end() )
				continue;
			const bool lenientMessage = errorObject && key == "message" &&
			                            value.kind_case() == gp::Value::kStringValue &&
			                            !value.string_value().empty();
			if ( !lenientMessage )
				return false;
		}
		for ( const auto &[key, value] : e )
		{
			auto it = a.find( key );
			if ( it == a.end() || !Matches( value, it->second, framing, key == "error" ) )
				return false;
		}
		return true;
	}
	default:
		return false;
	}
}

struct CaseResult
{
	bool ran = false;
	bool matched = true;
	std::string detail;
};

void Mutate( gp::Value &expect )
{
	auto *list = expect.mutable_list_value();
	if ( list->values_size() == 0 )
	{
		( *list->add_values()->mutable_struct_value()->mutable_fields() )["phantom"]
		    .set_bool_value( true );
		return;
	}
	gp::Value &first = *list->mutable_values( 0 );
	if ( first.kind_case() == gp::Value::kListValue )
		first.mutable_list_value()->add_values()->set_null_value( gp::NULL_VALUE );
	else
		( *first.mutable_struct_value()->mutable_fields() )["mutated"].set_bool_value( true );
}

CaseResult RunCase( const gp::Value &testCase, const debugapi::FramingProvider &framing,
    double clockStep, bool mutate )
{
	CaseResult result;
	FakeHost host;
	const gp::Value *product = Field( testCase, "product" );
	if ( product && product->string_value() != "client" )
	{
		host.info.product = debugapi::HostProduct::Dedicated;
		host.status.clientSignon = 0;
		host.status.playerCount = 0;
		if ( product->string_value() == "dedicated-idle" )
			host.status.serverActive = false;
	}

	auto transport = std::make_unique<debugapi::MemoryTransport>();
	debugapi::MemoryTransport &memory = *transport;
	auto server = debugapi::Server::Create( std::move( transport ), framing, host );
	if ( !server )
	{
		result.matched = false;
		result.detail = "server creation failed: " + server.Error();
		return result;
	}
	debugapi::MemoryPeer peer = memory.Connect();
	auto decoder = framing.createDecoder( 64 * 1024 * 1024 );
	result.ran = true;

	const gp::Value *steps = Field( testCase, "steps" );
	for ( const gp::Value &step : steps->list_value().values() )
	{
		std::string payload;
		if ( const gp::Value *text = Field( step, "sendText" ) )
			payload = text->string_value();
		else
			payload = Print( *Field( step, "send" ) );
		std::string framed;
		framing.encode( payload, framed );
		peer.Write( framed );

		const int pumps = static_cast<int>( Field( step, "pumps" )->number_value() );
		for ( int i = 0; i < pumps; ++i )
		{
			host.now += clockStep;
			server.Value()->Pump();
		}

		std::vector<std::string> frames;
		std::string error;
		(void)decoder->Feed( peer.Read(), frames, error );
		gp::Value actual;
		for ( const std::string &frame : frames )
		{
			if ( !Parse( frame, *actual.mutable_list_value()->add_values() ) )
			{
				result.matched = false;
				result.detail = "unparsable server frame: " + frame;
				return result;
			}
		}
		if ( frames.empty() )
			actual.mutable_list_value();

		gp::Value expect = *Field( step, "expect" );
		if ( mutate )
			Mutate( expect );
		if ( !Matches( expect, actual, framing.name ) )
		{
			result.matched = false;
			result.detail = "expected " + Print( expect ) + "\n  actual " + Print( actual );
			return result;
		}
	}

	// Nothing further arrives once the case's expectations are met.
	for ( int i = 0; i < 3; ++i )
	{
		host.now += clockStep;
		server.Value()->Pump();
	}
	const std::string trailing = peer.Read();
	if ( !trailing.empty() )
	{
		result.matched = false;
		result.detail = "unexpected trailing output: " + trailing;
	}
	return result;
}

} // namespace

int main()
{
	Results results;
	results.scope = "jsonrpc";
	results.Check( debugapi::SchemaMatchesMethods(), "the generated service matches the dispatcher" );

	std::ifstream stream( RepoPath( "debugapi/fixtures/jsonrpc_cases.json" ) );
	std::stringstream text;
	text << stream.rdbuf();
	gp::Value document;
	const bool loaded = stream.good() && Parse( text.str(), document );
	results.Check( loaded, "the shared fixture file loads" );
	const gp::Value *cases = loaded ? Field( document, "cases" ) : nullptr;
	const gp::Value *clock = loaded ? Field( document, "pumpClockStepSeconds" ) : nullptr;
	results.Check( cases && cases->list_value().values_size() >= 30 && clock,
	    "the fixture declares its cases and clock step" );
	if ( !cases || !clock )
		return testing::ReportConformance( results.checks, results.failures );

#ifdef DEBUGAPI_SEED_FIXTURE_MUTATION
	const bool mutate = true;
#else
	const bool mutate = false;
#endif
	unsigned long mismatched = 0;
	unsigned long total = 0;
	for ( const debugapi::FramingProvider *framing :
	    { &debugapi::NewlineFraming(), &debugapi::ContentLengthFraming() } )
	{
		for ( const gp::Value &testCase : cases->list_value().values() )
		{
			const std::string name =
			    std::string( framing->name ) + "/" + Field( testCase, "name" )->string_value();
			const CaseResult result =
			    RunCase( testCase, *framing, clock->number_value(), mutate );
			++total;
			if ( !result.matched )
				++mismatched;
			if ( !mutate )
			{
				results.scope = name;
				results.Check( result.ran && result.matched, "the case matches its fixture" );
				if ( !result.matched )
					std::fprintf( stderr, "  %s\n", result.detail.c_str() );
			}
		}
	}
#ifdef DEBUGAPI_SEED_FIXTURE_MUTATION
	std::printf( "mutated fixtures: %lu of %lu cases detected\n", mismatched, total );
	return testing::ReportConformance( 1, mismatched == total ? 0 : 1 );
#else
	std::printf( "fixture cases: %lu run, %lu mismatched\n", total, mismatched );
	return testing::ReportConformance( results.checks, results.failures );
#endif
}
