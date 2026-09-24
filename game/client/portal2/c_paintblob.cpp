//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side paint blob.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_paintblob.h"
#include "paint_stream_manager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar paintbomb_draw_num_paint_blobs( "paintbomb_draw_num_paint_blobs", "20", 0 );
ConVar paintbomb_draw_sphere_radius( "paintbomb_draw_sphere_radius", "25.0f", 0 );
ConVar paintbomb_draw_min_blob_radius( "paintbomb_draw_min_blob_radius", "0.6", 0 );
ConVar paintbomb_draw_max_blob_radius( "paintbomb_draw_max_blob_radius", "0.8", 0 );
ConVar paintbomb_draw_blob_speed_min( "paintbomb_draw_blob_speed_min", "50.0f", 0 );
ConVar paintbomb_draw_blob_speed_max( "paintbomb_draw_blob_speed_max", "80.0f", 0 );

C_PaintBlob::C_PaintBlob()
{
}


C_PaintBlob::~C_PaintBlob()
{
}


void CreateBlobsForPaintBomb( const Vector &vecBombPos, int paintType, PaintBlobVector_t &blobs )
{
	int numBlobs = paintbomb_draw_num_paint_blobs.GetInt();

	for ( int i = 0; i < numBlobs; ++i )
	{
		// Random position on the surface of the bomb
		Vector vecRand = RandomVector( -1.0f, 1.0f );
		VectorNormalize( vecRand );

		Vector vecBlobPos = vecBombPos + paintbomb_draw_sphere_radius.GetFloat() * vecRand;

		// The blobs fall in towards the center of the bomb
		Vector vecBlobDir = vecBlobPos - vecBombPos;
		VectorNormalize( vecBlobDir );

		Vector vecBlobVelocity = -vecBlobDir * RandomFloat( paintbomb_draw_blob_speed_min.GetFloat(), paintbomb_draw_blob_speed_max.GetFloat() );

		// Reconstruction note: the 2010 build called PaintBlobCreate( pos, vel, type, 0, 0, false, NULL ).
		// The imported blob API allocates from the stream manager pool and initializes separately.
		CPaintBlob *pBlob = PaintStreamManager.AllocatePaintBlob();
		if ( !pBlob )
			return;

		pBlob->Init( vecBlobPos, vecBlobVelocity, paintType, 0.0f, 0.0f, NULL, false, false );
		pBlob->SetRadiusScale( RandomFloat( paintbomb_draw_min_blob_radius.GetFloat(), paintbomb_draw_max_blob_radius.GetFloat() ) );

		blobs.AddToTail( pBlob );
	}
}


void C_PaintBlob::PaintBlobPaint( const trace_t &tr )
{
	// The client doesn't paint, it only plays the impact effect
	Vector vecTouchPos = tr.endpos;
	Vector vecNormal = tr.plane.normal;

	PlayEffect( vecTouchPos, vecNormal );
}
