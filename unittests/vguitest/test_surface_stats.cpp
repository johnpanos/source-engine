//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance of the VGUI surface counters (RFC 0010 V0,
//          VGuiSurfaceStats001): the shared per-frame helper and the module's
//          counter owner, vguimatsurface/SurfaceStats.cpp.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/vguitest/surface_stats_checks.h"
#include "vguimatsurface/SurfaceStats.h"

#include <chrono>
#include <thread>
#include <vector>

namespace
{
using surfacestatstest::Tally;

VGuiSurfaceStats_t Read( IVGuiSurfaceStats &stats )
{
	VGuiSurfaceStats_t out;
	stats.GetStats( out );
	return out;
}

void CheckCounters( Tally &tally )
{
	CSurfaceStats stats;
	VGuiSurfaceStats_t s = Read( stats );
	bool zero = true;
	for ( unsigned int i = 0; i < surfacestatstest::kCounterCount; ++i )
		zero = zero && s.*surfacestatstest::kCounters[i] == 0;
	tally.Check( zero, "a new owner reads all zero" );

	stats.NoteDraw( 4, 6, 24, false );
	s = Read( stats );
	tally.Check( s.draws == 1 && s.textDraws == 0 && s.vertices == 4 && s.indices == 6 &&
	                 s.vertexBytes == 96,
	    "a draw counts its vertices, indices and vertex bytes" );
	stats.NoteDraw( 8, 12, 24, true );
	s = Read( stats );
	tally.Check( s.draws == 2 && s.textDraws == 1 && s.vertices == 12 && s.indices == 18 &&
	                 s.vertexBytes == 288,
	    "a text draw also counts as a text draw" );
	stats.NoteDraw( -1, -1, 24, false );
	stats.NoteDraw( 4, 0, 0, false );
	s = Read( stats );
	tally.Check( s.draws == 4 && s.vertices == 16 && s.indices == 18 && s.vertexBytes == 288,
	    "negative counts and an unknown vertex size add no data" );

	stats.NoteTextureUpload( 16, 8, 4 );
	stats.NoteTextureUpload( -16, 8, 4 );
	stats.NoteGlyphUpload( 10, 12, 4 );
	stats.NoteGlyphUpload( 10, 0, 4 );
	stats.NoteCpuCopy( 100 );
	stats.NoteCpuCopy( 0 );
	s = Read( stats );
	tally.Check( s.textureUploads == 2 && s.textureUploadBytes == 512,
	    "uploads count every call and only a real rectangle's bytes" );
	tally.Check( s.glyphUploads == 2 && s.glyphUploadBytes == 480,
	    "glyph uploads count every glyph and only real bytes" );
	tally.Check( s.cpuCopies == 2 && s.cpuCopyBytes == 100, "CPU copies count calls and bytes" );

	stats.NoteFrame();
	stats.NoteFrame();
	{
		CSurfaceStats::PaintPass pass( stats );
		const auto start = std::chrono::steady_clock::now();
		while ( std::chrono::steady_clock::now() - start < std::chrono::milliseconds( 2 ) )
		{
		}
	}
	s = Read( stats );
	tally.Check( s.frames == 2, "frames counted" );
	tally.Check( s.paintPasses == 1 && s.paintMicroseconds >= 2000,
	    "a paint pass counts once with at least its elapsed time" );
	tally.Check( s.draws == 4 && s.textureUploads == 2, "frames and passes leave other counters" );

	// Two snapshots of the real owner through the shared helper.
	const VGuiSurfaceStats_t before = Read( stats );
	stats.NoteFrame();
	stats.NoteFrame();
	stats.NoteDraw( 6, 6, 20, false );
	stats.NoteDraw( 6, 6, 20, true );
	VGuiSurfaceStatsPerFrame_t f;
	tally.Check( VGuiSurfaceStats_PerFrame( before, Read( stats ), f ) && f.frames == 2 &&
	                 f.draws == 1.0 && f.textDraws == 0.5 && f.vertices == 6.0,
	    "the owner's snapshots give per-frame means" );
}

// Texture regeneration can run on the material system's render thread while
// the main thread paints: concurrent updates must all be counted.
void CheckConcurrentUpdates( Tally &tally )
{
	CSurfaceStats stats;
	const int threads = 4;
	const int perThread = 100000;
	std::vector<std::thread> workers;
	for ( int t = 0; t < threads; ++t )
	{
		workers.emplace_back(
		    [&stats, t]()
		    {
			    for ( int i = 0; i < perThread; ++i )
			    {
				    stats.NoteDraw( 2, 3, 10, ( i + t ) % 2 == 0 );
				    stats.NoteCpuCopy( 5 );
			    }
		    } );
	}
	for ( std::thread &worker : workers )
		worker.join();
	const VGuiSurfaceStats_t s = Read( stats );
	const unsigned long long n = (unsigned long long)threads * perThread;
	tally.Check( s.draws == n && s.textDraws == n / 2 && s.vertices == 2 * n &&
	                 s.indices == 3 * n && s.vertexBytes == 20 * n,
	    "concurrent draws are all counted" );
	tally.Check( s.cpuCopies == n && s.cpuCopyBytes == 5 * n, "concurrent copies are all counted" );
}

void CheckModuleOwner( Tally &tally )
{
	CSurfaceStats &a = SurfaceStats();
	CSurfaceStats &b = SurfaceStats();
	tally.Check( &a == &b, "the module has one counter owner" );
	IVGuiSurfaceStats *served = &a;
	const unsigned long long frames = Read( *served ).frames;
	a.NoteFrame();
	tally.Check(
	    Read( *served ).frames == frames + 1, "the interface reads the module's counters" );
}
} // namespace

int main()
{
	Tally tally;
	surfacestatstest::CheckRates( &VGuiSurfaceStats_PerFrame, tally );
	CheckCounters( tally );
	CheckConcurrentUpdates( tally );
	CheckModuleOwner( tally );
	return testing::ReportConformance( tally.checks, tally.failures );
}
