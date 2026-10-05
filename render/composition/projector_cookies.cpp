//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Projector cookies as one RGBA 2D array (RFC 0016 K11/K12); see
//			render/composition/projector_cookies.h.
//
//=============================================================================//

#include "render/composition/projector_cookies.h"

#include "texturecontainer/texture_image.h"
#include "texturecontainer/vtf_image_reader.h"

#include <optional>

#include <algorithm>
#include <cstring>
#include <span>

namespace render::composition
{

using namespace render::device;

namespace
{

using texturecontainer::PixelFormat;

// The linear (unorm) view of a cookie's format, and one white texel block
// (4 x 4 texels for the block formats).
struct CookieFormat
{
	Format format = Format::kUnknown;
	std::vector<std::uint8_t> white;
};

std::optional<CookieFormat> CookieFormatOf( PixelFormat format )
{
	switch ( format )
	{
	case PixelFormat::Rgba8Unorm:
	case PixelFormat::Rgba8Srgb:
		return CookieFormat{ Format::kRGBA8Unorm, { 255, 255, 255, 255 } };
	case PixelFormat::Bgra8Unorm:
	case PixelFormat::Bgra8Srgb:
		return CookieFormat{ Format::kBGRA8Unorm, { 255, 255, 255, 255 } };
	case PixelFormat::Bc1Unorm:
	case PixelFormat::Bc1Srgb:
		// Both endpoints white, every index 0.
		return CookieFormat{ Format::kBC1Unorm, { 255, 255, 255, 255, 0, 0, 0, 0 } };
	case PixelFormat::Bc2Unorm:
	case PixelFormat::Bc2Srgb:
		// Explicit alpha (4 bits a texel, all 15), then the BC1 white block.
		return CookieFormat{ Format::kBC2Unorm,
		    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0 } };
	case PixelFormat::Bc3Unorm:
	case PixelFormat::Bc3Srgb:
		// Interpolated alpha (endpoints 255, every index 0), then BC1 white.
		return CookieFormat{
		    Format::kBC3Unorm, { 255, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0 } };
	case PixelFormat::Bc4Unorm:
		return CookieFormat{ Format::kBC4Unorm, { 255, 255, 0, 0, 0, 0, 0, 0 } };
	case PixelFormat::Bc5Unorm:
		return CookieFormat{
		    Format::kBC5Unorm, { 255, 255, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0 } };
	default:
		return std::nullopt;
	}
}

bool BlockFormat( Format format )
{
	return format != Format::kRGBA8Unorm && format != Format::kBGRA8Unorm;
}

} // namespace

foundation::Expected<CookieImages, std::string> DecodeCookies(
    const mdl::IModelFiles &files, const std::vector<std::string> &names )
{
	using foundation::MakeUnexpected;
	std::vector<texturecontainer::TextureImage> images;
	std::optional<CookieFormat> format;
	for ( const std::string &name : names )
	{
		std::string bytes;
		if ( !files.Read( "materials/" + name + ".vtf", bytes ) )
			return MakeUnexpected( "cookie " + name + " is missing" );
		auto image = texturecontainer::ReadVtfImage(
		    std::as_bytes( std::span( bytes.data(), bytes.size() ) ) );
		if ( !image || image.Value().levels.empty() )
			return MakeUnexpected( "cookie " + name + " does not decode" );
		const std::optional<CookieFormat> mine = CookieFormatOf( image.Value().format );
		if ( !mine )
			return MakeUnexpected(
			    "cookie " + name + " is in a format the cookie array does not take" );
		if ( format && format->format != mine->format )
			return MakeUnexpected( "cookie " + name + " differs in format from the first" );
		format = mine;
		images.push_back( std::move( image ).Value() );
	}
	CookieImages out;
	out.names = names;
	if ( format )
		out.format = format->format;
	const std::vector<std::uint8_t> white =
	    format ? format->white : std::vector<std::uint8_t>{ 255, 255, 255, 255 };
	out.width = images.empty() ? 1 : images[0].levels[0].width;
	out.height = images.empty() ? 1 : images[0].levels[0].height;
	out.layers = std::max<std::uint32_t>( 2, std::uint32_t( images.size() ) + 1 );
	// A block format's layer is whole 4 x 4 blocks; a texel format's, texels.
	const bool blocks = BlockFormat( out.format );
	const std::uint64_t units =
	    blocks ? std::uint64_t( ( out.width + 3 ) / 4 ) * ( ( out.height + 3 ) / 4 )
	           : std::uint64_t( out.width ) * out.height;
	out.layerBytes = units * white.size();
	out.bytes.resize( out.layerBytes * out.layers );
	for ( std::uint64_t unit = 0; unit < units * out.layers; ++unit )
		std::memcpy( out.bytes.data() + unit * white.size(), white.data(), white.size() );
	for ( std::size_t i = 0; i < images.size(); ++i )
	{
		const texturecontainer::ImageLevel &level = images[i].levels[0];
		if ( level.width != out.width || level.height != out.height ||
		     level.bytes.size() != out.layerBytes )
			return MakeUnexpected( "cookie " + names[i] + " differs in size from the first" );
		std::memcpy( out.bytes.data() + i * out.layerBytes, level.bytes.data(), out.layerBytes );
	}
	return out;
}

