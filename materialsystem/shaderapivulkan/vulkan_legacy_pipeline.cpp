//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The native Vulkan pipeline family of the legacy shader ports
//          (vulkan_legacy_programs.h): the shared layout, each port's stages
//          and pipelines, the per-draw constants and the descriptor sets a
//          port's samplers resolve to.
//
//===========================================================================//

#include "vulkan_device.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>

namespace render_vulkan
{
namespace
{

void LegacyLog( const char *fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	std::fprintf( stderr, "[NativeVulkan] " );
	std::vfprintf( stderr, fmt, args );
	va_end( args );
}

void LegacyError( std::string *outError, const std::string &message )
{
	if ( outError )
		*outError = message;
	LegacyLog( "error: %s\n", message.c_str() );
}

} // namespace

bool CVulkanContext::InitLegacyPipeline( std::string *outError )
{
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( m_physicalDevice, &properties );
	if ( !m_clipPlanesSupported || properties.limits.maxPushConstantsSize < kSkinPushBytes ||
	     properties.limits.maxUniformBufferRange < sizeof( LegacyConstants ) ||
	     properties.limits.maxPerStageDescriptorSamplers < kLegacySamplerSlots )
	{
		LegacyLog( "legacy shader ports unavailable: push constants, uniform range, samplers "
		           "or clip distances\n" );
		return true;
	}
	m_uboAlignment =
	    std::max<VkDeviceSize>( 16, properties.limits.minUniformBufferOffsetAlignment );

	VkDescriptorSetLayoutBinding ubo = {};
	ubo.binding = 0;
	ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	ubo.descriptorCount = 1;
	ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo lb = {};
	lb.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	lb.bindingCount = 1;
	lb.pBindings = &ubo;
	if ( vkCreateDescriptorSetLayout( m_device, &lb, nullptr, &m_legacyUboLayout ) != VK_SUCCESS )
	{
		LegacyError( outError, "vkCreateDescriptorSetLayout (legacy constants) failed" );
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
	if ( vkCreateDescriptorPool( m_device, &pi, nullptr, &m_legacyUboPool ) != VK_SUCCESS )
	{
		LegacyError( outError, "vkCreateDescriptorPool (legacy constants) failed" );
		return false;
	}
	m_legacyUbos.assign( slots, SkinUniformBuffer() );
	for ( SkinUniformBuffer &slot : m_legacyUbos )
	{
		VkDescriptorSetAllocateInfo da = {};
		da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		da.descriptorPool = m_legacyUboPool;
		da.descriptorSetCount = 1;
		da.pSetLayouts = &m_legacyUboLayout;
		if ( vkAllocateDescriptorSets( m_device, &da, &slot.set ) != VK_SUCCESS )
		{
			LegacyError( outError, "vkAllocateDescriptorSets (legacy constants) failed" );
			return false;
		}
	}

	// The samplers: one combined image sampler per slot, written per draw.
	VkDescriptorSetLayoutBinding samplers[kLegacySamplerSlots] = {};
	for ( uint32_t i = 0; i < kLegacySamplerSlots; ++i )
	{
		samplers[i].binding = i;
		samplers[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplers[i].descriptorCount = 1;
		samplers[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	lb.bindingCount = kLegacySamplerSlots;
	lb.pBindings = samplers;
	if ( vkCreateDescriptorSetLayout( m_device, &lb, nullptr, &m_legacySamplerLayout ) !=
	     VK_SUCCESS )
	{
		LegacyError( outError, "vkCreateDescriptorSetLayout (legacy samplers) failed" );
		return false;
	}
	m_legacySamplerPools.assign( slots, LegacySamplerPool() );

	VkPushConstantRange pc = {};
	pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pc.size = kSkinPushBytes;
	VkDescriptorSetLayout sets[2];
	sets[kLegacySamplerSet] = m_legacySamplerLayout;
	sets[kLegacyConstantsSet] = m_legacyUboLayout;
	VkPipelineLayoutCreateInfo pl = {};
	pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl.setLayoutCount = 2;
	pl.pSetLayouts = sets;
	pl.pushConstantRangeCount = 1;
	pl.pPushConstantRanges = &pc;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	if ( vkCreatePipelineLayout( m_device, &pl, nullptr, &layout ) != VK_SUCCESS )
	{
		LegacyError( outError, "vkCreatePipelineLayout (legacy) failed" );
		return false;
	}

	// The whole vertex record (vulkan_legacy_programs.h), locations 0..7.
	static const struct
	{
		VkFormat format;
		uint32_t firstFloat;
	} kAttributes[8] = {
	    { VK_FORMAT_R32G32B32_SFLOAT, 0 },     // world position
	    { VK_FORMAT_R32G32B32_SFLOAT, 3 },     // color or vertex lighting
	    { VK_FORMAT_R32G32_SFLOAT, 6 },        // TEXCOORD0
	    { VK_FORMAT_R32G32_SFLOAT, 8 },        // TEXCOORD1
	    { VK_FORMAT_R32G32B32_SFLOAT, 10 },    // world normal
	    { VK_FORMAT_R32G32B32A32_SFLOAT, 13 }, // world tangent S and sign
	    { VK_FORMAT_R32_SFLOAT, 17 },          // color alpha
	    { VK_FORMAT_R32G32B32A32_SFLOAT, 18 }, // tangent T or TEXCOORD2
	};
	for ( uint32_t i = 0; i < 8; ++i )
	{
		m_legacyAttrs[i].location = i;
		m_legacyAttrs[i].binding = 0;
		m_legacyAttrs[i].format = kAttributes[i].format;
		m_legacyAttrs[i].offset =
		    static_cast<uint32_t>( sizeof( float ) * kAttributes[i].firstFloat );
	}
	m_legacyVin = m_texTemplate.vin;
	m_legacyVin.vertexAttributeDescriptionCount = 8;
	m_legacyVin.pVertexAttributeDescriptions = m_legacyAttrs;
	m_legacyStages.assign( static_cast<size_t>( LegacyProgramCount() ), LegacyStages() );
	m_legacyPipelineLayout = layout;

	// A sampler the pass did not enable has no texture on D3D9
	// (CShaderAPIDx8::ApplyTextureEnable), which reads ( 0, 0, 0, 1 ).
	const uint8_t noTexture[4] = { 0, 0, 0, 255 };
	m_legacyNoTextureHandle = CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, outError );
	m_legacyNoTextureCubeHandle = CreateManagedTexture(
	    1, 1, VK_FORMAT_R8G8B8A8_UNORM, outError, 0, 1, VK_FORMAT_UNDEFINED, true );
	if ( m_legacyNoTextureHandle < 0 || m_legacyNoTextureCubeHandle < 0 ||
	     !UploadManagedTexture(
	         m_legacyNoTextureHandle, noTexture, sizeof( noTexture ), outError ) )
		return false;
	for ( uint32_t face = 0; face < 6; ++face )
	{
		if ( !UploadManagedTexture(
		         m_legacyNoTextureCubeHandle, noTexture, sizeof( noTexture ), outError, 0, face ) )
			return false;
	}
	return true;
}

void CVulkanContext::DestroyLegacyPipeline()
{
	for ( LegacyStages &stages : m_legacyStages )
	{
		for ( const auto &entry : stages.pipelines )
		{
			if ( entry.second != VK_NULL_HANDLE )
				vkDestroyPipeline( m_device, entry.second, nullptr );
		}
		for ( VkShaderModule *module : { &stages.vert, &stages.frag } )
		{
			if ( *module != VK_NULL_HANDLE )
				vkDestroyShaderModule( m_device, *module, nullptr );
		}
	}
	m_legacyStages.clear();
	for ( int *handle : { &m_legacyNoTextureHandle, &m_legacyNoTextureCubeHandle } )
	{
		if ( *handle >= 0 )
			DestroyManagedTexture( *handle );
		*handle = -1;
	}
	for ( SkinUniformBuffer &slot : m_legacyUbos )
	{
		if ( slot.mapped )
			UnmapMemory( slot.memory );
		if ( slot.buffer != VK_NULL_HANDLE )
			vkDestroyBuffer( m_device, slot.buffer, nullptr );
		if ( slot.memory != VK_NULL_HANDLE )
			FreeMemory( slot.memory );
	}
	m_legacyUbos.clear();
	for ( LegacySamplerPool &pool : m_legacySamplerPools )
	{
		if ( pool.pool != VK_NULL_HANDLE )
			vkDestroyDescriptorPool( m_device, pool.pool, nullptr );
	}
	m_legacySamplerPools.clear();
	if ( m_legacySamplerLayout != VK_NULL_HANDLE )
		vkDestroyDescriptorSetLayout( m_device, m_legacySamplerLayout, nullptr );
	m_legacySamplerLayout = VK_NULL_HANDLE;
	if ( m_legacyUboPool != VK_NULL_HANDLE )
		vkDestroyDescriptorPool( m_device, m_legacyUboPool, nullptr );
	m_legacyUboPool = VK_NULL_HANDLE;
	if ( m_legacyUboLayout != VK_NULL_HANDLE )
		vkDestroyDescriptorSetLayout( m_device, m_legacyUboLayout, nullptr );
	m_legacyUboLayout = VK_NULL_HANDLE;
	if ( m_legacyPipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_legacyPipelineLayout, nullptr );
	m_legacyPipelineLayout = VK_NULL_HANDLE;
}

VkPipeline CVulkanContext::LegacyPipeline(
    int program, const DynRasterState &state, bool srgbPass, int samples )
{
	if ( m_legacyPipelineLayout == VK_NULL_HANDLE || program < 0 ||
	     program >= static_cast<int>( m_legacyStages.size() ) )
		return VK_NULL_HANDLE;
	LegacyStages &stages = m_legacyStages[static_cast<size_t>( program )];
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const auto existing = stages.pipelines.find( key );
	if ( existing != stages.pipelines.end() )
		return existing->second;
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	if ( pass == VK_NULL_HANDLE || stages.failed )
		return VK_NULL_HANDLE;
	const LegacyProgram &port = GetLegacyProgram( program );
	if ( stages.vert == VK_NULL_HANDLE )
	{
		std::string error;
		if ( !CreateShaderModule( port.vertSpv, port.vertBytes, &stages.vert, &error ) ||
		     !CreateShaderModule( port.fragSpv, port.fragBytes, &stages.frag, &error ) )
		{
			LegacyLog( "legacy port %s unavailable: %s\n", port.name, error.c_str() );
			stages.failed = true;
			return VK_NULL_HANDLE;
		}
	}
	VkPipeline pipeline = BuildMaterialPipeline(
	    state, stages.vert, stages.frag, m_legacyPipelineLayout, &m_legacyVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		LegacyLog( "vkCreateGraphicsPipelines (legacy port %s, state %#llx) failed\n", port.name,
		    static_cast<unsigned long long>( key ) );
	stages.pipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE )
		NotePipelineVariant( kPipelineLegacyFirst + program, key );
	return pipeline;
}

bool CVulkanContext::UploadLegacyConstants( std::vector<uint32_t> *offsets )
{
	offsets->clear();
	if ( m_dynLegacyConstants.empty() )
		return true;
	if ( m_legacyUbos.empty() )
		return false;
	// The render target each draw addresses, for its VPOS-based math.
	for ( const DynDraw &d : m_dynDrawRecords )
	{
		if ( d.kind != kRecordDraw || d.legacy < 0 ||
		     static_cast<size_t>( d.legacy ) >= m_dynLegacyConstants.size() )
			continue;
		uint32_t width = 0, height = 0;
		GetTargetExtent( d.target, &width, &height );
		float *target = m_dynLegacyConstants[static_cast<size_t>( d.legacy )].target;
		target[0] = static_cast<float>( width );
		target[1] = static_cast<float>( height );
		target[2] = width ? 1.0f / static_cast<float>( width ) : 0.0f;
		target[3] = height ? 1.0f / static_cast<float>( height ) : 0.0f;
	}
	// This frame's sampler sets: the slot's frame has finished (its fence was
	// waited on when the frame began), so its sets are free to reuse. A pool
	// holds a set for every legacy draw of the frame.
	LegacySamplerPool &pool =
	    m_legacySamplerPools[static_cast<size_t>( m_currentFrame ) % m_legacySamplerPools.size()];
	pool.written.clear();
	const uint32_t draws = static_cast<uint32_t>( m_dynLegacyConstants.size() );
	if ( pool.pool != VK_NULL_HANDLE && draws <= pool.capacity )
	{
		vkResetDescriptorPool( m_device, pool.pool, 0 );
	}
	else
	{
		if ( pool.pool != VK_NULL_HANDLE )
			vkDestroyDescriptorPool( m_device, pool.pool, nullptr );
		pool.pool = VK_NULL_HANDLE;
		pool.capacity = std::max<uint32_t>( 64, draws + draws / 2 );
		VkDescriptorPoolSize size = {};
		size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		size.descriptorCount = pool.capacity * kLegacySamplerSlots;
		VkDescriptorPoolCreateInfo pi = {};
		pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pi.maxSets = pool.capacity;
		pi.poolSizeCount = 1;
		pi.pPoolSizes = &size;
		if ( vkCreateDescriptorPool( m_device, &pi, nullptr, &pool.pool ) != VK_SUCCESS )
		{
			LegacyLog( "legacy sampler pool (%u sets) unavailable\n", pool.capacity );
			pool.pool = VK_NULL_HANDLE;
			pool.capacity = 0;
			return false;
		}
	}

	const VkDeviceSize block = sizeof( LegacyConstants );
	const VkDeviceSize stride = ( block + m_uboAlignment - 1 ) / m_uboAlignment * m_uboAlignment;
	SkinUniformBuffer &slot =
	    m_legacyUbos[static_cast<size_t>( m_currentFrame ) % m_legacyUbos.size()];
	if ( !EnsureUniformRingSlot(
	         slot, stride * m_dynLegacyConstants.size(), stride, block, "legacy constants" ) )
		return false;
	for ( size_t i = 0; i < m_dynLegacyConstants.size(); ++i )
	{
		std::memcpy( static_cast<unsigned char *>( slot.mapped ) + stride * i,
		    &m_dynLegacyConstants[i], block );
		offsets->push_back( static_cast<uint32_t>( stride * i ) );
	}
	return true;
}

int CVulkanContext::BindLegacySets(
    VkCommandBuffer cmd, const DynDraw &d, int openTarget, uint32_t constantsOffset, int *gammaSceneSamplers )
{
	const LegacyProgram &port = GetLegacyProgram( d.legacyProgram );
	const LegacyConstants &constants = m_dynLegacyConstants[static_cast<size_t>( d.legacy )];
	const int srgbSamplers = constants.bools[2] & 0xFFFF;
	const int enabledSamplers = ( constants.bools[2] >> 16 ) & 0xFFFF;
	int manualDecode = 0;
	*gammaSceneSamplers = 0;
	// Each slot's image: the texture bound to the D3D9 sampler, through its sRGB
	// view when the pass reads it as sRGB, or the white image of the port's type.
	VkDescriptorImageInfo images[kLegacySamplerSlots] = {};
	std::vector<uint64_t> key;
	for ( int i = 0; i < kLegacySamplerSlots; ++i )
	{
		const LegacySamplerSlot &slot = port.samplers[i];
		if ( slot.sampler < 0 )
			continue; // read by no stage
		// Sampler 0 is the draw's texture; a lightmap page bound to sampler 1
		// is its lightmap (BindStandardTexture).
		int handle = slot.sampler == 0 ? d.texHandle : d.samplerHandles[slot.sampler];
		if ( slot.sampler == 1 && d.lightmapHandle >= 0 )
			handle = d.lightmapHandle;
		// A sampler the pass did not enable reads as D3D9's sampler with no
		// texture set.
		if ( !( enabledSamplers & ( 1 << slot.sampler ) ) && slot.dim != kLegacySamplerVolume )
			handle = slot.dim == kLegacySamplerCube ? m_legacyNoTextureCubeHandle
			                                        : m_legacyNoTextureHandle;
		// A render target cannot be sampled inside its own pass.
		if ( handle == openTarget || handle < 0 ||
		     handle >= static_cast<int>( m_managedTextures.size() ) ||
		     m_managedTextures[static_cast<size_t>( handle )].view == VK_NULL_HANDLE )
			handle = -1;
		// A texture of another image type than the port declares reads the
		// white image of the declared type; without one the draw is declined.
		if ( slot.dim == kLegacySamplerCube && !ManagedTextureIsCube( handle ) )
			handle = m_whiteCubeHandle;
		else if ( slot.dim == kLegacySamplerVolume && !ManagedTextureIsVolume( handle ) )
			handle = m_whiteVolumeHandle;
		else if ( slot.dim == kLegacySampler2D &&
		          ( ManagedTextureIsCube( handle ) || ManagedTextureIsVolume( handle ) ) )
			handle = -1;
		images[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if ( handle < 0 )
		{
			if ( slot.dim != kLegacySampler2D || m_dynTexView == VK_NULL_HANDLE )
				return -1;
			images[i].imageView = m_dynTexView;
			images[i].sampler = m_dynTexSampler;
		}
		else
		{
			const ManagedTexture &texture = m_managedTextures[static_cast<size_t>( handle )];
			images[i].imageView = texture.view;
			// Retained gamma-domain effects read linear scene copies through
			// the shared transfer helper; material sRGB reads remain linear.
			if ( texture.format == VK_FORMAT_R16G16B16A16_SFLOAT &&
			     IsRenderTargetTexture( handle ) && !( srgbSamplers & ( 1 << slot.sampler ) ) )
				*gammaSceneSamplers |= 1 << i;
			images[i].sampler = m_samplers[texture.samplerState];
			if ( srgbSamplers & ( 1 << slot.sampler ) )
			{
				// A texture stored in an sRGB format is decoded through any view.
				if ( texture.srgbView != VK_NULL_HANDLE )
					images[i].imageView = texture.srgbView;
				else if ( texture.format != VK_FORMAT_R16G16B16A16_SFLOAT &&
				          !IsSrgbFormat( texture.format ) )
					manualDecode |= 1 << i;
			}
		}
		key.push_back( reinterpret_cast<uint64_t>( images[i].imageView ) );
		key.push_back( reinterpret_cast<uint64_t>( images[i].sampler ) );
	}
	LegacySamplerPool &pool =
	    m_legacySamplerPools[static_cast<size_t>( m_currentFrame ) % m_legacySamplerPools.size()];
	VkDescriptorSet samplerSet = VK_NULL_HANDLE;
	const auto written = pool.written.find( key );
	if ( written != pool.written.end() )
	{
		samplerSet = written->second;
	}
	else
	{
		VkDescriptorSetAllocateInfo da = {};
		da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		da.descriptorPool = pool.pool;
		da.descriptorSetCount = 1;
		da.pSetLayouts = &m_legacySamplerLayout;
		if ( pool.pool == VK_NULL_HANDLE ||
		     vkAllocateDescriptorSets( m_device, &da, &samplerSet ) != VK_SUCCESS )
			return -1;
		VkWriteDescriptorSet writes[kLegacySamplerSlots] = {};
		uint32_t writeCount = 0;
		for ( uint32_t i = 0; i < kLegacySamplerSlots; ++i )
		{
			if ( images[i].imageView == VK_NULL_HANDLE )
				continue;
			VkWriteDescriptorSet &w = writes[writeCount++];
			w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			w.dstSet = samplerSet;
			w.dstBinding = i;
			w.descriptorCount = 1;
			w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			w.pImageInfo = &images[i];
		}
		if ( writeCount )
			vkUpdateDescriptorSets( m_device, writeCount, writes, 0, nullptr );
		pool.written.emplace( std::move( key ), samplerSet );
	}
	const VkDescriptorSet sets[2] = {
	    samplerSet, m_legacyUbos[static_cast<size_t>( m_currentFrame ) % m_legacyUbos.size()].set };
	vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_legacyPipelineLayout, 0, 2,
	    sets, 1, &constantsOffset );
	return manualDecode;
}

} // namespace render_vulkan
