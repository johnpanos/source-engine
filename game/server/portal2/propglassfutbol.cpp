//========= Portal 2 reconstruction ============================================//
//
// Purpose: Glass futbol physics prop with its spawner, socket and catcher.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "props.h"
#include "portal_player.h"
#include "portal_base2d.h"
#include "portal_util_shared.h"
#include "portal_grabcontroller_shared.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_futbol_fake_force( "sv_futbol_fake_force", "500" );
ConVar sv_futbol_force_players_to_catch( "sv_futbol_force_players_to_catch", "1" );
ConVar sv_futbol_use_cooldown_time( "sv_futbol_use_cooldown_time", "0.7" );
ConVar sv_futbol_use_steals_from_holding_player( "sv_futbol_use_steals_from_holding_player", "1" );

class CPropGlassFutbol;
class CPropFutbolSpawner;

#define GLASS_FUTBOL_MODEL			"models/props/futbol.mdl"
#define GLASS_FUTBOL_SPAWNER_MODEL	"models/props/futbol_dispenser.mdl"
#define GLASS_FUTBOL_SOCKET_MODEL	"models/props/futbol_socket.mdl"

const char *g_szFutbolAnimThinkContext = "FutbolAnimateThinkContext";
const char *g_szFutbolThrownThinkContext = "FutbolThrownThinkContext";


//-----------------------------------------------------------------------------
// Purpose: Trace filter that only hits glass futbols
//-----------------------------------------------------------------------------
class CFilterOnlyGlassFutbol : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		if ( staticpropmgr->IsStaticProp( pServerEntity ) )
			return false;

		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
		return ( pEntity && FClassnameIs( pEntity, "prop_glass_futbol" ) );
	}

	virtual TraceType_t GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}
};


//-----------------------------------------------------------------------------
// Purpose: Holds a glass futbol that enters its catch box
//-----------------------------------------------------------------------------
class CFutbolCatcher : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFutbolCatcher, CBaseAnimating );
	DECLARE_DATADESC();

	CFutbolCatcher();
	virtual void Spawn( void );

protected:
	EHANDLE m_hCaughtFutbol;

	Vector m_vCatcherBoxHalfDiagonal;

	// Catch box, relative to the catch attachment
	Vector m_vecCatchBoxMins;
	Vector m_vecCatchBoxMaxs;
	Vector m_vecCatchBoxOrig;

	bool m_bDisableRecaptureOnPlayerGrab;

	COutputEvent m_OnFutbolReleased;
	COutputEvent m_OnFutbolCaught;

	virtual void CatchThink( void );
	virtual void CaptureThink( void );

	virtual void CaptureFutbol( CPropGlassFutbol *pFutbol );
};

LINK_ENTITY_TO_CLASS( futbol_catcher, CFutbolCatcher );

