//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.area-light.v1 - area lights (render/area_light.h) and the
//          emitters an emissive model publishes (render/emissive_area_
//          lights.h), RFC 0011 light set v2.
//
//          The contract's irradiance (Lambert's polygon formula, clipped to
//          the receiver's hemisphere) is judged against an independent
//          oracle: direct numerical integration of cos cos' / ( pi d^2 ) over
//          the rectangle, and closed forms for a receiver parallel to it. The
//          emitter fit is judged on its power, orientation, sidedness and
//          splitting; the selection on its budget, order and hysteresis. The
//          sensitivity builds substitute defective policies the oracle must
//          reject.
//
//===========================================================================//

#include "area_light_oracle.h"
#include "render/area_light.h"
#include "render/emissive_area_lights.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>

namespace
{
using namespace area_light;

unsigned long g_checks = 0;
unsigned long g_failures = 0;
#if defined( AREA_LIGHT_SEEDED_NO_CLIP ) || defined( AREA_LIGHT_SEEDED_TWO_SIDED ) ||              \
    defined( AREA_LIGHT_SEEDED_POINT ) || defined( AREA_LIGHT_SEEDED_NO_POWER ) ||                 \
    defined( AREA_LIGHT_SEEDED_NO_KEEP ) || defined( AREA_LIGHT_SEEDED_NO_SPLIT ) ||               \
    defined( AREA_LIGHT_SEEDED_NO_VIEW )
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

bool Near( double a, double b, double relative, double absolute = 1.0e-6 )
{
	return std::fabs( a - b ) <= std::max( absolute, relative * std::fabs( b ) );
}

// The policies under test: the real ones, or seeded defective ones.
float FormFactorUnderTest( const Rect &rect, const float p[3], const float n[3] )
{
#if defined( AREA_LIGHT_SEEDED_POINT )
	// A point light at the center: A cos cos' / ( pi d^2 ).
	float normal[3];
	Normal( rect, normal );
	const float d[3] = { rect.center[0] - p[0], rect.center[1] - p[1], rect.center[2] - p[2] };
	const float d2 = detail::Dot( d, d );
	const float len = std::sqrt( d2 );
	const float cosR = detail::Dot( d, n ) / len;
	const float cosE = -detail::Dot( d, normal ) / len;
	if ( cosR <= 0.0f || ( cosE <= 0.0f && !rect.twoSided ) )
		return 0.0f;
	return Area( rect ) * cosR * std::fabs( cosE ) / ( kPi * d2 );
#elif defined( AREA_LIGHT_SEEDED_NO_CLIP )
	if ( !Faces( rect, p ) )
		return 0.0f;
	float corners[4][3];
	Corners( rect, corners );
	float v[3];
	VectorFormFactor( corners, 4, p, v );
	const float f = detail::Dot( v, n );
	return f > 0.0f ? f : 0.0f;
#elif defined( AREA_LIGHT_SEEDED_TWO_SIDED )
	Rect both = rect;
	both.twoSided = true;
	return FormFactor( both, p, n );
#else
	return FormFactor( rect, p, n );
#endif
}

std::vector<emissive::Emitter> BuildUnderTest( const std::vector<emissive::Triangle> &triangles )
{
	std::vector<emissive::Emitter> emitters;
#if defined( AREA_LIGHT_SEEDED_NO_SPLIT )
	// One rectangle per group, however far its parts are apart.
	std::vector<const emissive::Triangle *> all;
	for ( const emissive::Triangle &t : triangles )
		all.push_back( &t );
	emissive::Emitter emitter;
	if ( !all.empty() && emissive::FitEmitter( all, emitter ) )
		emitters.push_back( emitter );
#else
	emitters = emissive::BuildEmitters( triangles );
#endif
#if defined( AREA_LIGHT_SEEDED_NO_POWER )
	// The mean radiance of the triangles, not their power over the rectangle.
	for ( emissive::Emitter &emitter : emitters )
	{
		double sum[3] = {}, count = 0;
		for ( const emissive::Triangle &t : triangles )
			if ( t.group == emitter.group )
			{
				for ( int k = 0; k < 3; ++k )
					sum[k] += t.radiance[k];
				++count;
			}
		for ( int k = 0; k < 3; ++k )
			emitter.radiance[k] = float( sum[k] / count );
	}
#endif
	return emitters;
}

void SelectUnderTest(
    const emissive::Candidate *candidates, int count, int budget, const float view[3], bool *lit )
{
#if defined( AREA_LIGHT_SEEDED_NO_KEEP )
	std::vector<emissive::Candidate> fresh( candidates, candidates + count );
	for ( emissive::Candidate &c : fresh )
		c.wasLit = false;
	emissive::SelectLit( fresh.data(), count, budget, view, lit );
#else
	emissive::SelectLit( candidates, count, budget, view, lit );
#endif
}

// Selection with the caller's view test; `asked` counts its calls, and
// `order` receives the lit candidates in rank order.
void SelectInViewUnderTest( const emissive::Candidate *candidates, int count, int budget,
    const float view[3], bool *lit, const bool *inView, int &asked,
    std::vector<int> *order = nullptr )
{
	asked = 0;
	auto test = [&]( int index )
	{
		++asked;
#if defined( AREA_LIGHT_SEEDED_NO_VIEW )
		// Every emitter ranked as if in view.
		(void)inView;
		(void)index;
		return true;
#else
		return inView[index];
#endif
	};
	emissive::SelectLit( candidates, count, budget, view, lit, test, order );
}

// The independent oracle (area_light_oracle.h).
double IntegratedFormFactor( const Rect &rect, const float p[3], const float n[3], int cells )
{
	return area_light_oracle::IntegratedFormFactor( rect, p, n, cells );
}

// The closed form for a receiver parallel to an a x b rectangle at height c
// whose normal passes through one corner.
double CornerFormFactor( double a, double b, double c )
{
	const double A = a / c, B = b / c;
	return ( A / std::sqrt( 1.0 + A * A ) * std::atan( B / std::sqrt( 1.0 + A * A ) ) +
	           B / std::sqrt( 1.0 + B * B ) * std::atan( A / std::sqrt( 1.0 + B * B ) ) ) /
	       ( 2.0 * kPi );
}

Rect MakeRect( float cx, float cy, float cz, float hu, float hv, bool twoSided = false )
{
	// In the plane z = cz, facing down (U x V = -z).
	Rect rect;
	rect.center[0] = cx;
	rect.center[1] = cy;
	rect.center[2] = cz;
	rect.halfU[1] = hu;
	rect.halfV[0] = hv;
	rect.twoSided = twoSided;
	return rect;
}

void AddQuad( std::vector<emissive::Triangle> &out, float x0, float y0, float x1, float y1, float z,
    float radiance, int group )
{
	emissive::Triangle a, b;
	const float corners[4][3] = { { x0, y0, z }, { x1, y0, z }, { x1, y1, z }, { x0, y1, z } };
	for ( int k = 0; k < 3; ++k )
	{
		a.p[0][k] = corners[0][k];
		a.p[1][k] = corners[1][k];
		a.p[2][k] = corners[2][k];
		b.p[0][k] = corners[0][k];
		b.p[1][k] = corners[2][k];
		b.p[2][k] = corners[3][k];
		a.radiance[k] = b.radiance[k] = radiance;
	}
	a.group = b.group = group;
	out.push_back( a );
	out.push_back( b );
}

double EmittedPower( const std::vector<emissive::Triangle> &triangles, int channel )
{
	double power = 0.0;
	for ( const emissive::Triangle &t : triangles )
	{
		float normal[3];
		power += double( emissive::detail::TriangleArea( t, normal ) ) * t.radiance[channel];
	}
	return power;
}

} // namespace

