//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frontend's frame-ordered capability adapters (RFC 0016 K3);
//			see public/render/legacy/capabilities.h. Moved from the material
//			system's render_capability_queue with the same behavior: every
//			queued call owns copies of the bytes its request borrows.
//
//			Seeded defects (render.legacy-capabilities sensitivity rows):
//			RENDER_LEGACY_CAPABILITIES_SEEDED_BORROWED_BYTES hands a queued
//			Upload the caller's borrowed pointers; ..._SEEDED_UNORDERED calls
//			the backend at once instead of queueing.
//
//=============================================================================//

#include "queued_capabilities.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

namespace render::legacy
{

namespace
{

using Bytes = std::vector<unsigned char>;

std::shared_ptr<Bytes> CopyBytes( const void *data, std::size_t bytes )
{
	auto copy = std::make_shared<Bytes>();
	if ( data && bytes )
	{
		const auto *begin = static_cast<const unsigned char *>( data );
		copy->assign( begin, begin + bytes );
	}
	return copy;
}

const void *DataOrNull( const std::shared_ptr<Bytes> &bytes )
{
	return bytes->empty() ? nullptr : bytes->data();
}

// A partial update's rectangles: never null, even when there are none (a
// partial update that changes only the probe grid table), so the provider
// still takes it as partial.
const world_mesh_gpu::ProbeAtlasRegion *RegionsOrEmpty( const std::shared_ptr<Bytes> &regions )
{
	static const world_mesh_gpu::ProbeAtlasRegion kNone;
	return regions->empty()
	           ? &kNone
	           : reinterpret_cast<const world_mesh_gpu::ProbeAtlasRegion *>( regions->data() );
}

// The sizes of the borrowed bytes, as world_mesh_upload.h documents them.
constexpr std::size_t kRgba16fTexelBytes = 8;

bool Active( const RenderCallQueueHost *host )
{
#if defined( RENDER_LEGACY_CAPABILITIES_SEEDED_UNORDERED )
	return false && host;
#else
	return host && host->active && host->queue && host->active();
#endif
}

// Queues call on the host's render call queue (Active( host ) holds).
void Queue( const RenderCallQueueHost *host, std::function<void()> call )
{
	auto *payload = new std::function<void()>( std::move( call ) );
	const bool queued = host->queue(
	    []( void *p )
	    {
		    ( *static_cast<std::function<void()> *>( p ) )();
	    },
	    payload,
	    []( void *p )
	    {
		    delete static_cast<std::function<void()> *>( p );
	    } );
	// The context stopped queueing between the check and the call: run it
	// here, still in order, since nothing was queued.
	if ( !queued )
	{
		( *payload )();
		delete payload;
	}
}

} // namespace

class QueuedCapabilities::WorldMesh final : public world_mesh_gpu::IWorldMeshUpload
{
public:
	WorldMesh( world_mesh_gpu::IWorldMeshUpload *provider, const RenderCallQueueHost *const &host )
	    : m_Provider( provider ), m_Host( host )
	{
	}

	bool Upload( const world_mesh_gpu::WorldMeshUploadRequest &request ) override
	{
		ForgetRejections();
		if ( !Queued() )
			return m_Provider->Upload( request );
		std::shared_ptr<Bytes> vertices = CopyBytes( request.vertices, request.vertexBytes );
		std::shared_ptr<Bytes> indices = CopyBytes( request.indices, request.indexBytes );
		world_mesh_gpu::WorldMeshUploadRequest copy = request;
		world_mesh_gpu::IWorldMeshUpload *provider = m_Provider;
		Queue( m_Host,
		    [provider, copy, vertices, indices]() mutable
		    {
#if !defined( RENDER_LEGACY_CAPABILITIES_SEEDED_BORROWED_BYTES )
			    copy.vertices = DataOrNull( vertices );
			    copy.indices = DataOrNull( indices );
#endif
			    provider->Upload( copy );
		    } );
		return true;
	}

