//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pbr-specular-aa.v1 (RFC 0012 A2): the GLSL filter against
// its C++ owner and an independent double-precision oracle, with the
// contract's identity, widening and threshold clauses. Seeded: disabled
// (fails widening), biased without derivatives (fails identity),
// unclamped (fails threshold).
//
//=============================================================================//

#include "lab_compute.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "spv/specular_aa_check_spv.h"

#include "render/pbr_specular_aa.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace render::lab
{
namespace
{
struct Case
{
	float roughness[4] = {};
	float normalDx[4] = {};
	float normalDy[4] = {};
};

// The contract's formula in double precision, written from the RFC, not the
// header: kernel = min(2 sigma^2 |dn|^2, threshold) added to alpha^2.
double Reference( const Case &c )
{
	double squared = 0;
	for ( int axis = 0; axis < 3; ++axis )
		squared += double( c.normalDx[axis] ) * c.normalDx[axis] +
		           double( c.normalDy[axis] ) * c.normalDy[axis];
	const double kernel = std::min( 2.0 * 0.15 * squared, 0.2 );
	if ( kernel <= 0 )
		return c.roughness[0];
	const double alpha = double( c.roughness[0] ) * c.roughness[0];
	return std::pow( std::min( alpha * alpha + kernel, 1.0 ), 0.25 );
}

float Owner( const Case &c )
{
	return pbr::SpecularAaRoughness( c.roughness[0], c.normalDx[0], c.normalDx[1], c.normalDx[2],
	    c.normalDy[0], c.normalDy[1], c.normalDy[2] );
}

// Cohorts: 0 zero derivatives; 1 small; 2 medium; 3 saturating the threshold.
std::vector<Case> Cases()
{
	const float roughness[] = { 0.02f, 0.05f, 0.1f, 0.2f, 0.35f, 0.5f, 0.75f, 1.0f };
	const float scale[] = { 0.0f, 0.02f, 0.25f, 4.0f };
	std::vector<Case> cases;
	for ( float s : scale )
		for ( float r : roughness )
			for ( int direction = 0; direction < 8; ++direction )
			{
				Case c;
				c.roughness[0] = r;
				const float a = float( direction ) * 0.785398f;
				c.normalDx[0] = s * std::cos( a );
				c.normalDx[1] = s * std::sin( a ) * 0.5f;
				c.normalDx[2] = s * 0.25f * float( direction % 3 );
				c.normalDy[0] = -s * std::sin( a ) * 0.75f;
				c.normalDy[1] = s * std::cos( a );
				c.normalDy[2] = direction % 2 ? -s * 0.5f : 0.0f;
				cases.push_back( c );
			}
	return cases;
}

constexpr int kPerCohort = 64;

void CheckOwner( Results &results )
{
	const auto cases = Cases();
	bool matches = true, identity = true;
	double worst = 0;
	for ( std::size_t i = 0; i < cases.size(); ++i )
	{
		const float owner = Owner( cases[i] );
		const double error = std::abs( owner - Reference( cases[i] ) );
		worst = std::max( worst, error );
		matches = matches && error <= 2e-6;
		if ( i < kPerCohort )
			identity = identity && owner == cases[i].roughness[0];
	}
	results.That( matches, "specular-aa.owner.reference", "worst " + std::to_string( worst ) );
	results.That( identity, "specular-aa.owner.identity" );
	results.That( pbr::kSpecularAaVersion == 1, "specular-aa.owner.version" );
}

std::optional<std::string> Run( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	CheckOwner( results );
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<device::IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		CheckKernel kernel( *device );
		if ( auto why = kernel.Create(
		         module.empty() ? std::span<const std::uint32_t>( spirv::kSpecularAaCheck )
		                        : module,
		         0, 0, "render_lab.specular-aa" ) )
			return why;
		const auto cases = Cases();
		resources::TextureCache textures( *device );
		std::vector<std::byte> bytes;
		if ( auto why = kernel.Run( textures, {}, unsigned( cases.size() ),
		         std::as_bytes( std::span( cases ) ), cases.size() * sizeof( float ), bytes ) )
			return why;
		std::vector<float> gpu( cases.size() );
		std::memcpy( gpu.data(), bytes.data(), gpu.size() * sizeof( float ) );

		bool identity = true, widens = true, threshold = true, finite = true;
		for ( int cohort = 0; cohort < 4; ++cohort )
		{
			bool matches = true;
			double worst = 0;
			for ( int k = 0; k < kPerCohort; ++k )
			{
				const std::size_t i = std::size_t( cohort ) * kPerCohort + k;
				const float input = cases[i].roughness[0];
				const float value = gpu[i];
				finite = finite && std::isfinite( value );
				const double error = std::abs( value - Owner( cases[i] ) );
				worst = std::max( worst, error );
				matches = matches && error <= 1e-5;
				if ( cohort == 0 )
					identity = identity && value == input;
				// A nonzero kernel strictly widens every lobe below 1.
				else if ( input < 1.0f )
					widens = widens && value > input;
				// The kernel adds at most the threshold.
				const double alpha = double( input ) * input;
				const double ceiling = std::pow( std::min( alpha * alpha + 0.2, 1.0 ), 0.25 );
				threshold = threshold && value <= ceiling + 1e-5;
			}
			results.That( matches, "specular-aa.gpu.owner." + std::to_string( cohort ),
			    "worst " + std::to_string( worst ) );
		}
		results.That( finite, "specular-aa.gpu.finite" );
		results.That( identity, "specular-aa.gpu.identity" );
		results.That( widens, "specular-aa.gpu.widens" );
		results.That( threshold, "specular-aa.gpu.threshold" );
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}
} // namespace

int RunSpecularAaSuite( int argc, char **argv )
{
	const Seeded seeded[] = { { "disabled", spirv::kSpecularAaDisabled, "specular-aa.gpu.widens" },
	    { "biased", spirv::kSpecularAaBiased, "specular-aa.gpu.identity" },
	    { "no-threshold", spirv::kSpecularAaNoThreshold, "specular-aa.gpu.threshold" } };
	return RunSeededSuite( argc, argv, "specular-aa", seeded, Run );
}
} // namespace render::lab
