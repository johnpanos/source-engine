//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.d3d12: port formats and usages in D3D12 terms.
//
//=============================================================================//

#include "d3d12_device.h"

namespace render::device::d3d12
{

foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode )
{
	return foundation::MakeUnexpected( DeviceError{ status, operation, nativeCode } );
}

namespace
{

DxFormat Same( DXGI_FORMAT format )
{
	return { format, format, format, format };
}

} // namespace

DxFormat FormatOf( Format format )
{
	switch ( format )
	{
	case Format::kR8Unorm:
		return Same( DXGI_FORMAT_R8_UNORM );
	case Format::kRGBA8Unorm:
		return Same( DXGI_FORMAT_R8G8B8A8_UNORM );
	case Format::kRGBA8Srgb:
		return Same( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB );
	case Format::kBGRA8Unorm:
		return Same( DXGI_FORMAT_B8G8R8A8_UNORM );
	case Format::kBGRA8Srgb:
		return Same( DXGI_FORMAT_B8G8R8A8_UNORM_SRGB );
	case Format::kRG16Float:
		return Same( DXGI_FORMAT_R16G16_FLOAT );
	case Format::kRGBA16Float:
		return Same( DXGI_FORMAT_R16G16B16A16_FLOAT );
	case Format::kR32Float:
		return Same( DXGI_FORMAT_R32_FLOAT );
	case Format::kRGBA32Float:
		return Same( DXGI_FORMAT_R32G32B32A32_FLOAT );
	case Format::kD32Float:
		return { DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32_TYPELESS };
	case Format::kD24UnormS8:
		return { DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT,
			DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_UNKNOWN };
	case Format::kD32FloatS8:
		return { DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
			DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_R32_TYPELESS };
	case Format::kRGBA16Unorm:
		return Same( DXGI_FORMAT_R16G16B16A16_UNORM );
	case Format::kBC1Unorm:
		return Same( DXGI_FORMAT_BC1_UNORM );
	case Format::kBC1Srgb:
		return Same( DXGI_FORMAT_BC1_UNORM_SRGB );
	case Format::kBC2Unorm:
		return Same( DXGI_FORMAT_BC2_UNORM );
	case Format::kBC2Srgb:
		return Same( DXGI_FORMAT_BC2_UNORM_SRGB );
	case Format::kBC3Unorm:
		return Same( DXGI_FORMAT_BC3_UNORM );
	case Format::kBC3Srgb:
		return Same( DXGI_FORMAT_BC3_UNORM_SRGB );
	case Format::kBC4Unorm:
		return Same( DXGI_FORMAT_BC4_UNORM );
	case Format::kBC5Unorm:
		return Same( DXGI_FORMAT_BC5_UNORM );
	case Format::kBC6HUfloat:
		return Same( DXGI_FORMAT_BC6H_UF16 );
	case Format::kBC7Unorm:
		return Same( DXGI_FORMAT_BC7_UNORM );
	case Format::kBC7Srgb:
		return Same( DXGI_FORMAT_BC7_UNORM_SRGB );
	case Format::kRGB10A2Unorm:
		return Same( DXGI_FORMAT_R10G10B10A2_UNORM );
	case Format::kRG11B10Float:
		return Same( DXGI_FORMAT_R11G11B10_FLOAT );
	case Format::kETC1Rgb: // D40: not claimed
	case Format::kETC1A4:
	case Format::kRGBA4Unorm: // D42: not claimed
	case Format::kUnknown:
	case Format::kCount:
		break;
	}
	return {};
}

D3D12_RESOURCE_STATES StateOf( ResourceUsage usage, bool depth )
{
	switch ( usage )
	{
	case ResourceUsage::kSampled:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
		       D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case ResourceUsage::kStorageRead:
	case ResourceUsage::kStorageWrite:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case ResourceUsage::kColorAttachment:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case ResourceUsage::kDepthRead:
		return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
		       D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case ResourceUsage::kDepthWrite:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case ResourceUsage::kResolveDestination:
		return D3D12_RESOURCE_STATE_RESOLVE_DEST;
	case ResourceUsage::kCopySource:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case ResourceUsage::kCopyDestination:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case ResourceUsage::kVertex:
	case ResourceUsage::kUniform:
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case ResourceUsage::kIndex:
		return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	case ResourceUsage::kIndirect:
		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case ResourceUsage::kUndefined:
	case ResourceUsage::kPresent:
	case ResourceUsage::kExternal:
	case ResourceUsage::kCount:
		break;
	}
	(void)depth;
	return D3D12_RESOURCE_STATE_COMMON;
}

} // namespace render::device::d3d12
