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
#include "vulkan_debug_utils.h"
#include "vulkan_shader_library.h"

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace render_vulkan
{

// A device's compute facts, from queries only.
struct ComputeCaps
{
	uint32_t deviceApiVersion = 0;
	bool compute = false;       // the graphics queue family also computes
	bool storageImages = false; // RGBA16F and R32F storage images, optimal tiling
	bool rayQuery = false;      // VK_KHR_ray_query + acceleration structures, all features
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
	AccelerationStructure,
};

// Thread-safe: with the queued material system, engine producers create and
// retire resources on the main thread while the render thread records
// dispatches and collects retirements. Handles and mapped pointers stay valid
// until their resource is collected.
class ComputeResources
{
public:
	// `physical`/`device` outlive this object; Shutdown releases everything
	// (after the owner waited for the device).
	bool Init( VkPhysicalDevice physical, VkDevice device, const ComputeCaps &enabled,
	    std::string *error );
	void Shutdown();
	bool Ready() const { return m_device != VK_NULL_HANDLE && m_enabled.compute; }
	// Borrowed, and outliving this object (either may be null): the code and
	// name of each program's module, and object names and dispatch labels.
	void SetDebugTools( const VulkanShaderLibrary *shaders, const VulkanDebugUtils *debug )
	{
		m_shaders = shaders;
		m_debug = debug;
	}
	const ComputeCaps &Enabled() const { return m_enabled; }

	// Resources, by handle (0 is never valid).
	// Host-visible, coherent and persistently mapped; `readback` prefers
	// host-cached memory (CPU reads), otherwise the first host-visible type
	// (device-local first on devices that expose it).
	uint32_t CreateBuffer( size_t bytes, std::string *error, bool readback = false );
	uint32_t CreateStorageImage(
	    uint32_t width, uint32_t height, VkFormat format, std::string *error );
	// A program over the given bindings (set 0, in order) and push bytes.
	uint32_t CreateProgram( const uint32_t *spirv, size_t bytes,
	    const std::vector<ComputeBinding> &bindings, uint32_t pushBytes, std::string *error );

	// Host-visible buffer memory (buffers are host-coherent).
	void *Map( uint32_t buffer );
	VkImage Image( uint32_t image ) const;
	VkBuffer BufferHandle( uint32_t buffer ) const;

	// Ray-query acceleration structures (only with Enabled().rayQuery):
	// triangle geometry (a bottom-level structure, opaque triangles) and
	// scenes of its instances (a top level). Each is built by RecordBuild,
	// which must precede, in submission order, every build or dispatch that
	// reads it. A scene's geometry must stay alive while the scene does.
	struct Instance
	{
		uint32_t geometry = 0;
		float transform[12] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 }; // row-major 3 x 4
		uint32_t customIndex = 0;                                     // 24 bits
		uint32_t mask = 0xFF;                                         // 8 bits: cull mask match
	};
	uint32_t CreateGeometry( const float *positions, uint32_t vertexCount, const uint32_t *indices,
	    uint32_t indexCount, std::string *error );
	uint32_t CreateScene( const Instance *instances, uint32_t count, std::string *error );
	// Records the structure's build and a barrier making it visible to later
	// builds and compute reads.
	bool RecordBuild( VkCommandBuffer cmd, uint32_t structure );

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
	size_t PendingRetirements() const
	{
		std::lock_guard<std::recursive_mutex> lock( m_mutex );
		return m_retired.size();
	}
	size_t LiveCount() const
	{
		std::lock_guard<std::recursive_mutex> lock( m_mutex );
		return m_resources.size();
	}
	// The seeded early-free defect (the retirement check's control): destroy
	// at Retire, ignoring the serial.
	void SetEarlyFreeDefect( bool on ) { m_earlyFree = on; }

private:
	struct Resource
	{
		uint32_t handle = 0;
		ComputeBinding kind = ComputeBinding::StorageBuffer;
		bool program = false;
		const char *name = nullptr; // a program's shader (material_spv_index.h)
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
		// An acceleration structure: its storage (buffer/memory above), build
		// inputs and scratch (extra), and what RecordBuild needs.
		VkAccelerationStructureKHR structure = VK_NULL_HANDLE;
		std::vector<std::pair<VkBuffer, VkDeviceMemory>> extra;
		VkAccelerationStructureTypeKHR structureType =
		    VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		VkAccelerationStructureGeometryKHR geometry = {};
		uint32_t primitives = 0;
		VkDeviceAddress scratch = 0;
	};
	struct Retired
	{
		uint32_t handle;
		uint64_t afterSerial;
	};

	Resource *Find( uint32_t handle );
	const Resource *Find( uint32_t handle ) const;
	bool Memory( const VkMemoryRequirements &requirements, VkMemoryPropertyFlags flags,
	    VkDeviceMemory *memory, std::string *error, bool deviceAddress = false );
	// A buffer with its own memory; mapped when host-visible; allocated for
	// device addresses when `usage` asks for them.
	bool MakeBuffer( VkDeviceSize bytes, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags,
	    VkBuffer *buffer, VkDeviceMemory *memory, void **mapped, std::string *error );
	VkDeviceAddress Address( VkBuffer buffer ) const;
	uint32_t FinishStructure(
	    Resource &resource, VkBuildAccelerationStructureFlagsKHR flags, std::string *error );
	void Destroy( Resource &resource );

	// Guards everything below; Find's pointers are valid only while it is held.
	mutable std::recursive_mutex m_mutex;
	VkPhysicalDevice m_physical = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	ComputeCaps m_enabled;
	const VulkanShaderLibrary *m_shaders = nullptr;
	const VulkanDebugUtils *m_debug = nullptr;
	std::vector<Resource> m_resources;
	std::vector<Retired> m_retired;
	uint32_t m_next = 0;
	bool m_earlyFree = false;
	// VK_KHR_acceleration_structure entry points (loaded with ray query).
	PFN_vkCreateAccelerationStructureKHR m_createStructure = nullptr;
	PFN_vkDestroyAccelerationStructureKHR m_destroyStructure = nullptr;
	PFN_vkGetAccelerationStructureBuildSizesKHR m_buildSizes = nullptr;
	PFN_vkCmdBuildAccelerationStructuresKHR m_cmdBuild = nullptr;
	PFN_vkGetAccelerationStructureDeviceAddressKHR m_structureAddress = nullptr;
	PFN_vkGetBufferDeviceAddress m_bufferAddress = nullptr;
	VkDeviceSize m_scratchAlignment = 1;
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
	uint32_t CreateBuffer( size_t bytes, gpu_compute::BufferUse use ) override;
	void *Map( uint32_t buffer ) override;
	uint32_t CreateProgram( const char *name, const gpu_compute::Binding *bindings, uint32_t count,
	    uint32_t pushBytes ) override;
	uint32_t CreateGeometry( const float *positions, uint32_t vertexCount, const uint32_t *indices,
	    uint32_t indexCount ) override;
	uint32_t CreateScene( const gpu_compute::SceneInstance *instances, uint32_t count ) override;
	uint64_t QueueDispatch( uint32_t program, const uint32_t *buffers, uint32_t count,
	    const void *push, uint32_t pushBytes, uint32_t groupsX, uint32_t groupsY,
	    uint32_t groupsZ ) override;
	uint64_t CompletedSerial() const override { return m_completedSerial(); }
	void Retire( uint32_t resource, uint64_t afterSerial ) override
	{
		m_resources.Retire( resource, afterSerial );
	}
	// Records every queued dispatch, in order, into the owner's submission.
	// Producers may queue on another thread than the one recording: a
	// dispatch's serial is never one already recorded.
	void Record( VkCommandBuffer cmd, uint64_t serial );
	bool Pending() const;

private:
	struct Queued
	{
		uint32_t build = 0; // a structure to build instead of a dispatch
		uint32_t program = 0;
		std::vector<uint32_t> buffers;
		std::vector<unsigned char> push;
		uint32_t groups[3] = {};
	};
	ComputeResources &m_resources;
	std::function<uint64_t()> m_nextSerial;
	std::function<uint64_t()> m_completedSerial;
	mutable std::mutex m_queueMutex;
	std::vector<Queued> m_queue; // guarded by m_queueMutex
	uint64_t m_lastRecorded = 0; // guarded by m_queueMutex
};

} // namespace render_vulkan

#endif // VULKAN_COMPUTE_H
