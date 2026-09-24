//========= Copyright Valve Corporation, All rights reserved. ============//
//
// debugapi.framing.v1: splits a transport byte stream into message frames and
// frames outgoing payloads. It sits between the transport and the JSON-RPC
// layer and knows nothing about either.
//
// Contract (checked by unittests/debugapitest/test_framing.cpp for every
// provider, including seeded bad ones):
//  - A decoder is per connection. Feed() accepts arbitrary chunking (a frame
//    split across calls, many frames in one call) and appends complete frames
//    in order; the concatenated frames never depend on chunk boundaries.
//  - encode(payload) followed by decoding yields exactly `payload` for any
//    payload the provider accepts (see each provider's payload rule).
//  - A frame larger than maxFrameBytes, or a malformed header, returns Fatal
//    with a diagnostic; frames completed before the fault are still appended.
//    After Fatal the decoder appends nothing more.
//
//=============================================================================//

#ifndef PUBLIC_DEBUGAPI_DEBUGAPI_FRAMING_H
#define PUBLIC_DEBUGAPI_DEBUGAPI_FRAMING_H

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace debugapi
{

enum class FrameStatus
{
	Open,
	Fatal,
};

class IFrameDecoder
{
public:
	virtual ~IFrameDecoder() = default;
	virtual FrameStatus Feed(
	    std::string_view bytes, std::vector<std::string> &frames, std::string &error ) = 0;
};

// Linked, root-selected provider descriptor.
struct FramingProvider
{
	const char *name;
	std::unique_ptr<IFrameDecoder> ( *createDecoder )( size_t maxFrameBytes );
	// Appends one framed payload to `out`.
	void ( *encode )( std::string_view payload, std::string &out );
};

// "newline": each frame is one line terminated by '\n' (a preceding '\r' is
// dropped). Payloads must not contain '\n'; blank lines are ignored.
const FramingProvider &NewlineFraming();

// "content-length": LSP-style "Content-Length: N\r\n" header block (other
// headers ignored, 4 KiB limit) terminated by "\r\n", then N payload bytes.
// Any payload is accepted.
const FramingProvider &ContentLengthFraming();

} // namespace debugapi

#endif // PUBLIC_DEBUGAPI_DEBUGAPI_FRAMING_H
