//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan device bring-up and presentation core.
//          See vulkan_device.h for the contract.
//
//===========================================================================//

#include "vulkan_device.h"
#include "spv/material_spv.h"
#include "vulkan_present_mode.h"
#include "render/pbr_split_sum_table.h"
#include "render/render_sample_count.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <unordered_map>

namespace render_vulkan
{

namespace
{

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

	if ( !CreateDevice( outError ) || !CreateSwapchain( outError ) ||
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

// The device's extensions and features, for the device the adapter chose
// (render.device.vulkan adds its own requirements to both).
void CVulkanContext::DescribeDevice( VkPhysicalDevice physical, uint32_t graphicsFamily,
    render::device::vulkan::HostDeviceFeatures *out )
{
	m_physicalDevice = physical;
	m_graphicsQueueFamily = graphicsFamily;
	std::vector<const char *> &deviceExts = m_deviceExtensions;
	deviceExts.clear();
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
		m_toolingInfo = has( VK_EXT_TOOLING_INFO_EXTENSION_NAME );
		// A portability implementation (MoltenVK) that exposes
		// VK_KHR_portability_subset requires the application to enable it.
		// (The extension's name macro is in the beta header.)
		static const char *const kPortabilitySubset = "VK_KHR_portability_subset";
		if ( has( kPortabilitySubset ) )
		{
			deviceExts.push_back( kPortabilitySubset );
			Log( "device is a Vulkan portability implementation (VK_KHR_portability_subset)\n" );
		}
	}

	// Exact occlusion counts, which auto-exposure's luminance histogram needs
	// (a non-precise query may report any nonzero value for a visible draw).
	VkPhysicalDeviceFeatures supported = {};
	vkGetPhysicalDeviceFeatures( m_physicalDevice, &supported );
	VkPhysicalDeviceFeatures features = {};
	features.occlusionQueryPrecise = supported.occlusionQueryPrecise;
	features.samplerAnisotropy = supported.samplerAnisotropy;
	m_preciseOcclusion = supported.occlusionQueryPrecise == VK_TRUE;
	// D3D9's wireframe fill mode (materials with $wireframe, Wireframe_DX9).
	m_fillModeNonSolid = supported.fillModeNonSolid == VK_TRUE;
	features.fillModeNonSolid = supported.fillModeNonSolid;
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
	// SOURCE_VK_NO_BC=1: decompress DXT as a device without BC does (the
	// Adreno 730's path, reproduced on a desktop).
	const char *noBlocks = getenv( "SOURCE_VK_NO_BC" );
	m_blockCompression =
	    supported.textureCompressionBC == VK_TRUE && !( noBlocks && *noBlocks == '1' );
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
	out->features = m_featureChain.Head();
	out->extensions = deviceExts.data();
	out->extensionCount = static_cast<uint32_t>( deviceExts.size() );
}

bool CVulkanContext::PrepareDeviceRequest( IVulkanSurfaceHost &host,
    const VulkanContextConfig &config, render::device::vulkan::HostDeviceRequest *outRequest,
    std::string *outError )
{
	m_host = &host;
	m_config = config;
	m_instanceExtensions.clear();
	if ( !m_host->GetInstanceExtensions( &m_instanceExtensions, outError ) )
		return false;
	render::device::vulkan::HostDeviceRequest &request = *outRequest;
	request = render::device::vulkan::HostDeviceRequest();
	request.applicationName = m_config.appName ? m_config.appName : "Source Native Vulkan";
	request.instanceExtensions = m_instanceExtensions.data();
	request.instanceExtensionCount = static_cast<uint32_t>( m_instanceExtensions.size() );
	request.validation = m_config.enableValidation || m_config.requireValidation;
	request.requireValidation = m_config.requireValidation;
	request.debugUtils = m_config.debugLabels != DebugLabelPolicy::Off;
	request.messageCallback = VulkanDebugCallbackTrampoline;
	request.messageUser = this;
	request.requireDiscrete = m_config.requireDiscreteGpu;
	request.user = this;
	request.createSurface = []( void *user, VkInstance instance, VkSurfaceKHR *surface )
	{
		CVulkanContext *context = static_cast<CVulkanContext *>( user );
		context->m_instance = instance;
		if ( !context->CreateSurface( &context->m_createError ) )
			return false;
		*surface = context->m_surface;
		return true;
	};
	request.describeDevice = []( void *user, VkPhysicalDevice physical, uint32_t graphicsFamily,
	                             render::device::vulkan::HostDeviceFeatures *out )
	{
		static_cast<CVulkanContext *>( user )->DescribeDevice( physical, graphicsFamily, out );
	};
	m_createError.clear();
	return true;
}

bool CVulkanContext::CreateDevice( std::string *outError )
{
	render::device::vulkan::IHostDeviceOwner *owner = m_config.deviceOwner;
	if ( !owner )
	{
		SetError( outError,
		    "no Vulkan device owner is bound (render.device.vulkan: the composition "
		    "root binds it with NativeVulkanShaderBackend_BindDeviceOwner)" );
		return false;
	}
	char error[512] = {};
	// The root created the device for this window (CreateHostDeviceFor) from
	// this context's own request; otherwise the owner creates it now.
	m_hostDevice = owner->Device();
	if ( !m_hostDevice )
	{
		render::device::vulkan::HostDeviceRequest request;
		if ( !PrepareDeviceRequest( *m_host, m_config, &request, outError ) )
			return false;
		m_hostDevice = owner->Create( request, error, sizeof( error ) );
		m_releaseDevice = m_hostDevice != nullptr;
	}
	if ( m_hostDevice )
		Log( "device %s\n", m_releaseDevice ? "created by its owner for this context"
		                                    : "borrowed from the root's owner" );
	if ( !m_hostDevice )
	{
		// The adapter destroyed the surface with the instance.
		m_surface = VK_NULL_HANDLE;
		m_instance = VK_NULL_HANDLE;
		SetError( outError, m_createError.empty() ? std::string( error )
		                                          : std::string( error ) + ": " + m_createError );
		return false;
	}
	m_adapterAllocations = m_hostDevice->LiveAllocations();
	const render::device::vulkan::HostDeviceInfo &info = m_hostDevice->Info();
	m_instance = info.instance;
	m_physicalDevice = info.physical;
	m_device = info.device;
	m_surface = info.surface;
	m_graphicsQueueFamily = info.graphicsFamily;
	m_presentQueueFamily = info.presentFamily;
	m_graphicsQueue = info.graphicsQueue;
	m_presentQueue = info.presentQueue;
	m_validationEnabled = info.validation;
	m_debugUtilsExtension = info.debugUtils;
	m_computeCaps = m_featureChain.Enabled();

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

	SetupDebugTools( m_toolingInfo );
	m_compute.SetDebugTools( &m_shaderLibrary, &m_debugUtils );
	std::string computeError;
	if ( !m_compute.Init( *m_hostDevice, m_computeCaps, &computeError ) )
		std::fprintf( stderr, "[NativeVulkan] compute unavailable: %s\n", computeError.c_str() );
	std::fprintf( stderr,
	    "[NativeVulkan] compute %s, storage images %s, ray query %s (device API %u.%u)\n",
	    m_computeCaps.compute ? "on" : "off", m_computeCaps.storageImages ? "on" : "off",
	    m_computeCaps.rayQuery ? "on" : "off", VK_API_VERSION_MAJOR( m_computeCaps.deviceApiVersion ),
	    VK_API_VERSION_MINOR( m_computeCaps.deviceApiVersion ) );
	// Without BC the material system decompresses every DXT texture to RGBA,
	// four to eight times the memory and sampling bandwidth.
	std::fprintf( stderr, "[NativeVulkan] BC (DXT) textures %s\n",
	    m_blockCompression ? "sampled compressed" : "unsupported: decompressed to RGBA" );
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
	if ( !fmtCount )
	{
		SetError( outError, "surface offers no formats" );
		return false;
	}
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

	// The swapchain's own format: the back buffers' 8-bit format, or, when
	// extended output is asked for and everything it needs is here, linear
	// half floats in the extended linear sRGB color space (RFC 0016 "Output";
	// render.presentation.v1 "Dynamic range"). A declined request says why.
	m_presentFormat = m_swapFormat;
	m_presentColorSpace = m_swapColorSpace;
	m_swapchainExtendedRequest = m_requestedExtendedOutput;
	m_extendedOutput = false;
	m_outputDescription = false;
	m_presentDescriptionDirty = false;
	if ( m_requestedExtendedOutput )
	{
		const char *declined = nullptr;
		if ( !m_corePassRecorder )
			declined = "no core-pass recorder records the frame's output";
		else if ( !m_config.hdrScene && !m_srgbAttachments )
			declined = "the back buffer has no sRGB view to read linear values through";
		else if ( !m_host->CanShowExtendedRange() )
			declined = "the window cannot show extended range";
		else
		{
			declined = "the surface offers no HDR10 or extended-linear format";
			// Linux compositors commonly expose HDR10 rather than scRGB.
			for ( const VkSurfaceFormatKHR &f : formats )
			{
				if ( m_config.hdrScene && f.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 &&
				     f.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT )
				{
					m_presentFormat = f.format;
					m_presentColorSpace = f.colorSpace;
					m_extendedOutput = true;
					declined = nullptr;
					break;
				}
			}
			// The same PQ pixels under the output's own image description,
			// which the host declares (on Wayland, color-management-v1): the
			// WSI declares nothing for PASS_THROUGH, the surface then needs no
			// color mapping (the output encodes white at the display's SDR
			// white, ResolvedHdrSettings) and a compositor may scan it out.
			if ( m_extendedOutput && m_presentColorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT &&
			     !m_outputDescriptionRefused && m_host->PrepareOutputDescription() )
			{
				for ( const VkSurfaceFormatKHR &f : formats )
				{
					if ( f.format == m_presentFormat &&
					     f.colorSpace == VK_COLOR_SPACE_PASS_THROUGH_EXT )
					{
						m_presentColorSpace = f.colorSpace;
						m_outputDescription = true;
						break;
					}
				}
			}
			for ( const VkSurfaceFormatKHR &f : formats )
			{
				if ( !m_extendedOutput && f.format == VK_FORMAT_R16G16B16A16_SFLOAT &&
				     f.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT )
				{
					m_presentFormat = f.format;
					m_presentColorSpace = f.colorSpace;
					m_extendedOutput = true;
					declined = nullptr;
					break;
				}
			}
		}
		if ( declined )
			Log( "extended output declined: %s; presenting in the standard range\n", declined );
		else
			Log( "HDR output: %s\n",
			    m_outputDescription ? "Rec. 2020 / PQ (10-bit), the output's image description"
			    : m_presentColorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT ? "Rec. 2020 / PQ (10-bit)"
			                                                             : "extended linear sRGB" );
	}

	if ( m_config.hdrScene )
	{
		// Scene precision is independent of the display's dynamic range.
		m_swapFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		m_swapFormatSrgb = VK_FORMAT_UNDEFINED;
		m_srgbAttachments = false;
		Log( "HDR scene: linear RGBA16F, %s output\n", m_extendedOutput ? "HDR" : "SDR tone-mapped" );
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
	info.imageFormat = m_presentFormat;
	info.imageColorSpace = m_presentColorSpace;
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
	// (not a half-float one: the capture image holds the back buffers' 8-bit
	// format).
	m_presentCapturable = ( caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT ) != 0 &&
	                      !m_extendedOutput;
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

	// A surface takes one color-management object: the WSI declares its
	// swapchain's color space itself unless it is PASS_THROUGH.
	if ( !m_outputDescription )
		m_host->DetachOutputDescription();
	r = vkCreateSwapchainKHR( m_device, &info, nullptr, &m_swapchain );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkCreateSwapchainKHR failed: " ) + ResultString( r ) );
		return false;
	}
	++m_swapchainGeneration;
	if ( m_outputDescription && !m_host->AttachOutputDescription() )
	{
		// The frames would be shown as uninterpreted values: rebuild with the
		// WSI's HDR10 color space, and do not try again on this window.
		Log( "the output's image description could not be attached; using HDR10\n" );
		m_outputDescriptionRefused = true;
		m_presentDescriptionDirty = true;
	}

	uint32_t actual = 0;
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &actual, nullptr );
	m_presentImages.resize( actual );
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &actual, m_presentImages.data() );
	for ( uint32_t i = 0; i < actual; ++i )
		m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, m_presentImages[i], "swapchain image %u", i );
	if ( !GrowRenderFinished( actual, outError ) )
		return false;
	m_imageValue.assign( actual, 0 );
	// The window's presentation layer follows the swapchain's range (both
	// ways).
	if ( ( m_extendedOutput || m_requestedExtendedOutput ) &&
	     !m_host->SetExtendedRange( m_extendedOutput ) )
		Log( "the window refused its presentation layer's %s range\n",
		    m_extendedOutput ? "extended" : "standard" );
	return CreateBackBuffers( outError );
}

