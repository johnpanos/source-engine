//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client paint stream: owns and draws a group of paint blobs.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_PAINT_STREAM_H
#define C_PAINT_STREAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseanimating.h"
#include "sharedvar.h"
#include "paint_blobs_shared.h"
#include "paint_stream_manager.h"

DECLARE_AUTO_LIST( IPaintStreamAutoList );

class C_PaintStream : public C_BaseAnimating, public IPaintStreamAutoList
{
	DECLARE_CLASS( C_PaintStream, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

public:
	C_PaintStream();
	virtual ~C_PaintStream();

	virtual void Spawn();
	DECLARE_SHAREDCLASS();

	virtual const QAngle& GetRenderAngles();
	virtual const Vector& GetRenderOrigin();
	virtual void GetRenderBoundsWorldspace( Vector& absMins, Vector& absMaxs );
	virtual void GetRenderBounds( Vector& theMins, Vector& theMaxs );

	// Reconstruction note: the 2010 build overrode a non-const WorldSpaceCenter();
	// the imported base declares it const.
	virtual const Vector& WorldSpaceCenter() const;
	virtual void ComputeWorldSpaceSurroundingBox( Vector *pMins, Vector *pMaxs );

	virtual int DrawModel( int flags, const RenderableInstance_t &instance );
	virtual bool ShouldDraw() { return true; }

	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual void UpdateOnRemove();

	void AddPaintBlob( CPaintBlob* pBlob );
	void RemoveAllPaintBlobs( void );

	virtual const Vector& WorldAlignMins() const;
	virtual const Vector& WorldAlignMaxs() const;

	void UpdateRenderBoundsAndOriginWorldspace();

	unsigned int GetBlobsCount() const;
	CPaintBlob* GetBlob( int id );
	const PaintBlobVector_t& GetBlobList() const { return m_blobs; }

	void Update();
	void PreUpdateBlobs();
	void PostUpdateBlobs();

	void SetPaintType( int nPaintType ) { m_nPaintType = nPaintType; }

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

	PaintBlobVector_t m_blobs;

	// listen server data shared with the server dll
	void GetCurrentBlobData( BlobDataVector_t& blobData );
	CSharedUtlVector( BlobInterpolationData_t, m_sharedBlobData );
	CSharedVar( CThreadFastMutex, m_sharedBlobDataMutex );

	TimeStampVector m_UsedChannelTimestamps;

	CNetworkVar( int, m_nPaintType );
	CNetworkVar( int, m_nRenderMode );
	CNetworkVector( m_vLightPosition );
	CNetworkVar( int, m_nMaxBlobCount );

	// rendering
	void DrawBlobs( IMaterial *pMaterial );
	Vector4D m_lightingBoxColors[6];
};

typedef C_PaintStream CPaintStream;

#endif // C_PAINT_STREAM_H
