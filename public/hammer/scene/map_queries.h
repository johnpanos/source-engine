//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Structural queries over a map document or a staged edit (RFC 0002,
//			hammer.scene): group and brush-entity membership, the leaves an
//			object stands for, owners, bounds, and entity lookup by name or
//			class. Pure functions of a DocumentReader; results are in id order.
//
//=============================================================================//

#ifndef HAMMER_SCENE_MAP_QUERIES_H
#define HAMMER_SCENE_MAP_QUERIES_H

#include "hammer/scene/map_document.h"
#include "hammer/scene/solid_geometry.h"

#include <optional>
#include <string_view>
#include <vector>

namespace hammer::scene
{

// The objects directly in 'group': solids and entities whose 'group' is it
// (a brush entity's solids belong to the entity, not the group) and the
// groups nested in it.
std::vector<ObjectId> GroupMembers( const DocumentReader &doc, ObjectId group );

// The solids owned by brush entity 'entity'.
std::vector<ObjectId> EntitySolids( const DocumentReader &doc, ObjectId entity );

// Every object 'ids' stand for, deduplicated: a group expands to its members
// recursively (the groups themselves included), an entity to itself and its
// solids, a solid to itself. Unknown ids are skipped.
std::vector<ObjectId> ExpandObjects( const DocumentReader &doc, const std::vector<ObjectId> &ids );

// Only the solids and entities of ExpandObjects (what geometry edits touch).
std::vector<ObjectId> ExpandToLeaves( const DocumentReader &doc, const std::vector<ObjectId> &ids );

// The object that directly contains 'id': a solid's owning entity, otherwise
// its group; invalid at the top level.
ObjectId ContainerOf( const DocumentReader &doc, ObjectId id );

// The outermost container chain end: follows ContainerOf to the top.
ObjectId TopLevelOf( const DocumentReader &doc, ObjectId id );

// The one visibility rule: an object is visible when it is not quick-hidden,
// its visgroups are shown (EditorInfo::visgroupShown), and every container
// (owning entity, enclosing groups) is visible too. Unknown ids are not.
bool IsVisible( const DocumentReader &doc, ObjectId id );

// Bounds of an object: a solid's vertices, an entity's origin box (point
// entities use +/- 'pointHalfSize'; brush entities the union of their solids),
// a group's members. Nothing when the object has no extent.
std::optional<Box> ObjectBounds( const DocumentReader &doc, ObjectId id, double pointHalfSize = 8.0 );
std::optional<Box> ObjectsBounds(
    const DocumentReader &doc, const std::vector<ObjectId> &ids, double pointHalfSize = 8.0 );

// Source's target-name match: case-insensitive, with a trailing '*' matching
// any suffix.
bool NameMatches( std::string_view pattern, std::string_view name );

std::vector<ObjectId> FindEntitiesByName( const DocumentReader &doc, std::string_view pattern );
std::vector<ObjectId> FindEntitiesByClass( const DocumentReader &doc, std::string_view classname );

// The side (and its solid) with persistent id 'sideVmfId', if any.
std::optional<FaceRef> FindSideById( const DocumentReader &doc, std::uint32_t sideVmfId );

} // namespace hammer::scene

#endif // HAMMER_SCENE_MAP_QUERIES_H
