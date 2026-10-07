//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The reduced 3DS material model (RFC 0026 decision 8, user
//			decision 2026-10-07): how the one surface program's points reach a
//			device whose artifacts are kPica, which has a programmable vertex
//			stage and a fixed-function fragment stage (six TEV combiners,
//			three texture units). Portable: the rules are data, checked on the
//			host; the vertex programs they name are picasso programs built
//			into 3DS builds (families/pica/).
//
//			The model, in gamma space (the PICA200 decodes no sRGB):
//			  colour = base x tint x light, with base and tint's alpha;
//			  light  = the lightmap page x 2 (a world or flat surface),
//			           the vertex colour (unlit: when $vertexcolor),
//			           the static prop's baked vertex colour x 2, or
//			           the model's ambient cube and four lights evaluated per
//			           vertex (spot cones and half-Lambert dropped) x 2 (a
//			           model draw, or the pbr point's world geometry without
//			           a baked lightmap: a model handed over as world geometry);
//			  self-illumination (base alpha) mixes in base x $selfillumtint;
//			  the alpha test is the GPU's (reference from $alphatest), the
//			  blend the port's.
//			Every other term is dropped by a recorded rule (ReducedPoint::
//			dropped names them) or, where dropping would change what the
//			surface is, refused by name (ReducedPoint::refusal).
//
//=============================================================================//

#ifndef RENDER_MATERIAL_SURFACE_REDUCED_H
#define RENDER_MATERIAL_SURFACE_REDUCED_H

#include "render/device/pica_format.h"
#include "render/material/surface_program.h"

#include <string>
#include <string_view>
#include <vector>

namespace render::material
{

// The vertex program a reduced point runs (families/pica/*.v.pica).
enum class ReducedVertex : std::uint8_t
{
	kFlat,        // flat and world layouts: the vertex colour under $vertexcolor
	kStaticLight, // world layout, a static prop's baked vertex light
	kModel,       // model layout: per-vertex model lighting
	kWorldLit     // world layout in world space (a model draw handed over as
	              // world geometry): per-vertex model lighting, no transform
};

struct ReducedPoint
{
	std::string refusal;                   // non-empty: the point is not drawn, and why
	std::vector<std::string_view> dropped; // terms this point loses, by name
	ReducedVertex vertex = ReducedVertex::kFlat;
	device::pica_format::FragmentProgram fragment;
};

// The material constants' offsets the reduced programs read (SurfaceConstants).
inline constexpr std::uint32_t kReducedTintOffset = 0;
inline constexpr std::uint32_t kReducedSelfIllumTintOffset = 6 * 16;

// The reduced form of a variant. Its alphaTestReference (-1: none) becomes
// the GPU's alpha test.
ReducedPoint ReduceSurface( const SurfaceVariant &variant );

// The reduced alpha-test reference of a material's constants: $alphatest's
// reference as 0 to 255, or -1 without $alphatest.
int ReducedAlphaTest( const SurfaceConstants &constants );

// The PVS1 artifact of a reduced vertex program: empty outside 3DS builds,
// whose programs picasso assembles (families/pica/).
std::vector<std::byte> ReducedVertexArtifact( ReducedVertex vertex );

} // namespace render::material

#endif // RENDER_MATERIAL_SURFACE_REDUCED_H
