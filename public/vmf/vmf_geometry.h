//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VMF keyvalues -> map geometry decoders (RFC 0002, content.vmf).
//			The VMF codec owns reading "solid", "side" and "dispinfo" blocks; the
//			geometry they describe is built by world.map-geometry, which knows nothing
//			of keyvalues. These functions walk a parsed document, decode the text
//			values, and hand the results to the geometry module:
//
//			  BuildSolidFromBlock    "solid" block    -> mapgeometry::BrushSolid
//			  BuildSceneFromDocument VMF document     -> mapgeometry::WorldScene
//			  ParseDispInfo          "dispinfo" block -> mapgeometry::DispInfo
//
//			Strict, MFC-free, GPU-free; never throws on malformed input.
//
//=============================================================================//

#ifndef VMF_VMF_GEOMETRY_H
#define VMF_VMF_GEOMETRY_H

#include "kvtext/keyvalues.h"
#include "mapgeometry/brush.h"
#include "mapgeometry/displacement.h"

#include <optional>

namespace vmf
{

// Extracts a single "solid" keyvalues block into a BrushSolid (parsing its side
// planes and materials). Returns a solid with no faces when the block has fewer
// than four valid side planes or does not bound a finite region.
mapgeometry::BrushSolid BuildSolidFromBlock( const kvtext::KeyValueNode &solidBlock );

// Imports a parsed VMF document (the root whose children are the top-level
// blocks) into a renderable WorldScene: every solid under "world" and under brush
// entities is resolved; entities are summarised. Never throws.
mapgeometry::WorldScene BuildSceneFromDocument( const kvtext::KeyValueNode &root );

// Parses a "dispinfo" keyvalues block. Returns nullopt when the block is malformed
// or internally inconsistent: power outside [1, 4]; a normals/distances/offsets
// grid whose row count != side or whose row length != side (offsets may be
// absent). "elevation" and "offsets" are optional; "power", "startposition",
// "normals" and "distances" are required.
std::optional<mapgeometry::DispInfo> ParseDispInfo( const kvtext::KeyValueNode &dispBlock );

} // namespace vmf

#endif // VMF_VMF_GEOMETRY_H
