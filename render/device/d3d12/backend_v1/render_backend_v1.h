//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.backend.v1 over Direct3D 12 (RFC 0024 X4): the provider the
//			SDL3-D3D12 presentation bridge (render/bridge/sdl3-d3d12) joins, as
//			render/device/vulkan/backend_v1 is for the SDL3-Vulkan bridge.
//
//			Each device owns an ID3D12Device, one direct queue and one fence;
//			a submission's token is the fence value it signals, and submissions
//			complete in order. Resources are textures with storage the bridge
//			sizes (back buffers) or plain handles (the contract's generic
//			resources). Destruction waits for the token of the resource's last
//			recorded use.
//
//			D3d12DeviceEndpoint is the bridge's private view: the native device
//			and queue, back-buffer textures, the native resource behind a
//			handle, a submission whose command list the bridge records, and the
//			completion gate tests hold (a queue Wait on a fence the CPU
//			signals on release, so held work is genuinely incomplete on the
//			GPU). No D3D type reaches the portable contract headers.
//
//=============================================================================//

#ifndef RENDER_DEVICE_D3D12_BACKEND_V1_RENDER_BACKEND_V1_H
#define RENDER_DEVICE_D3D12_BACKEND_V1_RENDER_BACKEND_V1_H

#include "render/device/device.h"
#include "render/render_backend.h"

// The Windows headers' min and max macros would break std::min and std::max.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace render_d3d12
{

class D3d12DeviceEndpoint
{
public:
	virtual ID3D12Device *NativeDevice() const = 0;
	virtual ID3D12CommandQueue *Queue() const = 0;
	virtual IDXGIFactory4 *Factory() const = 0;

	// A texture resource of the device (a back buffer): render target and
	// copy source, in COMMON state between submissions. kInvalidResource on
	// failure.
	virtual render::RenderResourceHandle CreateTexture(
	    std::uint32_t width, std::uint32_t height, DXGI_FORMAT format ) = 0;
	// The texture behind a live handle, or nullptr.
	virtual ID3D12Resource *Texture( render::RenderResourceHandle handle ) const = 0;

	// Submits a command list the callback records (in submission order with
	// Submit), recording 'uses' for deferred destruction; the token, or nullptr.
	virtual render::IRenderCompletionToken *SubmitRecorded(
	    const std::function<void( ID3D12GraphicsCommandList & )> &record,
	    const render::RenderResourceHandle *uses, std::size_t useCount ) = 0;
	// Adds native work to a context of this device: Submit records it, in
	// order, into the submission's command list (harnesses clear back buffers
	// with it).
	virtual void RecordNative( render::IRenderCommandContext &context,
	    std::function<void( ID3D12GraphicsCommandList & )> record ) = 0;
	// The fence value a token signals (0 for a foreign token).
	virtual std::uint64_t TokenValue( const render::IRenderCompletionToken &token ) const = 0;
	// Blocks until the fence reaches 'value' (bridge teardown and surface
	// release only, never a frame). False when the device is lost.
	virtual bool WaitFor( std::uint64_t value ) = 0;
	// Whether the fence has reached 'value', without blocking.
	virtual bool Completed( std::uint64_t value ) const = 0;
	// The value of the latest submission.
	virtual std::uint64_t LastSubmitted() const = 0;

	// render.device.v2 on this device's ID3D12Device and queue (host_device.h
	// CreateHosted, made on first use, destroyed before the device): the
	// core draws into a presentation's back buffer imported with
	// render::device::d3d12::ImportTexture, in submission order with the
	// bridge's copy and present. nullptr if it cannot be created.
	virtual render::device::IRenderDevice2 *Port() = 0;

	// Test hooks: while held, every later submission waits on the GPU for the
	// release. HasIncompleteGpuWork polls the fence without blocking.
	virtual bool HoldCompletion() = 0;
	virtual void ReleaseCompletion() = 0;
	virtual bool HasIncompleteGpuWork() const = 0;

protected:
	~D3d12DeviceEndpoint() = default;
};

class D3d12RenderBackend : public render::IRenderBackendProvider
{
public:
	virtual D3d12DeviceEndpoint *FindDevice( render::IRenderDevice &device ) = 0;
};

// The provider; nullptr and *outError when no D3D12 adapter opens.
std::unique_ptr<D3d12RenderBackend> MakeD3d12RenderBackend( std::string *outError );

} // namespace render_d3d12

#endif // RENDER_DEVICE_D3D12_BACKEND_V1_RENDER_BACKEND_V1_H
