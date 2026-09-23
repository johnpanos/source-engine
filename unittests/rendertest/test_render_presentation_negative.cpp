//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity oracle for render.presentation.v1. For each injectable
//			bridge defect, the shared presentation suite must fail overall AND fail
//			the specific check naming the violated obligation. Exit 0 means every
//			defect was detected.
//
//			Build/run: python3 tools/quality/conformance.py check --suite render.presentation.sensitivity
//
//=============================================================================//

#include "conformance/render_presentation_conformance.h"
#include "fakes/headless_presentation.h"
#include "fakes/null_render_backend.h"

#include <cstdio>
#include <functional>
#include <memory>
#include <vector>

using rendertest::HeadlessBridgeDefects;

namespace
{

struct Case
{
	const char *name;
	std::function<void( HeadlessBridgeDefects & )> inject;
	const char *expectedFailedCheck;
};

} // namespace

int main()
{
	const std::vector<Case> cases = {
		{ "recycleOnResize", []( HeadlessBridgeDefects &d ) { d.recycleOnResize = true; },
			"completion.retired_until_complete" },
		{ "resizeLosesDevice", []( HeadlessBridgeDefects &d ) { d.resizeLosesDevice = true; },
			"resize.keeps_device" },
		{ "fatalOnZeroSize", []( HeadlessBridgeDefects &d ) { d.fatalOnZeroSize = true; },
			"zero.backbuffer_suspends" },
		{ "ignoreSurfaceDestroyed",
			[]( HeadlessBridgeDefects &d ) { d.ignoreSurfaceDestroyed = true; },
			"loss.window_destroyed" },
		{ "ignoreSurfaceUnavailable",
			[]( HeadlessBridgeDefects &d ) { d.ignoreSurfaceUnavailable = true; },
			"surface.unavailable_suspends" },
		{ "sharedExtent", []( HeadlessBridgeDefects &d ) { d.sharedExtent = true; },
			"multi.independent_resize" },
		{ "acceptForeignDevice", []( HeadlessBridgeDefects &d ) { d.acceptForeignDevice = true; },
			"create.foreign_device" },
		{ "allowSecondPresentation",
			[]( HeadlessBridgeDefects &d ) { d.allowSecondPresentation = true; },
			"create.surface_busy" },
		{ "leakOnDestroy", []( HeadlessBridgeDefects &d ) { d.leakOnDestroy = true; },
			"lifetime.device_resources_baseline" },
		{ "keepListenerAttached",
			[]( HeadlessBridgeDefects &d ) { d.keepListenerAttached = true; },
			"lifetime.surface_detached" },
		{ "noLimit", []( HeadlessBridgeDefects &d ) { d.noLimit = true; }, "multi.over_limit" },
		{ "keepNativeAfterRelease",
			[]( HeadlessBridgeDefects &d ) { d.keepNativeAfterRelease = true; },
			"lifetime.native_released_before_window" },
		{ "resizeDuringFrame", []( HeadlessBridgeDefects &d ) { d.resizeDuringFrame = true; },
			"resize.rejected_during_frame" },
		{ "releaseDeviceWhileLive",
			[]( HeadlessBridgeDefects &d ) { d.releaseDeviceWhileLive = true; },
			"lifetime.release_device_order" },
	};

	int undetected = 0;
	for ( const Case &c : cases )
	{
		HeadlessBridgeDefects defects;
		c.inject( defects );
		std::unique_ptr<render::IRenderBackendProvider> provider =
			rendertest::MakeNullRenderBackend();
		rendertest::HeadlessSurfaces surfaces;
		std::unique_ptr<render::IRenderPresentationBridgeFactory> bridge =
			rendertest::MakeHeadlessPresentationBridge( *provider, surfaces, 4, defects );
		rendertest::HeadlessHarness harness( surfaces );

		render::conformance::Report report;
		const bool passed =
			render::conformance::RunPresentationConformance( *provider, *bridge, harness, report );
		const bool specific = report.Failed( c.expectedFailedCheck );
		if ( !passed && specific )
		{
			std::printf( "ok   defect '%s' detected via '%s'\n", c.name, c.expectedFailedCheck );
		}
		else
		{
			++undetected;
			std::printf( "FAIL defect '%s' NOT detected (overall_failed=%d, '%s'_failed=%d)\n",
				c.name, passed ? 0 : 1, c.expectedFailedCheck, specific ? 1 : 0 );
		}
	}

	std::printf( "render.presentation.sensitivity: %d defect(s), %d undetected\n",
		static_cast<int>( cases.size() ), undetected );
	return undetected == 0 ? 0 : 1;
}
