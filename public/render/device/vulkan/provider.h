//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan, the Vulkan adapter of render.device.v2
//			(RFC 0016 K1). The composition root names this header; nothing
//			here reaches a Vulkan, SDL or other native header (CAP007, CAP005).
//
//			The adapter requires timeline semaphores, synchronization2 and
//			dynamic rendering (Vulkan 1.3 core, or 1.2 with the extensions). A
//			device without one fails creation with kUnsupported; nothing falls
//			back. It creates no surface: presentation stays with the
//			render.presentation.v1 bridge.
//
//=============================================================================//

#ifndef RENDER_DEVICE_VULKAN_PROVIDER_H
#define RENDER_DEVICE_VULKAN_PROVIDER_H

#include "render/device/provider.h"

#include <atomic>
#include <cstdint>
#include <memory>

namespace render::device::vulkan
{

struct VulkanAdapterOptions
{
	// Enable the Khronos validation layer (with synchronization validation)
	// when it is installed. Without the layer the device is created anyway.
	bool validation = false;
	// Index into the enumerated physical devices; -1 picks the first discrete
	// adapter, else the first integrated one, else the first that qualifies.
	int adapterIndex = -1;
	// Bytes of the adapter-owned upload ring (at least 4096).
	std::uint64_t uploadRingBytes = std::uint64_t( 4 ) << 20;
	// Optional (tests and evidence): every warning or error the validation
	// layer reports, including those at teardown, is also counted here. The
	// counter must outlive the device.
	std::atomic<std::uint64_t> *validationCounter = nullptr;

	// Sensitivity fixtures only (render.device.v2.vulkan.sensitivity): each
	// makes the adapter break one port rule the shared suite must catch.
	// Never set by a product.
	struct Sensitivity
	{
		bool flipY = false;          // D13: clip Y down (no viewport flip)
		bool glDepthRange = false;   // D13: clip depth -1 to 1 mapped onto 0 to 1
		CapabilitySet falseClaims{}; // D15: claimed in the facts, not implemented
		bool ignoreColorWriteMasks = false; // D17: every channel written
		bool staleExport = false;           // D18: the export names memory the image does not use
		bool nullExternalImages = false;    // D18: claims kExternalImages, exports nothing
		bool transmittanceAsPremultiplied = false; // D21: kTransmittance drawn as kPremultiplied
	};
	Sensitivity sensitivity;
};

const DeviceProviderDescriptor &Describe();
DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const VulkanAdapterOptions &options );

// Warnings and errors the validation layer reported for device so far; 0 when
// validation is off or device was not made by this adapter.
std::uint64_t ValidationMessages( const IRenderDevice2 &device );
// Uploads that found the ring full and took a dedicated staging buffer; 0
// for a device not made by this adapter.
std::uint64_t DeferredUploads( const IRenderDevice2 &device );
// True when the Khronos validation layer is installed.
bool ValidationLayerAvailable();

} // namespace render::device::vulkan

#endif // RENDER_DEVICE_VULKAN_PROVIDER_H
