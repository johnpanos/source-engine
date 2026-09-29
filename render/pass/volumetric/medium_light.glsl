// render.pass.volumetric: the lights' light at a point of the medium, and the
// medium itself (RFC 0016 "Lighting model", participating media).
//
// A runtime light's falloff and spot cone are render.light-set.v1's
// (public/render/light_set.h: Falloff, InverseSquareFalloff, SpotFactor),
// through their one GLSL copy, render/shaders/common/runtime_light.glsl, as
// the surface program's clustered lights take them. The projector's rule is
// a GLSL mirror of projected_light::Project and Attenuation
// (public/render/projected_light.h, render.projected-light.v1) with no
// Lambert term (a medium has no normal); it moves to render/shaders/common
// with the surface program's projected-light term. The suite's oracle
// evaluates each independently.
//
// SEEDED_PHASE_IGNORED and SEEDED_ALBEDO_IGNORED build the defective variants
// render.lab.volumetric's sensitivity row must reject; no product uses them.

#include "../../shaders/common/runtime_light.glsl"

const float kPi = 3.14159265358979;

struct FogVolume
{
	vec4 minsExtinction;
	vec4 maxsAlbedo;
	vec4 emissionAnisotropy;
};

struct MediumLight
{
	vec4 positionKind;      // w 0 point, 1 spot
	vec4 colorFalloff;      // w 0 inverse square, 1 legacy
	vec4 direction;         // spot axis
	vec4 cone;              // innerCos, outerCos, radius, sourceRadius
	vec4 misc;              // minLight
};

struct MediumProjector
{
	vec4 origin; // w cookie layer
	vec4 forward;
	vec4 right;
	vec4 up;
	vec4 frustum; // tan half horizontal, tan half vertical, near, far
	vec4 color;
	vec4 atten;
};

// The light's diffuse light at normal incidence at `p` (lightmap unit),
// unshadowed and unattenuated by the medium.
vec3 LightAt( MediumLight light, vec3 p )
{
	const vec3 toPoint = p - light.positionKind.xyz;
	const float distanceSquared = dot( toPoint, toPoint );
	float falloff =
	    light.colorFalloff.w > 0.5
	        ? RuntimeLightFalloffLegacy( distanceSquared, light.cone.z, light.misc.x )
	        : RuntimeLightFalloffInverseSquare( distanceSquared, light.cone.z, light.cone.w );
	if ( light.positionKind.w > 0.5 )
	{
		const float cosAxis =
		    dot( toPoint * inversesqrt( max( distanceSquared, 1e-30 ) ), light.direction.xyz );
		falloff *= RuntimeLightSpot( cosAxis, light.cone.x, light.cone.y );
	}
	return light.colorFalloff.rgb * falloff;
}

// projected_light::Project: the cookie coordinates and depth of `p`, or false.
bool ProjectorProject( MediumProjector projector, vec3 p, out vec2 uv, out float distance )
{
	const vec3 d = p - projector.origin.xyz;
	const float z = dot( d, projector.forward.xyz );
	uv = vec2( 0.0 );
	distance = length( d );
	if ( !( z > projector.frustum.z ) || z > projector.frustum.w )
		return false;
	const float x = dot( d, projector.right.xyz ) / ( z * projector.frustum.x );
	const float y = dot( d, projector.up.xyz ) / ( z * projector.frustum.y );
	if ( x < -1.0 || x > 1.0 || y < -1.0 || y > 1.0 )
		return false;
	uv = vec2( 0.5 + 0.5 * x, 0.5 - 0.5 * y );
	return true;
}

// projected_light::Attenuation.
float ProjectorAttenuation( MediumProjector projector, float distance )
{
	if ( !( distance > 0.0 ) )
		return 0.0;
	const vec3 a = projector.atten.xyz;
	const float atten = clamp( a.x + a.y / distance + a.z / ( distance * distance ), 0.0, 1.0 );
	const float far = projector.frustum.w;
	const float end = far > 0.0 ? clamp( ( distance - far ) / ( 0.6 * far - far ), 0.0, 1.0 ) : 0.0;
	return atten * end;
}

// Henyey-Greenstein: the fraction per steradian scattered through an angle
// whose cosine (the light's travel against the scattered direction) is c.
float HenyeyGreenstein( float g, float c )
{
#ifdef SEEDED_PHASE_IGNORED
	return 1.0 / ( 4.0 * kPi );
#else
	const float g2 = g * g;
	const float denominator = max( 1.0 + g2 - 2.0 * g * c, 1e-8 );
	return ( 1.0 - g2 ) / ( 4.0 * kPi * denominator * sqrt( denominator ) );
#endif
}

// The segment [a, b]'s length inside an axis-aligned box.
float SegmentInBox( vec3 a, vec3 b, vec3 lo, vec3 hi )
{
	const vec3 d = b - a;
	float t0 = 0.0;
	float t1 = 1.0;
	for ( int k = 0; k < 3; ++k )
	{
		if ( abs( d[k] ) < 1e-20 )
		{
			if ( a[k] < lo[k] || a[k] > hi[k] )
				return 0.0;
			continue;
		}
		float ta = ( lo[k] - a[k] ) / d[k];
		float tb = ( hi[k] - a[k] ) / d[k];
		t0 = max( t0, min( ta, tb ) );
		t1 = min( t1, max( ta, tb ) );
	}
	return max( t1 - t0, 0.0 ) * length( d );
}

bool InBox( vec3 p, vec3 lo, vec3 hi )
{
	return all( greaterThanEqual( p, lo ) ) && all( lessThanEqual( p, hi ) );
}

// The height fog's extinction at height z.
float HeightExtinction( float z )
{
	return params.fog0.y * exp( -params.fog0.z * ( z - params.fog0.w ) );
}

// The integral of the height fog's extinction along [a, b].
float HeightOpticalDepth( vec3 a, vec3 b )
{
	const float length_ = length( b - a );
	if ( params.fog0.y == 0.0 )
		return 0.0;
	const float falloff = params.fog0.z;
	const float dz = b.z - a.z;
	if ( abs( falloff * dz ) < 1e-5 )
		return HeightExtinction( 0.5 * ( a.z + b.z ) ) * length_;
	return params.fog0.y * length_ *
	       ( exp( -falloff * ( a.z - params.fog0.w ) ) - exp( -falloff * ( b.z - params.fog0.w ) ) ) /
	       ( falloff * dz );
}
