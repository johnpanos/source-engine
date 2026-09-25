//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's RFC 0011 indirect-light host: the map's producer
//          switcher, r_indirect_producer, and the volume consumers sample.
//
//          The host runs on the main thread. CPU producers' jobs run in
//          order on it for now (the frame executor's serial mode); their
//          volumes are CPU data, uploaded to the renderer when the published
//          generation changes. The renderer retires each replaced upload
//          behind its own completion serial, so the host's resource tracker
//          only models frame latency for producers that own none.
//
//===========================================================================//

#include "indirect_light_host.h"

#include "convar.h"
#include "lightcache.h"
#include "materialsystem/imaterialsystem.h"
#include "render/indirect_switcher.h"
#include "render/light_set.h"
#include "render/world_mesh_upload.h"
#include "tier0/dbg.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
using namespace indirect_light;

void OnProducerChanged( IConVar *var, const char *oldValue, float oldFloat );
void ReportOffered();

ConVar r_indirect_producer( "r_indirect_producer", "baked", FCVAR_ARCHIVE,
    "Indirect-light producer (RFC 0011): baked, radiosity, sdf or rayquery; only the values in "
    "r_indirect_producer_offered are available on this device and map",
    OnProducerChanged );
ConVar r_indirect_producer_offered( "r_indirect_producer_offered", "baked", FCVAR_NONE,
    "The r_indirect_producer values this device and map offer (read-only; set per map)" );
// The contract's scripted fake, for switching tests only (a cheat: never offered to players).
ConVar r_indirect_test_fake( "r_indirect_test_fake", "0", FCVAR_CHEAT,
    "Offer the scripted fake producer (\"fake\") for switching tests; takes effect at the next "
    "map" );

// Frames in flight the host assumes for producers without device resources.
constexpr uint64_t kFrameLatency = 3;

class FrameTracker final : public IResourceTracker
{
public:
	uint64_t Acquire( size_t bytes ) override
	{
		m_live[++m_nextId] = bytes;
		return m_nextId;
	}
	void Release( uint64_t id, uint64_t afterSerial ) override { m_pending[id] = afterSerial; }
	uint64_t SubmittedSerial() const override { return m_frame; }
	uint64_t CompletedSerial() const override
	{
		return m_frame > kFrameLatency ? m_frame - kFrameLatency : 0;
	}
	void Advance()
	{
		++m_frame;
		for ( auto it = m_pending.begin(); it != m_pending.end(); )
		{
			if ( CompletedSerial() >= it->second )
			{
				m_live.erase( it->first );
				it = m_pending.erase( it );
			}
			else
				++it;
		}
	}
	uint64_t Frame() const { return m_frame; }

private:
	uint64_t m_nextId = 0;
	uint64_t m_frame = 0;
	std::map<uint64_t, size_t> m_live;
	std::map<uint64_t, uint64_t> m_pending;
};

class EngineCatalog final : public IProducerCatalog
{
public:
	bool Offers( ProducerKind kind, const IndirectScene &scene ) const override
	{
		if ( !scene.baked )
			return false;
		if ( kind == ProducerKind::Baked )
			return true;
		// The contract's scripted fake: switching tests only, never players.
		return kind == ProducerKind::ScriptedFake && r_indirect_test_fake.GetBool();
	}
	std::unique_ptr<IProducer> Create( ProducerKind kind ) override
	{
		if ( kind == ProducerKind::Baked )
			return std::make_unique<BakedProducer>();
		if ( kind == ProducerKind::ScriptedFake )
			return std::make_unique<ScriptedFakeProducer>();
		return nullptr;
	}
};

struct Host
{
	EngineCatalog catalog;
	FrameTracker tracker;
	std::unique_ptr<Switcher> switcher;
	IndirectScene scene;
	std::shared_ptr<const Volume> current;
	std::optional<mapcontainer::ProbeVolumeView> view;
	uint64_t uploadedGeneration = 0;
	uint64_t mapSerial = 0;
	bool deviceLost = false;
	bool reverting = false;
};

Host &TheHost()
{
	static Host host;
	return host;
}

world_mesh_gpu::IWorldMeshUpload *Uploader()
{
	return materials ? static_cast<world_mesh_gpu::IWorldMeshUpload *>(
	                       materials->QueryInterface( world_mesh_gpu::kWorldMeshUploadInterface ) )
	                 : nullptr;
}

// Makes `frame` what consumers sample: the CPU view (the ambient cube) and
// the renderer's per-pixel copy.
void Consume( const FrameVolume &frame )
{
	Host &host = TheHost();
	if ( !frame.volume || frame.generation == host.uploadedGeneration )
		return;
	host.uploadedGeneration = frame.generation;
	host.current = frame.volume;
	host.view.emplace( host.current->bytes.data(), host.current->layout );
	// Every model's ambient cube is re-evaluated from the new volume.
	R_StudioInitLightingCache();
	world_mesh_gpu::IWorldMeshUpload *uploader = Uploader();
	if ( host.deviceLost || !uploader || !uploader->IsResident() )
		return;
	const mapcontainer::ProbeVolumeLayout &layout = host.current->layout;
	std::vector<float> table( layout.gridCount * mapcontainer::kProbeGridTableFloats );
	mapcontainer::WriteProbeGridTable( layout, table.data() );
	world_mesh_gpu::ProbeVolumeUploadRequest request;
	request.atlasWidth = layout.atlasWidth;
	request.atlasHeight = layout.atlasHeight;
	request.atlas = host.current->bytes.data() + layout.atlasOffset;
	request.gridCount = layout.gridCount;
	request.tableFloats = mapcontainer::kProbeGridTableFloats;
	request.gridTable = table.data();
	if ( !uploader->UploadProbeVolume( request ) )
		Warning( "indirect light: probe volume upload failed; models use the ambient cube\n" );
}

