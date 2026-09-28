//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Cordon operations (RFC 0002, hammer.app; the Cordon tool's
//			commands) and the one cordon query builds and tools use. Cordons
//			live in DocumentSettings: a list of named cordons, each with its own
//			active flag and one or more boxes, and a document-wide enable
//			(cordonsActive).
//
//			Form. DocumentSettings::cordonForm records which VMF block the map
//			used. The older Single form ('cordon' { mins maxs active }) holds
//			exactly one unnamed cordon with one box whose active flag is the
//			document enable. Every operation keeps the form honest: when the
//			content stops fitting the Single form (a second cordon or box, a
//			name, or diverging flags), or when cordons are first added to a
//			map without any, the form becomes List. It never switches back to
//			Single on its own. In the Single form SetCordonsEnabled and
//			SetCordonActive on the one cordon change both flags together (they
//			are one 'active' key in that form).
//
//			Boxes must be finite with mins < maxs on every axis. A cordon keeps
//			at least one box (remove the cordon instead).
//
//			Query rule: when cordons are enabled, the active boxes are the
//			boxes of every active cordon; an object is inside when its bounds
//			intersect any active box, edges inclusive (legacy
//			CMapClass::IsCulledByCordon). No active box means no restriction.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_CORDON_OPS_H
#define HAMMER_APP_OPS_CORDON_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"

#include <cstddef>
#include <string>
#include <vector>

namespace hammer::app::ops
{

// Appends an active cordon with one box. 'index' receives its position.
EditResult AddCordon( scene::DocumentEdit &edit, const std::string &name, const scene::Box &box,
    std::size_t *index = nullptr );

EditResult RemoveCordon( scene::DocumentEdit &edit, std::size_t index );

EditResult RenameCordon( scene::DocumentEdit &edit, std::size_t index, const std::string &name );

EditResult SetCordonBox(
    scene::DocumentEdit &edit, std::size_t cordon, std::size_t boxIndex, const scene::Box &box );

EditResult AddCordonBox( scene::DocumentEdit &edit, std::size_t cordon, const scene::Box &box );

// Refuses to remove a cordon's last box.
EditResult RemoveCordonBox( scene::DocumentEdit &edit, std::size_t cordon, std::size_t boxIndex );

EditResult SetCordonActive( scene::DocumentEdit &edit, std::size_t index, bool active );

// The document-wide enable (the legacy "toggle cordon" command).
EditResult SetCordonsEnabled( scene::DocumentEdit &edit, bool enabled );

// True when 'box' is finite with mins < maxs on every axis.
bool ValidCordonBox( const scene::Box &box );

// The boxes that restrict the map now (the query rule above), in cordon then
// box order; empty when cordons are disabled or none is active.
std::vector<scene::Box> ActiveCordonBoxes( const scene::DocumentReader &doc );

// True when 'bounds' intersects one of 'active' (edges inclusive), or when
// 'active' is empty.
bool InsideCordons( const std::vector<scene::Box> &active, const scene::Box &bounds );
bool InsideCordons( const scene::DocumentReader &doc, const scene::Box &bounds );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_CORDON_OPS_H
