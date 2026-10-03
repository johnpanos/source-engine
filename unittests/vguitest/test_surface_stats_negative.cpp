//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity of the VGUI surface counter checks (RFC 0010 V0): each
//          deliberately wrong per-frame helper must fail the oracle in
//          unittests/vguitest/surface_stats_checks.h, and the real one must
//          pass it.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/vguitest/surface_stats_checks.h"

#include <cstdio>

namespace
{
using surfacestatstest::PerFrameFn;
using surfacestatstest::Tally;

// Divides by paint passes instead of frames.
bool PerPass(
    const VGuiSurfaceStats_t &a, const VGuiSurfaceStats_t &b, VGuiSurfaceStatsPerFrame_t &out )
{
	if ( !VGuiSurfaceStats_PerFrame( a, b, out ) )
		return false;
	const double passes = double( b.paintPasses - a.paintPasses );
	out.draws = double( b.draws - a.draws ) / passes;
	return true;
}

// Never checks one counter for going backwards (the last one).
bool MissesACounter(
    const VGuiSurfaceStats_t &a, const VGuiSurfaceStats_t &b, VGuiSurfaceStatsPerFrame_t &out )
{
	VGuiSurfaceStats_t forward = b;
	if ( forward.cpuCopyBytes < a.cpuCopyBytes )
		forward.cpuCopyBytes = a.cpuCopyBytes;
	return VGuiSurfaceStats_PerFrame( a, forward, out );
}

// Reports a rate (of zero frames) when no frame passed.
bool RateWithoutFrames(
    const VGuiSurfaceStats_t &a, const VGuiSurfaceStats_t &b, VGuiSurfaceStatsPerFrame_t &out )
{
	if ( b.frames == a.frames )
	{
		out = VGuiSurfaceStatsPerFrame_t();
		return true;
	}
	return VGuiSurfaceStats_PerFrame( a, b, out );
}

// Uses 1000 bytes per KiB.
bool DecimalKiB(
    const VGuiSurfaceStats_t &a, const VGuiSurfaceStats_t &b, VGuiSurfaceStatsPerFrame_t &out )
{
	if ( !VGuiSurfaceStats_PerFrame( a, b, out ) )
		return false;
	out.vertexKiB = double( b.vertexBytes - a.vertexBytes ) / 1000.0 / double( out.frames );
	return true;
}

// Leaves stale values in `out` when it reports no rate.
bool StaleOnFailure(
    const VGuiSurfaceStats_t &a, const VGuiSurfaceStats_t &b, VGuiSurfaceStatsPerFrame_t &out )
{
	VGuiSurfaceStatsPerFrame_t kept = out;
	if ( VGuiSurfaceStats_PerFrame( a, b, out ) )
		return true;
	out = kept;
	return false;
}

struct Variant
{
	const char *name;
	PerFrameFn perFrame;
};
} // namespace

int main()
{
	Tally outcome;
	{
		Tally real;
		surfacestatstest::CheckRates( &VGuiSurfaceStats_PerFrame, real );
		outcome.Check( real.checks > 0 && real.failures == 0, "the real helper passes the oracle" );
	}
	const Variant variants[] = { { "per paint pass", &PerPass },
	    { "misses a counter", &MissesACounter }, { "rate without frames", &RateWithoutFrames },
	    { "decimal KiB", &DecimalKiB }, { "stale output on failure", &StaleOnFailure } };
	for ( const Variant &variant : variants )
	{
		Tally bad;
		bad.verbose = false;
		surfacestatstest::CheckRates( variant.perFrame, bad );
		char what[96];
		std::snprintf( what, sizeof( what ), "bad helper '%s' detected (%lu failing checks)",
		    variant.name, bad.failures );
		std::printf( "%s %s\n", bad.failures > 0 ? "detected" : "MISSED", what );
		outcome.Check( bad.failures > 0, what );
	}
	return testing::ReportConformance( outcome.checks, outcome.failures );
}
