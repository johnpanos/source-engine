//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive oracle for render.presentation.v1 on the headless-null pair:
//			the shared presentation suite runs against the headless bridge over the
//			null provider and every obligation must hold. The SDL3-Vulkan pair runs
//			the same suite natively (unittests/shaderapivulkantest).
//
//			Build/run: python3 tools/quality/conformance.py check --suite render.presentation.headless
//
//=============================================================================//

#include "conformance/render_presentation_conformance.h"
#include "fakes/headless_presentation.h"
#include "fakes/null_render_backend.h"

#include <cstdio>
#include <memory>

int main()
{
	std::unique_ptr<render::IRenderBackendProvider> provider = rendertest::MakeNullRenderBackend();
	rendertest::HeadlessSurfaces surfaces;
	std::unique_ptr<render::IRenderPresentationBridgeFactory> bridge =
	    rendertest::MakeHeadlessPresentationBridge( *provider, surfaces );
	rendertest::HeadlessHarness harness( surfaces );

	render::conformance::Report report;
	const bool passed =
	    render::conformance::RunPresentationConformance( *provider, *bridge, harness, report );

	for ( const render::conformance::CheckResult &c : report.checks )
	{
		if ( !c.ok )
			std::printf( "FAIL %s: %s\n", c.id.c_str(), c.detail.c_str() );
	}
	for ( const render::conformance::CheckResult &c : report.skipped )
		std::printf( "SKIP %s: %s\n", c.id.c_str(), c.detail.c_str() );
	std::printf( "render.presentation.headless: %d check(s), %d failure(s) -> %s\n",
	    static_cast<int>( report.checks.size() ), report.FailureCount(), passed ? "PASS" : "FAIL" );

	// Every section must have run; a suite that skipped sections certifies nothing.
	if ( report.checks.size() < 40 || !report.skipped.empty() )
	{
		std::printf( "FAIL suite ran too few checks (%d); expected the full obligation set\n",
		    static_cast<int>( report.checks.size() ) );
		return 1;
	}
	return passed ? 0 : 1;
}
