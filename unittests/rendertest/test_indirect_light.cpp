//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.indirect-light.v1 and render.indirect-switching (RFC 0011
//          G3, G4): the shared producer suite run against the Baked producer,
//          the precomputed radiosity producer, a scripted fake and each
//          deliberately bad producer, then the
//          switcher's scenarios (baked <-> fake, a failed Begin, device loss
//          mid-fade, backgrounding, map change) against a fake GPU timeline.
//
//===========================================================================//

#include "render/indirect_light.h"
#include "render/indirect_radiosity.h"
#include "indirect_contract.h"
#include "render/indirect_switcher.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace
{
using namespace indirect_light;
using namespace indirect_contract;
using indirect_policy::Policy;

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool condition, const std::string &description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description.c_str() );
	}
}

// texel, and (optionally) layer 0 recomposed as seed direct + new indirect.
std::shared_ptr<Volume> Scaled( const Volume &seed, uint32_t layer, float scale )
{
	auto volume = std::make_shared<Volume>( seed );
	for ( uint32_t g = 0; g < volume->layout.gridCount; ++g )
	{
		const mapcontainer::ProbeGridLayout &grid = volume->layout.grids[g];
		const uint32_t rows = ( grid.probeCount + grid.tilesPerRow - 1 ) / grid.tilesPerRow;
		for ( uint32_t y = 0; y < rows * mapcontainer::kProbeIrradianceTile; ++y )
			for ( uint32_t x = 0; x < grid.tilesPerRow * mapcontainer::kProbeIrradianceTile; ++x )
				for ( int c = 0; c < 3; ++c )
				{
					const auto at = [&]( uint32_t l )
					{
						return volume->bytes.data() + volume->layout.atlasOffset +
						       ( uint64_t( grid.irradianceOrigin[l][1] + y ) *
						               volume->layout.atlasWidth +
						           grid.irradianceOrigin[l][0] + x ) *
						           8 +
						       2 * c;
					};
					uint16_t half;
					std::memcpy( &half, at( layer ), 2 );
					const uint16_t scaled =
					    FloatToHalf( mapcontainer::HalfToFloat( half ) * scale );
					std::memcpy( at( layer ), &scaled, 2 );
				}
	}
	return volume;
}

// Recomposes layer 0 (total) as the seed's direct light plus layer 1; with
// `doubleCount`, as the seed's whole total plus layer 1 (the defect).
void Recompose( Volume &volume, const Volume &seed, bool doubleCount = false )
{
	for ( uint32_t g = 0; g < volume.layout.gridCount; ++g )
	{
		const mapcontainer::ProbeGridLayout &grid = volume.layout.grids[g];
		const uint32_t rows = ( grid.probeCount + grid.tilesPerRow - 1 ) / grid.tilesPerRow;
		for ( uint32_t y = 0; y < rows * mapcontainer::kProbeIrradianceTile; ++y )
			for ( uint32_t x = 0; x < grid.tilesPerRow * mapcontainer::kProbeIrradianceTile; ++x )
				for ( int c = 0; c < 3; ++c )
				{
					const auto offset = [&]( uint32_t l )
					{
						return size_t( volume.layout.atlasOffset +
						               ( uint64_t( grid.irradianceOrigin[l][1] + y ) *
						                       volume.layout.atlasWidth +
						                   grid.irradianceOrigin[l][0] + x ) *
						                   8 +
						               2 * c );
					};
					uint16_t seedTotal, seedIndirect, indirect;
					std::memcpy( &seedTotal, seed.bytes.data() + offset( 0 ), 2 );
					std::memcpy( &seedIndirect, seed.bytes.data() + offset( 1 ), 2 );
					std::memcpy( &indirect, volume.bytes.data() + offset( 1 ), 2 );
					const float total =
					    mapcontainer::HalfToFloat( seedTotal ) -
					    ( doubleCount ? 0.0f : mapcontainer::HalfToFloat( seedIndirect ) ) +
					    mapcontainer::HalfToFloat( indirect );
					const uint16_t half = FloatToHalf( total );
					std::memcpy( volume.bytes.data() + offset( 0 ), &half, 2 );
				}
	}
}

