//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side paint blob.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_PAINTBLOB_H
#define C_PAINTBLOB_H

#ifdef _WIN32
#pragma once
#endif

#include "paint_blobs_shared.h"

class C_PaintBlob : public CBasePaintBlob
{
public:
	C_PaintBlob();
	~C_PaintBlob();

	// Reconstruction note: private in the 2010 DWARF; the imported
	// paint_blobs_shared.cpp calls it through CPaintBlob*, so it is public.
	virtual void PaintBlobPaint( const trace_t &tr );
};

// Creates the visual-only blobs of a paint bomb (see c_prop_paint_bomb.cpp)
void CreateBlobsForPaintBomb( const Vector &vecBombPos, int paintType, PaintBlobVector_t &blobs );

#endif // C_PAINTBLOB_H
