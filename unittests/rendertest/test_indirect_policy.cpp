//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.indirect-policy.v1 - the policy table, availability, and the
//          furnace composition: every policy reproduces the analytic total
//          light, and the seeded double count (total layer plus producer
//          indirect) fails it.
//
//===========================================================================//

#include "render/indirect_policy.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <initializer_list>

namespace
{
using namespace indirect_policy;

unsigned long g_checks = 0;
unsigned long g_failures = 0;

[[maybe_unused]] void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

// The furnace fixture (quality/fixtures/gi/furnace): rho 0.6, Le 0.3.
constexpr float kDirect = 0.3f;
constexpr float kIndirect = 0.45f;
constexpr float kTotal = 0.75f;

// The composition under test; the seeded build reads the total layer under
// the RuntimeIndirect variant (BakedPlusDelta data read by the wrong variant).
float Composed( Policy policy, float producerIndirect, float producerDelta )
{
#ifdef INDIRECT_POLICY_SEEDED_DOUBLE_COUNT
	if ( policy == Policy::RuntimeIndirect )
		return kTotal + producerIndirect;
#endif
	return WorldDiffuseLight( policy, kTotal, kDirect, producerIndirect, producerDelta );
}

bool FurnaceHolds( float light )
{
	return std::fabs( light - kTotal ) <= 1e-4f;
}

} // namespace

int main()
{
#ifdef INDIRECT_POLICY_SEEDED_DOUBLE_COUNT
	// Sensitivity: the furnace oracle must reject the double count.
	const bool detected = !FurnaceHolds( Composed( Policy::RuntimeIndirect, kIndirect, 0.0f ) );
	std::fprintf( stderr, "seeded double count %s\n", detected ? "detected" : "missed" );
	return testing::ReportConformance( 1, detected ? 0 : 1 );
#else
	static_assert( Select( Policy::Baked ).worldLayer == WorldLightmapLayer::Total &&
	                   !Select( Policy::Baked ).worldAddsProducerIndirect,
	    "the table is usable in constant expressions" );
	Check( Select( Policy::Baked ).worldLayer == WorldLightmapLayer::Total &&
	           !Select( Policy::Baked ).worldAddsProducerIndirect &&
	           !Select( Policy::Baked ).worldAddsProducerDelta &&
	           !Select( Policy::Baked ).modelsReadRuntimeVolume,
	    "Baked: the world reads the total layer and nothing else; models the baked volume" );
	Check( Select( Policy::BakedPlusDelta ).worldLayer == WorldLightmapLayer::Total &&
	           !Select( Policy::BakedPlusDelta ).worldAddsProducerIndirect &&
	           Select( Policy::BakedPlusDelta ).worldAddsProducerDelta &&
	           !Select( Policy::BakedPlusDelta ).modelsReadRuntimeVolume,
	    "BakedPlusDelta: total plus the producer's delta; models the baked volume + delta" );
	Check( Select( Policy::RuntimeIndirect ).worldLayer == WorldLightmapLayer::Direct &&
	           Select( Policy::RuntimeIndirect ).worldAddsProducerIndirect &&
	           !Select( Policy::RuntimeIndirect ).worldAddsProducerDelta &&
	           Select( Policy::RuntimeIndirect ).modelsReadRuntimeVolume,
	    "RuntimeIndirect: the direct layer plus the producer's indirect; models the runtime "
	    "volume" );
	for ( Policy policy : { Policy::Baked, Policy::BakedPlusDelta, Policy::RuntimeIndirect } )
	{
		const Selection s = Select( policy );
		Check( !( s.worldLayer == WorldLightmapLayer::Total && s.worldAddsProducerIndirect ) &&
		           !( s.worldLayer == WorldLightmapLayer::Direct && !s.worldAddsProducerIndirect ),
		    "a variant that reads the total layer adds no producer indirect, and one that reads "
		    "the direct layer adds it" );
	}
	Check( Available( Policy::Baked, false, false ) &&
	           Available( Policy::BakedPlusDelta, false, false ),
	    "Baked and BakedPlusDelta need no separated layers" );
	Check( !Available( Policy::RuntimeIndirect, false, false ) &&
	           !Available( Policy::RuntimeIndirect, true, false ) &&
	           !Available( Policy::RuntimeIndirect, false, true ) &&
	           Available( Policy::RuntimeIndirect, true, true ),
	    "RuntimeIndirect needs the direct and indirect layers" );
	// The furnace: the baked producer publishes the baked indirect and no delta.
	Check( FurnaceHolds( Composed( Policy::Baked, kIndirect, 0.0f ) ),
	    "furnace: Baked reproduces the analytic total" );
	Check( FurnaceHolds( Composed( Policy::BakedPlusDelta, kIndirect, 0.0f ) ),
	    "furnace: BakedPlusDelta with an unchanged scene reproduces it" );
	Check( FurnaceHolds( Composed( Policy::RuntimeIndirect, kIndirect, 0.0f ) ),
	    "furnace: RuntimeIndirect with the baked producer's indirect reproduces it" );
	Check( !FurnaceHolds( Composed( Policy::RuntimeIndirect, kIndirect * 2.0f, 0.0f ) ),
	    "furnace: a producer that doubles its indirect is detected" );
	return testing::ReportConformance( g_checks, g_failures );
#endif
}
