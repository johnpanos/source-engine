//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: particles.operator-strength.v1 - an operator's strength in one
//          particle system instance (public/particles/particle_operator_strength.h,
//          the rule CParticleCollection::CheckIfOperatorShouldRun applies).
//          Hand-computed Portal 2 / CS:GO cases for the per-instance time
//          offset, time scale and strength scale; bitwise equality with the
//          fade-only rule when the modulation fields are absent; and equal
//          results from the serial and pooled job-graph batches the client's
//          particle simulation runs (r_particle_job_graph 1 and 2). The
//          sensitivity build replaces the rule with the fade-only one, which
//          the hand-computed cases must reject.
//
//===========================================================================//

#include "particles/particle_operator_strength.h"
#include "jobsystem/parallel_batch.h"
#include "jobsystem/worker_backend.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <thread>
#include <vector>

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;
#ifdef PARTICLE_OPERATOR_STRENGTH_SEEDED_NO_MODULATION
constexpr bool kSeeded = true;
#else
constexpr bool kSeeded = false;
#endif
unsigned long g_rejected = 0;

void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		if ( kSeeded )
		{
			++g_rejected;
			std::fprintf( stderr, "seeded defect detected: %s\n", description );
			return;
		}
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

bool SameBits( float a, float b )
{
	return std::memcmp( &a, &b, sizeof( a ) ) == 0;
}

// The particle library's random stream: a shared constant table indexed by the
// system instance's seed plus the sample id (CParticleCollection::RandomFloat(
// int, float, float )). The table here holds exact binary fractions so the
// cases below are computed by hand.
constexpr int kRandomMask = 4095; // RANDOM_FLOAT_MASK
float g_randomFloats[kRandomMask + 1];

void InitRandomTable()
{
	for ( int i = 0; i <= kRandomMask; ++i )
		g_randomFloats[i] = float( i % 17 ) / 16.0f; // 0, 1/16, ..., 1
}

struct Instance
{
	int m_nRandomSeed;
	mutable int m_nDraws = 0;

	float RandomFloat( int nSampleId, float flMin, float flMax ) const
	{
		++m_nDraws;
		float flRand = g_randomFloats[( m_nRandomSeed + nSampleId ) & kRandomMask];
		flRand *= ( flMax - flMin );
		flRand += flMin;
		return flRand;
	}
};

// The operator unpack defaults (BEGIN_PARTICLE_OPERATOR_UNPACK).
ParticleOperatorModulation_t Defaults()
{
	ParticleOperatorModulation_t op;
	op.m_flOpStartFadeInTime = 0.0f;
	op.m_flOpEndFadeInTime = 0.0f;
	op.m_flOpStartFadeOutTime = 0.0f;
	op.m_flOpEndFadeOutTime = 0.0f;
	op.m_flOpFadeOscillatePeriod = 0.0f;
	op.m_nOpTimeOffsetSeed = 0;
	op.m_flOpTimeOffsetMin = 0.0f;
	op.m_flOpTimeOffsetMax = 0.0f;
	op.m_nOpTimeScaleSeed = 0;
	op.m_flOpTimeScaleMin = 1.0f;
	op.m_flOpTimeScaleMax = 1.0f;
	op.m_nOpStrengthScaleSeed = 0;
	op.m_flOpStrengthMinScale = 1.0f;
	op.m_flOpStrengthMaxScale = 1.0f;
	op.m_nOpEndCapState = -1;
	return op;
}

// The rule before this change, verbatim from particles.cpp at 71d986fa
// (mathlib's FLerp and tier0's min/max macros spelled out): end cap state, fade
// oscillation and the fade window; no modulation. The oracle for the
// absent-field cases, and the seeded defect.
float LegacyFLerp( float f1, float f2, float i1, float i2, float x )
{
	return f1 + ( f2 - f1 ) * ( x - i1 ) / ( i2 - i1 );
}

#define LEGACY_MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define LEGACY_MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

