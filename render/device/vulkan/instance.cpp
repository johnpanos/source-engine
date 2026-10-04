//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan (RFC 0016 K1): the instance, validation and
//			debug messenger, and adapter selection. Headless: no surface
//			extension is enabled. A portability implementation (MoltenVK) is
//			enumerated through VK_KHR_portability_enumeration, and
//			VK_KHR_portability_subset is enabled when the device exposes it.
//
//=============================================================================//

#include "vulkan_device.h"

#include <cstdio>
#include <cstring>

namespace render::device::vulkan
{

namespace
{

constexpr const char *kValidationLayer = "VK_LAYER_KHRONOS_validation";

VKAPI_ATTR VkBool32 VKAPI_CALL OnMessage( VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT *data, void *user )
{
	InstanceHandle *instance = static_cast<InstanceHandle *>( user );
	if ( !( severity & ( VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ) ) )
		return VK_FALSE;
	const char *message = data && data->pMessage ? data->pMessage : "(no message)";
	// The loader reports host configuration (an ICD it could not load) on the
	// same messenger; that is not a finding about this adapter's use of the API.
	if ( data && data->pMessageIdName &&
	     std::strcmp( data->pMessageIdName, "Loader Message" ) == 0 )
	{
		std::fprintf( stderr, "render.device.vulkan loader: %s\n", message );
		return VK_FALSE;
	}
	instance->messages.fetch_add( 1, std::memory_order_relaxed );
	if ( instance->sink )
		instance->sink->fetch_add( 1, std::memory_order_relaxed );
	std::fprintf( stderr, "render.device.vulkan validation: %s\n", message );
	return VK_FALSE;
}

bool HasLayer( const char *name )
{
	std::uint32_t count = 0;
	if ( vkEnumerateInstanceLayerProperties( &count, nullptr ) != VK_SUCCESS )
		return false;
	std::vector<VkLayerProperties> layers( count );
	if ( vkEnumerateInstanceLayerProperties( &count, layers.data() ) != VK_SUCCESS )
		return false;
	for ( const VkLayerProperties &layer : layers )
	{
		if ( std::strcmp( layer.layerName, name ) == 0 )
			return true;
	}
	return false;
}

bool HasInstanceExtension( const char *layer, const char *name )
{
	std::uint32_t count = 0;
	if ( vkEnumerateInstanceExtensionProperties( layer, &count, nullptr ) != VK_SUCCESS )
		return false;
	std::vector<VkExtensionProperties> extensions( count );
	if ( vkEnumerateInstanceExtensionProperties( layer, &count, extensions.data() ) != VK_SUCCESS )
		return false;
	for ( const VkExtensionProperties &extension : extensions )
	{
		if ( std::strcmp( extension.extensionName, name ) == 0 )
			return true;
	}
	return false;
}

bool HasDeviceExtension( const std::vector<VkExtensionProperties> &extensions, const char *name )
{
	for ( const VkExtensionProperties &extension : extensions )
	{
		if ( std::strcmp( extension.extensionName, name ) == 0 )
			return true;
	}
	return false;
}

// Why a physical device cannot serve the port, or its choice. The port's own
// device needs Vulkan 1.2 and dynamic rendering. A host device (host mode)
// may be Vulkan 1.1 with the timeline and synchronization2 extensions and may
// lack dynamic rendering: the legacy backend draws in render passes of its
// own (Adreno 730 on Samsung's Vulkan 1.1 driver, 2026-09-29).
DeviceResult<AdapterChoice> Evaluate( VkPhysicalDevice physical, bool host )
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties( physical, &properties );
	if ( properties.apiVersion < ( host ? VK_API_VERSION_1_1 : VK_API_VERSION_1_2 ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( properties.limits.maxBoundDescriptorSets < kMaxBindGroups )
		return Fail( DeviceStatus::kUnsupported, op );

	AdapterChoice choice;
	choice.physical = physical;
	choice.core12 = properties.apiVersion >= VK_API_VERSION_1_2;
	choice.core13 = properties.apiVersion >= VK_API_VERSION_1_3;

	std::uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( physical, &familyCount, nullptr );
	std::vector<VkQueueFamilyProperties> families( familyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( physical, &familyCount, families.data() );
	bool found = false;
	for ( std::uint32_t i = 0; i < familyCount && !found; ++i )
	{
		const VkQueueFlags wanted = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
		if ( ( families[i].queueFlags & wanted ) == wanted && families[i].queueCount > 0 )
		{
			choice.queueFamily = i;
			found = true;
		}
	}
	if ( !found )
		return Fail( DeviceStatus::kUnsupported, op );

	std::uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties( physical, nullptr, &extensionCount, nullptr );
	std::vector<VkExtensionProperties> extensions( extensionCount );
	vkEnumerateDeviceExtensionProperties( physical, nullptr, &extensionCount, extensions.data() );
	if ( !choice.core12 )
	{
		if ( !HasDeviceExtension( extensions, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME ) )
			return Fail( DeviceStatus::kUnsupported, op );
		choice.extensions.push_back( VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME );
	}
	// Whether the dynamic rendering extension is there to ask about.
	bool renderingExtension = false;
	if ( !choice.core13 )
	{
		if ( !HasDeviceExtension( extensions, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME ) )
			return Fail( DeviceStatus::kUnsupported, op );
		choice.extensions.push_back( VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME );
		renderingExtension =
		    HasDeviceExtension( extensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME );
		if ( !renderingExtension && !host )
			return Fail( DeviceStatus::kUnsupported, op );
	}
	// A portability implementation requires the application to enable it.
	if ( HasDeviceExtension( extensions, "VK_KHR_portability_subset" ) )
		choice.extensions.push_back( "VK_KHR_portability_subset" );
	// dmabuf export of LINEAR images (kExternalImages, clause D18). The
	// format modifier extension's own dependencies are core in Vulkan 1.2.
	if ( choice.core12 &&
	     HasDeviceExtension( extensions, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME ) &&
	     HasDeviceExtension( extensions, VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME ) &&
	     HasDeviceExtension( extensions, VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME ) )
	{
		choice.externalImages = true;
		choice.extensions.push_back( VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME );
		choice.extensions.push_back( VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME );
		choice.extensions.push_back( VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME );
	}

	// Only the structures this device's version and extensions define.
	VkPhysicalDeviceVulkan12Features features12{};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	VkPhysicalDeviceTimelineSemaphoreFeatures timeline{};
	timeline.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
	VkPhysicalDeviceVulkan13Features features13{};
	features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	VkPhysicalDeviceSynchronization2FeaturesKHR sync2{};
	sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
	VkPhysicalDeviceDynamicRenderingFeaturesKHR rendering{};
	rendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
	VkPhysicalDeviceFeatures2 features{};
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	auto *tail = reinterpret_cast<VkBaseOutStructure *>( &features );
	auto append = [&tail]( void *node )
	{
		tail->pNext = static_cast<VkBaseOutStructure *>( node );
		tail = tail->pNext;
	};
	append( choice.core12 ? static_cast<void *>( &features12 ) : &timeline );
	if ( choice.core13 )
		append( &features13 );
	else
	{
		append( &sync2 );
		if ( renderingExtension )
			append( &rendering );
	}
	vkGetPhysicalDeviceFeatures2( physical, &features );
	const bool timelineSemaphore =
	    choice.core12 ? features12.timelineSemaphore : timeline.timelineSemaphore;
	const bool synchronization2 =
	    choice.core13 ? features13.synchronization2 : sync2.synchronization2;
	choice.dynamicRendering =
	    choice.core13 ? features13.dynamicRendering : rendering.dynamicRendering;
	if ( !timelineSemaphore || !synchronization2 || ( !choice.dynamicRendering && !host ) )
		return Fail( DeviceStatus::kUnsupported, op );
	if ( choice.dynamicRendering && !choice.core13 )
		choice.extensions.push_back( VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME );
	choice.anisotropy = features.features.samplerAnisotropy == VK_TRUE;
	choice.textureCompressionBC = features.features.textureCompressionBC == VK_TRUE;
	choice.memoryBudget = HasDeviceExtension( extensions, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME );
	if ( choice.memoryBudget )
		choice.extensions.push_back( VK_EXT_MEMORY_BUDGET_EXTENSION_NAME );
	return choice;
}

int TypeRank( VkPhysicalDeviceType type )
{
	switch ( type )
	{
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return 0;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return 1;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return 2;
	default:
		return 3;
	}
}

} // namespace

InstanceHandle::~InstanceHandle()
{
	if ( messenger != VK_NULL_HANDLE )
	{
		auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" ) );
		if ( destroy )
			destroy( instance, messenger, nullptr );
	}
	if ( instance != VK_NULL_HANDLE )
		vkDestroyInstance( instance, nullptr );
}

bool ValidationLayerAvailable()
{
	return HasLayer( kValidationLayer );
}

DeviceResult<std::unique_ptr<InstanceHandle>> CreateInstance(
    bool validation, std::atomic<std::uint64_t> *sink )
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	auto handle = std::make_unique<InstanceHandle>();
	handle->sink = sink;

	std::uint32_t loaderVersion = VK_API_VERSION_1_0;
	if ( vkEnumerateInstanceVersion( &loaderVersion ) != VK_SUCCESS ||
	     loaderVersion < VK_API_VERSION_1_2 )
		return Fail( DeviceStatus::kUnavailable, op );

	std::vector<const char *> layers;
	std::vector<const char *> extensions;
	VkInstanceCreateFlags flags = 0;
	if ( HasInstanceExtension( nullptr, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME ) )
	{
		extensions.push_back( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
		flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
	bool validationFeatures = false;
	if ( validation && HasLayer( kValidationLayer ) )
	{
		layers.push_back( kValidationLayer );
		handle->validation = true;
		extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
		if ( HasInstanceExtension( kValidationLayer, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME ) )
		{
			extensions.push_back( VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME );
			validationFeatures = true;
		}
	}

	VkApplicationInfo application{};
	application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application.pApplicationName = "render.device.vulkan";
	application.pEngineName = "Source";
	application.apiVersion =
	    loaderVersion >= VK_API_VERSION_1_3 ? VK_API_VERSION_1_3 : VK_API_VERSION_1_2;
	handle->apiVersion = application.apiVersion;

	VkDebugUtilsMessengerCreateInfoEXT messenger{};
	messenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messenger.pfnUserCallback = &OnMessage;
	messenger.pUserData = handle.get();

	// Synchronization validation checks every barrier the adapter derives.
	const VkValidationFeatureEnableEXT enables[] = {
	    VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT };
	VkValidationFeaturesEXT features{};
	features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	features.enabledValidationFeatureCount = 1;
	features.pEnabledValidationFeatures = enables;
	features.pNext = &messenger;

	VkInstanceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.flags = flags;
	info.pApplicationInfo = &application;
	info.enabledLayerCount = static_cast<std::uint32_t>( layers.size() );
	info.ppEnabledLayerNames = layers.data();
	info.enabledExtensionCount = static_cast<std::uint32_t>( extensions.size() );
	info.ppEnabledExtensionNames = extensions.data();
	if ( handle->validation )
		info.pNext = validationFeatures ? static_cast<const void *>( &features ) : &messenger;
	const VkResult created = vkCreateInstance( &info, nullptr, &handle->instance );
	if ( created != VK_SUCCESS )
	{
		handle->instance = VK_NULL_HANDLE;
		return Fail( created == VK_ERROR_INCOMPATIBLE_DRIVER ? DeviceStatus::kUnavailable
		                                                     : StatusOf( created ),
		    op, created );
	}

	if ( handle->validation )
	{
		auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr( handle->instance, "vkCreateDebugUtilsMessengerEXT" ) );
		messenger.pNext = nullptr;
		if ( create )
		{
			const VkResult result =
			    create( handle->instance, &messenger, nullptr, &handle->messenger );
			if ( result != VK_SUCCESS )
				return Fail( StatusOf( result ), op, result );
		}
		handle->beginLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
		    vkGetInstanceProcAddr( handle->instance, "vkCmdBeginDebugUtilsLabelEXT" ) );
		handle->endLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
		    vkGetInstanceProcAddr( handle->instance, "vkCmdEndDebugUtilsLabelEXT" ) );
		handle->setObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
		    vkGetInstanceProcAddr( handle->instance, "vkSetDebugUtilsObjectNameEXT" ) );
		handle->debugUtils = true;
	}
	return handle;
}

