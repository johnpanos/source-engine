//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `water` material family (RFC 0016): its claim on a Water
//			material and the packing of a parameter block into the surface
//			program's constants. Its draws are the water point of the one
//			surface program (surface_program.h: kSurfaceWater), which owns the
//			bind groups and pipelines.
//
//			The point is Portal 2's water_ps2x (the CS:GO water_ps2x.fxc that
//			has Portal 2's parameter set):
//			- the normal: two layers of $normalmap scrolled along the flow
//			  map's vectors ($flowmap, $flow_*), phased apart by the noise
//			  texture, or $normalmap at the surface's coordinates without a
//			  flow map;
//			- with $basetexture and a flow map, the sludge: two layers of the
//			  base texture scrolled the same way ($color_flow_*), displaced by
//			  the normal ($color_flow_displacebynormalstrength), masked by the
//			  flow map's alpha, lit by the lightmap;
//			- the water fog color ($fogcolor on the sRGB curve), times the
//			  lightmap with $lightmapwaterfog; the sludge covers it by its
//			  alpha;
//			- the reflection: the view's reflection target ($reflecttexture,
//			  the image the client's reflection view drew, offset by the
//			  normal times $reflectamount), or with $forceenvmap the env map
//			  in the reflected direction; times $reflecttint; mixed in by the
//			  fresnel term (0.2 + 0.8 (1 - N.V)^5, or $forcefresnel) where the
//			  sludge is not above the water;
//			- with $refracttexture (REFRACT): the view's refraction target
//			  ($refracttexture, which the client's refraction view drew with
//			  the water fog's depth factor in alpha), offset by the normal
//			  times $refractamount; above water that depth scales the
//			  offsets, tints by $refracttint at the water's edge, fogs the
//			  refraction toward the (lightmapped) fog color, dims the
//			  reflection and the fresnel term, and a warped texel of near-zero
//			  depth (something in front of the water) falls back to the
//			  unwarped one; below water ($abovewater 0) the refraction is
//			  added to the fresnel-weighted reflection as it is;
//			- the output alpha is $waterblendfactor; the view's range fog
//			  follows, and no tone-map scale is applied to the reflection
//			  (TONEMAP_SCALE_NONE).
//			Constants follow the CS:GO water.cpp's DrawReflectionRefraction,
//			which has Portal 2's parameter set (retail's stdshader_dx9 names
//			$color_flow_displacebynormalstrength and $forceenvmap and no
//			$flow_timescale).
//
//			ClaimWater names what the point does not draw: water seen from
//			below without refraction, the cheap path (no reflection target,
//			refraction target or forced env map, or $forcecheap), a base texture
//			without a flow map (bumped-lightmap water), $flow_debug views,
//			multi-textured normals ($scroll1) and a $bumptransform. $flowmap-
//			scrollrate is read and, as retail's shader does, unused;
//			$flow_timescale is not a retail parameter and is ignored as
//			retail ignores it. $flashlighttint is accepted: the flashlight on
//			water is a projected light of the frame, which the world pass's
//			BSP surfaces do not take yet (open, as for every core surface).
//
//=============================================================================//

#ifndef RENDER_MATERIAL_WATER_FAMILY_H
#define RENDER_MATERIAL_WATER_FAMILY_H

#include "render/device/device.h"
#include "render/material/parameter_block.h"
#include "render/material/surface_program.h"

#include <string>

namespace render::material
{

struct WaterClaim
{
	bool claimed = false;
	std::string reason; // why not, when !claimed
	// $waterblendfactor below one blends (Portal 2's $pseudotranslucent
	// water); else opaque.
	device::BlendMode blend = device::BlendMode::kOpaque;
	bool reflectTarget = false; // the reflection is $reflecttexture (else the env map)
	bool refractTarget = false; // $refracttexture, the view's refraction target
	bool envReflection = false; // no reflection target: the env map reflects (when named)
	bool sludge = false;        // $basetexture with a flow map
	bool flow = false;          // a flow map
	// The water constants (SurfaceConstants::water*).
	SurfaceConstants constants;

	SurfaceVariant Variant() const
	{
		return { blend, blend == device::BlendMode::kOpaque, kSurfaceWater, 0,
		    SurfaceVertexLayout::kWorld };
	}
};

// Whether the family draws this block's material, and how. The block must be
// of the `water` family's schema (FamiliesFromMapping).
WaterClaim ClaimWater( const ParameterBlock &block );

} // namespace render::material

#endif // RENDER_MATERIAL_WATER_FAMILY_H