// Visibility stripped: every probe believes it sees 200 units in every direction.
void StripVisibility( Volume &volume )
{
	for ( uint32_t g = 0; g < volume.layout.gridCount; ++g )
	{
		const mapcontainer::ProbeGridLayout &grid = volume.layout.grids[g];
		const uint32_t rows = ( grid.probeCount + grid.tilesPerRow - 1 ) / grid.tilesPerRow;
		for ( uint32_t y = 0; y < rows * mapcontainer::kProbeVisibilityTile; ++y )
			for ( uint32_t x = 0; x < grid.tilesPerRow * mapcontainer::kProbeVisibilityTile; ++x )
			{
				unsigned char *texel =
				    volume.bytes.data() + volume.layout.atlasOffset +
				    ( uint64_t( grid.visibilityOrigin[1] + y ) * volume.layout.atlasWidth +
				        grid.visibilityOrigin[0] + x ) *
				        8;
				const uint16_t one = FloatToHalf( 1.0f );
				std::memcpy( texel, &one, 2 );
				std::memcpy( texel + 2, &one, 2 );
			}
	}
}

enum Defect
{
	kNoDefect,
	kPublishBeforeSeed, // a black first publication
	kIgnoreVisibility,  // leaks
	kFreeEarly,         // frees what a pending frame reads
	kReuseEpoch,
	kClaimsGeometryButIgnores,
	kDoubleCount, // total = seed total + its indirect under RuntimeIndirect
	kBlocksOnGpu,
	kFailBegin, // a legitimate failure: the switcher must keep the old producer
};

