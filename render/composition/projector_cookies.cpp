//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Projector cookies as one RGBA 2D array (RFC 0016 K11/K12); see
//			render/composition/projector_cookies.h.
//
//=============================================================================//

#include "render/composition/projector_cookies.h"

#include "texturecontainer/texture_image.h"
#include "texturecontainer/vtf_image_reader.h"

#include <algorithm>
#include <cstring>
#include <span>

namespace render::composition
{

using namespace render::device;

foundation::Expected<CookieImages, std::string> DecodeCookies(
    const mdl::IModelFiles &files, const std::vector<std::string> &names )
{
	using foundation::MakeUnexpected;
	std::vector<texturecontainer::TextureImage> images;
	for ( const std::string &name : names )
	{
		std::string bytes;
		if ( !files.Read( "materials/" + name + ".vtf", bytes ) )
			return MakeUnexpected( "cookie " + name + " is missing" );
		auto image = texturecontainer::ReadVtfImage(
		    std::as_bytes( std::span( bytes.data(), bytes.size() ) ) );
		if ( !image || image.Value().levels.empty() )
			return MakeUnexpected( "cookie " + name + " does not decode" );
		const texturecontainer::PixelFormat format = image.Value().format;
		if ( format != texturecontainer::PixelFormat::Rgba8Unorm &&
		     format != texturecontainer::PixelFormat::Rgba8Srgb )
			return MakeUnexpected( "cookie " + name + " is not 8-bit RGBA after decoding" );
		images.push_back( std::move( image ).Value() );
	}
	CookieImages out;
	out.names = names;
	out.width = images.empty() ? 1 : images[0].levels[0].width;
	out.height = images.empty() ? 1 : images[0].levels[0].height;
	out.layers = std::max<std::uint32_t>( 2, std::uint32_t( images.size() ) + 1 );
	out.layerBytes = std::uint64_t( out.width ) * out.height * 4;
	out.bytes.assign( out.layerBytes * out.layers, std::byte( 255 ) );
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
	desc.format = device::Format::kRGBA8Unorm;
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
