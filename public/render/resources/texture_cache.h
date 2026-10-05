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
//			StageMips stages a resident prefix of a mip chain: the texture
//			allocation contains only the levels supplied, uploaded from one
//			initialized upload buffer (16-byte aligned level offsets) in the
//			same submission. Stage is StageMips with mip 0 alone. Both caches
//			account payload bytes and trim by explicit priority when asked.
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
	std::uint64_t residentBytes = 0;
	std::uint32_t priority = 0; // larger scores survive budget eviction longer
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
	// levels[m] is mip m, rows tightly packed, max(1, width >> m) by
	// max(1, height >> m) texels per layer; 1 <= levels.size() <=
	// desc.mipLevels. A cube's level holds its six faces one after another
	// (+x, -x, +y, -y, +z, -z); every other texture's level holds one layer.
	// Levels past the last one given are not allocated; sampling clamps at the
	// last resident level. A level of the wrong size fails and stages nothing.
	foundation::Expected<TextureEntry, ResourceError> StageMips( std::string_view name,
	    const device::TextureDesc &desc, std::span<const std::span<const std::byte>> levels );
	const TextureEntry *Find( std::string_view name ) const;
	foundation::Expected<void, ResourceError> Evict( std::string_view name );
	bool SetPriority( std::string_view name, std::uint32_t score );
	std::uint64_t ResidentBytes() const;
	// Evicts lowest-priority resources until at most budgetBytes are accounted
	// resident. Ties use name order for deterministic results. Resources remain
	// alive behind the last submitted token via Retire().
	void EvictToBudget( std::uint64_t budgetBytes );

	// Returns the number recorded. A staging allocation failure leaves that
	// upload pending for retry; successfully recorded uploads are removed.
	std::size_t RecordUploads( device::CommandEncoder &encoder );
	void Retire( device::CompletionToken token );

	std::size_t Count() const { return m_Entries.size(); }
	std::size_t PendingUploads() const { return m_Uploads.size(); }

private:
	struct Level
	{
		std::uint64_t offset = 0; // in bytes
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		std::uint32_t layers = 1;
		std::uint64_t layerBytes = 0;
	};

	struct Upload
	{
		device::TextureId texture;
		device::TextureDesc desc;
		std::vector<std::byte> pixels; // every level, each at its offset
		std::vector<Level> levels;
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