	bool UploadLightmap( const world_mesh_gpu::WorldLightmapUploadRequest &request ) override
	{
		if ( !Queued() )
			return m_Provider->UploadLightmap( request );
		if ( request.regions )
		{
			// A partial update copies its rectangles' texels alone.
			std::shared_ptr<Bytes> total = CopyBytes( request.regionTotal,
			    world_mesh_gpu::ProbeRegionBytes( request.regions, request.regionCount ) );
			std::shared_ptr<Bytes> regions = CopyBytes(
			    request.regions, std::size_t( request.regionCount ) * sizeof( *request.regions ) );
			world_mesh_gpu::WorldLightmapUploadRequest copy = request;
			world_mesh_gpu::IWorldMeshUpload *provider = m_Provider;
			Queue( m_Host,
			    [provider, copy, total, regions]() mutable
			    {
				    copy.regions = RegionsOrEmpty( regions );
				    copy.regionTotal = DataOrNull( total );
				    provider->UploadLightmap( copy );
			    } );
			return true;
		}
		const std::size_t layerBytes =
		    std::size_t( request.width ) * request.height * kRgba16fTexelBytes;
		std::vector<std::shared_ptr<Bytes>> layers;
		for ( std::uint32_t i = 0;
		    i < request.layerCount && i < world_mesh_gpu::kWorldLightmapMaxUploadLayers; ++i )
			layers.push_back( CopyBytes( request.layers[i], layerBytes ) );
		world_mesh_gpu::WorldLightmapUploadRequest copy = request;
		world_mesh_gpu::IWorldMeshUpload *provider = m_Provider;
		Queue( m_Host,
		    [provider, copy, layers]() mutable
		    {
			    for ( std::size_t i = 0; i < layers.size(); ++i )
				    copy.layers[i] = DataOrNull( layers[i] );
			    provider->UploadLightmap( copy );
		    } );
		return true;
	}

	bool UploadProbeVolume( const world_mesh_gpu::ProbeVolumeUploadRequest &request ) override
	{
		if ( !Queued() )
			return m_Provider->UploadProbeVolume( request );
		// A partial update copies its rectangles' texels alone.
		const std::size_t atlasBytes =
		    request.regions
		        ? world_mesh_gpu::ProbeRegionBytes( request.regions, request.regionCount )
		        : std::size_t( request.atlasWidth ) * request.atlasHeight * kRgba16fTexelBytes;
		std::shared_ptr<Bytes> atlas =
		    CopyBytes( request.regions ? request.regionAtlas : request.atlas, atlasBytes );
		std::shared_ptr<Bytes> delta =
		    CopyBytes( request.regions ? request.regionDelta : request.deltaAtlas, atlasBytes );
		std::shared_ptr<Bytes> regions = CopyBytes(
		    request.regions, std::size_t( request.regionCount ) * sizeof( *request.regions ) );
		// The grid rows, then the moving occluders' rows (R50-RELIGHT).
		std::shared_ptr<Bytes> table =
		    CopyBytes( request.gridTable, std::size_t( request.gridCount + request.occluderCount ) *
		                                      request.tableFloats * sizeof( float ) );
		world_mesh_gpu::ProbeVolumeUploadRequest copy = request;
		world_mesh_gpu::IWorldMeshUpload *provider = m_Provider;
		Queue( m_Host,
		    [provider, copy, atlas, delta, table, regions]() mutable
		    {
			    if ( copy.regions )
			    {
				    copy.regions = RegionsOrEmpty( regions );
				    copy.regionAtlas = DataOrNull( atlas );
				    copy.regionDelta = DataOrNull( delta );
			    }
			    else
			    {
				    copy.atlas = DataOrNull( atlas );
				    copy.deltaAtlas = DataOrNull( delta );
			    }
			    copy.gridTable = static_cast<const float *>( DataOrNull( table ) );
			    provider->UploadProbeVolume( copy );
		    } );
		return true;
	}

	bool UploadShadowField( const world_mesh_gpu::ShadowFieldUploadRequest &request ) override
	{
		if ( !Queued() )
			return m_Provider->UploadShadowField( request );
		const std::size_t voxels =
		    std::size_t( request.dims[0] ) * request.dims[1] * request.dims[2];
		std::shared_ptr<Bytes> distances =
		    CopyBytes( request.distances, voxels * sizeof( std::uint16_t ) );
		world_mesh_gpu::ShadowFieldUploadRequest copy = request;
		world_mesh_gpu::IWorldMeshUpload *provider = m_Provider;
		Queue( m_Host,
		    [provider, copy, distances]() mutable
		    {
			    copy.distances = static_cast<const std::uint16_t *>( DataOrNull( distances ) );
			    provider->UploadShadowField( copy );
		    } );
		return true;
	}

	bool UploadReflectionProbes(
	    const world_mesh_gpu::ReflectionProbesUploadRequest &request ) override
	{
		if ( !Queued() )
			return m_Provider->UploadReflectionProbes( request );
		std::shared_ptr<Bytes> texels = CopyBytes(
		    request.texels, std::size_t( request.width ) * request.height * kRgba16fTexelBytes );
		world_mesh_gpu::ReflectionProbesUploadRequest copy = request;
		world_mesh_gpu::IWorldMeshUpload *provider = m_Provider;
		Queue( m_Host,
		    [provider, copy, texels]() mutable
		    {
			    copy.texels = static_cast<const std::uint16_t *>( DataOrNull( texels ) );
			    provider->UploadReflectionProbes( copy );
		    } );
		return true;
	}

