//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan (RFC 0016 K1): device lifetime, facts,
//			completion, release, loss and recovery, and the provider entry
//			points. See render/device/vulkan/vulkan_device.h for the model.
//
//			Completion: one timeline semaphore on the graphics queue. Token
//			values rise by one per submission and continue across epochs (a
//			recovered device's semaphore starts at the last value), so values
//			stay monotonic. Idle waits (vkDeviceWaitIdle) happen only in
//			WaitIdle, Recover and the destructor, the port's reviewed callers
//			(tools/render/vulkan_scans.json).
//
//			Host mode (host_device.h): a legacy host borrows the device. Its
//			submissions reserve timeline values through ReserveValue, and its
//			resources are released behind those values (ReleaseHostAfter,
//			CollectHost), so the host and the port share one completion.
//
//=============================================================================//

#include "vulkan_device.h"
#include "fsr_features.h"

#include <algorithm>
#include <cstring>

namespace render::device::vulkan
{

namespace
{

constexpr std::uint64_t kMinimumRingBytes = 4096;

template <typename T> T LoadDevice( VkDevice device, const char *core, const char *khr )
{
	PFN_vkVoidFunction function = vkGetDeviceProcAddr( device, core );
	if ( !function && khr )
		function = vkGetDeviceProcAddr( device, khr );
	return reinterpret_cast<T>( function );
}

} // namespace

SetLayout::~SetLayout()
{
	if ( layout != VK_NULL_HANDLE )
		vkDestroyDescriptorSetLayout( device, layout, nullptr );
}

std::uint32_t TextureRecord::Width( std::uint32_t mip ) const
{
	return std::max( 1u, desc.width >> mip );
}

std::uint32_t TextureRecord::Height( std::uint32_t mip ) const
{
	return std::max( 1u, desc.height >> mip );
}

VulkanDevice::VulkanDevice( const VulkanAdapterOptions &options, const HostDeviceRequest *host,
    std::shared_ptr<InstanceHandle> instance )
    : m_Options( options ), m_HostRequest( host ), m_HostMode( host != nullptr ),
      m_Instance( std::move( instance ) )
{
}

VulkanDevice::~VulkanDevice()
{
	ReleaseHold(); // held work must be able to finish before the idle wait
	if ( m_Device != VK_NULL_HANDLE )
		(void)vkDeviceWaitIdle( m_Device ); // reviewed: teardown
	DestroyLogical();
}

DeviceResult<void> VulkanDevice::Initialize()
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	if ( m_Options.uploadRingBytes < kMinimumRingBytes )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( m_HostMode )
	{
		if ( auto selected = SelectHost(); !selected )
			return selected;
	}
	else
	{
		auto instance = CreateInstance( m_Options.validation, m_Options.validationCounter );
		if ( !instance )
			return foundation::MakeUnexpected( instance.Error() );
		m_Instance = std::move( instance ).Value();
		auto adapter = SelectAdapter( m_Instance->instance, m_Options.adapterIndex );
		if ( !adapter )
			return foundation::MakeUnexpected( adapter.Error() );
		m_Adapter = std::move( adapter ).Value();
	}

	vkGetPhysicalDeviceProperties( m_Adapter.physical, &m_Properties );
	vkGetPhysicalDeviceMemoryProperties( m_Adapter.physical, &m_MemoryProperties );
	m_AdapterName = m_Properties.deviceName;

	const VkPhysicalDeviceLimits &limits = m_Properties.limits;
	m_Facts.diagnosticBackend = "vulkan";
	m_Facts.adapterName = m_AdapterName;
	// Only what this adapter implements: no transient aliasing, parallel
	// native recording, async queues or ray query yet (RFC 0016 K1).
	m_Facts.capabilities = { Capability::kCompute, Capability::kStorageBuffers };
	if ( m_Adapter.externalImages || m_Options.sensitivity.nullExternalImages )
		m_Facts.capabilities.Add( Capability::kExternalImages );
	if ( m_Adapter.textureCompressionBC )
		m_Facts.capabilities.Add( Capability::kTextureCompressionBC );
	// D23: timestamps where the queue family writes them.
	{
		std::uint32_t families = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( m_Adapter.physical, &families, nullptr );
		std::vector<VkQueueFamilyProperties> properties( families );
		vkGetPhysicalDeviceQueueFamilyProperties(
		    m_Adapter.physical, &families, properties.data() );
		if ( m_Adapter.queueFamily < families &&
		     properties[m_Adapter.queueFamily].timestampValidBits > 0 &&
		     m_Properties.limits.timestampPeriod > 0.0f )
		{
			m_Facts.capabilities.Add( Capability::kTimestamps );
			m_Facts.timestampPeriodNs = m_Properties.limits.timestampPeriod;
		}
	}
	for ( std::uint32_t bit = 0; bit < static_cast<std::uint32_t>( Capability::kCount ); ++bit )
	{
		const Capability claimed = static_cast<Capability>( bit );
		if ( m_Options.sensitivity.falseClaims.Has( claimed ) )
			m_Facts.capabilities.Add( claimed ); // sensitivity fixture only
	}
	m_Facts.limits.maxBindGroups = kMaxBindGroups;
	m_Facts.limits.maxTextureDimension2D = limits.maxImageDimension2D;
	m_Facts.limits.maxColorAttachments = limits.maxColorAttachments;
	m_Facts.limits.maxVertexBuffers = limits.maxVertexInputBindings;
	m_Facts.limits.uniformBufferAlignment =
	    static_cast<std::uint32_t>( limits.minUniformBufferOffsetAlignment );
	// VkSampleCountFlagBits bit n is 2^n samples, the port's encoding.
	m_Facts.limits.sampleCounts = static_cast<std::uint32_t>(
	    limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts & 0x7fu );
	m_Facts.artifactFormat = ArtifactFormat::kSpirv;
	return CreateLogical();
}

