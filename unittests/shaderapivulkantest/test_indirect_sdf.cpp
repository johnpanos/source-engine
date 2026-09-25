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
//          `--bench <volume.prbv> <field.sdfv> [world.wmsh] [--focus file]
//          [--budget n] [--proxy x0 y0 z0 x1 y1 z1] [--frames n]` instead
//          measures a map's update cost (RFC 0011 G6.4, the budget's gpu_ms:
//          the median over warm updates of the dispatch's timestamps): the
//          reference phase and a live phase after the first switchable light
//          (style 32) halves and, with --proxy, a box appears; also the CPU
//          time of Schedule before and after the change.
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
#include <cstdlib>
#include <fstream>
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
	    : m_device( device ), m_resources( resources ), m_service(
	                                                        resources,
	                                                        [this]
	                                                        {
		                                                        return m_submitted + 1;
	                                                        },
	                                                        [this]
	                                                        {
		                                                        return m_completed;
	                                                        } )
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
			    vkCmdWriteTimestamp(
			        c, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queries, query + 1 );
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
	std::memcpy( geometry->indices.data(),
	    bytes.data() + sizeof( header ) + geometry->positions.size() * 4,
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

// The contract field with its lights replaced: its one light cell lists
// every light, or (`emptyCell`, the defect of a cell that omits a light)
// none.
std::shared_ptr<const SdfData> WithLights(
    const SdfData &sdf, const std::vector<mapcontainer::SdfLight> &lights, bool emptyCell )
{
	const mapcontainer::SdfVolumeLayout &f = sdf.layout;
	std::vector<unsigned char> bytes( sdf.bytes.begin(), sdf.bytes.begin() + f.lightOffset );
	const uint32_t count = uint32_t( lights.size() ), entries = emptyCell ? 0 : count;
	std::memcpy( &bytes[44], &count, 4 );
	std::memcpy( &bytes[92], &entries, 4 );
	const unsigned char *raw = reinterpret_cast<const unsigned char *>( lights.data() );
	bytes.insert( bytes.end(), raw, raw + lights.size() * sizeof( mapcontainer::SdfLight ) );
	for ( uint32_t first : { 0u, entries } )
		bytes.insert( bytes.end(), reinterpret_cast<const unsigned char *>( &first ),
		    reinterpret_cast<const unsigned char *>( &first ) + 4 );
	for ( uint32_t l = 0; l < entries; ++l )
	{
		const uint16_t index = uint16_t( l );
		bytes.insert( bytes.end(), reinterpret_cast<const unsigned char *>( &index ),
		    reinterpret_cast<const unsigned char *>( &index ) + 2 );
	}
	if ( entries % 2 )
		bytes.insert( bytes.end(), 2, 0 );
	return SdfData::FromBytes( std::move( bytes ) );
}

mapcontainer::SdfLight SmallLight( bool spot, float x, float y, float z, int style )
{
	mapcontainer::SdfLight light = {};
	light.kind =
	    uint32_t( spot ? mapcontainer::SdfLightKind::Spot : mapcontainer::SdfLightKind::Sphere );
	light.style = style;
	light.rgb[0] = light.rgb[1] = light.rgb[2] = 400.0f;
	light.a[0] = x, light.a[1] = y, light.a[2] = z;
	if ( spot )
	{
		light.b[2] = -1.0f; // aims down
		light.c[0] = 2.0f;
		light.c[1] = 0.95f; // inner cone, 18 degrees
		light.c[2] = 0.90f; // outer cone, 26 degrees
		light.reserved[0] = 1.0f;
	}
	else
		light.b[0] = 2.0f;
	return light;
}

// Octahedral texel t's direction (sdf_probe_trace.comp OctDecode).
void TexelDirection( uint32_t t, float out[3] )
{
	constexpr uint32_t kInterior = 6;
	const float px = ( float( t % kInterior ) + 0.5f ) / float( kInterior ) * 2.0f - 1.0f;
	const float py = ( float( t / kInterior ) + 0.5f ) / float( kInterior ) * 2.0f - 1.0f;
	float v[3] = { px, py, 1.0f - std::fabs( px ) - std::fabs( py ) };
	if ( v[2] < 0.0f )
	{
		const float x = ( 1.0f - std::fabs( py ) ) * ( px >= 0.0f ? 1.0f : -1.0f );
		const float y = ( 1.0f - std::fabs( px ) ) * ( py >= 0.0f ? 1.0f : -1.0f );
		v[0] = x, v[1] = y;
	}
	const float length = std::sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
	for ( int k = 0; k < 3; ++k )
		out[k] = v[k] / length;
}

// A traced producer driven for `frames` frames with a focus and a budget;
// `scalar` (if >= 0) sets style 32 from frame `change` on.
struct Driven
{
	std::vector<float> reference;
	bool live = false;
	uint64_t probeUpdates = 0;
	std::vector<uint32_t> liveUpdates;
	std::vector<uint8_t> referenced;
};

Driven Drive( Frames &frames, const std::shared_ptr<const Volume> &seed,
    const std::shared_ptr<const SdfData> &sdf, uint32_t count, std::vector<uint32_t> focus,
    uint32_t budget, float scalar = -1.0f, uint32_t change = 0,
    std::vector<uint32_t> focusAfter = {} )
{
	Driven out;
	SdfTracedProducer producer;
	IndirectScene scene;
	scene.baked = seed;
	scene.sdf = sdf;
	scene.gpu = &frames.Service();
	scene.policy = indirect_policy::Policy::BakedPlusDelta;
	if ( !producer.Begin( scene, PublishedVolume{ 0, seed }, frames ) )
		return out;
	light_set::Snapshot lights;
	for ( uint32_t frame = 0; frame < count; ++frame )
	{
		if ( scalar >= 0.0f && frame == change )
		{
			lights.styleScalars.assign( 64, 1.0f );
			lights.styleScalars[32] = scalar;
			focus = focusAfter;
		}
		FrameWork work;
		work.resources = &frames;
		work.frameSerial = uint64_t( frame ) + 1;
		work.focusProbes = focus;
		work.probeBudget = budget;
		producer.Schedule( work, lights );
		for ( auto &job : work.jobs )
			job();
		frames.Submit();
	}
	frames.Drain();
	out.reference = producer.ReferenceField();
	out.live = producer.Live();
	out.probeUpdates = producer.ProbeUpdates();
	const uint32_t probes = seed->layout.grids[0].probeCount;
	for ( uint32_t p = 0; p < probes; ++p )
	{
		out.liveUpdates.push_back( producer.LiveUpdates( p ) );
		out.referenced.push_back( producer.Referenced( p ) );
	}
	(void)producer.End();
	frames.Drain();
	return out;
}

// Probe p's analytic direct light per texel (the field's third vec4).
float Analytic( const std::vector<float> &field, uint32_t p, uint32_t t )
{
	return field[( size_t( p ) * SdfTracedProducer::kTexels + t ) * 12 + 8];
}

// Sphere and spot lights against their closed forms at the contract's
// probes 0 (0, 0, 0) and 1 (32, 0, 0), from a light 32 units above probe 0:
// L r^2 / d^2 times each texel's cosine toward the light. The spot's cone
// (26 degrees) excludes probe 1 (45 degrees off its axis), which the sphere
// lights. A light cell that omits the light leaves the probe unlit.
void NativeLights( Frames &frames, const std::shared_ptr<const Volume> &seed, const SdfData &sdf )
{
	const uint32_t warm = SdfTracedProducer{}.Caps().warmupFrames;
	const auto run = [&]( bool spot, bool emptyCell )
	{
		const auto field =
		    WithLights( sdf, { SmallLight( spot, 0.0f, 0.0f, 32.0f, -1 ) }, emptyCell );
		Check( bool( field ), "a field with a native light validates" );
		return field ? Drive( frames, seed, field, warm, {}, 0 ).reference : std::vector<float>{};
	};
	for ( const bool spot : { false, true } )
	{
		const std::string name = spot ? "spot" : "sphere";
		const std::vector<float> field = run( spot, false );
		if ( field.empty() )
			continue;
		// Probe 0: the light straight up, at 32 units.
		const float peak = 400.0f * 4.0f / ( 32.0f * 32.0f );
		double worst = 0.0;
		for ( uint32_t t = 0; t < SdfTracedProducer::kTexels; ++t )
		{
			float dir[3];
			TexelDirection( t, dir );
			const float expected = peak * std::max( 0.0f, dir[2] );
			if ( expected > 0.3f * peak )
				worst = std::max(
				    worst, double( std::fabs( Analytic( field, 0, t ) - expected ) / expected ) );
		}
		std::printf( "%s light: probe 0 worst texel error %.3f\n", name.c_str(), worst );
		Check( worst < 0.05, name + " light: probe 0's analytic light is L r^2 / d^2 cos (5%)" );
		// Probe 1: 45 degrees off, at 32 sqrt 2.
		float best = 0.0f;
		for ( uint32_t t = 0; t < SdfTracedProducer::kTexels; ++t )
			best = std::max( best, Analytic( field, 1, t ) );
		const float sideways = 400.0f * 4.0f / ( 2.0f * 32.0f * 32.0f );
		std::printf( "%s light: probe 1 brightest texel %.4f (unshaded sphere %.4f)\n",
		    name.c_str(), best, sideways );
		Check( spot ? best < 0.01f * sideways : best > 0.8f * sideways,
		    spot ? name + " light: probe 1, outside the cone, is unlit"
		         : name + " light: probe 1 is lit (the cone check's control)" );
	}
	const std::vector<float> omitted = run( false, true );
	float lit = 0.0f;
	for ( uint32_t t = 0; t < SdfTracedProducer::kTexels && !omitted.empty(); ++t )
		lit = std::max( lit, Analytic( omitted, 0, t ) );
	Check( !omitted.empty() && lit == 0.0f,
	    "a light cell that omits the light leaves its probes unlit" );
}

// Focus and budget: with a focus of probe 0 and no budget, only probe 0
// builds its reference; a focus of every probe is the no-focus schedule byte
// for byte; after a change, a focused probe updates while an unfocused one
// waits, and a budget brings every probe to rest.
void Scheduling( Frames &frames, const std::shared_ptr<const Volume> &seed, const SdfData &sdf )
{
	const uint32_t warm = SdfTracedProducer{}.Caps().warmupFrames;
	const uint32_t probes = seed->layout.grids[0].probeCount;
	const auto field = WithLights( sdf, { SmallLight( false, 0.0f, 0.0f, 32.0f, 32 ) }, false );
	if ( !field )
		return;
	const Driven one = Drive( frames, seed, field, warm, { 0 }, 0 );
	uint32_t others = 0;
	for ( uint32_t p = 1; p < probes; ++p )
		others += one.referenced[p];
	Check( one.referenced[0] && others == 0 &&
	           one.probeUpdates == SdfTracedProducer::kReferenceUpdates,
	    "a focus of probe 0 without a budget references probe 0 only (" +
	        std::to_string( one.probeUpdates ) + " probe updates)" );
	std::vector<uint32_t> every( probes );
	for ( uint32_t p = 0; p < probes; ++p )
		every[p] = p;
	const Driven all = Drive( frames, seed, field, warm, every, 0 );
	const Driven none = Drive( frames, seed, field, warm, {}, 0 );
	Check( all.live && none.live && all.reference == none.reference,
	    "a focus of every probe is the no-focus schedule, byte for byte" );
	// Live: the reference with no focus, then style 32 at 0.5 with a focus of probe 0.
	const uint32_t settle = warm + 20;
	const Driven focused = Drive( frames, seed, field, settle, {}, 0, 0.5f, warm, { 0 } );
	Check( focused.liveUpdates[0] > 0 && focused.liveUpdates[1] == 0,
	    "after a change, the focused probe updates and an unfocused one waits (" +
	        std::to_string( focused.liveUpdates[0] ) + ", " +
	        std::to_string( focused.liveUpdates[1] ) + ")" );
	// 15 unfocused probes x 72 live updates at 8 a update: 135 updates, in
	// frames enough for a GPU shared with other work.
	const Driven budgeted = Drive( frames, seed, field, warm + 800, {}, 8, 0.5f, warm, { 0 } );
	bool rested = true;
	for ( uint32_t p = 0; p < probes; ++p )
		rested = rested && budgeted.liveUpdates[p] >= SdfTracedProducer::kActiveUpdates;
	Check( rested, "with a budget every probe's live updates finish" );
}

// A map's update cost: warm updates' median and p95 GPU time, in the
// reference phase and after a change at frame 120 (style 32 halves, and
// with --proxy a box appears). --focus names the probes updated every
// update (a file of probe indices, e.g. tools/quality/gi_focus.py's), the
// others take --budget an update; without --focus every probe updates.
struct BenchOptions
{
	const char *prbv = nullptr;
	const char *sdfv = nullptr;
	const char *wmsh = nullptr;
	std::vector<uint32_t> focus;
	uint32_t budget = 128;
	bool proxy = false;
	Proxy box;
	int frames = 240;
};

double Median( std::vector<double> times, double fraction = 0.5 )
{
	if ( times.empty() )
		return 0.0;
	std::sort( times.begin(), times.end() );
	return times[std::min( times.size() - 1, size_t( double( times.size() ) * fraction ) )];
}

int Bench( Device &d, Frames &frames, const BenchOptions &options )
{
	auto baked = Volume::FromBytes( LoadFile( options.prbv ) );
	auto sdf = SdfData::FromBytes( LoadFile( options.sdfv ) );
	std::shared_ptr<const WorldGeometry> geometry;
	if ( options.wmsh )
	{
		const std::vector<unsigned char> wmsh = LoadFile( options.wmsh );
		geometry = WorldGeometryFromMesh( wmsh.data(), wmsh.size() );
	}
	if ( !baked || !sdf || ( options.wmsh && !geometry ) )
	{
		std::fprintf( stderr, "bench: cannot load %s / %s\n", options.prbv, options.sdfv );
		return 2;
	}
	TracedProducer producer( options.wmsh ? TraceMode::RayQuery : TraceMode::Sdf );
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
	constexpr int kChange = 120;
	std::vector<double> reference;
	std::vector<Proxy> proxies;
	uint64_t probesBefore = 0;
	// Schedule's CPU time on the frame thread (its composition reads the
	// field back from host-cached memory), before and after the change.
	std::vector<double> scheduleBefore, scheduleAfter;
	for ( int frame = 0; frame < options.frames; ++frame )
	{
		if ( frame == kWarm )
			frames.ResetTiming();
		if ( frame == kChange )
		{
			frames.Drain();
			reference = frames.Times();
			probesBefore = producer.ProbeUpdates();
			frames.ResetTiming();
			lights.styleScalars.assign( 64, 1.0f );
			lights.styleScalars[32] = 0.5f; // the first switchable light
			if ( options.proxy )
				proxies = { options.box };
		}
		FrameWork work;
		work.resources = &frames;
		work.frameSerial = uint64_t( frame ) + 1;
		work.focusProbes = options.focus;
		work.probeBudget = options.budget;
		work.proxies = proxies;
		const auto started = std::chrono::steady_clock::now();
		producer.Schedule( work, lights );
		for ( auto &job : work.jobs )
			job();
		const double ms = std::chrono::duration<double, std::milli>(
		    std::chrono::steady_clock::now() - started )
		                      .count();
		if ( frame >= kWarm )
			( frame < kChange ? scheduleBefore : scheduleAfter ).push_back( ms );
		frames.Submit();
	}
	frames.Drain();
	const std::vector<double> live = frames.Times();
	std::printf( "BENCH {\"producer\": \"%s\", \"probes\": %u, \"voxels\": [%u, %u, %u], "
	             "\"lights\": %u, \"focus\": %zu, \"budget\": %u, \"reference_updates\": %zu, "
	             "\"reference_ms_median\": %.3f, \"reference_ms_p95\": %.3f, "
	             "\"live_updates\": %zu, \"live_ms_median\": %.3f, \"live_ms_p95\": %.3f, "
	             "\"probe_updates\": [%llu, %llu], \"live\": %s, "
	             "\"schedule_cpu_ms_median\": [%.4f, %.4f], \"schedule_cpu_ms_p95\": [%.4f, %.4f], "
	             "\"device\": \"%s\"}\n",
	    options.wmsh ? "rayquery" : "sdf", baked->layout.grids[0].probeCount, sdf->layout.dims[0],
	    sdf->layout.dims[1], sdf->layout.dims[2], sdf->layout.lightCount, options.focus.size(),
	    options.budget, reference.size(), Median( reference ), Median( reference, 0.95 ),
	    live.size(), Median( live ), Median( live, 0.95 ), (unsigned long long)probesBefore,
	    (unsigned long long)( producer.ProbeUpdates() - probesBefore ),
	    producer.Live() ? "true" : "false", Median( scheduleBefore ), Median( scheduleAfter ),
	    Median( scheduleBefore, 0.95 ), Median( scheduleAfter, 0.95 ), d.name.c_str() );
	(void)producer.End();
	frames.Drain();
	return 0;
}

// Probe p's mean over its irradiance tile's interior in `layer`.
float ProbeMeanAt( const Volume &volume, uint32_t layer, uint32_t probe )
{
	const mapcontainer::ProbeGridLayout &grid = volume.layout.grids[0];
	const uint32_t tile = mapcontainer::kProbeIrradianceTile;
	const uint32_t x0 = grid.irradianceOrigin[layer][0] + ( probe % grid.tilesPerRow ) * tile + 1;
	const uint32_t y0 = grid.irradianceOrigin[layer][1] + ( probe / grid.tilesPerRow ) * tile + 1;
	double sum = 0.0;
	for ( uint32_t v = 0; v < tile - 2; ++v )
		for ( uint32_t u = 0; u < tile - 2; ++u )
		{
			uint16_t half;
			std::memcpy( &half,
			    volume.bytes.data() + volume.layout.atlasOffset +
			        ( uint64_t( y0 + v ) * volume.layout.atlasWidth + x0 + u ) * 8,
			    2 );
			sum += mapcontainer::HalfToFloat( half );
		}
	return float( sum / double( ( tile - 2 ) * ( tile - 2 ) ) );
}

// RFC 0011 G10: light through an open portal pair. A pair covering the
// contract's thin wall (x 47 facing room A and x 49 facing room B, the
// wall's whole 64 x 64 units, each a 2-unit translation onto the other) is
// the wall erased: room B's probes, black behind the wall, take the light
// they take with the wall removed from the scene. The erased scene's light
// is its own reference field's (a producer publishes the change from its
// reference, which there already holds the leak): `erasedDark`, the mean
// ReferenceSides measures. The wall's run must stay black and the open
// pair's must match it.
std::string PortalTransport(
    Frames &frames, TraceMode mode, const IndirectScene &walled, float erasedDark )
{
	const auto run = [&]( const IndirectScene &base, bool portals, float *darkIndirect,
	                      float *darkTotal )
	{
		TracedProducer producer( mode );
		IndirectScene scene = base;
		scene.gpu = &frames.Service();
		scene.policy = indirect_policy::Policy::BakedPlusDelta;
		if ( !producer.Begin( scene, PublishedVolume{ 0, scene.baked }, frames ) )
			return false;
		Portal pair[2];
		for ( int side = 0; side < 2; ++side )
		{
			Portal &portal = pair[side];
			portal.origin[0] = side == 0 ? 47.0f : 49.0f;
			portal.origin[1] = 16.0f, portal.origin[2] = 16.0f;
			portal.forward[0] = side == 0 ? -1.0f : 1.0f;
			portal.right[1] = 1.0f;
			portal.up[2] = 1.0f;
			portal.halfWidth = portal.halfHeight = 32.0f;
			// Identity rotation, a translation onto the other face.
			portal.toLinked[0] = portal.toLinked[5] = portal.toLinked[10] = 1.0f;
			portal.toLinked[3] = side == 0 ? 2.0f : -2.0f;
		}
		const light_set::Snapshot lights;
		const uint32_t count = producer.Caps().warmupFrames + 120;
		for ( uint32_t frame = 0; frame < count; ++frame )
		{
			FrameWork work;
			work.resources = &frames;
			work.frameSerial = uint64_t( frame ) + 1;
			if ( portals && frame >= producer.Caps().warmupFrames )
				work.portals = std::span<const Portal>( pair, 2 );
			producer.Schedule( work, lights );
			for ( auto &job : work.jobs )
				job();
			frames.Submit();
		}
		frames.Drain();
		const auto published = producer.Published();
		const Volume &volume = published ? *published->volume : *scene.baked;
		const mapcontainer::ProbeGridLayout &grid = volume.layout.grids[0];
		double indirect = 0.0, total = 0.0, probes = 0.0;
		for ( uint32_t probe = 0; probe < grid.probeCount; ++probe )
		{
			const float x = grid.origin[0] + float( probe % grid.dims[0] ) * grid.spacing[0];
			if ( x <= 49.0f )
				continue;
			indirect += ProbeMeanAt( volume, 1, probe );
			total += ProbeMeanAt( volume, 0, probe );
			probes += 1.0;
		}
		*darkIndirect = float( indirect / std::max( probes, 1.0 ) );
		*darkTotal = float( total / std::max( probes, 1.0 ) );
		(void)producer.End();
		frames.Drain();
		return probes > 0.0;
	};
	float closedIndirect, closedTotal, openIndirect, openTotal;
	if ( !run( walled, false, &closedIndirect, &closedTotal ) ||
	     !run( walled, true, &openIndirect, &openTotal ) )
		return "the producer did not run";
	std::fprintf( stderr,
	    "room B probes, total (indirect): wall %.4f (%.4f), open pair %.4f (%.4f), wall erased "
	    "%.4f\n",
	    closedTotal, closedIndirect, openTotal, openIndirect, erasedDark );
	if ( !( erasedDark > 0.0f ) || !( closedTotal < 0.1f * erasedDark ) )
		return "room B is lit through the closed wall";
	// The erased wall leaves a 2-unit slot open to the outside around the
	// rooms' junction, which the pair does not: the two are alike, not equal
	// (the Cycles portal-light fixture is the accurate oracle).
	if ( !( openTotal > 0.5f * erasedDark && openTotal < 1.5f * erasedDark ) )
		return "the open pair does not carry the light the erased wall lets through";
	if ( std::fabs( ( openTotal - openIndirect ) - ( closedTotal - closedIndirect ) ) > 0.01f )
		return "light through the pair landed outside the indirect layer";
	return {};
}

// RFC 0011 G9: the frame's unbaked lights. Probe 0's published light with
// no unbaked light, with an inverse-square light on its side of the
// contract's thin wall (x 47..49), and with that light moved behind the
// wall. The light near it adds bounce (the indirect layer rises) but no
// direct light at the probe (total minus indirect is unchanged: direct light
// from unbaked lights is the world's and models' own); behind the wall the
// probe is back at its baseline. Returns "" or what failed.
std::string UnbakedLights( Frames &frames, TraceMode mode, const IndirectScene &base )
{
	TracedProducer producer( mode );
	IndirectScene scene = base;
	scene.gpu = &frames.Service();
	scene.policy = indirect_policy::Policy::BakedPlusDelta;
	if ( !producer.Begin( scene, PublishedVolume{ 0, scene.baked }, frames ) )
		return "Begin failed";
	light_set::Snapshot lights;
	const auto run = [&]( uint32_t count, float *total, float *indirect )
	{
		for ( uint32_t frame = 0; frame < count; ++frame )
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
		const auto published = producer.Published();
		const Volume &volume = published ? *published->volume : *scene.baked;
		*total = ProbeMean( volume, 0 );
		*indirect = ProbeMean( volume, 1 );
	};
	float total0, indirect0, total1, indirect1, total2, indirect2;
	run( producer.Caps().warmupFrames + 8, &total0, &indirect0 );
	light_set::RuntimeLight bulb;
	bulb.id = light_set::Builder::kFirstDynamicId;
	bulb.kind = light_set::LightKind::Dynamic;
	bulb.falloff = light_set::LightFalloff::InverseSquare;
	bulb.sourceRadius = light_set::kInverseSquareSourceRadius;
	bulb.position[0] = 16.0f, bulb.position[2] = 16.0f;
	bulb.color[0] = bulb.color[1] = bulb.color[2] = 2.0f;
	lights.lights = { bulb };
	run( 120, &total1, &indirect1 );
	lights.lights[0].position[0] = 80.0f; // behind the wall
	run( 120, &total2, &indirect2 );
	(void)producer.End();
	frames.Drain();
	std::fprintf( stderr,
	    "unbaked light, probe 0: indirect %.4f -> %.4f near -> %.4f behind the wall; direct "
	    "%.4f -> %.4f -> %.4f\n",
	    indirect0, indirect1, indirect2, total0 - indirect0, total1 - indirect1,
	    total2 - indirect2 );
	if ( !( indirect1 > indirect0 + 0.02f ) )
		return "the light near the probe adds no bounce";
	if ( std::fabs( ( total1 - indirect1 ) - ( total0 - indirect0 ) ) >
	     0.1f * std::max( total0 - indirect0, 0.01f ) )
		return "the unbaked light added direct light at the probe";
	if ( std::fabs( indirect2 - indirect0 ) > 0.25f * ( indirect1 - indirect0 ) )
		return "the light moved behind the wall still lights the probe";
	return {};
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
	if ( argc >= 4 && std::string( argv[1] ) == "--bench" )
	{
		BenchOptions options;
		options.prbv = argv[2];
		options.sdfv = argv[3];
		for ( int i = 4; i < argc; ++i )
		{
			const std::string arg = argv[i];
			if ( arg == "--focus" && i + 1 < argc )
			{
				std::ifstream file( argv[++i] );
				uint32_t probe;
				while ( file >> probe )
					options.focus.push_back( probe );
			}
			else if ( arg == "--budget" && i + 1 < argc )
				options.budget = uint32_t( std::strtoul( argv[++i], nullptr, 10 ) );
			else if ( arg == "--frames" && i + 1 < argc )
				options.frames = std::atoi( argv[++i] );
			else if ( arg == "--proxy" && i + 6 < argc )
			{
				options.proxy = true;
				for ( int k = 0; k < 3; ++k )
					options.box.lo[k] = std::strtof( argv[++i], nullptr );
				for ( int k = 0; k < 3; ++k )
					options.box.hi[k] = std::strtof( argv[++i], nullptr );
				options.box.reflectance = 0.5f;
			}
			else
				options.wmsh = argv[i];
		}
		const int status = Bench( d, frames, options );
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
	scene.afterFrame = [&]
	{
		frames.Submit();
	};
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
		    [mode]( const FakeGpu & )
		    {
			    return std::make_unique<TracedProducer>( mode );
		    },
		    scene, nullptr );
		for ( const auto &b : broken )
			std::fprintf( stderr, "  %s: %s\n", name, b.c_str() );
		Check( broken.empty(),
		    std::string( "the " ) + name + " producer passes the shared suite on the GPU" );
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
		       : ReferenceSides(
		             frames, mode, seed, WithoutTheWall( *sdf ), geometry, &leakyLit, &leakyDark );
		std::printf(
		    "%s without the wall: lit side %.4f, dark side %.4f\n", name, leakyLit, leakyDark );
		Check( leakyTraced && leakyDark >= 0.02f * leakyLit,
		    std::string( "erasing the wall (the leak defect) is detected for the " ) + name +
		        " producer" );
	}

	NativeLights( frames, seed, *sdf );
	Scheduling( frames, seed, *sdf );

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

	// RFC 0011 G9: unbaked lights reach the traced field.
	for ( TraceMode mode : { TraceMode::Sdf, TraceMode::RayQuery } )
	{
		if ( mode == TraceMode::RayQuery && !frames.Service().Capabilities().rayQuery )
			continue;
		const std::string name = mode == TraceMode::RayQuery ? "ray-query" : "SDF-traced";
		IndirectScene unbaked;
		unbaked.baked = seed;
		unbaked.sdf = sdf;
		unbaked.geometry = geometry;
		const std::string failed = UnbakedLights( frames, mode, unbaked );
		if ( !failed.empty() )
			std::fprintf( stderr, "  %s: %s\n", name.c_str(), failed.c_str() );
		Check( failed.empty(), "the " + name +
		                           " producer bounces an unbaked light, follows it "
		                           "behind the wall, and adds none of its direct light" );
	}

	// RFC 0011 G10: light through an open portal pair.
	for ( TraceMode mode : { TraceMode::Sdf, TraceMode::RayQuery } )
	{
		if ( mode == TraceMode::RayQuery && !frames.Service().Capabilities().rayQuery )
			continue;
		const std::string name = mode == TraceMode::RayQuery ? "ray-query" : "SDF-traced";
		IndirectScene walled;
		walled.baked = seed;
		walled.sdf = sdf;
		walled.geometry = geometry;
		// The exact erased wall: the ray-query field without the wall's two
		// faces (the SDF's erased slab also removes floor and ceiling). Without
		// ray query the SDF's own slab stands in, and only its darkness counts.
		float erasedLit = 0.0f, erasedDark = 0.0f;
		const bool exact = frames.Service().Capabilities().rayQuery;
		const bool erasedTraced =
		    exact ? ReferenceSides( frames, TraceMode::RayQuery, seed, sdf,
		                WithoutTheWallTriangles( *geometry ), &erasedLit, &erasedDark )
		          : ReferenceSides( frames, mode, seed, WithoutTheWall( *sdf ), geometry,
		                &erasedLit, &erasedDark );
		const std::string failed = erasedTraced
		                               ? PortalTransport( frames, mode, walled, erasedDark )
		                               : std::string( "the erased-wall reference did not run" );
		if ( !failed.empty() )
			std::fprintf( stderr, "  %s: %s\n", name.c_str(), failed.c_str() );
		Check( failed.empty(), "the " + name + " producer carries light through an open portal "
		                           "pair, in the indirect layer" );
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
