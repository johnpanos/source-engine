//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan device bring-up and presentation core.
//          See vulkan_device.h for the contract.
//
//===========================================================================//

#include "vulkan_device.h"
#include "demo_triangle_spv.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace render_vulkan
{

namespace
{

const char *const kValidationLayer = "VK_LAYER_KHRONOS_validation";
const char *const kSwapchainExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

void Log( const char *fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	std::fprintf( stderr, "[NativeVulkan] " );
	std::vfprintf( stderr, fmt, args );
	va_end( args );
}

void SetError( std::string *outError, const std::string &message )
{
	if ( outError )
		*outError = message;
	Log( "error: %s\n", message.c_str() );
}

const char *ResultString( VkResult r )
{
	switch ( r )
	{
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	default:
		return "VK_ERROR_<other>";
	}
}

} // namespace

VkBool32 VulkanDebugCallbackTrampoline( VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT /*type*/, const VkDebugUtilsMessengerCallbackDataEXT *data,
    void *userData )
{
	CVulkanContext *ctx = static_cast<CVulkanContext *>( userData );
	if ( severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
	{
		if ( ctx )
			++ctx->m_validationErrorCount;
		Log( "validation: %s\n", data && data->pMessage ? data->pMessage : "<null>" );
	}
	return VK_FALSE;
}

CVulkanContext::~CVulkanContext()
{
	Shutdown();
}

void CVulkanContext::SetClearColor( float r, float g, float b, float a )
{
	m_clearColor.float32[0] = r;
	m_clearColor.float32[1] = g;
	m_clearColor.float32[2] = b;
	m_clearColor.float32[3] = a;
}

const std::vector<uint8_t> &CVulkanContext::GetCapturedPixels( int *outW, int *outH ) const
{
	if ( outW )
		*outW = m_capturedWidth;
	if ( outH )
		*outH = m_capturedHeight;
	return m_capturedPixels;
}

bool CVulkanContext::Init(
    SDL_Window *window, const VulkanContextConfig &config, std::string *outError )
{
	if ( IsValid() )
	{
		SetError( outError, "Init called on an already-initialized context" );
		return false;
	}
	if ( !window )
	{
		SetError( outError, "Init called with a null SDL window" );
		return false;
	}

	m_window = window;
	m_config = config;
	m_framesInFlight =
	    std::max<uint32_t>( 1, std::min<uint32_t>( config.framesInFlight, kMaxFramesInFlight ) );
	m_validationErrorCount = 0;

	if ( !CreateInstance( outError ) || !SetupDebugMessenger( outError ) ||
	     !CreateSurface( outError ) || !PickPhysicalDevice( outError ) ||
	     !CreateLogicalDevice( outError ) || !CreateSwapchain( outError ) ||
	     !CreateRenderPass( outError ) || !CreateFramebuffers( outError ) ||
	     !CreateCommandResources( outError ) || !CreateSyncObjects( outError ) )
	{
		Shutdown();
		return false;
	}

	Log( "device '%s' vendor=0x%04x %s validation=%s %ux%u images=%zu\n", m_deviceName.c_str(),
	    m_vendorId, m_isDiscrete ? "discrete" : "integrated/other",
	    m_validationEnabled ? "on" : "off", m_swapExtent.width, m_swapExtent.height,
	    m_swapImages.size() );
	return true;
}

bool CVulkanContext::CreateInstance( std::string *outError )
{
	Uint32 sdlExtCount = 0;
	char const *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions( &sdlExtCount );
	if ( !sdlExtensions )
	{
		SetError(
		    outError, std::string( "SDL_Vulkan_GetInstanceExtensions failed: " ) + SDL_GetError() );
		return false;
	}

	std::vector<const char *> extensions( sdlExtensions, sdlExtensions + sdlExtCount );

	// Decide whether validation can/should be enabled.
	bool wantValidation = m_config.enableValidation || m_config.requireValidation;
	bool layerAvailable = false;
	if ( wantValidation )
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
		std::vector<VkLayerProperties> layers( layerCount );
		if ( layerCount )
			vkEnumerateInstanceLayerProperties( &layerCount, layers.data() );
		for ( const VkLayerProperties &lp : layers )
		{
			if ( std::strcmp( lp.layerName, kValidationLayer ) == 0 )
			{
				layerAvailable = true;
				break;
			}
		}
		if ( !layerAvailable )
		{
			if ( m_config.requireValidation )
			{
				SetError( outError,
				    "required validation layer VK_LAYER_KHRONOS_validation is not available" );
				return false;
			}
			Log( "validation requested but layer unavailable; continuing without it\n" );
		}
	}

	m_validationEnabled = wantValidation && layerAvailable;
	if ( m_validationEnabled )
		extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = m_config.appName ? m_config.appName : "Source Native Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = "Source";
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>( extensions.size() );
	createInfo.ppEnabledExtensionNames = extensions.data();
	const char *enabledLayers[] = { kValidationLayer };
	if ( m_validationEnabled )
	{
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = enabledLayers;
	}

	VkResult r = vkCreateInstance( &createInfo, nullptr, &m_instance );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateInstance failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

bool CVulkanContext::SetupDebugMessenger( std::string *outError )
{
	if ( !m_validationEnabled )
		return true;

	m_pfnCreateDebugMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
	    vkGetInstanceProcAddr( m_instance, "vkCreateDebugUtilsMessengerEXT" ) );
	m_pfnDestroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
	    vkGetInstanceProcAddr( m_instance, "vkDestroyDebugUtilsMessengerEXT" ) );
	if ( !m_pfnCreateDebugMessenger || !m_pfnDestroyDebugMessenger )
	{
		// Extension advertised but entry points missing: only fatal if required.
		if ( m_config.requireValidation )
		{
			SetError( outError, "debug utils messenger entry points unavailable" );
			return false;
		}
		return true;
	}

	VkDebugUtilsMessengerCreateInfoEXT info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	info.pfnUserCallback = VulkanDebugCallbackTrampoline;
	info.pUserData = this;

	VkResult r = m_pfnCreateDebugMessenger( m_instance, &info, nullptr, &m_debugMessenger );
	if ( r != VK_SUCCESS && m_config.requireValidation )
	{
		SetError( outError,
		    std::string( "vkCreateDebugUtilsMessengerEXT failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

bool CVulkanContext::CreateSurface( std::string *outError )
{
	if ( !SDL_Vulkan_CreateSurface( m_window, m_instance, nullptr, &m_surface ) )
	{
		SetError( outError, std::string( "SDL_Vulkan_CreateSurface failed: " ) + SDL_GetError() );
		return false;
	}
	return true;
}

bool CVulkanContext::PickPhysicalDevice( std::string *outError )
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices( m_instance, &count, nullptr );
	if ( count == 0 )
	{
		SetError( outError, "no Vulkan physical devices found" );
		return false;
	}
	std::vector<VkPhysicalDevice> devices( count );
	vkEnumeratePhysicalDevices( m_instance, &count, devices.data() );

	int bestScore = -1;
	VkPhysicalDevice best = VK_NULL_HANDLE;
	uint32_t bestGraphics = UINT32_MAX;
	uint32_t bestPresent = UINT32_MAX;

	for ( VkPhysicalDevice dev : devices )
	{
		// Must expose the swapchain extension.
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties( dev, nullptr, &extCount, nullptr );
		std::vector<VkExtensionProperties> exts( extCount );
		if ( extCount )
			vkEnumerateDeviceExtensionProperties( dev, nullptr, &extCount, exts.data() );
		bool hasSwapchain = false;
		for ( const VkExtensionProperties &e : exts )
			if ( std::strcmp( e.extensionName, kSwapchainExtension ) == 0 )
				hasSwapchain = true;
		if ( !hasSwapchain )
			continue;

		// Must present at least one surface format and present mode.
		uint32_t fmtCount = 0, pmCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR( dev, m_surface, &fmtCount, nullptr );
		vkGetPhysicalDeviceSurfacePresentModesKHR( dev, m_surface, &pmCount, nullptr );
		if ( fmtCount == 0 || pmCount == 0 )
			continue;

		// Find graphics + present queue families.
		uint32_t qfCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( dev, &qfCount, nullptr );
		std::vector<VkQueueFamilyProperties> qfs( qfCount );
		vkGetPhysicalDeviceQueueFamilyProperties( dev, &qfCount, qfs.data() );

		uint32_t graphics = UINT32_MAX, present = UINT32_MAX;
		for ( uint32_t i = 0; i < qfCount; ++i )
		{
			if ( ( qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) && graphics == UINT32_MAX )
				graphics = i;
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR( dev, i, m_surface, &presentSupport );
			if ( presentSupport )
			{
				// Prefer a family that does both.
				if ( ( qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) )
					present = i;
				else if ( present == UINT32_MAX )
					present = i;
			}
		}
		if ( graphics == UINT32_MAX || present == UINT32_MAX )
			continue;

		VkPhysicalDeviceProperties props = {};
		vkGetPhysicalDeviceProperties( dev, &props );
		bool discrete = ( props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU );
		if ( m_config.requireDiscreteGpu && !discrete )
			continue;

		int score = 0;
		if ( discrete )
			score += 1000;
		else if ( props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
			score += 500;
		else if ( props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU )
			score += 100;

		if ( score > bestScore )
		{
			bestScore = score;
			best = dev;
			bestGraphics = graphics;
			bestPresent = present;
		}
	}

	if ( best == VK_NULL_HANDLE )
	{
		SetError(
		    outError, m_config.requireDiscreteGpu
		                  ? "no discrete GPU with graphics+present+swapchain support was found"
		                  : "no Vulkan device with graphics+present+swapchain support was found" );
		return false;
	}

	m_physicalDevice = best;
	m_graphicsQueueFamily = bestGraphics;
	m_presentQueueFamily = bestPresent;

	VkPhysicalDeviceProperties props = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &props );
	m_deviceName = props.deviceName;
	m_vendorId = props.vendorID;
	m_deviceId = props.deviceID;
	m_isDiscrete = ( props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU );

	VkPhysicalDeviceMemoryProperties mem = {};
	vkGetPhysicalDeviceMemoryProperties( m_physicalDevice, &mem );
	m_deviceLocalMemoryBytes = 0;
	for ( uint32_t i = 0; i < mem.memoryHeapCount; ++i )
		if ( mem.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT )
			m_deviceLocalMemoryBytes += mem.memoryHeaps[i].size;

	return true;
}

bool CVulkanContext::CreateLogicalDevice( std::string *outError )
{
	float priority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueInfos;

	VkDeviceQueueCreateInfo gq = {};
	gq.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	gq.queueFamilyIndex = m_graphicsQueueFamily;
	gq.queueCount = 1;
	gq.pQueuePriorities = &priority;
	queueInfos.push_back( gq );

	if ( m_presentQueueFamily != m_graphicsQueueFamily )
	{
		VkDeviceQueueCreateInfo pq = gq;
		pq.queueFamilyIndex = m_presentQueueFamily;
		queueInfos.push_back( pq );
	}

	const char *deviceExts[] = { kSwapchainExtension };

	VkPhysicalDeviceFeatures features = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>( queueInfos.size() );
	createInfo.pQueueCreateInfos = queueInfos.data();
	createInfo.enabledExtensionCount = 1;
	createInfo.ppEnabledExtensionNames = deviceExts;
	createInfo.pEnabledFeatures = &features;

	VkResult r = vkCreateDevice( m_physicalDevice, &createInfo, nullptr, &m_device );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateDevice failed: " ) + ResultString( r ) );
		return false;
	}

	vkGetDeviceQueue( m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue );
	vkGetDeviceQueue( m_device, m_presentQueueFamily, 0, &m_presentQueue );
	return true;
}

bool CVulkanContext::CreateSwapchain( std::string *outError )
{
	VkSurfaceCapabilitiesKHR caps = {};
	VkResult r = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_physicalDevice, m_surface, &caps );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed: " ) +
		                        ResultString( r ) );
		return false;
	}

	// Choose surface format: prefer B8G8R8A8_UNORM (straightforward readback),
	// then any BGRA/RGBA 8-bit, else the first reported.
	uint32_t fmtCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( m_physicalDevice, m_surface, &fmtCount, nullptr );
	std::vector<VkSurfaceFormatKHR> formats( fmtCount );
	vkGetPhysicalDeviceSurfaceFormatsKHR( m_physicalDevice, m_surface, &fmtCount, formats.data() );
	VkSurfaceFormatKHR chosen = formats[0];
	for ( const VkSurfaceFormatKHR &f : formats )
	{
		if ( f.format == VK_FORMAT_B8G8R8A8_UNORM &&
		     f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
		{
			chosen = f;
			break;
		}
	}
	m_swapFormat = chosen.format;
	m_swapColorSpace = chosen.colorSpace;

	// Present mode: mailbox when requested and available, else guaranteed FIFO.
	m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if ( m_config.preferMailbox )
	{
		uint32_t pmCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR( m_physicalDevice, m_surface, &pmCount, nullptr );
		std::vector<VkPresentModeKHR> modes( pmCount );
		vkGetPhysicalDeviceSurfacePresentModesKHR(
		    m_physicalDevice, m_surface, &pmCount, modes.data() );
		for ( VkPresentModeKHR m : modes )
			if ( m == VK_PRESENT_MODE_MAILBOX_KHR )
				m_presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}

	// Extent: honor the surface's fixed extent, else clamp the drawable size.
	if ( caps.currentExtent.width != UINT32_MAX )
	{
		m_swapExtent = caps.currentExtent;
	}
	else
	{
		int w = 0, h = 0;
		SDL_GetWindowSizeInPixels( m_window, &w, &h );
		VkExtent2D e = {
		    static_cast<uint32_t>( std::max( 0, w ) ), static_cast<uint32_t>( std::max( 0, h ) ) };
		e.width =
		    std::max( caps.minImageExtent.width, std::min( caps.maxImageExtent.width, e.width ) );
		e.height = std::max(
		    caps.minImageExtent.height, std::min( caps.maxImageExtent.height, e.height ) );
		m_swapExtent = e;
	}

	if ( m_swapExtent.width == 0 || m_swapExtent.height == 0 )
	{
		// Zero-size (minimized) window: retain state without a swapchain; frames
		// are skipped until a non-zero Resize arrives. Not an error.
		m_swapchain = VK_NULL_HANDLE;
		m_swapImages.clear();
		return true;
	}

	uint32_t imageCount = caps.minImageCount + 1;
	if ( caps.maxImageCount > 0 && imageCount > caps.maxImageCount )
		imageCount = caps.maxImageCount;

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = m_surface;
	info.minImageCount = imageCount;
	info.imageFormat = m_swapFormat;
	info.imageColorSpace = m_swapColorSpace;
	info.imageExtent = m_swapExtent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	info.preTransform = caps.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = m_presentMode;
	info.clipped = VK_TRUE;
	info.oldSwapchain = VK_NULL_HANDLE;

	uint32_t families[2] = { m_graphicsQueueFamily, m_presentQueueFamily };
	if ( m_graphicsQueueFamily != m_presentQueueFamily )
	{
		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = families;
	}
	else
	{
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	r = vkCreateSwapchainKHR( m_device, &info, nullptr, &m_swapchain );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateSwapchainKHR failed: " ) + ResultString( r ) );
		return false;
	}

	uint32_t actual = 0;
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &actual, nullptr );
	m_swapImages.resize( actual );
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &actual, m_swapImages.data() );

	m_swapImageViews.resize( actual );
	for ( uint32_t i = 0; i < actual; ++i )
	{
		VkImageViewCreateInfo iv = {};
		iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv.image = m_swapImages[i];
		iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv.format = m_swapFormat;
		iv.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
		    VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		iv.subresourceRange.levelCount = 1;
		iv.subresourceRange.layerCount = 1;
		r = vkCreateImageView( m_device, &iv, nullptr, &m_swapImageViews[i] );
		if ( r != VK_SUCCESS )
		{
			SetError( outError, std::string( "vkCreateImageView failed: " ) + ResultString( r ) );
			return false;
		}
	}

	m_imagesInFlight.assign( actual, VK_NULL_HANDLE );

	if ( !CreateDepthResources( outError ) )
		return false;
	return true;
}

