//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Projected lights (render.projected-light.v1, RFC 0011 light set
//          v2): an env_projectedtexture as a light like any other. It shines
//          a cookie texture through a perspective frustum, and every consumer
//          lights with it by the same rule the legacy flashlight shader used
//          (common_flashlight_fxc.h DoFlashlight):
//
//              light = color x cookie( uv ) x atten x endFalloff x max(0, n.L)
//              atten = saturate( c + l / d + q / d^2 )
//              endFalloff: 1 up to 0.6 far, falling linearly to 0 at far
//
//          with d the distance to the light, uv the point's position in the
//          frustum (Project), and nothing outside the frustum or nearer than
//          its near plane. Units: the lightmap unit, as the flashlight pass
//          added it to what a surface draws.
//
//          Unlike the legacy pass, projected lights are shadowed: by the
//          world (consumers trace it) and by moving objects
//          (render/dynamic_occlusion.h, a disk of kSourceRadius).
//
//          Tier0-free C++ shared by the client, the engine and the conformance
//          suite (unittests/rendertest/test_projected_light.cpp).
//
//===========================================================================//

#ifndef RENDER_PROJECTED_LIGHT_H
#define RENDER_PROJECTED_LIGHT_H

#include <cmath>

namespace projected_light
{

constexpr float kPi = 3.14159265358979323846f;
// The size the light shadows with (a projector's lens).
constexpr float kSourceRadius = 4.0f;
// At most this many projected lights at once.
constexpr int kMaxProjectedLights = 16;

struct Light
{
	float origin[3] = {};
	float forward[3] = { 1, 0, 0 }; // unit basis of the frustum
	float right[3] = { 0, -1, 0 };
	float up[3] = { 0, 0, 1 };
	float horizontalFovDegrees = 90.0f;
	float verticalFovDegrees = 90.0f;
	float nearZ = 4.0f;
	float farZ = 750.0f;
	float color[3] = {};                     // linear, brightness and style folded in
	float atten[3] = { 0.0f, 100.0f, 0.0f }; // constant, linear, quadratic
	char cookie[128] = {};                   // the cookie texture's name
	int cookieFrame = 0;
	bool shadows = true;
	bool lightsWorld = true; // false: models only (env_projectedtexture lightworld 0)
};

namespace detail
{
inline float Dot( const float a[3], const float b[3] )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
} // namespace detail

// Where a point falls in the frustum: its cookie coordinates (0..1 across the
// frustum, u along right, v down from up) and its distance along forward.
// False outside the frustum or nearer than the near plane.
inline bool Project( const Light &light, const float p[3], float *u, float *v, float *depth )
{
	const float d[3] = { p[0] - light.origin[0], p[1] - light.origin[1], p[2] - light.origin[2] };
	const float z = detail::Dot( d, light.forward );
	if ( !( z > light.nearZ ) || z > light.farZ )
		return false;
	const float tanH = std::tan( 0.5f * light.horizontalFovDegrees * kPi / 180.0f );
	const float tanV = std::tan( 0.5f * light.verticalFovDegrees * kPi / 180.0f );
	const float x = detail::Dot( d, light.right ) / ( z * tanH );
	const float y = detail::Dot( d, light.up ) / ( z * tanV );
	if ( x < -1.0f || x > 1.0f || y < -1.0f || y > 1.0f )
		return false;
	*u = 0.5f + 0.5f * x;
	*v = 0.5f - 0.5f * y;
	*depth = z;
	return true;
}

// The distance attenuation and end falloff at distance d (the shader's).
[[nodiscard]] inline float Attenuation( const Light &light, float distance )
{
	if ( !( distance > 0.0f ) )
		return 0.0f;
	float atten =
	    light.atten[0] + light.atten[1] / distance + light.atten[2] / ( distance * distance );
	atten = atten < 0.0f ? 0.0f : ( atten > 1.0f ? 1.0f : atten );
	// RemapValClamped( d, far, 0.6 far, 0, 1 )
	const float far = light.farZ;
	float end = far > 0.0f ? ( distance - far ) / ( 0.6f * far - far ) : 0.0f;
	end = end < 0.0f ? 0.0f : ( end > 1.0f ? 1.0f : end );
	return atten * end;
}

// The light a point with unit normal n receives, the cookie sampled by
// `cookie( u, v, rgb )` (the contract's oracle, unshadowed).
template <typename Cookie>
void IrradianceAt(
    const Light &light, const float p[3], const float n[3], Cookie cookie, float out[3] )
{
	out[0] = out[1] = out[2] = 0.0f;
	float u, v, depth;
	if ( !Project( light, p, &u, &v, &depth ) )
		return;
	float l[3] = { light.origin[0] - p[0], light.origin[1] - p[1], light.origin[2] - p[2] };
	const float distance = std::sqrt( detail::Dot( l, l ) );
	for ( float &c : l )
		c /= distance;
	const float lambert = detail::Dot( l, n );
	if ( !( lambert > 0.0f ) )
		return;
	float rgb[3];
	cookie( u, v, rgb );
	const float scale = Attenuation( light, distance ) * lambert;
	for ( int k = 0; k < 3; ++k )
		out[k] = light.color[k] * rgb[k] * scale;
}

// The sphere around the lit part of the frustum (for bounding its surfaces).
inline void BoundingSphere( const Light &light, float center[3], float *radius )
{
	const float tanH = std::tan( 0.5f * light.horizontalFovDegrees * kPi / 180.0f );
	const float tanV = std::tan( 0.5f * light.verticalFovDegrees * kPi / 180.0f );
	const float far = light.farZ;
	const float halfDiagonal = far * std::sqrt( tanH * tanH + tanV * tanV );
	for ( int k = 0; k < 3; ++k )
		center[k] = light.origin[k] + light.forward[k] * 0.5f * far;
	*radius = std::sqrt( 0.25f * far * far + halfDiagonal * halfDiagonal );
}

// The client's projected lights for the frame, published to the engine (plain
// data), keyed by the entity handle of each env_projectedtexture.
static const char *const kProjectedLightsVersion = "VEngineProjectedLights001";

class IProjectedLights
{
public:
	// Replaces the frame's projected lights (at most kMaxProjectedLights).
	virtual void SetProjectedLights( const Light *lights, const int *keys, int count ) = 0;

protected:
	~IProjectedLights() {}
};

} // namespace projected_light

#endif // RENDER_PROJECTED_LIGHT_H
