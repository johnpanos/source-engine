//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Named owner of the world-aligned texture-axis policy (RFC 0002,
//			hammer.geometry): the u/v axes a face gets from its plane normal
//			when no authored axes exist. Ported from legacy Hammer's
//			CMapFace::InitializeQuakeStyleTextureAxes (hammer/mapface.cpp), the
//			same table vbsp's TextureAxisFromPlane uses.
//
//=============================================================================//

#ifndef HAMMER_GEOMETRY_TEXTURE_AXES_H
#define HAMMER_GEOMETRY_TEXTURE_AXES_H

#include "hammer/geometry/brush.h"

namespace hammer::geometry
{

struct TextureAxes
{
	Vec3d u;
	Vec3d v;
};

// Picks the first of floor, ceiling, west, east, south and north whose base
// normal has the largest positive dot product with 'normal' (a tie keeps the
// earlier entry, as legacy did), and returns that entry's u/v axes. A normal
// with no positive dot product (the zero vector) gets the floor axes.
TextureAxes WorldAlignedTextureAxes( const Vec3d &normal );

} // namespace hammer::geometry

#endif // HAMMER_GEOMETRY_TEXTURE_AXES_H
