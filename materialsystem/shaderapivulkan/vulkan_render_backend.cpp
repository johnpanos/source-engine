//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan implementation of render::IRenderBackendProvider.
//          See vulkan_render_backend.h.
//
//===========================================================================//

#include "vulkan_render_backend_native.h"

#include <vulkan/vulkan.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace render;

namespace render_vulkan
{
namespace
{

// Map a physical device's real Vulkan facts onto the contract's semantic
// feature set. Portable code asks these questions; it never asks "is this
// Vulkan". kNeverSupported is never advertised, so the conformance suite can
// always form an unsatisfiable required-feature request.
RenderFeatureSet FeaturesFor( VkPhysicalDevice phys )
{
	VkPhysicalDeviceProperties props = {};
	vkGetPhysicalDeviceProperties( phys, &props );

	RenderFeatureSet set;
	set.Add( RenderFeature::kSampledSrgb ); // core in Vulkan
	set.Add( RenderFeature::kDepthColorPairing );
	set.Add(
	    RenderFeature::kComputeShaders ); // every Vulkan queue family set has compute-capable HW
	set.Add( RenderFeature::kOffscreenRender );
	if ( props.limits.framebufferColorSampleCounts & VK_SAMPLE_COUNT_4_BIT )
		set.Add( RenderFeature::kMultiSample4x );
	// kRuntimeShaderCompile is intentionally NOT advertised: this provider ships
	// precompiled SPIR-V, not a runtime GLSL compiler.
	return set;
}

uint32_t MaxSampleCountFor( const VkPhysicalDeviceLimits &limits )
{
	const VkSampleCountFlags counts =
	    limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;
	if ( counts & VK_SAMPLE_COUNT_8_BIT )
		return 8;
	if ( counts & VK_SAMPLE_COUNT_4_BIT )
		return 4;
	if ( counts & VK_SAMPLE_COUNT_2_BIT )
		return 2;
	return 1;
}

int FindGraphicsQueueFamily( VkPhysicalDevice phys )
{
	uint32_t n = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( phys, &n, nullptr );
	std::vector<VkQueueFamilyProperties> qfs( n );
	vkGetPhysicalDeviceQueueFamilyProperties( phys, &n, qfs.data() );
	for ( uint32_t i = 0; i < n; ++i )
		if ( qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
			return static_cast<int>( i );
	return -1;
}

uint32_t FindMemoryType(
    VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags props, bool *found )
{
	VkPhysicalDeviceMemoryProperties mem = {};
	vkGetPhysicalDeviceMemoryProperties( phys, &mem );
	for ( uint32_t i = 0; i < mem.memoryTypeCount; ++i )
		if ( ( typeBits & ( 1u << i ) ) && ( mem.memoryTypes[i].propertyFlags & props ) == props )
		{
			*found = true;
			return i;
		}
	*found = false;
	return 0;
}

// ---------------------------------------------------------------------------
// Completion token: real submission backed by a VkFence, but completion is
// OBSERVED one submission at a time in submission order, exactly as the contract
// defines PollCompletion (RFC 0006: neither a CPU event nor a frame counter is
// proof of GPU completion -- here PollCompletion waits the oldest fence).
// ---------------------------------------------------------------------------
class VkCompletionToken : public IRenderCompletionToken
{
public:
	explicit VkCompletionToken( uint64_t id ) : m_Id( id ) {}
	bool IsComplete() const override { return m_Complete; }
	uint64_t Id() const { return m_Id; }
	void MarkComplete() { m_Complete = true; }

private:
	uint64_t m_Id;
	bool m_Complete = false;
};

class VkCommandContextImpl : public IRenderCommandContext
{
public:
	explicit VkCommandContextImpl( VkCommandBuffer cmd ) : m_Cmd( cmd ) {}
	void RecordUse( RenderResourceHandle h ) override { m_Used.push_back( h ); }
	VkCommandBuffer Cmd() const { return m_Cmd; }

private:
	VkCommandBuffer m_Cmd;
	std::vector<RenderResourceHandle> m_Used;
};

// ---------------------------------------------------------------------------
// Device
// ---------------------------------------------------------------------------
struct GpuResource
{
	RenderResourceType type = RenderResourceType::kBuffer;
	bool live = false;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

// Plain aggregates (no default member initializers) so brace-initialization
// with arguments works under every C++ standard this target may build with.
struct DeferredDestroy
{
	RenderResourceHandle handle;
	IRenderCompletionToken *token;
};

struct Outstanding
{
	VkCompletionToken *token;
	VkFence fence;
	VkCommandBuffer cmd;
};

// Native facts a bridge needs about a device, fixed at creation.
struct VulkanDeviceSetup
{
	VkInstance instance;
	VkPhysicalDevice phys;
	VkDevice device;
	VkQueue queue;
	uint32_t queueFamily;
	VkCommandPool pool;
	bool swapchainEnabled;
	VkSemaphore gate; // timeline semaphore for the completion-gate test hook, or null
};

class VulkanDevice : public IRenderDevice, public VulkanDeviceEndpoint
{
public:
	VulkanDevice( const VulkanDeviceSetup &setup, const RenderDeviceCaps &caps )
	    : m_Instance( setup.instance ), m_Phys( setup.phys ), m_Device( setup.device ),
	      m_Queue( setup.queue ), m_QueueFamily( setup.queueFamily ), m_Pool( setup.pool ),
	      m_SwapchainEnabled( setup.swapchainEnabled ), m_Gate( setup.gate ), m_Caps( caps )
	{
	}

	~VulkanDevice() override
	{
		ReleaseCompletion(); // never wait idle on work a test still holds
		vkDeviceWaitIdle( m_Device );
		for ( Outstanding &o : m_Outstanding )
		{
			if ( o.fence != VK_NULL_HANDLE )
				vkDestroyFence( m_Device, o.fence, nullptr );
			delete o.token;
		}
		for ( VkCompletionToken *t : m_RetiredTokens )
			delete t;
		for ( VkCommandContextImpl *c : m_Contexts )
			delete c;
		for ( GpuResource &r : m_Resources )
			DestroyResourceObjects( r );
		if ( m_Gate != VK_NULL_HANDLE )
			vkDestroySemaphore( m_Device, m_Gate, nullptr );
		vkDestroyCommandPool( m_Device, m_Pool, nullptr );
		vkDestroyDevice( m_Device, nullptr );
	}

	const RenderDeviceCaps &GetCapabilities() const override { return m_Caps; }
	RenderDeviceState GetState() const override { return m_State; }

	// -- Resources: real device-memory-backed Vulkan objects --
	RenderResourceHandle CreateResource( RenderResourceType type ) override
	{
		GpuResource r;
		r.type = type;
		if ( type == RenderResourceType::kTexture )
		{
			VkImageCreateInfo ii = {};
			ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			ii.imageType = VK_IMAGE_TYPE_2D;
			ii.format = VK_FORMAT_R8G8B8A8_UNORM;
			ii.extent = { 1, 1, 1 };
			ii.mipLevels = 1;
			ii.arrayLayers = 1;
			ii.samples = VK_SAMPLE_COUNT_1_BIT;
			ii.tiling = VK_IMAGE_TILING_OPTIMAL;
			ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if ( vkCreateImage( m_Device, &ii, nullptr, &r.image ) != VK_SUCCESS )
				return kInvalidResource;
			VkMemoryRequirements req = {};
			vkGetImageMemoryRequirements( m_Device, r.image, &req );
			if ( !AllocBind( req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &r.memory ) )
			{
				vkDestroyImage( m_Device, r.image, nullptr );
				return kInvalidResource;
			}
			vkBindImageMemory( m_Device, r.image, r.memory, 0 );
		}
		else
		{
			VkBufferCreateInfo bi = {};
			bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bi.size = 256;
			bi.usage = ( type == RenderResourceType::kShaderArtifact )
			               ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
			               : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			if ( vkCreateBuffer( m_Device, &bi, nullptr, &r.buffer ) != VK_SUCCESS )
				return kInvalidResource;
			VkMemoryRequirements req = {};
			vkGetBufferMemoryRequirements( m_Device, r.buffer, &req );
			if ( !AllocBind( req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &r.memory ) )
			{
				vkDestroyBuffer( m_Device, r.buffer, nullptr );
				return kInvalidResource;
			}
			vkBindBufferMemory( m_Device, r.buffer, r.memory, 0 );
		}
		r.live = true;
		m_Resources.push_back( r );
		return static_cast<RenderResourceHandle>( m_Resources.size() ); // 1-based
	}

	bool IsResourceLive( RenderResourceHandle h ) const override
	{
		const GpuResource *r = Slot( h );
		return r && r->live;
	}

	size_t GetLiveResourceCount() const override
	{
		size_t n = 0;
		for ( const GpuResource &r : m_Resources )
			if ( r.live )
				++n;
		return n;
	}

	void DestroyResourceWhenComplete(
	    RenderResourceHandle h, IRenderCompletionToken &token ) override
	{
		m_Deferred.push_back( DeferredDestroy{ h, &token } );
	}

	void CollectCompletedDestructions() override
	{
		std::vector<DeferredDestroy> remaining;
		for ( const DeferredDestroy &d : m_Deferred )
		{
			if ( d.token->IsComplete() )
			{
				GpuResource *r = Slot( d.handle );
				if ( r && r->live )
				{
					DestroyResourceObjects( *r );
					r->live = false;
				}
			}
			else
			{
				remaining.push_back( d );
			}
		}
		m_Deferred.swap( remaining );
	}

	// -- Submission: real command buffers + queue submit + VkFence --
	IRenderCommandContext *CreateCommandContext() override
	{
		VkCommandBufferAllocateInfo ai = {};
		ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		ai.commandPool = m_Pool;
		ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		ai.commandBufferCount = 1;
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		if ( vkAllocateCommandBuffers( m_Device, &ai, &cmd ) != VK_SUCCESS )
			return nullptr;
		VkCommandBufferBeginInfo bi = {};
		bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer( cmd, &bi );
		VkCommandContextImpl *ctx = new VkCommandContextImpl( cmd );
		m_Contexts.push_back( ctx );
		return ctx;
	}

	IRenderCompletionToken *Submit( IRenderCommandContext &context ) override
	{
		return SubmitWithSemaphores( context, VK_NULL_HANDLE, 0, VK_NULL_HANDLE );
	}

	// -- VulkanDeviceEndpoint (private, for presentation bridges) --
	VkInstance Instance() const override { return m_Instance; }
	VkPhysicalDevice PhysicalDevice() const override { return m_Phys; }
	VkDevice Device() const override { return m_Device; }
	VkQueue Queue() const override { return m_Queue; }
	uint32_t QueueFamily() const override { return m_QueueFamily; }
	bool SwapchainEnabled() const override { return m_SwapchainEnabled; }

	RenderResourceHandle CreateImage(
	    uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage ) override
	{
		GpuResource r;
		r.type = RenderResourceType::kTexture;
		VkImageCreateInfo ii = {};
		ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ii.imageType = VK_IMAGE_TYPE_2D;
		ii.format = format;
		ii.extent = { width, height, 1 };
		ii.mipLevels = 1;
		ii.arrayLayers = 1;
		ii.samples = VK_SAMPLE_COUNT_1_BIT;
		ii.tiling = VK_IMAGE_TILING_OPTIMAL;
		ii.usage = usage;
		if ( vkCreateImage( m_Device, &ii, nullptr, &r.image ) != VK_SUCCESS )
			return kInvalidResource;
		VkMemoryRequirements req = {};
		vkGetImageMemoryRequirements( m_Device, r.image, &req );
		if ( !AllocBind( req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &r.memory ) )
		{
			vkDestroyImage( m_Device, r.image, nullptr );
			return kInvalidResource;
		}
		vkBindImageMemory( m_Device, r.image, r.memory, 0 );
		r.live = true;
		m_Resources.push_back( r );
		return static_cast<RenderResourceHandle>( m_Resources.size() );
	}

	VkImage Image( RenderResourceHandle handle ) const override
	{
		const GpuResource *r = Slot( handle );
		return ( r && r->live ) ? r->image : VK_NULL_HANDLE;
	}

	VkCommandBuffer CommandBuffer( IRenderCommandContext &context ) const override
	{
		return static_cast<VkCommandContextImpl &>( context ).Cmd();
	}

	IRenderCompletionToken *SubmitWithSemaphores( IRenderCommandContext &context, VkSemaphore wait,
	    VkPipelineStageFlags waitStage, VkSemaphore signal ) override
	{
		VkCommandContextImpl &ctx = static_cast<VkCommandContextImpl &>( context );
		VkCommandBuffer cmd = ctx.Cmd();
		vkEndCommandBuffer( cmd );

		VkFenceCreateInfo fi = {};
		fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		VkFence fence = VK_NULL_HANDLE;
		vkCreateFence( m_Device, &fi, nullptr, &fence );

		VkSemaphore waits[2];
		VkPipelineStageFlags stages[2];
		uint64_t waitValues[2] = { 0, 0 };
		uint32_t waitCount = 0;
		if ( wait != VK_NULL_HANDLE )
		{
			waits[waitCount] = wait;
			stages[waitCount] = waitStage;
			++waitCount;
		}
		if ( m_Held )
		{
			// Held: this work cannot start until the host signals the release value.
			waits[waitCount] = m_Gate;
			stages[waitCount] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			waitValues[waitCount] = m_GateValue + 1;
			++waitCount;
		}
		uint64_t signalValue = 0;
		VkTimelineSemaphoreSubmitInfo timeline = {};
		timeline.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timeline.waitSemaphoreValueCount = waitCount;
		timeline.pWaitSemaphoreValues = waitValues;
		timeline.signalSemaphoreValueCount = signal != VK_NULL_HANDLE ? 1 : 0;
		timeline.pSignalSemaphoreValues = &signalValue;

		VkSubmitInfo si = {};
		si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		si.pNext = m_Held ? &timeline : nullptr;
		si.waitSemaphoreCount = waitCount;
		si.pWaitSemaphores = waits;
		si.pWaitDstStageMask = stages;
		si.commandBufferCount = 1;
		si.pCommandBuffers = &cmd;
		si.signalSemaphoreCount = signal != VK_NULL_HANDLE ? 1 : 0;
		si.pSignalSemaphores = &signal;
		vkQueueSubmit( m_Queue, 1, &si, fence );

		const uint64_t id = ++m_NextSubmissionId;
		VkCompletionToken *token = new VkCompletionToken( id );
		m_Outstanding.push_back( Outstanding{ token, fence, cmd } );
		return token;
	}

	uint32_t PollCompletion() override
	{
		if ( m_Outstanding.empty() )
			return 0;
		// Observe completion of the OLDEST outstanding submission (submission
		// order). Wait its real fence so completion reflects the GPU, not a CPU
		// event or frame counter.
		Outstanding o = m_Outstanding.front();
		m_Outstanding.erase( m_Outstanding.begin() );
		vkWaitForFences( m_Device, 1, &o.fence, VK_TRUE, UINT64_MAX );
		o.token->MarkComplete();
		if ( o.token->Id() > m_LastCompleted )
			m_LastCompleted = o.token->Id();
		vkDestroyFence( m_Device, o.fence, nullptr );
		vkFreeCommandBuffers( m_Device, m_Pool, 1, &o.cmd );
		m_RetiredTokens.push_back( o.token ); // token stays valid until device destruction
		return 1;
	}

	uint64_t LastCompletedSubmission() const override { return m_LastCompleted; }

	bool HoldCompletion() override
	{
		if ( m_Gate == VK_NULL_HANDLE )
			return false;
		m_Held = true;
		return true;
	}

	void ReleaseCompletion() override
	{
		if ( !m_Held )
			return;
		auto signalSemaphore = reinterpret_cast<PFN_vkSignalSemaphore>(
		    vkGetDeviceProcAddr( m_Device, "vkSignalSemaphore" ) );
		VkSemaphoreSignalInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		info.semaphore = m_Gate;
		info.value = ++m_GateValue;
		if ( signalSemaphore )
			signalSemaphore( m_Device, &info );
		m_Held = false;
	}

	bool HasIncompleteGpuWork() const override
	{
		for ( const Outstanding &o : m_Outstanding )
			if ( vkGetFenceStatus( m_Device, o.fence ) == VK_NOT_READY )
				return true;
		return false;
	}

	// -- Device loss: lifecycle state machine --
	bool SimulateDeviceLoss() override
	{
		vkDeviceWaitIdle( m_Device );
		m_State = RenderDeviceState::kDeviceLost;
		return true;
	}

	bool RecoverDevice() override
	{
		// This provider advertises recovery: return to available. (The logical
		// device is retained; a real driver-lost VkDevice recreation is a larger
		// concern handled by the presentation bridge, not this contract hook.)
		m_State = RenderDeviceState::kAvailable;
		return true;
	}

private:
	bool AllocBind(
	    const VkMemoryRequirements &req, VkMemoryPropertyFlags props, VkDeviceMemory *out )
	{
		bool found = false;
		uint32_t type = FindMemoryType( m_Phys, req.memoryTypeBits, props, &found );
		if ( !found )
			return false;
		VkMemoryAllocateInfo ai = {};
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = req.size;
		ai.memoryTypeIndex = type;
		return vkAllocateMemory( m_Device, &ai, nullptr, out ) == VK_SUCCESS;
	}

	void DestroyResourceObjects( GpuResource &r )
	{
		if ( r.buffer != VK_NULL_HANDLE )
		{
			vkDestroyBuffer( m_Device, r.buffer, nullptr );
			r.buffer = VK_NULL_HANDLE;
		}
		if ( r.image != VK_NULL_HANDLE )
		{
			vkDestroyImage( m_Device, r.image, nullptr );
			r.image = VK_NULL_HANDLE;
		}
		if ( r.memory != VK_NULL_HANDLE )
		{
			vkFreeMemory( m_Device, r.memory, nullptr );
			r.memory = VK_NULL_HANDLE;
		}
	}

	const GpuResource *Slot( RenderResourceHandle h ) const
	{
		if ( h == kInvalidResource || h > m_Resources.size() )
			return nullptr;
		return &m_Resources[static_cast<size_t>( h ) - 1];
	}
	GpuResource *Slot( RenderResourceHandle h )
	{
		return const_cast<GpuResource *>( static_cast<const VulkanDevice *>( this )->Slot( h ) );
	}

	VkInstance m_Instance;
	VkPhysicalDevice m_Phys;
	VkDevice m_Device;
	VkQueue m_Queue;
	uint32_t m_QueueFamily;
	VkCommandPool m_Pool;
	bool m_SwapchainEnabled;
	VkSemaphore m_Gate;
	uint64_t m_GateValue = 0;
	bool m_Held = false;
	RenderDeviceCaps m_Caps;
	RenderDeviceState m_State = RenderDeviceState::kAvailable;

	std::vector<GpuResource> m_Resources;
	std::vector<DeferredDestroy> m_Deferred;
	std::vector<VkCommandContextImpl *> m_Contexts;
	std::vector<Outstanding> m_Outstanding;
	std::vector<VkCompletionToken *> m_RetiredTokens;
	uint64_t m_NextSubmissionId = 0;
	uint64_t m_LastCompleted = 0;
};

// ---------------------------------------------------------------------------
// Provider
// ---------------------------------------------------------------------------
bool DeviceHasExtension( VkPhysicalDevice phys, const char *name )
{
	uint32_t n = 0;
	vkEnumerateDeviceExtensionProperties( phys, nullptr, &n, nullptr );
	std::vector<VkExtensionProperties> exts( n );
	if ( n )
		vkEnumerateDeviceExtensionProperties( phys, nullptr, &n, exts.data() );
	for ( const VkExtensionProperties &e : exts )
		if ( std::strcmp( e.extensionName, name ) == 0 )
			return true;
	return false;
}

bool DeviceSupportsTimeline( VkPhysicalDevice phys )
{
	VkPhysicalDeviceProperties props = {};
	vkGetPhysicalDeviceProperties( phys, &props );
	if ( props.apiVersion < VK_API_VERSION_1_2 )
		return false;
	VkPhysicalDeviceTimelineSemaphoreFeatures timeline = {};
	timeline.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
	VkPhysicalDeviceFeatures2 features = {};
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features.pNext = &timeline;
	vkGetPhysicalDeviceFeatures2( phys, &features );
	return timeline.timelineSemaphore == VK_TRUE;
}

class VulkanProvider : public VulkanRenderBackend
{
public:
	VulkanProvider( VkInstance instance, const VulkanProviderOptions &options, bool api12 )
	    : m_Instance( instance ), m_Options( options ), m_Api12( api12 )
	{
		uint32_t n = 0;
		vkEnumeratePhysicalDevices( m_Instance, &n, nullptr );
		m_Physical.resize( n );
		if ( n )
			vkEnumeratePhysicalDevices( m_Instance, &n, m_Physical.data() );

		for ( VkPhysicalDevice phys : m_Physical )
		{
			VkPhysicalDeviceProperties props = {};
			vkGetPhysicalDeviceProperties( phys, &props );
			RenderAdapterInfo info;
			std::snprintf(
			    info.id, sizeof( info.id ), "vk-%08x-%08x", props.vendorID, props.deviceID );
			std::snprintf( info.name, sizeof( info.name ), "%.63s", props.deviceName );
			info.vendorId = props.vendorID;
			info.isSoftware = ( props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU );
			VkPhysicalDeviceMemoryProperties mem = {};
			vkGetPhysicalDeviceMemoryProperties( phys, &mem );
			uint64_t deviceLocal = 0;
			for ( uint32_t i = 0; i < mem.memoryHeapCount; ++i )
				if ( mem.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT )
					deviceLocal += mem.memoryHeaps[i].size;
			info.deviceMemoryBytes = deviceLocal;
			info.supportedFeatures = FeaturesFor( phys );
			m_Adapters.push_back( info );
		}
	}

	~VulkanProvider() override
	{
		for ( VulkanDevice *d : m_Devices )
			delete d;
		if ( m_Instance != VK_NULL_HANDLE )
			vkDestroyInstance( m_Instance, nullptr );
	}

	RenderBackendId GetBackendId() const override
	{
		RenderBackendId id;
		id.id = "vulkan";
		id.name = "Native Vulkan Backend";
		id.version = 1;
		return id;
	}

	RenderProviderCaps GetProviderCaps() const override
	{
		RenderProviderCaps caps;
		caps.supportsOffscreenDevice = true;
		caps.supportsDeviceLossRecovery = true;
		caps.supportsRuntimeShaderCompile = false;
		return caps;
	}

	int GetAdapterCount() const override { return static_cast<int>( m_Adapters.size() ); }

	bool GetAdapterInfo( int index, RenderAdapterInfo *out ) const override
	{
		if ( index < 0 || index >= static_cast<int>( m_Adapters.size() ) || out == nullptr )
			return false;
		*out = m_Adapters[static_cast<size_t>( index )];
		return true;
	}

	IRenderDevice *CreateDevice(
	    const RenderDeviceRequest &request, RenderCreateError *error ) override
	{
		if ( request.adapterIndex < 0 ||
		     request.adapterIndex >= static_cast<int>( m_Adapters.size() ) )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf( error->message, sizeof( error->message ), "no adapter at index %d",
				    request.adapterIndex );
			}
			return nullptr;
		}

		const RenderAdapterInfo &adapter = m_Adapters[static_cast<size_t>( request.adapterIndex )];
		if ( !adapter.supportedFeatures.Contains( request.requiredFeatures ) )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kUnsupportedRequiredFeature;
				error->missingFeature = FirstMissing( adapter, request.requiredFeatures );
				std::snprintf( error->message, sizeof( error->message ),
				    "adapter does not support a required feature" );
			}
			return nullptr;
		}

