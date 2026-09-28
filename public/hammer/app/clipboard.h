//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor clipboard (RFC 0002, hammer.app): Copy, Paste, Paste
//			Special and Duplicate over detached map fragments (legacy
//			CMapDoc::Copy / Paste / OnEditPastespecial).
//
//			A MapFragment is a value independent of any document. Its objects
//			keep the ids they had in the source, but only as fragment-local
//			keys: every owner and group reference names another object of the
//			fragment or is invalid (world / top level), and Paste never
//			resolves a fragment id in the target document.
//
//			Copy rule. The ids expand like scene::ExpandObjects (a group brings
//			its members, a brush entity its solids). A reference to an object
//			that was not copied is dropped, so a solid copied without its brush
//			entity pastes as a world solid and an object copied without its
//			group pastes at the top level (legacy: the copied selection is what
//			pastes). Visgroup memberships are carried with the definitions
//			(id, name, color) of the visgroups they name; references to
//			visgroups the source tree does not define are dropped. (Legacy
//			Copy dropped all visgroup memberships; keeping them is a deliberate
//			improvement.)
//
//			Paste rule. Every pasted object gets a new runtime id and fresh
//			persistent VMF ids (solid, each side, entity, group). References
//			inside the fragment are remapped. An overlay's "sides"/"sides2"
//			keys (space-separated side VMF ids) are remapped to the new side
//			ids for sides pasted in the same copy; ids of sides that were not
//			copied are kept (they still name the original faces). Visgroups
//			are matched in the target: the same id with the same name, else
//			the first visgroup with the same name (tree order), else a new
//			top-level visgroup with that name and color.
//
//			Paste Special. Copy k (1-based) is placed with k times the offset
//			and, when given, k times the rotation angles (pitch, yaw, roll in
//			degrees, the legacy dialog's X/Y/Z fields) about the fragment's
//			bounds center; solids keep their texture alignment (texture lock,
//			as legacy Paste Special forces). With 'group', each copy is wrapped
//			in its own new top-level group. A transform that would make any
//			solid degenerate refuses the whole paste before anything is staged.
//
//			Name fix (legacy "make pasted entity names unique" / "add
//			prefix"): with Prefix or Suffix, each targetname defined in the
//			fragment becomes prefix+name or name+suffix, then, while that name
//			is already used in the target (or by an earlier pasted name), its
//			trailing number is incremented or "1" appended (legacy
//			IncrementStringName). Within each pasted copy every key value,
//			connection target and connection parameter that equals an old
//			name (case-insensitively) is rewritten to the new name; references
//			to names defined outside the fragment are untouched, and so are
//			wildcard references ("door*"). Keep leaves names as they are.
//
//=============================================================================//

#ifndef HAMMER_APP_CLIPBOARD_H
#define HAMMER_APP_CLIPBOARD_H

#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"

#include <optional>
#include <string>
#include <vector>

namespace hammer::app
{

struct MapFragment
{
	// The copied objects in source id order (fragment-local ids; see above).
	std::vector<scene::MapObject> objects;
	// The visgroups the objects name (id, name and color; no children).
	std::vector<scene::Visgroup> visgroups;
	// The copied objects' bounds in the source (point entities +/- 8 units);
	// nothing when none has an extent.
	std::optional<scene::Box> bounds;

	bool Empty() const { return objects.empty(); }
	std::size_t Count() const { return objects.size(); }
};

MapFragment Copy( const scene::DocumentReader &doc, const std::vector<scene::ObjectId> &ids );

struct PasteOptions
{
	mapgeometry::Vec3d offset;
	std::optional<mapgeometry::Vec3d> rotation; // (pitch, yaw, roll) degrees about the center
	int copies = 1;                             // 1..1024
	bool group = false;                         // wrap each copy in a new group

	enum class NameFix
	{
		Keep,
		Suffix,
		Prefix,
	};
	NameFix nameFix = NameFix::Keep;
	std::string nameText; // the prefix or suffix (may be empty: uniqueness only)
};

// Pastes 'fragment' into the edit. 'created' receives what the paste selects:
// per copy, its group (with 'group') or the copy's top-level objects, in
// creation order. Refuses an empty fragment (Nothing), a copy count out of
// range, non-finite offsets or angles, and degenerate transforms.
EditResult Paste( scene::DocumentEdit &edit, const MapFragment &fragment,
    const PasteOptions &options, std::vector<scene::ObjectId> *created = nullptr );

// Copy + Paste within one edit (legacy Clone): names kept, one copy at 'offset'.
EditResult Duplicate( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const mapgeometry::Vec3d &offset, std::vector<scene::ObjectId> *created = nullptr );

// The name Paste would give 'name' under 'options' when 'taken' (lower-cased
// names) are in use. Exposed for the Paste Special preview.
std::string FixedPasteName(
    const std::string &name, const PasteOptions &options, const std::vector<std::string> &taken );

} // namespace hammer::app

#endif // HAMMER_APP_CLIPBOARD_H