DeviceResult<void> VulkanDevice::SelectHost()
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	const HostDeviceRequest &request = *m_HostRequest;
	if ( !m_Instance )
	{
		std::uint32_t apiVersion = 0;
		auto instance = CreateHostInstance( request, &apiVersion );
		if ( !instance )
		{
			m_FailureReason = "the Vulkan instance could not be created";
			return foundation::MakeUnexpected( instance.Error() );
		}
		m_Instance = std::move( instance ).Value();
	}
	m_HostInfo.instance = m_Instance->instance;
	m_HostInfo.instanceApiVersion = m_Instance->apiVersion;
	m_HostInfo.validation = m_Instance->validation;
	m_HostInfo.debugUtils = m_Instance->debugUtils;
	if ( request.createSurface &&
	     !request.createSurface( request.user, m_Instance->instance, &m_HostInfo.surface ) )
	{
		m_HostInfo.surface = VK_NULL_HANDLE;
		m_FailureReason = "the host could not create its presentation surface";
		return Fail( DeviceStatus::kUnavailable, op );
	}
	const char *reason = nullptr;
	auto adapter = SelectHostAdapter( m_Instance->instance, m_HostInfo.surface,
	    request.requireDiscrete, request.adapterIndex, &reason );
	if ( !adapter )
	{
		m_FailureReason = reason;
		return foundation::MakeUnexpected( adapter.Error() );
	}
	m_Adapter = std::move( adapter ).Value();
	m_HostInfo.physical = m_Adapter.physical;
	m_HostInfo.graphicsFamily = m_Adapter.queueFamily;
	m_HostInfo.presentFamily = m_Adapter.presentFamily;
	m_HostInfo.core13 = m_Adapter.core13;
	m_HostInfo.dynamicRendering = m_Adapter.dynamicRendering;
	return {};
}

namespace
{

// The structure of sType in chain, or null.
VkBaseOutStructure *FindInChain( void *head, VkStructureType type )
{
	for ( VkBaseOutStructure *node = static_cast<VkBaseOutStructure *>( head ); node;
	    node = node->pNext )
	{
		if ( node->sType == type )
			return node;
	}
	return nullptr;
}

void Append( VkPhysicalDeviceFeatures2 &head, void *node )
{
	VkBaseOutStructure *tail = reinterpret_cast<VkBaseOutStructure *>( &head );
	while ( tail->pNext )
		tail = tail->pNext;
	tail->pNext = static_cast<VkBaseOutStructure *>( node );
}

// The port's required features, merged into a host's chain: set in the
// structures the chain already has, else appended from storage.
struct RequiredFeatures
{
	VkPhysicalDeviceVulkan12Features features12{};
	VkPhysicalDeviceTimelineSemaphoreFeatures timeline12{}; // Vulkan 1.1's extension
	VkPhysicalDeviceVulkan13Features features13{};
	VkPhysicalDeviceSynchronization2FeaturesKHR sync2{};
	VkPhysicalDeviceDynamicRenderingFeaturesKHR rendering{};