DeviceResult<std::unique_ptr<InstanceHandle>> CreateHostInstance(
    const HostDeviceRequest &request, std::uint32_t *apiVersion )
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	auto handle = std::make_unique<InstanceHandle>();
	std::uint32_t loaderVersion = VK_API_VERSION_1_0;
	if ( vkEnumerateInstanceVersion( &loaderVersion ) != VK_SUCCESS ||
	     loaderVersion < VK_API_VERSION_1_2 )
		return Fail( DeviceStatus::kUnavailable, op );

	std::vector<const char *> extensions;
	for ( std::uint32_t i = 0; i < request.instanceExtensionCount; ++i )
		extensions.push_back( request.instanceExtensions[i] );
	auto add = [&]( const char *name )
	{
		for ( const char *have : extensions )
		{
			if ( std::strcmp( have, name ) == 0 )
				return;
		}
		extensions.push_back( name );
	};
	std::vector<const char *> layers;
	const bool layer =
	    ( request.validation || request.requireValidation ) && HasLayer( kValidationLayer );
	if ( request.requireValidation && !layer )
		return Fail( DeviceStatus::kUnavailable, op );
	bool validationFeatures = false;
	if ( layer )
	{
		layers.push_back( kValidationLayer );
		handle->validation = true;
		if ( HasInstanceExtension( kValidationLayer, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME ) )
		{
			add( VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME );
			validationFeatures = true;
		}
	}
	handle->debugUtils = layer || ( request.debugUtils && HasInstanceExtension( nullptr,
	                                                          VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) );
	if ( handle->debugUtils )
		add( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
	VkInstanceCreateFlags flags = 0;
	if ( HasInstanceExtension( nullptr, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME ) )
	{
		add( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
		flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}

	VkApplicationInfo application{};
	application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application.pApplicationName =
	    request.applicationName ? request.applicationName : "render.device.vulkan host";
	application.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	application.pEngineName = "Source";
	application.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	// Synchronization2 and dynamic rendering are core from 1.3.
	application.apiVersion =
	    loaderVersion >= VK_API_VERSION_1_3 ? VK_API_VERSION_1_3 : VK_API_VERSION_1_2;
	handle->apiVersion = application.apiVersion;

	VkInstanceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.flags = flags;
	info.pApplicationInfo = &application;
	info.enabledLayerCount = static_cast<std::uint32_t>( layers.size() );
	info.ppEnabledLayerNames = layers.data();
	info.enabledExtensionCount = static_cast<std::uint32_t>( extensions.size() );
	info.ppEnabledExtensionNames = extensions.data();
	// A host's work is checked as the adapter's is: synchronization
	// validation over every barrier either records.
	const VkValidationFeatureEnableEXT enables[] = {
	    VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT };
	VkValidationFeaturesEXT features{};
	features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	features.enabledValidationFeatureCount = 1;
	features.pEnabledValidationFeatures = enables;
	if ( validationFeatures )
		info.pNext = &features;
	const VkResult created = vkCreateInstance( &info, nullptr, &handle->instance );
	if ( created != VK_SUCCESS )
	{
		handle->instance = VK_NULL_HANDLE;
		return Fail( created == VK_ERROR_INCOMPATIBLE_DRIVER ? DeviceStatus::kUnavailable
		                                                     : StatusOf( created ),
		    op, created );
	}
	if ( handle->debugUtils )
	{
		handle->beginLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
		    vkGetInstanceProcAddr( handle->instance, "vkCmdBeginDebugUtilsLabelEXT" ) );
		handle->endLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
		    vkGetInstanceProcAddr( handle->instance, "vkCmdEndDebugUtilsLabelEXT" ) );
		handle->setObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
		    vkGetInstanceProcAddr( handle->instance, "vkSetDebugUtilsObjectNameEXT" ) );
	}
	// The host's own validation callback (it counts and logs its messages).
	if ( layer && request.messageCallback )
	{
		auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr( handle->instance, "vkCreateDebugUtilsMessengerEXT" ) );
		VkDebugUtilsMessengerCreateInfoEXT messenger{};
		messenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		messenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		messenger.pfnUserCallback = request.messageCallback;
		messenger.pUserData = request.messageUser;
		const VkResult result =
		    create ? create( handle->instance, &messenger, nullptr, &handle->messenger )
		           : VK_ERROR_EXTENSION_NOT_PRESENT;
		if ( result != VK_SUCCESS && request.requireValidation )
			return Fail( StatusOf( result ), op, result );
	}
	*apiVersion = handle->apiVersion;
	return handle;
}