bool CVulkanContext::CreateDepthResources( std::string *outError )
{
	const size_t count = m_swapImages.size();
	m_depthImages.resize( count, VK_NULL_HANDLE );
	m_depthMemories.resize( count, VK_NULL_HANDLE );
	m_depthViews.resize( count, VK_NULL_HANDLE );

	for ( size_t i = 0; i < count; ++i )
	{
		VkImageCreateInfo img = {};
		img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		img.imageType = VK_IMAGE_TYPE_2D;
		img.format = m_depthFormat;
		img.extent = { m_swapExtent.width, m_swapExtent.height, 1 };
		img.mipLevels = 1;
		img.arrayLayers = 1;
		img.samples = VK_SAMPLE_COUNT_1_BIT;
		img.tiling = VK_IMAGE_TILING_OPTIMAL;
		img.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if ( vkCreateImage( m_device, &img, nullptr, &m_depthImages[i] ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateImage (depth) failed" );
			return false;
		}
		VkMemoryRequirements req = {};
		vkGetImageMemoryRequirements( m_device, m_depthImages[i], &req );
		bool found = false;
		uint32_t type =
		    FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &found );
		if ( !found )
		{
			SetError( outError, "no device-local memory type for depth image" );
			return false;
		}
		VkMemoryAllocateInfo ai = {};
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = req.size;
		ai.memoryTypeIndex = type;
		if ( vkAllocateMemory( m_device, &ai, nullptr, &m_depthMemories[i] ) != VK_SUCCESS )
		{
			SetError( outError, "vkAllocateMemory (depth) failed" );
			return false;
		}
		vkBindImageMemory( m_device, m_depthImages[i], m_depthMemories[i], 0 );

		VkImageViewCreateInfo iv = {};
		iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv.image = m_depthImages[i];
		iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv.format = m_depthFormat;
		iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		iv.subresourceRange.levelCount = 1;
		iv.subresourceRange.layerCount = 1;
		if ( vkCreateImageView( m_device, &iv, nullptr, &m_depthViews[i] ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateImageView (depth) failed" );
			return false;
		}
	}
	return true;
}

bool CVulkanContext::CreateRenderPass( std::string *outError )
{
	if ( m_renderPass != VK_NULL_HANDLE )
		return true; // render pass depends only on format, which is stable

	VkAttachmentDescription color = {};
	color.format = m_swapFormat;
	color.samples = VK_SAMPLE_COUNT_1_BIT;
	color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// Leave the image in a color-attachment layout; EndFrame() performs the
	// explicit transition to PRESENT_SRC (or the capture path) so a single
	// render pass serves both present and readback.
	color.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorRef = {};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment (cleared each frame) so depth-tested 3D geometry resolves
	// occlusion. 2D demos disable depth test/write, so they are unaffected.
	VkAttachmentDescription depth = {};
	depth.format = m_depthFormat;
	depth.samples = VK_SAMPLE_COUNT_1_BIT;
	depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef = {};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = &depthRef;

	VkSubpassDependency dep = {};
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = 0;
	dep.srcStageMask =
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dep.dstStageMask =
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dep.srcAccessMask = 0;
	dep.dstAccessMask =
	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = { color, depth };
	VkRenderPassCreateInfo rp = {};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp.attachmentCount = 2;
	rp.pAttachments = attachments;
	rp.subpassCount = 1;
	rp.pSubpasses = &subpass;
	rp.dependencyCount = 1;
	rp.pDependencies = &dep;

	VkResult r = vkCreateRenderPass( m_device, &rp, nullptr, &m_renderPass );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateRenderPass failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

bool CVulkanContext::CreateFramebuffers( std::string *outError )
{
	m_framebuffers.resize( m_swapImageViews.size() );
	for ( size_t i = 0; i < m_swapImageViews.size(); ++i )
	{
		VkImageView attachments[] = { m_swapImageViews[i], m_depthViews[i] };
		VkFramebufferCreateInfo fb = {};
		fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb.renderPass = m_renderPass;
		fb.attachmentCount = 2;
		fb.pAttachments = attachments;
		fb.width = m_swapExtent.width;
		fb.height = m_swapExtent.height;
		fb.layers = 1;
		VkResult r = vkCreateFramebuffer( m_device, &fb, nullptr, &m_framebuffers[i] );
		if ( r != VK_SUCCESS )
		{
			SetError( outError, std::string( "vkCreateFramebuffer failed: " ) + ResultString( r ) );
			return false;
		}
	}
	return true;
}

bool CVulkanContext::CreateCommandResources( std::string *outError )
{
	VkCommandPoolCreateInfo pool = {};
	pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool.queueFamilyIndex = m_graphicsQueueFamily;
	VkResult r = vkCreateCommandPool( m_device, &pool, nullptr, &m_commandPool );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateCommandPool failed: " ) + ResultString( r ) );
		return false;
	}

	m_commandBuffers.resize( m_framesInFlight );
	VkCommandBufferAllocateInfo alloc = {};
	alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc.commandPool = m_commandPool;
	alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc.commandBufferCount = m_framesInFlight;
	r = vkAllocateCommandBuffers( m_device, &alloc, m_commandBuffers.data() );
	if ( r != VK_SUCCESS )
	{
		SetError(
		    outError, std::string( "vkAllocateCommandBuffers failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

bool CVulkanContext::CreateSyncObjects( std::string *outError )
{
	m_imageAvailable.resize( m_framesInFlight );
	m_renderFinished.resize( m_framesInFlight );
	m_inFlight.resize( m_framesInFlight );

	VkSemaphoreCreateInfo sem = {};
	sem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fence = {};
	fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for ( uint32_t i = 0; i < m_framesInFlight; ++i )
	{
		if ( vkCreateSemaphore( m_device, &sem, nullptr, &m_imageAvailable[i] ) != VK_SUCCESS ||
		     vkCreateSemaphore( m_device, &sem, nullptr, &m_renderFinished[i] ) != VK_SUCCESS ||
		     vkCreateFence( m_device, &fence, nullptr, &m_inFlight[i] ) != VK_SUCCESS )
		{
			SetError( outError, "failed to create per-frame synchronization objects" );
			return false;
		}
	}
	return true;
}

uint32_t CVulkanContext::FindMemoryType(
    uint32_t typeBits, VkMemoryPropertyFlags props, bool *found ) const
{
	VkPhysicalDeviceMemoryProperties mem = {};
	vkGetPhysicalDeviceMemoryProperties( m_physicalDevice, &mem );
	for ( uint32_t i = 0; i < mem.memoryTypeCount; ++i )
	{
		if ( ( typeBits & ( 1u << i ) ) && ( mem.memoryTypes[i].propertyFlags & props ) == props )
		{
			if ( found )
				*found = true;
			return i;
		}
	}
	if ( found )
		*found = false;
	return 0;
}

bool CVulkanContext::CreateCaptureImage( std::string *outError )
{
	// Recreate only when the target extent changed.
	if ( m_captureImage != VK_NULL_HANDLE && m_captureExtent.width == m_swapExtent.width &&
	     m_captureExtent.height == m_swapExtent.height )
		return true;

	if ( m_captureImage != VK_NULL_HANDLE )
	{
		vkDestroyImage( m_device, m_captureImage, nullptr );
		m_captureImage = VK_NULL_HANDLE;
	}
	if ( m_captureMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_captureMemory, nullptr );
		m_captureMemory = VK_NULL_HANDLE;
	}

	VkImageCreateInfo img = {};
	img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img.imageType = VK_IMAGE_TYPE_2D;
	img.format = m_swapFormat;
	img.extent = { m_swapExtent.width, m_swapExtent.height, 1 };
	img.mipLevels = 1;
	img.arrayLayers = 1;
	img.samples = VK_SAMPLE_COUNT_1_BIT;
	img.tiling = VK_IMAGE_TILING_LINEAR;
	img.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult r = vkCreateImage( m_device, &img, nullptr, &m_captureImage );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateImage (capture) failed: " ) + ResultString( r ) );
		return false;
	}

	VkMemoryRequirements req = {};
	vkGetImageMemoryRequirements( m_device, m_captureImage, &req );
	bool found = false;
	uint32_t type = FindMemoryType( req.memoryTypeBits,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &found );
	if ( !found )
		type = FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &found );
	if ( !found )
	{
		SetError( outError, "no host-visible memory type for capture image" );
		return false;
	}

	VkMemoryAllocateInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = req.size;
	ai.memoryTypeIndex = type;
	r = vkAllocateMemory( m_device, &ai, nullptr, &m_captureMemory );
	if ( r != VK_SUCCESS )
	{
		SetError(
		    outError, std::string( "vkAllocateMemory (capture) failed: " ) + ResultString( r ) );
		return false;
	}
	vkBindImageMemory( m_device, m_captureImage, m_captureMemory, 0 );
	m_captureExtent = m_swapExtent;
	return true;
}

bool CVulkanContext::CreateShaderModule(
    const uint32_t *code, size_t sizeBytes, VkShaderModule *outModule, std::string *outError )
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = sizeBytes;
	info.pCode = code;
	VkResult r = vkCreateShaderModule( m_device, &info, nullptr, outModule );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateShaderModule failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

bool CVulkanContext::CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags props, VkBuffer *outBuffer, VkDeviceMemory *outMemory,
    std::string *outError )
{
	VkBufferCreateInfo bi = {};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = size;
	bi.usage = usage;
	bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkResult r = vkCreateBuffer( m_device, &bi, nullptr, outBuffer );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateBuffer failed: " ) + ResultString( r ) );
		return false;
	}

	VkMemoryRequirements req = {};
	vkGetBufferMemoryRequirements( m_device, *outBuffer, &req );
	bool found = false;
	uint32_t type = FindMemoryType( req.memoryTypeBits, props, &found );
	if ( !found )
	{
		SetError( outError, "no memory type satisfies the requested buffer properties" );
		return false;
	}

	VkMemoryAllocateInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = req.size;
	ai.memoryTypeIndex = type;
	r = vkAllocateMemory( m_device, &ai, nullptr, outMemory );
	if ( r != VK_SUCCESS )
	{
		SetError(
		    outError, std::string( "vkAllocateMemory (buffer) failed: " ) + ResultString( r ) );
		return false;
	}
	vkBindBufferMemory( m_device, *outBuffer, *outMemory, 0 );
	return true;
}

