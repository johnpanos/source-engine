//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server paint stream: owns and networks a group of paint blobs.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "paint_stream.h"
#include "cpaintblob.h"
#include "paint_stream_manager.h"
#include "paint_sprayer_shared.h"
#include "debugoverlay_shared.h"
#include "isaverestore.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar draw_paint_server_blobs( "draw_paint_server_blobs", "0", FCVAR_DEVELOPMENTONLY );
ConVar paintblob_old_data_time_offset( "paintblob_old_data_time_offset", "0.2f", FCVAR_DEVELOPMENTONLY, "How much time offset from what client is asking before the data is deleted." );

IMPLEMENT_SERVERCLASS_ST( CPaintStream, DT_PaintStream )
	SendPropInt( SENDINFO( m_nPaintType ) ),
	SendPropInt( SENDINFO( m_nRenderMode ) ),
	SendPropVector( SENDINFO( m_vLightPosition ), 32, SPROP_NOSCALE ),
	SendPropInt( SENDINFO( m_nMaxBlobCount ) ),
END_SEND_TABLE()


BEGIN_DATADESC( CPaintStream )
	DEFINE_FIELD( m_nBlobCounter, FIELD_INTEGER ),
	DEFINE_FIELD( m_nPaintType, FIELD_INTEGER ),
	// Reconstruction note: the 2010 build exposed the render mode as the
	// "BlobRenderMode" keyfield; the retail table saves it as a plain field.
	DEFINE_KEYFIELD( m_nRenderMode, FIELD_INTEGER, "BlobRenderMode" ),
	DEFINE_FIELD( m_vLightPosition, FIELD_VECTOR ),
	DEFINE_FIELD( m_nMaxBlobCount, FIELD_INTEGER ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( paint_stream, CPaintStream );

IMPLEMENT_AUTO_LIST( IPaintStreamAutoList );


// Extra room around the blob positions for the stream's render bounds.
// Reconstruction note: the binaries only record the literal 18 units.
static const float PAINT_STREAM_BOUNDS_BLOAT = 18.0f;


CPaintStream::CPaintStream()
{
	m_nBlobCounter = 0;
	m_nRenderMode = BLOB_RENDER_BLOBULATOR;
}


CPaintStream::~CPaintStream()
{
	RemoveAllPaintBlobs();

	m_sharedBlobData.GetForModify().Purge();
}


void CPaintStream::Precache()
{
	BaseClass::Precache();
}


void CPaintStream::Spawn()
{
	AddEffects( EF_NOSHADOW | EF_NORECEIVESHADOW );

	m_iCachedWorldBoundsUpdateTick = gpGlobals->tickcount;

	BaseClass::Spawn();
}


void CPaintStream::Init( const Vector& vLightPosition, int nPaintType, int nRenderMode, int nMaxBlobCount )
{
	m_vLightPosition = vLightPosition;
	m_nPaintType = nPaintType;
	m_nRenderMode = nRenderMode;
	m_nMaxBlobCount = nMaxBlobCount;

	// Portal 2 port: the server's blobs come from the manager's pool, which
	// nothing else creates on the server (the client does it on its first
	// update, C_PaintStream::OnDataChanged).
	PaintStreamManager.AllocatePaintBlobPool( nMaxBlobCount );
}


int CPaintStream::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}


void CPaintStream::AddPaintBlob( CPaintBlob* pBlob )
{
	if ( !pBlob )
		return;

	m_blobs.AddToTail( pBlob );

	CPaintBlob *pServerBlob = pBlob;
	pServerBlob->AssignBlobID( entindex() + m_nBlobCounter );
	++m_nBlobCounter;
}


