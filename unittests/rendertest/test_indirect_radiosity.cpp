//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The precomputed radiosity solver and producer (RFC 0011 G4)
//          against analytic values and the independent Python reference
//          (tools/quality/radiosity_transfer.py, states.txt):
//
//  - Furnace: from a cold start (no indirect light) the absolute solve of the
//    contract enclosure (albedo 0.6, direct 0.3) reaches the infinite-bounce
//    value 0.75 (indirect 0.45) within 2% in the producer's declared
//    convergenceFrames; the one-bounce defect settles at 0.48 and fails it.
//  - Each switched state's converged patch light and probe light match the
//    Python reference solve.
//  - The producer publishes the bake exactly at the baked scalars, responds
//    to a light style through the light set, returns to the bake when the
//    style does, and does no work in a still scene.
//
//===========================================================================//

#include "render/indirect_radiosity.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
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

std::vector<unsigned char> Load( const char *path )
{
	std::ifstream file( path, std::ios::binary );
	return std::vector<unsigned char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

// Probes of the contract that gather patches (its lit side).
std::vector<uint32_t> LitProbes( const Transfer &transfer )
{
	const auto view = transfer.View();
	std::vector<uint32_t> lit;
	for ( uint32_t i = 0; i < transfer.layout.probeCount; ++i )
		if ( view.GatherBegin( i ) != view.GatherEnd( i ) )
			lit.push_back( i );
	return lit;
}

float MeanProbe( const RadiositySolver &solver, const std::vector<uint32_t> &probes, bool indirect )
{
	double sum = 0.0;
	for ( uint32_t i : probes )
	{
		const float *texels = indirect ? solver.ProbeIndirect( i ) : solver.ProbeTotal( i );
		for ( uint32_t t = 0; t < RadiositySolver::kTexels; ++t )
			sum += texels[t * 3];
	}
	return float( sum / ( double( probes.size() ) * RadiositySolver::kTexels ) );
}

// Frames (updates) until the lit probes' indirect light is within 2% of the
// furnace's 0.45, or 0 if it never is within `limit`.
uint32_t FurnaceFrames( const std::shared_ptr<const Transfer> &transfer,
    const RadiosityOptions &options, uint32_t limit, float *settled )
{
	RadiositySolver solver;
	// Absolute: X is measured from zero light, starting cold.
	solver.Init( transfer, std::vector<float>( transfer->layout.sourceCount, 0.0f ), options );
	solver.SetScalars( std::vector<float>( transfer->layout.sourceCount, 1.0f ) );
	const std::vector<uint32_t> lit = LitProbes( *transfer );
	uint32_t reached = 0;
	for ( uint32_t frame = 1; frame <= limit; ++frame )
	{
		solver.Update( nullptr );
		*settled = MeanProbe( solver, lit, true );
		if ( !reached && std::fabs( *settled - 0.45f ) <= 0.02f * 0.45f )
			reached = frame;
	}
	return reached;
}

void Furnace( const std::shared_ptr<const Transfer> &transfer )
{
	const uint32_t declared = RadiosityProducer().Caps().convergenceFrames;
	float settled = 0.0f;
	const uint32_t frames = FurnaceFrames( transfer, RadiosityOptions(), declared * 4, &settled );
	std::fprintf(
	    stderr, "furnace: within 2%% after %u frame(s); settled indirect %.5f\n", frames, settled );
	Check( frames >= 1 && frames <= declared,
	    "the furnace reaches the infinite-bounce indirect light (0.45) within 2% in the declared " +
	        std::to_string( declared ) + " frames" );
	Check( std::fabs( settled - 0.45f ) < 1e-4f, "the furnace settles at 0.45" );

	RadiosityOptions oneBounce;
	oneBounce.oneBounce = true;
	const uint32_t never = FurnaceFrames( transfer, oneBounce, declared * 4, &settled );
	std::fprintf( stderr, "furnace (one-bounce defect): settled indirect %.5f\n", settled );
	Check( never == 0 && std::fabs( settled - 0.6f * 0.48f ) < 1e-3f,
	    "the one-bounce defect settles at 0.288 and never meets the furnace tolerance" );

	// More iterations per update converge in fewer frames: the frame count
	// follows from the declared iterations, not luck.
	RadiosityOptions fast;
	fast.iterationsPerUpdate = 2;
	const uint32_t fastFrames = FurnaceFrames( transfer, fast, declared * 4, &settled );
	Check( fastFrames >= 1 && fastFrames < frames, "more iterations per update take fewer frames" );
}

void MatchesPython( const std::shared_ptr<const Transfer> &transfer )
{
	std::ifstream states( "quality/fixtures/gi/rtrn/states.txt" );
	std::string line;
	int compared = 0;
	const std::vector<uint32_t> lit = LitProbes( *transfer );
	while ( std::getline( states, line ) )
	{
		std::istringstream fields( line );
		std::string name;
		std::vector<float> scalars( transfer->layout.sourceCount );
		float total = 0.0f, indirect = 0.0f;
		fields >> name;
		for ( float &scalar : scalars )
			fields >> scalar;
		fields >> total >> indirect;
		RadiositySolver solver;
		RadiosityOptions converge;
		converge.iterationsPerUpdate = 200;
		converge.convergedChange = 1e-7f;
		solver.Init( transfer, std::vector<float>( scalars.size(), 0.0f ), converge );
		solver.SetScalars( scalars );
		while ( solver.Update( nullptr ) )
			;
		bool patches = true;
		for ( uint32_t p = 0; p < transfer->layout.patchCount; ++p )
		{
			float expected = 0.0f;
			fields >> expected;
			patches = patches && std::fabs( solver.PatchChange( p )[0] - expected ) <=
			                         1e-5f + 1e-5f * std::fabs( expected );
		}
		Check( patches, "state " + name + ": every patch's light matches the Python solve" );
		const float cTotal = MeanProbe( solver, lit, false );
		const float cIndirect = MeanProbe( solver, lit, true );
		std::fprintf( stderr, "state %s: probe total %.6f (Python %.6f), indirect %.6f (%.6f)\n",
		    name.c_str(), cTotal, total, cIndirect, indirect );
		Check( std::fabs( cTotal - total ) < 1e-5f && std::fabs( cIndirect - indirect ) < 1e-5f,
		    "state " + name + ": the lit probes' light matches the Python reference" );
		++compared;
	}
	Check( compared == 4, "the four Python reference states were read" );
}

light_set::Snapshot Styles( float lamp )
{
	light_set::Snapshot lights;
	lights.styleScalars.assign( 64, 1.0f );
	lights.styleScalars[33] = lamp; // the contract's "Lamp"
	return lights;
}

void Producer(
    const std::shared_ptr<const Volume> &seed, const std::shared_ptr<const Transfer> &transfer )
{
	RadiosityProducer producer;
	IndirectScene scene;
	scene.baked = seed;
	scene.transfer = transfer;
	scene.policy = indirect_policy::Policy::BakedPlusDelta;
	struct NoResources final : IResourceTracker
	{
		uint64_t Acquire( size_t ) override { return 1; }
		void Release( uint64_t, uint64_t ) override {}
		uint64_t SubmittedSerial() const override { return 0; }
		uint64_t CompletedSerial() const override { return 0; }
	} resources;
	Check( bool( producer.Begin( scene, PublishedVolume{ 0, seed }, resources ) ),
	    "Begin with the transfer and its volume" );
	Check( producer.Published() && producer.Published()->volume->bytes == seed->bytes,
	    "at the baked scalars the first publication is the bake, byte for byte" );

	IndirectScene mismatched = scene;
	auto other = std::make_shared<Volume>( *seed );
	other->bytes[other->layout.atlasOffset +
	             ( uint64_t( other->layout.grids[0].stateOrigin[1] ) * other->layout.atlasWidth +
	                 other->layout.grids[0].stateOrigin[0] ) *
	                 8] ^= 1;
	mismatched.baked = other;
	RadiosityProducer rejected;
	const auto begun = rejected.Begin( mismatched, PublishedVolume{}, resources );
	Check( !begun && begun.Error() == IndirectError::MissingSceneData,
	    "Begin rejects a transfer baked for another volume" );

	const auto run = [&]( const light_set::Snapshot &lights, uint32_t frames )
	{
		uint32_t work = 0;
		for ( uint32_t frame = 0; frame < frames; ++frame )
		{
			FrameWork step;
			producer.Schedule( step, lights );
			work += uint32_t( step.jobs.size() );
			for ( auto &job : step.jobs )
				job();
		}
		return work;
	};
	const uint64_t stillEpoch = producer.Published()->epoch;
	Check( run( Styles( 1.0f ), 10 ) == 0 && producer.Published()->epoch == stillEpoch,
	    "a still scene schedules no work and publishes nothing new" );

	// The lamp off through its light style (33): converged within the
	// declared frames to the Python reference (lamp-off: indirect 0.3).
	const uint32_t declared = producer.Caps().convergenceFrames;
	run( Styles( 0.0f ), declared );
	const Volume &off = *producer.Published()->volume;
	const float indirect = off.MeanIrradiance( 1 ) / seed->MeanIrradiance( 1 ) * 0.45f;
	std::fprintf( stderr, "lamp off: lit indirect %.5f (reference 0.3)\n", indirect );
	Check( std::fabs( indirect - 0.3f ) < 0.02f * 0.3f,
	    "the lamp's style turned off converges to the reference within the declared frames" );
	Check(
	    run( Styles( 0.0f ), 2 * declared ) == 0, "once converged, the producer stops computing" );

	// And back on: the publication returns to the bake.
	run( Styles( 1.0f ), declared );
	const Volume &back = *producer.Published()->volume;
	Check( std::fabs( back.MeanIrradiance( 0 ) - seed->MeanIrradiance( 0 ) ) <
	               1e-3f * seed->MeanIrradiance( 0 ) &&
	           std::fabs( back.MeanIrradiance( 1 ) - seed->MeanIrradiance( 1 ) ) <
	               1e-3f * seed->MeanIrradiance( 1 ),
	    "the lamp back on returns the publication to the bake" );
	Check( back.SameTopology( *seed ), "publications keep the bake's topology" );
	(void)producer.End();
}

} // namespace

int main()
{
	const auto seed = Volume::FromBytes( Load( "quality/fixtures/gi/prbv/contract.prbv" ) );
	Check( seed != nullptr, "the contract volume loads" );
	if ( !seed )
		return testing::ReportConformance( g_checks, g_failures );
	const auto transfer =
	    Transfer::FromBytes( Load( "quality/fixtures/gi/rtrn/contract.rtrn" ), *seed );
	Check( transfer != nullptr, "the contract transfer loads with its volume" );
	if ( !transfer )
		return testing::ReportConformance( g_checks, g_failures );
	Furnace( transfer );
	MatchesPython( transfer );
	Producer( seed, transfer );
	return testing::ReportConformance( g_checks, g_failures );
}
