//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.projected-light.v1 - an env_projectedtexture as a light
//          (render/projected_light.h, RFC 0011): its frustum projection
//          against an independent matrix oracle (a world-to-light view matrix
//          times a perspective matrix, as the client builds the flashlight's
//          world-to-texture matrix), the legacy flashlight shader's
//          attenuation and end falloff, the Lambert term, the cookie, and the
//          frustum's bounding sphere. The sensitivity builds substitute
//          defective rules the oracle must reject.
//
//===========================================================================//

#include "render/projected_light.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{
// A small deterministic generator (the suite's inputs are fixed).
struct Rng
{
	unsigned long long state;
	explicit Rng( unsigned long long seed ) : state( seed * 6364136223846793005ull + 1442695040888963407ull ) {}
	float Uniform( float lo, float hi )
	{
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		return lo + ( hi - lo ) * float( ( state >> 40 ) & 0xffffff ) / float( 0x1000000 );
	}
};

using namespace projected_light;

unsigned long g_checks = 0;
unsigned long g_failures = 0;
#if defined( PROJECTED_LIGHT_SEEDED_NO_END_FALLOFF ) ||                                            \
    defined( PROJECTED_LIGHT_SEEDED_SWAPPED_FOV ) || defined( PROJECTED_LIGHT_SEEDED_NO_LAMBERT )
constexpr bool kSeeded = true;
#else
constexpr bool kSeeded = false;
#endif
unsigned long g_rejected = 0;

void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		if ( kSeeded )
		{
			++g_rejected;
			std::fprintf( stderr, "seeded defect detected: %s\n", description );
			return;
		}
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

// The rules under test: the real ones, or seeded defective ones.
bool ProjectUnderTest( const Light &light, const float p[3], float *u, float *v, float *depth )
{
#if defined( PROJECTED_LIGHT_SEEDED_SWAPPED_FOV )
	Light swapped = light;
	std::swap( swapped.horizontalFovDegrees, swapped.verticalFovDegrees );
	return Project( swapped, p, u, v, depth );
#else
	return Project( light, p, u, v, depth );
#endif
}

float AttenuationUnderTest( const Light &light, float distance )
{
#if defined( PROJECTED_LIGHT_SEEDED_NO_END_FALLOFF )
	float atten =
	    light.atten[0] + light.atten[1] / distance + light.atten[2] / ( distance * distance );
	return std::min( std::max( atten, 0.0f ), 1.0f );
#else
	return Attenuation( light, distance );
#endif
}

template <typename Cookie>
void IrradianceUnderTest(
    const Light &light, const float p[3], const float n[3], Cookie cookie, float out[3] )
{
#if defined( PROJECTED_LIGHT_SEEDED_NO_LAMBERT )
	const float up[3] = { light.origin[0] - p[0], light.origin[1] - p[1], light.origin[2] - p[2] };
	const float len = std::sqrt( up[0] * up[0] + up[1] * up[1] + up[2] * up[2] );
	const float toward[3] = { up[0] / len, up[1] / len, up[2] / len };
	(void)n;
	IrradianceAt( light, p, toward, cookie, out );
#else
	IrradianceAt( light, p, n, cookie, out );
#endif
}

// The oracle: 4x4 row-major matrices. World-to-light view (rows: right, up,
// forward, translated by the origin), then a D3D perspective (x' = x cot(h/2),
// y' = y cot(v/2), w = z), then the texture remap u = x'/w * 0.5 + 0.5,
// v = -y'/w * 0.5 + 0.5.
struct Mat4
{
	double m[4][4] = {};
};

Mat4 Multiply( const Mat4 &a, const Mat4 &b )
{
	Mat4 r;
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
			for ( int k = 0; k < 4; ++k )
				r.m[i][j] += a.m[i][k] * b.m[k][j];
	return r;
}