void CPaintStream::UpdateRenderBoundsAndOriginWorldspace()
{
	if ( m_iCachedWorldBoundsUpdateTick == gpGlobals->tickcount )
		return;

	m_iCachedWorldBoundsUpdateTick = gpGlobals->tickcount;

	if ( m_blobs.Count() == 0 )
	{
		m_vCachedWorldMaxs = m_vCachedWorldMins = m_vCachedRenderOrigin = Vector( 0.f, 0.f, 0.f );
		return;
	}

	Vector vecPos = m_blobs[0]->GetPosition();

	m_vCachedWorldMins = vecPos;
	m_vCachedWorldMaxs = vecPos;

	for ( int i = 1; i < m_blobs.Count(); ++i )
	{
		vecPos = m_blobs[i]->GetPosition();
		VectorMin( vecPos, m_vCachedWorldMins, m_vCachedWorldMins );
		VectorMax( vecPos, m_vCachedWorldMaxs, m_vCachedWorldMaxs );
	}

	m_vCachedRenderOrigin = 0.5f * ( m_vCachedWorldMins + m_vCachedWorldMaxs );

	Vector vRadius = ReplicateToVector( PAINT_STREAM_BOUNDS_BLOAT );
	m_vCachedWorldMins -= vRadius;
	m_vCachedWorldMaxs += vRadius;

	CCollisionProperty *prop = CollisionProp();
	prop->MarkSurroundingBoundsDirty();
}


//-----------------------------------------------------------------------------
// Purpose: Per-stream update used by the 2010 build.
// Reconstruction note: the imported PaintStreamManager simulates every stream's
// blobs in one batch through PreUpdateBlobs()/PostUpdateBlobs(); this keeps the
// 2010 single-stream entry point, expressed with the same helpers.
//-----------------------------------------------------------------------------
void CPaintStream::Update()
{
	if ( m_nCurrentUpdateFrame == gpGlobals->framecount )
		return;

	if ( m_blobs.Count() != 0 )
	{
		RemoveDeadBlobs();
		DebugDrawBlobs();

		PaintBlobUpdate( m_blobs );
	}

	UpdateRenderBoundsAndOriginWorldspace();
	UpdateBlobSharedData();

	m_nCurrentUpdateFrame = gpGlobals->framecount;
}


void CPaintStream::UpdateBlobSharedData()
{
	// Reconstruction note: the retail stream shares a mutex with the listen-server
	// client alongside the blob data; hold it while the data is rewritten.
	AUTO_LOCK( m_sharedBlobDataMutex.GetForModify() );

	BlobInterpolationDataVector_t& blobData = m_sharedBlobData.GetForModify();
	int lastIndex = blobData.AddToTail();
	blobData[lastIndex].m_flUpdateTime = gpGlobals->curtime;

	const int nBlobCount = m_blobs.Count();

	BlobDataVector_t& blobDataVector = blobData[lastIndex].m_blobData;
	blobDataVector.SetCount( nBlobCount );
	int nNumParticles = 0;
	for ( int i = 0; i < nBlobCount; ++i )
	{
		CPaintBlob *pBlob = m_blobs[i];
		if ( pBlob->IsSilent() )
			continue;

		bool bTeleportThisFrame = pBlob->HasBlobTeleportedThisFrame();

		BlobData_t& blob = blobDataVector[nNumParticles];
		blob.m_blobID = pBlob->GetBlobID();
		blob.m_flScale = pBlob->GetRadiusScale();
		blob.m_vPosition = pBlob->GetPosition();
		pBlob->GetTeleportationHistory( blob.m_teleportationHistory );
		blob.m_bTeleportedThisFrame = bTeleportThisFrame;

		// Reconstruction note: the 2010 build appended a second, negative-ID entry at
		// the ghost position for blobs passing through a portal. The imported
		// BlobData_t carries the ghost transform on the blob's own entry instead.
		blob.m_bGhosting = pBlob->IsGhosting();
		if ( blob.m_bGhosting )
		{
			pBlob->GetGhostMatrix( blob.m_matGhostTransform );
		}

		++nNumParticles;

		if ( bTeleportThisFrame )
		{
			pBlob->SetBlobTeleportedThisFrame( false );
			pBlob->ClearTeleportationHistory();
		}
	}

	blobDataVector.SetCountNonDestructively( nNumParticles );

	// remove data that is too old for the client to ask for
	const float flShouldRemoveTime = gpGlobals->curtime - paintblob_old_data_time_offset.GetFloat();
	int nToRemove = 0;
	for ( int i = 0; i < blobData.Count(); ++i )
	{
		if ( blobData[i].m_flUpdateTime < flShouldRemoveTime )
		{
			++nToRemove;
		}
		else
		{
			break;
		}
	}

	blobData.RemoveMultipleFromHead( nToRemove );
}