	void Merge( VkPhysicalDeviceFeatures2 &head, const AdapterChoice &adapter )
	{
		if ( adapter.anisotropy )
			head.features.samplerAnisotropy = VK_TRUE;
		if ( adapter.textureCompressionBC )
			head.features.textureCompressionBC = VK_TRUE;
		if ( auto *have =
		         FindInChain( &head, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES ) )
			reinterpret_cast<VkPhysicalDeviceVulkan12Features *>( have )->timelineSemaphore =
			    VK_TRUE;
		else if ( auto *timeline = FindInChain(
		              &head, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES ) )
			reinterpret_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>( timeline )
			    ->timelineSemaphore = VK_TRUE;
		else if ( adapter.core12 )
		{
			features12 = {};
			features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			features12.timelineSemaphore = VK_TRUE;
			Append( head, &features12 );
		}
		else
		{
			// Vulkan 1.1: the 1.2 structure is not defined; the extension's is.
			timeline12 = {};
			timeline12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
			timeline12.timelineSemaphore = VK_TRUE;
			Append( head, &timeline12 );
		}
		if ( adapter.core13 )
		{
			if ( auto *have =
			         FindInChain( &head, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES ) )
			{
				auto *features = reinterpret_cast<VkPhysicalDeviceVulkan13Features *>( have );
				features->synchronization2 = VK_TRUE;
				features->dynamicRendering = VK_TRUE;
				return;
			}
			features13 = {};
			features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			features13.synchronization2 = VK_TRUE;
			features13.dynamicRendering = VK_TRUE;
			Append( head, &features13 );
			return;
		}
		if ( auto *have = FindInChain(
		         &head, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR ) )
			reinterpret_cast<VkPhysicalDeviceSynchronization2FeaturesKHR *>( have )
			    ->synchronization2 = VK_TRUE;
		else
		{
			sync2 = {};
			sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
			sync2.synchronization2 = VK_TRUE;
			Append( head, &sync2 );
		}
		if ( !adapter.dynamicRendering )
			return;
		if ( auto *have = FindInChain(
		         &head, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR ) )
			reinterpret_cast<VkPhysicalDeviceDynamicRenderingFeaturesKHR *>( have )
			    ->dynamicRendering = VK_TRUE;
		else
		{
			rendering = {};
			rendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
			rendering.dynamicRendering = VK_TRUE;
			Append( head, &rendering );
		}
	}
};

bool BufferDeviceAddress( void *head )
{
	if ( auto *have = FindInChain( head, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES ) )
		return reinterpret_cast<VkPhysicalDeviceVulkan12Features *>( have )->bufferDeviceAddress;
	if ( auto *have =
	         FindInChain( head, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES ) )
		return reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>( have )
		    ->bufferDeviceAddress;
	return false;
}

} // namespace

