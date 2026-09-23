//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Read a packaged KTX2 image through the shared runtime reader.
//
//=============================================================================//

#include "ktx2_preview.h"

#include "texturecontainer/texture_image.h"

#include <cstddef>
#include <span>

namespace hammer::adapters::source
{

std::optional<hammer::formats::VtfImage> DecodeKtx2Preview(
    const std::string &bytes, std::string &error )
{
	const auto image =
	    texturecontainer::ReadKtx2Image( std::as_bytes( std::span( bytes.data(), bytes.size() ) ) );
	if ( !image )
	{
		error = "KTX2 container is invalid or unsupported by the shared reader";
		return std::nullopt;
	}
	const texturecontainer::TextureImage &decoded = image.Value();
	if ( decoded.levels.empty() ||
	     ( decoded.format != texturecontainer::PixelFormat::Rgba8Unorm &&
	         decoded.format != texturecontainer::PixelFormat::Rgba8Srgb &&
	         decoded.format != texturecontainer::PixelFormat::Bgra8Unorm &&
	         decoded.format != texturecontainer::PixelFormat::Bgra8Srgb ) )
	{
		error = "KTX2 preview requires a packaged RGBA8 or BGRA8 image";
		return std::nullopt;
	}

	const texturecontainer::ImageLevel &level = decoded.levels.front();
	hammer::formats::VtfImage result;
	result.width = static_cast<int>( level.width );
	result.height = static_cast<int>( level.height );
	result.rgba.resize( level.bytes.size() );
	const bool bgra = decoded.format == texturecontainer::PixelFormat::Bgra8Unorm ||
	                  decoded.format == texturecontainer::PixelFormat::Bgra8Srgb;
	for ( std::size_t pixel = 0; pixel < result.rgba.size(); pixel += 4 )
	{
		result.rgba[pixel] = std::to_integer<std::uint8_t>( level.bytes[pixel + ( bgra ? 2 : 0 )] );
		result.rgba[pixel + 1] = std::to_integer<std::uint8_t>( level.bytes[pixel + 1] );
		result.rgba[pixel + 2] =
		    std::to_integer<std::uint8_t>( level.bytes[pixel + ( bgra ? 0 : 2 )] );
		result.rgba[pixel + 3] = std::to_integer<std::uint8_t>( level.bytes[pixel + 3] );
	}
	return result;
}

} // namespace hammer::adapters::source
