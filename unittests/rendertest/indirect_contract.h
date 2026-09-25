//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shared producer suite of render.indirect-light.v1 (RFC 0011):
//          RunContract judges any IProducer against the contract's
//          obligations over a Scenario (the seed, the scene data, and the
//          geometry change with its independent oracle). The headless suite
//          (test_indirect_light.cpp) runs the CPU producers and the bad
//          producers; the native GPU suite runs the SDF-traced producer
//          (shaderapivulkantest/test_indirect_sdf.cpp) on the same code.
//
//===========================================================================//

#ifndef INDIRECT_CONTRACT_H
#define INDIRECT_CONTRACT_H

#include "render/indirect_light.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace indirect_contract
{
using namespace indirect_light;
using indirect_policy::Policy;

// A GPU timeline: frames complete kLag frames after submission unless
// stalled. A resource released before a submitted frame that may read it has
// completed is a violation.
class FakeGpu final : public IResourceTracker
{
public:
	static constexpr uint64_t kLag = 2;

	uint64_t Acquire( size_t bytes ) override
	{
		const uint64_t id = ++m_nextId;
		m_live[id] = bytes;
		return id;
	}
	void Release( uint64_t id, uint64_t afterSerial ) override
	{
		if ( m_lost.count( id ) )
			return; // the device took it; a late release is a no-op
		if ( !m_live.count( id ) )
		{
			m_violations.push_back( "release of an unknown or released resource" );
			return;
		}
		// Every frame submitted so far may read a producer's resource.
		if ( afterSerial < m_submitted )
			m_violations.push_back( "resource " + std::to_string( id ) + " freed after serial " +
			                        std::to_string( afterSerial ) + " while frame " +
			                        std::to_string( m_submitted ) + " may read it" );
		m_pending[id] = afterSerial;
	}
	uint64_t SubmittedSerial() const override { return m_submitted; }
	uint64_t CompletedSerial() const override { return m_completed; }

	void Submit()
	{
		++m_submitted;
		if ( !m_stalled && m_submitted > kLag )
			m_completed = m_submitted - kLag;
		Collect();
	}
	void Drain()
	{
		m_completed = m_submitted;
		Collect();
	}
	void Stall( bool stalled ) { m_stalled = stalled; }
	void Lose()
	{
		// Device loss: every resource is gone without waiting.
		for ( const auto &entry : m_live )
			m_lost.insert( entry.first );
		m_live.clear();
		m_pending.clear();
		m_completed = m_submitted;
	}
	size_t LiveCount() const { return m_live.size(); }
	const std::vector<std::string> &Violations() const { return m_violations; }
	void ClearViolations() { m_violations.clear(); }

private:
	void Collect()
	{
		for ( auto it = m_pending.begin(); it != m_pending.end(); )
		{
			if ( m_completed >= it->second )
			{
				m_live.erase( it->first );
				it = m_pending.erase( it );
			}
			else
				++it;
		}
	}
	uint64_t m_nextId = 0;
	uint64_t m_submitted = 0;
	uint64_t m_completed = 0;
	bool m_stalled = false;
	std::map<uint64_t, size_t> m_live;
	std::set<uint64_t> m_lost;
	std::map<uint64_t, uint64_t> m_pending;
	std::vector<std::string> m_violations;
};


std::vector<unsigned char> LoadFile( const char *path )
{
	std::vector<unsigned char> bytes;
	if ( std::FILE *file = std::fopen( path, "rb" ) )
	{
		unsigned char block[4096];
		size_t read;
		while ( ( read = std::fread( block, 1, sizeof( block ), file ) ) > 0 )
			bytes.insert( bytes.end(), block, block + read );
		std::fclose( file );
	}
	return bytes;
}

std::shared_ptr<const Volume> LoadSeed()
{
	return Volume::FromBytes( LoadFile( "quality/fixtures/gi/prbv/contract.prbv" ) );
}

// The seed's transfer: its lit side is a furnace enclosure (converged light
// 0.75, indirect 0.45; radiosity_transfer.fixture_contract).
std::shared_ptr<const Transfer> LoadTransfer( const Volume &seed )
{
	return Transfer::FromBytes( LoadFile( "quality/fixtures/gi/rtrn/contract.rtrn" ), seed );
}

// The suite's light change: every source at half its baked intensity.
constexpr float kLightScale = 0.5f;

// The scripted fake's world: a door that, opened (value 1), raises the
// indirect light by 20%. It converges linearly over convergenceFrames.
constexpr float kDoorGain = 0.2f;

float DirectLight( const Volume &volume )
{
	return volume.MeanIrradiance( 0 ) - volume.MeanIrradiance( 1 );
}

// The dark side of the seed's wall, with visibility.
float DarkSample( const Volume &volume )
{
	const float position[3] = { 56.0f, 16.0f, 16.0f };
	const float normal[3] = { 1.0f, 0.0f, 0.0f };
	float irradiance[3] = {};
	if ( !volume.View().Sample(
	         position, normal, mapcontainer::ProbeVolumeLayer::Total, true, irradiance ) )
		return -1.0f;
	return irradiance[0];
}

// What the suite runs a producer in.
struct Scenario
{
	std::shared_ptr<const Volume> seed;
	std::shared_ptr<const Transfer> transfer;
	std::shared_ptr<const SdfData> sdf;
	std::shared_ptr<const WorldGeometry> geometry; // for the ray-query producer
	gpu_compute::IGpuCompute *gpu = nullptr;
	// A real device's completion (null: the suite's fake timeline).
	IResourceTracker *resources = nullptr;
	// After each frame's jobs: a real device submits its queued work.
	std::function<void()> afterFrame;
	// The geometry change for producers claiming kGeometryMotion, and its
	// independent oracle: an empty string when the converged volume is right.
	std::vector<SceneChange> geometryChanges;
	std::vector<Proxy> geometryProxies;
	std::function<std::string( const Volume &volume, const Volume &seed, const ProducerCaps &caps )>
	    geometryOracle;
};

// The shared suite: returns the obligations a producer breaks.
std::vector<std::string> RunContract(
    const std::function<std::unique_ptr<IProducer>( const FakeGpu & )> &create,
    const Scenario &scenario, bool *destroyedEarly )
{
	std::vector<std::string> broken;
	const std::shared_ptr<const Volume> &seed = scenario.seed;
	const std::shared_ptr<const Transfer> &transfer = scenario.transfer;
	FakeGpu fake;
	// The resources a producer releases: a real device's, or the fake timeline.
	IResourceTracker &gpu = scenario.resources ? *scenario.resources : fake;
	const auto make = [&]
	{
		return create( fake );
	};
	const light_set::Snapshot lights;
	const ProducerCaps caps = make()->Caps();
	const float seedMean = seed->MeanIrradiance( 0 );
	const float seedDirect = DirectLight( *seed );
	const float seedDark = DarkSample( *seed );

	// Validation: missing scene data fails without side effects.
	{
		auto producer = make();
		IndirectScene scene;
		scene.gpu = scenario.gpu;
		const auto begun = producer->Begin( scene, PublishedVolume{}, gpu );
		if ( begun || begun.Error() != IndirectError::MissingSceneData || fake.LiveCount() )
			broken.push_back( "Begin without a probe volume must fail with missing-scene-data and "
			                  "leave nothing behind" );
		// An unclaimed policy is rejected.
		for ( Policy policy : { Policy::Baked, Policy::BakedPlusDelta, Policy::RuntimeIndirect } )
		{
			if ( caps.policies & PolicyBit( policy ) )
				continue;
			IndirectScene unsupported;
			unsupported.baked = seed;
			unsupported.transfer = transfer;
			unsupported.sdf = scenario.sdf;
			unsupported.geometry = scenario.geometry;
			unsupported.gpu = scenario.gpu;
			unsupported.policy = policy;
			auto other = make();
			const auto result = other->Begin( unsupported, PublishedVolume{ 0, seed }, gpu );
			if ( result || result.Error() != IndirectError::UnsupportedPolicy || fake.LiveCount() )
				broken.push_back( "an unclaimed policy must fail with unsupported-policy and leave "
				                  "nothing behind" );
		}
	}

	for ( Policy policy : { Policy::Baked, Policy::BakedPlusDelta, Policy::RuntimeIndirect } )
	{
		if ( !( caps.policies & PolicyBit( policy ) ) )
			continue;
		const std::string under = std::string( " (policy " ) +
		                          ( policy == Policy::Baked              ? "Baked"
		                              : policy == Policy::BakedPlusDelta ? "BakedPlusDelta"
		                                                                 : "RuntimeIndirect" ) +
		                          ")";
		auto producer = make();
		IndirectScene scene;
		scene.baked = seed;
		scene.transfer = transfer;
		scene.sdf = scenario.sdf;
		scene.geometry = scenario.geometry;
		scene.gpu = scenario.gpu;
		scene.policy = policy;
		if ( !producer->Begin( scene, PublishedVolume{ 0, seed }, gpu ) )
		{
			broken.push_back( "Begin with a valid scene fails" + under );
			continue;
		}
		uint64_t lastEpoch = 0;
		std::shared_ptr<const Volume> lastVolume;
		bool first = true;
		// A producer's warm-up (its Begin-time convergence) precedes the
		// changes; the light change and the geometry change are apart, each
		// judged convergenceFrames after it.
		const uint32_t lightFrame = caps.warmupFrames + 12;
		const uint32_t geometryFrame = lightFrame + caps.convergenceFrames + 10;
		const uint32_t frames = geometryFrame + caps.convergenceFrames + 8;
		const bool lightResponse = ( caps.responds & kLightIntensity ) != 0;
		const bool geometryResponse = ( caps.responds & kGeometryMotion ) != 0;
		double slowest = 0.0;
		for ( uint32_t frame = 1; frame <= frames; ++frame )
		{
			std::vector<SceneChange> changes;
			if ( frame == lightFrame )
				changes.push_back( { kLightIntensity, kEverySource, kLightScale } );
			if ( frame == geometryFrame )
				changes.insert( changes.end(), scenario.geometryChanges.begin(),
				    scenario.geometryChanges.end() );
			FrameWork work;
			work.frameSerial = gpu.SubmittedSerial() + 1;
			work.resources = &gpu;
			work.changes = std::span<const SceneChange>( changes );
			if ( frame >= geometryFrame )
				work.proxies = std::span<const Proxy>( scenario.geometryProxies );
			// Schedule runs while the GPU is stalled: it must not wait for it.
			fake.Stall( frame == 3 );
			const auto start = std::chrono::steady_clock::now();
			producer->Schedule( work, lights );
			slowest = std::max( slowest, std::chrono::duration<double, std::milli>(
			                                 std::chrono::steady_clock::now() - start )
			                                 .count() );
			for ( auto &job : work.jobs )
				job();
			fake.Submit();
			if ( scenario.afterFrame )
				scenario.afterFrame();
			const auto published = producer->Published();
			if ( !published )
				continue;
			const Volume &volume = *published->volume;
			if ( !volume.SameTopology( *seed ) )
				broken.push_back( "a published volume changes the scene's grid topology" + under );
			if ( first && volume.MeanIrradiance( 0 ) < ( 1.0f - caps.seedTolerance ) * seedMean )
				broken.push_back( "the first publication is darker than the seed" + under );
			first = false;
			if ( published->volume != lastVolume )
			{
				if ( published->epoch <= lastEpoch )
					broken.push_back( "a new publication reuses or lowers its epoch" + under );
				lastEpoch = published->epoch;
				lastVolume = published->volume;
			}
			if ( DarkSample( volume ) > seedDark + 0.02f )
				broken.push_back(
				    "a published volume leaks light through the seed's wall" + under );
			// Total minus indirect: the seed's direct light until the lights
			// change; the scaled direct light once a light-responding producer
			// has converged (and until the geometry changes).
			const float direct = DirectLight( volume );
			if ( frame < lightFrame &&
			     std::fabs( direct - seedDirect ) > 0.01f * seedDirect )
				broken.push_back( "total minus indirect departs from the seed's direct light (a "
				                  "double count)" +
				                  under );
			if ( frame >= lightFrame + caps.convergenceFrames && frame < geometryFrame )
			{
				const float expected = ( lightResponse ? kLightScale : 1.0f ) * seedDirect;
				if ( std::fabs( direct - expected ) >
				     std::max( 0.01f, caps.responseTolerance ) * seedDirect )
					broken.push_back( "total minus indirect departs from the seed's direct light (a "
					                  "double count)" +
					                  under );
			}
			if ( lightResponse && frame == lightFrame + caps.convergenceFrames + 2 )
			{
				const float expected = seed->MeanIrradiance( 1 ) * kLightScale;
				if ( std::fabs( volume.MeanIrradiance( 1 ) - expected ) >
				     caps.responseTolerance * expected )
					broken.push_back( "claims LightIntensity but its indirect light is not the "
					                  "halved lights' steady state within convergenceFrames" +
					                  under );
			}
			if ( geometryResponse && frame == geometryFrame + caps.convergenceFrames + 2 )
			{
				const std::string failure = scenario.geometryOracle
				                                ? scenario.geometryOracle( volume, *seed, caps )
				                                : std::string( "no geometry scenario" );
				if ( !failure.empty() )
					broken.push_back( "claims GeometryMotion but " + failure + under );
			}
		}
		fake.Stall( false );
		if ( !producer->Published() )
			broken.push_back( "never publishes" + under );
		if ( slowest > 5.0 )
			broken.push_back( "Schedule took " + std::to_string( slowest ) +
			                  " ms while the GPU was stalled: it waits for the GPU" + under );
		const RetireTicket ticket = producer->End();
		// Destroyed only once its ticket completes (the switcher's rule).
		while ( gpu.CompletedSerial() < ticket.afterSerial )
		{
			fake.Submit();
			if ( scenario.afterFrame )
				scenario.afterFrame();
		}
		producer.reset();
		fake.Drain();
		for ( const std::string &violation : fake.Violations() )
			broken.push_back( "lifetime: " + violation + under );
		fake.ClearViolations();
		if ( fake.LiveCount() )
			broken.push_back( "resources remain after the ticket completed" + under );
	}
	if ( destroyedEarly && *destroyedEarly )
		broken.push_back( "destroyed before its ticket completed" );
	// Deduplicate for the report.
	std::sort( broken.begin(), broken.end() );
	broken.erase( std::unique( broken.begin(), broken.end() ), broken.end() );
	return broken;
}

} // namespace indirect_contract

#endif // INDIRECT_CONTRACT_H
