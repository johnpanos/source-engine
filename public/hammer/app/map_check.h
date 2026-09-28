//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Check for Problems (RFC 0002, hammer.app; legacy CMapCheckDlg):
//			a pure scan of a document that lists what would break or surprise
//			a compile or the game, and exact fixes for the problems that have
//			one. The problems presenter lists CheckMap's result; a fix is an
//			ordinary operation over a DocumentEdit (one undo step).
//
//			Order is deterministic: by code (the enum order), then by the first
//			object's id, then by message. One problem is reported per object
//			and code; its message names every offending key, connection,
//			material or id.
//
//			Codes (severity; fix):
//			  NoPlayerStart        (Error; none) no entity whose class starts
//			                       with "info_player_" (info_player_start and
//			                       every game's spawn classes: deathmatch,
//			                       combine, rebel, terrorist, teamspawn, ...).
//			  EmptyClassname       (Error; none) an entity without a class.
//			  UnknownClass         (Warning; none) catalog only: a class the
//			                       catalog does not know.
//			  EmptyBrushEntity     (Error; deletes it) catalog only: a solid
//			                       (brush) class entity that owns no solids
//			                       (legacy ErrorEmptyEntity).
//			  PointEntityWithSolids(Error; none) catalog only: a point or other
//			                       non-brush class that owns solids.
//			  MissingTarget        (Warning; removes the keys) a key that names
//			                       entities (target_destination per the
//			                       catalog; without a known class the
//			                       conventional "target") whose value matches
//			                       no entity name (scene::NameMatches, so a
//			                       trailing '*' is a prefix) and is not a
//			                       special name.
//			  ConnectionMissingTarget (Warning; removes those connections) an
//			                       output whose target matches no entity and is
//			                       not a special name.
//			  UnknownOutput        (Warning; removes them) catalog only: an
//			                       output the entity's (known) class does not
//			                       declare.
//			  UnknownInput         (Warning; removes them) catalog only: an
//			                       input that no matched target declares while
//			                       every matched target's class is known.
//			  UnusedKeyvalues      (Warning; removes the keys) catalog only: keys
//			                       the entity's (known) class does not declare,
//			                       except the allowlist origin, angles, angle,
//			                       targetname, spawnflags (engine- and
//			                       editor-owned keys); multi_manager is skipped
//			                       as in legacy Hammer.
//			  DuplicateKeys        (Warning; keeps the first occurrence, the one
//			                       the editor shows) a key repeated on an entity
//			                       (legacy ErrorDuplicateKeys).
//			  InvalidSolid         (Error; deletes the solid, and a brush entity
//			                       it empties) sides that do not bound a closed
//			                       convex volume with four or more faces.
//			  DuplicatePlanes      (Error; normalizes the sides) two sides with
//			                       the same outward normal, or a side that bounds
//			                       no face (legacy DoesContainDuplicates and the
//			                       face-point check).
//			  MissingMaterial      (Warning; none) materials port only: a side
//			                       material the port does not know.
//			  DuplicateSideId      (Error; fresh ids for the later sides) a side
//			                       VMF id used more than once; the first use in
//			                       id order keeps it.
//			  DuplicateObjectId    (Error; a fresh id for the later object) a
//			                       solid, entity or group VMF id used twice
//			                       within its kind.
//			  UndefinedVisgroup    (Warning; removes the dangling ids) an object
//			                       naming visgroup ids the tree does not define.
//			  HiddenWithoutVisgroup(Warning; files it in the visgroup "_hidden
//			                       by Check for Problems", kept hidden, as legacy
//			                       FixHiddenObject) an object hidden by
//			                       visgroups that neither it nor a container is
//			                       in, so no visgroup could show it.
//			  EmptyGroup           (Warning; deletes it and groups it empties)
//			                       a group with no members.
//			  OutsideMapBounds     (Error; none) a solid or point entity beyond
//			                       +/-16384 units (legacy MAX_COORD_INTEGER).
//
//			Special target names, always valid: !self, !activator, !caller,
//			!player, !pvsplayer, !speechtarget, !picker (case-insensitive).
//			Duplicate targetnames are legal in Source and are not reported.
//			Legacy's MixedFaces (Quake '*' liquids), SolidContents (Quake 2),
//			node ids and the hidden-group states are not Source problems here
//			and are not checked.
//
//			Fixes are exact and idempotent: FixProblem re-derives the problem
//			from the edit's current state and changes only what that problem
//			names; a problem that no longer applies is NothingToDo, one without
//			a fix is Rejected. FixAll repeats check-and-fix until no fixable
//			problem remains, so CheckMap afterwards reports none.
//
//=============================================================================//

#ifndef HAMMER_APP_MAP_CHECK_H
#define HAMMER_APP_MAP_CHECK_H

#include "hammer/app/edit_session.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/ports/material_info.h"
#include "hammer/scene/change_set.h"

#include <string>
#include <vector>

namespace hammer::app
{

struct MapProblem
{
	enum class Code
	{
		NoPlayerStart,
		EmptyClassname,
		UnknownClass,
		EmptyBrushEntity,
		PointEntityWithSolids,
		MissingTarget,
		ConnectionMissingTarget,
		UnknownOutput,
		UnknownInput,
		UnusedKeyvalues,
		DuplicateKeys,
		InvalidSolid,
		DuplicatePlanes,
		MissingMaterial,
		DuplicateSideId,
		DuplicateObjectId,
		UndefinedVisgroup,
		HiddenWithoutVisgroup,
		EmptyGroup,
		OutsideMapBounds,
	};
	enum class Severity
	{
		Error,
		Warning,
	};

	Code code = Code::NoPlayerStart;
	Severity severity = Severity::Error;
	std::string message;
	std::vector<scene::ObjectId> objects; // what to select; empty for map-wide problems
	bool fixable = false;
};

// A stable identifier for a code ("no-player-start", ...), for presenters,
// scripts and logs.
const char *MapProblemCodeName( MapProblem::Code code );

// The coordinate limit objects must stay within (legacy MAX_COORD_INTEGER).
constexpr double kMapCoordinateLimit = 16384.0;

// The visgroup HiddenWithoutVisgroup's fix files objects in.
constexpr const char *kCheckHiddenVisgroupName = "_hidden by Check for Problems";

// Scans the document. 'catalog' and 'materials' are optional; checks that need
// them are skipped without them.
std::vector<MapProblem> CheckMap( const scene::DocumentReader &doc,
    const ports::IEntityCatalog *catalog, const ports::IMaterialInfo *materials );

// Applies the fix for 'problem' (see the header comment).
EditResult FixProblem(
    scene::DocumentEdit &edit, const MapProblem &problem, const ports::IEntityCatalog *catalog );

// Fixes every fixable problem CheckMap reports, repeating until none remains.
// NothingToDo when there was none.
EditResult FixAll( scene::DocumentEdit &edit, const ports::IEntityCatalog *catalog,
    const ports::IMaterialInfo *materials );

} // namespace hammer::app

#endif // HAMMER_APP_MAP_CHECK_H
