//========= Portal 2 reconstruction ============================================//
//
// Purpose: Exploding futbol, its spawner/dispenser, socket and generic catcher.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "prop_exploding_futbol.h"
#include "portal2/portal/portal_player.h"
#include "portal2/portal/portal_base2d.h"
#include "portal_util_shared.h"
#include "portal_grabcontroller_shared.h"
#include "indicator_panel.h"
#include "explode.h"
#include "fmtstr.h"
#include "debugoverlay_shared.h"
#include "movevars_shared.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *g_szExplodingFutbolAnimThinkContext = "ExplodingFutbolAnimateThinkContext";
const char *g_szExplodingFutbolTimerThinkContext = "ExplodingFutbolTimerThinkContext";
const char *g_szExplodingFutbolKillThinkContext = "ExplodingFutbolKillThinkContext";

ConVar exploding_futbol_explosion_debug( "exploding_futbol_explosion_debug", "0", FCVAR_CHEAT,
    "Debug the explosion of the exploding futbol." );
ConVar exploding_futbol_use_cooldown_time( "exploding_futbol_use_cooldown_time", "0.7f",
    FCVAR_CHEAT, "The cooldown time for the use key after the player picks up the futbol." );
ConVar exploding_futbol_explosion_radius( "exploding_futbol_explosion_radius", "200", FCVAR_CHEAT,
    "The radius of the explosion for the exploding futbol." );
ConVar exploding_futbol_explosion_magnitude( "exploding_futbol_explosion_magnitude", "0",
    FCVAR_CHEAT, "The magnitude of the explosion for the exploding futbol." );
ConVar exploding_futbol_explosion_damage( "exploding_futbol_explosion_damage", "250.0f",
    FCVAR_CHEAT, "The damage of the explosion for the exploding futbol." );
ConVar exploding_futbol_explosion_damage_falloff( "exploding_futbol_explosion_damage_falloff",
    "0.75f", FCVAR_CHEAT, "The percentage of damage taken at the edge of the explosion." );
ConVar exploding_futbol_start_color( "exploding_futbol_start_color", "255 255 255 255", FCVAR_CHEAT,
    "The starting color of the exploding futbol." );
ConVar exploding_futbol_end_color( "exploding_futbol_end_color", "255 106 0 255", FCVAR_CHEAT,
    "The ending color of the exploding futbol, before it starts the final explode sequence." );
ConVar exploding_futbol_flash_start_color( "exploding_futbol_flash_start_color", "255 255 0 255",
    FCVAR_CHEAT, "The start color for the futbol flashing before it explodes." );
ConVar exploding_futbol_flash_end_color( "exploding_futbol_flash_end_color", "255 0 0 255",
    FCVAR_CHEAT, "The final color of the exploding futbol, right before it explodes." );
ConVar exploding_futbol_flash_start_time( "exploding_futbol_flash_start_time", "3.0f", FCVAR_CHEAT,
    "The time before the futbol explodes when it start to flash." );
ConVar exploding_futbol_flash_duration( "exploding_futbol_flash_duration", "1.0f", FCVAR_CHEAT,
    "The flash duration of the exploding futbol, right before it explodes." );
ConVar exploding_futbol_hit_breakables( "exploding_futbol_hit_breakables", "1", FCVAR_CHEAT,
    "If the exploding futbol should hit breakable entities." );
ConVar exploding_futbol_explode_on_fizzle( "exploding_futbol_explode_on_fizzle", "0", FCVAR_CHEAT,
    "If the exploding futbol should explode when it fizzles." );
ConVar sv_futbol_funnel_max_correct( "sv_futbol_funnel_max_correct", "128.f", FCVAR_DEVELOPMENTONLY,
    "Max distance to move our hit-target if there's a portal nearby it" );

// Entities the explosion is allowed to damage (NULL terminated)
static const char *g_psxExplodableEntities[] = { "npc_portal_turret_floor", "prop_weighted_cube",
    "prop_physics", "func_breakable", "player", NULL };

#define EXPLODING_FUTBOL_MODEL "models/props/futbol.mdl"
#define EXPLODING_FUTBOL_SPAWNER_MODEL "models/props/futbol_dispenser.mdl"
#define EXPLODING_FUTBOL_SOCKET_MODEL "models/props/futbol_socket.mdl"

static Color FutbolColor( const ConVar &colorVar )
{
	color32 parsedColor;
	UTIL_StringToColor32( &parsedColor, colorVar.GetString() );
	return Color( parsedColor.r, parsedColor.g, parsedColor.b, parsedColor.a );
}

//-----------------------------------------------------------------------------
// Purpose: Trace filter that only hits exploding futbols
//-----------------------------------------------------------------------------
class CFilterOnlyExplodingFutbol : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		if ( staticpropmgr->IsStaticProp( pServerEntity ) )
			return false;

		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
		return ( pEntity && FClassnameIs( pEntity, "prop_exploding_futbol" ) );
	}

	virtual TraceType_t GetTraceType() const { return TRACE_ENTITIES_ONLY; }
};

//-----------------------------------------------------------------------------
// Purpose: Holds an exploding futbol that enters its catch box
//-----------------------------------------------------------------------------
class CExplodingFutbolCatcher : public CBaseAnimating
{
public:
	DECLARE_CLASS( CExplodingFutbolCatcher, CBaseAnimating );
	DECLARE_DATADESC();

