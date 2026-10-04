//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 value helpers (RFC 0016).
//
//=============================================================================//

#include "render/device/errors.h"
#include "render/device/facts.h"
#include "render/device/resources.h"

namespace render::device
{

const char *DescribeStatus( DeviceStatus status )
{
	switch ( status )
	{
	case DeviceStatus::kInvalidDescription:
		return "invalid description";
	case DeviceStatus::kUnsupported:
		return "unsupported";
	case DeviceStatus::kOutOfMemory:
		return "out of memory";
	case DeviceStatus::kDeviceLost:
		return "device lost";
	case DeviceStatus::kStaleEpoch:
		return "stale epoch";
	case DeviceStatus::kTooManyBindGroups:
		return "too many bind groups";
	case DeviceStatus::kLayoutMismatch:
		return "layout mismatch";
	case DeviceStatus::kInvalidHandle:
		return "invalid handle";
	case DeviceStatus::kInvalidState:
		return "invalid state";
	case DeviceStatus::kUnavailable:
		return "unavailable";
	case DeviceStatus::kInternal:
		return "internal";
	}
	return "unknown";
}

const char *DescribeOperation( DeviceOperation operation )
{
	switch ( operation )
	{
	case DeviceOperation::kCreateDevice:
		return "CreateDevice";
	case DeviceOperation::kCreateBuffer:
		return "CreateBuffer";
	case DeviceOperation::kCreateTexture:
		return "CreateTexture";
	case DeviceOperation::kCreateSampler:
		return "CreateSampler";
	case DeviceOperation::kCreateBindGroupLayout:
		return "CreateBindGroupLayout";
	case DeviceOperation::kCreateBindGroup:
		return "CreateBindGroup";
	case DeviceOperation::kCreatePipeline:
		return "CreatePipeline";
	case DeviceOperation::kBeginEncoder:
		return "BeginEncoder";
	case DeviceOperation::kSubmit:
		return "Submit";
	case DeviceOperation::kReadBuffer:
		return "ReadBuffer";
	case DeviceOperation::kRelease:
		return "Release";
	case DeviceOperation::kExportTexture:
		return "ExportTexture";
	}
	return "unknown";
}

const char *CapabilityName( Capability capability )
{
	switch ( capability )
	{
	case Capability::kCompute:
		return "compute";
	case Capability::kStorageBuffers:
		return "storage-buffers";
	case Capability::kTransientAliasing:
		return "transient-aliasing";
	case Capability::kParallelRecording:
		return "parallel-recording";
	case Capability::kAsyncCompute:
		return "async-compute";
	case Capability::kAsyncTransfer:
		return "async-transfer";
	case Capability::kRayQuery:
		return "ray-query";
	case Capability::kExternalImages:
		return "external-images";
	case Capability::kTextureCompressionBC:
		return "texture-compression-bc";
	case Capability::kTimestamps:
		return "timestamps";
	case Capability::kCount:
		break;
	}
	return "unknown";
}

std::optional<Capability> FirstMissing( CapabilitySet have, CapabilitySet required )
{
	for ( std::uint32_t i = 0; i < static_cast<std::uint32_t>( Capability::kCount ); ++i )
	{
		const Capability capability = static_cast<Capability>( i );
		if ( required.Has( capability ) && !have.Has( capability ) )
			return capability;
	}
	return std::nullopt;
}

std::uint32_t BytesPerTexel( Format format )
{
	switch ( format )
	{
	case Format::kR8Unorm:
		return 1;
	case Format::kRGB10A2Unorm:
	case Format::kRGBA8Unorm:
	case Format::kRGBA8Srgb:
	case Format::kBGRA8Unorm:
	case Format::kBGRA8Srgb:
	case Format::kRG16Float:
	case Format::kR32Float:
	case Format::kD32Float:
	case Format::kD24UnormS8:
	case Format::kD32FloatS8: // the depth aspect, as copies address it
		return 4;
	case Format::kRGBA16Float:
	case Format::kRGBA16Unorm:
		return 8;
	case Format::kRGBA32Float:
		return 16;
	case Format::kBC1Unorm: // blocks: BlockOf
	case Format::kBC1Srgb:
	case Format::kBC2Unorm:
	case Format::kBC2Srgb:
	case Format::kBC3Unorm:
	case Format::kBC3Srgb:
	case Format::kBC4Unorm:
	case Format::kBC5Unorm:
	case Format::kUnknown:
	case Format::kCount:
		break;
	}
	return 0;
}

bool IsDepthFormat( Format format )
{
	return format == Format::kD32Float || HasStencil( format );
}

bool IsBlockCompressed( Format format )
{
	return format >= Format::kBC1Unorm && format <= Format::kBC5Unorm;
}

FormatBlock BlockOf( Format format )
{
	switch ( format )
	{
	case Format::kBC1Unorm:
	case Format::kBC1Srgb:
	case Format::kBC4Unorm:
		return { 4, 4, 8 };
	case Format::kBC2Unorm:
	case Format::kBC2Srgb:
	case Format::kBC3Unorm:
	case Format::kBC3Srgb:
	case Format::kBC5Unorm:
		return { 4, 4, 16 };
	default:
		return { 1, 1, BytesPerTexel( format ) };
	}
}

std::uint64_t RegionBytes( Format format, std::uint32_t width, std::uint32_t height )
{
	const FormatBlock block = BlockOf( format );
	const std::uint64_t across = ( std::uint64_t( width ) + block.width - 1 ) / block.width;
	const std::uint64_t down = ( std::uint64_t( height ) + block.height - 1 ) / block.height;
	return across * down * block.bytes;
}

bool CopyRegionAligned( Format format, std::uint32_t mipWidth, std::uint32_t mipHeight,
    std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height,
    std::uint64_t bufferOffset )
{
	const FormatBlock block = BlockOf( format );
	if ( block.bytes == 0 || bufferOffset % block.bytes != 0 || x > mipWidth ||
	     width > mipWidth - x || y > mipHeight || height > mipHeight - y || x % block.width != 0 ||
	     y % block.height != 0 )
		return false;
	return ( width % block.width == 0 || x + width == mipWidth ) &&
	       ( height % block.height == 0 || y + height == mipHeight );
}

bool HasStencil( Format format )
{
	return format == Format::kD24UnormS8 || format == Format::kD32FloatS8;
}

} // namespace render::device