bool CVulkanContext::InitDemoTriangle( std::string *outError )
{
	if ( !IsValid() || m_renderPass == VK_NULL_HANDLE )
	{
		SetError( outError, "InitDemoTriangle before a valid Init()" );
		return false;
	}
	if ( m_demoPipeline != VK_NULL_HANDLE )
		return true;

	// Interleaved vertex: vec2 position, vec3 color. A green triangle large
	// enough to cover the frame center while leaving the corners cleared.
	struct DemoVertex
	{
		float pos[2];
		float color[3];
	};
	const DemoVertex verts[] = {
	    { { 0.0f, -0.7f }, { 0.0f, 1.0f, 0.0f } },
	    { { 0.7f, 0.7f }, { 0.0f, 1.0f, 0.0f } },
	    { { -0.7f, 0.7f }, { 0.0f, 1.0f, 0.0f } },
	};
	m_demoVertexCount = 3;

	if ( !CreateBuffer( sizeof( verts ), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &m_demoVertexBuffer, &m_demoVertexMemory, outError ) )
		return false;

	void *mapped = nullptr;
	if ( vkMapMemory( m_device, m_demoVertexMemory, 0, sizeof( verts ), 0, &mapped ) != VK_SUCCESS )
	{
		SetError( outError, "vkMapMemory (demo vertex buffer) failed" );
		return false;
	}
	std::memcpy( mapped, verts, sizeof( verts ) );
	vkUnmapMemory( m_device, m_demoVertexMemory );

	VkShaderModule vert = VK_NULL_HANDLE, frag = VK_NULL_HANDLE;
	if ( !CreateShaderModule(
	         g_demoTriangleVertSpv, sizeof( g_demoTriangleVertSpv ), &vert, outError ) )
		return false;
	if ( !CreateShaderModule(
	         g_demoTriangleFragSpv, sizeof( g_demoTriangleFragSpv ), &frag, outError ) )
	{
		vkDestroyShaderModule( m_device, vert, nullptr );
		return false;
	}

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName = "main";

	VkVertexInputBindingDescription binding = {};
	binding.binding = 0;
	binding.stride = sizeof( DemoVertex );
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attrs[2] = {};
	attrs[0].location = 0;
	attrs[0].binding = 0;
	attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
	attrs[0].offset = 0;
	attrs[1].location = 1;
	attrs[1].binding = 0;
	attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[1].offset = sizeof( float ) * 2;

	VkPipelineVertexInputStateCreateInfo vin = {};
	vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vin.vertexBindingDescriptionCount = 1;
	vin.pVertexBindingDescriptions = &binding;
	vin.vertexAttributeDescriptionCount = 2;
	vin.pVertexAttributeDescriptions = attrs;

	VkPipelineInputAssemblyStateCreateInfo ia = {};
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rs = {};
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = VK_CULL_MODE_NONE;
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo ms = {};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState cba = {};
	cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	cba.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo cb = {};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments = &cba;

	VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynStates;

	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	VkResult r = vkCreatePipelineLayout( m_device, &pl, nullptr, &m_demoPipelineLayout );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreatePipelineLayout failed: " ) + ResultString( r ) );
		vkDestroyShaderModule( m_device, vert, nullptr );
		vkDestroyShaderModule( m_device, frag, nullptr );
		return false;
	}

	VkGraphicsPipelineCreateInfo gp = {};
	gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp.stageCount = 2;
	gp.pStages = stages;
	gp.pVertexInputState = &vin;
	gp.pInputAssemblyState = &ia;
	gp.pViewportState = &vp;
	gp.pRasterizationState = &rs;
	gp.pMultisampleState = &ms;
	gp.pColorBlendState = &cb;
	gp.pDynamicState = &dyn;
	// Depth attachment present in the render pass; this 2D demo ignores depth.
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_FALSE;
	ds.depthWriteEnable = VK_FALSE;
	gp.pDepthStencilState = &ds;
	gp.layout = m_demoPipelineLayout;
	gp.renderPass = m_renderPass;
	gp.subpass = 0;

	r = vkCreateGraphicsPipelines( m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_demoPipeline );
	vkDestroyShaderModule( m_device, vert, nullptr );
	vkDestroyShaderModule( m_device, frag, nullptr );
	if ( r != VK_SUCCESS )
	{
		SetError(
		    outError, std::string( "vkCreateGraphicsPipelines failed: " ) + ResultString( r ) );
		return false;
	}

	Log( "demo triangle pipeline ready\n" );
	return true;
}

void CVulkanContext::DestroyDemoTriangle()
{
	if ( m_demoPipeline != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_demoPipeline, nullptr );
		m_demoPipeline = VK_NULL_HANDLE;
	}
	if ( m_demoPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( m_device, m_demoPipelineLayout, nullptr );
		m_demoPipelineLayout = VK_NULL_HANDLE;
	}
	if ( m_demoVertexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( m_device, m_demoVertexBuffer, nullptr );
		m_demoVertexBuffer = VK_NULL_HANDLE;
	}
	if ( m_demoVertexMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_demoVertexMemory, nullptr );
		m_demoVertexMemory = VK_NULL_HANDLE;
	}
	m_demoVertexCount = 0;
	m_drawDemoTriangle = false;
}

bool CVulkanContext::BeginSingleTimeCommands( VkCommandBuffer *outCmd, std::string *outError )
{
	VkCommandBufferAllocateInfo alloc = {};
	alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc.commandPool = m_commandPool;
	alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc.commandBufferCount = 1;
	if ( vkAllocateCommandBuffers( m_device, &alloc, outCmd ) != VK_SUCCESS )
	{
		SetError( outError, "vkAllocateCommandBuffers (single-time) failed" );
		return false;
	}
	VkCommandBufferBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer( *outCmd, &begin );
	return true;
}

