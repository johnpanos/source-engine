//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the projected light bridge (hard light wall): the
//			predicted collision shape history, rendering (plain and painted) and
//			impact particles. Touch, painting and obstruction handling live in
//			projectedwallentity_shared.cpp.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_projectedwallentity.h"
#include "c_portal_base2d.h"
#include "c_prop_portal.h"
#include "c_basetempentity.h"
#include "c_baseplayer.h"
#include "prediction.h"
#include "physics_shared.h"
#include "vphysics_interface.h"
#include "vphysics/constraints.h"
#include "mathlib/polyhedron.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imesh.h"
#include "particle_parse.h"
#include "debugoverlay_shared.h"
#include "gamerules.h"
#include "portal_util_shared.h"
#include "paint_color_manager.h"
#include "toolframework_client.h"
#include "toolframework/itoolentity.h"
#include "tier1/KeyValues.h"
#include "precache_register.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar cl_projected_wall_projection_speed( "cl_projected_wall_projection_speed", "150" );
ConVar cl_paintable_projected_wall_texture_wrap_rate( "cl_paintable_projected_wall_texture_wrap_rate", "64.0f" );
ConVar cl_draw_projected_wall_with_paint( "cl_draw_projected_wall_with_paint", "1" );

ConVar debug_projected_wall_drawing( "debug_projected_wall_drawing", "0" );

extern ConVar sv_thinnerprojectedwalls;


PRECACHE_REGISTER_BEGIN( GLOBAL, PrecacheEffectProjectedWallEntity )
	PRECACHE( MATERIAL, "effects/projected_wall" )
	PRECACHE( MATERIAL, "effects/projected_wall_rail" )
	PRECACHE( MATERIAL, "paint/bridge_paint_end_right" )
	PRECACHE( MATERIAL, "paint/bridge_paint_end_left" )
	PRECACHE( MATERIAL, "paint/bridge_paint_tile" )
	PRECACHE( MATERIAL, "paint/bridge_paint_single" )
PRECACHE_REGISTER_END()


IMPLEMENT_AUTO_LIST( IProjectedWallEntityAutoList );

