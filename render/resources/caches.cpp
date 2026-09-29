//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.resources texture and mesh caches (RFC 0016).
//
//=============================================================================//

#include "render/resources/mesh_cache.h"
#include "render/resources/texture_cache.h"

#include <algorithm>
#include <utility>

namespace render::resources
{

namespace
{

foundation::Unexpected<ResourceError> Fail( ResourceStatus status, device::DeviceError error = {} )
{
	return foundation::MakeUnexpected( ResourceError{ status, error } );
}

} // namespace

// --- TextureCache -----------------------------------------------------------

TextureCache::~TextureCache()
{
	// Everything may still be in use by the last submission.
	for ( const device::ResourceId &resource : m_Recorded )
		(void)m_Device.Release( resource, m_LastToken );
	for ( const device::ResourceId &resource : m_Replaced )
		(void)m_Device.Release( resource, m_LastToken );
	for ( const auto &[name, entry] : m_Entries )
		(void)m_Device.Release( entry.texture, m_LastToken );
}

foundation::Expected<TextureEntry, ResourceError> TextureCache::Stage(
    std::string_view name, const device::TextureDesc &desc, std::span<const std::byte> pixels )
{
	const std::span<const std::byte> levels[] = { pixels };
	return StageMips( name, desc, levels );
}

foundation::Expected<TextureEntry, ResourceError> TextureCache::StageMips( std::string_view name,
    const device::TextureDesc &desc, std::span<const std::span<const std::byte>> levels )
{
	if ( levels.empty() || levels.size() > desc.mipLevels )
		return Fail( ResourceStatus::kSizeMismatch );
	// Each level at a 16-byte aligned offset of one staging buffer: a
	// multiple of every texel size, as a buffer-to-texture copy needs.
	constexpr std::uint64_t kAlign = 16;
	const std::uint32_t texel = device::BytesPerTexel( desc.format );
	std::vector<Level> placed;
	std::uint64_t total = 0;
	for ( std::size_t m = 0; m < levels.size(); ++m )
	{
		const std::uint32_t width = std::max( desc.width >> m, 1u );
		const std::uint32_t height = std::max( desc.height >> m, 1u );
		const std::uint64_t expected = static_cast<std::uint64_t>( width ) * height * texel;
		if ( desc.width == 0 || desc.height == 0 || expected == 0 || levels[m].size() != expected )
			return Fail( ResourceStatus::kSizeMismatch );
		total = ( total + kAlign - 1 ) / kAlign * kAlign;
		placed.push_back( { total, width, height } );
		total += expected;
	}
	device::TextureDesc resident = desc;
	resident.usages.Add( device::ResourceUsage::kCopyDestination )
	    .Add( device::ResourceUsage::kSampled );
	auto texture = m_Device.CreateTexture( resident );
	if ( !texture )
		return Fail( ResourceStatus::kDevice, texture.Error() );

	TextureEntry entry{ texture.Value(), resident, 1 };
	const auto found = m_Entries.find( name );
	if ( found != m_Entries.end() )
	{
		entry.revision = found->second.revision + 1;
		m_Replaced.push_back( found->second.texture );
		// An upload not yet recorded for the replaced texture is dropped.
		std::erase_if( m_Uploads,
		    [&]( const Upload &upload )
		    {
			    return upload.texture == found->second.texture;
		    } );
		found->second = entry;
	}
	else
	{
		m_Entries.emplace( std::string( name ), entry );
	}
	Upload upload{ entry.texture, resident, std::vector<std::byte>( total ), std::move( placed ) };
	for ( std::size_t m = 0; m < levels.size(); ++m )
		std::copy( levels[m].begin(), levels[m].end(),
		    upload.pixels.begin() + std::ptrdiff_t( upload.levels[m].offset ) );
	m_Uploads.push_back( std::move( upload ) );
	return entry;
}

const TextureEntry *TextureCache::Find( std::string_view name ) const
{
	const auto found = m_Entries.find( name );
	return found == m_Entries.end() ? nullptr : &found->second;
}

foundation::Expected<void, ResourceError> TextureCache::Evict( std::string_view name )
{
	const auto found = m_Entries.find( name );
	if ( found == m_Entries.end() )
		return Fail( ResourceStatus::kUnknownName );
	std::erase_if( m_Uploads,
	    [&]( const Upload &upload )
	    {
		    return upload.texture == found->second.texture;
	    } );
	m_Replaced.push_back( found->second.texture );
	m_Entries.erase( found );
	return {};
}

std::size_t TextureCache::RecordUploads( device::CommandEncoder &encoder )
{
	std::size_t recorded = 0;
	for ( Upload &upload : m_Uploads )
	{
		device::BufferDesc staging;
		staging.size = upload.pixels.size();
		staging.usages = {
		    device::ResourceUsage::kCopyDestination, device::ResourceUsage::kCopySource };
		staging.memory = device::MemoryKind::kUpload;
		auto buffer = m_Device.CreateBuffer( staging );
		if ( !buffer )
			continue;
		m_Recorded.push_back( buffer.Value() );
		encoder.TransitionBuffer( buffer.Value(), device::ResourceUsage::kUndefined,
		    device::ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffer.Value(), 0, upload.pixels );
		encoder.TransitionBuffer( buffer.Value(), device::ResourceUsage::kCopyDestination,
		    device::ResourceUsage::kCopySource );
		encoder.TransitionTexture( upload.texture, device::ResourceUsage::kUndefined,
		    device::ResourceUsage::kCopyDestination );
		for ( std::size_t m = 0; m < upload.levels.size(); ++m )
		{
			const Level &level = upload.levels[m];
			encoder.CopyBufferToTexture( buffer.Value(), upload.texture,
			    { level.offset, static_cast<std::uint32_t>( m ), 0, level.width, level.height } );
		}
		encoder.TransitionTexture( upload.texture, device::ResourceUsage::kCopyDestination,
		    device::ResourceUsage::kSampled );
		++recorded;
	}
	m_Uploads.clear();
	return recorded;
}

void TextureCache::Retire( device::CompletionToken token )
{
	for ( const device::ResourceId &resource : m_Recorded )
		(void)m_Device.Release( resource, token );
	for ( const device::ResourceId &resource : m_Replaced )
		(void)m_Device.Release( resource, token );
	m_Recorded.clear();
	m_Replaced.clear();
	m_LastToken = token;
}

// --- MeshCache --------------------------------------------------------------

MeshCache::~MeshCache()
{
	for ( const device::ResourceId &resource : m_Replaced )
		(void)m_Device.Release( resource, m_LastToken );
	for ( const auto &[name, entry] : m_Entries )
	{
		(void)m_Device.Release( entry.vertices, m_LastToken );
		if ( entry.indices.IsValid() )
			(void)m_Device.Release( entry.indices, m_LastToken );
	}
}

foundation::Expected<MeshEntry, ResourceError> MeshCache::Stage(
    std::string_view name, const MeshData &data )
{
	const std::uint32_t indexSize = data.indexFormat == device::IndexFormat::kUint16 ? 2u : 4u;
	if ( data.vertices.empty() || data.vertexStride == 0 ||
	     data.vertices.size() % data.vertexStride != 0 || data.indices.size() % indexSize != 0 )
		return Fail( ResourceStatus::kSizeMismatch );

	MeshEntry entry;
	entry.vertexStride = data.vertexStride;
	entry.vertexCount = static_cast<std::uint32_t>( data.vertices.size() / data.vertexStride );
	entry.indexCount = static_cast<std::uint32_t>( data.indices.size() / indexSize );
	entry.indexFormat = data.indexFormat;
	entry.revision = 1;

	device::BufferDesc vertices;
	vertices.size = data.vertices.size();
	vertices.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kVertex };
	auto vertexBuffer = m_Device.CreateBuffer( vertices );
	if ( !vertexBuffer )
		return Fail( ResourceStatus::kDevice, vertexBuffer.Error() );
	entry.vertices = vertexBuffer.Value();
	if ( !data.indices.empty() )
	{
		device::BufferDesc indices;
		indices.size = data.indices.size();
		indices.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kIndex };
		auto indexBuffer = m_Device.CreateBuffer( indices );
		if ( !indexBuffer )
		{
			// Nothing used the new vertex buffer yet.
			(void)m_Device.Release( entry.vertices, {} );
			return Fail( ResourceStatus::kDevice, indexBuffer.Error() );
		}
		entry.indices = indexBuffer.Value();
	}

	const auto found = m_Entries.find( name );
	if ( found != m_Entries.end() )
	{
		entry.revision = found->second.revision + 1;
		const MeshEntry old = found->second;
		std::erase_if( m_Uploads,
		    [&]( const Upload &upload )
		    {
			    return upload.buffer == old.vertices || upload.buffer == old.indices;
		    } );
		m_Replaced.push_back( old.vertices );
		if ( old.indices.IsValid() )
			m_Replaced.push_back( old.indices );
		found->second = entry;
	}
	else
	{
		m_Entries.emplace( std::string( name ), entry );
	}
	m_Uploads.push_back( { entry.vertices, device::ResourceUsage::kVertex,
	    { data.vertices.begin(), data.vertices.end() } } );
	if ( entry.indices.IsValid() )
		m_Uploads.push_back( { entry.indices, device::ResourceUsage::kIndex,
		    { data.indices.begin(), data.indices.end() } } );
	return entry;
}