	CExplodingFutbolCatcher();

	virtual void Spawn( void );

	virtual void DrawDebugGeometryOverlays( void );

protected:
	virtual void CatchThink( void );
	virtual void CaptureThink( void );

	virtual void CaptureFutbol( CPropExplodingFutbol *pFutbol );

	EHANDLE m_hCaughtFutbol;
	bool m_bDisableRecaptureOnPlayerGrab;

	// Catch box, relative to the catch attachment
	Vector m_vecCatcherBoxMins;
	Vector m_vecCatcherBoxMaxs;
	Vector m_vecCatcherBoxOrigin;
	Vector m_vecCatcherBoxHalfDiagonal;

	COutputEvent m_OnFutbolReleased;
	COutputEvent m_OnFutbolCaught;
};

BEGIN_DATADESC( CExplodingFutbolCatcher )

DEFINE_FIELD( m_hCaughtFutbol, FIELD_EHANDLE ), DEFINE_FIELD( m_vecCatcherBoxMins, FIELD_VECTOR ),
    DEFINE_FIELD( m_vecCatcherBoxMaxs, FIELD_VECTOR ),
    DEFINE_FIELD( m_vecCatcherBoxOrigin, FIELD_VECTOR ),
    DEFINE_FIELD( m_bDisableRecaptureOnPlayerGrab, FIELD_BOOLEAN ),

    DEFINE_OUTPUT( m_OnFutbolReleased, "OnFutbolReleased" ),
    DEFINE_OUTPUT( m_OnFutbolCaught, "OnFutbolCaught" ),

    DEFINE_THINKFUNC( CatchThink ), DEFINE_THINKFUNC( CaptureThink ),

    END_DATADESC()

        LINK_ENTITY_TO_CLASS( exploding_futbol_catcher, CExplodingFutbolCatcher );

//-----------------------------------------------------------------------------
// Purpose: Dispenses exploding futbols and respawns them when they are destroyed
//-----------------------------------------------------------------------------
class CPropExplodingFutbolSpawner : public CExplodingFutbolCatcher
{
public:
	DECLARE_CLASS( CPropExplodingFutbolSpawner, CExplodingFutbolCatcher );
	DECLARE_DATADESC();

	CPropExplodingFutbolSpawner();

	virtual void Spawn( void );
	virtual void Precache( void );

	virtual void FutbolDestroyed( void );
	virtual void FutbolGrabbed( CPropExplodingFutbol *pFutbol, bool bSamePlayer );

	CPropIndicatorPanel *GetTimerPanel( void );

private:
	virtual void SpawnFutbol( void );

	void InputForceSpawn( inputdata_t &data );

	COutputEvent m_OnFutbolSpawned;
	COutputEvent m_OnFutbolGrabbed;

	bool m_bHasFutbol;
	bool m_bIsTimed;
	float m_flTimer;
	string_t m_strTimerIndicator;
};

BEGIN_DATADESC( CPropExplodingFutbolSpawner )

DEFINE_KEYFIELD( m_bHasFutbol, FIELD_BOOLEAN, "StartWithFutbol" ),
    DEFINE_KEYFIELD( m_bIsTimed, FIELD_BOOLEAN, "IsTimed" ),
    DEFINE_KEYFIELD( m_flTimer, FIELD_FLOAT, "Timer" ),
    DEFINE_KEYFIELD( m_strTimerIndicator, FIELD_STRING, "TimerIndicatorName" ),

    DEFINE_INPUTFUNC( FIELD_VOID, "ForceSpawn", InputForceSpawn ),

    DEFINE_OUTPUT( m_OnFutbolSpawned, "OnFutbolSpawned" ),
    DEFINE_OUTPUT( m_OnFutbolGrabbed, "OnFutbolGrabbed" ),

    END_DATADESC()

        LINK_ENTITY_TO_CLASS( prop_exploding_futbol_spawner, CPropExplodingFutbolSpawner );

//-----------------------------------------------------------------------------
// Purpose: Socket that accepts an exploding futbol
//-----------------------------------------------------------------------------
class CPropExplodingFutbolSocket : public CExplodingFutbolCatcher
{
public:
	DECLARE_CLASS( CPropExplodingFutbolSocket, CExplodingFutbolCatcher );

	CPropExplodingFutbolSocket() {}

	virtual void Precache( void );
	virtual void Spawn( void );

	virtual void CaptureFutbol( CPropExplodingFutbol *pFutbol );
};

LINK_ENTITY_TO_CLASS( prop_exploding_futbol_socket, CPropExplodingFutbolSocket );

//-----------------------------------------------------------------------------
// Exploding futbol
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CPropExplodingFutbol )