bool OracleProject( const Light &light, const float p[3], double *u, double *v, double *depth )
{
	Mat4 view;
	const float *rows[3] = { light.right, light.up, light.forward };
	for ( int r = 0; r < 3; ++r )
	{
		for ( int k = 0; k < 3; ++k )
			view.m[r][k] = rows[r][k];
		view.m[r][3] =
		    -( double( rows[r][0] ) * light.origin[0] + double( rows[r][1] ) * light.origin[1] +
		        double( rows[r][2] ) * light.origin[2] );
	}
	view.m[3][3] = 1.0;
	Mat4 perspective;
	perspective.m[0][0] =
	    1.0 / std::tan( 0.5 * light.horizontalFovDegrees * 3.14159265358979323846 / 180.0 );
	perspective.m[1][1] =
	    1.0 / std::tan( 0.5 * light.verticalFovDegrees * 3.14159265358979323846 / 180.0 );
	perspective.m[2][2] = 1.0;
	perspective.m[3][2] = 1.0; // w = z
	const Mat4 m = Multiply( perspective, view );
	double h[4] = {};
	for ( int i = 0; i < 4; ++i )
		h[i] = m.m[i][0] * p[0] + m.m[i][1] * p[1] + m.m[i][2] * p[2] + m.m[i][3];
	if ( !( h[3] > light.nearZ ) || h[3] > light.farZ )
		return false;
	const double x = h[0] / h[3], y = h[1] / h[3];
	if ( std::fabs( x ) > 1.0 || std::fabs( y ) > 1.0 )
		return false;
	*u = x * 0.5 + 0.5;
	*v = -y * 0.5 + 0.5;
	*depth = h[3];
	return true;
}

// A light at `origin` aimed along yaw/pitch, as the client builds its basis.
Light MakeLight( float x, float y, float z, float yawDeg, float pitchDeg, float hFov, float vFov )
{
	Light light;
	light.origin[0] = x;
	light.origin[1] = y;
	light.origin[2] = z;
	const float yaw = yawDeg * kPi / 180.0f, pitch = pitchDeg * kPi / 180.0f;
	const float f[3] = { std::cos( pitch ) * std::cos( yaw ), std::cos( pitch ) * std::sin( yaw ),
	    -std::sin( pitch ) };
	const float r[3] = { std::sin( yaw ), -std::cos( yaw ), 0.0f };
	const float u[3] = {
	    r[1] * f[2] - r[2] * f[1], r[2] * f[0] - r[0] * f[2], r[0] * f[1] - r[1] * f[0] };
	for ( int k = 0; k < 3; ++k )
	{
		light.forward[k] = f[k];
		light.right[k] = r[k];
		light.up[k] = u[k];
	}
	light.horizontalFovDegrees = hFov;
	light.verticalFovDegrees = vFov;
	light.nearZ = 4.0f;
	light.farZ = 800.0f;
	light.color[0] = light.color[1] = light.color[2] = 1.0f;
	return light;
}

} // namespace