// One back buffer per swapchain image, so a frame never renders into one an
// earlier frame is still presenting from (m_imageValue guards both). They
// are the video mode's size; the present blit scales them to the drawable.
bool CVulkanContext::CreateBackBuffers( std::string *outError )
{
	const uint32_t actual = static_cast<uint32_t>( m_presentImages.size() );
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
		if ( !CreateImage( img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_swapImages[i],
		         &m_backBufferMemories[i], "back buffer", outError ) )
			return false;
		m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, m_swapImages[i], "back buffer %u", i );
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
		VkResult r = vkCreateImageView( m_device, &iv, nullptr, &m_swapImageViews[i] );
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

	return CreateDepthResources( outError );
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
		img.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
		            ( m_sceneDepthUsable ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0 );
		img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if ( !CreateImage( img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_depthImages[i],
		         &m_depthMemories[i], "depth", outError ) )
			return false;
		m_debugUtils.NameF( VK_OBJECT_TYPE_IMAGE, m_depthImages[i], "back buffer depth %u", i );

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
    VkFormat colorFormat, VkSampleCountFlagBits samples, std::string *outError, int keep )
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
	// a render-target pass and comes back to it with its depth intact. A plane
	// not in `keep` (a pass after the frame's last use of it) is neither loaded
	// nor stored, which on a tiled GPU saves two full-screen memory transfers.
	const auto loadFor = [&]( int plane )
	{ return ( keep & plane ) ? loadOp : VK_ATTACHMENT_LOAD_OP_DONT_CARE; };
	const auto storeFor = [&]( int plane )
	{ return ( keep & plane ) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE; };
	VkAttachmentDescription depth = {};
	depth.format = m_depthFormat;
	depth.samples = samples;
	depth.loadOp = loadFor( kKeepDepth );
	depth.storeOp = storeFor( kKeepDepth );
	// Stencil persists like depth: portal recursion spans render passes.
	depth.stencilLoadOp = loadFor( kKeepStencil );
	depth.stencilStoreOp = storeFor( kKeepStencil );
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
	                    VkFormat colorFormat, int keep = kKeepDepthStencil ) -> bool
	{
		return CreateAttachmentPass( loadOp, colorInitial, colorFinal, depthInitial, outPass,
		    colorFormat, VK_SAMPLE_COUNT_1_BIT, outError, keep );
	};
	// The back buffer's load passes that keep less than depth and stencil.
	auto makeKeepPasses = [&]( bool srgb, VkFormat colorFormat ) -> bool
	{
		for ( int keep = kKeepNone; keep < kKeepDepthStencil; ++keep )
		{
			if ( !makePass( VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			         &m_renderPassLoadKeep[srgb][keep], colorFormat, keep ) )
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
	       makeKeepPasses( false, m_swapFormat ) &&
	       makePass( VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &m_renderPassTarget,
	           m_swapFormat ) &&
	       ( !m_srgbAttachments ||
	           ( makePass( VK_ATTACHMENT_LOAD_OP_LOAD, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &m_renderPassLoadSrgb,
	                 m_swapFormatSrgb ) &&
	               makeKeepPasses( true, m_swapFormatSrgb ) &&
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

	// Frames record into the render core's port encoders (RFC 0016 K3), so
	// the context allocates no frame command buffers of its own.
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
	if ( !GrowRenderFinished( std::max<size_t>( m_framesInFlight, m_presentImages.size() ),
	         outError ) )
		return false;

	VkSemaphoreCreateInfo sem = {};
	sem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for ( uint32_t i = 0; i < m_framesInFlight; ++i )
	{
		m_slotValue[i] = 0;
		if ( vkCreateSemaphore( m_device, &sem, nullptr, &m_imageAvailable[i] ) != VK_SUCCESS )
		{
			SetError( outError, "failed to create per-frame synchronization objects" );
			return false;
		}
	}
	return true;
}

bool CVulkanContext::CreateCaptureImage( VkExtent2D extent, std::string *outError )
{
	const VkFormat format = m_frameCaptureHdr   ? VK_FORMAT_R16G16B16A16_SFLOAT
	                        : m_config.hdrScene ? VK_FORMAT_R8G8B8A8_UNORM
	                                            : m_swapFormat;
	// Recreate only when the target extent changed.
	if ( m_captureImage != VK_NULL_HANDLE && m_captureExtent.width == extent.width &&
	     m_captureExtent.height == extent.height && m_captureFormat == format )
		return true;

	if ( m_captureOutput.IsValid() && Port() )
		(void)Port()->Release( m_captureOutput, render::device::CompletionToken() );
	m_captureOutput = {};
	if ( m_captureOutputImage != VK_NULL_HANDLE )
		vkDestroyImage( m_device, m_captureOutputImage, nullptr );
	m_captureOutputImage = VK_NULL_HANDLE;
	if ( m_captureOutputMemory != VK_NULL_HANDLE )
		FreeMemory( m_captureOutputMemory );
	m_captureOutputMemory = VK_NULL_HANDLE;
	if ( m_captureImage != VK_NULL_HANDLE )
	{
		vkDestroyImage( m_device, m_captureImage, nullptr );
		m_captureImage = VK_NULL_HANDLE;
	}
	if ( m_captureMemory != VK_NULL_HANDLE )
	{
		FreeMemory( m_captureMemory );
		m_captureMemory = VK_NULL_HANDLE;
	}

	VkImageCreateInfo img = {};
	img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img.imageType = VK_IMAGE_TYPE_2D;
	img.format = format;
	img.extent = { extent.width, extent.height, 1 };
	img.mipLevels = 1;
	img.arrayLayers = 1;
	img.samples = VK_SAMPLE_COUNT_1_BIT;
	img.tiling = VK_IMAGE_TILING_LINEAR;
	img.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	img.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Host-visible and host-cached first: the CPU reads every texel back
	// (ResolveCapturedPixels), and uncached (write-combined) memory made that
	// read take 1.3 s for a 1080p frame on RADV, every save's thumbnail a
	// hitch. Then coherent, then any host-visible type (reads invalidate).
	std::string cachedError, coherentError;
	if ( !CreateImage( img,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
	         &m_captureImage, &m_captureMemory, "capture", &cachedError ) &&
	     !CreateImage( img,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	         &m_captureImage, &m_captureMemory, "capture", &coherentError ) &&
	     !CreateImage( img, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_captureImage, &m_captureMemory,
	         "capture", outError ) )
		return false;
	if ( m_config.hdrScene && !m_frameCaptureHdr )
	{
		img.tiling = VK_IMAGE_TILING_OPTIMAL;
		img.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if ( !CreateImage( img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_captureOutputImage,
		         &m_captureOutputMemory, "SDR capture output", outError ) )
			return false;
		render::device::TextureDesc desc;
		desc.format = render::device::Format::kRGBA8Unorm;
		desc.width = extent.width;
		desc.height = extent.height;
		desc.usages = { render::device::ResourceUsage::kColorAttachment,
		    render::device::ResourceUsage::kCopySource };
		if ( !m_hostDevice->ImportImage( m_captureOutputImage, desc,
		         render::device::ResourceUsage::kColorAttachment, &m_captureOutput ) )
			return false;
	}
	m_captureExtent = extent;
	m_captureFormat = format;
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
    VkMemoryPropertyFlags props, VkBuffer *outBuffer, VulkanMemory *outMemory,
    std::string *outError )
{
	VkBufferCreateInfo bi = {};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = size;
	bi.usage = usage;
	bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	render::device::vulkan::HostMemory memory;
	memory.required = props;
	const VkResult r = m_hostDevice->CreateBuffer( bi, memory, outBuffer, outMemory );
	if ( r != VK_SUCCESS )
	{
		*outBuffer = VK_NULL_HANDLE;
		*outMemory = nullptr;
		SetError( outError, std::string( "buffer creation failed: " ) + ResultString( r ) );
		return false;
	}
	return true;
}

bool CVulkanContext::CreateImage( const VkImageCreateInfo &info, VkMemoryPropertyFlags props,
    VkImage *outImage, VulkanMemory *outMemory, const char *what, std::string *outError )
{
	render::device::vulkan::HostMemory memory;
	memory.required = props;
	const VkResult r = m_hostDevice->CreateImage( info, memory, outImage, outMemory );
	if ( r != VK_SUCCESS )
	{
		*outImage = VK_NULL_HANDLE;
		*outMemory = nullptr;
		SetError(
		    outError, std::string( "image creation (" ) + what + ") failed: " + ResultString( r ) );
		return false;
	}
	return true;
}

VkResult CVulkanContext::MapMemory( VulkanMemory memory, void **data )
{
	return m_hostDevice->Map( memory, data );
}

void CVulkanContext::UnmapMemory( VulkanMemory memory )
{
	m_hostDevice->Unmap( memory );
}

void CVulkanContext::FreeMemory( VulkanMemory memory )
{
	if ( memory )
		m_hostDevice->Free( memory );
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

bool CVulkanContext::EndSingleTimeCommands(
    VkCommandBuffer cmd, std::string *outError, uint64_t *outValue )
{
	CFrameCostScope cost( m_frameCost, kCostSingleSubmit );
	if ( outValue )
		*outValue = 0;
	if ( vkEndCommandBuffer( cmd ) != VK_SUCCESS )
	{
		SetError( outError, "vkEndCommandBuffer (single-time) failed" );
		vkFreeCommandBuffers( m_device, m_commandPool, 1, &cmd );
		return false;
	}
	// Later submissions on this queue follow it (its barriers make its writes
	// visible to them); nothing waits for it here.
	const VkSemaphore timeline = m_hostDevice->Timeline();
	const uint64_t value = m_hostDevice->NextSubmitValue();
	VkTimelineSemaphoreSubmitInfo values = {};
	values.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	values.signalSemaphoreValueCount = 1;
	values.pSignalSemaphoreValues = &value;
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = &values;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &timeline;
	const VkResult r = vkQueueSubmit( m_graphicsQueue, 1, &submit, VK_NULL_HANDLE );
	if ( r != VK_SUCCESS )
	{
		m_hostDevice->AbandonValue( value );
		vkFreeCommandBuffers( m_device, m_commandPool, 1, &cmd );
		SetError(
		    outError, std::string( "single-time command submit failed: " ) + ResultString( r ) );
		return false;
	}
	m_oneTimeCommands.emplace_back( value, cmd );
	if ( outValue )
		*outValue = value;
	return true;
}

void CVulkanContext::ReleaseBufferAfter( uint64_t value, VkBuffer buffer, VulkanMemory memory )
{
	m_hostDevice->ReleaseBufferAfter( value, buffer, memory );
}

void CVulkanContext::ReleaseStreamBufferAfter( uint64_t value, StreamBuffer &stream )
{
	if ( stream.mapped )
		UnmapMemory( stream.memory );
	if ( stream.buffer != VK_NULL_HANDLE || stream.memory )
		m_hostDevice->ReleaseBufferAfter( value, stream.buffer, stream.memory );
	stream = StreamBuffer();
}

bool CVulkanContext::InitDynamicMesh( std::string *outError )
{
	if ( !IsValid() || m_renderPass == VK_NULL_HANDLE )
	{
		SetError( outError, "InitDynamicMesh before a valid Init()" );
		return false;
	}
	if ( m_dynamicResourcesReady )
		return true;

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
		if ( !CreateImage( ii, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_dynTexImage, &m_dynTexMemory,
		         "dynamic texture", outError ) )
			return false;
		m_debugUtils.Name( VK_OBJECT_TYPE_IMAGE, m_dynTexImage, "built-in fallback texture" );

		VkBuffer staging = VK_NULL_HANDLE;
		VulkanMemory stagingMem = VK_NULL_HANDLE;
		if ( !CreateBuffer( sizeof( texels ), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		         &staging, &stagingMem, outError ) )
			return false;
		void *mapped = nullptr;
		MapMemory( stagingMem, &mapped );
		std::memcpy( mapped, texels, sizeof( texels ) );
		UnmapMemory( stagingMem );

		VkCommandBuffer cmd = VK_NULL_HANDLE;
		if ( !BeginSingleTimeCommands( &cmd, outError ) )
		{
			vkDestroyBuffer( m_device, staging, nullptr );
			FreeMemory( stagingMem );
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
		uint64_t uploaded = 0;
		if ( !EndSingleTimeCommands( cmd, outError, &uploaded ) )
		{
			vkDestroyBuffer( m_device, staging, nullptr );
			FreeMemory( stagingMem );
			return false;
		}
		ReleaseBufferAfter( uploaded, staging, stagingMem );

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
		// The reflection probes' neutral inputs: a two-cube array and a table
		// whose first word (the probe count) is 0, bound where a map has none.
		m_neutralProbeCubesHandle = CreateManagedTexture(
		    1, 1, VK_FORMAT_R8G8B8A8_UNORM, outError, 0, 1, VK_FORMAT_UNDEFINED, true, 1, 2 );
		NameManagedTexture( m_neutralProbeCubesHandle, "neutral reflection cubes" );
		if ( m_neutralProbeCubesHandle < 0 )
			return false;
		for ( uint32_t face = 0; face < 12; ++face )
			if ( !UploadManagedTexture(
			         m_neutralProbeCubesHandle, white, sizeof( white ), outError, 0, face ) )
				return false;
		m_neutralProbeTableHandle = CreateManagedTexture(
		    1, 1, VK_FORMAT_R32_UINT, outError, 0, 1, VK_FORMAT_UNDEFINED, false );
		NameManagedTexture( m_neutralProbeTableHandle, "neutral reflection table" );
		const uint32_t noProbes = 0;
		if ( m_neutralProbeTableHandle < 0 ||
		     !UploadManagedTexture( m_neutralProbeTableHandle,
		         reinterpret_cast<const uint8_t *>( &noProbes ), sizeof( noProbes ), outError ) )
			return false;
		SetManagedTextureSamplerState( m_neutralProbeTableHandle, kSamplerClampU | kSamplerClampV );
		// Opaque black 1x1: what a deferred render target samples (and thereby
		// "reads") before its storage is allocated on first use.
		m_unrenderedTargetHandle = CreateManagedTexture(
		    1, 1, VK_FORMAT_R8G8B8A8_UNORM, outError, 0, 1, VK_FORMAT_UNDEFINED, false );
		NameManagedTexture( m_unrenderedTargetHandle, "deferred render target (black)" );
		const uint8_t black[4] = { 0, 0, 0, 255 };
		if ( m_unrenderedTargetHandle < 0 ||
		     !UploadManagedTexture( m_unrenderedTargetHandle, black, sizeof( black ), outError ) )
			return false;
	}
	m_dynamicResourcesReady = true;
	Log( "dynamic mesh resources ready (scene depth %s)\n",
	    m_sceneDepthUsable ? "readable" : "unavailable" );
	return true;
}

void CVulkanContext::SetDirectLights( const DirectLight *lights, uint32_t count )
{
	m_directLightCount = std::min( count, kMaxDirectLights );
	for ( uint32_t i = 0; i < m_directLightCount; ++i )
		m_directLights[i] = lights[i];
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
	m_anisotropyLevel = selected;
}

int CVulkanContext::CreateManagedTexture( int width, int height, VkFormat format,
    std::string *outError, VkImageUsageFlags extraUsage, uint32_t mipLevels, VkFormat srgbAlias,
    bool cube, uint32_t depth, uint32_t cubeCount )
{
	CFrameCostScope cost( m_frameCost, kCostTextureCreate );
	const bool volume = depth > 1;
	if ( !IsValid() || width <= 0 || height <= 0 || ( cube && width != height ) || depth == 0 ||
	     ( volume && cube ) || cubeCount == 0 || ( cubeCount > 1 && !cube ) || cubeCount > 256 )
	{
		SetError( outError, "CreateManagedTexture with invalid size or context" );
		return -1;
	}
	if ( cubeCount > 1 &&
	     !m_hostDevice->Port().Facts().capabilities.Has(
	         render::device::Capability::kCubeArrays ) )
	{
		SetError( outError, "cube arrays are unsupported on the selected device" );
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
	t.layers = cube ? 6 * cubeCount : 1;
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
		// Frozen-path: core progress - P2:CE's VTF 7.6 BC7 textures (base
		// colors are sampled through the sRGB view, as BC1-BC3).
		case VK_FORMAT_BC7_UNORM_BLOCK:
			srgbAlias = VK_FORMAT_BC7_SRGB_BLOCK;
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
	if ( !CreateImage( ii, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &t.image, &t.memory,
	         "managed texture", outError ) )
		return -1;

	VkImageViewCreateInfo iv = {};
	iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	iv.image = t.image;
	iv.viewType = cube ? ( cubeCount > 1 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE )
	              : volume ? VK_IMAGE_VIEW_TYPE_3D
	                       : VK_IMAGE_VIEW_TYPE_2D;
	iv.format = format;
	iv.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, t.mipLevels, 0, t.layers };
	if ( vkCreateImageView( m_device, &iv, nullptr, &t.view ) != VK_SUCCESS )
	{
		FreeMemory( t.memory );
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
			FreeMemory( t.memory );
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
	// A render target whose storage is still deferred borrowed its view and
	// descriptor sets from the shared opaque-black image (m_unrenderedTargetHandle);
	// they are owned there, and its image/depth/framebuffers never existed.
	if ( t.storagePending )
	{
		t = ManagedTexture();
		return;
	}
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
		FreeMemory( t.depthMemory );
	if ( t.view != VK_NULL_HANDLE )
		vkDestroyImageView( m_device, t.view, nullptr );
	if ( t.image != VK_NULL_HANDLE )
		vkDestroyImage( m_device, t.image, nullptr );
	if ( t.memory != VK_NULL_HANDLE )
		FreeMemory( t.memory );
	t = ManagedTexture();
}

void CVulkanContext::DestroyManagedTexture( int handle )
{
	if ( !IsValid() || handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
		return;
	ManagedTexture &slot = m_managedTextures[static_cast<size_t>( handle )];
	// A deferred render target has no image yet; a slot with neither is already destroyed.
	if ( slot.image == VK_NULL_HANDLE && !slot.storagePending )
		return;
	// Retired after the frame being recorded is submitted and complete: until
	// then no record can see the handle reused.
	ReleaseManagedImport( handle );
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

void CVulkanContext::UpdateCompletedSerial()
{
	const uint64_t completed = m_hostDevice->CompletedValue();
	size_t done = 0;
	while ( done < m_serialValues.size() && m_serialValues[done].second <= completed )
		m_completedSerial = std::max( m_completedSerial, m_serialValues[done++].first );
	m_serialValues.erase( m_serialValues.begin(), m_serialValues.begin() + done );
	size_t kept = 0;
	for ( const std::pair<uint64_t, VkCommandBuffer> &submitted : m_oneTimeCommands )
	{
		if ( submitted.first <= completed )
			vkFreeCommandBuffers( m_device, m_commandPool, 1, &submitted.second );
		else
			m_oneTimeCommands[kept++] = submitted;
	}
	m_oneTimeCommands.resize( kept );
	// The adapter's own releases behind completed values.
	m_hostDevice->Collect();
}

void CVulkanContext::RetireCompletedTextures()
{
	if ( m_hostDevice )
		UpdateCompletedSerial();
	m_compute.Collect( m_completedSerial );
	size_t keptObjects = 0;
	for ( size_t i = 0; i < m_retiredObjects.size(); ++i )
	{
		const RetiredObject retired = m_retiredObjects[i];
		if ( retired.afterSerial <= m_completedSerial )
			retired.destroy( this, retired.a, retired.b );
		else
			m_retiredObjects[keptObjects++] = retired;
	}
	m_retiredObjects.resize( keptObjects );
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

int CVulkanContext::CreateRenderTargetTexture(
    int width, int height, std::string *outError, bool deferStorage )
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
	// Deferred: the target reads and clears to opaque black until something
	// actually draws into, copies into or imports it, at which point
	// MaterializeRenderTarget builds its storage. This avoids ~96 MiB per
	// full-frame target at 4K for the several the material system never uses.
	if ( deferStorage && m_unrenderedTargetHandle >= 0 )
	{
		const ManagedTexture &black =
		    m_managedTextures[static_cast<size_t>( m_unrenderedTargetHandle )];
		ManagedTexture t;
		t.width = static_cast<uint32_t>( width );
		t.height = static_cast<uint32_t>( height );
		t.mipLevels = 1;
		t.layers = 1;
		t.depth = 1;
		t.format = m_swapFormat;
		t.uploaded = true;
		t.renderTarget = true;
		t.storagePending = true;
		t.view = black.view;
		t.srgbView = black.srgbView;
		t.descSet = black.descSet;
		t.descSetSrgb = black.descSetSrgb;
		return StoreManagedTexture( t );
	}
	const int handle = CreateManagedTexture( width, height, m_swapFormat, outError,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 1,
	    m_srgbAttachments ? m_swapFormatSrgb : VK_FORMAT_UNDEFINED );
	if ( handle < 0 )
		return -1;
	if ( !BuildRenderTargetStorage( m_managedTextures[static_cast<size_t>( handle )], outError ) )
		return -1;
	return handle;
}

bool CVulkanContext::BuildRenderTargetStorage( ManagedTexture &t, std::string *outError )
{
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
	if ( !CreateImage( di, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &t.depthImage, &t.depthMemory,
	         "render-target depth", outError ) )
		return false;

	VkImageViewCreateInfo dv = {};
	dv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	dv.image = t.depthImage;
	dv.viewType = VK_IMAGE_VIEW_TYPE_2D;
	dv.format = m_depthFormat;
	dv.subresourceRange = { m_depthAspects, 0, 1, 0, 1 };
	if ( vkCreateImageView( m_device, &dv, nullptr, &t.depthView ) != VK_SUCCESS )
	{
		SetError( outError, "vkCreateImageView (render-target depth) failed" );
		return false;
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
		return false;
	}
	if ( t.srgbView != VK_NULL_HANDLE && m_srgbAttachments )
	{
		VkImageView srgbAttachments[] = { t.srgbView, t.depthView };
		fb.renderPass = m_renderPassTargetSrgb;
		fb.pAttachments = srgbAttachments;
		if ( vkCreateFramebuffer( m_device, &fb, nullptr, &t.framebufferSrgb ) != VK_SUCCESS )
		{
			SetError( outError, "vkCreateFramebuffer (render target, sRGB) failed" );
			return false;
		}
	}

	// Bring both images to the layouts m_renderPassTarget expects on entry, with
	// defined contents: a target sampled before anything renders into it reads
	// opaque black, as a freshly created D3D9 render target does in practice.
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( !BeginSingleTimeCommands( &cmd, outError ) )
		return false;
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
		return false;

	t.renderTarget = true;
	t.uploaded = true;
	return true;
}

bool CVulkanContext::MaterializeRenderTarget( int handle, std::string *outError )
{
	if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
	{
		SetError( outError, "MaterializeRenderTarget with an invalid handle" );
		return false;
	}
	const ManagedTexture &shell = m_managedTextures[static_cast<size_t>( handle )];
	if ( !shell.storagePending )
		return true;
	// Capture the shell's facts first: CreateManagedTexture may reallocate
	// m_managedTextures and invalidate `shell`.
	const uint32_t width = shell.width, height = shell.height;
	std::string label = shell.debugName;
	const int sampler = shell.samplerState;
	const int builtHandle =
	    CreateManagedTexture( static_cast<int>( width ), static_cast<int>( height ), m_swapFormat,
	        outError, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 1,
	        m_srgbAttachments ? m_swapFormatSrgb : VK_FORMAT_UNDEFINED );
	if ( builtHandle < 0 )
		return false;
	ManagedTexture built = m_managedTextures[static_cast<size_t>( builtHandle )];
	m_managedTextures[static_cast<size_t>( builtHandle )] = ManagedTexture();
	m_freeTextureHandles.push_back( builtHandle );
	if ( !BuildRenderTargetStorage( built, outError ) )
	{
		ReleaseManagedTextureObjects( built );
		return false;
	}
	built.debugName = std::move( label );
	ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
	t = built;
	t.storagePending = false;
	t.samplerState = sampler;
	if ( sampler != 0 )
	{
		// The fresh sets were written with the default sampler; a filter was
		// requested while the target was deferred, so point them at it now.
		for ( int srgb = 0; srgb < 2; ++srgb )
		{
			const VkDescriptorSet set = srgb ? t.descSetSrgb : t.descSet;
			if ( set == VK_NULL_HANDLE )
				continue;
			VkDescriptorImageInfo dii = {};
			dii.sampler = m_samplers[sampler];
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
	return true;
}

void CVulkanContext::EnsureRenderTargetStorage( int handle )
{
	if ( !IsStoragePending( handle ) )
		return;
	std::string error;
	if ( MaterializeRenderTarget( handle, &error ) )
		return;
	const std::string name = handle >= 0 && handle < static_cast<int>( m_managedTextures.size() )
	                             ? m_managedTextures[static_cast<size_t>( handle )].debugName
	                             : std::string();
	const std::string message =
	    name.empty()
	        ? std::string(
	              "render.device.vulkan: out of device memory allocating a render target: " ) +
	              error
	        : std::string(
	              "render.device.vulkan: out of device memory allocating render target '" ) +
	              name + "': " + error;
	if ( m_config.fatalError )
		m_config.fatalError( message.c_str() );
	else
	{
		Log( "fatal: %s\n", message.c_str() );
		std::abort();
	}
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
	// A deferred render target's sets are the shared opaque-black image's; never
	// retire them. The requested filter is applied when its storage is
	// materialized (MaterializeRenderTarget).
	if ( t.storagePending )
		return;
	if ( t.descSet == VK_NULL_HANDLE || m_samplers[samplerState] == VK_NULL_HANDLE )
		return;
	// Source sets sampler state while it creates the texture, before any frame
	// samples it. A later change must not rewrite a set that a submitted frame
	// may still be reading: the new sampler goes into fresh sets (a replay looks
	// the texture's set up by handle, so everything not yet submitted uses them)
	// and the old sets retire behind the newest submitted frame.
	VkDescriptorSet *const owned[2] = { &t.descSet, &t.descSetSrgb };
	VkDescriptorSet fresh[2] = {};
	bool allocated =
	    m_dynTexDescPool != VK_NULL_HANDLE && m_liveTextureSets + 2 < kMaxManagedTexSets;
	for ( int srgb = 0; srgb < 2 && allocated; ++srgb )
	{
		if ( *owned[srgb] == VK_NULL_HANDLE )
			continue;
		VkDescriptorSetAllocateInfo da = {};
		da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		da.descriptorPool = m_dynTexDescPool;
		da.descriptorSetCount = 1;
		da.pSetLayouts = &m_dynTexDescLayout;
		allocated = vkAllocateDescriptorSets( m_device, &da, &fresh[srgb] ) == VK_SUCCESS;
		if ( allocated )
			++m_liveTextureSets;
		else
			fresh[srgb] = VK_NULL_HANDLE;
	}
	if ( allocated )
	{
		for ( int srgb = 0; srgb < 2; ++srgb )
		{
			if ( *owned[srgb] == VK_NULL_HANDLE )
				continue;
			m_retiredObjects.push_back( { m_submitSerial,
			    []( CVulkanContext *context, uint64_t set, uint64_t )
			    {
				    const VkDescriptorSet retired = reinterpret_cast<VkDescriptorSet>( set );
				    vkFreeDescriptorSets(
				        context->m_device, context->m_dynTexDescPool, 1, &retired );
				    --context->m_liveTextureSets;
			    },
			    reinterpret_cast<uint64_t>( *owned[srgb] ), 0 } );
			*owned[srgb] = fresh[srgb];
		}
	}
	else
	{
		// The pool is full: rewrite in place once the newest submitted frame,
		// the last that can read the sets, has completed (a timeline wait).
		for ( VkDescriptorSet set : fresh )
		{
			if ( set != VK_NULL_HANDLE )
			{
				vkFreeDescriptorSets( m_device, m_dynTexDescPool, 1, &set );
				--m_liveTextureSets;
			}
		}
		CFrameCostScope cost( m_frameCost, kCostFenceWait );
		WaitForSubmittedFrame( m_submitSerial, UINT64_MAX );
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

bool IsSrgbFormat( VkFormat format )
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
		char message[160];
		std::snprintf( message, sizeof( message ),
		    "UploadManagedTexture region %ux%u at (%u,%u) outside level %u (%ux%u) of texture %d",
		    width, height, x, y, level, levelWidth, levelHeight, handle );
		SetError( outError, message );
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
		t.contentRevision = m_nextTextureContentRevision++;
		if ( level == 0 )
			t.uploaded = true;
		return true;
	}

	// Synchronous: after whatever is pending, so uploads keep their order.
	if ( !FlushPendingUploads( outError ) )
		return false;
	VkBuffer staging = VK_NULL_HANDLE;
	VulkanMemory stagingMem = VK_NULL_HANDLE;
	if ( !CreateBuffer( dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging,
	         &stagingMem, outError ) )
		return false;
	void *mapped = nullptr;
	MapMemory( stagingMem, &mapped );
	std::memcpy( mapped, data, dataSize );
	UnmapMemory( stagingMem );

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( !BeginSingleTimeCommands( &cmd, outError ) )
	{
		vkDestroyBuffer( m_device, staging, nullptr );
		FreeMemory( stagingMem );
		return false;
	}
	upload.offset = 0;
	RecordTextureUpload( cmd, t.image, upload, staging, 0 );
	uint64_t uploaded = 0;
	const bool ok = EndSingleTimeCommands( cmd, outError, &uploaded );
	if ( ok )
		ReleaseBufferAfter( uploaded, staging, stagingMem );
	else
	{
		vkDestroyBuffer( m_device, staging, nullptr );
		FreeMemory( stagingMem );
	}
	if ( ok )
	{
		t.contentRevision = m_nextTextureContentRevision++;
		if ( level == 0 )
			t.uploaded = true;
	}
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

void CVulkanContext::RecordTextureUploadRun( VkCommandBuffer cmd, VkImage image,
    const PendingUpload *uploads, size_t count, VkBuffer staging )
{
	// RecordTextureUpload's barriers once for the run: every region keeps the
	// texels outside it (preserve) and none overlaps another.
	const PendingUpload &first = uploads[0];
	VkImageMemoryBarrier toDst = {};
	toDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toDst.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toDst.image = image;
	toDst.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, first.level, 1, first.face, 1 };
	toDst.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toDst );
	std::vector<VkBufferImageCopy> copies( count );
	for ( size_t i = 0; i < count; ++i )
	{
		VkBufferImageCopy &copy = copies[i];
		copy = {};
		copy.bufferOffset = uploads[i].offset;
		copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, first.level, first.face, 1 };
		copy.imageOffset = {
		    static_cast<int32_t>( uploads[i].x ), static_cast<int32_t>( uploads[i].y ), 0 };
		copy.imageExtent = { uploads[i].width, uploads[i].height, 1 };
	}
	vkCmdCopyBufferToImage( cmd, staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    static_cast<uint32_t>( count ), copies.data() );
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
	// Consecutive regions of one image level that keep the rest of it and do
	// not overlap share one pair of barriers and one copy (a probe volume's
	// publication is thousands of tiles; a barrier pair each cost milliseconds
	// of recording). Overlapping regions start a new run, so later texels
	// still land after earlier ones.
	std::unordered_map<uint64_t, std::vector<uint32_t>> cells;
	const auto cellKey = []( uint32_t cx, uint32_t cy )
	{
		return ( uint64_t( cy ) << 32 ) | cx;
	};
	constexpr uint32_t kCell = 64;
	const auto overlaps = [&]( const PendingUpload &a, size_t first, bool add )
	{
		for ( uint32_t cy = a.y / kCell; cy <= ( a.y + a.height - 1 ) / kCell; ++cy )
			for ( uint32_t cx = a.x / kCell; cx <= ( a.x + a.width - 1 ) / kCell; ++cx )
			{
				std::vector<uint32_t> &list = cells[cellKey( cx, cy )];
				if ( !add )
				{
					for ( uint32_t k : list )
					{
						const PendingUpload &b = m_pendingUploads[first + k];
						if ( a.x < b.x + b.width && b.x < a.x + a.width && a.y < b.y + b.height &&
						     b.y < a.y + a.height )
							return true;
					}
				}
				else
					list.push_back( uint32_t( &a - &m_pendingUploads[first] ) );
			}
		return false;
	};
	const auto joins = []( const PendingUpload &first, const PendingUpload &u )
	{
		return u.handle == first.handle && u.level == first.level && u.face == first.face &&
		       u.preserve && !u.clearRest && u.depth <= 1;
	};
	for ( size_t i = 0; i < m_pendingUploads.size(); )
	{
		const PendingUpload &first = m_pendingUploads[i];
		const ManagedTexture &t = m_managedTextures[static_cast<size_t>( first.handle )];
		size_t end = i + 1;
		if ( joins( first, first ) )
		{
			cells.clear();
			overlaps( first, i, true );
			while ( end < m_pendingUploads.size() && joins( first, m_pendingUploads[end] ) &&
			        !overlaps( m_pendingUploads[end], i, false ) )
				overlaps( m_pendingUploads[end++], i, true );
		}
		if ( t.image != VK_NULL_HANDLE )
		{
			if ( end - i == 1 )
				RecordTextureUpload( cmd, t.image, first, stream.buffer, first.offset );
			else
				RecordTextureUploadRun(
				    cmd, t.image, &m_pendingUploads[i], end - i, stream.buffer );
		}
		i = end;
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
		uint64_t uploaded = 0;
		if ( EndSingleTimeCommands( cmd, outError, &uploaded ) )
			ReleaseStreamBufferAfter( uploaded, stream );
		else
			ok = false;
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

CVulkanContext::DynDraw &CVulkanContext::AppendRecord( int kind )
{
	// The first record after a present starts the next frame. Discarding the
	// presented frame here rather than at Present keeps it available for an
	// on-demand capture (ReadPixels) until the engine starts drawing again.
	if ( m_dynFramePresented )
		ClearDynamicQueue();
	// A render target drawn into for the first time allocates its deferred
	// storage now, before the draw records against it.
	if ( kind == kRecordDraw && IsStoragePending( m_dynTarget ) )
		EnsureRenderTargetStorage( m_dynTarget );
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
	// A deferred target already reads opaque black: a color-only clear to that
	// value is a no-op, so it needs neither storage nor a record (the engine
	// clears its never-used full-frame targets once at creation).
	if ( IsStoragePending( m_dynTarget ) && color && !depth && !stencil &&
	     m_clearColor.float32[0] == 0.0f && m_clearColor.float32[1] == 0.0f &&
	     m_clearColor.float32[2] == 0.0f && m_clearColor.float32[3] == 1.0f )
		return;
	EnsureRenderTargetStorage( m_dynTarget );
	DynDraw &d = AppendRecord( kRecordClear );
	NoteSceneChanged();
	d.clearColor = color;
	d.clearDepth = depth;
	d.clearStencil = stencil;
	for ( int i = 0; i < 4; ++i )
		{
		const float value = m_clearColor.float32[i];
		d.clearValue[i] = m_config.hdrScene && i < 3
		                     ? ( value <= 0.04045f ? value / 12.92f
		                                              : std::pow( ( value + 0.055f ) / 1.055f, 2.4f ) )
		                     : value;
	}
}

bool CVulkanContext::QueueCopyToTexture(
    int dstHandle, const int *srcRect, const int *dstRect, const DepthToAlpha *depthToAlpha )
{
	if ( !IsRenderTargetTexture( dstHandle ) || dstHandle == m_dynTarget )
		return false;
	// Copying into a deferred target allocates its storage now.
	EnsureRenderTargetStorage( dstHandle );
	// The depth image the alpha pass reads is the scene capture's: make sure it
	// exists, and have glass capture again after this copy has reused it.
	std::string error;
	const bool depthAlpha = depthToAlpha && EnsureSceneCapture( &error );
	DynDraw &d = AppendRecord( kRecordCopy );
	if ( dstHandle == m_sceneCaptureTarget || depthAlpha )
		NoteSceneChanged();
	d.copyDst = dstHandle;
	d.copyDepthToAlpha = depthAlpha;
	if ( depthAlpha )
		d.copyDepth = *depthToAlpha;
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
	m_queuedOcclusionQuery = query;
}

void CVulkanContext::QueueEndOcclusionQuery( int query )
{
	if ( query < 0 || query >= static_cast<int>( m_querySlots.size() ) )
		return;
	AppendRecord( kRecordQueryEnd ).query = query;
	if ( m_queuedOcclusionQuery == query )
		m_queuedOcclusionQuery = -1;
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

void CVulkanContext::BeginTargetPass( VkCommandBuffer cmd, int target, bool srgb, int keep )
{
	// Re-entering a target loads what it already holds; clears arrive as
	// explicit records, exactly where the engine issued them. `srgb` enters it
	// through its sRGB view.
	srgb = srgb && m_srgbAttachments;
	int kept = kKeepDepthStencil; // what the chosen pass loads and stores
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
		// Load/store ops are outside render-pass compatibility, so the passes
		// that keep less use the same framebuffers and pipelines.
		if ( keep == kKeepDepthStencil || m_renderPassLoadKeep[srgb][keep] == VK_NULL_HANDLE )
			rp.renderPass = srgb ? m_renderPassLoadSrgb : m_renderPassLoad;
		else
		{
			rp.renderPass = m_renderPassLoadKeep[srgb][keep];
			kept = keep;
		}
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
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
	{
		static const char *const kKinds[] = { "pass (no depth, no stencil)", "pass (no stencil)",
			"pass (no depth)", "pass" };
		GpuTimerMark( cmd, GpuTimerTargetLabel( kKinds[kept], target, srgb ) );
	}
	vkCmdBeginRenderPass( cmd, &rp, VK_SUBPASS_CONTENTS_INLINE );
	m_frameCost.Add( kCostRenderPass, 0 );
}

void CVulkanContext::NameManagedTexture( int handle, const char *name )
{
	if ( !name || handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
		return;
	ManagedTexture &t = m_managedTextures[static_cast<size_t>( handle )];
	// Kept without debug utils too: the GPU timers label passes by target.
	t.debugName = name;
	if ( !m_debugUtils.Active() )
		return;
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

int CVulkanContext::RecordBackBufferDepthReads( const DynDraw &r )
{
	// Only reads keep a plane: a write nothing later reads is dead, since the
	// next frame's first pass clears depth and stencil.
	if ( r.target != -1 )
		return kKeepNone;
	switch ( r.kind )
	{
	case kRecordDraw:
	{
		const DynRasterState &s = r.raster;
		const bool depth = s.depthTest && s.depthCompare != VK_COMPARE_OP_ALWAYS;
		const bool stencil = s.stencilEnable && s.stencilCompare != VK_COMPARE_OP_ALWAYS;
		return ( depth ? kKeepDepth : 0 ) | ( stencil ? kKeepStencil : 0 );
	}
	case kRecordClear:
		return kKeepNone;
	case kRecordCopy:
		return r.copyDepthToAlpha ? kKeepDepth : kKeepNone;
	case kRecordQueryBegin:
	case kRecordQueryEnd:
		// A query counts the samples of the draws inside it, which are judged
		// by their own state (the auto-exposure histogram draws at the end of
		// every HDR frame test no depth).
		return kKeepNone;
	default:
		return kKeepDepthStencil; // scene captures (they copy depth) and anything new
	}
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
bool CVulkanContext::FirstPassWantsSrgb() const
{
	if ( !m_passMerging || m_renderPassClearSrgb == VK_NULL_HANDLE || !m_dynamicResourcesReady )
		return false;
	for ( float component : m_clearColor.float32 )
		if ( component != 0.0f && component != 1.0f )
			return false;
	for ( const DynDraw &r : m_dynDrawRecords )
	{
		// A slot closes the pass; the view it reopens with is chosen there.
		if ( r.kind == kRecordCorePass )
			continue;
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
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
		GpuTimerMark( cmd, GpuTimerTargetLabel( "copy to", copy.copyDst, false ) );
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

bool CVulkanContext::EnsureDepthToAlpha( std::string *outError )
{
	if ( m_depthToAlphaPipeline != VK_NULL_HANDLE )
		return true;
	if ( m_depthToAlphaUnavailable || m_renderPassTarget == VK_NULL_HANDLE ||
	     m_dynTexDescLayout == VK_NULL_HANDLE )
	{
		SetError( outError, "the depth-to-alpha pass is unavailable" );
		return false;
	}
	m_depthToAlphaUnavailable = true; // until it is built
	VkPushConstantRange range = {};
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	range.size = sizeof( float ) * 9;
	VkPipelineLayoutCreateInfo li = {};
	li.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	li.setLayoutCount = 1;
	li.pSetLayouts = &m_dynTexDescLayout;
	li.pushConstantRangeCount = 1;
	li.pPushConstantRanges = &range;
	if ( vkCreatePipelineLayout( m_device, &li, nullptr, &m_depthToAlphaLayout ) != VK_SUCCESS )
	{
		SetError( outError, "depth-to-alpha pipeline layout failed" );
		return false;
	}
	VkShaderModule vert = VK_NULL_HANDLE, frag = VK_NULL_HANDLE, fragMs = VK_NULL_HANDLE;
	if ( !CreateShaderModule(
	         g_presentGammaVertSpv, sizeof( g_presentGammaVertSpv ), &vert, outError ) ||
	     !CreateShaderModule(
	         g_depthToAlphaFragSpv, sizeof( g_depthToAlphaFragSpv ), &frag, outError ) ||
	     !CreateShaderModule(
	         g_depthToAlphaMsFragSpv, sizeof( g_depthToAlphaMsFragSpv ), &fragMs, outError ) )
	{
		for ( VkShaderModule module : { vert, frag } )
			if ( module != VK_NULL_HANDLE )
				vkDestroyShaderModule( m_device, module, nullptr );
		DestroyDepthToAlpha();
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
	// The target pass carries the target's depth; this pass neither tests nor
	// writes it.
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState blendAttachment = {};
	blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_A_BIT;
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
	gp.pDepthStencilState = &ds;
	gp.pColorBlendState = &blend;
	gp.pDynamicState = &dyn;
	gp.layout = m_depthToAlphaLayout;
	gp.renderPass = m_renderPassTarget;
	VkResult r = vkCreateGraphicsPipelines(
	    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_depthToAlphaPipeline );
	if ( r == VK_SUCCESS )
	{
		stages[1].module = fragMs;
		r = vkCreateGraphicsPipelines(
		    m_device, VK_NULL_HANDLE, 1, &gp, nullptr, &m_depthToAlphaMsPipeline );
	}
	m_debugUtils.Name( VK_OBJECT_TYPE_PIPELINE, m_depthToAlphaPipeline, "depth to alpha" );
	m_debugUtils.Name(
	    VK_OBJECT_TYPE_PIPELINE, m_depthToAlphaMsPipeline, "depth to alpha (multisampled)" );
	for ( VkShaderModule module : { vert, frag, fragMs } )
		vkDestroyShaderModule( m_device, module, nullptr );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "depth-to-alpha pipeline failed: " ) + ResultString( r ) );
		DestroyDepthToAlpha();
		return false;
	}
	m_depthToAlphaUnavailable = false;
	return true;
}

void CVulkanContext::DestroyDepthToAlpha()
{
	for ( VkPipeline pipeline : { m_depthToAlphaPipeline, m_depthToAlphaMsPipeline } )
		if ( pipeline != VK_NULL_HANDLE )
			vkDestroyPipeline( m_device, pipeline, nullptr );
	m_depthToAlphaMsPipeline = VK_NULL_HANDLE;
	if ( m_depthToAlphaLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_depthToAlphaLayout, nullptr );
	m_depthToAlphaPipeline = VK_NULL_HANDLE;
	m_depthToAlphaLayout = VK_NULL_HANDLE;
}

// The depth-to-alpha pass reads a multisampled back buffer's depth directly (a
// multisampled depth cannot be copied into a single-sampled image): a
// depth-only view of it and a texture set, made on first use for the current
// MSAA targets (DestroyMsaaTargets releases them).
bool CVulkanContext::EnsureMsDepthSampleSet()
{
	if ( m_msDepthSampleSet != VK_NULL_HANDLE )
		return true;
	if ( !m_sceneDepthUsable || m_msDepth == VK_NULL_HANDLE || m_dynTexDescPool == VK_NULL_HANDLE ||
	     m_liveTextureSets + 1 >= kMaxManagedTexSets )
		return false;
	if ( m_msDepthSampleView == VK_NULL_HANDLE )
	{
		VkImageViewCreateInfo iv = {};
		iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv.image = m_msDepth;
		iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv.format = m_depthFormat;
		iv.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
		if ( vkCreateImageView( m_device, &iv, nullptr, &m_msDepthSampleView ) != VK_SUCCESS )
		{
			m_msDepthSampleView = VK_NULL_HANDLE;
			return false;
		}
	}
	VkDescriptorSetAllocateInfo da = {};
	da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	da.descriptorPool = m_dynTexDescPool;
	da.descriptorSetCount = 1;
	da.pSetLayouts = &m_dynTexDescLayout;
	if ( vkAllocateDescriptorSets( m_device, &da, &m_msDepthSampleSet ) != VK_SUCCESS )
	{
		m_msDepthSampleSet = VK_NULL_HANDLE;
		return false;
	}
	++m_liveTextureSets;
	VkDescriptorImageInfo image = {};
	image.sampler = m_dynTexSampler;
	image.imageView = m_msDepthSampleView;
	image.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_msDepthSampleSet;
	write.dstBinding = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &image;
	vkUpdateDescriptorSets( m_device, 1, &write, 0, nullptr );
	return true;
}

// After RecordTargetCopy, outside any render pass: the source's depth into the
// scene capture's depth image, then the copy's alpha from it in the
// destination's own pass (it rests in SHADER_READ_ONLY either side).
void CVulkanContext::RecordDepthToAlpha( VkCommandBuffer cmd, int srcTarget, const DynDraw &copy )
{
	uint32_t srcW = 0, srcH = 0;
	GetTargetExtent( srcTarget, &srcW, &srcH );
	std::string error;
	// A multisampled back buffer's depth is read in place; any other source's
	// is copied into the scene capture's depth image first.
	const bool multisampled = srcTarget == -1 && m_activeSamples > 1;
	const bool ready = EnsureDepthToAlpha( &error ) &&
	                   ( multisampled ? EnsureMsDepthSampleSet()
	                                  : m_sceneDepthHandle >= 0 &&
	                                        RecordSceneDepthCopy( cmd, srcTarget, srcW, srcH ) );
	if ( !ready )
	{
		static bool s_reported = false;
		if ( !s_reported )
			Log( "frame copy without depth in alpha: %s\n",
			    error.empty() ? "no single-sampled depth to copy" : error.c_str() );
		s_reported = true;
		++m_lastFrameDepthToAlphaSkipped;
		return;
	}
	ScopedDebugLabel label( m_debugUtils, "depth to alpha" );
	uint32_t depthW = srcW, depthH = srcH;
	VkDescriptorSet depthSet = m_msDepthSampleSet;
	if ( !multisampled )
	{
		const ManagedTexture &depth = m_managedTextures[static_cast<size_t>( m_sceneDepthHandle )];
		depthW = depth.width;
		depthH = depth.height;
		depthSet = depth.descSet;
	}
	const ManagedTexture &dst = m_managedTextures[static_cast<size_t>( copy.copyDst )];
	// The rectangles as RecordTargetCopy clamps them.
	const auto rect = []( const int *r, uint32_t w, uint32_t h, float *out )
	{
		float x0 = 0.0f, y0 = 0.0f, x1 = static_cast<float>( w ), y1 = static_cast<float>( h );
		if ( r[2] > 0 && r[3] > 0 )
		{
			x0 = static_cast<float>( std::max( 0, r[0] ) );
			y0 = static_cast<float>( std::max( 0, r[1] ) );
			x1 = static_cast<float>( std::min( static_cast<int>( w ), r[0] + r[2] ) );
			y1 = static_cast<float>( std::min( static_cast<int>( h ), r[1] + r[3] ) );
		}
		out[0] = x0;
		out[1] = y0;
		out[2] = x1 - x0;
		out[3] = y1 - y0;
		return out[2] > 0.0f && out[3] > 0.0f;
	};
	float src[4], dstRect[4];
	if ( !rect( copy.copySrcRect, srcW, srcH, src ) ||
	     !rect( copy.copyDstRect, dst.width, dst.height, dstRect ) )
		return;
	struct
	{
		float projection[4];
		float source[4];
		float invRange;
	} constants;
	std::memcpy( constants.projection, copy.copyDepth.projection, sizeof( constants.projection ) );
	constants.source[0] = src[0] / depthW;
	constants.source[1] = src[1] / depthH;
	constants.source[2] = src[2] / depthW;
	constants.source[3] = src[3] / depthH;
	constants.invRange = copy.copyDepth.invRange;

	// The multisampled depth rests as an attachment between passes.
	VkImageMemoryBarrier toRead = {};
	toRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toRead.image = m_msDepth;
	toRead.subresourceRange = { m_depthAspects, 0, 1, 0, 1 };
	toRead.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	toRead.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	toRead.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	toRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	if ( multisampled )
		vkCmdPipelineBarrier( cmd,
		    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toRead );

	BeginTargetPass( cmd, copy.copyDst, false, kKeepDepthStencil );
	VkViewport viewport = { dstRect[0], dstRect[1], dstRect[2], dstRect[3], 0.0f, 1.0f };
	VkRect2D scissor = { { static_cast<int32_t>( dstRect[0] ), static_cast<int32_t>( dstRect[1] ) },
		{ static_cast<uint32_t>( dstRect[2] ), static_cast<uint32_t>( dstRect[3] ) } };
	vkCmdSetViewport( cmd, 0, 1, &viewport );
	vkCmdSetScissor( cmd, 0, 1, &scissor );
	vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	    multisampled ? m_depthToAlphaMsPipeline : m_depthToAlphaPipeline );
	vkCmdBindDescriptorSets(
	    cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_depthToAlphaLayout, 0, 1, &depthSet, 0, nullptr );
	vkCmdPushConstants( cmd, m_depthToAlphaLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
	    sizeof( constants ), &constants );
	vkCmdDraw( cmd, 3, 1, 0, 0 );
	vkCmdEndRenderPass( cmd );
	if ( multisampled )
	{
		VkImageMemoryBarrier back = toRead;
		back.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		back.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		back.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		back.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		    0, 0, nullptr, 0, nullptr, 1, &back );
	}
	++m_lastFrameDepthToAlpha;
}

void CVulkanContext::DestroyDynamicMesh()
{
	m_dynamicResourcesReady = false;
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
	if ( m_worldVert != VK_NULL_HANDLE )
	{
		vkDestroyShaderModule( m_device, m_worldVert, nullptr );
		m_worldVert = VK_NULL_HANDLE;
	}
	for ( const auto &entry : m_portalPipelines )
		vkDestroyPipeline( m_device, entry.second, nullptr );
	m_portalPipelines.clear();
	DestroyDepthToAlpha();
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
	m_msDepthSampleSet = VK_NULL_HANDLE; // freed with the pool
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
		FreeMemory( m_dynTexMemory );
		m_dynTexMemory = VK_NULL_HANDLE;
	}
	// The device is idle here: every texture, deleted or not, can go.
	for ( ManagedTexture &t : m_managedTextures )
		ReleaseManagedTextureObjects( t );
	m_managedTextures.clear();
	m_whiteVolumeHandle = -1;
	m_neutralProbeCubesHandle = m_neutralProbeTableHandle = -1;
	m_reflectionProbeHandle = m_reflectionTableHandle = -1;
	m_sceneColorHandle = -1;
	m_sceneDepthHandle = -1;
	m_sceneDepthCaptured = false;
	for ( RetiredTexture &r : m_retiredTextures )
		ReleaseManagedTextureObjects( r.texture );
	m_retiredTextures.clear();
	for ( const RetiredObject &retired : m_retiredObjects )
		retired.destroy( this, retired.a, retired.b );
	m_retiredObjects.clear();
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
	     MapMemory( stream.memory, &stream.mapped ) != VK_SUCCESS )
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
		UnmapMemory( stream.memory );
	if ( stream.buffer != VK_NULL_HANDLE )
		vkDestroyBuffer( m_device, stream.buffer, nullptr );
	if ( stream.memory != VK_NULL_HANDLE )
		FreeMemory( stream.memory );
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
	const VkResult mapResult = MapMemory( staging.memory, &mapped );
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
	uint64_t uploadValue = 0;
	const bool uploaded = EndSingleTimeCommands( cmd, outError, &uploadValue );
	if ( uploaded )
		ReleaseStreamBufferAfter( uploadValue, staging );
	else
	{
		DestroyStreamBuffer( staging );
		DestroyStreamBuffer( newIndices );
		DestroyStreamBuffer( newVertices );
		return false;
	}

	SetWorldLightmapHandle( -1 );
	SetProbeVolumeHandles( -1, -1, 0 );
	SetProbeDeltaHandle( -1 );
	SetShadowField( -1, nullptr, 0.0f, nullptr );
	SetReflectionProbes( nullptr, 0, 0, 0, 0, nullptr );
	// Submitted frames may still read the old mesh; nothing recorded later
	// does (a replay binds the resident buffers). Released behind the newest
	// submission's value, without waiting.
	ReleaseStreamBufferAfter( m_hostDevice->SubmittedValue(), m_worldIndexBuffer );
	ReleaseStreamBufferAfter( m_hostDevice->SubmittedValue(), m_worldVertexBuffer );
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

// The probe words as an R32_UINT texture (kProbeTableWidth wide) the shaders
// fetch by index (world_pbr_probe.glsl ReflectionProbesWord).
static const uint32_t kProbeTableWidth = 4096;

// The words padded to whole rows of the table texture.
static std::vector<uint32_t> ProbeTableTexels( const std::vector<uint32_t> &words, uint32_t *rows )
{
	*rows = ( uint32_t( words.size() ) + kProbeTableWidth - 1 ) / kProbeTableWidth;
	std::vector<uint32_t> table( size_t( *rows ) * kProbeTableWidth, 0 );
	std::copy( words.begin(), words.end(), table.begin() );
	return table;
}

bool CVulkanContext::SetReflectionProbes( const uint32_t *words, uint32_t wordCount, uint32_t count,
    uint32_t faceSize, uint32_t mips, const uint8_t *blocks, std::string *outError )
{
	const auto release = [this]()
	{
		if ( m_reflectionProbeHandle >= 0 )
			DestroyManagedTexture( m_reflectionProbeHandle );
		if ( m_reflectionTableHandle >= 0 )
			DestroyManagedTexture( m_reflectionTableHandle );
		m_reflectionProbeHandle = m_reflectionTableHandle = -1;
	};
	if ( !words )
	{
		release();
		m_reflectionProbeWords.clear();
		return true;
	}
	if ( count == 0 || mips == 0 || faceSize == 0 || !blocks || wordCount < 16 )
	{
		if ( outError )
			*outError = "reflection probe upload is malformed";
		return false;
	}
	// Two cubes at least: one probe's array would be a plain cube view.
	const uint32_t cubes = std::max( count, 2u );
	std::string detail;
	const int radiance = CreateManagedTexture( int( faceSize ), int( faceSize ),
	    VK_FORMAT_BC6H_UFLOAT_BLOCK, &detail, 0, mips, VK_FORMAT_UNDEFINED, true, 1, cubes );
	NameManagedTexture( radiance, "RPRB reflection probes" );
	bool ok = radiance >= 0;
	size_t offset = 0;
	for ( uint32_t level = 0; ok && level < mips; ++level )
	{
		const uint32_t size = faceSize >> level;
		const size_t faceBytes = size_t( ( size + 3 ) / 4 ) * ( ( size + 3 ) / 4 ) * 16;
		for ( uint32_t layer = 0; ok && layer < 6 * count; ++layer )
		{
			ok =
			    UploadManagedTexture( radiance, blocks + offset, faceBytes, &detail, level, layer );
			offset += faceBytes;
		}
	}
	if ( !ok )
	{
		if ( radiance >= 0 )
			DestroyManagedTexture( radiance );
		if ( outError )
			*outError = "reflection probe cube array upload failed: " + detail;
		return false;
	}
	SetManagedTextureSamplerState(
	    radiance, kSamplerClampU | kSamplerClampV | kSamplerLinear | kSamplerMipLinear );
	std::vector<uint32_t> copy( words, words + wordCount );
	copy[4] = uint32_t( m_reflectionProbeMode );
	copy[5] = 0; // no relight in this backend
	uint32_t rows = 0;
	const std::vector<uint32_t> table = ProbeTableTexels( copy, &rows );
	const int tableHandle = CreateManagedTexture(
	    int( kProbeTableWidth ), int( rows ), VK_FORMAT_R32_UINT, &detail, 0, 1 );
	NameManagedTexture( tableHandle, "RPRB probe table" );
	if ( tableHandle < 0 ||
	     !UploadManagedTexture( tableHandle, reinterpret_cast<const uint8_t *>( table.data() ),
	         table.size() * 4, &detail ) )
	{
		DestroyManagedTexture( radiance );
		if ( tableHandle >= 0 )
			DestroyManagedTexture( tableHandle );
		if ( outError )
			*outError = "reflection probe table upload failed: " + detail;
		return false;
	}
	// Fetched by index: nearest, clamped.
	SetManagedTextureSamplerState( tableHandle, kSamplerClampU | kSamplerClampV );
	release();
	m_reflectionProbeHandle = radiance;
	m_reflectionTableHandle = tableHandle;
	m_reflectionProbeWords.swap( copy );
	return true;
}

void CVulkanContext::SetReflectionProbeMode( int mode )
{
	// Valid modes are 0..3 and 5..7 (reflection_probes.h); anything else
	// is the default blend.
	if ( mode < 0 || mode > 7 || mode == 4 )
		mode = 1;
	if ( mode == m_reflectionProbeMode )
		return;
	m_reflectionProbeMode = mode;
	if ( m_reflectionTableHandle < 0 || m_reflectionProbeWords.size() < 16 )
		return;
	// A new table with the new mode word; the old one is retired behind the
	// frames that read it.
	std::string error;
	m_reflectionProbeWords[4] = uint32_t( mode );
	uint32_t rows = 0;
	const std::vector<uint32_t> table = ProbeTableTexels( m_reflectionProbeWords, &rows );
	const int handle = CreateManagedTexture(
	    int( kProbeTableWidth ), int( rows ), VK_FORMAT_R32_UINT, &error, 0, 1 );
	if ( handle < 0 ||
	     !UploadManagedTexture(
	         handle, reinterpret_cast<const uint8_t *>( table.data() ), table.size() * 4, &error ) )
	{
		if ( handle >= 0 )
			DestroyManagedTexture( handle );
		Log( "mat_reflection_probes %d not applied: %s\n", mode, error.c_str() );
		return;
	}
	SetManagedTextureSamplerState( handle, kSamplerClampU | kSamplerClampV );
	DestroyManagedTexture( m_reflectionTableHandle );
	m_reflectionTableHandle = handle;
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

void CVulkanContext::ReleaseWorldMesh()
{
	if ( m_device == VK_NULL_HANDLE )
		return;
	++m_worldMeshRevision;
	SetWorldLightmapHandle( -1 );
	SetProbeVolumeHandles( -1, -1, 0 );
	SetProbeDeltaHandle( -1 );
	SetShadowField( -1, nullptr, 0.0f, nullptr );
	SetReflectionProbes( nullptr, 0, 0, 0, 0, nullptr );
	// Released behind the newest submission, without waiting (UploadWorldMesh).
	ReleaseStreamBufferAfter( m_hostDevice->SubmittedValue(), m_worldIndexBuffer );
	ReleaseStreamBufferAfter( m_hostDevice->SubmittedValue(), m_worldVertexBuffer );
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
	// A serial not yet known complete is still in m_serialValues.
	for ( const std::pair<uint64_t, uint64_t> &submitted : m_serialValues )
	{
		if ( submitted.first != serial )
			continue;
		if ( m_hostDevice->WaitValue( submitted.second, timeoutNs ) != VK_SUCCESS )
			return false;
		// One queue completes submissions in order.
		m_completedSerial = std::max( m_completedSerial, serial );
		return true;
	}
	return false;
}

bool CVulkanContext::PrepareFrame( bool *outSkip, std::string *outError )
{
	if ( outSkip )
		*outSkip = false;
	m_frameBeginUs = FrameClockMicros();
	m_frameBeginCpuUs = ThreadCpuMicros();
	if ( !IsValid() )
	{
		SetError( outError, "PrepareFrame on an invalid context" );
		return false;
	}
	if ( m_frameOpen )
	{
		SetError( outError, "PrepareFrame called while a frame is already open" );
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
	const bool outputRangePending = m_requestedExtendedOutput != m_swapchainExtendedRequest;
	if ( m_suboptimalPending )
	{
		m_suboptimalPending = false;
		m_suboptimalRebuilt = true;
		++m_suboptimalRebuilds;
		m_presentDescriptionDirty = true; // the shared rebuild trigger below
		Log( "swapchain suboptimal with an unchanged surface; rebuilding it (%u)\n",
		    m_suboptimalRebuilds );
	}
	// The compositor's preferred description changed (brightness, mode): the
	// swapchain is rebuilt and declares the new one.
	if ( m_outputDescription && m_host->OutputDescriptionChanged() )
		m_presentDescriptionDirty = true;
	// A new back-buffer size alone keeps the swapchain (RecreateBackBuffers).
	if ( drawableChanged || presentModePending || outputRangePending || m_acquireTimedOut ||
	     m_presentDescriptionDirty ||
	     ( backBufferPending && m_swapchain == VK_NULL_HANDLE ) )
	{
		if ( !RecreateSwapchain( outError ) )
			return false;
	}
	else if ( backBufferPending && !RecreateBackBuffers( outError ) )
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

	if ( m_slotValue[m_currentFrame] != 0 )
	{
		CFrameCostScope wait( m_frameCost, kCostFenceWait );
		m_hostDevice->WaitValue( m_slotValue[m_currentFrame], UINT64_MAX );
	}
	// One queue completes submissions in order: this slot's is done, so are all
	// before it.
	RetireCompletedTextures();
	ReadSlotGpuTime( m_currentFrame );
	// The slot's grouped sets are no longer read: they return to its pools.

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
	// Mesa's Wayland WSI reports new dmabuf feedback (a compositor's scanout
	// tranche) as a suboptimal acquire: the image is still used this frame
	// and the swapchain is rebuilt before the next (NoteSuboptimal).
	if ( r == VK_SUBOPTIMAL_KHR )
		NoteSuboptimal();
	else if ( r == VK_SUCCESS )
		m_suboptimalRebuilt = false;
	if ( r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR )
	{
		SetError( outError, std::string( "vkAcquireNextImageKHR failed: " ) + ResultString( r ) );
		return false;
	}

	// If a previous frame is still using this image, wait for its value.
	if ( m_imageValue[imageIndex] != 0 )
	{
		CFrameCostScope wait( m_frameCost, kCostFenceWait );
		m_hostDevice->WaitValue( m_imageValue[imageIndex], UINT64_MAX );
	}

	m_acquiredImage = imageIndex;

	// The back buffer is captured (what ReadPixels reads), or on request the
	// swapchain image the window presents. Its image is made here: recording
	// runs inside the adapter's Submit, where nothing may fail halfway.
	m_frameCapture = m_captureRequested && ( !m_capturePresented || m_presentCapturable );
	m_frameCapturePresented = m_frameCapture && m_capturePresented;
	m_frameCaptureHdr =
	    m_frameCapture && !m_frameCapturePresented && m_captureHdrRequested && m_config.hdrScene;
	if ( m_frameCapture &&
	     !CreateCaptureImage( m_frameCapturePresented ? m_presentExtent : m_swapExtent, outError ) )
		return false;
	m_frameOpen = true;
	return true;
}

render::device::IRenderDevice2 *CVulkanContext::Port()
{
	return m_hostDevice ? &m_hostDevice->Port() : nullptr;
}

bool CVulkanContext::HasFrameStage( FrameStage stage ) const
{
	switch ( stage )
	{
	case kFrameStageResolve:
		return m_activeSamples > 1;
	case kFrameStageCapture:
		return m_frameCapture && !m_frameCapturePresented;
	default:
		return stage >= kFrameStageComputeAndUploads && stage < kFrameStageCount;
	}
}

void CVulkanContext::AttachFrameStage( FrameStage stage, render::device::CommandEncoder &encoder )
{
	static const render::device::vulkan::IHostDevice::NativeRecord kRecords[kFrameStageCount] = {
	    []( void *user, VkCommandBuffer cmd )
	    {
		    static_cast<CVulkanContext *>( user )->RecordFrameStage(
		        kFrameStageComputeAndUploads, cmd );
	    },
	    []( void *user, VkCommandBuffer cmd )
	    {
		    static_cast<CVulkanContext *>( user )->RecordFrameStage( kFrameStageScene, cmd );
	    },
	    []( void *user, VkCommandBuffer cmd )
	    {
		    static_cast<CVulkanContext *>( user )->RecordFrameStage( kFrameStageResolve, cmd );
	    },
	    []( void *user, VkCommandBuffer cmd )
	    {
		    static_cast<CVulkanContext *>( user )->RecordFrameStage( kFrameStageCapture, cmd );
	    },
	    []( void *user, VkCommandBuffer cmd )
	    {
		    static_cast<CVulkanContext *>( user )->RecordFrameStage( kFrameStagePresent, cmd );
	    },
	};
	m_hostDevice->RecordNative( encoder, kRecords[stage], this );
	if ( stage == kFrameStageScene )
		RecordCorePassSections( encoder );
	if ( stage == kFrameStageCapture && m_config.hdrScene && !m_frameCaptureHdr )
	{
		auto targets = CoreOutputTargetsFor();
		targets.target = m_captureOutput;
		targets.targetFormat = render::device::Format::kRGBA8Unorm;
		targets.width = m_captureExtent.width;
		targets.height = m_captureExtent.height;
		targets.headroom = 1.0f;
		targets.linearScale = 1.0f;
		m_hostDevice->BeginSection( encoder );
		m_captureOutputRecorded =
		    m_corePassRecorder && m_corePassRecorder->RecordOutput( encoder, targets );
		m_hostDevice->EndSection( encoder );
	}
	if ( stage != kFrameStagePresent )
		return;
	RecordOutputSection( encoder );
	// Only the present (a blit, or the gamma pass's color writes) touches the
	// acquired swapchain image; rendering into the back buffer need not wait
	// for the acquire. The wait and signal apply to the whole submission.
	m_hostDevice->AddSubmitWait( encoder, m_imageAvailable[m_currentFrame],
	    VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT );
	m_hostDevice->AddSubmitSignal( encoder, m_renderFinished[m_acquiredImage] );
}

void CVulkanContext::QueueCorePass( uint32_t tag, const CorePassTerms &terms )
{
	if ( !m_corePassRecorder )
		return;
	// AppendRecord starts a new queue after present; publish the policy after
	// that reset so the following draws see it before vertex conversion.
	DynDraw &record = AppendRecord( kRecordCorePass );
	if ( tag == render::legacy::kCorePassLegacyHud )
		m_queueLegacyHud = true;
	if ( ( tag & render::legacy::kCorePassForwarded ) &&
	     ( tag & render::legacy::kCorePassLegacyOff ) )
	{
		m_queueCoreOnly = true;
		m_queueCustomEffects = ( tag & render::legacy::kCorePassCustomEffects ) != 0;
	}
	record.corePass = tag;
	record.corePassTerms = static_cast<uint32_t>( m_corePassTerms.size() );
	m_corePassTerms.push_back( terms );
}

bool CVulkanContext::SkipLegacyRecord( const DynDraw &d, bool legacyOff, bool legacyHud )
{
	// Depth-alpha snapshots are ordered inputs to core soft particles. Their
	// producer must survive the suppression of replaced legacy draws.
	return legacyOff && !legacyHud &&
	       ( ( d.kind == kRecordDraw && !d.coreCustomEffect && !d.queryInput ) ||
	           ( d.kind == kRecordCopy && !d.corePortalCopy && !d.copyDepthToAlpha ) ||
	           d.kind == kRecordSceneCapture ||
	           ( d.kind == kRecordClear && !d.clearDepth && !d.clearStencil ) );
}

void CVulkanContext::RecordCorePassSections( render::device::CommandEncoder &encoder )
{
	if ( !m_corePassRecorder )
		return;
	// One section per slot record, whether the replay reaches it or not (an
	// unreached section runs after the scene record, drawing into the back
	// buffer as it then stands).
	m_opaqueCandidates = m_opaqueBatches = m_opaqueFollowers = 0;
	bool legacyOff = false, legacyHud = false;
	std::size_t followers = 0;
	std::vector<std::uint32_t> tags;
	for ( std::size_t index = 0; index < m_dynDrawRecords.size(); ++index )
	{
		const DynDraw &d = m_dynDrawRecords[index];

		if ( d.kind != kRecordCorePass )
			continue;
		if ( d.corePass == render::legacy::kCorePassLegacyHud )
			legacyHud = true;
		if ( ( d.corePass & render::legacy::kCorePassForwarded ) &&
		     ( d.corePass & render::legacy::kCorePassLegacyOff ) )
			legacyOff = true;
		if ( followers )
		{
			m_hostDevice->BeginSection( encoder );
			--followers;
			m_hostDevice->EndSection( encoder );
			continue;
		}
		render::legacy::CorePassTarget target = CorePassTargetFor( d.target );
		if ( d.corePassTerms < m_corePassTerms.size() )
		{
			const CorePassTerms &terms = m_corePassTerms[d.corePassTerms];
			target.drawState = terms.drawState;
			std::memcpy( target.clipPlanes, terms.clipPlanes, sizeof( target.clipPlanes ) );
			target.minDepth = terms.minDepth;
			target.maxDepth = terms.maxDepth;
			target.lightmapScale = terms.lightmapScale;
			target.outputScale = terms.outputScale;
			std::copy( terms.eye, terms.eye + 3, target.eye );
			target.envmapScale = terms.envmapScale;
			target.specular = terms.specular;
			target.ssbumpNormalized = terms.ssbumpNormalized;
			target.fog = terms.fog;
			target.time = terms.time;
			std::memcpy( target.foliage, terms.foliage, sizeof( target.foliage ) );
			target.foliageAvailable = terms.foliageAvailable;
			target.waterReflectTintScale = terms.waterReflectTintScale;
		}
		tags.clear();
		tags.push_back( d.corePass );
		if ( m_opaqueBatching && d.corePassTerms < m_corePassTerms.size() )
		{
			for ( std::size_t next = index + 1; next < m_dynDrawRecords.size(); ++next )
			{
				const DynDraw &candidate = m_dynDrawRecords[next];
				if ( SkipLegacyRecord( candidate, legacyOff, legacyHud ) )
					continue;
				if ( candidate.kind != kRecordCorePass || candidate.target != d.target ||
				     candidate.corePassTerms >= m_corePassTerms.size() )
					break;
				// The opaque-batch contract admits only time-independent draws.
				// Preserve every other captured term; ordered animated draws are
				// rejected by the recorder and use their original time below.
				auto nextTerms = m_corePassTerms[candidate.corePassTerms];
				nextTerms.time = m_corePassTerms[d.corePassTerms].time;
				if ( !( nextTerms == m_corePassTerms[d.corePassTerms] ) )
					break;
				tags.push_back( candidate.corePass );
				// Policy markers themselves always remain a boundary.
				if ( candidate.corePass & render::legacy::kCorePassLegacyOff ||
				     candidate.corePass == render::legacy::kCorePassLegacyHud )
					break;
			}
		}
		m_opaqueCandidates += tags.size() > 1;
		// Managed render targets rest in sampled layout outside their legacy
		// render passes. Both color aliases name one image: transition only the
		// view the core will write, then restore it before the host resumes.
		const auto attachment = target.colorSrgb.IsValid() ? target.colorSrgb : target.color;
		const bool offscreen = d.target >= 0 && attachment.IsValid();
		const auto sampledAttachment = offscreen ? attachment : render::device::TextureId{};
		m_hostDevice->BeginSection( encoder, sampledAttachment );
		const auto count = m_corePassRecorder->RecordOpaqueBatch( tags, encoder, target );
		if ( count == 0 || count > tags.size() )
			Log( "error: opaque recorder returned an invalid consumed-slot count\n" );
		followers = count > 0 && count <= tags.size() ? count - 1 : 0;
		if ( followers )
		{
			++m_opaqueBatches;
			m_opaqueFollowers += followers;
		}
		m_hostDevice->EndSection( encoder, sampledAttachment );
	}
}

namespace
{

render::device::Format PortFormat( VkFormat format )
{
	switch ( format )
	{
	case VK_FORMAT_B8G8R8A8_UNORM:
		return render::device::Format::kBGRA8Unorm;
	case VK_FORMAT_R8G8B8A8_UNORM:
		return render::device::Format::kRGBA8Unorm;
	case VK_FORMAT_D24_UNORM_S8_UINT:
		return render::device::Format::kD24UnormS8;
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return render::device::Format::kD32FloatS8;
	case VK_FORMAT_D32_SFLOAT:
		return render::device::Format::kD32Float;
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		return render::device::Format::kBC1Unorm;
	case VK_FORMAT_BC2_UNORM_BLOCK:
		return render::device::Format::kBC2Unorm;
	case VK_FORMAT_BC3_UNORM_BLOCK:
		return render::device::Format::kBC3Unorm;
	case VK_FORMAT_BC7_UNORM_BLOCK:
		return render::device::Format::kBC7Unorm;
	case VK_FORMAT_R16G16B16A16_UNORM:
		return render::device::Format::kRGBA16Unorm;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		return render::device::Format::kRGB10A2Unorm;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return render::device::Format::kRGBA16Float;
	default:
		return render::device::Format::kUnknown;
	}
}

} // namespace

// The sRGB view format of a unorm 8-bit or BC format; kUnknown for others.
static render::device::Format SrgbPortFormat( render::device::Format format )
{
	using render::device::Format;
	switch ( format )
	{
	case Format::kRGBA8Unorm:
		return Format::kRGBA8Srgb;
	case Format::kBGRA8Unorm:
		return Format::kBGRA8Srgb;
	case Format::kBC1Unorm:
		return Format::kBC1Srgb;
	case Format::kBC2Unorm:
		return Format::kBC2Srgb;
	case Format::kBC3Unorm:
		return Format::kBC3Srgb;
	case Format::kBC7Unorm:
		return Format::kBC7Srgb;
	default:
		return Format::kUnknown;
	}
}

render::legacy::CorePassTarget CVulkanContext::CorePassTargetFor( int target )
{
	using render::device::ResourceUsage;
	using render::device::TextureId;
	render::legacy::CorePassTarget out;
	const bool multisampled = m_activeSamples > 1;
	const VkExtent2D extent = multisampled ? m_msExtent : m_swapExtent;
	out.colorFormat = PortFormat( m_swapFormat );
	out.depthFormat = PortFormat( m_depthFormat );
	out.width = extent.width;
	out.height = extent.height;
	out.samples = multisampled ? static_cast<uint32_t>( m_activeSamples ) : 1u;
	out.device = Port();
	out.textures = &m_coreTextures;
	out.colorSrgbFormat = SrgbPortFormat( out.colorFormat );
	if ( m_hostDevice )
		out.submitted = { render::device::QueueKind::kGraphics, m_hostDevice->Port().Epoch(),
		    m_hostDevice->SubmittedValue() };
	out.frame = m_submitSerial + 1;
	out.streamEpoch = m_streamEpoch;
	if ( !m_hostDevice )
		return out;
	if ( target >= 0 )
	{
		if ( !IsRenderTargetTexture( target ) )
			return out;
		const ManagedTexture &texture = m_managedTextures[static_cast<std::size_t>( target )];
		out.width = texture.width;
		out.height = texture.height;
		out.samples = 1; // managed targets are single-sample, independently of the back buffer
		out.colorFormat = PortFormat( texture.format );
		out.colorSrgbFormat = SrgbPortFormat( out.colorFormat );
		out.color = ImportManagedTexture( target, false );
		if ( m_srgbAttachments && texture.srgbView != VK_NULL_HANDLE )
			out.colorSrgb = ImportManagedTexture( target, true );
		out.colorCopySource = true;
		// ImportManagedTexture owns the handle's color aliases and invalidates
		// them on image replacement; its paired depth shares that retirement.
		if ( m_coreTextureImports.size() < m_managedTextures.size() )
			m_coreTextureImports.resize( m_managedTextures.size() );
		CoreTextureImport &import = m_coreTextureImports[static_cast<std::size_t>( target )];
		if ( !import.depth.IsValid() && texture.depthImage != VK_NULL_HANDLE )
		{
			render::device::TextureDesc desc;
			desc.format = out.depthFormat;
			desc.width = out.width;
			desc.height = out.height;
			desc.usages = { ResourceUsage::kDepthWrite };
			desc.debugName = "managed render target depth";
			(void)m_hostDevice->ImportImage(
			    texture.depthImage, desc, ResourceUsage::kDepthWrite, &import.depth );
		}
		out.depth = import.depth;
		return out;
	}
	if ( target != -1 || m_acquiredImage >= m_swapImages.size() )
		return out;
	const auto import = [&]( VkImage image, render::device::Format format, ResourceUsage home,
	                        bool copySource, const char *name, TextureId *id )
	{
		if ( id->IsValid() || image == VK_NULL_HANDLE ||
		     format == render::device::Format::kUnknown )
			return;
		render::device::TextureDesc desc;
		desc.format = format;
		desc.width = extent.width;
		desc.height = extent.height;
		desc.sampleCount = out.samples;
		desc.usages = { home };
		if ( !multisampled )
			desc.usages.Add( ResourceUsage::kSampled );
		if ( copySource )
			desc.usages.Add( ResourceUsage::kCopySource );
		desc.debugName = name;
		if ( !m_hostDevice->ImportImage( image, desc, home, id ) )
			*id = TextureId();
	};
	if ( multisampled )
	{
		out.colorCopySource = true;
		import( m_msColor, out.colorFormat, ResourceUsage::kColorAttachment, true,
		    "multisampled back buffer", &m_coreMsColor );
		if ( m_srgbAttachments && m_msColorViewSrgb != VK_NULL_HANDLE )
			import( m_msColor, SrgbPortFormat( out.colorFormat ), ResourceUsage::kColorAttachment,
			    true, "multisampled back buffer (sRGB)", &m_coreMsColorSrgb );
		out.colorSrgb = m_coreMsColorSrgb;
		import( m_msDepth, out.depthFormat, ResourceUsage::kDepthWrite, false, "multisampled depth",
		    &m_coreMsDepth );
		out.color = m_coreMsColor;
		out.depth = m_coreMsDepth;
		return out;
	}
	m_coreColor.resize( m_swapImages.size() );
	m_coreColorSrgb.resize( m_swapImages.size() );
	m_coreDepth.resize( m_swapImages.size() );
	const uint32_t i = m_acquiredImage;
	// These are engine-owned scene back buffers, created with TRANSFER_SRC.
	// HDR swapchain readback support is a separate presentation capability.
	out.colorCopySource = true;
	import( m_swapImages[i], out.colorFormat, ResourceUsage::kColorAttachment, true, "back buffer",
	    &m_coreColor[i] );
	if ( m_srgbAttachments )
		import( m_swapImages[i], SrgbPortFormat( out.colorFormat ), ResourceUsage::kColorAttachment,
		    true, "back buffer (sRGB)", &m_coreColorSrgb[i] );
	out.colorSrgb = m_coreColorSrgb[i];
	if ( i < m_depthImages.size() )
		import( m_depthImages[i], out.depthFormat, ResourceUsage::kDepthWrite, false, "depth",
		    &m_coreDepth[i] );
	out.color = m_coreColor[i];
	out.depth = m_coreDepth[i];
	return out;
}

render::device::TextureId CVulkanContext::ImportManagedTexture( int handle, bool srgb )
{
	using render::device::ResourceUsage;
	if ( !m_hostDevice || handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
		return {};
	// Importing a deferred target allocates its storage first (its contents are
	// only ever consumed here after something has drawn into it).
	EnsureRenderTargetStorage( handle );
	const ManagedTexture &texture = m_managedTextures[static_cast<size_t>( handle )];
	// srgb asks for linear values: an 8-bit or BC image decodes through its
	// sRGB view; a 16-bit or float image already holds linear values.
	const render::device::Format stored = PortFormat( texture.format );
	const bool linearStorage = stored == render::device::Format::kRGBA16Unorm ||
	                           stored == render::device::Format::kRGBA16Float;
	if ( srgb && linearStorage )
		srgb = false;
	const render::device::Format format = srgb ? SrgbPortFormat( stored ) : stored;
	// A 2D image, or a cube map (six layers, sampled as a cube). A render
	// target (single-sample, 2D) rests in the sampled layout between the
	// passes that draw into it ("sampled-layout in/out"), as a managed
	// texture does, so it imports the same way: the core reads what the
	// stream drew into it before the slot (the water point's reflection).
	const bool cube = texture.layers == 6;
	if ( texture.image == VK_NULL_HANDLE || !texture.uploaded ||
	     ( texture.renderTarget && texture.layers != 1 ) || ( texture.layers != 1 && !cube ) ||
	     texture.depth > 1 || format == render::device::Format::kUnknown ||
	     ( srgb && texture.srgbView == VK_NULL_HANDLE ) )
		return {};
	if ( m_coreTextureImports.size() < m_managedTextures.size() )
		m_coreTextureImports.resize( m_managedTextures.size() );
	CoreTextureImport &import = m_coreTextureImports[static_cast<size_t>( handle )];
	if ( import.image != texture.image )
		ReleaseManagedImport( handle ); // the handle's image was replaced
	if ( import.id[srgb].IsValid() )
		return import.id[srgb];
	render::device::TextureDesc desc;
	desc.dimension =
	    cube ? render::device::TextureDimension::kCube : render::device::TextureDimension::k2D;
	desc.format = format;
	desc.width = texture.width;
	desc.height = texture.height;
	desc.depthOrLayers = cube ? 6 : 1;
	desc.mipLevels = texture.mipLevels;
	desc.usages = { ResourceUsage::kSampled };
	if ( texture.renderTarget )
	{
		desc.usages.Add( ResourceUsage::kColorAttachment );
		desc.usages.Add( ResourceUsage::kCopySource );
	}
	desc.debugName = texture.debugName.empty() ? "managed texture" : texture.debugName.c_str();
	render::device::TextureId id;
	if ( !m_hostDevice->ImportImage( texture.image, desc, ResourceUsage::kSampled, &id ) )
		return {};
	import.image = texture.image;
	import.id[srgb] = id;
	return id;
}

void CVulkanContext::ReleaseManagedImport( int handle )
{
	if ( handle < 0 || handle >= static_cast<int>( m_coreTextureImports.size() ) )
		return;
	CoreTextureImport &import = m_coreTextureImports[static_cast<size_t>( handle )];
	render::device::IRenderDevice2 *port = Port();
	for ( render::device::TextureId id : { import.id[0], import.id[1], import.depth } )
	{
		if ( id.IsValid() && port )
		{
			// Behind the frame being recorded, the last that can sample it.
			const render::device::CompletionToken after{ render::device::QueueKind::kGraphics,
			    port->Epoch(), m_hostDevice->SubmittedValue() + 1 };
			(void)port->Release( id, after );
		}
	}
	import = CoreTextureImport();
}

render::legacy::CoreOutputTargets CVulkanContext::CoreOutputTargetsFor()
{
	using render::device::ResourceUsage;
	using render::device::TextureId;
	render::legacy::CoreOutputTargets out;
	const uint32_t i = m_acquiredImage;
	if ( !m_hostDevice || i >= m_swapImages.size() || i >= m_presentImages.size() )
		return out;
	out.device = Port();
	out.sceneFormat = m_config.hdrScene ? PortFormat( m_swapFormat )
	                                    : SrgbPortFormat( PortFormat( m_swapFormat ) );
	out.sceneWidth = m_swapExtent.width;
	out.sceneHeight = m_swapExtent.height;
	out.targetFormat = PortFormat( m_presentFormat );
	out.width = m_presentExtent.width;
	out.height = m_presentExtent.height;
	out.submitted = { render::device::QueueKind::kGraphics, m_hostDevice->Port().Epoch(),
		m_hostDevice->SubmittedValue() };
	const auto import = [&]( VkImage image, render::device::Format format,
	                        render::device::UsageSet usages, uint32_t width, uint32_t height,
	                        const char *name, TextureId *id )
	{
		if ( id->IsValid() || image == VK_NULL_HANDLE ||
		     format == render::device::Format::kUnknown )
			return;
		render::device::TextureDesc desc;
		desc.format = format;
		desc.width = width;
		desc.height = height;
		desc.usages = usages;
		desc.debugName = name;
		if ( !m_hostDevice->ImportImage( image, desc, ResourceUsage::kColorAttachment, id ) )
			*id = TextureId();
	};
	m_outputScene.resize( m_swapImages.size() );
	m_outputTarget.resize( m_presentImages.size() );
	import( m_swapImages[i], out.sceneFormat,
	    { ResourceUsage::kColorAttachment, ResourceUsage::kSampled }, out.sceneWidth,
	    out.sceneHeight, "back buffer (output, sRGB)", &m_outputScene[i] );
	import( m_presentImages[i], out.targetFormat, { ResourceUsage::kColorAttachment }, out.width,
	    out.height, "swapchain image (output)", &m_outputTarget[i] );
	out.scene = m_outputScene[i];
	out.target = m_outputTarget[i];
	// The back buffer holds display values: the output is the clip and the
	// encoding for the display's headroom (scene peak 1).
	float potential = 1.0f;
	m_host->ReadHeadroom( &out.headroom, &potential );
	out.headroom = m_extendedOutput ? std::max( 1.0f, out.headroom ) : 1.0f;
	if ( m_config.hdrScene )
	{
		float peakNits = 0.0f;
		ResolvedHdrSettings( &out.exposure, &peakNits );
		out.scenePeak = 16.0f;
#if defined( __linux__ )
		// Linux scRGB uses 80 cd/m^2 per linear unit; scene white is 203.
		if ( m_presentColorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT )
			out.linearScale = 203.0f / 80.0f;
#endif
		if ( m_extendedOutput )
			out.headroom = peakNits / 203.0f;
	}
	return out;
}

void CVulkanContext::ResolvedHdrSettings(
    float *outExposure, float *outPeakNits, bool *outFromDisplay ) const
{
	const float white = m_host ? m_host->ReadSdrWhiteNits() : 0.0f;
	float headroom = 1.0f, potential = 1.0f;
	if ( m_host )
		m_host->ReadHeadroom( &headroom, &potential );
	const bool reported = headroom > 1.0f;
	*outExposure = m_hdrExposure > 0.0f ? m_hdrExposure
	               : white > 0.0f       ? std::clamp( white / 203.0f, 0.25f, 4.0f )
	                                    : 1.0f;
	*outPeakNits =
	    m_hdrPeakNits > 0.0f ? m_hdrPeakNits
	    : reported ? std::clamp( ( white > 0.0f ? white : 203.0f ) * headroom, 203.0f, 10000.0f )
	               : 1000.0f;
	if ( outFromDisplay )
		*outFromDisplay = m_hdrPeakNits <= 0.0f && reported;
}

void CVulkanContext::RecordOutputSection( render::device::CommandEncoder &encoder )
{
	m_outputSectionRecorded = false;
	if ( ( !m_extendedOutput && !m_config.hdrScene ) || !m_corePassRecorder )
		return;
	const render::legacy::CoreOutputTargets targets = CoreOutputTargetsFor();
	m_hostDevice->BeginSection( encoder );
	if ( targets.scene.IsValid() && targets.target.IsValid() )
		m_outputSectionRecorded = m_corePassRecorder->RecordOutput( encoder, targets );
	m_hostDevice->EndSection( encoder );
	if ( !m_outputSectionRecorded && !m_outputFallbackLogged )
	{
		// Visible: the 8-bit back buffer blitted into a linear swapchain
		// shows too bright.
		Log( "required output pass was not recorded; refusing HDR presentation\n" );
		m_outputFallbackLogged = true;
	}
}

void CVulkanContext::RecordPresentOutput(
    VkCommandBuffer cmd, uint32_t imageIndex, VkImageLayout backBufferLayout )
{
	ScopedDebugLabel label( m_debugUtils, "present: output" );
	VkImageMemoryBarrier pre[2] = {};
	uint32_t count = 0;
	// The back buffer rests in its home (color attachment) for the output's
	// section; a capture left it a transfer source.
	if ( backBufferLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL )
	{
		VkImageMemoryBarrier &b = pre[count++];
		b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		b.srcQueueFamilyIndex = b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		b.image = m_swapImages[imageIndex];
		b.oldLayout = backBufferLayout;
		b.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		b.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		b.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
	}
	// The acquired swapchain image: its old contents are not needed. The
	// source stage chains to the acquire semaphore's wait.
	{
		VkImageMemoryBarrier &b = pre[count++];
		b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		b.srcQueueFamilyIndex = b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		b.image = m_presentImages[imageIndex];
		b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		b.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		b.srcAccessMask = 0;
		b.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	vkCmdPipelineBarrier( cmd,
	    VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
	    nullptr, 0, nullptr, count, pre );
	m_hostDevice->RunSection( cmd, 0 );
	RecordPresentedCaptureAndRelease( cmd, imageIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, false );
}

void CVulkanContext::ReleaseOutputImports( bool swapchainToo )
{
	render::device::IRenderDevice2 *port = Port();
	// The images go now: nothing submitted uses them any more.
	const auto release = [&]( std::vector<render::device::TextureId> &ids )
	{
		for ( render::device::TextureId &id : ids )
			if ( id.IsValid() && port )
				(void)port->Release( id, render::device::CompletionToken() );
		ids.clear();
	};
	release( m_outputScene );
	if ( swapchainToo )
		release( m_outputTarget );
}

void CVulkanContext::ReleaseCorePassImports( bool msaaOnly )
{
	render::device::IRenderDevice2 *port = Port();
	// The images go now: nothing submitted uses them any more.
	const auto release = [&]( render::device::TextureId &id )
	{
		if ( id.IsValid() && port )
			(void)port->Release( id, render::device::CompletionToken() );
		id = render::device::TextureId();
	};
	release( m_coreMsColor );
	release( m_coreMsColorSrgb );
	release( m_coreMsDepth );
	if ( !msaaOnly )
	{
		for ( render::device::TextureId &id : m_coreColor )
			release( id );
		for ( render::device::TextureId &id : m_coreColorSrgb )
			release( id );
		m_coreColorSrgb.clear();
		for ( render::device::TextureId &id : m_coreDepth )
			release( id );
		m_coreColor.clear();
		m_coreDepth.clear();
	}
	if ( port )
		(void)port->Poll();
}

bool CVulkanContext::RenderFrame( bool *outSkip, std::string *outError )
{
	if ( !PrepareFrame( outSkip, outError ) )
		return false;
	if ( outSkip && *outSkip )
		return true;
	// The same work AttachFrameStage adds to the graph's passes, on an encoder
	// of its own.
	render::device::CompletionToken token;
	const bool submitted = m_hostDevice->SubmitNative(
	    []( void *user, VkCommandBuffer cmd )
	    {
		    static_cast<CVulkanContext *>( user )->RecordFrameCommands( cmd );
	    },
	    this, m_imageAvailable[m_currentFrame],
	    VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
	    m_renderFinished[m_acquiredImage], &token );
	return FinishFrame( token, submitted, outError );
}

void CVulkanContext::RecordFrameCommands( VkCommandBuffer cmd )
{
	for ( int stage = 0; stage < kFrameStageCount; ++stage )
	{
		if ( HasFrameStage( static_cast<FrameStage>( stage ) ) )
			RecordFrameStage( static_cast<FrameStage>( stage ), cmd );
	}
}

void CVulkanContext::RecordFrameStage( FrameStage stage, VkCommandBuffer cmd )
{
	// Labels the stream opens are closed at the end of their stage, so they
	// nest inside the graph pass's own label.
	m_debugUtils.BeginFrameCommands( cmd );
	switch ( stage )
	{
	case kFrameStageComputeAndUploads:
		RecordFrameComputeAndUploads( cmd );
		break;
	case kFrameStageScene:
		RecordFrameScene( cmd );
		break;
	case kFrameStageResolve:
		// Everything after this reads the single-sampled back buffer.
		ResolveBackBuffer( cmd, m_acquiredImage );
		break;
	case kFrameStageCapture:
		RecordCapture( cmd, m_acquiredImage );
		break;
	case kFrameStagePresent:
		RecordFramePresent( cmd );
		break;
	case kFrameStageCount:
		break;
	}
	m_debugUtils.EndFrameCommands();
}

void CVulkanContext::RecordFrameComputeAndUploads( VkCommandBuffer cmd )
{
	m_recordBeginUs = FrameClockMicros();
	if ( m_timestampPool != VK_NULL_HANDLE )
	{
		const uint32_t first = m_currentFrame * kTimestampsPerFrame;
		vkCmdResetQueryPool( cmd, m_timestampPool, first, kTimestampsPerFrame );
		vkCmdWriteTimestamp( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_timestampPool, first );
	}
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
	{
		m_gpuTimerLabels[m_currentFrame].clear();
		vkCmdResetQueryPool(
		    cmd, m_gpuTimerPool, m_currentFrame * kMaxGpuTimerMarks, kMaxGpuTimerMarks );
		GpuTimerMark( cmd, "frame start: compute, uploads" );
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
}

void CVulkanContext::RecordFrameScene( VkCommandBuffer cmd )
{
	const uint32_t imageIndex = m_acquiredImage;
	// Queries are reset outside any render pass, before the stream that issues
	// them replays. Only the slots this stream issues are reset, so a result of
	// an earlier frame that the engine has yet to read survives.
	m_replayedQueries.clear();
	m_lastFrameSceneCaptures = 0;
	m_lastFrameSceneDepthCaptures = 0;
	m_lastFrameDepthToAlpha = 0;
	m_lastFrameDepthToAlphaSkipped = 0;
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
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
		GpuTimerMark( cmd, GpuTimerTargetLabel( "pass (cleared)", -1, firstPassSrgb ) );
	vkCmdBeginRenderPass( cmd, &rp, VK_SUBPASS_CONTENTS_INLINE );
	m_frameCost.Add( kCostRenderPass, 0 );

	// Dynamic geometry queued by the material system's mesh interface this frame.
	m_statsLegacyStreamDraws = 0;
	m_statsLegacyProgramDraws = 0;
	if ( !m_dynDrawRecords.empty() )
	{
		// Replay the stream's copies, clears, queries and core-pass sections in
		// engine order. Each record names the target it was issued against; a
		// change of target closes the open pass and opens the new target's.
		int openTarget = -1; // the swapchain pass opened above
		bool openSrgb = firstPassSrgb; // entered through the target's sRGB view
		// The index of the next slot record's section in the scene record.
		uint32_t coreSection = 0;
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
					     n.kind == kRecordSceneCapture || n.kind == kRecordCorePass )
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
		// Records before these indices may read the back buffer's depth or its
		// stencil; a back-buffer pass that begins at or after one neither loads
		// nor stores that plane (BeginTargetPass keep).
		size_t depthEnd = 0, stencilEnd = 0;
		for ( size_t i = 0; i < m_dynDrawRecords.size(); ++i )
		{
			const int use = RecordBackBufferDepthReads( m_dynDrawRecords[i] );
			if ( use & kKeepDepth )
				depthEnd = i + 1;
			if ( use & kKeepStencil )
				stencilEnd = i + 1;
		}
		m_statsDepthEnd[0] = depthEnd;
		m_statsDepthEnd[1] = stencilEnd;
		m_statsDepthEnd[2] = m_dynDrawRecords.size();
		const auto keepFrom = [&]( size_t record )
		{
			return ( record < depthEnd ? kKeepDepth : 0 ) |
			       ( record < stencilEnd ? kKeepStencil : 0 );
		};
		size_t labelCursor = 0;
		// RFC 0014 core plumbing (render/legacy/core_passes.h kCorePassLegacyOff):
		// from a slot so tagged, the core alone draws the frame.
		bool legacyOff = false;
		bool legacyHud = false;
		m_frameLegacyOff = false;
		for ( size_t recordIndex = 0; recordIndex < m_dynDrawRecords.size(); ++recordIndex )
		{
			ReplayFrameLabels( &labelCursor, recordIndex );
			DynDraw &d = m_dynDrawRecords[recordIndex];
			if ( d.kind == kRecordCorePass && d.corePass == render::legacy::kCorePassLegacyHud )
				legacyHud = true;
			if ( d.kind == kRecordCorePass && ( d.corePass & render::legacy::kCorePassForwarded ) &&
			     ( d.corePass & render::legacy::kCorePassLegacyOff ) )
				legacyOff = m_frameLegacyOff = true;
			if ( SkipLegacyRecord( d, legacyOff, legacyHud ) )
				continue;
			if ( d.kind == kRecordCorePass )
			{
				// Outside any pass, like a copy: the slot's section runs here,
				// and the next record opens a pass that loads what it drew.
				const uint32_t section = coreSection++;
				if ( !m_corePassRecorder ||
				     ( d.target >= 0 && !IsRenderTargetTexture( d.target ) ) )
					continue;
				endActiveQuery( false );
				if ( passOpen )
					vkCmdEndRenderPass( cmd );
				passOpen = false;
				lastCopy = nullptr;
				if ( m_hostDevice->RunSection( cmd, section ) && m_corePassesRun++ == 0 )
					Log( "core passes: the first slot's section ran in the scene replay\n" );
				continue;
			}
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
				const bool repeat =
				    m_passMerging && lastCopy && lastCopy->copyDst == d.copyDst &&
				    lastCopy->target == d.target &&
				    lastCopy->copyDepthToAlpha == d.copyDepthToAlpha &&
				    ( !d.copyDepthToAlpha ||
				        ( std::memcmp( lastCopy->copyDepth.projection, d.copyDepth.projection,
				              sizeof( d.copyDepth.projection ) ) == 0 &&
				            lastCopy->copyDepth.invRange == d.copyDepth.invRange ) ) &&
				    std::memcmp( lastCopy->copySrcRect, d.copySrcRect, sizeof( d.copySrcRect ) ) ==
				        0 &&
				    std::memcmp( lastCopy->copyDstRect, d.copyDstRect, sizeof( d.copyDstRect ) ) ==
				        0;
				if ( !repeat )
				{
					if ( openTarget == -1 && m_activeSamples > 1 )
						ResolveBackBuffer( cmd, m_acquiredImage );
					RecordTargetCopy( cmd, openTarget, d );
					if ( d.copyDepthToAlpha )
						RecordDepthToAlpha( cmd, openTarget, d );
				}
				lastCopy = &d;
				if ( !m_passMerging )
				{
					BeginTargetPass( cmd, openTarget, openSrgb, keepFrom( recordIndex + 1 ) );
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
				RecordSceneCapture( cmd, openTarget );
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
				BeginTargetPass( cmd, openTarget, openSrgb, keepFrom( recordIndex ) );
				passOpen = true;
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
				if ( d.clearColor && ( !legacyOff || legacyHud ) )
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
			// Only the present follows: no depth or stencil use remains.
			BeginTargetPass( cmd, -1, false, kKeepNone );
		}
	}
	// NOTE: the queue is NOT cleared here. It is cleared at frame start
	// (ClearBuffers) so the last frame's geometry stays available for an
	// on-demand screenshot capture (IShaderAPI::ReadPixels re-renders it).

	vkCmdEndRenderPass( cmd );
}

void CVulkanContext::RecordFramePresent( VkCommandBuffer cmd )
{
	const uint32_t imageIndex = m_acquiredImage;
	const bool capturePresented = m_frameCapturePresented;
	const bool captureBackBuffer = m_frameCapture && !m_frameCapturePresented && !m_config.hdrScene;
	const VkImageLayout backBufferLayout = captureBackBuffer
	                                           ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	                                           : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	if ( m_timestampPool != VK_NULL_HANDLE )
		vkCmdWriteTimestamp( cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_timestampPool,
		    m_currentFrame * kTimestampsPerFrame + 1 );
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
		GpuTimerMark( cmd, "present (waits for the swapchain image)" );
	ApplyPublishedGammaRamp();
	// An extended-linear swapchain takes the frame through the core's output
	// (RFC 0016 "Output"), which the monitor gamma ramp does not apply to.
	const char *path = "blit";
	if ( m_outputSectionRecorded )
	{
		RecordPresentOutput( cmd, imageIndex, backBufferLayout );
		path = "output";
	}
	else if ( m_config.hdrScene )
		return;
	// RFC 0014: a frame the core alone draws presents without the ramp.
	else if ( m_gammaActive && !m_frameLegacyOff &&
	          RecordPresentGamma( cmd, imageIndex, backBufferLayout, capturePresented ) )
		path = "gamma";
	else
		RecordPresentBlit( cmd, imageIndex, backBufferLayout, capturePresented );
	if ( path != m_presentPath )
	{
		Log( "present path: %s\n", path );
		m_presentPath = path;
	}
	m_captureRequested = m_capturePresented = m_captureHdrRequested = false;

	if ( m_computeFlush )
		m_computeFlush( cmd, m_submitSerial + 1 );
	if ( m_timestampPool != VK_NULL_HANDLE )
		vkCmdWriteTimestamp( cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_timestampPool,
		    m_currentFrame * kTimestampsPerFrame + 2 );
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
		GpuTimerMark( cmd, "end" );
	m_recordEndUs = FrameClockMicros();
	m_frameCost.Add( kCostRecord, m_recordEndUs - m_recordBeginUs );
}

bool CVulkanContext::RecordCapture( VkCommandBuffer cmd, uint32_t imageIndex )
{
	if ( m_config.hdrScene && !m_frameCaptureHdr )
	{
		if ( !m_captureOutputRecorded )
			return false;
		VkImageMemoryBarrier initialize = {};
		initialize.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		initialize.srcQueueFamilyIndex = initialize.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		initialize.image = m_captureOutputImage;
		initialize.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		initialize.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		initialize.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		initialize.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1,
		    &initialize );
		m_hostDevice->RunSection( cmd, 0 );
	}
	const VkImage source =
	    m_config.hdrScene && !m_frameCaptureHdr ? m_captureOutputImage : m_swapImages[imageIndex];
	ScopedDebugLabel label( m_debugUtils, "capture back buffer" );
	// Copy the just-rendered color image (currently COLOR_ATTACHMENT_OPTIMAL)
	// into the host-visible linear capture image.
	VkImageMemoryBarrier toSrc = {};
	toSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toSrc.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	toSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	toSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toSrc.image = source;
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
	vkCmdCopyImage( cmd, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_captureImage,
	    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy );

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
	if ( m_frameCaptureHdr )
	{
		// The output pass still samples the scene in its home layout this frame.
		VkImageMemoryBarrier restore = toSrc;
		restore.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		restore.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		restore.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		restore.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &restore );
	}
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
	NotePresent( !sameSize );
	vkCmdBlitImage( cmd, m_swapImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	    m_presentImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
	    sameSize ? VK_FILTER_NEAREST : m_presentFilter );

	RecordPresentedCaptureAndRelease( cmd, imageIndex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, capture );
}

// A suboptimal acquire or present with the surface unchanged (a changed one
// is rebuilt by its own path) asks for one rebuild before the next frame. The
// next clean acquire re-arms it, so a surface that stays suboptimal (a rotated
// display the compositor adapts) rebuilds once instead of every frame.
void CVulkanContext::NoteSuboptimal()
{
	if ( !m_suboptimalRebuilt && !SurfaceChangedSinceSwapchain() )
		m_suboptimalPending = true;
}

void CVulkanContext::NotePresent( bool scaled )
{
	++m_presentCount;
	if ( !scaled )
	{
		m_scaledPresentRun = 0;
		return;
	}
	++m_scaledPresentCount;
	++m_scaledPresentRun;
	m_longestScaledPresentRun = std::max( m_longestScaledPresentRun, m_scaledPresentRun );
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
	                             VkImage *outImage, VulkanMemory *outMemory ) -> bool
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
		return CreateImage( img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outImage, outMemory,
		    "multisample target", nullptr );
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
	    createImage( m_depthFormat,
	        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
	            ( m_sceneDepthUsable ? VkImageUsageFlags( VK_IMAGE_USAGE_SAMPLED_BIT ) : 0u ),
	        false, &m_msDepth, &m_msDepthMemory ) &&
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
	ReleaseCorePassImports( true );
	for ( VkFramebuffer *fb : { &m_msFramebuffer, &m_msFramebufferSrgb } )
	{
		if ( *fb != VK_NULL_HANDLE )
			vkDestroyFramebuffer( m_device, *fb, nullptr );
		*fb = VK_NULL_HANDLE;
	}
	if ( m_msDepthSampleSet != VK_NULL_HANDLE && m_dynTexDescPool != VK_NULL_HANDLE )
	{
		vkFreeDescriptorSets( m_device, m_dynTexDescPool, 1, &m_msDepthSampleSet );
		--m_liveTextureSets;
	}
	m_msDepthSampleSet = VK_NULL_HANDLE;
	for ( VkImageView *view :
	    { &m_msColorView, &m_msColorViewSrgb, &m_msDepthView, &m_msDepthSampleView } )
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
	for ( VulkanMemory *memory : { &m_msColorMemory, &m_msDepthMemory } )
	{
		if ( *memory != VK_NULL_HANDLE )
			FreeMemory( *memory );
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
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
		GpuTimerMark( cmd, "MSAA resolve" );
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
	if ( m_gammaPipeline != VK_NULL_HANDLE && m_gammaFormat == m_presentFormat )
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
	color.format = m_presentFormat;
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
		     MapMemory( m_gammaRampMemories[slot], &mapped ) != VK_SUCCESS )
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
	m_gammaFormat = m_presentFormat;
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
		iv.format = m_presentFormat;
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
			FreeMemory( m_gammaRampMemories[slot] );
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

	NotePresent( !sameSize );
	++m_gammaPresentCount;
	RecordPresentedCaptureAndRelease( cmd, imageIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	    capture );
	return true;
}

bool CVulkanContext::FinishFrame(
    const render::device::CompletionToken &token, bool submitted, std::string *outError )
{
	if ( !m_frameOpen )
	{
		SetError( outError, "FinishFrame called without an open frame" );
		return false;
	}
	const uint32_t imageIndex = m_acquiredImage;
	const bool doCapture = m_frameCapture;
	m_frameCost.Add( kCostSubmit, FrameClockMicros() - m_recordEndUs );
	if ( m_corePassRecorder )
		m_corePassRecorder->FrameSubmitted( token, submitted );
	if ( !submitted )
	{
		// No render submission consumed the acquire signal. Drain it before
		// reusing this binary semaphore, and retire the unpresented swapchain
		// image. Acquiring again directly would reuse a signalled semaphore.
		m_frameOpen = false;
		m_captureRequested = m_capturePresented = m_captureHdrRequested = false;
		if ( Port()->State() == render::device::DeviceState::kAvailable )
		{
			const VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			VkSubmitInfo drain = {};
			drain.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			drain.waitSemaphoreCount = 1;
			drain.pWaitSemaphores = &m_imageAvailable[m_currentFrame];
			drain.pWaitDstStageMask = &stage;
			if ( vkQueueSubmit( m_graphicsQueue, 1, &drain, VK_NULL_HANDLE ) != VK_SUCCESS ||
			     !RecreateSwapchain( outError ) )
				return false;
		}
		SetError( outError, "the frame's submission failed" );
		return false;
	}
	if ( m_config.hdrScene &&
	     ( !m_outputSectionRecorded || ( doCapture && !m_frameCapturePresented &&
	                                       !m_frameCaptureHdr && !m_captureOutputRecorded ) ) )
	{
		vkDeviceWaitIdle( m_device );
		m_frameOpen = false;
		SetError( outError, "required HDR scene output or SDR capture pass failed" );
		return false;
	}
	const uint64_t value = token.value;
	m_slotSerial[m_currentFrame] = ++m_submitSerial;
	m_slotValue[m_currentFrame] = value;
	m_imageValue[imageIndex] = value;
	m_serialValues.emplace_back( m_submitSerial, value );
	m_slotStatsFrame[m_currentFrame] = ++m_statsFrame;
	VkResult r = VK_SUCCESS;
	// The queries this frame replayed now have results on their way.
	for ( const std::pair<int, uint64_t> &issue : m_replayedQueries )
		m_querySlots[static_cast<size_t>( issue.first )].submitted = issue.second;
	m_replayedQueries.clear();

	// If we captured, resolve the pixels now: the submission must complete
	// before the host-visible copy is valid to read.
	if ( doCapture )
	{
		m_hostDevice->WaitValue( value, UINT64_MAX );
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
	{
		// Presented; the window system prefers another swapchain.
		NoteSuboptimal();
		return true;
	}
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
	// The render core's pipelines (the adapter's own creations) share the
	// store's cache, so they persist with it and later launches hit it.
	if ( m_hostDevice )
		m_hostDevice->SetPipelineCache( m_pipelineCache );
	Log( "pipeline store %s: %zu bytes of cache data\n", directory.c_str(),
	    haveData ? data.size() : static_cast<size_t>( 0 ) );
	return true;
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
	info.queryCount = kMaxFramesInFlight * kTimestampsPerFrame;
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
	const auto micros = [this]( uint64_t from, uint64_t to )
	{
		return static_cast<uint64_t>(
		    double( ( to - from ) & m_timestampMask ) * m_timestampPeriodNs / 1000.0 );
	};
	uint64_t stamps[kTimestampsPerFrame] = {};
	if ( vkGetQueryPoolResults( m_device, m_timestampPool, slot * kTimestampsPerFrame,
	         kTimestampsPerFrame, sizeof( stamps ), stamps, sizeof( uint64_t ),
	         VK_QUERY_RESULT_64_BIT ) == VK_SUCCESS )
	{
		m_gpuResultFrame = m_slotStatsFrame[slot];
		m_gpuRenderUs = micros( stamps[0], stamps[1] );
		m_gpuResultUs = micros( stamps[0], stamps[2] );
	}
	m_gpuTimerResult.clear();
	m_gpuTimerSequence.clear();
	const std::vector<std::string> &labels = m_gpuTimerLabels[slot];
	std::vector<uint64_t> marks( labels.size() );
	if ( m_gpuTimerPool != VK_NULL_HANDLE && marks.size() > 1 &&
	     vkGetQueryPoolResults( m_device, m_gpuTimerPool, slot * kMaxGpuTimerMarks,
	         static_cast<uint32_t>( marks.size() ), marks.size() * sizeof( uint64_t ), marks.data(),
	         sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT ) == VK_SUCCESS )
	{
		// Each segment runs from its mark to the next; totals per label, in
		// the order the labels first appear.
		for ( size_t i = 0; i + 1 < marks.size(); ++i )
		{
			const uint64_t us = micros( marks[i], marks[i + 1] );
			if ( m_slotStatsFrame[slot] % 120 == 0 )
			{
				char entry[16];
				std::snprintf(
				    entry, sizeof( entry ), ",%llu]", static_cast<unsigned long long>( us ) );
				m_gpuTimerSequence +=
				    ( m_gpuTimerSequence.empty() ? "[\"" : ",[\"" ) + labels[i] + "\"" + entry;
			}
			size_t at = 0;
			while ( at < m_gpuTimerResult.size() && m_gpuTimerResult[at].label != labels[i] )
				++at;
			if ( at == m_gpuTimerResult.size() )
				m_gpuTimerResult.push_back( { labels[i], 0, 0 } );
			++m_gpuTimerResult[at].count;
			m_gpuTimerResult[at].us += us;
		}
	}
	m_gpuTimerLabels[slot].clear();
	m_slotStatsFrame[slot] = 0;
}

bool CVulkanContext::EnableGpuTimers( std::string *outError )
{
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
		return true;
	// The frame timestamps' pool exists only where the queue has timestamps.
	if ( m_timestampPool == VK_NULL_HANDLE )
	{
		SetError( outError, "the graphics queue has no timestamps" );
		return false;
	}
	VkQueryPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	info.queryType = VK_QUERY_TYPE_TIMESTAMP;
	info.queryCount = kMaxFramesInFlight * kMaxGpuTimerMarks;
	if ( vkCreateQueryPool( m_device, &info, nullptr, &m_gpuTimerPool ) != VK_SUCCESS )
	{
		m_gpuTimerPool = VK_NULL_HANDLE;
		SetError( outError, "vkCreateQueryPool (GPU timers) failed" );
		return false;
	}
	return true;
}

void CVulkanContext::GpuTimerMark( VkCommandBuffer cmd, std::string label )
{
	// A frame with more boundaries than marks times its tail as one segment.
	std::vector<std::string> &labels = m_gpuTimerLabels[m_currentFrame];
	if ( labels.size() >= kMaxGpuTimerMarks )
		return;
	for ( char &c : label )
	{
		if ( c == '"' || c == '\\' || static_cast<unsigned char>( c ) < 0x20 )
			c = '_';
	}
	vkCmdWriteTimestamp( cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_gpuTimerPool,
	    m_currentFrame * kMaxGpuTimerMarks + static_cast<uint32_t>( labels.size() ) );
	labels.push_back( std::move( label ) );
}

std::string CVulkanContext::GpuTimerTargetLabel( const char *kind, int target, bool srgb ) const
{
	uint32_t width = 0, height = 0;
	GetTargetExtent( target, &width, &height );
	const std::string *name = IsRenderTargetTexture( target )
	                              ? &m_managedTextures[static_cast<size_t>( target )].debugName
	                              : nullptr;
	char label[192];
	std::snprintf( label, sizeof( label ), "%s %s %ux%u%s", kind,
	    !name           ? "back buffer"
	    : name->empty() ? "render target"
	                    : name->c_str(),
	    width, height, srgb ? " sRGB" : "" );
	return label;
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

void CVulkanContext::SetRenderDocFrames( std::vector<uint64_t> frames, bool ( *trigger )() )
{
	m_renderDocFrames = std::move( frames );
	m_renderDocTrigger = trigger;
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
		    "\"backend\":%llu,\"records\":%zu,\"legacy_stream_draws\":%zu,"
		    "\"legacy_program_draws\":%zu,\"vertex_bytes\":%zu,\"index_bytes\":%zu,"
		    "\"upload_bytes\":%llu,\"extent\":[%u,%u]",
		    static_cast<unsigned long long>( m_statsFrame ),
		    static_cast<unsigned long long>( m_frameBeginUs ),
		    static_cast<unsigned long long>(
		        m_prevFrameBeginUs ? m_frameBeginUs - m_prevFrameBeginUs : 0 ),
		    static_cast<unsigned long long>(
		        m_prevFrameBeginCpuUs ? m_frameBeginCpuUs - m_prevFrameBeginCpuUs : 0 ),
		    static_cast<unsigned long long>(
		        m_prevFrameEndUs ? m_frameBeginUs - m_prevFrameEndUs : 0 ),
		    static_cast<unsigned long long>( endUs - m_frameBeginUs ), m_dynDrawRecords.size(),
		    m_statsLegacyStreamDraws, m_statsLegacyProgramDraws,
		    m_dynQueued.size() * sizeof( float ), m_dynIndices.size() * sizeof( uint32_t ),
		    static_cast<unsigned long long>( m_frameCost.uploadBytes ), m_swapExtent.width,
		    m_swapExtent.height );
		std::fprintf( m_frameStatsFile, ",\"opaque_batch\":[%llu,%llu,%llu]",
		    static_cast<unsigned long long>( m_opaqueCandidates ),
		    static_cast<unsigned long long>( m_opaqueBatches ),
		    static_cast<unsigned long long>( m_opaqueFollowers ) );
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
			std::fprintf( m_frameStatsFile, ",\"gpu\":[%llu,%llu,%llu]",
			    static_cast<unsigned long long>( m_gpuResultFrame ),
			    static_cast<unsigned long long>( m_gpuResultUs ),
			    static_cast<unsigned long long>( m_gpuRenderUs ) );
		if ( m_gpuTimerPool != VK_NULL_HANDLE )
			std::fprintf( m_frameStatsFile, ",\"depth_end\":[%zu,%zu,%zu]", m_statsDepthEnd[0],
			    m_statsDepthEnd[1], m_statsDepthEnd[2] );
		if ( m_gpuResultFrame && !m_gpuTimerResult.empty() )
		{
			// Labels hold only JSON-safe characters (GpuTimerMark).
			std::fputs( ",\"gpu_passes\":[", m_frameStatsFile );
			for ( size_t i = 0; i < m_gpuTimerResult.size(); ++i )
				std::fprintf( m_frameStatsFile, "%s[\"%s\",%u,%llu]", i ? "," : "",
				    m_gpuTimerResult[i].label.c_str(), m_gpuTimerResult[i].count,
				    static_cast<unsigned long long>( m_gpuTimerResult[i].us ) );
			std::fputc( ']', m_frameStatsFile );
		}
		if ( m_gpuResultFrame && !m_gpuTimerSequence.empty() )
			std::fprintf( m_frameStatsFile, ",\"gpu_sequence\":[%s]", m_gpuTimerSequence.c_str() );
		// The present mode, whenever it differs from the last one recorded.
		if ( static_cast<int>( m_presentMode ) != m_statsPresentMode )
		{
			m_statsPresentMode = static_cast<int>( m_presentMode );
			std::fprintf( m_frameStatsFile, ",\"present_mode\":%d", m_statsPresentMode );
		}
		if ( !m_frameMarks.empty() )
			std::fprintf( m_frameStatsFile, ",\"mark\":\"%s\"", m_frameMarks.c_str() );
		if ( m_renderDocArmed )
			std::fputs( ",\"renderdoc_capture\":true", m_frameStatsFile );
		std::fputs( "}\n", m_frameStatsFile );
		// Bounded loss if the process is killed rather than shut down.
		if ( m_statsFrame % 60 == 0 )
			std::fflush( m_frameStatsFile );
	}
	m_gpuResultFrame = 0;
	m_frameMarks.clear();
	// This frame is presented: arm RenderDoc for the next one if it is listed.
	m_renderDocArmed = false;
	while ( !m_renderDocFrames.empty() && m_renderDocFrames.front() <= m_statsFrame )
		m_renderDocFrames.erase( m_renderDocFrames.begin() );
	if ( !m_renderDocFrames.empty() && m_renderDocFrames.front() == m_statsFrame + 1 )
	{
		m_renderDocFrames.erase( m_renderDocFrames.begin() );
		m_renderDocArmed = m_renderDocTrigger && m_renderDocTrigger();
	}
	m_lastFrameCost = m_frameCost;
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
	VkResult r = MapMemory( m_captureMemory, &mapped );
	if ( r != VK_SUCCESS )
	{
		SetError( outError, std::string( "vkMapMemory (capture) failed: " ) + ResultString( r ) );
		return false;
	}

	m_hostDevice->Invalidate( m_captureMemory, 0, VK_WHOLE_SIZE );

	const int w = static_cast<int>( m_captureExtent.width );
	const int h = static_cast<int>( m_captureExtent.height );
	if ( m_frameCaptureHdr )
	{
		m_capturedHdrPixels.resize( static_cast<size_t>( w ) * h * 4 );
		const uint8_t *base = static_cast<const uint8_t *>( mapped ) + layout.offset;
		for ( int y = 0; y < h; ++y )
			std::memcpy( m_capturedHdrPixels.data() + static_cast<size_t>( y ) * w * 4,
			    base + static_cast<size_t>( y ) * layout.rowPitch, static_cast<size_t>( w ) * 8 );
		UnmapMemory( m_captureMemory );
		m_capturedWidth = w;
		m_capturedHeight = h;
		return true;
	}
	m_capturedPixels.assign( static_cast<size_t>( w ) * h * 4, 0 );

	const bool bgra = !m_config.hdrScene && ( m_swapFormat == VK_FORMAT_B8G8R8A8_UNORM ||
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
	UnmapMemory( m_captureMemory );

	m_capturedWidth = w;
	m_capturedHeight = h;
	return true;
}

void CVulkanContext::DestroySwapchainObjects()
{
	ReleaseOutputImports( true );
	DestroyPresentGammaTargets();
	DestroyBackBuffers();
	m_presentImages.clear();
	m_imageValue.clear();

	if ( m_swapchain != VK_NULL_HANDLE )
	{
		vkDestroySwapchainKHR( m_device, m_swapchain, nullptr );
		m_swapchain = VK_NULL_HANDLE;
	}
}

void CVulkanContext::DestroyBackBuffers()
{
	ReleaseCorePassImports( false );
	ReleaseOutputImports( false );
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
	for ( VulkanMemory mem : m_depthMemories )
		if ( mem != VK_NULL_HANDLE )
			FreeMemory( mem );
	m_depthMemories.clear();

	for ( VkImage img : m_swapImages )
		if ( img != VK_NULL_HANDLE )
			vkDestroyImage( m_device, img, nullptr );
	m_swapImages.clear();
	for ( VulkanMemory mem : m_backBufferMemories )
		if ( mem != VK_NULL_HANDLE )
			FreeMemory( mem );
	m_backBufferMemories.clear();
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

// A new video mode keeps the swapchain: only the back buffers (and their depth
// and framebuffers) take the new size. Recreating the swapchain as well cost a
// vsync on X11 (vkCreateSwapchainKHR), on the thread that owns the device, for
// every resize, and the window's own change already rebuilds it once.
bool CVulkanContext::RecreateBackBuffers( std::string *outError )
{
	vkDeviceWaitIdle( m_device );
	m_completedSerial = m_submitSerial;
	RetireCompletedTextures();
	DestroyBackBuffers();
	return CreateBackBuffers( outError ) && CreateFramebuffers( outError );
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
	if ( m_swapchain != VK_NULL_HANDLE )
		return RecreateBackBuffers( outError );
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
	Log( "opaque batching: %llu batches, %llu follower slots\n",
	    static_cast<unsigned long long>( m_opaqueBatches ),
	    static_cast<unsigned long long>( m_opaqueFollowers ) );
	if ( m_corePassRecorder || m_corePassesRun )
		Log( "core passes: %llu slot sections ran\n",
		    static_cast<unsigned long long>( m_corePassesRun ) );
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
	// Core passes release what they made on this device before it goes.
	if ( m_corePassRecorder && m_hostDevice )
		m_corePassRecorder->ReleaseDevice( m_hostDevice->Port() );

	if ( m_captureOutput.IsValid() && Port() )
		(void)Port()->Release( m_captureOutput, render::device::CompletionToken() );
	m_captureOutput = {};
	if ( m_captureOutputImage != VK_NULL_HANDLE )
		vkDestroyImage( m_device, m_captureOutputImage, nullptr );
	m_captureOutputImage = VK_NULL_HANDLE;
	if ( m_captureOutputMemory != VK_NULL_HANDLE )
		FreeMemory( m_captureOutputMemory );
	m_captureOutputMemory = VK_NULL_HANDLE;
	if ( m_captureImage != VK_NULL_HANDLE )
	{
		vkDestroyImage( m_device, m_captureImage, nullptr );
		m_captureImage = VK_NULL_HANDLE;
	}
	if ( m_captureMemory != VK_NULL_HANDLE )
	{
		FreeMemory( m_captureMemory );
		m_captureMemory = VK_NULL_HANDLE;
	}
	m_captureExtent = { 0, 0 };
	m_captureFormat = VK_FORMAT_UNDEFINED;

	if ( m_device != VK_NULL_HANDLE )
	{
		DestroyStreamBuffer( m_worldIndexBuffer );
		DestroyStreamBuffer( m_worldVertexBuffer );
		m_worldVertexCount = 0;
		m_worldIndexCount = 0;
		++m_worldMeshRevision;
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
		if ( m_gpuTimerPool != VK_NULL_HANDLE )
		{
			vkDestroyQueryPool( m_device, m_gpuTimerPool, nullptr );
			m_gpuTimerPool = VK_NULL_HANDLE;
		}
		if ( m_pipelineCache != VK_NULL_HANDLE )
		{
			if ( m_hostDevice )
				m_hostDevice->SetPipelineCache( VK_NULL_HANDLE );
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
		m_imageAvailable.clear();
		m_renderFinished.clear();
		for ( uint64_t &value : m_slotValue )
			value = 0;
		m_serialValues.clear();
		// The pool below frees them.
		m_oneTimeCommands.clear();

		if ( m_commandPool != VK_NULL_HANDLE )
		{
			vkDestroyCommandPool( m_device, m_commandPool, nullptr );
			m_commandPool = VK_NULL_HANDLE;
		}

		for ( VkRenderPass *pass : { &m_renderPass, &m_renderPassLoad, &m_renderPassTarget,
		          &m_renderPassLoadSrgb, &m_renderPassTargetSrgb, &m_renderPassClearSrgb } )
		{
			if ( *pass != VK_NULL_HANDLE )
			{
				vkDestroyRenderPass( m_device, *pass, nullptr );
				*pass = VK_NULL_HANDLE;
			}
		}
		for ( auto &byView : m_renderPassLoadKeep )
		{
			for ( VkRenderPass &pass : byView )
			{
				if ( pass != VK_NULL_HANDLE )
					vkDestroyRenderPass( m_device, pass, nullptr );
				pass = VK_NULL_HANDLE;
			}
		}

		m_debugUtils.Reset();
		m_shaderLibrary = VulkanShaderLibrary();
		// Every release the adapter holds for this context is due (the
		// device is idle), and runs while this context's state is intact.
		m_hostDevice->Collect();
		if ( m_hostDevice->LiveAllocations() > m_adapterAllocations )
			Log( "shutdown: %llu allocations still live\n",
			    static_cast<unsigned long long>(
			        m_hostDevice->LiveAllocations() - m_adapterAllocations ) );
	}

	if ( m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE )
	{
		m_host->DestroySurface( m_instance, m_surface );
		m_surface = VK_NULL_HANDLE;
	}
	// The owner destroys the device, its allocator and the instance after
	// every borrower shut down: the root (ReleaseHostDevice), or this context
	// when it had the owner create the device.
	m_hostDevice = nullptr;
	if ( m_releaseDevice && m_config.deviceOwner )
		m_config.deviceOwner->Release();
	m_releaseDevice = false;
	m_device = VK_NULL_HANDLE;
	m_instance = VK_NULL_HANDLE;

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

render::device::SamplerDesc CVulkanContext::ManagedTextureSampler( int handle ) const
{
	using render::device::AddressMode;
	using render::device::Filter;
	const int state = ManagedTextureSamplerState( handle );
	render::device::SamplerDesc desc;
	const Filter filter = ( state & kSamplerLinear ) ? Filter::kLinear : Filter::kNearest;
	desc.magFilter = filter;
	desc.minFilter = filter;
	desc.mipFilter = ( state & kSamplerMipLinear ) ? Filter::kLinear : Filter::kNearest;
	desc.address = ( state & kSamplerClampU ) && ( state & kSamplerClampV )
	                   ? AddressMode::kClampToEdge
	                   : AddressMode::kRepeat;
	if ( ( state & kSamplerAnisotropic ) && m_anisotropyLevel > 1 )
	{
		desc.magFilter = Filter::kLinear;
		desc.minFilter = Filter::kLinear;
		desc.mipFilter = Filter::kLinear;
		desc.maxAnisotropy = static_cast<std::uint32_t>( m_anisotropyLevel );
	}
	return desc;
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

} // namespace render_vulkan