DeviceResult<void> VulkanDevice::CreateLogical()
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	const float priority = 1.0f;
	VkDeviceQueueCreateInfo queues[2]{};
	queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues[0].queueFamilyIndex = m_Adapter.queueFamily;
	queues[0].queueCount = 1;
	queues[0].pQueuePriorities = &priority;
	std::uint32_t queueCount = 1;
	if ( m_HostMode && m_Adapter.presentFamily != m_Adapter.queueFamily )
	{
		queues[1] = queues[0];
		queues[1].queueFamilyIndex = m_Adapter.presentFamily;
		queueCount = 2;
	}

	std::vector<const char *> extensions = m_Adapter.extensions;
	VkPhysicalDeviceFeatures2 ownFeatures{};
	ownFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	VkPhysicalDeviceFeatures2 *features = &ownFeatures;
	if ( m_HostMode && m_HostRequest && m_HostRequest->describeDevice )
	{
		HostDeviceFeatures wanted;
		m_HostRequest->describeDevice(
		    m_HostRequest->user, m_Adapter.physical, m_Adapter.queueFamily, &wanted );
		if ( wanted.features )
			features = wanted.features;
		for ( std::uint32_t i = 0; i < wanted.extensionCount; ++i )
		{
			const char *name = wanted.extensions[i];
			if ( std::none_of( extensions.begin(), extensions.end(),
			         [name]( const char *have )
			         {
				         return std::strcmp( have, name ) == 0;
			         } ) )
				extensions.push_back( name );
		}
	}
	FsrFeatures fsr;
	if ( m_Options.fsr411 )
	{
		if ( !fsr.Enable( m_Adapter.physical, *features, extensions ) )
		{
			m_FailureReason = "FSR 4.1.1 shader requirements unavailable";
			return Fail( DeviceStatus::kUnsupported, op );
		}
	}
	RequiredFeatures required;
	required.Merge( *features, m_Adapter );

	VkDeviceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.pNext = features;
	info.queueCreateInfoCount = queueCount;
	info.pQueueCreateInfos = queues;
	info.enabledExtensionCount = static_cast<std::uint32_t>( extensions.size() );
	info.ppEnabledExtensionNames = extensions.data();
	VkResult result = vkCreateDevice( m_Adapter.physical, &info, nullptr, &m_Device );
	if ( result != VK_SUCCESS )
	{
		m_Device = VK_NULL_HANDLE;
		m_FailureReason = "vkCreateDevice failed";
		return Fail( StatusOf( result ), op, result );
	}
	const std::uint32_t apiVersion =
	    m_Instance->apiVersion != 0 ? m_Instance->apiVersion : VK_API_VERSION_1_2;
	result = m_Memory.Create( m_Instance->instance, m_Adapter.physical, m_Device,
	    std::min( apiVersion, m_Properties.apiVersion ), BufferDeviceAddress( features ) );
	if ( result != VK_SUCCESS )
	{
		DestroyLogical();
		m_FailureReason = "the memory allocator could not be created";
		return Fail( StatusOf( result ), op, result );
	}

	m_Vk.cmdPipelineBarrier2 = LoadDevice<PFN_vkCmdPipelineBarrier2>(
	    m_Device, "vkCmdPipelineBarrier2", "vkCmdPipelineBarrier2KHR" );
	m_Vk.queueSubmit2 =
	    LoadDevice<PFN_vkQueueSubmit2>( m_Device, "vkQueueSubmit2", "vkQueueSubmit2KHR" );
	if ( m_Adapter.dynamicRendering )
	{
		m_Vk.cmdBeginRendering = LoadDevice<PFN_vkCmdBeginRendering>(
		    m_Device, "vkCmdBeginRendering", "vkCmdBeginRenderingKHR" );
		m_Vk.cmdEndRendering = LoadDevice<PFN_vkCmdEndRendering>(
		    m_Device, "vkCmdEndRendering", "vkCmdEndRenderingKHR" );
	}
	if ( m_Adapter.externalImages )
		m_Vk.getMemoryFd =
		    LoadDevice<PFN_vkGetMemoryFdKHR>( m_Device, "vkGetMemoryFdKHR", nullptr );
	m_Vk.getSemaphoreCounterValue = LoadDevice<PFN_vkGetSemaphoreCounterValue>(
	    m_Device, "vkGetSemaphoreCounterValue", "vkGetSemaphoreCounterValueKHR" );
	m_Vk.waitSemaphores =
	    LoadDevice<PFN_vkWaitSemaphores>( m_Device, "vkWaitSemaphores", "vkWaitSemaphoresKHR" );
	m_Vk.signalSemaphore =
	    LoadDevice<PFN_vkSignalSemaphore>( m_Device, "vkSignalSemaphore", "vkSignalSemaphoreKHR" );
	const bool rendering =
	    !m_Adapter.dynamicRendering || ( m_Vk.cmdBeginRendering && m_Vk.cmdEndRendering );
	if ( !m_Vk.cmdPipelineBarrier2 || !m_Vk.queueSubmit2 || !rendering ||
	     !m_Vk.getSemaphoreCounterValue || !m_Vk.waitSemaphores )
	{
		DestroyLogical();
		return Fail( DeviceStatus::kUnsupported, op );
	}
	vkGetDeviceQueue( m_Device, m_Adapter.queueFamily, 0, &m_Queue );
	m_HostInfo.device = m_Device;
	m_HostInfo.graphicsQueue = m_Queue;
	m_HostInfo.presentQueue = m_Queue;
	if ( m_HostMode && m_Adapter.presentFamily != m_Adapter.queueFamily )
		vkGetDeviceQueue( m_Device, m_Adapter.presentFamily, 0, &m_HostInfo.presentQueue );

	// The timeline continues from the last value, across epochs.
	VkSemaphoreTypeCreateInfo type{};
	type.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	type.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	type.initialValue = m_Submitted;
	VkSemaphoreCreateInfo semaphore{};
	semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore.pNext = &type;
	result = vkCreateSemaphore( m_Device, &semaphore, nullptr, &m_Timeline );
	if ( result != VK_SUCCESS )
	{
		m_Timeline = VK_NULL_HANDLE;
		DestroyLogical();
		return Fail( StatusOf( result ), op, result );
	}
	m_Completed.store( m_Submitted );

	VkDescriptorSetLayoutCreateInfo empty{};
	empty.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	result = vkCreateDescriptorSetLayout( m_Device, &empty, nullptr, &m_EmptySetLayout );
	if ( result != VK_SUCCESS )
	{
		m_EmptySetLayout = VK_NULL_HANDLE;
		DestroyLogical();
		return Fail( StatusOf( result ), op, result );
	}

	auto ring = CreateHostBuffer( m_Options.uploadRingBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, op );
	if ( !ring )
	{
		DestroyLogical();
		return foundation::MakeUnexpected( ring.Error() );
	}
	Name( VK_OBJECT_TYPE_BUFFER, reinterpret_cast<std::uint64_t>( ring.Value().buffer ),
	    "render.device.vulkan upload ring" );
	m_Ring.Attach( ring.Value(), m_Options.uploadRingBytes );
	return {};
}

