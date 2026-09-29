//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan's host interop (RFC 0016 K1, "One Vulkan
//			stack"). PRIVATE to the Vulkan family: the only users are the
//			adapter itself, the legacy native Vulkan backend
//			(materialsystem/shaderapivulkan, which K3 turns into the legacy
//			frontend), its presentation bridge and their GPU suites. Nothing
//			under public/render/device/ includes it.
//
//			The adapter owns the instance, the physical-device choice, the
//			logical device, its queues, the memory allocator (the pinned Vulkan
//			Memory Allocator) and completion (one timeline semaphore). A host
//			borrows them:
//
//			- IHostDeviceFactory::Create builds the device. The host names its
//			  instance extensions (its surface's), creates its surface between
//			  instance and device creation, and describes the device features
//			  and extensions it needs; the adapter adds its own requirements
//			  (timeline semaphores, synchronization2, dynamic rendering) and
//			  fails with a named reason when a device lacks one.
//			- Every buffer and image the host uses is created, mapped and
//			  destroyed here (vkCreateBuffer, vkCreateImage and vkAllocateMemory
//			  appear nowhere else; tools/render/vulkan_scans.py checks it).
//			- Completion: every graphics submission the host makes signals the
//			  adapter's timeline with a value from NextSubmitValue(), so a value
//			  is a completion token. Release*After frees a resource once the
//			  timeline reaches the value: never before, and never by an idle
//			  wait. Collect() runs the releases that are due.
//			- Port() is render.device.v2 over the same device and queue.
//
//			Threading: the host and Port() share the graphics queue and the
//			timeline counter. Both run on the render sequence (RFC 0016
//			decision 8); queue submissions from anywhere else take QueueLock.
//			Memory creation, mapping and destruction are thread-safe.
//
//			ABI: the legacy backend keeps the engine's libstdc++ ABI while the
//			adapter uses the new one, so nothing here passes a dual-ABI library
//			type (std::string, std::list). Errors are copied into a caller
//			buffer.
//
//=============================================================================//

#ifndef RENDER_DEVICE_VULKAN_HOST_DEVICE_H
#define RENDER_DEVICE_VULKAN_HOST_DEVICE_H

// HostDeviceFactory(): the composition root hands it to the host
// (NativeVulkanShaderBackend_BindDeviceFactory); GPU suites call it directly.
#include "render/device/resources.h"
#include "render/device/vulkan/host_binding.h"

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <memory>

namespace render::device
{
struct CompletionToken;
class CommandEncoder;
class IRenderDevice2;
}

namespace render::device::vulkan
{

// One suballocation of the adapter's allocator.
struct HostAllocationTag;
using HostAllocation = HostAllocationTag *;

// What a host asks of its device's memory: the property flags a memory type
// must have (the first type in the device's order that has them all is used,
// as a per-resource vkAllocateMemory with the same flags would), and whether
// the allocation stays mapped for its lifetime.
struct HostMemory
{
	VkMemoryPropertyFlags required = 0;
	// Breaks ties between eligible types (the adapter's own readback memory
	// prefers cached); never makes a type ineligible.
	VkMemoryPropertyFlags preferred = 0;
	bool mapped = false;
};

// The extensions and features a host needs, for the device the adapter chose.
// features is the host's own chain (VkPhysicalDeviceFeatures2 at its head);
// the adapter sets its required bits in any structure of the chain that
// carries them and appends the structures it lacks. Both stay valid until
// Create returns.
struct HostDeviceFeatures
{
	VkPhysicalDeviceFeatures2 *features = nullptr;
	const char *const *extensions = nullptr;
	std::uint32_t extensionCount = 0;
};

struct HostDeviceRequest
{
	const char *applicationName = nullptr;
	// Instance extensions the host needs (its surface's), besides the
	// adapter's own (portability enumeration, debug utils when enabled).
	const char *const *instanceExtensions = nullptr;
	std::uint32_t instanceExtensionCount = 0;
	// The Khronos validation layer: wanted, and whether its absence fails.
	bool validation = false;
	bool requireValidation = false;
	// VK_EXT_debug_utils for object names and labels even without validation.
	bool debugUtils = false;
	// The host's validation callback, receiving warnings and errors.
	PFN_vkDebugUtilsMessengerCallbackEXT messageCallback = nullptr;
	void *messageUser = nullptr;
	bool requireDiscrete = false;
	// The physical device, by enumeration index; -1 picks the best one that
	// qualifies (discrete first).
	int adapterIndex = -1;

