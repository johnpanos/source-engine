//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan device bring-up and presentation core.
//          See vulkan_device.h for the contract.
//
//===========================================================================//

#include "vulkan_device.h"
#include "demo_triangle_spv.h"
#include "material_spv.h"
#include "vulkan_present_mode.h"
#include "render/pbr_split_sum_table.h"
#include "render/render_sample_count.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <limits>

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

bool InstanceHasExtension( const char *name )
{
	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
	std::vector<VkExtensionProperties> extensions( count );
	if ( count )
		vkEnumerateInstanceExtensionProperties( nullptr, &count, extensions.data() );
	for ( const VkExtensionProperties &extension : extensions )
		if ( std::strcmp( extension.extensionName, name ) == 0 )
			return true;
	return false;
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
    IVulkanSurfaceHost &host, const VulkanContextConfig &config, std::string *outError )
{
	if ( IsValid() )
	{
		SetError( outError, "Init called on an already-initialized context" );
		return false;
	}

	m_host = &host;
	m_config = config;
	m_requestedVSync = config.vsync;
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
	CreateTimestampPool();
	QueryVulkanAdapterCaps( m_physicalDevice, m_swapFormat, &m_adapterCaps );
	if ( !ApplySampleCount( outError ) )
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
	std::vector<const char *> extensions;
	if ( !m_host->GetInstanceExtensions( &extensions, outError ) )
		return false;

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
	// VK_EXT_debug_utils: the validation messenger, and object names and labels
	// for capture tools (SetupDebugTools decides whether they are emitted).
	m_debugUtilsExtension =
	    m_validationEnabled || ( m_config.debugLabels != DebugLabelPolicy::Off &&
	                               InstanceHasExtension( VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) );
	if ( m_debugUtilsExtension && std::none_of( extensions.begin(), extensions.end(),
	                                  []( const char *name )
	                                  {
		                                  return std::strcmp(
		                                             name, VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) == 0;
	                                  } ) )
		extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = m_config.appName ? m_config.appName : "Source Native Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = "Source";
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	// Vulkan 1.2 where the loader has it: the compute foundation's feature
	// chain (RFC 0011 G5) uses its core structures; devices below 1.2 still
	// run everything else.
	uint32_t loaderVersion = VK_API_VERSION_1_1;
	if ( const auto enumerate = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
	         vkGetInstanceProcAddr( VK_NULL_HANDLE, "vkEnumerateInstanceVersion" ) ) )
		enumerate( &loaderVersion );
	appInfo.apiVersion = loaderVersion >= VK_API_VERSION_1_2 ? VK_API_VERSION_1_2 : VK_API_VERSION_1_1;

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
	// Read before creating: the surface is built from the same native surface.
	const uint64_t generation = m_host->GetNativeSurfaceGeneration();
	if ( !m_host->CreateSurface( m_instance, &m_surface, outError ) )
		return false;
	m_surfaceGeneration = generation;
	m_surfaceLost = false;
	return true;
}

bool CVulkanContext::EnsureSurfaceCurrent( bool *outReady, std::string *outError )
{
	*outReady = true;
	if ( !m_surfaceLost && m_host->GetNativeSurfaceGeneration() == m_surfaceGeneration )
		return true;
	if ( !m_host->IsNativeSurfaceAvailable() )
	{
		// No surface until the platform provides one again (Android's next
		// surfaceCreated); keep everything.
		*outReady = false;
		return true;
	}
	// Present nothing more to the old surface, then rebuild against the new one.
	vkDeviceWaitIdle( m_device );
	m_completedSerial = m_submitSerial;
	RetireCompletedTextures();
	DestroySwapchainObjects();
	m_host->DestroySurface( m_instance, m_surface );
	m_surface = VK_NULL_HANDLE;
	if ( !CreateSurface( outError ) )
		return false;
	VkBool32 presentable = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(
	    m_physicalDevice, m_presentQueueFamily, m_surface, &presentable );
	if ( !presentable )
	{
		SetError( outError, "the replacement surface cannot present from the selected queue" );
		return false;
	}
	if ( !CreateSwapchain( outError ) )
		return false;
	return m_swapchain == VK_NULL_HANDLE || CreateFramebuffers( outError );
}

bool CVulkanContext::SurfaceChangedSinceSwapchain()
{
	VkSurfaceCapabilitiesKHR caps = {};
	if ( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_physicalDevice, m_surface, &caps ) !=
	     VK_SUCCESS )
		return true; // The rebuild reports the failure.
	return caps.currentTransform != m_swapSurfaceTransform ||
	       caps.currentExtent.width != m_swapSurfaceExtent.width ||
	       caps.currentExtent.height != m_swapSurfaceExtent.height;
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

		const int score = ScorePhysicalDeviceType( props.deviceType );

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
	bool toolingInfo = false;
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
		// Debug shader variants carry NonSemantic.Shader.DebugInfo.100.
		m_nonSemanticInfo = !m_config.shaderDebugDirectory.empty() &&
		                    has( VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME );
		if ( m_nonSemanticInfo )
			deviceExts.push_back( VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME );
		// A capture tool announces itself through VK_EXT_tooling_info.
		toolingInfo = has( VK_EXT_TOOLING_INFO_EXTENSION_NAME );
	}

	// Exact occlusion counts, which auto-exposure's luminance histogram needs
	// (a non-precise query may report any nonzero value for a visible draw).
	VkPhysicalDeviceFeatures supported = {};
	vkGetPhysicalDeviceFeatures( m_physicalDevice, &supported );
	VkPhysicalDeviceFeatures features = {};
	features.occlusionQueryPrecise = supported.occlusionQueryPrecise;
	features.samplerAnisotropy = supported.samplerAnisotropy;
	m_preciseOcclusion = supported.occlusionQueryPrecise == VK_TRUE;
	// D3D9 user clip planes are clip distances. The planes travel in the push
	// constants, so a device must also hold the widest block that carries them
	// (PortalRefract's, kPortalPushBytes); without either, no clip planes are
	// reported and the material system falls back as on D3D9 hardware without
	// user clip planes.
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	m_maxAnisotropy =
	    supported.samplerAnisotropy
	        ? std::max(
	              1, std::min( 16, static_cast<int>( properties.limits.maxSamplerAnisotropy ) ) )
	        : 1;
	m_anisotropyLevel = std::min( 4, m_maxAnisotropy );
	m_portalPushSupported = properties.limits.maxPushConstantsSize >= kPortalPushBytes;
	m_descriptorSetLimit = properties.limits.maxBoundDescriptorSets;
	// Volume textures (the white volume, color-correction volumes, the SDF
	// shadow field) are checked against it whichever pipelines exist.
	m_maxImageDimension3D = properties.limits.maxImageDimension3D;
	if ( m_config.descriptorSetLimit > 0 )
		m_descriptorSetLimit = std::min( m_descriptorSetLimit, m_config.descriptorSetLimit );
	m_clipPlanesSupported = supported.shaderClipDistance == VK_TRUE &&
	                        properties.limits.maxClipDistances >= kMaxClipPlanes &&
	                        properties.limits.maxPushConstantsSize >= kTexturedPushBytes;
	features.shaderClipDistance = m_clipPlanesSupported ? VK_TRUE : VK_FALSE;
	// DXT textures upload as BC images and are decoded by the sampler, as D3D9
	// samples them; without the feature the material system decompresses them.
	m_blockCompression = supported.textureCompressionBC == VK_TRUE;
	features.textureCompressionBC = supported.textureCompressionBC;
	// A depth format with stencil, as D3D9 always creates one (D24S8): portal
	// views are drawn with stencil recursion (SelectDepthStencilFormat).
	const VkFormat depthStencil = SelectDepthStencilFormat( m_physicalDevice );
	if ( depthStencil != VK_FORMAT_UNDEFINED )
	{
		m_depthFormat = depthStencil;
		m_depthAspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		m_stencilBits = 8;
	}
	// Readable scene depth: attachments are copied into a sampled image of the
	// same format (RecordSceneCapture).
	{
		VkFormatProperties depthProps = {};
		vkGetPhysicalDeviceFormatProperties( m_physicalDevice, m_depthFormat, &depthProps );
		const VkFormatFeatureFlags readable = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
		                                      VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
		                                      VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
		m_sceneDepthUsable = ( depthProps.optimalTilingFeatures & readable ) == readable;
	}

	// Every feature goes through the VkPhysicalDeviceFeatures2 chain: the
	// ones above plus what the compute foundation's queries found.
	m_featureChain.Build( m_physicalDevice,
	    QueryComputeCaps( m_physicalDevice, m_graphicsQueueFamily ), features, &deviceExts );
	m_computeCaps = m_featureChain.Enabled();
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = m_featureChain.Chain();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>( queueInfos.size() );
	createInfo.pQueueCreateInfos = queueInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>( deviceExts.size() );
	createInfo.ppEnabledExtensionNames = deviceExts.data();

	VkResult r = vkCreateDevice( m_physicalDevice, &createInfo, nullptr, &m_device );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateDevice failed: " ) + ResultString( r ) );
		return false;
	}

	vkGetDeviceQueue( m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue );
	vkGetDeviceQueue( m_device, m_presentQueueFamily, 0, &m_presentQueue );
	SetupDebugTools( toolingInfo );
	m_compute.SetDebugTools( &m_shaderLibrary, &m_debugUtils );
	std::string computeError;
	if ( !m_compute.Init( m_physicalDevice, m_device, m_computeCaps, &computeError ) )
		std::fprintf( stderr, "[NativeVulkan] compute unavailable: %s\n", computeError.c_str() );
	std::fprintf( stderr,
	    "[NativeVulkan] compute %s, storage images %s, ray query %s (device API %u.%u)\n",
	    m_computeCaps.compute ? "on" : "off", m_computeCaps.storageImages ? "on" : "off",
	    m_computeCaps.rayQuery ? "on" : "off", VK_API_VERSION_MAJOR( m_computeCaps.deviceApiVersion ),
	    VK_API_VERSION_MINOR( m_computeCaps.deviceApiVersion ) );
	return true;
}

void CVulkanContext::SetupDebugTools( bool toolingInfo )
{
	bool toolWantsMarkers = false;
	std::string tools;
	const auto getTools =
	    toolingInfo
	        ? reinterpret_cast<PFN_vkGetPhysicalDeviceToolPropertiesEXT>(
	              vkGetInstanceProcAddr( m_instance, "vkGetPhysicalDeviceToolPropertiesEXT" ) )
	        : nullptr;
	if ( getTools )
	{
		uint32_t count = 0;
		getTools( m_physicalDevice, &count, nullptr );
		std::vector<VkPhysicalDeviceToolPropertiesEXT> found( count );
		for ( VkPhysicalDeviceToolPropertiesEXT &tool : found )
			tool.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT;
		if ( count && getTools( m_physicalDevice, &count, found.data() ) == VK_SUCCESS )
		{
			for ( uint32_t i = 0; i < count; ++i )
			{
				toolWantsMarkers = toolWantsMarkers ||
				                   ( found[i].purposes & VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT );
				tools += ( tools.empty() ? "" : ", " ) + std::string( found[i].name );
			}
		}
	}
	if ( DebugLabelsWanted( m_config.debugLabels, m_validationEnabled, toolWantsMarkers ) )
	{
		if ( m_debugUtilsExtension && m_debugUtils.Load( m_instance, m_device ) )
			Log( "debug names and labels on (%s)\n", m_config.debugLabels == DebugLabelPolicy::On
			                                             ? "-vkdebuglabels"
			                                         : m_validationEnabled ? "validation"
			                                                               : tools.c_str() );
		else
			Log( "debug names and labels unavailable (no VK_EXT_debug_utils)\n" );
	}
	if ( m_config.shaderDebugDirectory.empty() )
		return;
	const VulkanShaderLibrary::LoadReport report = m_shaderLibrary.LoadDebugDirectory(
	    m_config.shaderDebugDirectory.c_str(), m_nonSemanticInfo );
	Log( "debug shaders: %u from %s\n", report.loaded, m_config.shaderDebugDirectory.c_str() );
	for ( const std::string &rejected : report.rejected )
		Log( "debug shader not used: %s\n", rejected.c_str() );
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
	m_swapSurfaceExtent = caps.currentExtent;
	m_swapSurfaceTransform = caps.currentTransform;

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

	// Present mode: render.present-policy.v1 over what the surface offers.
	{
		uint32_t pmCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR( m_physicalDevice, m_surface, &pmCount, nullptr );
		std::vector<VkPresentModeKHR> modes( pmCount );
		if ( pmCount )
			vkGetPhysicalDeviceSurfacePresentModesKHR(
			    m_physicalDevice, m_surface, &pmCount, modes.data() );
		const VkPresentModeKHR previous = m_presentMode;
		const bool firstSwapchain = m_swapchainGeneration == 0;
		m_presentMode = SelectVulkanPresentMode( m_requestedVSync, modes.data(), pmCount );
		m_surfacePresentModes.assign( modes.begin(), modes.begin() + pmCount );
		m_swapchainVSync = m_requestedVSync;
		if ( firstSwapchain || previous != m_presentMode )
			Log( "present mode %s (vsync %s)\n", VulkanPresentModeName( m_presentMode ),
			    m_requestedVSync ? "on" : "off" );
	}

	// Extent: honor the surface's fixed extent, else clamp the drawable size.
	m_host->GetDrawableSize( &m_presentDrawable[0], &m_presentDrawable[1] );
	if ( caps.currentExtent.width != UINT32_MAX )
	{
		m_presentExtent = caps.currentExtent;
	}
	else
	{
		int w = 0, h = 0;
		m_host->GetDrawableSize( &w, &h );
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
	// The back buffer is drawn in the window's orientation and the compositor
	// rotates it on a rotated mobile display. Taking currentTransform instead
	// would promise pre-rotated images, which this renderer does not draw.
	info.preTransform = ( caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
	                        ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
	                        : caps.currentTransform;
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
	++m_swapchainGeneration;

	uint32_t actual = 0;
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &actual, nullptr );
	m_presentImages.resize( actual );
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &actual, m_presentImages.data() );
	for ( uint32_t i = 0; i < actual; ++i )
		m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, m_presentImages[i], "swapchain image %u", i );
	if ( !GrowRenderFinished( actual, outError ) )
		return false;

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
		// A multisampled frame resolves into the back buffer (TRANSFER_DST).
		img.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		// The present-time gamma pass samples the back buffer.
		if ( swapFeatures.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT )
			img.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
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
		m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, m_swapImages[i], "back buffer %u", i );
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
		img.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		            ( m_sceneDepthUsable ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0 );
		img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if ( vkCreateImage( m_device, &img, nullptr, &m_depthImages[i] ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateImage (depth) failed" );
			return false;
		}
		m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, m_depthImages[i], "back buffer depth %u", i );
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