class ScriptedFake final : public IProducer
{
public:
	explicit ScriptedFake(
	    Defect defect = kNoDefect, bool *destroyedEarly = nullptr, const FakeGpu *gpu = nullptr )
	    : m_defect( defect ), m_destroyedEarly( destroyedEarly ), m_gpu( gpu )
	{
	}
	~ScriptedFake() override
	{
		if ( m_destroyedEarly && m_gpu && m_ended && m_gpu->CompletedSerial() < m_ticket )
			*m_destroyedEarly = true;
	}
	ProducerCaps Caps() const override
	{
		ProducerCaps caps;
		caps.kind = ProducerKind::ScriptedFake;
		caps.responds = kGeometryMotion;
		caps.policies = PolicyBit( Policy::Baked ) | PolicyBit( Policy::RuntimeIndirect );
		caps.convergenceFrames = 4;
		return caps;
	}
	foundation::Expected<void, IndirectError> Begin( const IndirectScene &scene,
	    const PublishedVolume &seed, IResourceTracker &resources ) override
	{
		if ( m_defect == kFailBegin )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		if ( !scene.baked )
			return foundation::MakeUnexpected( IndirectError::MissingSceneData );
		if ( !( Caps().policies & PolicyBit( scene.policy ) ) )
			return foundation::MakeUnexpected( IndirectError::UnsupportedPolicy );
		m_seed = seed.volume ? seed.volume : scene.baked;
		m_policy = scene.policy;
		m_resources = &resources;
		m_resource = resources.Acquire( m_seed->bytes.size() );
		m_frames = 0;
		if ( m_defect == kPublishBeforeSeed )
			m_published = PublishedVolume{ ++m_epoch, Scaled( *m_seed, 0, 0.0f ) };
		return {};
	}
	void Schedule( FrameWork &work, const light_set::Snapshot & ) override
	{
		if ( m_defect == kBlocksOnGpu )
		{
			// Waits for the GPU to catch up: forbidden in Schedule.
			const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds( 20 );
			while ( work.resources->CompletedSerial() < work.resources->SubmittedSerial() &&
			        std::chrono::steady_clock::now() < until )
				std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
		for ( const SceneChange &change : work.changes )
			if ( change.response == kGeometryMotion && m_defect != kClaimsGeometryButIgnores )
				m_target = 1.0f + kDoorGain * change.value;
		// The update runs on the frame's executor.
		work.jobs.push_back(
		    [this]
		    {
			    Update();
		    } );
	}
	std::optional<PublishedVolume> Published() const override { return m_published; }
	RetireTicket End() override
	{
		m_ended = true;
		const uint64_t after = m_defect == kFreeEarly ? 0 : m_resources->SubmittedSerial();
		m_resources->Release( m_resource, after );
		m_ticket = after;
		return { after };
	}

private:
	void Update()
	{
		++m_frames;
		if ( m_frames < 2 )
			return; // a real producer converges before its first publication
		// A linear approach to the steady state over convergenceFrames.
		const float step = kDoorGain / float( Caps().convergenceFrames );
		m_scale += std::clamp( m_target - m_scale, -step, step );
		if ( m_published && m_published->volume && m_defect != kPublishBeforeSeed &&
		     std::fabs( m_scale - m_lastScale ) < 1e-6f )
			return; // nothing changed
		m_lastScale = m_scale;
		auto volume = Scaled( *m_seed, 1, m_scale );
		// Total = the seed's direct light plus this producer's indirect; the
		// defect adds its indirect to the seed's whole total under
		// RuntimeIndirect.
		Recompose(
		    *volume, *m_seed, m_defect == kDoubleCount && m_policy == Policy::RuntimeIndirect );
		if ( m_defect == kIgnoreVisibility )
			StripVisibility( *volume );
		const uint64_t epoch = m_defect == kReuseEpoch && m_epoch ? m_epoch : ++m_epoch;
		m_published = PublishedVolume{ epoch, volume };
	}

	Defect m_defect;
	bool *m_destroyedEarly;
	const FakeGpu *m_gpu;
	std::shared_ptr<const Volume> m_seed;
	Policy m_policy = Policy::Baked;
	IResourceTracker *m_resources = nullptr;
	uint64_t m_resource = 0;
	std::optional<PublishedVolume> m_published;
	uint64_t m_epoch = 0;
	uint32_t m_frames = 0;
	float m_target = 1.0f;
	float m_scale = 1.0f;
	float m_lastScale = -1.0f;
	bool m_ended = false;
	uint64_t m_ticket = 0;
};

class Catalog final : public IProducerCatalog
{
public:
	explicit Catalog( const FakeGpu &gpu ) : m_gpu( gpu ) {}
	bool Offers( ProducerKind kind, const IndirectScene & ) const override
	{
		return kind == ProducerKind::Baked || kind == ProducerKind::ScriptedFake;
	}
	std::unique_ptr<IProducer> Create( ProducerKind kind ) override
	{
		if ( kind == ProducerKind::Baked )
			return std::make_unique<BakedProducer>();
		if ( kind == ProducerKind::ScriptedFake )
		{
			++created;
			return std::make_unique<ScriptedFake>( fakeDefect, &destroyedEarly, &m_gpu );
		}
		return nullptr;
	}
	Defect fakeDefect = kNoDefect;
	bool destroyedEarly = false;
	int created = 0;

private:
	const FakeGpu &m_gpu;
};

// Runs frames through a switcher; returns the lowest frame mean relative to
// the seed and the number of frames.
struct Run
{
	float lowest = 1e9f;
	uint32_t fadeFrames = 0;
};

FrameVolume Step( Switcher &switcher, FakeGpu &gpu, Run *run, float seedMean )
{
	FrameWork work;
	work.frameSerial = gpu.SubmittedSerial() + 1;
	work.resources = &gpu;
	const light_set::Snapshot lights;
	const FrameVolume frame = switcher.Frame( work, lights );
	for ( auto &job : work.jobs )
		job();
	gpu.Submit();
	if ( run && frame.volume )
	{
		run->lowest = std::min( run->lowest, frame.volume->MeanIrradiance( 0 ) / seedMean );
		run->fadeFrames += frame.fading;
	}
	return frame;
}

} // namespace