DeviceResult<AdapterChoice> SelectAdapter( VkInstance instance, int adapterIndex )
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	std::uint32_t count = 0;
	if ( vkEnumeratePhysicalDevices( instance, &count, nullptr ) != VK_SUCCESS || count == 0 )
		return Fail( DeviceStatus::kUnavailable, op );
	std::vector<VkPhysicalDevice> physical( count );
	if ( vkEnumeratePhysicalDevices( instance, &count, physical.data() ) != VK_SUCCESS )
		return Fail( DeviceStatus::kUnavailable, op );

	if ( adapterIndex >= 0 )
	{
		if ( static_cast<std::uint32_t>( adapterIndex ) >= count )
			return Fail( DeviceStatus::kUnavailable, op );
		return Evaluate( physical[static_cast<std::size_t>( adapterIndex )], false );
	}

	std::optional<AdapterChoice> best;
	int bestRank = 0;
	DeviceError lastError{ DeviceStatus::kUnsupported, op, 0 };
	for ( VkPhysicalDevice candidate : physical )
	{
		DeviceResult<AdapterChoice> choice = Evaluate( candidate, false );
		if ( !choice )
		{
			lastError = choice.Error();
			continue;
		}
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties( candidate, &properties );
		const int rank = TypeRank( properties.deviceType );
		if ( !best || rank < bestRank )
		{
			best = std::move( choice ).Value();
			bestRank = rank;
		}
	}
	if ( !best )
		return foundation::MakeUnexpected( lastError );
	return std::move( *best );
}