void VulkanDevice::DestroyLogical()
{
	for ( auto &entry : m_ComputeInterop )
		if ( auto payload = entry.lock() )
			payload->DeviceDestroyed();
	// Teardown/recovery also retires every remaining logical handle.
	for ( std::size_t i = 0; i < ResourceActivity::kKinds; ++i )
		m_ResourceActivity.destroyed[i].store( m_ResourceActivity.created[i].load() );
	m_ResourceActivity.pending.store( 0 );
	// Every holder of a descriptor set layout goes before the device.
	m_Releases.clear();
	if ( m_Device != VK_NULL_HANDLE )
	{
		// A host's pending releases: the device is idle here (every caller
		// waited, or the device is lost), so each is due.
		std::deque<HostRelease> pending;
		{
			std::lock_guard<std::mutex> lock( m_HostReleaseMutex );
			pending.swap( m_HostReleases );
		}
		for ( const HostRelease &release : pending )
		{
			if ( release.destroy )
				release.destroy( release.object );
			if ( release.buffer != VK_NULL_HANDLE || release.allocation )
			{
				if ( release.image != VK_NULL_HANDLE )
					m_Memory.DestroyImage( release.image, release.allocation );
				else
					m_Memory.DestroyBuffer( release.buffer, release.allocation );
			}
			else if ( release.image != VK_NULL_HANDLE )
				m_Memory.DestroyImage( release.image, nullptr );
		}
		for ( auto &[id, pipeline] : m_Pipelines )
		{
			vkDestroyPipeline( m_Device, pipeline.pipeline, nullptr );
			vkDestroyPipelineLayout( m_Device, pipeline.pipelineLayout, nullptr );
		}
		for ( auto &[id, sampler] : m_Samplers )
			vkDestroySampler( m_Device, sampler.sampler, nullptr );
		for ( auto &[id, texture] : m_Textures )
			DestroyTexture( texture );
		for ( auto &[id, buffer] : m_Buffers )
			m_Memory.DestroyBuffer( buffer.buffer, buffer.memory );
	}
	m_Pipelines.clear();
	m_Samplers.clear();
	m_Textures.clear();
	m_Imported.clear();
	m_Buffers.clear();
	m_BindGroups.clear(); // their sets go with the pools below
	m_Layouts.clear();
	if ( m_Device == VK_NULL_HANDLE )
		return;

	for ( CommandContext &context : m_FreeContexts )
	{
		vkDestroyCommandPool( m_Device, context.pool, nullptr );
		if ( context.queries != VK_NULL_HANDLE )
			vkDestroyQueryPool( m_Device, context.queries, nullptr );
	}
	for ( CommandContext &context : m_InFlight )
	{
		vkDestroyCommandPool( m_Device, context.pool, nullptr );
		if ( context.queries != VK_NULL_HANDLE )
			vkDestroyQueryPool( m_Device, context.queries, nullptr );
		for ( HostBuffer &staging : context.staging )
			DestroyHostBuffer( staging );
	}
	m_FreeContexts.clear();
	m_InFlight.clear();
	for ( VkDescriptorPool pool : m_DescriptorPools )
		vkDestroyDescriptorPool( m_Device, pool, nullptr );
	m_DescriptorPools.clear();
	HostBuffer ring = m_Ring.Detach();
	DestroyHostBuffer( ring );
	vkDestroyDescriptorSetLayout( m_Device, m_EmptySetLayout, nullptr );
	m_EmptySetLayout = VK_NULL_HANDLE;
	vkDestroySemaphore( m_Device, m_Timeline, nullptr );
	m_Timeline = VK_NULL_HANDLE;
	vkDestroySemaphore( m_Device, m_Hold, nullptr );
	m_Hold = VK_NULL_HANDLE;
	m_HoldValue = 0;
	m_Holding = false;
	m_Memory.Destroy();
	vkDestroyDevice( m_Device, nullptr );
	m_Device = VK_NULL_HANDLE;
	m_Queue = VK_NULL_HANDLE;
	m_Vk = {};
}

std::uint64_t VulkanDevice::ValidationMessageCount() const
{
	return m_Instance ? m_Instance->messages.load() : 0;
}

// Completion -----------------------------------------------------------------

std::uint64_t VulkanDevice::CompletedValue() const
{
	if ( m_Device == VK_NULL_HANDLE || m_State != DeviceState::kAvailable )
		return m_Completed.load();
	std::uint64_t value = 0;
	const VkResult result = m_Vk.getSemaphoreCounterValue( m_Device, m_Timeline, &value );
	if ( result == VK_ERROR_DEVICE_LOST )
		m_State = DeviceState::kLost;
	else if ( result == VK_SUCCESS )
	{
		std::uint64_t cached = m_Completed.load();
		while ( value > cached && !m_Completed.compare_exchange_weak( cached, value ) )
		{
		}
	}
	return m_Completed.load();
}

bool VulkanDevice::IsComplete( CompletionToken token ) const
{
	if ( !token.NamesSubmission() || token.epoch < m_Epoch )
		return true;
	if ( token.epoch > m_Epoch || token.queue != QueueKind::kGraphics || token.value > m_Submitted )
		return false;
	return token.value <= CompletedValue();
}

void VulkanDevice::RecycleCompleted()
{
	const std::uint64_t completed = CompletedValue();
	while ( !m_InFlight.empty() && m_InFlight.front().value <= completed )
	{
		CommandContext context = std::move( m_InFlight.front() );
		m_InFlight.pop_front();
		for ( HostBuffer &staging : context.staging )
			DestroyHostBuffer( staging );
		context.staging.clear();
		context.compute.clear();
		if ( vkResetCommandPool( m_Device, context.pool, 0 ) == VK_SUCCESS )
		{
			m_FreeContexts.push_back( std::move( context ) );
		}
		else
		{
			vkDestroyCommandPool( m_Device, context.pool, nullptr );
			if ( context.queries != VK_NULL_HANDLE )
				vkDestroyQueryPool( m_Device, context.queries, nullptr );
		}
	}
	m_Ring.Retire( completed );
}

