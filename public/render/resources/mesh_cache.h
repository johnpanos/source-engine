//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.resources mesh residency (RFC 0016): named vertex and
//			index buffers, with the same stage, record and retire protocol as
//			TextureCache. Uploads go through the device's upload ring.
//
//=============================================================================//

#ifndef RENDER_RESOURCES_MESH_CACHE_H
#define RENDER_RESOURCES_MESH_CACHE_H

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

struct MeshEntry
{
	device::BufferId vertices;
	device::BufferId indices; // invalid for a non-indexed mesh
	std::uint32_t vertexCount = 0;
	std::uint32_t vertexStride = 0;
	std::uint32_t indexCount = 0;
	device::IndexFormat indexFormat = device::IndexFormat::kUint16;
	std::uint64_t revision = 0;
	std::uint64_t residentBytes = 0;
	std::uint32_t priority = 0; // larger scores survive budget eviction longer
};

struct MeshData
{
	std::span<const std::byte> vertices;
	std::uint32_t vertexStride = 0;
	std::span<const std::byte> indices;
	device::IndexFormat indexFormat = device::IndexFormat::kUint16;
};

class MeshCache
{
public:
	explicit MeshCache( device::IRenderDevice2 &device ) : m_Device( device ) {}
	~MeshCache();
	MeshCache( const MeshCache & ) = delete;
	MeshCache &operator=( const MeshCache & ) = delete;

	foundation::Expected<MeshEntry, ResourceError> Stage(
	    std::string_view name, const MeshData &data );
	const MeshEntry *Find( std::string_view name ) const;
	foundation::Expected<void, ResourceError> Evict( std::string_view name );
	bool SetPriority( std::string_view name, std::uint32_t score );
	std::uint64_t ResidentBytes() const;
	// Evicts lowest-priority resources until at most budgetBytes are accounted
	// resident. Ties use name order for deterministic results. Buffers remain
	// alive behind the last submitted token via Retire().
	void EvictToBudget( std::uint64_t budgetBytes );

	std::size_t RecordUploads( device::CommandEncoder &encoder );
	void Retire( device::CompletionToken token );

	std::size_t Count() const { return m_Entries.size(); }

private:
	struct Upload
	{
		device::BufferId buffer;
		device::ResourceUsage usage;
		std::vector<std::byte> bytes;
	};

	device::IRenderDevice2 &m_Device;
	std::map<std::string, MeshEntry, std::less<>> m_Entries;
	std::vector<Upload> m_Uploads;
	std::vector<device::ResourceId> m_Replaced;
	device::CompletionToken m_LastToken;
};

} // namespace render::resources

#endif // RENDER_RESOURCES_MESH_CACHE_H