bool CVulkanContext::EndSingleTimeCommands( VkCommandBuffer cmd, std::string *outError )
{
	if ( vkEndCommandBuffer( cmd ) != VK_SUCCESS )
	{
		SetError( outError, "vkEndCommandBuffer (single-time) failed" );
		vkFreeCommandBuffers( m_device, m_commandPool, 1, &cmd );
		return false;
	}
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	VkResult r = vkQueueSubmit( m_graphicsQueue, 1, &submit, VK_NULL_HANDLE );
	if ( r == VK_SUCCESS )
		r = vkQueueWaitIdle( m_graphicsQueue );
	vkFreeCommandBuffers( m_device, m_commandPool, 1, &cmd );
	if ( r != VK_SUCCESS )
	{
		SetError(
		    outError, std::string( "single-time command submit failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

bool CVulkanContext::InitTexturedQuad( std::string *outError )
{
	if ( !IsValid() || m_renderPass == VK_NULL_HANDLE )
	{
		SetError( outError, "InitTexturedQuad before a valid Init()" );
		return false;
	}
	if ( m_texQuadPipeline != VK_NULL_HANDLE )
		return true;

	// A small solid-magenta RGBA texture. Magenta appears nowhere else in the
	// scene, so reading it back off the quad unambiguously proves the texture
	// was uploaded, bound, and sampled.
	const uint32_t texW = 4, texH = 4;
	const VkDeviceSize texBytes = VkDeviceSize( texW ) * texH * 4;
	std::vector<uint8_t> pixels( texBytes );
	for ( uint32_t i = 0; i < texW * texH; ++i )
	{
		pixels[i * 4 + 0] = 255; // R
		pixels[i * 4 + 1] = 0;   // G
		pixels[i * 4 + 2] = 255; // B
		pixels[i * 4 + 3] = 255; // A
	}

	// Device-local sampled image.
	VkImageCreateInfo img = {};
	img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img.imageType = VK_IMAGE_TYPE_2D;
	img.format = VK_FORMAT_R8G8B8A8_UNORM;
	img.extent = { texW, texH, 1 };
	img.mipLevels = 1;
	img.arrayLayers = 1;
	img.samples = VK_SAMPLE_COUNT_1_BIT;
	img.tiling = VK_IMAGE_TILING_OPTIMAL;
	img.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if ( vkCreateImage( m_device, &img, nullptr, &m_texImage ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateImage (texture) failed" );
		return false;
	}
	VkMemoryRequirements req = {};
	vkGetImageMemoryRequirements( m_device, m_texImage, &req );
	bool found = false;
	uint32_t type =
	    FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &found );
	if ( !found )
	{
		SetError( outError, "no device-local memory type for texture" );
		return false;
	}
	VkMemoryAllocateInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = req.size;
	ai.memoryTypeIndex = type;
	if ( vkAllocateMemory( m_device, &ai, nullptr, &m_texMemory ) != VK_SUCCESS )
	{
		SetError( outError, "vkAllocateMemory (texture) failed" );
		return false;
	}
	vkBindImageMemory( m_device, m_texImage, m_texMemory, 0 );

	// Staging buffer with the pixels.
	VkBuffer staging = VK_NULL_HANDLE;
	VkDeviceMemory stagingMem = VK_NULL_HANDLE;
	if ( !CreateBuffer( texBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging,
	         &stagingMem, outError ) )
		return false;
	void *mapped = nullptr;
	vkMapMemory( m_device, stagingMem, 0, texBytes, 0, &mapped );
	std::memcpy( mapped, pixels.data(), texBytes );
	vkUnmapMemory( m_device, stagingMem );

	// Upload: UNDEFINED -> TRANSFER_DST, copy, TRANSFER_DST -> SHADER_READ_ONLY.
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( !BeginSingleTimeCommands( &cmd, outError ) )
	{
		vkDestroyBuffer( m_device, staging, nullptr );
		vkFreeMemory( m_device, stagingMem, nullptr );
		return false;
	}
	VkImageMemoryBarrier toDst = {};
	toDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.image = m_texImage;
	toDst.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	toDst.srcAccessMask = 0;
	toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
	    0, nullptr, 0, nullptr, 1, &toDst );

	VkBufferImageCopy copy = {};
	copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.imageExtent = { texW, texH, 1 };
	vkCmdCopyBufferToImage(
	    cmd, staging, m_texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy );

	VkImageMemoryBarrier toRead = toDst;
	toRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	toRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	toRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toRead );

	if ( !EndSingleTimeCommands( cmd, outError ) )
	{
		vkDestroyBuffer( m_device, staging, nullptr );
		vkFreeMemory( m_device, stagingMem, nullptr );
		return false;
	}
	vkDestroyBuffer( m_device, staging, nullptr );
	vkFreeMemory( m_device, stagingMem, nullptr );

	// View + sampler.
	VkImageViewCreateInfo iv = {};
	iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	iv.image = m_texImage;
	iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
	iv.format = VK_FORMAT_R8G8B8A8_UNORM;
	iv.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	if ( vkCreateImageView( m_device, &iv, nullptr, &m_texView ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateImageView (texture) failed" );
		return false;
	}
	VkSamplerCreateInfo sc = {};
	sc.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sc.magFilter = VK_FILTER_NEAREST;
	sc.minFilter = VK_FILTER_NEAREST;
	sc.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	if ( vkCreateSampler( m_device, &sc, nullptr, &m_texSampler ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateSampler failed" );
		return false;
	}

	// Descriptor set layout / pool / set.
	VkDescriptorSetLayoutBinding lb = {};
	lb.binding = 0;
	lb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	lb.descriptorCount = 1;
	lb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo dl = {};
	dl.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dl.bindingCount = 1;
	dl.pBindings = &lb;
	if ( vkCreateDescriptorSetLayout( m_device, &dl, nullptr, &m_texDescLayout ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateDescriptorSetLayout failed" );
		return false;
	}
	VkDescriptorPoolSize ps = {};
	ps.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ps.descriptorCount = 1;
	VkDescriptorPoolCreateInfo dp = {};
	dp.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dp.maxSets = 1;
	dp.poolSizeCount = 1;
	dp.pPoolSizes = &ps;
	if ( vkCreateDescriptorPool( m_device, &dp, nullptr, &m_texDescPool ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateDescriptorPool failed" );
		return false;
	}
	VkDescriptorSetAllocateInfo da = {};
	da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	da.descriptorPool = m_texDescPool;
	da.descriptorSetCount = 1;
	da.pSetLayouts = &m_texDescLayout;
	if ( vkAllocateDescriptorSets( m_device, &da, &m_texDescSet ) != VK_SUCCESS )
	{
		SetError( outError, "vkAllocateDescriptorSets failed" );
		return false;
	}
	VkDescriptorImageInfo dii = {};
	dii.sampler = m_texSampler;
	dii.imageView = m_texView;
	dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkWriteDescriptorSet wds = {};
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.dstSet = m_texDescSet;
	wds.dstBinding = 0;
	wds.descriptorCount = 1;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds.pImageInfo = &dii;
	vkUpdateDescriptorSets( m_device, 1, &wds, 0, nullptr );

	// Quad vertex buffer: pos (vec2) + uv (vec2), two triangles.
	struct QuadVertex
	{
		float pos[2];
		float uv[2];
	};
	const QuadVertex quad[] = {
	    { { -0.9f, -0.9f }, { 0.0f, 0.0f } },
	    { { 0.9f, -0.9f }, { 1.0f, 0.0f } },
	    { { 0.9f, 0.9f }, { 1.0f, 1.0f } },
	    { { -0.9f, -0.9f }, { 0.0f, 0.0f } },
	    { { 0.9f, 0.9f }, { 1.0f, 1.0f } },
	    { { -0.9f, 0.9f }, { 0.0f, 1.0f } },
	};
	m_texQuadVertexCount = 6;
	if ( !CreateBuffer( sizeof( quad ), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &m_texQuadVertexBuffer, &m_texQuadVertexMemory, outError ) )
		return false;
	void *qmap = nullptr;
	vkMapMemory( m_device, m_texQuadVertexMemory, 0, sizeof( quad ), 0, &qmap );
	std::memcpy( qmap, quad, sizeof( quad ) );
	vkUnmapMemory( m_device, m_texQuadVertexMemory );

	// Shaders + pipeline.
	VkShaderModule vert = VK_NULL_HANDLE, frag = VK_NULL_HANDLE;
	if ( !CreateShaderModule(
	         g_demoTexQuadVertSpv, sizeof( g_demoTexQuadVertSpv ), &vert, outError ) )
		return false;
	if ( !CreateShaderModule(
	         g_demoTexQuadFragSpv, sizeof( g_demoTexQuadFragSpv ), &frag, outError ) )
	{
		vkDestroyShaderModule( m_device, vert, nullptr );
		return false;
	}

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName = "main";

	VkVertexInputBindingDescription binding = {};
	binding.binding = 0;
	binding.stride = sizeof( QuadVertex );
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attrs[2] = {};
	attrs[0].location = 0;
	attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
	attrs[0].offset = 0;
	attrs[1].location = 1;
	attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
	attrs[1].offset = sizeof( float ) * 2;
	VkPipelineVertexInputStateCreateInfo vin = {};
	vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vin.vertexBindingDescriptionCount = 1;
	vin.pVertexBindingDescriptions = &binding;
	vin.vertexAttributeDescriptionCount = 2;
	vin.pVertexAttributeDescriptions = attrs;

	VkPipelineInputAssemblyStateCreateInfo ia = {};
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount = 1;
	VkPipelineRasterizationStateCreateInfo rs = {};
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = VK_CULL_MODE_NONE;
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.lineWidth = 1.0f;
	VkPipelineMultisampleStateCreateInfo ms = {};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPipelineColorBlendAttachmentState cba = {};
	cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo cb = {};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments = &cba;
	VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynStates;

	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.setLayoutCount = 1;
	pl.pSetLayouts = &m_texDescLayout;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &m_texQuadPipelineLayout ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreatePipelineLayout (texquad) failed" );
		vkDestroyShaderModule( m_device, vert, nullptr );
		vkDestroyShaderModule( m_device, frag, nullptr );
		return false;
	}
	VkGraphicsPipelineCreateInfo gp = {};
	gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp.stageCount = 2;
	gp.pStages = stages;
	gp.pVertexInputState = &vin;
	gp.pInputAssemblyState = &ia;
	gp.pViewportState = &vp;
	gp.pRasterizationState = &rs;
	gp.pMultisampleState = &ms;
	gp.pColorBlendState = &cb;
	gp.pDynamicState = &dyn;
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_FALSE;
	ds.depthWriteEnable = VK_FALSE;
	gp.pDepthStencilState = &ds;
	gp.layout = m_texQuadPipelineLayout;
	gp.renderPass = m_renderPass;
	gp.subpass = 0;
	VkResult r =
	    vkCreateGraphicsPipelines( m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_texQuadPipeline );
	vkDestroyShaderModule( m_device, vert, nullptr );
	vkDestroyShaderModule( m_device, frag, nullptr );
	if ( r != VK_SUCCESS )
	{
		SetError( outError,
		    std::string( "vkCreateGraphicsPipelines (texquad) failed: " ) + ResultString( r ) );
		return false;
	}

	Log( "textured quad pipeline ready\n" );
	return true;
}

void CVulkanContext::DestroyTexturedQuad()
{
	if ( m_texQuadPipeline != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_texQuadPipeline, nullptr );
		m_texQuadPipeline = VK_NULL_HANDLE;
	}
	if ( m_texQuadPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( m_device, m_texQuadPipelineLayout, nullptr );
		m_texQuadPipelineLayout = VK_NULL_HANDLE;
	}
	if ( m_texQuadVertexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( m_device, m_texQuadVertexBuffer, nullptr );
		m_texQuadVertexBuffer = VK_NULL_HANDLE;
	}
	if ( m_texQuadVertexMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_texQuadVertexMemory, nullptr );
		m_texQuadVertexMemory = VK_NULL_HANDLE;
	}
	if ( m_texDescPool != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorPool( m_device, m_texDescPool, nullptr );
		m_texDescPool = VK_NULL_HANDLE;
	}
	m_texDescSet = VK_NULL_HANDLE;
	if ( m_texDescLayout != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorSetLayout( m_device, m_texDescLayout, nullptr );
		m_texDescLayout = VK_NULL_HANDLE;
	}
	if ( m_texSampler != VK_NULL_HANDLE )
	{
		vkDestroySampler( m_device, m_texSampler, nullptr );
		m_texSampler = VK_NULL_HANDLE;
	}
	if ( m_texView != VK_NULL_HANDLE )
	{
		vkDestroyImageView( m_device, m_texView, nullptr );
		m_texView = VK_NULL_HANDLE;
	}
	if ( m_texImage != VK_NULL_HANDLE )
	{
		vkDestroyImage( m_device, m_texImage, nullptr );
		m_texImage = VK_NULL_HANDLE;
	}
	if ( m_texMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_texMemory, nullptr );
		m_texMemory = VK_NULL_HANDLE;
	}
	m_texQuadVertexCount = 0;
	m_drawTexturedQuad = false;
}

void CVulkanContext::SetIndexedUboColor( float r, float g, float b, float a )
{
	m_iuColor[0] = r;
	m_iuColor[1] = g;
	m_iuColor[2] = b;
	m_iuColor[3] = a;
	if ( m_iuUniformMapped )
		std::memcpy( m_iuUniformMapped, m_iuColor, sizeof( m_iuColor ) );
}

bool CVulkanContext::InitIndexedUbo( std::string *outError )
{
	if ( !IsValid() || m_renderPass == VK_NULL_HANDLE )
	{
		SetError( outError, "InitIndexedUbo before a valid Init()" );
		return false;
	}
	if ( m_indexedUboPipeline != VK_NULL_HANDLE )
		return true;

	// A quad from 4 vertices + 6 indices, drawn with vkCmdDrawIndexed.
	const float verts[] = {
	    -0.8f,
	    -0.8f,
	    0.8f,
	    -0.8f,
	    0.8f,
	    0.8f,
	    -0.8f,
	    0.8f,
	};
	const uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };
	m_iuIndexCount = 6;

	if ( !CreateBuffer( sizeof( verts ), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &m_iuVertexBuffer, &m_iuVertexMemory, outError ) )
		return false;
	void *vmap = nullptr;
	vkMapMemory( m_device, m_iuVertexMemory, 0, sizeof( verts ), 0, &vmap );
	std::memcpy( vmap, verts, sizeof( verts ) );
	vkUnmapMemory( m_device, m_iuVertexMemory );

	if ( !CreateBuffer( sizeof( indices ), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &m_iuIndexBuffer, &m_iuIndexMemory, outError ) )
		return false;
	void *imap = nullptr;
	vkMapMemory( m_device, m_iuIndexMemory, 0, sizeof( indices ), 0, &imap );
	std::memcpy( imap, indices, sizeof( indices ) );
	vkUnmapMemory( m_device, m_iuIndexMemory );

	// Persistently-mapped uniform (constant) buffer holding the fragment color.
	if ( !CreateBuffer( sizeof( m_iuColor ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &m_iuUniformBuffer, &m_iuUniformMemory, outError ) )
		return false;
	if ( vkMapMemory( m_device, m_iuUniformMemory, 0, sizeof( m_iuColor ), 0,
	         &m_iuUniformMapped ) != VK_SUCCESS )
	{
		SetError( outError, "vkMapMemory (uniform) failed" );
		return false;
	}
	std::memcpy( m_iuUniformMapped, m_iuColor, sizeof( m_iuColor ) );

	// Descriptor set: binding 0 = uniform buffer, fragment stage.
	VkDescriptorSetLayoutBinding lb = {};
	lb.binding = 0;
	lb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lb.descriptorCount = 1;
	lb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo dl = {};
	dl.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dl.bindingCount = 1;
	dl.pBindings = &lb;
	if ( vkCreateDescriptorSetLayout( m_device, &dl, nullptr, &m_iuDescLayout ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateDescriptorSetLayout (ubo) failed" );
		return false;
	}
	VkDescriptorPoolSize ps = {};
	ps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ps.descriptorCount = 1;
	VkDescriptorPoolCreateInfo dp = {};
	dp.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dp.maxSets = 1;
	dp.poolSizeCount = 1;
	dp.pPoolSizes = &ps;
	if ( vkCreateDescriptorPool( m_device, &dp, nullptr, &m_iuDescPool ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateDescriptorPool (ubo) failed" );
		return false;
	}
	VkDescriptorSetAllocateInfo da = {};
	da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	da.descriptorPool = m_iuDescPool;
	da.descriptorSetCount = 1;
	da.pSetLayouts = &m_iuDescLayout;
	if ( vkAllocateDescriptorSets( m_device, &da, &m_iuDescSet ) != VK_SUCCESS )
	{
		SetError( outError, "vkAllocateDescriptorSets (ubo) failed" );
		return false;
	}
	VkDescriptorBufferInfo dbi = {};
	dbi.buffer = m_iuUniformBuffer;
	dbi.offset = 0;
	dbi.range = sizeof( m_iuColor );
	VkWriteDescriptorSet wds = {};
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.dstSet = m_iuDescSet;
	wds.dstBinding = 0;
	wds.descriptorCount = 1;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	wds.pBufferInfo = &dbi;
	vkUpdateDescriptorSets( m_device, 1, &wds, 0, nullptr );

	// Shaders + pipeline (vertex input: vec2 position only).
	VkShaderModule vert = VK_NULL_HANDLE, frag = VK_NULL_HANDLE;
	if ( !CreateShaderModule(
	         g_demoIndexedUboVertSpv, sizeof( g_demoIndexedUboVertSpv ), &vert, outError ) )
		return false;
	if ( !CreateShaderModule(
	         g_demoIndexedUboFragSpv, sizeof( g_demoIndexedUboFragSpv ), &frag, outError ) )
	{
		vkDestroyShaderModule( m_device, vert, nullptr );
		return false;
	}

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName = "main";

	VkVertexInputBindingDescription binding = {};
	binding.binding = 0;
	binding.stride = sizeof( float ) * 2;
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attr = {};
	attr.location = 0;
	attr.format = VK_FORMAT_R32G32_SFLOAT;
	attr.offset = 0;
	VkPipelineVertexInputStateCreateInfo vin = {};
	vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vin.vertexBindingDescriptionCount = 1;
	vin.pVertexBindingDescriptions = &binding;
	vin.vertexAttributeDescriptionCount = 1;
	vin.pVertexAttributeDescriptions = &attr;

	VkPipelineInputAssemblyStateCreateInfo ia = {};
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount = 1;
	VkPipelineRasterizationStateCreateInfo rs = {};
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = VK_CULL_MODE_NONE;
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.lineWidth = 1.0f;
	VkPipelineMultisampleStateCreateInfo ms = {};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPipelineColorBlendAttachmentState cba = {};
	cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo cb = {};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments = &cba;
	VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynStates;

	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.setLayoutCount = 1;
	pl.pSetLayouts = &m_iuDescLayout;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &m_iuPipelineLayout ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreatePipelineLayout (ubo) failed" );
		vkDestroyShaderModule( m_device, vert, nullptr );
		vkDestroyShaderModule( m_device, frag, nullptr );
		return false;
	}
	VkGraphicsPipelineCreateInfo gp = {};
	gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp.stageCount = 2;
	gp.pStages = stages;
	gp.pVertexInputState = &vin;
	gp.pInputAssemblyState = &ia;
	gp.pViewportState = &vp;
	gp.pRasterizationState = &rs;
	gp.pMultisampleState = &ms;
	gp.pColorBlendState = &cb;
	gp.pDynamicState = &dyn;
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_FALSE;
	ds.depthWriteEnable = VK_FALSE;
	gp.pDepthStencilState = &ds;
	gp.layout = m_iuPipelineLayout;
	gp.renderPass = m_renderPass;
	gp.subpass = 0;
	VkResult r = vkCreateGraphicsPipelines(
	    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_indexedUboPipeline );
	vkDestroyShaderModule( m_device, vert, nullptr );
	vkDestroyShaderModule( m_device, frag, nullptr );
	if ( r != VK_SUCCESS )
	{
		SetError( outError,
		    std::string( "vkCreateGraphicsPipelines (ubo) failed: " ) + ResultString( r ) );
		return false;
	}

	Log( "indexed + uniform-buffer pipeline ready\n" );
	return true;
}

void CVulkanContext::DestroyIndexedUbo()
{
	if ( m_indexedUboPipeline != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_indexedUboPipeline, nullptr );
		m_indexedUboPipeline = VK_NULL_HANDLE;
	}
	if ( m_iuPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( m_device, m_iuPipelineLayout, nullptr );
		m_iuPipelineLayout = VK_NULL_HANDLE;
	}
	if ( m_iuDescPool != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorPool( m_device, m_iuDescPool, nullptr );
		m_iuDescPool = VK_NULL_HANDLE;
	}
	m_iuDescSet = VK_NULL_HANDLE;
	if ( m_iuDescLayout != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorSetLayout( m_device, m_iuDescLayout, nullptr );
		m_iuDescLayout = VK_NULL_HANDLE;
	}
	if ( m_iuUniformMapped )
	{
		vkUnmapMemory( m_device, m_iuUniformMemory );
		m_iuUniformMapped = nullptr;
	}
	if ( m_iuUniformBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( m_device, m_iuUniformBuffer, nullptr );
		m_iuUniformBuffer = VK_NULL_HANDLE;
	}
	if ( m_iuUniformMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_iuUniformMemory, nullptr );
		m_iuUniformMemory = VK_NULL_HANDLE;
	}
	if ( m_iuIndexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( m_device, m_iuIndexBuffer, nullptr );
		m_iuIndexBuffer = VK_NULL_HANDLE;
	}
	if ( m_iuIndexMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_iuIndexMemory, nullptr );
		m_iuIndexMemory = VK_NULL_HANDLE;
	}
	if ( m_iuVertexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( m_device, m_iuVertexBuffer, nullptr );
		m_iuVertexBuffer = VK_NULL_HANDLE;
	}
	if ( m_iuVertexMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_iuVertexMemory, nullptr );
		m_iuVertexMemory = VK_NULL_HANDLE;
	}
	m_iuIndexCount = 0;
	m_drawIndexedUbo = false;
}