DEFINE_KEYFIELD( m_strSpawnerName, FIELD_STRING, "SpawnerName" ),
    DEFINE_KEYFIELD( m_bShouldRespawn, FIELD_BOOLEAN, "ShouldRespawn" ),
    DEFINE_KEYFIELD( m_bExplodeOnTouch, FIELD_BOOLEAN, "ExplodeOnTouch" ),

    DEFINE_FIELD( m_hSpawner, FIELD_EHANDLE ), DEFINE_FIELD( m_Holder, FIELD_INTEGER ),
    DEFINE_FIELD( m_hLastHeldByPlayer, FIELD_EHANDLE ),
    DEFINE_FIELD( m_bTimerActive, FIELD_BOOLEAN ), DEFINE_FIELD( m_flExplosionTimer, FIELD_FLOAT ),
    DEFINE_FIELD( m_flTotalTimer, FIELD_FLOAT ), DEFINE_FIELD( m_flLastTickTime, FIELD_FLOAT ),
    DEFINE_FIELD( m_flLastTimerSoundTime, FIELD_FLOAT ),
    DEFINE_FIELD( m_flLastFlashTime, FIELD_FLOAT ),

    DEFINE_THINKFUNC( AnimThink ), DEFINE_THINKFUNC( TimerThink ), DEFINE_THINKFUNC( KillThink ),

    DEFINE_INPUTFUNC( FIELD_VOID, "Explode", InputExplode ),

    END_DATADESC()

        LINK_ENTITY_TO_CLASS( prop_exploding_futbol, CPropExplodingFutbol );

CPropExplodingFutbol::CPropExplodingFutbol()
    : m_Holder( EXPLODING_FUTBOL_HELD_BY_NONE ), m_strSpawnerName( NULL_STRING ),
      m_bTimerActive( false ), m_bExplodeOnTouch( true ), m_flExplosionTimer( 0.0f ),
      m_flTotalTimer( 0.0f ), m_flLastTickTime( 0.0f ), m_flLastTimerSoundTime( 0.0f ),
      m_flLastFlashTime( 0.0f )
{
}

void CPropExplodingFutbol::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel( EXPLODING_FUTBOL_MODEL );

	PrecacheScriptSound( "NPC_FloorTurret.DeployingKlaxon" );
	PrecacheScriptSound( "Portal.room1_TickTock" );
	PrecacheScriptSound( "EnergyBall.Explosion" );
}

void CPropExplodingFutbol::Spawn( void )
{
	SetModelName( MAKE_STRING( EXPLODING_FUTBOL_MODEL ) );
	KeyValue( "model", EXPLODING_FUTBOL_MODEL );

	if ( m_bShouldRespawn )
	{
		// Find our spawner if one wasn't handed to us
		if ( m_hSpawner.Get() == NULL )
		{
			if ( m_strSpawnerName != NULL_STRING )
			{
				CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, m_strSpawnerName );
				if ( pEnt )
				{
					CPropExplodingFutbolSpawner *pSpawner =
					    dynamic_cast<CPropExplodingFutbolSpawner *>( pEnt );
					if ( pSpawner )
					{
						m_hSpawner = pSpawner;
					}
					else
					{
						// Reconstruction note: the 2010 build formats this warning through
						// the failed (NULL) cast result; the entity that was found is used instead.
						Warning( "prop_exploding_futbol has an invalid spawner set (%s: %s). It "
						         "will not respawn.\n",
						    pEnt->GetClassname(), pEnt->GetDebugName() );
					}
				}
			}
			else
			{
				Warning( "prop_exploding_futbol has no spawner set. It will not respawn.\n" );
			}
		}
	}

	BaseClass::Spawn();

	ResetSequence( LookupSequence( "rot" ) );
	SetContextThink( &CPropExplodingFutbol::AnimThink, gpGlobals->curtime + 0.1f,
	    g_szExplodingFutbolAnimThinkContext );

	AddSpawnFlags( SF_PHYSPROP_ENABLE_ON_PHYSCANNON | SF_PHYSPROP_ENABLE_PICKUP_OUTPUT );
}

//-----------------------------------------------------------------------------
// Purpose: Funnel falling futbols into floor portals below them
//-----------------------------------------------------------------------------
void CPropExplodingFutbol::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );

	Vector vVelocity, vPosition;
	pPhysics->GetVelocity( &vVelocity, NULL );
	pPhysics->GetPosition( &vPosition, NULL );

	// Only funnel while falling
	if ( vVelocity.z < 0.0f )
	{
		Vector vPropOrigin;
		pPhysics->GetPosition( &vPropOrigin, NULL );

		const int iPortalCount = CPortal_Base2D_Shared::AllPortals.Count();
		if ( iPortalCount != 0 )
		{
			CPortal_Base2D *pFunnelInto = NULL;
			Vector vPropToFunnelPortal;
			float fClosestFunnelPortalDistSqr = FLT_MAX;

			CPortal_Base2D **pPortals = CPortal_Base2D_Shared::AllPortals.Base();
			for ( int i = 0; i != iPortalCount; ++i )
			{
				CPortal_Base2D *pTempPortal = pPortals[i];
				if ( pTempPortal->IsActivedAndLinked() && pTempPortal->IsFloorPortal() )
				{
					const Vector vPropToPortal = pTempPortal->GetAbsOrigin() - vPropOrigin;

					// Floor portals below us, within range
					if ( vPropToPortal.z >= -1024.0f && vPropToPortal.z < 0.0f )
					{
						const float fDistSqr = vPropToPortal.Length2DSqr();
						if ( fDistSqr < fClosestFunnelPortalDistSqr )
						{
							fClosestFunnelPortalDistSqr = fDistSqr;
							pFunnelInto = pTempPortal;
							vPropToFunnelPortal = vPropToPortal;
						}
					}
				}
			}

			if ( pFunnelInto )
			{
				const float flHeightFromPortal = -vPropToFunnelPortal.z;
				const float flVerticalSpeed = vVelocity.z;
				const float flGravity = sv_gravity.GetFloat();

				// Solve when we hit
				float flRoot1, flRoot2;
				SolveQuadratic(
				    -flGravity, 2.f * flVerticalSpeed, 2.f * flHeightFromPortal, flRoot1, flRoot2 );

				float flTimeToPortal = flRoot1 > 0.f ? flRoot1 : flRoot2;
				if ( flRoot2 < flRoot1 && flRoot2 >= 0.f && flRoot1 >= 0.f )
					flTimeToPortal = flRoot2;

				Vector vPortalPos = pFunnelInto->GetAbsOrigin();
				Vector vPredictedHitPos = vPosition + vVelocity * flTimeToPortal;
				float flMissDist = ( vPortalPos - vPredictedHitPos ).Length2D();

				// Reconstruction note: the 2010 build reads m_fNetworkHalfWidth directly;
				// the imported portal header exposes it through GetHalfWidth().
				if ( flMissDist > pFunnelInto->GetHalfWidth() &&
				     flMissDist < sv_futbol_funnel_max_correct.GetFloat() && flTimeToPortal > 0.f )
				{
					// Correct the horizontal velocity so we land in the portal
					Vector vVelocityToHitPortal = vPropToFunnelPortal / flTimeToPortal;
					vVelocityToHitPortal.z = vVelocity.z;
					pPhysics->SetVelocity( &vVelocityToHitPortal, NULL );
				}
			}
		}
	}
}

