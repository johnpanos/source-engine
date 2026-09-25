//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Frame-ordered provider capabilities under the queued material
//          system (render_capability_queue.h).
//
//===========================================================================//

#include "render_capability_queue.h"

#include "imaterialsysteminternal.h"
#include "tier1/functors.h"

#include <vector>

namespace
{

// A queued call that owns the bytes its request borrows. The call queue
// releases it whether it runs or the queue is flushed unrun.
class CQueuedCapabilityCall final : public CFunctorBase
{
public:
	explicit CQueuedCapabilityCall( std::function<void()> call ) : m_call( std::move( call ) ) {}
	void operator()() override { m_call(); }

private:
	std::function<void()> m_call;
};

void QueueCapabilityCall( CMatCallQueue *queue, std::function<void()> call )
{
	CFunctor *functor = new CQueuedCapabilityCall( std::move( call ) );
	queue->QueueFunctor( functor );
	functor->Release();
}

typedef std::vector<unsigned char> Bytes;

std::shared_ptr<Bytes> CopyBytes( const void *data, size_t bytes )
{
	std::shared_ptr<Bytes> copy = std::make_shared<Bytes>();
	if ( data && bytes )
	{
		const unsigned char *begin = static_cast<const unsigned char *>( data );
		copy->assign( begin, begin + bytes );
	}
	return copy;
}

const void *DataOrNull( const std::shared_ptr<Bytes> &bytes )
{
	return bytes->empty() ? nullptr : bytes->data();
}

// The sizes of the borrowed bytes, as world_mesh_upload.h documents them.
const size_t kRgba16fTexelBytes = 8;

} // namespace

bool CQueuedWorldMeshUpload::Upload( const world_mesh_gpu::WorldMeshUploadRequest &request )
{
	CMatCallQueue *queue = m_host.renderCallQueue();
	ForgetRejections();
	if ( !queue )
		return m_provider->Upload( request );
	std::shared_ptr<Bytes> vertices = CopyBytes( request.vertices, request.vertexBytes );
	std::shared_ptr<Bytes> indices = CopyBytes( request.indices, request.indexBytes );
	world_mesh_gpu::WorldMeshUploadRequest copy = request;
	world_mesh_gpu::IWorldMeshUpload *provider = m_provider;
	QueueCapabilityCall( queue,
	    [provider, copy, vertices, indices]() mutable
	    {
		    copy.vertices = DataOrNull( vertices );
		    copy.indices = DataOrNull( indices );
		    provider->Upload( copy );
	    } );
	return true;
}

bool CQueuedWorldMeshUpload::UploadLightmap(
    const world_mesh_gpu::WorldLightmapUploadRequest &request )
{
	CMatCallQueue *queue = m_host.renderCallQueue();
	if ( !queue )
		return m_provider->UploadLightmap( request );
	const size_t layerBytes = size_t( request.width ) * request.height * kRgba16fTexelBytes;
	std::vector<std::shared_ptr<Bytes>> layers;
	for ( uint32_t i = 0;
	    i < request.layerCount && i < world_mesh_gpu::kWorldLightmapMaxUploadLayers; ++i )
		layers.push_back( CopyBytes( request.layers[i], layerBytes ) );
	world_mesh_gpu::WorldLightmapUploadRequest copy = request;
	world_mesh_gpu::IWorldMeshUpload *provider = m_provider;
	QueueCapabilityCall( queue,
	    [provider, copy, layers]() mutable
	    {
		    for ( size_t i = 0; i < layers.size(); ++i )
			    copy.layers[i] = DataOrNull( layers[i] );
		    provider->UploadLightmap( copy );
	    } );
	return true;
}

bool CQueuedWorldMeshUpload::UploadProbeVolume(
    const world_mesh_gpu::ProbeVolumeUploadRequest &request )
{
	CMatCallQueue *queue = m_host.renderCallQueue();
	if ( !queue )
		return m_provider->UploadProbeVolume( request );
	const size_t atlasBytes =
	    size_t( request.atlasWidth ) * request.atlasHeight * kRgba16fTexelBytes;
	std::shared_ptr<Bytes> atlas = CopyBytes( request.atlas, atlasBytes );
	std::shared_ptr<Bytes> delta = CopyBytes( request.deltaAtlas, atlasBytes );
	std::shared_ptr<Bytes> table = CopyBytes(
	    request.gridTable, size_t( request.gridCount ) * request.tableFloats * sizeof( float ) );
	world_mesh_gpu::ProbeVolumeUploadRequest copy = request;
	world_mesh_gpu::IWorldMeshUpload *provider = m_provider;
	QueueCapabilityCall( queue,
	    [provider, copy, atlas, delta, table]() mutable
	    {
		    copy.atlas = DataOrNull( atlas );
		    copy.deltaAtlas = DataOrNull( delta );
		    copy.gridTable = static_cast<const float *>( DataOrNull( table ) );
		    provider->UploadProbeVolume( copy );
	    } );
	return true;
}

