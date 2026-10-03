//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A fizzler's view-independent emission (RFC 0016 R91).
//          SolidEnergy retains the ordered translucent surface. Its flow
//          radiance, before camera opacity and fixed-point output, feeds one
//          two-sided rectangle fitted to the largest authored field face.
//          This first slice integrates the animated image to uniform radiance;
//          it does not claim spatially textured emission or bounced light.
//
//===========================================================================//

#ifndef RENDER_ENERGY_FIELD_H
#define RENDER_ENERGY_FIELD_H

#include "render/area_light.h"

#include <algorithm>
#include <cmath>

namespace energy_field
{

// A rectangular brush face in model space. No collision bounds are substituted.
// The engine supplies geometry/UVs; the client supplies the entity transform.
struct Surface
{
	float p[4][3] = {};
	float uv[4][2] = {};
	float tangentS[3] = {};
	float tangentT[3] = {};
	char material[128] = {};
};

struct Flow
{
	float time = 0.0f;
	float powerUp = 1.0f;
	float intensity = 1.0f;
	float outputIntensity = 1.0f;
	float worldUvScale = 1.0f;
	float normalUvScale = 1.0f;
	float noiseScale = 1.0f;
	float interval = 0.4f;
	float scrollDistance = 0.2f;
	float lerpExponent = 1.0f;
	float color[3] = {};
	float vortexColor[3] = {};
	float vortexSize = 1.0f;
	float vortex[2][3] = {};
	bool vortexEnabled[2] = {};
	bool cheap = false;
	bool detail1 = false;
	bool detail2 = false;
	int detail1Blend = 0;
	int detail2Blend = 0;
};

inline float Fract( float x )
{
	return x - std::floor( x );
}
inline float Smooth( float a, float b, float x )
{
	const float t = std::clamp( ( x - a ) / ( b - a ), 0.0f, 1.0f );
	return t * t * ( 3.0f - 2.0f * t );
}

// CPU oracle of render/shaders/common/energy_field.glsl. Camera opacity,
// camera fade, exposure, bloom and framebuffer encoding never enter this term.
inline void Reveal( float base[4], float noise, float edge, float powerUp )
{
	if ( !( powerUp > 0.0f && powerUp < 1.0f ) )
	{
		base[3] += edge;
		base[1] += edge;
		return;
	}
	const float reveal = ( noise + ( 1.0f - edge ) ) * 0.5f;
	const float stage = std::clamp( powerUp * 3.0f, 0.0f, 1.0f );
	const float pulse =
	    Smooth( 0.02f, 0.0f, std::fabs( reveal - powerUp ) ) * powerUp * ( 1.0f - powerUp );
	const float mask = stage * Smooth( 0.02f, 0.0f, reveal - powerUp );
	base[3] = ( base[3] + pulse ) * mask + edge * stage;
	base[1] = ( base[1] + pulse ) * mask + edge * stage;
}

inline void Radiance(
    const Flow &flow, const float base[4], float bounds, float vortexIntensity, float out[3] )
{
	for ( int k = 0; k < 3; ++k )
	{
		const float field = base[3] * flow.color[k];
		const float vortex = base[1] * flow.vortexColor[k];
		out[k] = ( field + ( vortex - field ) * vortexIntensity ) * bounds * flow.intensity *
		         flow.outputIntensity;
	}
}

// Refuse a nonrectangle rather than illuminating empty parts of its bounds.
[[nodiscard]] inline bool Rectangle( const Surface &surface, area_light::Rect &out )
{
	area_light::Rect rect;
	for ( int k = 0; k < 3; ++k )
	{
		for ( int c = 0; c < 4; ++c )
			if ( !std::isfinite( surface.p[c][k] ) )
				return false;
		rect.center[k] = 0.5f * ( surface.p[0][k] + surface.p[2][k] );
		rect.halfU[k] = 0.5f * ( surface.p[1][k] - surface.p[0][k] );
		rect.halfV[k] = 0.5f * ( surface.p[3][k] - surface.p[0][k] );
		if ( std::fabs( surface.p[2][k] - surface.p[1][k] - surface.p[3][k] + surface.p[0][k] ) >
		     0.01f )
			return false;
	}
	const float area = area_light::Area( rect );
	if ( !( area > 0.0f ) || !std::isfinite( area ) ||
	     std::fabs( area_light::detail::Dot( rect.halfU, rect.halfV ) ) > area * 1e-4f )
		return false;
	rect.twoSided = true;
	out = rect;
	return true;
}

// Sampler(texture 0 base, 1 flow, 2 noise, 3 bounds, 4/5 details, u, v, RGBA).
// Base RGB is linear; flow/noise/bounds are data, not sRGB. A missing required
// sample refuses the whole light. The source reports this failure by name.
template <typename Sampler>
[[nodiscard]] bool MeanLight( const Surface &surface, const Flow &flow, int samples, Sampler sample,
    area_light::AreaLight &out )
{
	out = {};
	area_light::AreaLight light;
	if ( !Rectangle( surface, light.rect ) || samples < 1 || samples > 64 ||
	     !( flow.interval > 0.0f ) || !std::isfinite( flow.time ) )
		return false;
	for ( float value : { flow.powerUp, flow.intensity, flow.outputIntensity, flow.worldUvScale,
	          flow.normalUvScale, flow.noiseScale, flow.interval, flow.scrollDistance,
	          flow.lerpExponent, flow.vortexSize } )
		if ( !std::isfinite( value ) )
			return false;
	for ( int k = 0; k < 3; ++k )
	{
		if ( !std::isfinite( surface.tangentS[k] ) || !std::isfinite( surface.tangentT[k] ) ||
		     !std::isfinite( flow.color[k] ) || !std::isfinite( flow.vortexColor[k] ) )
			return false;
		for ( int v = 0; v < 2; ++v )
			if ( flow.vortexEnabled[v] && !std::isfinite( flow.vortex[v][k] ) )
				return false;
	}
	for ( const auto &uv : surface.uv )
		if ( !std::isfinite( uv[0] ) || !std::isfinite( uv[1] ) )
			return false;
	auto read = [&]( int texture, float u, float v, float rgba[4] )
	{
		if ( !std::isfinite( u ) || !std::isfinite( v ) || !sample( texture, u, v, rgba ) )
			return false;
		for ( int k = 0; k < 4; ++k )
			if ( !std::isfinite( rgba[k] ) )
				return false;
		return true;
	};
	if ( !( flow.powerUp > 0.0f ) || !( flow.intensity > 0.0f ) )
	{
		out = light;
		return true;
	}
	for ( int y = 0; y < samples; ++y )
	{
		for ( int x = 0; x < samples; ++x )
		{
			const float s = ( float( x ) + 0.5f ) / float( samples );
			const float t = ( float( y ) + 0.5f ) / float( samples );
			float p[3], uv[2], flowUv[2];
			for ( int k = 0; k < 3; ++k )
				p[k] = surface.p[0][k] + 2.0f * s * light.rect.halfU[k] +
				       2.0f * t * light.rect.halfV[k];
			for ( int k = 0; k < 2; ++k )
				uv[k] = surface.uv[0][k] + s * ( surface.uv[1][k] - surface.uv[0][k] ) +
				        t * ( surface.uv[3][k] - surface.uv[0][k] );
			flowUv[0] = area_light::detail::Dot( p, surface.tangentS );
			flowUv[1] = area_light::detail::Dot( p, surface.tangentT );
			float bounds[4], noise[4], vector[4] = {};
			if ( !read( 3, uv[0], uv[1], bounds ) ||
			     !read( 2, flowUv[0] * flow.noiseScale, flowUv[1] * flow.noiseScale, noise ) ||
			     ( !flow.cheap && !read( 1, flowUv[0] * flow.worldUvScale,
			                          flowUv[1] * flow.worldUvScale, vector ) ) )
				return false;
			float velocity[2] = {
			    ( vector[0] * 2.0f - 1.0f ) * bounds[0], ( vector[1] * 2.0f - 1.0f ) * bounds[0] };
			float vortexIntensity = 0.0f;
			for ( int v = 0; v < 2; ++v )
			{
				if ( !flow.vortexEnabled[v] )
					continue;
				float delta[3];
				for ( int k = 0; k < 3; ++k )
					delta[k] = p[k] - flow.vortex[v][k];
				const float distance = area_light::detail::Length( delta );
				const float strength =
				    distance > 0.0f ? std::clamp( flow.vortexSize / distance - 0.5f, 0.0f, 1.0f )
				                    : 1.0f;
				vortexIntensity += strength;
				const float direction[2] = { area_light::detail::Dot( delta, surface.tangentS ),
				    area_light::detail::Dot( delta, surface.tangentT ) };
				const float length = std::hypot( direction[0], direction[1] );
				if ( !flow.cheap && length > 0.0f )
					for ( int k = 0; k < 2; ++k )
						velocity[k] += ( direction[k] / length - velocity[k] ) * strength * 0.5f;
			}
			const float phase = flow.time / ( flow.interval * 2.0f ) + noise[1];
			float base[4] = {};
			for ( int run = 0; run < 2; ++run )
			{
				const float offset = float( run ) * 0.5f;
				float weight = std::fabs( 2.0f * Fract( phase + 0.5f - offset ) - 1.0f );
				weight = flow.cheap ? weight * weight : std::pow( weight, flow.lerpExponent );
				const float scroll = ( Fract( phase + offset ) - 0.5f ) * flow.scrollDistance *
				                     ( 1.0f + vortexIntensity );
				const float shift =
				    flow.cheap ? offset : std::floor( phase + offset ) * 0.311f + offset;
				float texel[4];
				if ( !read( 0,
				         flowUv[0] * flow.normalUvScale + shift +
				             ( flow.cheap ? 0.0f : scroll * velocity[0] ),
				         flowUv[1] * flow.normalUvScale + shift +
				             ( flow.cheap ? 0.0f : scroll * velocity[1] ),
				         texel ) )
					return false;
				for ( int k = 0; k < 4; ++k )
					base[k] += texel[k] * weight;
			}
			Reveal( base, noise[1], bounds[1], flow.powerUp );
			float radiance[3];
			Flow unscaled = flow;
			unscaled.outputIntensity = 1.0f;
			Radiance( unscaled, base, bounds[2], vortexIntensity, radiance );
			float detail1[4] = {}, detail2[4] = {};
			if ( ( flow.detail1 && !read( 4, flowUv[0] * flow.normalUvScale,
			                           flowUv[1] * flow.normalUvScale, detail1 ) ) ||
			     ( flow.detail2 && !read( 5, flowUv[0] * flow.noiseScale,
			                           flowUv[1] * flow.noiseScale, detail2 ) ) )
				return false;
			for ( int k = 0; k < 3; ++k )
			{
				if ( flow.detail1 )
					radiance[k] *= flow.detail1Blend == 0
					                   ? 2.0f * detail1[k]
					                   : detail1[k] + ( 1.0f - detail1[k] ) * base[3];
				if ( flow.detail2 )
				{
					if ( flow.detail2Blend == 0 )
						radiance[k] += detail2[k] * ( flow.detail1 ? detail1[k] : 1.0f );
					else
						radiance[k] *= detail2[k];
				}
				radiance[k] = std::max( 0.0f, radiance[k] * flow.outputIntensity );
			}
			for ( int k = 0; k < 3; ++k )
			{
				if ( !std::isfinite( radiance[k] ) || radiance[k] < 0.0f )
					return false;
				light.radiance[k] += radiance[k] / float( samples * samples );
			}
		}
	}
	light.reach = area_light::Reach( light.rect, light.radiance );
	{
		out = light;
		return true;
	}
}

} // namespace energy_field

#endif // RENDER_ENERGY_FIELD_H
