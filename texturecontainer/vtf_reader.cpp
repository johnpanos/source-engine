//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Adapt legacy VTF images into the owned texture image contract.
//
//=============================================================================//

#include "texturecontainer/vtf_image_reader.h"

#include "tier1/utlbuffer.h"
#include "vtf/vtf.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <optional>
#include <utility>

namespace texturecontainer
{
namespace
{

constexpr std::size_t kMaxContainerBytes = 512U * 1024U * 1024U;

std::optional<PixelFormat> FormatForVtf( ImageFormat format, bool srgb )
{
	switch ( format )
	{
	case IMAGE_FORMAT_RGBA8888:
		return srgb ? PixelFormat::Rgba8Srgb : PixelFormat::Rgba8Unorm;
	case IMAGE_FORMAT_BGRA8888:
		return srgb ? PixelFormat::Bgra8Srgb : PixelFormat::Bgra8Unorm;
	case IMAGE_FORMAT_DXT1:
	case IMAGE_FORMAT_DXT1_ONEBITALPHA:
		return srgb ? PixelFormat::Bc1Srgb : PixelFormat::Bc1Unorm;
	case IMAGE_FORMAT_DXT3:
		return srgb ? PixelFormat::Bc2Srgb : PixelFormat::Bc2Unorm;
	case IMAGE_FORMAT_DXT5:
		return srgb ? PixelFormat::Bc3Srgb : PixelFormat::Bc3Unorm;
	case IMAGE_FORMAT_ATI1N:
		return PixelFormat::Bc4Unorm;
	case IMAGE_FORMAT_ATI2N:
		return PixelFormat::Bc5Unorm;
	case IMAGE_FORMAT_RGBA16161616F:
		return PixelFormat::Rgba16Float;
	default:
		return std::nullopt;
	}
}

} // namespace

foundation::Expected<TextureImage, ReadError> ReadVtfImage( std::span<const std::byte> encoded )
{
	constexpr char magic[] = { 'V', 'T', 'F', '\0' };
	if ( encoded.size() < 64 || std::memcmp( encoded.data(), magic, sizeof( magic ) ) )
		return foundation::MakeUnexpected( ReadError::InvalidContainer );
	if ( encoded.size() > kMaxContainerBytes )
		return foundation::MakeUnexpected( ReadError::TooLarge );
	// The legacy decoder allocates a full mip chain for compatibility even when
	// older VTFs store fewer levels. Only the header's authored levels contain
	// file data; copying the extra allocation would expose unused bytes.
	const unsigned int authoredMips = std::to_integer<unsigned int>( encoded[0x38] );

	CUtlBuffer buffer( encoded.data(), static_cast<int>( encoded.size() ), CUtlBuffer::READ_ONLY );
	std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture(
	    CreateVTFTexture(), &DestroyVTFTexture );
	if ( !texture || !texture->Unserialize( buffer, false ) )
		return foundation::MakeUnexpected( ReadError::InvalidContainer );
	if ( texture->Depth() != 1 || texture->FaceCount() != 1 || texture->FrameCount() != 1 ||
	     texture->Width() <= 0 || texture->Height() <= 0 || texture->MipCount() <= 0 ||
	     authoredMips == 0 || authoredMips > static_cast<unsigned int>( texture->MipCount() ) )
		return foundation::MakeUnexpected( ReadError::UnsupportedTopology );

	const std::optional<PixelFormat> format =
	    FormatForVtf( texture->Format(), ( texture->Flags() & TEXTUREFLAGS_SRGB ) != 0 );
	if ( !format )
		return foundation::MakeUnexpected( ReadError::UnsupportedFormat );

	TextureImage result{ *format, {} };
	result.levels.reserve( authoredMips );
	std::size_t totalBytes = 0;
	for ( unsigned int mip = 0; mip < authoredMips; ++mip )
	{
		int width = 0, height = 0, depth = 0;
		texture->ComputeMipLevelDimensions( mip, &width, &height, &depth );
		const int bytes = texture->ComputeMipSize( mip );
		if ( width <= 0 || height <= 0 || depth != 1 || bytes <= 0 ||
		     static_cast<std::size_t>( bytes ) > kMaxContainerBytes - totalBytes )
			return foundation::MakeUnexpected( ReadError::InvalidImageSize );
		const unsigned char *source = texture->ImageData( 0, 0, mip );
		if ( !source )
			return foundation::MakeUnexpected( ReadError::InvalidImageSize );
		ImageLevel level;
		level.width = static_cast<std::uint32_t>( width );
		level.height = static_cast<std::uint32_t>( height );
		level.bytes.resize( bytes );
		std::memcpy( level.bytes.data(), source, bytes );
		result.levels.push_back( std::move( level ) );
		totalBytes += bytes;
	}
	return result;
}

} // namespace texturecontainer
