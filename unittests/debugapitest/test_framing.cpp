//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: debugapi.framing.v1 — the shared framing suite against the newline
//          and Content-Length providers. Sensitivity builds run it against one
//          deliberately bad provider and pass only if the suite detects it:
//            DEBUGAPI_SEED_FRAMING_CHUNK_BOUNDARY  a chunk end ends a frame
//            DEBUGAPI_SEED_FRAMING_UNBOUNDED       oversize frames accepted
//
//===========================================================================//

#include "conformance/checks.h"
#include "debugapi/debugapi_framing.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{

using namespace debugapitest;
using debugapi::FrameStatus;
using debugapi::FramingProvider;

struct Fixture
{
	const FramingProvider *provider;
	bool payloadMayContainNewline;
};

std::vector<std::string> Payloads( bool newlines )
{
	std::vector<std::string> payloads = {
	    "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"status\"}",
	    "x",
	    std::string( 70000, 'a' ),
	    "{\"text\":\"tab\\tquote\\\"\"}",
	    "\xE2\x9C\x93 utf-8",
	};
	if ( newlines )
	{
		payloads.push_back( "line\nbreak" );
		payloads.push_back( "Content-Length: 5\r\n\r\nfake" );
		payloads.push_back( "" );
	}
	return payloads;
}

// Feeds `stream` in chunks of `chunk` bytes (0 = all at once).
std::vector<std::string> Decode( const FramingProvider &provider, const std::string &stream,
    size_t chunk, size_t maxFrame, FrameStatus &status, std::string &error )
{
	auto decoder = provider.createDecoder( maxFrame );
	std::vector<std::string> frames;
	status = FrameStatus::Open;
	const size_t step = chunk == 0 ? stream.size() : chunk;
	for ( size_t offset = 0; offset < stream.size() && status == FrameStatus::Open; offset += step )
		status = decoder->Feed( std::string_view( stream ).substr( offset, step ), frames, error );
	return frames;
}

void RunFramingConformance( const Fixture &fixture, Results &results )
{
	const FramingProvider &provider = *fixture.provider;
	results.scope = provider.name;
	const std::vector<std::string> payloads = Payloads( fixture.payloadMayContainNewline );
	std::string stream;
	for ( const std::string &payload : payloads )
		provider.encode( payload, stream );

	// The frames never depend on chunk boundaries.
	for ( size_t chunk : { size_t( 0 ), size_t( 1 ), size_t( 2 ), size_t( 7 ), size_t( 4096 ) } )
	{
		FrameStatus status;
		std::string error;
		const auto frames = Decode( provider, stream, chunk, 1 << 20, status, error );
		results.Check( status == FrameStatus::Open, "valid streams stay open" );
		results.Check( frames == payloads, "encode then decode returns every payload, in order" );
	}

	// Oversize frames are fatal, keep earlier frames, and end the stream.
	{
		std::string oversize;
		provider.encode( "small", oversize );
		provider.encode( std::string( 2048, 'b' ), oversize );
		provider.encode( "after", oversize );
		for ( size_t chunk : { size_t( 0 ), size_t( 1 ), size_t( 100 ) } )
		{
			FrameStatus status;
			std::string error;
			const auto frames = Decode( provider, oversize, chunk, 1024, status, error );
			results.Check( status == FrameStatus::Fatal, "a frame over maxFrameBytes is fatal" );
			results.Check( !error.empty(), "a fatal status carries a diagnostic" );
			results.Check( frames.size() == 1 && frames[0] == "small",
			    "frames before the fault are kept and none after it" );
		}
		auto decoder = provider.createDecoder( 1024 );
		std::vector<std::string> frames;
		std::string error;
		(void)decoder->Feed( oversize, frames, error );
		std::string more;
		provider.encode( "later", more );
		results.Check( decoder->Feed( more, frames, error ) == FrameStatus::Fatal &&
		                   frames.size() == 1,
		    "a decoder stays fatal and appends nothing more" );
	}

	// A frame of exactly maxFrameBytes is accepted.
	{
		std::string exact;
		provider.encode( std::string( 1024, 'c' ), exact );
		FrameStatus status;
		std::string error;
		const auto frames = Decode( provider, exact, 3, 1024, status, error );
		results.Check( status == FrameStatus::Open && frames.size() == 1 &&
		                   frames[0].size() == 1024,
		    "a frame of exactly maxFrameBytes is accepted" );
	}
}

