//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A volume which bumps portal placement. Keeps a global list loaded in from the map
//			and provides an interface with which prop_portal can get this list and avoid successfully
//			creating portals partially inside the volume.
//
// $NoKeywords: $
//======================================================================================//

#include "cbase.h"
#include "triggers.h"
#ifdef PORTAL2
#include "trigger_portal_cleanser.h"
#endif
#include "portal_player.h"
#ifdef PORTAL2
// The Portal 2 gun; a same-directory lookup would find the Portal 1 header.
#include "portal2/portal/weapon_portalgun.h"
#else
#include "weapon_portalgun.h"
#endif
#include "prop_portal_shared.h"
#include "portal_shareddefs.h"
#include "physobj.h"
#include "portal/weapon_physcannon.h"
#ifdef PORTAL2
#include "portal_grabcontroller_shared.h"
#endif
#include "model_types.h"
#include "rumble_shared.h"
#ifdef PORTAL2
#include "fizzler_multiorigin_sound_player.h"
#include "ispatialpartition.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef PORTAL2
// Portal 2 names the delayed portal placement think context on the class.
#define s_pDelayedPlacementContext CProp_Portal::s_szDelayedPlacementThinkContext
#endif

static char *g_pszPortalNonCleansable[] = 
{ 
	"func_door", 
	"func_door_rotating", 
	"prop_door_rotating",
	"func_tracktrain",
	"env_ghostanimating",
	"physicsshadowclone",
	"prop_energy_ball",
	NULL,
};

#ifdef PORTAL2
// The class is declared in trigger_portal_cleanser.h: Portal 2 networks it
// (DT_TriggerPortalCleanser) and other entities call its fizzle helpers.
ConVar sv_portal_cleanser_think_rate( "sv_portal_cleanser_think_rate", "0.25f", FCVAR_CHEAT,
    "How often, in seconds should the portal cleanser think." );
ConVar sv_portal_cleanser_vortex_distance( "sv_portal_cleanser_vortex_distance", "96", FCVAR_CHEAT,
    "The distance from the fizzler at which an object is within range to create a vortex." );
ConVar debug_portal_cleanser_search_box( "debug_portal_cleanser_search_box", "0", FCVAR_CHEAT );

static const char s_szPlayerPassesTriggerFiltersThinkContext[] =
    "CTriggerPortalCleanser::PlayerPassesTriggerFiltersThink";

// The one looping fizzler sound, spatialized by the client at the nearest
// enabled cleanser (FizzlerMultiOriginSoundPlayer).
static EHANDLE s_hFizzlerSoundPlayer;

// clang-format off
BEGIN_DATADESC( CTriggerPortalCleanser )

	DEFINE_KEYFIELD( m_bVisible, FIELD_BOOLEAN, "Visible" ),
	DEFINE_KEYFIELD( m_bUseScanline, FIELD_BOOLEAN, "UseScanline" ),
	DEFINE_FIELD( m_flPortalShotTime, FIELD_TIME ),
	DEFINE_FIELD( m_bObject1InRange, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bObject2InRange, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hObject1, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hObject2, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bPlayersPassTriggerFilters, FIELD_BOOLEAN ),

	DEFINE_THINKFUNC( SearchThink ),
	DEFINE_THINKFUNC( PlayerPassesTriggerFiltersThink ),

	DEFINE_INPUTFUNC( FIELD_VOID, "FizzleTouchingPortals", InputFizzleTouchingPortals ),

	// Outputs
	DEFINE_OUTPUT( m_OnDissolve, "OnDissolve" ),
	DEFINE_OUTPUT( m_OnFizzle, "OnFizzle" ),
	DEFINE_OUTPUT( m_OnDissolveBox, "OnDissolveBox" ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CTriggerPortalCleanser, DT_TriggerPortalCleanser )
	SendPropBool( SENDINFO( m_bDisabled ) ),
	SendPropBool( SENDINFO( m_bVisible ) ),
	SendPropFloat( SENDINFO( m_flPortalShotTime ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bObject1InRange ) ),
	SendPropBool( SENDINFO( m_bObject2InRange ) ),
	SendPropEHandle( SENDINFO( m_hObject1 ) ),
	SendPropEHandle( SENDINFO( m_hObject2 ) ),
	SendPropBool( SENDINFO( m_bUseScanline ) ),
	SendPropBool( SENDINFO( m_bPlayersPassTriggerFilters ) ),
END_SEND_TABLE()

