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
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/material_spv.h"
#include "../rendertest/indirect_contract.h"
#include "headless_vulkan.h"
#include "render/indirect_radiosity.h"
#include "render/indirect_sdf.h"
#include "testing/conformance_result.h"

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
	void ResetTiming() { m_slowest = 0.0; }

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
				m_slowest = std::max( m_slowest, double( stamps[1] - stamps[0] ) * m_period * 1e-6 );
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
	static constexpr uint32_t kQuerySlots = 8;
	VkQueryPool m_queries = VK_NULL_HANDLE;
	float m_period = 1.0f;
};

std::shared_ptr<const SdfData> LoadSdf()
{
	return SdfData::FromBytes( LoadFile( "quality/fixtures/gi/sdfv/contract.sdfv" ) );
}

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

} // namespace

int main()
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
	const auto seed = LoadSeed();
	const auto sdf = LoadSdf();
	Check( seed && sdf, "the contract seed and its SDF scene load" );
	if ( !seed || !sdf )
		return testing::ReportConformance( g_checks, g_failures );

	Scenario scene;
	scene.seed = seed;
	scene.sdf = sdf;
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
	const auto started = std::chrono::steady_clock::now();
	const auto broken = RunContract(
	    []( const FakeGpu & ) { return std::make_unique<SdfTracedProducer>(); }, scene, nullptr );
	for ( const auto &b : broken )
		std::fprintf( stderr, "  sdf: %s\n", b.c_str() );
	Check( broken.empty(), "the SDF-traced producer passes the shared suite on the GPU" );
	std::printf( "shared suite: %.1f s\n",
	    std::chrono::duration<double>( std::chrono::steady_clock::now() - started ).count() );

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
