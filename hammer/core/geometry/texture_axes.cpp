//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the world-aligned texture-axis policy owner
//			(RFC 0002). Ported from hammer/mapface.cpp
//			InitializeQuakeStyleTextureAxes; no tier0/MFC dependencies.
//
//=============================================================================//

#include "hammer/geometry/texture_axes.h"

namespace hammer::geometry
{

namespace
{

struct BaseAxis
{
	Vec3d normal;
	Vec3d u;
	Vec3d v;
};

// Legacy order and values: floor, ceiling, west, east, south, north walls.
constexpr BaseAxis kBaseAxes[6] = {
    { { 0, 0, 1 }, { 1, 0, 0 }, { 0, -1, 0 } },
    { { 0, 0, -1 }, { 1, 0, 0 }, { 0, -1, 0 } },
    { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, -1 } },
    { { -1, 0, 0 }, { 0, 1, 0 }, { 0, 0, -1 } },
    { { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 } },
    { { 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, -1 } },
};

} // namespace

TextureAxes WorldAlignedTextureAxes( const Vec3d &normal )
{
	double best = 0.0;
	int bestAxis = 0;
	for ( int i = 0; i < 6; ++i )
	{
		const Vec3d &n = kBaseAxes[i].normal;
		const double dot = normal.x * n.x + normal.y * n.y + normal.z * n.z;
		if ( dot > best )
		{
			best = dot;
			bestAxis = i;
		}
	}
	return { kBaseAxes[bestAxis].u, kBaseAxes[bestAxis].v };
}

} // namespace hammer::geometry
