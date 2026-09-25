//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CPU cost of the SDF-traced indirect light path (RFC 0011 G6),
//          the work the GPU bench (indirect_sdf_native_conformance --bench)
//          does not time: the producer's Schedule on the frame thread (its
//          dispatch bookkeeping, reference capture and, on every completed
//          live update, the published volume's composition), the host's
//          work per published volume (the BakedPlusDelta change atlas and
//          the proxies' probe visibility on a copy), and the baked direct
//          light's occlusion (DirectOcclusion build per map, compose per
//          proxy change, serial and on worker threads).
//
//          The producer runs against HostCompute, render/gpu_compute.h in
//          host memory. Its dispatches run when a frame is submitted, outside
//          the timed region, and write a deterministic field: under the baked
//          configuration a fixed value per probe and texel, and under a live
//          one that value times light style 32's scalar, halved while a proxy
//          stands in the scene. Completion lags one submission, so a live
//          update publishes every frame (a fast GPU).
//
//          Every sample is checked. The unchanged scene publishes the bake
//          byte for byte. A converged probe publishes the bake times the
//          field's ratio. The change atlas is the published minus baked
//          indirect light. Occlusion without proxies is the bake; pooled
//          compose is serial compose byte for byte.
//
//          indirect_sdf_bench
//              smoke: the checked-in contract fixtures and a synthetic floor
//          indirect_sdf_bench producer <volume.prbv> <field.sdfv>
//              [--frames n] [--change n] [--budget n] [--focus file]
//              [--proxy x0 y0 z0 x1 y1 z1] [--label name]
//          indirect_sdf_bench occlusion (<world.wmsh> <lmap.ktx2> <field.sdfv> |
//              --synthetic texels lights) [--proxy x0 y0 z0 x1 y1 z1]
//              [--repeats n] [--workers n] [--label name]
//          Both also take --seed-slow stage=factor: the named stage made
//          `factor` times slower (a spin), the runner's sensitivity control.
//
//          Each mode prints one `BENCH {json}` line and a checks-v1 record.
//
//===========================================================================//

