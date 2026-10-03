//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shadow tile record (render.shadows.v1, RFC 0016 K7): what a
//          receiver needs to sample one shadow view in the atlas. The
//          producer (render.pass.shadows: the atlas, the caster passes and
//          PackShadowTile) writes it; every receiver reads it through
//          render/shaders/common/shadow_sample.glsl's ShadowTile, whose
//          layout this is (std140 and std430 alike). It is here, below the
//          material layer, because receivers are material programs (the
//          surface program's clustered and projected lights) and passes
//          (render.pass.volumetric) that cannot depend on a sibling pass.
//
//          viewProjection: the shadow view's world to clip matrix, row-major
//          with column vectors. transform: the tile transform applied after
//          the perspective divide (u = x * scaleU + biasU, v = y * scaleV +
//          biasV). bounds: the tile viewport's rectangle in atlas texture
//          coordinates (u0, v0, u1, v1). params: the receiver depth bias and
//          the atlas size in texels, then 0, 0.
//
//=============================================================================//

#ifndef RENDER_SHADOW_TILE_H
#define RENDER_SHADOW_TILE_H

#include <cstdint>

namespace render
{

// Runtime point/spot projections produced by render.pass.shadows. A cube
// is world-aligned in +X, -X, +Y, -Y, +Z, -Z order; it is never an oriented
// area-light cube. The values are the surface record's packed layout tag.
enum class RuntimeShadowLayout : std::uint8_t
{
	kSingle = 1,
	kWorldCube = 6
};

struct ShadowTileGpu
{
	float viewProjection[4][4] = {};
	float transform[4] = {}; // scaleU, biasU, scaleV, biasV
	float bounds[4] = {};    // u0, v0, u1, v1
	float params[4] = {};    // receiver depth bias, atlas size in texels, 0, 0
};
static_assert( sizeof( ShadowTileGpu ) == 112 );

} // namespace render

#endif // RENDER_SHADOW_TILE_H
