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

static const char *const kWorldMeshUploadInterface = "WorldMeshUpload003";

struct WorldMeshUploadRequest
{
	const void *vertices = nullptr;
	size_t vertexBytes = 0;
	const void *indices = nullptr;
	size_t indexBytes = 0;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
};

// The caller validates WMSH and owns request bytes until Upload returns.
// A successful Upload replaces the prior map's buffers; a failure leaves the
// prior buffers intact. Release waits for GPU consumers before freeing storage.
// This interface is available only from a provider that implements both WMSH
// upload and ordered batch submission for its current world view.
class IWorldMeshUpload
{
public:
	virtual ~IWorldMeshUpload() = default;
	virtual bool Upload( const WorldMeshUploadRequest &request ) = 0;
	// Optional LMAP v1 is an owned linear RGBA16F KTX2 page. The provider
	// validates and uploads it before any WMSH batch can draw. On failure the
	// caller releases the map's WMSH so no unlit substitute is presented.
	virtual bool UploadLightmapKtx2( const void *bytes, size_t size ) = 0;
	// The caller binds the material first. The provider runs its material pass
	// and queues this WMSH index range in the current ordered world view.
	virtual bool DrawBatch( uint32_t firstIndex, uint32_t indexCount ) = 0;
	virtual void Release() = 0;
	virtual bool IsResident() const = 0;
};

} // namespace world_mesh_gpu

#endif // RENDER_WORLD_MESH_UPLOAD_H