bool CVulkanContext::CreateAttachmentPass( VkAttachmentLoadOp loadOp, VkImageLayout colorInitial,
    VkImageLayout colorFinal, VkImageLayout depthInitial, VkRenderPass *outPass,
    VkFormat colorFormat, VkSampleCountFlagBits samples, std::string *outError )
{
	VkAttachmentDescription color = {};
	color.format = colorFormat;
	color.samples = samples;
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
	depth.samples = samples;
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
	deps[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
	                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
	                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
	                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
	deps[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
	                        VK_ACCESS_TRANSFER_WRITE_BIT;
	deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
	                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
	                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	deps[0].dstAccessMask =
	    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	deps[1].srcSubpass = 0;
	deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	deps[1].srcStageMask =
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	deps[1].srcAccessMask =
	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	deps[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT |
	                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
	                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	deps[1].dstAccessMask =
	    VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
	    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

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
		return CreateAttachmentPass( loadOp, colorInitial, colorFinal, depthInitial, outPass,
		    colorFormat, VK_SAMPLE_COUNT_1_BIT, outError );
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
	                   m_swapFormatSrgb ) &&
	               makePass( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED,
	                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
	                   &m_renderPassClearSrgb, m_swapFormatSrgb ) ) );
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

// The semaphores a submit signals for present, one per swapchain image: a
// present holds its image's until that image is acquired again, so a
// per-frame semaphore could be signaled while an earlier present still
// holds it (VUID-vkQueueSubmit-pSignalSemaphores-00067). They only grow: a
// recreated swapchain keeps those an old present may still hold.
bool CVulkanContext::GrowRenderFinished( size_t count, std::string *outError )
{
	VkSemaphoreCreateInfo sem = {};
	sem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	while ( m_renderFinished.size() < count )
	{
		VkSemaphore semaphore = VK_NULL_HANDLE;
		if ( vkCreateSemaphore( m_device, &sem, nullptr, &semaphore ) != VK_SUCCESS )
		{
			SetError( outError, "failed to create a present semaphore" );
			return false;
		}
		m_renderFinished.push_back( semaphore );
	}
	return true;
}

bool CVulkanContext::CreateSyncObjects( std::string *outError )
{
	m_imageAvailable.resize( m_framesInFlight );
	m_inFlight.resize( m_framesInFlight );
	if ( !GrowRenderFinished( std::max<size_t>( m_framesInFlight, m_presentImages.size() ),
	         outError ) )
		return false;

	VkSemaphoreCreateInfo sem = {};
	sem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fence = {};
	fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for ( uint32_t i = 0; i < m_framesInFlight; ++i )
	{
		if ( vkCreateSemaphore( m_device, &sem, nullptr, &m_imageAvailable[i] ) != VK_SUCCESS ||
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

// The Location of every Input variable of a vertex-stage SPIR-V module, as a
// bit mask. False for a module without a vertex entry point or one that is not
// well-formed SPIR-V; its pipelines then keep the attributes they declare.
static bool VertexInputLocations( const uint32_t *words, size_t count, uint64_t *outMask )
{
	enum
	{
		kOpEntryPoint = 15,
		kOpVariable = 59,
		kOpDecorate = 71,
		kDecorationLocation = 30,
		kStorageInput = 1,
		kExecutionModelVertex = 0
	};
	if ( !words || count < 5 || words[0] != 0x07230203u )
		return false;
	bool vertex = false;
	std::map<uint32_t, uint32_t> locations; // id -> Location
	std::vector<uint32_t> inputs;
	for ( size_t at = 5; at < count; )
	{
		const uint32_t length = words[at] >> 16;
		const uint32_t opcode = words[at] & 0xffffu;
		if ( length == 0 || at + length > count )
			return false;
		if ( opcode == kOpEntryPoint && length >= 2 && words[at + 1] == kExecutionModelVertex )
			vertex = true;
		else if ( opcode == kOpDecorate && length >= 4 && words[at + 2] == kDecorationLocation )
			locations[words[at + 1]] = words[at + 3];
		else if ( opcode == kOpVariable && length >= 4 && words[at + 3] == kStorageInput )
			inputs.push_back( words[at + 2] );
		at += length;
	}
	if ( !vertex )
		return false;
	uint64_t mask = 0;
	for ( uint32_t id : inputs )
	{
		const auto location = locations.find( id );
		if ( location != locations.end() && location->second < 64 )
			mask |= uint64_t( 1 ) << location->second;
	}
	*outMask = mask;
	return true;
}

bool CVulkanContext::CreateShaderModule( const uint32_t *embedded, size_t embeddedBytes,
    VkShaderModule *outModule, std::string *outError )
{
	// The embedded code, or its debug variant (vulkan_shader_library.h).
	const ShaderModuleCode resolved = m_shaderLibrary.Resolve( embedded, embeddedBytes );
	const uint32_t *code = resolved.code;
	const size_t sizeBytes = resolved.sizeBytes;
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
	m_debugUtils.NameF( VK_OBJECT_TYPE_SHADER_MODULE, *outModule, "%s%s",
	    resolved.name ? resolved.name : "unindexed shader",
	    resolved.debugVariant ? " (debug)" : "" );
	if ( m_debugUtils.Active() )
		m_moduleNames[*outModule] = resolved.name ? resolved.name : "unindexed shader";
	// A handle can be reused after its module is destroyed; forget the old one.
	m_vertexInputLocations.erase( *outModule );
	uint64_t locations = 0;
	if ( VertexInputLocations( code, sizeBytes / sizeof( uint32_t ), &locations ) )
		m_vertexInputLocations[*outModule] = locations;
	return true;
}

const VkPipelineVertexInputStateCreateInfo *CVulkanContext::FilterVertexInput(
    const VkPipelineVertexInputStateCreateInfo *input, VkShaderModule vertex,
    ConsumedVertexInput *storage ) const
{
	const auto found = m_vertexInputLocations.find( vertex );
	if ( !input || found == m_vertexInputLocations.end() )
		return input;
	storage->info = *input;
	storage->attributes.clear();
	for ( uint32_t i = 0; i < input->vertexAttributeDescriptionCount; ++i )
	{
		const VkVertexInputAttributeDescription &attribute = input->pVertexAttributeDescriptions[i];
		if ( attribute.location < 64 && ( found->second >> attribute.location ) & 1u )
			storage->attributes.push_back( attribute );
	}
	storage->info.vertexAttributeDescriptionCount =
	    static_cast<uint32_t>( storage->attributes.size() );
	storage->info.pVertexAttributeDescriptions =
	    storage->attributes.empty() ? nullptr : storage->attributes.data();
	return &storage->info;
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
	ConsumedVertexInput consumedInput;
	gp.pVertexInputState = FilterVertexInput( &vin, stages[0].module, &consumedInput );
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
	CFrameCostScope cost( m_frameCost, kCostSingleSubmit );
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
	ConsumedVertexInput consumedInput;
	gp.pVertexInputState = FilterVertexInput( &vin, stages[0].module, &consumedInput );
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
	ConsumedVertexInput consumedInput;
	gp.pVertexInputState = FilterVertexInput( &vin, stages[0].module, &consumedInput );
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
	ConsumedVertexInput consumedInput;
	gp.pVertexInputState = FilterVertexInput( &vin, stages[0].module, &consumedInput );
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
	ConsumedVertexInput consumedInput;
	gp.pVertexInputState = FilterVertexInput( &vin, stages[0].module, &consumedInput );
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
		m_debugUtils.Name( VK_OBJECT_TYPE_IMAGE, m_dynTexImage, "built-in fallback texture" );
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
			m_samplers[state] = CreateManagedSampler( state, m_anisotropyLevel );
			if ( m_samplers[state] == VK_NULL_HANDLE )
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
		m_whiteCubeHandle = CreateManagedTexture(
		    1, 1, VK_FORMAT_R8G8B8A8_UNORM, outError, 0, 1, VK_FORMAT_UNDEFINED, true );
		NameManagedTexture( m_whiteCubeHandle, "white cube" );
		if ( m_whiteCubeHandle < 0 )
			return false;
		const uint8_t white[4] = { 255, 255, 255, 255 };
		for ( uint32_t face = 0; face < 6; ++face )
			if ( !UploadManagedTexture(
			         m_whiteCubeHandle, white, sizeof( white ), outError, 0, face ) )
				return false;
		// The white volume: what an unread 3D sampler reads (the post
		// passes' unused color-correction volumes, the PBR frame set's
		// shadow field when the map has none).
		m_whiteVolumeHandle = CreateManagedTexture(
		    1, 1, VK_FORMAT_R8G8B8A8_UNORM, outError, 0, 1, VK_FORMAT_UNDEFINED, false, 2 );
		NameManagedTexture( m_whiteVolumeHandle, "white volume" );
		const uint8_t whiteVolume[8] = { 255, 255, 255, 255, 255, 255, 255, 255 };
		if ( m_whiteVolumeHandle < 0 || !UploadManagedTexture( m_whiteVolumeHandle, whiteVolume,
		                                    sizeof( whiteVolume ), outError ) )
			return false;

		// The UnlitGeneric push block is larger than the color pipelines': it
		// carries cModelViewProj (mat4), cModulationColor (vec4), and the two rows
		// of cBaseTextureTransform (2x vec4) -- 28 floats / 112 bytes, within the
		// 128-byte guaranteed push-constant minimum.
		VkPushConstantRange texPc = {};
		texPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		texPc.offset = 0;
		// + vec4 alphaParams (128 bytes), and the user clip planes when the device
		// clips (demo_dyn_tex_clip.vert reads them after the block).
		texPc.size = m_clipPlanesSupported ? static_cast<uint32_t>( kTexturedPushBytes )
		                                   : sizeof( float ) * 32;
		// Base, lightmap, cubemap and normal mask share the managed texture
		// descriptor layout; each draw selects the corresponding texture set.
		const VkDescriptorSetLayout texSetLayouts[4] = {
		    m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout };
		VkPipelineLayoutCreateInfo texPl = {};
		texPl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		texPl.setLayoutCount = 4;
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
		t.bindings[0] = binding;
		// The per-draw fog stream (DrawFog): one record per instance, the draw's
		// first instance selecting it.
		t.bindings[1].binding = 1;
		t.bindings[1].stride = sizeof( DrawFog );
		t.bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		std::memcpy( t.attrs, attrs, sizeof( attrs ) );
		for ( uint32_t i = 0; i < 4; ++i )
		{
			t.attrs[7 + i].location = 7 + i;
			t.attrs[7 + i].binding = 1;
			t.attrs[7 + i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			t.attrs[7 + i].offset = static_cast<uint32_t>( sizeof( float ) * 4 * i );
		}
		// The textured stage reads the world reflection direction from the normal
		// slot for a lightmapped cubemap pass.
		t.attrs[4].location = 4;
		t.attrs[4].format = VK_FORMAT_R32G32B32_SFLOAT;
		t.attrs[4].offset = sizeof( float ) * 10;
		// Refract carries its environment tint in the otherwise unused tangent slot.
		t.attrs[5].location = 5;
		t.attrs[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		t.attrs[5].offset = sizeof( float ) * 13;
		// The textured stage also reads the vertex color's alpha, which ends the
		// record ($vertexalpha; demo_dyn_tex.vert location 6).
		t.attrs[6].location = 6;
		t.attrs[6].format = VK_FORMAT_R32_SFLOAT;
		t.attrs[6].offset = sizeof( float ) * 17;
		t.vin = vin;
		t.vin.vertexBindingDescriptionCount = 2;
		t.vin.pVertexBindingDescriptions = t.bindings;
		t.vin.vertexAttributeDescriptionCount = 11;
		t.vin.pVertexAttributeDescriptions = t.attrs;
		// WMSH v1 keeps the packer's 40-byte corner layout on the GPU. The
		// dedicated vertex stage reads position, oct-normal and both UV sets;
		// tangent/handedness remain available in the same record for later
		// normal-map cohorts.
		m_worldBindings[0].binding = 0;
		m_worldBindings[0].stride = 40;
		m_worldBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		m_worldBindings[1] = t.bindings[1];
		m_worldAttrs[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 };
		m_worldAttrs[1] = { 1, 0, VK_FORMAT_R16G16_SNORM, 12 };
		m_worldAttrs[2] = { 2, 0, VK_FORMAT_R32G32_SFLOAT, 24 };
		m_worldAttrs[3] = { 3, 0, VK_FORMAT_R32G32_SFLOAT, 32 };
		for ( uint32_t i = 0; i < 4; ++i )
			m_worldAttrs[4 + i] = t.attrs[7 + i];
		m_worldVin = t.vin;
		m_worldVin.vertexBindingDescriptionCount = 2;
		m_worldVin.pVertexBindingDescriptions = m_worldBindings;
		m_worldVin.vertexAttributeDescriptionCount = 8;
		m_worldVin.pVertexAttributeDescriptions = m_worldAttrs;
		if ( !( m_clipPlanesSupported
		             ? CreateShaderModule( g_worldMeshClipVertSpv, sizeof( g_worldMeshClipVertSpv ),
		                   &m_worldVert, outError )
		             : CreateShaderModule( g_worldMeshVertSpv, sizeof( g_worldMeshVertSpv ),
		                   &m_worldVert, outError ) ) )
			return false;
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
		t.dynStates[5] = VK_DYNAMIC_STATE_DEPTH_BIAS;
		t.dyn = dyn;
		t.dyn.dynamicStateCount = 6;
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
	// Optional: without them LightmappedGeneric keeps the textured pipeline's
	// flat lightmap, and the bloom and color-correction passes are declined.
	std::string lightmappedError;
	if ( !InitLightmappedPipeline( &lightmappedError ) )
	{
		Log( "LightmappedGeneric pipeline unavailable: %s\n", lightmappedError.c_str() );
		DestroyLightmappedPipeline();
	}
	std::string postError;
	if ( !InitPostPipeline( &postError ) )
	{
		Log( "post-processing pipeline unavailable: %s\n", postError.c_str() );
		DestroyPostPipeline();
	}
	// The PBR and GI stages bind their textures in frame and material sets
	// (vulkan_descriptor_groups.h); without them those stages are declined.
	std::string pbrError;
	if ( !m_groupedDescriptors.Init( m_device, m_framesInFlight, &pbrError ) )
		Log( "grouped descriptor sets unavailable: %s\n", pbrError.c_str() );
	if ( !InitPbrDirectPipeline( &pbrError ) )
	{
		Log( "PBR direct pipeline unavailable: %s\n", pbrError.c_str() );
		DestroyPbrDirectPipeline();
	}
	else if ( !InitPbrWorldPipeline( &pbrError ) )
	{
		Log( "WMSH PBR pipeline unavailable: %s\n", pbrError.c_str() );
		DestroyPbrWorldPipeline();
	}
	else if ( !InitPbrGlassPipeline( &pbrError ) )
	{
		Log( "WMSH glass pipeline unavailable: %s\n", pbrError.c_str() );
		DestroyPbrGlassPipeline();
	}
	// Model PBR needs the split-sum table the direct pipeline owns.
	if ( m_pbrDirectReady && !InitPbrModelPipeline( &pbrError ) )
	{
		Log( "model PBR pipeline unavailable: %s\n", pbrError.c_str() );
		DestroyPbrModelPipeline();
	}

	Log( "dynamic mesh pipelines ready (PBR direct %s, WMSH PBR %s, glass %s, model PBR %s, "
	     "scene depth %s)\n",
	    m_pbrDirectReady ? "available" : "unavailable",
	    m_pbrWorldReady ? "available" : "unavailable",
	    m_pbrGlassReady ? "available" : "unavailable",
	    m_pbrModelReady ? "available" : "unavailable",
	    m_sceneDepthUsable ? "readable" : "unavailable" );
	return true;
}

uint64_t CVulkanContext::RasterStateKey( const DynRasterState &state )
{
	// Blend factors are below 32 and compare ops below 8. The alpha-write bit
	// lives above the pass/sample bits so every raster state has a distinct key.
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
	               : 0u ) |
	       ( state.alphaWrite ? 1ull << 36 : 0ull ) | ( state.depthBiasEnable ? 1ull << 37 : 0ull );
}

// The inverse of RasterStateKey. Bits 32-35 are PipelineKey's pass and sample
// selection; bits 36-37 belong to the raster state.
CVulkanContext::DynRasterState CVulkanContext::RasterStateFromKey( uint64_t key )
{
	const uint32_t k = static_cast<uint32_t>( key );
	DynRasterState state;
	state.blend = ( k & 1u ) != 0;
	state.srcFactor = static_cast<VkBlendFactor>( ( k >> 1 ) & 31u );
	state.dstFactor = static_cast<VkBlendFactor>( ( k >> 6 ) & 31u );
	state.depthTest = ( ( k >> 11 ) & 1u ) != 0;
	state.depthWrite = ( ( k >> 12 ) & 1u ) != 0;
	state.depthCompare = static_cast<VkCompareOp>( ( k >> 13 ) & 7u );
	state.colorWrite = ( ( k >> 16 ) & 1u ) != 0;
	state.alphaWrite = ( key & ( 1ull << 36 ) ) != 0;
	state.depthBiasEnable = ( key & ( 1ull << 37 ) ) != 0;
	state.cullMode = static_cast<VkCullModeFlags>( ( k >> 17 ) & 3u );
	state.stencilEnable = ( ( k >> 19 ) & 1u ) != 0;
	if ( state.stencilEnable )
	{
		state.stencilCompare = static_cast<VkCompareOp>( ( k >> 20 ) & 7u );
		state.stencilFail = static_cast<VkStencilOp>( ( k >> 23 ) & 7u );
		state.stencilDepthFail = static_cast<VkStencilOp>( ( k >> 26 ) & 7u );
		state.stencilPass = static_cast<VkStencilOp>( ( k >> 29 ) & 7u );
	}
	return state;
}

uint64_t CVulkanContext::PipelineKey( const DynRasterState &state, bool srgbPass, int samples )
{
	uint64_t log2Samples = 0;
	while ( ( 1 << ( log2Samples + 1 ) ) <= samples && log2Samples < 6 )
		++log2Samples;
	return RasterStateKey( state ) | ( srgbPass ? 1ull << 32 : 0ull ) | log2Samples << 33;
}

int CVulkanContext::PipelineKeySamples( uint64_t key )
{
	return 1 << static_cast<int>( ( key >> 33 ) & 7u );
}

VkRenderPass CVulkanContext::PipelineRenderPass( bool srgbPass, int samples ) const
{
	if ( samples <= 1 )
		return srgbPass ? m_renderPassLoadSrgb : m_renderPass;
	if ( samples != m_activeSamples )
		return VK_NULL_HANDLE;
	return srgbPass ? m_msPassLoadSrgb : m_msPassLoad;
}

VkPipeline CVulkanContext::TexturedPipeline(
    const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_dynTexPipelines.find( key );
	if ( existing != m_dynTexPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_texTemplate.stages[0].module == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_texTemplate.stages[0].module,
	    m_texTemplate.stages[1].module, m_dynTexPipelineLayout, &m_texTemplate.vin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (textured, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	// A failed state is cached too, so it is reported once rather than per draw.
	m_dynTexPipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelineTextured, key );
	return pipeline;
}

VkPipeline CVulkanContext::WorldTexturedPipeline(
    const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_worldTexPipelines.find( key );
	if ( existing != m_worldTexPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_worldVert == VK_NULL_HANDLE || m_texTemplate.stages[1].module == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_worldVert, m_texTemplate.stages[1].module,
	    m_dynTexPipelineLayout, &m_worldVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (WMSH textured, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_worldTexPipelines[key] = pipeline;
	return pipeline;
}

VkPipeline CVulkanContext::PortalPipeline( const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_portalPipelines.find( key );
	if ( existing != m_portalPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_portalVert == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline(
	    state, m_portalVert, m_portalFrag, m_portalPipelineLayout, &m_portalVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (PortalRefract, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_portalPipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelinePortal, key );
	return pipeline;
}

VkPipeline CVulkanContext::SkinPipeline( const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_skinPipelines.find( key );
	if ( existing != m_skinPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_skinVert == VK_NULL_HANDLE || m_skinPipelineLayout == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline(
	    state, m_skinVert, m_skinFrag, m_skinPipelineLayout, &m_skinVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (skin, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_skinPipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelineSkin, key );
	return pipeline;
}

VkPipeline CVulkanContext::SolidEnergyPipeline(
    const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_solidEnergyPipelines.find( key );
	if ( existing != m_solidEnergyPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_solidEnergyVert == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_solidEnergyVert, m_solidEnergyFrag,
	    m_skinPipelineLayout, &m_skinVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (SolidEnergy, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_solidEnergyPipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelineSolidEnergy, key );
	return pipeline;
}

VkPipeline CVulkanContext::PaintBlobPipeline(
    const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_paintBlobPipelines.find( key );
	if ( existing != m_paintBlobPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_paintBlobFrag == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline(
	    state, m_skinVert, m_paintBlobFrag, m_skinPipelineLayout, &m_skinVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (PaintBlob, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_paintBlobPipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelinePaintBlob, key );
	return pipeline;
}

VkPipeline CVulkanContext::LightmappedPipeline(
    const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_lightmappedPipelines.find( key );
	if ( existing != m_lightmappedPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_lightmappedVert == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_lightmappedVert, m_lightmappedFrag,
	    m_lightmappedPipelineLayout, &m_lightmappedVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (LightmappedGeneric, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_lightmappedPipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelineLightmapped, key );
	return pipeline;
}

VkPipeline CVulkanContext::PostPipeline( const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_postPipelines.find( key );
	if ( existing != m_postPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_postVert == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline(
	    state, m_postVert, m_postFrag, m_skinPipelineLayout, &m_skinVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (post-processing, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_postPipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelinePost, key );
	return pipeline;
}

VkPipeline CVulkanContext::PbrDirectPipeline(
    const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_pbrDirectPipelines.find( key );
	if ( existing != m_pbrDirectPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_pbrDirectFrag == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_texTemplate.stages[0].module,
	    m_pbrDirectFrag, m_pbrDirectPipelineLayout, &m_texTemplate.vin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		Log( "vkCreateGraphicsPipelines (PBR direct, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_pbrDirectPipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelinePbrDirect, key );
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
    const VkPipelineVertexInputStateCreateInfo *vertexInput, VkRenderPass renderPass, int samples )
{
	CFrameCostScope cost( m_frameCost, kCostPipelineCreate );
	VkPipelineColorBlendAttachmentState att = {};
	if ( state.colorWrite )
		att.colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
	if ( state.alphaWrite )
		att.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
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
	ConsumedVertexInput consumedInput;
	gp.pVertexInputState = FilterVertexInput( vertexInput, stages[0].module, &consumedInput );
	gp.pInputAssemblyState = &t.ia;
	gp.pViewportState = &t.vp;
	VkPipelineRasterizationStateCreateInfo rs = t.rs;
	rs.cullMode = state.cullMode;
	rs.depthBiasEnable = state.depthBiasEnable ? VK_TRUE : VK_FALSE;
	rs.frontFace = vertexInput == &m_worldVin || vertexInput == &m_worldPbrVin
	                   ? VK_FRONT_FACE_COUNTER_CLOCKWISE
	                   : VK_FRONT_FACE_CLOCKWISE;
	gp.pRasterizationState = &rs;
	VkPipelineMultisampleStateCreateInfo ms = t.ms;
	ms.rasterizationSamples = static_cast<VkSampleCountFlagBits>( samples );
	gp.pMultisampleState = &ms;
	gp.pColorBlendState = &cb;
	gp.pDynamicState = &t.dyn;
	gp.pDepthStencilState = &ds;
	gp.layout = layout;
	// The swapchain and render-target passes are render-pass compatible, and so
	// are the two sRGB passes with each other; a multisampled back buffer's
	// passes form their own class (PipelineRenderPass).
	gp.renderPass = renderPass;
	gp.subpass = 0;
	VkPipeline pipeline = VK_NULL_HANDLE;
	if ( vkCreateGraphicsPipelines( m_device, m_pipelineCache, 1, &gp, nullptr, &pipeline ) !=
	     VK_SUCCESS )
		pipeline = VK_NULL_HANDLE;
	if ( m_debugUtils.Active() && pipeline != VK_NULL_HANDLE )
	{
		const auto vertName = m_moduleNames.find( vert );
		const auto fragName = m_moduleNames.find( frag );
		m_debugUtils.NameF( VK_OBJECT_TYPE_PIPELINE, pipeline, "%s / %s%s",
		    vertName != m_moduleNames.end() ? vertName->second : "?",
		    fragName != m_moduleNames.end() ? fragName->second : "?",
		    samples > 1 ? " (MSAA)" : "" );
	}
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
// registers (see shaders/skin.frag). A device that cannot bind the block gets
// none of it, and the shader API declines the draws by name. The constants
// ring and the vertex stage are shared with model PBR and the world's direct
// lights, which bind three sets; only the $phong layout needs seven, so a
// device below that keeps them and declines $phong alone.
bool CVulkanContext::InitSkinPipeline( std::string *outError )
{
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	if ( !m_clipPlanesSupported || properties.limits.maxPushConstantsSize < kSkinPushBytes )
	{
		Log( "skin pipeline unavailable: push constants or clip distances\n" );
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

	if ( !CreateShaderModule( g_skinVertSpv, sizeof( g_skinVertSpv ), &m_skinVert, outError ) )
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
	if ( DescriptorSetLimit() < 7 )
	{
		Log( "skin pipeline unavailable: %u descriptor sets (the constants ring and vertex "
		     "stage remain for PBR)\n",
		    DescriptorSetLimit() );
		return true;
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
	if ( !CreateShaderModule( g_skinFragSpv, sizeof( g_skinFragSpv ), &m_skinFrag, outError ) )
		return false;
	if ( SkinPipeline( DynRasterState() ) == VK_NULL_HANDLE )
	{
		SetError( outError, "vkCreateGraphicsPipelines (skin) failed" );
		return false;
	}
	// SolidEnergy's stages on the same layout and vertex input. Without them
	// its draws are declined by name; the skin shader is unaffected.
	std::string solidEnergyError;
	if ( !CreateShaderModule( g_solidEnergyVertSpv, sizeof( g_solidEnergyVertSpv ),
	         &m_solidEnergyVert, &solidEnergyError ) ||
	     !CreateShaderModule( g_solidEnergyFragSpv, sizeof( g_solidEnergyFragSpv ),
	         &m_solidEnergyFrag, &solidEnergyError ) )
	{
		Log( "SolidEnergy pipeline unavailable: %s\n", solidEnergyError.c_str() );
		for ( VkShaderModule *module : { &m_solidEnergyVert, &m_solidEnergyFrag } )
		{
			if ( *module != VK_NULL_HANDLE )
				vkDestroyShaderModule( m_device, *module, nullptr );
			*module = VK_NULL_HANDLE;
		}
	}
	// The paint blobs' pixel stage after skin.vert. Without it their draws are
	// declined by name; the skin shader is unaffected.
	std::string paintBlobError;
	if ( !CreateShaderModule(
	         g_paintBlobFragSpv, sizeof( g_paintBlobFragSpv ), &m_paintBlobFrag, &paintBlobError ) )
	{
		Log( "PaintBlob pipeline unavailable: %s\n", paintBlobError.c_str() );
		m_paintBlobFrag = VK_NULL_HANDLE;
	}
	return true;
}

bool CVulkanContext::InitPbrDirectPipeline( std::string *outError )
{
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	if ( DescriptorSetLimit() < 3 )
	{
		SetError( outError, "PBR direct requires three descriptor sets" );
		return false;
	}
	VkFormatProperties formatProperties = {};
	vkGetPhysicalDeviceFormatProperties(
	    m_physicalDevice, VK_FORMAT_R32G32B32A32_SFLOAT, &formatProperties );
	if ( !( formatProperties.optimalTilingFeatures &
	         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT ) )
	{
		SetError( outError, "PBR split-sum format lacks linear filtering" );
		return false;
	}
	VkPushConstantRange pc = {};
	pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pc.size =
	    m_clipPlanesSupported ? static_cast<uint32_t>( kTexturedPushBytes ) : sizeof( float ) * 32;
	const VkDescriptorSetLayout sets[3] = {
	    m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout };
	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.setLayoutCount = 3;
	pl.pSetLayouts = sets;
	pl.pushConstantRangeCount = 1;
	pl.pPushConstantRanges = &pc;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &m_pbrDirectPipelineLayout ) !=
	     VK_SUCCESS )
	{
		SetError( outError, "vkCreatePipelineLayout (PBR direct) failed" );
		return false;
	}
	using namespace render::pbr;
	m_pbrSplitSumHandle = CreateManagedTexture(
	    kSplitSumSize, kSplitSumSize, VK_FORMAT_R32G32B32A32_SFLOAT, outError );
	NameManagedTexture( m_pbrSplitSumHandle, "PBR split-sum LUT" );
	if ( m_pbrSplitSumHandle < 0 )
		return false;
	std::vector<float> texels;
	texels.reserve( kSplitSumSize * kSplitSumSize * 4 );
	for ( const SplitSumCoefficients &coefficients : kSplitSumTable )
	{
		texels.push_back( coefficients.a );
		texels.push_back( coefficients.b );
		texels.push_back( 0.0f );
		texels.push_back( 1.0f );
	}
	if ( !UploadManagedTexture( m_pbrSplitSumHandle,
	         reinterpret_cast<const uint8_t *>( texels.data() ), texels.size() * sizeof( float ),
	         outError ) )
		return false;
	SetManagedTextureSamplerState(
	    m_pbrSplitSumHandle, kSamplerClampU | kSamplerClampV | kSamplerLinear );
	if ( m_managedTextures[static_cast<size_t>( m_pbrSplitSumHandle )].descSet == VK_NULL_HANDLE )
	{
		SetError( outError, "PBR split-sum descriptor unavailable" );
		return false;
	}
	const uint32_t *words = m_clipPlanesSupported ? g_pbrDirectClipFragSpv : g_pbrDirectFragSpv;
	const size_t bytes =
	    m_clipPlanesSupported ? sizeof( g_pbrDirectClipFragSpv ) : sizeof( g_pbrDirectFragSpv );
	if ( !CreateShaderModule( words, bytes, &m_pbrDirectFrag, outError ) )
		return false;
	if ( PbrDirectPipeline( DynRasterState() ) == VK_NULL_HANDLE )
	{
		SetError( outError, "vkCreateGraphicsPipelines (PBR direct) failed" );
		return false;
	}
	m_pbrDirectReady = true;
	return true;
}

void CVulkanContext::DestroyPbrDirectPipeline()
{
	for ( const auto &entry : m_pbrDirectPipelines )
	{
		if ( entry.second != VK_NULL_HANDLE )
			vkDestroyPipeline( m_device, entry.second, nullptr );
	}
	m_pbrDirectPipelines.clear();
	if ( m_pbrDirectFrag != VK_NULL_HANDLE )
		vkDestroyShaderModule( m_device, m_pbrDirectFrag, nullptr );
	m_pbrDirectFrag = VK_NULL_HANDLE;
	if ( m_pbrDirectPipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_pbrDirectPipelineLayout, nullptr );
	m_pbrDirectPipelineLayout = VK_NULL_HANDLE;
	if ( m_pbrSplitSumHandle >= 0 )
		DestroyManagedTexture( m_pbrSplitSumHandle );
	m_pbrSplitSumHandle = -1;
	m_pbrDirectReady = false;
}

void CVulkanContext::SetDirectLights( const DirectLight *lights, uint32_t count )
{
	m_directLightCount = std::min( count, kMaxDirectLights );
	for ( uint32_t i = 0; i < m_directLightCount; ++i )
		m_directLights[i] = lights[i];
}

bool CVulkanContext::UploadSkinConstants( std::vector<uint32_t> *offsets )
{
	offsets->clear();
	m_directLightOffset = UINT32_MAX;
	// The frame's direct lights ride in the same ring, one block after the
	// draws' constants.
	// View 3 (the diffuse light) draws through the direct-light variants
	// even without a light.
	const bool lights = ( m_directLightCount > 0 || m_indirectViewMode == 3 ) &&
	                    m_worldPbrExtendedLayout != VK_NULL_HANDLE;
	if ( ( m_dynSkinConstants.empty() && !lights ) || m_skinUbos.empty() )
		return !m_dynSkinConstants.empty() ? false : true;
	const VkDeviceSize block = sizeof( m_dynSkinConstants[0].ps );
	static_assert( sizeof( float[3][4] ) + sizeof( DirectLight ) * kMaxDirectLights <=
	                   sizeof( SkinConstants::ps ),
	    "the direct lights fit one constants block" );
	const VkDeviceSize stride = ( block + m_uboAlignment - 1 ) / m_uboAlignment * m_uboAlignment;
	const VkDeviceSize needed = stride * ( m_dynSkinConstants.size() + ( lights ? 1 : 0 ) );
	// This frame's slot: its fence was waited on when the frame began, so the
	// GPU no longer reads it.
	SkinUniformBuffer &slot = m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()];
	if ( needed > slot.capacity )
	{
		CFrameCostScope cost( m_frameCost, kCostBufferGrow );
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
	if ( lights )
	{
		unsigned char *out =
		    static_cast<unsigned char *>( slot.mapped ) + stride * m_dynSkinConstants.size();
		// world_pbr.frag DirectLights: the count and whether the shadow field
		// is bound, the field's origin and voxel, its dimensions, the lights.
		// z, w: view 3 (the diffuse light) and its exposure.
		float header[3][4] = { { static_cast<float>( m_directLightCount ),
		    m_shadowFieldHandle >= 0 ? 1.0f : 0.0f, m_indirectViewMode == 3 ? 1.0f : 0.0f,
		    m_indirectViewScale } };
		std::memcpy( header[1], m_shadowFieldOrigin, sizeof( header[1] ) );
		std::memcpy( header[2], m_shadowFieldDims, sizeof( header[2] ) );
		std::memcpy( out, header, sizeof( header ) );
		std::memcpy(
		    out + sizeof( header ), m_directLights, sizeof( DirectLight ) * m_directLightCount );
		m_directLightOffset = static_cast<uint32_t>( stride * m_dynSkinConstants.size() );
	}
	return true;
}

void CVulkanContext::DestroySkinPipeline()
{
	for ( const auto &entry : m_skinPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_skinPipelines.clear();
	for ( const auto &entry : m_solidEnergyPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_solidEnergyPipelines.clear();
	for ( const auto &entry : m_paintBlobPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_paintBlobPipelines.clear();
	for ( VkShaderModule *module :
	    { &m_skinVert, &m_skinFrag, &m_solidEnergyVert, &m_solidEnergyFrag, &m_paintBlobFrag } )
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

bool CVulkanContext::InitLightmappedPipeline( std::string *outError )
{
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	if ( m_skinPipelineLayout == VK_NULL_HANDLE || DescriptorSetLimit() < 9 ||
	     properties.limits.maxVertexInputAttributes < 13 )
	{
		SetError( outError, "needs the skin constants, nine descriptor sets and 13 attributes" );
		return false;
	}
	VkPushConstantRange pc = {};
	pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pc.size = kSkinPushBytes;
	// lightmapped.frag: s0 base, s1 lightmap, s2 envmap, s4 bump map, s5 second
	// bump map or envmap mask, s7 second base texture, the constants, s8 bump
	// mask, s12 detail.
	const VkDescriptorSetLayout sets[9] = { m_dynTexDescLayout, m_dynTexDescLayout,
	    m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout,
	    m_skinUboLayout, m_dynTexDescLayout, m_dynTexDescLayout };
	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.setLayoutCount = 9;
	pl.pSetLayouts = sets;
	pl.pushConstantRangeCount = 1;
	pl.pPushConstantRanges = &pc;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &m_lightmappedPipelineLayout ) !=
	     VK_SUCCESS )
	{
		SetError( outError, "vkCreatePipelineLayout (LightmappedGeneric) failed" );
		return false;
	}
	VkShaderModule vert = VK_NULL_HANDLE;
	if ( !CreateShaderModule(
	         g_lightmappedVertSpv, sizeof( g_lightmappedVertSpv ), &vert, outError ) ||
	     !CreateShaderModule(
	         g_lightmappedFragSpv, sizeof( g_lightmappedFragSpv ), &m_lightmappedFrag, outError ) )
	{
		if ( vert != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, vert, nullptr );
		return false;
	}
	// The textured record's position, color, uv, lightmap uv, normal, tangent S
	// (three of the tangent's four floats), alpha and the fog stream, then the
	// tangent T and the bumped lightmap offset.
	std::memcpy( m_lightmappedAttrs, m_texTemplate.attrs, sizeof( m_lightmappedAttrs[0] ) * 11 );
	m_lightmappedAttrs[5].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_lightmappedAttrs[11].location = 11;
	m_lightmappedAttrs[11].binding = 0;
	m_lightmappedAttrs[11].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_lightmappedAttrs[11].offset = sizeof( float ) * 18;
	m_lightmappedAttrs[12].location = 12;
	m_lightmappedAttrs[12].binding = 0;
	m_lightmappedAttrs[12].format = VK_FORMAT_R32_SFLOAT;
	m_lightmappedAttrs[12].offset = sizeof( float ) * 21;
	m_lightmappedVin = m_texTemplate.vin;
	m_lightmappedVin.vertexAttributeDescriptionCount = 13;
	m_lightmappedVin.pVertexAttributeDescriptions = m_lightmappedAttrs;
	m_lightmappedVert = vert;
	if ( LightmappedPipeline( DynRasterState() ) == VK_NULL_HANDLE )
	{
		m_lightmappedVert = VK_NULL_HANDLE;
		vkDestroyShaderModule( m_device, vert, nullptr );
		SetError( outError, "vkCreateGraphicsPipelines (LightmappedGeneric) failed" );
		return false;
	}
	return true;
}

void CVulkanContext::DestroyLightmappedPipeline()
{
	for ( const auto &entry : m_lightmappedPipelines )
	{
		if ( entry.second != VK_NULL_HANDLE )
			vkDestroyPipeline( m_device, entry.second, nullptr );
	}
	m_lightmappedPipelines.clear();
	for ( VkShaderModule *module : { &m_lightmappedVert, &m_lightmappedFrag } )
	{
		if ( *module != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, *module, nullptr );
		*module = VK_NULL_HANDLE;
	}
	if ( m_lightmappedPipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_lightmappedPipelineLayout, nullptr );
	m_lightmappedPipelineLayout = VK_NULL_HANDLE;
}

bool CVulkanContext::InitPostPipeline( std::string *outError )
{
	if ( m_skinPipelineLayout == VK_NULL_HANDLE )
	{
		SetError( outError, "needs the skin layout" );
		return false;
	}
	// The unused color-correction samplers read the white volume.
	if ( m_whiteVolumeHandle < 0 )
	{
		SetError( outError, "needs the white volume" );
		return false;
	}
	VkShaderModule vert = VK_NULL_HANDLE;
	if ( !CreateShaderModule( g_postVertSpv, sizeof( g_postVertSpv ), &vert, outError ) ||
	     !CreateShaderModule( g_postFragSpv, sizeof( g_postFragSpv ), &m_postFrag, outError ) )
	{
		if ( vert != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, vert, nullptr );
		return false;
	}
	m_postVert = vert;
	if ( PostPipeline( DynRasterState() ) == VK_NULL_HANDLE )
	{
		m_postVert = VK_NULL_HANDLE;
		vkDestroyShaderModule( m_device, vert, nullptr );
		SetError( outError, "vkCreateGraphicsPipelines (post-processing) failed" );
		return false;
	}
	return true;
}

void CVulkanContext::DestroyPostPipeline()
{
	for ( const auto &entry : m_postPipelines )
	{
		if ( entry.second != VK_NULL_HANDLE )
			vkDestroyPipeline( m_device, entry.second, nullptr );
	}
	m_postPipelines.clear();
	for ( VkShaderModule *module : { &m_postVert, &m_postFrag } )
	{
		if ( *module != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, *module, nullptr );
		*module = VK_NULL_HANDLE;
	}
}

void CVulkanContext::SetDynamicTransform( const float *m16 )
{
	if ( m16 )
		std::memcpy( m_dynTransform, m16, sizeof( m_dynTransform ) );
}

uint32_t CVulkanContext::MaxSampledTextureDimension() const
{
	if ( m_physicalDevice == VK_NULL_HANDLE )
		return 0;
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	return properties.limits.maxImageDimension2D;
}

VkSampler CVulkanContext::CreateManagedSampler( int state, int anisotropy ) const
{
	VkSamplerCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	const VkFilter filter = ( state & kSamplerLinear ) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	info.magFilter = filter;
	info.minFilter = filter;
	info.mipmapMode = ( state & kSamplerMipLinear ) ? VK_SAMPLER_MIPMAP_MODE_LINEAR
	                                                : VK_SAMPLER_MIPMAP_MODE_NEAREST;
	info.minLod = 0.0f;
	info.maxLod = ( state & ( kSamplerMipPoint | kSamplerMipLinear ) ) ? VK_LOD_CLAMP_NONE : 0.0f;
	info.addressModeU = ( state & kSamplerClampU ) ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
	                                               : VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.addressModeV = ( state & kSamplerClampV ) ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
	                                               : VK_SAMPLER_ADDRESS_MODE_REPEAT;
	// Only volumes read W. D3D9's third coordinate (TEXTUREFLAGS_CLAMPU) is
	// clamped with the first here: the volumes this backend creates (the color
	// correction lookups) clamp all three.
	info.addressModeW = info.addressModeU;
	if ( ( state & kSamplerAnisotropic ) && anisotropy > 1 )
	{
		info.magFilter = VK_FILTER_LINEAR;
		info.minFilter = VK_FILTER_LINEAR;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		info.anisotropyEnable = VK_TRUE;
		info.maxAnisotropy = static_cast<float>( anisotropy );
	}
	VkSampler sampler = VK_NULL_HANDLE;
	return vkCreateSampler( m_device, &info, nullptr, &sampler ) == VK_SUCCESS ? sampler
	                                                                           : VK_NULL_HANDLE;
}

void CVulkanContext::SetAnisotropicLevel( int level )
{
	const int selected = std::clamp( level, 1, m_maxAnisotropy );
	if ( selected == m_anisotropyLevel )
		return;
	if ( m_samplers[0] == VK_NULL_HANDLE )
	{
		m_anisotropyLevel = selected;
		return;
	}
	VkSampler replacement[kSamplerStates / 2] = {};
	for ( int state = kSamplerAnisotropic; state < kSamplerStates; ++state )
	{
		replacement[state - kSamplerAnisotropic] = CreateManagedSampler( state, selected );
		if ( replacement[state - kSamplerAnisotropic] == VK_NULL_HANDLE )
		{
			for ( VkSampler sampler : replacement )
				if ( sampler != VK_NULL_HANDLE )
					vkDestroySampler( m_device, sampler, nullptr );
			Log( "anisotropic sampler update failed; retaining previous level\n" );
			return;
		}
	}
	// Existing descriptor sets can be referenced by submitted frames. Replace
	// their sampler bindings only after those frames have finished.
	{
		CFrameCostScope cost( m_frameCost, kCostDeviceWaitIdle );
		if ( vkDeviceWaitIdle( m_device ) != VK_SUCCESS )
		{
			for ( VkSampler sampler : replacement )
				vkDestroySampler( m_device, sampler, nullptr );
			return;
		}
	}
	for ( int state = kSamplerAnisotropic; state < kSamplerStates; ++state )
	{
		VkSampler old = m_samplers[state];
		m_samplers[state] = replacement[state - kSamplerAnisotropic];
		for ( const ManagedTexture &texture : m_managedTextures )
		{
			if ( texture.samplerState != state )
				continue;
			for ( int srgb = 0; srgb < 2; ++srgb )
			{
				const VkDescriptorSet set = srgb ? texture.descSetSrgb : texture.descSet;
				if ( set == VK_NULL_HANDLE )
					continue;
				VkDescriptorImageInfo image = {};
				image.sampler = m_samplers[state];
				image.imageView = srgb ? texture.srgbView : texture.view;
				image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				VkWriteDescriptorSet write = {};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = set;
				write.dstBinding = 0;
				write.descriptorCount = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.pImageInfo = &image;
				vkUpdateDescriptorSets( m_device, 1, &write, 0, nullptr );
			}
		}
		vkDestroySampler( m_device, old, nullptr );
	}
	// A new sampler may reuse a destroyed one's handle: forget grouped sets
	// keyed by the old ones.
	m_groupedDescriptors.Invalidate();
	m_anisotropyLevel = selected;
}

int CVulkanContext::CreateManagedTexture( int width, int height, VkFormat format,
    std::string *outError, VkImageUsageFlags extraUsage, uint32_t mipLevels, VkFormat srgbAlias,
    bool cube, uint32_t depth )
{
	CFrameCostScope cost( m_frameCost, kCostTextureCreate );
	const bool volume = depth > 1;
	if ( !IsValid() || width <= 0 || height <= 0 || ( cube && width != height ) || depth == 0 ||
	     ( volume && cube ) )
	{
		SetError( outError, "CreateManagedTexture with invalid size or context" );
		return -1;
	}
	if ( volume && ( static_cast<uint32_t>( width ) > m_maxImageDimension3D ||
	                   static_cast<uint32_t>( height ) > m_maxImageDimension3D ||
	                   depth > m_maxImageDimension3D ) )
	{
		SetError( outError, "requested volume texture exceeds the selected device's 3D limit" );
		return -1;
	}
	// A volume is filled whole, one level; its uploads carry every slice.
	if ( volume )
		mipLevels = 1;
	const uint32_t maxDimension = MaxSampledTextureDimension();
	if ( static_cast<uint32_t>( width ) > maxDimension ||
	     static_cast<uint32_t>( height ) > maxDimension )
	{
		SetError( outError, "requested texture exceeds the selected device's 2D image limit" );
		return -1;
	}
	// Managed textures are sampled after a buffer-to-image upload. Both uses
	// must be supported by the selected device format.
	VkFormatProperties fp = {};
	vkGetPhysicalDeviceFormatProperties( m_physicalDevice, format, &fp );
	if ( ( fp.optimalTilingFeatures &
	         ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT ) ) !=
	     ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT ) )
	{
		SetError(
		    outError, "requested texture format cannot be sampled and uploaded on this device" );
		return -1;
	}

	ManagedTexture t;
	t.width = static_cast<uint32_t>( width );
	t.height = static_cast<uint32_t>( height );
	t.layers = cube ? 6 : 1;
	t.depth = depth;
	t.format = format;
	uint32_t fullChain = 1;
	for ( uint32_t size = std::max( t.width, t.height ); size > 1; size >>= 1 )
		++fullChain;
	t.mipLevels = std::max( 1u, std::min( mipLevels, fullChain ) );
	const VkImageUsageFlags usage =
	    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | extraUsage;
	VkImageFormatProperties imageLimits = {};
	const VkResult imageSupport = vkGetPhysicalDeviceImageFormatProperties( m_physicalDevice,
	    format, volume ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage,
	    cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0, &imageLimits );
	if ( imageSupport != VK_SUCCESS || t.width > imageLimits.maxExtent.width ||
	     t.height > imageLimits.maxExtent.height || t.depth > imageLimits.maxExtent.depth ||
	     t.mipLevels > imageLimits.maxMipLevels || t.layers > imageLimits.maxArrayLayers ||
	     !( imageLimits.sampleCounts & VK_SAMPLE_COUNT_1_BIT ) )
	{
		SetError( outError,
		    "requested texture format, extent or mip chain is unsupported on this device" );
		return -1;
	}

	VkImageCreateInfo ii = {};
	ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ii.imageType = volume ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
	ii.format = format;
	ii.extent = { t.width, t.height, t.depth };
	ii.mipLevels = t.mipLevels;
	ii.arrayLayers = t.layers;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = usage;
	ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if ( cube )
		ii.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
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
	iv.viewType = cube     ? VK_IMAGE_VIEW_TYPE_CUBE
	              : volume ? VK_IMAGE_VIEW_TYPE_3D
	                       : VK_IMAGE_VIEW_TYPE_2D;
	iv.format = format;
	iv.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, t.mipLevels, 0, t.layers };
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
	if ( t.mipLevels > 1 || cube )
	{
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		if ( !BeginSingleTimeCommands( &cmd, outError ) )
		{
			ReleaseManagedTextureObjects( t );
			return -1;
		}
		VkImageMemoryBarrier ready = {};
		ready.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		ready.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ready.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ready.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ready.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ready.image = t.image;
		ready.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, t.mipLevels, 0, t.layers };
		ready.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ready );
		if ( !EndSingleTimeCommands( cmd, outError ) )
		{
			ReleaseManagedTextureObjects( t );
			return -1;
		}
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
	return StoreManagedTexture( t );
}

int CVulkanContext::StoreManagedTexture( const ManagedTexture &texture )
{
	if ( !m_freeTextureHandles.empty() )
	{
		const int handle = m_freeTextureHandles.back();
		m_freeTextureHandles.pop_back();
		m_managedTextures[static_cast<size_t>( handle )] = texture;
		return handle;
	}
	m_managedTextures.push_back( texture );
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
	// Texels still waiting to be uploaded to it are of no use now.
	size_t keptUploads = 0;
	for ( const PendingUpload &upload : m_pendingUploads )
		if ( upload.handle != handle )
			m_pendingUploads[keptUploads++] = upload;
	m_pendingUploads.resize( keptUploads );
	if ( m_pendingUploads.empty() )
		m_pendingUploadData.clear();
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
	m_compute.Collect( m_completedSerial );
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
	CFrameCostScope cost( m_frameCost, kCostTextureCreate );
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
	di.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	           ( m_sceneDepthUsable ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0 );
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
	{
		CFrameCostScope cost( m_frameCost, kCostDeviceWaitIdle );
		vkDeviceWaitIdle( m_device );
	}
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
    int handle, const uint8_t *data, size_t dataSize, std::string *outError, uint32_t level,
    uint32_t face )
{
	if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
	{
		SetError( outError, "UploadManagedTexture with invalid handle/data" );
		return false;
	}
	const ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
	return UploadManagedTextureRegion( handle, 0, 0, std::max( 1u, t.width >> level ),
	    std::max( 1u, t.height >> level ), data, dataSize, outError, level, face );
}

// Formats whose samples the hardware decodes from sRGB on every view.
static bool IsSrgbFormat( VkFormat format )
{
	switch ( format )
	{
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		return true;
	default:
		return false;
	}
}

static size_t TextureBlockBytes( VkFormat format )
{
	switch ( format )
	{
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
		return 8;
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
	case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		return 16;
	default:
		return 0;
	}
}

bool CVulkanContext::UploadManagedTextureRegion( int handle, uint32_t x, uint32_t y, uint32_t width,
    uint32_t height, const uint8_t *data, size_t dataSize, std::string *outError, uint32_t level,
    uint32_t face )
{
	CFrameCostScope cost( m_frameCost, kCostTextureUpload );
	m_frameCost.uploadBytes += dataSize;
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
	if ( level >= t.mipLevels || face >= t.layers )
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
	// A volume (8-bit color, or an R16F distance field) is uploaded whole:
	// every slice, tightly packed, one after another.
	const size_t volumeTexelBytes = t.format == VK_FORMAT_R16_SFLOAT ? 2
	                                : t.format == VK_FORMAT_R8G8B8A8_UNORM ||
	                                        t.format == VK_FORMAT_B8G8R8A8_UNORM
	                                    ? 4
	                                    : 0;
	if ( t.depth > 1 &&
	     ( !whole || volumeTexelBytes == 0 ||
	         dataSize != static_cast<size_t>( width ) * height * t.depth * volumeTexelBytes ) )
	{
		SetError( outError, "UploadManagedTexture: a volume texture is uploaded whole" );
		return false;
	}
	// A block-compressed region must start on a 4x4 block and end on one or at
	// the level's edge (vkCmdCopyBufferToImage's rule for compressed images).
	const size_t blockBytes = TextureBlockBytes( t.format );
	const bool compressed = blockBytes != 0;
	if ( compressed && ( x % 4 || y % 4 || ( ( x + width ) % 4 && x + width != levelWidth ) ||
	                       ( ( y + height ) % 4 && y + height != levelHeight ) ) )
	{
		SetError( outError, "UploadManagedTexture: compressed region not block aligned" );
		return false;
	}
	if ( compressed )
	{
		const size_t blocksX = ( static_cast<size_t>( width ) + 3 ) / 4;
		const size_t blocksY = ( static_cast<size_t>( height ) + 3 ) / 4;
		if ( blocksX > std::numeric_limits<size_t>::max() / blocksY / blockBytes ||
		     dataSize != blocksX * blocksY * blockBytes )
		{
			SetError(
			    outError, "UploadManagedTexture: compressed payload size differs from region" );
			return false;
		}
	}
	// A single-level image replaced whole discards its old contents. A level of a
	// chain (made samplable at creation) and an image updated in part (VGUI's
	// font pages, a glyph at a time) keep the texels outside the region.
	PendingUpload upload;
	upload.handle = handle;
	upload.x = x;
	upload.y = y;
	upload.width = width;
	upload.height = height;
	upload.level = level;
	upload.face = face;
	upload.depth = t.depth;
	upload.preserve = t.mipLevels > 1 || t.layers > 1 || ( !whole && t.uploaded );
	// A part of a never-filled single-level image: the rest reads as zero rather
	// than undefined memory.
	upload.clearRest = !whole && !upload.preserve && !compressed;
	upload.size = dataSize;

	if ( dataSize <= kDeferredUploadMaxBytes )
	{
		// Staging offsets keep the 16-byte alignment every format here needs
		// (vkCmdCopyBufferToImage: a multiple of 4 and of the texel block).
		if ( m_pendingUploadData.size() + dataSize + 16 > kPendingUploadCapBytes &&
		     !FlushPendingUploads( outError ) )
			return false;
		upload.offset = ( m_pendingUploadData.size() + 15 ) & ~static_cast<size_t>( 15 );
		m_pendingUploadData.resize( upload.offset + dataSize );
		std::memcpy( m_pendingUploadData.data() + upload.offset, data, dataSize );
		m_pendingUploads.push_back( upload );
		if ( level == 0 )
			t.uploaded = true;
		return true;
	}

	// Synchronous: after whatever is pending, so uploads keep their order.
	if ( !FlushPendingUploads( outError ) )
		return false;
	VkBuffer staging = VK_NULL_HANDLE;
	VkDeviceMemory stagingMem = VK_NULL_HANDLE;
	if ( !CreateBuffer( dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging,
	         &stagingMem, outError ) )
		return false;
	void *mapped = nullptr;
	vkMapMemory( m_device, stagingMem, 0, dataSize, 0, &mapped );
	std::memcpy( mapped, data, dataSize );
	vkUnmapMemory( m_device, stagingMem );

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( !BeginSingleTimeCommands( &cmd, outError ) )
	{
		vkDestroyBuffer( m_device, staging, nullptr );
		vkFreeMemory( m_device, stagingMem, nullptr );
		return false;
	}
	upload.offset = 0;
	RecordTextureUpload( cmd, t.image, upload, staging, 0 );
	const bool ok = EndSingleTimeCommands( cmd, outError );
	vkDestroyBuffer( m_device, staging, nullptr );
	vkFreeMemory( m_device, stagingMem, nullptr );
	if ( ok && level == 0 )
		t.uploaded = true;
	return ok;
}

void CVulkanContext::RecordTextureUpload( VkCommandBuffer cmd, VkImage image,
    const PendingUpload &upload, VkBuffer staging, VkDeviceSize offset )
{
	const uint32_t level = upload.level;
	VkImageMemoryBarrier toDst = {};
	toDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toDst.oldLayout =
	    upload.preserve ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
	toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.image = image;
	toDst.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, level, 1, upload.face, 1 };
	// Earlier submissions' shader reads (a frame still in flight) finish before
	// the copy overwrites the texels, whether or not the contents are kept.
	toDst.srcAccessMask = upload.preserve ? VK_ACCESS_SHADER_READ_BIT : 0;
	toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toDst );
	if ( upload.clearRest )
	{
		const VkClearColorValue zero = {};
		const VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, level, 1, upload.face, 1 };
		vkCmdClearColorImage( cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &zero, 1, &range );
		VkMemoryBarrier cleared = {};
		cleared.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		cleared.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		cleared.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    0, 1, &cleared, 0, nullptr, 0, nullptr );
	}
	VkBufferImageCopy copy = {};
	copy.bufferOffset = offset;
	copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, level, upload.face, 1 };
	copy.imageOffset = { static_cast<int32_t>( upload.x ), static_cast<int32_t>( upload.y ), 0 };
	copy.imageExtent = { upload.width, upload.height, std::max( 1u, upload.depth ) };
	vkCmdCopyBufferToImage( cmd, staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy );
	VkImageMemoryBarrier toRead = toDst;
	toRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	toRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	toRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toRead );
}

bool CVulkanContext::RecordPendingUploads( VkCommandBuffer cmd, StreamBuffer &stream )
{
	if ( m_pendingUploads.empty() )
		return true;
	ScopedDebugLabel label( m_debugUtils, "texture uploads" );
	CFrameCostScope cost( m_frameCost, kCostTextureUpload );
	if ( !EnsureStreamBuffer(
	         stream, m_pendingUploadData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT ) )
		return false;
	std::memcpy( stream.mapped, m_pendingUploadData.data(), m_pendingUploadData.size() );
	for ( const PendingUpload &upload : m_pendingUploads )
	{
		const ManagedTexture &t = m_managedTextures[static_cast<size_t>( upload.handle )];
		if ( t.image != VK_NULL_HANDLE )
			RecordTextureUpload( cmd, t.image, upload, stream.buffer, upload.offset );
	}
	m_pendingUploads.clear();
	m_pendingUploadData.clear();
	return true;
}

bool CVulkanContext::FlushPendingUploads( std::string *outError )
{
	if ( m_pendingUploads.empty() )
		return true;
	// Its own staging buffer: the frame slots' may still be read by the GPU.
	StreamBuffer stream;
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	bool ok = BeginSingleTimeCommands( &cmd, outError );
	if ( ok )
	{
		ok = RecordPendingUploads( cmd, stream );
		ok = EndSingleTimeCommands( cmd, outError ) && ok;
	}
	DestroyStreamBuffer( stream );
	if ( !ok )
	{
		m_pendingUploads.clear();
		m_pendingUploadData.clear();
		SetError( outError, "pending texture uploads failed" );
	}
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
	NoteSceneChanged();
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
	if ( dstHandle == m_sceneCaptureTarget )
		NoteSceneChanged();
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
	const uint64_t readStart = wait ? FrameClockMicros() : 0;
	const VkResult r = vkGetQueryPoolResults( m_device, m_queryPool, static_cast<uint32_t>( query ),
	    1, sizeof( result ), result, sizeof( result ), flags );
	if ( wait )
		m_frameCost.Add( kCostQueryWait, FrameClockMicros() - readStart );
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

CVulkanContext::DynDraw &CVulkanContext::AppendDrawRecord()
{
	// Record this draw with the state current right now (the engine sets the
	// transform/shader/constant per object before each Draw).
	DynDraw &d = AppendRecord( kRecordDraw );
	d.shaderIndex = m_dynShaderIndex;
	// Anything but glass changes what a scene capture of this target holds.
	if ( d.shaderIndex == kDynShaderPbrGlass )
		m_sceneCaptureGlassDrawn = true;
	else
		NoteSceneChanged();
	std::memcpy( d.transform, m_dynTransform, sizeof( d.transform ) );
	std::memcpy( d.color, m_dynConstColor, sizeof( d.color ) );
	std::memcpy( d.modulation, m_dynModulation, sizeof( d.modulation ) );
	d.monitorContrast = m_dynMonitorContrast;
	std::memcpy( d.texXform0, m_dynTexXform0, sizeof( d.texXform0 ) );
	std::memcpy( d.texXform1, m_dynTexXform1, sizeof( d.texXform1 ) );
	std::memcpy( d.pbrAngles, m_dynPbrAngles, sizeof( d.pbrAngles ) );
	d.pbrWorld = m_dynPbrWorld;
	d.raster = m_dynRaster;
	d.depthBiasConstant = m_dynDepthBiasConstant;
	d.depthBiasSlope = m_dynDepthBiasSlope;
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
	d.fog = m_dynFog;
	d.texturedMode = m_dynTexturedMode;
	if ( d.shaderIndex == kDynShaderSkin || d.shaderIndex == kDynShaderSolidEnergy ||
	     d.shaderIndex == kDynShaderPbrModel || d.shaderIndex == kDynShaderLightmapped ||
	     d.shaderIndex == kDynShaderPost || d.shaderIndex == kDynShaderPaintBlob )
	{
		d.skin = static_cast<int>( m_dynSkinConstants.size() );
		m_dynSkinConstants.push_back( m_dynSkin );
	}
	return d;
}

float *CVulkanContext::BeginDynamicDraw(
    uint32_t maxVertices, uint32_t maxIndices, uint32_t **outIndices )
{
	DynDraw &d = AppendDrawRecord();
	d.firstVertex = static_cast<uint32_t>( m_dynQueued.size() / kDynVertexFloats );
	d.vertexCount = maxVertices;
	const size_t start = m_dynQueued.size();
	const size_t grown = start + static_cast<size_t>( maxVertices ) * kDynVertexFloats;
	// Geometric growth: an exact reserve per draw would reallocate (and copy
	// the whole frame's stream) on every draw of a frame larger than the last.
	if ( grown > m_dynQueued.capacity() )
		m_dynQueued.reserve( std::max( grown, m_dynQueued.capacity() * 2 ) );
	m_dynQueued.resize( grown );
	d.firstIndex = static_cast<uint32_t>( m_dynIndices.size() );
	d.indexCount = maxIndices;
	if ( maxIndices > 0 )
	{
		const size_t indices = m_dynIndices.size() + maxIndices;
		if ( indices > m_dynIndices.capacity() )
			m_dynIndices.reserve( std::max( indices, m_dynIndices.capacity() * 2 ) );
		m_dynIndices.resize( indices );
	}
	if ( outIndices )
		*outIndices = maxIndices > 0 ? m_dynIndices.data() + d.firstIndex : nullptr;
	return m_dynQueued.data() + start;
}

void CVulkanContext::EndDynamicDraw( uint32_t vertexCount, uint32_t indexCount )
{
	DynDraw &d = m_dynDrawRecords.back();
	const bool indexed = d.indexCount > 0; // as reserved by BeginDynamicDraw
	vertexCount = std::min( vertexCount, d.vertexCount );
	indexCount = std::min( indexCount, d.indexCount );
	if ( vertexCount == 0 || ( indexed && indexCount == 0 ) )
	{
		// Withdraw the draw and everything it reserved.
		m_dynQueued.resize( static_cast<size_t>( d.firstVertex ) * kDynVertexFloats );
		m_dynIndices.resize( d.firstIndex );
		if ( d.skin >= 0 )
			m_dynSkinConstants.pop_back();
		m_dynDrawRecords.pop_back();
		return;
	}
	m_dynQueued.resize( ( static_cast<size_t>( d.firstVertex ) + vertexCount ) * kDynVertexFloats );
	m_dynIndices.resize( static_cast<size_t>( d.firstIndex ) + indexCount );
	d.vertexCount = vertexCount;
	d.indexCount = indexCount;
}

CVulkanContext::DrawRange CVulkanContext::LastDrawRange() const
{
	DrawRange range = { 0, 0, 0, 0 };
	if ( !m_dynDrawRecords.empty() && m_dynDrawRecords.back().kind == kRecordDraw )
	{
		const DynDraw &d = m_dynDrawRecords.back();
		range.firstVertex = d.firstVertex;
		range.vertexCount = d.vertexCount;
		range.firstIndex = d.firstIndex;
		range.indexCount = d.indexCount;
	}
	return range;
}

bool CVulkanContext::StreamRangeEquals( const DrawRange &range, const float *vertices,
    uint32_t vertexCount, const uint32_t *indices, uint32_t indexCount ) const
{
	const size_t firstFloat = static_cast<size_t>( range.firstVertex ) * kDynVertexFloats;
	const size_t floats = static_cast<size_t>( vertexCount ) * kDynVertexFloats;
	return range.vertexCount == vertexCount && range.indexCount == indexCount &&
	       firstFloat + floats <= m_dynQueued.size() &&
	       static_cast<size_t>( range.firstIndex ) + indexCount <= m_dynIndices.size() &&
	       std::memcmp( m_dynQueued.data() + firstFloat, vertices, floats * sizeof( float ) ) ==
	           0 &&
	       std::memcmp( m_dynIndices.data() + range.firstIndex, indices,
	           indexCount * sizeof( uint32_t ) ) == 0;
}

void CVulkanContext::ReuseDynamicDraw( const DrawRange &range )
{
	if ( range.vertexCount == 0 )
		return;
	DynDraw &d = AppendDrawRecord();
	d.firstVertex = range.firstVertex;
	d.vertexCount = range.vertexCount;
	d.firstIndex = range.firstIndex;
	d.indexCount = range.indexCount;
}

void CVulkanContext::QueueDynamicTriangles( const float *posColorInterleaved, uint32_t vertexCount,
    const float *lightmapUv, const float *normalTangent, const float *vertexAlpha )
{
	if ( !posColorInterleaved || vertexCount == 0 )
		return;
	float *out = BeginDynamicDraw( vertexCount );
	for ( uint32_t v = 0; v < vertexCount; ++v, out += kDynVertexFloats )
	{
		// position (3) + color (3) + uv (2), lightmap uv (2), normal/tangent (7),
		// alpha (1).
		std::memcpy( out, posColorInterleaved + static_cast<size_t>( v ) * 8, 8 * sizeof( float ) );
		out[8] = lightmapUv ? lightmapUv[v * 2 + 0] : 0.0f;
		out[9] = lightmapUv ? lightmapUv[v * 2 + 1] : 0.0f;
		if ( normalTangent )
			std::memcpy(
			    out + 10, normalTangent + static_cast<size_t>( v ) * 7, 7 * sizeof( float ) );
		else
			std::fill( out + 10, out + 17, 0.0f );
		out[17] = vertexAlpha ? vertexAlpha[v] : 1.0f;
		std::fill( out + 18, out + kDynVertexFloats, 0.0f );
	}
	EndDynamicDraw( vertexCount );
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
			t->vertices += d.indexCount > 0 ? d.indexCount : d.vertexCount;
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
		// Elements drawn: an indexed draw renders its index count.
		info.vertexCount = d.indexCount > 0 ? d.indexCount : d.vertexCount;
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
	else if ( m_activeSamples > 1 )
	{
		rp.renderPass = srgb ? m_msPassLoadSrgb : m_msPassLoad;
		rp.framebuffer = srgb ? m_msFramebufferSrgb : m_msFramebuffer;
	}
	else
	{
		rp.renderPass = srgb ? m_renderPassLoadSrgb : m_renderPassLoad;
		rp.framebuffer =
		    srgb ? m_framebuffersSrgb[m_acquiredImage] : m_framebuffers[m_acquiredImage];
	}
	GetTargetExtent( target, &rp.renderArea.extent.width, &rp.renderArea.extent.height );
	if ( m_debugUtils.Active() )
	{
		const std::string &name = IsRenderTargetTexture( target )
		                              ? m_managedTextures[static_cast<size_t>( target )].debugName
		                              : std::string();
		char label[160];
		std::snprintf( label, sizeof( label ), "pass: %s%s",
		    target < 0     ? "back buffer"
		    : name.empty() ? "render target"
		                   : name.c_str(),
		    srgb ? " (sRGB)" : "" );
		m_debugUtils.InsertLabel( label );
	}
	vkCmdBeginRenderPass( cmd, &rp, VK_SUBPASS_CONTENTS_INLINE );
	m_frameCost.Add( kCostRenderPass, 0 );
}

void CVulkanContext::NameManagedTexture( int handle, const char *name )
{
	if ( !m_debugUtils.Active() || !name || handle < 0 ||
	     handle >= static_cast<int>( m_managedTextures.size() ) )
		return;
	ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
	t.debugName = name;
	m_debugUtils.Name( VK_OBJECT_TYPE_IMAGE, t.image, name );
	m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE_VIEW, t.view, "%s view", name );
	m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE_VIEW, t.srgbView, "%s sRGB view", name );
	m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, t.depthImage, "%s depth", name );
	m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE_VIEW, t.depthView, "%s depth view", name );
	m_debugUtils.NameF( VK_OBJECT_TYPE_FRAMEBUFFER, t.framebuffer, "%s", name );
	m_debugUtils.NameF( VK_OBJECT_TYPE_FRAMEBUFFER, t.framebufferSrgb, "%s sRGB", name );
	m_debugUtils.NameF( VK_OBJECT_TYPE_DESCRIPTOR_SET, t.descSet, "%s", name );
	m_debugUtils.NameF( VK_OBJECT_TYPE_DESCRIPTOR_SET, t.descSetSrgb, "%s sRGB", name );
}

void CVulkanContext::QueueFrameLabel( FrameLabelOp op, const char *name, uint32_t argb )
{
	if ( !m_debugUtils.Active() )
		return;
	// The first record after a present starts the next frame (AppendRecord).
	if ( m_dynFramePresented )
		ClearDynamicQueue();
	m_frameLabels.push_back( { m_dynDrawRecords.size(), op, argb, name ? name : "" } );
}

void CVulkanContext::ReplayFrameLabels( size_t *cursor, size_t throughRecord )
{
	for ( ; *cursor < m_frameLabels.size() && m_frameLabels[*cursor].record <= throughRecord;
	    ++*cursor )
	{
		const FrameLabel &label = m_frameLabels[*cursor];
		switch ( label.op )
		{
		case FrameLabelOp::Push:
		{
			// D3DCOLOR (A8R8G8B8); PIX events usually leave alpha at 0.
			const float color[4] = { ( ( label.argb >> 16 ) & 0xff ) / 255.0f,
			    ( ( label.argb >> 8 ) & 0xff ) / 255.0f, ( label.argb & 0xff ) / 255.0f, 1.0f };
			m_debugUtils.PushLabel( label.name.c_str(), label.argb ? color : nullptr );
			break;
		}
		case FrameLabelOp::Pop:
			m_debugUtils.PopLabel();
			break;
		case FrameLabelOp::Insert:
			m_debugUtils.InsertLabel( label.name.c_str() );
			break;
		}
	}
}

// D3D9 (SRGBWRITEENABLE) encodes an sRGB-writing draw in hardware and blends it
// in linear space; such a draw is drawn through the target's sRGB view (the
// hardware decodes, blends and encodes, rounding as D3D9 does). Everything else,
// color clears above all, uses the UNORM view, which stores what it is given.
static bool SrgbCapableShader( int shaderIndex )
{
	return shaderIndex == CVulkanContext::kDynShaderTextured ||
	       shaderIndex == CVulkanContext::kDynShaderPbrDirect ||
	       shaderIndex == CVulkanContext::kDynShaderPbrWorld ||
	       shaderIndex == CVulkanContext::kDynShaderPbrGlass ||
	       shaderIndex == CVulkanContext::kDynShaderPortalRefract ||
	       shaderIndex == CVulkanContext::kDynShaderSkin ||
	       shaderIndex == CVulkanContext::kDynShaderSolidEnergy ||
	       shaderIndex == CVulkanContext::kDynShaderPbrModel ||
	       shaderIndex == CVulkanContext::kDynShaderLightmapped ||
	       shaderIndex == CVulkanContext::kDynShaderPost ||
	       shaderIndex == CVulkanContext::kDynShaderPaintBlob;
}

bool CVulkanContext::RecordWantsSrgb( const DynDraw &r ) const
{
	return m_srgbAttachments && r.kind == kRecordDraw && SrgbCapableShader( r.shaderIndex ) &&
	       ( r.colorFlags & kColorSrgbWrite );
}

// Records whose result is the same through either view: queries, clears of
// depth/stencil alone, and draws that write no RGB (the view only changes how
// RGB is stored; these shaders' alpha test precedes their sRGB encode, and
// each has a pipeline for either view). With merging they stay in the open
// pass, since on a tiled GPU every pass break stores and reloads the target.
bool CVulkanContext::RecordViewAgnostic( const DynDraw &r ) const
{
	if ( r.kind == kRecordQueryBegin || r.kind == kRecordQueryEnd )
		return true;
	if ( !m_passMerging )
		return false;
	if ( r.kind == kRecordClear )
		return !r.clearColor;
	return r.kind == kRecordDraw && !r.raster.colorWrite && SrgbCapableShader( r.shaderIndex );
}

// Whether the frame's clearing pass opens through the back buffer's sRGB view:
// its first color draw writes sRGB, and the clear stores the same bytes through
// either view (each component 0 or 1, which sRGB encoding leaves unchanged).
// The demo draws recorded into that pass have UNORM-only pipelines.
bool CVulkanContext::FirstPassWantsSrgb() const
{
	if ( !m_passMerging || m_renderPassClearSrgb == VK_NULL_HANDLE ||
	     m_dynPipeline == VK_NULL_HANDLE || m_drawDemoTriangle || m_drawTexturedQuad ||
	     m_drawIndexedUbo || m_drawDemoDepth )
		return false;
	for ( float component : m_clearColor.float32 )
		if ( component != 0.0f && component != 1.0f )
			return false;
	for ( const DynDraw &r : m_dynDrawRecords )
	{
		if ( r.target != -1 || r.kind == kRecordCopy || r.kind == kRecordSceneCapture )
			return false;
		if ( !RecordViewAgnostic( r ) )
			return RecordWantsSrgb( r );
	}
	return false;
}

void CVulkanContext::RecordTargetCopy( VkCommandBuffer cmd, int srcTarget, const DynDraw &copy )
{
	ScopedDebugLabel label( m_debugUtils, "copy to render target" );
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
	m_frameCost.Add( kCostTargetCopy, 0 );

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
	for ( const auto &entry : m_worldTexPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_worldTexPipelines.clear();
	DestroyPbrGlassPipeline();
	DestroyPbrWorldPipeline();
	if ( m_worldVert != VK_NULL_HANDLE )
	{
		vkDestroyShaderModule( m_device, m_worldVert, nullptr );
		m_worldVert = VK_NULL_HANDLE;
	}
	for ( const auto &entry : m_portalPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_portalPipelines.clear();
	DestroyPbrModelPipeline();
	DestroyLightmappedPipeline();
	DestroyPostPipeline();
	DestroySkinPipeline();
	DestroyPbrDirectPipeline();
	m_groupedDescriptors.Shutdown();
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
	m_whiteVolumeHandle = -1;
	m_sceneColorHandle = -1;
	m_sceneDepthHandle = -1;
	m_sceneDepthCaptured = false;
	for ( RetiredTexture &r : m_retiredTextures )
		ReleaseManagedTextureObjects( r.texture );
	m_retiredTextures.clear();
	m_computeWork.clear();
	m_compute.Shutdown();
	m_freeTextureHandles.clear();
	m_liveTextureSets = 0;
	if ( m_dynPipelineLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( m_device, m_dynPipelineLayout, nullptr );
		m_dynPipelineLayout = VK_NULL_HANDLE;
	}
	for ( int slot = 0; slot < kMaxFramesInFlight; ++slot )
	{
		DestroyStreamBuffer( m_dynVertexStreams[slot] );
		DestroyStreamBuffer( m_dynIndexStreams[slot] );
		DestroyStreamBuffer( m_dynFogStreams[slot] );
		DestroyStreamBuffer( m_uploadStreams[slot] );
	}
	m_pendingUploads.clear();
	m_pendingUploadData.clear();
	m_dynQueued.clear();
	m_dynIndices.clear();
	++m_streamEpoch;
}

bool CVulkanContext::EnsureStreamBuffer(
    StreamBuffer &stream, VkDeviceSize bytes, VkBufferUsageFlags usage )
{
	if ( bytes <= stream.capacity )
		return stream.mapped != nullptr;
	CFrameCostScope cost( m_frameCost, kCostBufferGrow );
	DestroyStreamBuffer( stream );
	// Headroom so a frame slightly larger than the last does not regrow.
	const VkDeviceSize capacity = bytes + bytes / 2 + 4096;
	std::string error;
	if ( !CreateBuffer( capacity, usage,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &stream.buffer, &stream.memory, &error ) ||
	     vkMapMemory( m_device, stream.memory, 0, capacity, 0, &stream.mapped ) != VK_SUCCESS )
	{
		Log( "stream buffer (%llu bytes) unavailable: %s\n",
		    static_cast<unsigned long long>( capacity ), error.c_str() );
		DestroyStreamBuffer( stream );
		return false;
	}
	stream.capacity = capacity;
	return true;
}

void CVulkanContext::DestroyStreamBuffer( StreamBuffer &stream )
{
	if ( stream.mapped )
		vkUnmapMemory( m_device, stream.memory );
	if ( stream.buffer != VK_NULL_HANDLE )
		vkDestroyBuffer( m_device, stream.buffer, nullptr );
	if ( stream.memory != VK_NULL_HANDLE )
		vkFreeMemory( m_device, stream.memory, nullptr );
	stream = StreamBuffer();
}

bool CVulkanContext::UploadWorldMesh( const void *vertices, size_t vertexBytes, const void *indices,
    size_t indexBytes, std::string *outError )
{
	if ( !IsValid() || !vertices || !indices || !vertexBytes || !indexBytes ||
	     vertexBytes > 512ull * 1024 * 1024 || indexBytes > 512ull * 1024 * 1024 - vertexBytes )
	{
		SetError( outError, "invalid WMSH upload request or Vulkan device" );
		return false;
	}

	StreamBuffer newVertices;
	StreamBuffer newIndices;
	StreamBuffer staging;
	const size_t stagingBytes = vertexBytes + indexBytes;
	if ( !CreateBuffer( vertexBytes,
	         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
	             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &newVertices.buffer, &newVertices.memory,
	         outError ) ||
	     !CreateBuffer( indexBytes,
	         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
	             VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &newIndices.buffer, &newIndices.memory,
	         outError ) ||
	     !CreateBuffer( stagingBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &staging.buffer, &staging.memory, outError ) )
	{
		DestroyStreamBuffer( staging );
		DestroyStreamBuffer( newIndices );
		DestroyStreamBuffer( newVertices );
		return false;
	}
	void *mapped = nullptr;
	const VkResult mapResult = vkMapMemory( m_device, staging.memory, 0, stagingBytes, 0, &mapped );
	if ( mapResult != VK_SUCCESS )
	{
		SetError(
		    outError, std::string( "WMSH staging map failed: " ) + ResultString( mapResult ) );
		DestroyStreamBuffer( staging );
		DestroyStreamBuffer( newIndices );
		DestroyStreamBuffer( newVertices );
		return false;
	}
	staging.mapped = mapped;
	std::memcpy( mapped, vertices, vertexBytes );
	std::memcpy( static_cast<uint8_t *>( mapped ) + vertexBytes, indices, indexBytes );
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( !BeginSingleTimeCommands( &cmd, outError ) )
	{
		DestroyStreamBuffer( staging );
		DestroyStreamBuffer( newIndices );
		DestroyStreamBuffer( newVertices );
		return false;
	}
	const VkBufferCopy vertexCopy = { 0, 0, vertexBytes };
	const VkBufferCopy indexCopy = { vertexBytes, 0, indexBytes };
	vkCmdCopyBuffer( cmd, staging.buffer, newVertices.buffer, 1, &vertexCopy );
	vkCmdCopyBuffer( cmd, staging.buffer, newIndices.buffer, 1, &indexCopy );
	VkBufferMemoryBarrier barriers[2] = {};
	for ( int i = 0; i < 2; ++i )
	{
		barriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barriers[i].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barriers[i].dstAccessMask =
		    i == 0 ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : VK_ACCESS_INDEX_READ_BIT;
		barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[i].buffer = i == 0 ? newVertices.buffer : newIndices.buffer;
		barriers[i].offset = 0;
		barriers[i].size = VK_WHOLE_SIZE;
	}
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	    0, 0, nullptr, 2, barriers, 0, nullptr );
	const bool uploaded = EndSingleTimeCommands( cmd, outError );
	DestroyStreamBuffer( staging );
	if ( !uploaded )
	{
		DestroyStreamBuffer( newIndices );
		DestroyStreamBuffer( newVertices );
		return false;
	}

	if ( WorldMeshResident() )
	{
		const VkResult idle = vkDeviceWaitIdle( m_device );
		if ( idle != VK_SUCCESS )
		{
			SetError(
			    outError, std::string( "WMSH replacement wait failed: " ) + ResultString( idle ) );
			DestroyStreamBuffer( newIndices );
			DestroyStreamBuffer( newVertices );
			return false;
		}
	}
	SetWorldLightmapHandle( -1 );
	SetProbeVolumeHandles( -1, -1, 0 );
	SetProbeDeltaHandle( -1 );
	SetShadowField( -1, nullptr, 0.0f, nullptr );
	SetReflectionProbes( nullptr, 0, 0, 0 );
	DestroyStreamBuffer( m_worldIndexBuffer );
	DestroyStreamBuffer( m_worldVertexBuffer );
	newVertices.capacity = vertexBytes;
	newIndices.capacity = indexBytes;
	m_worldVertexBuffer = newVertices;
	m_worldIndexBuffer = newIndices;
	m_worldVertexCount = static_cast<uint32_t>( vertexBytes / 40 );
	m_worldIndexCount = static_cast<uint32_t>( indexBytes / sizeof( uint32_t ) );
	++m_worldMeshRevision;
	return true;
}

void CVulkanContext::SetWorldLightmapHandle( int handle )
{
	SetWorldLightmapHandles( handle, handle >= 0 ? m_worldLightmapDirectHandle : -1,
	    handle >= 0 ? m_worldLightmapIndirectHandle : -1 );
}

void CVulkanContext::SetWorldLightmapHandles( int total, int direct, int indirect )
{
	int *const slots[3] = {
	    &m_worldLightmapHandle, &m_worldLightmapDirectHandle, &m_worldLightmapIndirectHandle };
	const int values[3] = { total, direct, indirect };
	for ( int i = 0; i < 3; ++i )
	{
		if ( *slots[i] == values[i] )
			continue;
		const int old = *slots[i];
		*slots[i] = values[i];
		if ( old >= 0 )
			DestroyManagedTexture( old );
	}
}

void CVulkanContext::SetProbeDeltaHandle( int atlas )
{
	if ( m_probeDeltaHandle == atlas )
		return;
	const int old = m_probeDeltaHandle;
	m_probeDeltaHandle = atlas;
	if ( old >= 0 )
		DestroyManagedTexture( old );
}

void CVulkanContext::SetShadowField(
    int handle, const float origin[3], float voxel, const uint32_t dims[3] )
{
	if ( m_shadowFieldHandle >= 0 && m_shadowFieldHandle != handle )
		DestroyManagedTexture( m_shadowFieldHandle );
	m_shadowFieldHandle = handle;
	for ( int k = 0; k < 3; ++k )
	{
		m_shadowFieldOrigin[k] = handle >= 0 ? origin[k] : 0.0f;
		m_shadowFieldDims[k] = handle >= 0 ? static_cast<float>( dims[k] ) : 0.0f;
	}
	m_shadowFieldOrigin[3] = handle >= 0 ? voxel : 0.0f;
}

// The mode texel's value (0..7, reflection_probes.h's ReflectionProbeMode)
// as an IEEE binary16; the integers 0..7 are exact halves.
static const uint16_t kReflectionProbeModeHalf[8] = {
    0x0000, 0x3c00, 0x4000, 0x4200, 0x4400, 0x4500, 0x4600, 0x4700 };

bool CVulkanContext::SetReflectionProbes(
    const uint16_t *texels, uint32_t width, uint32_t height, uint32_t count, std::string *outError )
{
	if ( !texels )
	{
		if ( m_reflectionProbeHandle >= 0 )
			DestroyManagedTexture( m_reflectionProbeHandle );
		m_reflectionProbeHandle = -1;
		m_reflectionProbeTexels.clear();
		m_reflectionProbeWidth = m_reflectionProbeHeight = 0;
		return true;
	}
	std::vector<uint16_t> copy( texels, texels + size_t( width ) * height * 4 );
	// Texel 1 of row 0 is the mode and the relight switch
	// (shaders/reflection_probes.glsl); relight applies only to probes whose
	// table carries a relight row.
	copy[4] = kReflectionProbeModeHalf[m_reflectionProbeMode];
	copy[5] = m_reflectionProbeRelight ? 0x3c00 : 0x0000;
	std::string detail;
	const int handle =
	    CreateManagedTexture( int( width ), int( height ), VK_FORMAT_R16G16B16A16_SFLOAT, &detail );
	NameManagedTexture( handle, "RPRB reflection probes" );
	if ( handle < 0 ||
	     !UploadManagedTexture(
	         handle, reinterpret_cast<const uint8_t *>( copy.data() ), copy.size() * 2, &detail ) )
	{
		if ( handle >= 0 )
			DestroyManagedTexture( handle );
		if ( outError )
			*outError = "reflection probe texture upload failed: " + detail;
		return false;
	}
	// The table is read with texelFetch; the atlas with clamped bilinear
	// filtering inside each mip (the shader clamps to texel centres).
	SetManagedTextureSamplerState( handle, kSamplerClampU | kSamplerClampV | kSamplerLinear );
	if ( m_reflectionProbeHandle >= 0 )
		DestroyManagedTexture( m_reflectionProbeHandle );
	m_reflectionProbeHandle = handle;
	m_reflectionProbeTexels.swap( copy );
	m_reflectionProbeWidth = width;
	m_reflectionProbeHeight = height;
	(void)count;
	return true;
}

void CVulkanContext::SetReflectionProbeMode( int mode, bool relight )
{
	// Valid modes are 0..3 and 5..7 (reflection_probes.h); anything else
	// is the default blend.
	if ( mode < 0 || mode > 7 || mode == 4 )
		mode = 1;
	if ( mode == m_reflectionProbeMode && relight == m_reflectionProbeRelight )
		return;
	m_reflectionProbeMode = mode;
	m_reflectionProbeRelight = relight;
	if ( m_reflectionProbeHandle < 0 )
		return;
	// A new texture with the new mode texel; the old one is retired behind
	// the frames that read it.
	std::vector<uint16_t> texels;
	texels.swap( m_reflectionProbeTexels );
	std::string error;
	if ( !SetReflectionProbes(
	         texels.data(), m_reflectionProbeWidth, m_reflectionProbeHeight, 0, &error ) )
	{
		m_reflectionProbeTexels.swap( texels );
		Log( "mat_reflection_probes %d / mat_reflection_relight %d not applied: %s\n", mode,
		    relight ? 1 : 0, error.c_str() );
	}
}

void CVulkanContext::SetProbeVolumeHandles( int atlas, int grids, uint32_t gridCount )
{
	int *const slots[2] = { &m_probeAtlasHandle, &m_probeGridHandle };
	const int values[2] = { atlas, grids };
	for ( int i = 0; i < 2; ++i )
	{
		if ( *slots[i] == values[i] )
			continue;
		const int old = *slots[i];
		*slots[i] = values[i];
		if ( old >= 0 )
			DestroyManagedTexture( old );
	}
	m_probeGridCount = atlas >= 0 && grids >= 0 ? gridCount : 0;
}

bool CVulkanContext::QueueWorldMeshBatch( uint32_t firstIndex, uint32_t indexCount )
{
	if ( !WorldMeshResident() || m_worldVert == VK_NULL_HANDLE || !indexCount ||
	     firstIndex > m_worldIndexCount || indexCount > m_worldIndexCount - firstIndex ||
	     ( m_dynShaderIndex != kDynShaderTextured && m_dynShaderIndex != kDynShaderPbrWorld &&
	         m_dynShaderIndex != kDynShaderPbrGlass ) )
		return false;
	if ( m_dynShaderIndex == kDynShaderPbrWorld || m_dynShaderIndex == kDynShaderPbrGlass )
	{
		const bool ready =
		    m_dynShaderIndex == kDynShaderPbrGlass ? m_pbrGlassReady : m_pbrWorldReady;
		if ( !ready ||
		     !PbrWorldTexturesReady( m_dynBoundTexHandle, m_dynSamplerHandles[1],
		         m_dynSamplerHandles[2], m_dynPbrWorld.material[1] >= 0.5f,
		         ( m_dynColorFlags & kColorSrgbReadBase ) != 0 ) )
			return false;
	}
	// Glass refracts what was drawn before it.
	if ( m_dynShaderIndex == kDynShaderPbrGlass )
		QueueSceneCaptureIfNeeded( m_dynGlassKey );
	DynDraw &draw = AppendDrawRecord();
	draw.worldMesh = true;
	draw.worldMeshRevision = m_worldMeshRevision;
	draw.firstVertex = 0;
	draw.vertexCount = m_worldVertexCount;
	draw.firstIndex = firstIndex;
	draw.indexCount = indexCount;
	return true;
}

bool CVulkanContext::ReadWorldMeshBytes(
    void *vertices, size_t vertexBytes, void *indices, size_t indexBytes, std::string *outError )
{
	if ( !IsValid() || !WorldMeshResident() || !vertices || !indices || !vertexBytes ||
	     !indexBytes || vertexBytes != m_worldVertexBuffer.capacity ||
	     indexBytes != m_worldIndexBuffer.capacity )
	{
		SetError( outError, "invalid WMSH readback request" );
		return false;
	}
	StreamBuffer staging;
	const size_t bytes = vertexBytes + indexBytes;
	if ( !CreateBuffer( bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &staging.buffer, &staging.memory, outError ) )
	{
		DestroyStreamBuffer( staging );
		return false;
	}
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( !BeginSingleTimeCommands( &cmd, outError ) )
	{
		DestroyStreamBuffer( staging );
		return false;
	}
	const VkBufferCopy vertexCopy = { 0, 0, vertexBytes };
	const VkBufferCopy indexCopy = { 0, vertexBytes, indexBytes };
	vkCmdCopyBuffer( cmd, m_worldVertexBuffer.buffer, staging.buffer, 1, &vertexCopy );
	vkCmdCopyBuffer( cmd, m_worldIndexBuffer.buffer, staging.buffer, 1, &indexCopy );
	VkBufferMemoryBarrier hostRead = {};
	hostRead.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	hostRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	hostRead.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
	hostRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	hostRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	hostRead.buffer = staging.buffer;
	hostRead.offset = 0;
	hostRead.size = VK_WHOLE_SIZE;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0,
	    nullptr, 1, &hostRead, 0, nullptr );
	if ( !EndSingleTimeCommands( cmd, outError ) )
	{
		DestroyStreamBuffer( staging );
		return false;
	}
	void *mapped = nullptr;
	const VkResult mapResult = vkMapMemory( m_device, staging.memory, 0, bytes, 0, &mapped );
	if ( mapResult != VK_SUCCESS )
	{
		SetError(
		    outError, std::string( "WMSH readback map failed: " ) + ResultString( mapResult ) );
		DestroyStreamBuffer( staging );
		return false;
	}
	staging.mapped = mapped;
	std::memcpy( vertices, mapped, vertexBytes );
	std::memcpy( indices, static_cast<uint8_t *>( mapped ) + vertexBytes, indexBytes );
	DestroyStreamBuffer( staging );
	return true;
}

void CVulkanContext::ReleaseWorldMesh()
{
	if ( m_device == VK_NULL_HANDLE )
		return;
	++m_worldMeshRevision;
	if ( WorldMeshResident() )
		vkDeviceWaitIdle( m_device );
	SetWorldLightmapHandle( -1 );
	SetProbeVolumeHandles( -1, -1, 0 );
	SetProbeDeltaHandle( -1 );
	SetShadowField( -1, nullptr, 0.0f, nullptr );
	SetReflectionProbes( nullptr, 0, 0, 0 );
	DestroyStreamBuffer( m_worldIndexBuffer );
	DestroyStreamBuffer( m_worldVertexBuffer );
	m_worldVertexCount = 0;
	m_worldIndexCount = 0;
}

bool CVulkanContext::WorldMeshResident() const
{
	return m_worldVertexBuffer.buffer != VK_NULL_HANDLE &&
	       m_worldIndexBuffer.buffer != VK_NULL_HANDLE;
}

bool CVulkanContext::WaitForSubmittedFrame( uint64_t serial, uint64_t timeoutNs )
{
	if ( m_device == VK_NULL_HANDLE || serial == 0 || serial > m_submitSerial )
		return false;
	if ( serial <= m_completedSerial )
		return true;
	// A slot's fence is reset only just before that slot submits again, after
	// BeginFrame waited for it; so a submission not yet known complete still
	// owns its slot's fence.
	for ( uint32_t slot = 0; slot < m_framesInFlight; ++slot )
	{
		if ( m_slotSerial[slot] != serial )
			continue;
		if ( vkWaitForFences( m_device, 1, &m_inFlight[slot], VK_TRUE, timeoutNs ) != VK_SUCCESS )
			return false;
		// One queue completes submissions in order.
		m_completedSerial = std::max( m_completedSerial, serial );
		return true;
	}
	return false;
}

bool CVulkanContext::BeginFrame( bool *outSkip, std::string *outError )
{
	if ( outSkip )
		*outSkip = false;
	m_frameBeginUs = FrameClockMicros();
	m_frameBeginCpuUs = ThreadCpuMicros();
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
	bool surfaceReady = true;
	if ( !EnsureSurfaceCurrent( &surfaceReady, outError ) )
		return false;
	if ( !surfaceReady )
	{
		if ( outSkip )
			*outSkip = true;
		return true;
	}
	// A back-buffer size requested while a frame was open applies now, and a
	// drawable that changed size gets a swapchain of its new size.
	int drawable[2] = { 0, 0 };
	m_host->GetDrawableSize( &drawable[0], &drawable[1] );
	const bool drawableChanged =
	    drawable[0] != m_presentDrawable[0] || drawable[1] != m_presentDrawable[1];
	const bool backBufferPending = m_requestedBackBuffer.width > 0 &&
	                               m_requestedBackBuffer.height > 0 &&
	                               ( m_requestedBackBuffer.width != m_swapExtent.width ||
	                                   m_requestedBackBuffer.height != m_swapExtent.height );
	const bool presentModePending = m_requestedVSync != m_swapchainVSync;
	if ( ( drawableChanged || backBufferPending || presentModePending || m_acquireTimedOut ) &&
	     !RecreateSwapchain( outError ) )
		return false;
	m_acquireTimedOut = false;
	if ( m_swapchain != VK_NULL_HANDLE && !ApplySampleCount( outError ) )
		return false;
	if ( m_swapchain == VK_NULL_HANDLE )
	{
		// Zero-size (minimized) window: nothing to render this iteration.
		if ( outSkip )
			*outSkip = true;
		return true;
	}

	{
		CFrameCostScope wait( m_frameCost, kCostFenceWait );
		vkWaitForFences( m_device, 1, &m_inFlight[m_currentFrame], VK_TRUE, UINT64_MAX );
	}
	// One queue completes submissions in order: this slot's is done, so are all
	// before it.
	m_completedSerial = std::max( m_completedSerial, m_slotSerial[m_currentFrame] );
	RetireCompletedTextures();
	ReadSlotGpuTime( m_currentFrame );
	// The slot's grouped sets are no longer read: they return to its pools.
	m_groupedDescriptors.BeginFrame( m_currentFrame );

	uint32_t imageIndex = 0;
	VkResult r;
	{
		CFrameCostScope acquire( m_frameCost, kCostAcquire );
		r = vkAcquireNextImageKHR( m_device, m_swapchain, kAcquireTimeoutNs,
		    m_imageAvailable[m_currentFrame], VK_NULL_HANDLE, &imageIndex );
	}
	if ( r == VK_TIMEOUT || r == VK_NOT_READY )
	{
		// The window system kept every image (a Wayland compositor can hold a
		// FIFO swapchain's buffers indefinitely around a resize). Skip this frame
		// and replace the swapchain, which returns its buffers, instead of
		// blocking forever.
		++m_acquireTimeouts;
		Log( "swapchain image not released within %llu ms; recreating the swapchain\n",
		    static_cast<unsigned long long>( kAcquireTimeoutNs / 1000000 ) );
		m_acquireTimedOut = true;
		if ( outSkip )
			*outSkip = true;
		return true;
	}
	if ( r == VK_ERROR_SURFACE_LOST_KHR )
	{
		// The next frame replaces the surface (EnsureSurfaceCurrent).
		m_surfaceLost = true;
		if ( outSkip )
			*outSkip = true;
		return true;
	}
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
	{
		CFrameCostScope wait( m_frameCost, kCostFenceWait );
		vkWaitForFences( m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX );
	}
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
	m_debugUtils.BeginFrameCommands( cmd );
	m_recordBeginUs = FrameClockMicros();
	if ( m_timestampPool != VK_NULL_HANDLE )
	{
		const uint32_t first = m_currentFrame * 2;
		vkCmdResetQueryPool( cmd, m_timestampPool, first, 2 );
		vkCmdWriteTimestamp( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_timestampPool, first );
	}
	// Compute work queued since the last frame (RFC 0011 G5), ahead of every
	// render pass; its barriers make the writes visible to this frame's draws.
	if ( !m_computeWork.empty() )
	{
		ScopedDebugLabel label( m_debugUtils, "compute work" );
		for ( auto &work : m_computeWork )
			work( cmd, m_submitSerial + 1 );
	}
	m_computeWork.clear();
	// Texel uploads deferred since the last frame, ahead of every draw. This
	// slot's staging buffer is free: its fence was waited on above.
	if ( !RecordPendingUploads( cmd, m_uploadStreams[m_currentFrame] ) )
	{
		Log( "deferred texture uploads dropped: no staging buffer\n" );
		m_pendingUploads.clear();
		m_pendingUploadData.clear();
	}

	// Queries are reset outside any render pass, before the stream that issues
	// them replays. Only the slots this stream issues are reset, so a result of
	// an earlier frame that the engine has yet to read survives.
	m_replayedQueries.clear();
	m_lastFrameSceneCaptures = 0;
	m_lastFrameSceneDepthCaptures = 0;
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

	const bool firstPassSrgb = FirstPassWantsSrgb();
	VkRenderPassBeginInfo rp = {};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	if ( m_activeSamples > 1 )
	{
		rp.renderPass = firstPassSrgb ? m_msPassClearSrgb : m_msPassClear;
		rp.framebuffer = firstPassSrgb ? m_msFramebufferSrgb : m_msFramebuffer;
	}
	else
	{
		rp.renderPass = firstPassSrgb ? m_renderPassClearSrgb : m_renderPass;
		rp.framebuffer =
		    firstPassSrgb ? m_framebuffersSrgb[imageIndex] : m_framebuffers[imageIndex];
	}
	rp.renderArea.offset = { 0, 0 };
	rp.renderArea.extent = m_swapExtent;
	rp.clearValueCount = 2;
	rp.pClearValues = clears;
	m_debugUtils.InsertLabel(
	    firstPassSrgb ? "pass: back buffer, cleared (sRGB)" : "pass: back buffer, cleared" );
	vkCmdBeginRenderPass( cmd, &rp, VK_SUBPASS_CONTENTS_INLINE );
	m_frameCost.Add( kCostRenderPass, 0 );

	if ( m_drawDemoTriangle && m_demoPipeline != VK_NULL_HANDLE && m_activeSamples == 1 )
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

	if ( m_drawTexturedQuad && m_texQuadPipeline != VK_NULL_HANDLE && m_activeSamples == 1 )
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

	if ( m_drawIndexedUbo && m_indexedUboPipeline != VK_NULL_HANDLE && m_activeSamples == 1 )
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

	if ( m_drawDemoDepth && m_demoDepthPipeline != VK_NULL_HANDLE && m_activeSamples == 1 )
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
		const VkDeviceSize indexBytes = m_dynIndices.size() * sizeof( uint32_t );
		StreamBuffer &vertexStream = m_dynVertexStreams[m_currentFrame];
		StreamBuffer &indexStream = m_dynIndexStreams[m_currentFrame];
		const bool geometryOk = needed == 0 || EnsureStreamBuffer( vertexStream, needed,
		                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );
		const bool indicesOk = indexBytes == 0 || EnsureStreamBuffer( indexStream, indexBytes,
		                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT );
		if ( geometryOk && needed > 0 )
		{
			std::memcpy( vertexStream.mapped, m_dynQueued.data(), needed );
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers( cmd, 0, 1, &vertexStream.buffer, &offset );
		}
		if ( indicesOk && indexBytes > 0 )
		{
			std::memcpy( indexStream.mapped, m_dynIndices.data(), indexBytes );
			vkCmdBindIndexBuffer( cmd, indexStream.buffer, 0, VK_INDEX_TYPE_UINT32 );
		}

		// Replay the stream in engine order. Each record names the target it was
		// issued against; a change of target closes the open pass and opens the
		// new target's, so a portal view rendered into _rt_portal1 lands there and
		// is finished before the main view samples it.
		// The draws' fog, one record per run of draws that share it, in this
		// frame's instance stream (vertex binding 1 of the textured and world
		// pipelines); each draw's first instance selects its record.
		std::vector<uint32_t> fogIndex( m_dynDrawRecords.size(), 0 );
		{
			std::vector<DrawFog> fogRecords( 1 ); // record 0: no fog
			for ( size_t i = 0; i < m_dynDrawRecords.size(); ++i )
			{
				const DynDraw &r = m_dynDrawRecords[i];
				if ( r.kind != kRecordDraw || r.fog.color[3] < 0.0f )
					continue;
				if ( !( r.fog == fogRecords.back() ) )
					fogRecords.push_back( r.fog );
				fogIndex[i] = static_cast<uint32_t>( fogRecords.size() - 1 );
			}
			StreamBuffer &fogStream = m_dynFogStreams[m_currentFrame];
			const VkDeviceSize fogBytes = fogRecords.size() * sizeof( DrawFog );
			if ( EnsureStreamBuffer( fogStream, fogBytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) )
			{
				std::memcpy( fogStream.mapped, fogRecords.data(), fogBytes );
				VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers( cmd, 1, 1, &fogStream.buffer, &offset );
			}
			else
				std::fill( fogIndex.begin(), fogIndex.end(), 0u );
		}
		// The skin draws' pixel shader constants, in this frame's uniform buffer.
		std::vector<uint32_t> skinOffsets;
		const bool skinConstantsOk = UploadSkinConstants( &skinOffsets );
		int openTarget = -1; // the swapchain pass opened above
		bool openSrgb = firstPassSrgb; // entered through the target's sRGB view
		VkPipeline boundPipeline = VK_NULL_HANDLE;
		bool worldBuffersBound = false;
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
		// The view a record is drawn through. A view-agnostic record keeps the
		// open view; opening a pass (another target, or a query, which must share
		// one pass with the draws it counts) takes the view of the next record
		// that needs one.
		const auto viewFor = [&]( size_t index )
		{
			const DynDraw &r = m_dynDrawRecords[index];
			if ( !RecordViewAgnostic( r ) )
				return RecordWantsSrgb( r );
			if ( r.kind == kRecordQueryBegin || r.target != openTarget )
			{
				for ( size_t next = index + 1; next < m_dynDrawRecords.size(); ++next )
				{
					const DynDraw &n = m_dynDrawRecords[next];
					if ( n.target != r.target || n.kind == kRecordCopy ||
					     n.kind == kRecordSceneCapture )
						break;
					if ( !RecordViewAgnostic( n ) )
						return RecordWantsSrgb( n );
				}
			}
			return r.target == openTarget && openSrgb;
		};
		// A copy closes the pass; the next record reopens one, so back-to-back
		// copies open no empty pass between them. A copy identical to the one
		// just made, with nothing drawn or cleared since, is skipped. Without
		// merging the pass reopens at once and every copy is made.
		bool passOpen = true;
		const DynDraw *lastCopy = nullptr;
		// Whether the latest scene capture holds this view's depth (glass reads it).
		bool sceneDepthValid = false;
		size_t labelCursor = 0;
		for ( size_t recordIndex = 0; recordIndex < m_dynDrawRecords.size(); ++recordIndex )
		{
			ReplayFrameLabels( &labelCursor, recordIndex );
			const DynDraw &d = m_dynDrawRecords[recordIndex];
			// A render target deleted after these records were issued: what was
			// rendered into it is discarded, as it would be on D3D9.
			if ( ( d.target >= 0 && !IsRenderTargetTexture( d.target ) ) ||
			     ( d.kind == kRecordCopy && !IsRenderTargetTexture( d.copyDst ) ) )
				continue;
			if ( d.kind == kRecordCopy )
			{
				endActiveQuery( false );
				if ( passOpen )
					vkCmdEndRenderPass( cmd );
				passOpen = false;
				const bool repeat = m_passMerging && lastCopy && lastCopy->copyDst == d.copyDst &&
				                    std::memcmp( lastCopy->copySrcRect, d.copySrcRect,
				                        sizeof( d.copySrcRect ) ) == 0 &&
				                    std::memcmp( lastCopy->copyDstRect, d.copyDstRect,
				                        sizeof( d.copyDstRect ) ) == 0;
				if ( !repeat )
				{
					if ( openTarget == -1 && m_activeSamples > 1 )
						ResolveBackBuffer( cmd, m_acquiredImage );
					RecordTargetCopy( cmd, openTarget, d );
				}
				lastCopy = &d;
				if ( !m_passMerging )
				{
					BeginTargetPass( cmd, openTarget, openSrgb );
					passOpen = true;
				}
				continue;
			}
			if ( d.kind == kRecordSceneCapture )
			{
				// Like a copy, outside any pass, of the target the glass after it
				// draws into (the open one, or the record's own if none is open).
				endActiveQuery( false );
				if ( passOpen )
					vkCmdEndRenderPass( cmd );
				passOpen = false;
				if ( d.target != openTarget )
					openTarget = d.target;
				if ( openTarget == -1 && m_activeSamples > 1 )
					ResolveBackBuffer( cmd, m_acquiredImage );
				sceneDepthValid = RecordSceneCapture( cmd, openTarget );
				lastCopy = nullptr;
				continue;
			}
			if ( d.kind == kRecordDraw || d.kind == kRecordClear )
				lastCopy = nullptr;
			const bool wantSrgb = viewFor( recordIndex );
			if ( !passOpen || d.target != openTarget || wantSrgb != openSrgb )
			{
				endActiveQuery( false );
				if ( passOpen )
					vkCmdEndRenderPass( cmd );
				openTarget = d.target;
				openSrgb = wantSrgb;
				BeginTargetPass( cmd, openTarget, openSrgb );
				passOpen = true;
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

			if ( d.worldMesh &&
			     ( !WorldMeshResident() || d.worldMeshRevision != m_worldMeshRevision ) )
				continue;
			if ( ( !geometryOk && !d.worldMesh ) || d.vertexCount == 0 )
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

			// The multisampled back buffer's passes take pipelines of its sample
			// count; render targets stay single-sampled.
			const int passSamples = openTarget == -1 ? m_activeSamples : 1;
			VkPipeline selected = passSamples == 1 ? m_dynPipeline : VK_NULL_HANDLE;
			VkPipelineLayout selectedLayout = m_dynPipelineLayout;
			bool textured = false;
			bool pbrDirect = false;
			bool pbrWorld = false;
			bool glass = false; // pbrWorld's sets and block, plus the scene capture
			bool portal = false;
			bool skin = false;
			bool solidEnergy = false;
			bool paintBlob = false;
			bool pbrModel = false;
			bool pbrModelEnv = false;
			bool pbrModelProbe = false;
			bool lightmapped = false; // on skin's push block and constants
			bool post = false;        // on the skin layout
			bool pbrWorldLights = false;
			bool pbrWorldRuntime = false;
			bool pbrWorldDelta = false;
			// sRGB inputs decoded by the sampler and output encoded by the view,
			// which the shader must then not apply itself.
			int decodedFlags = openSrgb ? kColorSrgbWrite : 0;
			// The test-catalog pipelines (greenify, constant color, passthrough)
			// are single-sampled; under multisampling their draws are declined.
			if ( d.shaderIndex == kDynShaderGreenify && m_dynPipelineGreen != VK_NULL_HANDLE )
				selected = passSamples == 1 ? m_dynPipelineGreen : VK_NULL_HANDLE;
			else if ( d.shaderIndex == kDynShaderConstColor &&
			          m_dynPipelineConst != VK_NULL_HANDLE )
				selected = passSamples == 1 ? m_dynPipelineConst : VK_NULL_HANDLE;
			else if ( d.shaderIndex == kDynShaderTextured )
			{
				// The textured pipeline built for this draw's blend/depth state.
				selected = d.worldMesh ? WorldTexturedPipeline( d.raster, openSrgb, passSamples )
				                       : TexturedPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE )
					continue;
				selectedLayout = m_dynTexPipelineLayout;
				textured = true;
			}
			else if ( d.shaderIndex == kDynShaderPbrDirect )
			{
				selected = d.worldMesh ? VK_NULL_HANDLE
				                       : PbrDirectPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE )
					continue;
				selectedLayout = m_pbrDirectPipelineLayout;
				pbrDirect = true;
			}
			else if ( d.shaderIndex == kDynShaderPbrWorld )
			{
				if ( !PbrWorldTexturesReady( d.texHandle, d.samplerHandles[1], d.samplerHandles[2],
				         d.pbrWorld.material[1] >= 0.5f,
				         ( d.colorFlags & kColorSrgbReadBase ) != 0 ) )
					continue;
				// The extended variants: this frame's direct lights, and the
				// RuntimeIndirect policy (render/indirect_policy.h).
				pbrWorldLights = m_directLightOffset != UINT32_MAX && !IndirectViewShading();
				pbrWorldRuntime = !IndirectViewShading() && EffectiveIndirectPolicy() == 2;
				// BakedPlusDelta: the producer's change volume, also in the
				// indirect view.
				pbrWorldDelta = EffectiveIndirectPolicy() == 1 && ProbeDeltaResident() &&
				                m_worldPbrDeltaLayout != VK_NULL_HANDLE;
				const int extended = ( pbrWorldLights ? kWorldPbrDirectLights : 0 ) |
				                     ( pbrWorldRuntime ? kWorldPbrRuntimeIndirect : 0 ) |
				                     ( pbrWorldDelta ? kWorldPbrDeltaVolume : 0 );
				selected = d.worldMesh
				               ? WorldPbrPipeline( d.raster, openSrgb, passSamples, extended )
				               : VK_NULL_HANDLE;
				if ( selected == VK_NULL_HANDLE )
					continue;
				selectedLayout = extended ? m_worldPbrExtendedLayout : m_worldPbrPipelineLayout;
				pbrWorld = true;
			}
			else if ( d.shaderIndex == kDynShaderPbrGlass )
			{
				if ( !d.worldMesh || m_sceneColorHandle < 0 ||
				     !PbrWorldTexturesReady( d.texHandle, d.samplerHandles[1], d.samplerHandles[2],
				         d.pbrWorld.material[1] >= 0.5f,
				         ( d.colorFlags & kColorSrgbReadBase ) != 0 ) )
					continue;
				selected = WorldGlassPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE )
					continue;
				selectedLayout = m_worldGlassPipelineLayout;
				pbrWorld = true;
				glass = true;
			}
			else if ( d.shaderIndex == kDynShaderPortalRefract )
			{
				selected = PortalPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE )
					continue;
				selectedLayout = m_portalPipelineLayout;
				portal = true;
			}
			else if ( d.shaderIndex == kDynShaderSkin )
			{
				selected = SkinPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE || d.skin < 0 || !skinConstantsOk ||
				     static_cast<size_t>( d.skin ) >= skinOffsets.size() )
					continue;
				selectedLayout = m_skinPipelineLayout;
				skin = true;
			}
			else if ( d.shaderIndex == kDynShaderSolidEnergy )
			{
				selected = SolidEnergyPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE || d.skin < 0 || !skinConstantsOk ||
				     static_cast<size_t>( d.skin ) >= skinOffsets.size() )
					continue;
				selectedLayout = m_skinPipelineLayout;
				// The skin layout, push block and constants; its own samplers.
				skin = true;
				solidEnergy = true;
			}
			else if ( d.shaderIndex == kDynShaderPaintBlob )
			{
				selected = PaintBlobPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE || d.skin < 0 || !skinConstantsOk ||
				     static_cast<size_t>( d.skin ) >= skinOffsets.size() )
					continue;
				selectedLayout = m_skinPipelineLayout;
				// The skin layout, vertex stage, push block and constants; its
				// own pixel stage and samplers.
				skin = true;
				paintBlob = true;
			}
			else if ( d.shaderIndex == kDynShaderPbrModel )
			{
				const SkinConstants *c =
				    d.skin >= 0 && static_cast<size_t>( d.skin ) < m_dynSkinConstants.size()
				        ? &m_dynSkinConstants[static_cast<size_t>( d.skin )]
				        : nullptr;
				// The indirect view never reads the $envmap cube.
				pbrModelEnv = m_indirectViewMode == 0 && c && ( c->combos & kPbrModelEnvMap ) != 0;
				// The map's probe volume, per pixel, where the device supports it.
				pbrModelProbe =
				    m_probeSampling != 0 && ProbeVolumeResident() && ProbeVolumeSamplingSupported();
				selected =
				    PbrModelPipeline( d.raster, pbrModelEnv, openSrgb, passSamples, pbrModelProbe );
				if ( selected == VK_NULL_HANDLE || !c || !skinConstantsOk ||
				     static_cast<size_t>( d.skin ) >= skinOffsets.size() )
					continue;
				selectedLayout = m_pbrModelPipelineLayout;
				// The skin push block and constants; the grouped PBR sets.
				skin = true;
				pbrModel = true;
			}
			else if ( d.shaderIndex == kDynShaderLightmapped )
			{
				selected = d.worldMesh ? VK_NULL_HANDLE
				                       : LightmappedPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE || d.skin < 0 || !skinConstantsOk ||
				     static_cast<size_t>( d.skin ) >= skinOffsets.size() )
					continue;
				selectedLayout = m_lightmappedPipelineLayout;
				// The skin push block and constants; its own layout and samplers.
				skin = true;
				lightmapped = true;
			}
			else if ( d.shaderIndex == kDynShaderPost )
			{
				selected =
				    d.worldMesh ? VK_NULL_HANDLE : PostPipeline( d.raster, openSrgb, passSamples );
				if ( selected == VK_NULL_HANDLE || d.skin < 0 || !skinConstantsOk ||
				     static_cast<size_t>( d.skin ) >= skinOffsets.size() )
					continue;
				selectedLayout = m_skinPipelineLayout;
				skin = true;
				post = true;
			}
			if ( selected == VK_NULL_HANDLE )
				continue;
			if ( selected != boundPipeline )
			{
				vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, selected );
				boundPipeline = selected;
			}
			if ( textured || pbrDirect || pbrWorld || portal || skin )
			{
				vkCmdSetDepthBias( cmd, d.depthBiasConstant, 0.0f, d.depthBiasSlope );
				vkCmdSetStencilCompareMask(
				    cmd, VK_STENCIL_FACE_FRONT_AND_BACK, d.stencilTestMask );
				vkCmdSetStencilWriteMask( cmd, VK_STENCIL_FACE_FRONT_AND_BACK, d.stencilWriteMask );
				vkCmdSetStencilReference( cmd, VK_STENCIL_FACE_FRONT_AND_BACK, d.stencilRef );
			}
			if ( textured || pbrDirect || pbrWorld || portal || skin )
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
				else if ( solidEnergy )
				{
					// s0 base (sRGB), s1 detail 1 and s4 detail 2 (sRGB, decoded
					// by solidenergy.frag), s5 flow map, s6 flow noise, s7 flow
					// bounds, then this draw's constants.
					const VkDescriptorSet sets[7] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( d.samplerHandles[1], 0 ), sampledSet( d.samplerHandles[4], 0 ),
					    sampledSet( d.samplerHandles[5], 0 ), sampledSet( d.samplerHandles[6], 0 ),
					    sampledSet( d.samplerHandles[7], 0 ),
					    m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()].set };
					const uint32_t offset = skinOffsets[static_cast<size_t>( d.skin )];
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_skinPipelineLayout, 0, 7, sets, 1, &offset );
				}
				else if ( paintBlob )
				{
					// shaders/paintblob.frag: s0 base (sRGB), s1 bump, s3 spec
					// mask, s7 environment cube (the white cube without one; the
					// push block then clears kPaintBlobEnvMap), s4 light warp, s2
					// the frame copy, then this draw's constants.
					const int envmap = ManagedTextureIsCube( d.samplerHandles[7] )
					                       ? d.samplerHandles[7]
					                       : m_whiteCubeHandle;
					const VkDescriptorSet sets[7] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( d.samplerHandles[1], 0 ), sampledSet( d.samplerHandles[3], 0 ),
					    sampledSet( envmap, kColorSrgbReadSampler7 ),
					    sampledSet( d.samplerHandles[4], 0 ), sampledSet( d.samplerHandles[2], 0 ),
					    m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()].set };
					const uint32_t offset = skinOffsets[static_cast<size_t>( d.skin )];
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_skinPipelineLayout, 0, 7, sets, 1, &offset );
				}
				else if ( pbrModel )
				{
					// shaders/model_pbr.frag's grouped sets. Frame: the split-sum
					// table, the map's LMAP atlas (whose probe marker the shader
					// checks), and with the probe volume its atlas and grid table.
					// Material: s0 base (sRGB), s10 MRAO, s1 normal, s2 emission
					// (decoded by the shader), the $envmap cube (s3). Then the
					// constants.
					// A base stored in an sRGB format (a KTX2 BC7 sRGB package)
					// is decoded by the sampler through any view; the shader must
					// not decode it again.
					if ( d.texHandle >= 0 &&
					     d.texHandle < static_cast<int>( m_managedTextures.size() ) &&
					     IsSrgbFormat(
					         m_managedTextures[static_cast<size_t>( d.texHandle )].format ) )
						decodedFlags |= kColorSrgbReadBase;
					const CGroupedDescriptors::Image frame[CGroupedDescriptors::kFrameBindings] = {
					    GroupedImage( m_pbrSplitSumHandle, -1, openTarget, false ),
					    GroupedImage(
					        pbrModelEnv ? -1 : m_worldLightmapHandle, -1, openTarget, false ),
					    GroupedImage(
					        pbrModelProbe ? m_probeAtlasHandle : -1, -1, openTarget, false ),
					    GroupedImage(
					        pbrModelProbe ? m_probeGridHandle : -1, -1, openTarget, false ),
					    GroupedImage( -1, m_whiteVolumeHandle, openTarget, false ),
					    GroupedImage(
					        pbrModelEnv ? -1 : m_reflectionProbeHandle, -1, openTarget, false ) };
					bool baseSrgb = false;
					const CGroupedDescriptors::Image
					    material[CGroupedDescriptors::kMaterialBindings] = {
					        GroupedImage( d.texHandle, -1, openTarget,
					            ( d.colorFlags & kColorSrgbReadBase ) != 0, &baseSrgb ),
					        GroupedImage( d.samplerHandles[10], -1, openTarget, false ),
					        GroupedImage( d.samplerHandles[1], -1, openTarget, false ),
					        GroupedImage( d.samplerHandles[2], -1, openTarget, false ),
					        GroupedImage( pbrModelEnv && ManagedTextureIsCube( d.samplerHandles[3] )
					                          ? d.samplerHandles[3]
					                          : m_whiteCubeHandle,
					            m_whiteCubeHandle, openTarget, false ),
					        GroupedImage( -1, -1, openTarget, false ) };
					if ( baseSrgb )
						decodedFlags |= kColorSrgbReadBase;
					const VkDescriptorSet sets[3] = {
					    m_groupedDescriptors.Acquire( CGroupedDescriptors::kFrameGroup, frame ),
					    m_groupedDescriptors.Acquire(
					        CGroupedDescriptors::kMaterialGroup, material ),
					    m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()].set };
					if ( sets[0] == VK_NULL_HANDLE || sets[1] == VK_NULL_HANDLE )
						continue;
					const uint32_t offset = skinOffsets[static_cast<size_t>( d.skin )];
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_pbrModelPipelineLayout, 0, 3, sets, 1, &offset );
				}
				else if ( lightmapped )
				{
					// shaders/lightmapped.frag: s0 base, s1 lightmap, s2 envmap
					// (a cube, the white cube without one), s4 bump map, s5 second
					// bump map or envmap mask, s7 second base, the constants, s8
					// bump mask, s12 detail.
					const int envmap = ManagedTextureIsCube( d.samplerHandles[2] )
					                       ? d.samplerHandles[2]
					                       : m_whiteCubeHandle;
					const VkDescriptorSet sets[9] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( d.lightmapHandle, kColorSrgbReadLightmap ),
					    sampledSet( envmap, kColorSrgbReadSampler2 ),
					    sampledSet( d.samplerHandles[4], 0 ), sampledSet( d.samplerHandles[5], 0 ),
					    sampledSet( d.samplerHandles[7], kColorSrgbReadSampler7 ),
					    m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()].set,
					    sampledSet( d.samplerHandles[8], 0 ),
					    sampledSet( d.samplerHandles[12], kColorSrgbReadSampler12 ) };
					const uint32_t offset = skinOffsets[static_cast<size_t>( d.skin )];
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_lightmappedPipelineLayout, 0, 9, sets, 1, &offset );
				}
				else if ( post )
				{
					// shaders/screenspace_post.frag: s0 source, s1 frame buffer,
					// s2..s5 color-correction volumes (the white volume without
					// one), then the constants.
					const auto volume = [&]( int handle )
					{
						return sampledSet(
						    ManagedTextureIsVolume( handle ) ? handle : m_whiteVolumeHandle, 0 );
					};
					const VkDescriptorSet sets[7] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( d.samplerHandles[1], kColorSrgbReadLightmap ),
					    volume( d.samplerHandles[2] ), volume( d.samplerHandles[3] ),
					    volume( d.samplerHandles[4] ), volume( d.samplerHandles[5] ),
					    m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()].set };
					const uint32_t offset = skinOffsets[static_cast<size_t>( d.skin )];
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_skinPipelineLayout, 0, 7, sets, 1, &offset );
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
				else if ( pbrDirect )
				{
					const VkDescriptorSet sets[3] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( d.samplerHandles[1], 0 ),
					    m_managedTextures[static_cast<size_t>( m_pbrSplitSumHandle )].descSet };
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_pbrDirectPipelineLayout, 0, 3, sets, 0, nullptr );
				}
				else if ( pbrWorld )
				{
					// The grouped sets (world_pbr.frag, world_pbr_glass.frag).
					// Frame: the split-sum table, the lightmap, the variant's
					// indirect source and grid table, the direct lights' shadow
					// field. Material: base, MRAO, normal, then the emission map
					// and the $envmap cube, or for glass the scene capture's color
					// (through its sRGB view: linear light) and depth. A slot the
					// variant does not read holds the built-in texture of its
					// dimension; without a depth capture the push block tells
					// glass not to read it.
					const int environment = d.samplerHandles[kPbrWorldEnvironmentSampler];
					// The indirect view samples the LMAP indirect layer where the
					// lightmap would be; without one it binds the total page and
					// the push block says the layer is absent.
					const bool indirectViewLayer =
					    !glass && IndirectViewShading() && m_worldLightmapIndirectHandle >= 0;
					// RuntimeIndirect reads the direct layer (the seeded double
					// count: the total layer) plus the producer's indirect atlas.
					const int worldLightmap = indirectViewLayer ? m_worldLightmapIndirectHandle
					                          : pbrWorldRuntime && !m_indirectPolicySeedDouble
					                              ? m_worldLightmapDirectHandle
					                              : m_worldLightmapHandle;
					// RuntimeIndirect: the baked producer's indirect atlas (the
					// LMAP indirect layer). BakedPlusDelta: the producer's change
					// volume and the volume's grid table.
					const int indirectSource = pbrWorldRuntime ? m_worldLightmapIndirectHandle
					                           : pbrWorldDelta ? m_probeDeltaHandle
					                                           : -1;
					// RFC 0011 G9: the shadow field (a white volume, unread, when
					// the map has none).
					const int shadowField =
					    pbrWorldLights && m_shadowFieldHandle >= 0 ? m_shadowFieldHandle : -1;
					const CGroupedDescriptors::Image frame[CGroupedDescriptors::kFrameBindings] = {
					    GroupedImage( m_pbrSplitSumHandle, -1, openTarget, false ),
					    GroupedImage( worldLightmap, -1, openTarget, false ),
					    GroupedImage( indirectSource, -1, openTarget, false ),
					    GroupedImage(
					        pbrWorldDelta ? m_probeGridHandle : -1, -1, openTarget, false ),
					    GroupedImage( shadowField, m_whiteVolumeHandle, openTarget, false ),
					    GroupedImage( m_reflectionProbeHandle, -1, openTarget, false ) };
					bool baseSrgb = false;
					const CGroupedDescriptors::Image
					    material[CGroupedDescriptors::kMaterialBindings] = {
					        GroupedImage( d.texHandle, -1, openTarget,
					            ( d.colorFlags & kColorSrgbReadBase ) != 0, &baseSrgb ),
					        GroupedImage( d.samplerHandles[1], -1, openTarget, false ),
					        GroupedImage( d.samplerHandles[2], -1, openTarget, false ),
					        glass ? GroupedImage( std::max( m_sceneColorHandle, 0 ), -1, -1, true )
					              : GroupedImage( d.samplerHandles[kPbrWorldEmissionSampler], -1,
					                    openTarget, false ),
					        GroupedImage(
					            !glass && environment >= 0 ? environment : m_whiteCubeHandle,
					            m_whiteCubeHandle, openTarget, false ),
					        GroupedImage(
					            glass ? m_sceneDepthHandle : -1, -1, openTarget, false ) };
					if ( baseSrgb )
						decodedFlags |= kColorSrgbReadBase;
					const VkPipelineLayout layout = glass           ? m_worldGlassPipelineLayout
					                                : pbrWorldDelta ? m_worldPbrDeltaLayout
					                                : ( pbrWorldLights || pbrWorldRuntime )
					                                    ? m_worldPbrExtendedLayout
					                                    : m_worldPbrPipelineLayout;
					const VkDescriptorSet sets[2] = {
					    m_groupedDescriptors.Acquire( CGroupedDescriptors::kFrameGroup, frame ),
					    m_groupedDescriptors.Acquire(
					        CGroupedDescriptors::kMaterialGroup, material ) };
					if ( sets[0] == VK_NULL_HANDLE || sets[1] == VK_NULL_HANDLE )
						continue;
					vkCmdBindDescriptorSets(
					    cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 2, sets, 0, nullptr );
					if ( pbrWorldLights )
						vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1,
						    &m_skinUbos[static_cast<size_t>( m_currentFrame ) % m_skinUbos.size()]
						        .set,
						    1, &m_directLightOffset );
				}
				else
				{
					const bool secondSampler =
					    ( d.colorFlags &
					        ( kFragmentCable | kFragmentMonitor | kFragmentRefract ) ) != 0;
					const int secondTexture = secondSampler ? d.samplerHandles[1]
					                          : d.worldMesh && m_worldLightmapHandle >= 0
					                              ? m_worldLightmapHandle
					                              : d.lightmapHandle;
					if ( d.worldMesh && m_worldLightmapHandle >= 0 && !secondSampler )
						decodedFlags |= kColorSrgbReadLightmap; // LMAP is scene-linear HDR.
					const int cubeTexture =
					    ( d.colorFlags & ( kFragmentLightmappedEnvmap | kFragmentRefract ) ) != 0
					        ? d.samplerHandles[2]
					        : m_whiteCubeHandle;
					const int normalMaskTexture =
					    ( d.colorFlags & kFragmentNormalAlphaEnvmapMask ) != 0 ? d.samplerHandles[4]
					                                                           : -1;
					const VkDescriptorSet sets[4] = { sampledSet( d.texHandle, kColorSrgbReadBase ),
					    sampledSet( secondTexture, kColorSrgbReadLightmap ),
					    sampledSet( cubeTexture, kColorSrgbReadSampler2 ),
					    sampledSet( normalMaskTexture, 0 ) };
					vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					    m_dynTexPipelineLayout, 0, 4, sets, 0, nullptr );
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
				// model_pbr.frag's flags take the map probe while an LMAP
				// atlas or the map's RPRB reflection probes are resident.
				int combos = c.combos;
				if ( pbrModel && !pbrModelEnv &&
				     ( m_worldLightmapHandle >= 0 || m_reflectionProbeHandle >= 0 ) )
					combos |= kPbrModelMapProbe;
				if ( paintBlob && !ManagedTextureIsCube( d.samplerHandles[7] ) )
					combos &= ~kPaintBlobEnvMap;
				pushData[29] = static_cast<float>( combos );
				pushData[30] = static_cast<float>( d.colorFlags & ~decodedFlags );
				pushData[31] = d.outputScale;
				pushData[32] = static_cast<float>( c.numLights );
				pushData[33] = pushData[34] = pushData[35] = 0.0f;
				if ( pbrModel && m_indirectViewMode != 0 )
				{
					// model_pbr.frag -DINDIRECT_VIEW: params2.y view, .z scale.
					pushData[33] = static_cast<float>( m_indirectViewMode == 3 ? 1 : m_indirectViewMode );
					pushData[34] = m_indirectViewScale;
				}
				// model_pbr.frag -DPROBE_VOLUME: params2.w the sampling mode.
				if ( pbrModelProbe )
					pushData[35] = static_cast<float>( m_probeSampling );
				appendClipPlanes( pushData + 36 );
				pushFloats = kSkinPushBytes / sizeof( float );
			}
			else if ( pbrDirect )
			{
				std::memcpy( pushData + 16, d.modulation, sizeof( d.modulation ) );
				std::memcpy( pushData + 20, d.texXform0, sizeof( d.texXform0 ) );
				std::memcpy( pushData + 24, d.texXform1, sizeof( d.texXform1 ) );
				std::memcpy( pushData + 28, d.pbrAngles, sizeof( d.pbrAngles ) );
				pushFloats = 32;
				if ( m_clipPlanesSupported )
				{
					appendClipPlanes( pushData + 32 );
					pushFloats = kTexturedPushBytes / sizeof( float );
				}
			}
			else if ( pbrWorld )
			{
				const PbrWorldScene &scene = d.pbrWorld;
				std::memcpy( pushData + 16, scene.eye, sizeof( scene.eye ) );
				std::memcpy( pushData + 20, scene.lightDirection, sizeof( scene.lightDirection ) );
				std::memcpy( pushData + 24, scene.lightRadiance, sizeof( scene.lightRadiance ) );
				std::memcpy( pushData + 28, scene.material, sizeof( scene.material ) );
				if ( !glass && IndirectViewShading() )
				{
					// world_pbr.frag -DINDIRECT_VIEW: lightDirection is ( view,
					// scale, indirect layer bound, 0 ).
					pushData[20] = static_cast<float>( m_indirectViewMode );
					pushData[21] = m_indirectViewScale;
					pushData[22] = m_worldLightmapIndirectHandle >= 0 ? 1.0f : 0.0f;
					pushData[23] = 0.0f;
				}
				if ( glass )
				{
					// shaders/world_pbr_glass.frag's glass, capture and material.
					const ManagedTexture &sceneColor =
					    m_managedTextures[static_cast<size_t>( m_sceneColorHandle )];
					pushData[20] = scene.glass[0];
					pushData[21] = scene.glass[1];
					pushData[22] = scene.glass[2];
					pushData[23] = sceneDepthValid && m_sceneDepthHandle >= 0 ? 1.0f : 0.0f;
					pushData[24] = 1.0f / static_cast<float>( sceneColor.width );
					pushData[25] = 1.0f / static_cast<float>( sceneColor.height );
					pushData[26] = 0.5f * viewport.width;
					pushData[27] = 0.5f * viewport.height;
					pushData[30] = static_cast<float>( sceneColor.mipLevels );
				}
				pushFloats = 32;
				if ( m_clipPlanesSupported )
				{
					appendClipPlanes( pushData + 32 );
					pushFloats = kTexturedPushBytes / sizeof( float );
				}
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
				pushData[28] = ( d.colorFlags & kFragmentMonitor )
				                   ? d.monitorContrast
				                   : d.alphaRef; // alphaParams.x
				// alphaParams.y: multiply by the lightmap, or another pixel stage
				// (kTexturedMode*); .z: kColorSrgb* flags.
				pushData[29] = d.texturedMode != kTexturedModeDefault
				                   ? static_cast<float>( d.texturedMode )
				                   : ( d.lightmapHandle >= 0 ? 1.0f : 0.0f );
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
			if ( d.worldMesh )
			{
				if ( !worldBuffersBound )
				{
					VkDeviceSize offset = 0;
					vkCmdBindVertexBuffers( cmd, 0, 1, &m_worldVertexBuffer.buffer, &offset );
					vkCmdBindIndexBuffer( cmd, m_worldIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32 );
					worldBuffersBound = true;
				}
				vkCmdDrawIndexed( cmd, d.indexCount, 1, d.firstIndex, 0, fogIndex[recordIndex] );
			}
			else if ( d.indexCount > 0 )
			{
				if ( worldBuffersBound )
				{
					VkDeviceSize offset = 0;
					vkCmdBindVertexBuffers( cmd, 0, 1, &vertexStream.buffer, &offset );
					if ( indicesOk )
						vkCmdBindIndexBuffer( cmd, indexStream.buffer, 0, VK_INDEX_TYPE_UINT32 );
					worldBuffersBound = false;
				}
				if ( indicesOk )
					vkCmdDrawIndexed( cmd, d.indexCount, 1, d.firstIndex,
					    static_cast<int32_t>( d.firstVertex ), fogIndex[recordIndex] );
			}
			else
			{
				if ( worldBuffersBound )
				{
					VkDeviceSize offset = 0;
					vkCmdBindVertexBuffers( cmd, 0, 1, &vertexStream.buffer, &offset );
					worldBuffersBound = false;
				}
				vkCmdDraw( cmd, d.vertexCount, 1, d.firstVertex, fogIndex[recordIndex] );
			}
		}
		ReplayFrameLabels( &labelCursor, m_dynDrawRecords.size() );
		endActiveQuery( false );
		// EndFrame closes the swapchain pass and transitions the image for
		// present or capture, so the frame must end inside it. Either view's pass
		// leaves the image in the same layout.
		if ( !passOpen || openTarget != -1 || ( !m_passMerging && openSrgb ) )
		{
			if ( passOpen )
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
	ScopedDebugLabel label( m_debugUtils, "capture back buffer" );
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
	ScopedDebugLabel label( m_debugUtils, "present: blit" );
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

	RecordPresentedCaptureAndRelease( cmd, imageIndex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, capture );
}

