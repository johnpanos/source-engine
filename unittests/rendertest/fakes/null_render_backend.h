//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic null/recording render backend provider for the RFC 0001
//			conformance suite. It implements render::IRenderBackendProvider fully
//			in memory with no GPU: adapter enumeration, structured creation
//			failure, immutable capabilities, child-resource lifetime with
//			provider-owned completion tokens, ordered submission/completion,
//			and device-loss recovery. Presentation is not a device concern; the
//			headless bridge (headless_presentation_bridge.h) presents from it. It certifies only command and lifetime behavior -- never
//			image fidelity -- exactly as RFC 0001 requires of a null provider.
//
//			The same implementation doubles as the sensitivity oracle: injecting
//			one NullBackendDefect makes it violate exactly one obligation, which
//			the shared suite must then DETECT. This is the "deliberately bad
//			provider" the LSP policy in AGENTS.md requires so the suite's own
//			teeth are proven.
//
//=============================================================================//

#ifndef RENDERTEST_NULL_RENDER_BACKEND_H
#define RENDERTEST_NULL_RENDER_BACKEND_H

#include "render/render_backend.h"

#include <memory>

namespace rendertest
{

// A single injectable defect. A correct provider leaves every field false. Each
// flag corresponds to one obligation the shared conformance suite pins, so the
// sensitivity test can enable one flag and assert the matching check fails.
struct NullBackendDefects
{
	// Capabilities change between queries (violates caps.immutable).
	bool mutableCaps = false;
	// Deferred destruction fires when a frame/submit counter advances instead of
	// when the completion token completes (violates resource.deferred_until_complete;
	// this is precisely the "frame index is not proof" rule of RFC 0006).
	bool recycleOnFrameAdvance = false;
	// Submissions complete out of order (violates submit.ordered_completion).
	bool unorderedCompletion = false;
	// Destroyed resources are never recycled (violates resource.recycled_after_complete
	// and lifetime accounting).
	bool leakResources = false;
	// Required-feature validation is skipped and a device is returned anyway
	// (violates create.unsupported_required_feature).
	bool swallowRequiredFeature = false;
	// Device loss jumps straight to Available without entering kDeviceLost
	// (violates loss.enters_lost_state).
	bool lossSkipsLostState = false;
	// The provider does not recognize devices it created, so a presentation bridge
	// could not reject a foreign device (violates lifetime.owns_device).
	bool disownDevices = false;
};

// Builds a null render backend provider. With the default (all-false) defects it
// is a fully conforming provider.
std::unique_ptr<render::IRenderBackendProvider> MakeNullRenderBackend(
	NullBackendDefects defects = NullBackendDefects{} );

} // namespace rendertest

#endif // RENDERTEST_NULL_RENDER_BACKEND_H
