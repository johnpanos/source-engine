//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Drawing the paint maps on world surfaces (paint_render.cpp).
//
//=============================================================================//

#ifndef PAINT_RENDER_H
#define PAINT_RENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/utlvector.h"
#include "surfacehandle.h"

class IMatRenderContext;

// Brings the paint textures up to date with the paint pages (once per frame,
// before the world draws; cheap when nothing changed).
void R_PaintUpdateTextures();
// Releases the paint textures and materials.
void R_PaintShutdown();
// Draws the paint of these painted, opaque, non-displacement brush surfaces.
void R_DrawPaintedSurfaces(
    IMatRenderContext *pRenderContext, const CUtlVector<SurfaceHandle_t> &surfaces );

#endif // PAINT_RENDER_H
