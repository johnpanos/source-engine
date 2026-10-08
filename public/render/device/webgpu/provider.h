//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.webgpu, the WebGPU adapter of render.device.v2
//			(RFC 0029). The composition root names this header; nothing here
//			reaches webgpu.h, Dawn, Emscripten or SDL (CAP007, CAP005).
//
//			The adapter is written against the standard webgpu.h, so the same
//			sources run natively on the pinned Dawn (quality/toolchain/
//			webgpu.json; tools/render/webgpu_lane.py) and in a browser through
//			emdawnwebgpu. It accepts WGSL artifacts (ArtifactFormat::kWgsl),
//			which the build translates from the SPIR-V with the pinned tint
//			(tools/render/shader_artifacts.py wgsl_compile owns their form):
//			bind group g is @group(g), the draw constants (D16) a uniform block
//			the adapter binds with a dynamic offset in group 3, and each
//			artifact lists the WebGPU binding types its stage uses, which the
//			port's layouts do not say.
//
//			One sequence calls the device at a time (the render sequence, RFC
//			0016 "Threading"); encoders record CPU command lists on any thread,
//			and Submit replays them into one command buffer per submission, in
//			submission order, on the one queue. Tokens complete in order, when
//			the queue reports the submission's work done and every readback
//			buffer it wrote has been copied to the CPU; the device advances
//			them as Poll, IsComplete and WaitIdle process WebGPU's events, so
//			no product path blocks (WaitIdle and pipeline creation wait, which
//			a browser build allows through JSPI, RFC 0029 decision 7).
//
//			The port's conventions are WebGPU's own: clip depth 0 to 1, clip +Y
//			up, row 0 the top row. No correction is applied.
//
//=============================================================================//

#ifndef RENDER_DEVICE_WEBGPU_PROVIDER_H
#define RENDER_DEVICE_WEBGPU_PROVIDER_H

#include "render/device/provider.h"

#include <atomic>
#include <cstdint>
#include <memory>

namespace render::device::webgpu
{

struct WebGpuAdapterOptions
{
	// Labels objects and reports every WebGPU validation error (the uncaptured
	// error callback) to stderr and to validationCounter.
	bool validation = false;
	std::atomic<std::uint64_t> *validationCounter = nullptr;
	// The capabilities the device may claim: those it has and this allows
	// (RFC 0016 K10 "Capability negotiation").
	CapabilitySet allowed = CapabilitySet::All();
	// Deliberately bad behavior for the shared suite's sensitivity checks;
	// never set in a product.
	struct Sensitivity
	{
		bool skipReleaseWait = false;  // Release frees at the next Poll, token or not
		bool completeOnSubmit = false; // a token is complete as soon as it is submitted
	} sensitivity;
};

// "webgpu".
const DeviceProviderDescriptor &Describe();
DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const WebGpuAdapterOptions &options );

// WebGPU validation errors reported so far; 0 for a device not made by this
// adapter.
std::uint64_t ValidationErrors( const IRenderDevice2 &device );
// Tests only: the device reports kLost, as it does when WebGPU loses the
// device, so the loss clause (D7) and Recover() run. False for a device not
// made by this adapter.
bool SimulateDeviceLoss( IRenderDevice2 &device );
// Tests only: while held, submissions are recorded but not handed to the
// queue; releasing submits them in order (the shared suite's hold, D5/D10).
bool HoldSubmissions( IRenderDevice2 &device, bool held );

} // namespace render::device::webgpu

#endif // RENDER_DEVICE_WEBGPU_PROVIDER_H