std::size_t VulkanDevice::Poll()
{
	if ( m_Device == VK_NULL_HANDLE )
		return 0;
	RecycleCompleted();
	return CollectReleases();
}

std::size_t VulkanDevice::CollectReleases()
{
	std::size_t freed = 0;
	for ( auto it = m_Releases.begin(); it != m_Releases.end(); )
	{
		if ( !IsComplete( it->token ) )
		{
			++it;
			continue;
		}
		Erase( it->resource );
		it = m_Releases.erase( it );
		++freed;
	}
	return freed;
}

bool *VulkanDevice::ReleasedFlag( ResourceId resource )
{
	auto flag = [&]( auto &map ) -> bool *
	{
		const auto found = map.find( resource.value );
		return found == map.end() ? nullptr : &found->second.released;
	};
	switch ( resource.kind )
	{
	case ResourceKind::kBuffer:
		return flag( m_Buffers );
	case ResourceKind::kTexture:
		return flag( m_Textures );
	case ResourceKind::kSampler:
		return flag( m_Samplers );
	case ResourceKind::kPipeline:
		return flag( m_Pipelines );
	case ResourceKind::kBindGroupLayout:
		return flag( m_Layouts );
	case ResourceKind::kBindGroup:
		return flag( m_BindGroups );
	case ResourceKind::kNone:
		break;
	}
	return nullptr;
}

void VulkanDevice::Erase( ResourceId resource )
{
	if ( const bool *released = ReleasedFlag( resource ) )
	{
		++m_ResourceActivity.destroyed[std::size_t( resource.kind )];
		if ( *released )
			--m_ResourceActivity.pending;
	}
	switch ( resource.kind )
	{
	case ResourceKind::kBuffer:
		if ( auto found = m_Buffers.find( resource.value ); found != m_Buffers.end() )
		{
			m_Memory.DestroyBuffer( found->second.buffer, found->second.memory );
			m_Buffers.erase( found );
		}
		break;
	case ResourceKind::kTexture:
		if ( auto found = m_Textures.find( resource.value ); found != m_Textures.end() )
		{
			DestroyTexture( found->second );
			m_Textures.erase( found );
			m_Imported.erase( resource.value );
		}
		break;
	case ResourceKind::kSampler:
		if ( auto found = m_Samplers.find( resource.value ); found != m_Samplers.end() )
		{
			vkDestroySampler( m_Device, found->second.sampler, nullptr );
			m_Samplers.erase( found );
		}
		break;
	case ResourceKind::kPipeline:
		if ( auto found = m_Pipelines.find( resource.value ); found != m_Pipelines.end() )
		{
			vkDestroyPipeline( m_Device, found->second.pipeline, nullptr );
			vkDestroyPipelineLayout( m_Device, found->second.pipelineLayout, nullptr );
			m_Pipelines.erase( found );
		}
		break;
	case ResourceKind::kBindGroupLayout:
		m_Layouts.erase( resource.value );
		break;
	case ResourceKind::kBindGroup:
		if ( auto found = m_BindGroups.find( resource.value ); found != m_BindGroups.end() )
		{
			(void)vkFreeDescriptorSets( m_Device, found->second.pool, 1, &found->second.set );
			m_BindGroups.erase( found );
		}
		break;
	case ResourceKind::kNone:
		break;
	}
}

DeviceResult<void> VulkanDevice::Release( ResourceId resource, CompletionToken releaseAfter )
{
	bool *released = ReleasedFlag( resource );
	if ( !released || *released )
		return Fail( DeviceStatus::kInvalidHandle, DeviceOperation::kRelease );
	*released = true;
	m_Releases.push_back( { resource, releaseAfter } );
	++m_ResourceActivity.releaseRequests;
	++m_ResourceActivity.pending;
	return {};
}

ResourceActivity VulkanDevice::ReadResourceActivity() const
{
	ResourceActivity result;
	for ( std::size_t i = 0; i < ResourceActivity::kKinds; ++i )
	{
		result.created[i] = m_ResourceActivity.created[i].load();
		result.destroyed[i] = m_ResourceActivity.destroyed[i].load();
	}
	result.releaseRequests = m_ResourceActivity.releaseRequests.load();
	result.bufferBytes = m_ResourceActivity.bufferBytes.load();
	result.supported = true;
	result.epoch = Epoch();
	result.live = result.Created() > result.Destroyed() ? result.Created() - result.Destroyed() : 0;
	result.pending = m_ResourceActivity.pending.load();
	return result;
}