IMPLEMENT_CLIENTCLASS_DT( C_ProjectedWallEntity, DT_ProjectedWallEntity, CProjectedWallEntity )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropQAngles( RECVINFO_NAME( m_angNetworkAngles, m_angRotation ) ),
	RecvPropVector( RECVINFO( m_vWorldSpace_WallMins ) ),
	RecvPropVector( RECVINFO( m_vWorldSpace_WallMaxs ) ),
	RecvPropEHandle( RECVINFO( m_hColorPortal ) ),
	RecvPropFloat( RECVINFO( m_flLength ) ),
	RecvPropFloat( RECVINFO( m_flWidth ) ),
	RecvPropFloat( RECVINFO( m_flHeight ) ),
	RecvPropFloat( RECVINFO( m_flSegmentLength ) ),
	RecvPropFloat( RECVINFO( m_flParticleUpdateTime ) ),
	RecvPropInt( RECVINFO( m_nNumSegments ) ),
	RecvPropBool( RECVINFO( m_bIsHorizontal ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ProjectedWallEntity )
	DEFINE_PRED_FIELD( m_flLength, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flWidth, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flHeight, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_FIELD( m_fLastProjectedTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_flParticleUpdateTime, FIELD_FLOAT ),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( projected_wall_entity, C_ProjectedWallEntity );

C_ProjectedWallEntity::C_ProjectedWallEntity()
{
	m_pBodyMaterial = NULL;
	m_pPaintMaterialMid = NULL;
	m_pPaintMaterialEnd1 = NULL;
	m_pPaintMaterialEnd2 = NULL;
	m_pPaintMaterialSing = NULL;
	m_pSideRailMaterial = NULL;
	m_flCurDisplayLength = 0.0f;
	m_nNumSegments = 0;
	m_flSegmentLength = 0.0f;
	m_flParticleUpdateTime = 0.0f;
	m_flPrevParticleUpdateTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::UpdateOnRemove( void )
{
	StopParticleEffects( this );

	for ( int i = 0; i < m_WallCollideables.Count(); ++i )
	{
		physcollision->DestroyCollide( m_WallCollideables[i].pCollideable );
	}
	m_WallCollideables.RemoveAll();

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CollideType_t C_ProjectedWallEntity::GetCollideType( void )
{
	return ENTITY_SHOULD_COLLIDE;
}

//-----------------------------------------------------------------------------
// Purpose: Emit one textured quad centered on vOrigin
//-----------------------------------------------------------------------------
inline void C_ProjectedWallEntity::DrawQuadHelper( CMeshBuilder *meshBuilder, const Vector &vOrigin, const Vector &vRight, const Vector &vUp, float flTextureScaleU, float flTextureScaleV, float flAlpha )
{
	float flColors[4] = { 1.0f, 1.0f, 1.0f, flAlpha };

	Vector vert = vOrigin + vRight - vUp;
	meshBuilder->Color4fv( flColors );
	meshBuilder->TexCoord2f( 0, 0.0f, flTextureScaleV );
	meshBuilder->Position3fv( vert.Base() );
	meshBuilder->AdvanceVertex();

	vert = vOrigin + vRight + vUp;
	meshBuilder->Color4fv( flColors );
	meshBuilder->TexCoord2f( 0, 0.0f, 0.0f );
	meshBuilder->Position3fv( vert.Base() );
	meshBuilder->AdvanceVertex();

	vert = vOrigin - vRight + vUp;
	meshBuilder->Color4fv( flColors );
	meshBuilder->TexCoord2f( 0, flTextureScaleU, 0.0f );
	meshBuilder->Position3fv( vert.Base() );
	meshBuilder->AdvanceVertex();

	vert = vOrigin - vRight - vUp;
	meshBuilder->Color4fv( flColors );
	meshBuilder->TexCoord2f( 0, flTextureScaleU, flTextureScaleV );
	meshBuilder->Position3fv( vert.Base() );
	meshBuilder->AdvanceVertex();
}

//-----------------------------------------------------------------------------
// Purpose: Draw the bridge surface and its rails, or the per-segment paint layers
//-----------------------------------------------------------------------------
int C_ProjectedWallEntity::DrawModel( int flags, const RenderableInstance_t &instance )
{
	if ( !InitMaterials() )
		return 0;

	if ( cl_draw_projected_wall_with_paint.GetBool() && m_PaintPowers.Count() == m_nNumSegments )
	{
		CMatRenderContextPtr pRenderContext( materials );

		IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pBodyMaterial );
		CMeshBuilder meshBuilder;
		PaintWallWithPaint( pMesh, meshBuilder, BASE_SEGMENT );

		IMesh *pMeshMid = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pPaintMaterialMid );
		CMeshBuilder meshBuilderPaintMid;
		PaintWallWithPaint( pMeshMid, meshBuilderPaintMid, PAINT_MIDDLE_SEGMENT );

		IMesh *pMeshEnd1 = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pPaintMaterialEnd1 );
		CMeshBuilder meshBuilderPaintEnd1;
		PaintWallWithPaint( pMeshEnd1, meshBuilderPaintEnd1, PAINT_END_1_SEGMENT );

		IMesh *pMeshEnd2 = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pPaintMaterialEnd2 );
		CMeshBuilder meshBuilderPaintEnd2;
		PaintWallWithPaint( pMeshEnd2, meshBuilderPaintEnd2, PAINT_END_2_SEGMENT );

		IMesh *pMeshSing = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pPaintMaterialSing );
		CMeshBuilder meshBuilderPaintSing;
		PaintWallWithPaint( pMeshSing, meshBuilderPaintSing, PAINT_SINGLE_SEGMENT );
	}
	else
	{
		Vector vForward, vRight, vUp;
		AngleVectors( GetNetworkAngles(), &vForward, &vRight, &vUp );

		Vector vOrigin = GetStartPoint() + ( vForward * m_flCurDisplayLength ) / 2.0f;

		CMatRenderContextPtr pRenderContext( materials );

		IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pBodyMaterial );
		CMeshBuilder meshBuilder;

		// Reconstruction note: the build queries the game rules here and discards the result.
		bool bIsMultiplayer = GameRules()->IsMultiplayer();
		NOTE_UNUSED( bIsMultiplayer );

		// Half extents of the bridge
		vForward *= ( m_flCurDisplayLength / 2.0f );
		vRight *= ( m_flWidth / 2.0f );
		vUp *= ( m_flHeight / 2.0f );

		float flLengthTexRate = m_flCurDisplayLength / 128.0f;
		float flWidthTextRate = 1.0f;

		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 5 );

		// Top surface
		DrawQuadHelper( &meshBuilder, vOrigin + vUp, vRight, vForward, flWidthTextRate, flLengthTexRate, 0.1f );

		meshBuilder.End();
		pMesh->Draw();

		// Side rails
		pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pSideRailMaterial );
		CMeshBuilder meshBuilder2;

		meshBuilder2.Begin( pMesh, MATERIAL_QUADS, 5 );

		AngleVectors( GetNetworkAngles(), NULL, NULL, &vUp );
		vUp *= 8.0f;
		DrawQuadHelper( &meshBuilder2, vOrigin + vRight, vUp, vForward, flWidthTextRate, flLengthTexRate, 1.0f );
		DrawQuadHelper( &meshBuilder2, vOrigin - vRight, vUp, vForward, flWidthTextRate, flLengthTexRate, 1.0f );

		meshBuilder2.End();
		pMesh->Draw();
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Draw every segment of the wall that uses the given paint layer
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::PaintWallWithPaint( IMesh *pMesh, CMeshBuilder meshBuilder, ProjectedWallSegmentType type )
{
	Vector vecForward, vecRight, vecUp, vecForwardDir, vecRightDir, vecUpDir;
	AngleVectors( GetNetworkAngles(), &vecForwardDir, &vecRightDir, &vecUpDir );
	vecRight = vecRightDir * m_flWidth / 2.0f;
	vecUp = vecUpDir * m_flHeight / 2.0f;
	Vector vecOrigin = GetStartPoint();

	bool bDebugging = debug_projected_wall_drawing.GetBool();

	if ( bDebugging )
	{
		Vector vecWallEnd = vecOrigin + vecForwardDir * m_flCurDisplayLength;
		NDebugOverlay::Sphere( vecWallEnd, 5.0f, 0, 255, 0, true, 0.1f );
	}

	float flDrawnLength = 0.0f;
	float flPrevSegmentLength;
	float flCurrentSegmentLength = m_flSegmentLength;

	// The first segment is centered half a segment down the wall (or on a shorter wall)
	if ( m_flCurDisplayLength <= m_flSegmentLength )
	{
		flCurrentSegmentLength = m_flCurDisplayLength;
		vecForward = vecForwardDir * flCurrentSegmentLength / 2.0f;
		vecOrigin += vecForwardDir * m_flCurDisplayLength / 2.0f;
	}
	else
	{
		vecForward = vecForwardDir * m_flSegmentLength / 2.0f;
		vecOrigin += vecForwardDir * m_flSegmentLength / 2.0f;
	}

	// Reconstruction note: the build also declares vecOriginalOrigin/vecOriginalForward here;
	// they are never used.

	for ( int i = 0; i < m_nNumSegments; ++i )
	{
		if ( type == BASE_SEGMENT )
		{
			PaintWallWithPaintSegment( pMesh, meshBuilder, BASE_SEGMENT, vecOrigin, vecUp, vecForward, vecRight );
		}
		else if ( SetPaintSurfaceColor( m_PaintPowers[i] ) )
		{
			int nPrevSeg = i - 1;
			int nNextSeg = i + 1;

			if ( type == PAINT_SINGLE_SEGMENT )
			{
				// Painted segment with no painted neighbours
				if ( ( nPrevSeg == -1 || m_PaintPowers[nPrevSeg] == NO_POWER ) &&
					 ( m_PaintPowers[nNextSeg] == NO_POWER || nNextSeg >= m_nNumSegments ) )
				{
					PaintWallWithPaintSegment( pMesh, meshBuilder, PAINT_SINGLE_SEGMENT, vecOrigin, vecUp, vecForward, vecRight );
				}
			}
			else if ( type == PAINT_END_1_SEGMENT )
			{
				// First segment of a painted run
				if ( ( nPrevSeg == -1 || m_PaintPowers[nPrevSeg] == NO_POWER ) &&
					 nNextSeg < m_nNumSegments && m_PaintPowers[nNextSeg] != NO_POWER )
				{
					PaintWallWithPaintSegment( pMesh, meshBuilder, PAINT_END_1_SEGMENT, vecOrigin, vecUp, vecForward, vecRight );
				}
			}
			else if ( type == PAINT_END_2_SEGMENT )
			{
				// Last segment of a painted run
				// Reconstruction note: the build only tests the previous segment's paint when it is
				// segment 0; any other previous index (including -1) passes.
				if ( ( nPrevSeg != 0 || m_PaintPowers[nPrevSeg] != NO_POWER ) &&
					 ( nNextSeg >= m_nNumSegments || m_PaintPowers[nNextSeg] == NO_POWER ) )
				{
					PaintWallWithPaintSegment( pMesh, meshBuilder, PAINT_END_2_SEGMENT, vecOrigin, vecUp, vecForward, vecRight );
				}
			}
			else if ( type == PAINT_MIDDLE_SEGMENT && nPrevSeg >= 0 && nNextSeg < m_nNumSegments &&
					  m_PaintPowers[nPrevSeg] != NO_POWER && m_PaintPowers[nNextSeg] != NO_POWER )
			{
				PaintWallWithPaintSegment( pMesh, meshBuilder, PAINT_MIDDLE_SEGMENT, vecOrigin, vecUp, vecForward, vecRight );
			}
		}

		if ( bDebugging )
		{
			Color color( 255, 0, 255 );
			if ( i % 2 == 0 )
			{
				color = Color( 0, 255, 255 );
			}

			Vector vecMins, vecMaxs;
			vecMins = -vecForwardDir * flCurrentSegmentLength / 2.0f;
			vecMins -= vecRightDir * m_flWidth / 2.0f;
			vecMins -= vecUpDir * 5.0f;
			vecMaxs = vecForwardDir * flCurrentSegmentLength / 2.0f;
			vecMaxs += vecRightDir * m_flWidth / 2.0f;
			vecMaxs += vecUpDir * 5.0f;

			NDebugOverlay::Box( vecOrigin, vecMins, vecMaxs, color.r(), color.g(), color.b(), 64, 0.1f );
			NDebugOverlay::Sphere( vecOrigin, 4.0f, 255, 0, 0, true, 0.1f );
		}

		// Step to the center of the next segment, shortening the last one to fit the wall
		flDrawnLength += flCurrentSegmentLength;
		flPrevSegmentLength = flCurrentSegmentLength;
		flCurrentSegmentLength = m_flSegmentLength;

		float flOverDraw = ( flDrawnLength + flCurrentSegmentLength ) - m_flCurDisplayLength;
		if ( flOverDraw > 0.0f )
		{
			flCurrentSegmentLength -= flOverDraw;
		}

		vecForward = vecForwardDir * flCurrentSegmentLength / 2.0f;
		vecOrigin += vecForwardDir * ( flPrevSegmentLength / 2.0f + flCurrentSegmentLength / 2.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw one segment's top and sides
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::PaintWallWithPaintSegment( IMesh *pMesh, CMeshBuilder meshBuilder, ProjectedWallSegmentType type, Vector vecOrigin, Vector vecUp, Vector vecForward, Vector vecRight )
{
	float flLengthTexRate = m_flSegmentLength / cl_paintable_projected_wall_texture_wrap_rate.GetFloat();
	float flWidthTextRate = 1.0f;

	// Lift the paint layers off the bridge surface
	Vector vecNewOrigin = vecOrigin;
	if ( type != BASE_SEGMENT )
	{
		vecNewOrigin.z += 0.75f;
	}

	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 5 );

	// Top
	DrawQuadHelper( &meshBuilder, vecNewOrigin + vecUp, vecRight, vecForward, flWidthTextRate, flLengthTexRate, 1.0f );

	// Sides
	DrawQuadHelper( &meshBuilder, vecNewOrigin + vecRight, vecUp, vecForward, flWidthTextRate, flLengthTexRate, 1.0f );
	DrawQuadHelper( &meshBuilder, vecNewOrigin - vecRight, vecUp, vecForward, flWidthTextRate, flLengthTexRate, 1.0f );

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: Tint the paint layers with the color of the portal the bridge came from
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::ColorWallByPortal( IMaterial *pBody )
{
	float flColors[3] = { 1.0f, 1.0f, 1.0f };

	if ( m_hColorPortal.Get() )
	{
		C_Prop_Portal *pPortal = static_cast< C_Prop_Portal * >( m_hColorPortal.Get() );
		if ( pPortal->m_hFiredByPlayer.Get() )
		{
			Color clrPortal = UTIL_Portal_Color( pPortal->m_bIsPortal2 ? 2 : 1, pPortal->GetTeamNumber() );
			flColors[0] = clrPortal.r() / 255.0f;
			flColors[1] = clrPortal.g() / 255.0f;
			flColors[2] = clrPortal.b() / 255.0f;
		}
	}

	m_pPaintColorMid->SetVecValue( flColors, 3 );
	m_pPaintColorEnd1->SetVecValue( flColors, 3 );
	m_pPaintColorEnd2->SetVecValue( flColors, 3 );
	m_pPaintColorSing->SetVecValue( flColors, 3 );
}

//-----------------------------------------------------------------------------
// Purpose: Find the bridge and paint materials once
//-----------------------------------------------------------------------------
bool C_ProjectedWallEntity::InitMaterials( void )
{
	if ( m_pBodyMaterial == NULL )
	{
		m_pBodyMaterial = materials->FindMaterial( "effects/projected_wall", NULL, false );
		if ( m_pBodyMaterial == NULL )
			return false;
	}

	if ( m_pPaintMaterialMid == NULL )
	{
		m_pPaintMaterialMid = materials->FindMaterial( "paint/bridge_paint_tile", NULL, false );
		if ( m_pPaintMaterialMid == NULL )
			return false;
	}

	if ( m_pPaintMaterialEnd1 == NULL )
	{
		m_pPaintMaterialEnd1 = materials->FindMaterial( "paint/bridge_paint_end_right", NULL, false );
		if ( m_pPaintMaterialEnd1 == NULL )
			return false;
	}

	if ( m_pPaintMaterialEnd2 == NULL )
	{
		m_pPaintMaterialEnd2 = materials->FindMaterial( "paint/bridge_paint_end_left", NULL, false );
		if ( m_pPaintMaterialEnd2 == NULL )
			return false;
	}

	if ( m_pPaintMaterialSing == NULL )
	{
		m_pPaintMaterialSing = materials->FindMaterial( "paint/bridge_paint_single", NULL, false );
		if ( m_pPaintMaterialSing == NULL )
			return false;
	}

	if ( m_pSideRailMaterial == NULL )
	{
		m_pSideRailMaterial = materials->FindMaterial( "effects/projected_wall_rail", NULL, false );
		if ( m_pSideRailMaterial == NULL )
			return false;
	}

	bool bFound = false;
	if ( m_pPaintColorMid == NULL )
	{
		m_pPaintColorMid = m_pPaintMaterialMid->FindVar( "$color", &bFound, true );
		if ( !bFound )
			return false;
	}

	bFound = false;
	if ( m_pPaintColorEnd1 == NULL )
	{
		m_pPaintColorEnd1 = m_pPaintMaterialEnd1->FindVar( "$color", &bFound, true );
		if ( !bFound )
			return false;
	}

	bFound = false;
	if ( m_pPaintColorEnd2 == NULL )
	{
		m_pPaintColorEnd2 = m_pPaintMaterialEnd2->FindVar( "$color", &bFound, true );
		if ( !bFound )
			return false;
	}

	bFound = false;
	if ( m_pPaintColorSing == NULL )
	{
		m_pPaintColorSing = m_pPaintMaterialSing->FindVar( "$color", &bFound, true );
		if ( !bFound )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::GetRenderBounds( Vector &vecMins, Vector &vecMaxs )
{
	vecMins = m_vWorldSpace_WallMins - GetRenderOrigin();
	vecMaxs = m_vWorldSpace_WallMaxs - GetRenderOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: The render bounds are in world space
//-----------------------------------------------------------------------------
const QAngle &C_ProjectedWallEntity::GetRenderAngles( void )
{
	return vec3_angle;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
RenderableTranslucencyType_t C_ProjectedWallEntity::ComputeTranslucencyType( void )
{
	return RENDERABLE_IS_TRANSLUCENT;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::OnPreDataChanged( DataUpdateType_t type )
{
	BaseClass::OnPreDataChanged( type );

	m_flPrevParticleUpdateTime = m_flParticleUpdateTime;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		InitMaterials();
		SetupWallParticles();
	}

	if ( m_flParticleUpdateTime != m_flPrevParticleUpdateTime )
	{
		m_flPrevParticleUpdateTime = m_flParticleUpdateTime;
		SetupWallParticles();
	}

	CollisionProp()->MarkSurroundingBoundsDirty();
	CollisionProp()->MarkPartitionHandleDirty();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::OnProjected( void )
{
	BaseClass::OnProjected();

	ProjectWall();
	SetupWallParticles();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::Spawn( void )
{
	SetThink( &C_ProjectedWallEntity::ClientThink );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Grow the displayed bridge out from its start and watch for portal changes
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::ClientThink( void )
{
	BaseClass::ClientThink();

	float flDisplaySpeed = clamp( cl_projected_wall_projection_speed.GetFloat(), 0.0f, m_flLength );
	m_flCurDisplayLength = clamp( m_flCurDisplayLength + flDisplaySpeed, 0.0f, m_flLength );

	TestForProjectionChanges();

	if ( m_flParticleUpdateTime != m_flPrevParticleUpdateTime )
	{
		m_flPrevParticleUpdateTime = m_flParticleUpdateTime;
		SetupWallParticles();
	}

	SetNextClientThink( gpGlobals->curtime + 0.016f );
}

//-----------------------------------------------------------------------------
// Purpose: Build the wall's collision shape for the current projection. While
//			predicting, a short history of shapes keyed by time is kept so traces
//			and bounds can use the shape that was valid at the traced time.
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::ProjectWall( void )
{
	AddEffects( EF_NOINTERP );

	Vector vWallStartPoint = GetStartPoint();
	Vector vWallEndPoint = GetEndPoint();

	bool bCreate = true;
	int iReplaceIndex = -1;
	int iValidIndex = -1;

	if ( prediction->InPrediction() )
	{
		// Find the newest shape that is not in the future
		for ( int i = m_WallCollideables.Count() - 1; i >= 0; --i )
		{
			if ( m_WallCollideables[i].flTime[1] <= gpGlobals->curtime )
			{
				if ( m_WallCollideables[i].qAngles == GetNetworkAngles() &&
					 m_WallCollideables[i].vStart == GetStartPoint() &&
					 m_WallCollideables[i].vEnd == GetEndPoint() )
				{
					// Still valid
					bCreate = false;
					iValidIndex = i;
				}
				else if ( gpGlobals->curtime <= m_WallCollideables[i].flTime[1] )
				{
					// Re-predicting the same time, replace it
					physcollision->DestroyCollide( m_WallCollideables[i].pCollideable );
					m_WallCollideables[i].pCollideable = NULL;
					iReplaceIndex = i;
				}
				else if ( i + 1 < m_WallCollideables.Count() )
				{
					m_WallCollideables.InsertAfter( i );
					iReplaceIndex = i + 1;
				}

				break;
			}
		}
	}
	else
	{
		// Networked update: drop every shape older than the newest one that has become current
		for ( int i = m_WallCollideables.Count() - 1; i >= 1; --i )
		{
			if ( m_WallCollideables[i].flTime[0] <= gpGlobals->curtime )
			{
				for ( int j = 0; j < i; ++j )
				{
					physcollision->DestroyCollide( m_WallCollideables[j].pCollideable );
				}

				m_WallCollideables.RemoveMultipleFromHead( i );
				break;
			}
		}

		if ( m_WallCollideables.Count() != 0 )
		{
			if ( m_WallCollideables[0].qAngles == GetNetworkAngles() &&
				 m_WallCollideables[0].vStart == GetStartPoint() &&
				 m_WallCollideables[0].vEnd == GetEndPoint() )
			{
				// The server agrees with our prediction, it's now authoritative
				m_WallCollideables[0].flTime[0] = m_WallCollideables[0].flTime[1] = gpGlobals->curtime;
				m_WallCollideables[0].bIsPredicted = false;
				bCreate = false;
				iValidIndex = 0;
			}
			else
			{
				physcollision->DestroyCollide( m_WallCollideables[0].pCollideable );
				bCreate = true;
				iReplaceIndex = 0;
			}
		}
	}

	// Anybody standing on the old wall needs to re-find their ground
	FOR_EACH_VALID_SPLITSCREEN_PLAYER( hPlayer )
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( hPlayer );
		if ( pPlayer && pPlayer->GetGroundEntity() == this )
		{
			pPlayer->SetGroundEntity( NULL );
		}
	}

	Vector vecForward, vecRight, vecUp;
	AngleVectors( GetNetworkAngles(), &vecForward, &vecRight, &vecUp );

	if ( bCreate )
	{
		CPhysCollide *pWallCollideable;

		if ( sv_thinnerprojectedwalls.GetBool() )
		{
			// Flat quad
			Vector vScaledRight = vecRight * ( m_flWidth * 0.5f );
			Vector vStart = vWallStartPoint;
			Vector vEnd = vWallEndPoint;

			Vector vVerts[4];
			vVerts[0] = vStart + vScaledRight;
			vVerts[1] = vStart - vScaledRight;
			vVerts[2] = vEnd - vScaledRight;
			vVerts[3] = vEnd + vScaledRight;

			Vector *pVerts[4] = { &vVerts[0], &vVerts[1], &vVerts[2], &vVerts[3] };
			CPhysConvex *pTempConvex = physcollision->ConvexFromVerts( pVerts, 4 );
			pWallCollideable = physcollision->ConvertConvexToCollide( &pTempConvex, 1 );
		}
		else
		{
			// Box bounded by six planes
			float flWallPlanes[24];

			( -vecForward ).CopyToArray( &flWallPlanes[0] );
			flWallPlanes[3] = DotProduct( -vecForward, vWallStartPoint );

			vecForward.CopyToArray( &flWallPlanes[4] );
			flWallPlanes[7] = DotProduct( vecForward, vWallEndPoint );

			Vector vWallPointRight = vWallStartPoint + vecRight * ( m_flWidth * 0.5f );
			vecRight.CopyToArray( &flWallPlanes[8] );
			flWallPlanes[11] = DotProduct( vecRight, vWallPointRight );

			Vector vWallPointLeft = vWallStartPoint - vecRight * ( m_flWidth * 0.5f );
			( -vecRight ).CopyToArray( &flWallPlanes[12] );
			flWallPlanes[15] = DotProduct( -vecRight, vWallPointLeft );

			Vector vWallPointUp = vWallStartPoint + vecUp * ( m_flHeight * 0.5f );
			vecUp.CopyToArray( &flWallPlanes[16] );
			flWallPlanes[19] = DotProduct( vecUp, vWallPointUp );

			Vector vWallPointDown = vWallStartPoint - vecUp * ( m_flHeight * 0.5f );
			( -vecUp ).CopyToArray( &flWallPlanes[20] );
			flWallPlanes[23] = DotProduct( -vecUp, vWallPointDown );

			CPolyhedron *pTempPolyhedron = GeneratePolyhedronFromPlanes( flWallPlanes, 6, FLT_EPSILON, false );
			if ( pTempPolyhedron == NULL )
			{
				Warning( "CProjectedWallEntity: GeneratePolyhedronFromPlanes failed! Get a save game for me!.\n" );
				return;
			}

			CPhysConvex *pTempConvex = physcollision->ConvexFromConvexPolyhedron( *pTempPolyhedron );
			pTempPolyhedron->Release();

			if ( pTempConvex == NULL )
				return;

			pWallCollideable = physcollision->ConvertConvexToCollide( &pTempConvex, 1 );
		}

		if ( pWallCollideable == NULL )
			return;

		Vector vWorldMins, vWorldMaxs;
		physcollision->CollideGetAABB( &vWorldMins, &vWorldMaxs, pWallCollideable, vec3_origin, vec3_angle );

		WallCollideableAtTime_t temp;
		temp.vStart = GetStartPoint();
		temp.vEnd = GetEndPoint();
		temp.vWorldMins = vWorldMins;
		temp.vWorldMaxs = vWorldMaxs;
		temp.qAngles = GetNetworkAngles();
		temp.bIsPredicted = prediction->InPrediction();
		temp.flTime[0] = gpGlobals->tickcount * gpGlobals->interval_per_tick;
		temp.flTime[1] = gpGlobals->curtime;
		temp.pCollideable = pWallCollideable;

		if ( iReplaceIndex >= 0 )
		{
			m_WallCollideables[iReplaceIndex] = temp;
			iValidIndex = iReplaceIndex;
		}
		else
		{
			iValidIndex = m_WallCollideables.Count();
			m_WallCollideables.AddToTail( temp );
		}
	}

	{
		m_vWorldSpace_WallMins = m_WallCollideables[iValidIndex].vWorldMins;
		m_vWorldSpace_WallMaxs = m_WallCollideables[iValidIndex].vWorldMaxs;

		Vector vSetMins = m_vWorldSpace_WallMins - vWallStartPoint;
		Vector vSetMaxs = m_vWorldSpace_WallMaxs - vWallStartPoint;
		SetSize( vSetMins, vSetMaxs );
	}

	if ( bCreate )
	{
		// New wall, grow it out again
		m_flCurDisplayLength = 0.0f;
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	m_flLength = ( vWallStartPoint - vWallEndPoint ).Length();

	CollisionProp()->MarkSurroundingBoundsDirty();
	CollisionProp()->MarkPartitionHandleDirty();

	if ( prediction->InPrediction() )
	{
		DisplaceObstructingEntities();
	}

	m_nNumSegments = (int)ceilf( m_flLength / m_flSegmentLength );
	m_PaintPowers.SetCount( m_nNumSegments );
	CleansePaint();

	m_fLastProjectedTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Trace the bridge with half its width so the next segment stays on it
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::GetProjectionExtents( Vector &outMins, Vector &outMaxs )
{
	GetExtents( outMins, outMaxs, 0.5f );
}

ConVar cl_projectedbridge_aabbhack( "cl_projectedbridge_aabbhack", "0", 0, "When predicting projected bridges, the spatial partition AABB encompasses all AABB's for outstanding collideables" );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs )
{
	if ( cl_projectedbridge_aabbhack.GetBool() )
	{
		Vector vMins = m_vWorldSpace_WallMins;
		Vector vMaxs = m_vWorldSpace_WallMaxs;
		for ( int i = 0; i < m_WallCollideables.Count(); ++i )
		{
			VectorMin( m_WallCollideables[i].vWorldMins, vMins, vMins );
			VectorMax( m_WallCollideables[i].vWorldMaxs, vMaxs, vMaxs );
		}

		*pWorldMins = vMins;
		*pWorldMaxs = vMaxs;
		return;
	}

	if ( m_WallCollideables.Count() == 0 )
	{
		*pWorldMins = m_vWorldSpace_WallMins;
		*pWorldMaxs = m_vWorldSpace_WallMaxs;
		return;
	}

	if ( prediction->InPrediction() )
	{
		// Use the shape that was current when the wall was last projected
		for ( int i = m_WallCollideables.Count() - 1; i >= 0; --i )
		{
			if ( m_WallCollideables[i].flTime[1] <= m_fLastProjectedTime )
			{
				*pWorldMins = m_WallCollideables[i].vWorldMins;
				*pWorldMaxs = m_WallCollideables[i].vWorldMaxs;
				return;
			}
		}
	}

	*pWorldMins = m_WallCollideables[0].vWorldMins;
	*pWorldMaxs = m_WallCollideables[0].vWorldMaxs;
}

//-----------------------------------------------------------------------------
// Purpose: Trace against the collision shape that matches the current prediction time
//-----------------------------------------------------------------------------
bool C_ProjectedWallEntity::TestCollision( const Ray_t &ray, unsigned int mask, trace_t &trace )
{
	if ( m_WallCollideables.Count() == 0 )
		return false;

	CPhysCollide *pCollide = m_WallCollideables[0].pCollideable;
	if ( prediction->InPrediction() )
	{
		for ( int i = m_WallCollideables.Count() - 1; i >= 0; --i )
		{
			if ( m_WallCollideables[i].flTime[1] <= m_fLastProjectedTime )
			{
				pCollide = m_WallCollideables[i].pCollideable;
				break;
			}
		}
	}

	physcollision->TraceBox( ray, mask, NULL, pCollide, vec3_origin, vec3_angle, &trace );

	return trace.DidHit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_ProjectedWallEntity::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t &tr )
{
	return TestCollision( ray, fContentsMask, tr );
}

//-----------------------------------------------------------------------------
// Purpose: Tint the paint materials for a power; returns false for unpainted
//-----------------------------------------------------------------------------
bool C_ProjectedWallEntity::SetPaintSurfaceColor( const PaintPowerType &power )
{
	float flColors[3] = { 1.0f, 1.0f, 1.0f };

	bool bIsPainted = ( power != NO_POWER );
	if ( bIsPainted )
	{
		Color color = MapPowerToVisualColor( power );
		flColors[0] = color.r() / 255.0f;
		flColors[1] = color.g() / 255.0f;
		flColors[2] = color.b() / 255.0f;
	}

	m_pPaintColorMid->SetVecValue( flColors, 3 );
	m_pPaintColorEnd1->SetVecValue( flColors, 3 );
	m_pPaintColorEnd2->SetVecValue( flColors, 3 );
	m_pPaintColorSing->SetVecValue( flColors, 3 );

	return bIsPainted;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::SetPaintPower( int nSegment, PaintPowerType power )
{
	if ( nSegment < m_nNumSegments )
	{
		m_PaintPowers[nSegment] = power;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Record the bridge as non-conformant data so the tools can recreate it
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::GetToolRecordingState( KeyValues *msg )
{
	BaseClass::GetToolRecordingState( msg );

	KeyValues *pKV = CIFM_EntityKeyValuesHandler_AutoRegister::FindOrCreateNonConformantKeyValues( msg );
	pKV->SetString( CIFM_EntityKeyValuesHandler_AutoRegister::GetHandlerIDKeyString(), "C_ProjectedWallEntity" );
	pKV->SetInt( "entIndex", index );
	pKV->SetFloat( "width", m_flWidth );
	pKV->SetFloat( "height", m_flHeight );
	pKV->SetInt( "numSegments", m_nNumSegments );
	pKV->SetFloat( "segLength", m_flSegmentLength );
	pKV->SetFloat( "dispLength", m_flCurDisplayLength );

	Vector vStart = GetStartPoint();
	pKV->SetFloat( "sp_x", vStart.x );
	pKV->SetFloat( "sp_y", vStart.y );
	pKV->SetFloat( "sp_z", vStart.z );

	QAngle qAngles;
	qAngles = GetNetworkAngles();
	pKV->SetFloat( "na_y", qAngles.y );
	pKV->SetFloat( "na_p", qAngles.x );
	pKV->SetFloat( "na_r", qAngles.z );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::RestoreToToolRecordedState( KeyValues *pKV )
{
	m_vecStartPoint.x = pKV->GetFloat( "sp_x" );
	m_vecStartPoint.y = pKV->GetFloat( "sp_y" );
	m_vecStartPoint.z = pKV->GetFloat( "sp_z" );
	m_flWidth = pKV->GetFloat( "width" );
	m_flHeight = pKV->GetFloat( "height" );
	m_nNumSegments = pKV->GetInt( "numSegments" );
	m_flSegmentLength = pKV->GetFloat( "segLength" );
	m_flCurDisplayLength = pKV->GetFloat( "dispLength" );

	m_vWorldSpace_WallMins = Vector( -16384.0f, -16384.0f, -16384.0f );
	m_vWorldSpace_WallMaxs = Vector( 16384.0f, 16384.0f, 16384.0f );
	SetSize( m_vWorldSpace_WallMins, m_vWorldSpace_WallMaxs );

	QAngle qKVAngles;
	qKVAngles.y = pKV->GetFloat( "na_y" );
	qKVAngles.x = pKV->GetFloat( "na_p" );
	qKVAngles.z = pKV->GetFloat( "na_r" );

	matrix3x4_t *pMatrix = (matrix3x4_t *)pKV->GetPtr( "worldTransform" );
	if ( pMatrix )
	{
		Vector vPos;
		QAngle qAngles;
		MatrixAngles( *pMatrix, qAngles, vPos );
		qAngles = qKVAngles;

		SetNetworkOrigin( vPos );
		SetAbsOrigin( vPos );
		SetLocalOrigin( vPos );
		SetNetworkAngles( qAngles );
		SetAbsAngles( qAngles );
		SetLocalAngles( qAngles );
	}

	// Visible in every view
	m_VisibilityBits.SetAll();
}

//-----------------------------------------------------------------------------
// Purpose: Impact effects are skipped where the bridge enters a portal it lines up with
//-----------------------------------------------------------------------------
bool C_ProjectedWallEntity::ShouldSpawnParticles( C_Portal_Base2D *pPortal )
{
	if ( !pPortal->IsActivedAndLinked() )
		return true;

	C_Prop_Portal *pPropPortal = dynamic_cast< C_Prop_Portal * >( pPortal );
	if ( pPropPortal )
	{
		Vector vPortalUp, vWallUp;
		AngleVectors( pPortal->GetAbsAngles(), NULL, NULL, &vPortalUp );
		AngleVectors( GetAbsAngles(), NULL, NULL, &vWallUp );

		float flDot = DotProduct( vPortalUp, vWallUp );
		return ( fabs( clamp( flDot, -1.0f, 1.0f ) ) > 0.7 );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the impact particles at each end of the bridge
//-----------------------------------------------------------------------------
void C_ProjectedWallEntity::SetupWallParticles( void )
{
	const float flImpactPointOffset = 24.0f;

	StopParticleEffects( this );

	C_Portal_Base2D *pSourcePortal = m_hSourcePortal.Get();
	C_Portal_Base2D *pHitPortal = m_hHitPortal.Get();

	Vector vecParticlePos;

	Vector vWallEndPoint = GetEndPoint();
	Vector vWallStartPoint = GetStartPoint();

	QAngle qAngles = GetNetworkAngles();
	Vector vecForward, vecRight, vecUp;
	AngleVectors( qAngles, &vecForward, &vecRight, &vecUp );

	// Start of the bridge
	if ( pSourcePortal && ShouldSpawnParticles( pSourcePortal ) )
	{
		vecParticlePos = vWallStartPoint + ( vecForward * 2.0f ) + ( vecRight * flImpactPointOffset );
		DispatchParticleEffect( "projected_wall_impact", vecParticlePos, qAngles, this );

		vecParticlePos = vWallStartPoint + ( vecForward * 2.0f ) - ( vecRight * flImpactPointOffset );
		DispatchParticleEffect( "projected_wall_impact", vecParticlePos, qAngles, this );

		// Reconstruction note: the build tests the hit portal (not the source portal) here.
		C_Prop_Portal *pPortal = dynamic_cast< C_Prop_Portal * >( pHitPortal );
		if ( pPortal )
		{
			vecParticlePos = vWallStartPoint + ( vecForward * 2.0f );
			DispatchParticleEffect( "projected_wall_impact", vecParticlePos, qAngles, this );
		}
	}

	// End of the bridge
	if ( pHitPortal && !ShouldSpawnParticles( pHitPortal ) )
		return;

	vecParticlePos = vWallEndPoint - ( vecForward * 2.0f ) + ( vecRight * flImpactPointOffset );
	DispatchParticleEffect( "projected_wall_impact", vecParticlePos, qAngles, this );

	vecParticlePos = vWallEndPoint - ( vecForward * 2.0f ) - ( vecRight * flImpactPointOffset );
	DispatchParticleEffect( "projected_wall_impact", vecParticlePos, qAngles, this );

	if ( !pHitPortal )
	{
		vecParticlePos = vWallEndPoint - ( vecForward * 2.0f );
		DispatchParticleEffect( "projected_wall_impact", vecParticlePos, qAngles, this );
	}
}

class CProjectedWallEntity_NonConformantDataHandler : public CIFM_EntityKeyValuesHandler_RecreateEntities
{
public:
	CProjectedWallEntity_NonConformantDataHandler( void )
		: CIFM_EntityKeyValuesHandler_RecreateEntities( "C_ProjectedWallEntity" )
	{ }

	virtual void *CreateInstance( void )
	{
		return new C_ProjectedWallEntity;
	}

	virtual void DestroyInstance( void *pEntity )
	{
		C_ProjectedWallEntity *pCastEntity = (C_ProjectedWallEntity *)pEntity;
		clienttools->RemoveClientRenderable( pCastEntity );
		delete pCastEntity;
	}

	virtual void HandleInstance( void *pEntity, KeyValues *pKeyValues )
	{
		C_ProjectedWallEntity *pCastEntity = (C_ProjectedWallEntity *)pEntity;
		pCastEntity->RestoreToToolRecordedState( pKeyValues );

		if ( pCastEntity->RenderHandle() == INVALID_CLIENT_RENDER_HANDLE )
		{
			clienttools->AddClientRenderable( pCastEntity, false, RENDERABLE_IS_TRANSLUCENT );
		}

		clienttools->MarkClientRenderableDirty( pCastEntity );
	}
};

static CProjectedWallEntity_NonConformantDataHandler s_ProjectedWallEntityIFMHandler;


//-----------------------------------------------------------------------------
// Purpose: Server notification that a segment of a bridge was painted
//-----------------------------------------------------------------------------
class C_TEWallPaintedEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEWallPaintedEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	CBaseHandle		m_hEntity;
	int				m_colorIndex;
	int				m_nSegment;
};

void C_TEWallPaintedEvent::PostDataUpdate( DataUpdateType_t updateType )
{
	C_ProjectedWallEntity *pWall = dynamic_cast< C_ProjectedWallEntity * >( C_BaseEntity::Instance( m_hEntity ) );
	if ( pWall )
	{
		pWall->SetPaintPower( m_nSegment, (PaintPowerType)m_colorIndex );
	}
}

IMPLEMENT_CLIENTCLASS_EVENT( C_TEWallPaintedEvent, DT_TEWallPaintedEvent, CTEWallPaintedEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEWallPaintedEvent, DT_TEWallPaintedEvent )
	RecvPropEHandle( RECVINFO( m_hEntity ) ),
	RecvPropInt( RECVINFO( m_colorIndex ) ),
	RecvPropInt( RECVINFO( m_nSegment ) ),
END_RECV_TABLE()
