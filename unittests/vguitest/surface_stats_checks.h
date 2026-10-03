//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for the VGUI surface counters (RFC 0010 V0,
//          VGuiSurfaceStats001): the per-frame helper both readers share
//          (public/VGuiMatSurface/IVGuiSurfaceStats.h) and the module's
//          counter owner (vguimatsurface/SurfaceStats.h). The rate checks take
//          the helper as a function pointer so the sensitivity suite can run
//          them against deliberately wrong helpers.
//
//=============================================================================//

#ifndef UNITTESTS_VGUITEST_SURFACE_STATS_CHECKS_H
#define UNITTESTS_VGUITEST_SURFACE_STATS_CHECKS_H

#include "VGuiMatSurface/IVGuiSurfaceStats.h"

#include <cstdio>

namespace surfacestatstest
{

typedef bool ( *PerFrameFn )(
    const VGuiSurfaceStats_t &, const VGuiSurfaceStats_t &, VGuiSurfaceStatsPerFrame_t & );

// Every counter, so a check can visit each one; a counter added to the
// struct without a place here fails the size check below.
typedef unsigned long long VGuiSurfaceStats_t::*Counter;
const Counter kCounters[] = { &VGuiSurfaceStats_t::frames, &VGuiSurfaceStats_t::paintPasses,
    &VGuiSurfaceStats_t::paintMicroseconds, &VGuiSurfaceStats_t::draws,
    &VGuiSurfaceStats_t::textDraws, &VGuiSurfaceStats_t::vertices, &VGuiSurfaceStats_t::indices,
    &VGuiSurfaceStats_t::vertexBytes, &VGuiSurfaceStats_t::textureUploads,
    &VGuiSurfaceStats_t::textureUploadBytes, &VGuiSurfaceStats_t::glyphUploads,
    &VGuiSurfaceStats_t::glyphUploadBytes, &VGuiSurfaceStats_t::cpuCopies,
    &VGuiSurfaceStats_t::cpuCopyBytes };
const unsigned int kCounterCount = sizeof( kCounters ) / sizeof( kCounters[0] );
static_assert( sizeof( VGuiSurfaceStats_t ) ==
                   sizeof( kCounters ) / sizeof( kCounters[0] ) * sizeof( unsigned long long ),
    "every VGuiSurfaceStats_t counter is listed in kCounters" );

struct Tally
{
	unsigned long checks = 0;
	unsigned long failures = 0;
	bool verbose = true;

	void Check( bool condition, const char *what )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			if ( verbose )
				std::printf( "FAIL: %s\n", what );
		}
	}
};

inline bool AllZero( const VGuiSurfaceStatsPerFrame_t &f )
{
	return f.frames == 0 && f.paintPasses == 0 && f.paintMilliseconds == 0 && f.draws == 0 &&
	       f.textDraws == 0 && f.vertices == 0 && f.indices == 0 && f.vertexKiB == 0 &&
	       f.textureUploads == 0 && f.textureUploadKiB == 0 && f.glyphUploads == 0 &&
	       f.glyphUploadKiB == 0 && f.cpuCopies == 0 && f.cpuCopyKiB == 0;
}

// A pair of snapshots four frames apart with a distinct, exactly divisible
// difference in every counter.
inline void FourFrames( VGuiSurfaceStats_t &before, VGuiSurfaceStats_t &after )
{
	before = VGuiSurfaceStats_t();
	for ( unsigned int i = 0; i < kCounterCount; ++i )
		before.*kCounters[i] = 1000 + i;
	after = before;
	after.frames += 4;
	after.paintPasses += 8;           // 2 per frame
	after.paintMicroseconds += 6000;  // 1.5 ms per frame
	after.draws += 400;               // 100
	after.textDraws += 40;            // 10
	after.vertices += 1600;           // 400
	after.indices += 2400;            // 600
	after.vertexBytes += 4096 * 9;    // 9 KiB
	after.textureUploads += 2;        // 0.5
	after.textureUploadBytes += 8192; // 2 KiB
	after.glyphUploads += 1;          // 0.25
	after.glyphUploadBytes += 1024;   // 0.25 KiB
	after.cpuCopies += 12;            // 3
	after.cpuCopyBytes += 20480;      // 5 KiB
}

inline void CheckRates( PerFrameFn perFrame, Tally &tally )
{
	VGuiSurfaceStats_t before, after;
	FourFrames( before, after );
	VGuiSurfaceStatsPerFrame_t f;
	tally.Check( perFrame( before, after, f ), "four frames give a rate" );
	tally.Check( f.frames == 4, "frames between the snapshots" );
	tally.Check( f.paintPasses == 2.0, "paint passes per frame" );
	tally.Check( f.paintMilliseconds == 1.5, "paint milliseconds per frame" );
	tally.Check( f.draws == 100.0, "draws per frame" );
	tally.Check( f.textDraws == 10.0, "text draws per frame" );
	tally.Check( f.vertices == 400.0, "vertices per frame" );
	tally.Check( f.indices == 600.0, "indices per frame" );
	tally.Check( f.vertexKiB == 9.0, "vertex KiB per frame (1 KiB is 1024 bytes)" );
	tally.Check( f.textureUploads == 0.5, "texture uploads per frame" );
	tally.Check( f.textureUploadKiB == 2.0, "texture upload KiB per frame" );
	tally.Check( f.glyphUploads == 0.25, "glyph uploads per frame" );
	tally.Check( f.glyphUploadKiB == 0.25, "glyph upload KiB per frame" );
	tally.Check( f.cpuCopies == 3.0, "CPU copies per frame" );
	tally.Check( f.cpuCopyKiB == 5.0, "CPU copy KiB per frame" );

	// Equal snapshots: no frame passed, so there is no rate.
	f.draws = 7;
	tally.Check( !perFrame( after, after, f ) && AllZero( f ), "no frames: no rate, zeroed" );
	VGuiSurfaceStats_t noFrame = after;
	noFrame.draws += 50;
	f.draws = 7;
	tally.Check(
	    !perFrame( after, noFrame, f ) && AllZero( f ), "work without a frame: no rate, zeroed" );

	// Any counter going backwards means another or a restarted surface.
	for ( unsigned int i = 0; i < kCounterCount; ++i )
	{
		VGuiSurfaceStats_t restarted = after;
		restarted.*kCounters[i] = before.*kCounters[i] - 1;
		f.draws = 7;
		char what[96];
		std::snprintf( what, sizeof( what ), "counter %u going backwards: no rate, zeroed", i );
		tally.Check( !perFrame( before, restarted, f ) && AllZero( f ), what );
	}

	// One frame of large totals.
	VGuiSurfaceStats_t big = VGuiSurfaceStats_t(), bigAfter;
	for ( unsigned int i = 0; i < kCounterCount; ++i )
		big.*kCounters[i] = 1ull << 50;
	bigAfter = big;
	bigAfter.frames += 1;
	bigAfter.draws += 3;
	tally.Check( perFrame( big, bigAfter, f ) && f.frames == 1 && f.draws == 3.0 && f.vertices == 0,
	    "one frame of large totals" );
}

} // namespace surfacestatstest

#endif // UNITTESTS_VGUITEST_SURFACE_STATS_CHECKS_H