device::TextureDesc CookieArray::Desc() const
{
	device::TextureDesc desc;
	desc.format = m_Images.format;
	desc.width = m_Images.width;
	desc.height = m_Images.height;
	desc.depthOrLayers = m_Images.layers;
	desc.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
	return desc;
}

CookieArray::~CookieArray()
{
	if ( !m_Device )
		return;
	(void)m_Device->WaitIdle();
	Release( CompletionToken() );
}

void CookieArray::Release( CompletionToken token )
{
	if ( !m_Device )
		return;
	if ( m_Texture.IsValid() )
		(void)m_Device->Release( m_Texture, token );
	if ( m_Staging.IsValid() )
		(void)m_Device->Release( m_Staging, token );
	m_Texture = TextureId();
	m_Staging = BufferId();
	m_Device = nullptr;
}

std::optional<std::string> CookieArray::Create(
    IRenderDevice2 &device, const mdl::IModelFiles &files, const std::vector<std::string> &names )
{
	auto images = DecodeCookies( files, names );
	if ( !images )
		return images.Error();
	return Create( device, images.Value() );
}

std::optional<std::string> CookieArray::Create( IRenderDevice2 &device, const CookieImages &images )
{
	m_Device = &device;
	m_Images = images;
	TextureDesc desc = Desc();
	desc.debugName = "render.composition.cookies";
	auto texture = device.CreateTexture( desc );
	BufferDesc staging;
	staging.size = m_Images.bytes.size();
	staging.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	auto buffer = device.CreateBuffer( staging );
	if ( texture )
		m_Texture = texture.Value();
	if ( buffer )
		m_Staging = buffer.Value();
	if ( !texture || !buffer )
		return std::string( "the cookie array was refused" );
	return std::nullopt;
}

void CookieArray::RecordUpload( CommandEncoder &encoder )
{
	const std::uint32_t layers = m_Images.layers;
	encoder.TransitionBuffer(
	    m_Staging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( m_Staging, 0, m_Images.bytes );
	encoder.TransitionBuffer(
	    m_Staging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	encoder.TransitionTexture( m_Texture, ResourceUsage::kUndefined,
	    ResourceUsage::kCopyDestination, { 0, 1, 0, layers } );
	for ( std::uint32_t layer = 0; layer < layers; ++layer )
		encoder.CopyBufferToTexture( m_Staging, m_Texture,
		    { layer * m_Images.layerBytes, 0, layer, m_Images.width, m_Images.height } );
	encoder.TransitionTexture(
	    m_Texture, ResourceUsage::kCopyDestination, ResourceUsage::kSampled, { 0, 1, 0, layers } );
}

} // namespace render::composition
