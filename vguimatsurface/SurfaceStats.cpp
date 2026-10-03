//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The surface's counters behind VGuiSurfaceStats001 (RFC 0010 V0).
//
//=============================================================================//

#include "SurfaceStats.h"

void CSurfaceStats::NoteDraw( int vertices, int indices, int vertexSize, bool text )
{
	Add( m_Draws, 1 );
	if ( text )
		Add( m_TextDraws, 1 );
	if ( vertices > 0 )
	{
		Add( m_Vertices, std::uint64_t( vertices ) );
		if ( vertexSize > 0 )
			Add( m_VertexBytes, std::uint64_t( vertices ) * std::uint64_t( vertexSize ) );
	}
	if ( indices > 0 )
		Add( m_Indices, std::uint64_t( indices ) );
}

void CSurfaceStats::GetStats( VGuiSurfaceStats_t &stats )
{
	// Relaxed loads: each value is exact, the set is not one cut (see
	// IVGuiSurfaceStats.h), and no other memory is ordered through them.
	auto read = []( const Counter &counter )
	{
		return counter.load( std::memory_order_relaxed );
	};
	stats.frames = read( m_Frames );
	stats.paintPasses = read( m_PaintPasses );
	stats.paintMicroseconds = read( m_PaintMicroseconds );
	stats.draws = read( m_Draws );
	stats.textDraws = read( m_TextDraws );
	stats.vertices = read( m_Vertices );
	stats.indices = read( m_Indices );
	stats.vertexBytes = read( m_VertexBytes );
	stats.textureUploads = read( m_TextureUploads );
	stats.textureUploadBytes = read( m_TextureUploadBytes );
	stats.glyphUploads = read( m_GlyphUploads );
	stats.glyphUploadBytes = read( m_GlyphUploadBytes );
	stats.cpuCopies = read( m_CpuCopies );
	stats.cpuCopyBytes = read( m_CpuCopyBytes );
}

CSurfaceStats &SurfaceStats()
{
	static CSurfaceStats s_Stats;
	return s_Stats;
}