#else
//-----------------------------------------------------------------------------
// Purpose: Removes anything that touches it. If the trigger has a targetname,
//			firing it will toggle state.
//-----------------------------------------------------------------------------
class CTriggerPortalCleanser : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerPortalCleanser, CBaseTrigger );

	void Spawn( void );
	void Touch( CBaseEntity *pOther );

	static void FizzleBaseAnimating(
	    CTriggerPortalCleanser *pFizzler, CBaseAnimating *pBaseAnimating );

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnDissolve;
	COutputEvent m_OnFizzle;
	COutputEvent m_OnDissolveBox;
};

BEGIN_DATADESC( CTriggerPortalCleanser )

// Outputs
DEFINE_OUTPUT( m_OnDissolve, "OnDissolve" ),
DEFINE_OUTPUT( m_OnFizzle, "OnFizzle" ),
DEFINE_OUTPUT( m_OnDissolveBox, "OnDissolveBox" ),

END_DATADESC()


#endif // PORTAL2

LINK_ENTITY_TO_CLASS( trigger_portal_cleanser, CTriggerPortalCleanser );
// clang-format on

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::Spawn( void )
{	
	BaseClass::Spawn();
#ifdef PORTAL2
	Precache();
	// The client predicts portal fizzling (c_trigger_portal_cleanser.cpp).
	m_bClientSidePredicted = true;
#endif
	InitTrigger();
#ifdef PORTAL2
	if ( !s_hFizzlerSoundPlayer )
	{
		CReliableBroadcastRecipientFilter filter;
		s_hFizzlerSoundPlayer = FizzlerMultiOriginSoundPlayer::Create( filter, "VFX.FizzlerLp" );
	}

	if ( m_bVisible )
	{
		// The field is the trigger's own brush model (effects/fizzler).
		RemoveEffects( EF_NODRAW );
		SetThink( &CTriggerPortalCleanser::SearchThink );
		SetNextThink( gpGlobals->curtime + sv_portal_cleanser_think_rate.GetFloat() );
		if ( !m_bDisabled )
			PlayActivateSound();
	}

	SetContextThink( &CTriggerPortalCleanser::PlayerPassesTriggerFiltersThink,
	    gpGlobals->curtime + 1.0f, s_szPlayerPassesTriggerFiltersThinkContext );
#endif
}

// Creates a base entity with model/physics matching the parameter ent.
// Used to avoid higher level functions on a disolving entity, which should be inert
// and not react the way it used to (touches, etc).
// Uses simple physics entities declared in physobj.cpp
CBaseEntity* ConvertToSimpleProp ( CBaseEntity* pEnt )
{
	CBaseEntity *pRetVal = NULL;
	int modelindex = pEnt->GetModelIndex();
	const model_t *model = modelinfo->GetModel( modelindex );
	if ( model && modelinfo->GetModelType(model) == mod_brush )
	{
		pRetVal = CreateEntityByName( "simple_physics_brush" );
	}
	else
	{
		pRetVal = CreateEntityByName( "simple_physics_prop" );
	}

	pRetVal->KeyValue( "model", STRING(pEnt->GetModelName()) );
	pRetVal->SetAbsOrigin( pEnt->GetAbsOrigin() );
	pRetVal->SetAbsAngles( pEnt->GetAbsAngles() );
	pRetVal->Spawn();
	pRetVal->VPhysicsInitNormal( SOLID_VPHYSICS, 0, false );
	
	return pRetVal;
}


