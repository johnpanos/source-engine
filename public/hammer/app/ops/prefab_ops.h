//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Prefab insertion (RFC 0002, hammer.app; legacy CPrefab3D,
//			CToolEntity prefab creation and CMapDoc::OnInsertprefabOriginal).
//			A prefab is a MapFragment (usually every object of a decoded prefab
//			VMF, FragmentFromDocument); reading and decoding the file stays in
//			the caller (fragment_io.h). Insertion is built on app::Paste, so
//			every object gets fresh runtime and VMF ids, overlay "sides" are
//			remapped and visgroups matched exactly as for a paste.
//
//			Anchor rule. Legacy has two placements:
//			  * the Insert Prefab command centers the prefab's bounding box on
//			    the chosen point (CreateInBox with the prefab's own size, i.e.
//			    CPrefab3D::CreateAtPoint): PrefabAnchor::BoundsCenter, the
//			    default;
//			  * the Entity tool's click on a face puts the prefab's own origin
//			    (0 0 0 of the prefab file) at the snapped hit point
//			    (CObjectBar::BuildPrefabObjectAtPoint ->
//			    CreateAtPointAroundOrigin): PrefabAnchor::Origin.
//			Neither legacy path rests the prefab's bottom on the clicked
//			surface; with Origin, a prefab authored with its floor at z = 0
//			lands on the surface. The rotation (pitch, yaw, roll degrees) turns
//			the prefab about the anchor, then the anchor moves to 'at'; solids
//			keep their texture alignment (legacy forces texture lock) and
//			overlays their basis.
//
//			Grouping. With 'group', a prefab of more than one top-level object
//			is wrapped in one new group; a single top-level object is never
//			wrapped (legacy CPrefab3D::Create).
//
//			Name keywords. A targetname containing "&i" becomes the text
//			before it, then one more than the highest number N any target
//			entity uses as <before>N<after> (case-insensitive, all digits),
//			then the text after it; only the first "&i" is expanded. Key
//			values and connection targets inside the prefab that equal the old
//			name (case-insensitive) follow it (legacy ExpandObjectKeywords).
//			Other names are kept as authored.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_PREFAB_OPS_H
#define HAMMER_APP_OPS_PREFAB_OPS_H

#include "hammer/app/clipboard.h"
#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"
#include "mapgeometry/transform.h"

#include <optional>
#include <string>
#include <vector>

namespace hammer::app::ops
{

// Every object of 'doc' (with the visgroups they name) as a fragment, like
// Copy of all objects; nothing when the document holds no objects.
std::optional<MapFragment> FragmentFromDocument( const scene::MapDocument &doc );

enum class PrefabAnchor
{
	BoundsCenter, // the prefab's bounding-box center lands on 'at'
	Origin,       // the prefab's origin (0 0 0) lands on 'at'
};

// Inserts 'prefab' at 'at' turned by 'anglesPYR'. 'created' receives what
// the insertion selects (the new group, or the top-level objects). Refuses an
// empty prefab (Nothing), non-finite 'at' or angles, a BoundsCenter anchor
// for a prefab without extent, and rotations that make a solid degenerate.
EditResult InsertPrefab( scene::DocumentEdit &edit, const MapFragment &prefab,
    const mapgeometry::Vec3d &at, const mapgeometry::Vec3d &anglesPYR, bool group,
    std::vector<scene::ObjectId> *created = nullptr,
    PrefabAnchor anchor = PrefabAnchor::BoundsCenter );

// The fragment after 'xf': solids with texture lock, entities' origin and
// orientation, overlays' basis; bounds recomputed. Nothing when a solid
// would become degenerate.
std::optional<MapFragment> TransformedFragment(
    const MapFragment &fragment, const mapgeometry::Affine &xf );

// 'name' with its "&i" keyword expanded against the entities of 'target';
// 'name' unchanged when it has none.
std::string ExpandNameKeyword( const std::string &name, const scene::DocumentReader &target );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_PREFAB_OPS_H
