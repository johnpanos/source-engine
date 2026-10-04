//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent double-precision oracle for Source foliage vertex
// animation, including the published intro4 map's negative-height vines.
//
//=============================================================================//

#include "lab_compute.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "spv/tree_sway_check_spv.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

namespace render::lab
{
namespace
{
struct Case
{
	float positionTime[4] = {};
	float windMode[4] = {};
	float geometry[4] = { 1000.0f, 0.1f, 300.0f, 0.2f };
	float motion[4] = { 1.0f, 10.0f, 12.0f, 10.0f };
	float curves[4] = { 2.0f, 1.0f, 1.5f, 5.0f };
	float windControls[4] = { 3.0f, 6.0f, 0.0f, 0.0f };
	float objectToWorld[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
};
static_assert( sizeof( Case ) == 160 );

std::array<double, 3> Reference( const Case &c )
{
	std::array<double, 3> out = { c.positionTime[0], c.positionTime[1], c.positionTime[2] };
	const int mode = int( c.windMode[2] );
	if ( mode == 0 )
		return out;
	const double wx = c.windControls[2] ? 0.5 : c.windMode[0];
	const double wy = c.windControls[2] ? 0.5 : c.windMode[1];
	const double intensity = std::hypot( wx, wy );
	double wind[3];
	for ( int axis = 0; axis < 3; ++axis )
		wind[axis] = c.objectToWorld[axis] * wx + c.objectToWorld[4 + axis] * wy;
	const double x = out[0], y = out[1], z = out[2], time = c.positionTime[3];
	const double heightScale = std::clamp( ( z - double( c.geometry[0] ) * c.geometry[1] ) /
	                                           ( ( 1.0 - c.geometry[1] ) * c.geometry[0] ),
	    0.0, 1.0 );
	const double radiusStart = double( c.geometry[2] ) * c.geometry[3];
	const double radiusScale = std::clamp( std::hypot( x - radiusStart, y - radiusStart ) /
	                                           ( ( 1.0 - c.geometry[3] ) * c.geometry[2] ),
	    0.0, 1.0 );
	const bool active = mode == 2 ? z <= double( c.geometry[0] ) * c.geometry[1]
	                              : z >= double( c.geometry[0] ) * c.geometry[1];
	const double orthogonal =
	    1.0 - std::clamp( std::abs( wind[0] * x + wind[1] * y ) /
	                          ( std::max( std::hypot( wind[0], wind[1], wind[2] ), 0.0001 ) *
	                              std::max( std::hypot( x, y ), 0.0001 ) ),
	              0.0, 1.0 );
	const double trunk = c.motion[1] * std::pow( heightScale, c.curves[2] );
	const double branch = mode == 2 ? 0.0 : c.motion[1] * orthogonal * radiusScale * active;
	const double phase =
	    ( double( c.objectToWorld[3] ) + c.objectToWorld[7] + c.objectToWorld[11] ) * 19.0;
	const double slowTime = ( time + phase ) * c.motion[0];
	const double t = std::clamp(
	    ( intensity - c.windControls[0] ) / ( c.windControls[1] - c.windControls[0] ), 0.0, 1.0 );
	const double blend = t * t * ( 3.0 - 2.0 * t );
	const double sway0 =
	    std::lerp( std::sin( slowTime ), std::sin( slowTime * c.curves[0] ), blend );
	const double sway1 =
	    std::lerp( std::sin( slowTime * 2.31 ), std::sin( slowTime * 2.14 * c.curves[0] ), blend );
	const double scramble = c.motion[3] * std::pow( radiusScale, c.curves[1] ) * active;
	const double length = std::max( std::hypot( x, y, z ), 0.0001 );
	const double position[] = { x, y, z };
	for ( int axis = 0; axis < 3; ++axis )
	{
		out[axis] += wind[axis] * ( trunk * ( sway0 + 0.1 ) + branch * ( sway1 + 0.4 ) );
		const double scale = mode == 2 && axis < 2 ? 0.5 : 1.0;
		out[axis] += intensity * scramble * scale *
		             std::sin( c.curves[3] * time +
		                       position[( axis + 1 ) % 3] / length * c.motion[2] + phase );
	}
	return out;
}

std::optional<std::string> Run( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<device::IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		CheckKernel kernel( *device );
		if ( auto why = kernel.Create(
		         module.empty() ? std::span<const std::uint32_t>( spirv::kTreeSwayCheck ) : module,
		         0, 0, "render_lab.tree-sway" ) )
			return why;
		std::vector<Case> cases;
		// Distinct authored sets: leaves/cluster vines, fern bushes, suspended
		// vines and thick vines. Include both upright and hanging modes.
		const float geometry[][4] = { { -300, .1f, 30, .125f }, { 60, .1f, .5f, .25f },
		    { -128, .1f, .5f, .25f }, { 300, .1f, 10, .5f } };
		const float motion[][4] = { { .0125f, 1, 25, .015f }, { .0015f, .0015f, 10, .0075f },
		    { 0, 0, 12, .02f }, { 0, 0, 10, .009f } };
		const float curves[][4] = {
		    { 25, 2, 1.4f, 5 }, { 10, 7, 5, 2 }, { 12, 4, 3, 4 }, { 10, 4, 3, 2 } };
		for ( int authored = 0; authored < 4; ++authored )
			for ( int mode = 0; mode <= 2; ++mode )
				for ( int sample = 0; sample < 96; ++sample )
				{
					Case c;
					std::copy_n( geometry[authored], 4, c.geometry );
					std::copy_n( motion[authored], 4, c.motion );
					std::copy_n( curves[authored], 4, c.curves );
					c.positionTime[0] = float( sample % 8 ) * 6;
					c.positionTime[1] = float( sample % 7 ) * -5;
					c.positionTime[2] = sample == 0 ? 0 : ( sample / 8 - 2 ) * c.geometry[0] / 8;
					c.positionTime[3] = float( sample ) / 7;
					c.windMode[0] = float( sample % 5 ) * 4;
					c.windMode[1] = float( sample % 3 ) * -3;
					c.windMode[2] = float( mode );
					c.windControls[2] = sample % 4 == 0 ? 1.0f : 0.0f;
					c.objectToWorld[3] = 17;
					c.objectToWorld[7] = -24;
					c.objectToWorld[11] = 3;
					if ( sample % 2 )
					{
						c.objectToWorld[0] = c.objectToWorld[5] = 0;
						c.objectToWorld[1] = -1;
						c.objectToWorld[4] = 1;
					}
					cases.push_back( c );
				}
		resources::TextureCache textures( *device );
		std::vector<std::byte> bytes;
		if ( auto why = kernel.Run( textures, {}, unsigned( cases.size() ),
		         std::as_bytes( std::span( cases ) ), cases.size() * 16, bytes ) )
			return why;
		std::vector<float> gpu( bytes.size() / sizeof( float ) );
		std::memcpy( gpu.data(), bytes.data(), bytes.size() );
		for ( int cohort = 0; cohort < 12; ++cohort )
		{
			bool matches = true, finite = true;
			double worst = 0;
			for ( int sample = 0; sample < 96; ++sample )
			{
				const size_t i = cohort * 96 + sample;
				const auto expected = Reference( cases[i] );
				for ( int axis = 0; axis < 3; ++axis )
				{
					const double error = std::abs( gpu[i * 4 + axis] - expected[axis] );
					worst = std::max( worst, error );
					finite = finite && std::isfinite( gpu[i * 4 + axis] );
					matches = matches && error <= .002 + std::abs( expected[axis] ) * 2e-5;
				}
			}
			results.That( finite, "tree-sway.finite." + std::to_string( cohort ) );
			results.That( matches, "tree-sway.oracle." + std::to_string( cohort ),
			    "worst source-unit error " + std::to_string( worst ) );
		}
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}
}

int RunTreeSwaySuite( int argc, char **argv )
{
	const Seeded seeded[] = {
	    { "static-ignored", spirv::kTreeSwayStaticIgnored, "tree-sway.oracle." },
	    { "wind-unrotated", spirv::kTreeSwayWindUnrotated, "tree-sway.oracle." },
	    { "hanging-ignored", spirv::kTreeSwayHangingIgnored, "tree-sway.oracle." },
	    { "root-ignored", spirv::kTreeSwayRootIgnored, "tree-sway.oracle." } };
	return RunSeededSuite( argc, argv, "tree-sway", seeded, Run );
}
} // namespace render::lab
