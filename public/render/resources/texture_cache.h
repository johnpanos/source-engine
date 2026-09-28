//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.resources texture residency (RFC 0016). The cache owns
//			named device textures. Staging creates (or replaces) a texture at
//			once and queues its bytes; RecordUploads records every queued upload
//			into the caller's encoder and leaves each texture in kSampled;
//			Retire(token), called with that encoder's submission token, releases
//			staging buffers and replaced textures behind it. Nothing is freed
//			before the GPU is done with it.
//
//			Staging and recording belong to one sequence (the render sequence);
//			the cache is not thread-safe.
//
//=============================================================================//

#ifndef RENDER_RESOURCES_TEXTURE_CACHE_H
#define RENDER_RESOURCES_TEXTURE_CACHE_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/resources/resource_error.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace render::resources
{

struct TextureEntry
{
	device::TextureId texture;
	device::TextureDesc desc;
	std::uint64_t revision = 0; // rises by one per replacement
};

class TextureCache
{
public:
	explicit TextureCache( device::IRenderDevice2 &device ) : m_Device( device ) {}
	~TextureCache();
	TextureCache( const TextureCache & ) = delete;
	TextureCache &operator=( const TextureCache & ) = delete;

	// pixels are mip 0, layer 0, rows tightly packed.
	foundation::Expected<TextureEntry, ResourceError> Stage(
	    std::string_view name, const device::TextureDesc &desc, std::span<const std::byte> pixels );
	const TextureEntry *Find( std::string_view name ) const;
	foundation::Expected<void, ResourceError> Evict( std::string_view name );

	std::size_t RecordUploads( device::CommandEncoder &encoder );
	void Retire( device::CompletionToken token );

	std::size_t Count() const { return m_Entries.size(); }
	std::size_t PendingUploads() const { return m_Uploads.size(); }

private:
	struct Upload
	{
		device::TextureId texture;
		device::TextureDesc desc;
		std::vector<std::byte> pixels;
	};

	device::IRenderDevice2 &m_Device;
	std::map<std::string, TextureEntry, std::less<>> m_Entries;
	std::vector<Upload> m_Uploads;
	std::vector<device::ResourceId> m_Recorded; // staging, released at Retire
	std::vector<device::ResourceId> m_Replaced; // old textures, released at Retire
	device::CompletionToken m_LastToken;
};

} // namespace render::resources

#endif // RENDER_RESOURCES_TEXTURE_CACHE_H
