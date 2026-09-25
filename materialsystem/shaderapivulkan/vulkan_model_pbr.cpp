//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan PBRMetalRough pipeline for dynamic meshes (models and
//          props). It shares the skin pipeline's vertex input, vertex stage
//          and constants ring, on the grouped PBR layout (frame, material and
//          constants sets); shaders/model_pbr.frag evaluates the RFC 0007
//          layered BRDF under Source's ambient cube and local lights.
//
//===========================================================================//

#include "vulkan_device.h"
#include "material_spv.h"

#include <cstdarg>
#include <cstdio>

namespace render_vulkan
{
namespace
{

void ModelPbrLog( const char *fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	std::fprintf( stderr, "[NativeVulkan] " );
	std::vfprintf( stderr, fmt, args );
	va_end( args );
}

void ModelPbrError( std::string *outError, const std::string &message )
{
	if ( outError )
		*outError = message;
	ModelPbrLog( "error: %s\n", message.c_str() );
}

} // namespace

VkPipeline CVulkanContext::PbrModelPipeline(
    const DynRasterState &state, bool envCube, bool srgbPass, int samples, bool probeVolume )
{
	// The indirect view has no $envmap variant: it never reads the cube.
	// View 3 shows models as view 1 does (their ambient cube).
	const int base = m_indirectViewMode != 0 ? 2 : envCube ? 1 : 0;
	const int variant = base + ( probeVolume ? 3 : 0 );
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	std::map<uint64_t, VkPipeline> &pipelines = m_pbrModelPipelines[variant];
	const auto existing = pipelines.find( key );
	if ( existing != pipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE || m_skinVert == VK_NULL_HANDLE ||
	     m_pbrModelPipelineLayout == VK_NULL_HANDLE || m_pbrModelFrag[variant] == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_skinVert, m_pbrModelFrag[variant],
	    m_pbrModelPipelineLayout, &m_skinVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		ModelPbrLog( "vkCreateGraphicsPipelines (model PBR%s%s, state %#llx) failed\n",
		    base == 2 ? " indirect view"
		    : envCube ? " envmap"
		              : "",
		    probeVolume ? " probe volume" : "", static_cast<unsigned long long>( key ) );
	pipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE && variant == base && base != 2 )
		NotePipelineVariant( envCube ? kPipelinePbrModelEnv : kPipelinePbrModel, key );
	return pipeline;
}

// Needs the skin vertex stage and constants ring, and the split-sum table of
// the PBR direct pipeline. Without them, model draws of PBRMetalRough are
// declined by name.
bool CVulkanContext::InitPbrModelPipeline( std::string *outError )
{
	if ( m_skinVert == VK_NULL_HANDLE || m_skinUboLayout == VK_NULL_HANDLE )
	{
		ModelPbrError( outError, "model PBR requires the skin vertex stage and constants ring" );
		return false;
	}
	if ( m_pbrSplitSumHandle < 0 )
	{
		ModelPbrError( outError, "model PBR requires the split-sum table" );
		return false;
	}
	if ( !CreatePbrPipelineLayout( kSkinPushBytes, &m_pbrModelPipelineLayout, outError ) ||
	     !CreateShaderModule(
	         g_modelPbrFragSpv, sizeof( g_modelPbrFragSpv ), &m_pbrModelFrag[0], outError ) ||
	     !CreateShaderModule(
	         g_modelPbrEnvFragSpv, sizeof( g_modelPbrEnvFragSpv ), &m_pbrModelFrag[1], outError ) ||
	     !CreateShaderModule( g_modelPbrIndirectFragSpv, sizeof( g_modelPbrIndirectFragSpv ),
	         &m_pbrModelFrag[2], outError ) )
		return false;
	// Per-pixel probe-volume sampling reads the probe atlas and grid table
	// from the frame set, on the same layout. Optional: without the variants
	// models keep the ambient cube (the engine evaluates it from the same
	// volume); nothing else changes.
	std::string probeError;
	if ( !CreateShaderModule( g_modelPbrProbeFragSpv, sizeof( g_modelPbrProbeFragSpv ),
	         &m_pbrModelFrag[3], &probeError ) ||
	     !CreateShaderModule( g_modelPbrEnvProbeFragSpv, sizeof( g_modelPbrEnvProbeFragSpv ),
	         &m_pbrModelFrag[4], &probeError ) ||
	     !CreateShaderModule( g_modelPbrIndirectProbeFragSpv,
	         sizeof( g_modelPbrIndirectProbeFragSpv ), &m_pbrModelFrag[5], &probeError ) ||
	     PbrModelPipeline( DynRasterState(), false, false, 1, true ) == VK_NULL_HANDLE )
	{
		ModelPbrLog( "model PBR probe-volume variants unavailable%s%s\n",
		    probeError.empty() ? "" : ": ", probeError.c_str() );
		for ( int variant = 3; variant < 6; ++variant )
		{
			for ( const auto &entry : m_pbrModelPipelines[variant] )
				if ( entry.second != VK_NULL_HANDLE )
					vkDestroyPipeline( m_device, entry.second, nullptr );
			m_pbrModelPipelines[variant].clear();
			if ( m_pbrModelFrag[variant] != VK_NULL_HANDLE )
				vkDestroyShaderModule( m_device, m_pbrModelFrag[variant], nullptr );
			m_pbrModelFrag[variant] = VK_NULL_HANDLE;
		}
	}
	if ( PbrModelPipeline( DynRasterState(), false ) == VK_NULL_HANDLE ||
	     PbrModelPipeline( DynRasterState(), true ) == VK_NULL_HANDLE )
	{
		ModelPbrError( outError, "vkCreateGraphicsPipelines (model PBR) failed" );
		return false;
	}
	m_pbrModelReady = true;
	return true;
}

void CVulkanContext::DestroyPbrModelPipeline()
{
	for ( std::map<uint64_t, VkPipeline> &pipelines : m_pbrModelPipelines )
	{
		for ( const auto &entry : pipelines )
		{
			if ( entry.second != VK_NULL_HANDLE )
				vkDestroyPipeline( m_device, entry.second, nullptr );
		}
		pipelines.clear();
	}
	for ( VkShaderModule &module : m_pbrModelFrag )
	{
		if ( module != VK_NULL_HANDLE )
			vkDestroyShaderModule( m_device, module, nullptr );
		module = VK_NULL_HANDLE;
	}
	if ( m_pbrModelPipelineLayout != VK_NULL_HANDLE )
		vkDestroyPipelineLayout( m_device, m_pbrModelPipelineLayout, nullptr );
	m_pbrModelPipelineLayout = VK_NULL_HANDLE;
	m_pbrModelReady = false;
}

} // namespace render_vulkan
