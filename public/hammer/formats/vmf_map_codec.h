//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The strict VMF map codec (RFC 0002, hammer.formats): the
//			hammer::ports::IMapCodec that turns Valve Map Format text into a
//			hammer::scene::MapDocument and back. Every block and key maps to a
//			typed field of the scene model (authored plane points, texture axes,
//			typed displacements, entity key order and duplicates, connections
//			with their separator, groups, visgroups, hidden objects, editor
//			logical positions and comments, version and view settings, cameras,
//			both cordon forms). There is no verbatim carry: content the model
//			cannot hold is a CodecError.
//
//			Contract: unittests/hammertest/contracts/formats.vmf_map_codec.v1.md
//			(and the port's ports.map_codec.v1.md). In short:
//			  * Decode rejects what it cannot model (an unknown block or key, an
//			    unparsable connection, a malformed number, plane or vector, a
//			    solid with fewer than four sides, displacement rows that do not
//			    match the power, a duplicate id, a groupid naming no group). The
//			    error names the block path ("world/solid[12]/side[3]: unknown
//			    key 'foo'") and the source line.
//			  * The one warning is lossless: objects and sides without ids get
//			    fresh ids above every id in the file.
//			  * Runtime ids are issued in a fixed order: world solids (file
//			    order), world groups (file order), then each entity followed by
//			    its solids (file order). Encode writes objects in that same
//			    order, so Decode(Encode(Decode(text))) issues identical ids.
//			  * Numbers are written with scene::FormatNumber when that text reads
//			    back to the same double, and as the shortest exact decimal
//			    otherwise, so every coordinate round-trips bit for bit.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_VMF_MAP_CODEC_H
#define HAMMER_FORMATS_VMF_MAP_CODEC_H

#include "hammer/ports/map_codec.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace hammer::formats
{

class VmfMapCodec final : public hammer::ports::IMapCodec
{
public:
	std::string_view FormatName() const override { return "vmf"; }

	foundation::Expected<hammer::ports::DecodedMap, hammer::ports::CodecError> Decode(
	    std::string_view text, std::uint32_t serial ) const override;

	// Fails for content VMF text cannot hold: a '"' in any key or value, a
	// non-finite number, a connection field holding its own separator, an 'id'
	// or 'classname' among entity or world keys, displacement arrays whose
	// sizes do not match the power, or a reference to a missing entity (solid
	// owner) or group.
	foundation::Expected<std::string, hammer::ports::CodecError> Encode(
	    const hammer::scene::MapDocument &document ) const override;
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_VMF_MAP_CODEC_H