void CTriggerPortalCleanser::Touch( CBaseEntity *pOther )
{
	if ( !PassesTriggerFilters( pOther ) )
		return;

	if ( pOther->IsPlayer() )
	{
		CPortal_Player *pPlayer = ToPortalPlayer( pOther );

		if ( pPlayer )
		{
			CWeaponPortalgun *pPortalgun = dynamic_cast<CWeaponPortalgun*>( pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) );

			if ( pPortalgun )
			{
				bool bFizzledPortal = false;

				if ( pPortalgun->CanFirePortal1() )
				{
					CProp_Portal *pPortal = CProp_Portal::FindPortal( pPortalgun->GetLinkageGroupID(), false );

					if ( pPortal && pPortal->m_bActivated )
					{
						pPortal->DoFizzleEffect( PORTAL_FIZZLE_KILLED, false );
						pPortal->Fizzle();
						// HACK HACK! Used to make the gun visually change when going through a cleanser!
						pPortalgun->m_fEffectsMaxSize1 = 50.0f;

						bFizzledPortal = true;
					}

					// Cancel portals that are still mid flight
					if ( pPortal && pPortal->GetNextThink( s_pDelayedPlacementContext ) > gpGlobals->curtime )
					{
						pPortal->SetContextThink( NULL, gpGlobals->curtime, s_pDelayedPlacementContext ); 
						pPortalgun->m_fEffectsMaxSize2 = 50.0f;
						bFizzledPortal = true;
					}
				}

				if ( pPortalgun->CanFirePortal2() )
				{
					CProp_Portal *pPortal = CProp_Portal::FindPortal( pPortalgun->GetLinkageGroupID(), true );

					if ( pPortal && pPortal->m_bActivated )
					{
						pPortal->DoFizzleEffect( PORTAL_FIZZLE_KILLED, false );
						pPortal->Fizzle();
						// HACK HACK! Used to make the gun visually change when going through a cleanser!
						pPortalgun->m_fEffectsMaxSize2 = 50.0f;

						bFizzledPortal = true;
					}
					
					// Cancel portals that are still mid flight
					if ( pPortal && pPortal->GetNextThink( s_pDelayedPlacementContext ) > gpGlobals->curtime )
					{
						pPortal->SetContextThink( NULL, gpGlobals->curtime, s_pDelayedPlacementContext ); 
						pPortalgun->m_fEffectsMaxSize2 = 50.0f;
						bFizzledPortal = true;
					}
				}

				if ( bFizzledPortal )
				{
					pPortalgun->SendWeaponAnim( ACT_VM_FIZZLE );
					pPortalgun->SetLastFiredPortal( 0 );
					m_OnFizzle.FireOutput( pOther, this );
					pPlayer->RumbleEffect( RUMBLE_RPG_MISSILE, 0, RUMBLE_FLAG_RESTART );
				}
			}
		}

		return;
	}

	CBaseAnimating *pBaseAnimating = dynamic_cast<CBaseAnimating*>( pOther );
	if ( pBaseAnimating )
	{
		FizzleBaseAnimating( this, pBaseAnimating );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Dissolves an object as a cleanser does. pFizzler, when not NULL, is
//			the cleanser whose outputs fire; Portal 2 also fizzles cubes this way
//			without a cleanser (for example when a cube is replaced).
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::FizzleBaseAnimating( CTriggerPortalCleanser *pFizzler, CBaseAnimating *pBaseAnimating )
{
	if ( pBaseAnimating && !pBaseAnimating->IsDissolving() )
	{
		int i = 0;

		while ( g_pszPortalNonCleansable[ i ] )
		{
			if ( FClassnameIs( pBaseAnimating, g_pszPortalNonCleansable[ i ] ) )
			{
				// Don't dissolve non cleansable objects
				return;
			}

			++i;
		}

		// The portal weight box, used for puzzles in the portal mod is differentiated by its name
		// always being 'box'. We use special logic when the cleanser dissolves a box so this is a special output for it.
		if ( pBaseAnimating->NameMatches( "box" ) )
		{
			if ( pFizzler )
				pFizzler->m_OnDissolveBox.FireOutput( pBaseAnimating, pFizzler );
		}

		if ( FClassnameIs( pBaseAnimating, "updateitem2" ) )
		{
			pBaseAnimating->EmitSound( "UpdateItem.Fizzle" );
		}

		Vector vOldVel;
		AngularImpulse vOldAng;
		pBaseAnimating->GetVelocity( &vOldVel, &vOldAng );

		IPhysicsObject* pOldPhys = pBaseAnimating->VPhysicsGetObject();

		if ( pOldPhys && ( pOldPhys->GetGameFlags() & FVPHYSICS_PLAYER_HELD ) )
		{
			CPortal_Player *pPlayer = (CPortal_Player *)GetPlayerHoldingEntity( pBaseAnimating );
			if( pPlayer )
			{
				// Modify the velocity for held objects so it gets away from the player
				pPlayer->ForceDropOfCarriedPhysObjects( pBaseAnimating );

				pPlayer->GetAbsVelocity();
				vOldVel = pPlayer->GetAbsVelocity() + Vector( pPlayer->EyeDirection2D().x * 4.0f, pPlayer->EyeDirection2D().y * 4.0f, -32.0f );
			}
		}

		// Swap object with an disolving physics model to avoid touch logic
		CBaseEntity *pDisolvingObj = ConvertToSimpleProp( pBaseAnimating );
		if ( pDisolvingObj )
		{
			// Remove old prop, transfer name and children to the new simple prop
			pDisolvingObj->SetName( pBaseAnimating->GetEntityName() );
			UTIL_TransferPoseParameters( pBaseAnimating, pDisolvingObj );
			TransferChildren( pBaseAnimating, pDisolvingObj );
			pDisolvingObj->SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );
			pBaseAnimating->AddSolidFlags( FSOLID_NOT_SOLID );
			pBaseAnimating->AddEffects( EF_NODRAW );

			IPhysicsObject* pPhys = pDisolvingObj->VPhysicsGetObject();
			if ( pPhys )
			{
				pPhys->EnableGravity( false );

				Vector vVel = vOldVel;
				AngularImpulse vAng = vOldAng;

				// Disolving hurts, damp and blur the motion a little
				vVel *= 0.5f;
				vAng.z += 20.0f;

				pPhys->SetVelocity( &vVel, &vAng );
			}

			pBaseAnimating->AddFlag( FL_DISSOLVING );
			UTIL_Remove( pBaseAnimating );
		}
		
		CBaseAnimating *pDisolvingAnimating = dynamic_cast<CBaseAnimating*>( pDisolvingObj );
		if ( pDisolvingAnimating ) 
		{
			pDisolvingAnimating->Dissolve( "", gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}

		if ( pFizzler )
			pFizzler->m_OnDissolve.FireOutput( pBaseAnimating, pFizzler );
	}
}

#ifdef PORTAL2
//-----------------------------------------------------------------------------
// Purpose: A portal shot was stopped by this cleanser.
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::SetPortalShot( void )
{
	// The client pulses the field's intensity (cl_portal_cleanser_shot_pulse_*).
	m_flPortalShotTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Fizzles every portal inside the cleanser volume.
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::FizzleTouchingPortals( void )
{
	Vector vMin, vMax;
	CollisionProp()->WorldSpaceAABB( &vMin, &vMax );

	Vector vBoxCenter = ( vMin + vMax ) * 0.5f;
	Vector vBoxExtents = ( vMax - vMin ) * 0.5f;

	int iPortalCount = CProp_Portal_Shared::AllPortals.Count();
	CProp_Portal **pPortals = CProp_Portal_Shared::AllPortals.Base();
	for ( int i = 0; i != iPortalCount; ++i )
	{
		CProp_Portal *pTempPortal = pPortals[i];
		if ( UTIL_IsBoxIntersectingPortal( vBoxCenter, vBoxExtents, pTempPortal, 0.0f ) )
		{
			pTempPortal->DoFizzleEffect( PORTAL_FIZZLE_KILLED, true );
			pTempPortal->Fizzle();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Turning a cleanser on fizzles the portals already inside it.
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::Enable( void )
{
	BaseClass::Enable();
	FizzleTouchingPortals();
	if ( m_bVisible )
		PlayActivateSound();
}

void CTriggerPortalCleanser::Disable( void )
{
	BaseClass::Disable();
	if ( m_bVisible )
		PlayDeactivateSound();
}

void CTriggerPortalCleanser::InputEnable( inputdata_t &inputdata )
{
	Enable();
}

void CTriggerPortalCleanser::InputDisable( inputdata_t &inputdata )
{
	Disable();
}

void CTriggerPortalCleanser::InputToggle( inputdata_t &inputdata )
{
	if ( m_bDisabled )
		Enable();
	else
		Disable();
}

void CTriggerPortalCleanser::Precache( void )
{
	BaseClass::Precache();
	PrecacheScriptSound( "VFX.FizzlerLp" );
	PrecacheScriptSound( "VFX.FizzlerStart" );
	PrecacheScriptSound( "VFX.FizzlerDestroy" );
	PrecacheParticleSystem( "cleanser_scanline" );
}

void CTriggerPortalCleanser::Activate( void )
{
	BaseClass::Activate();
	if ( m_bVisible && !m_bDisabled )
		PlayActivateSound();
}

void CTriggerPortalCleanser::PlayActivateSound( void )
{
	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), "VFX.FizzlerStart", &GetAbsOrigin() );
}

void CTriggerPortalCleanser::PlayDeactivateSound( void )
{
	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), "VFX.FizzlerDestroy", &GetAbsOrigin() );
}

//-----------------------------------------------------------------------------
// Purpose: Collects the vortex objects near a cleanser (retail CleanserVortexTraceEnum).
//-----------------------------------------------------------------------------
class CleanserVortexTraceEnum : public IPartitionEnumerator
{
public:
	CleanserVortexTraceEnum( CBaseEntity **pList, int nMaxCount, CTriggerPortalCleanser *pCleanser )
	    : m_pList( pList ), m_nMaxCount( nMaxCount ), m_nCount( 0 ), m_pCleanser( pCleanser )
	{
	}

	virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity )
	{
		if ( staticpropmgr->IsStaticProp( pHandleEntity ) )
			return ITERATION_CONTINUE;

		CBaseEntity *pEntity = gEntList.GetBaseEntity( pHandleEntity->GetRefEHandle() );
		if ( !pEntity || pEntity == m_pCleanser )
			return ITERATION_CONTINUE;
		if ( m_pCleanser && !m_pCleanser->PassesTriggerFilters( pEntity ) )
			return ITERATION_CONTINUE;

		static const char *const s_pszVortexClasses[] = {
		    "prop_weighted_cube",
		    "npc_portal_turret_floor",
		    "simple_physics_prop",
		    "prop_monster_box",
		    "hot_potato",
		};
		for ( int i = 0; i < ARRAYSIZE( s_pszVortexClasses ); ++i )
		{
			if ( FClassnameIs( pEntity, s_pszVortexClasses[i] ) )
			{
				if ( m_nCount >= m_nMaxCount )
					return ITERATION_STOP;
				m_pList[m_nCount++] = pEntity;
				return ITERATION_CONTINUE;
			}
		}
		return ITERATION_CONTINUE;
	}

	int GetCount( void ) const { return m_nCount; }

private:
	CBaseEntity **m_pList;
	int m_nMaxCount;
	int m_nCount;
	CTriggerPortalCleanser *m_pCleanser;
};

