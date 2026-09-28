//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.legacy-capabilities (RFC 0016 K3 "Side channels gone"):
//			the legacy frontend's frame-ordered capabilities
//			(render/legacy/capabilities.h) over a fake render call queue.
//
//			Without a queue, or while the calling context does not queue,
//			every call reaches the backend at once. While it queues, nothing
//			reaches the backend until the queue runs, calls run in the order
//			they were made among the context's own queued calls, each call
//			owns copies of the bytes it borrowed (the caller's buffers are
//			overwritten before the queue runs), a flushed queue releases every
//			payload without running it, and a queued DrawBatch learns the
//			backend's rejection of its material one frame late, forgotten at
//			the next Upload. The light set is copied the same way.
//
//			Seeded defects (sensitivity rows) live in queued_capabilities.cpp:
//			RENDER_LEGACY_CAPABILITIES_SEEDED_BORROWED_BYTES and
//			RENDER_LEGACY_CAPABILITIES_SEEDED_UNORDERED.
//
//=============================================================================//

#include "../../../../render/legacy/queued_capabilities.h"
#include "testing/checks.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{

using namespace render::legacy;

// The fake material system's render call queue.
struct FakeQueue
{
	struct Entry
	{
		void ( *call )( void * );
		void *payload;
		void ( *destroy )( void * );
		std::string marker; // a context call of the fake material system
	};

	bool active = false;
	bool refuse = false; // stops queueing between active() and queue()
	IMaterial *bound = nullptr;
	std::vector<Entry> entries;
	std::vector<std::string> *log = nullptr;
	int queued = 0;
	int destroyed = 0;

	void Context( const char *marker )
	{
		entries.push_back( { nullptr, nullptr, nullptr, marker } );
	}

	void Run()
	{
		std::vector<Entry> pending;
		pending.swap( entries );
		for ( Entry &entry : pending )
		{
			if ( !entry.call )
			{
				log->push_back( entry.marker );
				continue;
			}
			entry.call( entry.payload );
			entry.destroy( entry.payload );
			++destroyed;
		}
	}

	void Flush()
	{
		std::vector<Entry> pending;
		pending.swap( entries );
		for ( Entry &entry : pending )
		{
			if ( entry.call )
			{
				entry.destroy( entry.payload );
				++destroyed;
			}
		}
	}
};

FakeQueue *g_Queue = nullptr;

RenderCallQueueHost MakeHost()
{
	RenderCallQueueHost host;
	host.active = []()
	{
		return g_Queue->active;
	};
	host.queue = []( void ( *call )( void * ), void *payload, void ( *destroy )( void * ) )
	{
		if ( !g_Queue->active || g_Queue->refuse )
			return false;
		g_Queue->entries.push_back( { call, payload, destroy, {} } );
		++g_Queue->queued;
		return true;
	};
	host.boundMaterial = []()
	{
		return g_Queue->bound;
	};
	return host;
}

class RecordingWorldMesh final : public world_mesh_gpu::IWorldMeshUpload
{
public:
	explicit RecordingWorldMesh( std::vector<std::string> &log ) : m_Log( log ) {}

