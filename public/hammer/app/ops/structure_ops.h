//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Structural operations on the object hierarchy (RFC 0002,
//			hammer.app): deleting with cleanup, grouping, brush entity
//			membership (tie to entity, move to world) and quick hide.
//
//			Cleanup rules, one owner for every entry point:
//			  * deleting a group deletes its members; deleting an entity deletes
//			    its solids;
//			  * a brush entity left with no solids is deleted;
//			  * a group left with no members is deleted (recursively upward).
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_STRUCTURE_OPS_H
#define HAMMER_APP_OPS_STRUCTURE_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/change_set.h"

#include <string>
#include <vector>

namespace hammer::app::ops
{

EditResult DeleteObjects( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids );

// Removes brush entities without solids and groups without members, as the
// cleanup rules above; exposed so other operations can finish with it. An
// entity counts as a brush entity when it owned solids in the base document or
// is named in 'formerOwners' (owners whose solids the operation moved or
// deleted); a point entity is never removed.
void RemoveEmptyContainers(
    scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &formerOwners = {} );

// Groups the given objects (a solid of a brush entity stands for its entity)
// under a new group. When they all share one enclosing group the new group
// nests in it; otherwise it is top level.
EditResult GroupObjects(
    scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, scene::ObjectId &created );

// Dissolves the given groups: members move to each group's enclosing group.
EditResult UngroupObjects( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &groups );

// Makes the solids the ids stand for owned by one brush entity: a new entity of
// 'classname' (defaults applied from the catalog, which must list a solid class
// when given) or, when 'existing' is valid, that brush entity. Solids taken
// from other brush entities leave them (empty ones are deleted).
EditResult TieToEntity( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const std::string &classname, const ports::IEntityCatalog *catalog, scene::ObjectId existing,
    scene::ObjectId &entity );

// Returns the solids of the given brush entities (or the given solids) to the
// world; emptied entities are deleted.
EditResult MoveToWorld( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids );

// Quick hide: sets the 'hidden' flag of the given objects.
EditResult SetHidden(
    scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, bool hidden );
// Hides every visible top-level object not among 'keep' (after expansion).
EditResult HideUnselected( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &keep );
// Clears every quick-hide flag.
EditResult UnhideAll( scene::DocumentEdit &edit );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_STRUCTURE_OPS_H
