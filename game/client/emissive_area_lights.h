//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Emissive surfaces light their surroundings (RFC 0011 light set v2,
//          render/area_light.h). Each frame the client publishes, as area
//          lights, the self-illuminated parts of the models it draws
//          ($selfillum materials: eyes, indicator lights, door strips) and
//          the lit screens that register as sources (the Portal 2 chamber
//          signs), at most r_area_lights of them, the most important at the
//          view (render/emissive_area_lights.h).
//
//=============================================================================//
#ifndef EMISSIVE_AREA_LIGHTS_H
#define EMISSIVE_AREA_LIGHTS_H
#ifdef _WIN32
#pragma once
#endif

#include "render/area_light.h"

// A source of area lights besides emissive models: a lit screen or panel.
class IEmissiveAreaLightSource
{
public:
	// Writes at most nMax of this frame's lights (world space, reach set by
	// area_light::Reach) and their keys (EmissiveAreaLights_PanelKey); returns
	// the count.
	virtual int GetAreaLights( area_light::AreaLight *pLights, int *pKeys, int nMax ) = 0;

protected:
	~IEmissiveAreaLightSource() {}
};

void EmissiveAreaLights_AddSource( IEmissiveAreaLightSource *pSource );
void EmissiveAreaLights_RemoveSource( IEmissiveAreaLightSource *pSource );

// Stable 24-bit keys: a model's emitter by entity and emitter index, a panel
// by entity.
inline int EmissiveAreaLights_EntityKey( int nEntIndex, int nEmitter )
{
	return ( ( nEntIndex & 0xffff ) << 8 ) | ( nEmitter & 0x7f );
}

inline int EmissiveAreaLights_PanelKey( int nEntIndex )
{
	return ( ( nEntIndex & 0xffff ) << 8 ) | 0x80;
}

// A panel's tile (RFC 0016 render.world-panel.v1: a panel casts one light per
// tile of its image); tile 0 is the panel's key.
inline int EmissiveAreaLights_PanelTileKey( int nEntIndex, int nTile )
{
	return EmissiveAreaLights_PanelKey( nEntIndex ) | ( nTile & 0x3f );
}

// A texture's gamma-encoded RGBA at (s, t) (wrapping, nearest texel of its
// mip no larger than 256 texels), decoded on the CPU and cached for the
// level: the sampler of a panel's light (world_panel::TileRadiance). False for
// a texture only the GPU holds (a render target, a font's glyph page).
class ITexture;
bool EmissiveAreaLights_SampleTexture( ITexture *pTexture, float s, float t, float rgba[4] );

#endif // EMISSIVE_AREA_LIGHTS_H
