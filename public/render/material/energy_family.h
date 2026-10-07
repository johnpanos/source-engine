//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SolidEnergy on the core (RFC 0016 K12, R91): Portal 2's fizzlers,
//			light bridges, tractor beams and laser planes as the surface
//			program's energy point (SurfaceVariant::energy).
//
//			ClaimEnergy packs solidenergy_dx9_helper.cpp's combos and
//			constants: a flow field ($flowmap with its noise and bounds; the
//			flow radiance is render.energy-field.v1, energy_field.glsl) or a
//			base with up to two detail layers, the tangent and fresnel
//			opacity terms, power-up and two vortices. The combos the helper
//			picks per draw (ACTIVE, POWERUP, VORTEX1/2) are uniform flags,
//			so proxies that change $powerup, $flow_color_intensity or the
//			vortices change no pipeline. solidenergy_vs20's coordinates are
//			evaluated per pixel from the world vertex's position and tangent
//			frame. A material missing a texture its combo samples is refused
//			by name.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_ENERGY_FAMILY_H
#define RENDER_MATERIAL_ENERGY_FAMILY_H

#include "render/material/unlit_family.h"

namespace render::material
{

// The combo flags of SurfaceConstants::energy[9].x (surface_program.glsl's
// kEnergy* constants).
enum EnergyFlag : std::uint32_t
{
	kEnergyAdditive = 1,
	kEnergyDetail1 = 2,
	kEnergyDetail2 = 4,
	kEnergyTangentT = 8,
	kEnergyTangentS = 16,
	kEnergyFresnel = 32,
	kEnergyVertexColor = 64,
	kEnergyFlowMap = 128,
	kEnergyModelFormat = 256,
	kEnergyFlowCheap = 512,
	kEnergyPowerUp = 1024,
	kEnergyVortex1 = 2048,
	kEnergyVortex2 = 4096,
};

// The claim's textures: the base, $detail1, $detail2 and the flow field's
// three (SurfaceTextures' energy names) are filled by the resolver.
UnlitClaim ClaimEnergy( const ParameterBlock &block );

} // namespace render::material

#endif // RENDER_MATERIAL_ENERGY_FAMILY_H
