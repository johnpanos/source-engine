//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Fizzler emission before camera opacity, against the CPU oracle,
//          plus geometry, integration, failure and state controls. Receiver
//          pixels are judged by the area-lights suite's fizzler case.
//
//===========================================================================//

#include "lab_compute.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"
#include "render/energy_field.h"
#include "spv/energy_field_check_spv.h"

#include <atomic>
#include <cmath>
#include <cstring>
#include <limits>
#include <random>

namespace render::lab
{
namespace
{
struct Case
{
	float base[4];
	float reveal[4];
	float flow[4];
	float vortex[4];
};

energy_field::Surface Surface()
{
	energy_field::Surface surface;
	const float p[4][3] = {
	    { -60, -40, 120 }, { 60, -40, 120 }, { 60, 40, 120 }, { -60, 40, 120 } };
	const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	std::memcpy( surface.p, p, sizeof( p ) );
	std::memcpy( surface.uv, uv, sizeof( uv ) );
	surface.tangentS[0] = surface.tangentT[1] = 1.0f;
	return surface;
}

void SourceChecks( Results &results )
{
	const auto surface = Surface();
	energy_field::Flow flow;
	flow.color[0] = 0.1f;
	flow.color[1] = 0.4f;
	flow.color[2] = 1.0f;
	flow.vortexColor[0] = 1.0f;
	flow.intensity = 2.0f;
	flow.time = 0.2f;
	flow.normalUvScale = flow.noiseScale = 0.01f;
	auto sample = []( int texture, float, float, float rgba[4] )
	{
		const float texels[6][4] = { { 0, 0.25f, 0, 0.5f }, { 0.5f, 0.5f, 0, 1 }, { 0, 0, 0, 1 },
		    { 1, 0, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 } };
		std::memcpy( rgba, texels[texture], sizeof( texels[0] ) );
		return true;
	};
	area_light::AreaLight light;
	results.That( energy_field::MeanLight( surface, flow, 16, sample, light ) &&
	                  std::fabs( light.radiance[0] - 0.1f ) < 1e-5f &&
	                  std::fabs( light.radiance[1] - 0.4f ) < 1e-5f &&
	                  std::fabs( light.radiance[2] - 1.0f ) < 1e-5f,
	    "source.mean-matches-analytic" );
	results.That(
	    light.rect.twoSided && std::fabs( area_light::Area( light.rect ) - 9600.0f ) < 0.01f,
	    "source.authored-face-area" );
	const float front[3] = { 0, 0, 240 }, frontNormal[3] = { 0, 0, -1 };
	const float back[3] = { 0, 0, 0 }, backNormal[3] = { 0, 0, 1 };
	float a[3], b[3];
	area_light::IrradianceAt( light, front, frontNormal, a );
	area_light::IrradianceAt( light, back, backNormal, b );
	results.That( a[2] > 0.0f && std::fabs( a[2] - b[2] ) < 1e-6f, "source.two-sides-equal" );
	flow.powerUp = 0;
	results.That( energy_field::MeanLight( surface, flow, 16, sample, light ) && light.reach == 0,
	    "source.off-gives-zero" );
	flow.powerUp = 1;
	flow.intensity = 0;
	results.That( energy_field::MeanLight( surface, flow, 16, sample, light ) && light.reach == 0,
	    "source.zero-intensity" );
	flow.intensity = 2;
	flow.vortexEnabled[0] = true;
	std::memcpy( flow.vortex[0], surface.p[0], sizeof( flow.vortex[0] ) );
	flow.vortexSize = 120;
	results.That( energy_field::MeanLight( surface, flow, 16, sample, light ) &&
	                  light.radiance[0] > 0.1f && light.radiance[2] < 1.0f,
	    "source.vortex-changes-emission" );
	int calls = 0;
	results.That( !energy_field::MeanLight(
	                  surface, flow, 16,
	                  [&]( int tex, float s, float t, float rgba[4] )
	                  {
		                  return ++calls < 8 && sample( tex, s, t, rgba );
	                  },
	                  light ) &&
	                  light.reach == 0 && light.radiance[0] == 0,
	    "source.missing-texture-rollback" );
	auto bad = surface;
	bad.p[2][0] += 10;
	results.That(
	    !energy_field::MeanLight( bad, flow, 16, sample, light ), "source.nonrectangle-refused" );
	bad = surface;
	bad.p[0][0] = std::numeric_limits<float>::quiet_NaN();
	results.That(
	    !energy_field::MeanLight( bad, flow, 16, sample, light ), "source.nonfinite-refused" );
	results.That( !energy_field::MeanLight( surface, flow, 0, sample, light ),
	    "source.zero-samples-refused" );
	flow.interval = 0;
	results.That( !energy_field::MeanLight( surface, flow, 16, sample, light ),
	    "source.invalid-interval-refused" );
	flow.interval = std::numeric_limits<float>::infinity();
	results.That( !energy_field::MeanLight( surface, flow, 16, sample, light ),
	    "source.infinite-interval-refused" );
	flow.interval = 1;
	flow.outputIntensity = std::numeric_limits<float>::quiet_NaN();
	results.That( !energy_field::MeanLight( surface, flow, 16, sample, light ),
	    "source.nonfinite-output-refused" );
	flow.outputIntensity = 1;
	results.That( !energy_field::MeanLight(
	                  surface, flow, 16,
	                  [&]( int tex, float s, float t, float rgba[4] )
	                  {
		                  sample( tex, s, t, rgba );
		                  rgba[0] = std::numeric_limits<float>::quiet_NaN();
		                  return true;
	                  },
	                  light ) &&
	                  light.reach == 0,
	    "source.nonfinite-texture-rollback" );
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	SourceChecks( results );
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<device::IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		CheckKernel kernel( *device );
		if ( auto why = kernel.Create(
		         module.empty() ? std::span<const std::uint32_t>( spirv::kEnergyFieldCheck )
		                        : module,
		         0, 0, "render_lab.energy-field" ) )
			return why;
		std::mt19937 random( 20261002u );
		std::uniform_real_distribution<float> unit( 0.0f, 1.0f );
		std::vector<Case> cases;
		for ( int i = 0; i < 512; ++i )
		{
			Case c;
			for ( int k = 0; k < 4; ++k )
			{
				c.base[k] = unit( random );
				c.reveal[k] = unit( random );
				c.flow[k] = unit( random );
				c.vortex[k] = unit( random );
			}
			c.reveal[2] = i % 8 == 0 ? 1.0f : unit( random );
			c.reveal[3] = i % 8 == 1 ? 0.0f : unit( random ) * 10.0f;
			c.vortex[3] = i % 8 == 2 ? 0.0f : unit( random ) * 2.0f;
			cases.push_back( c );
		}
		resources::TextureCache textures( *device );
		std::vector<std::byte> output;
		if ( auto why = kernel.Run( textures, {}, unsigned( cases.size() ),
		         std::as_bytes( std::span( cases ) ), cases.size() * 16, output ) )
			return why;
		std::vector<float> values( output.size() / sizeof( float ) );
		std::memcpy( values.data(), output.data(), output.size() );
		for ( std::size_t i = 0; i < cases.size(); ++i )
		{
			const Case &c = cases[i];
			float base[4];
			std::memcpy( base, c.base, sizeof( base ) );
			energy_field::Reveal( base, c.reveal[0], c.reveal[1], c.reveal[2] );
			energy_field::Flow flow;
			std::memcpy( flow.color, c.flow, sizeof( flow.color ) );
			std::memcpy( flow.vortexColor, c.vortex, sizeof( flow.vortexColor ) );
			flow.intensity = c.reveal[3];
			float expected[4];
			energy_field::Radiance( flow, base, c.flow[3], c.vortex[3], expected );
			for ( int k = 0; k < 3; ++k )
				expected[k] = std::max( 0.0f, expected[k] );
			expected[3] = base[3];
			bool same = true;
			for ( int k = 0; k < 4; ++k )
				same = same && std::isfinite( values[4 * i + k] ) &&
				       std::fabs( values[4 * i + k] - expected[k] ) <=
				           2e-5f + 2e-5f * std::fabs( expected[k] );
			results.That( same, "emission.case-" + std::to_string( i ) );
		}
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}
const Seeded kSeeded[] = { { "reveal-ignored", spirv::kEnergyFieldRevealIgnored, "emission." },
    { "intensity-ignored", spirv::kEnergyFieldIntensityIgnored, "emission." } };
} // namespace

int RunEnergyFieldSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "energy-field", kSeeded, RunOnce );
}
} // namespace render::lab
