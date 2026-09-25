//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.indirect-light.sdf (RFC 0011 G6): the shared producer
//          suite (unittests/rendertest/indirect_contract.h) run against the
//          SDF-traced producer on a real device, headless, through the
//          renderer's compute service (render/gpu_compute.h over
//          vulkan_compute's GpuComputeService, the engine's own).
//
//          The scene (quality/fixtures/gi/sdfv/contract.sdfv) matches the
//          contract seed: room A a furnace (emission 0.3, reflectance 0.6:
//          light 0.75, indirect 0.45), room B beyond a thin wall black. The
//          suite's light change halves every source; its geometry change
//          encloses the probe at the origin in a proxy box, whose oracle is
//          that the probe goes dark (total and indirect near zero). Also
//          reported: the GPU time of one update (a fenced submission).
//
//          `--bench <volume.prbv> <field.sdfv>` instead measures a map's
//          update cost (RFC 0011 G6.4, the budget's gpu_ms: the median over
//          warm updates of the dispatch's timestamps): the reference phase
//          and a live phase after the first switchable light (style 32)
//          halves.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/material_spv.h"
#include "../rendertest/indirect_contract.h"
#include "headless_vulkan.h"
#include "render/indirect_radiosity.h"
#include "render/indirect_sdf.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <deque>
#include <string>
#include <vector>

namespace
{
using namespace indirect_light;
using namespace indirect_contract;
using namespace headless_vulkan;

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

// Frames on the headless device: each submits the compute service's queued
// dispatches with a fence; completion is polled, never waited for, except
// at teardown.
class Frames final : public IResourceTracker
{
public:
	Frames( Device &device, ComputeResources &resources )
	    : m_device( device ), m_resources( resources ),
	      m_service( resources, [this] { return m_submitted + 1; },
	          [this] { return m_completed; } )
	{
	}
	// Before the device goes.
	void Release()
	{
		if ( m_queries != VK_NULL_HANDLE )
			vkDestroyQueryPool( m_device.device, m_queries, nullptr );
		m_queries = VK_NULL_HANDLE;
	}
	GpuComputeService &Service() { return m_service; }
	uint64_t Acquire( size_t ) override { return ++m_ids; }
	void Release( uint64_t, uint64_t ) override {}
	uint64_t SubmittedSerial() const override { return m_submitted; }
	uint64_t CompletedSerial() const override { return m_completed; }

	void Submit()
	{
		const uint64_t serial = ++m_submitted;
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		const auto start = std::chrono::steady_clock::now();
		const bool work = m_service.Pending();
		if ( m_queries == VK_NULL_HANDLE )
		{
			VkQueryPoolCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
			info.queryType = VK_QUERY_TYPE_TIMESTAMP;
			info.queryCount = 2 * kQuerySlots;
			vkCreateQueryPool( m_device.device, &info, nullptr, &m_queries );
			VkPhysicalDeviceProperties properties = {};
			vkGetPhysicalDeviceProperties( m_device.physical, &properties );
			m_period = properties.limits.timestampPeriod;
		}
		const uint32_t query = uint32_t( serial % kQuerySlots ) * 2;
		VkFence fence = headless_vulkan::Submit( m_device, &cmd,
		    [&]( VkCommandBuffer c )
		    {
			    vkCmdResetQueryPool( c, m_queries, query, 2 );
			    vkCmdWriteTimestamp( c, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_queries, query );
			    m_service.Record( c, serial );
			    vkCmdWriteTimestamp( c, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queries, query + 1 );
		    } );
		m_inFlight.push_back( { serial, fence, cmd, start, work } );
		Poll();
		// Two frames in flight, as a swapchain paces a renderer: frames are
		// counted in real submissions, not in how fast the test loop spins.
		while ( m_inFlight.size() > 2 )
		{
			vkWaitForFences( m_device.device, 1, &m_inFlight.front().fence, VK_TRUE, UINT64_MAX );
			Complete();
		}
	}
	void Poll()
	{
		while ( !m_inFlight.empty() &&
		        vkGetFenceStatus( m_device.device, m_inFlight.front().fence ) == VK_SUCCESS )
			Complete();
	}
	void Drain()
	{
		while ( !m_inFlight.empty() )
		{
			vkWaitForFences( m_device.device, 1, &m_inFlight.front().fence, VK_TRUE, UINT64_MAX );
			Complete();
		}
	}
	// The GPU time of the slowest submission that carried work (timestamps).
	double SlowestMs() const { return m_slowest; }
	void ResetTiming()
	{
		m_slowest = 0.0;
		m_times.clear();
	}
	// Every timed submission since ResetTiming, in milliseconds.
	const std::vector<double> &Times() const { return m_times; }

private:
	struct Pending
	{
		uint64_t serial;
		VkFence fence;
		VkCommandBuffer cmd;
		std::chrono::steady_clock::time_point start;
		bool work;
	};
	void Complete()
	{
		const Pending &done = m_inFlight.front();
		if ( done.work )
		{
			uint64_t stamps[2] = {};
			if ( vkGetQueryPoolResults( m_device.device, m_queries,
			         uint32_t( done.serial % kQuerySlots ) * 2, 2, sizeof( stamps ), stamps,
			         sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT ) == VK_SUCCESS )
			{
				const double ms = double( stamps[1] - stamps[0] ) * m_period * 1e-6;
				m_slowest = std::max( m_slowest, ms );
				m_times.push_back( ms );
			}
		}
		headless_vulkan::Finish( m_device, done.fence, done.cmd );
		m_completed = done.serial;
		m_resources.Collect( m_completed );
		m_inFlight.pop_front();
	}

