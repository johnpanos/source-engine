//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless cordon filtering for the Hammer editor (RFC 0002,
//			hammer.formats). A cordon is a world-space box that restricts which map
//			geometry a partial compile/export includes -- the "cordon tools"
//			feature. Given a parsed VMF document and a cordon box, this produces a
//			filtered document keeping only the world solids whose bounds intersect
//			the box and the entities that belong inside it. Strict, MFC-free,
//			GPU-free core over the keyvalues codec and geometry Vec3d.
//
//			A world solid is kept iff its axis-aligned bounds (the union of its
//			side-plane points) intersect the cordon box. An entity with an "origin"
//			(a point entity) is kept iff that origin is inside the box; a brush
//			entity (one carrying "solid" children) is kept iff any of its solids'
//			bounds intersect the box; a positionless entity (no origin, no solids,
//			e.g. logic_auto) is always kept.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_CORDON_H
#define HAMMER_FORMATS_CORDON_H

#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/brush.h" // geometry::Vec3d

#include <cstddef>

namespace hammer::formats
{

struct CordonBox
{
	geometry::Vec3d mins;
	geometry::Vec3d maxs;
};

struct CordonResult
{
	KeyValueNode document;         // the filtered VMF
	std::size_t solidsKept = 0;
	std::size_t solidsRemoved = 0;
	std::size_t entitiesKept = 0;
	std::size_t entitiesRemoved = 0;
};

// Returns 'root' filtered to the cordon 'box' (see the header comment for the
// keep rules). A box with mins > maxs on any axis intersects nothing, so all
// positional geometry is removed. Non-world/non-entity blocks (versioninfo, etc.)
// are preserved verbatim.
CordonResult ApplyCordon( const KeyValueNode &root, const CordonBox &box );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_CORDON_H
