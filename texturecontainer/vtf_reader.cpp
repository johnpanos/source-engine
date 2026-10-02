//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Adapt legacy VTF images into the owned texture image contract.
//
//=============================================================================//

#include "texturecontainer/vtf_image_reader.h"

#include "texturecontainer/vtf_container.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <optional>
#include <utility>

namespace texturecontainer
{
namespace
{

constexpr std::size_t kMaxContainerBytes = vtf::kMaxImageBytes;

std::optional<PixelFormat> FormatForVtf( int format, bool srgb )
{
	switch ( format )
	{
	case 0:
	case 2:
	case 3: // 24-bit texels expand to RGBA8 with opaque alpha
		return srgb ? PixelFormat::Rgba8Srgb : PixelFormat::Rgba8Unorm;
	case 12:
		return srgb ? PixelFormat::Bgra8Srgb : PixelFormat::Bgra8Unorm;
	case 13:
	case 20:
		return srgb ? PixelFormat::Bc1Srgb : PixelFormat::Bc1Unorm;
	case 14:
		return srgb ? PixelFormat::Bc2Srgb : PixelFormat::Bc2Unorm;
	case 15:
		return srgb ? PixelFormat::Bc3Srgb : PixelFormat::Bc3Unorm;
	case 38:
		return PixelFormat::Bc4Unorm;
	case 37:
		return PixelFormat::Bc5Unorm;
	case 70:
		return srgb ? PixelFormat::Bc7Srgb : PixelFormat::Bc7Unorm;
	case 24:
		return PixelFormat::Rgba16Float;
	default:
		return std::nullopt;
	}
}

} // namespace

foundation::Expected<TextureImage, ReadError> ReadVtfImage( std::span<const std::byte> encoded )
{
	return ReadVtfImage( encoded, nullptr );
}

foundation::Expected<TextureImage, ReadError> ReadVtfImage(
    std::span<const std::byte> encoded, vtf::Decompressor decompressor )
{
	if ( encoded.size() > kMaxContainerBytes )
		return foundation::MakeUnexpected( ReadError::TooLarge );
	auto parsed = vtf::ReadHeader( encoded );
	if ( !parsed )
		return foundation::MakeUnexpected( ReadError::InvalidContainer );
	const auto &header = parsed.Value();
	if ( header.depth != 1 || ( header.flags & vtf::kEnvmap ) || header.frames != 1 )
		return foundation::MakeUnexpected( ReadError::UnsupportedTopology );
	const auto format = FormatForVtf( header.format, ( header.flags & 0x40 ) != 0 );
	if ( !format )
		return foundation::MakeUnexpected( ReadError::UnsupportedFormat );
	auto layout = vtf::ReadLayout( encoded, header );
	if ( !layout )
		return foundation::MakeUnexpected( ReadError::InvalidContainer );
	if ( layout.Value().compression && !decompressor )
		return foundation::MakeUnexpected( ReadError::UnsupportedSupercompression );
	TextureImage result{ *format, {} };
	result.levels.resize( header.mips );
	std::size_t totalBytes = 0;
	for ( const auto &run : layout.Value().images )
	{
		const auto outputBytes = header.format == 2 || header.format == 3
		                             ? vtf::ImageBytes( 0, run.width, run.height )
		                             : run.decodedBytes;
		if ( !outputBytes || outputBytes > kMaxContainerBytes - totalBytes )
			return foundation::MakeUnexpected( ReadError::TooLarge );
		totalBytes += outputBytes;
		auto bytes = vtf::ReadImage( encoded, layout.Value(), run, decompressor );
		if ( !bytes )
			return foundation::MakeUnexpected( ReadError::InvalidContainer );
		ImageLevel &level = result.levels[run.mip];
		level.width = run.width;
		level.height = run.height;
		if ( header.format == 2 || header.format == 3 )
		{
			const bool bgr = header.format == 3;
			const auto texels = std::size_t( run.width ) * run.height;
			level.bytes.resize( texels * 4 );
			for ( std::size_t i = 0; i < texels; ++i )
			{
				const auto *in = bytes.Value().data() + i * 3;
				level.bytes[i * 4] = in[bgr ? 2 : 0];
				level.bytes[i * 4 + 1] = in[1];
				level.bytes[i * 4 + 2] = in[bgr ? 0 : 2];
				level.bytes[i * 4 + 3] = std::byte{ 255 };
			}
		}
		else
			level.bytes = std::move( bytes ).Value();
	}
	return result;
}

} // namespace texturecontainer
