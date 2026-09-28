//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editing settings that operations read and the UI, scripts and
//			tools set (RFC 0002, hammer.app): the face texture new geometry gets,
//			the entity class the Entity tool places, the grid, texture lock and
//			selection granularity. One owner, so a tool, a menu action and a
//			script command that "use the active material" agree.
//
//			Settings are session-scoped editor state, not document content: they
//			record no history and are not saved in the map.
//
//=============================================================================//

#ifndef HAMMER_APP_EDITOR_SETTINGS_H
#define HAMMER_APP_EDITOR_SETTINGS_H

#include "hammer/app/ops/create_ops.h"
#include "hammer/app/selection.h"
#include "hammer/scene/map_objects.h"

#include <string>

namespace hammer::app
{

struct EditorSettings
{
	EditorSettings()
	{
		faceTexture.material = "DEV/DEV_MEASUREGENERIC01B";
		faceTexture.u.scale = 0.25;
		faceTexture.v.scale = 0.25;
	}

	// The material, scales and lightmap scale new faces get (axes are derived
	// per face).
	scene::FaceTexture faceTexture;
	std::string entityClass = "info_player_start";
	ops::PrimitiveSpec primitive;
	ops::ArchSpec arch;
	double gridSize = 64.0;
	bool snapToGrid = true;
	bool textureLock = true;
	SelectionGranularity granularity = SelectionGranularity::Groups;
	// Offset Duplicate and Paste apply (legacy pastes in place).
	mapgeometry::Vec3d duplicateOffset = mapgeometry::Vec3d( 16, 16, 0 );
};

} // namespace hammer::app

#endif // HAMMER_APP_EDITOR_SETTINGS_H