bool CQueuedWorldMeshUpload::UploadShadowField(
    const world_mesh_gpu::ShadowFieldUploadRequest &request )
{
	CMatCallQueue *queue = m_host.renderCallQueue();
	if ( !queue )
		return m_provider->UploadShadowField( request );
	const size_t voxels = size_t( request.dims[0] ) * request.dims[1] * request.dims[2];
	std::shared_ptr<Bytes> distances = CopyBytes( request.distances, voxels * sizeof( uint16_t ) );
	world_mesh_gpu::ShadowFieldUploadRequest copy = request;
	world_mesh_gpu::IWorldMeshUpload *provider = m_provider;
	QueueCapabilityCall( queue,
	    [provider, copy, distances]() mutable
	    {
		    copy.distances = static_cast<const uint16_t *>( DataOrNull( distances ) );
		    provider->UploadShadowField( copy );
	    } );
	return true;
}

bool CQueuedWorldMeshUpload::UploadReflectionProbes(
    const world_mesh_gpu::ReflectionProbesUploadRequest &request )
{
	CMatCallQueue *queue = m_host.renderCallQueue();
	if ( !queue )
		return m_provider->UploadReflectionProbes( request );
	std::shared_ptr<Bytes> texels =
	    CopyBytes( request.texels, size_t( request.width ) * request.height * kRgba16fTexelBytes );
	world_mesh_gpu::ReflectionProbesUploadRequest copy = request;
	world_mesh_gpu::IWorldMeshUpload *provider = m_provider;
	QueueCapabilityCall( queue,
	    [provider, copy, texels]() mutable
	    {
		    copy.texels = static_cast<const uint16_t *>( DataOrNull( texels ) );
		    provider->UploadReflectionProbes( copy );
	    } );
	return true;
}

bool CQueuedWorldMeshUpload::DrawBatch( uint32_t firstIndex, uint32_t indexCount )
{
	CMatCallQueue *queue = m_host.renderCallQueue();
	if ( !queue )
		return m_provider->DrawBatch( firstIndex, indexCount );
	// The engine binds the batch's material (a queued call) first; the draw
	// follows it on the queue so the provider runs that material's pass.
	IMaterial *material = m_host.boundMaterial();
	queue->QueueCall(
	    this, &CQueuedWorldMeshUpload::ExecuteDrawBatch, material, firstIndex, indexCount );
	std::lock_guard<std::mutex> lock( m_rejectedMutex );
	return m_rejected.find( material ) == m_rejected.end();
}

void CQueuedWorldMeshUpload::ExecuteDrawBatch(
    IMaterial *material, uint32_t firstIndex, uint32_t indexCount )
{
	// Only a resident mesh's non-empty draw says anything about the material.
	if ( m_provider->DrawBatch( firstIndex, indexCount ) || !indexCount ||
	     !m_provider->IsResident() )
		return;
	std::lock_guard<std::mutex> lock( m_rejectedMutex );
	m_rejected.insert( material );
}

void CQueuedWorldMeshUpload::Release()
{
	CMatCallQueue *queue = m_host.renderCallQueue();
	ForgetRejections();
	if ( !queue )
	{
		m_provider->Release();
		return;
	}
	queue->QueueCall( m_provider, &world_mesh_gpu::IWorldMeshUpload::Release );
}

void CQueuedWorldMeshUpload::ForgetRejections()
{
	std::lock_guard<std::mutex> lock( m_rejectedMutex );
	m_rejected.clear();
}

void CQueuedLightSetConsumer::PublishLightSet( const light_set::Snapshot &snapshot )
{
	CMatCallQueue *queue = m_host.renderCallQueue();
	if ( !queue )
	{
		m_provider->PublishLightSet( snapshot );
		return;
	}
	std::shared_ptr<const light_set::Snapshot> copy =
	    std::make_shared<const light_set::Snapshot>( snapshot );
	light_set::ILightSetConsumer *provider = m_provider;
	QueueCapabilityCall( queue,
	    [provider, copy]()
	    {
		    provider->PublishLightSet( *copy );
	    } );
}