void CVulkanContext::RecordPresentedCaptureAndRelease( VkCommandBuffer cmd, uint32_t imageIndex,
    VkImageLayout layout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, bool capture )
{
	VkImageMemoryBarrier toPresent = {};
	toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	toPresent.image = m_presentImages[imageIndex];
	toPresent.oldLayout = layout;
	toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	toPresent.srcAccessMask = srcAccess;
	toPresent.dstAccessMask = 0;
	VkPipelineStageFlags presentSrcStage = srcStage;
	if ( capture )
	{
		// What the window shows: the swapchain image as presented.
		VkImageMemoryBarrier copyIn[2] = { toPresent, toPresent };
		copyIn[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		copyIn[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		copyIn[1].image = m_captureImage;
		copyIn[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		copyIn[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		copyIn[1].srcAccessMask = 0;
		copyIn[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(
		    cmd, srcStage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, copyIn );
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
		presentSrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	vkCmdPipelineBarrier( cmd, presentSrcStage, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
	    0, nullptr, 1, &toPresent );
}

//-----------------------------------------------------------------------------
// Multisampled back buffer (mat_antialias, render.sample-count.v1)
//-----------------------------------------------------------------------------
bool CVulkanContext::ApplySampleCount( std::string *outError )
{
	const int wanted =
	    render::ClampSampleCount( m_requestedSamples, m_adapterCaps.backBufferSampleMask );
	const bool targetsCurrent = m_msColor != VK_NULL_HANDLE &&
	                            m_msExtent.width == m_swapExtent.width &&
	                            m_msExtent.height == m_swapExtent.height;
	if ( wanted == m_activeSamples && ( wanted == 1 || targetsCurrent ) )
		return true;
	if ( m_frameOpen )
		return true; // applies at the next frame boundary
	vkDeviceWaitIdle( m_device );
	DestroyMsaaTargets();
	const int previous = m_activeSamples;
	m_activeSamples = 1;
	if ( wanted > 1 )
	{
		std::string error;
		if ( !CreateMsaaTargets( wanted, &error ) )
		{
			// Explicitly single-sampled: the back buffer always exists.
			DestroyMsaaTargets();
			Log( "%dx multisampling unavailable, single-sampled: %s\n", wanted, error.c_str() );
			return true;
		}
		m_activeSamples = wanted;
	}
	if ( m_activeSamples != previous )
		Log(
		    "back buffer multisampling %dx (requested %d)\n", m_activeSamples, m_requestedSamples );
	(void)outError;
	return true;
}

bool CVulkanContext::CreateMsaaTargets( int samples, std::string *outError )
{
	const VkSampleCountFlagBits sampleBits = static_cast<VkSampleCountFlagBits>( samples );
	if ( m_msPassSamples != samples )
	{
		DestroyMsaaPasses();
		const bool built =
		    CreateAttachmentPass( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED,
		        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, &m_msPassClear,
		        m_swapFormat, sampleBits, outError ) &&
		    CreateAttachmentPass( VK_ATTACHMENT_LOAD_OP_LOAD,
		        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &m_msPassLoad, m_swapFormat,
		        sampleBits, outError ) &&
		    ( !m_srgbAttachments ||
		        ( CreateAttachmentPass( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED,
		              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
		              &m_msPassClearSrgb, m_swapFormatSrgb, sampleBits, outError ) &&
		            CreateAttachmentPass( VK_ATTACHMENT_LOAD_OP_LOAD,
		                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &m_msPassLoadSrgb,
		                m_swapFormatSrgb, sampleBits, outError ) ) );
		if ( !built )
		{
			DestroyMsaaPasses();
			return false;
		}
		m_msPassSamples = samples;
	}

	const auto createImage = [&]( VkFormat format, VkImageUsageFlags usage, bool mutableSrgb,
	                             VkImage *outImage, VkDeviceMemory *outMemory ) -> bool
	{
		const VkFormat viewFormats[2] = { m_swapFormat, m_swapFormatSrgb };
		VkImageFormatListCreateInfo formatList = {};
		formatList.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
		formatList.viewFormatCount = 2;
		formatList.pViewFormats = viewFormats;
		VkImageCreateInfo img = {};
		img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		img.imageType = VK_IMAGE_TYPE_2D;
		img.format = format;
		img.extent = { m_swapExtent.width, m_swapExtent.height, 1 };
		img.mipLevels = 1;
		img.arrayLayers = 1;
		img.samples = sampleBits;
		img.tiling = VK_IMAGE_TILING_OPTIMAL;
		img.usage = usage;
		img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if ( mutableSrgb )
		{
			img.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
			img.pNext = &formatList;
		}
		if ( vkCreateImage( m_device, &img, nullptr, outImage ) != VK_SUCCESS )
			return false;
		VkMemoryRequirements req = {};
		vkGetImageMemoryRequirements( m_device, *outImage, &req );
		bool found = false;
		VkMemoryAllocateInfo ai = {};
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = req.size;
		ai.memoryTypeIndex =
		    FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &found );
		return found && vkAllocateMemory( m_device, &ai, nullptr, outMemory ) == VK_SUCCESS &&
		       vkBindImageMemory( m_device, *outImage, *outMemory, 0 ) == VK_SUCCESS;
	};
	const auto createView = [&]( VkImage image, VkFormat format, VkImageAspectFlags aspects,
	                            VkImageView *outView ) -> bool
	{
		VkImageViewCreateInfo iv = {};
		iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv.image = image;
		iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv.format = format;
		iv.subresourceRange = { aspects, 0, 1, 0, 1 };
		return vkCreateImageView( m_device, &iv, nullptr, outView ) == VK_SUCCESS;
	};
	const auto createFramebuffer = [&]( VkRenderPass pass, VkImageView color,
	                                   VkFramebuffer *outFramebuffer ) -> bool
	{
		VkImageView attachments[] = { color, m_msDepthView };
		VkFramebufferCreateInfo fb = {};
		fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb.renderPass = pass;
		fb.attachmentCount = 2;
		fb.pAttachments = attachments;
		fb.width = m_swapExtent.width;
		fb.height = m_swapExtent.height;
		fb.layers = 1;
		return vkCreateFramebuffer( m_device, &fb, nullptr, outFramebuffer ) == VK_SUCCESS;
	};
	const bool created =
	    createImage( m_swapFormat,
	        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	        m_srgbAttachments, &m_msColor, &m_msColorMemory ) &&
	    createImage( m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false, &m_msDepth,
	        &m_msDepthMemory ) &&
	    createView( m_msColor, m_swapFormat, VK_IMAGE_ASPECT_COLOR_BIT, &m_msColorView ) &&
	    ( !m_srgbAttachments || createView( m_msColor, m_swapFormatSrgb, VK_IMAGE_ASPECT_COLOR_BIT,
	                                &m_msColorViewSrgb ) ) &&
	    createView( m_msDepth, m_depthFormat, m_depthAspects, &m_msDepthView ) &&
	    createFramebuffer( m_msPassClear, m_msColorView, &m_msFramebuffer ) &&
	    ( !m_srgbAttachments ||
	        createFramebuffer( m_msPassClearSrgb, m_msColorViewSrgb, &m_msFramebufferSrgb ) );
	m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, m_msColor, "back buffer %dx MSAA", samples );
	m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, m_msDepth, "back buffer depth %dx MSAA", samples );
	if ( !created )
	{
		SetError( outError, "multisampled back buffer could not be created" );
		return false;
	}
	m_msExtent = m_swapExtent;
	return true;
}

void CVulkanContext::DestroyMsaaTargets()
{
	if ( m_device == VK_NULL_HANDLE )
		return;
	for ( VkFramebuffer *fb : { &m_msFramebuffer, &m_msFramebufferSrgb } )
	{
		if ( *fb != VK_NULL_HANDLE )
			vkDestroyFramebuffer( m_device, *fb, nullptr );
		*fb = VK_NULL_HANDLE;
	}
	for ( VkImageView *view : { &m_msColorView, &m_msColorViewSrgb, &m_msDepthView } )
	{
		if ( *view != VK_NULL_HANDLE )
			vkDestroyImageView( m_device, *view, nullptr );
		*view = VK_NULL_HANDLE;
	}
	for ( VkImage *image : { &m_msColor, &m_msDepth } )
	{
		if ( *image != VK_NULL_HANDLE )
			vkDestroyImage( m_device, *image, nullptr );
		*image = VK_NULL_HANDLE;
	}
	for ( VkDeviceMemory *memory : { &m_msColorMemory, &m_msDepthMemory } )
	{
		if ( *memory != VK_NULL_HANDLE )
			vkFreeMemory( m_device, *memory, nullptr );
		*memory = VK_NULL_HANDLE;
	}
	m_msExtent = { 0, 0 };
}

void CVulkanContext::DestroyMsaaPasses()
{
	if ( m_device == VK_NULL_HANDLE )
		return;
	for ( VkRenderPass *pass :
	    { &m_msPassClear, &m_msPassLoad, &m_msPassClearSrgb, &m_msPassLoadSrgb } )
	{
		if ( *pass != VK_NULL_HANDLE )
			vkDestroyRenderPass( m_device, *pass, nullptr );
		*pass = VK_NULL_HANDLE;
	}
	m_msPassSamples = 0;
}

void CVulkanContext::ResolveBackBuffer( VkCommandBuffer cmd, uint32_t imageIndex )
{
	ScopedDebugLabel label( m_debugUtils, "MSAA resolve" );
	VkImageMemoryBarrier in[2] = {};
	for ( VkImageMemoryBarrier &b : in )
	{
		b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	}
	// The multisampled color, written by the passes before this.
	in[0].image = m_msColor;
	in[0].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	in[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	in[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	in[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	// The back buffer is replaced whole; earlier reads of it (copies) finish first.
	in[1].image = m_swapImages[imageIndex];
	in[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	in[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	in[1].srcAccessMask = 0;
	in[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, in );

	VkImageResolve region = {};
	region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.extent = { m_swapExtent.width, m_swapExtent.height, 1 };
	vkCmdResolveImage( cmd, m_msColor, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	    m_swapImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
	++m_resolveCount;

	VkImageMemoryBarrier out[2] = { in[0], in[1] };
	out[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	out[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	out[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	out[0].dstAccessMask =
	    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	out[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	out[1].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	out[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	out[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
	                       VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT |
	        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	    0, 0, nullptr, 0, nullptr, 2, out );
}

//-----------------------------------------------------------------------------
// Monitor gamma at present time (render.gamma-ramp.v1)
//-----------------------------------------------------------------------------
void CVulkanContext::PublishGammaRamp( const render::GammaRamp16 &ramp )
{
	std::lock_guard<std::mutex> lock( m_gammaMutex );
	m_publishedRamp = ramp;
	m_publishedRampRevision.fetch_add( 1, std::memory_order_release );
}

void CVulkanContext::ApplyPublishedGammaRamp()
{
	const uint64_t revision = m_publishedRampRevision.load( std::memory_order_acquire );
	if ( revision == m_appliedRampRevision )
		return;
	{
		std::lock_guard<std::mutex> lock( m_gammaMutex );
		m_activeRamp = m_publishedRamp;
		m_appliedRampRevision = m_publishedRampRevision.load( std::memory_order_relaxed );
	}
	const bool active = !render::IsIdentityAt8Bit( m_activeRamp );
	if ( active != m_gammaActive )
		Log( "monitor gamma %s\n", active ? "applied at present" : "identity (plain present)" );
	m_gammaActive = active;
}

bool CVulkanContext::EnsurePresentGamma( std::string *outError )
{
	if ( m_gammaPipeline != VK_NULL_HANDLE && m_gammaFormat == m_swapFormat )
		return true;
	DestroyPresentGamma();

	VkFormatProperties features = {};
	vkGetPhysicalDeviceFormatProperties( m_physicalDevice, m_swapFormat, &features );
	if ( !( features.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT ) ||
	     !( features.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ) )
	{
		SetError( outError, "the swapchain format cannot be sampled or rendered for gamma" );
		return false;
	}

	// The swapchain image's previous contents are not needed; the pass leaves it
	// as a color attachment for the explicit capture/present transition. The
	// external dependency chains to the acquire semaphore's wait stage.
	VkAttachmentDescription color = {};
	color.format = m_swapFormat;
	color.samples = VK_SAMPLE_COUNT_1_BIT;
	color.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	VkRenderPassCreateInfo rp = {};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp.attachmentCount = 1;
	rp.pAttachments = &color;
	rp.subpassCount = 1;
	rp.pSubpasses = &subpass;
	rp.dependencyCount = 1;
	rp.pDependencies = &dependency;
	if ( vkCreateRenderPass( m_device, &rp, nullptr, &m_gammaRenderPass ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateRenderPass (gamma present) failed" );
		DestroyPresentGamma();
		return false;
	}

	VkDescriptorSetLayoutBinding bindings[2] = {};
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo dl = {};
	dl.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dl.bindingCount = 2;
	dl.pBindings = bindings;
	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.setLayoutCount = 1;
	pl.pSetLayouts = &m_gammaSetLayout;
	if ( vkCreateDescriptorSetLayout( m_device, &dl, nullptr, &m_gammaSetLayout ) != VK_SUCCESS ||
	     vkCreatePipelineLayout( m_device, &pl, nullptr, &m_gammaPipelineLayout ) != VK_SUCCESS )
	{
		SetError( outError, "gamma present layouts could not be created" );
		DestroyPresentGamma();
		return false;
	}

	VkSamplerCreateInfo sc = {};
	sc.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sc.magFilter = sc.minFilter = VK_FILTER_NEAREST;
	sc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sc.addressModeU = sc.addressModeV = sc.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	if ( vkCreateSampler( m_device, &sc, nullptr, &m_gammaSamplerNearest ) != VK_SUCCESS )
	{
		SetError( outError, "gamma present sampler could not be created" );
		DestroyPresentGamma();
		return false;
	}
	// Scaling filters as the blit would (m_presentFilter).
	sc.magFilter = sc.minFilter = m_presentFilter;
	if ( vkCreateSampler( m_device, &sc, nullptr, &m_gammaSamplerLinear ) != VK_SUCCESS )
	{
		SetError( outError, "gamma present sampler could not be created" );
		DestroyPresentGamma();
		return false;
	}

	VkDescriptorPoolSize sizes[2] = {};
	sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sizes[0].descriptorCount = kMaxFramesInFlight;
	sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sizes[1].descriptorCount = kMaxFramesInFlight;
	VkDescriptorPoolCreateInfo dp = {};
	dp.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dp.maxSets = kMaxFramesInFlight;
	dp.poolSizeCount = 2;
	dp.pPoolSizes = sizes;
	VkDescriptorSetLayout layouts[kMaxFramesInFlight];
	for ( VkDescriptorSetLayout &layout : layouts )
		layout = m_gammaSetLayout;
	VkDescriptorSetAllocateInfo da = {};
	da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	da.descriptorSetCount = kMaxFramesInFlight;
	da.pSetLayouts = layouts;
	if ( vkCreateDescriptorPool( m_device, &dp, nullptr, &m_gammaDescriptorPool ) != VK_SUCCESS ||
	     ( da.descriptorPool = m_gammaDescriptorPool,
	         vkAllocateDescriptorSets( m_device, &da, m_gammaSets ) != VK_SUCCESS ) )
	{
		SetError( outError, "gamma present descriptors could not be allocated" );
		DestroyPresentGamma();
		return false;
	}

	// One ramp buffer per frame slot, written only after that slot's fence.
	const VkDeviceSize rampBytes = sizeof( float ) * 256;
	for ( int slot = 0; slot < kMaxFramesInFlight; ++slot )
	{
		void *mapped = nullptr;
		if ( !CreateBuffer( rampBytes, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		         &m_gammaRampBuffers[slot], &m_gammaRampMemories[slot], outError ) ||
		     vkMapMemory( m_device, m_gammaRampMemories[slot], 0, rampBytes, 0, &mapped ) !=
		         VK_SUCCESS )
		{
			SetError( outError, "gamma ramp buffer could not be created" );
			DestroyPresentGamma();
			return false;
		}
		m_gammaRampMapped[slot] = static_cast<float *>( mapped );
	}

	VkShaderModule vert = VK_NULL_HANDLE, frag = VK_NULL_HANDLE;
	if ( !CreateShaderModule(
	         g_presentGammaVertSpv, sizeof( g_presentGammaVertSpv ), &vert, outError ) ||
	     !CreateShaderModule(
	         g_presentGammaFragSpv, sizeof( g_presentGammaFragSpv ), &frag, outError ) )
	{
		if ( vert != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, vert, nullptr );
		DestroyPresentGamma();
		return false;
	}
	VkPipelineShaderStageCreateInfo stages[2] = {};
	for ( VkPipelineShaderStageCreateInfo &stage : stages )
	{
		stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.pName = "main";
	}
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	VkPipelineVertexInputStateCreateInfo vin = {};
	vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
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
	VkPipelineColorBlendAttachmentState blendAttachment = {};
	blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo blend = {};
	blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend.attachmentCount = 1;
	blend.pAttachments = &blendAttachment;
	const VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynamicStates;
	VkGraphicsPipelineCreateInfo gp = {};
	gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp.stageCount = 2;
	gp.pStages = stages;
	ConsumedVertexInput consumedInput;
	gp.pVertexInputState = FilterVertexInput( &vin, stages[0].module, &consumedInput );
	gp.pInputAssemblyState = &ia;
	gp.pViewportState = &vp;
	gp.pRasterizationState = &rs;
	gp.pMultisampleState = &ms;
	gp.pColorBlendState = &blend;
	gp.pDynamicState = &dyn;
	gp.layout = m_gammaPipelineLayout;
	gp.renderPass = m_gammaRenderPass;
	const VkResult r =
	    vkCreateGraphicsPipelines( m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_gammaPipeline );
	m_debugUtils.Name( VK_OBJECT_TYPE_PIPELINE, m_gammaPipeline, "present gamma" );
	vkDestroyShaderModule( m_device, vert, nullptr );
	vkDestroyShaderModule( m_device, frag, nullptr );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "gamma present pipeline failed: " ) + ResultString( r ) );
		DestroyPresentGamma();
		return false;
	}
	m_gammaFormat = m_swapFormat;
	return true;
}

bool CVulkanContext::EnsurePresentGammaTargets( std::string *outError )
{
	if ( m_presentFramebuffers.size() == m_presentImages.size() )
		return true;
	DestroyPresentGammaTargets();
	m_presentImageViews.assign( m_presentImages.size(), VK_NULL_HANDLE );
	m_presentFramebuffers.assign( m_presentImages.size(), VK_NULL_HANDLE );
	for ( size_t i = 0; i < m_presentImages.size(); ++i )
	{
		VkImageViewCreateInfo iv = {};
		iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv.image = m_presentImages[i];
		iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv.format = m_swapFormat;
		iv.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		VkFramebufferCreateInfo fb = {};
		fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb.renderPass = m_gammaRenderPass;
		fb.attachmentCount = 1;
		fb.width = m_presentExtent.width;
		fb.height = m_presentExtent.height;
		fb.layers = 1;
		if ( vkCreateImageView( m_device, &iv, nullptr, &m_presentImageViews[i] ) != VK_SUCCESS ||
		     ( fb.pAttachments = &m_presentImageViews[i],
		         vkCreateFramebuffer( m_device, &fb, nullptr, &m_presentFramebuffers[i] ) !=
		             VK_SUCCESS ) )
		{
			SetError( outError, "gamma present framebuffers could not be created" );
			DestroyPresentGammaTargets();
			return false;
		}
	}
	return true;
}

void CVulkanContext::DestroyPresentGammaTargets()
{
	for ( VkFramebuffer fb : m_presentFramebuffers )
		if ( fb != VK_NULL_HANDLE )
			vkDestroyFramebuffer( m_device, fb, nullptr );
	m_presentFramebuffers.clear();
	for ( VkImageView view : m_presentImageViews )
		if ( view != VK_NULL_HANDLE )
			vkDestroyImageView( m_device, view, nullptr );
	m_presentImageViews.clear();
}

void CVulkanContext::DestroyPresentGamma()
{
	DestroyPresentGammaTargets();
	if ( m_device == VK_NULL_HANDLE )
		return;
	if ( m_gammaPipeline != VK_NULL_HANDLE )
		vkDestroyPipeline( m_device, m_gammaPipeline, nullptr );
	if ( m_gammaPipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_gammaPipelineLayout, nullptr );
	if ( m_gammaSetLayout != VK_NULL_HANDLE )
		vkDestroyDescriptorSetLayout( m_device, m_gammaSetLayout, nullptr );
	if ( m_gammaDescriptorPool != VK_NULL_HANDLE )
		vkDestroyDescriptorPool( m_device, m_gammaDescriptorPool, nullptr );
	if ( m_gammaSamplerNearest != VK_NULL_HANDLE )
		vkDestroySampler( m_device, m_gammaSamplerNearest, nullptr );
	if ( m_gammaSamplerLinear != VK_NULL_HANDLE )
		vkDestroySampler( m_device, m_gammaSamplerLinear, nullptr );
	if ( m_gammaRenderPass != VK_NULL_HANDLE )
		vkDestroyRenderPass( m_device, m_gammaRenderPass, nullptr );
	for ( int slot = 0; slot < kMaxFramesInFlight; ++slot )
	{
		if ( m_gammaRampBuffers[slot] != VK_NULL_HANDLE )
			vkDestroyBuffer( m_device, m_gammaRampBuffers[slot], nullptr );
		if ( m_gammaRampMemories[slot] != VK_NULL_HANDLE )
			vkFreeMemory( m_device, m_gammaRampMemories[slot], nullptr );
		m_gammaRampBuffers[slot] = VK_NULL_HANDLE;
		m_gammaRampMemories[slot] = VK_NULL_HANDLE;
		m_gammaRampMapped[slot] = nullptr;
		m_gammaSets[slot] = VK_NULL_HANDLE;
	}
	m_gammaPipeline = VK_NULL_HANDLE;
	m_gammaPipelineLayout = VK_NULL_HANDLE;
	m_gammaSetLayout = VK_NULL_HANDLE;
	m_gammaDescriptorPool = VK_NULL_HANDLE;
	m_gammaSamplerNearest = m_gammaSamplerLinear = VK_NULL_HANDLE;
	m_gammaRenderPass = VK_NULL_HANDLE;
	m_gammaFormat = VK_FORMAT_UNDEFINED;
}

bool CVulkanContext::RecordPresentGamma(
    VkCommandBuffer cmd, uint32_t imageIndex, VkImageLayout backBufferLayout, bool capture )
{
	ScopedDebugLabel label( m_debugUtils, "present: gamma" );
	if ( m_gammaUnavailable )
		return false;
	std::string error;
	if ( !EnsurePresentGamma( &error ) || !EnsurePresentGammaTargets( &error ) )
	{
		// Explicitly degraded, once: presents fall back to the unramped blit.
		Log( "monitor gamma unavailable, presenting without it: %s\n", error.c_str() );
		m_gammaUnavailable = true;
		return false;
	}

	// This slot's fence was waited in BeginFrame, so its ramp buffer is free.
	float *entries = m_gammaRampMapped[m_currentFrame];
	for ( int i = 0; i < 256; ++i )
		entries[i] = float( m_activeRamp[i] ) / 65535.0f;
	const bool sameSize = m_swapExtent.width == m_presentExtent.width &&
	                      m_swapExtent.height == m_presentExtent.height;
	VkDescriptorImageInfo image = {};
	image.sampler = sameSize ? m_gammaSamplerNearest : m_gammaSamplerLinear;
	image.imageView = m_swapImageViews[imageIndex];
	image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkDescriptorBufferInfo ramp = { m_gammaRampBuffers[m_currentFrame], 0, sizeof( float ) * 256 };
	VkWriteDescriptorSet writes[2] = {};
	for ( VkWriteDescriptorSet &write : writes )
	{
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_gammaSets[m_currentFrame];
		write.descriptorCount = 1;
	}
	writes[0].dstBinding = 0;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[0].pImageInfo = &image;
	writes[1].dstBinding = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[1].pBufferInfo = &ramp;
	vkUpdateDescriptorSets( m_device, 2, writes, 0, nullptr );

	// The back buffer: its color writes (or the capture's read) done, sampled.
	// The next frame's first pass discards it (initial layout UNDEFINED).
	VkImageMemoryBarrier toRead = {};
	toRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toRead.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	toRead.image = m_swapImages[imageIndex];
	toRead.oldLayout = backBufferLayout;
	toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	toRead.srcAccessMask = backBufferLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	                           ? VK_ACCESS_TRANSFER_READ_BIT
	                           : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier( cmd,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toRead );

	VkRenderPassBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin.renderPass = m_gammaRenderPass;
	begin.framebuffer = m_presentFramebuffers[imageIndex];
	begin.renderArea.extent = m_presentExtent;
	vkCmdBeginRenderPass( cmd, &begin, VK_SUBPASS_CONTENTS_INLINE );
	vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_gammaPipeline );
	vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_gammaPipelineLayout, 0, 1,
	    &m_gammaSets[m_currentFrame], 0, nullptr );
	const VkViewport viewport = {
	    0.0f, 0.0f, float( m_presentExtent.width ), float( m_presentExtent.height ), 0.0f, 1.0f };
	const VkRect2D scissor = { { 0, 0 }, m_presentExtent };
	vkCmdSetViewport( cmd, 0, 1, &viewport );
	vkCmdSetScissor( cmd, 0, 1, &scissor );
	vkCmdDraw( cmd, 3, 1, 0, 0 );
	vkCmdEndRenderPass( cmd );

	++m_presentCount;
	++m_gammaPresentCount;
	if ( !sameSize )
		++m_scaledPresentCount;
	RecordPresentedCaptureAndRelease( cmd, imageIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	    capture );
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
	// Everything after this reads the single-sampled back buffer.
	if ( m_activeSamples > 1 )
		ResolveBackBuffer( cmd, imageIndex );

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
	const VkImageLayout backBufferLayout = captureBackBuffer
	                                           ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	                                           : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	ApplyPublishedGammaRamp();
	if ( !m_gammaActive ||
	     !RecordPresentGamma( cmd, imageIndex, backBufferLayout, capturePresented ) )
		RecordPresentBlit( cmd, imageIndex, backBufferLayout, capturePresented );
	m_captureRequested = m_capturePresented = false;

	if ( m_computeFlush )
		m_computeFlush( cmd, m_submitSerial + 1 );
	if ( m_timestampPool != VK_NULL_HANDLE )
		vkCmdWriteTimestamp( cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_timestampPool,
		    m_currentFrame * 2 + 1 );
	m_debugUtils.EndFrameCommands();
	VkResult r = vkEndCommandBuffer( cmd );
	m_frameCost.Add( kCostRecord, FrameClockMicros() - m_recordBeginUs );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkEndCommandBuffer failed: " ) + ResultString( r ) );
		return false;
	}

	// Only the present (a blit, or the gamma pass's color writes) touches the
	// acquired swapchain image; rendering into the back buffer need not wait
	// for the acquire.
	VkPipelineStageFlags waitStage =
	    VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &m_imageAvailable[m_currentFrame];
	submit.pWaitDstStageMask = &waitStage;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &m_renderFinished[imageIndex];

	vkResetFences( m_device, 1, &m_inFlight[m_currentFrame] );
	{
		CFrameCostScope submitCost( m_frameCost, kCostSubmit );
		r = vkQueueSubmit( m_graphicsQueue, 1, &submit, m_inFlight[m_currentFrame] );
	}
	if ( r == VK_SUCCESS )
	{
		m_slotSerial[m_currentFrame] = ++m_submitSerial;
		m_slotStatsFrame[m_currentFrame] = ++m_statsFrame;
	}
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
	present.pWaitSemaphores = &m_renderFinished[imageIndex];
	present.swapchainCount = 1;
	present.pSwapchains = &m_swapchain;
	present.pImageIndices = &imageIndex;

	{
		CFrameCostScope presentCost( m_frameCost, kCostPresent );
		r = vkQueuePresentKHR( m_presentQueue, &present );
	}
	m_frameOpen = false;
	m_currentFrame = ( m_currentFrame + 1 ) % m_framesInFlight;
	WriteFrameStats( FrameClockMicros() );

	if ( r == VK_ERROR_SURFACE_LOST_KHR )
	{
		m_surfaceLost = true; // Replaced before the next frame. Not fatal.
		return true;
	}
	if ( r == VK_ERROR_OUT_OF_DATE_KHR || ( r == VK_SUBOPTIMAL_KHR && SurfaceChangedSinceSwapchain() ) )
	{
		// Present surface changed; rebuild for the next frame. Not fatal.
		if ( !RecreateSwapchain( outError ) )
			return false;
		return true;
	}
	if ( r == VK_SUBOPTIMAL_KHR )
		return true; // Presented; the compositor adapts (e.g. rotates) the image.
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkQueuePresentKHR failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

static const char kPipelineCacheFile[] = "vulkan_pipelines.cache";
static const char kPipelineKeysFile[] = "vulkan_pipelines.keys";
// The first line of the keys file. Bump the version when RasterStateKey's
// encoding or a family's meaning changes, so old keys are ignored, not misread.
static const char kPipelineKeysHeader[] = "vulkan-pipeline-keys/v1";

static bool ReadWholeFile( const std::string &path, std::vector<char> *out )
{
	FILE *file = std::fopen( path.c_str(), "rb" );
	if ( !file )
		return false;
	out->clear();
	char buffer[65536];
	size_t got;
	while ( ( got = std::fread( buffer, 1, sizeof( buffer ), file ) ) > 0 )
		out->insert( out->end(), buffer, buffer + got );
	const bool ok = !std::ferror( file );
	std::fclose( file );
	return ok;
}

// Replaces `path` only once the new contents are complete on disk, so an
// interrupted save leaves the previous store rather than a truncated one.
static bool WriteFileAtomically( const std::string &path, const void *data, size_t size )
{
	const std::string temporary = path + ".tmp";
	FILE *file = std::fopen( temporary.c_str(), "wb" );
	if ( !file )
		return false;
	const bool written = std::fwrite( data, 1, size, file ) == size;
	const bool closed = std::fclose( file ) == 0;
	if ( !written || !closed || std::rename( temporary.c_str(), path.c_str() ) != 0 )
	{
		std::remove( temporary.c_str() );
		return false;
	}
	return true;
}

bool CVulkanContext::OpenPipelineStore( const std::string &directory, std::string *outError )
{
	if ( !IsValid() || directory.empty() )
	{
		SetError( outError, "pipeline store needs a device and a directory" );
		return false;
	}
	if ( m_pipelineCache != VK_NULL_HANDLE )
	{
		SetError( outError, "pipeline store is already open" );
		return false;
	}
	std::vector<char> data;
	const bool haveData = ReadWholeFile( directory + "/" + kPipelineCacheFile, &data );
	VkPipelineCacheCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	info.initialDataSize = haveData ? data.size() : 0;
	info.pInitialData = haveData && !data.empty() ? data.data() : nullptr;
	VkResult r = vkCreatePipelineCache( m_device, &info, nullptr, &m_pipelineCache );
	if ( r != VK_SUCCESS && info.initialDataSize )
	{
		// Data the driver will not take starts the cache empty.
		info.initialDataSize = 0;
		info.pInitialData = nullptr;
		r = vkCreatePipelineCache( m_device, &info, nullptr, &m_pipelineCache );
	}
	if ( r != VK_SUCCESS )
	{
		m_pipelineCache = VK_NULL_HANDLE;
		SetError( outError, std::string( "vkCreatePipelineCache failed: " ) + ResultString( r ) );
		return false;
	}
	m_pipelineStoreDirectory = directory;
	Log( "pipeline store %s: %zu bytes of cache data\n", directory.c_str(),
	    haveData ? data.size() : static_cast<size_t>( 0 ) );
	return true;
}

int CVulkanContext::PrewarmPipelines()
{
	if ( m_pipelineStoreDirectory.empty() )
		return 0;
	std::vector<char> text;
	if ( !ReadWholeFile( m_pipelineStoreDirectory + "/" + kPipelineKeysFile, &text ) )
		return 0;
	text.push_back( '\0' );
	const char *line = text.data();
	if ( std::strncmp( line, kPipelineKeysHeader, sizeof( kPipelineKeysHeader ) - 1 ) != 0 )
	{
		Log( "pipeline keys ignored: not %s\n", kPipelineKeysHeader );
		return 0;
	}
	int built = 0;
	for ( line = std::strchr( line, '\n' ); line; line = std::strchr( line, '\n' ) )
	{
		++line;
		int family = -1;
		unsigned long long key = 0;
		if ( std::sscanf( line, "%d %llx", &family, &key ) != 2 )
			continue;
		const DynRasterState state = RasterStateFromKey( key );
		const bool srgb = ( key >> 32 ) & 1u;
		// Multisampled variants build only for the active sample count.
		const int samples = PipelineKeySamples( key );
		if ( samples != 1 && samples != m_activeSamples )
			continue;
		// A key from another build can name a variant this device cannot make;
		// it fails here, once, and is not written back.
		VkPipeline pipeline = VK_NULL_HANDLE;
		if ( family == kPipelineTextured )
			pipeline = TexturedPipeline( state, srgb, samples );
		else if ( family == kPipelinePortal )
			pipeline = PortalPipeline( state, srgb, samples );
		else if ( family == kPipelineSkin )
			pipeline = SkinPipeline( state, srgb, samples );
		else if ( family == kPipelineSolidEnergy )
			pipeline = SolidEnergyPipeline( state, srgb, samples );
		else if ( family == kPipelinePaintBlob )
			pipeline = PaintBlobPipeline( state, srgb, samples );
		else if ( family == kPipelinePbrModel || family == kPipelinePbrModelEnv )
			pipeline = PbrModelPipeline( state, family == kPipelinePbrModelEnv, srgb, samples );
		else if ( family == kPipelinePbrDirect )
			pipeline = PbrDirectPipeline( state, srgb, samples );
		else if ( family == kPipelineLightmapped )
			pipeline = LightmappedPipeline( state, srgb, samples );
		else if ( family == kPipelinePost )
			pipeline = PostPipeline( state, srgb, samples );
		if ( pipeline != VK_NULL_HANDLE )
			++built;
	}
	return built;
}

bool CVulkanContext::SavePipelineStore( std::string *outError )
{
	if ( m_pipelineCache == VK_NULL_HANDLE || m_pipelineStoreDirectory.empty() )
	{
		SetError( outError, "pipeline store is not open" );
		return false;
	}
	size_t size = 0;
	std::vector<char> data;
	if ( vkGetPipelineCacheData( m_device, m_pipelineCache, &size, nullptr ) == VK_SUCCESS )
	{
		data.resize( size );
		if ( size &&
		     vkGetPipelineCacheData( m_device, m_pipelineCache, &size, data.data() ) != VK_SUCCESS )
			size = 0;
		data.resize( size );
	}
	std::vector<std::pair<int, uint64_t>> variants = m_pipelineVariants;
	std::sort( variants.begin(), variants.end() );
	variants.erase( std::unique( variants.begin(), variants.end() ), variants.end() );
	std::string keys = std::string( kPipelineKeysHeader ) + "\n";
	for ( const std::pair<int, uint64_t> &variant : variants )
	{
		char line[64];
		std::snprintf( line, sizeof( line ), "%d %llx\n", variant.first,
		    static_cast<unsigned long long>( variant.second ) );
		keys += line;
	}
	const std::string base = m_pipelineStoreDirectory + "/";
	if ( ( !data.empty() &&
	         !WriteFileAtomically( base + kPipelineCacheFile, data.data(), data.size() ) ) ||
	     !WriteFileAtomically( base + kPipelineKeysFile, keys.data(), keys.size() ) )
	{
		SetError( outError, "cannot write the pipeline store in " + m_pipelineStoreDirectory );
		return false;
	}
	return true;
}

void CVulkanContext::CreateTimestampPool()
{
	// Optional: without timestamps the stats carry CPU costs only.
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( m_physicalDevice, &familyCount, nullptr );
	std::vector<VkQueueFamilyProperties> families( familyCount );
	if ( familyCount )
		vkGetPhysicalDeviceQueueFamilyProperties( m_physicalDevice, &familyCount, families.data() );
	if ( m_graphicsQueueFamily >= familyCount ||
	     families[m_graphicsQueueFamily].timestampValidBits == 0 ||
	     properties.limits.timestampPeriod <= 0.0f )
		return;
	VkQueryPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	info.queryType = VK_QUERY_TYPE_TIMESTAMP;
	info.queryCount = kMaxFramesInFlight * 2;
	if ( vkCreateQueryPool( m_device, &info, nullptr, &m_timestampPool ) != VK_SUCCESS )
	{
		m_timestampPool = VK_NULL_HANDLE;
		return;
	}
	const uint32_t bits = families[m_graphicsQueueFamily].timestampValidBits;
	m_timestampMask = bits >= 64 ? ~0ull : ( ( 1ull << bits ) - 1 );
	m_timestampPeriodNs = properties.limits.timestampPeriod;
}

void CVulkanContext::ReadSlotGpuTime( uint32_t slot )
{
	// Called once the slot's fence has signalled: its timestamps are final.
	if ( m_timestampPool == VK_NULL_HANDLE || m_slotStatsFrame[slot] == 0 )
		return;
	uint64_t stamps[2] = { 0, 0 };
	if ( vkGetQueryPoolResults( m_device, m_timestampPool, slot * 2, 2, sizeof( stamps ), stamps,
	         sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT ) == VK_SUCCESS )
	{
		const uint64_t ticks = ( stamps[1] - stamps[0] ) & m_timestampMask;
		m_gpuResultFrame = m_slotStatsFrame[slot];
		m_gpuResultUs = static_cast<uint64_t>( double( ticks ) * m_timestampPeriodNs / 1000.0 );
	}
	m_slotStatsFrame[slot] = 0;
}

bool CVulkanContext::OpenFrameStats( const char *path, std::string *outError )
{
	if ( !path || !*path )
	{
		SetError( outError, "frame stats need an output path" );
		return false;
	}
	FILE *file = std::fopen( path, "w" );
	if ( !file )
	{
		SetError( outError, std::string( "cannot create frame stats file " ) + path );
		return false;
	}
	CloseFrameStats();
	m_frameStatsFile = file;
	std::fprintf( m_frameStatsFile,
	    "{\"schema\":\"vulkan-frame-stats/v1\",\"device\":\"%s\",\"frames_in_flight\":%u,"
	    "\"gpu_timestamps\":%s,\"present_mode\":%d}\n",
	    m_deviceName.c_str(), m_framesInFlight, m_timestampPool ? "true" : "false",
	    static_cast<int>( m_presentMode ) );
	m_statsPresentMode = static_cast<int>( m_presentMode );
	return true;
}

void CVulkanContext::CloseFrameStats()
{
	if ( !m_frameStatsFile )
		return;
	std::fclose( m_frameStatsFile );
	m_frameStatsFile = nullptr;
}

void CVulkanContext::MarkFrame( const char *label )
{
	if ( !label || m_frameMarks.size() > 256 )
		return;
	if ( !m_frameMarks.empty() )
		m_frameMarks += ',';
	for ( const char *c = label; *c && c - label < 64; ++c )
	{
		const bool safe = ( *c >= 'a' && *c <= 'z' ) || ( *c >= 'A' && *c <= 'Z' ) ||
		                  ( *c >= '0' && *c <= '9' ) || *c == '_' || *c == '.' || *c == '-';
		m_frameMarks += safe ? *c : '_';
	}
}

void CVulkanContext::WriteFrameStats( uint64_t endUs )
{
	if ( m_frameStatsFile )
	{
		// f: stats frame id (submission order); t: frame start (steady clock,
		// microseconds); interval: since the previous frame's start; engine: the
		// caller's time between the previous frame's end and this start; backend:
		// BeginFrame through present; cpu: the presenting thread's CPU time over
		// the same interval.
		std::fprintf( m_frameStatsFile,
		    "{\"f\":%llu,\"t\":%llu,\"interval\":%llu,\"cpu\":%llu,\"engine\":%llu,"
		    "\"backend\":%llu,\"records\":%zu,\"vertex_bytes\":%zu,\"index_bytes\":%zu,"
		    "\"upload_bytes\":%llu",
		    static_cast<unsigned long long>( m_statsFrame ),
		    static_cast<unsigned long long>( m_frameBeginUs ),
		    static_cast<unsigned long long>(
		        m_prevFrameBeginUs ? m_frameBeginUs - m_prevFrameBeginUs : 0 ),
		    static_cast<unsigned long long>(
		        m_prevFrameBeginCpuUs ? m_frameBeginCpuUs - m_prevFrameBeginCpuUs : 0 ),
		    static_cast<unsigned long long>(
		        m_prevFrameEndUs ? m_frameBeginUs - m_prevFrameEndUs : 0 ),
		    static_cast<unsigned long long>( endUs - m_frameBeginUs ), m_dynDrawRecords.size(),
		    m_dynQueued.size() * sizeof( float ), m_dynIndices.size() * sizeof( uint32_t ),
		    static_cast<unsigned long long>( m_frameCost.uploadBytes ) );
		std::fputs( ",\"cost\":{", m_frameStatsFile );
		bool first = true;
		for ( int kind = 0; kind < kFrameCostKinds; ++kind )
		{
			if ( !m_frameCost.count[kind] )
				continue;
			std::fprintf( m_frameStatsFile, "%s\"%s\":[%u,%llu]", first ? "" : ",",
			    FrameCostName( kind ), m_frameCost.count[kind],
			    static_cast<unsigned long long>( m_frameCost.us[kind] ) );
			first = false;
		}
		std::fputc( '}', m_frameStatsFile );
		// Emit's conversion by draw size (vulkan_frame_stats.h buckets): per
		// bucket [draws, unique vertices, microseconds].
		const EmitConvertStats &convert = m_frameCost.convert;
		if ( m_frameCost.count[kCostEmitConvert] )
		{
			std::fprintf( m_frameStatsFile,
			    ",\"convert\":{\"skinned\":%llu,\"pooled\":%u,\"buckets\":[",
			    static_cast<unsigned long long>( convert.skinnedVertices ), convert.pooledDraws );
			for ( int bucket = 0; bucket < kEmitConvertBuckets; ++bucket )
				std::fprintf( m_frameStatsFile, "%s[%u,%llu,%llu]", bucket ? "," : "",
				    convert.draws[bucket],
				    static_cast<unsigned long long>( convert.vertices[bucket] ),
				    static_cast<unsigned long long>( convert.ns[bucket] / 1000u ) );
			std::fputs( "]}", m_frameStatsFile );
		}
		if ( m_gpuResultFrame )
			std::fprintf( m_frameStatsFile, ",\"gpu\":[%llu,%llu]",
			    static_cast<unsigned long long>( m_gpuResultFrame ),
			    static_cast<unsigned long long>( m_gpuResultUs ) );
		// The present mode, whenever it differs from the last one recorded.
		if ( static_cast<int>( m_presentMode ) != m_statsPresentMode )
		{
			m_statsPresentMode = static_cast<int>( m_presentMode );
			std::fprintf( m_frameStatsFile, ",\"present_mode\":%d", m_statsPresentMode );
		}
		if ( !m_frameMarks.empty() )
			std::fprintf( m_frameStatsFile, ",\"mark\":\"%s\"", m_frameMarks.c_str() );
		std::fputs( "}\n", m_frameStatsFile );
		// Bounded loss if the process is killed rather than shut down.
		if ( m_statsFrame % 60 == 0 )
			std::fflush( m_frameStatsFile );
	}
	m_gpuResultFrame = 0;
	m_frameMarks.clear();
	m_frameCost.Reset();
	m_prevFrameBeginUs = m_frameBeginUs;
	m_prevFrameBeginCpuUs = m_frameBeginCpuUs;
	m_prevFrameEndUs = endUs;
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
	DestroyPresentGammaTargets();
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
	m_adapterCaps = VulkanAdapterCaps();
	CloseFrameStats();
	if ( m_device != VK_NULL_HANDLE && !m_pipelineStoreDirectory.empty() )
	{
		std::string storeError;
		if ( !SavePipelineStore( &storeError ) )
			Log( "pipeline store not saved: %s\n", storeError.c_str() );
	}
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
		DestroyStreamBuffer( m_worldIndexBuffer );
		DestroyStreamBuffer( m_worldVertexBuffer );
		m_worldVertexCount = 0;
		m_worldIndexCount = 0;
		++m_worldMeshRevision;
		DestroyDemoTriangle();
		DestroyTexturedQuad();
		DestroyIndexedUbo();
		DestroyDemoDepth();
		DestroyDynamicMesh();
		DestroySwapchainObjects();
		DestroyMsaaTargets();
		DestroyMsaaPasses();
		DestroyPresentGamma();
		if ( m_queryPool != VK_NULL_HANDLE )
		{
			vkDestroyQueryPool( m_device, m_queryPool, nullptr );
			m_queryPool = VK_NULL_HANDLE;
		}
		if ( m_timestampPool != VK_NULL_HANDLE )
		{
			vkDestroyQueryPool( m_device, m_timestampPool, nullptr );
			m_timestampPool = VK_NULL_HANDLE;
		}
		if ( m_pipelineCache != VK_NULL_HANDLE )
		{
			vkDestroyPipelineCache( m_device, m_pipelineCache, nullptr );
			m_pipelineCache = VK_NULL_HANDLE;
		}
		m_pipelineVariants.clear();
		m_pipelineStoreDirectory.clear();
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
		          &m_renderPassLoadSrgb, &m_renderPassTargetSrgb, &m_renderPassClearSrgb } )
		{
			if ( *pass != VK_NULL_HANDLE )
			{
				vkDestroyRenderPass( m_device, *pass, nullptr );
				*pass = VK_NULL_HANDLE;
			}
		}

		m_debugUtils.Reset();
		m_shaderLibrary = VulkanShaderLibrary();
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
		m_host->DestroySurface( m_instance, m_surface );
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
	m_host = nullptr;
}

} // namespace render_vulkan