DeviceResult<AdapterChoice> SelectHostAdapter( VkInstance instance, VkSurfaceKHR surface,
    bool requireDiscrete, int adapterIndex, const char **reason )
{
	const DeviceOperation op = DeviceOperation::kCreateDevice;
	*reason = "no Vulkan physical device";
	std::uint32_t count = 0;
	if ( vkEnumeratePhysicalDevices( instance, &count, nullptr ) != VK_SUCCESS || count == 0 )
		return Fail( DeviceStatus::kUnavailable, op );
	std::vector<VkPhysicalDevice> physical( count );
	if ( vkEnumeratePhysicalDevices( instance, &count, physical.data() ) != VK_SUCCESS )
		return Fail( DeviceStatus::kUnavailable, op );

	// The first requirement a device failed, in order, for the error.
	int furthest = -1;
	auto fail = [&]( int step, const char *why )
	{
		if ( step > furthest )
		{
			furthest = step;
			*reason = why;
		}
	};
	std::optional<AdapterChoice> best;
	int bestRank = 0;
	if ( adapterIndex >= 0 )
	{
		if ( static_cast<std::uint32_t>( adapterIndex ) >= count )
		{
			*reason = "no Vulkan physical device at the requested index";
			return Fail( DeviceStatus::kUnavailable, op );
		}
		physical = { physical[static_cast<std::size_t>( adapterIndex )] };
	}
	for ( VkPhysicalDevice candidate : physical )
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties( candidate, &properties );
		if ( requireDiscrete && properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
		{
			fail( 0, "no discrete GPU was found" );
			continue;
		}
		if ( surface != VK_NULL_HANDLE )
		{
			std::uint32_t extensionCount = 0;
			vkEnumerateDeviceExtensionProperties( candidate, nullptr, &extensionCount, nullptr );
			std::vector<VkExtensionProperties> extensions( extensionCount );
			vkEnumerateDeviceExtensionProperties(
			    candidate, nullptr, &extensionCount, extensions.data() );
			std::uint32_t formats = 0;
			std::uint32_t modes = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR( candidate, surface, &formats, nullptr );
			vkGetPhysicalDeviceSurfacePresentModesKHR( candidate, surface, &modes, nullptr );
			if ( !HasDeviceExtension( extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) ||
			     formats == 0 || modes == 0 )
			{
				fail( 1, "no Vulkan device can present to the window surface" );
				continue;
			}
		}
		DeviceResult<AdapterChoice> choice = Evaluate( candidate, true );
		if ( !choice )
		{
			fail( 2, "no presentable Vulkan device offers Vulkan 1.1 with timeline semaphores, "
			         "synchronization2, four bound descriptor sets and a graphics and compute "
			         "queue (render.device.vulkan host requirements)" );
			continue;
		}
		AdapterChoice adapter = std::move( choice ).Value();
		adapter.presentFamily = adapter.queueFamily;
		if ( surface != VK_NULL_HANDLE )
		{
			// The graphics family when it presents, else the first that does.
			VkBool32 presents = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(
			    candidate, adapter.queueFamily, surface, &presents );
			if ( !presents )
			{
				std::uint32_t families = 0;
				vkGetPhysicalDeviceQueueFamilyProperties( candidate, &families, nullptr );
				bool found = false;
				for ( std::uint32_t i = 0; i < families && !found; ++i )
				{
					vkGetPhysicalDeviceSurfaceSupportKHR( candidate, i, surface, &presents );
					if ( presents )
					{
						adapter.presentFamily = i;
						found = true;
					}
				}
				if ( !found )
				{
					fail( 1, "no Vulkan device can present to the window surface" );
					continue;
				}
			}
			// The window's device also enables the swapchain.
			adapter.extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
		}
		const int rank = TypeRank( properties.deviceType );
		if ( !best || rank < bestRank )
		{
			best = std::move( adapter );
			bestRank = rank;
		}
	}
	if ( !best )
		return Fail( DeviceStatus::kUnsupported, op );
	*reason = nullptr;
	return std::move( *best );
}

} // namespace render::device::vulkan