std::size_t VulkanDevice::LiveResourceCount() const
{
	return m_Buffers.size() + m_Textures.size() + m_Samplers.size() + m_Layouts.size() +
	       m_BindGroups.size() + m_Pipelines.size();
}

// Host completion and releases -------------------------------------------------

std::uint64_t VulkanDevice::ReserveValue()
{
	return ++m_Submitted;
}

void VulkanDevice::AbandonValue( std::uint64_t value )
{
	// Signal it from the queue, after every earlier submission, so waits on
	// it (and on later values) still complete in order.
	VkSemaphoreSubmitInfo signal{};
	signal.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signal.semaphore = m_Timeline;
	signal.value = value;
	signal.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	VkSubmitInfo2 submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submit.signalSemaphoreInfoCount = 1;
	submit.pSignalSemaphoreInfos = &signal;
	std::lock_guard<std::mutex> lock( m_QueueMutex );
	if ( m_Vk.queueSubmit2( m_Queue, 1, &submit, VK_NULL_HANDLE ) == VK_ERROR_DEVICE_LOST )
		MarkLost();
}

VkResult VulkanDevice::WaitValue( std::uint64_t value, std::uint64_t timeoutNs )
{
	if ( value <= CompletedValue() )
		return VK_SUCCESS;
	VkSemaphoreWaitInfo wait{};
	wait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	wait.semaphoreCount = 1;
	wait.pSemaphores = &m_Timeline;
	wait.pValues = &value;
	const VkResult result = m_Vk.waitSemaphores( m_Device, &wait, timeoutNs );
	if ( result == VK_ERROR_DEVICE_LOST )
		MarkLost();
	(void)CompletedValue();
	return result;
}

void VulkanDevice::ReleaseHostAfter( const HostRelease &release )
{
	{
		std::lock_guard<std::mutex> lock( m_HostReleaseMutex );
		m_HostReleases.push_back( release );
	}
	if ( release.value <= CompletedValue() )
		(void)CollectHost();
}

std::size_t VulkanDevice::CollectHost()
{
	const std::uint64_t completed = CompletedValue();
	std::vector<HostRelease> due;
	{
		std::lock_guard<std::mutex> lock( m_HostReleaseMutex );
		for ( auto it = m_HostReleases.begin(); it != m_HostReleases.end(); )
		{
			if ( it->value <= completed )
			{
				due.push_back( *it );
				it = m_HostReleases.erase( it );
			}
			else
				++it;
		}
	}
	for ( const HostRelease &release : due )
	{
		if ( release.destroy )
			release.destroy( release.object );
		if ( release.image != VK_NULL_HANDLE )
			m_Memory.DestroyImage( release.image, release.allocation );
		else if ( release.buffer != VK_NULL_HANDLE || release.allocation )
			m_Memory.DestroyBuffer( release.buffer, release.allocation );
	}
	return due.size();
}

std::size_t VulkanDevice::PendingHostReleases() const
{
	std::lock_guard<std::mutex> lock( m_HostReleaseMutex );
	return m_HostReleases.size();
}

// Idle, loss and recovery ------------------------------------------------------

DeviceResult<void> VulkanDevice::WaitIdle()
{
	if ( m_Device == VK_NULL_HANDLE )
		return {};
	// Idle means every accepted submission has run, held ones included.
	ReleaseHold();
	const VkResult result = vkDeviceWaitIdle( m_Device ); // reviewed: the port's WaitIdle
	if ( result == VK_ERROR_DEVICE_LOST )
	{
		MarkLost();
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kSubmit, result );
	}
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), DeviceOperation::kSubmit, result );
	(void)CompletedValue();
	return {};
}

DeviceResult<void> VulkanDevice::Recover()
{
	if ( m_State == DeviceState::kAvailable )
		return {};
	// A host's device is rebuilt by its host (the legacy backend does not
	// recover a lost device); its handles would dangle.
	if ( m_HostMode )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	m_State = DeviceState::kRecovering;
	// Reviewed idle wait: loss recovery. A lost device returns at once.
	ReleaseHold();
	if ( m_Device != VK_NULL_HANDLE )
		(void)vkDeviceWaitIdle( m_Device );
	DestroyLogical();
	++m_Epoch;
	if ( auto created = CreateLogical(); !created )
	{
		m_State = DeviceState::kFatal;
		return created;
	}
	m_State = DeviceState::kAvailable;
	return {};
}

// Test hold -------------------------------------------------------------------

