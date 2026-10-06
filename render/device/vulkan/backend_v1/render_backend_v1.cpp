//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.backend.v1 over render.device.vulkan. See
//			render_backend_v1.h.
//
//=============================================================================//

#include "render_backend_v1.h"
#include "../host_device.h"

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
int FindGraphicsQueueFamily( VkPhysicalDevice phys );

RenderFeatureSet FeaturesFor( VkPhysicalDevice phys )
{
	VkPhysicalDeviceProperties props = {};
	vkGetPhysicalDeviceProperties( phys, &props );

	RenderFeatureSet set;
	set.Add( RenderFeature::kSampledSrgb ); // core in Vulkan
	set.Add( RenderFeature::kDepthColorPairing );
	// Compute and storage images from the device's queries (RFC 0011 G5);
	// neither needs a device feature enabled. Ray query does, and this
	// provider does not enable it, so it never claims it.
	const int family = FindGraphicsQueueFamily( phys );
	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( phys, &familyCount, nullptr );
	std::vector<VkQueueFamilyProperties> families( familyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( phys, &familyCount, families.data() );
	const bool compute =
	    family >= 0 && ( families[size_t( family )].queueFlags & VK_QUEUE_COMPUTE_BIT );
	const auto storage = [phys]( VkFormat format )
	{
		VkFormatProperties properties = {};
		vkGetPhysicalDeviceFormatProperties( phys, format, &properties );
		return ( properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT ) != 0;
	};
	if ( compute )
		set.Add( RenderFeature::kComputeShaders );
	if ( compute && storage( VK_FORMAT_R16G16B16A16_SFLOAT ) && storage( VK_FORMAT_R32_SFLOAT ) )
		set.Add( RenderFeature::kStorageImages );
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

// ---------------------------------------------------------------------------
// Completion token: a real submission that signals the adapter's timeline,
// but completion is OBSERVED one submission at a time in submission order,
// exactly as the contract defines PollCompletion (RFC 0006: neither a CPU event
// nor a frame counter is proof of GPU completion -- here PollCompletion waits
// the oldest submission's timeline value).
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
	render::device::vulkan::HostAllocation memory = nullptr;
	bool borrowed = false; // RegisterBorrowedImage: the caller owns the image
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
	uint64_t value; // the adapter timeline value the submission signals
	VkCommandBuffer cmd;
};

// Native facts a bridge needs about a device, fixed at creation.
struct VulkanDeviceSetup
{
	std::unique_ptr<render::device::vulkan::IHostDevice> host;
	VkCommandPool pool;
	bool swapchainEnabled;
	VkSemaphore gate; // timeline semaphore for the completion-gate test hook, or null
};

class ProviderDevice : public IRenderDevice, public VulkanDeviceEndpoint
{
public:
	ProviderDevice( VulkanDeviceSetup &&setup, const RenderDeviceCaps &caps )
	    : m_Host( std::move( setup.host ) ), m_Instance( m_Host->Info().instance ),
	      m_Phys( m_Host->Info().physical ), m_Device( m_Host->Info().device ),
	      m_Queue( m_Host->Info().graphicsQueue ), m_QueueFamily( m_Host->Info().graphicsFamily ),
	      m_Pool( setup.pool ), m_SwapchainEnabled( setup.swapchainEnabled ), m_Gate( setup.gate ),
	      m_Caps( caps )
	{
	}

	~ProviderDevice() override
	{
		ReleaseCompletion(); // never wait on work a test still holds
		// Teardown: every submission completes before its objects go.
		m_Host->WaitValue( m_Host->SubmittedValue(), UINT64_MAX );
		for ( Outstanding &o : m_Outstanding )
			delete o.token;
		for ( VkCompletionToken *t : m_RetiredTokens )
			delete t;
		for ( VkCommandContextImpl *c : m_Contexts )
			delete c;
		for ( GpuResource &r : m_Resources )
			DestroyResourceObjects( r );
		if ( m_Gate != VK_NULL_HANDLE )
			vkDestroySemaphore( m_Device, m_Gate, nullptr );
		vkDestroyCommandPool( m_Device, m_Pool, nullptr );
		m_Host->Collect();
		// m_Host (the adapter) destroys the device.
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
			if ( m_Host->CreateImage( ii, DeviceLocal(), &r.image, &r.memory ) != VK_SUCCESS )
				return kInvalidResource;
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
			if ( m_Host->CreateBuffer( bi, DeviceLocal(), &r.buffer, &r.memory ) != VK_SUCCESS )
				return kInvalidResource;
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

	// -- Submission: real command buffers, completion on the adapter timeline --
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
	render::device::vulkan::IHostDevice &Host() override { return *m_Host; }
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
		if ( m_Host->CreateImage( ii, DeviceLocal(), &r.image, &r.memory ) != VK_SUCCESS )
			return kInvalidResource;
		r.live = true;
		m_Resources.push_back( r );
		return static_cast<RenderResourceHandle>( m_Resources.size() );
	}

	RenderResourceHandle RegisterBorrowedImage( VkImage image ) override
	{
		if ( image == VK_NULL_HANDLE )
			return kInvalidResource;
		GpuResource r;
		r.type = RenderResourceType::kTexture;
		r.image = image;
		r.borrowed = true;
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
		// The bridge's binary semaphore (if any), then the adapter's timeline
		// at this submission's value: its completion.
		VkSemaphore signals[2];
		uint64_t signalValues[2] = { 0, 0 };
		uint32_t signalCount = 0;
		if ( signal != VK_NULL_HANDLE )
			signals[signalCount++] = signal;
		const uint64_t value = m_Host->NextSubmitValue();
		signals[signalCount] = m_Host->Timeline();
		signalValues[signalCount++] = value;
		VkTimelineSemaphoreSubmitInfo timeline = {};
		timeline.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timeline.waitSemaphoreValueCount = waitCount;
		timeline.pWaitSemaphoreValues = waitValues;
		timeline.signalSemaphoreValueCount = signalCount;
		timeline.pSignalSemaphoreValues = signalValues;

		VkSubmitInfo si = {};
		si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		si.pNext = &timeline;
		si.waitSemaphoreCount = waitCount;
		si.pWaitSemaphores = waits;
		si.pWaitDstStageMask = stages;
		si.commandBufferCount = 1;
		si.pCommandBuffers = &cmd;
		si.signalSemaphoreCount = signalCount;
		si.pSignalSemaphores = signals;
		m_Host->LockQueue();
		const VkResult submitted = vkQueueSubmit( m_Queue, 1, &si, VK_NULL_HANDLE );
		m_Host->UnlockQueue();
		if ( submitted != VK_SUCCESS )
			m_Host->AbandonValue( value );

		const uint64_t id = ++m_NextSubmissionId;
		VkCompletionToken *token = new VkCompletionToken( id );
		m_Outstanding.push_back( Outstanding{ token, value, cmd } );
		return token;
	}

	uint32_t PollCompletion() override
	{
		if ( m_Outstanding.empty() )
			return 0;
		// Observe completion of the OLDEST outstanding submission (submission
		// order). Wait for its timeline value so completion reflects the GPU,
		// not a CPU event or frame counter.
		Outstanding o = m_Outstanding.front();
		m_Outstanding.erase( m_Outstanding.begin() );
		m_Host->WaitValue( o.value, UINT64_MAX );
		o.token->MarkComplete();
		if ( o.token->Id() > m_LastCompleted )
			m_LastCompleted = o.token->Id();
		vkFreeCommandBuffers( m_Device, m_Pool, 1, &o.cmd );
		m_Host->Collect();
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
		return !m_Outstanding.empty() && m_Host->CompletedValue() < m_Outstanding.back().value;
	}

	// -- Device loss: lifecycle state machine --
	bool SimulateDeviceLoss() override
	{
		// Everything submitted completes before the modeled loss.
		m_Host->WaitValue( m_Host->SubmittedValue(), UINT64_MAX );
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
	static render::device::vulkan::HostMemory DeviceLocal()
	{
		render::device::vulkan::HostMemory memory;
		memory.required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		return memory;
	}

	void DestroyResourceObjects( GpuResource &r )
	{
		if ( r.borrowed )
			; // the caller owns the image
		else if ( r.image != VK_NULL_HANDLE )
			m_Host->DestroyImage( r.image, r.memory );
		else if ( r.buffer != VK_NULL_HANDLE || r.memory )
			m_Host->DestroyBuffer( r.buffer, r.memory );
		r.buffer = VK_NULL_HANDLE;
		r.image = VK_NULL_HANDLE;
		r.memory = nullptr;
	}

	const GpuResource *Slot( RenderResourceHandle h ) const
	{
		if ( h == kInvalidResource || h > m_Resources.size() )
			return nullptr;
		return &m_Resources[static_cast<size_t>( h ) - 1];
	}
	GpuResource *Slot( RenderResourceHandle h )
	{
		return const_cast<GpuResource *>( static_cast<const ProviderDevice *>( this )->Slot( h ) );
	}

	std::unique_ptr<render::device::vulkan::IHostDevice> m_Host; // destroyed last
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

class VulkanProvider : public VulkanRenderBackend
{
public:
	VulkanProvider( std::unique_ptr<render::device::vulkan::IHostInstance> host,
	    const VulkanProviderOptions &options )
	    : m_Host( std::move( host ) ), m_Instance( m_Host->Instance() ), m_Options( options )
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
		for ( ProviderDevice *d : m_Devices )
			delete d;
		// Each device kept the adapter's instance alive; m_Host releases it last.
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
		if ( FindGraphicsQueueFamily( phys ) < 0 )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf(
				    error->message, sizeof( error->message ), "no graphics queue family" );
			}
			return nullptr;
		}

		// The adapter (render.device.vulkan) creates the device on this
		// provider's instance. Presentation is not a device feature: a bridge
		// presents from this device, so only the swapchain extension it will
		// need is added.
		struct Wanted
		{
			bool swapchain;
			std::vector<const char *> extensions;
			VkPhysicalDeviceFeatures2 features;
		} wanted = { m_Options.enableSwapchain &&
		                 DeviceHasExtension( phys, VK_KHR_SWAPCHAIN_EXTENSION_NAME ),
		    {}, {} };
		wanted.features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		render::device::vulkan::HostDeviceRequest host;
		host.adapterIndex = request.adapterIndex;
		host.user = &wanted;
		host.describeDevice = []( void *user, VkPhysicalDevice, uint32_t,
		                          render::device::vulkan::HostDeviceFeatures *out )
		{
			Wanted *w = static_cast<Wanted *>( user );
			if ( w->swapchain )
				w->extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
			out->features = &w->features;
			out->extensions = w->extensions.data();
			out->extensionCount = static_cast<uint32_t>( w->extensions.size() );
		};
		char reason[256] = {};
		std::unique_ptr<render::device::vulkan::IHostDevice> device =
		    m_Host->CreateDevice( host, reason, sizeof( reason ) );
		if ( !device )
		{
			if ( error )
			{
				error->status = RenderCreateStatus::kInvalidAdapter;
				std::snprintf( error->message, sizeof( error->message ), "%s", reason );
			}
			return nullptr;
		}
		const VkDevice handle = device->Info().device;

		VkCommandPoolCreateInfo pci = {};
		pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pci.queueFamilyIndex = device->Info().graphicsFamily;
		VkCommandPool pool = VK_NULL_HANDLE;
		if ( vkCreateCommandPool( handle, &pci, nullptr, &pool ) != VK_SUCCESS )
		{
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

		// The adapter enables timeline semaphores on every device.
		VkSemaphore gateSemaphore = VK_NULL_HANDLE;
		if ( m_Options.enableCompletionGate )
		{
			VkSemaphoreTypeCreateInfo type = {};
			type.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			type.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
			type.initialValue = 0;
			VkSemaphoreCreateInfo sci = {};
			sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			sci.pNext = &type;
			if ( vkCreateSemaphore( handle, &sci, nullptr, &gateSemaphore ) != VK_SUCCESS )
				gateSemaphore = VK_NULL_HANDLE;
		}

		VulkanDeviceSetup setup;
		setup.host = std::move( device );
		setup.pool = pool;
		setup.swapchainEnabled = wanted.swapchain;
		setup.gate = gateSemaphore;
		ProviderDevice *dev = new ProviderDevice( std::move( setup ), caps );
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
		for ( const ProviderDevice *d : m_Devices )
			if ( d == &device )
				return true;
		return false;
	}

	VkInstance Instance() const override { return m_Instance; }

	VulkanDeviceEndpoint *FindDevice( IRenderDevice &device ) override
	{
		for ( ProviderDevice *d : m_Devices )
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

	std::unique_ptr<render::device::vulkan::IHostInstance> m_Host;
	VkInstance m_Instance;
	VulkanProviderOptions m_Options;
	std::vector<VkPhysicalDevice> m_Physical;
	std::vector<RenderAdapterInfo> m_Adapters;
	std::vector<ProviderDevice *> m_Devices;
};

} // namespace

std::unique_ptr<VulkanRenderBackend> MakeVulkanRenderBackend(
    const VulkanProviderOptions &options, std::string *outError )
{
	// The adapter's instance, with the instance extensions a bridge needs.
	std::vector<const char *> extensions;
	for ( const std::string &e : options.instanceExtensions )
		extensions.push_back( e.c_str() );
	render::device::vulkan::HostDeviceRequest request;
	request.applicationName = "Source Native Vulkan render-backend provider";
	request.instanceExtensions = extensions.data();
	request.instanceExtensionCount = static_cast<uint32_t>( extensions.size() );
	char reason[256] = {};
	std::unique_ptr<render::device::vulkan::IHostInstance> host =
	    render::device::vulkan::HostDeviceFactory().CreateInstance(
	        request, reason, sizeof( reason ) );
	if ( !host )
	{
		if ( outError )
			*outError =
			    std::string( "no usable Vulkan loader, driver or instance extension: " ) + reason;
		return nullptr;
	}

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices( host->Instance(), &deviceCount, nullptr );
	if ( deviceCount == 0 )
	{
		if ( outError )
			*outError = "no Vulkan physical devices present";
		return nullptr;
	}

	return std::unique_ptr<VulkanRenderBackend>( new VulkanProvider( std::move( host ), options ) );
}

std::unique_ptr<IRenderBackendProvider> MakeVulkanRenderBackend( std::string *outError )
{
	return MakeVulkanRenderBackend( VulkanProviderOptions(), outError );
}

} // namespace render_vulkan