	bool Upload( const world_mesh_gpu::WorldMeshUploadRequest &request ) override
	{
		const auto *vertices = static_cast<const unsigned char *>( request.vertices );
		const auto *indices = static_cast<const unsigned char *>( request.indices );
		m_Log.push_back( "upload " + std::to_string( vertices ? vertices[0] : -1 ) + " " +
		                 std::to_string( indices ? indices[0] : -1 ) );
		return true;
	}
	bool UploadLightmap( const world_mesh_gpu::WorldLightmapUploadRequest &request ) override
	{
		const auto *layer = static_cast<const unsigned char *>( request.layers[0] );
		m_Log.push_back( "lightmap " + std::to_string( layer ? layer[7] : -1 ) );
		return true;
	}
	bool UploadProbeVolume( const world_mesh_gpu::ProbeVolumeUploadRequest & ) override
	{
		m_Log.push_back( "probes" );
		return true;
	}
	bool UploadShadowField( const world_mesh_gpu::ShadowFieldUploadRequest &request ) override
	{
		m_Log.push_back(
		    "shadow " + std::to_string( request.distances ? request.distances[5] : -1 ) );
		return true;
	}
	bool UploadReflectionProbes( const world_mesh_gpu::ReflectionProbesUploadRequest & ) override
	{
		m_Log.push_back( "reflection" );
		return true;
	}
	// Rejects batches whose first index is 99.
	bool DrawBatch( std::uint32_t firstIndex, std::uint32_t indexCount ) override
	{
		m_Log.push_back( "draw " + std::to_string( firstIndex ) );
		return firstIndex != 99 || !indexCount;
	}
	void Release() override { m_Log.push_back( "release" ); }
	bool IsResident() const override { return true; }

private:
	std::vector<std::string> &m_Log;
};

class RecordingLightSet final : public light_set::ILightSetConsumer
{
public:
	explicit RecordingLightSet( std::vector<std::string> &log ) : m_Log( log ) {}
	void PublishLightSet( const light_set::Snapshot &snapshot ) override
	{
		m_Log.push_back( "lights " + std::to_string( snapshot.lights.size() ) + " " +
		                 std::to_string( snapshot.epoch ) );
	}

private:
	std::vector<std::string> &m_Log;
};

std::string Join( const std::vector<std::string> &log )
{
	std::string joined;
	for ( const std::string &entry : log )
		joined += ( joined.empty() ? "" : "; " ) + entry;
	return joined;
}

} // namespace