void ReportOffered()
{
	Host &host = TheHost();
	std::string offered;
	for ( ProducerKind kind : { ProducerKind::Baked, ProducerKind::PrecomputedRadiosity,
	          ProducerKind::SdfTraced, ProducerKind::RayQuery, ProducerKind::ScriptedFake } )
	{
		if ( host.catalog.Offers( kind, host.scene ) )
		{
			if ( !offered.empty() )
				offered += ' ';
			offered += ProducerName( kind );
		}
	}
	r_indirect_producer_offered.SetValue( offered.c_str() );
}

void OnProducerChanged( IConVar *var, const char *oldValue, float )
{
	Host &host = TheHost();
	if ( host.reverting || !host.switcher )
		return;
	ConVarRef ref( var );
	ProducerKind kind;
	const char *requested = ref.GetString();
	const auto revert = [&]( const char *reason )
	{
		Warning( "r_indirect_producer \"%s\" unavailable (%s); keeping %s\n", requested, reason,
		    ProducerName( host.switcher->Selected() ) );
		host.reverting = true;
		ref.SetValue( oldValue );
		host.reverting = false;
	};
	if ( !ParseProducer( requested, &kind ) )
	{
		revert( "unknown-producer" );
		return;
	}
	ReportOffered();
	const auto selected = host.switcher->Select( kind );
	if ( !selected )
	{
		revert( IndirectErrorName( selected.Error() ) );
		return;
	}
	Msg( "indirect light: switching to %s\n", ProducerName( kind ) );
}

} // namespace

void IndirectLight_BeginMap( const unsigned char *prbv, size_t size )
{
	Host &host = TheHost();
	IndirectLight_EndMap();
	auto baked = Volume::FromBytes( std::vector<unsigned char>( prbv, prbv + size ) );
	if ( !baked )
		return;
	host.scene = IndirectScene();
	host.scene.mapSerial = ++host.mapSerial;
	host.scene.baked = baked;
	host.switcher = std::make_unique<Switcher>( host.catalog, host.tracker );
	ReportOffered();
	ProducerKind requested = ProducerKind::Baked;
	const char *saved = r_indirect_producer.GetString();
	const bool parsed = ParseProducer( saved, &requested );
	const auto begun =
	    host.switcher->BeginMap( host.scene, parsed ? requested : ProducerKind::Baked );
	if ( !parsed || !begun )
	{
		// The saved value stays; the default is used and the substitution recorded.
		Warning( "indirect light: r_indirect_producer \"%s\" is not offered here (%s); using baked "
		         "(offered: %s)\n",
		    saved, parsed ? IndirectErrorName( begun.Error() ) : "unknown-producer",
		    r_indirect_producer_offered.GetString() );
	}
	FrameWork work;
	work.frameSerial = host.tracker.Frame() + 1;
	work.resources = &host.tracker;
	const light_set::Snapshot none;
	Consume( host.switcher->Frame( work, none ) );
	for ( auto &job : work.jobs )
		job();
	host.tracker.Advance();
	Msg( "indirect light: map %llu, producer %s (offered: %s)\n",
	    (unsigned long long)host.scene.mapSerial, ProducerName( host.switcher->Active() ),
	    r_indirect_producer_offered.GetString() );
}

void IndirectLight_EndMap()
{
	Host &host = TheHost();
	if ( host.switcher )
		host.switcher->EndMap();
	host.switcher.reset();
	host.view.reset();
	host.current.reset();
	host.uploadedGeneration = 0;
	host.scene = IndirectScene();
}

void IndirectLight_Frame( const light_set::Snapshot &lights )
{
	Host &host = TheHost();
	if ( !host.switcher )
		return;
	FrameWork work;
	work.frameSerial = host.tracker.Frame() + 1;
	work.resources = &host.tracker;
	const FrameVolume frame = host.switcher->Frame( work, lights );
	// The frame executor's serial mode: jobs in order on this thread.
	for ( auto &job : work.jobs )
		job();
	host.tracker.Advance();
	Consume( frame );
}

const mapcontainer::ProbeVolumeView *IndirectLight_CurrentVolume()
{
	Host &host = TheHost();
	return host.view ? &*host.view : nullptr;
}

void IndirectLight_Background()
{
	if ( !TheHost().switcher )
		return;
	TheHost().switcher->Background();
	Msg( "indirect light: background (%s stops scheduling)\n",
	    ProducerName( TheHost().switcher->Active() ) );
}

void IndirectLight_Resume()
{
	if ( !TheHost().switcher )
		return;
	TheHost().switcher->Resume();
	Msg( "indirect light: resume (%s)\n", ProducerName( TheHost().switcher->Active() ) );
}

void IndirectLight_DeviceLost()
{
	Host &host = TheHost();
	host.deviceLost = true;
	if ( host.switcher )
		host.switcher->DeviceLost();
}

void IndirectLight_DeviceRestored()
{
	Host &host = TheHost();
	host.deviceLost = false;
	if ( !host.switcher )
		return;
	const auto recovered = host.switcher->DeviceRecovered();
	if ( !recovered )
		Warning( "indirect light: %s is unavailable after device recovery (%s); using baked\n",
		    r_indirect_producer.GetString(), IndirectErrorName( recovered.Error() ) );
	// The recreated device gets the volume again.
	host.uploadedGeneration = 0;
	FrameWork work;
	work.frameSerial = host.tracker.Frame() + 1;
	work.resources = &host.tracker;
	const light_set::Snapshot none;
	const FrameVolume frame = host.switcher->Frame( work, none );
	for ( auto &job : work.jobs )
		job();
	host.tracker.Advance();
	Consume( frame );
}
