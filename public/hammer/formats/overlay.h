//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless overlay (info_overlay) geometry for the Hammer editor
//			(RFC 0002, hammer.formats). An info_overlay projects a material onto a
//			set of brush faces using a stored basis (origin + U/V/normal axes) and
//			four corner points in overlay space -- the decal-on-geometry feature.
//			This strict, MFC-free, GPU-free core parses the entity into a structured
//			Overlay and computes the world-space quad a renderer would draw. It owns
//			no presentation.
//
//			World corner_i = BasisOrigin + BasisU * uv_i.x + BasisV * uv_i.y
//			                 + BasisNormal * uv_i.z, for the four uv0..uv3 corners.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_OVERLAY_H
#define HAMMER_FORMATS_OVERLAY_H

#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/brush.h" // geometry::Vec3d

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace hammer::formats
{

struct Overlay
{
	std::string material;              // the projected material
	std::vector<int> sides;            // target brush-face ids the overlay applies to
	geometry::Vec3d basisOrigin;       // projection origin in world space
	geometry::Vec3d basisU;            // U axis
	geometry::Vec3d basisV;            // V axis
	geometry::Vec3d basisNormal;       // face normal
	std::array<geometry::Vec3d, 4> uv; // corner positions in overlay (U/V/normal) space
	double startU = 0.0;
	double endU = 1.0;
	double startV = 0.0;
	double endV = 1.0;

	// The overlay's four corners transformed into world space.
	std::array<geometry::Vec3d, 4> WorldCorners() const;
};

// Parses one info_overlay entity block. Returns nullopt when the block is not an
// info_overlay or a required field (material, non-empty sides, BasisOrigin/U/V/
// Normal, uv0..uv3) is missing or malformed. StartU/EndU/StartV/EndV are optional.
std::optional<Overlay> ParseOverlay( const KeyValueNode &entity );

// Extracts every valid info_overlay entity from a parsed VMF document.
std::vector<Overlay> ExtractOverlays( const KeyValueNode &root );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_OVERLAY_H