bool CVulkanContext::InitDemoDepth( std::string *outError )
{
	if ( !IsValid() || m_renderPass == VK_NULL_HANDLE )
	{
		SetError( outError, "InitDemoDepth before a valid Init()" );
		return false;
	}
	if ( m_demoDepthPipeline != VK_NULL_HANDLE )
		return true;

	// Two overlapping triangles at different depths. The NEAR (blue, z=0.2) one
	// is drawn FIRST, the FAR (green, z=0.8) one SECOND. With depth testing the
	// far triangle's center fragments fail against the near depth and are
	// discarded, so the center stays blue -- proving occlusion works regardless
	// of draw order (a no-depth path would show green, drawn last).
	struct DepthVertex
	{
		float pos[3];
		float color[3];
	};
	const DepthVertex verts[] = {
	    { { -0.7f, -0.7f, 0.2f }, { 0.0f, 0.0f, 1.0f } }, // near (blue)
	    { { 0.7f, -0.7f, 0.2f }, { 0.0f, 0.0f, 1.0f } },
	    { { 0.0f, 0.7f, 0.2f }, { 0.0f, 0.0f, 1.0f } },
	    { { -0.7f, 0.7f, 0.8f }, { 0.0f, 1.0f, 0.0f } }, // far (green)
	    { { 0.7f, 0.7f, 0.8f }, { 0.0f, 1.0f, 0.0f } },
	    { { 0.0f, -0.7f, 0.8f }, { 0.0f, 1.0f, 0.0f } },
	};
	m_depthDemoVertexCount = 6;

	if ( !CreateBuffer( sizeof( verts ), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &m_depthDemoVertexBuffer, &m_depthDemoVertexMemory, outError ) )
		return false;
	void *vmap = nullptr;
	vkMapMemory( m_device, m_depthDemoVertexMemory, 0, sizeof( verts ), 0, &vmap );
	std::memcpy( vmap, verts, sizeof( verts ) );
	vkUnmapMemory( m_device, m_depthDemoVertexMemory );

	VkShaderModule vert = VK_NULL_HANDLE, frag = VK_NULL_HANDLE;
	if ( !CreateShaderModule( g_demoDepthVertSpv, sizeof( g_demoDepthVertSpv ), &vert, outError ) )
		return false;
	if ( !CreateShaderModule(
	         g_demoTriangleFragSpv, sizeof( g_demoTriangleFragSpv ), &frag, outError ) )
	{
		vkDestroyShaderModule( m_device, vert, nullptr );
		return false;
	}

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName = "main";

	VkVertexInputBindingDescription binding = {};
	binding.binding = 0;
	binding.stride = sizeof( DepthVertex );
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attrs[2] = {};
	attrs[0].location = 0;
	attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[0].offset = 0;
	attrs[1].location = 1;
	attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[1].offset = sizeof( float ) * 3;
	VkPipelineVertexInputStateCreateInfo vin = {};
	vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vin.vertexBindingDescriptionCount = 1;
	vin.pVertexBindingDescriptions = &binding;
	vin.vertexAttributeDescriptionCount = 2;
	vin.pVertexAttributeDescriptions = attrs;

	VkPipelineInputAssemblyStateCreateInfo ia = {};
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount = 1;
	VkPipelineRasterizationStateCreateInfo rs = {};
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = VK_CULL_MODE_NONE;
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.lineWidth = 1.0f;
	VkPipelineMultisampleStateCreateInfo ms = {};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPipelineColorBlendAttachmentState cba = {};
	cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo cb = {};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments = &cba;
	VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynStates;

	// Depth testing ON: this is the whole point of the demo.
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_TRUE;
	ds.depthWriteEnable = VK_TRUE;
	ds.depthCompareOp = VK_COMPARE_OP_LESS;

	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &m_demoDepthPipelineLayout ) !=
	     VK_SUCCESS )
	{
		SetError( outError, "vkCreatePipelineLayout (depth) failed" );
		vkDestroyShaderModule( m_device, vert, nullptr );
		vkDestroyShaderModule( m_device, frag, nullptr );
		return false;
	}
	VkGraphicsPipelineCreateInfo gp = {};
	gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp.stageCount = 2;
	gp.pStages = stages;
	gp.pVertexInputState = &vin;
	gp.pInputAssemblyState = &ia;
	gp.pViewportState = &vp;
	gp.pRasterizationState = &rs;
	gp.pMultisampleState = &ms;
	gp.pColorBlendState = &cb;
	gp.pDynamicState = &dyn;
	gp.pDepthStencilState = &ds;
	gp.layout = m_demoDepthPipelineLayout;
	gp.renderPass = m_renderPass;
	gp.subpass = 0;
	VkResult r = vkCreateGraphicsPipelines(
	    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_demoDepthPipeline );
	vkDestroyShaderModule( m_device, vert, nullptr );
	vkDestroyShaderModule( m_device, frag, nullptr );
	if ( r != VK_SUCCESS )
	{
		SetError( outError,
		    std::string( "vkCreateGraphicsPipelines (depth) failed: " ) + ResultString( r ) );
		return false;
	}

	Log( "depth-test pipeline ready\n" );
	return true;
}

void CVulkanContext::DestroyDemoDepth()
{
	if ( m_demoDepthPipeline != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_demoDepthPipeline, nullptr );
		m_demoDepthPipeline = VK_NULL_HANDLE;
	}
	if ( m_demoDepthPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( m_device, m_demoDepthPipelineLayout, nullptr );
		m_demoDepthPipelineLayout = VK_NULL_HANDLE;
	}
	if ( m_depthDemoVertexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( m_device, m_depthDemoVertexBuffer, nullptr );
		m_depthDemoVertexBuffer = VK_NULL_HANDLE;
	}
	if ( m_depthDemoVertexMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_depthDemoVertexMemory, nullptr );
		m_depthDemoVertexMemory = VK_NULL_HANDLE;
	}
	m_depthDemoVertexCount = 0;
	m_drawDemoDepth = false;
}

