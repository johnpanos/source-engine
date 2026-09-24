//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal 2 portal gun (server).
//
// Portal 2 reconstruction: this file was absent from the imported Portal 2
// source (the Portal 1 gun occupies game/server/portal/weapon_portalgun.cpp).
// Behavior follows the Steam2 depot 852_3 server.dylib (DWARF and decompiled
// functions); firing, tracing and placement live in the retained Portal 2
// game/shared/portal2/weapon_portalgun_shared.cpp.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_portalgun_shared.h"
#include "portal_player.h"
#include "portal_mp_gamerules.h"
#include "particle_parse.h"
#include "soundenvelope.h"
#include "portal_grabcontroller_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *CWeaponPortalgun::s_szGunEffectsThinkContext = "PortalgunEffectsThinkContext";
const char *CWeaponPortalgun::s_szTogglePotatosThinkContext = "TogglePotatosThinkContext";

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPortalgun, DT_WeaponPortalgun )

BEGIN_NETWORK_TABLE( CWeaponPortalgun, DT_WeaponPortalgun )
	SendPropBool( SENDINFO( m_bCanFirePortal1 ) ),
	SendPropBool( SENDINFO( m_bCanFirePortal2 ) ),
	SendPropInt( SENDINFO( m_iLastFiredPortal ) ),
	SendPropBool( SENDINFO( m_bOpenProngs ) ),
	SendPropFloat( SENDINFO( m_fEffectsMaxSize1 ) ), // HACK HACK! Used to make the gun visually change when going through a cleanser!
	SendPropFloat( SENDINFO( m_fEffectsMaxSize2 ) ),
	SendPropInt( SENDINFO( m_EffectState ) ),
	SendPropEHandle( SENDINFO( m_hPrimaryPortal ) ),
	SendPropEHandle( SENDINFO( m_hSecondaryPortal ) ),
	SendPropVector( SENDINFO( m_vecBluePortalPos ), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecOrangePortalPos ), -1, SPROP_NOSCALE ),
END_NETWORK_TABLE()

BEGIN_DATADESC( CWeaponPortalgun )

	DEFINE_KEYFIELD( m_bCanFirePortal1, FIELD_BOOLEAN, "CanFirePortal1" ),
	DEFINE_KEYFIELD( m_bCanFirePortal2, FIELD_BOOLEAN, "CanFirePortal2" ),
	DEFINE_KEYFIELD( m_bShowingPotatos, FIELD_BOOLEAN, "ShowingPotatos" ),
	DEFINE_FIELD( m_iLastFiredPortal, FIELD_INTEGER ),
	DEFINE_FIELD( m_bOpenProngs, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fEffectsMaxSize1, FIELD_FLOAT ),
	DEFINE_FIELD( m_fEffectsMaxSize2, FIELD_FLOAT ),
	DEFINE_FIELD( m_EffectState, FIELD_INTEGER ),
	DEFINE_FIELD( m_iPortalLinkageGroupID, FIELD_CHARACTER ),
	DEFINE_FIELD( m_hFreezeEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hPrimaryPortal, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hSecondaryPortal, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecBluePortalPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecOrangePortalPos, FIELD_POSITION_VECTOR ),

	DEFINE_INPUTFUNC( FIELD_VOID, "ChargePortal1", InputChargePortal1 ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ChargePortal2", InputChargePortal2 ),
	DEFINE_INPUTFUNC( FIELD_VOID, "FirePortal1", FirePortal1Helper ),
	DEFINE_INPUTFUNC( FIELD_VOID, "FirePortal2", FirePortal2Helper ),
	DEFINE_INPUTFUNC( FIELD_VECTOR, "FirePortalDirection1", FirePortalDirection1 ),
	DEFINE_INPUTFUNC( FIELD_VECTOR, "FirePortalDirection2", FirePortalDirection2 ),

	DEFINE_SOUNDPATCH( m_pMiniGravHoldSound ),

	DEFINE_OUTPUT( m_OnFiredPortal1, "OnFiredPortal1" ),
	DEFINE_OUTPUT( m_OnFiredPortal2, "OnFiredPortal2" ),

	DEFINE_THINKFUNC( GunEffectsThink ),
	DEFINE_THINKFUNC( TogglePotatosThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_portalgun, CWeaponPortalgun );
PRECACHE_WEAPON_REGISTER( weapon_portalgun );


void CWeaponPortalgun::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetThink( NULL );
	SetContextThink( &CWeaponPortalgun::GunEffectsThink, gpGlobals->curtime + 0.1f, s_szGunEffectsThinkContext );

	if ( GameRules()->IsMultiplayer() )
	{
		CBaseEntity *pOwner = GetOwner();
		if ( pOwner && pOwner->IsPlayer() )
		{
			m_iPortalLinkageGroupID = pOwner->entindex();

			m_hPrimaryPortal = CProp_Portal::FindPortal( m_iPortalLinkageGroupID, false, true );
			m_hSecondaryPortal = CProp_Portal::FindPortal( m_iPortalLinkageGroupID, true, true );
		}
	}
}

