//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: func_instance operations (RFC 0002, hammer.app; legacy
//			CMapDoc::CollapseInstances and the compiler's instance merge,
//			utils/vbsp/map.cpp MergeInstance). The instance's content is a
//			MapFragment the caller loads (fragment_io.h LoadFragment on the
//			resolved InstanceFile); these operations never read files.
//
//			Collapse replaces the func_instance with its content, merged the
//			way the compiler merges it, so the collapsed map compiles like the
//			instanced one. For each content entity, in this order (vbsp):
//			  1. parameters: every key value, the classname and each
//			     connection's target, input and parameter get the instance's
//			     "replaceNN" substitutions (instance_fixup.h);
//			  2. name fixup with the instance's "fixup_style" and fixup name
//			     ("targetname", else "name", else "InstanceAuto<n>" with the
//			     smallest n no target entity name uses): every name-typed key
//			     value and every connection target, except '@' and '!' names,
//			     whether or not the name is defined inside the instance (the
//			     compiler fixes every name; references out of an instance must
//			     be '@' names). Name-typed keys are the class's
//			     target_source/target_destination keys per the catalog; with
//			     no catalog, or a class it does not know, the conventional
//			     "targetname", "target", "parentname", "filtername",
//			     "damagefilter" and "lightingorigin";
//			  3. transform by the instance's "origin" and "angles" (rotate,
//			     then translate): solids with texture lock, entities' origin
//			     and orientation, overlays' basis.
//			The merged objects get fresh runtime and VMF ids through
//			app::Paste (overlay "sides" follow the new side ids). Top-level
//			content objects join the func_instance's group. Materials are not
//			replaced: this tree's compiler has no material substitution.
//
//			Nested instances: a func_instance inside the content is kept as
//			an entity (parameters and fixup applied to its own keys, its
//			"file" verbatim, resolved like any instance of the target map);
//			collapse it with another call. Collapsing outer-first gives the
//			compiler's names for prefix fixups ("outer-inner-name").
//
//			Not modeled: the compiler's remapping of other FGD position and
//			angle keys (vecline, axis, angle_negative_pitch "pitch") beyond
//			"origin", "angles" and "angle"; func_instance_io_proxy / "instance:"
//			I/O; func_instance_parms defaults.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_INSTANCE_OPS_H
#define HAMMER_APP_OPS_INSTANCE_OPS_H

#include "foundation/expected.h"
#include "hammer/app/clipboard.h"
#include "hammer/app/edit_session.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/change_set.h"

#include <string>
#include <vector>

namespace hammer::app::ops
{

// Every func_instance entity (case-insensitive class), in id order.
std::vector<scene::ObjectId> FindInstances( const scene::DocumentReader &doc );

// The instance's "file" as the compiler looks it up: backslashes turned
// into '/', ".vmf" appended when the name has no extension. Empty when the
// entity is not a func_instance or has no file.
std::string InstanceFile( const scene::Entity &entity );

// The content 'instance' merges, placed: 'instanceContent' with steps 1 to 3
// of the rule above applied (parameters, name fixup, transform), nested
// func_instances kept as entities. The fixup name is the instance's
// "targetname", else its "name" key, else 'autoFixupName' (CollapseInstance
// passes the InstanceAuto<n> name). The error is the refusal CollapseInstance
// reports for a malformed "origin"/"angles", an unknown "fixup_style" or a
// transform that makes a solid degenerate. The one owner of the merge rule:
// CollapseInstance and the viewports' instance preview both use it.
foundation::Expected<MapFragment, std::string> PlaceInstanceContent( const scene::Entity &instance,
    const MapFragment &instanceContent, const std::string &autoFixupName,
    const ports::IEntityCatalog *catalog = nullptr );

// Replaces func_instance 'instanceEntity' with 'instanceContent' (see the
// rule above). 'created' receives the merged top-level objects. An empty
// content only removes the instance. Refuses a non-instance, an instance
// owning solids, a malformed "origin"/"angles" or unknown "fixup_style",
// and transforms that make a solid degenerate, before anything is staged.
EditResult CollapseInstance( scene::DocumentEdit &edit, scene::ObjectId instanceEntity,
    const MapFragment &instanceContent, std::vector<scene::ObjectId> *created = nullptr,
    const ports::IEntityCatalog *catalog = nullptr );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_INSTANCE_OPS_H