BEGIN_DATADESC( CFutbolCatcher )

	DEFINE_FIELD( m_hCaughtFutbol, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecCatchBoxMins, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecCatchBoxMaxs, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecCatchBoxOrig, FIELD_VECTOR ),
	DEFINE_FIELD( m_bDisableRecaptureOnPlayerGrab, FIELD_BOOLEAN ),

	DEFINE_OUTPUT( m_OnFutbolReleased, "OnFutbolReleased" ),
	DEFINE_OUTPUT( m_OnFutbolCaught, "OnFutbolCaught" ),

	DEFINE_THINKFUNC( CatchThink ),
	DEFINE_THINKFUNC( CaptureThink ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Dispenses glass futbols and respawns them when they break
//-----------------------------------------------------------------------------
class CPropFutbolSpawner : public CFutbolCatcher
{
public:
	DECLARE_CLASS( CPropFutbolSpawner, CFutbolCatcher );
	DECLARE_DATADESC();

	CPropFutbolSpawner();

	virtual void Spawn( void );
	virtual void Precache( void );

	virtual void FutbolDestroyed( void );
	virtual void FutbolGrabbed( void );

private:
	bool m_bHasFutbol;

	COutputEvent m_OnFutbolSpawned;
	COutputEvent m_OnFutbolGrabbed;

	void InputForceSpawn( inputdata_t &data );

	virtual void SpawnFutbol( void );
};

LINK_ENTITY_TO_CLASS( prop_glass_futbol_spawner, CPropFutbolSpawner );

BEGIN_DATADESC( CPropFutbolSpawner )

	DEFINE_KEYFIELD( m_bHasFutbol, FIELD_BOOLEAN, "StartWithFutbol" ),

	DEFINE_OUTPUT( m_OnFutbolSpawned, "OnFutbolSpawned" ),
	DEFINE_OUTPUT( m_OnFutbolGrabbed, "OnFutbolGrabbed" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "ForceSpawn", InputForceSpawn ),

END_DATADESC()


enum futbol_holder_type_t
{
	FUTBOL_HELD_BY_NONE = 0,
	FUTBOL_HELD_BY_PLAYER,
	FUTBOL_HELD_BY_SPAWNER,
	FUTBOL_HELD_BY_CATCHER,

	FUTBOL_HELD_BY_COUNT
};


//-----------------------------------------------------------------------------
// Purpose: Glass futbol
//-----------------------------------------------------------------------------
class CPropGlassFutbol : public CPhysicsProp
{
public:
	DECLARE_CLASS( CPropGlassFutbol, CPhysicsProp );
	DECLARE_DATADESC();

	CPropGlassFutbol();

	virtual QAngle PreferredCarryAngles( void ) { return QAngle( 180.0f, -90.0f, 180.0f ); }
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer ) { return true; }

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void Event_Killed( const CTakeDamageInfo &info );

	// Adds a fake force after the ball is thrown
	void ThrownThink( void );

	// Plays the rotation animation
	void AnimThink( void );

	// Throw the ball along the player's aim when dropped
	virtual void OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason );

	// Track who's holding the ball
	virtual void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );

	// Spawner that respawns this ball when it is destroyed
	virtual void SetSpawner( CPropFutbolSpawner *pMySpawner );

	virtual int OnTakeDamage( const CTakeDamageInfo &info );

	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );

	futbol_holder_type_t GetHolder( void ) { return m_Holder; }
	void SetHolder( futbol_holder_type_t type );

	CPortal_Player *GetLastPlayerToHold( void ) { return m_hLastHeldByPlayer.Get(); }

private:
	CHandle<CPropFutbolSpawner> m_hSpawner;
	string_t m_strSpawnerName;
	futbol_holder_type_t m_Holder;
	Vector m_vecThrowDirection;
	CHandle<CPortal_Player> m_hLastHeldByPlayer;
};

LINK_ENTITY_TO_CLASS( prop_glass_futbol, CPropGlassFutbol );