void CWeaponPortalgun::Activate( void )
{
	BaseClass::Activate();

	CreateSounds();

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer )
	{
		CBaseEntity *pHeldObject = GetPlayerHeldEntity( pPlayer );
		OpenProngs( ( pHeldObject ) ? ( false ) : ( true ) );
		OpenProngs( ( pHeldObject ) ? ( true ) : ( false ) );

		if ( GameRules()->IsMultiplayer() )
		{
			m_iPortalLinkageGroupID = pPlayer->entindex();

			m_hPrimaryPortal = CProp_Portal::FindPortal( m_iPortalLinkageGroupID, false, true );
			m_hSecondaryPortal = CProp_Portal::FindPortal( m_iPortalLinkageGroupID, true, true );
		}
	}

	// HACK HACK! Used to make the gun visually change when going through a cleanser!
	m_fEffectsMaxSize1 = 4.0f;
	m_fEffectsMaxSize2 = 4.0f;
}

void CWeaponPortalgun::OnPickedUp( CBaseCombatCharacter *pNewOwner )
{
	if ( GameRules()->IsMultiplayer() && pNewOwner && pNewOwner->IsPlayer() )
	{
		m_iPortalLinkageGroupID = pNewOwner->entindex();
	}

	m_hPrimaryPortal = CProp_Portal::FindPortal( m_iPortalLinkageGroupID, false, true );
	m_hSecondaryPortal = CProp_Portal::FindPortal( m_iPortalLinkageGroupID, true, true );

	// The gun takes its new owner's team (and passes it to its portals).
	if ( pNewOwner )
	{
		ChangeTeam( pNewOwner->GetTeamNumber() );
	}

	BaseClass::OnPickedUp( pNewOwner );
}

void CWeaponPortalgun::ChangeTeam( int iTeamNum )
{
	BaseClass::ChangeTeam( iTeamNum );

	if ( m_hPrimaryPortal.Get() )
	{
		m_hPrimaryPortal->ChangeTeam( iTeamNum );
	}

	if ( m_hSecondaryPortal.Get() )
	{
		m_hSecondaryPortal->ChangeTeam( iTeamNum );
	}
}

void CWeaponPortalgun::SetLinkageGroupID( unsigned char iNewID )
{
	m_iPortalLinkageGroupID = iNewID;

	m_hPrimaryPortal = CProp_Portal::FindPortal( m_iPortalLinkageGroupID, false, true );
	m_hSecondaryPortal = CProp_Portal::FindPortal( m_iPortalLinkageGroupID, true, true );
}

void CWeaponPortalgun::UpdatePortalAssociation( void )
{
	SetLinkageGroupID( m_iPortalLinkageGroupID );
}

