//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the paint bomb, drawn as a ball of paint blobs.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_prop_paint_bomb.h"
#include "c_paintblob.h"
#include "c_paintblob_render.h"
#include "paint_stream_manager.h"
#include "paint_sprayer_shared.h"
#include "view.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar paintbomb_draw_sphere_radius;
extern ConVar paintblob_update_per_second;
extern ConVar draw_paint_isosurface;
extern ConVar paintblob_draw_distance_from_eye;
extern ConVar paintblob_isosurface_box_width;


IMPLEMENT_CLIENTCLASS_DT( C_PropPaintBomb, DT_PropPaintBomb, CPropPaintBomb )
	RecvPropInt( RECVINFO( m_nPaintPowerType ) ),
END_RECV_TABLE()


ConVar cl_draw_paint_bomb_with_blobs( "cl_draw_paint_bomb_with_blobs", "1", 0 );

C_PropPaintBomb::C_PropPaintBomb()
{
	m_nPaintPowerType = NO_POWER;
	m_nPrevPaintPowerType = NO_POWER;
	m_flAccumulatedTime = 0.0f;
	m_flLastUpdateTime = gpGlobals->curtime;
}


C_PropPaintBomb::~C_PropPaintBomb()
{
}


void C_PropPaintBomb::Precache()
{
	BaseClass::Precache();
}


void C_PropPaintBomb::Spawn()
{
	BaseClass::Spawn();

	memset( m_lightingBoxColors, 0, sizeof( m_lightingBoxColors ) );

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}


void C_PropPaintBomb::UpdateOnRemove()
{
	// Reconstruction note: the 2010 build lowered the stream manager's blob
	// count and deleted the blobs; the imported manager owns a blob pool, so
	// the blobs made by CreateBlobsForPaintBomb are handed back to it.
	for( int i = 0; i < m_PaintBlobs.Count(); ++i )
	{
		PaintStreamManager.FreeBlob( m_PaintBlobs[i] );
	}
	m_PaintBlobs.Purge();

	BaseClass::UpdateOnRemove();
}


void C_PropPaintBomb::OnPreDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnPreDataChanged( updateType );

	m_nPrevPaintPowerType = m_nPaintPowerType;
}


void C_PropPaintBomb::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		if( cl_draw_paint_bomb_with_blobs.GetBool() )
		{
			CreateBlobsForPaintBomb( GetAbsOrigin(), m_nPaintPowerType, m_PaintBlobs );
		}

		m_vPrevPosition = GetAbsOrigin();
	}
}


void C_PropPaintBomb::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		// The blobs are drawn by DrawModel, not the studio fast path
		m_bCanUseFastPath = false;
	}
}


void C_PropPaintBomb::UpdateBlobs( float flDeltaTime )
{
	Vector vecNewBombPos = GetAbsOrigin();
	float flMaxDistanceFromBomb = paintbomb_draw_sphere_radius.GetFloat();

	int numBlobs = m_PaintBlobs.Count();
	for( int i = 0; i < numBlobs; ++i )
	{
		CBasePaintBlob *pBlob = m_PaintBlobs[i];

		// Move the blob with the bomb
		Vector vecBombPosOffset = vecNewBombPos - m_vPrevPosition;

		Vector vecNewBlobPos = pBlob->GetPosition() + vecBombPosOffset;

		// Turn the blobs around when they leave the bomb's sphere
		Vector vecBlobDir = pBlob->GetVelocity();
		if( ( vecNewBlobPos - vecNewBombPos ).LengthSqr() > flMaxDistanceFromBomb * flMaxDistanceFromBomb &&
			DotProduct( vecBlobDir, vecNewBombPos - vecNewBlobPos ) < 0.0f )
		{
			pBlob->SetVelocity( -vecBlobDir );
		}

		vecNewBlobPos = vecNewBlobPos + vecBlobDir * flDeltaTime;
		pBlob->SetPosition( vecNewBlobPos );
	}

	m_vPrevPosition = vecNewBombPos;
}


void C_PropPaintBomb::ClientThink()
{
	BaseClass::ClientThink();

	m_flAccumulatedTime += gpGlobals->curtime - m_flLastUpdateTime;
	m_flLastUpdateTime = gpGlobals->curtime;

	// Update the blobs at a fixed rate
	float flDeltaTime = 1.0f / paintblob_update_per_second.GetFloat();
	while( m_flAccumulatedTime > 0.0f )
	{
		flDeltaTime = MIN( m_flAccumulatedTime, flDeltaTime );
		UpdateBlobs( flDeltaTime );
		m_flAccumulatedTime -= flDeltaTime;
	}

	// Reconstruction note: the decompiled argument here is unreadable; Spawn
	// passes CLIENT_THINK_ALWAYS and the bomb updates every frame.
	SetNextClientThink( CLIENT_THINK_ALWAYS );
}


int C_PropPaintBomb::DrawModel( int flags, const RenderableInstance_t &instance )
{
	int nCount = m_PaintBlobs.Count();
	if( !draw_paint_isosurface.GetBool() || nCount == 0 )
		return 0;

	Vector vCenter = vec3_origin;
	for( int i = 0; i < nCount; ++i )
	{
		vCenter += m_PaintBlobs[i]->GetPosition();
	}
	vCenter /= nCount;

	modelrender->SetupLighting( vCenter );
	NPaintRenderer::Paintblob_SafeLightCubeUpdate( vCenter, m_lightingBoxColors );
	if( materials && materials->GetRenderContext() )
	{
		materials->GetRenderContext()->SetAmbientLightCube( m_lightingBoxColors );
	}

	// Don't draw the blobs that are too close to the player's eye
	C_BasePlayer *pPlayer = GetSplitScreenViewPlayer();
	Vector vecPlayerPos = pPlayer->EyePosition();
	float flDistThreshold = paintblob_draw_distance_from_eye.GetFloat();

	int iNumParticles = 0;
	g_PaintblobRenderParticles.SetCount( nCount );
	for( int i = 0; i < nCount; ++i )
	{
		CBasePaintBlob *pBlob = m_PaintBlobs[i];
		Vector vecWorldCenter = pBlob->GetPosition();
		if( vecWorldCenter.DistToSqr( vecPlayerPos ) > flDistThreshold * flDistThreshold )
		{
			ImpParticleWithOneInterpolant *imp_particle = &g_PaintblobRenderParticles[iNumParticles];
			imp_particle->center = vecWorldCenter;
			imp_particle->setFieldScale( pBlob->GetRadiusScale() );
			imp_particle->interpolants1.set( 1.0f, 1.0f, 1.0f );
			++iNumParticles;
		}
	}
	g_PaintblobRenderParticles.SetCountNonDestructively( iNumParticles );

	IMaterial *pMaterial = materials->FindMaterial( PaintStreamManager.GetPaintMaterialName( m_nPaintPowerType ), TEXTURE_GROUP_OTHER, true );

	PortalMatrixList_t portalMatrixList;
	NPaintRenderer::Paintblob_Draw( BLOB_RENDER_BLOBULATOR, GetClientRenderable(), vCenter, pMaterial, paintblob_isosurface_box_width.GetFloat(), portalMatrixList, false );

	return 1;
}


void C_PropPaintBomb::CleansePaintPower()
{
	// Both 2010 builds leave this empty: the server owns the bomb's paint type
	// and the change arrives through m_nPaintPowerType.
}
