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
	case VK_SUCCESS: return "VK_SUCCESS";
	case VK_NOT_READY: return "VK_NOT_READY";
	case VK_TIMEOUT: return "VK_TIMEOUT";
	case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	default: return "VK_ERROR_<other>";
	}
}

} // namespace

VkBool32 VulkanDebugCallbackTrampoline(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT /*type*/,
	const VkDebugUtilsMessengerCallbackDataEXT *data,
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

bool CVulkanContext::Init( SDL_Window *window, const VulkanContextConfig &config, std::string *outError )
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
	m_framesInFlight = std::max<uint32_t>( 1, std::min<uint32_t>( config.framesInFlight, kMaxFramesInFlight ) );
	m_validationErrorCount = 0;

	if ( !CreateInstance( outError ) ||
		 !SetupDebugMessenger( outError ) ||
		 !CreateSurface( outError ) ||
		 !PickPhysicalDevice( outError ) ||
		 !CreateLogicalDevice( outError ) ||
		 !CreateSwapchain( outError ) ||
		 !CreateRenderPass( outError ) ||
		 !CreateFramebuffers( outError ) ||
		 !CreateCommandResources( outError ) ||
		 !CreateSyncObjects( outError ) )
	{
		Shutdown();
		return false;
	}

	Log( "device '%s' vendor=0x%04x %s validation=%s %ux%u images=%zu\n",
		m_deviceName.c_str(), m_vendorId, m_isDiscrete ? "discrete" : "integrated/other",
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
		SetError( outError, std::string( "SDL_Vulkan_GetInstanceExtensions failed: " ) + SDL_GetError() );
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
				SetError( outError, "required validation layer VK_LAYER_KHRONOS_validation is not available" );
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
		SetError( outError, std::string( "vkCreateDebugUtilsMessengerEXT failed: " ) + ResultString( r ) );
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
		SetError( outError, m_config.requireDiscreteGpu
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
		SetError( outError, std::string( "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed: " ) + ResultString( r ) );
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
		if ( f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
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
		vkGetPhysicalDeviceSurfacePresentModesKHR( m_physicalDevice, m_surface, &pmCount, modes.data() );
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
		VkExtent2D e = { static_cast<uint32_t>( std::max( 0, w ) ), static_cast<uint32_t>( std::max( 0, h ) ) };
		e.width = std::max( caps.minImageExtent.width, std::min( caps.maxImageExtent.width, e.width ) );
		e.height = std::max( caps.minImageExtent.height, std::min( caps.maxImageExtent.height, e.height ) );
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

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;

	VkSubpassDependency dep = {};
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = 0;
	dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.srcAccessMask = 0;
	dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo rp = {};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp.attachmentCount = 1;
	rp.pAttachments = &color;
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
		VkImageView attachments[] = { m_swapImageViews[i] };
		VkFramebufferCreateInfo fb = {};
		fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb.renderPass = m_renderPass;
		fb.attachmentCount = 1;
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
		SetError( outError, std::string( "vkAllocateCommandBuffers failed: " ) + ResultString( r ) );
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

uint32_t CVulkanContext::FindMemoryType( uint32_t typeBits, VkMemoryPropertyFlags props, bool *found ) const
{
	VkPhysicalDeviceMemoryProperties mem = {};
	vkGetPhysicalDeviceMemoryProperties( m_physicalDevice, &mem );
	for ( uint32_t i = 0; i < mem.memoryTypeCount; ++i )
	{
		if ( ( typeBits & ( 1u << i ) ) &&
			 ( mem.memoryTypes[i].propertyFlags & props ) == props )
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
	if ( m_captureImage != VK_NULL_HANDLE &&
		 m_captureExtent.width == m_swapExtent.width &&
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
		SetError( outError, std::string( "vkAllocateMemory (capture) failed: " ) + ResultString( r ) );
		return false;
	}
	vkBindImageMemory( m_device, m_captureImage, m_captureMemory, 0 );
	m_captureExtent = m_swapExtent;
	return true;
}

bool CVulkanContext::CreateShaderModule( const uint32_t *code, size_t sizeBytes,
	VkShaderModule *outModule, std::string *outError )
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
	VkMemoryPropertyFlags props, VkBuffer *outBuffer, VkDeviceMemory *outMemory, std::string *outError )
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
		SetError( outError, std::string( "vkAllocateMemory (buffer) failed: " ) + ResultString( r ) );
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
	struct DemoVertex { float pos[2]; float color[3]; };
	const DemoVertex verts[] = {
		{ {  0.0f, -0.7f }, { 0.0f, 1.0f, 0.0f } },
		{ {  0.7f,  0.7f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.7f,  0.7f }, { 0.0f, 1.0f, 0.0f } },
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
	if ( !CreateShaderModule( g_demoTriangleVertSpv, sizeof( g_demoTriangleVertSpv ), &vert, outError ) )
		return false;
	if ( !CreateShaderModule( g_demoTriangleFragSpv, sizeof( g_demoTriangleFragSpv ), &frag, outError ) )
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
	gp.layout = m_demoPipelineLayout;
	gp.renderPass = m_renderPass;
	gp.subpass = 0;

	r = vkCreateGraphicsPipelines( m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_demoPipeline );
	vkDestroyShaderModule( m_device, vert, nullptr );
	vkDestroyShaderModule( m_device, frag, nullptr );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateGraphicsPipelines failed: " ) + ResultString( r ) );
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

	VkClearValue clear = {};
	clear.color = m_clearColor;

	VkRenderPassBeginInfo rp = {};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp.renderPass = m_renderPass;
	rp.framebuffer = m_framebuffers[imageIndex];
	rp.renderArea.offset = { 0, 0 };
	rp.renderArea.extent = m_swapExtent;
	rp.clearValueCount = 1;
	rp.pClearValues = &clear;
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
	vkCmdCopyImage( cmd,
		m_swapImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
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
	vkCmdPipelineBarrier( cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0, 0, nullptr, 0, nullptr, 2, post );
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
		vkCmdPipelineBarrier( cmd,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &toPresent );
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

	const bool bgra = ( m_swapFormat == VK_FORMAT_B8G8R8A8_UNORM ||
						m_swapFormat == VK_FORMAT_B8G8R8A8_SRGB );
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