ExplodingFutbolHolderType_t CPropExplodingFutbol::GetHolder( void )
{
	return m_Holder;
}

void CPropExplodingFutbol::SetHolder( ExplodingFutbolHolderType_t holder )
{
	m_Holder = holder;
}

CPortal_Player *CPropExplodingFutbol::GetLastPlayerToHold( void )
{
	return m_hLastHeldByPlayer.Get();
}

void CPropExplodingFutbol::SetSpawner( CPropExplodingFutbolSpawner *pSpawner )
{
	m_hSpawner = pSpawner;

	// Futbols handed out by a spawner always respawn and explode on contact
	m_bShouldRespawn = true;
	m_bExplodeOnTouch = true;
}

CPropExplodingFutbolSpawner *CPropExplodingFutbol::GetSpawner( void )
{
	return m_hSpawner;
}

void CPropExplodingFutbol::InputExplode( inputdata_t &in )
{
	KillFutbol();
}

void CPropExplodingFutbol::KillFutbol( void )
{
	CTakeDamageInfo info( this, this, GetHealth(), DMG_BLAST );
	info.SetDamagePosition( GetAbsOrigin() );
	Vector forward;
	GetVectors( &forward, NULL, NULL );
	info.SetDamageForce( forward * 500.0f );
	Event_Killed( info );
}

void CPropExplodingFutbol::DestroyFutbol( bool bExplode )
{
	if ( bExplode )
	{
		ExplodeFutbol();
	}

	if ( m_bShouldRespawn && m_hSpawner.Get() )
	{
		m_hSpawner->FutbolDestroyed();
	}
}

void CPropExplodingFutbol::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	DestroyFutbol( true );
}

void CPropExplodingFutbol::ExplodeFutbol( void )
{
	float flExplosionRadius = exploding_futbol_explosion_radius.GetFloat();

	m_ExplodedEntities.RemoveAll();

	ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(),
	    exploding_futbol_explosion_magnitude.GetInt(), flExplosionRadius,
	    SF_ENVEXPLOSION_NOSMOKE | SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS,
	    exploding_futbol_explosion_damage.GetFloat(), this );

	// Explode through any portals within the radius
	PortalRadiusExtensionVector portalRadiusExtensions;
	ExtendRadiusThroughPortals(
	    GetAbsOrigin(), GetAbsAngles(), flExplosionRadius, portalRadiusExtensions );
	for ( int i = 0; i < portalRadiusExtensions.Count(); ++i )
	{
		DoExplosion( portalRadiusExtensions[i].vecOrigin, flExplosionRadius,
		    portalRadiusExtensions[i].pPortalTo );
	}
}

void CPropExplodingFutbol::DamageEntity(
    CBaseEntity *pEntity, const Vector &vecCenter, const Vector &vecForward )
{
	Vector vecDamagePos = vecCenter - vecForward * 5.0f;

	if ( pEntity->IsPlayer() )
	{
		// Players take less damage the further they are from the explosion
		float flDist = vecCenter.DistTo( pEntity->GetAbsOrigin() );
		float flBaseDamage = exploding_futbol_explosion_damage.GetFloat();
		float flPercent = flDist / exploding_futbol_explosion_radius.GetFloat();
		float flDamage = Lerp( flPercent, flBaseDamage,
		    flBaseDamage * exploding_futbol_explosion_damage_falloff.GetFloat() );

		if ( exploding_futbol_explosion_debug.GetBool() )
		{
			CFmtStr msg;
			msg.sprintf( "Distance: %f, Percent: %f, Damage: %f\n", flDist, flPercent, flDamage );
			NDebugOverlay::Text( pEntity->GetAbsOrigin(), msg, true, 10.0f );
		}

		CTakeDamageInfo info( this, this, flDamage, DMG_BLAST );
		info.SetDamagePosition( vecDamagePos );
		pEntity->TakeDamage( info );
	}
	else if ( exploding_futbol_hit_breakables.GetBool() &&
	          FClassnameIs( pEntity, "func_breakable" ) )
	{
		// Always break breakables
		CTakeDamageInfo info( this, this, pEntity->GetHealth(), DMG_BLAST );
		info.SetDamagePosition( vecCenter );
		info.SetDamageForce( vecForward * exploding_futbol_explosion_damage.GetFloat() );
		pEntity->TakeDamage( info );
	}
	else
	{
		CTakeDamageInfo info( this, this, exploding_futbol_explosion_damage.GetFloat(), DMG_BLAST );
		info.SetDamagePosition( vecDamagePos );
		CalculateExplosiveDamageForce(
		    &info, pEntity->GetAbsOrigin() - vecDamagePos, vecDamagePos );
		pEntity->VPhysicsTakeDamage( info );
	}
}