BEGIN_DATADESC( CPropGlassFutbol )

	DEFINE_KEYFIELD( m_strSpawnerName, FIELD_STRING, "SpawnerName" ),

	DEFINE_FIELD( m_hSpawner, FIELD_EHANDLE ),
	DEFINE_FIELD( m_Holder, FIELD_INTEGER ),
	DEFINE_FIELD( m_vecThrowDirection, FIELD_VECTOR ),
	DEFINE_FIELD( m_hLastHeldByPlayer, FIELD_EHANDLE ),

	DEFINE_THINKFUNC( ThrownThink ),
	DEFINE_THINKFUNC( AnimThink ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Socket that accepts a glass futbol
//-----------------------------------------------------------------------------
class CPropFutbolSocket : public CFutbolCatcher
{
	DECLARE_CLASS( CPropFutbolSocket, CFutbolCatcher );
public:
	CPropFutbolSocket() {}

	virtual void Spawn( void );
	virtual void Precache( void );
};

LINK_ENTITY_TO_CLASS( prop_glass_futbol_socket, CPropFutbolSocket );


//-----------------------------------------------------------------------------
// Futbol catcher
//-----------------------------------------------------------------------------
CFutbolCatcher::CFutbolCatcher()
	: m_vCatcherBoxHalfDiagonal( 25.0f, 25.0f, 25.0f ),
	m_vecCatchBoxMins( vec3_origin ),
	m_vecCatchBoxMaxs( vec3_origin ),
	m_vecCatchBoxOrig( vec3_origin ),
	m_bDisableRecaptureOnPlayerGrab( false )
{
}


void CFutbolCatcher::Spawn( void )
{
	BaseClass::Spawn();

	GetAttachment( "ball", m_vecCatchBoxOrig );
	m_vecCatchBoxMins = -m_vCatcherBoxHalfDiagonal;
	m_vecCatchBoxMaxs = m_vCatcherBoxHalfDiagonal;

	SetThink( &CFutbolCatcher::CatchThink );
	SetNextThink( gpGlobals->curtime + gpGlobals->interval_per_tick );
}


void CFutbolCatcher::CatchThink( void )
{
	Ray_t ray;
	ray.Init( m_vecCatchBoxOrig, m_vecCatchBoxOrig, m_vecCatchBoxMins, m_vecCatchBoxMaxs );
	CFilterOnlyGlassFutbol filter;
	trace_t tr;
	UTIL_TraceRay( ray, MASK_SOLID, &filter, &tr );

	if ( tr.m_pEnt && tr.m_pEnt != GetWorldEntity() && !m_bDisableRecaptureOnPlayerGrab )
	{
		CPropGlassFutbol *pFutbol = dynamic_cast<CPropGlassFutbol*>( tr.m_pEnt );

		// Only catch balls that are loose or being carried
		if ( pFutbol && ( pFutbol->GetHolder() == FUTBOL_HELD_BY_NONE || pFutbol->GetHolder() == FUTBOL_HELD_BY_PLAYER ) )
		{
			CaptureFutbol( pFutbol );
		}
	}

	// Allow recapture once the released ball has left the catch box
	if ( tr.m_pEnt == NULL )
	{
		m_bDisableRecaptureOnPlayerGrab = false;
	}

	SetNextThink( gpGlobals->curtime + gpGlobals->interval_per_tick );
}


void CFutbolCatcher::CaptureFutbol( CPropGlassFutbol *pFutbol )
{
	if ( pFutbol == NULL )
		return;

	m_hCaughtFutbol = pFutbol;

	// Hold it in place
	IPhysicsObject *pPhysicsObject = pFutbol->VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->EnableMotion( false );
	}

	pFutbol->Teleport( &m_vecCatchBoxOrig, &vec3_angle, &vec3_origin );

	SetThink( &CFutbolCatcher::CaptureThink );

	pFutbol->SetHolder( FUTBOL_HELD_BY_CATCHER );
	m_OnFutbolCaught.FireOutput( pFutbol, pFutbol->GetLastPlayerToHold() );
}


void CFutbolCatcher::CaptureThink( void )
{
	// Go back to catching once the ball has been taken away
	CPropGlassFutbol *pFutbol = static_cast<CPropGlassFutbol*>( m_hCaughtFutbol.Get() );
	if ( pFutbol == NULL || pFutbol->GetHolder() != FUTBOL_HELD_BY_CATCHER )
	{
		SetThink( &CFutbolCatcher::CatchThink );
		m_OnFutbolReleased.FireOutput( this, this );
		m_hCaughtFutbol = NULL;
		m_bDisableRecaptureOnPlayerGrab = true;
	}

	SetNextThink( gpGlobals->curtime + gpGlobals->interval_per_tick );
}


//-----------------------------------------------------------------------------
// Glass futbol
//-----------------------------------------------------------------------------
CPropGlassFutbol::CPropGlassFutbol()
	: m_strSpawnerName( NULL_STRING ),
	m_Holder( FUTBOL_HELD_BY_NONE ),
	m_vecThrowDirection( vec3_origin )
{
}


void CPropGlassFutbol::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel( GLASS_FUTBOL_MODEL );
}


