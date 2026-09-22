//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance oracle for the RFC 0001 render backend contract
//          run against the NATIVE VULKAN provider. This is the substitutability
//          proof: it runs the exact same shared suite as the null provider
//          (test_render_backend.cpp) against a genuinely GPU-backed provider, so
//          a green run here means Vulkan is an interchangeable implementation of
//          the render.contracts contract the material system depends on (LSP).
//
//          Requires a usable Vulkan device; when none is present the provider
//          factory returns nullptr and the test reports an explicit skip
//          (exit 77) rather than a false pass, so a missing lane cannot certify
//          the gate.
//
//          Build/run: python3 waf build --targets=render_backend_vulkan_conformance
//
//=============================================================================//

#include "conformance/render_backend_conformance.h"
#include "vulkan_render_backend.h"

#include <cstdio>
#include <memory>
#include <string>

int main()
{
	std::string initError;
	std::unique_ptr<render::IRenderBackendProvider> provider =
	    render_vulkan::MakeVulkanRenderBackend( &initError );
	if ( !provider )
	{
		std::printf(
		    "SKIP render.backend.vulkan: no usable Vulkan device (%s)\n", initError.c_str() );
		return 77; // explicit skip, never counted as a pass
	}

	const render::RenderBackendId id = provider->GetBackendId();
	std::printf( "render.backend.vulkan: provider '%s' (%s), %d adapter(s)\n", id.id ? id.id : "?",
	    id.name ? id.name : "?", provider->GetAdapterCount() );
	render::RenderAdapterInfo a0;
	if ( provider->GetAdapterInfo( 0, &a0 ) )
		std::printf( "  adapter0: %s (vendor 0x%04x, %.0f MiB, software=%d)\n", a0.name,
		    a0.vendorId, double( a0.deviceMemoryBytes ) / ( 1024.0 * 1024.0 ),
		    a0.isSoftware ? 1 : 0 );

	render::conformance::Report report;
	const bool passed = render::conformance::RunRenderBackendConformance( *provider, report );

	for ( const render::conformance::CheckResult &c : report.checks )
		if ( !c.ok )
			std::printf( "FAIL %s: %s\n", c.id.c_str(), c.detail.c_str() );

	std::printf( "render.backend.vulkan: %d check(s), %d failure(s) -> %s\n",
	    static_cast<int>( report.checks.size() ), report.FailureCount(), passed ? "PASS" : "FAIL" );

	if ( report.checks.size() < 20 )
	{
		std::printf( "FAIL suite ran too few checks (%d); expected the full obligation set\n",
		    static_cast<int>( report.checks.size() ) );
		return 1;
	}

	return passed ? 0 : 1;
}
