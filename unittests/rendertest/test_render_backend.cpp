//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance oracle for the RFC 0001 render backend contract.
//			Runs the shared conformance suite against the deterministic null
//			provider and requires every backend-agnostic obligation to hold. When a
//			Vulkan/D3D9/GL provider lands, it runs THIS SAME suite; a green run here
//			means the contract and suite are ready for that port.
//
//			Build/run: python3 tools/quality/conformance.py check --suite render.backend.null
//
//=============================================================================//

#include "conformance/render_backend_conformance.h"
#include "fakes/null_render_backend.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <memory>

int main()
{
	std::unique_ptr<render::IRenderBackendProvider> provider = rendertest::MakeNullRenderBackend();

	render::conformance::Report report;
	const bool passed = render::conformance::RunRenderBackendConformance( *provider, report );

	for ( const render::conformance::CheckResult &c : report.checks )
	{
		if ( !c.ok )
			std::printf( "FAIL %s: %s\n", c.id.c_str(), c.detail.c_str() );
	}

	std::printf( "render.backend.null: %d check(s), %d failure(s) -> %s\n",
		static_cast<int>( report.checks.size() ), report.FailureCount(),
		passed ? "PASS" : "FAIL" );

	// A conforming provider must also actually exercise a meaningful number of
	// checks; a suite that silently ran nothing cannot certify anything.
	if ( report.checks.size() < 20 )
	{
		std::printf( "FAIL suite ran too few checks (%d); expected the full obligation set\n",
			static_cast<int>( report.checks.size() ) );
		return 1;
	}

	return testing::ReportConformance( report.checks.size(), report.FailureCount() );
}