bool CVulkanContext::InitDynamicMesh( std::string *outError )
{
	if ( !IsValid() || m_renderPass == VK_NULL_HANDLE )
	{
		SetError( outError, "InitDynamicMesh before a valid Init()" );
		return false;
	}
	if ( m_dynPipeline != VK_NULL_HANDLE )
		return true;

	// Position (vec3) + color (vec3), depth-tested -- the same vertex shape as
	// the depth demo, but fed from a growable dynamic vertex buffer. Two
	// fragment shaders form a bounded material-shader catalog selected at draw
	// (BeginPass): vertex-color passthrough and "greenify".
	VkShaderModule vert = VK_NULL_HANDLE, frag = VK_NULL_HANDLE, fragGreen = VK_NULL_HANDLE,
	               fragConst = VK_NULL_HANDLE;
	if ( !CreateShaderModule( g_demoDynVertSpv, sizeof( g_demoDynVertSpv ), &vert, outError ) )
		return false;
	if ( !CreateShaderModule(
	         g_demoTriangleFragSpv, sizeof( g_demoTriangleFragSpv ), &frag, outError ) )
	{
		vkDestroyShaderModule( m_device, vert, nullptr );
		return false;
	}
	if ( !CreateShaderModule(
	         g_demoGreenifyFragSpv, sizeof( g_demoGreenifyFragSpv ), &fragGreen, outError ) )
	{
		vkDestroyShaderModule( m_device, vert, nullptr );
		vkDestroyShaderModule( m_device, frag, nullptr );
		return false;
	}
	if ( !CreateShaderModule(
	         g_demoConstColorFragSpv, sizeof( g_demoConstColorFragSpv ), &fragConst, outError ) )
	{
		vkDestroyShaderModule( m_device, vert, nullptr );
		vkDestroyShaderModule( m_device, frag, nullptr );
		vkDestroyShaderModule( m_device, fragGreen, nullptr );
		return false;
	}

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName = "main";

	// Shared dynamic vertex: position (vec3) + color (vec3) + uv (vec2), 8 floats.
	// Shaders that ignore a component (e.g. color for the textured shader, uv for
	// the others) simply do not read that attribute.
	VkVertexInputBindingDescription binding = {};
	binding.binding = 0;
	binding.stride = sizeof( float ) * 8;
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attrs[3] = {};
	attrs[0].location = 0;
	attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[0].offset = 0;
	attrs[1].location = 1;
	attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[1].offset = sizeof( float ) * 3;
	attrs[2].location = 2;
	attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
	attrs[2].offset = sizeof( float ) * 6;
	VkPipelineVertexInputStateCreateInfo vin = {};
	vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vin.vertexBindingDescriptionCount = 1;
	vin.pVertexBindingDescriptions = &binding;
	vin.vertexAttributeDescriptionCount = 3;
	vin.pVertexAttributeDescriptions = attrs;

	VkPipelineInputAssemblyStateCreateInfo ia = {};
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount = 1;
	VkPipelineRasterizationStateCreateInfo rs = {};
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = VK_CULL_MODE_NONE;
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.lineWidth = 1.0f;
	VkPipelineMultisampleStateCreateInfo ms = {};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPipelineColorBlendAttachmentState cba = {};
	cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo cb = {};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments = &cba;
	VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynStates;
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_TRUE;
	ds.depthWriteEnable = VK_TRUE;
	ds.depthCompareOp = VK_COMPARE_OP_LESS;

	// One layout shared by all three material pipelines, carrying a push constant
	// block { mat4 mvp; vec4 color; }: the vertex stage reads the transform
	// (offset 0), the fragment stage reads the color (offset 64). Pipelines that
	// ignore parts of it are unaffected.
	VkPushConstantRange pcRange = {};
	pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pcRange.offset = 0;
	pcRange.size = sizeof( float ) * 20; // mat4 (16) + vec4 (4)
	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.pushConstantRangeCount = 1;
	pl.pPushConstantRanges = &pcRange;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &m_dynPipelineLayout ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreatePipelineLayout (dynamic mesh) failed" );
		vkDestroyShaderModule( m_device, vert, nullptr );
		vkDestroyShaderModule( m_device, frag, nullptr );
		vkDestroyShaderModule( m_device, fragGreen, nullptr );
		vkDestroyShaderModule( m_device, fragConst, nullptr );
		return false;
	}
	VkGraphicsPipelineCreateInfo gp = {};
	gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp.stageCount = 2;
	gp.pStages = stages;
	gp.pVertexInputState = &vin;
	gp.pInputAssemblyState = &ia;
	gp.pViewportState = &vp;
	gp.pRasterizationState = &rs;
	gp.pMultisampleState = &ms;
	gp.pColorBlendState = &cb;
	gp.pDynamicState = &dyn;
	gp.pDepthStencilState = &ds;
	gp.layout = m_dynPipelineLayout;
	gp.renderPass = m_renderPass;
	gp.subpass = 0;
	// Passthrough (vertex color) pipeline.
	VkResult r =
	    vkCreateGraphicsPipelines( m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_dynPipeline );
	// "greenify" material-shader pipeline: identical state, greenify fragment.
	if ( r == VK_SUCCESS )
	{
		stages[1].module = fragGreen;
		r = vkCreateGraphicsPipelines(
		    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_dynPipelineGreen );
	}
	// "constant color" material-shader pipeline: reads the push-constant color.
	if ( r == VK_SUCCESS )
	{
		stages[1].module = fragConst;
		r = vkCreateGraphicsPipelines(
		    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_dynPipelineConst );
	}
	vkDestroyShaderModule( m_device, vert, nullptr );
	vkDestroyShaderModule( m_device, frag, nullptr );
	vkDestroyShaderModule( m_device, fragGreen, nullptr );
	vkDestroyShaderModule( m_device, fragConst, nullptr );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateGraphicsPipelines (dynamic mesh) failed: " ) +
		                        ResultString( r ) );
		return false;
	}

	// --- "$basetexture" material pipeline: a built-in 2-tone texture (left half
	// red, right half green) sampled at the mesh UVs, so a textured material
	// renders through the mesh path with UV-correct sampling. ---
	{
		const uint32_t texW = 2, texH = 1;
		const uint8_t texels[texW * texH * 4] = {
		    255,
		    0,
		    0,
		    255, // texel (0,0): red
		    0,
		    255,
		    0,
		    255, // texel (1,0): green
		};
		VkImageCreateInfo ii = {};
		ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ii.imageType = VK_IMAGE_TYPE_2D;
		ii.format = VK_FORMAT_R8G8B8A8_UNORM;
		ii.extent = { texW, texH, 1 };
		ii.mipLevels = 1;
		ii.arrayLayers = 1;
		ii.samples = VK_SAMPLE_COUNT_1_BIT;
		ii.tiling = VK_IMAGE_TILING_OPTIMAL;
		ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if ( vkCreateImage( m_device, &ii, nullptr, &m_dynTexImage ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateImage (dynamic texture) failed" );
			return false;
		}
		VkMemoryRequirements req = {};
		vkGetImageMemoryRequirements( m_device, m_dynTexImage, &req );
		bool found = false;
		uint32_t type =
		    FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &found );
		if ( !found )
		{
			SetError( outError, "no device-local memory for dynamic texture" );
			return false;
		}
		VkMemoryAllocateInfo ai = {};
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = req.size;
		ai.memoryTypeIndex = type;
		if ( vkAllocateMemory( m_device, &ai, nullptr, &m_dynTexMemory ) != VK_SUCCESS )
		{
			SetError( outError, "vkAllocateMemory (dynamic texture) failed" );
			return false;
		}
		vkBindImageMemory( m_device, m_dynTexImage, m_dynTexMemory, 0 );

		VkBuffer staging = VK_NULL_HANDLE;
		VkDeviceMemory stagingMem = VK_NULL_HANDLE;
		if ( !CreateBuffer( sizeof( texels ), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		         &staging, &stagingMem, outError ) )
			return false;
		void *mapped = nullptr;
		vkMapMemory( m_device, stagingMem, 0, sizeof( texels ), 0, &mapped );
		std::memcpy( mapped, texels, sizeof( texels ) );
		vkUnmapMemory( m_device, stagingMem );

		VkCommandBuffer cmd = VK_NULL_HANDLE;
		if ( !BeginSingleTimeCommands( &cmd, outError ) )
		{
			vkDestroyBuffer( m_device, staging, nullptr );
			vkFreeMemory( m_device, stagingMem, nullptr );
			return false;
		}
		VkImageMemoryBarrier toDst = {};
		toDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		toDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		toDst.image = m_dynTexImage;
		toDst.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toDst );
		VkBufferImageCopy copy = {};
		copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy.imageExtent = { texW, texH, 1 };
		vkCmdCopyBufferToImage(
		    cmd, staging, m_dynTexImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy );
		VkImageMemoryBarrier toRead = toDst;
		toRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		toRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		toRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toRead );
		if ( !EndSingleTimeCommands( cmd, outError ) )
		{
			vkDestroyBuffer( m_device, staging, nullptr );
			vkFreeMemory( m_device, stagingMem, nullptr );
			return false;
		}
		vkDestroyBuffer( m_device, staging, nullptr );
		vkFreeMemory( m_device, stagingMem, nullptr );

		VkImageViewCreateInfo iv = {};
		iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv.image = m_dynTexImage;
		iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv.format = VK_FORMAT_R8G8B8A8_UNORM;
		iv.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		if ( vkCreateImageView( m_device, &iv, nullptr, &m_dynTexView ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateImageView (dynamic texture) failed" );
			return false;
		}
		VkSamplerCreateInfo sc = {};
		sc.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sc.magFilter = VK_FILTER_NEAREST;
		sc.minFilter = VK_FILTER_NEAREST;
		sc.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sc.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sc.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		if ( vkCreateSampler( m_device, &sc, nullptr, &m_dynTexSampler ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateSampler (dynamic texture) failed" );
			return false;
		}

		VkDescriptorSetLayoutBinding lb = {};
		lb.binding = 0;
		lb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		lb.descriptorCount = 1;
		lb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		VkDescriptorSetLayoutCreateInfo dl = {};
		dl.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dl.bindingCount = 1;
		dl.pBindings = &lb;
		if ( vkCreateDescriptorSetLayout( m_device, &dl, nullptr, &m_dynTexDescLayout ) !=
		     VK_SUCCESS )
		{
			SetError( outError, "vkCreateDescriptorSetLayout (dynamic texture) failed" );
			return false;
		}
		// One combined-image-sampler set for the built-in texture plus one per
		// material-supplied managed texture, so each draw can sample its own texture
		// (per-draw binding). Sized for a map's texture set.
		VkDescriptorPoolSize ps = {};
		ps.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		ps.descriptorCount = kMaxManagedTexSets;
		VkDescriptorPoolCreateInfo dp = {};
		dp.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dp.maxSets = kMaxManagedTexSets;
		dp.poolSizeCount = 1;
		dp.pPoolSizes = &ps;
		if ( vkCreateDescriptorPool( m_device, &dp, nullptr, &m_dynTexDescPool ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateDescriptorPool (dynamic texture) failed" );
			return false;
		}
		VkDescriptorSetAllocateInfo da = {};
		da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		da.descriptorPool = m_dynTexDescPool;
		da.descriptorSetCount = 1;
		da.pSetLayouts = &m_dynTexDescLayout;
		if ( vkAllocateDescriptorSets( m_device, &da, &m_dynTexDescSet ) != VK_SUCCESS )
		{
			SetError( outError, "vkAllocateDescriptorSets (dynamic texture) failed" );
			return false;
		}
		VkDescriptorImageInfo dii = {};
		dii.sampler = m_dynTexSampler;
		dii.imageView = m_dynTexView;
		dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = m_dynTexDescSet;
		wds.dstBinding = 0;
		wds.descriptorCount = 1;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wds.pImageInfo = &dii;
		vkUpdateDescriptorSets( m_device, 1, &wds, 0, nullptr );

		// The UnlitGeneric push block is larger than the color pipelines': it
		// carries cModelViewProj (mat4), cModulationColor (vec4), and the two rows
		// of cBaseTextureTransform (2x vec4) -- 28 floats / 112 bytes, within the
		// 128-byte guaranteed push-constant minimum.
		VkPushConstantRange texPc = {};
		texPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		texPc.offset = 0;
		texPc.size = sizeof( float ) * 32; // + vec4 alphaParams (128 bytes)
		VkPipelineLayoutCreateInfo texPl = {};
		texPl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		texPl.setLayoutCount = 1;
		texPl.pSetLayouts = &m_dynTexDescLayout;
		texPl.pushConstantRangeCount = 1;
		texPl.pPushConstantRanges = &texPc;
		if ( vkCreatePipelineLayout( m_device, &texPl, nullptr, &m_dynTexPipelineLayout ) !=
		     VK_SUCCESS )
		{
			SetError( outError, "vkCreatePipelineLayout (dynamic texture) failed" );
			return false;
		}

		VkShaderModule texVert = VK_NULL_HANDLE, texFrag = VK_NULL_HANDLE;
		if ( !CreateShaderModule(
		         g_demoDynTexVertSpv, sizeof( g_demoDynTexVertSpv ), &texVert, outError ) )
			return false;
		if ( !CreateShaderModule(
		         g_demoDynTexFragSpv, sizeof( g_demoDynTexFragSpv ), &texFrag, outError ) )
		{
			vkDestroyShaderModule( m_device, texVert, nullptr );
			return false;
		}
		VkPipelineShaderStageCreateInfo texStages[2] = { stages[0], stages[1] };
		texStages[0].module = texVert;
		texStages[1].module = texFrag;
		gp.pStages = texStages;
		gp.layout = m_dynTexPipelineLayout;
		r = vkCreateGraphicsPipelines(
		    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_dynPipelineTex );

		// Blend variants of the textured (UnlitGeneric) pipeline. Same shaders and
		// layout; only the color-blend attachment and depth-write change, matching
		// the D3D9 compositing a material selects via IShaderShadow:
		//   $translucent: src.a*src + (1-src.a)*dst, depth write off
		//   $additive:    src + dst,                 depth write off
		// Blended geometry does not write depth (Source draws it after opaque).
		if ( r == VK_SUCCESS )
		{
			VkPipelineColorBlendAttachmentState alphaAtt = cba;
			alphaAtt.blendEnable = VK_TRUE;
			alphaAtt.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			alphaAtt.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			alphaAtt.colorBlendOp = VK_BLEND_OP_ADD;
			alphaAtt.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			alphaAtt.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			alphaAtt.alphaBlendOp = VK_BLEND_OP_ADD;
			VkPipelineColorBlendStateCreateInfo cbAlpha = cb;
			cbAlpha.pAttachments = &alphaAtt;

			VkPipelineColorBlendAttachmentState addAtt = cba;
			addAtt.blendEnable = VK_TRUE;
			addAtt.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			addAtt.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			addAtt.colorBlendOp = VK_BLEND_OP_ADD;
			addAtt.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			addAtt.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			addAtt.alphaBlendOp = VK_BLEND_OP_ADD;
			VkPipelineColorBlendStateCreateInfo cbAdd = cb;
			cbAdd.pAttachments = &addAtt;

			VkPipelineDepthStencilStateCreateInfo dsBlend = ds;
			dsBlend.depthWriteEnable = VK_FALSE;

			gp.pColorBlendState = &cbAlpha;
			gp.pDepthStencilState = &dsBlend;
			r = vkCreateGraphicsPipelines(
			    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_dynPipelineTexAlpha );
			if ( r == VK_SUCCESS )
			{
				gp.pColorBlendState = &cbAdd;
				r = vkCreateGraphicsPipelines(
				    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_dynPipelineTexAdd );
			}
			// Restore the opaque state on the shared struct for hygiene.
			gp.pColorBlendState = &cb;
			gp.pDepthStencilState = &ds;
		}

		vkDestroyShaderModule( m_device, texVert, nullptr );
		vkDestroyShaderModule( m_device, texFrag, nullptr );
		if ( r != VK_SUCCESS )
		{
			SetError(
			    outError, std::string( "vkCreateGraphicsPipelines (dynamic texture) failed: " ) +
			                  ResultString( r ) );
			return false;
		}
	}

	Log( "dynamic mesh pipelines ready (4 material shaders incl. textured + blend variants)\n" );
	return true;
}

void CVulkanContext::SetDynamicTransform( const float *m16 )
{
	if ( m16 )
		std::memcpy( m_dynTransform, m16, sizeof( m_dynTransform ) );
}

int CVulkanContext::CreateManagedTexture(
    int width, int height, VkFormat format, std::string *outError )
{
	if ( !IsValid() || width <= 0 || height <= 0 )
	{
		SetError( outError, "CreateManagedTexture with invalid size or context" );
		return -1;
	}
	// Require the device to support sampling this format (block-compressed BC1/BC3
	// are supported on desktop GPUs; fail loudly rather than create an unusable
	// image).
	VkFormatProperties fp = {};
	vkGetPhysicalDeviceFormatProperties( m_physicalDevice, format, &fp );
	if ( !( fp.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT ) )
	{
		SetError( outError, "requested texture format is not sampleable on this device" );
		return -1;
	}

	ManagedTexture t;
	t.width = static_cast<uint32_t>( width );
	t.height = static_cast<uint32_t>( height );
	t.format = format;

	VkImageCreateInfo ii = {};
	ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ii.imageType = VK_IMAGE_TYPE_2D;
	ii.format = format;
	ii.extent = { t.width, t.height, 1 };
	ii.mipLevels = 1;
	ii.arrayLayers = 1;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if ( vkCreateImage( m_device, &ii, nullptr, &t.image ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateImage (managed texture) failed" );
		return -1;
	}
	VkMemoryRequirements req = {};
	vkGetImageMemoryRequirements( m_device, t.image, &req );
	bool found = false;
	uint32_t type =
	    FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &found );
	if ( !found )
	{
		vkDestroyImage( m_device, t.image, nullptr );
		SetError( outError, "no device-local memory for managed texture" );
		return -1;
	}
	VkMemoryAllocateInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = req.size;
	ai.memoryTypeIndex = type;
	if ( vkAllocateMemory( m_device, &ai, nullptr, &t.memory ) != VK_SUCCESS )
	{
		vkDestroyImage( m_device, t.image, nullptr );
		SetError( outError, "vkAllocateMemory (managed texture) failed" );
		return -1;
	}
	vkBindImageMemory( m_device, t.image, t.memory, 0 );

	VkImageViewCreateInfo iv = {};
	iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	iv.image = t.image;
	iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
	iv.format = format;
	iv.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	if ( vkCreateImageView( m_device, &iv, nullptr, &t.view ) != VK_SUCCESS )
	{
		vkFreeMemory( m_device, t.memory, nullptr );
		vkDestroyImage( m_device, t.image, nullptr );
		SetError( outError, "vkCreateImageView (managed texture) failed" );
		return -1;
	}

	// Allocate this texture's own descriptor set (if the pool/layout are ready and
	// have room), so per-draw texture binding can point each draw at its texture.
	if ( m_dynTexDescLayout != VK_NULL_HANDLE && m_dynTexDescPool != VK_NULL_HANDLE &&
	     m_managedTextures.size() + 1 < kMaxManagedTexSets )
	{
		VkDescriptorSetAllocateInfo da = {};
		da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		da.descriptorPool = m_dynTexDescPool;
		da.descriptorSetCount = 1;
		da.pSetLayouts = &m_dynTexDescLayout;
		if ( vkAllocateDescriptorSets( m_device, &da, &t.descSet ) == VK_SUCCESS )
		{
			VkDescriptorImageInfo dii = {};
			dii.sampler = m_dynTexSampler;
			dii.imageView = t.view;
			dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkWriteDescriptorSet wds = {};
			wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			wds.dstSet = t.descSet;
			wds.dstBinding = 0;
			wds.descriptorCount = 1;
			wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			wds.pImageInfo = &dii;
			vkUpdateDescriptorSets( m_device, 1, &wds, 0, nullptr );
		}
	}

	m_managedTextures.push_back( t );
	return static_cast<int>( m_managedTextures.size() - 1 );
}

bool CVulkanContext::UploadManagedTexture(
    int handle, const uint8_t *data, size_t dataSize, std::string *outError )
{
	if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) || !data ||
	     dataSize == 0 )
	{
		SetError( outError, "UploadManagedTexture with invalid handle/data" );
		return false;
	}
	ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
	const VkDeviceSize bytes = dataSize;

	VkBuffer staging = VK_NULL_HANDLE;
	VkDeviceMemory stagingMem = VK_NULL_HANDLE;
	if ( !CreateBuffer( bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging,
	         &stagingMem, outError ) )
		return false;
	void *mapped = nullptr;
	vkMapMemory( m_device, stagingMem, 0, bytes, 0, &mapped );
	std::memcpy( mapped, data, bytes );
	vkUnmapMemory( m_device, stagingMem );

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( !BeginSingleTimeCommands( &cmd, outError ) )
	{
		vkDestroyBuffer( m_device, staging, nullptr );
		vkFreeMemory( m_device, stagingMem, nullptr );
		return false;
	}
	VkImageMemoryBarrier toDst = {};
	toDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.image = t.image;
	toDst.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
	    0, nullptr, 0, nullptr, 1, &toDst );
	VkBufferImageCopy copy = {};
	copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.imageExtent = { t.width, t.height, 1 };
	vkCmdCopyBufferToImage( cmd, staging, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy );
	VkImageMemoryBarrier toRead = toDst;
	toRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	toRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	toRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toRead );
	const bool ok = EndSingleTimeCommands( cmd, outError );
	vkDestroyBuffer( m_device, staging, nullptr );
	vkFreeMemory( m_device, stagingMem, nullptr );
	return ok;
}

void CVulkanContext::BindManagedTexture( int handle )
{
	// Just record which managed texture is bound; each queued draw captures this
	// handle (QueueDynamicTriangles) and binds that texture's own descriptor set at
	// replay, so different objects in a frame sample their own textures. A handle
	// with no allocated descriptor set (or < 0) falls back to the built-in texture.
	m_dynBoundTexHandle = handle;
}

