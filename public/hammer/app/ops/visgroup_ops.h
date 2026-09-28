//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Visgroup operations (RFC 0002, hammer.app): the visgroup tree
//			(create, rename, delete, reparent), membership, and showing or
//			hiding a visgroup's objects, plus the queries the visgroup
//			presenter reads. Visgroups live in DocumentSettings::visgroups;
//			membership is each object's EditorInfo::visgroupIds (persistent
//			visgroup ids), and visibility is each object's
//			EditorInfo::visgroupShown, which scene::IsVisible reads.
//
//			Membership (legacy VisGroups_ObjectCanBelongToVisGroup): loose
//			solids, entities and groups can belong to visgroups, including
//			members of groups; a solid of a brush entity cannot, so it stands
//			for its entity. A group stands for itself (its members are not
//			given the id).
//
//			A visgroup's members are the objects that list its id or the id of
//			one of its descendants (legacy IsInVisGroupRecursive). What
//			showing or hiding touches is those members and everything they
//			contain (a group's members, an entity's solids), as legacy
//			CMapClass::VisGroupShow recurses into children.
//
//			Visibility rule (one owner; no per-visgroup flag is stored, the VMF
//			has none):
//			  * hiding visgroup V clears visgroupShown on every object V
//			    touches;
//			  * showing V sets visgroupShown (and visgroupAutoShown, as the
//			    legacy user-visgroup show does) on every object V touches,
//			    except an object that is suppressed: it, or one of its
//			    containers, lists another visgroup W outside V's subtree whose
//			    other members (the members of W not touched by this show) exist
//			    and are all hidden. So an object in several visgroups is shown
//			    only while every one of its visgroups is shown.
//			  * A visgroup whose members are all shared with V has no state of
//			    its own and never suppresses (no flag is stored to remember it).
//
//			An object left hidden without any covering visgroup (it and its
//			containers list none) by DeleteVisgroup or RemoveFromVisgroup is
//			shown again, the legacy CMapClass::CheckVisibility rule.
//
//			Visgroup ids referenced by objects but missing from the tree are
//			left alone here and reported by the map check (map_check.h).
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_VISGROUP_OPS_H
#define HAMMER_APP_OPS_VISGROUP_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"

#include <string>
#include <vector>

namespace hammer::app::ops
{

// A fresh persistent visgroup id: one more than the largest id in the tree or
// referenced by any object (so a dangling reference never adopts a new
// visgroup).
int NextVisgroupId( const scene::DocumentReader &doc );

// Creates a visgroup named 'name' (non-empty; duplicate names are allowed, as
// in legacy Hammer) under 'parentId' (0 = top level, appended last).
EditResult CreateVisgroup(
    scene::DocumentEdit &edit, const std::string &name, int parentId, int &createdId );

EditResult RenameVisgroup( scene::DocumentEdit &edit, int visgroupId, const std::string &name );

// Removes the visgroup: its child visgroups move to its parent (at its place,
// in order), its id is removed from every object's visgroupIds, and objects
// left hidden without a covering visgroup are shown.
EditResult DeleteVisgroup( scene::DocumentEdit &edit, int visgroupId );

// Reparents the visgroup under 'newParentId' (0 = top level), appended last.
// Refuses a parent that is the visgroup itself or one of its descendants.
EditResult MoveVisgroup( scene::DocumentEdit &edit, int visgroupId, int newParentId );

// Adds the objects 'ids' stand for (a brush solid stands for its entity) to
// the visgroup. With 'removeFromOthers' (legacy "move to visgroup") their other
// memberships are dropped first. Visibility is not changed.
EditResult AddToVisgroup( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    int visgroupId, bool removeFromOthers = false );

// Removes the objects from the visgroup (the same representatives as
// AddToVisgroup); objects left hidden without a covering visgroup are shown.
EditResult RemoveFromVisgroup(
    scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, int visgroupId );

// Shows or hides the visgroup's objects (the visibility rule above).
EditResult SetVisgroupVisible( scene::DocumentEdit &edit, int visgroupId, bool visible );

enum class VisgroupState
{
	Shown,  // every member is shown
	Hidden, // every member is hidden
	Mixed,  // some of each
	Empty,  // no member (in the visgroup or its descendants)
};

// The presenter state of a visgroup, derived from its members' visgroupShown
// (legacy VisGroups_UpdateForObject/UpdateParents). Empty for an unknown id.
VisgroupState VisgroupVisibility( const scene::DocumentReader &doc, int visgroupId );

// The objects listing 'visgroupId' (with 'recursive', or any of its
// descendants' ids), in id order.
std::vector<scene::ObjectId> VisgroupMembers(
    const scene::DocumentReader &doc, int visgroupId, bool recursive = true );

// The ids of the visgroup and all its descendants (empty for an unknown id).
std::vector<int> VisgroupSubtree( const scene::DocumentReader &doc, int visgroupId );

// True when 'id' or one of its containers lists at least one visgroup id.
bool CoveredByVisgroup( const scene::DocumentReader &doc, scene::ObjectId id );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_VISGROUP_OPS_H
