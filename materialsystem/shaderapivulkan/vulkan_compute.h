//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The native Vulkan compute foundation (RFC 0011 G5): what a device
//          can do for compute producers, queried per device and enabled
//          through a VkPhysicalDeviceFeatures2 chain; storage buffers and
//          images; compute programs and dispatch on the graphics queue; and
//          retirement of every compute resource behind the completion serial
//          of the last submission that may use it.
//
//          The owner (CVulkanContext, or a headless test device) creates the
//          VkDevice with DeviceFeatureChain, then hands the device to
//          ComputeResources. Dispatches are recorded into the owner's command
//          buffer; a dispatch's writes are visible to later compute and
//          fragment reads in that buffer (RecordDispatch's barrier).
//
//===========================================================================//

#ifndef VULKAN_COMPUTE_H
#define VULKAN_COMPUTE_H

#include "render/gpu_compute.h"

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace render_vulkan
{

// A device's compute facts, from queries only.
struct ComputeCaps
{
	uint32_t deviceApiVersion = 0;
	bool compute = false;        // the graphics queue family also computes
	bool storageImages = false;  // RGBA16F and R32F storage images, optimal tiling
	bool rayQuery = false;       // VK_KHR_ray_query + acceleration structures, all features
	bool timelineSemaphore = false;
	uint32_t maxWorkgroupInvocations = 0;
	uint32_t maxBoundDescriptorSets = 0;
};

ComputeCaps QueryComputeCaps( VkPhysicalDevice physical, uint32_t queueFamily );

// The semantic feature bits (render/render_backend.h RenderFeature) of a caps
// value: kComputeShaders, kStorageImages, kRayQuery. Pass the *enabled* caps
// of a created device; that is the only truthful source of these bits.
uint32_t ComputeFeatureBits( const ComputeCaps &caps );

// Builds the device create chain: the caller's VkPhysicalDeviceFeatures (in
// features2.features) plus, for each queried capability, its features and
// extensions. `enabled` records exactly what the chain enables; a feature
// bit may be reported only when it is enabled here.
class DeviceFeatureChain
{
public:
	void Build( VkPhysicalDevice physical, const ComputeCaps &caps,
	    const VkPhysicalDeviceFeatures &baseFeatures, std::vector<const char *> *extensions );
	// Links into VkDeviceCreateInfo::pNext (pEnabledFeatures must be null).
	const void *Chain() const { return &m_features2; }
	const ComputeCaps &Enabled() const { return m_enabled; }

private:
	VkPhysicalDeviceFeatures2 m_features2 = {};
	VkPhysicalDeviceVulkan12Features m_vulkan12 = {};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructure = {};
	VkPhysicalDeviceRayQueryFeaturesKHR m_rayQuery = {};
	ComputeCaps m_enabled;
};

enum class ComputeBinding : uint8_t
{
	StorageBuffer,
	StorageImage,
};

class ComputeResources
{
public:
	// `physical`/`device` outlive this object; Shutdown releases everything
	// (after the owner waited for the device).
	bool Init( VkPhysicalDevice physical, VkDevice device, const ComputeCaps &enabled,
	    std::string *error );
	void Shutdown();
	bool Ready() const { return m_device != VK_NULL_HANDLE && m_enabled.compute; }
	const ComputeCaps &Enabled() const { return m_enabled; }

	// Resources, by handle (0 is never valid).
	uint32_t CreateBuffer( size_t bytes, std::string *error );
	uint32_t CreateStorageImage( uint32_t width, uint32_t height, VkFormat format,
	    std::string *error );
	// A program over the given bindings (set 0, in order) and push bytes.
	uint32_t CreateProgram( const uint32_t *spirv, size_t bytes,
	    const std::vector<ComputeBinding> &bindings, uint32_t pushBytes, std::string *error );

	// Host-visible buffer memory (buffers are host-coherent).
	void *Map( uint32_t buffer );
	VkImage Image( uint32_t image ) const;
	VkBuffer BufferHandle( uint32_t buffer ) const;

	// Records bind, push, dispatch and a barrier making the writes visible to
	// later compute and fragment reads and transfers, into the submission
	// with serial `serial` (its descriptor set is freed once that completes).
	// Images are in GENERAL layout (RecordImageLayout moves them there once).
	bool RecordDispatch( VkCommandBuffer cmd, uint64_t serial, uint32_t program,
	    const std::vector<uint32_t> &resources, const void *push, uint32_t pushBytes,
	    uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ );
	void RecordImageLayout( VkCommandBuffer cmd, uint32_t image );

	// Retirement: `resource` is destroyed once Collect sees `afterSerial`
	// complete (the last submission that may read it).
	void Retire( uint32_t resource, uint64_t afterSerial );
	void Collect( uint64_t completedSerial );
	bool Alive( uint32_t resource ) const;
	size_t PendingRetirements() const { return m_retired.size(); }
	// The seeded early-free defect (the retirement check's control): destroy
	// at Retire, ignoring the serial.
	void SetEarlyFreeDefect( bool on ) { m_earlyFree = on; }

private:
	struct Resource
	{
		uint32_t handle = 0;
		ComputeBinding kind = ComputeBinding::StorageBuffer;
		bool program = false;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		void *mapped = nullptr;
		VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkDescriptorPool pool = VK_NULL_HANDLE;
		std::vector<ComputeBinding> bindings;
		uint32_t pushBytes = 0;
		// A program's descriptor sets in flight, with their submission serials.
		std::vector<std::pair<VkDescriptorSet, uint64_t>> sets;
	};
	struct Retired
	{
		uint32_t handle;
		uint64_t afterSerial;
	};

	Resource *Find( uint32_t handle );
	const Resource *Find( uint32_t handle ) const;
	bool Memory( const VkMemoryRequirements &requirements, VkMemoryPropertyFlags flags,
	    VkDeviceMemory *memory, std::string *error );
	void Destroy( Resource &resource );

	VkPhysicalDevice m_physical = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	ComputeCaps m_enabled;
	std::vector<Resource> m_resources;
	std::vector<Retired> m_retired;
	uint32_t m_next = 0;
	bool m_earlyFree = false;
};

// render/gpu_compute.h over ComputeResources: the renderer's compute service
// for engine-side producers. The owner records the queued dispatches into its
// next submission (Record) and reports serials: `nextSerial` is the serial of
// that submission, `completedSerial` the newest complete one.
class GpuComputeService final : public gpu_compute::IGpuCompute
{
public:
	GpuComputeService( ComputeResources &resources, std::function<uint64_t()> nextSerial,
	    std::function<uint64_t()> completedSerial )
	    : m_resources( resources ), m_nextSerial( std::move( nextSerial ) ),
	      m_completedSerial( std::move( completedSerial ) )
	{
	}
	gpu_compute::Caps Capabilities() const override;
	uint32_t CreateBuffer( size_t bytes ) override;
	void *Map( uint32_t buffer ) override;
	uint32_t CreateProgram( const char *name, uint32_t buffers, uint32_t pushBytes ) override;
	uint64_t QueueDispatch( uint32_t program, const uint32_t *buffers, uint32_t count,
	    const void *push, uint32_t pushBytes, uint32_t groupsX, uint32_t groupsY,
	    uint32_t groupsZ ) override;
	uint64_t CompletedSerial() const override { return m_completedSerial(); }
	void Retire( uint32_t resource, uint64_t afterSerial ) override
	{
		m_resources.Retire( resource, afterSerial );
	}
	// Records every queued dispatch, in order, into the owner's submission.
	void Record( VkCommandBuffer cmd, uint64_t serial );
	bool Pending() const { return !m_queue.empty(); }

private:
	struct Queued
	{
		uint32_t program;
		std::vector<uint32_t> buffers;
		std::vector<unsigned char> push;
		uint32_t groups[3];
	};
	ComputeResources &m_resources;
	std::function<uint64_t()> m_nextSerial;
	std::function<uint64_t()> m_completedSerial;
	std::vector<Queued> m_queue;
};

} // namespace render_vulkan

#endif // VULKAN_COMPUTE_H
