//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.d3d12, the Direct3D 12 adapter of render.device.v2
//			(RFC 0024). The composition root names this header; nothing here
//			reaches a D3D, DXGI, COM or Windows header (CAP007, CAP011).
//
//			The adapter opens the default DXGI adapter at feature level 11_0 or
//			above and one direct queue. Tokens are values of one ID3D12Fence.
//			Encoders record CPU command lists (any thread); Submit validates
//			them against the usage state, then replays them into one
//			command list per submission on the calling thread.
//
//			Gate X1 (RFC 0024): resources, uploads, copies, clears, attachment
//			passes, timestamps, completion, release and loss. Pipelines need
//			DXIL artifacts (X2): until then CreatePipeline fails with
//			kUnsupported, after the port's shared description rules.
//
//			On Linux the required lane runs it under Wine with vkd3d-proton
//			(tools/render/d3d12_lane.py).
//
//=============================================================================//

#ifndef RENDER_DEVICE_D3D12_PROVIDER_H
#define RENDER_DEVICE_D3D12_PROVIDER_H

#include "render/device/provider.h"

#include <atomic>
#include <cstdint>
#include <memory>

namespace render::device::d3d12
{

struct D3d12AdapterOptions
{
	// Enable the D3D12 debug layer and count its error and corruption
	// messages (validationCounter). Without a debug layer the device is
	// created anyway.
	bool validation = false;
	// Bytes of the adapter-owned upload ring (at least 4096).
	std::uint64_t uploadRingBytes = std::uint64_t( 4 ) << 20;
	// Optional: every error message the debug layer reports is counted here.
	// The counter must outlive the device.
	std::atomic<std::uint64_t> *validationCounter = nullptr;
	// The capabilities the device may claim: those it has and this allows.
	CapabilitySet allowed = CapabilitySet::All();
	// Sensitivity fixtures only: each breaks one port rule the shared suite
	// must catch. Never set by a product.
	struct Sensitivity
	{
		bool unsafeUploadReuse = false; // D10: ring ranges retire at submission
		bool skipReleaseWait = false;   // D5: released resources free at once
	};
	Sensitivity sensitivity;
};

const DeviceProviderDescriptor &Describe(); // "d3d12"
DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const D3d12AdapterOptions &options );

// Debug-layer messages counted so far; 0 for a device not made here.
std::uint64_t ValidationMessages( const IRenderDevice2 &device );
// Uploads that found the ring full; 0 for a device not made here.
std::uint64_t DeferredUploads( const IRenderDevice2 &device );
// Tests only: while held, Submit accepts work and returns its token but
// executes nothing; releasing executes the held submissions in order, as does
// WaitIdle. False for a device not made here.
bool HoldSubmissions( IRenderDevice2 &device, bool held );
// Tests only: the device reports kLost, as after DXGI_ERROR_DEVICE_REMOVED, so
// D7 and Recover() run. False for a device not made here.
bool SimulateDeviceLoss( IRenderDevice2 &device );

} // namespace render::device::d3d12

#endif // RENDER_DEVICE_D3D12_PROVIDER_H