void CPropGlassFutbol::Spawn( void )
{
	Precache();
	KeyValue( "model", GLASS_FUTBOL_MODEL );

	// Find our spawner if one wasn't handed to us
	if ( m_hSpawner.Get() == NULL )
	{
		CBaseEntity *pEnt = NULL;
		if ( m_strSpawnerName != NULL_STRING )
		{
			pEnt = gEntList.FindEntityByName( NULL, m_strSpawnerName );
		}

		if ( pEnt )
		{
			CPropFutbolSpawner *pSpawner = dynamic_cast<CPropFutbolSpawner*>( pEnt );
			if ( pSpawner )
			{
				m_hSpawner = pSpawner;
			}
			else
			{
				// Reconstruction note: the 2010 build formats this warning through the
				// failed (NULL) cast result; the entity that was found is used instead.
				Warning( "prop_glass_futbol has an invalid spawner set (%s: %s). It will not respawn.\n", pEnt->GetClassname(), pEnt->GetDebugName() );
			}
		}
		else
		{
			Warning( "prop_glass_futbol has an no spawner set. It will not respawn.\n" );
		}
	}

	BaseClass::Spawn();

	ResetSequence( LookupSequence( "rot" ) );
	SetContextThink( &CPropGlassFutbol::AnimThink, gpGlobals->curtime + 0.1f, g_szFutbolAnimThinkContext );

	AddSpawnFlags( SF_PHYSPROP_ENABLE_ON_PHYSCANNON | SF_PHYSPROP_ENABLE_PICKUP_OUTPUT );
}


void CPropGlassFutbol::Event_Killed( const CTakeDamageInfo &info )
{
	// Fizzled balls vanish without breaking into gibs
	if ( info.GetAttacker() && FClassnameIs( info.GetAttacker(), "trigger_portal_cleanser" ) )
	{
		CBaseEntity::Event_Killed( info );
	}
	else
	{
		BaseClass::Event_Killed( info );
	}

	if ( m_hSpawner.Get() )
	{
		m_hSpawner->FutbolDestroyed();
	}
}


void CPropGlassFutbol::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	CPortal_Player *pHoldingPlayer = dynamic_cast<CPortal_Player*>( pPhysGunUser );

	m_hLastHeldByPlayer = pHoldingPlayer;

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->EnableMotion( true );
	}

	if ( reason == PICKED_UP_BY_PLAYER && pHoldingPlayer && sv_futbol_use_steals_from_holding_player.GetBool() )
	{
		pHoldingPlayer->SetUseKeyCooldownTime( sv_futbol_use_cooldown_time.GetFloat() );
	}

	if ( m_hSpawner.Get() )
	{
		m_hSpawner->FutbolGrabbed();
	}

	// Steal it from whoever is holding it now
	if ( m_Holder == FUTBOL_HELD_BY_PLAYER )
	{
		CBasePlayer *pOtherPlayer = GetPlayerHoldingEntity( this );
		if ( pOtherPlayer )
		{
			pOtherPlayer->ForceDropOfCarriedPhysObjects( NULL );
		}
	}

	m_Holder = FUTBOL_HELD_BY_PLAYER;
}


void CPropGlassFutbol::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	if ( pPhysGunUser == NULL )
		return;

	BaseClass::OnPhysGunDrop( pPhysGunUser, reason );

	Vector forward;
	pPhysGunUser->EyeVectors( &forward );

	// Throwing through a portal: aim along the transformed view direction
	if ( ToPortalPlayer( pPhysGunUser )->IsHeldObjectOnOppositeSideOfPortal() )
	{
		CPortal_Base2D *pPortal = ToPortalPlayer( pPhysGunUser )->GetHeldObjectPortal();
		UTIL_Portal_VectorTransform( pPortal->m_matrixThisToLinked, forward, forward );
	}

	m_vecThrowDirection = forward;
	SetContextThink( &CPropGlassFutbol::ThrownThink, gpGlobals->curtime + gpGlobals->interval_per_tick, g_szFutbolThrownThinkContext );

	m_Holder = FUTBOL_HELD_BY_NONE;
}


void CPropGlassFutbol::SetSpawner( CPropFutbolSpawner *pMySpawner )
{
	m_hSpawner = pMySpawner;
}


int CPropGlassFutbol::OnTakeDamage( const CTakeDamageInfo &info )
{
	// Can't be damaged while sitting in a spawner or catcher
	if ( m_Holder != FUTBOL_HELD_BY_SPAWNER && m_Holder != FUTBOL_HELD_BY_CATCHER )
	{
		return BaseClass::OnTakeDamage( info );
	}

	return 0;
}