void CWeaponPortalgun::ClearPortalPositions( void )
{
	m_vecBluePortalPos = vec3_invalid;
	m_vecOrangePortalPos = vec3_invalid;
}

void CWeaponPortalgun::PortalPlaced( void )
{
	// Multiplayer rounds count every placed portal.
	CPortalMPGameRules *pRules = PortalMPGameRules();
	if ( pRules )
	{
		pRules->PortalPlaced();
	}
}

void CWeaponPortalgun::CreateSounds( void )
{
	// Portal 2 does not play the Portal 1 hold hum; m_pMiniGravHoldSound stays
	// NULL unless restored from a Portal 1 save.
}

void CWeaponPortalgun::StopLoopingSounds( void )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	controller.SoundDestroy( m_pMiniGravHoldSound );
	m_pMiniGravHoldSound = NULL;

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
// Purpose: Allows a generic think function before the others are called
//-----------------------------------------------------------------------------
bool CWeaponPortalgun::PreThink( void )
{
	// Animate
	StudioFrameAdvance();

	// Do not interrupt current think function
	return false;
}

void CWeaponPortalgun::GunEffectsThink( void )
{
	// Allow descended classes a chance to do something before the think function
	if ( PreThink() )
		return;

	SetContextThink( &CWeaponPortalgun::GunEffectsThink, gpGlobals->curtime + 0.1f, s_szGunEffectsThinkContext );

	CPortal_Player *pPlayer = ToPortalPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	// Co-op guns are skinned for their team; the view model matches the gun.
	if ( pPlayer->GetViewModel() && GameRules()->IsMultiplayer() )
	{
		m_nSkin = ( pPlayer->GetTeamNumber() == TEAM_RED ) ? 2 : 1;
		pPlayer->GetViewModel()->m_nSkin = m_nSkin;
	}

	// HACK HACK! Used to make the gun visually change when going through a cleanser!
	if ( m_fEffectsMaxSize1 > 4.0f )
	{
		m_fEffectsMaxSize1 -= gpGlobals->frametime * 400.0f;
		if ( m_fEffectsMaxSize1 < 4.0f )
			m_fEffectsMaxSize1 = 4.0f;
	}

	if ( m_fEffectsMaxSize2 > 4.0f )
	{
		m_fEffectsMaxSize2 -= gpGlobals->frametime * 400.0f;
		if ( m_fEffectsMaxSize2 < 4.0f )
			m_fEffectsMaxSize2 = 4.0f;
	}
}

void CWeaponPortalgun::OpenProngs( bool bOpenProngs )
{
	if ( m_bOpenProngs == bOpenProngs )
		return;

	m_bOpenProngs = bOpenProngs;

	DoEffect( ( m_bOpenProngs ) ? ( EFFECT_HOLDING ) : ( EFFECT_READY ) );

	SendWeaponAnim( ( m_bOpenProngs ) ? ( ACT_VM_PICKUP ) : ( ACT_VM_RELEASE ) );
}

void CWeaponPortalgun::InputChargePortal1( inputdata_t &inputdata )
{
	DispatchParticleEffect( "portal_1_charge", PATTACH_POINT_FOLLOW, this, "muzzle" );
}

void CWeaponPortalgun::InputChargePortal2( inputdata_t &inputdata )
{
	DispatchParticleEffect( "portal_2_charge", PATTACH_POINT_FOLLOW, this, "muzzle" );
}

void CWeaponPortalgun::FreezeEntity( CBaseEntity *pFreezeEntity )
{
	if ( pFreezeEntity->GetMoveType() != MOVETYPE_VPHYSICS )
		return;

	IPhysicsObject *pPhysObj = pFreezeEntity->VPhysicsGetObject();
	if ( pPhysObj )
	{
		pPhysObj->EnableMotion( false );
		m_hFreezeEntity = pFreezeEntity;
	}
}

