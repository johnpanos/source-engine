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
	if ( m_sceneColorHandle >= 0 )
	{
		const ManagedTexture &color = m_managedTextures[static_cast<size_t>( m_sceneColorHandle )];
		if ( color.width == width && color.height == height )
			return true;
		DestroySceneCapture();
	}

	// Color: the back buffer's format, so a blit copies it unconverted, with
	// its sRGB view (glass reads linear light) and a full mip chain made by
	// linear blits.
	VkFormatProperties colorProps = {};
	vkGetPhysicalDeviceFormatProperties( m_physicalDevice, m_swapFormat, &colorProps );
	const VkFormatFeatureFlags blits = VK_FORMAT_FEATURE_BLIT_SRC_BIT |
	                                   VK_FORMAT_FEATURE_BLIT_DST_BIT |
	                                   VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
	if ( ( colorProps.optimalTilingFeatures & blits ) != blits )
	{
		CaptureError( outError, "back buffer format cannot be blitted into a filtered mip chain" );
		return false;
	}
	const int color = CreateManagedTexture( static_cast<int>( width ), static_cast<int>( height ),
	    m_swapFormat, outError, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, UINT32_MAX,
	    m_srgbAttachments ? m_swapFormatSrgb : VK_FORMAT_UNDEFINED );
	if ( color < 0 )
		return false;
	m_sceneColorHandle = color;
	NameManagedTexture( color, "scene capture color" );
	SetManagedTextureSamplerState(
	    color, kSamplerClampU | kSamplerClampV | kSamplerLinear | kSamplerMipLinear );
	ManagedTexture &colorTexture = m_managedTextures[static_cast<size_t>( color )];
	if ( colorTexture.descSet == VK_NULL_HANDLE )
	{
		DestroySceneCapture();
		CaptureError( outError, "no descriptor set for the scene color capture" );
		return false;
	}
	colorTexture.uploaded = true;
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
		// Glass still works without depth; only foreground rejection is lost.
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
	if ( m_sceneColorHandle >= 0 )
		DestroyManagedTexture( m_sceneColorHandle );
	if ( m_sceneDepthHandle >= 0 )
		DestroyManagedTexture( m_sceneDepthHandle );
	m_sceneColorHandle = -1;
	m_sceneDepthHandle = -1;
	m_sceneCaptureCurrent = false;
}

bool CVulkanContext::RecordSceneCapture( VkCommandBuffer cmd, int target )
{
	ScopedDebugLabel label( m_debugUtils, "scene capture (glass)" );
	if ( m_sceneColorHandle < 0 )
		return false;
	if ( m_gpuTimerPool != VK_NULL_HANDLE )
		GpuTimerMark( cmd, "scene capture (glass)" );
	ManagedTexture &color = m_managedTextures[static_cast<size_t>( m_sceneColorHandle )];
	uint32_t targetW = 0, targetH = 0;
	GetTargetExtent( target, &targetW, &targetH );
	const uint32_t width = std::min( targetW, color.width );
	const uint32_t height = std::min( targetH, color.height );
	if ( width == 0 || height == 0 )
		return false;
	++m_lastFrameSceneCaptures;
	m_frameCost.Add( kCostTargetCopy, 0 );

	// Color: the target's pixels into mip 0, then each level from the one above.
	const bool srcIsTexture = IsRenderTargetTexture( target );
	const VkImage srcImage = srcIsTexture ? m_managedTextures[static_cast<size_t>( target )].image
	                                      : m_swapImages[m_acquiredImage];
	const VkImageLayout srcRest = srcIsTexture ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	                                           : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkImageMemoryBarrier in[2] = {
	    ImageBarrier( srcImage, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, srcRest,
	        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	        VK_ACCESS_TRANSFER_READ_BIT ),
	    ImageBarrier( color.image, VK_IMAGE_ASPECT_COLOR_BIT, 0, color.mipLevels,
	        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	        VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT ) };
	vkCmdPipelineBarrier( cmd,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, in );
	VkImageBlit copy = {};
	copy.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.srcOffsets[1] = { static_cast<int32_t>( width ), static_cast<int32_t>( height ), 1 };
	copy.dstOffsets[1] = copy.srcOffsets[1];
	vkCmdBlitImage( cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, color.image,
	    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy, VK_FILTER_NEAREST );
	VkImageMemoryBarrier srcBack = ImageBarrier( srcImage, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
	    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcRest, VK_ACCESS_TRANSFER_READ_BIT,
	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT );
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
	    nullptr, 0, nullptr, 1, &srcBack );
	int32_t levelW = static_cast<int32_t>( width );
	int32_t levelH = static_cast<int32_t>( height );
	for ( uint32_t level = 1; level < color.mipLevels; ++level )
	{
		VkImageMemoryBarrier above =
		    ImageBarrier( color.image, VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 1,
		        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT );
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    0, 0, nullptr, 0, nullptr, 1, &above );
		VkImageBlit down = {};
		down.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 0, 1 };
		down.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1 };
		down.srcOffsets[1] = { levelW, levelH, 1 };
		levelW = std::max( 1, levelW / 2 );
		levelH = std::max( 1, levelH / 2 );
		down.dstOffsets[1] = { levelW, levelH, 1 };
		vkCmdBlitImage( cmd, color.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, color.image,
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &down, VK_FILTER_LINEAR );
	}
	VkImageMemoryBarrier sampled[2] = {
	    ImageBarrier( color.image, VK_IMAGE_ASPECT_COLOR_BIT, 0, color.mipLevels - 1,
	        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	        VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT ),
	    ImageBarrier( color.image, VK_IMAGE_ASPECT_COLOR_BIT, color.mipLevels - 1, 1,
	        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT ) };
	const uint32_t firstSampled = color.mipLevels > 1 ? 0 : 1;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 2 - firstSampled,
	    sampled + firstSampled );

	m_sceneDepthCaptured = RecordSceneDepthCopy( cmd, target, width, height );
	return m_sceneDepthCaptured;
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

