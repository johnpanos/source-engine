//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan device bring-up and presentation core.
//          See vulkan_device.h for the contract.
//
//===========================================================================//

#include "vulkan_device.h"
#include "demo_triangle_spv.h"
#include "material_spv.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <initializer_list>

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

	std::vector<const char *> deviceExts = { kSwapchainExtension };
	// sRGB views of the swapchain images (linear-space blending of sRGB writes).
	{
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties( m_physicalDevice, nullptr, &extCount, nullptr );
		std::vector<VkExtensionProperties> exts( extCount );
		if ( extCount )
			vkEnumerateDeviceExtensionProperties(
			    m_physicalDevice, nullptr, &extCount, exts.data() );
		const auto has = [&]( const char *name )
		{
			for ( const VkExtensionProperties &e : exts )
				if ( std::strcmp( e.extensionName, name ) == 0 )
					return true;
			return false;
		};
		if ( has( VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME ) &&
		     has( VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME ) &&
		     has( VK_KHR_MAINTENANCE_2_EXTENSION_NAME ) )
		{
			deviceExts.push_back( VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME );
			deviceExts.push_back( VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME );
			deviceExts.push_back( VK_KHR_MAINTENANCE_2_EXTENSION_NAME );
			m_srgbAttachments = true;
		}
	}

	// Exact occlusion counts, which auto-exposure's luminance histogram needs
	// (a non-precise query may report any nonzero value for a visible draw).
	VkPhysicalDeviceFeatures supported = {};
	vkGetPhysicalDeviceFeatures( m_physicalDevice, &supported );
	VkPhysicalDeviceFeatures features = {};
	features.occlusionQueryPrecise = supported.occlusionQueryPrecise;
	m_preciseOcclusion = supported.occlusionQueryPrecise == VK_TRUE;
	// D3D9 user clip planes are clip distances. The planes travel in the push
	// constants, so a device must also hold the widest block that carries them
	// (PortalRefract's, kPortalPushBytes); without either, no clip planes are
	// reported and the material system falls back as on D3D9 hardware without
	// user clip planes.
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	m_portalPushSupported = properties.limits.maxPushConstantsSize >= kPortalPushBytes;
	m_clipPlanesSupported = supported.shaderClipDistance == VK_TRUE &&
	                        properties.limits.maxClipDistances >= kMaxClipPlanes &&
	                        properties.limits.maxPushConstantsSize >= kTexturedPushBytes;
	features.shaderClipDistance = m_clipPlanesSupported ? VK_TRUE : VK_FALSE;
	// DXT textures upload as BC images and are decoded by the sampler, as D3D9
	// samples them; without the feature the material system decompresses them.
	m_blockCompression = supported.textureCompressionBC == VK_TRUE;
	features.textureCompressionBC = supported.textureCompressionBC;
	// A depth format with stencil, as D3D9 always creates one (D24S8): portal
	// views are drawn with stencil recursion. D24S8 first, then D32S8.
	for ( VkFormat candidate : { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT } )
	{
		VkFormatProperties fp = {};
		vkGetPhysicalDeviceFormatProperties( m_physicalDevice, candidate, &fp );
		if ( fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
		{
			m_depthFormat = candidate;
			m_depthAspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			m_stencilBits = 8;
			break;
		}
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>( queueInfos.size() );
	createInfo.pQueueCreateInfos = queueInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>( deviceExts.size() );
	createInfo.ppEnabledExtensionNames = deviceExts.data();
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

bool CVulkanContext::CreateSwapchain( std::string *outError, VkSwapchainKHR oldSwapchain )
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
	m_swapFormatSrgb = m_swapFormat == VK_FORMAT_B8G8R8A8_UNORM   ? VK_FORMAT_B8G8R8A8_SRGB
	                   : m_swapFormat == VK_FORMAT_R8G8B8A8_UNORM ? VK_FORMAT_R8G8B8A8_SRGB
	                                                              : VK_FORMAT_UNDEFINED;
	if ( m_swapFormatSrgb == VK_FORMAT_UNDEFINED )
		m_srgbAttachments = false;
	if ( m_srgbAttachments )
	{
		VkFormatProperties fp = {};
		vkGetPhysicalDeviceFormatProperties( m_physicalDevice, m_swapFormatSrgb, &fp );
		if ( !( fp.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT ) )
			m_srgbAttachments = false;
	}

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
	SDL_GetWindowSizeInPixels( m_window, &m_presentDrawable[0], &m_presentDrawable[1] );
	if ( caps.currentExtent.width != UINT32_MAX )
	{
		m_presentExtent = caps.currentExtent;
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
		m_presentExtent = e;
	}

	if ( m_presentExtent.width == 0 || m_presentExtent.height == 0 )
	{
		// Zero-size (minimized) window: retain state without a swapchain; frames
		// are skipped until a non-zero Resize arrives. Not an error.
		m_swapchain = VK_NULL_HANDLE;
		m_swapImages.clear();
		m_presentImages.clear();
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
	info.imageExtent = m_presentExtent;
	info.imageArrayLayers = 1;
	// The swapchain image only receives the scaled back buffer (RecordPresentBlit).
	if ( !( caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT ) )
	{
		SetError( outError, "swapchain images cannot be transfer destinations" );
		return false;
	}
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	// ... and, where the surface allows, a copy source for RequestPresentedCapture.
	m_presentCapturable = ( caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT ) != 0;
	if ( m_presentCapturable )
		info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	info.preTransform = caps.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = m_presentMode;
	info.clipped = VK_TRUE;
	info.oldSwapchain = oldSwapchain;

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
	m_presentImages.resize( actual );
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &actual, m_presentImages.data() );

	// One back buffer per swapchain image, so a frame never renders into one an
	// earlier frame is still presenting from (m_imagesInFlight guards both).
	m_swapExtent = ( m_requestedBackBuffer.width > 0 && m_requestedBackBuffer.height > 0 )
	                   ? m_requestedBackBuffer
	                   : m_presentExtent;
	VkFormatProperties swapFeatures = {};
	vkGetPhysicalDeviceFormatProperties( m_physicalDevice, m_swapFormat, &swapFeatures );
	const VkFormatFeatureFlags blit =
	    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT;
	if ( ( swapFeatures.optimalTilingFeatures & blit ) != blit )
	{
		SetError( outError, "the swapchain format cannot be blitted for present" );
		return false;
	}
	m_presentFilter =
	    ( swapFeatures.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT )
	        ? VK_FILTER_LINEAR
	        : VK_FILTER_NEAREST;
	const VkFormat viewFormats[2] = { m_swapFormat, m_swapFormatSrgb };
	VkImageFormatListCreateInfo formatList = {};
	formatList.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
	formatList.viewFormatCount = 2;
	formatList.pViewFormats = viewFormats;
	m_swapImages.assign( actual, VK_NULL_HANDLE );
	m_backBufferMemories.assign( actual, VK_NULL_HANDLE );
	for ( uint32_t i = 0; i < actual; ++i )
	{
		VkImageCreateInfo img = {};
		img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		img.imageType = VK_IMAGE_TYPE_2D;
		img.format = m_swapFormat;
		img.extent = { m_swapExtent.width, m_swapExtent.height, 1 };
		img.mipLevels = 1;
		img.arrayLayers = 1;
		img.samples = VK_SAMPLE_COUNT_1_BIT;
		img.tiling = VK_IMAGE_TILING_OPTIMAL;
		img.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if ( m_srgbAttachments )
		{
			img.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
			img.pNext = &formatList;
		}
		if ( vkCreateImage( m_device, &img, nullptr, &m_swapImages[i] ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateImage (back buffer) failed" );
			return false;
		}
		VkMemoryRequirements req = {};
		vkGetImageMemoryRequirements( m_device, m_swapImages[i], &req );
		bool found = false;
		const uint32_t type =
		    FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &found );
		VkMemoryAllocateInfo ai = {};
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = req.size;
		ai.memoryTypeIndex = type;
		if ( !found ||
		     vkAllocateMemory( m_device, &ai, nullptr, &m_backBufferMemories[i] ) != VK_SUCCESS )
		{
			SetError( outError, "vkAllocateMemory (back buffer) failed" );
			return false;
		}
		vkBindImageMemory( m_device, m_swapImages[i], m_backBufferMemories[i], 0 );
	}

	m_swapImageViews.resize( actual );
	m_swapImageViewsSrgb.assign( m_srgbAttachments ? actual : 0, VK_NULL_HANDLE );
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
		if ( r == VK_SUCCESS && m_srgbAttachments )
		{
			iv.format = m_swapFormatSrgb;
			r = vkCreateImageView( m_device, &iv, nullptr, &m_swapImageViewsSrgb[i] );
		}
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
		iv.subresourceRange.aspectMask = m_depthAspects;
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

	// Three passes that differ only in load ops and layouts, which Vulkan
	// excludes from render-pass compatibility: every pipeline built against
	// m_renderPass is therefore valid in all three. Formats, sample counts and
	// dependencies must stay identical between them.
	auto makePass = [&]( VkAttachmentLoadOp loadOp, VkImageLayout colorInitial,
	                    VkImageLayout colorFinal, VkImageLayout depthInitial, VkRenderPass *outPass,
	                    VkFormat colorFormat ) -> bool
	{
		VkAttachmentDescription color = {};
		color.format = colorFormat;
		color.samples = VK_SAMPLE_COUNT_1_BIT;
		color.loadOp = loadOp;
		color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color.initialLayout = colorInitial;
		color.finalLayout = colorFinal;

		VkAttachmentReference colorRef = {};
		colorRef.attachment = 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Depth is stored, not discarded: a frame leaves the swapchain image for
		// a render-target pass and comes back to it with its depth intact.
		VkAttachmentDescription depth = {};
		depth.format = m_depthFormat;
		depth.samples = VK_SAMPLE_COUNT_1_BIT;
		depth.loadOp = loadOp;
		depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// Stencil persists like depth: portal recursion spans render passes.
		depth.stencilLoadOp = loadOp;
		depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth.initialLayout = depthInitial;
		depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthRef = {};
		depthRef.attachment = 1;
		depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		subpass.pDepthStencilAttachment = &depthRef;

		// In: earlier attachment writes, copies, and shader reads of a render
		// target (write-after-read) finish before this pass writes. Out: this
		// pass's writes are visible to later sampling, copies and passes.
		VkSubpassDependency deps[2] = {};
		deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		deps[0].dstSubpass = 0;
		deps[0].srcStageMask =
		    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		deps[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
		                        VK_ACCESS_TRANSFER_WRITE_BIT;
		deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
		                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		deps[1].srcSubpass = 0;
		deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		deps[1].srcAccessMask =
		    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		deps[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
		                       VK_PIPELINE_STAGE_TRANSFER_BIT |
		                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		deps[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[] = { color, depth };
		VkRenderPassCreateInfo rp = {};
		rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp.attachmentCount = 2;
		rp.pAttachments = attachments;
		rp.subpassCount = 1;
		rp.pSubpasses = &subpass;
		rp.dependencyCount = 2;
		rp.pDependencies = deps;

		VkResult r = vkCreateRenderPass( m_device, &rp, nullptr, outPass );
		if ( r != VK_SUCCESS )
		{
			SetError( outError, std::string( "vkCreateRenderPass failed: " ) + ResultString( r ) );
			return false;
		}
		return true;
	};

	// The frame's first pass clears the swapchain image and leaves it in a
	// color-attachment layout; EndFrame() performs the explicit transition to
	// PRESENT_SRC (or the capture path) so one pass serves present and readback.
	// The sRGB passes are the load passes over sRGB views of the same images.
	return makePass( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED,
	           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, &m_renderPass,
	           m_swapFormat ) &&
	       makePass( VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &m_renderPassLoad,
	           m_swapFormat ) &&
	       makePass( VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &m_renderPassTarget,
	           m_swapFormat ) &&
	       ( !m_srgbAttachments ||
	           ( makePass( VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &m_renderPassLoadSrgb,
	                 m_swapFormatSrgb ) &&
	               makePass( VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &m_renderPassTargetSrgb,
	                   m_swapFormatSrgb ) ) );
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
	m_framebuffersSrgb.assign( m_srgbAttachments ? m_swapImageViews.size() : 0, VK_NULL_HANDLE );
	for ( size_t i = 0; i < m_framebuffersSrgb.size(); ++i )
	{
		VkImageView attachments[] = { m_swapImageViewsSrgb[i], m_depthViews[i] };
		VkFramebufferCreateInfo fb = {};
		fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb.renderPass = m_renderPassLoadSrgb;
		fb.attachmentCount = 2;
		fb.pAttachments = attachments;
		fb.width = m_swapExtent.width;
		fb.height = m_swapExtent.height;
		fb.layers = 1;
		if ( vkCreateFramebuffer( m_device, &fb, nullptr, &m_framebuffersSrgb[i] ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateFramebuffer (sRGB) failed" );
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

bool CVulkanContext::CreateCaptureImage( VkExtent2D extent, std::string *outError )
{
	// Recreate only when the target extent changed.
	if ( m_captureImage != VK_NULL_HANDLE && m_captureExtent.width == extent.width &&
	     m_captureExtent.height == extent.height )
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
	img.extent = { extent.width, extent.height, 1 };
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
	m_captureExtent = extent;
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

	// Shared dynamic vertex: position (vec3) + color (vec3) + uv (vec2) + lightmap
	// uv (vec2), then normal, tangent and color alpha, kDynVertexFloats floats. Shaders that ignore a component (e.g.
	// color for the textured shader, uv for the others) do not read that attribute.
	VkVertexInputBindingDescription binding = {};
	binding.binding = 0;
	binding.stride = sizeof( float ) * kDynVertexFloats;
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attrs[4] = {};
	attrs[0].location = 0;
	attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[0].offset = 0;
	attrs[1].location = 1;
	attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[1].offset = sizeof( float ) * 3;
	attrs[2].location = 2;
	attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
	attrs[2].offset = sizeof( float ) * 6;
	attrs[3].location = 3;
	attrs[3].format = VK_FORMAT_R32G32_SFLOAT;
	attrs[3].offset = sizeof( float ) * 8;
	// Normal (10..12) and tangent (13..16) complete the record; only
	// PortalRefract reads them (m_portalVin).
	VkPipelineVertexInputStateCreateInfo vin = {};
	vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vin.vertexBindingDescriptionCount = 1;
	vin.pVertexBindingDescriptions = &binding;
	vin.vertexAttributeDescriptionCount = 4;
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

	// --- "$basetexture" material pipeline: the default texture sampled when a
	// draw binds none. The textured shader MULTIPLIES by this sample, so the only
	// correct default is opaque white -- the multiplicative identity, matching
	// what D3D9 gets from TEXTURE_WHITE. A patterned default would silently tint
	// or mask every draw whose material texture is missing, turning a texture
	// residency gap into a whole-frame corruption that looks like a raster bug. ---
	{
		const uint32_t texW = 1, texH = 1;
		const uint8_t texels[texW * texH * 4] = { 255, 255, 255, 255 };
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
		// One sampler per state combination. Material textures tile: world UVs
		// span many repeats, so clamping by default would collapse every tiled
		// surface onto its edge texel. Wrap is D3D9's default; Source clamps the
		// textures that need it through TexWrap.
		for ( int state = 0; state < kSamplerStates; ++state )
		{
			VkSamplerCreateInfo sc = {};
			sc.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			const VkFilter filter =
			    ( state & kSamplerLinear ) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
			sc.magFilter = filter;
			sc.minFilter = filter;
			sc.mipmapMode = ( state & kSamplerMipLinear ) ? VK_SAMPLER_MIPMAP_MODE_LINEAR
			                                              : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sc.minLod = 0.0f;
			sc.maxLod =
			    ( state & ( kSamplerMipPoint | kSamplerMipLinear ) ) ? VK_LOD_CLAMP_NONE : 0.0f;
			sc.addressModeU = ( state & kSamplerClampU ) ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
			                                             : VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sc.addressModeV = ( state & kSamplerClampV ) ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
			                                             : VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sc.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			if ( vkCreateSampler( m_device, &sc, nullptr, &m_samplers[state] ) != VK_SUCCESS )
			{
				SetError( outError, "vkCreateSampler (dynamic texture) failed" );
				return false;
			}
		}
		m_dynTexSampler = m_samplers[0];

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
		// Deleted textures return their sets (DestroyManagedTexture).
		dp.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
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
		// + vec4 alphaParams (128 bytes), and the user clip planes when the device
		// clips (demo_dyn_tex_clip.vert reads them after the block).
		texPc.size = m_clipPlanesSupported ? kTexturedPushBytes : sizeof( float ) * 32;
		// Set 0 is the base texture and set 1 the lightmap: both are one
		// combined image sampler, so each managed texture's single set serves
		// either role.
		const VkDescriptorSetLayout texSetLayouts[2] = { m_dynTexDescLayout, m_dynTexDescLayout };
		VkPipelineLayoutCreateInfo texPl = {};
		texPl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		texPl.setLayoutCount = 2;
		texPl.pSetLayouts = texSetLayouts;
		texPl.pushConstantRangeCount = 1;
		texPl.pPushConstantRanges = &texPc;
		if ( vkCreatePipelineLayout( m_device, &texPl, nullptr, &m_dynTexPipelineLayout ) !=
		     VK_SUCCESS )
		{
			SetError( outError, "vkCreatePipelineLayout (dynamic texture) failed" );
			return false;
		}

		VkShaderModule texVert = VK_NULL_HANDLE, texFrag = VK_NULL_HANDLE;
		if ( !( m_clipPlanesSupported ? CreateShaderModule( g_materialTexClipVertSpv,
		                                    sizeof( g_materialTexClipVertSpv ), &texVert, outError )
		                              : CreateShaderModule( g_materialTexVertSpv,
		                                    sizeof( g_materialTexVertSpv ), &texVert, outError ) ) )
			return false;
		if ( !CreateShaderModule(
		         g_materialTexFragSpv, sizeof( g_materialTexFragSpv ), &texFrag, outError ) )
		{
			vkDestroyShaderModule( m_device, texVert, nullptr );
			return false;
		}
		// Keep the textured pipelines' fixed state and shader modules: pipelines
		// for further blend/depth states are built from them on first use.
		TexturedPipelineTemplate &t = m_texTemplate;
		t.stages[0] = stages[0];
		t.stages[1] = stages[1];
		t.stages[0].module = texVert;
		t.stages[1].module = texFrag;
		t.binding = binding;
		std::memcpy( t.attrs, attrs, sizeof( attrs ) );
		// The textured stage also reads the vertex color's alpha, which ends the
		// record ($vertexalpha; demo_dyn_tex.vert location 6).
		t.attrs[4].location = 6;
		t.attrs[4].format = VK_FORMAT_R32_SFLOAT;
		t.attrs[4].offset = sizeof( float ) * 17;
		t.vin = vin;
		t.vin.pVertexBindingDescriptions = &t.binding;
		t.vin.vertexAttributeDescriptionCount = 5;
		t.vin.pVertexAttributeDescriptions = t.attrs;
		t.ia = ia;
		t.vp = vp;
		t.rs = rs;
		t.ms = ms;
		// Material pipelines also take D3D9's stencil reference and masks per draw.
		t.dynStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
		t.dynStates[1] = VK_DYNAMIC_STATE_SCISSOR;
		t.dynStates[2] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
		t.dynStates[3] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
		t.dynStates[4] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
		t.dyn = dyn;
		t.dyn.dynamicStateCount = 5;
		t.dyn.pDynamicStates = t.dynStates;

		// Build the default (opaque) state now so a device that cannot create
		// the material pipeline fails initialization rather than every draw.
		if ( TexturedPipeline( DynRasterState() ) == VK_NULL_HANDLE )
		{
			SetError( outError, "vkCreateGraphicsPipelines (dynamic texture) failed" );
			return false;
		}
	}
	if ( !InitPortalPipeline( outError ) )
		return false;
	if ( !InitSkinPipeline( outError ) )
		return false;

	Log( "dynamic mesh pipelines ready (4 material shaders incl. textured + blend variants)\n" );
	return true;
}

uint32_t CVulkanContext::RasterStateKey( const DynRasterState &state )
{
	// Blend factors are below 32 and compare ops below 8, so the state packs
	// into disjoint bit fields.
	return ( state.blend ? 1u : 0u ) | ( static_cast<uint32_t>( state.srcFactor ) & 31u ) << 1 |
	       ( static_cast<uint32_t>( state.dstFactor ) & 31u ) << 6 |
	       ( state.depthTest ? 1u : 0u ) << 11 | ( state.depthWrite ? 1u : 0u ) << 12 |
	       ( static_cast<uint32_t>( state.depthCompare ) & 7u ) << 13 |
	       ( state.colorWrite ? 1u : 0u ) << 16 |
	       ( static_cast<uint32_t>( state.cullMode ) & 3u ) << 17 |
	       ( state.stencilEnable ? 1u : 0u ) << 19 |
	       ( state.stencilEnable
	               ? ( static_cast<uint32_t>( state.stencilCompare ) & 7u ) << 20 |
	                     ( static_cast<uint32_t>( state.stencilFail ) & 7u ) << 23 |
	                     ( static_cast<uint32_t>( state.stencilDepthFail ) & 7u ) << 26 |
	                     ( static_cast<uint32_t>( state.stencilPass ) & 7u ) << 29
	               : 0u );
}

VkPipeline CVulkanContext::TexturedPipeline( const DynRasterState &state, bool srgbPass )
{
	const uint64_t key = RasterStateKey( state ) | ( srgbPass ? 1ull << 32 : 0ull );
	const auto existing = m_dynTexPipelines.find( key );
	if ( existing != m_dynTexPipelines.end() )
		return existing->second;
	if ( m_texTemplate.stages[0].module == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_texTemplate.stages[0].module,
	    m_texTemplate.stages[1].module, m_dynTexPipelineLayout, &m_texTemplate.vin,
	    srgbPass ? m_renderPassLoadSrgb : m_renderPass );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (textured, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	// A failed state is cached too, so it is reported once rather than per draw.
	m_dynTexPipelines[key] = pipeline;
	return pipeline;
}

VkPipeline CVulkanContext::PortalPipeline( const DynRasterState &state, bool srgbPass )
{
	const uint64_t key = RasterStateKey( state ) | ( srgbPass ? 1ull << 32 : 0ull );
	const auto existing = m_portalPipelines.find( key );
	if ( existing != m_portalPipelines.end() )
		return existing->second;
	if ( m_portalVert == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_portalVert, m_portalFrag,
	    m_portalPipelineLayout, &m_portalVin, srgbPass ? m_renderPassLoadSrgb : m_renderPass );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (PortalRefract, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_portalPipelines[key] = pipeline;
	return pipeline;
}

VkPipeline CVulkanContext::SkinPipeline( const DynRasterState &state, bool srgbPass )
{
	const uint64_t key = RasterStateKey( state ) | ( srgbPass ? 1ull << 32 : 0ull );
	const auto existing = m_skinPipelines.find( key );
	if ( existing != m_skinPipelines.end() )
		return existing->second;
	if ( m_skinVert == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_skinVert, m_skinFrag,
	    m_skinPipelineLayout, &m_skinVin, srgbPass ? m_renderPassLoadSrgb : m_renderPass );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (skin, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_skinPipelines[key] = pipeline;
	return pipeline;
}

static VkStencilOpState StencilFaceState( const CVulkanContext::DynRasterState &state )
{
	VkStencilOpState face = {};
	face.failOp = state.stencilFail;
	face.passOp = state.stencilPass;
	face.depthFailOp = state.stencilDepthFail;
	face.compareOp = state.stencilCompare;
	// The masks and the reference are dynamic (set per draw).
	return face;
}

VkPipeline CVulkanContext::BuildMaterialPipeline( const DynRasterState &state, VkShaderModule vert,
    VkShaderModule frag, VkPipelineLayout layout,
    const VkPipelineVertexInputStateCreateInfo *vertexInput, VkRenderPass renderPass )
{
	VkPipelineColorBlendAttachmentState att = {};
	att.colorWriteMask = state.colorWrite ? VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	                                      : 0;
	att.blendEnable = state.blend ? VK_TRUE : VK_FALSE;
	att.srcColorBlendFactor = state.srcFactor;
	att.dstColorBlendFactor = state.dstFactor;
	att.colorBlendOp = VK_BLEND_OP_ADD;
	att.srcAlphaBlendFactor = state.srcFactor;
	att.dstAlphaBlendFactor = state.dstFactor;
	att.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo cb = {};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments = &att;
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = state.depthTest ? VK_TRUE : VK_FALSE;
	ds.depthWriteEnable = state.depthWrite ? VK_TRUE : VK_FALSE;
	ds.depthCompareOp = state.depthCompare;
	ds.stencilTestEnable = ( state.stencilEnable && m_stencilBits > 0 ) ? VK_TRUE : VK_FALSE;
	ds.front = StencilFaceState( state );
	ds.back = ds.front;

	const TexturedPipelineTemplate &t = m_texTemplate;
	VkPipelineShaderStageCreateInfo stages[2] = { t.stages[0], t.stages[1] };
	stages[0].module = vert;
	stages[1].module = frag;
	VkGraphicsPipelineCreateInfo gp = {};
	gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp.stageCount = 2;
	gp.pStages = stages;
	gp.pVertexInputState = vertexInput;
	gp.pInputAssemblyState = &t.ia;
	gp.pViewportState = &t.vp;
	VkPipelineRasterizationStateCreateInfo rs = t.rs;
	rs.cullMode = state.cullMode;
	rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
	gp.pRasterizationState = &rs;
	gp.pMultisampleState = &t.ms;
	gp.pColorBlendState = &cb;
	gp.pDynamicState = &t.dyn;
	gp.pDepthStencilState = &ds;
	gp.layout = layout;
	// The swapchain and render-target passes are render-pass compatible, and so
	// are the two sRGB passes with each other.
	gp.renderPass = renderPass;
	gp.subpass = 0;
	VkPipeline pipeline = VK_NULL_HANDLE;
	if ( vkCreateGraphicsPipelines( m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &pipeline ) !=
	     VK_SUCCESS )
		pipeline = VK_NULL_HANDLE;
	return pipeline;
}

// PortalRefract: its own shaders (a GLSL port of portal_refract_vs20.fxc and
// portal_refract_ps2x.fxc), three sampler sets (s0 refraction, s1 noise, s2
// color) and a push block with its registers (see shaders/portal_refract.vert).
// A device whose push constants cannot hold the block gets no pipeline, and the
// shader API declines the material by name.
bool CVulkanContext::InitPortalPipeline( std::string *outError )
{
	if ( !m_portalPushSupported || !m_clipPlanesSupported )
	{
		Log( "PortalRefract pipeline unavailable: push constants or clip distances too small\n" );
		return true;
	}
	VkPushConstantRange pc = {};
	pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pc.size = kPortalPushBytes;
	const VkDescriptorSetLayout sets[3] = {
	    m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout };
	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.setLayoutCount = 3;
	pl.pSetLayouts = sets;
	pl.pushConstantRangeCount = 1;
	pl.pPushConstantRanges = &pc;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &m_portalPipelineLayout ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreatePipelineLayout (PortalRefract) failed" );
		return false;
	}
	if ( !CreateShaderModule(
	         g_portalRefractVertSpv, sizeof( g_portalRefractVertSpv ), &m_portalVert, outError ) ||
	     !CreateShaderModule(
	         g_portalRefractFragSpv, sizeof( g_portalRefractFragSpv ), &m_portalFrag, outError ) )
		return false;
	// Position, color, uv and lightmap uv as the textured stage reads them (not
	// its vertex alpha, which PortalRefract does not use).
	std::memcpy( m_portalAttrs, m_texTemplate.attrs, sizeof( m_portalAttrs[0] ) * 4 );
	m_portalAttrs[4].location = 4;
	m_portalAttrs[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_portalAttrs[4].offset = sizeof( float ) * 10;
	m_portalAttrs[5].location = 5;
	m_portalAttrs[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_portalAttrs[5].offset = sizeof( float ) * 13;
	m_portalVin = m_texTemplate.vin;
	m_portalVin.vertexAttributeDescriptionCount = 6;
	m_portalVin.pVertexAttributeDescriptions = m_portalAttrs;
	if ( PortalPipeline( DynRasterState() ) == VK_NULL_HANDLE )
	{
		SetError( outError, "vkCreateGraphicsPipelines (PortalRefract) failed" );
		return false;
	}
	return true;
}

// VertexLitGeneric's $phong path: its own shaders (a GLSL port of skin_vs20.fxc
// and skin_ps20b.fxc), six sampler sets, the pixel shader constants in a
// dynamic uniform buffer (set 6) and a push block with the vertex stage's
// registers (see shaders/skin.frag). A device that cannot bind seven sets or
// the block gets no pipeline, and the shader API declines the draws by name.
bool CVulkanContext::InitSkinPipeline( std::string *outError )
{
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	if ( !m_clipPlanesSupported || properties.limits.maxBoundDescriptorSets < 7 ||
	     properties.limits.maxPushConstantsSize < kSkinPushBytes )
	{
		Log( "skin pipeline unavailable: descriptor sets, push constants or clip distances\n" );
		return true;
	}
	m_uboAlignment =
	    std::max<VkDeviceSize>( 16, properties.limits.minUniformBufferOffsetAlignment );

	VkDescriptorSetLayoutBinding ubo = {};
	ubo.binding = 0;
	ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	ubo.descriptorCount = 1;
	ubo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo lb = {};
	lb.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	lb.bindingCount = 1;
	lb.pBindings = &ubo;
	if ( vkCreateDescriptorSetLayout( m_device, &lb, nullptr, &m_skinUboLayout ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateDescriptorSetLayout (skin constants) failed" );
		return false;
	}
	const uint32_t slots = std::max<uint32_t>( 1u, m_framesInFlight );
	VkDescriptorPoolSize size = {};
	size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	size.descriptorCount = slots;
	VkDescriptorPoolCreateInfo pi = {};
	pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pi.maxSets = slots;
	pi.poolSizeCount = 1;
	pi.pPoolSizes = &size;
	if ( vkCreateDescriptorPool( m_device, &pi, nullptr, &m_skinUboPool ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateDescriptorPool (skin constants) failed" );
		return false;
	}
	m_skinUbos.assign( slots, SkinUniformBuffer() );
	for ( SkinUniformBuffer &slot : m_skinUbos )
	{
		VkDescriptorSetAllocateInfo da = {};
		da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		da.descriptorPool = m_skinUboPool;
		da.descriptorSetCount = 1;
		da.pSetLayouts = &m_skinUboLayout;
		if ( vkAllocateDescriptorSets( m_device, &da, &slot.set ) != VK_SUCCESS )
		{
			SetError( outError, "vkAllocateDescriptorSets (skin constants) failed" );
			return false;
		}
	}

	VkPushConstantRange pc = {};
	pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pc.size = kSkinPushBytes;
	const VkDescriptorSetLayout sets[7] = { m_dynTexDescLayout, m_dynTexDescLayout,
	    m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout,
	    m_skinUboLayout };
	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.setLayoutCount = 7;
	pl.pSetLayouts = sets;
	pl.pushConstantRangeCount = 1;
	pl.pPushConstantRanges = &pc;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &m_skinPipelineLayout ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreatePipelineLayout (skin) failed" );
		return false;
	}
	if ( !CreateShaderModule( g_skinVertSpv, sizeof( g_skinVertSpv ), &m_skinVert, outError ) ||
	     !CreateShaderModule( g_skinFragSpv, sizeof( g_skinFragSpv ), &m_skinFrag, outError ) )
		return false;
	// Position, attenuation (the color slot) and uv as the textured stage reads
	// them, then the normal, tangent and the fourth attenuation (the alpha slot).
	std::memcpy( m_skinAttrs, m_texTemplate.attrs, sizeof( m_skinAttrs[0] ) * 4 );
	m_skinAttrs[4].location = 4;
	m_skinAttrs[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_skinAttrs[4].offset = sizeof( float ) * 10;
	m_skinAttrs[5].location = 5;
	m_skinAttrs[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_skinAttrs[5].offset = sizeof( float ) * 13;
	m_skinAttrs[6].location = 6;
	m_skinAttrs[6].format = VK_FORMAT_R32_SFLOAT;
	m_skinAttrs[6].offset = sizeof( float ) * 17;
	m_skinVin = m_texTemplate.vin;
	m_skinVin.vertexAttributeDescriptionCount = 7;
	m_skinVin.pVertexAttributeDescriptions = m_skinAttrs;
	if ( SkinPipeline( DynRasterState() ) == VK_NULL_HANDLE )
	{
		SetError( outError, "vkCreateGraphicsPipelines (skin) failed" );
		return false;
	}
	return true;
}

bool CVulkanContext::UploadSkinConstants( std::vector<uint32_t> *offsets )
{
	offsets->clear();
	if ( m_dynSkinConstants.empty() || m_skinUbos.empty() )
		return !m_dynSkinConstants.empty() ? false : true;
	const VkDeviceSize block = sizeof( m_dynSkinConstants[0].ps );
	const VkDeviceSize stride = ( block + m_uboAlignment - 1 ) / m_uboAlignment * m_uboAlignment;
	const VkDeviceSize needed = stride * m_dynSkinConstants.size();
	// This frame's slot: its fence was waited on when the frame began, so the
	// GPU no longer reads it.
	SkinUniformBuffer &slot = m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()];
	if ( needed > slot.capacity )
	{
		if ( slot.mapped )
			vkUnmapMemory( m_device, slot.memory );
		if ( slot.buffer != VK_NULL_HANDLE )
			vkDestroyBuffer( m_device, slot.buffer, nullptr );
		if ( slot.memory != VK_NULL_HANDLE )
			vkFreeMemory( m_device, slot.memory, nullptr );
		slot.mapped = nullptr;
		slot.buffer = VK_NULL_HANDLE;
		slot.memory = VK_NULL_HANDLE;
		slot.capacity = 0;
		const VkDeviceSize capacity = needed + needed / 2 + stride * 16;
		std::string error;
		if ( !CreateBuffer( capacity, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		         &slot.buffer, &slot.memory, &error ) ||
		     vkMapMemory( m_device, slot.memory, 0, capacity, 0, &slot.mapped ) != VK_SUCCESS )
		{
			Log( "skin constants buffer (%llu bytes) unavailable: %s\n",
			    static_cast<unsigned long long>( capacity ), error.c_str() );
			slot.mapped = nullptr;
			return false;
		}
		slot.capacity = capacity;
		VkDescriptorBufferInfo bi = {};
		bi.buffer = slot.buffer;
		bi.offset = 0;
		bi.range = block;
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = slot.set;
		wds.dstBinding = 0;
		wds.descriptorCount = 1;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		wds.pBufferInfo = &bi;
		vkUpdateDescriptorSets( m_device, 1, &wds, 0, nullptr );
	}
	for ( size_t i = 0; i < m_dynSkinConstants.size(); ++i )
	{
		std::memcpy( static_cast<unsigned char *>( slot.mapped ) + stride * i,
		    m_dynSkinConstants[i].ps, block );
		offsets->push_back( static_cast<uint32_t>( stride * i ) );
	}
	return true;
}

void CVulkanContext::DestroySkinPipeline()
{
	for ( const auto &entry : m_skinPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_skinPipelines.clear();
	for ( VkShaderModule *module : { &m_skinVert, &m_skinFrag } )
	{
		if ( *module != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, *module, nullptr );
		*module = VK_NULL_HANDLE;
	}
	for ( SkinUniformBuffer &slot : m_skinUbos )
	{
		if ( slot.mapped )
			vkUnmapMemory( m_device, slot.memory );
		if ( slot.buffer != VK_NULL_HANDLE )
			vkDestroyBuffer( m_device, slot.buffer, nullptr );
		if ( slot.memory != VK_NULL_HANDLE )
			vkFreeMemory( m_device, slot.memory, nullptr );
	}
	m_skinUbos.clear();
	if ( m_skinPipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_skinPipelineLayout, nullptr );
	m_skinPipelineLayout = VK_NULL_HANDLE;
	if ( m_skinUboPool != VK_NULL_HANDLE )
		vkDestroyDescriptorPool( m_device, m_skinUboPool, nullptr );
	m_skinUboPool = VK_NULL_HANDLE;
	if ( m_skinUboLayout != VK_NULL_HANDLE )
		vkDestroyDescriptorSetLayout( m_device, m_skinUboLayout, nullptr );
	m_skinUboLayout = VK_NULL_HANDLE;
}

void CVulkanContext::SetDynamicTransform( const float *m16 )
{
	if ( m16 )
		std::memcpy( m_dynTransform, m16, sizeof( m_dynTransform ) );
}

int CVulkanContext::CreateManagedTexture( int width, int height, VkFormat format,
    std::string *outError, VkImageUsageFlags extraUsage, uint32_t mipLevels, VkFormat srgbAlias )
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
	uint32_t fullChain = 1;
	for ( uint32_t size = std::max( t.width, t.height ); size > 1; size >>= 1 )
		++fullChain;
	t.mipLevels = std::max( 1u, std::min( mipLevels, fullChain ) );

	VkImageCreateInfo ii = {};
	ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ii.imageType = VK_IMAGE_TYPE_2D;
	ii.format = format;
	ii.extent = { t.width, t.height, 1 };
	ii.mipLevels = t.mipLevels;
	ii.arrayLayers = 1;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | extraUsage;
	ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// 8-bit and BC color formats also get an sRGB view for sRGB sampling.
	if ( srgbAlias == VK_FORMAT_UNDEFINED )
	{
		switch ( format )
		{
		case VK_FORMAT_R8G8B8A8_UNORM:
			srgbAlias = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		case VK_FORMAT_B8G8R8A8_UNORM:
			srgbAlias = VK_FORMAT_B8G8R8A8_SRGB;
			break;
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			srgbAlias = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			break;
		case VK_FORMAT_BC2_UNORM_BLOCK:
			srgbAlias = VK_FORMAT_BC2_SRGB_BLOCK;
			break;
		case VK_FORMAT_BC3_UNORM_BLOCK:
			srgbAlias = VK_FORMAT_BC3_SRGB_BLOCK;
			break;
		default:
			break;
		}
		VkFormatProperties sp = {};
		if ( srgbAlias != VK_FORMAT_UNDEFINED )
			vkGetPhysicalDeviceFormatProperties( m_physicalDevice, srgbAlias, &sp );
		if ( !( sp.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT ) )
			srgbAlias = VK_FORMAT_UNDEFINED;
	}
	const VkFormat viewFormats[2] = { format, srgbAlias };
	VkImageFormatListCreateInfo formatList = {};
	formatList.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
	formatList.viewFormatCount = 2;
	formatList.pViewFormats = viewFormats;
	if ( srgbAlias != VK_FORMAT_UNDEFINED )
	{
		ii.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
		ii.pNext = &formatList;
	}
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
	iv.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, t.mipLevels, 0, 1 };
	if ( vkCreateImageView( m_device, &iv, nullptr, &t.view ) != VK_SUCCESS )
	{
		vkFreeMemory( m_device, t.memory, nullptr );
		vkDestroyImage( m_device, t.image, nullptr );
		SetError( outError, "vkCreateImageView (managed texture) failed" );
		return -1;
	}
	if ( srgbAlias != VK_FORMAT_UNDEFINED )
	{
		iv.format = srgbAlias;
		if ( vkCreateImageView( m_device, &iv, nullptr, &t.srgbView ) != VK_SUCCESS )
		{
			vkDestroyImageView( m_device, t.view, nullptr );
			vkFreeMemory( m_device, t.memory, nullptr );
			vkDestroyImage( m_device, t.image, nullptr );
			SetError( outError, "vkCreateImageView (managed texture, sRGB) failed" );
			return -1;
		}
	}
	// A mip chain is filled level by level, so every level is made samplable up
	// front; each upload then moves only its own level.
	if ( t.mipLevels > 1 )
	{
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		if ( !BeginSingleTimeCommands( &cmd, outError ) )
			return -1;
		VkImageMemoryBarrier ready = {};
		ready.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		ready.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ready.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ready.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ready.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ready.image = t.image;
		ready.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, t.mipLevels, 0, 1 };
		ready.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ready );
		if ( !EndSingleTimeCommands( cmd, outError ) )
			return -1;
	}

	// Allocate this texture's own descriptor set (if the pool/layout are ready and
	// have room), so per-draw texture binding can point each draw at its texture.
	// The sRGB view has a set of its own.
	if ( m_dynTexDescLayout != VK_NULL_HANDLE && m_dynTexDescPool != VK_NULL_HANDLE &&
	     m_liveTextureSets + 2 < kMaxManagedTexSets )
	{
		const auto allocateSet = [&]( VkImageView view, VkDescriptorSet *outSet )
		{
			VkDescriptorSetAllocateInfo da = {};
			da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			da.descriptorPool = m_dynTexDescPool;
			da.descriptorSetCount = 1;
			da.pSetLayouts = &m_dynTexDescLayout;
			if ( vkAllocateDescriptorSets( m_device, &da, outSet ) != VK_SUCCESS )
			{
				*outSet = VK_NULL_HANDLE;
				return;
			}
			++m_liveTextureSets;
			VkDescriptorImageInfo dii = {};
			dii.sampler = m_dynTexSampler;
			dii.imageView = view;
			dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkWriteDescriptorSet wds = {};
			wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			wds.dstSet = *outSet;
			wds.dstBinding = 0;
			wds.descriptorCount = 1;
			wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			wds.pImageInfo = &dii;
			vkUpdateDescriptorSets( m_device, 1, &wds, 0, nullptr );
		};
		allocateSet( t.view, &t.descSet );
		if ( t.srgbView != VK_NULL_HANDLE )
			allocateSet( t.srgbView, &t.descSetSrgb );
	}

	if ( !m_freeTextureHandles.empty() )
	{
		const int handle = m_freeTextureHandles.back();
		m_freeTextureHandles.pop_back();
		m_managedTextures[static_cast<size_t>( handle )] = t;
		return handle;
	}
	m_managedTextures.push_back( t );
	return static_cast<int>( m_managedTextures.size() - 1 );
}

void CVulkanContext::ReleaseManagedTextureObjects( ManagedTexture &t )
{
	VkDescriptorSet sets[2] = { t.descSet, t.descSetSrgb };
	for ( VkDescriptorSet set : sets )
	{
		if ( set != VK_NULL_HANDLE && m_dynTexDescPool != VK_NULL_HANDLE )
		{
			vkFreeDescriptorSets( m_device, m_dynTexDescPool, 1, &set );
			--m_liveTextureSets;
		}
	}
	if ( t.framebuffer != VK_NULL_HANDLE )
		vkDestroyFramebuffer( m_device, t.framebuffer, nullptr );
	if ( t.framebufferSrgb != VK_NULL_HANDLE )
		vkDestroyFramebuffer( m_device, t.framebufferSrgb, nullptr );
	if ( t.srgbView != VK_NULL_HANDLE )
		vkDestroyImageView( m_device, t.srgbView, nullptr );
	if ( t.depthView != VK_NULL_HANDLE )
		vkDestroyImageView( m_device, t.depthView, nullptr );
	if ( t.depthImage != VK_NULL_HANDLE )
		vkDestroyImage( m_device, t.depthImage, nullptr );
	if ( t.depthMemory != VK_NULL_HANDLE )
		vkFreeMemory( m_device, t.depthMemory, nullptr );
	if ( t.view != VK_NULL_HANDLE )
		vkDestroyImageView( m_device, t.view, nullptr );
	if ( t.image != VK_NULL_HANDLE )
		vkDestroyImage( m_device, t.image, nullptr );
	if ( t.memory != VK_NULL_HANDLE )
		vkFreeMemory( m_device, t.memory, nullptr );
	t = ManagedTexture();
}

void CVulkanContext::DestroyManagedTexture( int handle )
{
	if ( !IsValid() || handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) ||
	     m_managedTextures[static_cast<size_t>( handle )].image == VK_NULL_HANDLE )
		return;
	// Retired after the frame being recorded is submitted and complete: until
	// then no record can see the handle reused.
	ManagedTexture &slot = m_managedTextures[static_cast<size_t>( handle )];
	m_retiredTextures.push_back( { slot, handle, m_submitSerial + 1 } );
	slot = ManagedTexture();
	if ( m_dynBoundTexHandle == handle )
		m_dynBoundTexHandle = -1;
	if ( m_dynLightmapHandle == handle )
		m_dynLightmapHandle = -1;
	if ( m_dynTarget == handle )
		m_dynTarget = -1;
	RetireCompletedTextures();
}

void CVulkanContext::RetireCompletedTextures()
{
	size_t kept = 0;
	for ( RetiredTexture &r : m_retiredTextures )
	{
		if ( r.afterSerial <= m_completedSerial )
		{
			ReleaseManagedTextureObjects( r.texture );
			m_freeTextureHandles.push_back( r.handle );
		}
		else
			m_retiredTextures[kept++] = r;
	}
	m_retiredTextures.resize( kept );
}

int CVulkanContext::CreateRenderTargetTexture( int width, int height, std::string *outError )
{
	if ( !IsValid() || m_renderPassTarget == VK_NULL_HANDLE )
	{
		SetError( outError, "CreateRenderTargetTexture before the render passes exist" );
		return -1;
	}
	// The swapchain format keeps the target render-pass compatible with every
	// pipeline; it must also be blittable, since frame copies scale between them.
	VkFormatProperties fp = {};
	vkGetPhysicalDeviceFormatProperties( m_physicalDevice, m_swapFormat, &fp );
	const VkFormatFeatureFlags needed = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
	                                    VK_FORMAT_FEATURE_BLIT_SRC_BIT |
	                                    VK_FORMAT_FEATURE_BLIT_DST_BIT;
	if ( ( fp.optimalTilingFeatures & needed ) != needed )
	{
		SetError( outError, "swapchain format cannot back a render-target texture" );
		return -1;
	}
	const int handle = CreateManagedTexture( width, height, m_swapFormat, outError,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 1,
	    m_srgbAttachments ? m_swapFormatSrgb : VK_FORMAT_UNDEFINED );
	if ( handle < 0 )
		return -1;
	ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];

	VkImageCreateInfo di = {};
	di.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	di.imageType = VK_IMAGE_TYPE_2D;
	di.format = m_depthFormat;
	di.extent = { t.width, t.height, 1 };
	di.mipLevels = 1;
	di.arrayLayers = 1;
	di.samples = VK_SAMPLE_COUNT_1_BIT;
	di.tiling = VK_IMAGE_TILING_OPTIMAL;
	di.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	di.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	di.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if ( vkCreateImage( m_device, &di, nullptr, &t.depthImage ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateImage (render-target depth) failed" );
		return -1;
	}
	VkMemoryRequirements req = {};
	vkGetImageMemoryRequirements( m_device, t.depthImage, &req );
	bool found = false;
	const uint32_t type =
	    FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &found );
	VkMemoryAllocateInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = req.size;
	ai.memoryTypeIndex = type;
	if ( !found || vkAllocateMemory( m_device, &ai, nullptr, &t.depthMemory ) != VK_SUCCESS )
	{
		SetError( outError, "no device memory for render-target depth" );
		return -1;
	}
	vkBindImageMemory( m_device, t.depthImage, t.depthMemory, 0 );

	VkImageViewCreateInfo dv = {};
	dv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	dv.image = t.depthImage;
	dv.viewType = VK_IMAGE_VIEW_TYPE_2D;
	dv.format = m_depthFormat;
	dv.subresourceRange = { m_depthAspects, 0, 1, 0, 1 };
	if ( vkCreateImageView( m_device, &dv, nullptr, &t.depthView ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateImageView (render-target depth) failed" );
		return -1;
	}

	VkImageView attachments[] = { t.view, t.depthView };
	VkFramebufferCreateInfo fb = {};
	fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb.renderPass = m_renderPassTarget;
	fb.attachmentCount = 2;
	fb.pAttachments = attachments;
	fb.width = t.width;
	fb.height = t.height;
	fb.layers = 1;
	if ( vkCreateFramebuffer( m_device, &fb, nullptr, &t.framebuffer ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateFramebuffer (render target) failed" );
		return -1;
	}
	if ( t.srgbView != VK_NULL_HANDLE && m_srgbAttachments )
	{
		VkImageView srgbAttachments[] = { t.srgbView, t.depthView };
		fb.renderPass = m_renderPassTargetSrgb;
		fb.pAttachments = srgbAttachments;
		if ( vkCreateFramebuffer( m_device, &fb, nullptr, &t.framebufferSrgb ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateFramebuffer (render target, sRGB) failed" );
			return -1;
		}
	}

	// Bring both images to the layouts m_renderPassTarget expects on entry, with
	// defined contents: a target sampled before anything renders into it reads
	// opaque black, as a freshly created D3D9 render target does in practice.
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( !BeginSingleTimeCommands( &cmd, outError ) )
		return -1;
	VkImageMemoryBarrier toDst[2] = {};
	for ( VkImageMemoryBarrier &b : toDst )
	{
		b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	toDst[0].image = t.image;
	toDst[0].subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	toDst[1].image = t.depthImage;
	toDst[1].subresourceRange = { m_depthAspects, 0, 1, 0, 1 };
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
	    0, nullptr, 0, nullptr, 2, toDst );
	const VkClearColorValue black = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	vkCmdClearColorImage(
	    cmd, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &black, 1, &toDst[0].subresourceRange );
	const VkClearDepthStencilValue far = { 1.0f, 0 };
	vkCmdClearDepthStencilImage( cmd, t.depthImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &far, 1,
	    &toDst[1].subresourceRange );
	VkImageMemoryBarrier ready[2] = { toDst[0], toDst[1] };
	for ( VkImageMemoryBarrier &b : ready )
	{
		b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		b.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	ready[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ready[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	ready[1].newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	ready[1].dstAccessMask =
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0,
	    nullptr, 0, nullptr, 2, ready );
	if ( !EndSingleTimeCommands( cmd, outError ) )
		return -1;

	t.renderTarget = true;
	t.uploaded = true;
	return handle;
}

void CVulkanContext::SetManagedTextureSamplerState( int handle, int samplerState )
{
	if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) || samplerState < 0 ||
	     samplerState >= kSamplerStates )
		return;
	ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
	if ( t.samplerState == samplerState )
		return;
	t.samplerState = samplerState;
	if ( t.descSet == VK_NULL_HANDLE || m_samplers[samplerState] == VK_NULL_HANDLE )
		return;
	// Source sets sampler state while it creates the texture, before any frame
	// samples it. A later change must not rewrite a set that a submitted frame
	// may still be reading, so let the device finish first.
	vkDeviceWaitIdle( m_device );
	for ( int srgb = 0; srgb < 2; ++srgb )
	{
		const VkDescriptorSet set = srgb ? t.descSetSrgb : t.descSet;
		if ( set == VK_NULL_HANDLE )
			continue;
		VkDescriptorImageInfo dii = {};
		dii.sampler = m_samplers[samplerState];
		dii.imageView = srgb ? t.srgbView : t.view;
		dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = set;
		wds.dstBinding = 0;
		wds.descriptorCount = 1;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wds.pImageInfo = &dii;
		vkUpdateDescriptorSets( m_device, 1, &wds, 0, nullptr );
	}
}

bool CVulkanContext::UploadManagedTexture(
    int handle, const uint8_t *data, size_t dataSize, std::string *outError, uint32_t level )
{
	if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
	{
		SetError( outError, "UploadManagedTexture with invalid handle/data" );
		return false;
	}
	const ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
	return UploadManagedTextureRegion( handle, 0, 0, std::max( 1u, t.width >> level ),
	    std::max( 1u, t.height >> level ), data, dataSize, outError, level );
}

static bool IsBlockCompressedFormat( VkFormat format )
{
	switch ( format )
	{
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
		return true;
	default:
		return false;
	}
}

bool CVulkanContext::UploadManagedTextureRegion( int handle, uint32_t x, uint32_t y, uint32_t width,
    uint32_t height, const uint8_t *data, size_t dataSize, std::string *outError, uint32_t level )
{
	if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) || !data ||
	     dataSize == 0 || width == 0 || height == 0 )
	{
		SetError( outError, "UploadManagedTexture with invalid handle/data" );
		return false;
	}
	ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
	// A render target's contents come from rendering; its image is in the
	// swapchain format, which caller pixel data does not match.
	if ( t.renderTarget )
		return true;
	if ( level >= t.mipLevels )
	{
		SetError( outError, "UploadManagedTexture past the texture's mip chain" );
		return false;
	}
	const uint32_t levelWidth = std::max( 1u, t.width >> level );
	const uint32_t levelHeight = std::max( 1u, t.height >> level );
	if ( x >= levelWidth || y >= levelHeight || width > levelWidth - x || height > levelHeight - y )
	{
		SetError( outError, "UploadManagedTexture region outside the level" );
		return false;
	}
	const bool whole = x == 0 && y == 0 && width == levelWidth && height == levelHeight;
	// A block-compressed region must start on a 4x4 block and end on one or at
	// the level's edge (vkCmdCopyBufferToImage's rule for compressed images).
	const bool compressed = IsBlockCompressedFormat( t.format );
	if ( compressed && ( x % 4 || y % 4 || ( ( x + width ) % 4 && x + width != levelWidth ) ||
	                       ( ( y + height ) % 4 && y + height != levelHeight ) ) )
	{
		SetError( outError, "UploadManagedTexture: compressed region not block aligned" );
		return false;
	}
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
	// A single-level image replaced whole discards its old contents. A level of a
	// chain (made samplable at creation) and an image updated in part (VGUI's
	// font pages, a glyph at a time) keep the texels outside the region.
	const bool preserve = t.mipLevels > 1 || ( !whole && t.uploaded );
	toDst.oldLayout =
	    preserve ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
	toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.image = t.image;
	toDst.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, level, 1, 0, 1 };
	toDst.srcAccessMask = preserve ? VK_ACCESS_SHADER_READ_BIT : 0;
	toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd,
	    preserve ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toDst );
	// A part of a never-filled single-level image: the rest reads as zero rather
	// than undefined memory.
	if ( !whole && !preserve && !compressed )
	{
		const VkClearColorValue zero = {};
		const VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, level, 1, 0, 1 };
		vkCmdClearColorImage(
		    cmd, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &zero, 1, &range );
		VkMemoryBarrier cleared = {};
		cleared.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		cleared.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		cleared.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    0, 1, &cleared, 0, nullptr, 0, nullptr );
	}
	VkBufferImageCopy copy = {};
	copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1 };
	copy.imageOffset = { static_cast<int32_t>( x ), static_cast<int32_t>( y ), 0 };
	copy.imageExtent = { width, height, 1 };
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
	if ( ok && level == 0 )
		t.uploaded = true;

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

CVulkanContext::DynDraw &CVulkanContext::AppendRecord( int kind )
{
	// The first record after a present starts the next frame. Discarding the
	// presented frame here rather than at Present keeps it available for an
	// on-demand capture (ReadPixels) until the engine starts drawing again.
	if ( m_dynFramePresented )
		ClearDynamicQueue();
	DynDraw d;
	d.kind = kind;
	d.target = m_dynTarget;
	d.tag = m_dynTag;
	std::memcpy( d.viewport, m_dynViewport, sizeof( d.viewport ) );
	d.scissorEnabled = m_dynScissorEnabled;
	std::memcpy( d.scissor, m_dynScissor, sizeof( d.scissor ) );
	m_dynDrawRecords.push_back( d );
	return m_dynDrawRecords.back();
}

void CVulkanContext::SetRenderTarget( int handle )
{
	m_dynTarget = IsRenderTargetTexture( handle ) ? handle : -1;
	m_dynViewport[0] = m_dynViewport[1] = m_dynViewport[2] = m_dynViewport[3] = 0.0f;
	m_dynViewport[4] = 0.0f;
	m_dynViewport[5] = 1.0f;
}

void CVulkanContext::SetViewport( int x, int y, int width, int height, float minZ, float maxZ )
{
	m_dynViewport[0] = static_cast<float>( x );
	m_dynViewport[1] = static_cast<float>( y );
	m_dynViewport[2] = static_cast<float>( width );
	m_dynViewport[3] = static_cast<float>( height );
	m_dynViewport[4] = minZ;
	m_dynViewport[5] = maxZ;
}

void CVulkanContext::SetScissor( bool enable, int x, int y, int width, int height )
{
	m_dynScissorEnabled = enable;
	m_dynScissor[0] = x;
	m_dynScissor[1] = y;
	m_dynScissor[2] = width;
	m_dynScissor[3] = height;
}

void CVulkanContext::QueueClear( bool color, bool depth, bool stencil )
{
	stencil = stencil && m_stencilBits > 0;
	if ( !color && !depth && !stencil )
		return;
	DynDraw &d = AppendRecord( kRecordClear );
	d.clearColor = color;
	d.clearDepth = depth;
	d.clearStencil = stencil;
	for ( int i = 0; i < 4; ++i )
		d.clearValue[i] = m_clearColor.float32[i];
}

bool CVulkanContext::QueueCopyToTexture( int dstHandle, const int *srcRect, const int *dstRect )
{
	if ( !IsRenderTargetTexture( dstHandle ) || dstHandle == m_dynTarget )
		return false;
	DynDraw &d = AppendRecord( kRecordCopy );
	d.copyDst = dstHandle;
	if ( srcRect )
		std::memcpy( d.copySrcRect, srcRect, sizeof( d.copySrcRect ) );
	if ( dstRect )
		std::memcpy( d.copyDstRect, dstRect, sizeof( d.copyDstRect ) );
	return true;
}

int CVulkanContext::CreateOcclusionQuery( std::string *outError )
{
	if ( !IsValid() )
	{
		SetError( outError, "occlusion query on an invalid context" );
		return -1;
	}
	if ( !m_preciseOcclusion )
	{
		SetError( outError, "the device cannot count occlusion samples exactly "
		                    "(occlusionQueryPrecise is not supported)" );
		return -1;
	}
	if ( m_queryPool == VK_NULL_HANDLE )
	{
		VkQueryPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		info.queryType = VK_QUERY_TYPE_OCCLUSION;
		info.queryCount = kMaxOcclusionQueries;
		const VkResult r = vkCreateQueryPool( m_device, &info, nullptr, &m_queryPool );
		if ( r != VK_SUCCESS )
		{
			SetError( outError, std::string( "vkCreateQueryPool failed: " ) + ResultString( r ) );
			return -1;
		}
		m_querySlots.assign( kMaxOcclusionQueries, OcclusionQuerySlot() );
	}
	for ( size_t slot = 0; slot < m_querySlots.size(); ++slot )
	{
		if ( !m_querySlots[slot].live )
		{
			m_querySlots[slot] = OcclusionQuerySlot();
			m_querySlots[slot].live = true;
			return static_cast<int>( slot );
		}
	}
	SetError( outError, "all occlusion query slots are in use" );
	return -1;
}

void CVulkanContext::DestroyOcclusionQuery( int query )
{
	// The slot's pool entry is reset before any later reuse records into it.
	if ( query >= 0 && query < static_cast<int>( m_querySlots.size() ) )
		m_querySlots[static_cast<size_t>( query )].live = false;
}

void CVulkanContext::QueueBeginOcclusionQuery( int query )
{
	if ( query < 0 || query >= static_cast<int>( m_querySlots.size() ) ||
	     !m_querySlots[static_cast<size_t>( query )].live )
		return;
	DynDraw &d = AppendRecord( kRecordQueryBegin );
	OcclusionQuerySlot &slot = m_querySlots[static_cast<size_t>( query )];
	slot.failed = false;
	d.query = query;
	d.querySerial = ++slot.issued;
}

void CVulkanContext::QueueEndOcclusionQuery( int query )
{
	if ( query < 0 || query >= static_cast<int>( m_querySlots.size() ) )
		return;
	AppendRecord( kRecordQueryEnd ).query = query;
}

int64_t CVulkanContext::OcclusionQueryResult( int query, bool wait )
{
	if ( query < 0 || query >= static_cast<int>( m_querySlots.size() ) )
		return kQueryFailed;
	const OcclusionQuerySlot &slot = m_querySlots[static_cast<size_t>( query )];
	if ( !slot.live || slot.failed || slot.issued == 0 )
		return kQueryFailed;
	if ( slot.submitted != slot.issued )
	{
		// Still in the frame being recorded: nothing can be waited for until it
		// is submitted at present, so a caller that must have an answer now gets
		// a failure rather than a wait that never ends.
		return wait ? kQueryFailed : kQueryPending;
	}
	uint64_t result[2] = { 0, 0 }; // samples passed, availability
	VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
	if ( wait )
		flags |= VK_QUERY_RESULT_WAIT_BIT;
	const VkResult r = vkGetQueryPoolResults( m_device, m_queryPool, static_cast<uint32_t>( query ),
	    1, sizeof( result ), result, sizeof( result ), flags );
	if ( r != VK_SUCCESS && r != VK_NOT_READY )
		return kQueryFailed;
	if ( !result[1] )
		return kQueryPending;
	return static_cast<int64_t>( result[0] );
}

void CVulkanContext::FailUnsubmittedQueries()
{
	// The stream is being discarded: an issue it holds that no frame submitted
	// will never produce a result.
	for ( const DynDraw &d : m_dynDrawRecords )
	{
		if ( d.kind != kRecordQueryBegin || d.query < 0 ||
		     d.query >= static_cast<int>( m_querySlots.size() ) )
			continue;
		OcclusionQuerySlot &slot = m_querySlots[static_cast<size_t>( d.query )];
		if ( slot.issued == d.querySerial && slot.submitted != d.querySerial )
			slot.failed = true;
	}
}

void CVulkanContext::QueueDynamicTriangles( const float *posColorInterleaved, uint32_t vertexCount,
    const float *lightmapUv, const float *normalTangent, const float *vertexAlpha )
{
	if ( !posColorInterleaved || vertexCount == 0 )
		return;
	// Record this draw with the state current right now (the engine sets the
	// transform/shader/constant per object before each Draw). 8 floats/vertex:
	// position (3) + color (3) + uv (2).
	DynDraw &d = AppendRecord( kRecordDraw );
	d.firstVertex = static_cast<uint32_t>( m_dynQueued.size() / kDynVertexFloats );
	d.vertexCount = vertexCount;
	d.shaderIndex = m_dynShaderIndex;
	std::memcpy( d.transform, m_dynTransform, sizeof( d.transform ) );
	std::memcpy( d.color, m_dynConstColor, sizeof( d.color ) );
	std::memcpy( d.modulation, m_dynModulation, sizeof( d.modulation ) );
	std::memcpy( d.texXform0, m_dynTexXform0, sizeof( d.texXform0 ) );
	std::memcpy( d.texXform1, m_dynTexXform1, sizeof( d.texXform1 ) );
	d.raster = m_dynRaster;
	d.alphaRef = m_dynAlphaRef;
	d.texHandle = m_dynBoundTexHandle;
	d.lightmapHandle = m_dynLightmapHandle;
	d.colorFlags = m_dynColorFlags;
	d.outputScale = m_dynOutputScale;
	d.stencilRef = m_dynStencilRef;
	d.stencilTestMask = m_dynStencilTestMask;
	d.stencilWriteMask = m_dynStencilWriteMask;
	d.clipPlaneCount = m_clipPlanesSupported ? m_dynClipPlaneCount : 0;
	std::memcpy( d.clipPlanes, m_dynClipPlanes, sizeof( d.clipPlanes ) );
	std::memcpy( d.samplerHandles, m_dynSamplerHandles, sizeof( d.samplerHandles ) );
	d.portal = m_dynPortal;
	if ( d.shaderIndex == kDynShaderSkin )
	{
		d.skin = static_cast<int>( m_dynSkinConstants.size() );
		m_dynSkinConstants.push_back( m_dynSkin );
	}
	m_dynQueued.reserve(
	    m_dynQueued.size() + static_cast<size_t>( vertexCount ) * kDynVertexFloats );
	for ( uint32_t v = 0; v < vertexCount; ++v )
	{
		const float *vertex = posColorInterleaved + static_cast<size_t>( v ) * 8;
		m_dynQueued.insert( m_dynQueued.end(), vertex, vertex + 8 );
		m_dynQueued.push_back( lightmapUv ? lightmapUv[v * 2 + 0] : 0.0f );
		m_dynQueued.push_back( lightmapUv ? lightmapUv[v * 2 + 1] : 0.0f );
		if ( normalTangent )
		{
			const float *nt = normalTangent + static_cast<size_t>( v ) * 7;
			m_dynQueued.insert( m_dynQueued.end(), nt, nt + 7 );
		}
		else
			m_dynQueued.insert( m_dynQueued.end(), 7, 0.0f );
		m_dynQueued.push_back( vertexAlpha ? vertexAlpha[v] : 1.0f );
	}
}

std::string CVulkanContext::DescribeStream() const
{
	struct Totals
	{
		int target;
		size_t draws, clears, copies, vertices;
	};
	std::vector<Totals> totals;
	std::string order;
	int last = -2;
	for ( const DynDraw &d : m_dynDrawRecords )
	{
		Totals *t = nullptr;
		for ( Totals &e : totals )
			if ( e.target == d.target )
				t = &e;
		if ( !t )
		{
			totals.push_back( { d.target, 0, 0, 0, 0 } );
			t = &totals.back();
		}
		if ( d.kind == kRecordDraw )
		{
			++t->draws;
			t->vertices += d.vertexCount;
		}
		else if ( d.kind == kRecordClear )
			++t->clears;
		else
			++t->copies;
		if ( d.target != last && order.size() < 400 )
			order += ( d.target < 0 ? std::string( " bb" ) : " " + std::to_string( d.target ) );
		last = d.target;
	}
	std::string out;
	for ( const Totals &t : totals )
	{
		char line[160];
		snprintf( line, sizeof( line ), "target %d: draws=%zu clears=%zu copies=%zu verts=%zu\n",
		    t.target, t.draws, t.clears, t.copies, t.vertices );
		out += line;
	}
	out += "order:" + order + "\n";
	return out;
}

std::vector<CVulkanContext::StreamRecordInfo> CVulkanContext::DescribeStreamRecords() const
{
	std::vector<StreamRecordInfo> out;
	out.reserve( m_dynDrawRecords.size() );
	for ( const DynDraw &d : m_dynDrawRecords )
	{
		StreamRecordInfo info = {};
		info.kind = d.kind;
		info.target = d.target;
		info.tag = d.tag;
		info.clearColor = d.clearColor;
		info.clearDepth = d.clearDepth;
		std::memcpy( info.clearValue, d.clearValue, sizeof( info.clearValue ) );
		info.shaderIndex = d.shaderIndex;
		info.texHandle = d.texHandle;
		info.raster = d.raster;
		info.vertexCount = d.vertexCount;
		std::memcpy( info.modulation, d.modulation, sizeof( info.modulation ) );
		std::memcpy( info.viewport, d.viewport, sizeof( info.viewport ) );
		const size_t base = static_cast<size_t>( d.firstVertex ) * kDynVertexFloats;
		std::memcpy( info.texXform0, d.texXform0, sizeof( info.texXform0 ) );
		std::memcpy( info.texXform1, d.texXform1, sizeof( info.texXform1 ) );
		if ( d.kind == kRecordDraw && base + 6 <= m_dynQueued.size() )
		{
			info.firstColor[0] = m_dynQueued[base + 3];
			info.firstColor[1] = m_dynQueued[base + 4];
			info.firstColor[2] = m_dynQueued[base + 5];
			info.uvMin[0] = info.uvMin[1] = 1e30f;
			info.uvMax[0] = info.uvMax[1] = -1e30f;
			for ( uint32_t v = 0; v < d.vertexCount; ++v )
			{
				const size_t at = base + static_cast<size_t>( v ) * kDynVertexFloats + 6;
				if ( at + 1 >= m_dynQueued.size() )
					break;
				for ( int k = 0; k < 2; ++k )
				{
					info.uvMin[k] = std::min( info.uvMin[k], m_dynQueued[at + k] );
					info.uvMax[k] = std::max( info.uvMax[k], m_dynQueued[at + k] );
				}
			}
		}
		out.push_back( info );
	}
	return out;
}

void CVulkanContext::GetTargetExtent( int target, uint32_t *outW, uint32_t *outH ) const
{
	if ( IsRenderTargetTexture( target ) )
	{
		*outW = m_managedTextures[static_cast<size_t>( target )].width;
		*outH = m_managedTextures[static_cast<size_t>( target )].height;
		return;
	}
	*outW = m_swapExtent.width;
	*outH = m_swapExtent.height;
}

void CVulkanContext::BeginTargetPass( VkCommandBuffer cmd, int target, bool srgb )
{
	// Re-entering a target loads what it already holds; clears arrive as
	// explicit records, exactly where the engine issued them. `srgb` enters it
	// through its sRGB view.
	srgb = srgb && m_srgbAttachments;
	VkRenderPassBeginInfo rp = {};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	if ( IsRenderTargetTexture( target ) )
	{
		const ManagedTexture &t = m_managedTextures[static_cast<size_t>( target )];
		rp.renderPass = srgb ? m_renderPassTargetSrgb : m_renderPassTarget;
		rp.framebuffer = srgb ? t.framebufferSrgb : t.framebuffer;
	}
	else
	{
		rp.renderPass = srgb ? m_renderPassLoadSrgb : m_renderPassLoad;
		rp.framebuffer =
		    srgb ? m_framebuffersSrgb[m_acquiredImage] : m_framebuffers[m_acquiredImage];
	}
	GetTargetExtent( target, &rp.renderArea.extent.width, &rp.renderArea.extent.height );
	vkCmdBeginRenderPass( cmd, &rp, VK_SUBPASS_CONTENTS_INLINE );
}

void CVulkanContext::RecordTargetCopy( VkCommandBuffer cmd, int srcTarget, const DynDraw &copy )
{
	// Called outside any render pass. The source is the swapchain image (resting
	// in COLOR_ATTACHMENT_OPTIMAL between passes) or a render-target texture
	// (resting in SHADER_READ_ONLY); the destination is always a render target.
	const bool srcIsTexture = IsRenderTargetTexture( srcTarget );
	const VkImage srcImage = srcIsTexture
	                             ? m_managedTextures[static_cast<size_t>( srcTarget )].image
	                             : m_swapImages[m_acquiredImage];
	const VkImageLayout srcRest = srcIsTexture ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	                                           : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	const ManagedTexture &dst = m_managedTextures[static_cast<size_t>( copy.copyDst )];

	uint32_t srcW = 0, srcH = 0;
	GetTargetExtent( srcTarget, &srcW, &srcH );
	auto toBlitRegion = []( const int *rect, uint32_t w, uint32_t h, VkOffset3D *offsets )
	{
		int x0 = 0, y0 = 0, x1 = static_cast<int>( w ), y1 = static_cast<int>( h );
		if ( rect[2] > 0 && rect[3] > 0 )
		{
			x0 = std::max( 0, rect[0] );
			y0 = std::max( 0, rect[1] );
			x1 = std::min( static_cast<int>( w ), rect[0] + rect[2] );
			y1 = std::min( static_cast<int>( h ), rect[1] + rect[3] );
		}
		offsets[0] = { x0, y0, 0 };
		offsets[1] = { x1, y1, 1 };
		return x1 > x0 && y1 > y0;
	};
	VkImageBlit blit = {};
	blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	if ( !toBlitRegion( copy.copySrcRect, srcW, srcH, blit.srcOffsets ) ||
	     !toBlitRegion( copy.copyDstRect, dst.width, dst.height, blit.dstOffsets ) )
		return;

	VkImageMemoryBarrier toTransfer[2] = {};
	for ( VkImageMemoryBarrier &b : toTransfer )
	{
		b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		b.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	toTransfer[0].image = srcImage;
	toTransfer[0].oldLayout = srcRest;
	toTransfer[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	toTransfer[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	toTransfer[1].image = dst.image;
	toTransfer[1].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	toTransfer[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toTransfer[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, toTransfer );

	vkCmdBlitImage( cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image,
	    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR );

	VkImageMemoryBarrier back[2] = { toTransfer[0], toTransfer[1] };
	back[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	back[0].newLayout = srcRest;
	back[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	back[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	back[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	back[1].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	back[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	back[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
	        VK_PIPELINE_STAGE_TRANSFER_BIT,
	    0, 0, nullptr, 0, nullptr, 2, back );
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
	for ( const auto &entry : m_dynTexPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_dynTexPipelines.clear();
	for ( const auto &entry : m_portalPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_portalPipelines.clear();
	DestroySkinPipeline();
	for ( VkShaderModule *module : { &m_portalVert, &m_portalFrag } )
	{
		if ( *module != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, *module, nullptr );
		*module = VK_NULL_HANDLE;
	}
	if ( m_portalPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( m_device, m_portalPipelineLayout, nullptr );
		m_portalPipelineLayout = VK_NULL_HANDLE;
	}
	for ( VkPipelineShaderStageCreateInfo &stage : m_texTemplate.stages )
	{
		if ( stage.module != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, stage.module, nullptr );
		stage.module = VK_NULL_HANDLE;
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
	for ( VkSampler &sampler : m_samplers )
	{
		if ( sampler != VK_NULL_HANDLE )
			vkDestroySampler( m_device, sampler, nullptr );
		sampler = VK_NULL_HANDLE;
	}
	m_dynTexSampler = VK_NULL_HANDLE; // one of m_samplers
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
	// The device is idle here: every texture, deleted or not, can go.
	for ( ManagedTexture &t : m_managedTextures )
		ReleaseManagedTextureObjects( t );
	m_managedTextures.clear();
	for ( RetiredTexture &r : m_retiredTextures )
		ReleaseManagedTextureObjects( r.texture );
	m_retiredTextures.clear();
	m_freeTextureHandles.clear();
	m_liveTextureSets = 0;
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
	// A back-buffer size requested while a frame was open applies now, and a
	// drawable that changed size gets a swapchain of its new size.
	int drawable[2] = { 0, 0 };
	SDL_GetWindowSizeInPixels( m_window, &drawable[0], &drawable[1] );
	const bool drawableChanged =
	    drawable[0] != m_presentDrawable[0] || drawable[1] != m_presentDrawable[1];
	const bool backBufferPending = m_requestedBackBuffer.width > 0 &&
	                               m_requestedBackBuffer.height > 0 &&
	                               ( m_requestedBackBuffer.width != m_swapExtent.width ||
	                                   m_requestedBackBuffer.height != m_swapExtent.height );
	if ( ( drawableChanged || backBufferPending ) && !RecreateSwapchain( outError ) )
		return false;
	if ( m_swapchain == VK_NULL_HANDLE )
	{
		// Zero-size (minimized) window: nothing to render this iteration.
		if ( outSkip )
			*outSkip = true;
		return true;
	}

	vkWaitForFences( m_device, 1, &m_inFlight[m_currentFrame], VK_TRUE, UINT64_MAX );
	// One queue completes submissions in order: this slot's is done, so are all
	// before it.
	m_completedSerial = std::max( m_completedSerial, m_slotSerial[m_currentFrame] );
	RetireCompletedTextures();

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

	// Queries are reset outside any render pass, before the stream that issues
	// them replays. Only the slots this stream issues are reset, so a result of
	// an earlier frame that the engine has yet to read survives.
	m_replayedQueries.clear();
	if ( m_queryPool != VK_NULL_HANDLE && m_dynPipeline != VK_NULL_HANDLE )
	{
		for ( const DynDraw &d : m_dynDrawRecords )
		{
			if ( d.kind == kRecordQueryBegin )
				vkCmdResetQueryPool( cmd, m_queryPool, static_cast<uint32_t>( d.query ), 1 );
		}
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
	if ( m_dynPipeline != VK_NULL_HANDLE && !m_dynDrawRecords.empty() )
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

		const bool geometryOk = bufferOk && ( needed == 0 || m_dynMapped );
		if ( geometryOk && needed > 0 )
		{
			std::memcpy( m_dynMapped, m_dynQueued.data(), needed );
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers( cmd, 0, 1, &m_dynVertexBuffer, &offset );
		}

		// Replay the stream in engine order. Each record names the target it was
		// issued against; a change of target closes the open pass and opens the
		// new target's, so a portal view rendered into _rt_portal1 lands there and
		// is finished before the main view samples it.
		// The skin draws' pixel shader constants, in this frame's uniform buffer.
		std::vector<uint32_t> skinOffsets;
		const bool skinConstantsOk = UploadSkinConstants( &skinOffsets );
		int openTarget = -1; // the swapchain pass opened above
		bool openSrgb = false; // entered through the target's sRGB view
		VkPipeline boundPipeline = VK_NULL_HANDLE;
		// A query must begin and end inside one render pass, and only one
		// occlusion query may be active at a time. One that would span a pass
		// boundary is ended there and fails rather than report a partial count.
		int activeQuery = -1;
		const auto endActiveQuery = [&]( bool complete )
		{
			if ( activeQuery < 0 )
				return;
			vkCmdEndQuery( cmd, m_queryPool, static_cast<uint32_t>( activeQuery ) );
			if ( !complete )
				m_querySlots[static_cast<size_t>( activeQuery )].failed = true;
			activeQuery = -1;
		};
		// D3D9 (SRGBWRITEENABLE) encodes an sRGB-writing draw in hardware and
		// blends it in linear space; such a draw is drawn through the target's
		// sRGB view (the hardware decodes, blends and encodes, rounding as D3D9
		// does). Everything else, clears above all, uses the UNORM view, which
		// stores what it is given.
		const auto drawWantsSrgb = [&]( const DynDraw &r )
		{
			return m_srgbAttachments && r.kind == kRecordDraw &&
			       ( r.shaderIndex == kDynShaderTextured ||
			           r.shaderIndex == kDynShaderPortalRefract ||
			           r.shaderIndex == kDynShaderSkin ) &&
			       ( r.colorFlags & kColorSrgbWrite );
		};
		for ( size_t recordIndex = 0; recordIndex < m_dynDrawRecords.size(); ++recordIndex )
		{
			const DynDraw &d = m_dynDrawRecords[recordIndex];
			// A render target deleted after these records were issued: what was
			// rendered into it is discarded, as it would be on D3D9.
			if ( ( d.target >= 0 && !IsRenderTargetTexture( d.target ) ) ||
			     ( d.kind == kRecordCopy && !IsRenderTargetTexture( d.copyDst ) ) )
				continue;
			if ( d.kind == kRecordCopy )
			{
				endActiveQuery( false );
				vkCmdEndRenderPass( cmd );
				RecordTargetCopy( cmd, openTarget, d );
				BeginTargetPass( cmd, openTarget, openSrgb );
				continue;
			}
			// A query begins in the pass its next draw needs, so the query and the
			// draws it counts share one render pass; it ends in the open pass.
			bool wantSrgb = drawWantsSrgb( d );
			if ( d.kind == kRecordQueryEnd )
				wantSrgb = openSrgb;
			else if ( d.kind == kRecordQueryBegin )
			{
				wantSrgb = openSrgb;
				for ( size_t next = recordIndex + 1; next < m_dynDrawRecords.size(); ++next )
				{
					const DynDraw &n = m_dynDrawRecords[next];
					if ( n.kind == kRecordQueryBegin || n.kind == kRecordQueryEnd )
						continue;
					if ( n.target == d.target && n.kind != kRecordCopy )
						wantSrgb = drawWantsSrgb( n );
					break;
				}
			}
			if ( d.target != openTarget || wantSrgb != openSrgb )
			{
				endActiveQuery( false );
				vkCmdEndRenderPass( cmd );
				openTarget = d.target;
				openSrgb = wantSrgb;
				BeginTargetPass( cmd, openTarget, openSrgb );
				boundPipeline = VK_NULL_HANDLE;
			}
			if ( d.kind == kRecordQueryBegin )
			{
				endActiveQuery( false );
				// A slot is reset once per replay, so a second issue of the same
				// query in one frame cannot begin; that issue fails.
				bool reissued = false;
				for ( const std::pair<int, uint64_t> &issue : m_replayedQueries )
					reissued = reissued || issue.first == d.query;
				if ( reissued )
				{
					m_querySlots[static_cast<size_t>( d.query )].failed = true;
					continue;
				}
				vkCmdBeginQuery( cmd, m_queryPool, static_cast<uint32_t>( d.query ),
				    VK_QUERY_CONTROL_PRECISE_BIT );
				activeQuery = d.query;
				m_replayedQueries.emplace_back( d.query, d.querySerial );
				continue;
			}
			if ( d.kind == kRecordQueryEnd )
			{
				if ( d.query == activeQuery )
					endActiveQuery( true );
				continue;
			}

			// D3D9 semantics: the viewport maps clip space and bounds clears; the
			// scissor, when enabled, further bounds rasterization.
			uint32_t targetW = 0, targetH = 0;
			GetTargetExtent( openTarget, &targetW, &targetH );
			VkViewport viewport = {};
			viewport.x = d.viewport[0];
			viewport.y = d.viewport[1];
			viewport.width = d.viewport[2];
			viewport.height = d.viewport[3];
			viewport.minDepth = d.viewport[4];
			viewport.maxDepth = d.viewport[5];
			if ( viewport.width <= 0.0f || viewport.height <= 0.0f )
			{
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = static_cast<float>( targetW );
				viewport.height = static_cast<float>( targetH );
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
			}
			auto clampRect = [&]( float x, float y, float w, float h ) -> VkRect2D
			{
				const int32_t x0 = std::max( 0, static_cast<int32_t>( x ) );
				const int32_t y0 = std::max( 0, static_cast<int32_t>( y ) );
				const int32_t x1 =
				    std::min( static_cast<int32_t>( targetW ), static_cast<int32_t>( x + w ) );
				const int32_t y1 =
				    std::min( static_cast<int32_t>( targetH ), static_cast<int32_t>( y + h ) );
				VkRect2D r = {};
				r.offset = { x0, y0 };
				r.extent = { static_cast<uint32_t>( std::max( 0, x1 - x0 ) ),
				    static_cast<uint32_t>( std::max( 0, y1 - y0 ) ) };
				return r;
			};
			const VkRect2D viewRect =
			    clampRect( viewport.x, viewport.y, viewport.width, viewport.height );

			if ( d.kind == kRecordClear )
			{
				if ( viewRect.extent.width == 0 || viewRect.extent.height == 0 )
					continue;
				VkClearAttachment clears[2] = {};
				uint32_t clearCount = 0;
				if ( d.clearColor )
				{
					clears[clearCount].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					clears[clearCount].colorAttachment = 0;
					for ( int i = 0; i < 4; ++i )
						clears[clearCount].clearValue.color.float32[i] = d.clearValue[i];
					++clearCount;
				}
				if ( d.clearDepth || d.clearStencil )
				{
					clears[clearCount].aspectMask =
					    ( d.clearDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0 ) |
					    ( d.clearStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
					clears[clearCount].clearValue.depthStencil = { 1.0f, 0 };
					++clearCount;
				}
				VkClearRect rect = {};
				rect.rect = viewRect;
				rect.layerCount = 1;
				vkCmdClearAttachments( cmd, clearCount, clears, 1, &rect );
				continue;
			}

			if ( !geometryOk || d.vertexCount == 0 )
				continue;
			VkRect2D scissor = clampRect(
			    0.0f, 0.0f, static_cast<float>( targetW ), static_cast<float>( targetH ) );
			if ( d.scissorEnabled )
				scissor = clampRect( static_cast<float>( d.scissor[0] ),
				    static_cast<float>( d.scissor[1] ), static_cast<float>( d.scissor[2] ),
				    static_cast<float>( d.scissor[3] ) );
			if ( scissor.extent.width == 0 || scissor.extent.height == 0 )
				continue;
			// Material transforms are D3D's: clip-space +Y is the top of the
			// viewport. Vulkan's clip-space Y points down, so the draw viewport is
			// flipped (negative height, core in Vulkan 1.1) to put +Y at the top,
			// for the swapchain and render targets alike. D3D9 also puts pixel
			// centers at integer coordinates where Vulkan puts them at +0.5, so
			// the viewport moves half a pixel right and down: Source's screen-space
			// quads (DrawScreenSpaceRectangle offsets by -0.5 for D3D9) then cover
			// the pixels and sample the texel centers D3D9 does. Clears above
			// address pixels, not clip space, and use the unflipped rectangle.
			VkViewport d3dViewport = viewport;
			d3dViewport.x = viewport.x + 0.5f;
			d3dViewport.y = viewport.y + viewport.height + 0.5f;
			d3dViewport.height = -viewport.height;
			vkCmdSetViewport( cmd, 0, 1, &d3dViewport );
			vkCmdSetScissor( cmd, 0, 1, &scissor );

			VkPipeline selected = m_dynPipeline;
			VkPipelineLayout selectedLayout = m_dynPipelineLayout;
			bool textured = false;
			bool portal = false;
			bool skin = false;
			// sRGB inputs decoded by the sampler and output encoded by the view,
			// which the shader must then not apply itself.
			int decodedFlags = openSrgb ? kColorSrgbWrite : 0;
			if ( d.shaderIndex == kDynShaderGreenify && m_dynPipelineGreen != VK_NULL_HANDLE )
				selected = m_dynPipelineGreen;
			else if ( d.shaderIndex == kDynShaderConstColor &&
			          m_dynPipelineConst != VK_NULL_HANDLE )
				selected = m_dynPipelineConst;
			else if ( d.shaderIndex == kDynShaderTextured )
			{
				// The textured pipeline built for this draw's blend/depth state.
				selected = TexturedPipeline( d.raster, openSrgb );
				if ( selected == VK_NULL_HANDLE )
					continue;
				selectedLayout = m_dynTexPipelineLayout;
				textured = true;
			}
			else if ( d.shaderIndex == kDynShaderPortalRefract )
			{
				selected = PortalPipeline( d.raster, openSrgb );
				if ( selected == VK_NULL_HANDLE )
					continue;
				selectedLayout = m_portalPipelineLayout;
				portal = true;
			}
			else if ( d.shaderIndex == kDynShaderSkin )
			{
				selected = SkinPipeline( d.raster, openSrgb );
				if ( selected == VK_NULL_HANDLE || d.skin < 0 || !skinConstantsOk ||
				     static_cast<size_t>( d.skin ) >= skinOffsets.size() )
					continue;
				selectedLayout = m_skinPipelineLayout;
				skin = true;
			}
			if ( selected != boundPipeline )
			{
				vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, selected );
				boundPipeline = selected;
			}
			if ( textured || portal || skin )
			{
				vkCmdSetStencilCompareMask(
				    cmd, VK_STENCIL_FACE_FRONT_AND_BACK, d.stencilTestMask );
				vkCmdSetStencilWriteMask( cmd, VK_STENCIL_FACE_FRONT_AND_BACK, d.stencilWriteMask );
				vkCmdSetStencilReference( cmd, VK_STENCIL_FACE_FRONT_AND_BACK, d.stencilRef );
			}
			if ( textured || portal || skin )
			{
				// Bind this draw's own texture descriptor set (per-draw texture),
				// falling back to the built-in set when the draw has no managed
				// texture or its set was not allocated. A render target cannot be
				// sampled inside its own pass (it is the color attachment there).
				// The lightmap (set 1) resolves the same way; without one the
				// built-in white set is bound and the shader does not sample it.
				// A sampler the material reads as sRGB samples the texture's sRGB
				// view, which decodes each texel before filtering as D3D9's
				// SRGBTEXTURE does; the shader then skips its own decode for it.
				const auto sampledSet = [&]( int handle, int srgbFlag ) -> VkDescriptorSet
				{
					if ( handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) &&
					     handle != openTarget &&
					     m_managedTextures[static_cast<size_t>( handle )].descSet !=
					         VK_NULL_HANDLE )
					{
						const ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
						if ( ( d.colorFlags & srgbFlag ) && t.descSetSrgb != VK_NULL_HANDLE )
						{
							decodedFlags |= srgbFlag;
							return t.descSetSrgb;
						}
						return t.descSet;
					}
					return m_dynTexDescSet;
				};
				if ( portal )
				{
					const VkDescriptorSet sets[3] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( d.samplerHandles[1], kColorSrgbReadLightmap ),
					    sampledSet( d.samplerHandles[2], kColorSrgbReadSampler2 ) };
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_portalPipelineLayout, 0, 3, sets, 0, nullptr );
				}
				else if ( skin )
				{
					// s0 base (sRGB), s1 specular warp, s2 diffuse warp, s3 normal
					// map, s7 exponent map, s14 self-illumination mask, then this
					// draw's pixel shader constants.
					const VkDescriptorSet sets[7] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( d.samplerHandles[1], 0 ), sampledSet( d.samplerHandles[2], 0 ),
					    sampledSet( d.samplerHandles[3], 0 ), sampledSet( d.samplerHandles[7], 0 ),
					    sampledSet( d.samplerHandles[14], 0 ),
					    m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()].set };
					const uint32_t offset = skinOffsets[static_cast<size_t>( d.skin )];
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_skinPipelineLayout, 0, 7, sets, 1, &offset );
				}
				else
				{
					const VkDescriptorSet sets[2] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( d.lightmapHandle, kColorSrgbReadLightmap ) };
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_dynTexPipelineLayout, 0, 2, sets, 0, nullptr );
				}
			}
			// Push this draw's state. The textured (UnlitGeneric) pipeline reads
			// the faithful Source block { mat4 cModelViewProj; vec4
			// cModulationColor; vec4 cBaseTextureTransform[0]; [1] } (28 floats);
			// the color pipelines read { mat4 mvp; vec4 color } (20 floats).
			// User clip planes follow each block; a plane of zeros keeps everything.
			const auto appendClipPlanes = [&]( float *out )
			{
				for ( int i = 0; i < kMaxClipPlanes; ++i )
					for ( int k = 0; k < 4; ++k )
						out[i * 4 + k] = i < d.clipPlaneCount ? d.clipPlanes[i][k] : 0.0f;
			};
			float pushData[std::max( kPortalPushBytes, kSkinPushBytes ) / sizeof( float )];
			std::memcpy( pushData, d.transform, sizeof( d.transform ) );
			uint32_t pushFloats;
			if ( portal )
			{
				// shaders/portal_refract.vert's block.
				const PortalConstants &c = d.portal;
				std::memcpy( pushData, c.model, sizeof( c.model ) );
				std::memcpy( pushData + 16, c.viewProj, sizeof( c.viewProj ) );
				std::memcpy( pushData + 32, c.texXform0, sizeof( c.texXform0 ) );
				std::memcpy( pushData + 36, c.texXform1, sizeof( c.texXform1 ) );
				pushData[40] = c.time;
				pushData[41] = c.openAmount;
				pushData[42] = c.active;
				pushData[43] = c.colorScale;
				pushData[44] = d.alphaRef;
				pushData[45] = static_cast<float>( c.stage );
				pushData[46] = static_cast<float>( d.colorFlags & ~decodedFlags );
				pushData[47] = d.outputScale;
				appendClipPlanes( pushData + 48 );
				pushFloats = kPortalPushBytes / sizeof( float );
			}
			else if ( skin )
			{
				// shaders/skin.vert's block.
				const SkinConstants &c = m_dynSkinConstants[static_cast<size_t>( d.skin )];
				std::memcpy( pushData, c.viewProj, sizeof( c.viewProj ) );
				std::memcpy( pushData + 16, c.texXform0, sizeof( c.texXform0 ) );
				std::memcpy( pushData + 20, c.texXform1, sizeof( c.texXform1 ) );
				std::memcpy( pushData + 24, c.eyePos, sizeof( c.eyePos ) );
				pushData[28] = d.alphaRef;
				pushData[29] = static_cast<float>( c.combos );
				pushData[30] = static_cast<float>( d.colorFlags & ~decodedFlags );
				pushData[31] = d.outputScale;
				pushData[32] = static_cast<float>( c.numLights );
				pushData[33] = pushData[34] = pushData[35] = 0.0f;
				appendClipPlanes( pushData + 36 );
				pushFloats = kSkinPushBytes / sizeof( float );
			}
			else if ( textured )
			{
				// luminance_compare_ps2x reads c0 (the constant color) where the
				// other variants read cModulationColor.
				const float *modulation =
				    ( d.colorFlags & kFragmentLuminanceCompare ) ? d.color : d.modulation;
				std::memcpy( pushData + 16, modulation, sizeof( d.modulation ) );
				std::memcpy( pushData + 20, d.texXform0, sizeof( d.texXform0 ) );
				std::memcpy( pushData + 24, d.texXform1, sizeof( d.texXform1 ) );
				pushData[28] = d.alphaRef; // alphaParams.x
				// alphaParams.y: multiply by the lightmap; .z: kColorSrgb* flags.
				pushData[29] = d.lightmapHandle >= 0 ? 1.0f : 0.0f;
				pushData[30] = static_cast<float>( d.colorFlags & ~decodedFlags );
				pushData[31] = d.outputScale; // alphaParams.w
				pushFloats = 32;
				if ( m_clipPlanesSupported )
				{
					appendClipPlanes( pushData + 32 );
					pushFloats = kTexturedPushBytes / sizeof( float );
				}
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
		endActiveQuery( false );
		// EndFrame closes the swapchain pass and transitions the image for
		// present or capture, so the frame must end inside it.
		if ( openTarget != -1 || openSrgb )
		{
			vkCmdEndRenderPass( cmd );
			BeginTargetPass( cmd, -1 );
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

	// Capture image -> GENERAL for host read. The back buffer stays a transfer
	// source for the present blit.
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
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0,
	    nullptr, 0, nullptr, 1, &capGeneral );
	return true;
}

void CVulkanContext::RecordPresentBlit(
    VkCommandBuffer cmd, uint32_t imageIndex, VkImageLayout backBufferLayout, bool capture )
{
	VkImageMemoryBarrier pre[2] = {};
	for ( VkImageMemoryBarrier &b : pre )
	{
		b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	}
	// The back buffer: its color writes (or the capture's read) done, as a source.
	pre[0].oldLayout = backBufferLayout;
	pre[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	pre[0].image = m_swapImages[imageIndex];
	pre[0].srcAccessMask = backBufferLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	                           ? VK_ACCESS_TRANSFER_READ_BIT
	                           : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	pre[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	// The swapchain image, available once the acquire semaphore's wait (at the
	// transfer stage) is satisfied; its old contents are not needed.
	pre[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	pre[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	pre[1].image = m_presentImages[imageIndex];
	pre[1].srcAccessMask = 0;
	pre[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, pre );

	VkImageBlit blit = {};
	blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	blit.srcOffsets[1] = { static_cast<int32_t>( m_swapExtent.width ),
	    static_cast<int32_t>( m_swapExtent.height ), 1 };
	blit.dstOffsets[1] = { static_cast<int32_t>( m_presentExtent.width ),
	    static_cast<int32_t>( m_presentExtent.height ), 1 };
	const bool sameSize = m_swapExtent.width == m_presentExtent.width &&
	                      m_swapExtent.height == m_presentExtent.height;
	++m_presentCount;
	if ( !sameSize )
		++m_scaledPresentCount;
	vkCmdBlitImage( cmd, m_swapImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	    m_presentImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
	    sameSize ? VK_FILTER_NEAREST : m_presentFilter );

	VkImageMemoryBarrier toPresent = pre[1];
	toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	toPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	toPresent.dstAccessMask = 0;
	if ( capture )
	{
		// What the window shows: the swapchain image after the blit.
		VkImageMemoryBarrier copyIn[2] = { pre[1], pre[1] };
		copyIn[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		copyIn[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		copyIn[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		copyIn[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		copyIn[1].image = m_captureImage;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    0, 0, nullptr, 0, nullptr, 2, copyIn );
		VkImageCopy copy = {};
		copy.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy.extent = { m_presentExtent.width, m_presentExtent.height, 1 };
		vkCmdCopyImage( cmd, m_presentImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		    m_captureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy );
		VkImageMemoryBarrier capGeneral = copyIn[1];
		capGeneral.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		capGeneral.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		capGeneral.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		capGeneral.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0,
		    nullptr, 0, nullptr, 1, &capGeneral );
		toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		toPresent.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	    0, 0, nullptr, 0, nullptr, 1, &toPresent );
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

	// The back buffer is captured (what ReadPixels reads), or on request the
	// swapchain image the window presents.
	bool doCapture = m_captureRequested && ( !m_capturePresented || m_presentCapturable );
	const bool capturePresented = doCapture && m_capturePresented;
	const bool captureBackBuffer = doCapture && !m_capturePresented;
	if ( doCapture &&
	     !CreateCaptureImage( capturePresented ? m_presentExtent : m_swapExtent, outError ) )
		return false;
	if ( captureBackBuffer && !RecordCapture( cmd, imageIndex ) )
		return false;
	RecordPresentBlit( cmd, imageIndex,
	    captureBackBuffer ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	                      : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	    capturePresented );
	m_captureRequested = m_capturePresented = false;

	VkResult r = vkEndCommandBuffer( cmd );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkEndCommandBuffer failed: " ) + ResultString( r ) );
		return false;
	}

	// Only the present blit touches the acquired swapchain image; rendering into
	// the back buffer need not wait for the acquire.
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
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
	if ( r == VK_SUCCESS )
		m_slotSerial[m_currentFrame] = ++m_submitSerial;
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkQueueSubmit failed: " ) + ResultString( r ) );
		return false;
	}
	// The queries this frame replayed now have results on their way.
	for ( const std::pair<int, uint64_t> &issue : m_replayedQueries )
		m_querySlots[static_cast<size_t>( issue.first )].submitted = issue.second;
	m_replayedQueries.clear();

	// If we captured, resolve the pixels now: the submission must complete
	// before the host-visible copy is valid to read.
	if ( doCapture )
	{
		vkWaitForFences( m_device, 1, &m_inFlight[m_currentFrame], VK_TRUE, UINT64_MAX );
		if ( !ResolveCapturedPixels( outError ) )
			return false;
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
	for ( VkFramebuffer fb : m_framebuffersSrgb )
		if ( fb != VK_NULL_HANDLE )
			vkDestroyFramebuffer( m_device, fb, nullptr );
	m_framebuffersSrgb.clear();

	for ( VkImageView iv : m_swapImageViews )
		if ( iv != VK_NULL_HANDLE )
			vkDestroyImageView( m_device, iv, nullptr );
	m_swapImageViews.clear();
	for ( VkImageView iv : m_swapImageViewsSrgb )
		if ( iv != VK_NULL_HANDLE )
			vkDestroyImageView( m_device, iv, nullptr );
	m_swapImageViewsSrgb.clear();

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

	for ( VkImage img : m_swapImages )
		if ( img != VK_NULL_HANDLE )
			vkDestroyImage( m_device, img, nullptr );
	m_swapImages.clear();
	for ( VkDeviceMemory mem : m_backBufferMemories )
		if ( mem != VK_NULL_HANDLE )
			vkFreeMemory( m_device, mem, nullptr );
	m_backBufferMemories.clear();
	m_presentImages.clear();
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
	m_completedSerial = m_submitSerial;
	RetireCompletedTextures();
	// The old swapchain keeps presenting until the new one replaces it.
	const VkSwapchainKHR oldSwapchain = m_swapchain;
	m_swapchain = VK_NULL_HANDLE;
	DestroySwapchainObjects();

	const bool created = CreateSwapchain( outError, oldSwapchain );
	if ( oldSwapchain != VK_NULL_HANDLE )
		vkDestroySwapchainKHR( m_device, oldSwapchain, nullptr );
	if ( !created )
		return false;
	if ( m_swapchain == VK_NULL_HANDLE )
		return true; // zero-size; retained without targets

	if ( !CreateFramebuffers( outError ) )
		return false;
	return true;
}

bool CVulkanContext::SetBackBufferSize( int width, int height, std::string *outError )
{
	const VkExtent2D requested = { static_cast<uint32_t>( std::max( 0, width ) ),
	    static_cast<uint32_t>( std::max( 0, height ) ) };
	const bool changed = requested.width != m_requestedBackBuffer.width ||
	                     requested.height != m_requestedBackBuffer.height;
	m_requestedBackBuffer = requested;
	if ( !IsValid() || !changed || m_frameOpen )
		return true;
	const VkExtent2D current = m_swapExtent;
	const VkExtent2D target =
	    ( requested.width > 0 && requested.height > 0 ) ? requested : m_presentExtent;
	if ( m_swapchain != VK_NULL_HANDLE && target.width == current.width &&
	     target.height == current.height )
		return true;
	return RecreateSwapchain( outError );
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
		if ( m_queryPool != VK_NULL_HANDLE )
		{
			vkDestroyQueryPool( m_device, m_queryPool, nullptr );
			m_queryPool = VK_NULL_HANDLE;
		}
		m_querySlots.clear();
		m_replayedQueries.clear();

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

		for ( VkRenderPass *pass : { &m_renderPass, &m_renderPassLoad, &m_renderPassTarget,
		          &m_renderPassLoadSrgb, &m_renderPassTargetSrgb } )
		{
			if ( *pass != VK_NULL_HANDLE )
			{
				vkDestroyRenderPass( m_device, *pass, nullptr );
				*pass = VK_NULL_HANDLE;
			}
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
	m_presentExtent = { 0, 0 };
}

} // namespace render_vulkan