void CWeaponPortalgun::UnfreezeEntity( void )
{
	CBaseEntity *pFreezeEntity = m_hFreezeEntity.Get();
	if ( pFreezeEntity )
	{
		IPhysicsObject *pPhysObj = pFreezeEntity->VPhysicsGetObject();
		if ( pPhysObj )
		{
			pPhysObj->EnableMotion( true );
			pPhysObj->Wake();
		}
	}

	m_hFreezeEntity = NULL;
}

void CWeaponPortalgun::FirePortalDirection1( inputdata_t &inputdata )
{
	Vector vDirection;
	inputdata.value.Vector3D( vDirection );

	PortalPlacementResult_t eResult = FirePortal( false, &vDirection );
	if ( PortalPlacementSucceeded( eResult ) )
	{
		CProp_Portal *pPortal = m_hPrimaryPortal.Get();
		if ( pPortal )
		{
			m_vecBluePortalPos = pPortal->m_vDelayedPosition;
		}

		SetLastFiredPortal( 1 );
	}

	CBaseCombatCharacter *pOwner = GetOwner();
	WeaponSound( ( pOwner && pOwner->IsPlayer() ) ? SINGLE : SINGLE_NPC );
}

void CWeaponPortalgun::FirePortalDirection2( inputdata_t &inputdata )
{
	Vector vDirection;
	inputdata.value.Vector3D( vDirection );

	PortalPlacementResult_t eResult = FirePortal( true, &vDirection );
	if ( PortalPlacementSucceeded( eResult ) )
	{
		CProp_Portal *pPortal = m_hSecondaryPortal.Get();
		if ( pPortal )
		{
			m_vecOrangePortalPos = pPortal->m_vDelayedPosition;
		}

		SetLastFiredPortal( 2 );
	}

	CBaseCombatCharacter *pOwner = GetOwner();
	WeaponSound( ( pOwner && pOwner->IsPlayer() ) ? WPN_DOUBLE : DOUBLE_NPC );
}

ConVar sv_portal_shot_push( "sv_portal_shot_push", "0", FCVAR_REPLICATED, "[PORTALMP] Amount of force to apply to a player if your shot hits them. <= 0 passes through the player" );
ConVar sv_portal_shot_fizzles_enemy_portals( "sv_portal_shot_fizzles_enemy_portals", "1", FCVAR_REPLICATED, "[PORTALMP] Your portal shots will fizzle any enemy player portals that they hit" );
ConVar sv_portal_shot_fizzles_friendly_portals( "sv_portal_shot_fizzles_friendly_portals", "1", FCVAR_REPLICATED, "[PORTALMP] Your portal shots will fizzle any friendly player portals that they hit" );

//-----------------------------------------------------------------------------
// Effects are drawn by the client; the server only drives the hold sound.
//-----------------------------------------------------------------------------
void CWeaponPortalgun::StartEffects( void )
{
}

void CWeaponPortalgun::DestroyEffects( void )
{
	// Stop everything
	StopEffects();
}

void CWeaponPortalgun::DoEffectReady( void )
{
	if ( m_pMiniGravHoldSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundChangeVolume( m_pMiniGravHoldSound, 0.0f, 0.1f );
	}
}

void CWeaponPortalgun::DoEffectHolding( void )
{
	if ( m_pMiniGravHoldSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundChangeVolume( m_pMiniGravHoldSound, 1.0f, 0.1f );
	}
}

void CWeaponPortalgun::DoEffectNone( void )
{
	if ( m_pMiniGravHoldSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundChangeVolume( m_pMiniGravHoldSound, 0.0f, 0.1f );
	}
}

void CC_UpgradePortalGun( void )
{
	CPortal_Player *pPlayer = ToPortalPlayer( UTIL_GetCommandClient() );
	if ( pPlayer == NULL )
		return;

	CWeaponPortalgun *pPortalGun = static_cast<CWeaponPortalgun *>( pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) );
	if ( pPortalGun == NULL )
	{
		Msg( "Portalgun upgrade failed! Player not holding a portalgun.\n" );
		return;
	}

	Msg( "Upgrading Portalgun\n" );
	pPortalGun->SetCanFirePortal1();
	pPortalGun->SetCanFirePortal2();
}