	Device &m_device;
	ComputeResources &m_resources;
	GpuComputeService m_service;
	std::deque<Pending> m_inFlight;
	uint64_t m_submitted = 0;
	uint64_t m_completed = 0;
	uint64_t m_ids = 0;
	double m_slowest = 0.0;
	std::vector<double> m_times;
	static constexpr uint32_t kQuerySlots = 8;
	VkQueryPool m_queries = VK_NULL_HANDLE;
	float m_period = 1.0f;
};

std::shared_ptr<const SdfData> LoadSdf()
{
	return SdfData::FromBytes( LoadFile( "quality/fixtures/gi/sdfv/contract.sdfv" ) );
}

// TRIS bytes (tools/quality/sdf_volume.py write_tris): the contract scene's
// triangles for the ray-query producer.
std::shared_ptr<const WorldGeometry> LoadTriangles( const char *path )
{
	const std::vector<unsigned char> bytes = LoadFile( path );
	uint32_t header[3] = {};
	if ( bytes.size() < sizeof( header ) )
		return nullptr;
	std::memcpy( header, bytes.data(), sizeof( header ) );
	if ( header[0] != 0x53495254u ||
	     bytes.size() != sizeof( header ) + size_t( header[1] ) * 12 + size_t( header[2] ) * 4 )
		return nullptr;
	auto geometry = std::make_shared<WorldGeometry>();
	geometry->positions.resize( size_t( header[1] ) * 3 );
	geometry->indices.resize( header[2] );
	std::memcpy( geometry->positions.data(), bytes.data() + sizeof( header ),
	    geometry->positions.size() * 4 );
	std::memcpy( geometry->indices.data(), bytes.data() + sizeof( header ) + geometry->positions.size() * 4,
	    geometry->indices.size() * 4 );
	for ( uint32_t index : geometry->indices )
		if ( index >= header[1] )
			return nullptr;
	return geometry;
}

// The compute service with ray query hidden: a device without it.
class WithoutRayQuery final : public gpu_compute::IGpuCompute
{
public:
	explicit WithoutRayQuery( gpu_compute::IGpuCompute &inner ) : m_inner( inner ) {}
	gpu_compute::Caps Capabilities() const override
	{
		gpu_compute::Caps caps = m_inner.Capabilities();
		caps.rayQuery = false;
		return caps;
	}
	uint32_t CreateBuffer( size_t bytes, gpu_compute::BufferUse use ) override
	{
		return m_inner.CreateBuffer( bytes, use );
	}
	void *Map( uint32_t buffer ) override { return m_inner.Map( buffer ); }
	uint32_t CreateProgram( const char *name, const gpu_compute::Binding *bindings, uint32_t count,
	    uint32_t pushBytes ) override
	{
		return m_inner.CreateProgram( name, bindings, count, pushBytes );
	}
	uint32_t CreateGeometry( const float *, uint32_t, const uint32_t *, uint32_t ) override
	{
		return 0;
	}
	uint32_t CreateScene( const gpu_compute::SceneInstance *, uint32_t ) override { return 0; }
	uint64_t QueueDispatch( uint32_t program, const uint32_t *buffers, uint32_t count,
	    const void *push, uint32_t pushBytes, uint32_t groupsX, uint32_t groupsY,
	    uint32_t groupsZ ) override
	{
		return m_inner.QueueDispatch(
		    program, buffers, count, push, pushBytes, groupsX, groupsY, groupsZ );
	}
	uint64_t CompletedSerial() const override { return m_inner.CompletedSerial(); }
	void Retire( uint32_t resource, uint64_t afterSerial ) override
	{
		m_inner.Retire( resource, afterSerial );
	}

private:
	gpu_compute::IGpuCompute &m_inner;
};

// The probe at the origin: the mean of its layer's interior texels.
float ProbeMean( const Volume &volume, uint32_t layer )
{
	const mapcontainer::ProbeGridLayout &grid = volume.layout.grids[0];
	const uint32_t x0 = grid.irradianceOrigin[layer][0] + 1;
	const uint32_t y0 = grid.irradianceOrigin[layer][1] + 1;
	double sum = 0.0;
	for ( uint32_t v = 0; v < 6; ++v )
		for ( uint32_t u = 0; u < 6; ++u )
		{
			uint16_t half;
			std::memcpy( &half,
			    volume.bytes.data() + volume.layout.atlasOffset +
			        ( uint64_t( y0 + v ) * volume.layout.atlasWidth + x0 + u ) * 8,
			    2 );
			sum += mapcontainer::HalfToFloat( half );
		}
	return float( sum / 36.0 );
}

// The traced field's own estimate of the baked scene (its reference phase),
// per side of the contract's thin wall (x 47..49): the mean total light of
// the probes left (x < 47) and right (x > 49) of it.
bool ReferenceSides( Frames &frames, TraceMode mode, const std::shared_ptr<const Volume> &seed,
    const std::shared_ptr<const SdfData> &sdf, const std::shared_ptr<const WorldGeometry> &geometry,
    float *lit, float *dark )
{
	TracedProducer producer( mode );
	IndirectScene scene;
	scene.baked = seed;
	scene.sdf = sdf;
	scene.geometry = geometry;
	scene.gpu = &frames.Service();
	scene.policy = indirect_policy::Policy::BakedPlusDelta;
	if ( !producer.Begin( scene, PublishedVolume{ 0, seed }, frames ) )
		return false;
	const light_set::Snapshot lights;
	for ( uint32_t frame = 0; frame < producer.Caps().warmupFrames; ++frame )
	{
		FrameWork work;
		work.resources = &frames;
		work.frameSerial = uint64_t( frame ) + 1;
		producer.Schedule( work, lights );
		for ( auto &job : work.jobs )
			job();
		frames.Submit();
	}
	frames.Drain();
	Check( producer.Live(), "the reference phase ends within warmupFrames" );
	const std::vector<float> field = producer.ReferenceField();
	(void)producer.End();
	frames.Drain();
	const mapcontainer::ProbeGridLayout &grid = seed->layout.grids[0];
	if ( field.size() != size_t( grid.probeCount ) * SdfTracedProducer::kTexels * 12 )
		return false;
	double sums[2] = {}, counts[2] = {};
	for ( uint32_t probe = 0; probe < grid.probeCount; ++probe )
	{
		const float x = grid.origin[0] + float( probe % grid.dims[0] ) * grid.spacing[0];
		const int side = x > 49.0f ? 1 : 0;
		for ( uint32_t t = 0; t < SdfTracedProducer::kTexels; ++t )
			sums[side] += field[( size_t( probe ) * SdfTracedProducer::kTexels + t ) * 12];
		counts[side] += SdfTracedProducer::kTexels;
	}
	*lit = float( sums[0] / counts[0] );
	*dark = float( sums[1] / counts[1] );
	return true;
}

// The contract field with its thin wall erased (the leak defect): the wall's
// voxels take the clamp distance.
std::shared_ptr<const SdfData> WithoutTheWall( const SdfData &sdf )
{
	std::vector<unsigned char> bytes = sdf.bytes;
	const mapcontainer::SdfVolumeLayout &f = sdf.layout;
	const uint16_t far = FloatToHalf( f.maxDistance );
	for ( uint32_t z = 0; z < f.dims[2]; ++z )
		for ( uint32_t y = 0; y < f.dims[1]; ++y )
			for ( uint32_t x = 0; x < f.dims[0]; ++x )
			{
				const float cx = f.origin[0] + float( x ) * f.voxel;
				if ( cx < 40.0f || cx > 56.0f )
					continue;
				const size_t voxel = ( size_t( z ) * f.dims[1] + y ) * f.dims[0] + x;
				std::memcpy(
				    bytes.data() + f.voxelOffset + voxel * mapcontainer::kSdfVoxelBytes, &far, 2 );
			}
	return SdfData::FromBytes( std::move( bytes ) );
}

// The contract triangles without the thin wall's two faces (x 47 and 49).
std::shared_ptr<const WorldGeometry> WithoutTheWallTriangles( const WorldGeometry &geometry )
{
	auto out = std::make_shared<WorldGeometry>();
	out->positions = geometry.positions;
	for ( size_t t = 0; t + 2 < geometry.indices.size(); t += 3 )
	{
		bool wall = true;
		for ( int k = 0; k < 3; ++k )
		{
			const float x = geometry.positions[size_t( geometry.indices[t + k] ) * 3];
			wall = wall && ( x == 47.0f || x == 49.0f );
		}
		if ( !wall )
			out->indices.insert( out->indices.end(), geometry.indices.begin() + std::ptrdiff_t( t ),
			    geometry.indices.begin() + std::ptrdiff_t( t + 3 ) );
	}
	return out;
}

// A map's update cost: warm updates' median and p95 GPU time.
int Bench( Device &d, Frames &frames, const char *prbvPath, const char *sdfvPath,
    const char *wmshPath )
{
	auto baked = Volume::FromBytes( LoadFile( prbvPath ) );
	auto sdf = SdfData::FromBytes( LoadFile( sdfvPath ) );
	std::shared_ptr<const WorldGeometry> geometry;
	if ( wmshPath )
	{
		const std::vector<unsigned char> wmsh = LoadFile( wmshPath );
		geometry = WorldGeometryFromMesh( wmsh.data(), wmsh.size() );
	}
	if ( !baked || !sdf || ( wmshPath && !geometry ) )
	{
		std::fprintf( stderr, "bench: cannot load %s / %s\n", prbvPath, sdfvPath );
		return 2;
	}
	TracedProducer producer( wmshPath ? TraceMode::RayQuery : TraceMode::Sdf );
	IndirectScene scene;
	scene.baked = baked;
	scene.sdf = sdf;
	scene.geometry = geometry;
	scene.gpu = &frames.Service();
	scene.policy = indirect_policy::Policy::BakedPlusDelta;
	if ( !producer.Begin( scene, PublishedVolume{ 0, baked }, frames ) )
		return 2;
	light_set::Snapshot lights;
	constexpr int kWarm = 8;
	for ( int frame = 0; frame < 240; ++frame )
	{
		if ( frame == kWarm )
			frames.ResetTiming();
		if ( frame == 120 )
		{
			lights.styleScalars.assign( 64, 1.0f );
			lights.styleScalars[32] = 0.5f; // the first switchable light
		}
		FrameWork work;
		work.resources = &frames;
		work.frameSerial = uint64_t( frame ) + 1;
		producer.Schedule( work, lights );
		for ( auto &job : work.jobs )
			job();
		frames.Submit();
	}
	frames.Drain();
	std::vector<double> times = frames.Times();
	std::sort( times.begin(), times.end() );
	if ( times.empty() )
		return 2;
	std::printf( "BENCH {\"producer\": \"%s\", \"probes\": %u, \"voxels\": [%u, %u, %u], \"updates\": %zu, "
	             "\"gpu_ms_median\": %.3f, \"gpu_ms_p95\": %.3f, \"gpu_ms_max\": %.3f, "
	             "\"device\": \"%s\"}\n",
	    wmshPath ? "rayquery" : "sdf", baked->layout.grids[0].probeCount, sdf->layout.dims[0], sdf->layout.dims[1],
	    sdf->layout.dims[2], times.size(), times[times.size() / 2], times[times.size() * 95 / 100],
	    times.back(), d.name.c_str() );
	(void)producer.End();
	frames.Drain();
	return 0;
}

} // namespace

