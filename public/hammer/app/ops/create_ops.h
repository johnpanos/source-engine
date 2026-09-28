//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Creating map objects (RFC 0002, hammer.app): the Block tool's
//			primitives (legacy stock solids: block, wedge, cylinder, spike,
//			sphere) and the arch, and point entities with their class defaults.
//
//			Primitive vertices follow legacy Hammer: polygon points are placed
//			on the ellipse inscribed in the box, starting at +Y and going
//			clockwise seen from above, and rounded to whole units (legacy
//			polyMake's V_rint), so a primitive drawn on the grid stays on it.
//			Each face gets the given material with world-aligned axes.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_CREATE_OPS_H
#define HAMMER_APP_OPS_CREATE_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"

#include <string>
#include <vector>

namespace hammer::app::ops
{

enum class PrimitiveKind
{
	Block,
	Wedge,    // the box cut diagonally: full height at -Y, zero at +Y (legacy)
	Cylinder, // an n-sided prism
	Spike,    // an n-sided pyramid, apex at the top center
	Sphere,   // an n-sided polyhedral sphere (one convex solid)
};

struct PrimitiveSpec
{
	PrimitiveKind kind = PrimitiveKind::Block;
	int sides = 8; // cylinder, spike and sphere: 3..32
	// The world axis the primitive's height runs along (the free axis of the
	// view it was drawn in): 0, 1 or 2.
	int axis = 2;
};

// The solid (not yet in a document) for 'spec' inside 'box'. Nothing when
// the box has a non-positive extent, the side count is out of range, or the
// rounded vertices do not bound a volume.
std::optional<scene::Solid> MakePrimitive(
    const PrimitiveSpec &spec, const scene::Box &box, const scene::FaceTexture &texture );

// Adds the primitive to the world and returns its id through 'created'.
EditResult CreatePrimitive( scene::DocumentEdit &edit, const PrimitiveSpec &spec,
    const scene::Box &box, const scene::FaceTexture &texture, scene::ObjectId &created );

struct ArchSpec
{
	int sides = 8;           // segments, 3..128
	double wallWidth = 32.0; // radial thickness
	double arc = 360.0;      // degrees covered, (0, 360]
	double startAngle = 0.0; // degrees, from +X counterclockwise
	double addHeight = 0.0;  // z added per segment (a spiral)
};

// An arch: one solid per segment inside 'box' (XY ellipse, Z extent), grouped.
// 'created' receives the group id; 'segments' the solids.
EditResult CreateArch( scene::DocumentEdit &edit, const ArchSpec &spec, const scene::Box &box,
    const scene::FaceTexture &texture, scene::ObjectId &created,
    std::vector<scene::ObjectId> *segments = nullptr );

// The keys a new entity of 'info' starts with (legacy Hammer writes every
// schema key with a default): each key with a non-empty default, spawnflags
// as the sum of default-on flags. Keys already on 'entity' are kept.
void ApplyClassDefaults( scene::Entity &entity, const ports::EntityClassInfo &info );

// Places a point entity. With a catalog, the class must exist and not be a
// solid class, and its defaults are applied; without one any non-empty class
// is accepted.
EditResult PlaceEntity( scene::DocumentEdit &edit, const std::string &classname,
    const mapgeometry::Vec3d &origin, const ports::IEntityCatalog *catalog,
    scene::ObjectId &created );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_CREATE_OPS_H