//-----------------------------------------------------------------------------
// Purpose: Networks the two nearest vortex objects the field bends around.
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::SearchThink( void )
{
	const float flRange = sv_portal_cleanser_vortex_distance.GetFloat();
	Vector vMins, vMaxs;
	CollisionProp()->WorldSpaceAABB( &vMins, &vMaxs );
	vMins -= Vector( flRange, flRange, flRange );
	vMaxs += Vector( flRange, flRange, flRange );

	CBaseEntity *pObjects[32];
	CleanserVortexTraceEnum enumerator( pObjects, ARRAYSIZE( pObjects ), this );
	partition->EnumerateElementsInBox(
	    PARTITION_ENGINE_NON_STATIC_EDICTS, vMins, vMaxs, false, &enumerator );

	// The nearest two by squared distance from the cleanser's bounds.
	float flDist[2] = { FLT_MAX, FLT_MAX };
	CBaseEntity *pNearest[2] = { NULL, NULL };
	for ( int i = 0; i < enumerator.GetCount(); ++i )
	{
		const float flDistance =
		    CollisionProp()->CalcDistanceFromPoint( pObjects[i]->WorldSpaceCenter() );
		const float flSqr = flDistance * flDistance;
		if ( flSqr < flDist[0] )
		{
			flDist[1] = flDist[0];
			pNearest[1] = pNearest[0];
			flDist[0] = flSqr;
			pNearest[0] = pObjects[i];
		}
		else if ( flSqr < flDist[1] )
		{
			flDist[1] = flSqr;
			pNearest[1] = pObjects[i];
		}
	}

	const float flRangeSqr = flRange * flRange;
	m_bObject1InRange = flDist[0] < flRangeSqr;
	if ( m_bObject1InRange && m_hObject1.Get() != pNearest[0] )
		m_hObject1 = pNearest[0];
	m_bObject2InRange = flDist[1] < flRangeSqr;
	if ( m_bObject2InRange && m_hObject2.Get() != pNearest[1] )
		m_hObject2 = pNearest[1];

	if ( debug_portal_cleanser_search_box.GetBool() )
	{
		const float flDuration = sv_portal_cleanser_think_rate.GetFloat();
		NDebugOverlay::Box( vec3_origin, vMins, vMaxs, 255, 0, 0, 64, flDuration );
		if ( m_bObject1InRange && m_hObject1 )
			NDebugOverlay::Line(
			    WorldSpaceCenter(), m_hObject1->WorldSpaceCenter(), 0, 255, 0, true, flDuration );
		if ( m_bObject2InRange && m_hObject2 )
			NDebugOverlay::Line(
			    WorldSpaceCenter(), m_hObject2->WorldSpaceCenter(), 0, 0, 255, true, flDuration );
	}

	SetNextThink( gpGlobals->curtime + sv_portal_cleanser_think_rate.GetFloat() );
}

//-----------------------------------------------------------------------------
// Purpose: Tells the client's prediction whether players pass this trigger's filters.
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::PlayerPassesTriggerFiltersThink( void )
{
	// The first living player decides; until one exists, check again later.
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer && pPlayer->IsAlive() )
		{
			m_bPlayersPassTriggerFilters = PassesTriggerFilters( pPlayer );
			SetContextThink( NULL, TICK_NEVER_THINK, s_szPlayerPassesTriggerFiltersThinkContext );
			return;
		}
	}
	SetContextThink( &CTriggerPortalCleanser::PlayerPassesTriggerFiltersThink,
	    gpGlobals->curtime + 1.0f, s_szPlayerPassesTriggerFiltersThinkContext );
}
#endif // PORTAL2
