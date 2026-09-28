//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The full-fidelity VMF map codec (RFC 0002, hammer.formats): the
//			hammer::ports::IMapCodec that turns Valve Map Format text into a
//			hammer::scene::MapDocument and back. Every field the document models
//			survives a load/save (authored plane points, texture axes and shifts,
//			displacements, entity key order and duplicates, connections with
//			their separator, groups, visgroups, hidden objects, cameras, both
//			cordon forms), and content the document does not model is kept
//			verbatim in its 'extra' fields or in the settings' unknown blocks.
//
//			Contract: unittests/hammertest/contracts/formats.vmf_map_codec.v1.md
//			(and the port's ports.map_codec.v1.md). In short:
//			  * Decode never loses content. What it cannot model it keeps
//			    verbatim (a solid with a malformed side or fewer than four sides
//			    stays a raw 'solid' block where it was found, an unparsable
//			    connection a raw pair in a preserved 'connections' block) and
//			    reports a warning with the source line.
//			  * Missing or malformed persistent ids, and duplicate side ids, are
//			    replaced by fresh ones above every id in the file, with a warning.
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

	// Fails for content VMF text cannot hold: a '"' in any key, value or name, a
	// block name that is empty or holds whitespace or braces, a non-finite
	// number, a connection field holding its own separator, or a reference to a
	// missing entity (solid owner) or group.
	foundation::Expected<std::string, hammer::ports::CodecError> Encode(
	    const hammer::scene::MapDocument &document ) const override;
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_VMF_MAP_CODEC_H
