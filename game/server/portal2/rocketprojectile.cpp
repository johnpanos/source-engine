//========= Portal 2 reconstruction ============================================//
//
// Purpose: Rocket fired by the rocket tripwire turret.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "rocketprojectile.h"
#include "soundenvelope.h"
#include "explode.h"
#include "smoke_trail.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ROCKET_PROJECTILE_MODEL		"models/props_bts/rocket.mdl"
#define ROCKET_PROJECTILE_FIRE_SOUND	"NPC_FloorTurret.RocketFire"
#define ROCKET_PROJECTILE_LOOP_SOUND	"NPC_FloorTurret.RocketFlyLoop"

BEGIN_DATADESC( CRocket_Turret_Projectile )

	DEFINE_FIELD( m_hLauncher, FIELD_EHANDLE ),

	DEFINE_FUNCTION( MissileTouch ),
	DEFINE_SOUNDPATCH( m_pAmbientSound ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( rocket_turret_projectile, CRocket_Turret_Projectile );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetBlocksLOS( false );

	SetSolid( SOLID_BBOX );
	SetModel( ROCKET_PROJECTILE_MODEL );
	UTIL_SetSize( this, -Vector( 8, 8, 8 ), Vector( 8, 8, 8 ) );

	SetThink( NULL );

	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_BOUNCE );

	m_takedamage = DAMAGE_YES;
	m_iHealth = m_iMaxHealth = 100;
	m_bloodColor = DONT_BLEED;

	SetTouch( &CRocket_Turret_Projectile::MissileTouch );

	CreateSounds();
	CreateSmokeTrail();

	EmitSound( ROCKET_PROJECTILE_FIRE_SOUND );
}

