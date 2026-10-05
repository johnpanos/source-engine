//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's participating media (RFC 0016 K11 step g); see
//			lab_media.h.
//
//=============================================================================//

#include "lab_media.h"

#include "render/light_set.h"
#include "render/projected_light.h"
#include "texturecontainer/texture_image.h"
#include "texturecontainer/vtf_image_reader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <span>

namespace render::lab
{

namespace
{

using namespace render::device;
namespace volumetric = render::pass::volumetric;

using namespace render::pass::lights;

constexpr double kPi = 3.14159265358979323846;

} // namespace

device::TextureDesc CookieArray::Desc() const
{
	device::TextureDesc desc;
	desc.format = device::Format::kRGBA8Unorm;
	desc.width = m_Width;
	desc.height = m_Height;
	desc.depthOrLayers = m_Layers;
	desc.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
	return desc;
}

CookieArray::~CookieArray()
{
	if ( !m_Device )
		return;
	(void)m_Device->WaitIdle();
	if ( m_Texture.IsValid() )
		(void)m_Device->Release( m_Texture, CompletionToken() );
	if ( m_Staging.IsValid() )
		(void)m_Device->Release( m_Staging, CompletionToken() );
}

std::optional<std::string> CookieArray::Create(
    IRenderDevice2 &device, const GameFiles &files, const std::vector<std::string> &names )
{
	m_Device = &device;
	std::vector<texturecontainer::TextureImage> images;
	for ( const std::string &name : names )
	{
		std::string bytes;
		if ( !files.Read( "materials/" + name + ".vtf", bytes ) )
			return "cookie " + name + " is missing";
		auto image = texturecontainer::ReadVtfImage(
		    std::as_bytes( std::span( bytes.data(), bytes.size() ) ) );
		if ( !image || image.Value().levels.empty() )
			return "cookie " + name + " does not decode";
		const texturecontainer::PixelFormat format = image.Value().format;
		if ( format != texturecontainer::PixelFormat::Rgba8Unorm &&
		     format != texturecontainer::PixelFormat::Rgba8Srgb )
			return "cookie " + name + " is not 8-bit RGBA after decoding";
		images.push_back( std::move( image ).Value() );
	}
	m_Width = images.empty() ? 1 : images[0].levels[0].width;
	m_Height = images.empty() ? 1 : images[0].levels[0].height;
	m_Layers = std::max<std::uint32_t>( 2, std::uint32_t( images.size() ) + 1 );
	m_LayerBytes = std::uint64_t( m_Width ) * m_Height * 4;
	m_Bytes.assign( m_LayerBytes * m_Layers, std::byte( 255 ) );
	for ( std::size_t i = 0; i < images.size(); ++i )
	{
		const texturecontainer::ImageLevel &level = images[i].levels[0];
		if ( level.width != m_Width || level.height != m_Height ||
		     level.bytes.size() != m_LayerBytes )
			return "cookie " + names[i] + " differs in size from the first";
		std::memcpy( m_Bytes.data() + i * m_LayerBytes, level.bytes.data(), m_LayerBytes );
	}
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm; // the cookie is linear (pixel / 255)
	desc.width = m_Width;
	desc.height = m_Height;
	desc.depthOrLayers = m_Layers;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	desc.debugName = "render_lab.cookies";
	auto texture = device.CreateTexture( desc );
	BufferDesc staging;
	staging.size = m_Bytes.size();
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
	encoder.TransitionBuffer(
	    m_Staging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( m_Staging, 0, m_Bytes );
	encoder.TransitionBuffer(
	    m_Staging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	encoder.TransitionTexture( m_Texture, ResourceUsage::kUndefined,
	    ResourceUsage::kCopyDestination, { 0, 1, 0, m_Layers } );
	for ( std::uint32_t layer = 0; layer < m_Layers; ++layer )
		encoder.CopyBufferToTexture(
		    m_Staging, m_Texture, { layer * m_LayerBytes, 0, layer, m_Width, m_Height } );
	encoder.TransitionTexture( m_Texture, ResourceUsage::kCopyDestination, ResourceUsage::kSampled,
	    { 0, 1, 0, m_Layers } );
}

} // namespace render::lab