void CVulkanContext::QueueDynamicTriangles( const float *posColorInterleaved, uint32_t vertexCount )
{
	if ( !posColorInterleaved || vertexCount == 0 )
		return;
	// Record this draw with the state current right now (the engine sets the
	// transform/shader/constant per object before each Draw). 8 floats/vertex:
	// position (3) + color (3) + uv (2).
	DynDraw d;
	d.firstVertex = static_cast<uint32_t>( m_dynQueued.size() / 8 );
	d.vertexCount = vertexCount;
	d.shaderIndex = m_dynShaderIndex;
	std::memcpy( d.transform, m_dynTransform, sizeof( d.transform ) );
	std::memcpy( d.color, m_dynConstColor, sizeof( d.color ) );
	std::memcpy( d.modulation, m_dynModulation, sizeof( d.modulation ) );
	std::memcpy( d.texXform0, m_dynTexXform0, sizeof( d.texXform0 ) );
	std::memcpy( d.texXform1, m_dynTexXform1, sizeof( d.texXform1 ) );
	d.blendMode = m_dynBlendMode;
	d.alphaRef = m_dynAlphaRef;
	d.texHandle = m_dynBoundTexHandle;
	m_dynDrawRecords.push_back( d );
	m_dynQueued.insert( m_dynQueued.end(), posColorInterleaved,
	    posColorInterleaved + static_cast<size_t>( vertexCount ) * 8 );
}

void CVulkanContext::DestroyDynamicMesh()
{
	if ( m_dynPipeline != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_dynPipeline, nullptr );
		m_dynPipeline = VK_NULL_HANDLE;
	}
	if ( m_dynPipelineGreen != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_dynPipelineGreen, nullptr );
		m_dynPipelineGreen = VK_NULL_HANDLE;
	}
	if ( m_dynPipelineConst != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_dynPipelineConst, nullptr );
		m_dynPipelineConst = VK_NULL_HANDLE;
	}
	if ( m_dynPipelineTex != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_dynPipelineTex, nullptr );
		m_dynPipelineTex = VK_NULL_HANDLE;
	}
	if ( m_dynPipelineTexAlpha != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_dynPipelineTexAlpha, nullptr );
		m_dynPipelineTexAlpha = VK_NULL_HANDLE;
	}
	if ( m_dynPipelineTexAdd != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( m_device, m_dynPipelineTexAdd, nullptr );
		m_dynPipelineTexAdd = VK_NULL_HANDLE;
	}
	if ( m_dynTexPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( m_device, m_dynTexPipelineLayout, nullptr );
		m_dynTexPipelineLayout = VK_NULL_HANDLE;
	}
	if ( m_dynTexDescPool != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorPool( m_device, m_dynTexDescPool, nullptr );
		m_dynTexDescPool = VK_NULL_HANDLE;
	}
	m_dynTexDescSet = VK_NULL_HANDLE;
	if ( m_dynTexDescLayout != VK_NULL_HANDLE )
	{
		vkDestroyDescriptorSetLayout( m_device, m_dynTexDescLayout, nullptr );
		m_dynTexDescLayout = VK_NULL_HANDLE;
	}
	if ( m_dynTexSampler != VK_NULL_HANDLE )
	{
		vkDestroySampler( m_device, m_dynTexSampler, nullptr );
		m_dynTexSampler = VK_NULL_HANDLE;
	}
	if ( m_dynTexView != VK_NULL_HANDLE )
	{
		vkDestroyImageView( m_device, m_dynTexView, nullptr );
		m_dynTexView = VK_NULL_HANDLE;
	}
	if ( m_dynTexImage != VK_NULL_HANDLE )
	{
		vkDestroyImage( m_device, m_dynTexImage, nullptr );
		m_dynTexImage = VK_NULL_HANDLE;
	}
	if ( m_dynTexMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_dynTexMemory, nullptr );
		m_dynTexMemory = VK_NULL_HANDLE;
	}
	for ( ManagedTexture &t : m_managedTextures )
	{
		if ( t.view != VK_NULL_HANDLE )
			vkDestroyImageView( m_device, t.view, nullptr );
		if ( t.image != VK_NULL_HANDLE )
			vkDestroyImage( m_device, t.image, nullptr );
		if ( t.memory != VK_NULL_HANDLE )
			vkFreeMemory( m_device, t.memory, nullptr );
	}
	m_managedTextures.clear();
	if ( m_dynPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( m_device, m_dynPipelineLayout, nullptr );
		m_dynPipelineLayout = VK_NULL_HANDLE;
	}
	if ( m_dynMapped )
	{
		vkUnmapMemory( m_device, m_dynVertexMemory );
		m_dynMapped = nullptr;
	}
	if ( m_dynVertexBuffer != VK_NULL_HANDLE )
	{
		vkDestroyBuffer( m_device, m_dynVertexBuffer, nullptr );
		m_dynVertexBuffer = VK_NULL_HANDLE;
	}
	if ( m_dynVertexMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_dynVertexMemory, nullptr );
		m_dynVertexMemory = VK_NULL_HANDLE;
	}
	m_dynCapacityBytes = 0;
	m_dynQueued.clear();
}

bool CVulkanContext::BeginFrame( bool *outSkip, std::string *outError )
{
	if ( outSkip )
		*outSkip = false;
	if ( !IsValid() )
	{
		SetError( outError, "BeginFrame on an invalid context" );
		return false;
	}
	if ( m_frameOpen )
	{
		SetError( outError, "BeginFrame called while a frame is already open" );
		return false;
	}
	if ( m_swapchain == VK_NULL_HANDLE )
	{
		// Zero-size window: nothing to render this iteration.
		if ( outSkip )
			*outSkip = true;
		return true;
	}

	vkWaitForFences( m_device, 1, &m_inFlight[m_currentFrame], VK_TRUE, UINT64_MAX );

	uint32_t imageIndex = 0;
	VkResult r = vkAcquireNextImageKHR( m_device, m_swapchain, UINT64_MAX,
	    m_imageAvailable[m_currentFrame], VK_NULL_HANDLE, &imageIndex );
	if ( r == VK_ERROR_OUT_OF_DATE_KHR )
	{
		if ( !RecreateSwapchain( outError ) )
			return false;
		if ( outSkip )
			*outSkip = true;
		return true;
	}
	if ( r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR )
	{
		SetError( outError, std::string( "vkAcquireNextImageKHR failed: " ) + ResultString( r ) );
		return false;
	}

	// If a previous frame is still using this image, wait on its fence.
	if ( m_imagesInFlight[imageIndex] != VK_NULL_HANDLE )
		vkWaitForFences( m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX );
	m_imagesInFlight[imageIndex] = m_inFlight[m_currentFrame];

	m_acquiredImage = imageIndex;

	VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];
	vkResetCommandBuffer( cmd, 0 );

	VkCommandBufferBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	r = vkBeginCommandBuffer( cmd, &begin );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkBeginCommandBuffer failed: " ) + ResultString( r ) );
		return false;
	}

	VkClearValue clears[2] = {};
	clears[0].color = m_clearColor;
	clears[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo rp = {};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp.renderPass = m_renderPass;
	rp.framebuffer = m_framebuffers[imageIndex];
	rp.renderArea.offset = { 0, 0 };
	rp.renderArea.extent = m_swapExtent;
	rp.clearValueCount = 2;
	rp.pClearValues = clears;
	vkCmdBeginRenderPass( cmd, &rp, VK_SUBPASS_CONTENTS_INLINE );

	if ( m_drawDemoTriangle && m_demoPipeline != VK_NULL_HANDLE )
	{
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>( m_swapExtent.width );
		viewport.height = static_cast<float>( m_swapExtent.height );
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapExtent;
		vkCmdSetViewport( cmd, 0, 1, &viewport );
		vkCmdSetScissor( cmd, 0, 1, &scissor );

		vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_demoPipeline );
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers( cmd, 0, 1, &m_demoVertexBuffer, &offset );
		vkCmdDraw( cmd, m_demoVertexCount, 1, 0, 0 );
	}

	if ( m_drawTexturedQuad && m_texQuadPipeline != VK_NULL_HANDLE )
	{
		VkViewport viewport = {};
		viewport.width = static_cast<float>( m_swapExtent.width );
		viewport.height = static_cast<float>( m_swapExtent.height );
		viewport.maxDepth = 1.0f;
		VkRect2D scissor = {};
		scissor.extent = m_swapExtent;
		vkCmdSetViewport( cmd, 0, 1, &viewport );
		vkCmdSetScissor( cmd, 0, 1, &scissor );

		vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_texQuadPipeline );
		vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_texQuadPipelineLayout, 0,
		    1, &m_texDescSet, 0, nullptr );
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers( cmd, 0, 1, &m_texQuadVertexBuffer, &offset );
		vkCmdDraw( cmd, m_texQuadVertexCount, 1, 0, 0 );
	}

	if ( m_drawIndexedUbo && m_indexedUboPipeline != VK_NULL_HANDLE )
	{
		VkViewport viewport = {};
		viewport.width = static_cast<float>( m_swapExtent.width );
		viewport.height = static_cast<float>( m_swapExtent.height );
		viewport.maxDepth = 1.0f;
		VkRect2D scissor = {};
		scissor.extent = m_swapExtent;
		vkCmdSetViewport( cmd, 0, 1, &viewport );
		vkCmdSetScissor( cmd, 0, 1, &scissor );

		vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedUboPipeline );
		vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_iuPipelineLayout, 0, 1,
		    &m_iuDescSet, 0, nullptr );
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers( cmd, 0, 1, &m_iuVertexBuffer, &offset );
		vkCmdBindIndexBuffer( cmd, m_iuIndexBuffer, 0, VK_INDEX_TYPE_UINT16 );
		vkCmdDrawIndexed( cmd, m_iuIndexCount, 1, 0, 0, 0 );
	}

	if ( m_drawDemoDepth && m_demoDepthPipeline != VK_NULL_HANDLE )
	{
		VkViewport viewport = {};
		viewport.width = static_cast<float>( m_swapExtent.width );
		viewport.height = static_cast<float>( m_swapExtent.height );
		viewport.maxDepth = 1.0f;
		VkRect2D scissor = {};
		scissor.extent = m_swapExtent;
		vkCmdSetViewport( cmd, 0, 1, &viewport );
		vkCmdSetScissor( cmd, 0, 1, &scissor );

		vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_demoDepthPipeline );
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers( cmd, 0, 1, &m_depthDemoVertexBuffer, &offset );
		vkCmdDraw( cmd, m_depthDemoVertexCount, 1, 0, 0 );
	}

	// Dynamic geometry queued by the material system's mesh interface this frame.
	if ( m_dynPipeline != VK_NULL_HANDLE && !m_dynQueued.empty() )
	{
		const VkDeviceSize needed = m_dynQueued.size() * sizeof( float );
		bool bufferOk = true;
		if ( needed > m_dynCapacityBytes )
		{
			// Grow the persistently-mapped host-visible vertex buffer.
			if ( m_dynMapped )
			{
				vkUnmapMemory( m_device, m_dynVertexMemory );
				m_dynMapped = nullptr;
			}
			if ( m_dynVertexBuffer != VK_NULL_HANDLE )
				vkDestroyBuffer( m_device, m_dynVertexBuffer, nullptr );
			if ( m_dynVertexMemory != VK_NULL_HANDLE )
				vkFreeMemory( m_device, m_dynVertexMemory, nullptr );
			m_dynVertexBuffer = VK_NULL_HANDLE;
			m_dynVertexMemory = VK_NULL_HANDLE;

			VkDeviceSize cap = needed + needed / 2 + 4096;
			std::string bufErr;
			if ( CreateBuffer( cap, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			         &m_dynVertexBuffer, &m_dynVertexMemory, &bufErr ) &&
			     vkMapMemory( m_device, m_dynVertexMemory, 0, cap, 0, &m_dynMapped ) == VK_SUCCESS )
			{
				m_dynCapacityBytes = cap;
			}
			else
			{
				bufferOk = false;
				m_dynCapacityBytes = 0;
			}
		}

		if ( bufferOk && m_dynMapped )
		{
			std::memcpy( m_dynMapped, m_dynQueued.data(), needed );
			VkViewport viewport = {};
			viewport.width = static_cast<float>( m_swapExtent.width );
			viewport.height = static_cast<float>( m_swapExtent.height );
			viewport.maxDepth = 1.0f;
			VkRect2D scissor = {};
			scissor.extent = m_swapExtent;
			vkCmdSetViewport( cmd, 0, 1, &viewport );
			vkCmdSetScissor( cmd, 0, 1, &scissor );
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers( cmd, 0, 1, &m_dynVertexBuffer, &offset );

			// Replay each recorded draw with the state captured at that draw, so
			// the many objects in the frame each render with their own transform
			// and shader rather than all collapsing to the last-set state.
			VkPipeline boundPipeline = VK_NULL_HANDLE;
			for ( const DynDraw &d : m_dynDrawRecords )
			{
				VkPipeline selected = m_dynPipeline;
				VkPipelineLayout selectedLayout = m_dynPipelineLayout;
				bool textured = false;
				if ( d.shaderIndex == kDynShaderGreenify && m_dynPipelineGreen != VK_NULL_HANDLE )
					selected = m_dynPipelineGreen;
				else if ( d.shaderIndex == kDynShaderConstColor &&
				          m_dynPipelineConst != VK_NULL_HANDLE )
					selected = m_dynPipelineConst;
				else if ( d.shaderIndex == kDynShaderTextured &&
				          m_dynPipelineTex != VK_NULL_HANDLE )
				{
					// Pick the textured pipeline variant matching this draw's blend
					// mode (opaque / $translucent alpha / $additive).
					selected = m_dynPipelineTex;
					if ( d.blendMode == kDynBlendAlpha && m_dynPipelineTexAlpha != VK_NULL_HANDLE )
						selected = m_dynPipelineTexAlpha;
					else if ( d.blendMode == kDynBlendAdditive &&
					          m_dynPipelineTexAdd != VK_NULL_HANDLE )
						selected = m_dynPipelineTexAdd;
					selectedLayout = m_dynTexPipelineLayout;
					textured = true;
				}
				if ( selected != boundPipeline )
				{
					vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, selected );
					boundPipeline = selected;
				}
				if ( textured )
				{
					// Bind this draw's own texture descriptor set (per-draw texture),
					// falling back to the built-in set when the draw has no managed
					// texture or its set was not allocated.
					VkDescriptorSet set = m_dynTexDescSet;
					if ( d.texHandle >= 0 &&
					     d.texHandle < static_cast<int>( m_managedTextures.size() ) &&
					     m_managedTextures[static_cast<size_t>( d.texHandle )].descSet !=
					         VK_NULL_HANDLE )
						set = m_managedTextures[static_cast<size_t>( d.texHandle )].descSet;
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_dynTexPipelineLayout, 0, 1, &set, 0, nullptr );
				}
				// Push this draw's state. The textured (UnlitGeneric) pipeline reads
				// the faithful Source block { mat4 cModelViewProj; vec4
				// cModulationColor; vec4 cBaseTextureTransform[0]; [1] } (28 floats);
				// the color pipelines read { mat4 mvp; vec4 color } (20 floats).
				float pushData[32];
				std::memcpy( pushData, d.transform, sizeof( d.transform ) );
				uint32_t pushFloats;
				if ( textured )
				{
					std::memcpy( pushData + 16, d.modulation, sizeof( d.modulation ) );
					std::memcpy( pushData + 20, d.texXform0, sizeof( d.texXform0 ) );
					std::memcpy( pushData + 24, d.texXform1, sizeof( d.texXform1 ) );
					pushData[28] = d.alphaRef; // alphaParams.x
					pushData[29] = 0.0f;
					pushData[30] = 0.0f;
					pushData[31] = 0.0f;
					pushFloats = 32;
				}
				else
				{
					std::memcpy( pushData + 16, d.color, sizeof( d.color ) );
					pushFloats = 20;
				}
				vkCmdPushConstants( cmd, selectedLayout,
				    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
				    static_cast<uint32_t>( sizeof( float ) ) * pushFloats, pushData );
				vkCmdDraw( cmd, d.vertexCount, 1, d.firstVertex, 0 );
			}
		}
	}
	// NOTE: the queue is NOT cleared here. It is cleared at frame start
	// (ClearBuffers) so the last frame's geometry stays available for an
	// on-demand screenshot capture (IShaderAPI::ReadPixels re-renders it).

	m_frameOpen = true;
	return true;
}

