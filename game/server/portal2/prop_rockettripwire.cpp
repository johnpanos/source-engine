//========= Portal 2 reconstruction ============================================//
//
// Purpose: Stationary rocket turret that fires when its tripwire is crossed.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "prop_rockettripwire.h"
#include "rocketprojectile.h"
#include "info_placement_helper.h"
#include "portal_util_shared.h"
#include "physicsshadowclone.h"
#include "portal_grabcontroller_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ROCKET_TRIPWIRE_MODEL			"models/props/tripwire_turret.mdl"
#define ROCKET_TRIPWIRE_BEAM_SPRITE		"effects/bluelaser1.vmt"
#define ROCKET_TRIPWIRE_ENDPOINT_SPRITE	"sprites/light_glow03.vmt"

#define ROCKET_TRIPWIRE_RANGE			8192.0f
#define ROCKET_TRIPWIRE_FIRE_DELAY		1.2f
#define ROCKET_TRIPWIRE_RELOAD_TIME		2.5f

LINK_ENTITY_TO_CLASS( prop_rocket_tripwire, CPropRocketTripwire );

BEGIN_DATADESC( CPropRocketTripwire )

	DEFINE_FIELD( m_hCurRocket, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hPlacementHelper, FIELD_EHANDLE ),
	DEFINE_FIELD( m_nBeamEndpointTexture, FIELD_INTEGER ),

	DEFINE_KEYFIELD( m_flRocketSpeed, FIELD_FLOAT, "RocketSpeed" ),
	DEFINE_KEYFIELD( m_flRocketLifetime, FIELD_FLOAT, "RocketLifetime" ),
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),

	DEFINE_FIELD( m_flTimeLastFired, FIELD_FLOAT ),
	DEFINE_FIELD( m_flRocketTimeOfDeath, FIELD_FLOAT ),
	DEFINE_FIELD( m_State, FIELD_INTEGER ),
	DEFINE_FIELD( m_iMuzzleAttachment, FIELD_INTEGER ),
	DEFINE_FIELD( m_bHasRocketOut, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDrawBeams, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hRocket, FIELD_EHANDLE ),

	DEFINE_OUTPUT( m_OnTripped, "OnTripped" ),
	DEFINE_OUTPUT( m_OnRocketExplode, "OnRocketExplode" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "ForceFire", InputForceFire ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableTripwire", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableTripwire", InputDisable ),

	DEFINE_THINKFUNC( FireRocketThink ),
	DEFINE_THINKFUNC( WaitingThink ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPropRocketTripwire, DT_PropRocketTripwire )
	SendPropInt( SENDINFO( m_iMuzzleAttachment ) ),
	SendPropInt( SENDINFO( m_nBeamEndpointTexture ) ),
	SendPropBool( SENDINFO( m_bDrawBeams ) ),
	SendPropEHandle( SENDINFO( m_hRocket ) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CPropRocketTripwire::CPropRocketTripwire()
{
	m_flRocketSpeed = 400.0f;
	m_flRocketLifetime = 15.0f;
	m_flTimeLastFired = 0.0f;
	m_hPlacementHelper = NULL;
	m_hCurRocket = NULL;
	m_bHasRocketOut = false;
	m_flRocketTimeOfDeath = 0.0f;
	m_bDrawBeams = true;
	m_hRocket = NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CPropRocketTripwire::~CPropRocketTripwire()
{
	UTIL_Remove( m_hPlacementHelper );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::Precache( void )
{
	PrecacheModel( ROCKET_TRIPWIRE_MODEL );
	PrecacheModel( ROCKET_TRIPWIRE_BEAM_SPRITE );
	m_nBeamEndpointTexture = PrecacheModel( ROCKET_TRIPWIRE_ENDPOINT_SPRITE );

	UTIL_PrecacheOther( "rocket_turret_projectile" );

	PrecacheScriptSound( "NPC_TripwireTurret.Reload" );
	PrecacheScriptSound( "NPC_TripwireTurret.LockedBeep" );
	PrecacheScriptSound( "NPC_TripwireTurret.LockingBeep" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::Spawn( void )
{
	Precache();

	SetModel( ROCKET_TRIPWIRE_MODEL );

	m_iMuzzleAttachment = LookupAttachment( "barrel" );

	// Reconstruction note: the binary computes the muzzle transform and aim
	// direction here but does not use them.
	Vector vecMuzzle;
	QAngle angMuzzleDir;
	GetAttachment( m_iMuzzleAttachment, vecMuzzle, angMuzzleDir );

	Vector vecAimDir = EyeDirection();

	m_hPlacementHelper = static_cast< CInfoPlacementHelper* >( CreateEntityByName( "info_placement_helper" ) );
	if ( m_hPlacementHelper != NULL )
	{
		m_hPlacementHelper->KeyValue( "radius", "52" );
		m_hPlacementHelper->KeyValue( "force_placement", "1" );
		DispatchSpawn( m_hPlacementHelper.Get() );
		m_hPlacementHelper->Activate();
	}

	if ( m_bDisabled )
	{
		DisableTripwire();
	}
	else
	{
		EnableTripwire();
	}

	SetThink( &CPropRocketTripwire::WaitingThink );
	SetNextThink( gpGlobals->curtime + 0.1 );
}

//-----------------------------------------------------------------------------
// Purpose: Rockets and the tripwire leave from the barrel
//-----------------------------------------------------------------------------
Vector CPropRocketTripwire::EyePosition( void )
{
	Vector vMuzzlePos;
	GetAttachment( m_iMuzzleAttachment, vMuzzlePos );
	return vMuzzlePos;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Vector CPropRocketTripwire::EyeDirection( void )
{
	Vector vMuzzlePos, vMuzzleDir;
	GetAttachment( m_iMuzzleAttachment, vMuzzlePos, &vMuzzleDir );
	return vMuzzleDir;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::UpdateOnRemove( void )
{
	if ( m_hPlacementHelper.Get() )
	{
		UTIL_Remove( m_hPlacementHelper.Get() );
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Dispatches to the think for the current state
//-----------------------------------------------------------------------------
void CPropRocketTripwire::WaitingThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.1 );

	switch ( m_State )
	{
	case ACTIVE:
		ActiveThink();
		break;

	case FIRING:
		PreFiringThink();
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Watch the tripwire for players and moving/carried objects
//-----------------------------------------------------------------------------
void CPropRocketTripwire::ActiveThink( void )
{
	Ray_t rayDmg;
	Vector vForward = EyeDirection();
	Vector vEndPoint = EyePosition() + vForward * ROCKET_TRIPWIRE_RANGE;
	rayDmg.Init( EyePosition(), vEndPoint );

	trace_t traceDmg;
	CTraceFilterSimple subfilter( this, COLLISION_GROUP_NONE );
	CTraceFilterTranslateClones filter( &subfilter );
	float flRequiredParameter = 2.0f;
	CPortal_Base2D *pFirstPortal = UTIL_Portal_FirstAlongRay( rayDmg, flRequiredParameter );
	UTIL_Portal_TraceRay_Bullets( pFirstPortal, rayDmg, MASK_VISIBLE_AND_NPCS, &filter, &traceDmg, false );

	if ( traceDmg.m_pEnt && ( traceDmg.m_pEnt->IsPlayer() || IsAlternativeTripwireTarget( traceDmg.m_pEnt ) ) )
	{
		PrepareToFireRocket();
		m_OnTripped.FireOutput( this, traceDmg.m_pEnt );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Beep and fire after a short delay
//-----------------------------------------------------------------------------
void CPropRocketTripwire::PrepareToFireRocket( void )
{
	if ( m_State == DISABLED )
		return;

	SetThink( &CPropRocketTripwire::FireRocketThink );
	SetNextThink( gpGlobals->curtime + ROCKET_TRIPWIRE_FIRE_DELAY );
	EmitSound( "NPC_FloorTurret.LockedBeep" );
	m_State = FIRING;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::FireRocketThink( void )
{
	FireRocket();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::FireRocket( void )
{
	// Only one rocket out at a time
	if ( m_hCurRocket )
	{
		UTIL_RemoveImmediate( m_hCurRocket );
	}

	Vector vForward = EyeDirection();
	QAngle spawnAngles;
	VectorAngles( vForward, spawnAngles );

	CRocket_Turret_Projectile *pRocket = static_cast< CRocket_Turret_Projectile* >( CBaseEntity::Create( "rocket_turret_projectile", EyePosition(), spawnAngles, this ) );
	m_hCurRocket = pRocket;

	m_hRocket = pRocket;

	pRocket->SetAbsVelocity( vForward * m_flRocketSpeed );
	pRocket->SetLauncher( this );

	m_flTimeLastFired = gpGlobals->curtime;
	m_bHasRocketOut = true;

	SetThink( &CPropRocketTripwire::WaitingThink );
	SetNextThink( gpGlobals->curtime + 0.1 );
}

//-----------------------------------------------------------------------------
// Purpose: Carried objects and fast moving physics objects also trip the wire
//-----------------------------------------------------------------------------
bool CPropRocketTripwire::IsAlternativeTripwireTarget( CBaseEntity *pEnt )
{
	if ( pEnt == NULL )
		return false;

	if ( GetPlayerHoldingEntity( pEnt ) )
		return true;

	if ( pEnt->GetMoveType() == MOVETYPE_VPHYSICS )
	{
		Vector vVel;
		pEnt->GetVelocity( &vVel, NULL );
		if ( vVel.LengthSqr() > 100.0f )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Waits for the rocket to die, then reloads
//-----------------------------------------------------------------------------
void CPropRocketTripwire::PreFiringThink( void )
{
	CRocket_Turret_Projectile *pRocket = m_hCurRocket.Get();
	if ( pRocket )
	{
		// Self destruct rockets that have been out too long
		if ( gpGlobals->curtime - m_flTimeLastFired > m_flRocketLifetime )
		{
			pRocket->Explode();
			m_flTimeLastFired = gpGlobals->curtime;
		}
	}
	else if ( m_bHasRocketOut )
	{
		// The rocket just went away
		m_flRocketTimeOfDeath = gpGlobals->curtime;
		m_bHasRocketOut = false;
		m_OnRocketExplode.FireOutput( this, this );

		m_bDrawBeams = false;
		EmitSound( "NPC_TripwireTurret.Reload" );
	}
	else if ( m_flRocketTimeOfDeath + ROCKET_TRIPWIRE_RELOAD_TIME < gpGlobals->curtime && !m_bDisabled )
	{
		// Reloaded, arm the tripwire again
		m_State = ACTIVE;
		m_bDrawBeams = true;
		EmitSound( "NPC_RocketTurret.LockingBeep" );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::InputForceFire( inputdata_t &inputdata )
{
	PrepareToFireRocket();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::InputEnable( inputdata_t &inputdata )
{
	EnableTripwire();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::InputDisable( inputdata_t &inputdata )
{
	DisableTripwire();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::EnableTripwire( void )
{
	m_State = ACTIVE;
	m_bDrawBeams = true;
	m_bDisabled = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRocketTripwire::DisableTripwire( void )
{
	m_State = DISABLED;
	m_bDrawBeams = false;
	m_bDisabled = true;
}
