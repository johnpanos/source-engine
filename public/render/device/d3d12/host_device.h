//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.d3d12 host interop (RFC 0024 X4), private to the
//			adapter's module (the render.backend.v1 provider in backend_v1/, and
//			the SDL3-D3D12 bridge and its tests through it): render.device.v2
//			over an ID3D12Device and direct queue another owner made, and a
//			native texture as a port texture. As render.device.vulkan's
//			host_device.h is for the Vulkan host: the host and the port share
//			the queue, so their submissions run in the order they are made.
//
//=============================================================================//

#ifndef RENDER_DEVICE_D3D12_HOST_DEVICE_H
#define RENDER_DEVICE_D3D12_HOST_DEVICE_H

#include "render/device/d3d12/provider.h"

#include <d3d12.h>
#include <dxgi1_6.h>

namespace render::device::d3d12
{

// A port device on the host's device and queue (each AddRef'd for the
// device's life). The host keeps the device and queue alive and outlives the
// port device. Loss cannot be recovered on a hosted device (Recover fails
// kFatal): the host recreates both.
DeviceResult<std::unique_ptr<IRenderDevice2>> CreateHosted( const D3d12AdapterOptions &options,
    IDXGIFactory4 *factory, ID3D12Device *device, ID3D12CommandQueue *queue );

// A native texture of the same ID3D12Device as a port texture, in `home`
// usage (whose D3D12 state it is in now: COMMON for kExternal); the port
// device holds a reference until the texture's Release completes. desc must
// describe it (format, size, one mip, one sample).
DeviceResult<TextureId> ImportTexture( IRenderDevice2 &device, ID3D12Resource *texture,
    const TextureDesc &desc, ResourceUsage home );

} // namespace render::device::d3d12

#endif // RENDER_DEVICE_D3D12_HOST_DEVICE_H
