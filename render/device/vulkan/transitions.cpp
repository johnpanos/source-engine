//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan (RFC 0016 K1): the one table from port usages
//			to synchronization2 stage and access masks and image layouts, and
//			the format and usage-flag mappings. Every barrier the adapter emits
//			reads its scopes from ScopeOf.
//
//=============================================================================//

#include "vulkan_device.h"

#include <iterator>

namespace render::device::vulkan
{

namespace
{

constexpr VkPipelineStageFlags2 kShaders = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
                                           VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                                           VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
constexpr VkPipelineStageFlags2 kDepthTests =
    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;

// Indexed by ResourceUsage. Buffer-only usages carry GENERAL, which no image
// reaches: images never declare them (ImageUsageFlags ignores them).
constexpr UsageScope kScopes[] = {
    // kUndefined: nothing to wait for; contents may be discarded.
    { VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED },
    // kSampled
    { kShaders, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
    // kStorageRead
    { kShaders, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_GENERAL },
    // kStorageWrite
    { kShaders, VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
        VK_IMAGE_LAYOUT_GENERAL },
    // kColorAttachment
    { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
    // kDepthRead: a read-only depth attachment that shaders may also sample.
    { kDepthTests | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
    // kDepthWrite
    { kDepthTests,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
    // kResolveDestination: dynamic rendering resolves in the color output stage.
    { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
    // kCopySource
    { VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
    // kCopyDestination (copies, uploads and clears)
    { VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL },
    // kPresent: the presentation engine's semaphores order it.
    { VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
    // kVertex
    { VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
        VK_IMAGE_LAYOUT_GENERAL },
    // kIndex
    { VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT, VK_IMAGE_LAYOUT_GENERAL },
    // kIndirect
    { VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,
        VK_IMAGE_LAYOUT_GENERAL },
    // kUniform
    { kShaders, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_GENERAL },
    // kExternal: another API reads the memory after the token completes; the
    // GENERAL layout is the one linear image memory is defined in.
    { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_GENERAL },
};
static_assert( std::size( kScopes ) == static_cast<std::size_t>( ResourceUsage::kCount ),
    "one scope per usage" );

} // namespace

DeviceStatus StatusOf( VkResult result )
{
	switch ( result )
	{
	case VK_ERROR_OUT_OF_HOST_MEMORY:
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
	case VK_ERROR_OUT_OF_POOL_MEMORY:
	case VK_ERROR_FRAGMENTED_POOL:
		return DeviceStatus::kOutOfMemory;
	case VK_ERROR_DEVICE_LOST:
		return DeviceStatus::kDeviceLost;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
	case VK_ERROR_FEATURE_NOT_PRESENT:
	case VK_ERROR_EXTENSION_NOT_PRESENT:
	case VK_ERROR_LAYER_NOT_PRESENT:
		return DeviceStatus::kUnsupported;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
	case VK_ERROR_INITIALIZATION_FAILED:
		return DeviceStatus::kUnavailable;
	default:
		return DeviceStatus::kInternal;
	}
}

const UsageScope &ScopeOf( ResourceUsage usage )
{
	const std::size_t index = static_cast<std::size_t>( usage );
	return kScopes[index < std::size( kScopes ) ? index : 0];
}

VkBufferUsageFlags BufferUsageFlags( UsageSet usages )
{
	VkBufferUsageFlags flags = 0;
	if ( usages.Has( ResourceUsage::kCopySource ) )
		flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if ( usages.Has( ResourceUsage::kCopyDestination ) )
		flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if ( usages.Has( ResourceUsage::kVertex ) )
		flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if ( usages.Has( ResourceUsage::kIndex ) )
		flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if ( usages.Has( ResourceUsage::kIndirect ) )
		flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	if ( usages.Has( ResourceUsage::kUniform ) )
		flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if ( usages.Has( ResourceUsage::kStorageRead ) || usages.Has( ResourceUsage::kStorageWrite ) )
		flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	// A buffer that declares only image usages still needs a valid usage.
	if ( flags == 0 )
		flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	return flags;
}

VkImageUsageFlags ImageUsageFlags( UsageSet usages )
{
	VkImageUsageFlags flags = 0;
	if ( usages.Has( ResourceUsage::kSampled ) )
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if ( usages.Has( ResourceUsage::kStorageRead ) || usages.Has( ResourceUsage::kStorageWrite ) )
		flags |= VK_IMAGE_USAGE_STORAGE_BIT;
	if ( usages.Has( ResourceUsage::kColorAttachment ) ||
	     usages.Has( ResourceUsage::kResolveDestination ) )
		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if ( usages.Has( ResourceUsage::kDepthRead ) || usages.Has( ResourceUsage::kDepthWrite ) )
		flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if ( usages.Has( ResourceUsage::kCopySource ) )
		flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if ( usages.Has( ResourceUsage::kCopyDestination ) )
		flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	return flags;
}

VkFormat ToVkFormat( Format format )
{
	switch ( format )
	{
	case Format::kR8Unorm:
		return VK_FORMAT_R8_UNORM;
	case Format::kRGBA8Unorm:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case Format::kRGBA8Srgb:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case Format::kBGRA8Unorm:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case Format::kBGRA8Srgb:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case Format::kRG16Float:
		return VK_FORMAT_R16G16_SFLOAT;
	case Format::kRGB10A2Unorm:
		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case Format::kRGBA16Float:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case Format::kR32Float:
		return VK_FORMAT_R32_SFLOAT;
	case Format::kRGBA32Float:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case Format::kD32Float:
		return VK_FORMAT_D32_SFLOAT;
	case Format::kD24UnormS8:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case Format::kD32FloatS8:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	case Format::kRGBA16Unorm:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case Format::kBC1Unorm:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK; // D3D9's DXT1 keeps its one-bit alpha
	case Format::kBC1Srgb:
		return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
	case Format::kBC2Unorm:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case Format::kBC2Srgb:
		return VK_FORMAT_BC2_SRGB_BLOCK;
	case Format::kBC3Unorm:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case Format::kBC3Srgb:
		return VK_FORMAT_BC3_SRGB_BLOCK;
	case Format::kBC4Unorm:
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case Format::kBC5Unorm:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case Format::kUnknown:
	case Format::kCount:
		break;
	}
	return VK_FORMAT_UNDEFINED;
}

VkImageAspectFlags BarrierAspects( Format format )
{
	if ( HasStencil( format ) )
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	return IsDepthFormat( format ) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageAspectFlags CopyAspect( Format format )
{
	return IsDepthFormat( format ) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
}

} // namespace render::device::vulkan
