//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless VMF `func_instance` expansion for the Hammer editor (RFC 0002,
//			hammer.formats). A func_instance entity references another VMF ("file")
//			and places it at an origin/angles; expansion loads that child, rotates
//			and translates its world solids and entities into the parent, and
//			replaces the func_instance with the merged result -- the nested-prefab
//			mechanism Source maps rely on. Strict, MFC-free, GPU-free core over the
//			keyvalues codec (hammer.formats), the IFileStore port (hammer.ports),
//			and the geometry Vec3d (hammer.geometry).
//
//			Rotation is the Source QAngle convention (pitch=Y, yaw=Z, roll=X).
//			World solids are transformed by rotating each side plane's three points
//			and translating by the origin; child entities are appended with their
//			"origin" transformed. Nested instances expand recursively, bounded by a
//			depth guard so a self- or cyclic reference is a reported error, never an
//			infinite loop.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_INSTANCING_H
#define HAMMER_FORMATS_INSTANCING_H

#include "hammer/formats/keyvalues.h"
#include "hammer/ports/file_store.h"

#include <string>

namespace hammer::formats
{

struct InstanceExpandResult
{
	bool ok = false;
	KeyValueNode document; // flattened: func_instance entities replaced by transformed contents
	std::string error;     // human-readable diagnostic when !ok
	int expanded = 0;      // number of func_instance entities expanded (all levels)
};

// Expands every `func_instance` entity in 'root' (a parsed VMF document) by
// loading its referenced VMF through 'store', transforming that child's world
// solids and entities by the instance's `origin` + `angles`, and merging them in.
// Nested instances expand recursively; 'maxDepth' bounds the recursion so a self-
// or cyclic reference is reported as an error rather than looping forever. A
// missing/unreadable/ malformed child file is an error. On success, `document`
// is the flattened VMF with no `func_instance` entities remaining.
InstanceExpandResult ExpandInstances(
    const KeyValueNode &root, ports::IFileStore &store, int maxDepth = 16 );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_INSTANCING_H