void CPaintStream::DebugDrawBlobs()
{
	bool bDebugDraw = draw_paint_server_blobs.GetBool();
	Color debugColor( 255, 0, 255 );

	if ( !bDebugDraw )
		return;

	for ( int i = 0; i < m_blobs.Count(); ++i )
	{
		NDebugOverlay::Cross3D( m_blobs[i]->GetPosition(), 2.f, debugColor.r(), debugColor.g(), debugColor.b(), false, 0.1f );
	}
}


void CPaintStream::AddPaintToDatabase() const
{
	for ( int i = 0; i < m_blobs.Count(); ++i )
	{
		CPaintBlob *pBlob = m_blobs[i];
		pBlob->AddToPaintDatabase();
	}
}


void CPaintStream::SavePaintBlobState( ISave *pSave )
{
	int count = m_blobs.Count();

	pSave->StartBlock();
	pSave->WriteInt( &count );

	if ( count > 0 )
	{
		EHANDLE hPaintStream = this;
		pSave->WriteEHandle( &hPaintStream );

		int power = m_nPaintType;
		pSave->WriteInt( &power );

		for ( int i = 0; i < count; ++i )
		{
			CBasePaintBlob *pBlob = m_blobs[i];
			pSave->WriteVector( pBlob->GetPosition() );
			pSave->WriteVector( pBlob->GetVelocity() );

			float flMaxStreakTime = pBlob->GetStreakTime();
			pSave->WriteFloat( &flMaxStreakTime );

			float flStreakSpeedDampenRate = pBlob->GetStreakSpeedDampenRate();
			pSave->WriteFloat( &flStreakSpeedDampenRate );
		}
	}

	pSave->EndBlock();
}


void CPaintStream::RestorePaintBlobState( IRestore *pRestore )
{
	// don't let the blobs simulate while they are being recreated
	ConVarRef blobs_paused( "blobs_paused" );
	bool bOldBlobPauseState = blobs_paused.GetBool();
	blobs_paused.SetValue( false );

	pRestore->StartBlock();

	int count = pRestore->ReadInt();
	if ( count > 0 )
	{
		EHANDLE hPaintStream;
		pRestore->ReadEHandle( &hPaintStream );
		CPaintStream *pPaintStream = dynamic_cast< CPaintStream* >( hPaintStream.Get() );

		int power = pRestore->ReadInt();
		for ( int i = 0; i < count; ++i )
		{
			Vector vPosition;
			pRestore->ReadVector( &vPosition, 1 );
			Vector vVelocity;
			pRestore->ReadVector( &vVelocity, 1 );
			float flStreakTime;
			pRestore->ReadFloat( &flStreakTime );
			float flStreakSpeedDampen;
			pRestore->ReadFloat( &flStreakSpeedDampen );

			if ( pPaintStream )
			{
				// Reconstruction note: the 2010 build called PaintBlobCreate(); the
				// imported code allocates blobs from the PaintStreamManager pool.
				CPaintBlob *pBlob = PaintStreamManager.AllocatePaintBlob( false );
				if ( pBlob )
				{
					pBlob->Init( vPosition, vVelocity, power, flStreakTime, flStreakSpeedDampen, pPaintStream, false, false );
					pPaintStream->AddPaintBlob( pBlob );
				}
			}
		}
	}

	pRestore->EndBlock();

	blobs_paused.SetValue( bOldBlobPauseState );
}
