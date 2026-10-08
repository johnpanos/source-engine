//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan scene capture: the opaque scene's color (with a mip
//          chain for rough refraction) and its depth, copied from the open
//          target into sampled images before glass draws read them.
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

void CaptureLog( const char *fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	std::fprintf( stderr, "[NativeVulkan] " );
	std::vfprintf( stderr, fmt, args );
	va_end( args );
}

void CaptureError( std::string *outError, const char *message )
{
	if ( outError )
		*outError = message;
}

VkImageMemoryBarrier ImageBarrier( VkImage image, VkImageAspectFlags aspects, uint32_t firstLevel,
    uint32_t levels, VkImageLayout from, VkImageLayout to, VkAccessFlags srcAccess,
    VkAccessFlags dstAccess )
{
	VkImageMemoryBarrier b = {};
	b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.image = image;
	b.subresourceRange = { aspects, firstLevel, levels, 0, 1 };
	b.oldLayout = from;
	b.newLayout = to;
	b.srcAccessMask = srcAccess;
	b.dstAccessMask = dstAccess;
	return b;
}

} // namespace

bool CVulkanContext::EnsureSceneCapture( std::string *outError )
{
	const uint32_t width = m_swapExtent.width;
	const uint32_t height = m_swapExtent.height;
	if ( !IsValid() || width == 0 || height == 0 )
	{
		CaptureError( outError, "scene capture needs a back buffer" );
		return false;
	}
	if ( m_sceneDepthHandle >= 0 )
	{
		const ManagedTexture &depth = m_managedTextures[static_cast<size_t>( m_sceneDepthHandle )];
		if ( depth.width == width && depth.height == height )
			return true;
		DestroySceneCapture();
	}
	if ( !m_sceneDepthUsable )
		return true;

	// Depth: the attachments' format, copied aspect for aspect, sampled
	// through a depth-only view without filtering.
	ManagedTexture depth;
	depth.width = width;
	depth.height = height;
	depth.format = m_depthFormat;
	depth.samplerState = kSamplerClampU | kSamplerClampV;
	VkImageCreateInfo ii = {};
	ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ii.imageType = VK_IMAGE_TYPE_2D;
	ii.format = m_depthFormat;
	ii.extent = { width, height, 1 };
	ii.mipLevels = 1;
	ii.arrayLayers = 1;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	           VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	bool ok = CreateImage( ii, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depth.image, &depth.memory,
	    "scene depth", outError );
	if ( ok )
	{
		VkImageViewCreateInfo iv = {};
		iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv.image = depth.image;
		iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv.format = m_depthFormat;
		iv.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
		ok = vkCreateImageView( m_device, &iv, nullptr, &depth.view ) == VK_SUCCESS;
	}
	// Defined contents before any capture: the far plane.
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if ( ok && BeginSingleTimeCommands( &cmd, outError ) )
	{
		VkImageMemoryBarrier toDst =
		    ImageBarrier( depth.image, m_depthAspects, 0, 1, VK_IMAGE_LAYOUT_UNDEFINED,
		        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT );
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toDst );
		const VkClearDepthStencilValue far = { 1.0f, 0 };
		vkCmdClearDepthStencilImage( cmd, depth.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &far,
		    1, &toDst.subresourceRange );
		VkImageMemoryBarrier ready = ImageBarrier( depth.image, m_depthAspects, 0, 1,
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		    VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT );
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ready );
		ok = EndSingleTimeCommands( cmd, outError );
	}
	else
		ok = false;
	if ( ok && m_dynTexDescPool != VK_NULL_HANDLE && m_liveTextureSets + 1 < kMaxManagedTexSets )
	{
		VkDescriptorSetAllocateInfo da = {};
		da.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		da.descriptorPool = m_dynTexDescPool;
		da.descriptorSetCount = 1;
		da.pSetLayouts = &m_dynTexDescLayout;
		ok = vkAllocateDescriptorSets( m_device, &da, &depth.descSet ) == VK_SUCCESS;
		if ( ok )
		{
			++m_liveTextureSets;
			VkDescriptorImageInfo image = {};
			image.sampler = m_samplers[depth.samplerState] != VK_NULL_HANDLE
			                    ? m_samplers[depth.samplerState]
			                    : m_dynTexSampler;
			image.imageView = depth.view;
			image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = depth.descSet;
			write.dstBinding = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &image;
			vkUpdateDescriptorSets( m_device, 1, &write, 0, nullptr );
		}
	}
	else
		ok = false;
	if ( !ok )
	{
		// Without it the depth-to-alpha copy is skipped (soft particles lose
		// their depth feathering).
		ReleaseManagedTextureObjects( depth );
		CaptureLog( "scene depth capture unavailable: %s\n",
		    outError && !outError->empty() ? outError->c_str() : "image or descriptor set" );
		m_sceneDepthUsable = false;
		return true;
	}
	depth.uploaded = true;
	m_sceneDepthHandle = StoreManagedTexture( depth );
	NameManagedTexture( m_sceneDepthHandle, "scene capture depth" );
	return true;
}

void CVulkanContext::DestroySceneCapture()
{
	// Retired, not destroyed: submitted frames may still sample them.
	if ( m_sceneDepthHandle >= 0 )
		DestroyManagedTexture( m_sceneDepthHandle );
	m_sceneDepthHandle = -1;
}

bool CVulkanContext::RecordSceneDepthCopy(
    VkCommandBuffer cmd, int target, uint32_t width, uint32_t height )
{
	// Depth, when this target's depth is single-sampled. A multisampled back
	// buffer's depth cannot be copied into a single-sampled image.
	if ( m_sceneDepthHandle < 0 || !m_sceneDepthEnabled || ( target == -1 && m_activeSamples > 1 ) )
		return false;
	const bool srcIsTexture = IsRenderTargetTexture( target );
	const VkImage depthSrc = srcIsTexture
	                             ? m_managedTextures[static_cast<size_t>( target )].depthImage
	                             : m_depthImages[m_acquiredImage];
	const ManagedTexture &depth = m_managedTextures[static_cast<size_t>( m_sceneDepthHandle )];
	if ( depthSrc == VK_NULL_HANDLE )
		return false;
	width = std::min( width, depth.width );
	height = std::min( height, depth.height );
	VkImageMemoryBarrier depthIn[2] = {
	    ImageBarrier( depthSrc, m_depthAspects, 0, 1,
	        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT ),
	    ImageBarrier( depth.image, m_depthAspects, 0, 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_SHADER_READ_BIT,
	        VK_ACCESS_TRANSFER_WRITE_BIT ) };
	vkCmdPipelineBarrier( cmd,
	    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
	        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, depthIn );
	VkImageCopy region = {};
	region.srcSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
	region.dstSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
	region.extent = { width, height, 1 };
	vkCmdCopyImage( cmd, depthSrc, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, depth.image,
	    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
	VkImageMemoryBarrier depthOut[2] = {
	    ImageBarrier( depthSrc, m_depthAspects, 0, 1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT,
	        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
	            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ),
	    ImageBarrier( depth.image, m_depthAspects, 0, 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
	        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT ) };
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
	        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
	    0, 0, nullptr, 0, nullptr, 2, depthOut );
	++m_lastFrameSceneDepthCaptures;
	return true;
}

} // namespace render_vulkan