void CPropExplodingFutbol::DoExplosion(
    const Vector &vecCenter, float flExplosionRadius, CPortal_Base2D *pPortal )
{
	bool bDebugging = exploding_futbol_explosion_debug.GetBool();
	Color debugColor( 0, 255, 255 );
	if ( pPortal )
	{
		debugColor = Color( 255, 0, 255 );
	}

	CBaseEntity *pEnts[128];
	int nNumFound =
	    UTIL_EntitiesInSphere( pEnts, ARRAYSIZE( pEnts ), vecCenter, flExplosionRadius, 0 );
	for ( int i = 0; i < nNumFound; ++i )
	{
		if ( pEnts[i] == NULL )
			continue;

		// Only damage the entity types we care about
		bool bTestEntity = false;
		for ( int x = 0; g_psxExplodableEntities[x] != NULL; ++x )
		{
			if ( FClassnameIs( pEnts[i], g_psxExplodableEntities[x] ) )
			{
				bTestEntity = true;
				break;
			}
		}

		if ( !bTestEntity )
			continue;

		// Don't damage anything twice for the same explosion
		int iExplodedEntitiesCount = m_ExplodedEntities.Count();
		for ( int j = 0; j < iExplodedEntitiesCount; ++j )
		{
			if ( m_ExplodedEntities[j] == pEnts[i] )
			{
				bTestEntity = false;
				break;
			}
		}

		if ( !bTestEntity )
			continue;

		bool bSuccess = false;

		Vector vecRayStart = vecCenter;
		Vector vecRayEnd = pEnts[i]->GetAbsOrigin();

		CTraceFilterSimpleList traceFilter( COLLISION_GROUP_NONE );
		traceFilter.AddEntityToIgnore( this );
		for ( int j = 0; j < iExplodedEntitiesCount; ++j )
		{
			traceFilter.AddEntityToIgnore( m_ExplodedEntities[j] );
		}

		if ( pPortal == NULL )
		{
			Ray_t entRay;
			entRay.Init( vecRayStart, vecRayEnd );

			trace_t trace;
			enginetrace->TraceRay( entRay, MASK_SHOT, &traceFilter, &trace );

			if ( trace.m_pEnt != GetWorldEntity() )
			{
				Vector forward;
				GetVectors( &forward, NULL, NULL );
				DamageEntity( pEnts[i], vecCenter, forward );
				bSuccess = true;
			}
		}
		else
		{
			// Trace from the futbol through the portal to the entity
			CPortal_Base2D *pInPortal = pPortal->GetLinkedPortal();
			Vector vecEndPos;
			UTIL_Portal_PointTransform( pPortal->m_matrixThisToLinked, vecRayEnd, vecEndPos );

			Ray_t portalRay;
			portalRay.Init( GetAbsOrigin(), vecEndPos );

			trace_t trace;
			bool bPortalTrace = UTIL_Portal_TraceRay_Bullets(
			    pInPortal, portalRay, MASK_SHOT, &traceFilter, &trace, true );
			if ( bPortalTrace )
			{
				UTIL_Portal_PointTransform(
				    pInPortal->m_matrixThisToLinked, GetAbsOrigin(), vecRayStart );

				Vector vecForward = pEnts[i]->GetAbsOrigin() - vecRayStart;
				VectorNormalize( vecForward );

				DamageEntity( pEnts[i], vecRayStart, vecForward );
				bSuccess = true;
			}

			if ( bDebugging )
			{
				NDebugOverlay::Sphere( vecEndPos, 5.0f, 128, 255, 128, false, 10.0f );
				NDebugOverlay::Line( GetAbsOrigin(), vecEndPos, 0, 0, 255, false, 10.0f );
			}
		}

		if ( bDebugging )
		{
			Color lineColor( 255, 0, 0 );
			if ( bSuccess )
			{
				lineColor = Color( 0, 255, 0 );
			}

			NDebugOverlay::Sphere( vecRayStart, 2.0f, 255, 255, 0, false, 10.0f );
			NDebugOverlay::Line(
			    vecRayStart, vecRayEnd, lineColor.r(), lineColor.g(), lineColor.b(), false, 10.0f );
			NDebugOverlay::Sphere( vecRayEnd, 2.0f, 0, 0, 255, false, 10.0f );
		}

		m_ExplodedEntities.AddToTail( pEnts[i] );
	}

	if ( bDebugging )
	{
		NDebugOverlay::Sphere( vecCenter, flExplosionRadius, debugColor.r(), debugColor.g(),
		    debugColor.b(), false, 10.0f );
		NDebugOverlay::Sphere(
		    vecCenter, 2.0f, debugColor.r(), debugColor.g(), debugColor.b(), false, 10.0f );
	}
}

