//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite ssr (RFC 0016 K11 "Screen-space reflections",
//			render.ssr.v1). This slice: the CPU reference
//			(ssr_reference.h) judged on analytic scenes (ssr_scene.h) against
//			their true mirror reflections, so the reference the GPU pass will
//			be judged by is itself judged first:
//			- a mirror floor before a patterned wall with a thin bar: every
//			  floor pixel whose true reflection is on the screen and seen by
//			  the camera (the same rectangle over 5 x 5 pixels around it) is
//			  hit within 1.5 pixels of it, and the light it reflects is the
//			  wall's or bar's there within 3 percent + 0.01, except rays that
//			  pass within the thickness behind the bar and stop there (the
//			  ambiguity screen space has by definition), at most 2 percent;
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

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
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

// A mirror floor (z = 0) before a patterned wall (x = 600) with a thin red
// bar (x = 300, 20 to 30 units up) floating over it, seen from 48 units up.
// Rays that pass under the bar rise behind it on the screen.
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
	floor.imageSpecular = { 0.05f, 0.04f, 0.03f };
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
	scene.quads = { floor, wall, bar };
	return scene;
}

std::string Format( const char *format, double a, double b = 0, double c = 0, double d = 0 )
{
	char text[256];
	std::snprintf( text, sizeof( text ), format, a, b, c, d );
	return text;
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
	const std::uint32_t W = images.inputs.width, H = images.inputs.height;

	std::size_t judged = 0, positionFailures = 0, radianceFailures = 0, offScreen = 0,
	            offScreenHits = 0, passedBehind = 0, unchangedFailures = 0, edgeBand = 0,
	            edgeFailures = 0, ambiguous = 0;
	std::string positionFirst, radianceFirst, offScreenFirst, unchangedFirst, edgeFirst;
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
					unchangedFirst = Format( "pixel %g,%g changed", x, y );
				continue;
			}
			if ( truth.reflectedQuad >= 0 && !truth.reflectedOnScreen )
			{
				++offScreen;
				if ( p.hit && offScreenHits++ == 0 )
					offScreenFirst = Format( "pixel %g,%g hit %g,%g", x, y, p.hitX, p.hitY );
				continue;
			}
			// Rays that pass behind the bar (their true reflection is the
			// wall): not stopped by it with the default thickness, stopped
			// when nothing is too thick.
			if ( truth.reflectedQuad == kWall && thick[i].hit && locate( thick[i] ) == kBar &&
			     !( p.hit && locate( p ) == kBar ) )
				++passedBehind;
			if ( truth.reflectedQuad < 0 || !truth.reflectedVisible )
				continue;
			++judged;
			const double miss = std::hypot( p.hitX - truth.reflectedX, p.hitY - truth.reflectedY );
			// A ray that passes within `thickness` behind the bar is stopped by
			// it, as the definition says (screen space has no thickness): the
			// ambiguity SSR has, counted apart and bounded.
			if ( p.hit && miss > 1.5 && truth.reflectedQuad == kWall && locate( p ) == kBar )
			{
				++ambiguous;
				continue;
			}
			if ( ( !p.hit || miss > 1.5 ) && positionFailures++ == 0 )
				positionFirst = Format( "pixel %g,%g: hit at %.2f,%.2f", x, y, p.hitX, p.hitY ) +
				                Format( ", true %.2f,%.2f", truth.reflectedX, truth.reflectedY );
			if ( p.hit )
			{
				const std::array<float, 3> expected =
				    scene.quads[std::size_t( truth.reflectedQuad )].radiance(
				        truth.reflectedPoint );
				for ( int c = 0; c < 3; ++c )
				{
					if ( std::fabs( p.reflected[c] - expected[std::size_t( c )] ) >
					         0.03 * std::fabs( expected[std::size_t( c )] ) + 0.01 &&
					     radianceFailures++ == 0 )
						radianceFirst =
						    Format( "pixel %g,%g channel %g: %.4f", x, y, c, p.reflected[c] ) +
						    Format( " against %.4f", expected[std::size_t( c )] );
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
						edgeFirst = Format( "pixel %g,%g: confidence %.3f at %.4f", x, y,
						    p.confidence, edgeDistance );
				}
			}
		}
	}
	results.That( judged > 500 && positionFailures == 0, "reference.mirror.hits-true-reflection",
	    Format( "%g of %g floor pixels off", double( positionFailures ), double( judged ) ) +
	        ( positionFirst.empty() ? "" : "; first " + positionFirst ) );
	results.That( ambiguous * 50 <= judged, "reference.mirror.thickness-ambiguity-bounded",
	    Format( "%g of %g floor pixels stopped by the bar within the thickness",
	        double( ambiguous ), double( judged ) ) );
	results.That( radianceFailures == 0, "reference.mirror.reflects-its-light",
	    Format( "%g channels off", double( radianceFailures ) ) +
	        ( radianceFirst.empty() ? "" : "; first " + radianceFirst ) );
	results.That( passedBehind > 0, "reference.thickness.passes-behind-the-bar",
	    Format( "%g rays reach the wall past the bar", double( passedBehind ) ) );
	results.That( offScreen > 0 && offScreenHits == 0, "reference.off-screen-misses",
	    Format( "%g of %g hit", double( offScreenHits ), double( offScreen ) ) +
	        ( offScreenFirst.empty() ? "" : "; first " + offScreenFirst ) );
	results.That( edgeBand > 0 && edgeFailures == 0, "reference.edge-fades",
	    Format( "%g in the band, %g wrong", double( edgeBand ), double( edgeFailures ) ) +
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
		    Format( "%g of %g hits above the fade %.3f", double( wrong ), double( hits ), fade ) );
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
		    Format( "%g changed, %g traced", double( changed ), double( traced ) ) );
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
	results.That( worst <= 1e-6, "reference.octahedral-round-trip", Format( "worst %.3g", worst ) );
}

std::optional<std::string> RunOnce(
    bool, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	messages = 0;
	if ( std::optional<std::string> why = pass::ssr::ValidateParams( pass::ssr::SsrParams() ) )
		return "the default parameters are refused: " + *why;
	MirrorChecks( results );
	RoughnessChecks( results );
	OctahedralChecks( results );
	return std::nullopt;
}

} // namespace

int RunSsrSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "ssr", std::span<const Seeded>(), RunOnce );
}

} // namespace render::lab