float LegacyFadeInOut( float flFadeInStart, float flFadeInEnd, float flFadeOutStart,
    float flFadeOutEnd, float flCurTime )
{
	if ( flFadeInStart > flCurTime ) // started yet?
		return 0.0;

	if ( ( flFadeOutEnd > 0. ) && ( flFadeOutEnd < flCurTime ) ) // timed out?
		return 0.;

	// handle out of order cases
	flFadeInEnd = LEGACY_MAX( flFadeInEnd, flFadeInStart );
	flFadeOutStart = LEGACY_MAX( flFadeOutStart, flFadeInEnd );
	flFadeOutEnd = LEGACY_MAX( flFadeOutEnd, flFadeOutStart );

	float flStrength = 1.0;
	if ( ( flFadeInEnd > flCurTime ) && ( flFadeInEnd > flFadeInStart ) )
		flStrength =
		    LEGACY_MIN( flStrength, LegacyFLerp( 0, 1, flFadeInStart, flFadeInEnd, flCurTime ) );

	if ( ( flCurTime > flFadeOutStart ) && ( flFadeOutEnd > flFadeOutStart ) )
		flStrength =
		    LEGACY_MIN( flStrength, LegacyFLerp( 0, 1, flFadeOutEnd, flFadeOutStart, flCurTime ) );

	return flStrength;
}

float FadeOnly( const ParticleOperatorModulation_t &op, bool bInEndCap, float flCurTime )
{
	if ( op.m_nOpEndCapState != -1 )
	{
		if ( bInEndCap != ( op.m_nOpEndCapState == 1 ) )
			return 0.0f;
	}
	float flTime = flCurTime;
	if ( op.m_flOpFadeOscillatePeriod > 0.0 )
		flTime = fmod( flCurTime * ( 1.0 / op.m_flOpFadeOscillatePeriod ), 1.0 );
	return LegacyFadeInOut( op.m_flOpStartFadeInTime, op.m_flOpEndFadeInTime,
	    op.m_flOpStartFadeOutTime, op.m_flOpEndFadeOutTime, flTime );
}

float Strength( const ParticleOperatorModulation_t &op, const Instance &instance, float flCurTime,
    bool bInEndCap = false )
{
#ifdef PARTICLE_OPERATOR_STRENGTH_SEEDED_NO_MODULATION
	(void)instance;
	return FadeOnly( op, bInEndCap, flCurTime );
#else
	return ParticleOperatorStrength::Compute( op, bInEndCap, flCurTime,
	    [&instance]( int nSampleId, float flMin, float flMax )
	    {
		    return instance.RandomFloat( nSampleId, flMin, flMax );
	    } );
#endif
}

void CheckStrengthScale()
{
	// Draw index ( 100 + 3 ) & 4095 = 103: 103 % 17 = 1 -> 1/16. Range [0.5, 2.1] would not
	// be exact; [0.5, 4.5] gives 0.5 + 4 * 1/16 = 0.75.
	Instance instance{ 100 };
	ParticleOperatorModulation_t op = Defaults();
	op.m_nOpStrengthScaleSeed = 3;
	op.m_flOpStrengthMinScale = 0.5f;
	op.m_flOpStrengthMaxScale = 4.5f;
	Check( Strength( op, instance, 1.0f ) == 0.75f,
	    "strength scale: the draw in [min, max] multiplies a full-strength operator (0.75)" );

	// Retail squares the fade strength: fade 0.5 (half way through the fade-out)
	// times max( 0, 0.5 * draw ). min = max = 0.5 -> 0.5 * 0.25 = 0.125.
	op = Defaults();
	op.m_flOpStartFadeOutTime = 0.5f;
	op.m_flOpEndFadeOutTime = 1.0f;
	op.m_nOpStrengthScaleSeed = 13142;
	op.m_flOpStrengthMinScale = 0.5f;
	op.m_flOpStrengthMaxScale = 0.5f;
	Check( Strength( op, instance, 0.75f ) == 0.125f,
	    "strength scale during a fade: fade * max( 0, fade * draw ), as retail (0.125)" );

	// A negative draw clamps the multiplier at 0, so the operator does not run.
	op = Defaults();
	op.m_nOpStrengthScaleSeed = 7;
	op.m_flOpStrengthMinScale = -2.0f;
	op.m_flOpStrengthMaxScale = -1.0f;
	Check( Strength( op, instance, 1.0f ) == 0.0f, "a negative strength draw stops the operator" );

	// Each instance draws from its own stream: seed 100 + 3 -> 1/16, seed 101 + 3 -> 2/16.
	op = Defaults();
	op.m_nOpStrengthScaleSeed = 3;
	op.m_flOpStrengthMinScale = 0.0f;
	op.m_flOpStrengthMaxScale = 16.0f;
	Instance other{ 101 };
	Check( Strength( op, instance, 1.0f ) == 1.0f && Strength( op, other, 1.0f ) == 2.0f,
	    "two instances of one system draw different strengths (1 and 2)" );
	Check( Strength( op, instance, 1.0f ) == Strength( op, instance, 7.0f ),
	    "an instance keeps its strength draw for its whole life" );
}