void CPropExplodingFutbol::ActivateFutbolTimer( float flTimer )
{
	m_flTotalTimer = flTimer;
	m_flExplosionTimer = flTimer;

	if ( flTimer > 0.0f )
	{
		m_bTimerActive = true;
		m_flLastTickTime = gpGlobals->curtime;
		m_flLastTimerSoundTime = gpGlobals->curtime - 1.0f;
		m_flLastFlashTime = gpGlobals->curtime;

		Color futbolColor = FutbolColor( exploding_futbol_start_color );
		SetRenderColor( futbolColor.r(), futbolColor.g(), futbolColor.b() );

		SetContextThink( &CPropExplodingFutbol::TimerThink,
		    gpGlobals->curtime + gpGlobals->interval_per_tick,
		    g_szExplodingFutbolTimerThinkContext );
	}
}

void CPropExplodingFutbol::StopFutbolTimer( void )
{
	m_bTimerActive = false;

	Color futbolColor = FutbolColor( exploding_futbol_start_color );
	SetRenderColor( futbolColor.r(), futbolColor.g(), futbolColor.b() );
}

void CPropExplodingFutbol::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	CPortal_Player *pHoldingPlayer = ToPortalPlayer( pPhysGunUser );

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->EnableMotion( true );
	}

	if ( reason == PICKED_UP_BY_PLAYER && pHoldingPlayer )
	{
		// Don't let the player immediately drop it again
		pHoldingPlayer->SetUseKeyCooldownTime( exploding_futbol_use_cooldown_time.GetFloat() );
	}

	// Picking the ball back up after dropping it doesn't count as a new grab
	bool bSamePlayer =
	    ( pHoldingPlayer == GetLastPlayerToHold() && m_Holder == EXPLODING_FUTBOL_HELD_BY_NONE );

	if ( m_hSpawner.Get() )
	{
		m_hSpawner->FutbolGrabbed( this, bSamePlayer );
	}

	m_hLastHeldByPlayer = pHoldingPlayer;

	// Steal it from whoever is holding it now
	if ( m_Holder == EXPLODING_FUTBOL_HELD_BY_PLAYER )
	{
		CBasePlayer *pOtherPlayer = GetPlayerHoldingEntity( this );
		if ( pOtherPlayer )
		{
			pOtherPlayer->ForceDropOfCarriedPhysObjects( NULL );
		}
	}

	m_Holder = EXPLODING_FUTBOL_HELD_BY_PLAYER;
}

void CPropExplodingFutbol::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	if ( pPhysGunUser == NULL )
		return;

	BaseClass::OnPhysGunDrop( pPhysGunUser, reason );

	if ( m_Holder == EXPLODING_FUTBOL_HELD_BY_PLAYER )
	{
		m_Holder = EXPLODING_FUTBOL_HELD_BY_NONE;
	}
}

int CPropExplodingFutbol::OnTakeDamage( const CTakeDamageInfo &info )
{
	// Can't be damaged while sitting in a spawner or catcher
	if ( m_bExplodeOnTouch && m_Holder != EXPLODING_FUTBOL_HELD_BY_SPAWNER &&
	     m_Holder != EXPLODING_FUTBOL_HELD_BY_CATCHER )
	{
		return BaseClass::OnTakeDamage( info );
	}

	return 0;
}

void CPropExplodingFutbol::OnFizzled( void )
{
	DestroyFutbol( exploding_futbol_explode_on_fizzle.GetBool() );
}

void CPropExplodingFutbol::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	// Blow up on the next tick if we hit anything while loose
	if ( m_bExplodeOnTouch && m_Holder == EXPLODING_FUTBOL_HELD_BY_NONE )
	{
		SetContextThink( &CPropExplodingFutbol::KillThink,
		    gpGlobals->curtime + gpGlobals->interval_per_tick,
		    g_szExplodingFutbolKillThinkContext );
	}

	BaseClass::VPhysicsCollision( index, pEvent );
}

void CPropExplodingFutbol::KillThink( void )
{
	KillFutbol();
}

void CPropExplodingFutbol::AnimThink( void )
{
	StudioFrameAdvance();
	SetNextThink(
	    gpGlobals->curtime + gpGlobals->interval_per_tick, g_szExplodingFutbolAnimThinkContext );
}

Color ColorLerp( float flPercent, const Color &startColor, const Color &endColor )
{
	float r = startColor.r() + (int)( endColor.r() - startColor.r() ) * flPercent;
	float g = startColor.g() + (int)( endColor.g() - startColor.g() ) * flPercent;
	float b = startColor.b() + (int)( endColor.b() - startColor.b() ) * flPercent;
	float a = startColor.a() + (int)( endColor.a() - startColor.a() ) * flPercent;

	return Color( (int)r, (int)g, (int)b, (int)a );
}

