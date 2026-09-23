//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity oracle for the RFC 0001 render backend conformance suite.
//			A suite that never fails certifies nothing (RFC 0005). For each
//			injectable defect, this builds a provider that violates exactly one
//			obligation and requires the shared suite to DETECT it -- both by
//			failing overall and by failing the specific check that names the
//			violated obligation. Exit 0 means every defect was caught (sensitivity
//			confirmed); a non-zero exit means the suite has a blind spot.
//
//			Build/run: python3 tools/quality/conformance.py check --suite render.backend.sensitivity
//
//=============================================================================//

#include "conformance/render_backend_conformance.h"
#include "fakes/null_render_backend.h"

#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

using rendertest::NullBackendDefects;

namespace
{

struct Case
{
	const char *name;					// human label for the injected defect
	std::function<void( NullBackendDefects & )> inject;
	const char *expectedFailedCheck;	// the obligation the suite must flag
};

} // namespace

int main()
{
	const std::vector<Case> cases = {
		{ "mutableCaps", []( NullBackendDefects &d ) { d.mutableCaps = true; },
			"caps.immutable" },
		{ "recycleOnFrameAdvance", []( NullBackendDefects &d ) { d.recycleOnFrameAdvance = true; },
			"resource.deferred_until_complete" },
		{ "unorderedCompletion", []( NullBackendDefects &d ) { d.unorderedCompletion = true; },
			"submit.ordered_completion" },
		{ "leakResources", []( NullBackendDefects &d ) { d.leakResources = true; },
			"resource.recycled_after_complete" },
		{ "swallowRequiredFeature", []( NullBackendDefects &d ) { d.swallowRequiredFeature = true; },
			"create.unsupported_required_feature" },
		{ "lossSkipsLostState", []( NullBackendDefects &d ) { d.lossSkipsLostState = true; },
			"loss.enters_lost_state" },
		{ "disownDevices", []( NullBackendDefects &d ) { d.disownDevices = true; },
			"lifetime.owns_device" },
	};

	int undetected = 0;
	for ( const Case &c : cases )
	{
		NullBackendDefects defects;
		c.inject( defects );
		std::unique_ptr<render::IRenderBackendProvider> provider =
			rendertest::MakeNullRenderBackend( defects );

		render::conformance::Report report;
		const bool passed = render::conformance::RunRenderBackendConformance( *provider, report );

		const bool overallCaught = !passed;
		const bool specificCaught = report.Failed( c.expectedFailedCheck );

		if ( overallCaught && specificCaught )
		{
			std::printf( "ok   defect '%s' detected via '%s'\n", c.name, c.expectedFailedCheck );
		}
		else
		{
			++undetected;
			std::printf( "FAIL defect '%s' NOT detected (overall_failed=%d, '%s'_failed=%d)\n",
				c.name, overallCaught ? 1 : 0, c.expectedFailedCheck, specificCaught ? 1 : 0 );
		}
	}

	std::printf( "render.backend.sensitivity: %d defect(s), %d undetected\n",
		static_cast<int>( cases.size() ), undetected );
	return undetected == 0 ? 0 : 1;
}