//-----------------------------------------------------------------------------
// Purpose: Explode on contact, except with the launcher it came from
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::MissileTouch( CBaseEntity *pOther )
{
	Vector vVel = GetAbsVelocity();

	// Touched the launcher; fizzle out and tumble away
	if ( pOther == m_hLauncher.Get() )
	{
		Dissolve( NULL, gpGlobals->curtime + 0.1f, false, ENTITY_DISSOLVE_NORMAL );

		Vector vBounceVel = Vector( -vVel.x, -vVel.y, 200 ) * 0.1f;
		SetAbsVelocity( vBounceVel );

		QAngle vBounceAngles;
		VectorAngles( Vector( -vVel.x, -vVel.y, 200 ), vBounceAngles );
		SetAbsAngles( vBounceAngles );

		SetLocalAngularVelocity( QAngle( 180, 90, 45 ) );

		UTIL_Remove( m_hRocketTrail );

		SetSolid( SOLID_NONE );

		if ( m_hRocketTrail )
		{
			m_hRocketTrail->SetLifetime( 0.1f );
			m_hRocketTrail = NULL;
		}

		return;
	}

	// Don't touch triggers (but DO hit weapons)
	if ( pOther->IsSolidFlagSet( FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS ) && pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON )
		return;

	Explode();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::Explode( void )
{
	// Don't explode against the skybox. Just pretend that
	// the missile flies off into the distance.
	Vector forward;
	GetVectors( &forward, NULL, NULL );

	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + forward * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	m_takedamage = DAMAGE_NO;
	SetSolid( SOLID_NONE );

	if ( tr.fraction == 1.0 || !( tr.surface.flags & SURF_SKY ) )
	{
		DoExplosion();
	}

	if ( FClassnameIs( tr.m_pEnt, "func_breakable" ) )
	{
		// Rockets always destroy breakables
		CTakeDamageInfo info( this, this, tr.m_pEnt->GetHealth(), DMG_BLAST );
		info.SetDamagePosition( GetAbsOrigin() );
		info.SetDamageForce( Forward() * 500.0f );
		tr.m_pEnt->TakeDamage( info );
	}

	if ( m_hRocketTrail )
	{
		m_hRocketTrail->SetLifetime( 0.1f );
		m_hRocketTrail = NULL;
	}

	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( ROCKET_PROJECTILE_LOOP_SOUND );
	PrecacheScriptSound( ROCKET_PROJECTILE_FIRE_SOUND );

	PrecacheModel( ROCKET_PROJECTILE_MODEL );
}

//-----------------------------------------------------------------------------
// Purpose: Portal teleports break the trail; start a fresh one on the far side
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	if ( eventType != NOTIFY_EVENT_TELEPORT )
		return;

	// Once through a portal the launcher no longer owns the rocket
	SetOwnerEntity( NULL );

	UTIL_Remove( m_hRocketTrail );
	m_hRocketTrail = NULL;

	CreateSmokeTrail();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::SetLauncher( EHANDLE hLauncher )
{
	m_hLauncher = hLauncher;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::DoExplosion( void )
{
	// Explode
	ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), 200, 100,
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 100.0f, this );

	// Push physics objects the explosion can see
	string_t strTurretName = AllocPooledString( "npc_portal_turret_floor" );
	string_t strPropName = AllocPooledString( "prop_physics" );
	string_t strCubeName = AllocPooledString( "prop_weighted_cube" );

	Vector vecRadius( 128, 128, 128 );
	CBaseEntity *pEnts[128];
	int nNumFound = UTIL_EntitiesInBox( pEnts, ARRAYSIZE( pEnts ), GetAbsOrigin() - vecRadius, GetAbsOrigin() + vecRadius, 0 );
	for ( int i = 0; i < nNumFound; ++i )
	{
		if ( pEnts[i] == NULL )
			continue;

		if ( pEnts[i]->m_iClassname != strTurretName && pEnts[i]->m_iClassname != strPropName && pEnts[i]->m_iClassname != strCubeName )
			continue;

		trace_t tr;
		UTIL_TraceLine( GetAbsOrigin(), pEnts[i]->WorldSpaceCenter(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction < 1.0f )
			continue;

		Vector vForward;
		GetVectors( &vForward, NULL, NULL );

		Vector vDamagePos = GetAbsOrigin() - vForward * 5.0f;

		CTakeDamageInfo info( this, this, 200.0f, DMG_BLAST );
		info.SetDamagePosition( vDamagePos );
		CalculateExplosiveDamageForce( &info, pEnts[i]->GetAbsOrigin() - vDamagePos, vDamagePos );
		pEnts[i]->VPhysicsTakeDamage( info );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::CreateSounds( void )
{
	if ( !m_pAmbientSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

		CPASAttenuationFilter filter( this, 0.8f );

		m_pAmbientSound = controller.SoundCreate( filter, entindex(), ROCKET_PROJECTILE_LOOP_SOUND );
		controller.Play( m_pAmbientSound, 1.0f, 100 );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::StopLoopingSounds( void )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	controller.SoundDestroy( m_pAmbientSound );
	m_pAmbientSound = NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::CreateSmokeTrail( void )
{
	if ( m_hRocketTrail )
		return;

	// Smoke trail.
	if ( ( m_hRocketTrail = RocketTrail::CreateRocketTrail() ) != NULL )
	{
		m_hRocketTrail->m_Opacity = 0.2f;
		m_hRocketTrail->m_SpawnRate = 100;
		m_hRocketTrail->m_ParticleLifetime = 0.8f;
		m_hRocketTrail->m_StartColor.Init( 0.65f, 0.65f, 0.65f );
		m_hRocketTrail->m_EndColor.Init( 0.0, 0.0, 0.0 );
		m_hRocketTrail->m_StartSize = 8;
		m_hRocketTrail->m_EndSize = 32;
		m_hRocketTrail->m_SpawnRadius = 4;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 16;

		m_hRocketTrail->SetLifetime( 999 );
		m_hRocketTrail->FollowEntity( this, NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRocket_Turret_Projectile::UpdateOnRemove( void )
{
	StopLoopingSounds();

	if ( m_hRocketTrail )
	{
		UTIL_Remove( m_hRocketTrail );
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Debug command that fires a rocket from the player's eyes
//-----------------------------------------------------------------------------
static void fire_rocket_projectile_f( void )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();

	Vector ptEyes, vForward;
	QAngle vLookAng;
	ptEyes = pPlayer->EyePosition();
	pPlayer->EyeVectors( &vForward );
	vLookAng = pPlayer->EyeAngles();

	CRocket_Turret_Projectile *pRocket = (CRocket_Turret_Projectile *)CBaseEntity::Create( "rocket_turret_projectile", ptEyes, vLookAng, pPlayer );
	if ( !pRocket )
		return;

	pRocket->SetThink( NULL );
	pRocket->SetMoveType( MOVETYPE_FLY );

	pRocket->SetModel( ROCKET_PROJECTILE_MODEL );
	UTIL_SetSize( pRocket, vec3_origin, vec3_origin );

	pRocket->CreateSmokeTrail();

	pRocket->SetAbsVelocity( vForward * 550 );
	pRocket->SetLauncher( NULL );
}

ConCommand fire_rocket_projectile( "fire_rocket_projectile", fire_rocket_projectile_f, "Fires a rocket turret projectile from the player's eyes for testing.", FCVAR_CHEAT );