int main()
{
	const std::shared_ptr<const Volume> seed = LoadSeed();
	Check( seed != nullptr, "the contract seed (quality/fixtures/gi/prbv/contract.prbv) loads" );
	if ( !seed )
		return testing::ReportConformance( g_checks, g_failures );
	Check( std::fabs( DirectLight( *seed ) - 0.5f * 0.3f ) < 0.01f && DarkSample( *seed ) >= 0.0f &&
	           DarkSample( *seed ) < 0.02f,
	    "the seed's lit side holds the furnace's direct light and its dark side none" );
	const std::shared_ptr<const Transfer> transfer = LoadTransfer( *seed );
	Check( transfer != nullptr,
	    "the seed's transfer (quality/fixtures/gi/rtrn/contract.rtrn) loads and pairs with it" );
	if ( !transfer )
		return testing::ReportConformance( g_checks, g_failures );
	// The fake's world: its scripted door, opened, raises the indirect light
	// by kDoorGain.
	Scenario world;
	world.seed = seed;
	world.transfer = transfer;
	world.geometryChanges = { { kGeometryMotion, 1, 1.0f } };
	world.geometryOracle = []( const Volume &volume, const Volume &seed, const ProducerCaps &caps )
	{
		const float expected = seed.MeanIrradiance( 1 ) * ( 1.0f + kDoorGain );
		return std::fabs( volume.MeanIrradiance( 1 ) - expected ) <= caps.responseTolerance * expected
		           ? std::string()
		           : std::string( "its indirect light is not the opened door's steady state within "
		                          "convergenceFrames" );
	};

	// The shared suite: the good producers pass...
	{
		const auto broken = RunContract(
		    []( const FakeGpu & )
		    {
			    return std::make_unique<BakedProducer>();
		    },
		    world, nullptr );
		for ( const auto &b : broken )
			std::fprintf( stderr, "  baked: %s\n", b.c_str() );
		Check( broken.empty(), "the Baked producer passes the shared suite" );
	}
	{
		bool early = false;
		const auto broken = RunContract(
		    [&]( const FakeGpu &gpu )
		    {
			    return std::make_unique<ScriptedFake>( kNoDefect, &early, &gpu );
		    },
		    world, &early );
		for ( const auto &b : broken )
			std::fprintf( stderr, "  fake: %s\n", b.c_str() );
		Check( broken.empty(), "the scripted fake passes the shared suite" );
	}
	{
		const auto broken = RunContract(
		    []( const FakeGpu & )
		    {
			    return std::make_unique<ScriptedFakeProducer>( 1.0f, 2 );
		    },
		    world, nullptr );
		for ( const auto &b : broken )
			std::fprintf( stderr, "  product fake: %s\n", b.c_str() );
		Check( broken.empty(), "the product's scripted fake passes the shared suite" );
	}
	{
		const auto broken = RunContract(
		    []( const FakeGpu & )
		    {
			    return std::make_unique<RadiosityProducer>();
		    },
		    world, nullptr );
		for ( const auto &b : broken )
			std::fprintf( stderr, "  radiosity: %s\n", b.c_str() );
		Check( broken.empty(), "the precomputed radiosity producer passes the shared suite" );
	}
	{
		// The one-bounce radiosity defect claims LightIntensity but converges
		// to one bounce of the change only.
		RadiosityOptions oneBounce;
		oneBounce.oneBounce = true;
		const auto broken = RunContract(
		    [&]( const FakeGpu & )
		    {
			    return std::make_unique<RadiosityProducer>( oneBounce );
		    },
		    world, nullptr );
		bool caught = false;
		for ( const auto &b : broken )
			caught |= b.find( "claims LightIntensity" ) != std::string::npos;
		std::fprintf( stderr, "bad producer (one-bounce radiosity): %zu broken obligation(s)%s\n",
		    broken.size(), caught ? "" : " -- NOT caught" );
		for ( const auto &b : broken )
			std::fprintf( stderr, "    %s\n", b.c_str() );
		Check( caught, "the suite rejects a one-bounce radiosity producer" );
	}
	// ...and each deliberately bad producer fails it, for its own defect.
	const struct
	{
		Defect defect;
		const char *name;
		const char *expected;
	} bad[] = {
	    { kPublishBeforeSeed, "publishes before seeding", "darker than the seed" },
	    { kIgnoreVisibility, "ignores visibility", "leaks light" },
	    { kFreeEarly, "frees a volume a pending frame reads", "lifetime:" },
	    { kReuseEpoch, "reuses an epoch", "epoch" },
	    { kClaimsGeometryButIgnores, "claims GeometryMotion without responding",
	        "claims GeometryMotion" },
	    { kDoubleCount, "double-counts under RuntimeIndirect", "double count" },
	    { kBlocksOnGpu, "blocks on the GPU inside Schedule", "waits for the GPU" },
	};
	for ( const auto &entry : bad )
	{
		bool early = false;
		const auto broken = RunContract(
		    [&]( const FakeGpu &gpu )
		    {
			    return std::make_unique<ScriptedFake>( entry.defect, &early, &gpu );
		    },
		    world, &early );
		bool caught = false;
		for ( const auto &b : broken )
			caught |= b.find( entry.expected ) != std::string::npos;
		std::fprintf( stderr, "bad producer (%s): %zu broken obligation(s)%s\n", entry.name,
		    broken.size(), caught ? "" : " -- NOT caught" );
		for ( const auto &b : broken )
			std::fprintf( stderr, "    %s\n", b.c_str() );
		Check( caught, std::string( "the suite rejects a producer that " ) + entry.name );
	}

	// render.indirect-switching: baked -> fake with a fade.
	const float seedMean = seed->MeanIrradiance( 0 );
	constexpr float kBlackTolerance = 0.05f;
	{
		FakeGpu gpu;
		Catalog catalog( gpu );
		Switcher switcher( catalog, gpu, 8 );
		IndirectScene scene;
		scene.baked = seed;
		Check( bool( switcher.BeginMap( scene, ProducerKind::Baked ) ),
		    "a map begins with the Baked producer" );
		Run run;
		for ( int i = 0; i < 3; ++i )
			Step( switcher, gpu, &run, seedMean );
		Check( bool( switcher.Select( ProducerKind::ScriptedFake ) ) &&
		           switcher.Pending() == ProducerKind::ScriptedFake &&
		           switcher.Active() == ProducerKind::Baked,
		    "selecting the fake begins it while Baked stays published" );
		for ( int i = 0; i < 20; ++i )
			Step( switcher, gpu, &run, seedMean );
		Check( switcher.Active() == ProducerKind::ScriptedFake && !switcher.Pending(),
		    "the fake becomes active after its first publication and the fade" );
		Check( run.fadeFrames >= 7, "the switch fades over the declared frames" );
		Check( run.lowest >= 1.0f - kBlackTolerance,
		    "no black frame: every frame's mean stays within tolerance of the seed's" );
		Check( switcher.PeakResidentBytes() <= 3 * seed->bytes.size(),
		    "peak resident volumes during the fade: the old, the new and the blend" );
		for ( int i = 0; i < 5; ++i )
			Step( switcher, gpu, &run, seedMean );
		Check( switcher.RetiredCount() == 0, "the ended Baked producer retires after its ticket" );
		Check( gpu.Violations().empty(), "no resource freed while a submitted frame reads it" );
		// And back to Baked.
		Check( bool( switcher.Select( ProducerKind::Baked ) ), "selecting Baked again" );
		for ( int i = 0; i < 20; ++i )
			Step( switcher, gpu, &run, seedMean );
		Check( switcher.Active() == ProducerKind::Baked && run.lowest >= 1.0f - kBlackTolerance,
		    "fake -> Baked switches back with no black frame" );
		for ( int i = 0; i < 5; ++i )
			Step( switcher, gpu, &run, seedMean );
		Check( switcher.RetiredCount() == 0 && !catalog.destroyedEarly &&
		           gpu.Violations().empty() && gpu.LiveCount() == 0,
		    "the fake is destroyed only after its ticket; its resources are all released" );
	}
	// A failed Begin: the old producer stays and nothing changes.
	{
		FakeGpu gpu;
		Catalog catalog( gpu );
		catalog.fakeDefect = kFailBegin;
		Switcher switcher( catalog, gpu, 8 );
		IndirectScene scene;
		scene.baked = seed;
		(void)switcher.BeginMap( scene, ProducerKind::Baked );
		const FrameVolume before = Step( switcher, gpu, nullptr, seedMean );
		const auto selected = switcher.Select( ProducerKind::ScriptedFake );
		const FrameVolume after = Step( switcher, gpu, nullptr, seedMean );
		Check( !selected && selected.Error() == IndirectError::MissingSceneData &&
		           switcher.Active() == ProducerKind::Baked && !switcher.Pending() &&
		           switcher.Selected() == ProducerKind::Baked &&
		           after.generation == before.generation && gpu.LiveCount() == 0,
		    "a failed Begin keeps Baked, reports the error, and mutates nothing" );
	}
	// An unoffered request at map begin is reported and Baked stays.
	{
		FakeGpu gpu;
		Catalog catalog( gpu );
		Switcher switcher( catalog, gpu, 8 );
		IndirectScene scene;
		scene.baked = seed;
		const auto begun = switcher.BeginMap( scene, ProducerKind::RayQuery );
		Check( !begun && begun.Error() == IndirectError::Unavailable &&
		           switcher.Active() == ProducerKind::Baked,
		    "an unoffered saved producer is reported and the default (Baked) is used" );
	}
	// Device loss mid-fade, then recovery.
	{
		FakeGpu gpu;
		Catalog catalog( gpu );
		Switcher switcher( catalog, gpu, 8 );
		IndirectScene scene;
		scene.baked = seed;
		(void)switcher.BeginMap( scene, ProducerKind::Baked );
		Run run;
		(void)switcher.Select( ProducerKind::ScriptedFake );
		bool lostMidFade = false;
		for ( int i = 0; i < 40 && !lostMidFade; ++i )
		{
			const FrameVolume frame = Step( switcher, gpu, &run, seedMean );
			if ( frame.fading && frame.weight > 0.3f )
			{
				gpu.Lose();
				switcher.DeviceLost();
				lostMidFade = true;
			}
		}
		Check( lostMidFade, "device loss arrives during the fade" );
		const FrameVolume lost = Step( switcher, gpu, &run, seedMean );
		Check( switcher.Active() == ProducerKind::Baked && lost.volume == seed && !lost.fading,
		    "device loss republishes the baked volume at once" );
		Check( bool( switcher.DeviceRecovered() ), "recovery re-establishes the selection" );
		for ( int i = 0; i < 25; ++i )
			Step( switcher, gpu, &run, seedMean );
		Check(
		    switcher.Active() == ProducerKind::ScriptedFake && run.lowest >= 1.0f - kBlackTolerance,
		    "after recovery the previous selection returns, with no black frame" );
	}
	// Backgrounding: no scheduling while in the background.
	{
		FakeGpu gpu;
		Catalog catalog( gpu );
		Switcher switcher( catalog, gpu, 4 );
		IndirectScene scene;
		scene.baked = seed;
		(void)switcher.BeginMap( scene, ProducerKind::ScriptedFake );
		for ( int i = 0; i < 12; ++i )
			Step( switcher, gpu, nullptr, seedMean );
		switcher.Background();
		const FrameVolume paused = Step( switcher, gpu, nullptr, seedMean );
		FrameWork work;
		work.frameSerial = gpu.SubmittedSerial() + 1;
		work.resources = &gpu;
		const light_set::Snapshot lights;
		(void)switcher.Frame( work, lights );
		Check( work.jobs.empty(), "a backgrounded producer schedules no work" );
		gpu.Submit();
		switcher.Resume();
		Run run;
		for ( int i = 0; i < 5; ++i )
			Step( switcher, gpu, &run, seedMean );
		Check( paused.volume && switcher.Active() == ProducerKind::ScriptedFake &&
		           run.lowest >= 1.0f - kBlackTolerance,
		    "resume continues the active producer with no black frame" );
	}
	// Map change: every producer ends and retires behind its ticket.
	{
		FakeGpu gpu;
		Catalog catalog( gpu );
		{
			Switcher switcher( catalog, gpu, 4 );
			IndirectScene scene;
			scene.baked = seed;
			(void)switcher.BeginMap( scene, ProducerKind::ScriptedFake );
			for ( int i = 0; i < 12; ++i )
				Step( switcher, gpu, nullptr, seedMean );
			switcher.EndMap();
			Check( switcher.RetiredCount() >= 1, "a map change retires its producers" );
			for ( int i = 0; i < 4; ++i )
				Step( switcher, gpu, nullptr, seedMean );
			Check( switcher.RetiredCount() == 0 && !catalog.destroyedEarly,
			    "they are destroyed once their tickets complete" );
		}
		gpu.Drain();
		Check( gpu.LiveCount() == 0 && gpu.Violations().empty(),
		    "a map change releases every resource behind its completion serial" );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
