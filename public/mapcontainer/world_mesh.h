//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded, endian-independent WMSH v1 validation for map-scoped
//          runtime consumers and packaging tools (RFC 0008).
//
//=============================================================================//

#ifndef MAPCONTAINER_WORLD_MESH_H
#define MAPCONTAINER_WORLD_MESH_H

#include <cstddef>
#include <cstdint>

namespace mapcontainer
{

enum class WorldMeshError
{
	Ok = 0,
	Truncated,
	BadMagic,
	UnsupportedVersion,
	InvalidLayout,
	InvalidCounts,
	InvalidSections,
	InvalidVertices,
	InvalidIndices,
	InvalidBatches,
	InvalidMeshlets,
	InvalidLeaves,
	InvalidMaterials,
};

struct WorldMeshSummary
{
	uint32_t version;
	uint32_t vertexCount;
	uint32_t indexCount;
	uint32_t triangleCount;
	uint32_t batchCount;
	uint32_t meshletCount;
	uint32_t leafCount;
	uint32_t leafReferenceCount;
	uint32_t materialCount;
	uint64_t sectionOffsets[8];
};

// Validates complete bytes without retaining them or allocating. The caller
// owns and keeps the payload alive for any subsequent upload. No C++20 types or
// exceptions cross this legacy-consumer boundary.
WorldMeshError ValidateWorldMesh(
    const void *pData, size_t size, WorldMeshSummary *pSummary = nullptr ) noexcept;
const char *WorldMeshErrorName( WorldMeshError error ) noexcept;

} // namespace mapcontainer

#endif // MAPCONTAINER_WORLD_MESH_H