const MeshEntry *MeshCache::Find( std::string_view name ) const
{
	const auto found = m_Entries.find( name );
	return found == m_Entries.end() ? nullptr : &found->second;
}

foundation::Expected<void, ResourceError> MeshCache::Evict( std::string_view name )
{
	const auto found = m_Entries.find( name );
	if ( found == m_Entries.end() )
		return Fail( ResourceStatus::kUnknownName );
	const MeshEntry old = found->second;
	std::erase_if( m_Uploads,
	    [&]( const Upload &upload )
	    {
		    return upload.buffer == old.vertices || upload.buffer == old.indices;
	    } );
	m_Replaced.push_back( old.vertices );
	if ( old.indices.IsValid() )
		m_Replaced.push_back( old.indices );
	m_Entries.erase( found );
	return {};
}

std::size_t MeshCache::RecordUploads( device::CommandEncoder &encoder )
{
	for ( const Upload &upload : m_Uploads )
	{
		encoder.TransitionBuffer( upload.buffer, device::ResourceUsage::kUndefined,
		    device::ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( upload.buffer, 0, upload.bytes );
		encoder.TransitionBuffer(
		    upload.buffer, device::ResourceUsage::kCopyDestination, upload.usage );
	}
	const std::size_t recorded = m_Uploads.size();
	m_Uploads.clear();
	return recorded;
}

void MeshCache::Retire( device::CompletionToken token )
{
	for ( const device::ResourceId &resource : m_Replaced )
		(void)m_Device.Release( resource, token );
	m_Replaced.clear();
	m_LastToken = token;
}

} // namespace render::resources
