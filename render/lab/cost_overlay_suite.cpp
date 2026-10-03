//========= Copyright Valve Corporation, All rights reserved. ============//
// Native pixel oracle for the render core's measured cost panel (RFC 0014).
#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/pass/debug/debug_overlays.h"

#include <atomic>
#include <cmath>

namespace render::lab
{
namespace
{
std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t>,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counted{ 0 };
	std::unique_ptr<device::IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counted, device ) )
		return why;
	{
		std::unique_ptr<Canvas> canvas;
		if ( auto why = Canvas::Create( *device, 800, 400, canvas ) )
			return why;
		resources::TextureCache textures( *device );
		material::GroupResidency groups( *device, textures );
		pass::debug::DebugOverlays overlays;
		pass::debug::CostRow rows[] = { { "LIGHTING", 0, 2, 12 }, { "SHADOW", 1, 6, 1 } };
		pass::debug::CostOverlay costs{ rows, 10, 12, 0, true };
		auto render = [&]( bool enabled, CanvasImage &image )
		{
			return canvas->Render( textures, groups, {}, { 0.2f, 0.3f, 0.4f, 1 }, &image,
			    [&]( device::CommandEncoder &encoder, device::TextureId color, device::TextureId )
			        -> std::optional<std::string>
			    {
				    if ( enabled && !overlays.RecordCosts( *device, encoder,
				            { color, kCanvasColor, 800, 400, 1, false }, costs ) )
					    return "cost overlay refused";
				    return std::nullopt;
			    } );
		};
		CanvasImage off, on, swapped, waiting, unsupported, after;
		if ( auto why = render( false, off ) ) return why;
		if ( auto why = render( true, on ) ) return why;
		results.That( on.At( 750, 350 )[0] == off.At( 750, 350 )[0], "cost.outside-unchanged" );
		// First measured row starts at y=72. Its bars occupy y=87..90.
		const auto hotGpu = []( const CanvasImage &image )
		{
			const float *pixel = image.At( 450, 88 );
			return pixel[0] > 0.8f && pixel[1] < 0.8f && pixel[2] < 0.1f;
		};
		results.That( hotGpu( on ), "cost.gpu-hot-bar" );
		const float *cpu = on.At( 40, 88 );
		results.That( cpu[2] > cpu[0] && cpu[1] > cpu[0], "cost.cpu-cool-bar" );
		results.That( on.At( 200, 88 )[0] < 0.06f, "cost.cpu-bar-has-measured-length" );
		results.That( on.At( 610, 88 )[0] < 0.06f, "cost.gpu-bar-has-measured-length" );
		// Negative control: swapping CPU/GPU must break the GPU pixel oracle.
		std::swap( rows[0].cpuMilliseconds, rows[0].gpuMilliseconds );
		if ( auto why = render( true, swapped ) ) return why;
		results.That( !hotGpu( swapped ), "cost.rejects-swapped-cpu-gpu" );
		costs.rows = {};
		if ( auto why = render( true, waiting ) ) return why;
		costs.supported = false;
		if ( auto why = render( true, unsupported ) ) return why;
		results.That( waiting.rgba != unsupported.rgba, "cost.unsupported-is-not-waiting" );
		results.That( on.rgba != waiting.rgba, "cost.waiting-is-not-zero-cost" );
		if ( auto why = render( false, after ) ) return why;
		results.That( off.rgba == after.rgba, "cost.disabled-restores-image" );
		// Optional evidence image, only when the caller prepared the output directory.
		if ( std::filesystem::is_directory( "quality-results/render-cost-overlay" ) )
			results.That( WritePfm( "quality-results/render-cost-overlay/cost-panel.pfm", 800, 400,
			    on.rgba ), "cost.evidence-image" );
	}
	device.reset();
	messages = counted.load();
	return std::nullopt;
}
} // namespace
int RunCostOverlaySuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "cost-overlay", {}, RunOnce );
}
} // namespace render::lab
