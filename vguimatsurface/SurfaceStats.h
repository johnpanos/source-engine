//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The surface's counters behind VGuiSurfaceStats001 (RFC 0010 V0;
//          public/VGuiMatSurface/IVGuiSurfaceStats.h owns their meaning).
//          One owner for the module: the surface, its texture dictionary and
//          its font cache report here, and the interface reads a snapshot.
//
//          Each counter is a relaxed atomic. Texture regeneration can run on
//          the material system's render thread, so updates come from more
//          than one thread; nothing orders other memory through them.
//
//=============================================================================//

#ifndef VGUIMATSURFACE_SURFACESTATS_H
#define VGUIMATSURFACE_SURFACESTATS_H

#include "VGuiMatSurface/IVGuiSurfaceStats.h"

#include <atomic>
#include <chrono>
#include <cstdint>

class CSurfaceStats final : public IVGuiSurfaceStats
{
public:
	void GetStats( VGuiSurfaceStats_t &stats ) override;

	void NoteFrame() { Add( m_Frames, 1 ); }
	void NotePaintPass( std::uint64_t microseconds )
	{
		Add( m_PaintPasses, 1 );
		Add( m_PaintMicroseconds, microseconds );
	}
	// One IMesh::Draw of `vertices` vertices of `vertexSize` bytes each.
	void NoteDraw( int vertices, int indices, int vertexSize, bool text );
	// One Download of a width x height rectangle at `texelBytes` per texel.
	void NoteTextureUpload( int width, int height, int texelBytes )
	{
		Add( m_TextureUploads, 1 );
		Add( m_TextureUploadBytes, Area( width, height, texelBytes ) );
	}
	void NoteGlyphUpload( int width, int height, int texelBytes )
	{
		Add( m_GlyphUploads, 1 );
		Add( m_GlyphUploadBytes, Area( width, height, texelBytes ) );
	}
	void NoteCpuCopy( std::uint64_t bytes )
	{
		Add( m_CpuCopies, 1 );
		Add( m_CpuCopyBytes, bytes );
	}

	// Times a top-level paint pass: NotePaintPass with the elapsed time when
	// it goes out of scope.
	class PaintPass
	{
	public:
		explicit PaintPass( CSurfaceStats &stats )
		    : m_Stats( stats ), m_Start( std::chrono::steady_clock::now() )
		{
		}
		~PaintPass()
		{
			const auto elapsed = std::chrono::steady_clock::now() - m_Start;
			m_Stats.NotePaintPass( std::uint64_t(
			    std::chrono::duration_cast<std::chrono::microseconds>( elapsed ).count() ) );
		}
		PaintPass( const PaintPass & ) = delete;
		PaintPass &operator=( const PaintPass & ) = delete;

	private:
		CSurfaceStats &m_Stats;
		std::chrono::steady_clock::time_point m_Start;
	};

private:
	typedef std::atomic<std::uint64_t> Counter;

	static void Add( Counter &counter, std::uint64_t value )
	{
		counter.fetch_add( value, std::memory_order_relaxed );
	}
	// Bytes of a rectangle; negative or empty dimensions count as nothing.
	static std::uint64_t Area( int width, int height, int texelBytes )
	{
		if ( width <= 0 || height <= 0 || texelBytes <= 0 )
			return 0;
		return std::uint64_t( width ) * std::uint64_t( height ) * std::uint64_t( texelBytes );
	}

	Counter m_Frames{ 0 };
	Counter m_PaintPasses{ 0 };
	Counter m_PaintMicroseconds{ 0 };
	Counter m_Draws{ 0 };
	Counter m_TextDraws{ 0 };
	Counter m_Vertices{ 0 };
	Counter m_Indices{ 0 };
	Counter m_VertexBytes{ 0 };
	Counter m_TextureUploads{ 0 };
	Counter m_TextureUploadBytes{ 0 };
	Counter m_GlyphUploads{ 0 };
	Counter m_GlyphUploadBytes{ 0 };
	Counter m_CpuCopies{ 0 };
	Counter m_CpuCopyBytes{ 0 };
};

// The module's counters (vguimatsurface only).
CSurfaceStats &SurfaceStats();

#endif // VGUIMATSURFACE_SURFACESTATS_H