void CC_UpgradePotatoGun( void )
{
	CPortal_Player *pPlayer = ToPortalPlayer( UTIL_GetCommandClient() );
	if ( pPlayer == NULL )
		return;

	CWeaponPortalgun *pPortalGun = static_cast<CWeaponPortalgun *>( pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) );
	if ( pPortalGun == NULL )
	{
		Msg( "Potatogun upgrade failed! Player not holding a portalgun.\n" );
		return;
	}

	Msg( "Upgrading Portalgun with Potato\n" );
	pPortalGun->SetCanFirePortal1();
	pPortalGun->SetCanFirePortal2();
	pPortalGun->SetPotatosOnPortalgun( true );
}

static ConCommand upgrade_portal( "upgrade_portalgun", CC_UpgradePortalGun, "Equips the player with a single portal portalgun. Use twice for a dual portal portalgun.\n\tArguments:   	none ", FCVAR_CHEAT );
static ConCommand upgrade_potatogun( "upgrade_potatogun", CC_UpgradePotatoGun, "Upgrades to the portalgun to the dual portalgun with potatos attached", FCVAR_CHEAT );

static void change_portalgun_linkage_id_f( const CCommand &args )
{
	if ( sv_cheats->GetBool() == false ) // no cheating in multiplayer
		return;

	if ( args.ArgC() < 2 )
		return;

	CPortal_Player *pPlayer = ToPortalPlayer( UTIL_GetCommandClient() );
	if ( pPlayer == NULL )
		return;

	for ( int i = 0; i < MAX_WEAPONS; ++i )
	{
		CWeaponPortalgun *pPortalGun = dynamic_cast<CWeaponPortalgun *>( pPlayer->GetWeapon( i ) );
		if ( pPortalGun == NULL )
			continue;

		unsigned char iNewID;
		if ( args.ArgC() > 2 )
		{
			// cycle through the range (low, high, increment)
			unsigned char iLow = (unsigned char)atoi( args[1] );
			unsigned char iHigh = (unsigned char)atoi( args[2] );
			int iIncrement = ( args.ArgC() > 3 ) ? atoi( args[3] ) : 1;

			int iTest = pPortalGun->GetLinkageGroupID() + iIncrement;
			if ( iTest > iHigh )
				iNewID = iLow;
			else if ( iTest < iLow )
				iNewID = iHigh;
			else
				iNewID = (unsigned char)iTest;
		}
		else
		{
			iNewID = (unsigned char)atoi( args[1] );
		}

		pPortalGun->SetLinkageGroupID( iNewID );
		return;
	}
}

ConCommand change_portalgun_linkage_id( "change_portalgun_linkage_id", change_portalgun_linkage_id_f, "Changes the portal linkage ID for the portal gun held by the commanding player. Give it three numbers to cycle through a range (low, high, increment)", FCVAR_CHEAT );

void CWeaponPortalgun::SetPotatosOnPortalgun( bool bShowPotatos )
{
	m_bShowingPotatos = bShowPotatos;

	// The view model body group is switched on the next think, after the gun
	// has been deployed.
	SetContextThink( &CWeaponPortalgun::TogglePotatosThink, gpGlobals->curtime + 0.1f, s_szTogglePotatosThinkContext );
}

void CWeaponPortalgun::TogglePotatosThink( void )
{
	SetContextThink( NULL, TICK_NEVER_THINK, s_szTogglePotatosThinkContext );

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL )
		return;

	CBaseViewModel *pViewModel = pPlayer->GetViewModel();
	if ( pViewModel == NULL )
		return;

	int iBodyGroup = pViewModel->FindBodygroupByName( "potatos_vmodel" );
	pViewModel->SetBodygroup( iBodyGroup, m_bShowingPotatos );
}