int main()
{
	// Projection against the matrix oracle over random lights and points,
	// including unequal fields of view.
	{
		Rng rng( 42 );
		auto pos = []( Rng &r ) { return r.Uniform( -400.0f, 400.0f ); }; auto ang = []( Rng &r ) { return r.Uniform( -80.0f, 80.0f ); }; auto fov = []( Rng &r ) { return r.Uniform( 30.0f, 120.0f ); };
		int agree = 0, total = 0, inside = 0;
		double worst = 0.0;
		for ( int l = 0; l < 200; ++l )
		{
			const Light light = MakeLight( pos( rng ), pos( rng ), pos( rng ) * 0.25f,
			    ang( rng ) * 2.0f, ang( rng ), fov( rng ), fov( rng ) );
			for ( int i = 0; i < 200; ++i )
			{
				const float p[3] = { pos( rng ), pos( rng ), pos( rng ) };
				double ou = 0.0, ov = 0.0, od = 0.0;
				float u, v, d;
				const bool oracle = OracleProject( light, p, &ou, &ov, &od );
				const bool test = ProjectUnderTest( light, p, &u, &v, &d );
				// Points within 1e-4 of the frustum's edge are ambiguous in float.
				if ( oracle &&
				     ( std::fabs( ou - 0.5 ) > 0.4999 || std::fabs( ov - 0.5 ) > 0.4999 ) )
					continue;
				++total;
				inside += oracle;
				if ( oracle != test )
					continue;
				if ( oracle )
				{
					const double error = std::max( std::fabs( ou - u ),
					    std::max( std::fabs( ov - v ), std::fabs( od - d ) / od ) );
					worst = std::max( worst, error );
					if ( error > 1e-4 )
						continue;
				}
				++agree;
			}
		}
		std::fprintf( stderr, "projection: %d of %d agree (%d inside), worst %.2g\n", agree, total,
		    inside, worst );
		Check(
		    inside > 1000 && agree == total, "the frustum projection matches the matrix oracle" );
	}

	// The flashlight shader's attenuation: saturate( c + l/d + q/d^2 ) times the
	// end falloff (1 until 0.6 far, 0 at far).
	{
		Light light = MakeLight( 0, 0, 0, 0, 0, 90, 90 );
		light.atten[0] = 0.0f;
		light.atten[1] = 100.0f;
		light.atten[2] = 0.0f;
		light.farZ = 1000.0f;
		bool matches = true;
		for ( float d = 1.0f; d < 1200.0f; d += 7.0f )
		{
			float expected = std::min( 1.0f, 100.0f / d );
			const float end =
			    std::min( 1.0f, std::max( 0.0f, ( d - 1000.0f ) / ( 600.0f - 1000.0f ) ) );
			expected *= end;
			matches &= std::fabs( AttenuationUnderTest( light, d ) - expected ) < 1e-5f;
		}
		Check( matches, "attenuation is the shader's, with its end falloff" );
		Check( AttenuationUnderTest( light, 1000.0f ) == 0.0f &&
		           AttenuationUnderTest( light, 600.0f ) > 0.16f,
		    "the light fades out between 0.6 far and far" );
	}

	// Irradiance: color x cookie x attenuation x Lambert; nothing outside the
	// frustum or behind the surface.
	{
		Light light = MakeLight( 0, 0, 200, 0, 90, 60, 60 ); // straight down
		light.color[0] = 2.0f;
		light.color[1] = 1.0f;
		light.color[2] = 0.5f;
		const float up[3] = { 0, 0, 1 };
		auto half = []( float, float, float rgb[3] )
		{
			rgb[0] = rgb[1] = rgb[2] = 0.5f;
		};
		float out[3];
		const float below[3] = { 0, 0, 0 };
		IrradianceUnderTest( light, below, up, half, out );
		const float expected = 0.5f * std::min( 1.0f, 100.0f / 200.0f );
		Check( std::fabs( out[0] - 2.0f * expected ) < 1e-5f &&
		           std::fabs( out[2] - 0.5f * expected ) < 1e-5f,
		    "the light under the projector is color x cookie x attenuation" );
		const float tilted[3] = { 0.6f, 0, 0.8f };
		IrradianceUnderTest( light, below, tilted, half, out );
		Check( std::fabs( out[1] - 0.8f * expected ) < 1e-5f,
		    "a tilted surface takes the Lambert cosine" );
		const float away[3] = { 0, 0, -1 };
		IrradianceUnderTest( light, below, away, half, out );
		Check( out[0] == 0.0f, "a surface facing away takes nothing" );
		const float outside[3] = { 300, 0, 0 };
		IrradianceUnderTest( light, outside, up, half, out );
		Check( out[0] == 0.0f, "a point outside the frustum takes nothing" );
		// The cookie is sampled where the point projects: left of center is u < 0.5.
		float seenU = -1.0f;
		auto record = [&seenU]( float u, float, float rgb[3] )
		{
			seenU = u;
			rgb[0] = rgb[1] = rgb[2] = 1.0f;
		};
		const float onRight[3] = {
		    0, -50, 0 }; // right of a down-facing light aimed with yaw 0 is -y
		IrradianceUnderTest( light, onRight, up, record, out );
		Check( seenU > 0.6f, "the cookie is sampled at the point's place in the frustum" );
	}

	// The bounding sphere holds the whole frustum.
	{
		const Light light = MakeLight( 10, 20, 30, 40, 20, 100, 50 );
		float center[3], radius;
		BoundingSphere( light, center, &radius );
		const float tanH = std::tan( 50.0f * kPi / 180.0f ),
		            tanV = std::tan( 25.0f * kPi / 180.0f );
		bool holds = true;
		for ( int s = -1; s <= 1; s += 2 )
			for ( int t = -1; t <= 1; t += 2 )
				for ( float z : { light.nearZ, light.farZ } )
				{
					float p[3];
					for ( int k = 0; k < 3; ++k )
						p[k] =
						    light.origin[k] + z * ( light.forward[k] + s * tanH * light.right[k] +
						                              t * tanV * light.up[k] );
					const float d[3] = { p[0] - center[0], p[1] - center[1], p[2] - center[2] };
					holds &=
					    std::sqrt( d[0] * d[0] + d[1] * d[1] + d[2] * d[2] ) <= radius * 1.0001f;
				}
		Check( holds, "the bounding sphere holds the frustum's corners" );
	}

	if ( kSeeded )
	{
		std::fprintf( stderr, "%lu seeded rejection(s)\n", g_rejected );
		if ( g_rejected == 0 )
		{
			std::fprintf( stderr, "FAIL: the seeded defect was not detected\n" );
			return testing::ReportConformance( g_checks, g_checks );
		}
		return testing::ReportConformance( g_checks, 0 );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
