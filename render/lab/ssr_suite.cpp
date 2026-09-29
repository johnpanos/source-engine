//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite ssr (RFC 0016 K11 "Screen-space reflections",
//			render.ssr.v1). This slice: the CPU reference
//			(ssr_reference.h) judged on analytic scenes (ssr_scene.h) against
//			their true mirror reflections, so the reference the GPU pass will
//			be judged by is itself judged first:
//			- a mirror floor before a patterned wall with a thin bar: the
//			  rays that stop early are exactly the screen-space ambiguity set,
//			  computed from the analytic scene (render.ssr.v1, "The ambiguity
//			  set"): every clear pixel whose true reflection the camera sees
//			  is hit within 1.5 pixels of it and reflects its light within 3
//			  percent + 0.01, and every ambiguous one stops early; a seeded
//			  reference with the thickness in front of the surface fails the
//			  comparison;
//			- the thickness test is exercised: rays whose true reflection is
//			  the wall but whose screen path crosses the bar are stopped by
//			  it only when nothing is too thick;
//			- reflections whose true point is off the screen do not hit;
//			- hits near the screen's edge fade: at most edgeFade from it the
//			  confidence is below 1, and within a hundredth of the screen
//			  below 0.1;
//			- the roughness fade: at 0.35 (between 0.75 and 1 of the 0.4
//			  cutoff) the confidence is the fade's value times the edge and
//			  thickness terms, and at 0.45 every pixel is its lit value,
//			  bitwise; rough rectangles and the background are unchanged,
//			  bitwise;
//			- the octahedral encoding returns unit vectors within 1e-6.
//			Tolerances were fixed before the first run.
//
//=============================================================================//

#include "lab_suite.h"
#include "lab_support.h"
#include "ssr_reference.h"
#include "ssr_scene.h"
#include "suites.h"

#include "render/device/device.h"
#include "render/math/matrix.h"
#include "render/pass/ssr/ssr.h"
#include "render/resources/texture_cache.h"
#include "spv/ssr_variants_spv.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <random>
#include <string>

