//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Selection values and the selection policy (RFC 0002, hammer.app;
//			"Selection and property editing"). A Selection is a plain value: the
//			selected objects, the selected faces (face editing), and the primary
//			object (the last one picked, which property panels show first).
//
//			Policy, in one owner so the pointer, keyboard, menu and script entry
//			points agree:
//			  * Granularity decides what a hit on an object selects: Groups selects
//			    the outermost group (or brush entity) containing the hit, Objects
//			    the brush entity or loose solid ignoring groups, Solids the solid
//			    itself (even inside a brush entity) -- legacy Hammer's three
//			    selection modes.
//			  * Combine applies Replace / Add / Toggle / Remove.
//			  * Prune drops ids and faces that no longer exist.
//
//=============================================================================//

#ifndef HAMMER_APP_SELECTION_H
#define HAMMER_APP_SELECTION_H

#include "hammer/scene/map_document.h"

#include <vector>

namespace hammer::app
{

enum class SelectionGranularity
{
	Groups,
	Objects,
	Solids,
};

enum class SelectMode
{
	Replace,
	Add,
	Toggle,
	Remove,
};

struct Selection
{
	std::vector<scene::ObjectId> objects; // sorted, unique
	std::vector<scene::FaceRef> faces;    // sorted, unique
	scene::ObjectId primary;              // a member of 'objects', or invalid

	bool Empty() const { return objects.empty() && faces.empty(); }
	bool Contains( scene::ObjectId id ) const;
	bool ContainsFace( const scene::FaceRef &face ) const;

	friend bool operator==( const Selection &, const Selection & ) = default;
};

// What a hit on 'hit' selects under 'granularity'.
scene::ObjectId ResolvePick(
    const scene::DocumentReader &doc, scene::ObjectId hit, SelectionGranularity granularity );

// The selection after applying 'ids' with 'mode' to 'current'. The primary
// becomes the last id added (Replace/Add/Toggle-on); a removed primary falls
// back to the largest remaining id. Faces are kept.
Selection CombineObjects(
    const Selection &current, const std::vector<scene::ObjectId> &ids, SelectMode mode );
// The same for faces; objects are kept.
Selection CombineFaces(
    const Selection &current, const std::vector<scene::FaceRef> &faces, SelectMode mode );

// Drops objects and faces that no longer exist in 'doc' (and a dead primary).
Selection Prune( const scene::DocumentReader &doc, const Selection &selection );

// Selection helpers built on the policy.
Selection SelectAll( const scene::DocumentReader &doc, SelectionGranularity granularity,
    bool includeHidden = false );
// Every top-level object (per granularity) not currently selected.
Selection InvertSelection(
    const scene::DocumentReader &doc, const Selection &current, SelectionGranularity granularity );

} // namespace hammer::app

#endif // HAMMER_APP_SELECTION_H