void CheckTimeOffset()
{
	// Offset draw: ( 8 + 0 ) % 17 = 8 -> 0.5 over [0, 1] = 0.5.
	Instance instance{ 8 };
	ParticleOperatorModulation_t op = Defaults();
	op.m_flOpStartFadeInTime = 1.0f;
	op.m_flOpEndFadeInTime = 2.0f;
	op.m_nOpTimeOffsetSeed = 4096; // ( 8 + 4096 ) & 4095 = 8
	op.m_flOpTimeOffsetMin = 0.0f;
	op.m_flOpTimeOffsetMax = 1.0f;
	Check( Strength( op, instance, 1.0f ) == 0.5f,
	    "time offset: the clock runs 0.5 ahead, half way up the fade-in (0.5)" );

	op.m_flOpTimeOffsetMin = -5.0f;
	op.m_flOpTimeOffsetMax = -5.0f;
	op.m_flOpStartFadeInTime = 0.0f;
	op.m_flOpEndFadeInTime = 0.0f;
	op.m_flOpStartFadeOutTime = 0.0f;
	op.m_flOpEndFadeOutTime = 0.0f;
	Check( Strength( op, instance, 1.0f ) == 1.0f,
	    "a time offset past the start clamps the clock at 0, where the operator runs" );

	// A fade oscillation replaces the offset clock with the unmodulated phase.
	op = Defaults();
	op.m_flOpFadeOscillatePeriod = 2.0f;
	op.m_flOpEndFadeInTime = 1.0f;
	op.m_nOpTimeOffsetSeed = 4096;
	op.m_flOpTimeOffsetMin = 0.25f;
	op.m_flOpTimeOffsetMax = 0.25f;
	Check( Strength( op, instance, 1.0f ) == 0.5f,
	    "fade oscillation uses the unmodulated clock's phase (0.5)" );
}

void CheckTimeScale()
{
	// Scale draw min = max = 0.5: the clock runs 1 / 0.5 = 2x past the fade-in start.
	Instance instance{ 0 };
	ParticleOperatorModulation_t op = Defaults();
	op.m_flOpStartFadeInTime = 1.0f;
	op.m_flOpEndFadeInTime = 3.0f;
	op.m_nOpTimeScaleSeed = 1;
	op.m_flOpTimeScaleMin = 0.5f;
	op.m_flOpTimeScaleMax = 0.5f;
	Check( Strength( op, instance, 1.5f ) == 0.5f,
	    "time scale 0.5: the fade-in runs twice as fast (0.5 at t = 1.5, not 0.25)" );
	Check(
	    Strength( op, instance, 0.5f ) == 0.0f, "time scale leaves time before the fade-in start" );

	// Draws under 0.0001 are clamped, so time runs 10000x.
	op.m_flOpTimeScaleMin = 0.0f;
	op.m_flOpTimeScaleMax = 0.0f;
	op.m_flOpEndFadeInTime = 2.0f;
	op.m_flOpStartFadeOutTime = 2.0f;
	op.m_flOpEndFadeOutTime = 1001.0f;
	// time = 1 + 10000 * 0.0625 = 626: into the fade-out, ( 626 - 1001 ) / ( 2 - 1001 ).
	float flTime = 1.0f + float( 1.0 / .0001 ) * ( 1.0625f - 1.0f );
	float flExpected = 0.0f + ( 1.0f - 0.0f ) * ( flTime - 1001.0f ) / ( 2.0f - 1001.0f );
	Check( SameBits( Strength( op, instance, 1.0625f ), flExpected ),
	    "a time scale draw under 0.0001 is clamped to 0.0001" );
}

void CheckEndCap()
{
	Instance instance{ 0 };
	ParticleOperatorModulation_t op = Defaults();
	op.m_nOpEndCapState = 1;
	Check( Strength( op, instance, 1.0f, false ) == 0.0f &&
	           Strength( op, instance, 1.0f, true ) == 1.0f,
	    "end cap state 1 runs only in the end cap" );
	op.m_nOpEndCapState = 0;
	Check( Strength( op, instance, 1.0f, false ) == 1.0f &&
	           Strength( op, instance, 1.0f, true ) == 0.0f,
	    "end cap state 0 runs only while the system plays" );
}

