//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Displaced faces of document solids (RFC 0002, hammer.scene): the
//			bridge from a side's typed Displacement to the displacement core's
//			surface (world.map-geometry displacement.h), so the viewport,
//			picking and displacement editing share one construction.
//
//=============================================================================//

#ifndef HAMMER_SCENE_DISPLACEMENT_GEOMETRY_H
#define HAMMER_SCENE_DISPLACEMENT_GEOMETRY_H

#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/displacement.h"

#include <optional>

namespace hammer::scene
{

// The displaced surface of side 'sideIndex' (which must carry a displacement
// and bound a four-cornered face). Absent arrays count as zero (normals,
// distances, offsets, alphas); 'baseOnly' ignores distances, offsets and
// elevation (the undisplaced grid). Nothing when the side is not displaced,
// its face is not a quad, or the arrays do not match the power.
std::optional<mapgeometry::DisplacementSurface> BuildDisplacement(
    const Solid &solid, std::size_t sideIndex, bool baseOnly = false );

} // namespace hammer::scene

#endif // HAMMER_SCENE_DISPLACEMENT_GEOMETRY_H