bool VulkanDevice::Hold( bool held )
{
	if ( !held )
	{
		ReleaseHold();
		return true;
	}
	if ( m_Device == VK_NULL_HANDLE || !m_Vk.signalSemaphore )
		return false;
	if ( m_Hold == VK_NULL_HANDLE )
	{
		VkSemaphoreTypeCreateInfo type{};
		type.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		type.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		type.initialValue = 0;
		VkSemaphoreCreateInfo semaphore{};
		semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore.pNext = &type;
		if ( vkCreateSemaphore( m_Device, &semaphore, nullptr, &m_Hold ) != VK_SUCCESS )
		{
			m_Hold = VK_NULL_HANDLE;
			return false;
		}
		Name( VK_OBJECT_TYPE_SEMAPHORE, reinterpret_cast<std::uint64_t>( m_Hold ),
		    "render.device.vulkan test hold" );
	}
	m_Holding = true;
	return true;
}

void VulkanDevice::ReleaseHold()
{
	if ( !m_Holding )
		return;
	m_Holding = false;
	++m_HoldValue;
	VkSemaphoreSignalInfo signal{};
	signal.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	signal.semaphore = m_Hold;
	signal.value = m_HoldValue;
	if ( m_Vk.signalSemaphore( m_Device, &signal ) == VK_ERROR_DEVICE_LOST )
		MarkLost();
}

// Encoders and readback --------------------------------------------------------

DeviceResult<CommandEncoder> VulkanDevice::BeginEncoder( QueueKind queue )
{
	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, DeviceOperation::kBeginEncoder );
	// Only the graphics queue: async compute and transfer are not claimed.
	if ( queue != QueueKind::kGraphics )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kBeginEncoder );
	return CommandEncoder( queue, std::make_unique<VulkanEncoder>( *this, queue ) );
}

DeviceResult<void> VulkanDevice::ReadBuffer(
    BufferId id, std::uint64_t offset, std::span<std::byte> out )
{
	const DeviceOperation op = DeviceOperation::kReadBuffer;
	BufferRecord *buffer = LiveBuffer( id.value );
	if ( !buffer )
		return Fail( DeviceStatus::kInvalidHandle, op );
	if ( buffer->desc.memory != MemoryKind::kReadback || offset > buffer->desc.size ||
	     out.size() > buffer->desc.size - offset || !buffer->mapped )
		return Fail( DeviceStatus::kInvalidDescription, op );
	if ( !buffer->coherent )
	{
		const VkResult result = m_Memory.Invalidate( buffer->memory, 0, VK_WHOLE_SIZE );
		if ( result != VK_SUCCESS )
			return Fail( StatusOf( result ), op, result );
	}
	if ( !out.empty() )
		std::memcpy( out.data(), buffer->mapped + offset, out.size() );
	return {};
}

BufferRecord *VulkanDevice::LiveBuffer( std::uint64_t id )
{
	const auto found = m_Buffers.find( id );
	return found != m_Buffers.end() && !found->second.released ? &found->second : nullptr;
}

TextureRecord *VulkanDevice::LiveTexture( std::uint64_t id )
{
	const auto found = m_Textures.find( id );
	return found != m_Textures.end() && !found->second.released ? &found->second : nullptr;
}

std::optional<LayoutView> VulkanDevice::FindLayout( BindGroupLayoutId id ) const
{
	const auto found = m_Layouts.find( id.value );
	if ( found == m_Layouts.end() || found->second.released )
		return std::nullopt;
	return LayoutView{ found->second.role, found->second.bindings };
}

// Provider ---------------------------------------------------------------------

namespace
{

DeviceResult<std::unique_ptr<IRenderDevice2>> CreateFromRequest( const DeviceRequest &request )
{
	VulkanAdapterOptions options;
	options.validation = request.validation;
	auto device = Create( options );
	if ( !device )
		return device;
	if ( FirstMissing( device.Value()->Facts().capabilities, request.required ) )
		return Fail( DeviceStatus::kUnsupported, DeviceOperation::kCreateDevice );
	return device;
}

} // namespace

const DeviceProviderDescriptor &Describe()
{
	static const DeviceProviderDescriptor descriptor{ "vulkan", &CreateFromRequest };
	return descriptor;
}

DeviceResult<std::unique_ptr<IRenderDevice2>> Create( const VulkanAdapterOptions &options )
{
	auto device = std::make_unique<VulkanDevice>( options );
	if ( auto initialized = device->Initialize(); !initialized )
		return foundation::MakeUnexpected( initialized.Error() );
	return std::unique_ptr<IRenderDevice2>( std::move( device ) );
}

std::uint64_t ValidationMessages( const IRenderDevice2 &device )
{
	const VulkanDevice *vulkan = dynamic_cast<const VulkanDevice *>( &device );
	return vulkan ? vulkan->ValidationMessageCount() : 0;
}

std::uint64_t DeferredUploads( const IRenderDevice2 &device )
{
	const VulkanDevice *vulkan = dynamic_cast<const VulkanDevice *>( &device );
	return vulkan ? vulkan->DeferredUploadCount() : 0;
}

bool HoldSubmissions( IRenderDevice2 &device, bool held )
{
	VulkanDevice *vulkan = dynamic_cast<VulkanDevice *>( &device );
	return vulkan && vulkan->Hold( held );
}

} // namespace render::device::vulkan
