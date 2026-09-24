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
	const bool indirectView = m_indirectViewMode != 0;
	std::map<uint64_t, VkPipeline> &pipelines =
	    indirectView ? m_worldPbrIndirectPipelines : m_worldPbrPipelines;
	const VkShaderModule fragment = indirectView ? m_worldPbrIndirectFrag : m_worldPbrFrag;
	const auto existing = pipelines.find( key );
	if ( existing != pipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE )
		return VK_NULL_HANDLE; // no pass of this sample count exists now
	if ( m_worldPbrVert == VK_NULL_HANDLE || fragment == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline(
	    state, m_worldPbrVert, fragment, m_worldPbrPipelineLayout, &m_worldPbrVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		WorldPbrLog( "vkCreateGraphicsPipelines (WMSH PBR%s, state %#llx) failed\n",
		    indirectView ? " indirect view" : "", static_cast<unsigned long long>( key ) );
	pipelines[key] = pipeline;
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
	if ( properties.limits.maxBoundDescriptorSets < 7 ||
	     properties.limits.maxPushConstantsSize < pushBytes )
	{
		WorldPbrError( outError, "WMSH PBR needs seven texture sets and its scene push block" );
		return false;
	}
	for ( int i = 0; i < 4; ++i )
		m_worldPbrAttrs[i] = m_worldAttrs[i];
	m_worldPbrAttrs[4] = { 4, 0, VK_FORMAT_R16G16_SNORM, 16 };
	m_worldPbrAttrs[5] = { 5, 0, VK_FORMAT_R8_SINT, 20 };
	m_worldPbrVin = m_worldVin;
	m_worldPbrVin.vertexAttributeDescriptionCount = 6;
	m_worldPbrVin.pVertexAttributeDescriptions = m_worldPbrAttrs;
	// Base, MRAO, normal, LMAP, split sum, emission and the $envmap cube.
	VkDescriptorSetLayout layouts[7];
	for ( VkDescriptorSetLayout &layout : layouts )
		layout = m_dynTexDescLayout;
	VkPushConstantRange range = {};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	range.size = pushBytes;
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = 7;
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
	const uint32_t *indirectWords =
	    m_clipPlanesSupported ? g_worldPbrIndirectClipFragSpv : g_worldPbrIndirectFragSpv;
	const size_t indirectBytes = m_clipPlanesSupported ? sizeof( g_worldPbrIndirectClipFragSpv )
	                                                   : sizeof( g_worldPbrIndirectFragSpv );
	if ( !CreateShaderModule( vertexWords, vertexBytes, &m_worldPbrVert, outError ) ||
	     !CreateShaderModule( fragmentWords, fragmentBytes, &m_worldPbrFrag, outError ) ||
	     !CreateShaderModule( indirectWords, indirectBytes, &m_worldPbrIndirectFrag, outError ) )
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
	for ( std::map<uint64_t, VkPipeline> *pipelines :
	    { &m_worldPbrPipelines, &m_worldPbrIndirectPipelines } )
	{
		for ( const auto &entry : *pipelines )
			if ( entry.second != VK_NULL_HANDLE )
				vkDestroyPipeline( m_device, entry.second, nullptr );
		pipelines->clear();
	}
	for ( VkShaderModule *module : { &m_worldPbrVert, &m_worldPbrFrag, &m_worldPbrIndirectFrag } )
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

VkPipeline CVulkanContext::WorldGlassPipeline(
    const DynRasterState &state, bool srgbPass, int samples )
{
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	const auto existing = m_worldGlassPipelines.find( key );
	if ( existing != m_worldGlassPipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE || m_worldPbrVert == VK_NULL_HANDLE ||
	     m_worldGlassFrag == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_worldPbrVert, m_worldGlassFrag,
	    m_worldGlassPipelineLayout, &m_worldPbrVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		WorldPbrLog( "vkCreateGraphicsPipelines (WMSH glass, state %#llx) failed\n",
		    static_cast<unsigned long long>( key ) );
	m_worldGlassPipelines[key] = pipeline;
	return pipeline;
}

bool CVulkanContext::InitPbrGlassPipeline( std::string *outError )
{
	if ( !m_pbrWorldReady )
	{
		WorldPbrError( outError, "WMSH glass requires the WMSH PBR pipeline" );
		return false;
	}
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	if ( properties.limits.maxBoundDescriptorSets < 7 )
	{
		WorldPbrError( outError, "WMSH glass needs seven texture sets" );
		return false;
	}
	VkDescriptorSetLayout layouts[7];
	for ( VkDescriptorSetLayout &layout : layouts )
		layout = m_dynTexDescLayout;
	VkPushConstantRange range = {};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	range.size = m_clipPlanesSupported ? kTexturedPushBytes : 128;
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = 7;
	info.pSetLayouts = layouts;
	info.pushConstantRangeCount = 1;
	info.pPushConstantRanges = &range;
	if ( vkCreatePipelineLayout( m_device, &info, nullptr, &m_worldGlassPipelineLayout ) !=
	     VK_SUCCESS )
	{
		WorldPbrError( outError, "vkCreatePipelineLayout (WMSH glass) failed" );
		return false;
	}
	const uint32_t *fragmentWords =
	    m_clipPlanesSupported ? g_worldGlassClipFragSpv : g_worldGlassFragSpv;
	const size_t fragmentBytes =
	    m_clipPlanesSupported ? sizeof( g_worldGlassClipFragSpv ) : sizeof( g_worldGlassFragSpv );
	if ( !CreateShaderModule( fragmentWords, fragmentBytes, &m_worldGlassFrag, outError ) )
		return false;
	DynRasterState state;
	state.depthWrite = false;
	if ( WorldGlassPipeline( state ) == VK_NULL_HANDLE )
	{
		WorldPbrError( outError, "vkCreateGraphicsPipelines (WMSH glass) failed" );
		return false;
	}
	m_pbrGlassReady = true;
	return true;
}

void CVulkanContext::DestroyPbrGlassPipeline()
{
	for ( const auto &entry : m_worldGlassPipelines )
		if ( entry.second != VK_NULL_HANDLE )
			vkDestroyPipeline( m_device, entry.second, nullptr );
	m_worldGlassPipelines.clear();
	if ( m_worldGlassFrag != VK_NULL_HANDLE )
		vkDestroyShaderModule( m_device, m_worldGlassFrag, nullptr );
	m_worldGlassFrag = VK_NULL_HANDLE;
	if ( m_worldGlassPipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_worldGlassPipelineLayout, nullptr );
	m_worldGlassPipelineLayout = VK_NULL_HANDLE;
	m_pbrGlassReady = false;
}

// Linear formats world_pbr.frag can read a tangent-space normal's .rg from.
// Material-system VTF normal maps arrive as 8-bit RGBA/BGRA (CreateTexture has
// no two-channel mapping); packaged KTX2 normals as R8G8 or BC5.
static bool PbrWorldNormalFormat( VkFormat format )
{
	return format == VK_FORMAT_R8G8_UNORM || format == VK_FORMAT_BC5_UNORM_BLOCK ||
	       format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_B8G8R8A8_UNORM;
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
	const bool normalLinear = !useNormal || PbrWorldNormalFormat( normalTexture->format );
	return baseSrgb && maskLinear && normalLinear &&
	       lightmap->format == VK_FORMAT_R16G16B16A16_SFLOAT;
}

bool CVulkanContext::PbrWorldNormalReady( int handle ) const
{
	if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
		return false;
	const ManagedTexture &texture = m_managedTextures[static_cast<size_t>( handle )];
	return texture.uploaded && texture.descSet != VK_NULL_HANDLE &&
	       PbrWorldNormalFormat( texture.format );
}

bool CVulkanContext::SelectPbrWorldMaterial(
    int mrao, int normal, const float eye[3], float alphaReference )
{
	return SelectPbrWorldMaterial( mrao, normal, eye, alphaReference, PbrWorldMaps() );
}

bool CVulkanContext::SelectPbrWorldMaterial(
    int mrao, int normal, const float eye[3], float alphaReference, const PbrWorldMaps &maps )
{
	if ( !eye || !m_pbrWorldReady )
		return false;
	const auto uploaded = [this]( int handle ) -> const ManagedTexture *
	{
		if ( handle < 0 || handle >= static_cast<int>( m_managedTextures.size() ) )
			return nullptr;
		const ManagedTexture &texture = m_managedTextures[static_cast<size_t>( handle )];
		return texture.uploaded && texture.descSet != VK_NULL_HANDLE ? &texture : nullptr;
	};
	PbrWorldScene scene;
	for ( int i = 0; i < 3; ++i )
		scene.eye[i] = eye[i];
	scene.material[0] = alphaReference;
	scene.material[1] = PbrWorldNormalReady( normal ) ? 1.0f : 0.0f;
	if ( !PbrWorldTexturesReady( m_dynBoundTexHandle, mrao, normal, scene.material[1] >= 0.5f,
	         ( m_dynColorFlags & kColorSrgbReadBase ) != 0 ) )
		return false;
	if ( maps.emission >= 0 )
	{
		if ( !uploaded( maps.emission ) || !( maps.emissionScale >= 0.0f ) )
			return false;
		scene.material[2] = maps.emissionScale;
	}
	if ( !( maps.clearCoat >= 0.0f && maps.clearCoat <= 1.0f &&
	         maps.clearCoatRoughness >= 0.0f && maps.clearCoatRoughness <= 1.0f ) )
		return false;
	// The push block's spare lanes (world_pbr.frag).
	scene.eye[3] = maps.clearCoat;
	scene.lightRadiance[3] = maps.clearCoatRoughness;
	if ( maps.environment >= 0 )
	{
		const ManagedTexture *cube = uploaded( maps.environment );
		if ( !cube || cube->layers != 6 )
			return false;
		scene.material[3] = static_cast<float>( cube->mipLevels );
	}
	BindManagedSampler( 1, mrao );
	BindManagedSampler( 2, normal );
	BindManagedSampler( kPbrWorldEmissionSampler, maps.emission );
	BindManagedSampler( kPbrWorldEnvironmentSampler, maps.environment );
	SetDynamicPbrWorldScene( scene );
	SelectDynamicShader( kDynShaderPbrWorld );
	return true;
}

bool CVulkanContext::SelectPbrGlassMaterial( int mrao, int normal, const float eye[3],
    float alphaReference, const PbrGlassParams &glass )
{
	if ( !m_pbrGlassReady || !( glass.transmission >= 0.0f && glass.transmission <= 1.0f ) ||
	     !( glass.ior >= 1.0f ) || !( glass.thickness >= 0.0f ) )
		return false;
	if ( !SelectPbrWorldMaterial( mrao, normal, eye, alphaReference ) )
		return false;
	m_dynPbrWorld.glass[0] = glass.transmission;
	m_dynPbrWorld.glass[1] = glass.ior;
	m_dynPbrWorld.glass[2] = glass.thickness;
	m_dynGlassKey = glass.materialKey;
	SelectDynamicShader( kDynShaderPbrGlass );
	return true;
}

void CVulkanContext::SetIndirectLightView( int mode, float scale )
{
	m_indirectViewMode = mode >= 0 && mode <= 2 ? mode : 0;
	m_indirectViewScale = scale > 0.0f && scale < 1.0e6f ? scale : 1.0f;
}

} // namespace render_vulkan