namespace render::lab
{

namespace
{

using math::float3;

constexpr int kFloor = 0;
constexpr int kWall = 1;
constexpr int kBar = 2;

std::array<float, 3> WallRadiance( const float3 &p )
{
	return { 0.5f + 0.4f * std::sin( p.y / 40.0f ), 0.5f + 0.4f * std::cos( p.z / 30.0f ), 0.2f };
}

// A mirror floor (z = 0) before a patterned wall (x = 600) with two thin
// bars floating over it (x = 300, 20 to 30 units up, and x = 150, 10 to 25
// units up), seen from 48 units up. Rays that pass under a bar rise behind
// it on the screen.
SsrScene MirrorScene( float floorRoughness )
{
	SsrScene scene;
	scene.eye = { 0.0f, 0.0f, 48.0f };
	scene.target = { 400.0f, 0.0f, 0.0f };
	SsrQuad floor;
	floor.corner = { -100.0f, -400.0f, 0.0f };
	floor.u = { 1100.0f, 0.0f, 0.0f };
	floor.v = { 0.0f, 800.0f, 0.0f };
	floor.normal = { 0.0f, 0.0f, 1.0f };
	floor.roughness = floorRoughness;
	floor.weight = { 0.9f, 0.8f, 0.7f };
	floor.iblRadiance = { 0.06f, 0.05f, 0.04f };
	floor.radiance = []( const float3 & )
	{
		return std::array<float, 3>{ 0.1f, 0.1f, 0.1f };
	};
	SsrQuad wall;
	wall.corner = { 600.0f, -400.0f, 0.0f };
	wall.u = { 0.0f, 800.0f, 0.0f };
	wall.v = { 0.0f, 0.0f, 400.0f };
	wall.normal = { -1.0f, 0.0f, 0.0f };
	wall.radiance = WallRadiance;
	SsrQuad bar;
	bar.corner = { 300.0f, -60.0f, 20.0f };
	bar.u = { 0.0f, 120.0f, 0.0f };
	bar.v = { 0.0f, 0.0f, 10.0f };
	bar.normal = { -1.0f, 0.0f, 0.0f };
	bar.radiance = []( const float3 &p )
	{
		return std::array<float, 3>{ 0.9f, 0.1f + p.z / 600.0f, 0.1f };
	};
	// A second, nearer bar, where the screen walk resolves the ambiguity.
	SsrQuad nearBar = bar;
	nearBar.corner = { 150.0f, -40.0f, 10.0f };
	nearBar.u = { 0.0f, 80.0f, 0.0f };
	nearBar.v = { 0.0f, 0.0f, 15.0f };
	nearBar.radiance = []( const float3 &p )
	{
		return std::array<float, 3>{ 0.1f, 0.2f + p.z / 100.0f, 0.9f };
	};
	scene.quads = { floor, wall, bar, nearBar };
	return scene;
}

std::string Text( const char *format, double a, double b = 0, double c = 0, double d = 0 )
{
	char text[256];
	std::snprintf( text, sizeof( text ), format, a, b, c, d );
	return text;
}

enum class Ambiguity
{
	kClear,
	kAmbiguous,
	kUnclassified,
};

// The screen-space ambiguity of a pixel's true reflected ray (render.ssr.v1,
// "The ambiguity set"): whether it passes behind another rectangle by less
// than the thickness on its way to its true reflection.
Ambiguity Classify( const SsrScene &scene, const SsrSceneImages &images, std::size_t index,
    const pass::ssr::SsrParams &params, const math::float4x4 &fromClip )
{
	const SsrReferenceInputs &in = images.inputs;
	const SsrTruth &truth = images.truth[index];
	const std::uint32_t W = in.width, H = in.height;
	const double px = double( index % W ) + 0.5, py = double( index / W ) + 0.5;
	const float depth = in.depth[index];
	const auto world = [&]( double sx, double sy, double z )
	{
		return math::TransformPoint(
		    fromClip, { float( sx / W * 2.0 - 1.0 ), float( 1.0 - sy / H * 2.0 ), float( z ) } );
	};
	const auto clipOf = [&]( const float3 &p )
	{
		return math::Transform( in.toClip, { p.x, p.y, p.z, 1.0f } );
	};
	const SsrQuad &own = scene.quads[std::size_t( truth.quad )];
	const float3 P = images.points[index];
	const float3 right = world( px + 1.0, py, depth );
	const float3 here = world( px, py, depth );
	const float footprint = std::sqrt( ( right.x - here.x ) * ( right.x - here.x ) +
	                                   ( right.y - here.y ) * ( right.y - here.y ) +
	                                   ( right.z - here.z ) * ( right.z - here.z ) );
	const float3 O{ P.x + own.normal.x * footprint, P.y + own.normal.y * footprint,
	    P.z + own.normal.z * footprint };
	// The ray from O (the definition's, parallel to the true ray from P) to
	// the plane of the rectangle it truly hits.
	const float3 view{ scene.eye.x - P.x, scene.eye.y - P.y, scene.eye.z - P.z };
	const float viewLength = std::sqrt( view.x * view.x + view.y * view.y + view.z * view.z );
	const float3 v{ view.x / viewLength, view.y / viewLength, view.z / viewLength };
	const float nv = own.normal.x * v.x + own.normal.y * v.y + own.normal.z * v.z;
	const float3 R{ 2.0f * nv * own.normal.x - v.x, 2.0f * nv * own.normal.y - v.y,
	    2.0f * nv * own.normal.z - v.z };
	const SsrQuad &target = scene.quads[std::size_t( truth.reflectedQuad )];
	const float denominator = R.x * target.normal.x + R.y * target.normal.y + R.z * target.normal.z;
	const float t = ( ( target.corner.x - O.x ) * target.normal.x +
	                    ( target.corner.y - O.y ) * target.normal.y +
	                    ( target.corner.z - O.z ) * target.normal.z ) /
	                denominator;
	const float3 end{ O.x + R.x * t, O.y + R.y * t, O.z + R.z * t };
	const math::float4 cO = clipOf( O ), cX = clipOf( end );
	const double o[3] = { cO.x / cO.w, cO.y / cO.w, cO.z / cO.w };
	const double x[3] = { cX.x / cX.w, cX.y / cX.w, cX.z / cX.w };
	const auto screen = [&]( const double n[3], double &sx, double &sy )
	{
		sx = ( n[0] + 1.0 ) * 0.5 * W;
		sy = ( 1.0 - n[1] ) * 0.5 * H;
	};
	double sOx, sOy, sXx, sXy;
	screen( o, sOx, sOy );
	screen( x, sXx, sXy );
	const long originTexelX = long( std::floor( sOx ) ), originTexelY = long( std::floor( sOy ) );
	const double length = std::hypot( sXx - sOx, sXy - sOy );
	const int steps = std::max( 1, int( std::ceil( length / 0.05 ) ) );

	// The clip w of rectangle q's plane under a screen point.
	const auto planeW = [&]( int q, double sx, double sy )
	{
		const SsrQuad &quad = scene.quads[std::size_t( q )];
		const float3 nearPoint = world( sx, sy, 0.0 );
		const float3 farPoint = world( sx, sy, 1.0 );
		const float3 dir{
		    farPoint.x - nearPoint.x, farPoint.y - nearPoint.y, farPoint.z - nearPoint.z };
		const float along =
		    ( ( quad.corner.x - nearPoint.x ) * quad.normal.x +
		        ( quad.corner.y - nearPoint.y ) * quad.normal.y +
		        ( quad.corner.z - nearPoint.z ) * quad.normal.z ) /
		    ( dir.x * quad.normal.x + dir.y * quad.normal.y + dir.z * quad.normal.z );
		const float3 hit{
		    nearPoint.x + dir.x * along, nearPoint.y + dir.y * along, nearPoint.z + dir.z * along };
		return double( clipOf( hit ).w );
	};
	bool ambiguous = false, unsure = false;
	double previousRay = 0.0;
	for ( int k = 0; k <= steps; ++k )
	{
		const double u = double( k ) / steps;
		const double n[3] = {
		    o[0] + ( x[0] - o[0] ) * u, o[1] + ( x[1] - o[1] ) * u, o[2] + ( x[2] - o[2] ) * u };
		double sx, sy;
		screen( n, sx, sy );
		const double wRay = 1.0 / double( math::Transform( fromClip,
		                              { float( n[0] ), float( n[1] ), float( n[2] ), 1.0f } )
		                                  .w );
		const double rayPerPixel =
		    k > 0 ? std::fabs( wRay - previousRay ) / ( length / steps ) : 0.0;
		previousRay = wRay;
		if ( long( std::floor( sx ) ) == originTexelX && long( std::floor( sy ) ) == originTexelY )
			continue;
		// Every rectangle other than the true one seen at the step or one
		// pixel from it: a texel the step crosses may hold any of them.
		int seen[9];
		int count = 0;
		for ( int dy = -1; dy <= 1; ++dy )
			for ( int dx = -1; dx <= 1; ++dx )
			{
				float3 point;
				seen[count++] = CastCamera( scene, in, fromClip, sx + dx, sy + dy, point );
			}
		for ( int candidate = 0; candidate < 9; ++candidate )
		{
			const int q = seen[candidate];
			bool repeated = q < 0 || q == truth.reflectedQuad;
			for ( int earlier = 0; earlier < candidate && !repeated; ++earlier )
				repeated = seen[earlier] == q;
			if ( repeated )
				continue;
			const bool interior = std::all_of( seen, seen + 9,
			    [q]( int s )
			    {
				    return s == q;
			    } );
			// The rectangle's plane under the step and one pixel along each
			// axis.
			const double wSeen = planeW( q, sx, sy );
			const double gradient = std::fabs( planeW( q, sx + 1.0, sy ) - wSeen ) +
			                        std::fabs( planeW( q, sx, sy + 1.0 ) - wSeen );
			// Half the seen plane's w change per pixel (both axes summed), the
			// ray's, and 1e-3 of w.
			const double delta = 0.5 * gradient + rayPerPixel + 1e-3 * wRay;
			const double behind = wRay - wSeen;
			const double T = params.thickness;
			if ( interior )
			{
				if ( behind >= delta && behind <= T - delta )
					ambiguous = true;
				else if ( std::fabs( behind ) < delta || std::fabs( behind - T ) < delta )
					unsure = true;
			}
			else if ( behind > -delta && behind < T + delta )
				unsure = true;
		}
	}
	if ( ambiguous )
		return Ambiguity::kAmbiguous;
	return unsure ? Ambiguity::kUnclassified : Ambiguity::kClear;
}

// The exact-set check (render.ssr.v1 S1): clear pixels hit their true
// reflection within 1.5 pixels, ambiguous ones stop early. Returns the
// violations and describes the first.
struct SetOutcome
{
	std::size_t judged = 0, ambiguous = 0, unclassified = 0, violations = 0;
	std::string first;
};

SetOutcome CompareSets( const SsrSceneImages &images, const std::vector<SsrReferencePixel> &pixels,
    const std::vector<Ambiguity> &classes )
{
	SetOutcome outcome;
	const std::uint32_t W = images.inputs.width;
	for ( std::size_t i = 0; i < pixels.size(); ++i )
	{
		const SsrTruth &truth = images.truth[i];
		if ( truth.quad != kFloor || truth.reflectedQuad < 0 || !truth.reflectedVisible )
			continue;
		++outcome.judged;
		const SsrReferencePixel &p = pixels[i];
		const bool correct =
		    p.hit && std::hypot( p.hitX - truth.reflectedX, p.hitY - truth.reflectedY ) <= 1.5;
		const Ambiguity c = classes[i];
		outcome.ambiguous += c == Ambiguity::kAmbiguous ? 1 : 0;
		outcome.unclassified += c == Ambiguity::kUnclassified ? 1 : 0;
		const bool violation =
		    ( c == Ambiguity::kClear && !correct ) || ( c == Ambiguity::kAmbiguous && correct );
		if ( violation && outcome.violations++ == 0 )
			outcome.first = Text( "pixel %g,%g ", double( i % W ), double( i / W ) ) +
			                ( c == Ambiguity::kClear ? "(clear): " : "(ambiguous): " ) +
			                ( p.hit ? Text( "hit at %.2f,%.2f", p.hitX, p.hitY ) : "no hit" ) +
			                Text( ", true %.2f,%.2f", truth.reflectedX, truth.reflectedY );
	}
	return outcome;
}

void MirrorChecks( Results &results )
{
	const pass::ssr::SsrParams params;
	const SsrScene scene = MirrorScene( 0.05f );
	const SsrSceneImages images = RayCastScene( scene );
	const std::vector<SsrReferencePixel> pixels = ReferenceSsr( images.inputs, params );
	pass::ssr::SsrParams unbounded = params;
	unbounded.thickness = 1e9f;
	const std::vector<SsrReferencePixel> thick = ReferenceSsr( images.inputs, unbounded );
	const std::vector<SsrReferencePixel> inFront =
	    ReferenceSsr( images.inputs, params, SsrReferenceDefect::kThicknessInFront );
	const std::uint32_t W = images.inputs.width, H = images.inputs.height;
	const math::float4x4 fromClip = *math::Inverse( images.inputs.toClip );

	std::vector<Ambiguity> classes( pixels.size(), Ambiguity::kClear );
	for ( std::size_t i = 0; i < pixels.size(); ++i )
	{
		const SsrTruth &truth = images.truth[i];
		if ( truth.quad == kFloor && truth.reflectedQuad >= 0 && truth.reflectedVisible )
			classes[i] = Classify( scene, images, i, params, fromClip );
	}
	const SetOutcome sets = CompareSets( images, pixels, classes );
	results.That( sets.judged > 500 && sets.ambiguous > 0 && sets.violations == 0,
	    "reference.mirror.stops-are-the-ambiguity-set",
	    Text( "%g judged, %g ambiguous, %g unclassified, %g violations", double( sets.judged ),
	        double( sets.ambiguous ), double( sets.unclassified ), double( sets.violations ) ) +
	        ( sets.first.empty() ? "" : "; first " + sets.first ) );
	const SetOutcome control = CompareSets( images, inFront, classes );
	results.That( control.violations > 0, "reference.mirror.catches-thickness-in-front",
	    Text( "%g violations with the thickness in front", double( control.violations ) ) );

	std::size_t radianceFailures = 0, offScreen = 0, offScreenHits = 0, passedBehind = 0,
	            unchangedFailures = 0, edgeBand = 0, edgeFailures = 0;
	std::string radianceFirst, offScreenFirst, unchangedFirst, edgeFirst;
	const auto locate = [&]( const SsrReferencePixel &p )
	{
		return images.truth[std::size_t( p.hitTexelY ) * W + p.hitTexelX].quad;
	};
	for ( std::uint32_t y = 0; y < H; ++y )
	{
		for ( std::uint32_t x = 0; x < W; ++x )
		{
			const std::size_t i = std::size_t( y ) * W + x;
			const SsrTruth &truth = images.truth[i];
			const SsrReferencePixel &p = pixels[i];
			const float *lit = &images.inputs.lit[i * 4];
			if ( truth.quad != kFloor )
			{
				if ( std::memcmp( p.out, lit, sizeof( p.out ) ) != 0 && unchangedFailures++ == 0 )
					unchangedFirst = Text( "pixel %g,%g changed", x, y );
				continue;
			}
			if ( truth.reflectedQuad >= 0 && !truth.reflectedOnScreen )
			{
				++offScreen;
				if ( p.hit && offScreenHits++ == 0 )
					offScreenFirst = Text( "pixel %g,%g hit %g,%g", x, y, p.hitX, p.hitY );
				continue;
			}
			// Rays whose true reflection is the wall but whose screen path
			// crosses the bar: not stopped by it with the default thickness,
			// stopped when nothing is too thick.
			if ( truth.reflectedQuad == kWall && thick[i].hit && locate( thick[i] ) == kBar &&
			     !( p.hit && locate( p ) == kBar ) )
				++passedBehind;
			if ( truth.reflectedQuad < 0 || !truth.reflectedVisible ||
			     classes[i] != Ambiguity::kClear || !p.hit )
				continue;
			// The light a clear pixel reflects.
			const std::array<float, 3> expected =
			    scene.quads[std::size_t( truth.reflectedQuad )].radiance( truth.reflectedPoint );
			for ( int c = 0; c < 3; ++c )
			{
				if ( std::fabs( p.reflected[c] - expected[std::size_t( c )] ) >
				         0.03 * std::fabs( expected[std::size_t( c )] ) + 0.01 &&
				     radianceFailures++ == 0 )
					radianceFirst =
					    Text( "pixel %g,%g channel %g: %.4f", x, y, c, p.reflected[c] ) +
					    Text( " against %.4f", expected[std::size_t( c )] );
			}
			// The edge band.
			const double edgeDistance =
			    std::min( { p.hitX / W, 1.0 - p.hitX / W, p.hitY / H, 1.0 - p.hitY / H } );
			if ( edgeDistance < params.edgeFade )
			{
				++edgeBand;
				const bool ok =
				    p.confidence < 1.0 && ( edgeDistance >= 0.01 || p.confidence < 0.1 );
				if ( !ok && edgeFailures++ == 0 )
					edgeFirst = Text(
					    "pixel %g,%g: confidence %.3f at %.4f", x, y, p.confidence, edgeDistance );
			}
		}
	}
	results.That( radianceFailures == 0, "reference.mirror.reflects-its-light",
	    Text( "%g channels off", double( radianceFailures ) ) +
	        ( radianceFirst.empty() ? "" : "; first " + radianceFirst ) );
	results.That( passedBehind > 0, "reference.thickness.passes-behind-the-bar",
	    Text( "%g rays reach the wall past the bar", double( passedBehind ) ) );
	results.That( offScreen > 0 && offScreenHits == 0, "reference.off-screen-misses",
	    Text( "%g of %g hit", double( offScreenHits ), double( offScreen ) ) +
	        ( offScreenFirst.empty() ? "" : "; first " + offScreenFirst ) );
	results.That( edgeBand > 0 && edgeFailures == 0, "reference.edge-fades",
	    Text( "%g in the band, %g wrong", double( edgeBand ), double( edgeFailures ) ) +
	        ( edgeFirst.empty() ? "" : "; first " + edgeFirst ) );
	results.That( unchangedFailures == 0, "reference.rough-and-background-unchanged-bitwise",
	    unchangedFirst );
}

void RoughnessChecks( Results &results )
{
	const pass::ssr::SsrParams params;
	// Between the fade's start (0.3) and the cutoff (0.4).
	{
		const SsrSceneImages images = RayCastScene( MirrorScene( 0.35f ) );
		const std::vector<SsrReferencePixel> pixels = ReferenceSsr( images.inputs, params );
		const double t = ( 0.35 - 0.3 ) / ( 0.4 - 0.3 );
		const double fade = 1.0 - t * t * ( 3.0 - 2.0 * t );
		std::size_t hits = 0, wrong = 0;
		for ( const SsrReferencePixel &p : pixels )
		{
			if ( !p.hit )
				continue;
			++hits;
			if ( p.confidence > fade + 1e-6 )
				++wrong;
		}
		results.That( hits > 0 && wrong == 0, "reference.roughness.fades",
		    Text( "%g of %g hits above the fade %.3f", double( wrong ), double( hits ), fade ) );
	}
	// Above the cutoff: nothing traced, every pixel its lit value.
	{
		const SsrSceneImages images = RayCastScene( MirrorScene( 0.45f ) );
		const std::vector<SsrReferencePixel> pixels = ReferenceSsr( images.inputs, params );
		std::size_t changed = 0, traced = 0;
		for ( std::size_t i = 0; i < pixels.size(); ++i )
		{
			traced += pixels[i].traced ? 1 : 0;
			if ( std::memcmp( pixels[i].out, &images.inputs.lit[i * 4], sizeof( float ) * 4 ) != 0 )
				++changed;
		}
		results.That( changed == 0 && traced == 0,
		    "reference.roughness.above-cutoff-unchanged-bitwise",
		    Text( "%g changed, %g traced", double( changed ), double( traced ) ) );
	}
}

void OctahedralChecks( Results &results )
{
	std::mt19937 random( 20260929u );
	std::normal_distribution<float> gauss( 0.0f, 1.0f );
	double worst = 0.0;
	for ( int i = 0; i < 10000; ++i )
	{
		float v[3] = { gauss( random ), gauss( random ), gauss( random ) };
		const float length = std::sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
		for ( float &c : v )
			c /= length;
		float back[3];
		pass::ssr::OctDecode( pass::ssr::OctEncode( v[0], v[1], v[2] ), back );
		for ( int c = 0; c < 3; ++c )
			worst = std::max( worst, double( std::fabs( back[c] - v[c] ) ) );
	}
	results.That( worst <= 1e-6, "reference.octahedral-round-trip", Text( "worst %.3g", worst ) );
}

// ---------------------------------------------------------------------------
// The GPU pass against the reference (render.ssr.v1 S5, S6).

// The inputs as the GPU reads them: the half-float ones rounded to half.
SsrReferenceInputs AsUploaded( const SsrReferenceInputs &inputs )
{
	SsrReferenceInputs out = inputs;
	for ( std::vector<float> *image : { &out.lit, &out.iblRadiance, &out.specularWeight } )
		for ( float &v : *image )
			v = HalfToFloat( FloatToHalf( v ) );
	return out;
}

struct GpuResult
{
	std::vector<float> output;      // RGBA per pixel
	std::vector<float> diagnostics; // two vec4 per pixel
};

std::optional<std::string> RunGpu( device::IRenderDevice2 &device,
    pass::ssr::ScreenSpaceReflections &pass, const SsrReferenceInputs &in, GpuResult &result )
{
	using namespace render::device;
	const std::uint32_t W = in.width, H = in.height;
	const std::size_t pixels = std::size_t( W ) * H;
	resources::TextureCache cache( device );
	auto stage = [&]( const char *name, Format format, std::span<const std::byte> bytes )
	{
		TextureDesc desc;
		desc.format = format;
		desc.width = W;
		desc.height = H;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		auto staged = cache.Stage( name, desc, bytes );
		return staged ? staged.Value().texture : TextureId();
	};
	auto halves = [&]( const std::vector<float> &values )
	{
		std::vector<std::uint16_t> out( values.size() );
		for ( std::size_t i = 0; i < values.size(); ++i )
			out[i] = FloatToHalf( values[i] );
		return out;
	};
	const std::vector<std::uint16_t> lit = halves( in.lit ), ibl = halves( in.iblRadiance ),
	                                 weight = halves( in.specularWeight );
	pass::ssr::SsrDirectTargets targets;
	targets.depth = stage( "depth", Format::kR32Float, std::as_bytes( std::span( in.depth ) ) );
	targets.normalRoughness = stage( "normal-roughness", Format::kRGBA32Float,
	    std::as_bytes( std::span( in.normalRoughness ) ) );
	targets.iblRadiance = stage( "ibl", Format::kRGBA16Float, std::as_bytes( std::span( ibl ) ) );
	targets.specularWeight =
	    stage( "weight", Format::kRGBA16Float, std::as_bytes( std::span( weight ) ) );
	targets.lit = stage( "lit", Format::kRGBA16Float, std::as_bytes( std::span( lit ) ) );
	targets.width = W;
	targets.height = H;
	TextureDesc outputDesc;
	outputDesc.format = Format::kRGBA16Float;
	outputDesc.width = W;
	outputDesc.height = H;
	outputDesc.usages = { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource };
	auto output = device.CreateTexture( outputDesc );
	auto buffer = [&]( std::uint64_t size ) -> BufferId
	{
		BufferDesc desc;
		desc.size = size;
		desc.usages = UsageSet( { ResourceUsage::kCopyDestination } );
		desc.memory = MemoryKind::kReadback;
		auto made = device.CreateBuffer( desc );
		return made ? made.Value() : BufferId();
	};
	const std::uint64_t outputBytes = pixels * 8, diagnosticBytes = pixels * 32;
	const BufferId outputReadback = buffer( outputBytes );
	const BufferId diagnosticReadback = buffer( diagnosticBytes );
	if ( !output || !targets.depth.IsValid() || !targets.normalRoughness.IsValid() ||
	     !targets.iblRadiance.IsValid() || !targets.specularWeight.IsValid() ||
	     !targets.lit.IsValid() || !outputReadback.IsValid() || !diagnosticReadback.IsValid() )
		return std::string( "an SSR resource was refused" );
	targets.output = output.Value();

	auto encoded = device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoded )
		return std::string( "no encoder" );
	CommandEncoder &encoder = encoded.Value();
	cache.RecordUploads( encoder );
	encoder.TransitionTexture(
	    targets.output, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
	pass::ssr::SsrView view;
	view.toClip = in.toClip;
	std::copy( in.eye, in.eye + 3, view.eye );
	if ( !pass.Record( encoder, targets, view ) )
		return std::string( "the pass did not record" );
	encoder.TransitionTexture(
	    targets.output, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
	encoder.TransitionBuffer(
	    outputReadback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.CopyTextureToBuffer( targets.output, outputReadback, { 0, 0, 0, W, H } );
	encoder.TransitionBuffer(
	    pass.Diagnostics(), ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
	encoder.TransitionBuffer(
	    diagnosticReadback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.CopyBuffer( pass.Diagnostics(), diagnosticReadback, { 0, 0, diagnosticBytes } );
	auto token = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	if ( !token )
		return std::string( "the SSR frame was refused at submission" );
	(void)device.WaitIdle();
	pass.Collect( token.Value() );
	cache.Retire( token.Value() );
	std::vector<std::byte> bytes( outputBytes );
	std::vector<std::byte> diagnostics( diagnosticBytes );
	const bool read = bool( device.ReadBuffer( outputReadback, 0, bytes ) ) &&
	                  bool( device.ReadBuffer( diagnosticReadback, 0, diagnostics ) );
	for ( ResourceId id : { ResourceId( outputReadback ), ResourceId( diagnosticReadback ),
	          ResourceId( targets.output ) } )
		(void)device.Release( id, token.Value() );
	if ( !read )
		return std::string( "the SSR frame did not read back" );
	result.output.resize( pixels * 4 );
	for ( std::size_t i = 0; i < pixels * 4; ++i )
	{
		std::uint16_t half;
		std::memcpy( &half, bytes.data() + i * 2, 2 );
		result.output[i] = HalfToFloat( half );
	}
	result.diagnostics.resize( pixels * 8 );
	std::memcpy( result.diagnostics.data(), diagnostics.data(), diagnosticBytes );
	return std::nullopt;
}

bool Unchanged( const SsrReferencePixel &p )
{
	return !p.traced || !p.hit || !( p.confidence > 0.0 );
}

// One scene through the pass and the reference (render.ssr.v1, "GPU against
// the reference").
std::optional<std::string> GpuScene( device::IRenderDevice2 &device,
    pass::ssr::ScreenSpaceReflections &pass, const std::string &name, const SsrScene &scene,
    std::size_t minimumJudged, Results &results )
{
	const pass::ssr::SsrParams params;
	const SsrReferenceInputs in = AsUploaded( RayCastScene( scene ).inputs );
	const std::vector<SsrReferencePixel> reference = ReferenceSsr( in, params );
	std::vector<std::vector<SsrReferencePixel>> tilted;
	for ( int axis = 0; axis < 3; ++axis )
		for ( double sign : { -1.0, 1.0 } )
		{
			std::array<double, 3> tilt{ 0.0, 0.0, 0.0 };
			tilt[std::size_t( axis )] = sign * 1e-4;
			tilted.push_back( ReferenceSsr( in, params, SsrReferenceDefect::kNone, tilt ) );
		}
	GpuResult gpu;
	if ( std::optional<std::string> why = RunGpu( device, pass, in, gpu ) )
		return why;

	std::size_t judged = 0, unjudged = 0, hitFailures = 0, valueFailures = 0, bitwiseFailures = 0,
	            hits = 0;
	std::string hitFirst, valueFirst, bitwiseFirst;
	const std::uint32_t W = in.width;
	for ( std::size_t i = 0; i < reference.size(); ++i )
	{
		const SsrReferencePixel &r = reference[i];
		const float *out = &gpu.output[i * 4];
		const float *lit = &in.lit[i * 4];
		const float *diagnostic = &gpu.diagnostics[i * 8];
		const bool gpuHit = diagnostic[6] > 0.5f;
		const auto where = [&]( const char *what )
		{
			return Text( "pixel %g,%g: ", double( i % W ), double( i / W ) ) + what;
		};
		bool stable = true;
		for ( const std::vector<SsrReferencePixel> &t : tilted )
			stable =
			    stable && t[i].hit == r.hit &&
			    ( !r.hit || ( t[i].hitTexelX == r.hitTexelX && t[i].hitTexelY == r.hitTexelY ) );
		if ( !r.traced )
		{
			// Background and rough: unchanged, bitwise, always.
			if ( std::memcmp( out, lit, 16 ) != 0 && bitwiseFailures++ == 0 )
				bitwiseFirst = where( "not traced, but changed" );
			continue;
		}
		if ( !stable )
		{
			++unjudged;
			continue;
		}
		++judged;
		hits += r.hit ? 1 : 0;
		bool same = gpuHit == r.hit;
		if ( same && r.hit )
			same = std::uint32_t( diagnostic[4] ) == r.hitTexelX &&
			       std::uint32_t( diagnostic[5] ) == r.hitTexelY &&
			       std::fabs( diagnostic[2] - r.confidence ) <= 1e-3 &&
			       std::fabs( diagnostic[3] - r.mip ) <= 1e-3;
		if ( !same && hitFailures++ == 0 )
			hitFirst = where( "" ) +
			           Text( "reference hit %g texel %g,%g confidence %.4f", double( r.hit ),
			               double( r.hitTexelX ), double( r.hitTexelY ), r.confidence ) +
			           Text( ", GPU hit %g texel %g,%g confidence %.4f", diagnostic[6],
			               diagnostic[4], diagnostic[5], diagnostic[2] );
		if ( Unchanged( r ) )
		{
			if ( std::memcmp( out, lit, 16 ) != 0 && bitwiseFailures++ == 0 )
				bitwiseFirst = where( "unchanged by the reference, changed by the GPU" );
			continue;
		}
		bool close = out[3] == lit[3];
		for ( int c = 0; c < 3; ++c )
			close = close && std::fabs( out[c] - r.out[c] ) <= 2e-3 * std::fabs( r.out[c] ) + 1e-3;
		if ( !close && valueFailures++ == 0 )
			valueFirst = where( "" ) +
			             Text( "reference %.4f %.4f %.4f", r.out[0], r.out[1], r.out[2] ) +
			             Text( ", GPU %.4f %.4f %.4f", out[0], out[1], out[2] );
	}
	const std::string prefix = "gpu." + name + ".";
	const std::string counts = Text(
	    "%g judged (%g hits), %g unjudged", double( judged ), double( hits ), double( unjudged ) );
	results.That( judged >= minimumJudged && hitFailures == 0, prefix + "hits-match-the-reference",
	    counts + Text( ", %g differ", double( hitFailures ) ) +
	        ( hitFirst.empty() ? "" : "; first " + hitFirst ) );
	results.That( valueFailures == 0, prefix + "composite-matches-the-reference",
	    Text( "%g differ", double( valueFailures ) ) +
	        ( valueFirst.empty() ? "" : "; first " + valueFirst ) );
	results.That( bitwiseFailures == 0, prefix + "unchanged-pixels-bitwise",
	    Text( "%g differ", double( bitwiseFailures ) ) +
	        ( bitwiseFirst.empty() ? "" : "; first " + bitwiseFirst ) );
	return std::nullopt;
}

std::optional<std::string> GpuChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<device::IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		auto pass =
		    pass::ssr::ScreenSpaceReflections::CreateWithTrace( *device, pass::ssr::SsrParams(),
		        module.empty() ? std::span<const std::uint32_t>( spirv::kSsrTraceDiagnostics )
		                       : module );
		if ( !pass )
			return std::string( "the SSR pass was refused" );
		// Above the cutoff nothing is traced: every pixel is judged bitwise.
		const struct
		{
			const char *name;
			float roughness;
			std::size_t minimumJudged;
		} scenes[] = {
		    { "mirror", 0.05f, 1000 }, { "glossy", 0.35f, 1000 }, { "rough", 0.45f, 0 } };
		for ( const auto &scene : scenes )
		{
			if ( std::optional<std::string> why = GpuScene( *device, *pass.Value(), scene.name,
			         MirrorScene( scene.roughness ), scene.minimumJudged, results ) )
				return why;
		}
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	messages = 0;
	if ( std::optional<std::string> why = pass::ssr::ValidateParams( pass::ssr::SsrParams() ) )
		return "the default parameters are refused: " + *why;
	// The reference's checks do not depend on the trace program: a seeded
	// run judges the GPU alone.
	if ( module.empty() )
	{
		MirrorChecks( results );
		RoughnessChecks( results );
		OctahedralChecks( results );
	}
	return GpuChecks( validate, module, results, messages );
}

const Seeded kSsrSeeded[] = { { "thickness-ignored", spirv::kSsrTraceThicknessIgnored, "gpu." },
    { "no-edge-fade", spirv::kSsrTraceNoEdgeFade, "gpu." },
    { "wrong-mip", spirv::kSsrTraceWrongMip, "gpu." } };

} // namespace

int RunSsrSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "ssr", kSsrSeeded, RunOnce );
}

} // namespace render::lab