		VkPhysicalDevice phys = m_Physical[static_cast<size_t>( request.adapterIndex )];
		int queueFamily = FindGraphicsQueueFamily( phys );
		if ( queueFamily < 0 )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf(
				    error->message, sizeof( error->message ), "no graphics queue family" );
			}
			return nullptr;
		}

		float priority = 1.0f;
		VkDeviceQueueCreateInfo qci = {};
		qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qci.queueFamilyIndex = static_cast<uint32_t>( queueFamily );
		qci.queueCount = 1;
		qci.pQueuePriorities = &priority;
		VkDeviceCreateInfo dci = {};
		dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		dci.queueCreateInfoCount = 1;
		dci.pQueueCreateInfos = &qci;

		// Presentation is not a device feature: a bridge presents from this device,
		// so the provider only enables the swapchain extension it will need.
		const char *swapchainExt = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		const bool swapchain =
		    m_Options.enableSwapchain && DeviceHasExtension( phys, swapchainExt );
		if ( swapchain )
		{
			dci.enabledExtensionCount = 1;
			dci.ppEnabledExtensionNames = &swapchainExt;
		}
		VkPhysicalDeviceTimelineSemaphoreFeatures timeline = {};
		timeline.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
		timeline.timelineSemaphore = VK_TRUE;
		const bool gate =
		    m_Options.enableCompletionGate && m_Api12 && DeviceSupportsTimeline( phys );
		if ( gate )
			dci.pNext = &timeline;

		VkDevice device = VK_NULL_HANDLE;
		if ( vkCreateDevice( phys, &dci, nullptr, &device ) != VK_SUCCESS )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf( error->message, sizeof( error->message ), "vkCreateDevice failed" );
			}
			return nullptr;
		}

		VkQueue queue = VK_NULL_HANDLE;
		vkGetDeviceQueue( device, static_cast<uint32_t>( queueFamily ), 0, &queue );

		VkCommandPoolCreateInfo pci = {};
		pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pci.queueFamilyIndex = static_cast<uint32_t>( queueFamily );
		VkCommandPool pool = VK_NULL_HANDLE;
		if ( vkCreateCommandPool( device, &pci, nullptr, &pool ) != VK_SUCCESS )
		{
			vkDestroyDevice( device, nullptr );
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf(
				    error->message, sizeof( error->message ), "vkCreateCommandPool failed" );
			}
			return nullptr;
		}

		VkPhysicalDeviceProperties props = {};
		vkGetPhysicalDeviceProperties( phys, &props );
		RenderDeviceCaps caps;
		caps.features = adapter.supportedFeatures;
		caps.maxTextureDimension = props.limits.maxImageDimension2D;
		caps.maxColorTargets = props.limits.maxColorAttachments;
		caps.maxSampleCount = MaxSampleCountFor( props.limits );

		if ( error )
			error->status = RenderCreateStatus::kOk;

		VkSemaphore gateSemaphore = VK_NULL_HANDLE;
		if ( gate )
		{
			VkSemaphoreTypeCreateInfo type = {};
			type.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			type.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
			type.initialValue = 0;
			VkSemaphoreCreateInfo sci = {};
			sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			sci.pNext = &type;
			if ( vkCreateSemaphore( device, &sci, nullptr, &gateSemaphore ) != VK_SUCCESS )
				gateSemaphore = VK_NULL_HANDLE;
		}

		VulkanDeviceSetup setup;
		setup.instance = m_Instance;
		setup.phys = phys;
		setup.device = device;
		setup.queue = queue;
		setup.queueFamily = static_cast<uint32_t>( queueFamily );
		setup.pool = pool;
		setup.swapchainEnabled = swapchain;
		setup.gate = gateSemaphore;
		VulkanDevice *dev = new VulkanDevice( setup, caps );
		m_Devices.push_back( dev );
		return dev;
	}

	void DestroyDevice( IRenderDevice *device ) override
	{
		for ( size_t i = 0; i < m_Devices.size(); ++i )
			if ( m_Devices[i] == device )
			{
				delete m_Devices[i];
				m_Devices.erase( m_Devices.begin() + static_cast<std::ptrdiff_t>( i ) );
				return;
			}
	}

	size_t GetLiveDeviceCount() const override { return m_Devices.size(); }

	bool OwnsDevice( const IRenderDevice &device ) const override
	{
		for ( const VulkanDevice *d : m_Devices )
			if ( d == &device )
				return true;
		return false;
	}

	VkInstance Instance() const override { return m_Instance; }

	VulkanDeviceEndpoint *FindDevice( IRenderDevice &device ) override
	{
		for ( VulkanDevice *d : m_Devices )
			if ( d == &device )
				return d;
		return nullptr;
	}