int main()
{
	testing::Checks checks;
	std::vector<std::string> log;
	FakeQueue queue;
	queue.log = &log;
	g_Queue = &queue;
	const RenderCallQueueHost host = MakeHost();
	RecordingWorldMesh mesh( log );
	RecordingLightSet lights( log );
	gpu_compute::IGpuCompute *const compute = reinterpret_cast<gpu_compute::IGpuCompute *>( &log );

	QueuedCapabilities capabilities;
	checks.That( !capabilities.WorldMeshUpload() && !capabilities.LightSetConsumer() &&
	                 !capabilities.GpuCompute(),
	    "adopt.nothing-before-a-backend" );
	render::LegacyShaderServices services;
	services.worldMeshUpload = &mesh;
	services.lightSetConsumer = &lights;
	services.gpuCompute = compute;
	capabilities.Adopt( services );
	world_mesh_gpu::IWorldMeshUpload *upload = capabilities.WorldMeshUpload();
	light_set::ILightSetConsumer *publish = capabilities.LightSetConsumer();
	if ( !checks.That( upload && publish, "adopt.the-backend-capabilities-are-offered" ) )
		return checks.Report();
	checks.That( capabilities.GpuCompute() == compute, "adopt.compute-passes-through" );

	unsigned char vertices[4] = { 11, 0, 0, 0 };
	unsigned char indices[4] = { 22, 0, 0, 0 };
	world_mesh_gpu::WorldMeshUploadRequest request;
	request.vertices = vertices;
	request.vertexBytes = sizeof( vertices );
	request.indices = indices;
	request.indexBytes = sizeof( indices );

	// No queue bound, then a bound queue that is not queueing: direct.
	upload->Upload( request );
	checks.Equal( Join( log ), std::string( "upload 11 22" ), "direct.without-a-queue" );
	log.clear();
	capabilities.BindQueue( &host );
	upload->Upload( request );
	checks.Equal(
	    Join( log ), std::string( "upload 11 22" ), "direct.while-the-context-does-not-queue" );
	checks.Equal( queue.queued, 0, "direct.nothing-queued" );
	log.clear();

	// Queued: ordered among the context's calls, with copied bytes.
	queue.active = true;
	queue.Context( "context-a" );
	const bool accepted = upload->Upload( request );
	vertices[0] = 99;
	indices[0] = 98;
	unsigned char layer[16] = {};
	layer[7] = 33;
	world_mesh_gpu::WorldLightmapUploadRequest lightmap;
	lightmap.width = 2;
	lightmap.height = 1;
	lightmap.layerCount = 1;
	lightmap.layers[0] = layer;
	upload->UploadLightmap( lightmap );
	layer[7] = 0;
	std::uint16_t distances[8] = { 0, 0, 0, 0, 0, 44, 0, 0 };
	world_mesh_gpu::ShadowFieldUploadRequest shadow;
	shadow.dims[0] = 2;
	shadow.dims[1] = 2;
	shadow.dims[2] = 2;
	shadow.distances = distances;
	upload->UploadShadowField( shadow );
	distances[5] = 0;
	queue.Context( "context-b" );
	light_set::Snapshot snapshot;
	snapshot.epoch = 7;
	snapshot.lights.resize( 3 );
	publish->PublishLightSet( snapshot );
	snapshot.lights.clear();
	upload->Release();
	checks.That( accepted, "queued.an-upload-is-accepted-for-the-frame" );
	checks.Equal(
	    log.size(), std::size_t( 0 ), "queued.nothing-reaches-the-backend-before-the-queue-runs" );
	queue.Run();
	checks.Equal( Join( log ),
	    std::string(
	        "context-a; upload 11 22; lightmap 33; shadow 44; context-b; lights 3 7; release" ),
	    "queued.calls-run-in-order-with-copied-bytes" );
	checks.Equal( queue.destroyed, queue.queued, "queued.every-payload-is-released" );
	std::printf( "INFO legacy capabilities: queued run: %s\n", Join( log ).c_str() );
	log.clear();

	// A flushed queue releases its payloads unrun.
	const int before = queue.destroyed;
	upload->Upload( request );
	publish->PublishLightSet( snapshot );
	queue.Flush();
	checks.That( log.empty() && queue.destroyed == before + 2, "flush.payloads-released-unrun" );

	// DrawBatch: the rejection of a material is learned one frame late and
	// forgotten at the next Upload.
	int a = 0, b = 0;
	IMaterial *materialA = reinterpret_cast<IMaterial *>( &a );
	IMaterial *materialB = reinterpret_cast<IMaterial *>( &b );
	queue.bound = materialA;
	const bool firstA = upload->DrawBatch( 99, 6 );
	queue.bound = materialB;
	const bool firstB = upload->DrawBatch( 12, 6 );
	queue.Run();
	queue.bound = materialA;
	const bool secondA = upload->DrawBatch( 99, 6 );
	queue.bound = materialB;
	const bool secondB = upload->DrawBatch( 12, 6 );
	queue.Run();
	checks.That( firstA && firstB, "draw.accepted-until-the-backend-has-run" );
	checks.That( !secondA && secondB, "draw.a-rejected-material-is-reported-the-next-frame" );
	upload->Upload( request );
	queue.Run();
	queue.bound = materialA;
	checks.That( upload->DrawBatch( 99, 6 ), "draw.an-upload-forgets-rejections" );
	queue.Run();
	log.clear();

	// The context stops queueing between the check and the call: the call
	// runs at once, in order.
	queue.refuse = true;
	upload->UploadShadowField( shadow );
	checks.Equal( Join( log ), std::string( "shadow 0" ), "refused.the-call-runs-at-once" );
	queue.refuse = false;
	queue.active = false;
	log.clear();

	// A new backend replaces the old one's capabilities.
	capabilities.Adopt( render::LegacyShaderServices() );
	checks.That( !capabilities.WorldMeshUpload() && !capabilities.LightSetConsumer() &&
	                 !capabilities.GpuCompute(),
	    "adopt.a-backend-without-capabilities-offers-none" );
	return checks.Report();
}
