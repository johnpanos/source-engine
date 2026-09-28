//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Map persistence port (RFC 0002, hammer.ports; "Persistence
//			contracts"). A codec turns file text into a detached MapDocument and
//			back. The application consumes this port and never selects or
//			constructs a codec itself; hammer.formats provides the VMF codec and
//			the composition root wires it.
//
//			Contract (see unittests/hammertest/contracts/ports.map_codec.v1.md):
//			  * Decode builds a new document and returns it only on success; it
//			    never touches an existing document. Recoverable oddities (an
//			    unparsable connection, a duplicate side id) are warnings, and the
//			    affected data is preserved verbatim where the format allows.
//			  * Encode(Decode(text)) preserves every field the document models;
//			    Decode(Encode(doc)) is SameContent with 'doc' up to runtime ids.
//			  * Both are pure: no I/O, no globals; safe on any thread for
//			    distinct inputs.
//
//=============================================================================//

#ifndef HAMMER_PORTS_MAP_CODEC_H
#define HAMMER_PORTS_MAP_CODEC_H

#include "foundation/expected.h"
#include "hammer/scene/map_document.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::ports
{

struct CodecDiagnostic
{
	std::string message;
	int line = 0; // 1-based source line; 0 when not tied to one
};

struct DecodedMap
{
	scene::MapDocument document;
	std::vector<CodecDiagnostic> warnings;
};

struct CodecError
{
	std::string message;
	int line = 0;
};

class IMapCodec
{
public:
	virtual ~IMapCodec() = default;

	// Short format name for reports ("vmf").
	virtual std::string_view FormatName() const = 0;

	// Parses 'text' into a new document whose ids carry 'serial'.
	virtual foundation::Expected<DecodedMap, CodecError> Decode(
	    std::string_view text, std::uint32_t serial ) const = 0;

	// Serializes 'document'. Fails only for content the format cannot express.
	virtual foundation::Expected<std::string, CodecError> Encode(
	    const scene::MapDocument &document ) const = 0;
};

} // namespace hammer::ports

#endif // HAMMER_PORTS_MAP_CODEC_H
