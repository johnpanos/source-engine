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

struct GlAdapterOptions
{
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
		bool unsafeUploadReuse = false;     // D10: ring ranges retire at submission
		bool transmittanceAsPremultiplied = false; // D21: kTransmittance drawn as kPremultiplied
	};
	Sensitivity sensitivity;
};

const DeviceProviderDescriptor &Describe();
DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const GlAdapterOptions &options );

// Messages the debug output reported for device so far (see
// validationCounter); 0 when validation is off or device was not made by this
// adapter.
std::uint64_t ValidationMessages( const IRenderDevice2 &device );
// Uploads that found the ring full and were copied from the encoder's own
// storage; 0 for a device not made by this adapter.
std::uint64_t DeferredUploads( const IRenderDevice2 &device );
// Tests only: the device reports kLost, as it does when the context reports a
// reset (GL_KHR_robustness), so the loss clause (D7) and Recover() run. False
// for a device not made by this adapter.
bool SimulateContextLoss( IRenderDevice2 &device );

} // namespace render::device::gl

#endif // RENDER_DEVICE_GL_PROVIDER_H
