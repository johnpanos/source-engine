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
#include "render/direct_occlusion.h"
#include "render/indirect_radiosity.h"
#include "indirect_contract.h"
#include "render/indirect_switcher.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <random>
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
	// Frames naming their changed probes, and probes that changed without
	// being named (against the frame before).
	uint32_t sparseFrames = 0;
	uint32_t sparseFadeFrames = 0;
	size_t uncovered = 0;
	std::shared_ptr<const Volume> previous;
	uint64_t previousGeneration = 0;
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
		if ( frame.changed && run->previous && frame.changedSince == run->previousGeneration )
		{
			++run->sparseFrames;
			run->sparseFadeFrames += frame.fading;
			for ( uint32_t p : DifferingProbes( *run->previous, *frame.volume ) )
				run->uncovered +=
				    !std::binary_search( frame.changed->begin(), frame.changed->end(), p );
		}
		run->previous = frame.volume;
		run->previousGeneration = frame.generation;
	}
	return frame;
}

// The sparse update's helpers (RFC 0016 K12, the change volume): tile
// rectangles, the change atlas and copies probe by probe, the probes the
// proxies cut, and the probes a sample reads.
void SparseHelpers( const Volume &seed )
{
	const mapcontainer::ProbeVolumeLayout &layout = seed.layout;
	const mapcontainer::ProbeGridLayout &grid = layout.grids[0];
	const uint32_t probes = ChangeComposer::TotalProbes( seed );
	mapcontainer::ProbeAtlasRect rects[mapcontainer::kProbeTileRectsMax];
	const uint32_t count = mapcontainer::ProbeTileRects( layout, 1,
	    mapcontainer::kProbeTilesIrradiance | mapcontainer::kProbeTilesVisibility, rects );
	Check( count == layout.layerCount + 1 &&
	           rects[0].x == grid.irradianceOrigin[0][0] +
	                             1 % grid.tilesPerRow * mapcontainer::kProbeIrradianceTile &&
	           rects[0].width == mapcontainer::kProbeIrradianceTile &&
	           rects[count - 1].width == mapcontainer::kProbeVisibilityTile &&
	           mapcontainer::ProbeTileRects(
	               layout, probes, mapcontainer::kProbeTilesIrradiance, rects ) == 0,
	    "sparse: a probe's tiles are its layers' and its visibility tile; none past the grids" );

	// Two probes change: the change atlas updated for them alone equals the
	// one made whole; for the first alone it does not (the control).
	const std::shared_ptr<Volume> scaled = Scaled( seed, 1, 1.75f );
	std::vector<uint32_t> changed; // two probes the scaling changes
	for ( uint32_t p = 0; p < probes && changed.size() < 2; ++p )
	{
		Volume probe = seed;
		const std::vector<uint32_t> single = { p };
		CopyProbeTiles( probe, *scaled, single, mapcontainer::kProbeTilesIrradiance );
		if ( probe.bytes != seed.bytes )
			changed.push_back( p );
	}
	Volume now = seed;
	CopyProbeTiles( now, *scaled, changed, mapcontainer::kProbeTilesIrradiance );
	std::vector<unsigned char> whole, sparse, control;
	Check( changed.size() == 2 && ChangeAtlas( now, seed, &whole ) &&
	           ChangeAtlas( seed, seed, &sparse ) &&
	           ChangeAtlasProbes( now, seed, changed, &sparse ) && sparse == whole,
	    "sparse: the change atlas updated for the changed probes equals the whole one" );
	const std::vector<uint32_t> one(
	    changed.begin(), changed.begin() + ( changed.empty() ? 0 : 1 ) );
	Check( ChangeAtlas( seed, seed, &control ) && ChangeAtlasProbes( now, seed, one, &control ) &&
	           control != whole,
	    "sparse: a change atlas missing a changed probe differs (the control)" );
	Volume copied = seed;
	CopyProbeTiles( copied, now, changed, mapcontainer::kProbeTilesIrradiance );
	Check(
	    copied.bytes == now.bytes, "sparse: copying the changed probes' tiles copies the change" );

	// The proxies' cut: exactly the probes whose visibility tiles changed.
	Volume occluded = seed;
	const float mid[3] = { grid.origin[0] + 0.5f * grid.spacing[0],
	    grid.origin[1] + 0.5f * grid.spacing[1], grid.origin[2] + 0.5f * grid.spacing[2] };
	Proxy box;
	for ( int k = 0; k < 3; ++k )
	{
		box.lo[k] = mid[k] - 0.1f * grid.spacing[k];
		box.hi[k] = mid[k] + 0.1f * grid.spacing[k];
	}
	std::vector<uint32_t> cut;
	const size_t cutCount =
	    OccludeProbeVisibility( occluded, std::span<const Proxy>( &box, 1 ), &cut );
	std::vector<uint32_t> differs;
	for ( uint32_t p = 0; p < probes; ++p )
	{
		Volume restored = occluded;
		const std::vector<uint32_t> single = { p };
		CopyProbeTiles( restored, seed, single, mapcontainer::kProbeTilesVisibility );
		if ( restored.bytes != occluded.bytes )
			differs.push_back( p );
	}
	std::sort( cut.begin(), cut.end() );
	Check( cutCount > 0 && cut.size() == cutCount && cut == differs,
	    "sparse: the cut list names exactly the probes whose visibility the proxy cut" );
	Volume restored = occluded;
	CopyProbeTiles( restored, seed, cut, mapcontainer::kProbeTilesVisibility );
	Check( restored.bytes == seed.bytes, "sparse: restoring the cut tiles gives the volume back" );

	// The fade's probes (RFC 0016 K12): DifferingProbes names exactly the
	// probes whose tiles differ, and BlendProbes over them is Blend within a
	// half-float step (it leaves equal texels as they are); naming half of
	// them is caught.
	{
		Volume to = seed;
		std::vector<uint32_t> chosen;
		for ( uint32_t p = 1; p < probes; p += 5 )
			chosen.push_back( p );
		for ( uint32_t p : chosen )
		{
			mapcontainer::ProbeAtlasRect rects[mapcontainer::kProbeTileRectsMax];
			const uint32_t count = mapcontainer::ProbeTileRects(
			    to.layout, p, mapcontainer::kProbeTilesIrradiance, rects );
			for ( uint32_t r = 0; r < count; ++r )
				for ( uint32_t y = rects[r].y; y < rects[r].y + rects[r].height; ++y )
					for ( uint32_t x = rects[r].x; x < rects[r].x + rects[r].width; ++x )
					{
						unsigned char *texel = to.bytes.data() + to.layout.atlasOffset +
						                       ( size_t( y ) * to.layout.atlasWidth + x ) * 8;
						for ( int c = 0; c < 3; ++c )
						{
							uint16_t h;
							std::memcpy( &h, texel + 2 * c, 2 );
							h = FloatToHalf( mapcontainer::HalfToFloat( h ) * 1.5f + 0.25f );
							std::memcpy( texel + 2 * c, &h, 2 );
						}
					}
		}
		const std::vector<uint32_t> differing = DifferingProbes( seed, to );
		Check( differing == chosen, "fade: DifferingProbes names exactly the changed probes" );
		const auto whole = Blend( seed, to, 0.4f );
		const auto part = BlendProbes( seed, to, 0.4f, differing );
		const std::vector<uint32_t> firstHalf(
		    differing.begin(), differing.begin() + std::ptrdiff_t( differing.size() / 2 ) );
		const auto missing = BlendProbes( seed, to, 0.4f, firstHalf );
		const auto within = [&]( const Volume &a, const Volume &b )
		{
			for ( size_t i = a.layout.atlasOffset; i + 1 < a.bytes.size(); i += 2 )
			{
				uint16_t x, y;
				std::memcpy( &x, a.bytes.data() + i, 2 );
				std::memcpy( &y, b.bytes.data() + i, 2 );
				const float fx = mapcontainer::HalfToFloat( x );
				const float fy = mapcontainer::HalfToFloat( y );
				if ( std::fabs( fx - fy ) > 1e-3f * std::max( 1.0f, std::fabs( fx ) ) )
					return false;
			}
			return true;
		};
		Check( whole && part && within( *whole, *part ),
		    "fade: blending only the differing probes is the whole blend" );
		Check( whole && missing && !within( *whole, *missing ),
		    "fade: a blend that misses half the differing probes is caught" );
	}

	// Visiting only the probes within the proxies' reach equals visiting every
	// probe: proxies of many sizes, inside, straddling and outside the grid.
	{
		std::mt19937 rng( 1234 );
		std::uniform_real_distribution<float> unit( -0.5f, 1.5f );
		bool same = true;
		size_t cutTotal = 0;
		for ( int round = 0; round < 40 && same; ++round )
		{
			std::vector<Proxy> proxies( 1 + round % 4 );
			for ( Proxy &proxy : proxies )
				for ( int k = 0; k < 3; ++k )
				{
					const float extent = float( grid.dims[k] ) * grid.spacing[k];
					const float at = grid.origin[k] + unit( rng ) * extent;
					const float half = ( 0.05f + 0.3f * float( round % 5 ) ) * grid.spacing[k];
					proxy.lo[k] = at - half;
					proxy.hi[k] = at + half;
				}
			Volume near = seed, all = seed;
			std::vector<uint32_t> nearCut, allCut;
			const size_t nearCount = OccludeProbeVisibility( near, proxies, &nearCut );
			const size_t allCount = OccludeProbeVisibility( all, proxies, &allCut, true );
			same = nearCount == allCount && nearCut == allCut && near.bytes == all.bytes;
			cutTotal += allCount;
		}
		Check( same && cutTotal > 0,
		    "occlusion: visiting the probes within the proxies' reach equals visiting all" );
	}

	// A sample reads the eight probes SampleProbes names: scaling every
	// other probe leaves it unchanged, scaling one of them does not.
	const mapcontainer::ProbeVolumeView view = seed.View();
	const float up[3] = { 0, 0, 1 };
	uint32_t read[8];
	const uint32_t reads = view.SampleProbes( mid, up, read );
	std::vector<uint32_t> others;
	for ( uint32_t p = 0; p < probes; ++p )
		if ( std::find( read, read + reads, p ) == read + reads )
			others.push_back( p );
	const std::shared_ptr<Volume> bright = Scaled( seed, 0, 4.0f );
	Volume away = seed, near = seed;
	CopyProbeTiles( away, *bright, others, mapcontainer::kProbeTilesIrradiance );
	CopyProbeTiles( near, *bright, std::vector<uint32_t>( read, read + reads ),
	    mapcontainer::kProbeTilesIrradiance );
	float base[3] = {}, fromAway[3] = {}, fromNear[3] = {};
	const bool sampled =
	    view.Sample( mid, up, mapcontainer::ProbeVolumeLayer::Total, true, base ) &&
	    away.View().Sample( mid, up, mapcontainer::ProbeVolumeLayer::Total, true, fromAway ) &&
	    near.View().Sample( mid, up, mapcontainer::ProbeVolumeLayer::Total, true, fromNear );
	Check( reads == 8 && sampled && std::memcmp( base, fromAway, sizeof( base ) ) == 0 &&
	           fromNear[0] > base[0],
	    "sparse: a sample reads the eight probes SampleProbes names and no other" );
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
	SparseHelpers( *seed );
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
		Check( run.sparseFadeFrames >= 7 && run.uncovered == 0,
		    "the fade's frames name their changed probes, and every changed probe is named" );
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