bool CVulkanContext::ReadSceneDepth(
    std::vector<float> *outDepth, uint32_t *outWidth, uint32_t *outHeight, std::string *outError )
{
	if ( !outDepth || !IsValid() || m_sceneDepthHandle < 0 || !m_sceneDepthCaptured )
	{
		CaptureError( outError, "no scene depth has been captured" );
		return false;
	}
	const ManagedTexture &depth = m_managedTextures[static_cast<size_t>( m_sceneDepthHandle )];
	// The depth aspect copies out as 32-bit floats (D32_SFLOAT and
	// D32_SFLOAT_S8_UINT), 32-bit words holding 24-bit UNORM depth in the low
	// bits (D24_UNORM_S8_UINT, X8_D24_UNORM_PACK32) or 16-bit UNORM (D16 and
	// D16_UNORM_S8_UINT); readback returns depth in [0, 1] for each.
	enum DepthWords
	{
		kFloat32,
		kUnorm24,
		kUnorm16
	};
	DepthWords words;
	switch ( m_depthFormat )
	{
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		words = kFloat32;
		break;
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		words = kUnorm24;
		break;
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D16_UNORM_S8_UINT:
		words = kUnorm16;
		break;
	default:
		CaptureError( outError, "scene depth readback does not know this depth format" );
		return false;
	}
	// The copy below follows every submitted frame on the queue, and its
	// barrier chains after their last depth write (fragment stage); only its
	// own completion is waited for.
	const VkDeviceSize texelBytes = words == kUnorm16 ? 2 : 4;
	const VkDeviceSize bytes = VkDeviceSize( depth.width ) * depth.height * texelBytes;
	VkBuffer buffer = VK_NULL_HANDLE;
	VulkanMemory memory = VK_NULL_HANDLE;
	if ( !CreateBuffer( bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer,
	         &memory, outError ) )
		return false;
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	bool ok = BeginSingleTimeCommands( &cmd, outError );
	if ( ok )
	{
		VkImageMemoryBarrier toSrc = ImageBarrier( depth.image, m_depthAspects, 0, 1,
		    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		    VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT );
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toSrc );
		VkBufferImageCopy region = {};
		region.imageSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
		region.imageExtent = { depth.width, depth.height, 1 };
		vkCmdCopyImageToBuffer(
		    cmd, depth.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region );
		VkImageMemoryBarrier back = ImageBarrier( depth.image, m_depthAspects, 0, 1,
		    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		    VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT );
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &back );
		uint64_t copied = 0;
		ok = EndSingleTimeCommands( cmd, outError, &copied ) &&
		     m_hostDevice->WaitValue( copied, UINT64_MAX ) == VK_SUCCESS;
	}
	void *mapped = nullptr;
	if ( ok && MapMemory( memory, &mapped ) == VK_SUCCESS )
	{
		const size_t count = size_t( depth.width ) * depth.height;
		outDepth->resize( count );
		if ( words == kFloat32 )
		{
			const float *texels = static_cast<const float *>( mapped );
			outDepth->assign( texels, texels + count );
		}
		else if ( words == kUnorm24 )
		{
			const uint32_t *texels = static_cast<const uint32_t *>( mapped );
			for ( size_t i = 0; i < count; ++i )
				( *outDepth )[i] = float( texels[i] & 0xffffffu ) / float( 0xffffffu );
		}
		else
		{
			const uint16_t *texels = static_cast<const uint16_t *>( mapped );
			for ( size_t i = 0; i < count; ++i )
				( *outDepth )[i] = float( texels[i] ) / 65535.0f;
		}
		UnmapMemory( memory );
		if ( outWidth )
			*outWidth = depth.width;
		if ( outHeight )
			*outHeight = depth.height;
	}
	else
		ok = false;
	vkDestroyBuffer( m_device, buffer, nullptr );
	FreeMemory( memory );
	return ok;
}

} // namespace render_vulkan