	bool DrawBatch( std::uint32_t firstIndex, std::uint32_t indexCount ) override
	{
		if ( !Queued() )
			return m_Provider->DrawBatch( firstIndex, indexCount );
		// The engine binds the batch's material (a queued call) first; the draw
		// follows it on the queue so the provider runs that material's pass.
		IMaterial *material = m_Host->boundMaterial ? m_Host->boundMaterial() : nullptr;
		Queue( m_Host,
		    [this, material, firstIndex, indexCount]()
		    {
			    ExecuteDrawBatch( material, firstIndex, indexCount );
		    } );
		std::lock_guard<std::mutex> lock( m_RejectedMutex );
		return m_Rejected.find( material ) == m_Rejected.end();
	}

	void Release() override
	{
		ForgetRejections();
		if ( !Queued() )
		{
			m_Provider->Release();
			return;
		}
		world_mesh_gpu::IWorldMeshUpload *provider = m_Provider;
		Queue( m_Host,
		    [provider]()
		    {
			    provider->Release();
		    } );
	}

	bool IsResident() const override { return m_Provider->IsResident(); }

private:
	// Whether calls go through the queue now.
	bool Queued() const { return Active( m_Host ); }

	// Runs on the render thread (a queued DrawBatch).
	void ExecuteDrawBatch( IMaterial *material, std::uint32_t firstIndex, std::uint32_t indexCount )
	{
		// Only a resident mesh's non-empty draw says anything about the material.
		if ( m_Provider->DrawBatch( firstIndex, indexCount ) || !indexCount ||
		     !m_Provider->IsResident() )
			return;
		std::lock_guard<std::mutex> lock( m_RejectedMutex );
		m_Rejected.insert( material );
	}

	void ForgetRejections()
	{
		std::lock_guard<std::mutex> lock( m_RejectedMutex );
		m_Rejected.clear();
	}

	world_mesh_gpu::IWorldMeshUpload *m_Provider;
	const RenderCallQueueHost *const &m_Host;
	// Materials whose queued batches the provider rejected, per map.
	std::mutex m_RejectedMutex;
	std::set<const IMaterial *> m_Rejected;
};

class QueuedCapabilities::LightSet final : public light_set::ILightSetConsumer
{
public:
	LightSet( light_set::ILightSetConsumer *provider, const RenderCallQueueHost *const &host )
	    : m_Provider( provider ), m_Host( host )
	{
	}

	void PublishLightSet( const light_set::Snapshot &snapshot ) override
	{
		if ( !Active( m_Host ) )
		{
			m_Provider->PublishLightSet( snapshot );
			return;
		}
		auto copy = std::make_shared<const light_set::Snapshot>( snapshot );
		light_set::ILightSetConsumer *provider = m_Provider;
		Queue( m_Host,
		    [provider, copy]()
		    {
			    provider->PublishLightSet( *copy );
		    } );
	}

private:
	light_set::ILightSetConsumer *m_Provider;
	const RenderCallQueueHost *const &m_Host;
};

// Slots land in the stream in the order of the calls around them.
class QueuedCapabilities::CoreSlots final : public ICorePassSlots
{
public:
	CoreSlots( ICorePassSlots *provider, const RenderCallQueueHost *const &host )
	    : m_Provider( provider ), m_Host( host )
	{
	}

	void MarkSlot( std::uint32_t tag ) override
	{
		if ( !Active( m_Host ) )
		{
			m_Provider->MarkSlot( tag );
			return;
		}
		ICorePassSlots *provider = m_Provider;
		Queue( m_Host,
		    [provider, tag]()
		    {
			    provider->MarkSlot( tag );
		    } );
	}

private:
	ICorePassSlots *m_Provider;
	const RenderCallQueueHost *const &m_Host;
};

QueuedCapabilities::QueuedCapabilities() = default;
QueuedCapabilities::~QueuedCapabilities() = default;

void QueuedCapabilities::BindQueue( const RenderCallQueueHost *host )
{
	m_Host = host;
}

void QueuedCapabilities::Adopt( const LegacyShaderServices &services )
{
	m_WorldMesh.reset();
	m_LightSet.reset();
	m_CoreSlots.reset();
	if ( services.corePassSlots )
		m_CoreSlots = std::make_unique<CoreSlots>( services.corePassSlots, m_Host );
	if ( services.worldMeshUpload )
		m_WorldMesh = std::make_unique<WorldMesh>( services.worldMeshUpload, m_Host );
	if ( services.lightSetConsumer )
		m_LightSet = std::make_unique<LightSet>( services.lightSetConsumer, m_Host );
	m_GpuCompute = services.gpuCompute;
}

world_mesh_gpu::IWorldMeshUpload *QueuedCapabilities::WorldMeshUpload()
{
	return m_WorldMesh.get();
}

light_set::ILightSetConsumer *QueuedCapabilities::LightSetConsumer()
{
	return m_LightSet.get();
}

ICorePassSlots *QueuedCapabilities::CorePassSlots()
{
	return m_CoreSlots.get();
}

} // namespace render::legacy
