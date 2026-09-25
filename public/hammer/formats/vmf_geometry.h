//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VMF keyvalues -> Hammer geometry decoders (RFC 0002, hammer.formats).
//			The VMF codec owns reading "solid", "side" and "dispinfo" blocks; the
//			geometry they describe is built by hammer.geometry, which knows nothing
//			of keyvalues. These functions walk a parsed document, decode the text
//			values, and hand the results to the geometry module:
//
//			  BuildSolidFromBlock    "solid" block    -> geometry::BrushSolid
//			  BuildSceneFromDocument VMF document     -> geometry::WorldScene
//			  ParseDispInfo          "dispinfo" block -> geometry::DispInfo
//
//			Strict, MFC-free, GPU-free; never throws on malformed input.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_VMF_GEOMETRY_H
#define HAMMER_FORMATS_VMF_GEOMETRY_H

#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/brush.h"
#include "hammer/geometry/displacement.h"

#include <optional>

namespace hammer::formats
{

// Extracts a single "solid" keyvalues block into a BrushSolid (parsing its side
// planes and materials). Returns a solid with no faces when the block has fewer
// than four valid side planes or does not bound a finite region.
geometry::BrushSolid BuildSolidFromBlock( const KeyValueNode &solidBlock );

// Imports a parsed VMF document (the root whose children are the top-level
// blocks) into a renderable WorldScene: every solid under "world" and under brush
// entities is resolved; entities are summarised. Never throws.
geometry::WorldScene BuildSceneFromDocument( const KeyValueNode &root );

// Parses a "dispinfo" keyvalues block. Returns nullopt when the block is malformed
// or internally inconsistent: power outside [1, 4]; a normals/distances/offsets
// grid whose row count != side or whose row length != side (offsets may be
// absent). "elevation" and "offsets" are optional; "power", "startposition",
// "normals" and "distances" are required.
std::optional<geometry::DispInfo> ParseDispInfo( const KeyValueNode &dispBlock );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_VMF_GEOMETRY_H
