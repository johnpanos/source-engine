//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server paint blob: paints surfaces and pushes physics objects.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef CPAINTBLOB_H
#define CPAINTBLOB_H
#ifdef _WIN32
#pragma once
#endif

#include "paint_blobs_shared.h"

class CPaintBlob : public CBasePaintBlob
{
public:
	CPaintBlob();
	~CPaintBlob();

	void AddToPaintDatabase();

	void AssignBlobID( int id ) { m_ID = id; }
	int GetBlobID() { return m_ID; }

	// Reconstruction note: the 2010 DWARF declares SetBlobTeleportedThisFrame() and
	// HasBlobTeleportedThisFrame() (and m_bTeleportedThisFrame) on CPaintBlob. The
	// imported paint_blobs_shared.h moved that flag and both accessors to
	// CBasePaintBlob, where the shared update code sets it, so they are inherited
	// here rather than shadowed.

	void AddBlobTeleportationHistory( const BlobTeleportationHistory_t& history );
	void ClearTeleportationHistory();
	void GetTeleportationHistory( BlobTeleportationHistoryVector_t& historyList );

	// Reconstruction note: the 2010 build also declared PaintBlobCleanse(),
	// PaintBlobStreakPaint() and ResolveCollision() as CPaintBlob virtuals defined in
	// paint_blobs_shared.cpp. The imported shared code implements them on
	// CBasePaintBlob and calls PaintBlobPaint() through a CPaintBlob pointer, so
	// PaintBlobPaint() is the public override of the base pure virtual.
	virtual void PaintBlobPaint( const trace_t &tr );

private:
	void SetupPaintInfo( const trace_t &tr );

	//Paint info
	CBaseEntity *m_pPaintingEntity;
	unsigned short m_nSurfaceFlags;
	int m_ID;

	BlobTeleportationHistoryVector_t m_teleportationHistory;
};

#endif // CPAINTBLOB_H
