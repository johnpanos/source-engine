//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Map-scoped WMSH upload and diagnostic draw capability.
//
//=============================================================================//

#ifndef RENDER_WORLD_MESH_UPLOAD_H
#define RENDER_WORLD_MESH_UPLOAD_H

#include <cstddef>
#include <cstdint>

namespace world_mesh_gpu
{

static const char *const kWorldMeshUploadInterface = "WorldMeshUpload005";

struct WorldMeshUploadRequest
{
	const void *vertices = nullptr;
	size_t vertexBytes = 0;
	const void *indices = nullptr;
	size_t indexBytes = 0;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
};

// Separated light a map's lightmap may carry (LMAP v2 layer roles; see
// public/mapcontainer/world_lightmap.h, which owns the encoding).
enum class WorldLightmapRole : uint32_t
{
	Total = 0,
	Direct = 1,
	Indirect = 2,
};

static const uint32_t kWorldLightmapMaxUploadLayers = 3;

// Linear RGBA16F texels, rows top-left first, one pointer per layer; layer 0
// is the total page. The caller validated the LMAP lump and owns the bytes
// until UploadLightmap returns.
struct WorldLightmapUploadRequest
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t layerCount = 0;
	const void *layers[kWorldLightmapMaxUploadLayers] = {};
	WorldLightmapRole roles[kWorldLightmapMaxUploadLayers] = {};
};

// The caller validates WMSH and owns request bytes until Upload returns.
// A successful Upload replaces the prior map's buffers; a failure leaves the
// prior buffers intact. Release waits for GPU consumers before freeing storage.
// This interface is available only from a provider that implements both WMSH
// upload and ordered batch submission for its current world view.
// The map's validated RFC 0011 PRBV probe volume in its GPU form: the lump's
// RGBA16F atlas (rows top first) and the grid table
// mapcontainer::WriteProbeGridTable writes (gridCount rows of `tableFloats`
// floats, RGBA32F texels).
struct ProbeVolumeUploadRequest
{
	uint32_t atlasWidth = 0;
	uint32_t atlasHeight = 0;
	const void *atlas = nullptr;
	uint32_t gridCount = 0;
	uint32_t tableFloats = 0;
	const float *gridTable = nullptr;
};

class IWorldMeshUpload
{
public:
	virtual ~IWorldMeshUpload() = default;
	virtual bool Upload( const WorldMeshUploadRequest &request ) = 0;
	// The map's optional lightmap layers (LMAP v1: the total page; v2 also its
	// separated light). The provider uploads every layer before any WMSH batch
	// can draw. On failure the caller releases the map's WMSH so no unlit
	// substitute is presented.
	virtual bool UploadLightmap( const WorldLightmapUploadRequest &request ) = 0;
	// The map's optional probe volume, after the lightmap. Optional for the
	// map: on failure models keep the engine's ambient cube (evaluated from
	// the same volume) and nothing else changes.
	virtual bool UploadProbeVolume( const ProbeVolumeUploadRequest &request ) = 0;
	// The caller binds the material first. The provider runs its material pass
	// and queues this WMSH index range in the current ordered world view.
	virtual bool DrawBatch( uint32_t firstIndex, uint32_t indexCount ) = 0;
	virtual void Release() = 0;
	virtual bool IsResident() const = 0;
};

} // namespace world_mesh_gpu

#endif // RENDER_WORLD_MESH_UPLOAD_H