int main( int argc, char **argv )
{
	Device d;
	if ( !CreateDevice( &d ) )
	{
		std::fprintf( stderr, "SKIP: no Vulkan device\n" );
		return 77;
	}
	ComputeResources resources;
	std::string error;
	Check( resources.Init( d.physical, d.device, d.chain.Enabled(), &error ), "compute: " + error );
	Frames frames( d, resources );
	if ( ( argc == 4 || argc == 5 ) && std::string( argv[1] ) == "--bench" )
	{
		const int status = Bench( d, frames, argv[2], argv[3], argc == 5 ? argv[4] : nullptr );
		vkDeviceWaitIdle( d.device );
		frames.Release();
		resources.Shutdown();
		DestroyDevice( d );
		return status;
	}
	const auto seed = LoadSeed();
	const auto sdf = LoadSdf();
	const auto geometry = LoadTriangles( "quality/fixtures/gi/sdfv/contract.tris" );
	Check( seed && sdf && geometry, "the contract seed and its SDF and triangle scenes load" );
	if ( !seed || !sdf || !geometry )
		return testing::ReportConformance( g_checks, g_failures );

	Scenario scene;
	scene.seed = seed;
	scene.sdf = sdf;
	scene.geometry = geometry;
	scene.gpu = &frames.Service();
	scene.resources = &frames;
	scene.afterFrame = [&] { frames.Submit(); };
	// A proxy box around the probe at the origin, clear of room A's walls.
	Proxy box;
	box.lo[0] = box.lo[1] = box.lo[2] = -9.0f;
	box.hi[0] = box.hi[1] = box.hi[2] = 9.0f;
	box.reflectance = 0.6f;
	scene.geometryProxies = { box };
	scene.geometryOracle = []( const Volume &volume, const Volume &seed, const ProducerCaps & )
	{
		const float total = ProbeMean( volume, 0 );
		const float indirect = ProbeMean( volume, 1 );
		std::fprintf( stderr, "enclosed probe: total %.4f indirect %.4f (seed %.4f %.4f)\n", total,
		    indirect, ProbeMean( seed, 0 ), ProbeMean( seed, 1 ) );
		return total < 0.05f * ProbeMean( seed, 0 ) && indirect < 0.05f * ProbeMean( seed, 1 )
		           ? std::string()
		           : std::string( "the probe it encloses in a proxy is not dark" );
	};
	const bool rayQuery = frames.Service().Capabilities().rayQuery;
	std::printf( "device: %s, ray query %s\n", d.name.c_str(), rayQuery ? "on" : "off" );
	for ( TraceMode mode : { TraceMode::Sdf, TraceMode::RayQuery } )
	{
		const bool rq = mode == TraceMode::RayQuery;
		const char *name = rq ? "ray-query" : "SDF-traced";
		if ( rq && !rayQuery )
		{
			std::printf( "the ray-query producer: no ray query on this device\n" );
			continue;
		}
		const auto started = std::chrono::steady_clock::now();
		const auto broken = RunContract(
		    [mode]( const FakeGpu & ) { return std::make_unique<TracedProducer>( mode ); }, scene,
		    nullptr );
		for ( const auto &b : broken )
			std::fprintf( stderr, "  %s: %s\n", name, b.c_str() );
		Check( broken.empty(), std::string( "the " ) + name + " producer passes the shared suite on the GPU" );
		std::printf( "%s shared suite: %.1f s\n", name,
		    std::chrono::duration<double>( std::chrono::steady_clock::now() - started ).count() );

		// Thin wall: the composition (the bake times the field's ratio) would
		// hide a leak in the traced field, so the field's own reference is
		// judged: the dark side of the wall stays dark while the lit side
		// holds the furnace's light. Erasing the wall (from the distance field,
		// or its triangles) must fail it.
		float lit = 0.0f, dark = 0.0f;
		const bool traced = ReferenceSides( frames, mode, seed, sdf, geometry, &lit, &dark );
		std::printf( "%s reference field: lit side %.4f, dark side %.4f\n", name, lit, dark );
		Check( traced && lit > 0.5f && dark < 0.02f * lit,
		    std::string( "the " ) + name + " field sees no light through the thin wall" );
		float leakyLit = 0.0f, leakyDark = 0.0f;
		const bool leakyTraced =
		    rq ? ReferenceSides( frames, mode, seed, sdf, WithoutTheWallTriangles( *geometry ),
		             &leakyLit, &leakyDark )
		       : ReferenceSides( frames, mode, seed, WithoutTheWall( *sdf ), geometry, &leakyLit,
		             &leakyDark );
		std::printf( "%s without the wall: lit side %.4f, dark side %.4f\n", name, leakyLit,
		    leakyDark );
		Check( leakyTraced && leakyDark >= 0.02f * leakyLit,
		    std::string( "erasing the wall (the leak defect) is detected for the " ) + name +
		        " producer" );
	}

	// A device without ray query does not run the ray-query producer: Begin
	// fails with missing-feature and creates nothing.
	{
		WithoutRayQuery plain( frames.Service() );
		RayQueryProducer producer;
		IndirectScene without;
		without.baked = seed;
		without.sdf = sdf;
		without.geometry = geometry;
		without.gpu = &plain;
		without.policy = indirect_policy::Policy::BakedPlusDelta;
		const size_t before = resources.LiveCount();
		const auto begun = producer.Begin( without, PublishedVolume{ 0, seed }, frames );
		Check( !begun && begun.Error() == IndirectError::MissingFeature &&
		           resources.LiveCount() == before,
		    "without ray query the ray-query producer fails with missing-feature, creating "
		    "nothing" );
		Check( ( producer.Caps().requiredFeatures & ( 1u << 7 ) ) != 0,
		    "the ray-query producer requires kRayQuery" );
	}

	// The radiosity producer, which claims no GeometryMotion, run on the SDF
	// scene's geometry change: it cannot darken the enclosed probe (the
	// scenario tells the two apart; G6.2 does the same on the door map).
	{
		RadiosityProducer radiosity;
		(void)radiosity;
		Check( !( radiosity.Caps().responds & kGeometryMotion ),
		    "the radiosity producer does not claim GeometryMotion" );
	}

	// GPU cost of one update on the contract scene (16 probes).
	{
		SdfTracedProducer producer;
		IndirectScene scene2;
		scene2.baked = seed;
		scene2.sdf = sdf;
		scene2.gpu = &frames.Service();
		scene2.policy = indirect_policy::Policy::BakedPlusDelta;
		Check( bool( producer.Begin( scene2, PublishedVolume{ 0, seed }, frames ) ),
		    "Begin on the contract scene" );
		frames.ResetTiming();
		const light_set::Snapshot lights;
		for ( int frame = 0; frame < 40; ++frame )
		{
			FrameWork work;
			work.resources = &frames;
			producer.Schedule( work, lights );
			frames.Submit();
			frames.Drain();
		}
		std::printf( "update GPU time (timestamps, 16 probes): %.3f ms\n", frames.SlowestMs() );
		(void)producer.End();
		frames.Drain();
	}
	frames.Drain();
	vkDeviceWaitIdle( d.device );
	frames.Release();
	resources.Shutdown();
	DestroyDevice( d );
	std::printf( "validation layer %s, %lu message(s)\n", d.validation ? "on" : "absent",
	    g_validationMessages );
	if ( d.validation )
		Check( g_validationMessages == 0, "no validation messages" );
	return testing::ReportConformance( g_checks, g_failures );
}