void CPropExplodingFutbol::TimerThink( void )
{
	if ( !m_bTimerActive )
		return;

	float flDeltaTime = gpGlobals->curtime - m_flLastTickTime;
	m_flLastTickTime = gpGlobals->curtime;
	m_flExplosionTimer -= flDeltaTime;

	float flSoundDelay = 1.0f;
	string_t strSoundName = MAKE_STRING( "Portal.room1_TickTock" );

	Color startColor, endColor, currentColor;
	float flPercent;

	if ( m_flExplosionTimer <= exploding_futbol_flash_start_time.GetFloat() )
	{
		// Final countdown: flash and klaxon
		flSoundDelay = exploding_futbol_flash_duration.GetFloat();
		strSoundName = MAKE_STRING( "NPC_FloorTurret.DeployingKlaxon" );

		float flCurrentFlashDuration = gpGlobals->curtime - m_flLastFlashTime;
		if ( flCurrentFlashDuration > exploding_futbol_flash_duration.GetFloat() )
		{
			m_flLastFlashTime = gpGlobals->curtime;
			flCurrentFlashDuration = 0.0f;
		}

		startColor = FutbolColor( exploding_futbol_flash_start_color );
		endColor = FutbolColor( exploding_futbol_flash_end_color );
		flPercent = flCurrentFlashDuration / exploding_futbol_flash_duration.GetFloat();
	}
	else
	{
		startColor = FutbolColor( exploding_futbol_start_color );
		endColor = FutbolColor( exploding_futbol_end_color );
		flPercent = ( m_flTotalTimer - m_flExplosionTimer ) / m_flTotalTimer;
	}

	if ( gpGlobals->curtime - m_flLastTimerSoundTime > flSoundDelay )
	{
		EmitSound( STRING( strSoundName ) );
		m_flLastTimerSoundTime = gpGlobals->curtime;
	}

	currentColor = ColorLerp( flPercent, startColor, endColor );
	SetRenderColor( currentColor.r(), currentColor.g(), currentColor.b() );

	if ( m_flExplosionTimer <= 0.0f )
	{
		EmitSound( "EnergyBall.Explosion" );
		KillFutbol();
		StopFutbolTimer();
	}

	if ( m_bTimerActive )
	{
		SetNextThink( gpGlobals->curtime + gpGlobals->interval_per_tick,
		    g_szExplodingFutbolTimerThinkContext );
	}
}

//-----------------------------------------------------------------------------
// Exploding futbol catcher
//-----------------------------------------------------------------------------
CExplodingFutbolCatcher::CExplodingFutbolCatcher()
    : m_bDisableRecaptureOnPlayerGrab( false ), m_vecCatcherBoxMins( vec3_origin ),
      m_vecCatcherBoxMaxs( vec3_origin ), m_vecCatcherBoxOrigin( vec3_origin ),
      m_vecCatcherBoxHalfDiagonal( 25.0f, 25.0f, 25.0f )
{
}

void CExplodingFutbolCatcher::Spawn( void )
{
	BaseClass::Spawn();

	GetAttachment( "ball", m_vecCatcherBoxOrigin );
	m_vecCatcherBoxMins = -m_vecCatcherBoxHalfDiagonal;
	m_vecCatcherBoxMaxs = m_vecCatcherBoxHalfDiagonal;

	SetThink( &CExplodingFutbolCatcher::CatchThink );
	SetNextThink( gpGlobals->curtime + gpGlobals->interval_per_tick );
}

void CExplodingFutbolCatcher::DrawDebugGeometryOverlays( void )
{
	BaseClass::DrawDebugGeometryOverlays();

	NDebugOverlay::Box(
	    m_vecCatcherBoxOrigin, m_vecCatcherBoxMins, m_vecCatcherBoxMaxs, 255, 0, 0, 100, 0.0f );
}

void CExplodingFutbolCatcher::CatchThink( void )
{
	Ray_t ray;
	ray.Init(
	    m_vecCatcherBoxOrigin, m_vecCatcherBoxOrigin, m_vecCatcherBoxMins, m_vecCatcherBoxMaxs );
	CFilterOnlyExplodingFutbol filter;
	trace_t trace;
	enginetrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

	if ( trace.m_pEnt && trace.m_pEnt != GetWorldEntity() && !m_bDisableRecaptureOnPlayerGrab )
	{
		CPropExplodingFutbol *pFutbol = dynamic_cast<CPropExplodingFutbol *>( trace.m_pEnt );
		if ( pFutbol && ( pFutbol->GetHolder() == EXPLODING_FUTBOL_HELD_BY_NONE ||
		                    pFutbol->GetHolder() == EXPLODING_FUTBOL_HELD_BY_PLAYER ) )
		{
			CaptureFutbol( pFutbol );
		}
	}

	// Allow recapture once the released futbol has left the catch box
	if ( trace.m_pEnt == NULL )
	{
		m_bDisableRecaptureOnPlayerGrab = false;
	}

	SetNextThink( gpGlobals->curtime + gpGlobals->interval_per_tick );
}

void CExplodingFutbolCatcher::CaptureFutbol( CPropExplodingFutbol *pFutbol )
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

	pFutbol->Teleport( &m_vecCatcherBoxOrigin, &vec3_angle, &vec3_origin );
	pFutbol->SetHolder( EXPLODING_FUTBOL_HELD_BY_CATCHER );

	CPropExplodingFutbolSpawner *pSpawner = pFutbol->GetSpawner();
	if ( pSpawner )
	{
		CPropIndicatorPanel *pTimerPanel = pSpawner->GetTimerPanel();
		if ( pTimerPanel )
		{
			pTimerPanel->ResetTimer();
		}
	}

	pFutbol->StopFutbolTimer();

	m_OnFutbolCaught.FireOutput( pFutbol, pFutbol->GetLastPlayerToHold() );

	SetThink( &CExplodingFutbolCatcher::CaptureThink );
}