	void *user = nullptr;
	// Optional. Creates the presentation surface on the new instance; the
	// chosen device must then present to it from some queue family. The host
	// destroys the surface before it releases the device.
	bool ( *createSurface )( void *user, VkInstance instance, VkSurfaceKHR *surface ) = nullptr;
	// Optional. Describes what the host needs of the chosen device.
	void ( *describeDevice )( void *user, VkPhysicalDevice physical, std::uint32_t graphicsFamily,
	    HostDeviceFeatures *out ) = nullptr;
};

struct HostDeviceInfo
{
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physical = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE; // the one createSurface made, if any
	std::uint32_t graphicsFamily = 0;
	std::uint32_t presentFamily = 0;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	std::uint32_t instanceApiVersion = 0;
	bool validation = false; // the Khronos layer is enabled
	bool debugUtils = false; // VK_EXT_debug_utils is enabled
	bool core13 = false;     // synchronization2 and dynamic rendering are core
};

class IHostDevice
{
public:
	virtual ~IHostDevice() = default;

	virtual const HostDeviceInfo &Info() const = 0;
	// render.device.v2 over the same device.
	virtual IRenderDevice2 &Port() = 0;

	// Memory. Create* bind the object to its memory; Destroy* free both at
	// once and are for objects the GPU no longer uses (the host has waited,
	// or never submitted work that names them). mapped receives the address
	// when HostMemory::mapped is set.
	virtual VkResult CreateBuffer( const VkBufferCreateInfo &info, const HostMemory &memory,
	    VkBuffer *buffer, HostAllocation *allocation, void **mapped = nullptr ) = 0;
	virtual VkResult CreateImage( const VkImageCreateInfo &info, const HostMemory &memory,
	    VkImage *image, HostAllocation *allocation ) = 0;
	virtual void DestroyBuffer( VkBuffer buffer, HostAllocation allocation ) = 0;
	virtual void DestroyImage( VkImage image, HostAllocation allocation ) = 0;
	// Frees an allocation whose buffer or image the host destroyed itself
	// (vkDestroyBuffer and vkDestroyImage stay with the host's teardown).
	virtual void Free( HostAllocation allocation ) = 0;
	// Maps a host-visible allocation at its start (reference counted).
	virtual VkResult Map( HostAllocation allocation, void **data ) = 0;
	virtual void Unmap( HostAllocation allocation ) = 0;
	// Makes device writes visible to the host for a non-coherent type.
	virtual VkResult Invalidate(
	    HostAllocation allocation, VkDeviceSize offset, VkDeviceSize size ) = 0;
	virtual VkMemoryPropertyFlags Properties( HostAllocation allocation ) const = 0;

	// Completion. NextSubmitValue reserves the value the host's next
	// submission on the graphics queue must signal on Timeline(); values rise
	// by one per reservation and never repeat. A reserved value the host does
	// not signal (a failed submission) must be passed to AbandonValue so later
	// waits do not stall on it.
	virtual VkSemaphore Timeline() const = 0;
	virtual std::uint64_t NextSubmitValue() = 0;
	virtual void AbandonValue( std::uint64_t value ) = 0;
	virtual std::uint64_t SubmittedValue() const = 0;
	virtual std::uint64_t CompletedValue() = 0;
	// Waits until the timeline reaches value; VK_TIMEOUT after timeoutNs.
	virtual VkResult WaitValue( std::uint64_t value, std::uint64_t timeoutNs ) = 0;

	// Releases after completion: the resource is freed once the timeline has
	// reached value (at once when it already has). ReleaseAfter runs any
	// host destruction (views, framebuffers, pools) at that point.
	virtual void ReleaseBufferAfter(
	    std::uint64_t value, VkBuffer buffer, HostAllocation allocation ) = 0;
	virtual void ReleaseImageAfter(
	    std::uint64_t value, VkImage image, HostAllocation allocation ) = 0;
	virtual void ReleaseAfter(
	    std::uint64_t value, void ( *destroy )( void *object ), void *object ) = 0;
	// Runs every due release; returns how many ran.
	virtual std::size_t Collect() = 0;
	// Releases still waiting for their value.
	virtual std::size_t PendingReleases() const = 0;

	// The graphics queue's external synchronization, for submissions made
	// off the render sequence.
	virtual void LockQueue() = 0;
	virtual void UnlockQueue() = 0;

	// Live suballocations and bytes (evidence and leak checks).
	virtual std::uint64_t LiveAllocations() const = 0;
	virtual std::uint64_t LiveBytes() const = 0;

