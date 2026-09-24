//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan WMSH PBR pipeline and material image requirements.
//
//===========================================================================//

#include "vulkan_device.h"
#include "material_spv.h"

#include <cstdarg>
#include <cstdio>
#include <initializer_list>

namespace render_vulkan
{
namespace
{

void WorldPbrLog( const char *fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	std::fprintf( stderr, "[NativeVulkan] " );
	std::vfprintf( stderr, fmt, args );
	va_end( args );
}

void WorldPbrError( std::string *outError, const std::string &message )
{
	if ( outError )
		*outError = message;
	WorldPbrLog( "error: %s\n", message.c_str() );
}

} // namespace

VkPipeline CVulkanContext::WorldPbrPipeline(
    const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_worldPbrPipelines.find( key );
	if ( existing != m_worldPbrPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_worldPbrVert == VK_NULL_HANDLE || m_worldPbrFrag == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_worldPbrVert, m_worldPbrFrag,
	    m_worldPbrPipelineLayout, &m_worldPbrVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		WorldPbrLog( "vkCreateGraphicsPipelines (WMSH PBR, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_worldPbrPipelines[key] = pipeline;
	return pipeline;
}

bool CVulkanContext::InitPbrWorldPipeline( std::string *outError )
{
	if ( !m_pbrDirectReady || m_worldVert == VK_NULL_HANDLE )
	{
		WorldPbrError( outError, "WMSH PBR requires the native world and split-sum pipelines" );
		return false;
	}
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	const uint32_t pushBytes = m_clipPlanesSupported ? kTexturedPushBytes : 128;
	if ( properties.limits.maxBoundDescriptorSets < 5 ||
	     properties.limits.maxPushConstantsSize < pushBytes )
	{
		WorldPbrError( outError, "WMSH PBR needs five texture sets and its scene push block" );
		return false;
	}
	for ( int i = 0; i < 4; ++i )
		m_worldPbrAttrs[i] = m_worldAttrs[i];
	m_worldPbrAttrs[4] = { 4, 0, VK_FORMAT_R16G16_SNORM, 16 };
	m_worldPbrAttrs[5] = { 5, 0, VK_FORMAT_R8_SINT, 20 };
	m_worldPbrVin = m_worldVin;
	m_worldPbrVin.vertexAttributeDescriptionCount = 6;
	m_worldPbrVin.pVertexAttributeDescriptions = m_worldPbrAttrs;
	const VkDescriptorSetLayout layouts[5] = { m_dynTexDescLayout, m_dynTexDescLayout,
	    m_dynTexDescLayout, m_dynTexDescLayout, m_dynTexDescLayout };
	VkPushConstantRange range = {};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	range.size = pushBytes;
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = 5;
	info.pSetLayouts = layouts;
	info.pushConstantRangeCount = 1;
	info.pPushConstantRanges = &range;
	if ( vkCreatePipelineLayout( m_device, &info, nullptr, &m_worldPbrPipelineLayout ) !=
	     VK_SUCCESS )
	{
		WorldPbrError( outError, "vkCreatePipelineLayout (WMSH PBR) failed" );
		return false;
	}
	const uint32_t *vertexWords = m_clipPlanesSupported ? g_worldPbrClipVertSpv : g_worldPbrVertSpv;
	const size_t vertexBytes =
	    m_clipPlanesSupported ? sizeof( g_worldPbrClipVertSpv ) : sizeof( g_worldPbrVertSpv );
	const uint32_t *fragmentWords =
	    m_clipPlanesSupported ? g_worldPbrClipFragSpv : g_worldPbrFragSpv;
	const size_t fragmentBytes =
	    m_clipPlanesSupported ? sizeof( g_worldPbrClipFragSpv ) : sizeof( g_worldPbrFragSpv );
	if ( !CreateShaderModule( vertexWords, vertexBytes, &m_worldPbrVert, outError ) ||
	     !CreateShaderModule( fragmentWords, fragmentBytes, &m_worldPbrFrag, outError ) )
		return false;
	if ( WorldPbrPipeline( DynRasterState() ) == VK_NULL_HANDLE )
	{
		WorldPbrError( outError, "vkCreateGraphicsPipelines (WMSH PBR) failed" );
		return false;
	}
	m_pbrWorldReady = true;
	return true;
}

void CVulkanContext::DestroyPbrWorldPipeline()
{
	for ( const auto &entry : m_worldPbrPipelines )
		if ( entry.second != VK_NULL_HANDLE )
			vkDestroyPipeline( m_device, entry.second, nullptr );
	m_worldPbrPipelines.clear();
	for ( VkShaderModule *module : { &m_worldPbrVert, &m_worldPbrFrag } )
	{
		if ( *module != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, *module, nullptr );
		*module = VK_NULL_HANDLE;
	}
	if ( m_worldPbrPipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_worldPbrPipelineLayout, nullptr );
	m_worldPbrPipelineLayout = VK_NULL_HANDLE;
	m_pbrWorldReady = false;
}

bool CVulkanContext::PbrWorldTexturesReady(
    int base, int mrao, int normal, bool useNormal, bool baseReadSrgb ) const
{
	const auto ready = [&]( int handle ) -> const ManagedTexture *
	{
		if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
			return nullptr;
		const ManagedTexture &texture = m_managedTextures[static_cast<size_t>( handle )];
		return texture.uploaded && texture.descSet != VK_NULL_HANDLE ? &texture : nullptr;
	};
	const ManagedTexture *baseTexture = ready( base );
	const ManagedTexture *maskTexture = ready( mrao );
	const ManagedTexture *normalTexture = useNormal ? ready( normal ) : nullptr;
	const ManagedTexture *lightmap = ready( m_worldLightmapHandle );
	if ( !baseTexture || !maskTexture || !lightmap || ( useNormal && !normalTexture ) )
		return false;
	const bool baseSrgb = baseTexture->format == VK_FORMAT_R8G8B8A8_SRGB ||
	                      baseTexture->format == VK_FORMAT_BC7_SRGB_BLOCK ||
	                      ( baseReadSrgb && baseTexture->descSetSrgb != VK_NULL_HANDLE );
	const bool maskLinear = maskTexture->format == VK_FORMAT_R8G8B8A8_UNORM ||
	                        maskTexture->format == VK_FORMAT_BC7_UNORM_BLOCK ||
	                        maskTexture->format == VK_FORMAT_ASTC_4x4_UNORM_BLOCK ||
	                        maskTexture->format == VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
	const bool normalLinear = !useNormal || normalTexture->format == VK_FORMAT_R8G8_UNORM ||
	                          normalTexture->format == VK_FORMAT_BC5_UNORM_BLOCK;
	return baseSrgb && maskLinear && normalLinear &&
	       lightmap->format == VK_FORMAT_R16G16B16A16_SFLOAT;
}

bool CVulkanContext::PbrWorldNormalReady( int handle ) const
{
	if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
		return false;
	const ManagedTexture &texture = m_managedTextures[static_cast<size_t>( handle )];
	return texture.uploaded && texture.descSet != VK_NULL_HANDLE &&
	       ( texture.format == VK_FORMAT_R8G8_UNORM ||
	           texture.format == VK_FORMAT_BC5_UNORM_BLOCK );
}

bool CVulkanContext::SelectPbrWorldMaterial(
    int mrao, int normal, const float eye[3], float alphaReference )
{
	if ( !eye || !m_pbrWorldReady )
		return false;
	PbrWorldScene scene;
	for ( int i = 0; i < 3; ++i )
		scene.eye[i] = eye[i];
	scene.material[0] = alphaReference;
	scene.material[1] = PbrWorldNormalReady( normal ) ? 1.0f : 0.0f;
	if ( !PbrWorldTexturesReady( m_dynBoundTexHandle, mrao, normal, scene.material[1] >= 0.5f,
	         ( m_dynColorFlags & kColorSrgbReadBase ) != 0 ) )
		return false;
	BindManagedSampler( 1, mrao );
	BindManagedSampler( 2, normal );
	SetDynamicPbrWorldScene( scene );
	SelectDynamicShader( kDynShaderPbrWorld );
	return true;
}

} // namespace render_vulkan