bool CVulkanContext::RecordCapture( VkCommandBuffer cmd, uint32_t imageIndex )
{
	// Copy the just-rendered color image (currently COLOR_ATTACHMENT_OPTIMAL)
	// into the host-visible linear capture image.
	VkImageMemoryBarrier toSrc = {};
	toSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toSrc.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	toSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	toSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toSrc.image = m_swapImages[imageIndex];
	toSrc.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	toSrc.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	VkImageMemoryBarrier capDst = {};
	capDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	capDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	capDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	capDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	capDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	capDst.image = m_captureImage;
	capDst.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	capDst.srcAccessMask = 0;
	capDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	VkImageMemoryBarrier pre[] = { toSrc, capDst };
	vkCmdPipelineBarrier( cmd,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, pre );

	VkImageCopy copy = {};
	copy.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.extent = { m_swapExtent.width, m_swapExtent.height, 1 };
	vkCmdCopyImage( cmd, m_swapImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	    m_captureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy );

	// Capture image -> GENERAL for host read; swap image -> PRESENT_SRC.
	VkImageMemoryBarrier capGeneral = {};
	capGeneral.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	capGeneral.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	capGeneral.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	capGeneral.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	capGeneral.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	capGeneral.image = m_captureImage;
	capGeneral.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	capGeneral.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	capGeneral.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

	VkImageMemoryBarrier toPresent = {};
	toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toPresent.image = m_swapImages[imageIndex];
	toPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	toPresent.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	toPresent.dstAccessMask = 0;

	VkImageMemoryBarrier post[] = { capGeneral, toPresent };
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
	    nullptr, 2, post );
	return true;
}

bool CVulkanContext::EndFrame( std::string *outError )
{
	if ( !m_frameOpen )
	{
		SetError( outError, "EndFrame called without an open frame" );
		return false;
	}

	VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];
	uint32_t imageIndex = m_acquiredImage;

	vkCmdEndRenderPass( cmd );

	bool doCapture = m_captureRequested;
	if ( doCapture )
	{
		if ( !CreateCaptureImage( outError ) )
			return false;
		if ( !RecordCapture( cmd, imageIndex ) )
			return false;
	}
	else
	{
		VkImageMemoryBarrier toPresent = {};
		toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		toPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		toPresent.image = m_swapImages[imageIndex];
		toPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		toPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		toPresent.dstAccessMask = 0;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &toPresent );
	}

	VkResult r = vkEndCommandBuffer( cmd );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkEndCommandBuffer failed: " ) + ResultString( r ) );
		return false;
	}

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &m_imageAvailable[m_currentFrame];
	submit.pWaitDstStageMask = &waitStage;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &m_renderFinished[m_currentFrame];

	vkResetFences( m_device, 1, &m_inFlight[m_currentFrame] );
	r = vkQueueSubmit( m_graphicsQueue, 1, &submit, m_inFlight[m_currentFrame] );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkQueueSubmit failed: " ) + ResultString( r ) );
		return false;
	}

	// If we captured, resolve the pixels now: the submission must complete
	// before the host-visible copy is valid to read.
	if ( doCapture )
	{
		vkWaitForFences( m_device, 1, &m_inFlight[m_currentFrame], VK_TRUE, UINT64_MAX );
		if ( !ResolveCapturedPixels( outError ) )
			return false;
		m_captureRequested = false;
	}

	VkPresentInfoKHR present = {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.waitSemaphoreCount = 1;
	present.pWaitSemaphores = &m_renderFinished[m_currentFrame];
	present.swapchainCount = 1;
	present.pSwapchains = &m_swapchain;
	present.pImageIndices = &imageIndex;

	r = vkQueuePresentKHR( m_presentQueue, &present );
	m_frameOpen = false;
	m_currentFrame = ( m_currentFrame + 1 ) % m_framesInFlight;

	if ( r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR )
	{
		// Present surface changed; rebuild for the next frame. Not fatal.
		if ( !RecreateSwapchain( outError ) )
			return false;
		return true;
	}
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkQueuePresentKHR failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

bool CVulkanContext::ResolveCapturedPixels( std::string *outError )
{
	VkImageSubresource sub = {};
	sub.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	VkSubresourceLayout layout = {};
	vkGetImageSubresourceLayout( m_device, m_captureImage, &sub, &layout );

	void *mapped = nullptr;
	VkResult r = vkMapMemory( m_device, m_captureMemory, 0, VK_WHOLE_SIZE, 0, &mapped );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkMapMemory (capture) failed: " ) + ResultString( r ) );
		return false;
	}

	VkMappedMemoryRange range = {};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = m_captureMemory;
	range.offset = 0;
	range.size = VK_WHOLE_SIZE;
	vkInvalidateMappedMemoryRanges( m_device, 1, &range );

	const int w = static_cast<int>( m_captureExtent.width );
	const int h = static_cast<int>( m_captureExtent.height );
	m_capturedPixels.assign( static_cast<size_t>( w ) * h * 4, 0 );

	const bool bgra =
	    ( m_swapFormat == VK_FORMAT_B8G8R8A8_UNORM || m_swapFormat == VK_FORMAT_B8G8R8A8_SRGB );
	const uint8_t *base = static_cast<const uint8_t *>( mapped ) + layout.offset;
	for ( int y = 0; y < h; ++y )
	{
		const uint8_t *src = base + static_cast<size_t>( y ) * layout.rowPitch;
		uint8_t *dst = m_capturedPixels.data() + static_cast<size_t>( y ) * w * 4;
		for ( int x = 0; x < w; ++x )
		{
			const uint8_t *p = src + x * 4;
			if ( bgra )
			{
				dst[x * 4 + 0] = p[2];
				dst[x * 4 + 1] = p[1];
				dst[x * 4 + 2] = p[0];
				dst[x * 4 + 3] = p[3];
			}
			else
			{
				dst[x * 4 + 0] = p[0];
				dst[x * 4 + 1] = p[1];
				dst[x * 4 + 2] = p[2];
				dst[x * 4 + 3] = p[3];
			}
		}
	}
	vkUnmapMemory( m_device, m_captureMemory );

	m_capturedWidth = w;
	m_capturedHeight = h;
	return true;
}

void CVulkanContext::DestroySwapchainObjects()
{
	for ( VkFramebuffer fb : m_framebuffers )
		if ( fb != VK_NULL_HANDLE )
			vkDestroyFramebuffer( m_device, fb, nullptr );
	m_framebuffers.clear();

	for ( VkImageView iv : m_swapImageViews )
		if ( iv != VK_NULL_HANDLE )
			vkDestroyImageView( m_device, iv, nullptr );
	m_swapImageViews.clear();

	for ( VkImageView iv : m_depthViews )
		if ( iv != VK_NULL_HANDLE )
			vkDestroyImageView( m_device, iv, nullptr );
	m_depthViews.clear();
	for ( VkImage img : m_depthImages )
		if ( img != VK_NULL_HANDLE )
			vkDestroyImage( m_device, img, nullptr );
	m_depthImages.clear();
	for ( VkDeviceMemory mem : m_depthMemories )
		if ( mem != VK_NULL_HANDLE )
			vkFreeMemory( m_device, mem, nullptr );
	m_depthMemories.clear();

	m_swapImages.clear();
	m_imagesInFlight.clear();

	if ( m_swapchain != VK_NULL_HANDLE )
	{
		vkDestroySwapchainKHR( m_device, m_swapchain, nullptr );
		m_swapchain = VK_NULL_HANDLE;
	}
}

bool CVulkanContext::RecreateSwapchain( std::string *outError )
{
	if ( !IsValid() )
	{
		SetError( outError, "RecreateSwapchain on an invalid context" );
		return false;
	}
	vkDeviceWaitIdle( m_device );
	DestroySwapchainObjects();

	if ( !CreateSwapchain( outError ) )
		return false;
	if ( m_swapchain == VK_NULL_HANDLE )
		return true; // zero-size; retained without targets

	if ( !CreateFramebuffers( outError ) )
		return false;
	return true;
}

bool CVulkanContext::Resize( int width, int height, std::string *outError )
{
	(void)width;
	(void)height;
	// The surface reports the authoritative drawable extent; recreate against it.
	return RecreateSwapchain( outError );
}

void CVulkanContext::Shutdown()
{
	if ( m_device != VK_NULL_HANDLE )
		vkDeviceWaitIdle( m_device );

	if ( m_captureImage != VK_NULL_HANDLE )
	{
		vkDestroyImage( m_device, m_captureImage, nullptr );
		m_captureImage = VK_NULL_HANDLE;
	}
	if ( m_captureMemory != VK_NULL_HANDLE )
	{
		vkFreeMemory( m_device, m_captureMemory, nullptr );
		m_captureMemory = VK_NULL_HANDLE;
	}
	m_captureExtent = { 0, 0 };

	if ( m_device != VK_NULL_HANDLE )
	{
		DestroyDemoTriangle();
		DestroyTexturedQuad();
		DestroyIndexedUbo();
		DestroyDemoDepth();
		DestroyDynamicMesh();
		DestroySwapchainObjects();

		for ( VkSemaphore s : m_imageAvailable )
			if ( s != VK_NULL_HANDLE )
				vkDestroySemaphore( m_device, s, nullptr );
		for ( VkSemaphore s : m_renderFinished )
			if ( s != VK_NULL_HANDLE )
				vkDestroySemaphore( m_device, s, nullptr );
		for ( VkFence f : m_inFlight )
			if ( f != VK_NULL_HANDLE )
				vkDestroyFence( m_device, f, nullptr );
		m_imageAvailable.clear();
		m_renderFinished.clear();
		m_inFlight.clear();

		if ( m_commandPool != VK_NULL_HANDLE )
		{
			vkDestroyCommandPool( m_device, m_commandPool, nullptr );
			m_commandPool = VK_NULL_HANDLE;
		}
		m_commandBuffers.clear();

		if ( m_renderPass != VK_NULL_HANDLE )
		{
			vkDestroyRenderPass( m_device, m_renderPass, nullptr );
			m_renderPass = VK_NULL_HANDLE;
		}

		vkDestroyDevice( m_device, nullptr );
		m_device = VK_NULL_HANDLE;
	}

	if ( m_debugMessenger != VK_NULL_HANDLE && m_pfnDestroyDebugMessenger )
	{
		m_pfnDestroyDebugMessenger( m_instance, m_debugMessenger, nullptr );
		m_debugMessenger = VK_NULL_HANDLE;
	}

	if ( m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE )
	{
		SDL_Vulkan_DestroySurface( m_instance, m_surface, nullptr );
		m_surface = VK_NULL_HANDLE;
	}

	if ( m_instance != VK_NULL_HANDLE )
	{
		vkDestroyInstance( m_instance, nullptr );
		m_instance = VK_NULL_HANDLE;
	}

	m_physicalDevice = VK_NULL_HANDLE;
	m_graphicsQueue = VK_NULL_HANDLE;
	m_presentQueue = VK_NULL_HANDLE;
	m_graphicsQueueFamily = UINT32_MAX;
	m_presentQueueFamily = UINT32_MAX;
	m_frameOpen = false;
	m_currentFrame = 0;
	m_captureRequested = false;
	m_capturePending = false;
	m_swapExtent = { 0, 0 };
}

} // namespace render_vulkan
