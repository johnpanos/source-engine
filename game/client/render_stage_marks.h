//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The client's stage marks for the render core (RFC 0016). The
//			engine marks every 3D view it pushes; the client marks what it
//			draws inside them, in the legacy frame's order. Without a render
//			core the marker system is absent and marking does nothing.
//
//=============================================================================//

#ifndef RENDER_STAGE_MARKS_H
#define RENDER_STAGE_MARKS_H

#include "render/legacy/material_blocks.h"
#include "render/legacy/stage_markers.h"
#include "render/legacy/temporal_views.h"
extern IRenderTemporalViews *g_pRenderTemporalViews;

// Set in CHLClient::Init from the app system factory; NULL without a core.
extern IRenderStageMarkers *g_pRenderStageMarkers;
extern IRenderMaterialBlocks *g_pRenderMaterialBlocks;

inline void ClientRender_MarkStage( RenderStageMarker stage )
{
	if ( g_pRenderStageMarkers )
		g_pRenderStageMarkers->MarkStage( stage );
}

#endif // RENDER_STAGE_MARKS_H
