//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client paint stream: owns and draws a group of paint blobs.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#if 0
#include "cbase.h"
#include "c_paint_stream.h"
#include "c_paintblob.h"
#include "c_paintblob_render.h"
#include "paint_stream_manager.h"
#include "paint_sprayer_shared.h"
#include "c_portal_base2d.h"
#include "portal_util_shared.h"
#include "debugoverlay_shared.h"
#include "view.h"
#include "view_scene.h"
#include "viewrender.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "portal2_engine_compat.h"
#include "tier0/memdbgon.h"

ConVar draw_paint_isosurface( "draw_paint_isosurface", "1", FCVAR_CHEAT );
ConVar draw_paint_client_blobs( "draw_paint_client_blobs", "0", FCVAR_CHEAT );
ConVar paintblob_draw_distance_from_eye( "paintblob_draw_distance_from_eye", "18.0f", FCVAR_CHEAT );
ConVar paintblob_isosurface_box_width( "paintblob_isosurface_box_width", "8.0f", FCVAR_CHEAT );

// The offset is to make client time be behind the server update time
ConVar paintblob_interpolation_time_offset( "paintblob_interpolation_time_offset", "0.1f", FCVAR_CHEAT, "The offset is to make client time be behind the server update time, so blobs don't always take last update frame from the server." );

IMPLEMENT_CLIENTCLASS_DT( C_PaintStream, DT_PaintStream, CPaintStream )
	RecvPropInt( RECVINFO( m_nPaintType ) ),
	RecvPropInt( RECVINFO( m_nRenderMode ) ),
	// Reconstruction note: not in the 2010 table; received to match the imported
	// server stream (game/server/portal2/paint_stream.cpp).
	RecvPropVector( RECVINFO( m_vLightPosition ) ),
	RecvPropInt( RECVINFO( m_nMaxBlobCount ) ),
END_RECV_TABLE()

LINK_ENTITY_TO_CLASS( paint_stream, C_PaintStream );

IMPLEMENT_AUTO_LIST( IPaintStreamAutoList );

// Extra room around the blob positions for the stream's render bounds.
// Reconstruction note: the binaries only record the literal 18 units.
static const float PAINT_STREAM_BOUNDS_BLOAT = 18.0f;

C_PaintStream::C_PaintStream()
{
	m_nCurrentUpdateFrame = 0;
	m_nMaxBlobCount = 0;

	RemoveFromLeafSystem();
}


C_PaintStream::~C_PaintStream()
{
	RemoveAllPaintBlobs();
}


void C_PaintStream::Spawn()
{
	m_bCanUseFastPath = false;

	memset( m_lightingBoxColors, 0, sizeof( m_lightingBoxColors ) );

	m_iCachedWorldBoundsUpdateTick = gpGlobals->tickcount;

	RemoveFromLeafSystem();

	BaseClass::Spawn();
}


//-----------------------------------------------------------------------------
// Purpose: Reconstruction note: not in the 2010 unit. The imported
//			PaintStreamManager allocates blobs from a pool that has to be
//			created with the stream's blob budget, which the client only
//			knows once the first network update has arrived.
//-----------------------------------------------------------------------------
void C_PaintStream::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		PaintStreamManager.AllocatePaintBlobPool( m_nMaxBlobCount );
	}
}


void C_PaintStream::DebugDrawBlobs()
{
	bool bDebugDraw = draw_paint_client_blobs.GetBool();
	Color debugColor( 0, 0, 0 );

	if ( bDebugDraw )
	{
		debugColor = Color( 0, 255, 255 );

		for ( int i = 0; i < m_blobs.Count(); ++i )
		{
			NDebugOverlay::Cross3D( m_blobs[i]->GetPosition(), 2.0f, debugColor.r(), debugColor.g(), debugColor.b(), false, 0.1f );
		}
	}
}


const QAngle& C_PaintStream::GetRenderAngles()
{
	return vec3_angle;
}