// Absent fields: every combination of fade window, oscillation, end cap state
// and time gives the fade-only rule's exact bits and draws nothing.
void CheckAbsentFieldsAreNoOps()
{
	static const float kFades[] = { 0.0f, 0.1f, 0.25f, 0.4f, 0.8f, 1.0f, 2.5f };
	static const float kPeriods[] = { 0.0f, 0.3f, 1.0f };
	static const int kEndCap[] = { -1, 0, 1 };
	static const float kTimes[] = { -1.0f, 0.0f, 0.01f, 0.05f, 0.1f, 0.2f, 0.3f, 0.45f, 0.5f, 0.7f,
	    0.99f, 1.0f, 1.7f, 3.0f, 100.0f };
	unsigned long compared = 0;
	unsigned long mismatches = 0;
	Instance instance{ 12345 };
	for ( float a : kFades )
		for ( float b : kFades )
			for ( float c : kFades )
				for ( float d : kFades )
					for ( float period : kPeriods )
						for ( int endCap : kEndCap )
						{
							ParticleOperatorModulation_t op = Defaults();
							op.m_flOpStartFadeInTime = a;
							op.m_flOpEndFadeInTime = b;
							op.m_flOpStartFadeOutTime = c;
							op.m_flOpEndFadeOutTime = d;
							op.m_flOpFadeOscillatePeriod = period;
							op.m_nOpEndCapState = endCap;
							for ( bool bInEndCap : { false, true } )
								for ( float t : kTimes )
								{
									++compared;
									if ( !SameBits( Strength( op, instance, t, bInEndCap ),
									         FadeOnly( op, bInEndCap, t ) ) )
										++mismatches;
								}
						}
	std::printf( "absent fields: %lu cases, %lu mismatches\n", compared, mismatches );
	Check( mismatches == 0, "absent modulation fields give the fade-only rule's exact bits" );
	Check( instance.m_nDraws == 0, "absent modulation fields draw nothing from the random stream" );

	// Nonzero ranges with a zero seed draw nothing either (the seed enables each term).
	ParticleOperatorModulation_t op = Defaults();
	op.m_flOpTimeOffsetMin = 3.0f;
	op.m_flOpTimeOffsetMax = 4.0f;
	op.m_flOpTimeScaleMin = 0.25f;
	op.m_flOpStrengthMinScale = 0.0f;
	op.m_flOpStrengthMaxScale = 0.0f;
	op.m_flOpEndFadeInTime = 1.0f;
	Check( SameBits( Strength( op, instance, 0.5f ), FadeOnly( op, false, 0.5f ) ) &&
	           instance.m_nDraws == 0,
	    "ranges without a seed are ignored, as retail" );
}

//-----------------------------------------------------------------------------
// Serial and pooled batches: the client simulates each particle system on the
// engine pool (r_particle_job_graph 2) or in order (1). Each item here is one
// system instance with retail modulation (br_train_effects.pcf's seeded
// operators), simulated over 150 ticks.
//-----------------------------------------------------------------------------
class ThreadBackend final : public jobsystem::IWorkerBackend
{
public:
	explicit ThreadBackend( int workers ) : m_workers( workers ) {}

	void ParallelFor( int count, const std::function<void( int )> &body ) override
	{
		std::vector<std::thread> threads;
		for ( int index = 1; index < count; ++index )
			threads.emplace_back( body, index );
		if ( count > 0 )
			body( 0 );
		for ( auto &thread : threads )
			thread.join();
	}

	int WorkerCount() const override { return m_workers; }

private:
	int m_workers;
};

constexpr unsigned kInstances = 257;
constexpr int kTicks = 150;

std::vector<ParticleOperatorModulation_t> RetailOperators()
{
	std::vector<ParticleOperatorModulation_t> ops;
	ParticleOperatorModulation_t op = Defaults();
	op.m_nOpStrengthScaleSeed = 1241; // Movement Basic
	op.m_flOpStartFadeOutTime = 1.0f;
	ops.push_back( op );
	op = Defaults();
	op.m_nOpStrengthScaleSeed = 13142; // Color Fade
	op.m_flOpStrengthMinScale = 0.5f;
	op.m_flOpStrengthMaxScale = 0.5f;
	op.m_flOpEndFadeInTime = 0.05f;
	op.m_flOpStartFadeOutTime = 0.4f;
	op.m_flOpEndFadeOutTime = 0.8f;
	ops.push_back( op );
	op = Defaults();
	op.m_nOpStrengthScaleSeed = 1345; // emit_continuously
	op.m_flOpStrengthMinScale = 0.25f;
	op.m_flOpStrengthMaxScale = 1.5f;
	op.m_flOpStartFadeOutTime = 0.02f;
	op.m_flOpEndFadeOutTime = 0.18f;
	ops.push_back( op );
	op = Defaults();
	op.m_nOpTimeScaleSeed = 1; // the one retail time-scale operator shape
	op.m_flOpTimeScaleMin = 0.1f;
	op.m_flOpEndFadeInTime = 0.5f;
	op.m_nOpTimeOffsetSeed = 77;
	op.m_flOpTimeOffsetMin = 0.1f;
	op.m_flOpTimeOffsetMax = 1.0f;
	ops.push_back( op );
	return ops;
}