void CPropGlassFutbol::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	// Players with free hands automatically catch loose balls that hit them
	if ( sv_futbol_force_players_to_catch.GetBool() )
	{
		CBaseEntity *pHitEntity = pEvent->pEntities[!index];
		if ( pHitEntity->IsPlayer() )
		{
			CPortal_Player *pPlayer = dynamic_cast<CPortal_Player*>( pHitEntity );
			if ( pPlayer && GetPlayerHeldEntity( pPlayer ) == NULL && m_Holder == FUTBOL_HELD_BY_NONE )
			{
				pPlayer->PickupObject( this, true );

				// Reconstruction note: the 2010 build passes the cooldown ConVar's
				// boolean value (0 or 1 seconds), not its float value.
				pPlayer->SetUseKeyCooldownTime( sv_futbol_use_cooldown_time.GetBool() );
			}
		}
	}

	BaseClass::VPhysicsCollision( index, pEvent );
}


void CPropGlassFutbol::SetHolder( futbol_holder_type_t type )
{
	m_Holder = type;
}


void CPropGlassFutbol::ThrownThink( void )
{
	Vector vecForce = m_vecThrowDirection * sv_futbol_fake_force.GetFloat();

	IPhysicsObject *pPhys = VPhysicsGetObject();
	if ( pPhys )
	{
		pPhys->SetVelocityInstantaneous( &vec3_origin, NULL );
		pPhys->ApplyForceCenter( vecForce );
	}

	ApplyAbsVelocityImpulse( vecForce );

	SetContextThink( NULL, TICK_NEVER_THINK, g_szFutbolThrownThinkContext );
}


void CPropGlassFutbol::AnimThink( void )
{
	StudioFrameAdvance();
	SetNextThink( gpGlobals->curtime + gpGlobals->interval_per_tick, g_szFutbolAnimThinkContext );
}


//-----------------------------------------------------------------------------
// Futbol spawner
//-----------------------------------------------------------------------------
CPropFutbolSpawner::CPropFutbolSpawner()
	: m_bHasFutbol( false )
{
	m_vCatcherBoxHalfDiagonal = Vector( 7.0f, 7.0f, 7.0f );
}


void CPropFutbolSpawner::Spawn( void )
{
	Precache();
	SetModel( GLASS_FUTBOL_SPAWNER_MODEL );
	SetSolid( SOLID_VPHYSICS );

	BaseClass::Spawn();

	if ( m_bHasFutbol )
	{
		SpawnFutbol();
	}
}


void CPropFutbolSpawner::Precache( void )
{
	PrecacheModel( GLASS_FUTBOL_SPAWNER_MODEL );
}


void CPropFutbolSpawner::FutbolDestroyed( void )
{
	SpawnFutbol();
}


void CPropFutbolSpawner::FutbolGrabbed( void )
{
	if ( m_bHasFutbol )
	{
		m_OnFutbolGrabbed.FireOutput( this, this );
	}

	m_bHasFutbol = false;
}


void CPropFutbolSpawner::InputForceSpawn( inputdata_t &data )
{
	SpawnFutbol();
}


void CPropFutbolSpawner::SpawnFutbol( void )
{
	CPropGlassFutbol *pBall = static_cast<CPropGlassFutbol*>( CreateEntityByName( "prop_glass_futbol" ) );
	if ( pBall )
	{
		m_OnFutbolSpawned.FireOutput( this, this );

		Vector vecBallSpawnPoint;
		GetAttachment( "ball", vecBallSpawnPoint );
		pBall->SetAbsOrigin( vecBallSpawnPoint );
		pBall->SetSpawner( this );
		pBall->SetHolder( FUTBOL_HELD_BY_SPAWNER );

		m_bHasFutbol = true;

		DispatchSpawn( pBall );

		CaptureFutbol( pBall );
	}
}


//-----------------------------------------------------------------------------
// Futbol socket
//-----------------------------------------------------------------------------
void CPropFutbolSocket::Spawn( void )
{
	Precache();
	SetModel( GLASS_FUTBOL_SOCKET_MODEL );
	SetSolid( SOLID_VPHYSICS );

	int nSequence = LookupSequence( "hold" );
	SetSequence( nSequence );

	BaseClass::Spawn();
}


void CPropFutbolSocket::Precache( void )
{
	PrecacheModel( GLASS_FUTBOL_SOCKET_MODEL );
}
