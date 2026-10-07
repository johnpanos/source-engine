//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica, the PICA200 (Nintendo 3DS) adapter of
//			render.device.v2 (RFC 0026). The composition root names this
//			header; nothing here reaches a libctru, citro3d, SDL or other
//			native header (CAP007, CAP005).
//
//			The PICA200's fragment stage is fixed function, so the adapter
//			accepts ArtifactFormat::kPica: a PVS1 vertex program (a picasso
//			DVLB with the port's uniform mapping) and a PFP1 combiner program
//			(render/device/pica/artifacts.h owns both forms). It claims none
//			of the optional capabilities.
//
//			Only the 3DS build can create a device; elsewhere Create fails
//			with kUnsupported, naming the adapter, and the host suite checks
//			the adapter's portable parts alone.
//
//=============================================================================//

#ifndef RENDER_DEVICE_PICA_PROVIDER_H
#define RENDER_DEVICE_PICA_PROVIDER_H

#include "render/device/provider.h"

#include <cstdint>
#include <cstddef>
#include <memory>
#include <span>

namespace render::device::pica
{

struct PicaAdapterOptions
{
	// Bytes of citro3d's GPU command buffer, which one frame of draws fills
	// (at least 4096; the device opens a new frame when it is full). Uploads
	// need no ring: the replay copies them on the CPU.
	std::uint32_t commandListBytes = 256u << 10;
	// Sensitivity fixtures only (render.device.v2.pica.sensitivity): each makes
	// the adapter break one port rule the shared suite must catch. Never set
	// by a product.
	struct Sensitivity
	{
		bool depthNotNegated = false;       // D13: clip depth mapped with the wrong sign
		bool dropDrawConstants = false;     // D16: combiner constants ignore the block
		bool ignoreColorWriteMasks = false; // D17: every channel written
		bool blendAsOpaque = false;         // D21: every blend mode drawn opaque
		bool texturesUpsideDown = false;    // sampled: uploads stored bottom row first
		bool windingReversed = false;       // raster: the front face is the other one
		bool cpuRacesGpu = false;           // raster: CPU writes never wait for the GPU
		bool etc1BytesUnswapped = false;    // D40: ETC1 words kept in the port's byte order
	};
	Sensitivity sensitivity;
};

// Tests only: the device reports kLost, so the loss clause (D7) and Recover()
// run (the PICA200 has no loss of its own to force). False for a device not
// made by this adapter.
bool SimulateDeviceLoss( IRenderDevice2 &device );

// RFC 0026 P4: shows the region (0, 0, width, height) of source on the 3DS
// top screen (400 x 240), scaled to fill it, and returns once the GPU has
// written the screen's framebuffer (the display swaps at its next vertical
// blank). source is a texture of this device in kSampled usage that the GPU
// can sample (a power of two of at least 8 on a side). Fails
// kInvalidHandle or kInvalidState for a source that does not qualify, and
// kUnsupported on a device not made by this adapter or off the 3DS.
DeviceResult<void> PresentTopScreen(
    IRenderDevice2 &device, TextureId source, std::uint32_t width, std::uint32_t height );

// RFC 0026 P5 (the legacy frontend's bridge): the bytes of a kUpload buffer
// of this device, which lives in GPU-visible linear memory, for the CPU to
// write in place. The device submits synchronously, so the bytes are free to
// write whenever no recorded, unsubmitted encoder reads them; the caller
// owns that rule. After writing, FlushUploadBuffer makes the range visible
// to the GPU. An empty span for any other buffer or device.
std::span<std::byte> MapUploadBuffer( IRenderDevice2 &device, BufferId buffer );
void FlushUploadBuffer(
    IRenderDevice2 &device, BufferId buffer, std::uint64_t offset, std::uint64_t size );

// "pica".
const DeviceProviderDescriptor &Describe();

DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const PicaAdapterOptions &options );

} // namespace render::device::pica

#endif // RENDER_DEVICE_PICA_PROVIDER_H
