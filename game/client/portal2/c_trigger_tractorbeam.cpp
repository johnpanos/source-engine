//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the tractor beam (excursion funnel): the trigger volume
//			that pushes objects, its rendering and effects, and the projected
//			segment entity that owns it.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_trigger_tractorbeam.h"
#include "c_baseplayer.h"
#include "physics_shared.h"
#include "ispatialpartition.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "functionproxy.h"
#include "imaterialproxydict.h"
#include "toolframework_client.h"
#include "toolframework/itoolentity.h"
#include "tier1/KeyValues.h"
#include "soundinfo.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_AUTO_LIST( ITriggerTractorBeamAutoList );

BEGIN_PREDICTION_DATA( C_ProjectedTractorBeamEntity )
END_PREDICTION_DATA()

IMPLEMENT_CLIENTCLASS_DT( C_ProjectedTractorBeamEntity, DT_ProjectedTractorBeamEntity, CProjectedTractorBeamEntity )
	RecvPropEHandle( RECVINFO( m_hTractorBeamTrigger ) ),
END_RECV_TABLE()

LINK_ENTITY_TO_CLASS( projected_tractor_beam_entity, C_ProjectedTractorBeamEntity );


//-----------------------------------------------------------------------------
// Purpose: A new start point means the beam's particles must be rebuilt
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::RecvProxy_Start( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_Trigger_TractorBeam *pBeam = (C_Trigger_TractorBeam *)pStruct;

	Vector vStart;
	vStart.Init( pData->m_Value.m_Vector[0], pData->m_Value.m_Vector[1], pData->m_Value.m_Vector[2] );

	if ( vStart != pBeam->m_vStart )
	{
		pBeam->m_vStart = vStart;
		pBeam->m_flStartTime = gpGlobals->curtime;
		pBeam->m_bRecreateParticles = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: A new end point means the beam's particles must be rebuilt
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::RecvProxy_End( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_Trigger_TractorBeam *pBeam = (C_Trigger_TractorBeam *)pStruct;

	Vector vEnd;
	vEnd.Init( pData->m_Value.m_Vector[0], pData->m_Value.m_Vector[1], pData->m_Value.m_Vector[2] );

	if ( vEnd != pBeam->m_vEnd )
	{
		pBeam->m_vEnd = vEnd;
		pBeam->m_flStartTime = gpGlobals->curtime;
		pBeam->m_bRecreateParticles = true;
	}
}

IMPLEMENT_CLIENTCLASS_DT( C_Trigger_TractorBeam, DT_Trigger_TractorBeam, CTrigger_TractorBeam )
	RecvPropBool( RECVINFO( m_bDisabled ) ),
	RecvPropFloat( RECVINFO( m_gravityScale ) ),
	RecvPropFloat( RECVINFO( m_addAirDensity ) ),
	RecvPropFloat( RECVINFO( m_linearLimit ) ),
	RecvPropFloat( RECVINFO( m_linearLimitDelta ) ),
	RecvPropFloat( RECVINFO( m_linearLimitTime ) ),
	RecvPropFloat( RECVINFO( m_linearLimitStart ) ),
	RecvPropFloat( RECVINFO( m_linearLimitStartTime ) ),
	RecvPropFloat( RECVINFO( m_linearScale ) ),
	RecvPropFloat( RECVINFO( m_angularLimit ) ),
	RecvPropFloat( RECVINFO( m_angularScale ) ),
	RecvPropQAngles( RECVINFO( m_linearForceAngles ) ),
	RecvPropEHandle( RECVINFO( m_hProxyEntity ) ),
	RecvPropVector( RECVINFO( m_vStart ), 0, C_Trigger_TractorBeam::RecvProxy_Start ),
	RecvPropVector( RECVINFO( m_vEnd ), 0, C_Trigger_TractorBeam::RecvProxy_End ),
	RecvPropFloat( RECVINFO( m_linearForce ) ),
	RecvPropFloat( RECVINFO( m_flRadius ) ),
	RecvPropBool( RECVINFO( m_bReversed ) ),
	RecvPropBool( RECVINFO( m_bFromPortal ) ),
	RecvPropBool( RECVINFO( m_bToPortal ) ),
	RecvPropBool( RECVINFO( m_bDisablePlayerMove ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_Trigger_TractorBeam )
	DEFINE_PRED_FIELD( m_vStart, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vEnd, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( trigger_tractorbeam, C_Trigger_TractorBeam );

C_Trigger_TractorBeam::C_Trigger_TractorBeam()
{
	// Reconstruction note: the 2010 constructor also appended the beam to
	// CTrigger_TractorBeam_Shared::AllTractorBeams. The retail shared code enumerates beams
	// through ITriggerTractorBeamAutoList instead, so that registration is gone.
	// The remaining members rely on C_BaseEntity's zero-filling operator new, as in the build.
	m_bDisabled = false;
}

C_Trigger_TractorBeam::~C_Trigger_TractorBeam()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::Spawn( void )
{
	BaseClass::Spawn();

	if ( m_pMaterial1 == NULL )
	{
		m_pMaterial1 = materials->FindMaterial( "effects/tractor_beam", NULL, false );
	}

	if ( m_pMaterial2 == NULL )
	{
		m_pMaterial2 = materials->FindMaterial( "effects/tractor_beam2", NULL, false );
	}

	if ( m_pMaterial3 == NULL )
	{
		m_pMaterial3 = materials->FindMaterial( "effects/tractor_beam3", NULL, false );
	}

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: (Re)create the core particle system that runs the length of the beam
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::CreateParticles( void )
{
	// Kill the old effect
	if ( m_hCoreEffect )
	{
		ParticleProp()->StopEmission( m_hCoreEffect, false, true );
		m_hCoreEffect = NULL;
	}

	m_hCoreEffect = ParticleProp()->Create( "tractor_beam_core", PATTACH_CUSTOMORIGIN, -1, vec3_origin );
	if ( m_hCoreEffect )
	{
		ParticleProp()->AddControlPoint( m_hCoreEffect, 1, this, PATTACH_CUSTOMORIGIN );
		ParticleProp()->AddControlPoint( m_hCoreEffect, 2, this, PATTACH_CUSTOMORIGIN );
		ParticleProp()->AddControlPoint( m_hCoreEffect, 3, this, PATTACH_CUSTOMORIGIN );

		Vector vDir = m_vEnd - m_vStart;
		VectorNormalize( vDir );

		Vector vRight, vUp;
		VectorVectors( vDir, vRight, vUp );

		m_hCoreEffect->SetControlPoint( 0, m_vStart );
		m_hCoreEffect->SetControlPointOrientation( 0, vDir, vRight, vUp );
		m_hCoreEffect->SetControlPoint( 1, m_vEnd );
		m_hCoreEffect->SetControlPointOrientation( 1, -vDir, vRight, vUp );

		Vector vVelocity = vDir * m_linearForce;
		m_hCoreEffect->SetControlPoint( 2, vVelocity );

		// Reconstruction note: the direction colour is computed but the build passes the
		// forward (blue) colour to control point 3 in both directions.
		Vector color = m_bReversed ? Vector( 255, 160, 32 ) : Vector( 64, 160, 255 );
		NOTE_UNUSED( color );
		m_hCoreEffect->SetControlPoint( 3, Vector( 64, 160, 255 ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::UpdateOnRemove( void )
{
	if ( m_pController )
	{
		// Wake everything we were holding so it falls out of the beam
		const int nNumObjects = m_pController->CountObjects();
		IPhysicsObject **pObjects = (IPhysicsObject **)stackalloc( nNumObjects * sizeof( IPhysicsObject * ) );
		m_pController->GetObjects( pObjects );

		for ( int i = 0; i < nNumObjects; ++i )
		{
			if ( pObjects[i] )
			{
				pObjects[i]->Wake();
			}
		}

		physenv->DestroyMotionController( m_pController );
		m_pController = NULL;
	}

	if ( m_hCoreEffect )
	{
		ParticleProp()->StopEmission( m_hCoreEffect, false, true );
		m_hCoreEffect = NULL;
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		// Projected entities should reflect in water because they are noticeable
		AddEffects( EF_MARKED_FOR_FAST_REFLECTION );

		if ( physenv && m_pController == NULL )
		{
			m_pController = physenv->CreateMotionController( this );

			// Reconstruction note: the 2010 client called a parameterless UpdateBeam(); the
			// retail version takes the endpoints and a signed force (negative reverses).
			UpdateBeam( m_vStart, m_vEnd, m_bReversed ? -m_linearForce : m_linearForce );
		}
	}
	else if ( m_bRecreateParticles )
	{
		m_bRecreateParticles = false;
	}
	else
	{
		return;
	}

	CreateParticles();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_BasePlayer *C_Trigger_TractorBeam::GetPredictionOwner( void )
{
	return GetSimulatingPlayer();
}

//-----------------------------------------------------------------------------
// Purpose: Tractor beams live in the client trigger list so they can be touched
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::UpdatePartitionListEntry( void )
{
	::partition->RemoveAndInsert(
		PARTITION_CLIENT_SOLID_EDICTS | PARTITION_CLIENT_RESPONSIVE_EDICTS | PARTITION_CLIENT_NON_STATIC_EDICTS, // remove
		PARTITION_CLIENT_TRIGGER_ENTITIES, // add
		CollisionProp()->GetPartitionHandle() );
}

//-----------------------------------------------------------------------------
// Purpose: Play the beam's sounds from the point on the beam nearest the listener
//-----------------------------------------------------------------------------
bool C_Trigger_TractorBeam::GetSoundSpatialization( SpatializationInfo_t &info )
{
	// Out of PVS
	if ( IsDormant() )
		return false;

	if ( info.pOrigin )
	{
		float t;
		CalcClosestPointOnLine( info.info.vListenerOrigin, m_vStart, m_vEnd, *info.pOrigin, &t );

		if ( t < 0.0f )
		{
			*info.pOrigin = m_vStart;
		}
		else if ( t > 1.0f )
		{
			*info.pOrigin = m_vEnd;
		}
	}

	if ( info.pAngles )
	{
		VectorCopy( CollisionProp()->GetCollisionAngles(), *info.pAngles );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::StartTouch( C_BaseEntity *pOther )
{
	if ( m_pController )
	{
		IPhysicsObject *pPhysObject = pOther->VPhysicsGetObject();
		if ( pPhysObject )
		{
			m_pController->AttachObject( pPhysObject, true );
		}
	}

	if ( pOther->IsPlayer() )
	{
		C_BasePlayer *pPlayer = ToBasePlayer( pOther );
		if ( pPlayer )
		{
			pPlayer->m_Local.m_bSlowMovement = true;
			pPlayer->SetGravity( FLT_MIN );
		}
	}

	BaseClass::StartTouch( pOther );
}

// Reconstruction note: the 2010 client defined EndTouch() here (detach, clear the player's
// motion-controller flag, slow movement and gravity). In the retail code EndTouch() is
// implemented once in trigger_tractorbeam_shared.cpp for both realms, so it is not
// duplicated in this file.

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::PhysicsSimulate( void )
{
	BaseClass::PhysicsSimulate();
}

//-----------------------------------------------------------------------------
// Purpose: Draw one cylinder of the beam as a ring of quads
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::DrawColumnSegment( CMeshBuilder &meshBuilder, Vector &vecStart, const Vector &vDir, float flLength, const Vector &vecXAxis, const Vector &vecYAxis, float flRadius, float flAlpha, float flTextureOffset )
{
	// Reconstruction note: the build declares 'Vector vStart' and 'flTextureOffset' here but
	// neither contributes to the emitted geometry.
	Vector vecPosition;
	Vector vecLastPosition = vecStart + ( vecXAxis * flRadius );

	const float flRadRadStep = ( M_PI * 2.0f ) / 64.0f;

	float flSin, flCos;
	float flV;

	int r, g, b;
	if ( m_bReversed )
	{
		r = 255;
		g = 160;
		b = 32;
	}
	else
	{
		r = 64;
		g = 160;
		b = 255;
	}

	float flLastV = 0.0f;

	for ( int i = 1; i <= 64; i++ )
	{
		SinCos( flRadRadStep * i, &flSin, &flCos );
		vecPosition = vecStart + ( vecXAxis * flCos * flRadius ) + ( vecYAxis * flSin * flRadius );

		flV = (float) i / 64.0f;

		Vector vert;

		const float flU = flLength / 256.0f;

		float pColors[4] = { ( r / 255.0f ) * flAlpha, ( g / 255.0f ) * flAlpha, ( b / 255.0f ) * flAlpha, 1.0f };

		meshBuilder.Color4fv( pColors );
		meshBuilder.TexCoord2f( 0, 0.0f, flV );
		meshBuilder.Position3fv( vecPosition.Base() );
		meshBuilder.AdvanceVertex();

		vert = vecPosition + ( vDir * flLength );

		meshBuilder.Color4fv( pColors );
		meshBuilder.TexCoord2f( 0, flU, flV );
		meshBuilder.Position3fv( vert.Base() );
		meshBuilder.AdvanceVertex();

		vert = vecLastPosition + ( vDir * flLength );

		meshBuilder.Color4fv( pColors );
		meshBuilder.TexCoord2f( 0, flU, flLastV );
		meshBuilder.Position3fv( vert.Base() );
		meshBuilder.AdvanceVertex();

		vert = vecLastPosition;

		meshBuilder.Color4fv( pColors );
		meshBuilder.TexCoord2f( 0, 0.0f, flLastV );
		meshBuilder.Position3fv( vert.Base() );
		meshBuilder.AdvanceVertex();

		vecLastPosition = vecPosition;
		flLastV = flV;
	}

	vecStart = vecStart + ( vDir * flLength );
}

//-----------------------------------------------------------------------------
// Purpose: Draw a full column of the beam with one material
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::DrawColumn( IMaterial *pMaterial, const Vector &vecStart, const Vector &vDir, float flLength, const Vector &vecXAxis, const Vector &vecYAxis, float flRadius, float flAlpha, bool bPinchIn, bool bPinchOut, float flTextureOffset )
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( pMaterial, GetClientRenderable() );

	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, pMaterial );
	CMeshBuilder meshBuilder;

	// Reconstruction note: pinched ends reserve room for their extra rings, but this build
	// only emits the main segment below.
	int nTotalSegments = 64;
	if ( bPinchIn )
	{
		nTotalSegments += 512;
	}

	if ( bPinchOut )
	{
		nTotalSegments += 512;
	}

	meshBuilder.Begin( pMesh, MATERIAL_QUADS, nTotalSegments * 4 );

	float flMainLength = flLength;

	Vector vStart = vecStart;
	DrawColumnSegment( meshBuilder, vStart, vDir, flMainLength, vecXAxis, vecYAxis, flRadius, flAlpha, flTextureOffset );

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_Trigger_TractorBeam::ShouldDraw( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw the three nested beam cylinders, growing out from the start point
//-----------------------------------------------------------------------------
int C_Trigger_TractorBeam::DrawModel( int flags, const RenderableInstance_t &instance )
{
	Vector vDir = ( m_vEnd - m_vStart );
	float flLength = VectorNormalize( vDir );

	// Extend the beam over the first half second after it changes
	float flStartDelta = gpGlobals->curtime - m_flStartTime;
	if ( flStartDelta < 0.5f )
	{
		flLength *= SimpleSplineRemapValClamped( gpGlobals->curtime, m_flStartTime, m_flStartTime + 0.5f, 0.0f, 1.0f );
	}

	QAngle angles;
	VectorAngles( vDir, angles );

	float flRadius = 58.0f;

	matrix3x4_t xform;
	AngleMatrix( angles, m_vStart, xform );
	Vector xAxis, yAxis;
	MatrixGetColumn( xform, 2, xAxis );
	MatrixGetColumn( xform, 1, yAxis );

	DrawColumn( m_pMaterial1, m_vStart, vDir, flLength, xAxis, yAxis, flRadius, 1.0f, m_bFromPortal, m_bToPortal, 0.0f );
	DrawColumn( m_pMaterial2, m_vStart, vDir, flLength, xAxis, yAxis, flRadius - 2.0f, 0.75f, m_bFromPortal, m_bToPortal, 0.0f );
	DrawColumn( m_pMaterial3, m_vStart, vDir, flLength, xAxis, yAxis, flRadius - 4.0f, 0.5f, m_bFromPortal, m_bToPortal, 0.0f );

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Record the beam as non-conformant data so the tools can recreate it
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::GetToolRecordingState( KeyValues *msg )
{
	BaseClass::GetToolRecordingState( msg );
}

void C_Trigger_TractorBeam::RestoreToToolRecordedState( KeyValues *pKV )
{
	(void)pKV;
}

//-----------------------------------------------------------------------------
// Purpose: The source effect aims at the end of the beam
//-----------------------------------------------------------------------------
void C_Trigger_TractorBeam::OnNewParticleEffect( const char *pszParticleName, CNewParticleEffect *pNewParticleEffect )
{
	if ( V_stricmp( pszParticleName, "tractor_beam_src" ) == 0 )
	{
		pNewParticleEffect->SetControlPoint( 2, m_vEnd );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Material proxy that scrolls the beam textures with the beam's speed
//-----------------------------------------------------------------------------
class CTractorBeamProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
};

bool CTractorBeamProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	return CResultProxy::Init( pMaterial, pKeyValues );
}

void CTractorBeamProxy::OnBind( void *pC_BaseEntity )
{
	if ( !pC_BaseEntity )
		return;

	C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );
	if ( !pEntity )
		return;

	C_Trigger_TractorBeam *pBeam = dynamic_cast< C_Trigger_TractorBeam * >( pEntity );
	if ( pBeam )
	{
		float flSpeed = pBeam->GetSpeed() / 512.0f;
		SetFloatResult( flSpeed );
	}
}

EXPOSE_MATERIAL_PROXY( CTractorBeamProxy, TractorBeam );
