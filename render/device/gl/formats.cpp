//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl: port formats and texture targets in GL.
//
//=============================================================================//

#include "gl_device.h"

namespace render::device::gl
{

foundation::Unexpected<DeviceError> Fail(
    DeviceStatus status, DeviceOperation operation, std::int32_t nativeCode )
{
	return foundation::MakeUnexpected( DeviceError{ status, operation, nativeCode } );
}

GlFormat FormatOf( Format format )
{
	switch ( format )
	{
	case Format::kR8Unorm:
		return { GL_R8, GL_RED, GL_UNSIGNED_BYTE, false };
	case Format::kRGBA8Unorm:
		return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, false };
	case Format::kRGBA8Srgb:
		return { GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, false };
	// GL has no BGRA storage: RGBA storage, BGRA transfers.
	case Format::kBGRA8Unorm:
		return { GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE, false };
	case Format::kBGRA8Srgb:
		return { GL_SRGB8_ALPHA8, GL_BGRA, GL_UNSIGNED_BYTE, false };
	case Format::kRG16Float:
		return { GL_RG16F, GL_RG, GL_HALF_FLOAT, false };
	case Format::kRGB10A2Unorm:
		return { GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, false };
	case Format::kRGBA16Float:
		return { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, false };
	case Format::kR32Float:
		return { GL_R32F, GL_RED, GL_FLOAT, false };
	case Format::kRGBA32Float:
		return { GL_RGBA32F, GL_RGBA, GL_FLOAT, false };
	case Format::kD32Float:
		return { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, false };
	// Copies address the depth aspect (render/device/port.cpp BytesPerTexel).
	// D24's transfers take no 32-bit form equal to the port's, so the adapter
	// refuses its copies (execute.cpp).
	case Format::kD24UnormS8:
		return { GL_DEPTH24_STENCIL8, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, false };
	case Format::kD32FloatS8:
		return { GL_DEPTH32F_STENCIL8, GL_DEPTH_COMPONENT, GL_FLOAT, false };
	case Format::kRGBA16Unorm:
		return { GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT, false };
	case Format::kBC1Unorm:
		return { GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0, true };
	case Format::kBC1Srgb:
		return { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0, 0, true };
	case Format::kBC2Unorm:
		return { GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, 0, true };
	case Format::kBC2Srgb:
		return { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0, 0, true };
	case Format::kBC3Unorm:
		return { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0, true };
	case Format::kBC3Srgb:
		return { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0, 0, true };
	case Format::kBC4Unorm:
		return { GL_COMPRESSED_RED_RGTC1, 0, 0, true };
	case Format::kBC5Unorm:
		return { GL_COMPRESSED_RG_RGTC2, 0, 0, true };
	case Format::kUnknown:
	case Format::kCount:
		break;
	}
	return {};
}

GLenum TextureTarget( const TextureDesc &desc )
{
	switch ( desc.dimension )
	{
	case TextureDimension::k3D:
		return GL_TEXTURE_3D;
	case TextureDimension::kCube:
		return desc.depthOrLayers > 6 ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
	case TextureDimension::k2D:
		break;
	}
	if ( desc.sampleCount > 1 )
		return desc.depthOrLayers > 1 ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_MULTISAMPLE;
	return desc.depthOrLayers > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
}

} // namespace render::device::gl