struct BatchContext
{
	const std::vector<ParticleOperatorModulation_t> *m_pOps;
	std::vector<float> m_Strengths;

	static void Process( void *pContext, unsigned nItem )
	{
		BatchContext *pThis = static_cast<BatchContext *>( pContext );
		// One seed per system instance, scattered over the table (and small enough
		// that seed + sample id cannot overflow).
		Instance instance{ int( ( 2654435761u * ( nItem + 1 ) ) & 0xFFFFFu ) };
		const std::vector<ParticleOperatorModulation_t> &ops = *pThis->m_pOps;
		float *pOut = &pThis->m_Strengths[size_t( nItem ) * kTicks * ops.size()];
		for ( int nTick = 0; nTick < kTicks; ++nTick )
		{
			float flCurTime = float( nTick ) * ( 1.0f / 60.0f );
			for ( const ParticleOperatorModulation_t &op : ops )
				*pOut++ = Strength( op, instance, flCurTime );
		}
	}
};

void CheckSerialEqualsPooled()
{
	std::vector<ParticleOperatorModulation_t> ops = RetailOperators();
	const size_t nValues = size_t( kInstances ) * kTicks * ops.size();
	BatchContext serial{ &ops, std::vector<float>( nValues ) };
	BatchContext pooled{ &ops, std::vector<float>( nValues ) };
	BatchContext reversed{ &ops, std::vector<float>( nValues ) };

	jobsystem::BatchDesc desc;
	desc.name = "particles.operator-strength";
	desc.count = kInstances;
	desc.maxParticipants = 5; // four workers and the caller
	desc.process = &BatchContext::Process;
	ThreadBackend backend( 4 );

	desc.context = &serial;
	Check( jobsystem::ExecuteParallelBatch( desc, &backend, jobsystem::BatchMode::Serial ),
	    "the serial batch runs" );
	desc.context = &pooled;
	Check( jobsystem::ExecuteParallelBatch( desc, &backend, jobsystem::BatchMode::Parallel ),
	    "the pooled batch runs" );
	for ( unsigned n = kInstances; n-- > 0; )
		BatchContext::Process( &reversed, n );

	Check( std::memcmp( serial.m_Strengths.data(), pooled.m_Strengths.data(),
	           nValues * sizeof( float ) ) == 0,
	    "pooled strengths equal serial strengths bit for bit" );
	Check( std::memcmp( serial.m_Strengths.data(), reversed.m_Strengths.data(),
	           nValues * sizeof( float ) ) == 0,
	    "strengths do not depend on the order instances are simulated in" );

	// The modulation is live in this workload: instances differ from each other.
	unsigned long nDiffering = 0;
	const size_t nPerInstance = size_t( kTicks ) * ops.size();
	for ( unsigned n = 1; n < kInstances; ++n )
		if ( std::memcmp( &serial.m_Strengths[0], &serial.m_Strengths[n * nPerInstance],
		         nPerInstance * sizeof( float ) ) != 0 )
			++nDiffering;
	std::printf( "serial/pooled: %u instances x %d ticks x %zu operators; %lu of %u instances "
	             "differ from the first\n",
	    kInstances, kTicks, ops.size(), nDiffering, kInstances - 1 );
	Check(
	    nDiffering > kInstances / 2, "system instances no longer run their operators in lockstep" );
}

} // namespace

int main()
{
	InitRandomTable();
	CheckStrengthScale();
	CheckTimeOffset();
	CheckTimeScale();
	CheckEndCap();
	CheckAbsentFieldsAreNoOps();
	CheckSerialEqualsPooled();

	if ( kSeeded )
	{
		std::fprintf( stderr, "%lu seeded rejection(s)\n", g_rejected );
		if ( g_rejected == 0 )
		{
			std::fprintf( stderr, "FAIL: the seeded defect was not detected\n" );
			return testing::ReportConformance( g_checks, g_checks );
		}
		return testing::ReportConformance( g_checks, 0 );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