#include "render/direct_occlusion.h"
#include "render/indirect_sdf.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace
{
using namespace indirect_light;

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

std::vector<unsigned char> LoadFile( const char *path )
{
	std::ifstream file( path, std::ios::binary );
	return std::vector<unsigned char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

using Clock = std::chrono::steady_clock;

double Ms( Clock::time_point from, Clock::time_point to )
{
	return std::chrono::duration<double, std::milli>( to - from ).count();
}

// --seed-slow stage=factor: after a timed stage, spin for (factor - 1) times
// its duration, so the recorded time is the stage `factor` times slower.
std::map<std::string, double> g_slow;

double Timed( const char *stage, Clock::time_point from )
{
	const Clock::time_point to = Clock::now();
	const double ms = Ms( from, to );
	const auto found = g_slow.find( stage );
	if ( found == g_slow.end() || found->second <= 1.0 )
		return ms;
	const Clock::time_point until =
	    to + std::chrono::duration_cast<Clock::duration>(
	             std::chrono::duration<double, std::milli>( ms * ( found->second - 1.0 ) ) );
	while ( Clock::now() < until )
	{
	}
	return Ms( from, Clock::now() );
}

struct Stats
{
	size_t count = 0;
	double median = 0.0;
	double p95 = 0.0;
	double max = 0.0;
};

Stats Summarize( std::vector<double> times )
{
	Stats stats;
	stats.count = times.size();
	if ( times.empty() )
		return stats;
	std::sort( times.begin(), times.end() );
	const auto at = [&]( double fraction )
	{
		return times[std::min( times.size() - 1, size_t( double( times.size() ) * fraction ) )];
	};
	stats.median = at( 0.5 );
	stats.p95 = at( 0.95 );
	stats.max = times.back();
	return stats;
}

std::string Json( const char *name, const Stats &stats )
{
	char text[256];
	std::snprintf( text, sizeof( text ),
	    "\"%s\": {\"count\": %zu, \"median_ms\": %.4f, \"p95_ms\": %.4f, \"max_ms\": %.4f}", name,
	    stats.count, stats.median, stats.p95, stats.max );
	return text;
}

uint64_t Digest( const unsigned char *bytes, size_t size )
{
	uint64_t h = 1469598103934665603ull;
	for ( size_t i = 0; i < size; ++i )
		h = ( h ^ bytes[i] ) * 1099511628211ull;
	return h;
}

// The engine's frame executor, reduced to its contract: body( context, i )
// for every i, on `workers` threads (the caller among them), returning once
// all complete.
class ThreadExecutor final : public IBatchExecutor
{
public:
	explicit ThreadExecutor( uint32_t workers ) : m_workers( std::max( 1u, workers ) ) {}

	void ParallelFor(
	    const char *, uint32_t count, void ( *body )( void *, uint32_t ), void *context ) override
	{
		std::atomic<uint32_t> next{ 0 };
		const auto run = [&]
		{
			for ( uint32_t i = next.fetch_add( 1 ); i < count; i = next.fetch_add( 1 ) )
				body( context, i );
		};
		std::vector<std::thread> threads;
		for ( uint32_t w = 1; w < m_workers && w < count; ++w )
			threads.emplace_back( run );
		run();
		for ( std::thread &thread : threads )
			thread.join();
	}

private:
	uint32_t m_workers;
};

// Resources the traced producers track through the GPU service, not here.
class NullTracker final : public IResourceTracker
{
public:
	uint64_t Acquire( size_t ) override { return ++m_next; }
	void Release( uint64_t, uint64_t ) override {}
	uint64_t SubmittedSerial() const override { return 0; }
	uint64_t CompletedSerial() const override { return 0; }

private:
	uint64_t m_next = 0;
};

// render/gpu_compute.h in host memory (see the header comment).
class HostCompute final : public gpu_compute::IGpuCompute
{
public:
	static constexpr uint32_t kTexels = TracedProducer::kTexels;
	static constexpr uint32_t kFloats = 12; // per texel: total, indirect, direct (4 each)

	gpu_compute::Caps Capabilities() const override
	{
		gpu_compute::Caps caps;
		caps.compute = true;
		return caps;
	}
	uint32_t CreateBuffer( size_t bytes, gpu_compute::BufferUse ) override
	{
		m_buffers[++m_next].assign( bytes, 0 );
		return m_next;
	}
	void *Map( uint32_t buffer ) override
	{
		const auto found = m_buffers.find( buffer );
		return found == m_buffers.end() ? nullptr : found->second.data();
	}
	uint32_t CreateProgram(
	    const char *name, const gpu_compute::Binding *, uint32_t count, uint32_t ) override
	{
		if ( std::string( name ) != "sdf-probe-trace" || count != 9 )
			return 0;
		m_programs.push_back( ++m_next );
		return m_next;
	}
	uint32_t CreateGeometry( const float *, uint32_t, const uint32_t *, uint32_t ) override
	{
		return 0;
	}
	uint32_t CreateScene( const gpu_compute::SceneInstance *, uint32_t ) override { return 0; }
	uint64_t QueueDispatch( uint32_t program, const uint32_t *buffers, uint32_t count,
	    const void *push, uint32_t pushBytes, uint32_t, uint32_t, uint32_t ) override
	{
		if ( std::find( m_programs.begin(), m_programs.end(), program ) == m_programs.end() ||
		     count != 9 || pushBytes != 8 )
			return 0;
		Dispatch dispatch;
		std::copy( buffers, buffers + count, dispatch.buffers );
		std::memcpy( &dispatch.entries, push, 4 );
		m_queued.push_back( dispatch );
		return m_submitted + 1;
	}
	uint64_t CompletedSerial() const override { return m_completed; }
	void Retire( uint32_t resource, uint64_t ) override { m_buffers.erase( resource ); }

	// Runs the queued dispatches (the "device"), then completes the previous
	// submission.
	void Submit()
	{
		for ( const Dispatch &dispatch : m_queued )
			Trace( dispatch );
		m_queued.clear();
		++m_submitted;
		m_completed = m_submitted - 1;
	}
	// Completes everything submitted.
	void Drain()
	{
		Submit();
		m_completed = m_submitted;
	}

	// The scale of the latest live dispatch's field against the baked one.
	float LiveScale() const { return m_liveScale; }

	// The emulated field's value for a probe's texel at a scale (the baked
	// configuration's is 1).
	static float Target( uint32_t probe, uint32_t texel, float scale )
	{
		return scale * ( 0.2f + 0.05f * float( ( probe * 7 + texel ) % 9 ) );
	}

private:
	// Bindings (indirect_sdf.h Dispatch, SDF mode): voxels, params,
	// positions, previous, field, lights, cell firsts, cell entries, schedule.
	struct Dispatch
	{
		uint32_t buffers[9] = {};
		uint32_t entries = 0;
	};

	void Trace( const Dispatch &dispatch )
	{
		const auto &params =
		    *reinterpret_cast<const SdfTraceParams *>( m_buffers[dispatch.buffers[1]].data() );
		const float *previous =
		    reinterpret_cast<const float *>( m_buffers[dispatch.buffers[3]].data() );
		float *field = reinterpret_cast<float *>( m_buffers[dispatch.buffers[4]].data() );
		const uint32_t *schedule =
		    reinterpret_cast<const uint32_t *>( m_buffers[dispatch.buffers[8]].data() );
		// A live dispatch (world and proxy instances) under style 32's
		// scalar, halved again while a proxy stands in the scene.
		const bool live = params.counts[1] == ( kWorldInstanceMask | kProxyInstanceMask );
		const float scale =
		    live ? params.sourceScale[32] * ( params.counts[0] ? 0.5f : 1.0f ) : 1.0f;
		if ( live )
			m_liveScale = scale;
		for ( uint32_t e = 0; e < dispatch.entries; ++e )
		{
			const uint32_t probe = schedule[e * 2];
			float alpha;
			std::memcpy( &alpha, &schedule[e * 2 + 1], 4 );
			for ( uint32_t t = 0; t < kTexels; ++t )
			{
				const size_t at = ( size_t( probe ) * kTexels + t ) * kFloats;
				const float value = Target( probe, t, scale );
				const float parts[3] = { value, 0.6f * value, 0.4f * value };
				for ( int part = 0; part < 3; ++part )
					for ( int c = 0; c < 4; ++c )
					{
						const size_t i = at + size_t( part ) * 4 + size_t( c );
						const float target = c < 3 ? parts[part] : 0.0f;
						field[i] = previous[i] + alpha * ( target - previous[i] );
					}
			}
		}
	}

	std::map<uint32_t, std::vector<unsigned char>> m_buffers;
	std::vector<uint32_t> m_programs;
	std::vector<Dispatch> m_queued;
	uint32_t m_next = 0;
	uint64_t m_submitted = 0;
	uint64_t m_completed = 0;
	float m_liveScale = 1.0f;
};

// An interior texel of a probe's irradiance layer (0 total, 1 indirect),
// rgb channel c.
float InteriorTexel( const Volume &volume, uint32_t probe, uint32_t layer, uint32_t texel, int c )
{
	const mapcontainer::ProbeVolumeLayout &layout = volume.layout;
	const mapcontainer::ProbeGridLayout &grid = layout.grids[0];
	constexpr uint32_t tile = mapcontainer::kProbeIrradianceTile;
	constexpr uint32_t interior = tile - 2;
	const uint32_t x = grid.irradianceOrigin[layer][0] + ( probe % grid.tilesPerRow ) * tile + 1 +
	                   texel % interior;
	const uint32_t y = grid.irradianceOrigin[layer][1] + ( probe / grid.tilesPerRow ) * tile + 1 +
	                   texel / interior;
	uint16_t half;
	std::memcpy( &half,
	    volume.bytes.data() + layout.atlasOffset + ( uint64_t( y ) * layout.atlasWidth + x ) * 8 +
	        2 * c,
	    2 );
	return mapcontainer::HalfToFloat( half );
}

float ChangeTexel( const std::vector<unsigned char> &change, const Volume &volume, uint32_t probe,
    uint32_t texel, int c )
{
	const mapcontainer::ProbeVolumeLayout &layout = volume.layout;
	const mapcontainer::ProbeGridLayout &grid = layout.grids[0];
	constexpr uint32_t tile = mapcontainer::kProbeIrradianceTile;
	constexpr uint32_t interior = tile - 2;
	const uint32_t x =
	    grid.irradianceOrigin[1][0] + ( probe % grid.tilesPerRow ) * tile + 1 + texel % interior;
	const uint32_t y =
	    grid.irradianceOrigin[1][1] + ( probe / grid.tilesPerRow ) * tile + 1 + texel / interior;
	uint16_t half;
	std::memcpy( &half, change.data() + ( uint64_t( y ) * layout.atlasWidth + x ) * 8 + 2 * c, 2 );
	return mapcontainer::HalfToFloat( half );
}

bool Near( float a, float b, float relative )
{
	return std::fabs( a - b ) <= relative * std::max( std::fabs( a ), std::fabs( b ) ) + 2e-3f;
}

struct ProducerOptions
{
	std::string prbv;
	std::string sdfv;
	std::string label = "producer";
	int frames = 360;
	int change = 160;
	uint32_t budget = 128;
	std::vector<uint32_t> focus;
	bool proxy = false;
	Proxy box;
};

// The producer's CPU cost: Schedule per frame in the reference phase and,
// after light style 32 halves (and the proxy appears), in the live phase,
// split by whether the frame published; the host's work per published
// volume; Begin and the scene's load.
bool BenchProducer( const ProducerOptions &options )
{
	// The engine validates lumps already in memory: the read is not timed.
	std::vector<unsigned char> prbvBytes = LoadFile( options.prbv.c_str() );
	std::vector<unsigned char> sdfvBytes = LoadFile( options.sdfv.c_str() );
	const Clock::time_point loadStarted = Clock::now();
	auto baked = Volume::FromBytes( std::move( prbvBytes ) );
	auto sdf = SdfData::FromBytes( std::move( sdfvBytes ) );
	const double loadMs = Timed( "load", loadStarted );
	Check( baked && sdf, options.label + ": the volume and field load" );
	if ( !baked || !sdf )
		return false;
	const std::vector<uint32_t> &focus = options.focus;

	HostCompute gpu;
	NullTracker tracker;
	SdfTracedProducer producer;
	IndirectScene scene;
	scene.baked = baked;
	scene.sdf = sdf;
	scene.gpu = &gpu;
	scene.policy = indirect_policy::Policy::BakedPlusDelta;
	const Clock::time_point beginStarted = Clock::now();
	const bool begun = bool( producer.Begin( scene, PublishedVolume{ 0, baked }, tracker ) );
	const double beginMs = Timed( "begin", beginStarted );
	Check( begun, options.label + ": Begin" );
	if ( !begun )
		return false;

	light_set::Snapshot lights;
	lights.styleScalars.assign( 64, 1.0f );
	std::vector<Proxy> proxies;
	std::vector<double> reference, idle, liveQuiet, livePublish, visibility, changeAtlas;
	std::vector<double> frameTotal; // a publishing frame's Schedule and host work
	uint64_t epoch = producer.Published() ? producer.Published()->epoch : 0;
	size_t publications = 0;
	bool bakeKept = true;
	std::vector<unsigned char> change;
	std::shared_ptr<const Volume> last;
	for ( int frame = 0; frame < options.frames; ++frame )
	{
		if ( frame == options.change )
		{
			// Before the change the unchanged scene is the bake, byte for byte.
			const auto published = producer.Published();
			bakeKept = published && published->volume && published->volume->bytes == baked->bytes;
			lights.styleScalars[32] = 0.5f;
			if ( options.proxy )
				proxies = { options.box };
		}
		const bool referencePhase = !producer.Live();
		FrameWork work;
		work.resources = &tracker;
		work.frameSerial = uint64_t( frame ) + 1;
		work.focusProbes = focus;
		work.probeBudget = options.budget;
		work.proxies = proxies;
		const Clock::time_point started = Clock::now();
		producer.Schedule( work, lights );
		for ( auto &job : work.jobs )
			job();
		const double ms = Timed( "schedule", started );
		const auto published = producer.Published();
		const bool fresh = published && published->epoch != epoch;
		if ( fresh )
			epoch = published->epoch;
		// The first frames make the ring's first uses; then, before the
		// change, the reference phase and the still scene; after it, the
		// frames that publish and those that do not.
		if ( frame >= options.change )
			( fresh ? livePublish : liveQuiet ).push_back( ms );
		else if ( frame >= 4 )
			( referencePhase ? reference : idle ).push_back( ms );
		if ( fresh && frame >= options.change )
		{
			double total = ms;
			// The host's work per published volume (indirect_light_host.cpp
			// Consume): the proxies' probe visibility on a copy, and the
			// world's change atlas.
			++publications;
			last = published->volume;
			if ( !proxies.empty() )
			{
				const Clock::time_point copied = Clock::now();
				auto occluded = std::make_shared<Volume>( *published->volume );
				(void)OccludeProbeVisibility( *occluded, proxies );
				visibility.push_back( Timed( "visibility", copied ) );
				total += visibility.back();
			}
			const Clock::time_point changed = Clock::now();
			const bool made = ChangeAtlas( *published->volume, *baked, &change );
			changeAtlas.push_back( Timed( "change-atlas", changed ) );
			frameTotal.push_back( total + changeAtlas.back() );
			if ( !made )
				bakeKept = false;
		}
		gpu.Submit();
	}
	gpu.Drain();
	Check( bakeKept, options.label + ": before the change the published volume is the bake, and "
	                                 "every publication has a change atlas" );
	Check( !reference.empty(), options.label + ": the reference phase is measured (" +
	                               std::to_string( reference.size() ) + " frames)" );
	Check( publications > 0, options.label + ": the change publishes" );

	// Every converged active probe publishes the bake times the field's ratio
	// (the emulated field's live scale), and its change atlas texel is the
	// published minus the baked indirect light. An inactive probe (inside
	// geometry) is never traced and publishes the bake.
	const std::vector<float> positions = ProbePositions( *baked );
	size_t converged = 0, wrong = 0, wrongChange = 0, inactiveChanged = 0;
	std::string first;
	if ( last )
	{
		(void)ChangeAtlas( *last, *baked, &change );
		const float ratio = gpu.LiveScale();
		const uint32_t probes = baked->layout.grids[0].probeCount;
		for ( uint32_t p = 0; p < probes; ++p )
		{
			if ( positions[p * 4 + 3] < 0.5f )
			{
				inactiveChanged +=
				    InteriorTexel( *last, p, 0, 0, 0 ) != InteriorTexel( *baked, p, 0, 0, 0 );
				continue;
			}
			if ( producer.LiveUpdates( p ) < TracedProducer::kActiveUpdates ||
			     !producer.Referenced( p ) )
				continue;
			const float base = InteriorTexel( *baked, p, 0, 0, 0 );
			if ( base < 1e-2f )
				continue;
			++converged;
			const float now = InteriorTexel( *last, p, 0, 0, 0 );
			if ( !( ratio < 1.0f && Near( now, ratio * base, 0.02f ) ) )
			{
				if ( !wrong++ )
					first = "probe " + std::to_string( p ) + " " + std::to_string( now ) + " vs " +
					        std::to_string( ratio * base );
			}
			const float expected =
			    InteriorTexel( *last, p, 1, 0, 0 ) - InteriorTexel( *baked, p, 1, 0, 0 );
			wrongChange += !Near( ChangeTexel( change, *last, p, 0, 0 ), expected, 0.01f );
		}
	}
	const bool checked = converged > 0;
	Check( !wrong, options.label +
	                   ": every converged probe publishes the bake times the "
	                   "field's ratio (" +
	                   std::to_string( wrong ) + " of " + std::to_string( converged ) +
	                   " differ; first " + first + ")" );
	Check( !wrongChange, options.label + ": the change atlas is published minus baked (" +
	                         std::to_string( wrongChange ) + " probes differ)" );
	Check( !inactiveChanged, options.label + ": inactive probes publish the bake (" +
	                             std::to_string( inactiveChanged ) + " differ)" );
	Check( checked, options.label + ": some probe converges after the change" );

	const uint64_t digest = last ? Digest( last->bytes.data(), last->bytes.size() ) : 0;
	uint32_t active = 0;
	for ( size_t p = 0; p * 4 < positions.size(); ++p )
		active += positions[p * 4 + 3] >= 0.5f;
	std::printf( "BENCH {\"mode\": \"producer\", \"label\": \"%s\", \"probes\": %u, "
	             "\"active\": %u, \"voxels\": [%u, %u, %u], \"lights\": %u, \"atlas\": [%u, "
	             "%u], \"focus\": %zu, \"budget\": %u, \"publications\": %zu, "
	             "\"probe_updates\": %llu, \"load_ms\": %.4f, \"begin_ms\": %.4f, %s, %s, %s, "
	             "%s, %s, %s, %s, \"digest\": \"%016llx\"}\n",
	    options.label.c_str(), baked->layout.grids[0].probeCount, active, sdf->layout.dims[0],
	    sdf->layout.dims[1], sdf->layout.dims[2], sdf->layout.lightCount, baked->layout.atlasWidth,
	    baked->layout.atlasHeight, focus.size(), options.budget, publications,
	    (unsigned long long)producer.ProbeUpdates(), loadMs, beginMs,
	    Json( "schedule_reference", Summarize( reference ) ).c_str(),
	    Json( "schedule_idle", Summarize( idle ) ).c_str(),
	    Json( "schedule_live", Summarize( liveQuiet ) ).c_str(),
	    Json( "schedule_publish", Summarize( livePublish ) ).c_str(),
	    Json( "consume_visibility", Summarize( visibility ) ).c_str(),
	    Json( "consume_change_atlas", Summarize( changeAtlas ) ).c_str(),
	    Json( "frame_publish", Summarize( frameTotal ) ).c_str(), (unsigned long long)digest );
	(void)producer.End();
	return true;
}

// A synthetic lightmapped floor: `side` x `side` texels over a 16-unit
// texel grid, total = direct + 0.1, with `lights` rectangle lights in a row
// above it.
struct SyntheticFloor
{
	std::vector<float> positions, uvs;
	std::vector<uint32_t> indices;
	std::vector<unsigned char> total, direct;
	std::vector<mapcontainer::SdfLight> lights;
	uint32_t side = 0;

	SyntheticFloor( uint32_t texels, uint32_t lightCount )
	{
		side = std::max( 8u, uint32_t( std::sqrt( double( texels ) ) ) );
		const float extent = float( side ) * 16.0f;
		positions = { 0, 0, 0, extent, 0, 0, extent, extent, 0, 0, extent, 0 };
		uvs = { 0, 0, 1, 0, 1, 1, 0, 1 };
		indices = { 0, 1, 2, 0, 2, 3 };
		total = Layer( 1.1f );
		direct = Layer( 1.0f );
		for ( uint32_t l = 0; l < lightCount; ++l )
		{
			mapcontainer::SdfLight light = {};
			light.kind = uint32_t( mapcontainer::SdfLightKind::Rect );
			light.style = -1;
			light.rgb[0] = light.rgb[1] = light.rgb[2] = 10.0f;
			light.a[0] = extent * ( float( l ) + 0.5f ) / float( lightCount );
			light.a[1] = extent * 0.5f;
			light.a[2] = 256.0f;
			light.b[0] = 16.0f; // b x c = -z: it emits down
			light.c[1] = -16.0f;
			lights.push_back( light );
		}
	}

	std::vector<unsigned char> Layer( float value ) const
	{
		std::vector<unsigned char> layer( size_t( side ) * side * 8 );
		const uint16_t half = FloatToHalf( value ), one = FloatToHalf( 1.0f );
		for ( size_t t = 0; t < size_t( side ) * side; ++t )
		{
			for ( int c = 0; c < 3; ++c )
				std::memcpy( &layer[t * 8 + 2 * c], &half, 2 );
			std::memcpy( &layer[t * 8 + 6], &one, 2 );
		}
		return layer;
	}
};

struct OcclusionOptions
{
	std::string wmsh, lmap, sdfv;
	std::string label = "occlusion";
	uint32_t syntheticTexels = 0;
	uint32_t syntheticLights = 0;
	bool proxy = false;
	Proxy box;
	int repeats = 12;
	uint32_t workers = 0;
};

// The baked direct light's occlusion: Build once per map, and Compose per
// proxy change, serial and on worker threads.
bool BenchOcclusion( const OcclusionOptions &options )
{
	DirectOcclusion occlusion;
	std::unique_ptr<SyntheticFloor> floor;
	std::vector<unsigned char> wmsh, lmap;
	double buildMs = 0.0;
	Proxy box = options.box;
	if ( options.syntheticTexels )
	{
		floor =
		    std::make_unique<SyntheticFloor>( options.syntheticTexels, options.syntheticLights );
		const Clock::time_point started = Clock::now();
		const bool built = occlusion.BuildLayers( floor->positions, floor->uvs, floor->indices,
		    floor->side, floor->side, floor->total.data(), floor->direct.data(), floor->lights );
		buildMs = Timed( "occlusion-build", started );
		Check( built, options.label + ": builds" );
		if ( !options.proxy )
		{
			// A door-sized box under the middle light.
			const float mid = float( floor->side ) * 8.0f;
			box.lo[0] = mid - 32.0f, box.lo[1] = mid - 64.0f, box.lo[2] = 64.0f;
			box.hi[0] = mid + 32.0f, box.hi[1] = mid + 64.0f, box.hi[2] = 72.0f;
		}
	}
	else
	{
		wmsh = LoadFile( options.wmsh.c_str() );
		lmap = LoadFile( options.lmap.c_str() );
		auto sdf = SdfData::FromBytes( LoadFile( options.sdfv.c_str() ) );
		Check( sdf != nullptr, options.label + ": the field loads" );
		if ( !sdf )
			return false;
		const mapcontainer::SdfVolumeLayout &f = sdf->layout;
		std::vector<mapcontainer::SdfLight> lights( f.lightCount );
		std::memcpy( lights.data(), sdf->bytes.data() + f.lightOffset,
		    lights.size() * sizeof( mapcontainer::SdfLight ) );
		const Clock::time_point started = Clock::now();
		const bool built =
		    occlusion.Build( wmsh.data(), wmsh.size(), lmap.data(), lmap.size(), 0, lights );
		buildMs = Timed( "occlusion-build", started );
		Check( built, options.label + ": builds from the map's WMSH, LMAP and lights" );
		Check( options.proxy, options.label + ": a map's occlusion bench names its --proxy" );
	}
	if ( !occlusion.Ready() )
		return false;

	std::vector<unsigned char> serialOut, pooledOut, bakeOut;
	(void)occlusion.Compose( {}, nullptr, &bakeOut );
	const size_t layerBytes = bakeOut.size();
	const uint32_t workers =
	    options.workers ? options.workers : std::max( 1u, std::thread::hardware_concurrency() );
	ThreadExecutor pool( workers );
	std::vector<double> serial, pooled;
	size_t blocked = 0;
	bool stable = true, same = true;
	uint64_t digest = 0;
	const std::span<const Proxy> proxies( &box, 1 );
	for ( int r = 0; r < options.repeats; ++r )
	{
		Clock::time_point started = Clock::now();
		const size_t now = occlusion.Compose( proxies, nullptr, &serialOut );
		serial.push_back( Timed( "occlusion-compose", started ) );
		started = Clock::now();
		(void)occlusion.Compose( proxies, &pool, &pooledOut );
		pooled.push_back( Timed( "occlusion-compose-pooled", started ) );
		const uint64_t d = Digest( serialOut.data(), serialOut.size() );
		stable = stable && ( r == 0 || ( d == digest && now == blocked ) );
		same = same && pooledOut == serialOut;
		digest = d;
		blocked = now;
	}
	std::vector<unsigned char> none;
	(void)occlusion.Compose( {}, &pool, &none );
	Check(
	    none == bakeOut && layerBytes > 0, options.label + ": no proxy leaves the bake's bytes" );
	Check( blocked > 0, options.label + ": the proxy blocks direct light (" +
	                        std::to_string( blocked ) + " texels)" );
	Check( stable, options.label + ": every compose repeats byte for byte" );
	Check( same, options.label + ": the pooled compose is the serial compose byte for byte" );
	std::printf( "BENCH {\"mode\": \"occlusion\", \"label\": \"%s\", \"covered_texels\": %zu, "
	             "\"layer_bytes\": %zu, \"blocked_texels\": %zu, \"workers\": %u, "
	             "\"build_ms\": %.4f, %s, %s, \"digest\": \"%016llx\"}\n",
	    options.label.c_str(), occlusion.CoveredTexels(), layerBytes, blocked, workers, buildMs,
	    Json( "compose_serial", Summarize( serial ) ).c_str(),
	    Json( "compose_pooled", Summarize( pooled ) ).c_str(), (unsigned long long)digest );
	return true;
}

bool ParseProxy( int &i, int argc, char **argv, Proxy *box )
{
	if ( i + 6 >= argc )
		return false;
	for ( int k = 0; k < 3; ++k )
		box->lo[k] = std::strtof( argv[++i], nullptr );
	for ( int k = 0; k < 3; ++k )
		box->hi[k] = std::strtof( argv[++i], nullptr );
	box->reflectance = 0.5f;
	return true;
}

bool ParseSlow( const std::string &value )
{
	const size_t equals = value.find( '=' );
	if ( equals == std::string::npos )
		return false;
	g_slow[value.substr( 0, equals )] = std::strtod( value.c_str() + equals + 1, nullptr );
	return true;
}

int Usage()
{
	std::fprintf( stderr, "usage: indirect_sdf_bench [producer <prbv> <sdfv> [options] | "
	                      "occlusion (<wmsh> <lmap> <sdfv> | --synthetic texels lights) "
	                      "[options]] (see the source header)\n" );
	return 2;
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc == 1 )
	{
		// Smoke: the contract scene (16 probes) and a small floor.
		ProducerOptions producer;
		producer.prbv = "quality/fixtures/gi/prbv/contract.prbv";
		producer.sdfv = "quality/fixtures/gi/sdfv/contract.sdfv";
		producer.label = "contract";
		producer.frames = 240;
		producer.change = 120;
		producer.proxy = true;
		producer.box.lo[0] = producer.box.lo[1] = producer.box.lo[2] = -9.0f;
		producer.box.hi[0] = producer.box.hi[1] = producer.box.hi[2] = 9.0f;
		BenchProducer( producer );
		OcclusionOptions occlusion;
		occlusion.label = "floor";
		occlusion.syntheticTexels = 64 * 64;
		occlusion.syntheticLights = 3;
		occlusion.repeats = 3;
		occlusion.workers = 3;
		BenchOcclusion( occlusion );
		return testing::ReportConformance( g_checks, g_failures );
	}
	const std::string mode = argv[1];
	if ( mode == "producer" && argc >= 4 )
	{
		ProducerOptions options;
		options.prbv = argv[2];
		options.sdfv = argv[3];
		for ( int i = 4; i < argc; ++i )
		{
			const std::string arg = argv[i];
			if ( arg == "--frames" && i + 1 < argc )
				options.frames = std::atoi( argv[++i] );
			else if ( arg == "--change" && i + 1 < argc )
				options.change = std::atoi( argv[++i] );
			else if ( arg == "--budget" && i + 1 < argc )
				options.budget = uint32_t( std::strtoul( argv[++i], nullptr, 10 ) );
			else if ( arg == "--label" && i + 1 < argc )
				options.label = argv[++i];
			else if ( arg == "--focus" && i + 1 < argc )
			{
				std::ifstream file( argv[++i] );
				uint32_t probe;
				while ( file >> probe )
					options.focus.push_back( probe );
			}
			else if ( arg == "--proxy" && ParseProxy( i, argc, argv, &options.box ) )
				options.proxy = true;
			else if ( arg == "--seed-slow" && i + 1 < argc && ParseSlow( argv[i + 1] ) )
				++i;
			else
				return Usage();
		}
		BenchProducer( options );
		return testing::ReportConformance( g_checks, g_failures );
	}
	if ( mode == "occlusion" && argc >= 3 )
	{
		OcclusionOptions options;
		int i = 2;
		if ( std::string( argv[2] ) == "--synthetic" && argc >= 5 )
		{
			options.syntheticTexels = uint32_t( std::strtoul( argv[3], nullptr, 10 ) );
			options.syntheticLights = uint32_t( std::strtoul( argv[4], nullptr, 10 ) );
			i = 5;
		}
		else if ( argc >= 5 )
		{
			options.wmsh = argv[2];
			options.lmap = argv[3];
			options.sdfv = argv[4];
			i = 5;
		}
		else
			return Usage();
		for ( ; i < argc; ++i )
		{
			const std::string arg = argv[i];
			if ( arg == "--repeats" && i + 1 < argc )
				options.repeats = std::max( 1, std::atoi( argv[++i] ) );
			else if ( arg == "--workers" && i + 1 < argc )
				options.workers = uint32_t( std::strtoul( argv[++i], nullptr, 10 ) );
			else if ( arg == "--label" && i + 1 < argc )
				options.label = argv[++i];
			else if ( arg == "--proxy" && ParseProxy( i, argc, argv, &options.box ) )
				options.proxy = true;
			else if ( arg == "--seed-slow" && i + 1 < argc && ParseSlow( argv[i + 1] ) )
				++i;
			else
				return Usage();
		}
		BenchOcclusion( options );
		return testing::ReportConformance( g_checks, g_failures );
	}
	return Usage();
}
