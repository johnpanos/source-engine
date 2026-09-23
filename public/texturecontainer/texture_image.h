//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Owned, container-neutral image data for strict C++20 consumers.
//
//=============================================================================//

#ifndef TEXTURECONTAINER_TEXTURE_IMAGE_H
#define TEXTURECONTAINER_TEXTURE_IMAGE_H

#include "foundation/expected.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace texturecontainer
{

enum class PixelFormat
{
	Rgba8Unorm,
	Rgba8Srgb,
	Bgra8Unorm,
	Bgra8Srgb,
	R8Unorm,
	Rg8Unorm,
	Rgba16Float,
	Rgb9e5Float,
	Bc1Unorm,
	Bc1Srgb,
	Bc2Unorm,
	Bc2Srgb,
	Bc3Unorm,
	Bc3Srgb,
	Bc4Unorm,
	Bc5Unorm,
	Bc6hUfloat,
	Bc7Srgb,
	Astc4x4Srgb,
	Astc4x4Float,
	Etc2RgbaSrgb,
	EacR11Unorm,
	EacRg11Unorm,
};

struct BlockLayout
{
	std::uint32_t width;
	std::uint32_t height;
	std::uint32_t bytes;
};

// One owner for the byte layout shared by container validation and backend
// upload. Invalid enum values have a zero layout and must be rejected.
constexpr BlockLayout LayoutForPixelFormat( PixelFormat format ) noexcept
{
	switch ( format )
	{
	case PixelFormat::R8Unorm:
		return { 1, 1, 1 };
	case PixelFormat::Rg8Unorm:
		return { 1, 1, 2 };
	case PixelFormat::Rgba16Float:
		return { 1, 1, 8 };
	case PixelFormat::Rgba8Unorm:
	case PixelFormat::Rgba8Srgb:
	case PixelFormat::Bgra8Unorm:
	case PixelFormat::Bgra8Srgb:
	case PixelFormat::Rgb9e5Float:
		return { 1, 1, 4 };
	case PixelFormat::Bc1Unorm:
	case PixelFormat::Bc1Srgb:
	case PixelFormat::Bc4Unorm:
	case PixelFormat::EacR11Unorm:
		return { 4, 4, 8 };
	case PixelFormat::Bc2Unorm:
	case PixelFormat::Bc2Srgb:
	case PixelFormat::Bc3Unorm:
	case PixelFormat::Bc3Srgb:
	case PixelFormat::Bc5Unorm:
	case PixelFormat::Bc6hUfloat:
	case PixelFormat::Bc7Srgb:
	case PixelFormat::Astc4x4Srgb:
	case PixelFormat::Astc4x4Float:
	case PixelFormat::Etc2RgbaSrgb:
	case PixelFormat::EacRg11Unorm:
		return { 4, 4, 16 };
	}
	return { 0, 0, 0 };
}

struct ImageLevel
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::byte> bytes;
};

struct TextureImage
{
	PixelFormat format;
	std::vector<ImageLevel> levels;
};

enum class ReadError
{
	InvalidContainer,
	UnsupportedFormat,
	UnsupportedTopology,
	UnsupportedOrientation,
	UnsupportedSupercompression,
	TooLarge,
	InvalidImageSize,
};

// The input is borrowed for this call only. On success all mip bytes are owned
// by the result. Packaged KTX2 data must already be transcoded for the target
// profile; UASTC masters are not a runtime texture format.
[[nodiscard]] foundation::Expected<TextureImage, ReadError> ReadKtx2Image(
    std::span<const std::byte> encoded );

} // namespace texturecontainer

#endif // TEXTURECONTAINER_TEXTURE_IMAGE_H