const Vector& C_PaintStream::GetRenderOrigin()
{
	return m_vCachedRenderOrigin;
}


void C_PaintStream::GetRenderBoundsWorldspace( Vector& absMins, Vector& absMaxs )
{
	absMins = m_vCachedWorldMins;
	absMaxs = m_vCachedWorldMaxs;
}


void C_PaintStream::GetRenderBounds( Vector& theMins, Vector& theMaxs )
{
	theMins = m_vCachedWorldMins - m_vCachedRenderOrigin;
	theMaxs = m_vCachedWorldMaxs - m_vCachedRenderOrigin;
}


const Vector& C_PaintStream::WorldSpaceCenter() const
{
	// the render origin is the center of the blobs' bounds
	return m_vCachedRenderOrigin;
}


void C_PaintStream::ComputeWorldSpaceSurroundingBox( Vector *pMins, Vector *pMaxs )
{
	GetRenderBoundsWorldspace( *pMins, *pMaxs );
}


void C_PaintStream::AddPaintBlob( CPaintBlob* pBlob )
{
	if ( pBlob )
	{
		m_blobs.AddToTail( pBlob );
	}
}


void C_PaintStream::UpdateRenderBoundsAndOriginWorldspace()
{
	if ( m_iCachedWorldBoundsUpdateTick == gpGlobals->tickcount )
		return;

	m_iCachedWorldBoundsUpdateTick = gpGlobals->tickcount;

	if ( Portal2Engine::IsClientLocalToActiveServer() )
	{
		// the listen server simulates the blobs, use its shared data
		BlobDataVector_t blobData;
		GetCurrentBlobData( blobData );

		if ( blobData.Count() == 0 )
		{
			m_vCachedWorldMaxs = m_vCachedWorldMins = m_vCachedRenderOrigin = Vector( 0.f, 0.f, 0.f );
			RemoveFromLeafSystem();
		}
		else
		{
			Vector vecPos = blobData[0].m_vPosition;

			m_vCachedWorldMins = vecPos;
			m_vCachedWorldMaxs = vecPos;

			for ( int i = 1; i < blobData.Count(); ++i )
			{
				VectorMin( blobData[i].m_vPosition, m_vCachedWorldMins, m_vCachedWorldMins );
				VectorMax( blobData[i].m_vPosition, m_vCachedWorldMaxs, m_vCachedWorldMaxs );
			}

			m_vCachedRenderOrigin = 0.5f * ( m_vCachedWorldMins + m_vCachedWorldMaxs );

			Vector vRadius = ReplicateToVector( PAINT_STREAM_BOUNDS_BLOAT );
			m_vCachedWorldMins -= vRadius;
			m_vCachedWorldMaxs += vRadius;

			CCollisionProperty *prop = CollisionProp();
			prop->MarkSurroundingBoundsDirty();

			AddToLeafSystem();
		}

		UpdateVisibility();
	}
	else
	{
		if ( m_blobs.Count() == 0 )
		{
			m_vCachedWorldMaxs = m_vCachedWorldMins = m_vCachedRenderOrigin = Vector( 0.f, 0.f, 0.f );
			RemoveFromLeafSystem();
		}
		else
		{
			Vector vecPos = m_blobs[0]->GetPosition();

			m_vCachedWorldMins = vecPos;
			m_vCachedWorldMaxs = vecPos;

			for ( int i = 1; i < m_blobs.Count(); ++i )
			{
				VectorMin( m_blobs[i]->GetPosition(), m_vCachedWorldMins, m_vCachedWorldMins );
				VectorMax( m_blobs[i]->GetPosition(), m_vCachedWorldMaxs, m_vCachedWorldMaxs );
			}

			m_vCachedRenderOrigin = 0.5f * ( m_vCachedWorldMins + m_vCachedWorldMaxs );

			Vector vRadius = ReplicateToVector( PAINT_STREAM_BOUNDS_BLOAT );
			m_vCachedWorldMins -= vRadius;
			m_vCachedWorldMaxs += vRadius;

			CCollisionProperty *prop = CollisionProp();
			prop->MarkSurroundingBoundsDirty();

			AddToLeafSystem();
		}

		UpdateVisibility();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Collect the transforms of the portals that intersect a box
//-----------------------------------------------------------------------------
void BuildPortalMatrixListInAABB( const Vector& vCenter, const Vector& vExtents, PortalMatrixList_t& portalMatrixList )
{
	for ( int i = 0; i < CPortal_Base2D_Shared::AllPortals.Count(); ++i )
	{
		C_Portal_Base2D *pPortal = CPortal_Base2D_Shared::AllPortals[i];
		if ( UTIL_IsBoxIntersectingPortal( vCenter, vExtents, pPortal, 0.0f ) )
		{
			// push the matrix back a bit behind the portal surface
			matrix3x4_t matTransform;
			AngleMatrix( pPortal->GetAbsAngles(), pPortal->GetAbsOrigin() - 5.0f * pPortal->m_plane_Origin.normal, matTransform );
			portalMatrixList.AddToTail( matTransform );
		}
	}
}


void C_PaintStream::DrawBlobs( IMaterial *pMaterial )
{
	int nCount;
	Vector vCenter = vec3_origin;

	PortalMatrixList_t portalMatrixList;
	BuildPortalMatrixListInAABB( m_vCachedRenderOrigin, 0.5f * ( m_vCachedWorldMaxs - m_vCachedWorldMins ), portalMatrixList );

	if ( Portal2Engine::IsClientLocalToActiveServer() )
	{
		// the listen server simulates the blobs, draw its interpolated shared data
		BlobDataVector_t blobData;
		GetCurrentBlobData( blobData );

		nCount = blobData.Count();
		if ( nCount == 0 || !draw_paint_isosurface.GetBool() )
			return;

		for ( int i = 0; i < nCount; ++i )
		{
			vCenter += blobData[i].m_vPosition;
		}
		vCenter /= nCount;

		modelrender->SetupLighting( vCenter );
		NPaintRenderer::Paintblob_SafeLightCubeUpdate( vCenter, m_lightingBoxColors );
		if ( materials && materials->GetRenderContext() )
		{
			materials->GetRenderContext()->SetAmbientLightCube( m_lightingBoxColors );
		}

		C_BasePlayer *pPlayer = GetSplitScreenViewPlayer();
		if ( !pPlayer )
			return;

		// don't draw the blobs that are too close to the player's eye
		Vector vecPlayerPos = pPlayer->EyePosition();
		float flDistThreshold = paintblob_draw_distance_from_eye.GetFloat();

		int iNumParticles = 0;
		g_PaintblobRenderParticles.SetCount( nCount );
		for ( int i = 0; i < nCount; ++i )
		{
			Vector vecWorldCenter = blobData[i].m_vPosition;
			if ( vecWorldCenter.DistToSqr( vecPlayerPos ) > flDistThreshold * flDistThreshold )
			{
				ImpParticleWithOneInterpolant *imp_particle = &g_PaintblobRenderParticles[iNumParticles];
				imp_particle->center = vecWorldCenter;
				imp_particle->setFieldScale( blobData[i].m_flScale );
				imp_particle->interpolants1.set( 1.0f, 1.0f, 1.0f );
				++iNumParticles;
			}

			// Reconstruction note: the 2010 server appended a separate entry for
			// ghosting blobs; the imported BlobData_t carries the ghost transform.
			if ( blobData[i].m_bGhosting )
			{
				ImpParticleWithOneInterpolant *imp_particle = &g_PaintblobRenderParticles[iNumParticles];
				imp_particle->center = blobData[i].m_matGhostTransform * vecWorldCenter;
				imp_particle->setFieldScale( blobData[i].m_flScale );
				imp_particle->interpolants1.set( 1.0f, 1.0f, 1.0f );
				++iNumParticles;
				g_PaintblobRenderParticles.AddToTail();
			}
		}
		g_PaintblobRenderParticles.SetCountNonDestructively( iNumParticles );

		NPaintRenderer::Paintblob_Draw( m_nRenderMode, GetClientRenderable(), vCenter, pMaterial, paintblob_isosurface_box_width.GetFloat(), portalMatrixList, false );
	}
	else
	{
		nCount = m_blobs.Count();
		if ( nCount == 0 || !draw_paint_isosurface.GetBool() )
			return;

		for ( int i = 0; i < nCount; ++i )
		{
			vCenter += m_blobs[i]->GetPosition();
		}
		vCenter /= nCount;

		modelrender->SetupLighting( vCenter );
		NPaintRenderer::Paintblob_SafeLightCubeUpdate( vCenter, m_lightingBoxColors );
		if ( materials && materials->GetRenderContext() )
		{
			materials->GetRenderContext()->SetAmbientLightCube( m_lightingBoxColors );
		}

		C_BasePlayer *pPlayer = GetSplitScreenViewPlayer();
		if ( !pPlayer )
			return;

		// don't draw the blobs that are too close to the player's eye
		Vector vecPlayerPos = pPlayer->EyePosition();
		float flDistThreshold = paintblob_draw_distance_from_eye.GetFloat();

		int iNumParticles = 0;
		g_PaintblobRenderParticles.SetCount( nCount );
		for ( int i = 0; i < nCount; ++i )
		{
			CPaintBlob *pBlob = m_blobs[i];
			if ( pBlob->IsSilent() )
				continue;

			Vector vecWorldCenter = pBlob->GetPosition();
			if ( vecWorldCenter.DistToSqr( vecPlayerPos ) > flDistThreshold * flDistThreshold )
			{
				ImpParticleWithOneInterpolant *imp_particle = &g_PaintblobRenderParticles[iNumParticles];
				imp_particle->center = vecWorldCenter;
				imp_particle->setFieldScale( pBlob->GetRadiusScale() );
				imp_particle->interpolants1.set( 1.0f, 1.0f, 1.0f );
				++iNumParticles;
			}

			// draw a second copy of the blob on the other side of the portal it is going through
			if ( pBlob->IsGhosting() )
			{
				// Reconstruction note: the 2010 blob stored its ghost position; the
				// imported blob exposes the portal transform instead.
				VMatrix matGhostTransform;
				pBlob->GetGhostMatrix( matGhostTransform );

				ImpParticleWithOneInterpolant *imp_particle = &g_PaintblobRenderParticles[iNumParticles];
				imp_particle->center = matGhostTransform * vecWorldCenter;
				imp_particle->setFieldScale( pBlob->GetRadiusScale() );
				imp_particle->interpolants1.set( 1.0f, 1.0f, 1.0f );
				++iNumParticles;
				g_PaintblobRenderParticles.AddToTail();
			}
		}
		g_PaintblobRenderParticles.SetCountNonDestructively( iNumParticles );

		NPaintRenderer::Paintblob_Draw( m_nRenderMode, GetClientRenderable(), vCenter, pMaterial, paintblob_isosurface_box_width.GetFloat(), portalMatrixList, false );
	}
}


int C_PaintStream::DrawModel( int flags, const RenderableInstance_t &instance )
{
	if ( !m_bReadyToDraw )
		return 0;

	if ( m_blobs.Count() == 0 && !Portal2Engine::IsClientLocalToActiveServer() )
		return 0;

	IMaterial *pMaterial = materials->FindMaterial( PaintStreamManager.GetPaintMaterialName( m_nPaintType ), TEXTURE_GROUP_OTHER, true );

	// refract the frame buffer
	if ( CurrentViewID() != VIEW_SHADOW_DEPTH_TEXTURE && pMaterial->NeedsPowerOfTwoFrameBufferTexture() )
	{
		const CViewSetup *pViewSetup = view->GetViewSetup();
		UpdateScreenEffectTexture( 0, pViewSetup->x, pViewSetup->y, pViewSetup->width, pViewSetup->height );
	}

	DrawBlobs( pMaterial );

	return 1;
}


//-----------------------------------------------------------------------------
// Purpose: Per-stream update used by the 2010 build.
// Reconstruction note: the imported PaintStreamManager simulates every stream's
// blobs in one batch through PreUpdateBlobs()/PostUpdateBlobs(); this keeps the
// 2010 single-stream entry point, expressed with the same helpers.
//-----------------------------------------------------------------------------
void C_PaintStream::Update()
{
	if ( m_nCurrentUpdateFrame == gpGlobals->framecount )
		return;

	// the listen server simulates the blobs
	if ( !Portal2Engine::IsClientLocalToActiveServer() && m_blobs.Count() != 0 )
	{
		RemoveDeadBlobs();
		DebugDrawBlobs();

		PaintBlobUpdate( m_blobs );
	}

	UpdateRenderBoundsAndOriginWorldspace();

	m_nCurrentUpdateFrame = gpGlobals->framecount;
}


//-----------------------------------------------------------------------------
// Purpose: Interpolate a blob that went through one or more portals between
//			two server updates along the path through the portals
//-----------------------------------------------------------------------------
void InterpolateTeleportingBlob( float flPercent, const BlobData_t& blobA, const BlobData_t& blobB, BlobDataVector_t& blobDataVector )
{
	const BlobTeleportationHistoryVector_t& history = blobB.m_teleportationHistory;

	int nHistoryCount = history.Count();

	// length of each segment of the path: start to first portal, portal to portal, last portal to end
	float *distStack = (float*)stackalloc( ( nHistoryCount + 1 ) * sizeof( float ) );

	const int nLastHistory = nHistoryCount - 1;

	float flTotalDistance = distStack[0] = ( history[0].m_vEnterPosition - blobA.m_vPosition ).Length();

	for ( int i = 1; i < nHistoryCount; ++i )
	{
		distStack[i] = ( history[i].m_vEnterPosition - history[i - 1].m_vExitPosition ).Length();
		flTotalDistance += distStack[i];
	}

	distStack[nHistoryCount] = ( blobB.m_vPosition - history[nLastHistory].m_vExitPosition ).Length();
	flTotalDistance += distStack[nHistoryCount];

	float flQueryDist = flPercent * flTotalDistance;

	BlobData_t blobData;
	blobData.m_blobID = blobA.m_blobID;
	blobData.m_flScale = Lerp( flPercent, blobA.m_flScale, blobB.m_flScale );

	// before the first portal
	if ( flQueryDist < distStack[0] )
	{
		float flPositionPercent = flQueryDist / distStack[0];
		blobData.m_vPosition = Lerp( flPositionPercent, blobA.m_vPosition, history[0].m_vEnterPosition );
		blobDataVector.AddToTail( blobData );
		return;
	}

	// between two portals
	// Reconstruction note: the binary compares the query distance with the
	// individual segment lengths, not the accumulated distance; kept as is.
	for ( int i = 0; i < nLastHistory; ++i )
	{
		if ( distStack[i] <= flQueryDist && flQueryDist < distStack[i + 1] )
		{
			float flPositionPercent = ( flQueryDist - distStack[i] ) / distStack[i + 1];
			blobData.m_vPosition = Lerp( flPositionPercent, history[i].m_vExitPosition, history[i + 1].m_vEnterPosition );
			blobDataVector.AddToTail( blobData );
			return;
		}
	}

	// after the last portal
	float flPositionPercent = ( flQueryDist - distStack[nLastHistory] ) / distStack[nHistoryCount];
	blobData.m_vPosition = Lerp( flPositionPercent, history[nLastHistory].m_vExitPosition, blobB.m_vPosition );
	blobDataVector.AddToTail( blobData );
}


//-----------------------------------------------------------------------------
// Purpose: Interpolate between two server updates. Both lists are sorted by blob ID.
//-----------------------------------------------------------------------------
void InterpolateBlobData( float flPercent, const BlobDataVector_t& dataA, const BlobDataVector_t& dataB, BlobDataVector_t& blobDataVector )
{
	int a = 0;
	int b = 0;
	while ( a < dataA.Count() || b < dataB.Count() )
	{
		const BlobData_t *blobA = ( a < dataA.Count() ) ? &dataA[a] : NULL;
		const BlobData_t *blobB = ( b < dataB.Count() ) ? &dataB[b] : NULL;

		if ( blobA && blobB )
		{
			if ( blobA->m_blobID == blobB->m_blobID )
			{
				if ( blobB->m_bTeleportedThisFrame )
				{
					InterpolateTeleportingBlob( flPercent, *blobA, *blobB, blobDataVector );
				}
				else
				{
					BlobData_t blobData;
					blobData.m_blobID = blobA->m_blobID;
					blobData.m_flScale = Lerp( flPercent, blobA->m_flScale, blobB->m_flScale );
					blobData.m_vPosition = Lerp( flPercent, blobA->m_vPosition, blobB->m_vPosition );
					blobDataVector.AddToTail( blobData );
				}

				++a;
				++b;
			}
			else if ( blobA->m_blobID < blobB->m_blobID )
			{
				// the blob was removed in the newer update
				blobDataVector.AddToTail( *blobA );
				++a;
			}
			else
			{
				// the blob was created in the newer update
				++b;
			}
		}
		else if ( blobA )
		{
			blobDataVector.AddToTail( *blobA );
			++a;
		}
		else
		{
			++b;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Get the blob data shared by the listen server at the current
//			(slightly delayed) client time
//-----------------------------------------------------------------------------
void C_PaintStream::GetCurrentBlobData( BlobDataVector_t& blobData )
{
	// Reconstruction note: the 2010 client read the shared data without a lock;
	// the imported server rewrites it under m_sharedBlobDataMutex.
	AUTO_LOCK( m_sharedBlobDataMutex.GetForModify() );

	int nTotalFrame = m_sharedBlobData.Count();
	if ( nTotalFrame == 0 )
		return;

	const float flCurrentTime = gpGlobals->curtime - paintblob_interpolation_time_offset.GetFloat();

	// don't have data that old yet
	float flOldestDataTime = m_sharedBlobData.Get( 0 ).m_flUpdateTime;
	if ( flCurrentTime < flOldestDataTime )
		return;

	// find the two updates around the current time
	int nCurrentFrame = 0;
	int nLastFrame = nTotalFrame - 1;
	float flTimeA = 0.0f;
	float flTimeB = 0.0f;
	for ( ; nCurrentFrame < nLastFrame; ++nCurrentFrame )
	{
		flTimeA = m_sharedBlobData[nCurrentFrame].m_flUpdateTime;
		flTimeB = m_sharedBlobData[nCurrentFrame + 1].m_flUpdateTime;
		if ( flTimeA < flCurrentTime && flCurrentTime < flTimeB )
			break;
	}

	if ( nCurrentFrame == nLastFrame )
	{
		// use the newest data
		const BlobDataVector_t& temp = m_sharedBlobData[nLastFrame].m_blobData;
		blobData.CopyArray( temp.Base(), temp.Count() );
	}
	else
	{
		const BlobDataVector_t& dataA = m_sharedBlobData[nCurrentFrame].m_blobData;
		const BlobDataVector_t& dataB = m_sharedBlobData[nCurrentFrame + 1].m_blobData;

		float flDeltaTime = flTimeB - flTimeA;
		float flPercent = ( flCurrentTime - flTimeA ) / flDeltaTime;
		InterpolateBlobData( flPercent, dataA, dataB, blobData );
	}
}
#endif
