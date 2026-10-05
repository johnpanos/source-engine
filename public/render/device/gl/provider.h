//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl, the OpenGL 4.5 adapter of render.device.v2
//			(RFC 0016 K10). The composition root names this header; nothing
//			here reaches a GL, EGL, SDL or other native header (CAP007, CAP005).
//
//			The adapter needs an OpenGL 4.5 core context (glClipControl, direct
//			state access, fence sync objects, compute). It accepts GLSL 4.50
//			artifacts (ArtifactFormat::kGlsl450), which the build cross-compiles
//			from the SPIR-V with the pinned SPIRV-Cross
//			(tools/render/shader_artifacts.py cross_compile owns their form).
//
//			The same adapter has an OpenGL ES 3.1 dialect (RFC 0022,
//			GlAdapterOptions::api, the "gles" descriptor): it needs an ES 3.1
//			context with GL_EXT_clip_control, per-attachment blend state and
//			base-vertex draws, and accepts GLSL ES 3.10 artifacts
//			(ArtifactFormat::kGlslEs310) of the same form.
//
//			Its context is its own (EGL, surfaceless, so it needs no window)
//			and is current only inside the device's calls, on the calling
//			thread, which restores what was current before. One sequence calls
//			the device at a time (the render sequence, RFC 0016 "Threading");
//			encoders record CPU command lists on any thread, and Submit replays
//			them on the calling thread in submission order. Tokens are fence
//			sync objects.
//
//=============================================================================//

#ifndef RENDER_DEVICE_GL_PROVIDER_H
#define RENDER_DEVICE_GL_PROVIDER_H

#include "render/device/provider.h"

#include <atomic>
#include <cstdint>
#include <memory>

namespace render::device::gl
{

// The GL dialect a device speaks (RFC 0022).
enum class GlApiKind : std::uint8_t
{
	kDesktop45, // OpenGL 4.5 core
	kEs31       // OpenGL ES 3.1 (a 3.2 context is accepted)
};

struct GlAdapterOptions
{
	GlApiKind api = GlApiKind::kDesktop45;
	// Enable GL debug output (KHR_debug) on a debug context. Without it the
	// device is created anyway.
	bool validation = false;
	// Bytes of the adapter-owned upload ring (at least 4096).
	std::uint64_t uploadRingBytes = std::uint64_t( 4 ) << 20;
	// Optional (tests and evidence): every error, and every warning of high or
	// medium severity, the debug output reports is counted here. The counter
	// must outlive the device.
	std::atomic<std::uint64_t> *validationCounter = nullptr;
	// The capabilities the device may claim: those it has and this allows. A
	// profile masks a capability to take the path that works without it
	// (RFC 0016 K10 "Capability negotiation").
	CapabilitySet allowed = CapabilitySet::All();
	// Sensitivity fixtures only (render.device.v2.gl.sensitivity): each makes
	// the adapter break one port rule the shared suite must catch. Never set
	// by a product.
	struct Sensitivity
	{
		bool lowerLeftOrigin = false;       // D13: GL's own origin, clip Y down the rows
		bool negativeOneToOneDepth = false; // D13: GL's own -1 to 1 clip depth
		CapabilitySet falseClaims{};        // D15: claimed in the facts, not implemented
		bool ignoreColorWriteMasks = false; // D17: every channel written
		bool dropSpecialization = false;    // D20: the constants never reach the program
		bool reverseSamplerComparison = false; // D24: compare reference >= stored
		bool unsafeUploadReuse = false;     // D10: ring ranges retire at submission
		bool transmittanceAsPremultiplied = false; // D21: kTransmittance drawn as kPremultiplied
	};
	Sensitivity sensitivity;
};

// "gl" (desktop 4.5) and "gles" (ES 3.1).
const DeviceProviderDescriptor &Describe();
const DeviceProviderDescriptor &DescribeEs();
DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const GlAdapterOptions &options );

// Messages the debug output reported for device so far (see
// validationCounter); 0 when validation is off or device was not made by this
// adapter.
std::uint64_t ValidationMessages( const IRenderDevice2 &device );
// Uploads that found the ring full and were copied from the encoder's own
// storage; 0 for a device not made by this adapter.
std::uint64_t DeferredUploads( const IRenderDevice2 &device );
// Tests only: while held (true), Submit accepts work and returns its token but
// issues none of it to GL, as a driver that has not begun the work yet: the
// latest schedule a token allows, so a ring range handed out again before its
// token completes is overwritten before it is read, on any driver. Releasing
// (false) issues the held submissions in order; WaitIdle issues them too.
// False for a device not made by this adapter.
bool HoldSubmissions( IRenderDevice2 &device, bool held );
// Tests only: the device reports kLost, as it does when the context reports a
// reset (GL_KHR_robustness), so the loss clause (D7) and Recover() run. False
// for a device not made by this adapter.
bool SimulateContextLoss( IRenderDevice2 &device );

} // namespace render::device::gl

#endif // RENDER_DEVICE_GL_PROVIDER_H
