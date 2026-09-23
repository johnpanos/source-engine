//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for platform.window.v1 (RFC 0001 rank 7, roadmap
//			R14, Q-FOUNDATION). Feeds the SAME shared window/input suite
//			providers that each break one contract rule and requires the named
//			check for that rule to fail, while the conforming provider passes.
//			Proves the shared suite is not vacuous.
//
//			Build/run: tools/quality/conformance.py check --suite platform.window.sensitivity
//
//=============================================================================//

#include "fake_window_system.h"
#include "testing/conformance_result.h"
#include "window_conformance.h"

#include <cstdio>

namespace
{

using namespace platformtest;

struct Seed
{
	const char *check;
	bool FakeWindowDefects::*defect;
};

const Seed kSeeds[] = {
	{ "create.rejects_zero_size", &FakeWindowDefects::acceptZeroSize },
	{ "poll.empty_span_consumes_nothing", &FakeWindowDefects::emptySpanConsumes },
	{ "poll.capacity_one_no_loss", &FakeWindowDefects::dropExpansionOverflow },
	{ "poll.fifo_mixed", &FakeWindowDefects::reverseMixedOrder },
	{ "key.modifier_after_transition", &FakeWindowDefects::modifiersBeforeTransition },
	{ "key.sides_independent", &FakeWindowDefects::lastSideClearsModifier },
	{ "key.modifier_by_layout", &FakeWindowDefects::modifiersFromPhysicalKey },
	{ "key.layout_usage", &FakeWindowDefects::ignoreLayoutUsage },
	{ "text.malformed_replacement", &FakeWindowDefects::noReplacementCharacter },
	{ "mouse.button_fold", &FakeWindowDefects::wrongButtonFold },
	{ "mouse.double_click", &FakeWindowDefects::tripleDoubleClick },
	{ "cursor.warp_motion_suppressed", &FakeWindowDefects::warpNotSuppressed },
	{ "clipboard.roundtrip_utf8", &FakeWindowDefects::clipboardAsciiOnly },
	{ "gamepad.axes", &FakeWindowDefects::unclampedAxes },
	{ "window.close_keeps_window", &FakeWindowDefects::closeDestroysWindow },
	{ "window.minimized_zero_extent", &FakeWindowDefects::minimizeKeepsExtent },
	{ "resize.extent_before_delivery", &FakeWindowDefects::extentAfterSizeEvent },
	{ "surface.background_release", &FakeWindowDefects::backgroundKeepsSurface },
	{ "destroy.listener_notified", &FakeWindowDefects::destroySkipsListener },
	{ "destroy.drops_queued_events", &FakeWindowDefects::deliverDestroyedEvents },
	{ "destroy.ids_not_reused", &FakeWindowDefects::reuseIds },
	{ "destroy.twice_unknown", &FakeWindowDefects::destroyUnknownSucceeds },
};

WindowReport RunAgainst( const FakeWindowDefects &defects )
{
	FakeWindowConfig config;
	config.defects = defects;
	FakeWindowSystem system( config );
	(void)system.Connect();
	(void)system.Initialize();
	FakeWindowDriver driver( system );
	WindowCapabilities caps;
	caps.windows = &system;
	caps.events = &system;
	caps.cursor = &system;
	caps.clipboard = &system;
	caps.messageBox = &system;
	caps.gamepads = &system;
	WindowReport report = RunWindowConformance( caps, driver );
	system.Shutdown();
	system.Disconnect();
	return report;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;

	std::printf( "conforming provider:\n" );
	const WindowReport clean = RunAgainst( FakeWindowDefects{} );
	++checks;
	if ( clean.failures != 0 || clean.checks == 0 )
	{
		++failures;
		std::printf( "FAIL conforming provider rejected (%d failures)\n", clean.failures );
	}

	for ( const Seed &seed : kSeeds )
	{
		FakeWindowDefects defects;
		defects.*seed.defect = true;
		std::printf( "seeded %s:\n", seed.check );
		const WindowReport report = RunAgainst( defects );
		++checks;
		if ( !report.Failed( seed.check ) )
		{
			++failures;
			std::printf( "FAIL seeded defect for %s was not detected by its check\n", seed.check );
		}
		else
		{
			std::printf( "ok %s detected (%d checks failed in total)\n", seed.check, report.failures );
		}
	}

	std::printf( "%s test_window_negative: %d of %zu defects detected\n", failures ? "FAIL" : "ok",
	    checks - 1 - ( failures - ( clean.failures != 0 ) ), std::size( kSeeds ) );
	return testing::ReportConformance( checks, failures );
}
