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
BEGIN_DATADESC( CTriggerPortalCleanser )

	DEFINE_INPUTFUNC( FIELD_VOID, "FizzleTouchingPortals", InputFizzleTouchingPortals ),

	// Outputs
	DEFINE_OUTPUT( m_OnDissolve, "OnDissolve" ),
	DEFINE_OUTPUT( m_OnFizzle, "OnFizzle" ),
	DEFINE_OUTPUT( m_OnDissolveBox, "OnDissolveBox" ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CTriggerPortalCleanser, DT_TriggerPortalCleanser )
	SendPropBool( SENDINFO( m_bDisabled ) ),
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


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::Spawn( void )
{	
	BaseClass::Spawn();
#ifdef PORTAL2
	// The client predicts portal fizzling (c_trigger_portal_cleanser.cpp).
	m_bClientSidePredicted = true;
#endif
	InitTrigger();
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

//-----------------------------------------------------------------------------
// Purpose: A portal shot was stopped by this cleanser.
//-----------------------------------------------------------------------------
void CTriggerPortalCleanser::SetPortalShot( void )
{
	// Portal 2 port: the retail client flashes the fizzler field where a portal
	// shot hits it. That effect is not reconstructed, so the server only keeps
	// the time of the last shot and reports the missing effect once.
	m_flLastPortalShotTime = gpGlobals->curtime;

	static bool s_bWarned = false;
	if ( !s_bWarned )
	{
		s_bWarned = true;
		DevWarning( "Portal 2: the fizzler portal shot effect is not supported by this engine\n" );
	}
}

#ifdef PORTAL2
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
}
#endif // PORTAL2
