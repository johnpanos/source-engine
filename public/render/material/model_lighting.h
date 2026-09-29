//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Source's model lighting as the legacy frontend supplies it
//			(RFC 0016 K11), the one owner of its packing until RFC 0016 K7's
//			light set replaces it: an ambient cube and up to four sorted local
//			lights, packed as CShaderAPIDx8::SetLight packs cLightInfo. The
//			surface program's draw group (surface_lighting.glsl) and the
//			vertexlit family's draw group read this block.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_MODEL_LIGHTING_H
#define RENDER_MATERIAL_MODEL_LIGHTING_H

#include <cstddef>
#include <cstdint>
#include <span>

namespace render::material
{

// A Source model light, as the legacy frontend describes it (LightDesc_t).
enum class ModelLightType : std::uint8_t
{
	kPoint,
	kDirectional,
	kSpot
};

struct ModelLightDesc
{
	ModelLightType type = ModelLightType::kPoint;
	float color[3] = {};
	float position[3] = {}; // a directional light's too: the vertex term reads it
	float direction[3] = { 0.0f, 0.0f, 1.0f };
	float attenuation[3] = { 1.0f, 0.0f, 0.0f }; // constant, linear, quadratic
	float theta = 0.0f;                          // spot inner cone angle (radians)
	float phi = 0.0f;                            // spot outer cone angle (radians)
	float falloff = 0.0f;                        // spot exponent
};

// The packed block (std140, surface_lighting.glsl and vertexlit.vert). A
// value-initialized block has no lights and a black cube: the neutral block
// a draw without model lighting binds.
struct ModelLighting
{
	struct Light
	{
		float color[4] = {};     // w: 1 for a directional light
		float direction[4] = {}; // w: 1 for a spot light
		float position[4] = {};
		float spot[4] = {};        // exponent, cos(theta / 2), cos(phi / 2), 1 / their difference
		float attenuation[4] = {}; // constant, linear, quadratic
	};
	float eye[4] = {}; // xyz: the eye position (legacy cEyePos); w: the number of lights
	float cube[6][4] = {};
	Light lights[4];
};
static_assert( sizeof( ModelLighting ) == 432 );

inline constexpr std::size_t kMaxModelLights = 4;

// Packs the eye, the ambient cube (+x, -x, +y, -y, +z, -z) and up to four
// lights as the legacy shader API does: lights sorted spot, point,
// directional (SortLights, stable), each as CShaderAPIDx8::SetLight builds
// cLightInfo. Lights past the fourth are dropped, as the port drops them.
ModelLighting PackSourceModelLighting(
    const float eye[3], const float cube[6][3], std::span<const ModelLightDesc> lights );

} // namespace render::material

#endif // RENDER_MATERIAL_MODEL_LIGHTING_H