int main()
{
	const float down[3] = { 0.0f, 0.0f, -1.0f };
	const float up[3] = { 0.0f, 0.0f, 1.0f };

	// Parallel receivers: the closed form, centered and off-center.
	{
		const Rect rect = MakeRect( 0, 0, 10, 8, 5 );
		const float p[3] = { 0, 0, 0 };
		const double exact = 4.0 * CornerFormFactor( 16.0 / 2.0, 10.0 / 2.0, 10.0 );
		Check( Near( FormFactorUnderTest( rect, p, up ), exact, 1.0e-4 ),
		    "a centered parallel receiver takes the closed-form form factor" );
		const float q[3] = { 8, 5, 0 }; // under a corner (U runs along y, V along x)
		const float qq[3] = { 5, 8, 0 };
		Check( Near( FormFactorUnderTest( rect, qq, up ), CornerFormFactor( 10.0, 16.0, 10.0 ),
		           1.0e-4 ),
		    "a receiver under a corner takes the corner closed form" );
		(void)q;
	}

	// General placements against numerical integration.
	{
		bool agree = true;
		double worst = 0.0;
		const Rect rects[] = {
		    MakeRect( 0, 0, 30, 20, 6 ), MakeRect( 5, -3, 12, 2, 2 ), MakeRect( 0, 0, 4, 40, 40 ) };
		const float points[][3] = { { 0, 0, 0 }, { 25, 10, 0 }, { -40, 3, 20 }, { 3, 1, 2 } };
		const float normals[][3] = {
		    { 0, 0, 1 }, { 0.6f, 0, 0.8f }, { 1, 0, 0 }, { 0, -0.8f, 0.6f } };
		for ( const Rect &rect : rects )
			for ( const float *p : points )
				for ( const float *n : normals )
				{
					if ( p[2] >= rect.center[2] )
						continue;
					const double oracle = IntegratedFormFactor( rect, p, n, 600 );
					const double f = FormFactorUnderTest( rect, p, n );
					worst = std::max( worst, std::fabs( f - oracle ) );
					agree &= Near( f, oracle, 5.0e-3, 2.0e-5 );
				}
		std::fprintf( stderr, "form factor vs integration: worst |error| %.3g\n", worst );
		Check( agree, "the form factor matches numerical integration over tilted receivers" );
	}

	// A receiver perpendicular to the light, half its rectangle below its
	// horizon: only the part above counts (hemisphere clipping).
	{
		const Rect rect = MakeRect( 0, 0, 10, 10, 10 );
		const float p[3] = { 0, 0, 0 };
		const float side[3] = { 1, 0, 0 };
		const double oracle = IntegratedFormFactor( rect, p, side, 800 );
		Check( Near( FormFactorUnderTest( rect, p, side ), oracle, 5.0e-3 ),
		    "a perpendicular receiver takes only the part above its horizon" );
		const float wall[3] = { 20, 0, 10 }; // beside the light, in its plane
		Check( FormFactorUnderTest( rect, wall, side ) == 0.0f,
		    "a receiver in the light's own plane takes nothing" );
	}

	// Sidedness: a one-sided light lights only its front.
	{
		const Rect rect = MakeRect( 0, 0, 10, 5, 5 );
		const float above[3] = { 0, 0, 20 };
		Check( FormFactorUnderTest( rect, above, down ) == 0.0f,
		    "a one-sided light gives nothing behind it" );
		const Rect both = MakeRect( 0, 0, 10, 5, 5, true );
		const float below[3] = { 0, 0, 0 };
		Check( Near( FormFactorUnderTest( both, above, down ),
		           FormFactorUnderTest( both, below, up ), 1.0e-5 ),
		    "a two-sided light lights both sides alike" );
	}

	// Limits: an enormous light fills the hemisphere; far away it is a point.
	{
		const Rect huge = MakeRect( 0, 0, 1, 1.0e5f, 1.0e5f );
		const float p[3] = { 0, 0, 0 };
		Check( Near( FormFactorUnderTest( huge, p, up ), 1.0, 1.0e-3 ),
		    "a light filling the hemisphere has form factor 1" );
		const Rect small = MakeRect( 0, 0, 500, 2, 1 );
		Check( Near( FormFactorUnderTest( small, p, up ), 8.0 / ( kPi * 500.0 * 500.0 ), 1.0e-3 ),
		    "far away a light is A / ( pi d^2 )" );
	}

	// Near field: close to a light the point approximation is badly wrong and
	// the contract is not.
	{
		const Rect rect = MakeRect( 0, 0, 2, 12, 12 );
		const float p[3] = { 6, 0, 0 };
		const double oracle = IntegratedFormFactor( rect, p, up, 800 );
		Check( Near( FormFactorUnderTest( rect, p, up ), oracle, 5.0e-3 ),
		    "near a large light the form factor is the integral, not a point's" );
	}

	// The irradiance: radiance times form factor times window; the window is
	// 1 at the light, 0 at its reach, and falls monotonically.
	{
		AreaLight light;
		light.rect = MakeRect( 0, 0, 50, 10, 10 );
		light.radiance[0] = 1.0f;
		light.radiance[1] = 0.5f;
		light.radiance[2] = 0.25f;
		light.reach = Reach( light.rect, light.radiance );
		Check( Near( light.reach, std::sqrt( 400.0 / ( kPi * kReachThreshold ) ), 1.0e-4 ),
		    "the reach is where the axis irradiance falls to the threshold" );
		Check( Window( 0.0f, light.reach ) == 1.0f && Window( light.reach, light.reach ) == 0.0f,
		    "the window is 1 at the light and 0 at its reach" );
		bool monotonic = true;
		float last = 1.0f;
		for ( int i = 1; i <= 100; ++i )
		{
			const float w = Window( light.reach * i / 100.0f, light.reach );
			monotonic &= w <= last;
			last = w;
		}
		Check( monotonic, "the window falls monotonically" );
		const float p[3] = { 3, 4, 0 };
		float light3[3];
		IrradianceAt( light, p, up, light3 );
		const double f =
		    FormFactor( light.rect, p, up ) * Window( DistanceTo( light.rect, p ), light.reach );
		Check( Near( light3[0], f, 1.0e-6 ) && Near( light3[1], 0.5 * f, 1.0e-6 ) &&
		           Near( light3[2], 0.25 * f, 1.0e-6 ),
		    "the irradiance is radiance x form factor x window per channel" );
		AreaLight bright = light;
		bright.radiance[0] = 1000.0f;
		Check( Reach( bright.rect, bright.radiance ) == kMaxReach, "the reach is capped" );
	}

	// The model stand-in: a point light that gives the receiver, facing it,
	// the area light's full light, from the vector irradiance's direction.
	{
		AreaLight light;
		light.rect = MakeRect( 10, 0, 30, 8, 4 );
		light.radiance[0] = light.radiance[1] = light.radiance[2] = 2.0f;
		light.reach = Reach( light.rect, light.radiance );
		const float p[3] = { 0, 0, 0 };
		const Representative rep = RepresentativeAt( light, p );
		double d[3], d2 = 0.0;
		for ( int k = 0; k < 3; ++k )
		{
			d[k] = rep.position[k] - p[k];
			d2 += d[k] * d[k];
		}
		const double len = std::sqrt( d2 );
		const float toward[3] = { float( d[0] / len ), float( d[1] / len ), float( d[2] / len ) };
		const double stand = rep.intensityAt100[0] * kRepresentativeReferenceDistance *
		                     kRepresentativeReferenceDistance / d2;
		float exact[3];
		IrradianceAt( light, p, toward, exact );
		Check( rep.lit && Near( stand, exact[0], 2.0e-3 ),
		    "the stand-in gives the receiver facing it the area light's light" );
		// No other direction receives more from the area light.
		bool strongest = true;
		for ( int i = 0; i < 64; ++i )
		{
			const float a = 6.2831853f * i / 64.0f;
			float n[3] = { std::cos( a ) * 0.6f, std::sin( a ) * 0.6f, 0.8f };
			float other[3];
			IrradianceAt( light, p, n, other );
			strongest &= other[0] <= exact[0] * 1.0005f;
		}
		Check( strongest, "the stand-in's direction is the brightest one" );
		const float behind[3] = { 10, 0, 60 };
		Check( !RepresentativeAt( light, behind ).lit, "no stand-in behind a one-sided light" );
	}

	// Sampling: a constant texture samples to its value; a half-masked one to
	// the covered fraction.
	{
		const float uv[3][2] = { { 0, 0 }, { 1, 0 }, { 0, 1 } };
		float rgb[3];
		emissive::SampleTriangle(
		    uv, 64, 64,
		    []( float, float, float out[3] )
		    {
			    out[0] = 0.5f, out[1] = 0.25f, out[2] = 1.0f;
		    },
		    rgb );
		Check( Near( rgb[0], 0.5, 1e-6 ) && Near( rgb[1], 0.25, 1e-6 ) && Near( rgb[2], 1.0, 1e-6 ),
		    "a constant texture samples to its value" );
		// Emitting where u < 0.5: over the triangle u+v<1 that is 3/4 of its area.
		emissive::SampleTriangle(
		    uv, 64, 64,
		    []( float u, float, float out[3] )
		    {
			    out[0] = out[1] = out[2] = u < 0.5f ? 1.0f : 0.0f;
		    },
		    rgb );
		Check( Near( rgb[0], 0.75, 0.03 ), "a half-masked texture samples to the covered area" );
	}

	// Live surface image selection: source color and the unlit state must
	// affect receiver light, rather than only the visible surface.
	{
		const float rgb[3] = { 1, 2, 4 };
		float out[3];
		emissive::UnlitRadiance( rgb, 0, true, false, 0.5f, out );
		Check( out[0] == 0 && out[1] == 0 && out[2] == 0,
		    "transparent fullbright texels emit no light" );
		emissive::UnlitRadiance( rgb, 0.25f, true, false, 0.5f, out );
		Check( out[0] == 0.25f && out[1] == 0.5f && out[2] == 1,
		    "fullbright source power includes image coverage" );
		emissive::UnlitRadiance( rgb, 0.25f, false, true, 0.5f, out );
		Check(
		    out[0] == 0 && out[1] == 0 && out[2] == 0, "clipped fullbright texels emit no light" );
		std::vector<area_light::EmissiveTriangle> mapped( 2 );
		const float corners[4][3] = { { -4, -4, 0 }, { 4, -4, 0 }, { 4, 4, 0 }, { -4, 4, 0 } };
		const int indices[2][3] = { { 0, 1, 2 }, { 0, 2, 3 } };
		for ( int t = 0; t < 2; ++t )
			for ( int c = 0; c < 3; ++c )
			{
				std::copy_n( corners[indices[t][c]], 3, mapped[t].position[c] );
				mapped[t].uv[c][0] = ( mapped[t].position[c][0] + 4 ) / 8;
				mapped[t].uv[c][1] = ( mapped[t].position[c][1] + 4 ) / 8;
			}
		const float colors[3][3] = { { 0, 8, 16 }, { 16, 4, 0 }, { 0, 0, 0 } };
		float received[2][3] = {};
		for ( int frame = 0; frame < 3; ++frame )
		{
			const auto emitters = emissive::BuildMappedEmitters( mapped, 64, 64,
			    [&]( float, float, float out[3] )
			    {
				    std::copy_n( colors[frame], 3, out );
			    } );
			Check( emitters.size() == ( frame == 2 ? 0u : 1u ),
			    "mapped emitting frames fit; a dark frame publishes no emitter" );
			if ( frame < 2 && emitters.size() == 1 )
			{
				AreaLight light;
				light.rect = emitters[0].rect;
				std::copy_n( emitters[0].radiance, 3, light.radiance );
				light.reach = area_light::Reach( light.rect, light.radiance );
				const float p[3] = { 0, 0, 8 }, n[3] = { 0, 0, -1 };
				IrradianceAt( light, p, n, received[frame] );
			}
		}
		Check( received[0][0] == 0 && received[0][2] > received[0][1] && received[1][2] == 0 &&
		           received[1][0] > received[1][1] && Near( received[0][2], received[1][0], 1e-5 ),
		    "mapped cyan and orange frames cast their selected color and equal power" );
	}

	// Attachment sources use the authored front, independently of the view.
	{
		float transform[3][4] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 } };
		const float red[3] = { 1, 0, 0 };
		const auto on = emissive::AttachmentEmitter( transform, 1, red, 8 );
		Check( on && !on->rect.twoSided && Near( Area( on->rect ), 4, 1e-6 ) &&
		           on->radiance[0] == 8 && on->radiance[1] == 0 && on->radiance[2] == 0,
		    "attachment aperture retains reviewed size, radiance and one-sided front" );
		const float front[3] = { 8, 0, 0 }, back[3] = { -8, 0, 0 };
		Check( on && Faces( on->rect, front ) && !Faces( on->rect, back ),
		    "attachment front comes from its local X axis" );
		const auto dim = emissive::AttachmentEmitter( transform, 1, red, 4 );
		Check( dim && on && Near( dim->radiance[0], on->radiance[0] / 2, 1e-6 ),
		    "attachment brightness scales source radiance" );
		transform[0][3] = 20;
		transform[0][0] = -1;
		transform[1][1] = -1;
		const auto moved = emissive::AttachmentEmitter( transform, 1, red, 8 );
		const float movedFront[3] = { 12, 0, 0 };
		Check( moved && moved->rect.center[0] == 20 && Faces( moved->rect, movedFront ),
		    "moving and rotating an attachment changes its center and front" );
		Check( !emissive::AttachmentEmitter( transform, 1, red, 0 ),
		    "a hidden or dark attachment publishes no source" );
		Check( !emissive::AttachmentEmitter( transform, 0, red, 8 ),
		    "zero-size attachment apertures are refused" );
		transform[1][1] = 0;
		Check( !emissive::AttachmentEmitter( transform, 1, red, 8 ),
		    "degenerate attachment axes are refused" );
		transform[1][1] = std::nanf( "" );
		Check( !emissive::AttachmentEmitter( transform, 1, red, 8 ),
		    "nonfinite attachment geometry is refused" );
	}

	// The fit: a flat square keeps its size, facing and power.
	{
		std::vector<emissive::Triangle> triangles;
		AddQuad( triangles, 0, 0, 4, 4, 0, 0.8f, 0 );
		const std::vector<emissive::Emitter> emitters = BuildUnderTest( triangles );
		Check( emitters.size() == 1, "one small square is one emitter" );
		if ( emitters.size() == 1 )
		{
			const emissive::Emitter &e = emitters[0];
			float n[3];
			Normal( e.rect, n );
			Check( Near( e.rect.center[0], 2.0, 1e-4 ) && Near( e.rect.center[1], 2.0, 1e-4 ),
			    "the emitter sits on the square's center" );
			Check( Near( Area( e.rect ), 16.0, 1e-3 ), "a uniform square fits its own area" );
			Check( Near( n[2], 1.0, 1e-5 ) && !e.rect.twoSided,
			    "the square emits one-sided along its normal" );
			Check( Near( e.radiance[0] * Area( e.rect ), EmittedPower( triangles, 0 ), 1e-4 ),
			    "the emitter keeps the square's power" );
		}
	}

	// A sparse emitter (dots over a panel) keeps its power over a larger
	// rectangle: lower radiance, not the dots' radiance.
	{
		std::vector<emissive::Triangle> triangles;
		for ( int i = 0; i < 4; ++i )
			for ( int j = 0; j < 4; ++j )
				AddQuad( triangles, i * 5.0f, j * 5.0f, i * 5.0f + 1, j * 5.0f + 1, 0, 1.0f, 0 );
		const std::vector<emissive::Emitter> emitters = BuildUnderTest( triangles );
		double power = 0.0;
		for ( const emissive::Emitter &e : emitters )
			power += e.radiance[0] * Area( e.rect ) * ( e.rect.twoSided ? 2.0 : 1.0 );
		Check(
		    Near( power, EmittedPower( triangles, 0 ), 1e-3 ), "sparse emitters keep their power" );
	}

	// A closed shape (a cube) emits on every side: two-sided, power kept.
	{
		std::vector<emissive::Triangle> triangles;
		auto face = [&]( const float c[4][3] )
		{
			emissive::Triangle a, b;
			for ( int k = 0; k < 3; ++k )
			{
				a.p[0][k] = c[0][k], a.p[1][k] = c[1][k], a.p[2][k] = c[2][k];
				b.p[0][k] = c[0][k], b.p[1][k] = c[2][k], b.p[2][k] = c[3][k];
				a.radiance[k] = b.radiance[k] = 1.0f;
			}
			triangles.push_back( a );
			triangles.push_back( b );
		};
		const float faces[6][4][3] = { { { 0, 0, 0 }, { 0, 2, 0 }, { 2, 2, 0 }, { 2, 0, 0 } },
		    { { 0, 0, 2 }, { 2, 0, 2 }, { 2, 2, 2 }, { 0, 2, 2 } },
		    { { 0, 0, 0 }, { 2, 0, 0 }, { 2, 0, 2 }, { 0, 0, 2 } },
		    { { 0, 2, 0 }, { 0, 2, 2 }, { 2, 2, 2 }, { 2, 2, 0 } },
		    { { 0, 0, 0 }, { 0, 0, 2 }, { 0, 2, 2 }, { 0, 2, 0 } },
		    { { 2, 0, 0 }, { 2, 2, 0 }, { 2, 2, 2 }, { 2, 0, 2 } } };
		for ( const auto &f : faces )
			face( f );
		const std::vector<emissive::Emitter> emitters = BuildUnderTest( triangles );
		// Each face its own one-sided light, facing out of the cube.
		int outward = 0;
		double power = 0.0;
		for ( const emissive::Emitter &e : emitters )
		{
			float n[3];
			Normal( e.rect, n );
			const float out[3] = {
			    e.rect.center[0] - 1.0f, e.rect.center[1] - 1.0f, e.rect.center[2] - 1.0f };
			outward += !e.rect.twoSided && detail::Dot( n, out ) > 0.9f;
			power += e.radiance[0] * Area( e.rect ) * ( e.rect.twoSided ? 2.0 : 1.0 );
		}
		Check( emitters.size() == 6 && outward == 6,
		    "a closed shape is one outward light per facing" );
		Check(
		    Near( power, EmittedPower( triangles, 0 ), 1e-3 ), "a closed shape keeps its power" );
	}

	// Splitting: two emitting patches far apart in one group are two lights,
	// each on its own patch; groups never merge; dark triangles emit nothing.
	{
		std::vector<emissive::Triangle> triangles;
		AddQuad( triangles, 0, 0, 2, 2, 0, 1.0f, 0 );
		AddQuad( triangles, 100, 0, 102, 2, 0, 1.0f, 0 );
		AddQuad( triangles, 0, 50, 2, 52, 0, 1.0f, 1 );
		AddQuad( triangles, 0, -50, 40, -10, 0, 0.0f, 1 );
		const std::vector<emissive::Emitter> emitters = BuildUnderTest( triangles );
		int onFirst = 0, onSecond = 0, onThird = 0;
		for ( const emissive::Emitter &e : emitters )
		{
			onFirst += e.group == 0 && Near( e.rect.center[0], 1.0, 1e-3 );
			onSecond += e.group == 0 && Near( e.rect.center[0], 101.0, 1e-3 );
			onThird += e.group == 1 && Near( e.rect.center[1], 51.0, 1e-3 );
		}
		Check( emitters.size() == 3 && onFirst == 1 && onSecond == 1 && onThird == 1,
		    "distant patches split, groups stay apart, dark triangles are ignored" );
	}

	// Facing: two panels facing apart a few units from each other (the two
	// faces of a door) are two lights, each facing its own way; together they
	// keep the power.
	{
		std::vector<emissive::Triangle> triangles;
		AddQuad( triangles, 0, 0, 10, 10, 0, 1.0f, 0 ); // faces +z
		std::vector<emissive::Triangle> back;
		AddQuad( back, 0, 0, 10, 10, -4, 0.5f, 0 );
		for ( emissive::Triangle &t : back )
			std::swap( t.p[1], t.p[2] ); // faces -z
		triangles.insert( triangles.end(), back.begin(), back.end() );
		const std::vector<emissive::Emitter> emitters = BuildUnderTest( triangles );
		int up = 0, down = 0;
		double power = 0.0;
		for ( const emissive::Emitter &e : emitters )
		{
			float n[3];
			Normal( e.rect, n );
			up += !e.rect.twoSided && n[2] > 0.99f;
			down += !e.rect.twoSided && n[2] < -0.99f;
			power += e.radiance[0] * Area( e.rect ) * ( e.rect.twoSided ? 2.0 : 1.0 );
		}
		Check( up == 1 && down == 1, "panels facing apart are lights facing apart" );
		Check( Near( power, EmittedPower( triangles, 0 ), 1e-3 ),
		    "panels facing apart keep their power" );
	}

	// Selection: the budget, importance at the view, and hysteresis.
	{
		auto make = []( float x, float radiance, bool wasLit )
		{
			emissive::Candidate c;
			c.light.rect = MakeRect( x, 0, 50, 5, 5 );
			c.light.radiance[0] = c.light.radiance[1] = c.light.radiance[2] = radiance;
			c.light.reach = Reach( c.light.rect, c.light.radiance );
			c.wasLit = wasLit;
			return c;
		};
		const float view[3] = { 0, 0, 0 };
		const emissive::Candidate three[] = {
		    make( 400, 1.0f, false ), make( 0, 1.0f, false ), make( 100, 1.0f, false ) };
		bool lit[3];
		SelectUnderTest( three, 3, 2, view, lit );
		Check(
		    !lit[0] && lit[1] && lit[2], "the budget takes the lights most important at the view" );
		const emissive::Candidate close[] = { make( 0, 1.0f, true ), make( 0, 1.1f, false ) };
		bool litClose[2];
		SelectUnderTest( close, 2, 1, view, litClose );
		Check( litClose[0] && !litClose[1],
		    "a lit light keeps its light against a slightly stronger one" );
		const emissive::Candidate clear[] = { make( 0, 1.0f, true ), make( 0, 2.0f, false ) };
		bool litClear[2];
		SelectUnderTest( clear, 2, 1, view, litClear );
		Check( !litClear[0] && litClear[1], "a clearly stronger light takes it" );

		// A large emitter the view cannot see (a lava pit a floor below)
		// against a small one beside the view: the visible one is lit, and the
		// hidden one still takes budget left over.
		emissive::Candidate hidden;
		hidden.light.rect = MakeRect( 0, 0, 400, 60, 60 );
		hidden.light.radiance[0] = hidden.light.radiance[1] = hidden.light.radiance[2] = 0.35f;
		hidden.light.reach = Reach( hidden.light.rect, hidden.light.radiance );
		emissive::Candidate strip = make( 20, 0.3f, false );
		strip.light.rect = MakeRect( 20, 0, 10, 16, 1 );
		strip.light.reach = Reach( strip.light.rect, strip.light.radiance );
		const emissive::Candidate unseen[] = { hidden, strip };
		const bool unseenInView[] = { false, true };
		bool litUnseen[2];
		int asked = 0;
		SelectInViewUnderTest( unseen, 2, 1, view, litUnseen, unseenInView, asked );
		Check( !litUnseen[0] && litUnseen[1],
		    "an emitter in view is lit before a stronger one out of view" );
		std::vector<int> order;
		SelectInViewUnderTest( unseen, 2, 2, view, litUnseen, unseenInView, asked, &order );
		Check( litUnseen[0] && litUnseen[1], "an emitter out of view takes budget left over" );
		Check( order == std::vector<int>{ 1, 0 },
		    "the lit are listed most important first, those out of view last" );

		// The view test is asked in rank order only until the budget fills.
		const emissive::Candidate four[] = { make( 0, 1.0f, false ), make( 10, 1.0f, false ),
		    make( 20, 1.0f, false ), make( 30, 1.0f, false ) };
		const bool allInView[] = { true, true, true, true };
		bool litFour[4];
		SelectInViewUnderTest( four, 4, 2, view, litFour, allInView, asked );
		Check( asked == 2 && litFour[0] && litFour[1] && !litFour[2] && !litFour[3],
		    "the view test is asked only until the budget is filled" );
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