	// Host work inside a port encoder (RFC 0016 K3: the legacy frontend
	// records the legacy stream into the frame's graph passes). The adapter
	// calls record( user, cmd ) while it translates the encoder at Submit, at
	// this point of the encoder and outside any rendering the port opened.
	// The host touches only objects it owns (no port resource), and leaves
	// nothing bound that the port relies on: the adapter rebinds its own
	// pipeline, groups and viewport afterwards. An encoder that is rendering
	// fails its submission.
	using NativeRecord = void ( * )( void *user, VkCommandBuffer cmd );
	virtual void RecordNative( CommandEncoder &encoder, NativeRecord record, void *user ) = 0;
	// Port commands that host work runs where it chooses (RFC 0016 K5: core
	// passes inside the legacy scene). BeginSection and EndSection bracket
	// port commands recorded right after a RecordNative or after another
	// section of it; the record calls RunSection( cmd, index ) at the point
	// its section `index` belongs, outside any rendering of its own. Sections
	// run once each and in order: asking for one runs every earlier one not
	// yet run, and those never asked for run when the record returns. A
	// section begins and ends outside the port's rendering with every
	// imported texture in its home usage, closes the labels it opens, and
	// binds what it uses; the adapter orders it against the host work around
	// it as it orders host work against the port. An encoder that breaks
	// these rules fails its submission. RunSection is false outside a
	// record's translation, for another command buffer, or past the record's
	// last section.
	virtual void BeginSection( CommandEncoder &encoder ) = 0;
	virtual void EndSection( CommandEncoder &encoder ) = 0;
	virtual bool RunSection( VkCommandBuffer cmd, std::uint32_t index ) = 0;
	// Binary semaphores the encoder's submission waits on (before `stage`)
	// and signals: a swapchain image's acquire and present.
	virtual void AddSubmitWait(
	    CommandEncoder &encoder, VkSemaphore semaphore, VkPipelineStageFlags2 stage ) = 0;
	virtual void AddSubmitSignal( CommandEncoder &encoder, VkSemaphore semaphore ) = 0;
	// Host work submitted alone on the graphics queue (a frame outside any
	// frame graph): an encoder whose only work is record, waiting on `wait`
	// before `waitStage` and signaling `signal` (either may be null). True
	// with the submission's token, or false when the port refused it.
	virtual bool SubmitNative( NativeRecord record, void *user, VkSemaphore wait,
	    VkPipelineStageFlags2 waitStage, VkSemaphore signal, CompletionToken *token ) = 0;

	// A host image as a port texture (RFC 0016 K5: core passes that draw
	// into the legacy scene's targets). desc describes the image as the host
	// created it (format, extent, samples, mips) and names the usages the
	// port may put it in, all of which the image was created for. home is
	// the usage whose layout the host keeps the image in: it is there at
	// import, at every RecordNative point of a submission that uses it and
	// when the submission ends (Submit refuses a submission that leaves it
	// elsewhere), and the host's own accesses stay within home's stages and
	// accesses or are made available by its own barriers. The adapter orders
	// the port's first access after host work, and host work after the
	// port's writes. home may be kExternal (GENERAL): the host's own
	// accesses then read the image as another API would, after the port's
	// last writes, as a presentation bridge's blit reads its back buffer.
	// The adapter owns only its views: Port().Release( id,
	// token ) frees them after token, and the host keeps the image alive
	// until then. False, with no texture, when desc names no usage, home is
	// not one of them or has no image layout, or the device is lost.
	virtual bool ImportImage(
	    VkImage image, const TextureDesc &desc, ResourceUsage home, TextureId *texture ) = 0;
};

// An instance several host devices share (a provider that enumerates
// adapters before it creates devices, and whose bridges make surfaces on the
// instance). Its devices keep it alive.
class IHostInstance
{
public:
	virtual ~IHostInstance() = default;
	virtual VkInstance Instance() const = 0;
	// A device on this instance; request's instance fields are ignored.
	virtual std::unique_ptr<IHostDevice> CreateDevice(
	    const HostDeviceRequest &request, char *error, std::size_t errorSize ) = 0;
};

class IHostDeviceFactory
{
public:
	// A device for request, or null with the reason in error (always
	// terminated when errorSize > 0).
	virtual std::unique_ptr<IHostDevice> Create(
	    const HostDeviceRequest &request, char *error, std::size_t errorSize ) const = 0;
	// An instance with request's instance extensions, validation and messenger.
	virtual std::unique_ptr<IHostInstance> CreateInstance(
	    const HostDeviceRequest &request, char *error, std::size_t errorSize ) const = 0;

protected:
	~IHostDeviceFactory() = default;
};

} // namespace render::device::vulkan

#endif // RENDER_DEVICE_VULKAN_HOST_DEVICE_H
