//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server side of the excursion funnel: the trigger volume that pulls
//			physics objects and players, and its projected carrier entity.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "trigger_tractorbeam_shared.h"
#include "soundenvelope.h"
#include "portal_player.h"
#include "npc_portal_turret_floor.h"
#include "prop_weightedcube.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_AUTO_LIST( ITriggerTractorBeamAutoList );

BEGIN_DATADESC( CProjectedTractorBeamEntity )

	DEFINE_FIELD( m_hTractorBeamTrigger, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CProjectedTractorBeamEntity, DT_ProjectedTractorBeamEntity )
	SendPropEHandle( SENDINFO( m_hTractorBeamTrigger ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( projected_tractor_beam_entity, CProjectedTractorBeamEntity );



BEGIN_DATADESC( CTrigger_TractorBeam )

	DEFINE_PHYSPTR( m_pController ),

	DEFINE_INPUT( m_gravityScale, FIELD_FLOAT, "SetGravityScale" ),
	DEFINE_INPUT( m_addAirDensity, FIELD_FLOAT, "SetAdditionalAirDensity" ),
	DEFINE_INPUT( m_linearLimit, FIELD_FLOAT, "SetVelocityLimit" ),
	DEFINE_INPUT( m_linearLimitDelta, FIELD_FLOAT, "SetVelocityLimitDelta" ),
	DEFINE_INPUT( m_linearScale, FIELD_FLOAT, "SetVelocityScale" ),
	DEFINE_INPUT( m_angularLimit, FIELD_FLOAT, "SetAngVelocityLimit" ),
	DEFINE_INPUT( m_angularScale, FIELD_FLOAT, "SetAngVelocityScale" ),
	DEFINE_INPUT( m_linearForce, FIELD_FLOAT, "SetLinearForce" ),
	DEFINE_INPUT( m_linearForceAngles, FIELD_VECTOR, "SetLinearForceAngles" ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetVelocityLimitTime", InputSetVelocityLimitTime ),

	DEFINE_SOUNDPATCH( m_sndPlayerInBeam ),

	DEFINE_FIELD( m_gravityScale, FIELD_FLOAT ),
	DEFINE_FIELD( m_addAirDensity, FIELD_FLOAT ),
	DEFINE_FIELD( m_linearLimit, FIELD_FLOAT ),
	DEFINE_FIELD( m_linearLimitDelta, FIELD_FLOAT ),
	DEFINE_FIELD( m_linearLimitTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_linearLimitStart, FIELD_TIME ),
	DEFINE_FIELD( m_linearLimitStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_linearScale, FIELD_FLOAT ),
	DEFINE_FIELD( m_angularLimit, FIELD_FLOAT ),
	DEFINE_FIELD( m_angularScale, FIELD_FLOAT ),
	DEFINE_FIELD( m_linearForceAngles, FIELD_VECTOR ),
	DEFINE_FIELD( m_vStart, FIELD_VECTOR ),
	DEFINE_FIELD( m_vEnd, FIELD_VECTOR ),
	DEFINE_FIELD( m_bReversed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFromPortal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bToPortal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDisablePlayerMove, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_hProxyEntity, FIELD_EHANDLE ),

	DEFINE_THINKFUNC( TractorThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_tractorbeam, CTrigger_TractorBeam );

IMPLEMENT_SERVERCLASS_ST( CTrigger_TractorBeam, DT_Trigger_TractorBeam )
	SendPropBool( SENDINFO( m_bDisabled ) ),
	SendPropFloat( SENDINFO( m_gravityScale ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_addAirDensity ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_linearLimit ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_linearLimitDelta ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_linearLimitTime ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_linearLimitStart ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_linearLimitStartTime ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_linearScale ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_angularLimit ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_angularScale ), 0, SPROP_NOSCALE ),
	SendPropQAngles( SENDINFO( m_linearForceAngles ), 0, SPROP_NOSCALE ),
	SendPropEHandle( SENDINFO( m_hProxyEntity ) ),
	SendPropVector( SENDINFO( m_vStart ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vEnd ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_linearForce ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flRadius ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bReversed ) ),
	SendPropBool( SENDINFO( m_bFromPortal ) ),
	SendPropBool( SENDINFO( m_bToPortal ) ),
	SendPropBool( SENDINFO( m_bDisablePlayerMove ) ),
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTrigger_TractorBeam::CTrigger_TractorBeam()
{
	// Reconstruction note: the 2010 server kept its beams in
	// CTrigger_TractorBeam_Shared::AllTractorBeams; the retail shared code (and the 2010
	// client) enumerate ITriggerTractorBeamAutoList, whose constructor registers us.
	SetBeamRadius( 55.0f );
}

CTrigger_TractorBeam::~CTrigger_TractorBeam()
{
	// ITriggerTractorBeamAutoList's destructor unregisters us
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::Spawn( void )
{
	Precache();

	SetSolid( SOLID_VPHYSICS );
	AddSolidFlags( FSOLID_TRIGGER | FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );

	CreateVPhysics();

	SetTransmitState( FL_EDICT_PVSCHECK );

	SetThink( &CTrigger_TractorBeam::TractorThink );
	SetNextThink( gpGlobals->curtime );

	m_bDisablePlayerMove = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::Precache( void )
{
	PrecacheParticleSystem( "tractor_beam_src" );

	PrecacheScriptSound( "VFX.PlayerEnterTbeam" );
	PrecacheScriptSound( "VFX.TBeamPosPolarity" );
}

//-----------------------------------------------------------------------------
// Purpose: Start the ambient beam hum
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::Activate( void )
{
	BaseClass::Activate();

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	CPASFilter filter( GetAbsOrigin() );
	m_sndAmbient = controller.SoundCreate( filter, entindex(), "VFX.TBeamPosPolarity" );
	controller.Play( m_sndAmbient, 1.0f, 100.0f );
}

//-----------------------------------------------------------------------------
// Purpose: Track whether the beam comes out of or goes into a portal
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::TractorThink( void )
{
	if ( m_hProxyEntity.Get() )
	{
		m_bFromPortal = ( m_hProxyEntity->GetSourcePortal() != NULL );
		m_bToPortal = ( m_hProxyEntity->GetHitPortal() != NULL );
	}

	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: The physics object is made by UpdateBeam(); we only need the controller
//-----------------------------------------------------------------------------
bool CTrigger_TractorBeam::CreateVPhysics( void )
{
	m_pController = physenv->CreateMotionController( this );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Wake everything in the beam so it notices the changed forces
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::WakeTouchingObjects( void )
{
	CBaseEntity *list[256];
	CBaseEntity *pEntity;

	Ray_t ray;
	Vector vExtents( m_flRadius, m_flRadius, m_flRadius );
	ray.Init( m_vStart, m_vEnd, -vExtents, vExtents );
	int nNumFound = UTIL_EntitiesAlongRay( list, ARRAYSIZE( list ), ray, 0 );
	for ( int i = 0; i < nNumFound; ++i )
	{
		pEntity = list[i];
		if ( pEntity == NULL )
			continue;

		// Disabled reflective cubes need to start moving again
		if ( UTIL_IsReflectiveCube( pEntity ) )
		{
			CPropWeightedCube *pReflectiveCube = assert_cast< CPropWeightedCube* >( pEntity );
			pReflectiveCube->ExitDisabledState();
		}

		IPhysicsObject *pPhysObj = pEntity->VPhysicsGetObject();
		if ( pPhysObj )
		{
			pPhysObj->Wake();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::UpdateOnRemove( void )
{
	if ( m_pController )
	{
		// Wake everything we were holding so it falls
		const int nNumObjects = m_pController->CountObjects();
		IPhysicsObject **pObjects = new IPhysicsObject*[nNumObjects];
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

		delete [] pObjects;
	}

	StopParticleEffects( this );
	StopLoopingSounds();

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::OnRestore( void )
{
	BaseClass::OnRestore();

	if ( m_pController )
	{
		m_pController->SetEventHandler( this );
	}

	SetTransmitState( FL_EDICT_PVSCHECK );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::StopLoopingSounds( void )
{
	if ( m_sndAmbient )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.Shutdown( m_sndAmbient );
		controller.SoundDestroy( m_sndAmbient );
		m_sndAmbient = NULL;
	}

	if ( m_sndPlayerInBeam )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.Shutdown( m_sndPlayerInBeam );
		controller.SoundDestroy( m_sndPlayerInBeam );
		m_sndPlayerInBeam = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );

	if ( !PassesTriggerFilters( pOther ) )
		return;

	if ( FClassnameIs( pOther, "npc_portal_turret_floor" ) )
	{
		CNPC_Portal_FloorTurret *pTurret = assert_cast< CNPC_Portal_FloorTurret* >( pOther );
		if ( pTurret )
		{
			pTurret->OnEnteredTractorBeam();
		}
	}
	else if ( UTIL_IsReflectiveCube( pOther ) )
	{
		// Reconstruction note: not in the 2010 builds; the retail EndTouch
		// (trigger_tractorbeam_shared.cpp) calls OnExitedTractorBeam() on reflective cubes,
		// and this is the matching entry notification.
		CPropWeightedCube *pCube = assert_cast< CPropWeightedCube* >( pOther );
		if ( pCube )
		{
			pCube->OnEnteredTractorBeam();
		}
	}

	CBasePlayer *pPlayer = ToBasePlayer( pOther );
	if ( pPlayer )
	{
		// Reconstruction note: the 2010 builds set PFLAG_VPHYSICS_MOTIONCONTROLLER,
		// m_Local.m_bSlowMovement and a FLT_MIN gravity here and undid them in EndTouch. The
		// retail EndTouch hands the player to CPortal_Player::SetLeaveTractorBeam() instead,
		// whose counterpart SetInTractorBeam() does the slow movement and gravity setup; the
		// player is then moved by CPortalGameMovement::TBeamMove() rather than our controller.
		CPortal_Player *pPortalPlayer = ToPortalPlayer( pPlayer );
		if ( pPortalPlayer )
		{
			pPortalPlayer->SetInTractorBeam( this );
		}

		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		if ( m_sndPlayerInBeam )
		{
			controller.Shutdown( m_sndPlayerInBeam );
			controller.SoundDestroy( m_sndPlayerInBeam );
			m_sndPlayerInBeam = NULL;
		}

		// Only the player entering the beam hears this
		EmitSound_t ep;
		ep.m_nChannel = CHAN_STATIC;
		ep.m_pSoundName = "VFX.PlayerEnterTbeam";
		ep.m_flVolume = 1.0f;
		ep.m_SoundLevel = SNDLVL_75dB;

		CPASAttenuationFilter filter( GetAbsOrigin() );
		filter.MakeReliable();
		filter.RemoveAllRecipients();
		filter.AddRecipient( pPlayer );

		m_sndPlayerInBeam = controller.SoundCreate( filter, entindex(), ep );
		controller.Play( m_sndPlayerInBeam, 1.0f, 100.0f );
	}
	else
	{
		triggerevent_t event;
		if ( m_pController && PhysGetTriggerEvent( &event, this ) && event.pObject )
		{
			event.pObject->Wake();
			m_pController->AttachObject( event.pObject, true );
		}

		// Reconstruction note: the retail EndTouch drops this beam from the entity's beam
		// history and hands the entity back to the previous beam; record this one as current.
		EntityBeamHistory_t &beamHistory = g_TractorBeamManager.GetHistoryFromEnt( pOther );
		beamHistory.UpdateBeam( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Attach an entity that is already inside the beam to our controller
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::ForceAttachEntity( CBaseEntity *pEntity )
{
	// Reconstruction note: required by the retail EndTouch but absent from the 2010 builds;
	// it performs the same attach StartTouch does for the entity's physics object.
	IPhysicsObject *pPhysObject = pEntity->VPhysicsGetObject();
	if ( pPhysObject && m_pController )
	{
		pPhysObject->Wake();
		m_pController->AttachObject( pPhysObject, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Ramp the velocity limit to a new value over time
//			Parameters are "<new limit> <duration>"
//-----------------------------------------------------------------------------
void CTrigger_TractorBeam::InputSetVelocityLimitTime( inputdata_t &inputdata )
{
	m_linearLimitStart = GetLinearLimit();
	m_linearLimitStartTime = gpGlobals->curtime;

	float args[2];
	UTIL_StringToFloatArray( args, 2, inputdata.value.String() );
	m_linearLimit = args[0];
	m_linearLimitTime = args[1];
}

//-----------------------------------------------------------------------------
// Purpose: Create a beam volume between two points, owned by a projected beam segment
//-----------------------------------------------------------------------------
CTrigger_TractorBeam *CTrigger_TractorBeam::CreateTractorBeam( const Vector &vStart, const Vector &vEnd, CBaseEntity *pOwner )
{
	CTrigger_TractorBeam *pBeam = (CTrigger_TractorBeam *) CreateEntityByName( "trigger_tractorbeam" );
	if ( pBeam == NULL )
		return NULL;

	pBeam->SetOwnerEntity( pOwner );

	// Reconstruction note: the 2010 UpdateBeam() took the owner as the proxy entity and read
	// the endpoints and force from it; the retail UpdateBeam() takes them as arguments.
	CProjectedTractorBeamEntity *pProxy = static_cast< CProjectedTractorBeamEntity* >( pOwner );
	pBeam->SetProxyEntity( pProxy );
	pBeam->UpdateBeam( vStart, vEnd, pProxy ? pProxy->GetLinearForce() : -1.0f );

	DispatchSpawn( pBeam );

	return pBeam;
}



CProjectedTractorBeamEntity *CProjectedTractorBeamEntity::CreateNewInstance( void )
{
	return (CProjectedTractorBeamEntity *) CreateEntityByName( "projected_tractor_beam_entity" );
}

CBaseProjectedEntity *CProjectedTractorBeamEntity::CreateNewProjectedEntity( void )
{
	return CreateNewInstance();
}

//-----------------------------------------------------------------------------
// Purpose: Refit the beam volume to the new projection
//-----------------------------------------------------------------------------
void CProjectedTractorBeamEntity::OnProjected( void )
{
	BaseClass::OnProjected();

	CTrigger_TractorBeam *pBeam = m_hTractorBeamTrigger.Get();
	if ( pBeam )
	{
		// The volume is predicted by whoever predicts us
		if ( IsPlayerSimulated() )
		{
			pBeam->SetPlayerSimulated( GetSimulatingPlayer() );
		}
		else
		{
			pBeam->UnsetPlayerSimulated();
		}

		pBeam->UpdateBeam( GetStartPoint(), GetEndPoint(), GetLinearForce() );
	}
}
