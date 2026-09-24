//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan PBRMetalRough pipeline for dynamic meshes (models and
//          props). It shares the skin pipeline's layout, vertex input and
//          vertex stage; shaders/model_pbr.frag evaluates the RFC 0007 layered
//          BRDF under Source's ambient cube and local lights.
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
    const DynRasterState &state, bool envCube, bool srgbPass, int samples )
{
	// The indirect view has no $envmap variant: it never reads set 5.
	const int variant = m_indirectViewMode != 0 ? 2 : envCube ? 1 : 0;
	const uint64_t key = PipelineKey( state, srgbPass, samples );
	const VkRenderPass pass = PipelineRenderPass( srgbPass, samples );
	std::map<uint64_t, VkPipeline> &pipelines = m_pbrModelPipelines[variant];
	const auto existing = pipelines.find( key );
	if ( existing != pipelines.end() )
		return existing->second;
	if ( pass == VK_NULL_HANDLE || m_skinVert == VK_NULL_HANDLE ||
	     m_pbrModelFrag[variant] == VK_NULL_HANDLE )
		return VK_NULL_HANDLE;
	VkPipeline pipeline = BuildMaterialPipeline( state, m_skinVert, m_pbrModelFrag[variant],
	    m_skinPipelineLayout, &m_skinVin, pass, samples );
	if ( pipeline == VK_NULL_HANDLE )
		ModelPbrLog( "vkCreateGraphicsPipelines (model PBR%s, state %#llx) failed\n",
		    variant == 2 ? " indirect view" : envCube ? " envmap" : "",
		    static_cast<unsigned long long>( key ) );
	pipelines[key] = pipeline;
	if ( pipeline != VK_NULL_HANDLE && variant != 2 )
		NotePipelineVariant( envCube ? kPipelinePbrModelEnv : kPipelinePbrModel, key );
	return pipeline;
}

// Needs the skin layout (seven sets and its push block) and the split-sum
// table of the PBR direct pipeline. Without either, model draws of
// PBRMetalRough are declined by name.
bool CVulkanContext::InitPbrModelPipeline( std::string *outError )
{
	if ( m_skinPipelineLayout == VK_NULL_HANDLE || m_skinVert == VK_NULL_HANDLE )
	{
		ModelPbrError( outError, "model PBR requires the skin pipeline layout" );
		return false;
	}
	if ( m_pbrSplitSumHandle < 0 )
	{
		ModelPbrError( outError, "model PBR requires the split-sum table" );
		return false;
	}
	if ( !CreateShaderModule(
	         g_modelPbrFragSpv, sizeof( g_modelPbrFragSpv ), &m_pbrModelFrag[0], outError ) ||
	     !CreateShaderModule( g_modelPbrEnvFragSpv, sizeof( g_modelPbrEnvFragSpv ),
	         &m_pbrModelFrag[1], outError ) ||
	     !CreateShaderModule( g_modelPbrIndirectFragSpv, sizeof( g_modelPbrIndirectFragSpv ),
	         &m_pbrModelFrag[2], outError ) )
		return false;
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
	m_pbrModelReady = false;
}

} // namespace render_vulkan