void CExplodingFutbolCatcher::CaptureThink( void )
{
	// Go back to catching once the futbol has been taken away
	CPropExplodingFutbol *pFutbol = static_cast<CPropExplodingFutbol *>( m_hCaughtFutbol.Get() );
	if ( pFutbol == NULL || pFutbol->GetHolder() != EXPLODING_FUTBOL_HELD_BY_CATCHER )
	{
		SetThink( &CExplodingFutbolCatcher::CatchThink );
		m_OnFutbolReleased.FireOutput( this, this );
		m_hCaughtFutbol = NULL;
		m_bDisableRecaptureOnPlayerGrab = true;
	}

	SetNextThink( gpGlobals->curtime + gpGlobals->interval_per_tick );
}

//-----------------------------------------------------------------------------
// Exploding futbol spawner
//-----------------------------------------------------------------------------
CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner()
    : m_bHasFutbol( false ), m_bIsTimed( false ), m_flTimer( 0.0f ),
      m_strTimerIndicator( NULL_STRING )
{
	m_vecCatcherBoxHalfDiagonal = Vector( 20.0f, 20.0f, 20.0f );
}

void CPropExplodingFutbolSpawner::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel( EXPLODING_FUTBOL_SPAWNER_MODEL );
	PrecacheScriptSound( "Portal.button_down" );
}

void CPropExplodingFutbolSpawner::Spawn( void )
{
	Precache();
	SetModel( EXPLODING_FUTBOL_SPAWNER_MODEL );
	SetSolid( SOLID_VPHYSICS );

	BaseClass::Spawn();

	if ( m_bHasFutbol )
	{
		SpawnFutbol();
	}
}

void CPropExplodingFutbolSpawner::FutbolDestroyed( void )
{
	CPropIndicatorPanel *pTimerPanel = GetTimerPanel();
	if ( pTimerPanel )
	{
		pTimerPanel->ResetTimer();
	}

	SpawnFutbol();
}

void CPropExplodingFutbolSpawner::FutbolGrabbed( CPropExplodingFutbol *pFutbol, bool bSamePlayer )
{
	EmitSound( "Portal.button_down" );

	if ( !bSamePlayer )
	{
		CPropIndicatorPanel *pTimerPanel = GetTimerPanel();
		if ( pTimerPanel )
		{
			pTimerPanel->SetTimerDuration( m_flTimer );
			pTimerPanel->StartTimer();
		}

		if ( m_bIsTimed )
		{
			pFutbol->ActivateFutbolTimer( m_flTimer );
		}

		if ( m_bHasFutbol )
		{
			m_OnFutbolGrabbed.FireOutput( this, this );
		}
	}

	m_bHasFutbol = false;
}

CPropIndicatorPanel *CPropExplodingFutbolSpawner::GetTimerPanel( void )
{
	if ( m_bIsTimed && m_strTimerIndicator != NULL_STRING )
	{
		CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, m_strTimerIndicator );
		if ( pEnt )
		{
			CPropIndicatorPanel *pTimerPanel = dynamic_cast<CPropIndicatorPanel *>( pEnt );
			if ( pTimerPanel )
				return pTimerPanel;

			// Reconstruction note: the 2010 build formats this warning through the
			// failed (NULL) cast result; the entity that was found is used instead.
			Warning( "prop_exploding_futbol_spawner has an invalid timer set (%s: %s). It will not "
			         "respawn.\n",
			    pEnt->GetClassname(), pEnt->GetDebugName() );
		}
	}

	return NULL;
}

void CPropExplodingFutbolSpawner::SpawnFutbol( void )
{
	CPropExplodingFutbol *pFutbol =
	    static_cast<CPropExplodingFutbol *>( CreateEntityByName( "prop_exploding_futbol" ) );
	if ( pFutbol )
	{
		m_OnFutbolSpawned.FireOutput( this, this );

		Vector vecFutbolSpawnPoint;
		GetAttachment( "ball", vecFutbolSpawnPoint );
		pFutbol->SetAbsOrigin( vecFutbolSpawnPoint );
		pFutbol->SetSpawner( this );
		pFutbol->SetHolder( EXPLODING_FUTBOL_HELD_BY_SPAWNER );

		m_bHasFutbol = true;

		DispatchSpawn( pFutbol );
		CaptureFutbol( pFutbol );
	}
}

void CPropExplodingFutbolSpawner::InputForceSpawn( inputdata_t &data )
{
	SpawnFutbol();
}

//-----------------------------------------------------------------------------
// Exploding futbol socket
//-----------------------------------------------------------------------------
void CPropExplodingFutbolSocket::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel( EXPLODING_FUTBOL_SOCKET_MODEL );
	PrecacheScriptSound( "Portal.elevator_ding" );
}

void CPropExplodingFutbolSocket::Spawn( void )
{
	Precache();
	SetModel( EXPLODING_FUTBOL_SOCKET_MODEL );
	SetSolid( SOLID_VPHYSICS );

	int nSequence = LookupSequence( "hold" );
	SetSequence( nSequence );

	BaseClass::Spawn();
}

void CPropExplodingFutbolSocket::CaptureFutbol( CPropExplodingFutbol *pFutbol )
{
	EmitSound( "Portal.elevator_ding" );

	BaseClass::CaptureFutbol( pFutbol );
}
