//========= Portal 2 reconstruction ============================================//
//
// Purpose: Thermal discouragement beam (env_portal_laser). Moves through
//			portals, reflects off reflective cubes and powers laser targets.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "env_portal_laser.h"
#include "particle_parse.h"
#include "soundenvelope.h"
#include "info_placement_helper.h"
#include "point_laser_target.h"
#include "portal_player.h"
#include "portal_base2d.h"
#include "portal_util_shared.h"
#include "portalsimulation.h"
#include "physicsshadowclone.h"
#include "prop_weightedcube.h"
#include "npc_portal_turret_floor.h"
#include "props.h"
#include "decals.h"
#include "vprof.h"
#include "UtlSortVector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar hitbox_damage_enabled;

ConVar sv_player_collide_with_laser( "sv_player_collide_with_laser", "1", FCVAR_CHEAT );

ConVar new_portal_laser( "new_portal_laser", "1", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
ConVar portal_laser_normal_update( "portal_laser_normal_update", "0.05f", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
ConVar portal_laser_high_precision_update(
    "portal_laser_high_precision_update", "0.03f", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
ConVar sv_debug_laser( "sv_debug_laser", "0", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );
ConVar sv_laser_cube_autoaim( "sv_laser_cube_autoaim", "0", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );

int g_nTotalLaser = 0;

// Source send-table and data-description macros require their declaration layout.
// clang-format off
IMPLEMENT_SERVERCLASS_ST( CPortalLaser, DT_PortalLaser )
	SendPropEHandle( SENDINFO( m_hReflector ) ),
	// Reconstruction note: retail passes 2 (SPROP_COORD) in the nBits slot and
	// keeps the default SPROP_NOSCALE flags, so the points are sent as full floats.
	SendPropVector( SENDINFO( m_vStartPoint ), SPROP_COORD ),
	SendPropVector( SENDINFO( m_vEndPoint ), SPROP_COORD ),
	SendPropBool( SENDINFO( m_bLaserOn ) ),
	SendPropBool( SENDINFO( m_bIsLethal ) ),
	SendPropBool( SENDINFO( m_bIsAutoAiming ) ),
	SendPropBool( SENDINFO( m_bShouldSpark ) ),
	SendPropBool( SENDINFO( m_bUseParentDir ) ),
	SendPropQAngles( SENDINFO( m_angParentAngles ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( env_portal_laser, CPortalLaser );

BEGIN_DATADESC( CPortalLaser )

	DEFINE_THINKFUNC( StrikeThink ),

	DEFINE_FIELD( m_bFromReflectedCube, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hReflector, FIELD_EHANDLE ),
	DEFINE_FIELD( m_pChildLaser, FIELD_CLASSPTR ),
	DEFINE_AUTO_ARRAY( m_pSoundProxy, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_pPlacementHelper, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_bLaserOn, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iLaserAttachment, FIELD_INTEGER ),

	DEFINE_KEYFIELD( m_ModelName, FIELD_MODELNAME, "model" ),
	DEFINE_KEYFIELD( m_bStartOff, FIELD_BOOLEAN, "StartState" ),
	DEFINE_KEYFIELD( m_bIsLethal, FIELD_BOOLEAN, "LethalDamage" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

END_DATADESC()

IMPLEMENT_AUTO_LIST( IPortalLaserAutoList );
// clang-format on

static const char LASER_LOOPING_SOUND[] = "Laser.BeamLoop";
static const char LETHAL_LASER_LOOPING_SOUND[] = "LaserGreen.BeamLoop";
static const char LASER_BURN_SOUND[] = "HL2Player.BurnPain";

#define PORTAL_LASER_DEFAULT_MODEL	"models/props/laser_emitter.mdl"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CPortalLaser::CPortalLaser()
{
	m_pChildLaser = NULL;
	m_bFromReflectedCube = false;
	m_hReflector = NULL;
	m_vStartPoint = vec3_origin;
	m_vEndPoint = vec3_origin;
	m_bLaserOn = false;
	m_bIsAutoAiming = false;
	m_bShouldSpark = false;
	m_bUseParentDir = false;
	m_angParentAngles = vec3_angle;
	m_bGlowInitialized = false;

	++g_nTotalLaser;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CPortalLaser::~CPortalLaser()
{
	--g_nTotalLaser;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::Spawn()
{
	BaseClass::Spawn();

	Precache();

	// Lasers spawned off reflective cubes have no model of their own
	if ( !m_bFromReflectedCube )
	{
		if ( STRING( m_ModelName )[0] == '\0' )
		{
			SetModel( PORTAL_LASER_DEFAULT_MODEL );
		}
		else
		{
			SetModel( STRING( m_ModelName ) );
		}

		SetSolid( SOLID_VPHYSICS );

		m_iLaserAttachment = LookupAttachment( "laser_attachment" );
		if ( m_iLaserAttachment == 0 )
		{
			Warning( "env_portal_laser '%s' : model named '%s' does not have attachment 'laser_attachment'\n", STRING( GetEntityName() ), STRING( m_ModelName ) );
		}

		CreateVPhysics();
	}

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		m_pAmbientSound[i] = NULL;
	}

	CreateHelperEntities();

	if ( !m_bStartOff )
	{
		TurnOn();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CPortalLaser::UpdateTransmitState()
{
	// The client draws the whole beam, which can leave the emitter's PVS
	// through portals
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: Create the portal placement helper at the end of the beam
//-----------------------------------------------------------------------------
void CPortalLaser::CreateHelperEntities()
{
	if ( m_pPlacementHelper == NULL )
	{
		m_pPlacementHelper = static_cast< CInfoPlacementHelper* >( CreateEntityByName( "info_placement_helper" ) );
		m_pPlacementHelper->SetAbsOrigin( GetAbsOrigin() );
		m_pPlacementHelper->SetAbsAngles( GetAbsAngles() );
		m_pPlacementHelper->KeyValue( "radius", "16" );
		m_pPlacementHelper->KeyValue( "hide_until_placed", "0" );
		DispatchSpawn( m_pPlacementHelper );
	}

	CreateSoundProxies();
}

//-----------------------------------------------------------------------------
// Purpose: Each player hears the beam loop from its nearest point, so every
//			player gets a private proxy entity and looping sound.
//-----------------------------------------------------------------------------
void CPortalLaser::CreateSoundProxies()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i + 1 );
		if ( pPlayer == NULL || !pPlayer->IsConnected() )
			continue;

		if ( m_pSoundProxy[i] == NULL )
		{
			m_pSoundProxy[i] = CreateEntityByName( "info_target" );
			m_pSoundProxy[i]->SetAbsOrigin( GetAbsOrigin() );
			m_pSoundProxy[i]->AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
			m_pSoundProxy[i]->DispatchUpdateTransmitState();
		}

		if ( m_pAmbientSound[i] == NULL )
		{
			CSingleUserRecipientFilter filter( pPlayer );

			if ( m_bIsLethal )
			{
				m_pAmbientSound[i] = controller.SoundCreate( filter, m_pSoundProxy[i]->entindex(), LETHAL_LASER_LOOPING_SOUND );
			}
			else
			{
				m_pAmbientSound[i] = controller.SoundCreate( filter, m_pSoundProxy[i]->entindex(), LASER_LOOPING_SOUND );
			}

			controller.Play( m_pAmbientSound[i], 1.0f, 100 );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::Activate()
{
	CreateHelperEntities();

	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::UpdateOnRemove()
{
	if ( m_pPlacementHelper )
	{
		UTIL_Remove( m_pPlacementHelper );
	}

	TurnOff();

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::Precache()
{
	if ( m_bIsLethal )
	{
		PrecacheScriptSound( LETHAL_LASER_LOOPING_SOUND );
	}
	else
	{
		PrecacheScriptSound( LASER_LOOPING_SOUND );
	}

	PrecacheScriptSound( LASER_BURN_SOUND );

	PrecacheParticleSystem( "laser_start_glow" );
	PrecacheParticleSystem( "reflector_start_glow" );

	if ( m_bFromReflectedCube )
		return;

	if ( STRING( m_ModelName )[0] != '\0' )
	{
		PrecacheModel( STRING( m_ModelName ) );
	}
	else
	{
		PrecacheModel( PORTAL_LASER_DEFAULT_MODEL );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CPortalLaser::CreateVPhysics()
{
	VPhysicsInitStatic();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: The laser is on while it is thinking
//-----------------------------------------------------------------------------
bool CPortalLaser::IsOn()
{
	return ( m_pfnThink != NULL );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::InputTurnOn( inputdata_t &inputdata )
{
	if ( IsOn() )
		return;

	TurnOn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::InputTurnOff( inputdata_t &inputdata )
{
	if ( !IsOn() )
		return;

	TurnOff();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::InputToggle( inputdata_t &inputdata )
{
	if ( IsOn() )
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::TurnOff()
{
	m_bLaserOn = false;
	m_bShouldSpark = false;

	RemoveChildLaser();

	TurnOffGlow();

	TurnOffLaserSound();

	SetNextThink( TICK_NEVER_THINK );
	SetThink( NULL );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::TurnOn()
{
	m_bLaserOn = true;

	if ( IsOn() )
		return;

	SetThink( &CPortalLaser::StrikeThink );

	// Reconstruction note: the binary picks the high precision rate for lasers
	// that are NOT spawned from reflective cubes here, while StrikeThink uses the
	// opposite selection. Both are kept as found.
	float flNextUpdate = m_bFromReflectedCube ? portal_laser_normal_update.GetFloat() : portal_laser_high_precision_update.GetFloat();
	SetNextThink( gpGlobals->curtime + flNextUpdate );
}

//-----------------------------------------------------------------------------
// Purpose: Legacy impact damage at the end of the beam
//-----------------------------------------------------------------------------
void CPortalLaser::BeamDamage( trace_t *ptr )
{
	if ( ptr->fraction != 1.0f )
	{
		CBaseEntity *pHit = ptr->m_pEnt;
		if ( pHit )
		{
			ClearMultiDamage();

			Vector dir = ptr->endpos - GetAbsOrigin();
			VectorNormalize( dir );

			int nDamageType = DMG_BURN;

			CTakeDamageInfo info( this, this, 150.0f * gpGlobals->frametime, nDamageType );
			CalculateMeleeDamageForce( &info, dir, ptr->endpos );
			pHit->DispatchTraceAttack( info, dir, ptr );
			ApplyMultiDamage();

			UTIL_DecalTrace( ptr, "RedGlowFade" );

			CBaseAnimating *pAnim = pHit->GetBaseAnimating();
			if ( pAnim && ( FClassnameIs( pAnim, "npc_portal_turret_floor" ) || FClassnameIs( pAnim, "npc_hover_turret" ) ) )
			{
				if ( hitbox_damage_enabled.GetBool() )
				{
					CTakeDamageInfo info;
					info.SetDamage( 1.0f );
					info.SetDamageType( DMG_CRUSH );

					CNPC_Portal_FloorTurret *pTurret = static_cast< CNPC_Portal_FloorTurret* >( pAnim );
					pTurret->Event_Killed( info );
					pTurret->DisabledThink();
					pTurret->SetThink( NULL );
					return;
				}

				pAnim->Ignite( 30.0f );
				return;
			}

			if ( pHit->GetMoveType() == MOVETYPE_VPHYSICS )
			{
				CPhysicsProp *pProp = dynamic_cast< CPhysicsProp* >( pHit );
				if ( pProp )
				{
					pProp->Ignite( 30.0f, false );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::TurnOnGlow()
{
	m_bGlowInitialized = true;

	CBaseEntity *pReflector = m_hReflector.Get();
	if ( UTIL_IsSchrodinger( pReflector ) )
		return;

	if ( m_bFromReflectedCube )
	{
		DispatchParticleEffect(
		    "reflector_start_glow", PATTACH_ABSORIGIN_FOLLOW, pReflector, 0, true );
	}
	else
	{
		DispatchParticleEffect(
		    "laser_start_glow", PATTACH_POINT_FOLLOW, this, m_iLaserAttachment, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::TurnOffGlow()
{
	m_bGlowInitialized = false;

	CBaseEntity *pGlowEntity = m_bFromReflectedCube ? m_hReflector.Get() : this;
	if ( pGlowEntity )
	{
		StopParticleEffects( pGlowEntity );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::TurnOffLaserSound()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( m_pAmbientSound[i] )
		{
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			controller.SoundDestroy( m_pAmbientSound[i] );
			m_pAmbientSound[i] = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: The beam terminates at tr.endpos
//-----------------------------------------------------------------------------
void CPortalLaser::FireAtPoint( trace_t &tr, bool bImpact )
{
	BeamDamage( &tr );

	if ( bImpact )
	{
		// The client places the sparks at the end of its own trace
		m_bShouldSpark = true;
	}
	else
	{
		m_bShouldSpark = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Track the point of this beam segment closest to each player
//-----------------------------------------------------------------------------
void CPortalLaser::UpdateSoundPosition( const Vector &vecStart, const Vector &vecEnd )
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i + 1 );
		if ( pPlayer == NULL || !pPlayer->IsConnected() )
		{
			m_vecNearestSoundSource[i] = GetAbsOrigin();
			continue;
		}

		Vector vecPlayer = pPlayer->EyePosition();
		Vector vecNearestPoint;
		CalcClosestPointOnLineSegment( vecPlayer, vecStart, vecEnd, vecNearestPoint );

		float flDistToPlayerSqr = ( vecPlayer - m_vecNearestSoundSource[i] ).LengthSqr();
		float flNearestToPlayerSqr = ( vecPlayer - vecNearestPoint ).LengthSqr();

		if ( flDistToPlayerSqr > flNearestToPlayerSqr || m_vecNearestSoundSource[i] == vec3_invalid )
		{
			m_vecNearestSoundSource[i] = vecNearestPoint;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Laser damage is applied per second
//-----------------------------------------------------------------------------
void CPortalLaser::DamageEntity( CBaseEntity *pVictim, float flAmount )
{
	CTakeDamageInfo info( this, this, flAmount * gpGlobals->frametime, DMG_BURN );
	CalculateMeleeDamageForce( &info, Vector( 1, 0, 0 ), pVictim->WorldSpaceCenter() );
	pVictim->TakeDamage( info );
}

struct LaserVictimInfo_t
{
	CBaseEntity *pVictim;
	float flFraction;
};

class CLaserVictimLess
{
public:
	bool Less( const LaserVictimInfo_t &lhs, const LaserVictimInfo_t &rhs, void *pCtx )
	{
		return ( lhs.flFraction < rhs.flFraction );
	}
};

//-----------------------------------------------------------------------------
// Purpose: Width of the box swept along the beam when looking for victims.
// Reconstruction note: both enumerators grow the width by (length / 256) up
// to three times, the effect of a side-effecting argument to the legacy clamp()
// macro whose result was unused. The binary's arithmetic is kept here.
//-----------------------------------------------------------------------------
static float LaserSweepWidth( float flLength )
{
	const float flStep = flLength / 256.0f;
	float flLaserWidth = 16.0f;

	flLaserWidth += flStep;
	if ( flLaserWidth <= 128.0f )
	{
		flLaserWidth += flStep;
		if ( flLaserWidth >= 16.0f )
		{
			flLaserWidth += flStep;
		}
	}

	return flLaserWidth;
}

//-----------------------------------------------------------------------------
// Purpose: Pushes players away from the beam and burns them.
// Shared by the legacy and the current damage paths.
//-----------------------------------------------------------------------------
static void LaserPushPlayer( CBaseEntity *pEntity, const Vector &vecStart, const Vector &vecEnd, Vector &vecPushVelocity, bool &bSkip )
{
	bSkip = false;

	Vector vecPlayerVelocity = pEntity->GetAbsVelocity();
	VectorNormalize( vecPlayerVelocity );

	Vector vecNearestPoint;
	CalcClosestPointOnLineSegment( pEntity->GetAbsOrigin(), vecStart, vecEnd, vecNearestPoint );

	Vector vecPlayerPos = pEntity->GetAbsOrigin();
	vecPlayerPos.z = vecNearestPoint.z;

	Vector vecLineToLaser = vecPlayerPos - vecNearestPoint;
	VectorNormalize( vecLineToLaser );
	vecLineToLaser.z = 0.0f;

	Vector vecBounce;

	if ( vecPlayerVelocity.LengthSqr() < ( FLT_EPSILON * FLT_EPSILON ) )
	{
		// Standing still, push perpendicular to the beam
		Vector vecDirection = vecEnd - vecStart;
		VectorNormalize( vecDirection );
		vecBounce = CrossProduct( vecDirection, Vector( 0, 0, 1 ) );
	}
	else
	{
		// Reflect the player's velocity off the beam
		float flDot = DotProduct( vecPlayerVelocity, vecLineToLaser );
		vecBounce = vecPlayerVelocity + flDot * ( vecLineToLaser * -2.0f );
		VectorNormalize( vecBounce );
		vecBounce.z = 0.0f;

		// Moving away from the beam already
		if ( DotProduct( vecBounce, vecLineToLaser ) < 0.0f )
		{
			bSkip = true;
			return;
		}
	}

	vecPushVelocity = vecBounce * 100.0f;

	if ( pEntity->GetFlags() & FL_ONGROUND )
	{
		pEntity->SetGroundEntity( NULL );
		pEntity->SetGroundChangeTime( gpGlobals->curtime + 0.5f );
		vecPushVelocity *= 2.0f;
	}

	pEntity->SetAbsVelocity( vecPushVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: Legacy (new_portal_laser 0) victim search and damage
//-----------------------------------------------------------------------------
bool CPortalLaser::StrikeEntitiesAlongLaser( const Vector &vecStart, const Vector &vecEnd, Vector *pVecOut )
{
	Vector vecDirection = vecEnd - vecStart;

	float flLaserWidth = LaserSweepWidth( vecDirection.Length() );
	VectorNormalize( vecDirection );

	Vector vecMins( -flLaserWidth, -flLaserWidth, -flLaserWidth );
	Vector vecMaxs( flLaserWidth, flLaserWidth, flLaserWidth );

	CBaseEntity *list[512];
	Ray_t ray;
	ray.Init( vecStart, vecEnd, vecMins, vecMaxs );
	CFlaggedEntitiesEnum rayEnum( list, ARRAYSIZE( list ), FL_CLIENT | FL_NPC | FL_OBJECT );
	int nCount = UTIL_EntitiesAlongRay( ray, &rayEnum );
	CBaseEntity *pEntity = NULL;

	CUtlSortVector< LaserVictimInfo_t, CLaserVictimLess > vsrtVictims;

	for ( int i = 0; i < nCount; ++i )
	{
		pEntity = list[i];

		if ( !FClassnameIs( pEntity, "point_laser_target" ) && !FClassnameIs( pEntity, "npc_portal_turret_floor" ) && !pEntity->IsPlayer() )
			continue;

		if ( !pEntity->IsAlive() )
			continue;

		Vector vecNearest;
		float flFraction;
		CalcClosestPointOnLineSegment( pEntity->GetAbsOrigin(), vecStart, vecEnd, vecNearest, &flFraction );

		float flNearestDistSqr = ( vecNearest - pEntity->GetAbsOrigin() ).Length2DSqr();
		if ( flNearestDistSqr > 256.0f && !m_bFromReflectedCube )
			continue;

		if ( pEntity->IsPlayer() )
		{
			if ( !sv_player_collide_with_laser.GetBool() )
				continue;

			// Jumping players may clear a beam that is below their center
			bool bIsJumping = ( pEntity->GetGroundEntity() == NULL );
			if ( bIsJumping )
			{
				float flZDist = pEntity->WorldSpaceCenter().z - vecNearest.z;
				if ( flZDist >= 0.0f )
					continue;
			}

			CPortal_Player *pPlayer = ToPortalPlayer( pEntity );
			if ( vecNearest.z > pPlayer->GetAbsOrigin().z + pPlayer->GetHullMaxs().z )
				continue;

			if ( flNearestDistSqr > 256.0f )
				continue;
		}

		if ( flFraction > 0.0f )
		{
			LaserVictimInfo_t victim;
			victim.pVictim = pEntity;
			victim.flFraction = flFraction;
			vsrtVictims.InsertNoSort( victim );
		}
	}

	vsrtVictims.RedoSort();

	bool bBestIsTurret = false;
	for ( int i = 0; i < vsrtVictims.Count(); ++i )
	{
		pEntity = vsrtVictims[i].pVictim;

		if ( FClassnameIs( pEntity, "point_laser_target" ) && !bBestIsTurret )
		{
			DamageEntity( pEntity, 1.0f );

			CPortalLaserTarget *pLaserTarget = dynamic_cast< CPortalLaserTarget* >( pEntity );
			if ( pLaserTarget && pLaserTarget->IsTerminalPoint() )
			{
				if ( pVecOut )
				{
					*pVecOut = pLaserTarget->WorldSpaceCenter();
				}

				return true;
			}
		}
		else if ( FClassnameIs( pEntity, "npc_portal_turret_floor" ) )
		{
			// Turrets block targets behind them
			bBestIsTurret = true;
		}
		else if ( pEntity->IsPlayer() && pEntity->GetMoveType() != MOVETYPE_NOCLIP )
		{
			Vector vecPushVelocity;
			bool bSkip;
			LaserPushPlayer( pEntity, vecStart, vecEnd, vecPushVelocity, bSkip );
			if ( bSkip )
				continue;

			DamageEntity( pEntity, m_bIsLethal ? 100000.0f : 150.0f );
			pEntity->EmitSound( LASER_BURN_SOUND );
		}
	}

	if ( pVecOut )
	{
		*pVecOut = vecEnd;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Damage the victims found along each beam segment
//-----------------------------------------------------------------------------
void CPortalLaser::DamageEntitiesAlongLaser( const CUtlVector< PortalLaserInfo_t > &infoList, bool bAutoAim )
{
	bool bBlockTarget = false;
	bool bDebug = sv_debug_laser.GetBool();

	for ( int i = 0; i < infoList.Count(); ++i )
	{
		const CUtlVector< CBaseEntity* > &victimList = infoList[i].sortedEntList;
		const Vector &vecStart = infoList[i].vecStart;
		const Vector &vecEnd = infoList[i].vecEnd;

		for ( int j = 0; j < victimList.Count(); ++j )
		{
			CBaseEntity *pEntity = victimList[j];

			if ( bDebug )
			{
				ICollideable *pCollision = pEntity->GetCollideable();
				if ( pCollision )
				{
					NDebugOverlay::BoxAngles( pCollision->GetCollisionOrigin(), pCollision->OBBMins(), pCollision->OBBMaxs(), pCollision->GetCollisionAngles(), 255, 255, 0, 0, 0.0f );
				}
			}

			if ( FClassnameIs( pEntity, "point_laser_target" ) && !bBlockTarget )
			{
				CPortalLaserTarget *pTarget = static_cast< CPortalLaserTarget* >( pEntity );
				if ( pTarget->IsTerminalPoint() )
				{
					m_bShouldSpark = false;

					// Only auto aimed (or reflected) beams power terminal targets
					if ( bAutoAim || m_bFromReflectedCube )
					{
						DamageEntity( pTarget, 1.0f );
					}
				}
				else
				{
					DamageEntity( pTarget, 1.0f );
				}
			}
			else if ( FClassnameIs( pEntity, "npc_portal_turret_floor" ) )
			{
				bBlockTarget = true;
			}
			else if ( pEntity->IsPlayer() && pEntity->GetMoveType() != MOVETYPE_NOCLIP )
			{
				Vector vecPushVelocity;
				bool bSkip;
				LaserPushPlayer( pEntity, vecStart, vecEnd, vecPushVelocity, bSkip );
				if ( bSkip )
					continue;

				DamageEntity( pEntity, m_bIsLethal ? 100000.0f : 150.0f );
				pEntity->EmitSound( LASER_BURN_SOUND );
			}
		}
	}
}

ConVar sv_laser_tight_box( "sv_laser_tight_box", "1.25f", FCVAR_DEVELOPMENTONLY );

//-----------------------------------------------------------------------------
// Purpose: Records the segment and its sorted victims; returns the terminal
//			laser target hit along it, if any.
//-----------------------------------------------------------------------------
CBaseEntity *CPortalLaser::GetEntitiesAlongLaser( const Vector &vecStart, const Vector &vecEnd, Vector *pVecOut, CUtlVector< PortalLaserInfo_t > &infoList, bool bIsFirstTrace )
{
	PortalLaserInfo_t &info = infoList[ infoList.AddToTail() ];
	info.vecStart = vecStart;
	info.vecEnd = vecEnd;

	Vector vecDirection = vecEnd - vecStart;

	// The auto-aim probe uses a wide box, the real beam a tight one
	float flLaserWidth;
	if ( bIsFirstTrace )
	{
		flLaserWidth = LaserSweepWidth( vecDirection.Length() );
	}
	else
	{
		flLaserWidth = sv_laser_tight_box.GetFloat();
	}

	VectorNormalize( vecDirection );

	Vector vecMins( -flLaserWidth, -flLaserWidth, -flLaserWidth );
	Vector vecMaxs( flLaserWidth, flLaserWidth, flLaserWidth );

	if ( sv_debug_laser.GetBool() )
	{
		Vector tempMax( flLaserWidth + 1500.0f, flLaserWidth, flLaserWidth );
		if ( bIsFirstTrace )
		{
			NDebugOverlay::BoxDirection( vecStart, vecMins, tempMax, vecDirection.Normalized(), 255, 0, 0, 0, 0.1f );
		}
		else
		{
			NDebugOverlay::BoxDirection( vecStart, vecMins, tempMax, vecDirection.Normalized(), 0, 255, 0, 0, 0.1f );
		}
	}

	CBaseEntity *list[512];
	Ray_t ray;
	ray.Init( vecStart, vecEnd, vecMins, vecMaxs );
	CFlaggedEntitiesEnum rayEnum( list, ARRAYSIZE( list ), FL_CLIENT | FL_NPC | FL_OBJECT );
	int nCount = UTIL_EntitiesAlongRay( ray, &rayEnum );
	CBaseEntity *pEntity = NULL;

	CUtlSortVector< LaserVictimInfo_t, CLaserVictimLess > vsrtVictims;

	for ( int i = 0; i < nCount; ++i )
	{
		pEntity = list[i];

		if ( !FClassnameIs( pEntity, "point_laser_target" ) && !FClassnameIs( pEntity, "npc_portal_turret_floor" ) && !pEntity->IsPlayer() )
			continue;

		if ( !pEntity->IsAlive() )
			continue;

		Vector vecNearest;
		float flFraction;
		CalcClosestPointOnLineSegment( pEntity->GetAbsOrigin(), vecStart, vecEnd, vecNearest, &flFraction );

		float flNearestDistSqr = ( vecNearest - pEntity->GetAbsOrigin() ).Length2DSqr();
		if ( flNearestDistSqr > 256.0f && !m_bFromReflectedCube )
			continue;

		if ( pEntity->IsPlayer() )
		{
			if ( !sv_player_collide_with_laser.GetBool() )
				continue;

			// Jumping players may clear a beam that is below their center
			bool bIsJumping = ( pEntity->GetGroundEntity() == NULL );
			if ( bIsJumping )
			{
				float flZDist = pEntity->WorldSpaceCenter().z - vecNearest.z;
				if ( flZDist >= 0.0f )
					continue;
			}

			// The beam must pass between the player's feet and head
			CPortal_Player *pPlayer = ToPortalPlayer( pEntity );
			if ( vecNearest.z > pPlayer->GetAbsOrigin().z + pPlayer->GetHullMaxs().z )
				continue;

			if ( pPlayer->GetAbsOrigin().z > vecNearest.z )
				continue;

			if ( flNearestDistSqr > 256.0f )
				continue;
		}

		if ( flFraction > 0.0f )
		{
			LaserVictimInfo_t victim;
			victim.pVictim = pEntity;
			victim.flFraction = flFraction;
			vsrtVictims.InsertNoSort( victim );
		}
	}

	vsrtVictims.RedoSort();

	bool bBlockTarget = false;
	for ( int i = 0; i < vsrtVictims.Count(); ++i )
	{
		pEntity = vsrtVictims[i].pVictim;
		info.sortedEntList.AddToTail( pEntity );

		if ( FClassnameIs( pEntity, "point_laser_target" ) && !bBlockTarget )
		{
			CPortalLaserTarget *pLaserTarget = dynamic_cast< CPortalLaserTarget* >( pEntity );
			if ( pLaserTarget && pLaserTarget->IsTerminalPoint() )
			{
				if ( pVecOut )
				{
					*pVecOut = pLaserTarget->WorldSpaceCenter();
				}

				return pEntity;
			}
		}
		else if ( FClassnameIs( pEntity, "npc_portal_turret_floor" ) )
		{
			// Turrets block targets behind them
			bBlockTarget = true;
		}
	}

	if ( pVecOut )
	{
		*pVecOut = vecEnd;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Should the beam bend toward this entity?
//-----------------------------------------------------------------------------
bool CPortalLaser::ShouldAutoAim( CBaseEntity *pEntity )
{
	// Retail (852 decompile) answers false for no entity; TraceLaser passes
	// NULL when the beam hit nothing it should aim at.
	if ( !pEntity || !FClassnameIs( pEntity, "point_laser_target" ) )
		return false;

	CPortalLaserTarget *pLaserTarget = dynamic_cast< CPortalLaserTarget* >( pEntity );
	bool bShouldAutoAimTarget = ( pLaserTarget && pLaserTarget->IsTerminalPoint() );

	if ( m_bFromReflectedCube )
	{
		return sv_laser_cube_autoaim.GetBool() && bShouldAutoAimTarget;
	}

	return bShouldAutoAimTarget;
}

//-----------------------------------------------------------------------------
// Purpose: Traces the beam through any number of portals, collecting the
//			segments and their victims.
//-----------------------------------------------------------------------------
CBaseEntity *CPortalLaser::TraceLaser( bool bIsFirstTrace, const Vector &vecStart, const Vector &vecDirection, float &flTotalBeamLength, trace_t &tr, CUtlVector< PortalLaserInfo_t > &infoList, Vector *pVecAutoAimOffset )
{
	flTotalBeamLength = 0.0f;

	Vector vStart = vecStart;
	Vector vDir = vecDirection;

	CTraceFilterSimpleClassnameList traceFilter( this, COLLISION_GROUP_NONE );
	traceFilter.AddClassnameToIgnore( "projected_wall_entity" );
	traceFilter.AddClassnameToIgnore( "player" );
	traceFilter.AddClassnameToIgnore( "point_laser_target" );

	CTraceFilterSimple parentFilter( GetParent(), COLLISION_GROUP_NONE );
	CTraceFilterChain traceChain( &traceFilter, &parentFilter );

	CUtlVector< CPortal_Base2D* > portalList;

	CBaseEntity *pHitEntity = NULL;
	bool bGoingThroughPortal = true;
	bool bDebug = sv_debug_laser.GetBool();

	while ( bGoingThroughPortal )
	{
		UTIL_ClearTrace( tr );

		Ray_t ray;
		ray.Init( vStart, vStart + vDir * MAX_TRACE_LENGTH );

		UTIL_TraceRay( ray, MASK_SHOT & ~CONTENTS_WINDOW, &traceChain, &tr );

		if ( bDebug )
		{
			if ( pVecAutoAimOffset )
			{
				NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 255, 0, true, 0.1f );
			}
			else
			{
				NDebugOverlay::Line( tr.startpos, tr.endpos, 0, 0, 255, true, 0.1f );
			}
		}

		UpdateSoundPosition( tr.startpos, tr.endpos );

		CPortal_Base2D *pFirstPortal = NULL;
		if ( UTIL_DidTraceTouchPortals( ray, tr, &pFirstPortal ) && pFirstPortal && pFirstPortal->IsActivedAndLinked() )
		{
			Vector vStrike;
			pHitEntity = GetEntitiesAlongLaser( tr.startpos, tr.endpos, &vStrike, infoList, bIsFirstTrace );

			Vector vPointOnLine;
			CalcClosestPointOnLineSegment( vStrike, tr.startpos, tr.endpos, vPointOnLine );
			flTotalBeamLength += ( vPointOnLine - tr.startpos ).Length();

			if ( pHitEntity )
			{
				if ( pVecAutoAimOffset )
				{
					*pVecAutoAimOffset = pHitEntity->WorldSpaceCenter() - vPointOnLine;
				}

				break;
			}

			// Continue the trace out of the linked portal
			Ray_t rayTransformed;
			UTIL_Portal_RayTransform( pFirstPortal->m_matrixThisToLinked, ray, rayTransformed );

			vDir = rayTransformed.m_Delta;
			VectorNormalize( vDir );

			UTIL_Portal_PointTransform( pFirstPortal->m_matrixThisToLinked, tr.endpos, vStart );
		}
		else
		{
			Vector vStrike;
			pHitEntity = GetEntitiesAlongLaser( tr.startpos, tr.endpos, &vStrike, infoList, bIsFirstTrace );
			if ( pHitEntity )
			{
				Vector vPointOnLine;
				CalcClosestPointOnLineSegment( vStrike, tr.startpos, tr.endpos, vPointOnLine );
				flTotalBeamLength += ( vPointOnLine - tr.startpos ).Length();

				if ( pVecAutoAimOffset )
				{
					*pVecAutoAimOffset = pHitEntity->WorldSpaceCenter() - vPointOnLine;
				}
			}
			else
			{
				flTotalBeamLength += ( tr.endpos - tr.startpos ).Length();
			}

			bGoingThroughPortal = false;
			break;
		}

		// Remember the portals we went through (newest first) to map the auto-aim offset back
		portalList.AddToHead( pFirstPortal->GetLinkedPortal() );
	}

	// The beam itself hit a target solid
	if ( pHitEntity == NULL && ShouldAutoAim( tr.m_pEnt ) )
	{
		pHitEntity = CPhysicsShadowClone::IsShadowClone( tr.m_pEnt ) ? static_cast< CPhysicsShadowClone* >( tr.m_pEnt )->GetClonedEntity().Get() : tr.m_pEnt;

		if ( pVecAutoAimOffset )
		{
			Vector vPointOnLine;
			CalcClosestPointOnLine( pHitEntity->WorldSpaceCenter(), tr.startpos, tr.endpos, vPointOnLine );
			*pVecAutoAimOffset = pHitEntity->WorldSpaceCenter() - vPointOnLine;
		}
	}

	if ( ShouldAutoAim( pHitEntity ) && pVecAutoAimOffset )
	{
		Vector vAutoAimOffset = *pVecAutoAimOffset;
		for ( int i = 0; i < portalList.Count(); ++i )
		{
			CPortal_Base2D *pPortal = portalList[i];
			UTIL_Portal_VectorTransform( pPortal->m_matrixThisToLinked, vAutoAimOffset, vAutoAimOffset );
		}

		*pVecAutoAimOffset = vAutoAimOffset;
	}

	return pHitEntity;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::RemoveChildLaser()
{
	if ( m_pChildLaser )
	{
		UTIL_Remove( m_pChildLaser );
		m_pChildLaser = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Continue the beam with a child laser (through a portal or off a cube)
//-----------------------------------------------------------------------------
void CPortalLaser::UpdateNextLaser( const Vector &vecStart, const Vector &vecDirection, CBaseEntity *pParent )
{
	if ( m_pChildLaser == NULL )
	{
		m_pChildLaser = static_cast< CPortalLaser* >( CreateEntityByName( "env_portal_laser" ) );

		if ( pParent && UTIL_IsReflectiveCube( pParent ) )
		{
			m_pChildLaser->m_hReflector = pParent;
			m_pChildLaser->SetParent( pParent );
			m_pChildLaser->m_bFromReflectedCube = true;
		}

		DispatchSpawn( m_pChildLaser );
	}
	else if ( pParent && UTIL_IsReflectiveCube( pParent ) && m_pChildLaser->m_hReflector.Get() != pParent )
	{
		// Moved on to a different cube; the old one no longer holds the laser
		CPropWeightedCube *pOldCube = static_cast< CPropWeightedCube* >( m_pChildLaser->m_hReflector.Get() );
		// Reconstruction note: the binary calls SetLaser without checking the old
		// cube, which is NULL for a laser that was not reflected before.
		if ( pOldCube )
		{
			pOldCube->SetLaser( NULL );
		}

		m_pChildLaser->m_hReflector = pParent;
		m_pChildLaser->SetParent( pParent );
		m_pChildLaser->m_bFromReflectedCube = true;
	}

	m_pChildLaser->FireLaser( vecStart, vecDirection, pParent );
}

//-----------------------------------------------------------------------------
// Purpose: Redirects the beam if pEntity is a reflective cube
//-----------------------------------------------------------------------------
bool CPortalLaser::ReflectLaserFromEntity( CBaseEntity *pEntity )
{
	if ( UTIL_IsReflectiveCube( pEntity ) )
	{
		CPropWeightedCube *pCube = static_cast< CPropWeightedCube* >( pEntity );

		// Already redirecting another laser
		if ( pCube->HasLaser() )
			return true;

		Vector vecForward;
		AngleVectors( pCube->GetAbsAngles(), &vecForward );

		Vector vecOffset = pCube->WorldSpaceCenter() + vecForward * 22.0f;

		// If the cube is in a portal and the laser would come out the other side, transform it
		CPortalSimulator *pSimulator = CPortalSimulator::GetSimulatorThatOwnsEntity( pCube );
		if ( pSimulator && pSimulator->EntityIsInPortalHole( pCube ) )
		{
			const VPlane &portalPlane = pSimulator->GetInternalData().Placement.PortalPlane;
			if ( portalPlane.DistTo( vecOffset ) < 0.0f && portalPlane.DistTo( pCube->WorldSpaceCenter() ) > 0.0f )
			{
				const VMatrix &matThisToLinked = pSimulator->GetInternalData().Placement.matThisToLinked;
				vecOffset = matThisToLinked * vecOffset;
				vecForward = matThisToLinked.ApplyRotation( vecForward );
			}
		}

		UTIL_Portal_Laser_Prevent_Tilting( vecForward );

		UpdateNextLaser( vecOffset, vecForward, pCube );

		pCube->SetLaser( m_pChildLaser );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Fires the laser from vecStart along vecDirection
//-----------------------------------------------------------------------------
void CPortalLaser::FireLaser( const Vector &vecStart, const Vector &vecDirection, CBaseEntity *pParent )
{
	if ( new_portal_laser.GetBool() )
	{
		float flTotalBeamLength;
		trace_t tr;
		Vector vAutoAimOffset;
		Vector vDir = vecDirection;

		// First trace with the wide box to see if a target should pull the beam
		CUtlVector< PortalLaserInfo_t > firstInfoList;
		CBaseEntity *pHitTarget = TraceLaser( true, vecStart, vDir, flTotalBeamLength, tr, firstInfoList, &vAutoAimOffset );
		bool bAutoAimSuccess = false;
		if ( ShouldAutoAim( pHitTarget ) )
		{
			trace_t tempTrace;
			Vector vNewEndPoint = vecStart + vecDirection * flTotalBeamLength + vAutoAimOffset;
			Vector vNewDirection = ( vNewEndPoint - vecStart ).Normalized();
			float flNewBeamLength;

			// Aim at the target; it is only valid if the bent beam still reaches it
			CUtlVector< PortalLaserInfo_t > secondInfoList;
			CBaseEntity *pSecondHitTarget = TraceLaser( false, vecStart, vNewDirection, flNewBeamLength, tempTrace, secondInfoList, NULL );

			if ( sv_debug_laser.GetBool() )
			{
				// Reconstruction note: the binary prints the entity pointers with %d.
				engine->Con_NPrintf( 0, "first hit = %p", pHitTarget );
				engine->Con_NPrintf( 1, "second hit = %p", pSecondHitTarget );
			}

			bAutoAimSuccess = ( pHitTarget == pSecondHitTarget );
			if ( bAutoAimSuccess )
			{
				tr = tempTrace;
				flTotalBeamLength = flNewBeamLength;
				vDir = vNewDirection;

				DamageEntitiesAlongLaser( secondInfoList, true );
			}
			else
			{
				DamageEntitiesAlongLaser( firstInfoList, false );
			}
		}
		else
		{
			// No target to aim at, trace the real beam
			CUtlVector< PortalLaserInfo_t > secondInfoList;
			UTIL_ClearTrace( tr );
			TraceLaser( false, vecStart, vDir, flTotalBeamLength, tr, secondInfoList, NULL );
			DamageEntitiesAlongLaser( secondInfoList, false );
		}

		// The client re-traces this line through portals to draw it
		m_vStartPoint = vecStart;
		m_vEndPoint = vecStart + vDir * flTotalBeamLength;
		m_bIsAutoAiming = bAutoAimSuccess;

		// Targets stop the beam (retail skips the test when nothing was hit)
		if ( pHitTarget && FClassnameIs( pHitTarget, "point_laser_target" ) )
			return;

		if ( tr.m_pEnt )
		{
			CBaseEntity *pEnt = CPhysicsShadowClone::IsShadowClone( tr.m_pEnt ) ? static_cast< CPhysicsShadowClone* >( tr.m_pEnt )->GetClonedEntity().Get() : tr.m_pEnt;
			if ( ReflectLaserFromEntity( pEnt ) )
				return;
		}

		RemoveChildLaser();

		UTIL_SetOrigin( m_pPlacementHelper, tr.endpos );
		FireAtPoint( tr, true );
	}
	else
	{
		CTraceFilterSimpleClassnameList traceFilter( this, COLLISION_GROUP_NONE );
		traceFilter.AddClassnameToIgnore( "projected_wall_entity" );
		traceFilter.AddClassnameToIgnore( "player" );
		traceFilter.AddClassnameToIgnore( "point_laser_target" );

		Ray_t ray;
		ray.Init( vecStart, vecStart + vecDirection * MAX_TRACE_LENGTH );

		trace_t tr;
		UTIL_TraceRay( ray, MASK_SHOT & ~CONTENTS_WINDOW, &traceFilter, &tr );

		UpdateSoundPosition( tr.startpos, tr.endpos );

		CPortal_Base2D *pFirstPortal = NULL;
		if ( UTIL_DidTraceTouchPortals( ray, tr, &pFirstPortal ) && pFirstPortal && pFirstPortal->IsActivedAndLinked() )
		{
			Vector vecNewTermPoint;
			if ( StrikeEntitiesAlongLaser( tr.startpos, tr.endpos, &vecNewTermPoint ) )
			{
				m_vStartPoint = vecStart;
				m_vEndPoint = vecNewTermPoint;
			}
			else
			{
				m_vStartPoint = vecStart;
				m_vEndPoint = tr.endpos;

				FireAtPoint( tr, false );

				// Continue out of the linked portal with a child laser
				Ray_t rayTransformed;
				UTIL_Portal_RayTransform( pFirstPortal->m_matrixThisToLinked, ray, rayTransformed );

				Vector vecDirection = rayTransformed.m_Delta;
				VectorNormalize( vecDirection );

				Vector vecStartPos;
				UTIL_Portal_PointTransform( pFirstPortal->m_matrixThisToLinked, tr.endpos, vecStartPos );

				UpdateNextLaser( vecStartPos, vecDirection, NULL );
			}
		}
		else
		{
			Vector vecNewTermPoint;
			m_vStartPoint = vecStart;

			if ( StrikeEntitiesAlongLaser( tr.startpos, tr.endpos, &vecNewTermPoint ) )
			{
				m_vEndPoint = vecNewTermPoint;
			}
			else
			{
				m_vEndPoint = tr.endpos;

				if ( tr.m_pEnt )
				{
					CBaseEntity *pEnt = CPhysicsShadowClone::IsShadowClone( tr.m_pEnt ) ? static_cast< CPhysicsShadowClone* >( tr.m_pEnt )->GetClonedEntity().Get() : tr.m_pEnt;
					if ( ReflectLaserFromEntity( pEnt ) )
						return;
				}

				RemoveChildLaser();

				UTIL_SetOrigin( m_pPlacementHelper, tr.endpos );
				FireAtPoint( tr, true );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPortalLaser::StrikeThink()
{
	VPROF_BUDGET( "CPortalLaser::StrikeThink", VPROF_BUDGETGROUP_GAME );

	m_bShouldSpark = false;

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		m_vecNearestSoundSource[i] = vec3_invalid;
	}

	Vector vecDir;
	Vector vecOrigin;
	CBaseEntity *pParent = m_hReflector.Get();
	if ( pParent && m_bFromReflectedCube )
	{
		// Reflected lasers come out of the front of the cube. The client
		// computes the same start point (C_PortalLaser::ClientThink).
		m_bUseParentDir = false;
		AngleVectors( pParent->GetAbsAngles(), &vecDir );
		vecOrigin = pParent->WorldSpaceCenter() + vecDir * 22.0f;

		// A cube sticking out of a portal emits from the other side
		CPortalSimulator *pSimulator = CPortalSimulator::GetSimulatorThatOwnsEntity( pParent );
		if ( pSimulator && pSimulator->EntityIsInPortalHole( pParent ) )
		{
			const VPlane &portalPlane = pSimulator->GetInternalData().Placement.PortalPlane;
			if ( portalPlane.DistTo( vecOrigin ) < 0.0f &&
			     portalPlane.DistTo( pParent->WorldSpaceCenter() ) > 0.0f )
			{
				const VMatrix &matThisToLinked =
				    pSimulator->GetInternalData().Placement.matThisToLinked;
				vecOrigin = matThisToLinked * vecOrigin;
				vecDir = matThisToLinked.ApplyRotation( vecDir );
			}
		}

		SetNextThink( gpGlobals->curtime + portal_laser_high_precision_update.GetFloat() );
	}
	else
	{
		GetAttachment( m_iLaserAttachment, vecOrigin, &vecDir );

		SetNextThink( gpGlobals->curtime + portal_laser_normal_update.GetFloat() );
	}

	UTIL_Portal_Laser_Prevent_Tilting( vecDir );

	// The glow starts once the first beam fires
	if ( !m_bGlowInitialized )
	{
		TurnOnGlow();
	}

	FireLaser( vecOrigin, vecDir, pParent );

	CreateSoundProxies();

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( m_pSoundProxy[i] )
		{
			UTIL_SetOrigin( m_pSoundProxy[i], m_vecNearestSoundSource[i] );
		}
	}

	if ( sv_debug_laser.GetBool() )
	{
		engine->Con_NPrintf( 0, "num lasers = %d", g_nTotalLaser );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Closest point on the visible beam to vPos
//-----------------------------------------------------------------------------
Vector CPortalLaser::ClosestPointOnLineSegment( const Vector &vPos )
{
	// Reconstruction note: not present in the 2010 binaries. The retail
	// consumer uses it to measure a cube's distance to the beam, so this
	// measures against the beam's current start and end points.
	Vector vClosest;
	CalcClosestPointOnLineSegment( vPos, m_vStartPoint, m_vEndPoint, vClosest );
	return vClosest;
}
