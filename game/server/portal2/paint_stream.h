//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server paint stream: owns and networks a group of paint blobs.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PAINT_STREAM_H
#define PAINT_STREAM_H
#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"
#include "sharedvar.h"
#include "paint_blobs_shared.h"
#include "paint_sprayer_shared.h"
#include "paint_stream_manager.h"

class CPaintBlob;
class ISave;
class IRestore;

DECLARE_AUTO_LIST( IPaintStreamAutoList );

class CPaintStream : public CBaseAnimating, public IPaintStreamAutoList
{
	DECLARE_CLASS( CPaintStream, CBaseAnimating );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CPaintStream();
	virtual ~CPaintStream();

	virtual void Spawn();
	virtual void Precache();
	DECLARE_SHAREDCLASS();
	virtual void UpdateOnRemove();
	virtual int UpdateTransmitState();

	// Reconstruction note: the retail server passes the stream's light position,
	// paint type, render mode and blob budget in one call before DispatchSpawn()
	// (the 2010 build only had SetPaintType() and SetStreamRenderMode()). The
	// original name of this initializer is not recorded.
	void Init( const Vector& vLightPosition, int nPaintType, int nRenderMode, int nMaxBlobCount );

	void AddPaintBlob( CPaintBlob* pBlob );
	void RemoveAllPaintBlobs( void );

	const Vector& WorldAlignMins() const;
	const Vector& WorldAlignMaxs() const;

	void UpdateRenderBoundsAndOriginWorldspace();

	unsigned int GetBlobsCount() const;
	CPaintBlob* GetBlob( int id );
	const PaintBlobVector_t& GetBlobList() const { return m_blobs; }

	void Update();
	void PreUpdateBlobs();
	void PostUpdateBlobs();

	void SetPaintType( int nPaintType ) { m_nPaintType = nPaintType; }
	void SetStreamRenderMode( int nRenderMode ) { m_nRenderMode = nRenderMode; }

	// save/restore
	void SavePaintBlobState( ISave *pSave );
	void RestorePaintBlobState( IRestore *pRestore );

private:
	void DebugDrawBlobs();

	void RemoveDeadBlobs();
	void RemoveTeleportedThisFrameBlobs();
	void ResetBlobsTeleportedThisFrame();

	void QueuePaintEffect();

	Vector m_vCachedRenderOrigin;
	Vector m_vCachedWorldMins;
	Vector m_vCachedWorldMaxs;
	int m_iCachedWorldBoundsUpdateTick;

	int m_nCurrentUpdateFrame;

	int m_nBlobCounter;
	PaintBlobVector_t m_blobs;

	// listen server data shared with the client dll
	CSharedUtlVector( BlobInterpolationData_t, m_sharedBlobData );
	CSharedVar( CThreadFastMutex, m_sharedBlobDataMutex );

	TimeStampVector m_UsedChannelTimestamps;

	CNetworkVar( int, m_nPaintType );
	CNetworkVar( int, m_nRenderMode );
	CNetworkVector( m_vLightPosition );
	CNetworkVar( int, m_nMaxBlobCount );

	void AddPaintToDatabase() const;
	void UpdateBlobSharedData();
};

#endif // PAINT_STREAM_H
