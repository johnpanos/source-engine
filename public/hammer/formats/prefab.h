//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless prefab instantiation for the Hammer editor (RFC 0002,
//			hammer.formats). A prefab is a stored VMF fragment (a library object);
//			inserting it places a transformed copy of its world solids and entities
//			into the map at an origin/angles. That placement is the SAME QAngle
//			transform func_instance uses, so this builds on the shared
//			hammer/formats/vmf_transform owner (DRY) rather than re-deriving it.
//			Strict, MFC-free, GPU-free. Unlike a func_instance (a live reference
//			expanded from a file store), a prefab is instantiated inline from an
//			already-parsed fragment.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_PREFAB_H
#define HAMMER_FORMATS_PREFAB_H

#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/brush.h" // geometry::Vec3d

#include <vector>

namespace hammer::formats
{

// Where and how to place a prefab (Source QAngle, degrees).
struct Placement
{
	geometry::Vec3d origin;
	double pitch = 0.0;
	double yaw = 0.0;
	double roll = 0.0;
};

// The objects a prefab instantiation produces, ready to merge into the map's
// world/entity lists.
struct PrefabInstance
{
	std::vector<KeyValueNode> solids;   // transformed world solids
	std::vector<KeyValueNode> entities; // transformed (origin-placed) entities
};

// Instantiates 'prefabDoc' (a parsed VMF fragment) at 'at': every world solid and
// entity is transformed (rotate then translate) through the shared VMF transform.
// Solid/entity counts are preserved; an identity placement returns the fragment's
// objects unchanged in value.
PrefabInstance InstantiatePrefab( const KeyValueNode &prefabDoc, const Placement &at );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_PREFAB_H
