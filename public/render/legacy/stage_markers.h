//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RenderStageMarkers001 (RFC 0016): how the client DLL marks the
//			stages of the legacy frame for the render core. The launcher adds
//			it as an app system, so the client finds it through its normal
//			appSystemFactory lookup; a product without the render core has no
//			such system and the client marks nothing.
//
//			A preserved ABI package (architecture/modules.json legacyAbi): C++11,
//			no foundation types, and the enum values never change. They match
//			render::frame::Stage.
//
//=============================================================================//

#ifndef RENDER_LEGACY_STAGE_MARKERS_H
#define RENDER_LEGACY_STAGE_MARKERS_H

#include "appframework/IAppSystem.h"

#define RENDER_STAGE_MARKERS_INTERFACE_VERSION "RenderStageMarkers001"

enum RenderStageMarker
{
	RENDER_STAGE_FRAME_BEGIN = 0, // marked by the engine, not the client
	RENDER_STAGE_VIEW_BEGIN = 1,
	RENDER_STAGE_SKYBOX = 2,
	RENDER_STAGE_OPAQUE = 3,
	RENDER_STAGE_TRANSLUCENT = 4,
	RENDER_STAGE_VIEW_MODEL = 5,
	RENDER_STAGE_POST_PROCESS = 6,
	RENDER_STAGE_VIEW_END = 7,
	RENDER_STAGE_HUD = 8,
	RENDER_STAGE_FRAME_END = 9, // marked by the engine, not the client
	RENDER_STAGE_COUNT
};

class IRenderStageMarkers : public IAppSystem
{
public:
	// Returns false when the stage breaks the frame's order (a view stage
	// outside a view, a stage going backwards) or no frame is open. The
	// frame continues either way.
	virtual bool MarkStage( RenderStageMarker stage ) = 0;
	// Marks rejected since the core was composed.
	virtual unsigned int GetOrderViolations() const = 0;
};

#endif // RENDER_LEGACY_STAGE_MARKERS_H