private:
	RenderFeature FirstMissing(
	    const RenderAdapterInfo &adapter, const RenderFeatureSet &required ) const
	{
		for ( uint32_t bit = 0; bit < 32; ++bit )
		{
			const RenderFeature f = static_cast<RenderFeature>( bit );
			if ( required.Has( f ) && !adapter.supportedFeatures.Has( f ) )
				return f;
		}
		return RenderFeature::kNeverSupported;
	}

	VkInstance m_Instance;
	VulkanProviderOptions m_Options;
	bool m_Api12;
	std::vector<VkPhysicalDevice> m_Physical;
	std::vector<RenderAdapterInfo> m_Adapters;
	std::vector<VulkanDevice *> m_Devices;
};

} // namespace

std::unique_ptr<VulkanRenderBackend> MakeVulkanRenderBackend(
    const VulkanProviderOptions &options, std::string *outError )
{
	// Vulkan 1.2 where the loader has it (timeline semaphores for the completion
	// gate); 1.1 is enough otherwise.
	uint32_t loaderVersion = VK_API_VERSION_1_1;
	auto enumerateVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
	    vkGetInstanceProcAddr( VK_NULL_HANDLE, "vkEnumerateInstanceVersion" ) );
	if ( enumerateVersion )
		enumerateVersion( &loaderVersion );
	const bool api12 = loaderVersion >= VK_API_VERSION_1_2;

	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "Source Native Vulkan render-backend provider";
	app.apiVersion = api12 ? VK_API_VERSION_1_2 : VK_API_VERSION_1_1;

	std::vector<const char *> extensions;
	for ( const std::string &e : options.instanceExtensions )
		extensions.push_back( e.c_str() );

	VkInstanceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pApplicationInfo = &app;
	ci.enabledExtensionCount = static_cast<uint32_t>( extensions.size() );
	ci.ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

	VkInstance instance = VK_NULL_HANDLE;
	VkResult r = vkCreateInstance( &ci, nullptr, &instance );
	if ( r != VK_SUCCESS )
	{
		if ( outError )
			*outError = r == VK_ERROR_EXTENSION_NOT_PRESENT
			                ? "a required Vulkan instance extension is unavailable"
			                : "vkCreateInstance failed (no usable Vulkan loader/driver)";
		return nullptr;
	}

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );
	if ( deviceCount == 0 )
	{
		if ( outError )
			*outError = "no Vulkan physical devices present";
		vkDestroyInstance( instance, nullptr );
		return nullptr;
	}

	return std::unique_ptr<VulkanRenderBackend>( new VulkanProvider( instance, options, api12 ) );
}

std::unique_ptr<IRenderBackendProvider> MakeVulkanRenderBackend( std::string *outError )
{
	return MakeVulkanRenderBackend( VulkanProviderOptions(), outError );
}

} // namespace render_vulkan
