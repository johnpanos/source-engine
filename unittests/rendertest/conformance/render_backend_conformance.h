//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared render-backend conformance suite (RFC 0001 "Render backend
//			conformance tests", verified under RFC 0005 Q-PRESENTATION/Q-PRODUCT).
//			One reusable suite that exercises every backend-agnostic obligation of
//			the render.contracts API against ANY provider implementing
//			render::IRenderBackendProvider -- the deterministic null fake today and
//			a Vulkan/D3D9/GL provider when those land. This is the "one shared
//			suite against every claiming implementation, including fakes" the LSP
//			policy in AGENTS.md requires.
//
//			The suite reports structured results and never aborts, so a positive
//			run can assert it passes and a sensitivity run can assert a broken
//			provider is DETECTED. Optional sections (offscreen devices, device-loss
//			recovery) run only when the provider advertises the capability, so a null/recording provider certifies only its
//			claimed command and lifetime behavior -- not image fidelity, which
//			needs real render-provider evidence.
//
//=============================================================================//

#ifndef RENDERTEST_RENDER_BACKEND_CONFORMANCE_H
#define RENDERTEST_RENDER_BACKEND_CONFORMANCE_H

#include "render/render_backend.h"

#include <string>
#include <vector>

namespace render
{
namespace conformance
{

// A single recorded check. 'id' is a stable identifier (e.g. "caps.immutable")
// so sensitivity tests can assert exactly which obligation a broken provider
// violated.
struct CheckResult
{
	std::string id;
	bool ok = false;
	std::string detail;
};

// The full result of a conformance run.
struct Report
{
	std::vector<CheckResult> checks;
	// Obligations this run could not exercise, with the reason. A skip never
	// counts as a pass; another profile must cover it.
	std::vector<CheckResult> skipped;

	void Skip( const std::string &id, const std::string &reason )
	{
		CheckResult result;
		result.id = id;
		result.ok = false;
		result.detail = reason;
		skipped.push_back( result );
	}

	void Record( const std::string &id, bool ok, const std::string &detail = "" )
	{
		CheckResult result;
		result.id = id;
		result.ok = ok;
		result.detail = detail;
		checks.push_back( result );
	}

	bool Passed() const
	{
		for ( const CheckResult &c : checks )
		{
			if ( !c.ok )
				return false;
		}
		return true;
	}

	int FailureCount() const
	{
		int n = 0;
		for ( const CheckResult &c : checks )
		{
			if ( !c.ok )
				++n;
		}
		return n;
	}

	// True if a check with this id was recorded and failed. Used by sensitivity
	// tests to assert the intended obligation was the one caught.
	bool Failed( const std::string &id ) const
	{
		for ( const CheckResult &c : checks )
		{
			if ( c.id == id && !c.ok )
				return true;
		}
		return false;
	}
};

// Runs the full conformance suite against 'provider', recording every check into
// 'report'. Does not abort; returns report.Passed() for convenience.
bool RunRenderBackendConformance( render::IRenderBackendProvider &provider, Report &report );

} // namespace conformance
} // namespace render

#endif // RENDERTEST_RENDER_BACKEND_CONFORMANCE_H