[[maybe_unused]] void NewlineSpecifics( Results &results )
{
	results.scope = "newline";
	const FramingProvider &provider = debugapi::NewlineFraming();
	FrameStatus status;
	std::string error;
	const auto frames = Decode( provider, "a\r\n\n  \r\nb\n", 1, 64, status, error );
	results.Check( status == FrameStatus::Open && frames == std::vector<std::string>{ "a", "b" },
	    "CR before LF is dropped and blank lines are ignored" );
	const auto partial = Decode( provider, "no terminator", 0, 64, status, error );
	results.Check( partial.empty(), "an unterminated line is not a frame yet" );
}

[[maybe_unused]] void ContentLengthSpecifics( Results &results )
{
	results.scope = "content-length";
	const FramingProvider &provider = debugapi::ContentLengthFraming();
	FrameStatus status;
	std::string error;
	auto frames = Decode( provider,
	    "content-length: 2\r\nContent-Type: application/vscode-jsonrpc\r\n\r\nhi", 1, 64, status,
	    error );
	results.Check( status == FrameStatus::Open && frames == std::vector<std::string>{ "hi" },
	    "the header name is case-insensitive and other headers are ignored" );
	for ( const char *bad : { "Content-Length: x\r\n\r\n", "Content-Type: a\r\n\r\n",
	          "Content-Length: 1\r\nContent-Length: 1\r\n\r\nz", "garbage\r\n\r\n",
	          "Content-Length: -1\r\n\r\n" } )
	{
		frames = Decode( provider, bad, 0, 64, status, error );
		results.Check( status == FrameStatus::Fatal && frames.empty() && !error.empty(),
		    "a malformed, missing or duplicate Content-Length is fatal" );
	}
	frames = Decode( provider, std::string( 5000, 'h' ), 100, 1 << 20, status, error );
	results.Check( status == FrameStatus::Fatal, "an unterminated header block is bounded" );
}

#if defined( DEBUGAPI_SEED_FRAMING_CHUNK_BOUNDARY ) || defined( DEBUGAPI_SEED_FRAMING_UNBOUNDED )
#define DEBUGAPI_SEEDED 1

class SeededDecoder final : public debugapi::IFrameDecoder
{
public:
	explicit SeededDecoder( size_t maxFrameBytes )
	    : m_Inner( debugapi::NewlineFraming().createDecoder(
	          maxFrameBytes
#ifdef DEBUGAPI_SEED_FRAMING_UNBOUNDED
	          * 1000
#endif
	          ) )
	{
	}

	FrameStatus Feed(
	    std::string_view bytes, std::vector<std::string> &frames, std::string &error ) override
	{
#ifdef DEBUGAPI_SEED_FRAMING_CHUNK_BOUNDARY
		std::string terminated( bytes );
		if ( !terminated.empty() && terminated.back() != '\n' )
			terminated.push_back( '\n' );
		return m_Inner->Feed( terminated, frames, error );
#else
		return m_Inner->Feed( bytes, frames, error );
#endif
	}

private:
	std::unique_ptr<debugapi::IFrameDecoder> m_Inner;
};

std::unique_ptr<debugapi::IFrameDecoder> CreateSeeded( size_t maxFrameBytes )
{
	return std::make_unique<SeededDecoder>( maxFrameBytes );
}

const FramingProvider kSeeded{ "seeded-bad", &CreateSeeded, debugapi::NewlineFraming().encode };
#endif

} // namespace

int main()
{
	Results results;
#ifdef DEBUGAPI_SEEDED
	RunFramingConformance( Fixture{ &kSeeded, false }, results );
	std::printf( "seeded framing: %lu of %lu checks failed\n", results.failures, results.checks );
	return testing::ReportConformance( 1, results.failures > 0 ? 0 : 1 );
#else
	RunFramingConformance( Fixture{ &debugapi::NewlineFraming(), false }, results );
	RunFramingConformance( Fixture{ &debugapi::ContentLengthFraming(), true }, results );
	NewlineSpecifics( results );
	ContentLengthSpecifics( results );
	return testing::ReportConformance( results.checks, results.failures );
#endif
}
