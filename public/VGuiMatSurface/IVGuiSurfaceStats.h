//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VGuiSurfaceStats001 (RFC 0010 V0 counters): what the material
//          system surface submits for the UI, so draw-list, caching and
//          transfer work is judged against a measured baseline.
//
//          Totals are monotonic from surface start. A reader keeps two
//          snapshots and divides their difference by the frames between
//          them (VGuiSurfaceStats_PerFrame), so no reader resets another's
//          view. Each counter is updated atomically, but a snapshot is not
//          one atomic cut: counters can be up to one update apart. They are
//          diagnostics, never inputs to rendering.
//
//          The surface serves this interface through its QueryInterface, as
//          it serves VGuiWorldPanelRecorder001. It is first-party and not
//          part of the frozen VGUI ABI (legacy.vgui-abi).
//
//=============================================================================//

#ifndef IVGUISURFACESTATS_H
#define IVGUISURFACESTATS_H
#ifdef _WIN32
#pragma once
#endif

#define VGUI_SURFACE_STATS_INTERFACE_VERSION "VGuiSurfaceStats001"

struct VGuiSurfaceStats_t
{
	// ISurface::RunFrame calls: one per engine frame.
	unsigned long long frames;
	// Top-level PaintTraverse calls (the engine makes one to three per frame)
	// and the main-thread time inside them: panels' Paint, the surface's
	// clipping and mesh building, and glyph rasterization. Under the queued
	// material system the render thread's replay is not included.
	unsigned long long paintPasses;
	unsigned long long paintMicroseconds;
	// IMesh::Draw calls the surface issued, of which text batches; and the
	// vertices, indices and vertex bytes it wrote into dynamic meshes. The
	// bytes are what the surface writes; the backend's own copies and
	// uploads of them are not counted here.
	unsigned long long draws;
	unsigned long long textDraws;
	unsigned long long vertices;
	unsigned long long indices;
	unsigned long long vertexBytes;
	// ITexture::Download calls for the surface's procedural textures (glyph
	// pages included), and the texels in their rectangles in the texture's
	// format.
	unsigned long long textureUploads;
	unsigned long long textureUploadBytes;
	// Of those: glyphs rasterized into font pages, and their bytes.
	unsigned long long glyphUploads;
	unsigned long long glyphUploadBytes;
	// CPU copies of texel data by the surface: the caller's pixels into the
	// texture's backing copy, and the backing copy into the material
	// system's image when it regenerates (an upload or a restore). The
	// backend's staging copy is outside the surface and not counted.
	unsigned long long cpuCopies;
	unsigned long long cpuCopyBytes;
};

// Per-frame means between two snapshots.
struct VGuiSurfaceStatsPerFrame_t
{
	unsigned long long frames; // frames between the snapshots
	double paintPasses;
	double paintMilliseconds;
	double draws;
	double textDraws;
	double vertices;
	double indices;
	double vertexKiB;
	double textureUploads;
	double textureUploadKiB;
	double glyphUploads;
	double glyphUploadKiB;
	double cpuCopies;
	double cpuCopyKiB;
};

// The per-frame means from `before` to `after`. False, with `out` zeroed,
// when no frame passed between them or any counter went backwards (a
// different or restarted surface): there is no rate to report.
inline bool VGuiSurfaceStats_PerFrame( const VGuiSurfaceStats_t &before,
    const VGuiSurfaceStats_t &after, VGuiSurfaceStatsPerFrame_t &out )
{
	typedef unsigned long long VGuiSurfaceStats_t::*Counter;
	static const Counter counters[] = { &VGuiSurfaceStats_t::frames,
	    &VGuiSurfaceStats_t::paintPasses, &VGuiSurfaceStats_t::paintMicroseconds,
	    &VGuiSurfaceStats_t::draws, &VGuiSurfaceStats_t::textDraws, &VGuiSurfaceStats_t::vertices,
	    &VGuiSurfaceStats_t::indices, &VGuiSurfaceStats_t::vertexBytes,
	    &VGuiSurfaceStats_t::textureUploads, &VGuiSurfaceStats_t::textureUploadBytes,
	    &VGuiSurfaceStats_t::glyphUploads, &VGuiSurfaceStats_t::glyphUploadBytes,
	    &VGuiSurfaceStats_t::cpuCopies, &VGuiSurfaceStats_t::cpuCopyBytes };
	out = VGuiSurfaceStatsPerFrame_t();
	for ( unsigned int i = 0; i < sizeof( counters ) / sizeof( counters[0] ); ++i )
	{
		if ( after.*counters[i] < before.*counters[i] )
			return false;
	}
	const unsigned long long frames = after.frames - before.frames;
	if ( frames == 0 )
		return false;
	const double n = double( frames );
	out.frames = frames;
	out.paintPasses = double( after.paintPasses - before.paintPasses ) / n;
	out.paintMilliseconds =
	    double( after.paintMicroseconds - before.paintMicroseconds ) / 1000.0 / n;
	out.draws = double( after.draws - before.draws ) / n;
	out.textDraws = double( after.textDraws - before.textDraws ) / n;
	out.vertices = double( after.vertices - before.vertices ) / n;
	out.indices = double( after.indices - before.indices ) / n;
	out.vertexKiB = double( after.vertexBytes - before.vertexBytes ) / 1024.0 / n;
	out.textureUploads = double( after.textureUploads - before.textureUploads ) / n;
	out.textureUploadKiB =
	    double( after.textureUploadBytes - before.textureUploadBytes ) / 1024.0 / n;
	out.glyphUploads = double( after.glyphUploads - before.glyphUploads ) / n;
	out.glyphUploadKiB = double( after.glyphUploadBytes - before.glyphUploadBytes ) / 1024.0 / n;
	out.cpuCopies = double( after.cpuCopies - before.cpuCopies ) / n;
	out.cpuCopyKiB = double( after.cpuCopyBytes - before.cpuCopyBytes ) / 1024.0 / n;
	return true;
}

class IVGuiSurfaceStats
{
public:
	virtual void GetStats( VGuiSurfaceStats_t &stats ) = 0;

protected:
	~IVGuiSurfaceStats() {}
};

#endif // IVGUISURFACESTATS_H
