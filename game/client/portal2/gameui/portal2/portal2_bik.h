//========= Portal 2 port ======================================================//
//
// Purpose: The Bink movie API the Portal 2 GameUI uses (g_pBIK, a CS:GO-era
//          IBik with playback flags), implemented on this engine's video
//          services (IVideoServices / g_pVideo). A movie plays when a video
//          provider for its file type is configured (the Bink provider in
//          video/video_bink, or an alternate-format provider through
//          IVideoServices' alternate media lookup). Otherwise CreateMaterial()
//          returns BIKHANDLE_INVALID and reports once that movies are
//          unavailable; the callers then skip the movie (menu background falls
//          back to its static image, attract movies close).
//
//=============================================================================//

#ifndef PORTAL2_BIK_H
#define PORTAL2_BIK_H
#ifdef _WIN32
#pragma once
#endif

#include "avi/ibik.h"
#include "tier1/utlvector.h"

class IMaterial;
class IVideoMaterial;

// CS:GO BIK playback flags.
#ifndef BIK_LOOP
#define BIK_LOOP	( 1 << 0 )	// restart at the end instead of finishing
#define BIK_PRELOAD	( 1 << 1 )	// read the whole movie into memory
#endif

class CPortal2BinkPlayer
{
public:
	BIKMaterial_t CreateMaterial( const char *pMaterialName, const char *pFileName, const char *pPathID, int nBikFlags );
	void DestroyMaterial( BIKMaterial_t hMaterial );

	// Advances playback. Returns false once a non-looping movie has finished
	// (or for an invalid handle), as Bink's Update() does.
	bool Update( BIKMaterial_t hMaterial );

	// True while the movie is playing. The video providers advance frames by
	// elapsed time inside Update(), so every frame may call it.
	bool ReadyForSwap( BIKMaterial_t hMaterial );

	// Only the Xbox installer asks; video providers stream from disk.
	bool IsMovieResidentInMemory( BIKMaterial_t hMaterial ) { return false; }

	IMaterial *GetMaterial( BIKMaterial_t hMaterial );
	void GetTexCoordRange( BIKMaterial_t hMaterial, float *pMaxU, float *pMaxV );
	void GetFrameSize( BIKMaterial_t hMaterial, int *pWidth, int *pHeight );

private:
	IVideoMaterial *Lookup( BIKMaterial_t hMaterial ) const;

	CUtlVector< IVideoMaterial * > m_Materials;	// index == handle; NULL marks a free slot
};

extern CPortal2BinkPlayer *g_pBIK;

#endif // PORTAL2_BIK_H
