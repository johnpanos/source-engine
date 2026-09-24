//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal 2 portal gun (client).
//
// Portal 2 reconstruction: this file was absent from the imported Portal 2
// source (the Portal 1 gun occupies game/client/portal/c_weapon_portalgun.cpp,
// whose sprite and beam effects it keeps). Behavior follows the Steam2 depot
// 852_3 client.dylib (DWARF and decompiled functions): the team-tinted light,
// the view model dynamic light and the cleanser effect are Portal 2 additions.
// Firing and placement live in the retained Portal 2
// game/shared/portal2/weapon_portalgun_shared.cpp.
//
//=============================================================================//

#include "cbase.h"

#include "c_portal_player.h"
#include "c_te_effect_dispatch.h"
#include "iviewrender_beams.h"
#include "model_types.h"
#include "fx_interpvalue.h"
#include "clienteffectprecachesystem.h"
#include "bone_setup.h"
#include "c_rumble.h"
#include "rumble_shared.h"
#include "dlight.h"
#include "iefx.h"
#include "portal_util_shared.h"
#include "portal_mp_gamerules.h"

#include "weapon_portalgun_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	SPRITE_SCALE 128.0f

ConVar cl_portalgun_effects_min_alpha( "cl_portalgun_effects_min_alpha", "96", FCVAR_CLIENTDLL );
ConVar cl_portalgun_effects_max_alpha( "cl_portalgun_effects_max_alpha", "128", FCVAR_CLIENTDLL );

ConVar cl_portalgun_effects_min_size( "cl_portalgun_effects_min_size", "3.0", FCVAR_CLIENTDLL );
ConVar cl_portalgun_beam_size( "cl_portalgun_beam_size", "0.04", FCVAR_CLIENTDLL );

ConVar portal_viewmodel_use_dlight( "portal_viewmodel_use_dlight", "0", FCVAR_CLIENTDLL );
ConVar portal_viewmodel_radius( "portal_viewmodel_radius", "24", FCVAR_CLIENTDLL );
ConVar portal_viewmodel_offset( "portal_viewmodel_offset", "-12", FCVAR_CLIENTDLL );


// Precache the effects
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectPortalgun )
CLIENTEFFECT_MATERIAL( PORTALGUN_BEAM_SPRITE )
CLIENTEFFECT_MATERIAL( PORTALGUN_BEAM_SPRITE_NOZ )
CLIENTEFFECT_MATERIAL( PORTALGUN_GLOW_SPRITE )
CLIENTEFFECT_MATERIAL( PORTALGUN_ENDCAP_SPRITE )
CLIENTEFFECT_MATERIAL( PORTALGUN_GRAV_ACTIVE_GLOW )
CLIENTEFFECT_MATERIAL( PORTALGUN_PORTAL1_FIRED_LAST_GLOW )
CLIENTEFFECT_MATERIAL( PORTALGUN_PORTAL2_FIRED_LAST_GLOW )
CLIENTEFFECT_MATERIAL( PORTALGUN_PORTAL_TINTED_GLOW )
CLIENTEFFECT_MATERIAL( PORTALGUN_PORTAL_MUZZLE_GLOW_SPRITE )
CLIENTEFFECT_MATERIAL( PORTALGUN_PORTAL_TUBE_BEAM_SPRITE )
CLIENTEFFECT_REGISTER_END()


CPortalgunEffectBeam::CPortalgunEffectBeam( void )
	: m_pBeam( NULL ),
	  m_fBrightness( 255.0f )
{}

CPortalgunEffectBeam::~CPortalgunEffectBeam( void )
{
	Release();
}

void CPortalgunEffectBeam::Release( void )
{
	if ( m_pBeam != NULL )
	{
		m_pBeam->flags = 0;
		m_pBeam->die = gpGlobals->curtime - 1;

		m_pBeam = NULL;
	}
}

void CPortalgunEffectBeam::Init( int startAttachment, int endAttachment, C_BaseEntity *pEntity, bool firstPerson )
{
	if ( m_pBeam != NULL )
		return;

	BeamInfo_t beamInfo;

	beamInfo.m_pStartEnt = pEntity;
	beamInfo.m_nStartAttachment = startAttachment;
	beamInfo.m_pEndEnt = pEntity;
	beamInfo.m_nEndAttachment = endAttachment;
	beamInfo.m_nType = TE_BEAMPOINTS;
	beamInfo.m_vecStart = vec3_origin;
	beamInfo.m_vecEnd = vec3_origin;

	beamInfo.m_pszModelName = ( firstPerson ) ? PORTALGUN_BEAM_SPRITE_NOZ : PORTALGUN_BEAM_SPRITE;

	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 0.0f;

	if ( firstPerson )
	{
		beamInfo.m_flWidth = 0.0f;
		beamInfo.m_flEndWidth = 2.0f;
	}
	else
	{
		beamInfo.m_flWidth = 0.5f;
		beamInfo.m_flEndWidth = 2.0f;
	}

	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 16;
	beamInfo.m_flBrightness = 128.0;
	beamInfo.m_flSpeed = 150.0f;
	beamInfo.m_nStartFrame = 0.0;
	beamInfo.m_flFrameRate = 30.0;
	beamInfo.m_flRed = 255.0;
	beamInfo.m_flGreen = 255.0;
	beamInfo.m_flBlue = 255.0;
	beamInfo.m_nSegments = 8;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = FBEAM_FOREVER;

	m_pBeam = beams->CreateBeamEntPoint( beamInfo );

	if ( m_pBeam )
	{
		m_pBeam->m_bDrawInMainRender = false;
		m_pBeam->m_bDrawInPortalRender = false;
	}
}

void CPortalgunEffectBeam::SetVisibleViewModel( bool visible /*= true*/ )
{
	if ( m_pBeam == NULL )
		return;

	m_pBeam->m_bDrawInMainRender = visible;
}

int CPortalgunEffectBeam::IsVisibleViewModel( void ) const
{
	if ( m_pBeam == NULL )
		return false;

	return m_pBeam->m_bDrawInMainRender;
}

void CPortalgunEffectBeam::SetVisible3rdPerson( bool visible /*= true*/ )
{
	if ( m_pBeam == NULL )
		return;

	m_pBeam->m_bDrawInPortalRender = visible;
}

int CPortalgunEffectBeam::SetVisible3rdPerson( void ) const
{
	if ( m_pBeam == NULL )
		return false;

	return m_pBeam->m_bDrawInPortalRender;
}

void CPortalgunEffectBeam::SetBrightness( float fBrightness )
{
	m_fBrightness = clamp( fBrightness, 0.0f, 255.0f );
}

void CPortalgunEffectBeam::DrawBeam( void )
{
	if ( m_pBeam )
		m_pBeam->DrawModel( 0 );
}


IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPortalgun, DT_WeaponPortalgun )

BEGIN_NETWORK_TABLE( C_WeaponPortalgun, DT_WeaponPortalgun )
	RecvPropBool( RECVINFO( m_bCanFirePortal1 ) ),
	RecvPropBool( RECVINFO( m_bCanFirePortal2 ) ),
	RecvPropInt( RECVINFO( m_iLastFiredPortal ) ),
	RecvPropBool( RECVINFO( m_bOpenProngs ) ),
	RecvPropFloat( RECVINFO( m_fEffectsMaxSize1 ) ), // HACK HACK! Used to make the gun visually change when going through a cleanser!
	RecvPropFloat( RECVINFO( m_fEffectsMaxSize2 ) ),
	RecvPropInt( RECVINFO( m_EffectState ) ),
	RecvPropEHandle( RECVINFO( m_hPrimaryPortal ) ),
	RecvPropEHandle( RECVINFO( m_hSecondaryPortal ) ),
	RecvPropVector( RECVINFO( m_vecBluePortalPos ) ),
	RecvPropVector( RECVINFO( m_vecOrangePortalPos ) ),
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( C_WeaponPortalgun )
	DEFINE_PRED_FIELD( m_bCanFirePortal1, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bCanFirePortal2, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iLastFiredPortal, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bOpenProngs, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_EffectState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecBluePortalPos, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecOrangePortalPos, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_portalgun, C_WeaponPortalgun );
PRECACHE_WEAPON_REGISTER( weapon_portalgun );


C_WeaponPortalgun::~C_WeaponPortalgun( void )
{
	// The engine owns the light; let it expire now.
	if ( m_pELight )
	{
		m_pELight->die = gpGlobals->curtime;
		m_pELight = NULL;
	}
}

void C_WeaponPortalgun::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	m_pELight = NULL;

	SetNextThink( gpGlobals->curtime + 0.1 );
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the view model light, sprites and beams
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::StartEffects( void )
{
	int i;

	C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );
	C_BaseEntity *pModelView = ( pOwner ) ? ( pOwner->GetViewModel() ) : ( NULL );
	C_BaseEntity *pModelWorld = this;

	if ( !pModelView )
	{
		pModelView = pModelWorld;
	}

	int nBodyLightAttachment = pModelView->LookupAttachment( "Body_light" );
	if ( nBodyLightAttachment == 0 )
		return;

	// ------------------------------------------
	// View model dynamic light
	// ------------------------------------------

	if ( portal_viewmodel_use_dlight.GetBool() && m_pELight == NULL && GetOwner() )
	{
		m_pELight = effects->CL_AllocElight( LIGHT_INDEX_MUZZLEFLASH + GetOwner()->entindex() );
	}

	if ( m_pELight )
	{
		m_pELight->die = gpGlobals->curtime + 0.1f;
		m_pELight->flags = DLIGHT_NO_WORLD_ILLUMINATION;
		m_pELight->radius = portal_viewmodel_radius.GetFloat();
		m_pELight->style = 0;
		// Portal 2 port: this engine's dlight_t has no exclusive light receiver,
		// so the light also reaches the world and props near the view model.
		static bool s_bWarnedExclusiveLight = false;
		if ( !s_bWarnedExclusiveLight )
		{
			s_bWarnedExclusiveLight = true;
			DevWarning( "Portal 2: view-model-only dynamic lights are not supported by this engine\n" );
		}

		Vector colorMagSprites = GetEffectColor( m_iLastFiredPortal );
		m_pELight->color.r = colorMagSprites.x;
		m_pELight->color.g = colorMagSprites.y;
		m_pELight->color.b = colorMagSprites.z;
		m_pELight->color.exponent = 0;

		Vector vecOrigin;
		QAngle qAngles;
		pModelView->GetAttachment( nBodyLightAttachment, vecOrigin, qAngles );

		Vector vForward, vRight, vUp;
		AngleVectors( qAngles, &vForward, &vRight, &vUp );
		m_pELight->origin = vecOrigin + vRight * portal_viewmodel_offset.GetFloat();
	}

	// ------------------------------------------
	// Lights
	// ------------------------------------------

	static const char *s_pszLightMaterials[] =
	{
		PORTALGUN_GRAV_ACTIVE_GLOW,			// PORTALGUN_GRAVLIGHT
		PORTALGUN_PORTAL1_FIRED_LAST_GLOW,	// PORTALGUN_PORTAL1LIGHT
		PORTALGUN_PORTAL2_FIRED_LAST_GLOW,	// PORTALGUN_PORTAL2LIGHT
		PORTALGUN_PORTAL_TINTED_GLOW,		// PORTALGUN_PORTALTINTEDLIGHT
	};

	for ( i = 0; i < (int)ARRAYSIZE( s_pszLightMaterials ); ++i )
	{
		// View model light, then its world model twin
		CPortalgunEffect &viewLight = m_Parameters[PORTALGUN_GRAVLIGHT + i * 2];
		if ( viewLight.GetMaterial() == NULL )
		{
			viewLight.GetScale().SetAbsolute( 0.018f * SPRITE_SCALE );
			viewLight.GetAlpha().SetAbsolute( 128.0f );
			viewLight.SetAttachment( nBodyLightAttachment );
			viewLight.SetVisible( false );

			if ( viewLight.SetMaterial( s_pszLightMaterials[i] ) == false )
			{
				// This means the texture was not found
				Assert( 0 );
			}
		}

		CPortalgunEffect &worldLight = m_Parameters[PORTALGUN_GRAVLIGHT_WORLD + i * 2];
		if ( worldLight.GetMaterial() == NULL )
		{
			worldLight.GetScale().SetAbsolute( 0.03f * SPRITE_SCALE );
			worldLight.GetAlpha().SetAbsolute( 128.0f );
			worldLight.SetAttachment( pModelWorld->LookupAttachment( "Body_light" ) );
			worldLight.SetVisible( false );

			if ( worldLight.SetMaterial( s_pszLightMaterials[i] ) == false )
			{
				// This means the texture was not found
				Assert( 0 );
			}
		}
	}

	// The Portal 1 glow and endcap sprites are not used by the Portal 2 gun.
	for ( i = PORTALGUN_GLOW1; i <= PORTALGUN_ENDCAP3_WORLD; i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

	// ------------------------------------------
	// Internal sprites
	// ------------------------------------------

	i = PORTALGUN_MUZZLE_GLOW;
	if ( m_Parameters[i].GetMaterial() == NULL )
	{
		m_Parameters[i].GetScale().SetAbsolute( 0.1f * SPRITE_SCALE );
		m_Parameters[i].GetAlpha().SetAbsolute( 32.0f );
		m_Parameters[i].SetAttachment( pModelView->LookupAttachment( "Inside_effects" ) );
		m_Parameters[i].SetVisible( false );

		if ( m_Parameters[i].SetMaterial( PORTALGUN_PORTAL_MUZZLE_GLOW_SPRITE ) == false )
		{
			// This means the texture was not found
			Assert( 0 );
		}
	}

	i = PORTALGUN_MUZZLE_GLOW_WORLD;
	if ( m_Parameters[i].GetMaterial() == NULL )
	{
		m_Parameters[i].GetScale().SetAbsolute( 0.025f * SPRITE_SCALE );
		m_Parameters[i].GetAlpha().SetAbsolute( 64.0f );
		m_Parameters[i].SetAttachment( pModelWorld->LookupAttachment( "Inside_effects" ) );
		m_Parameters[i].SetVisible( false );

		if ( m_Parameters[i].SetMaterial( PORTALGUN_PORTAL_MUZZLE_GLOW_SPRITE ) == false )
		{
			// This means the texture was not found
			Assert( 0 );
		}
	}

	// ------------------------------------------
	// Tube sprites
	// ------------------------------------------

	static const char *attachNamesTubeBeam[] =
	{
		"Beam_point1",
		"Beam_point2",
		"Beam_point3",
		"Beam_point4",
		"Beam_point5",
	};

	for ( i = PORTALGUN_TUBE_BEAM1; i < ( PORTALGUN_TUBE_BEAM1 + NUM_TUBE_BEAM_SPRITES ); i++ )
	{
		if ( m_Parameters[i].GetMaterial() == NULL )
		{
			m_Parameters[i].GetScale().SetAbsolute( cl_portalgun_beam_size.GetFloat() * SPRITE_SCALE );
			m_Parameters[i].GetAlpha().SetAbsolute( 255.0f );
			m_Parameters[i].SetAttachment( pModelView->LookupAttachment( attachNamesTubeBeam[i - PORTALGUN_TUBE_BEAM1] ) );
			m_Parameters[i].SetVisible( false );

			if ( m_Parameters[i].SetMaterial( PORTALGUN_PORTAL_TUBE_BEAM_SPRITE ) == false )
			{
				// This means the texture was not found
				Assert( 0 );
			}
		}
	}

	for ( i = PORTALGUN_TUBE_BEAM1_WORLD; i < ( PORTALGUN_TUBE_BEAM1_WORLD + NUM_TUBE_BEAM_SPRITES_WORLD ); i++ )
	{
		if ( m_Parameters[i].GetMaterial() == NULL )
		{
			m_Parameters[i].GetScale().SetAbsolute( cl_portalgun_beam_size.GetFloat() * SPRITE_SCALE );
			m_Parameters[i].GetAlpha().SetAbsolute( 255.0f );
			m_Parameters[i].SetAttachment( pModelView->LookupAttachment( attachNamesTubeBeam[i - PORTALGUN_TUBE_BEAM1_WORLD] ) );
			m_Parameters[i].SetVisible( false );

			if ( m_Parameters[i].SetMaterial( PORTALGUN_PORTAL_TUBE_BEAM_SPRITE ) == false )
			{
				// This means the texture was not found
				Assert( 0 );
			}
		}
	}

	// ------------------------------------------
	// Beams
	// ------------------------------------------

	// Setup the beams
	int iBeam = 0;

	if ( pModelView != pModelWorld )
	{
		m_Beams[iBeam++].Init( pModelView->LookupAttachment( "Arm1_attach3" ), pModelView->LookupAttachment( "muzzle" ), pModelView, true );
		m_Beams[iBeam++].Init( pModelView->LookupAttachment( "Arm2_attach3" ), pModelView->LookupAttachment( "muzzle" ), pModelView, true );
		m_Beams[iBeam++].Init( pModelView->LookupAttachment( "Arm3_attach3" ), pModelView->LookupAttachment( "muzzle" ), pModelView, true );
	}
	else
	{
		iBeam += 3;
	}

	m_Beams[iBeam++].Init( pModelWorld->LookupAttachment( "Arm1_attach3" ), pModelWorld->LookupAttachment( "muzzle" ), pModelWorld, false );
	m_Beams[iBeam++].Init( pModelWorld->LookupAttachment( "Arm2_attach3" ), pModelWorld->LookupAttachment( "muzzle" ), pModelWorld, false );
	m_Beams[iBeam++].Init( pModelWorld->LookupAttachment( "Arm3_attach3" ), pModelWorld->LookupAttachment( "muzzle" ), pModelWorld, false );
}

void C_WeaponPortalgun::DestroyEffects( void )
{
	// Free our beams
	for ( int i = 0; i < NUM_PORTALGUN_BEAMS; ++i )
	{
		m_Beams[i].Release();
	}

	// Stop everything
	StopEffects();
}

//-----------------------------------------------------------------------------
// Purpose: Ready effects
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::DoEffectReady( void )
{
	int i;

	// Turn off the endcap sprites
	for ( i = PORTALGUN_ENDCAP1; i < ( PORTALGUN_ENDCAP1 + NUM_ENDCAP_SPRITES ); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

	// Turn on the tube beam sprites
	Vector colorMagSprites = GetEffectColor( PORTALGUN_MUZZLE_GLOW );
	for ( i = PORTALGUN_TUBE_BEAM1; i < ( PORTALGUN_TUBE_BEAM1 + NUM_TUBE_BEAM_SPRITES ); i++ )
	{
		m_Parameters[i].SetColor( colorMagSprites );
		m_Parameters[i].SetVisibleViewModel();
	}

	// Turn the beams off
	for ( i = 0; i < NUM_PORTALGUN_BEAMS; ++i )
	{
		m_Beams[i].SetVisibleViewModel( false );
		m_Beams[i].SetVisible3rdPerson( false );
	}

	C_BasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer )
	{
		RumbleEffect( RUMBLE_PHYSCANNON_OPEN, 0, RUMBLE_FLAG_STOP );
	}
}


//-----------------------------------------------------------------------------
// Holding effects
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::DoEffectHolding( void )
{
	int i;

	// Turn on the tube beam sprites
	Vector colorMagSprites = GetEffectColor( PORTALGUN_MUZZLE_GLOW );
	for ( i = PORTALGUN_TUBE_BEAM1; i < ( PORTALGUN_TUBE_BEAM1 + NUM_TUBE_BEAM_SPRITES ); i++ )
	{
		m_Parameters[i].SetColor( colorMagSprites );
		m_Parameters[i].SetVisibleViewModel();
	}

	// Show the first-person beams
	for ( i = 0; i < NUM_PORTALGUN_BEAMS / 2; ++i )
	{
		m_Beams[i].SetVisible3rdPerson( false );
		m_Beams[i].SetVisibleViewModel();
		m_Beams[i].SetBrightness( 128.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown for the weapon when it's holstered
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::DoEffectNone( void )
{
	for ( int i = 0; i < NUM_PORTALGUN_PARAMETERS; i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

	for ( int i = 0; i < NUM_PORTALGUN_BEAMS; ++i )
	{
		m_Beams[i].SetVisibleViewModel( false );
		m_Beams[i].SetVisible3rdPerson( false );
	}
}

void C_WeaponPortalgun::OnPreDataChanged( DataUpdateType_t updateType )
{
	m_bOldCanFirePortal1 = m_bCanFirePortal1;
	m_bOldCanFirePortal2 = m_bCanFirePortal2;

	BaseClass::OnPreDataChanged( updateType );
}

void C_WeaponPortalgun::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		// Start thinking (Baseclass stops it)
		SetNextClientThink( CLIENT_THINK_ALWAYS );

		{
			C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, true );
			StartEffects();
		}

		DoEffect( m_EffectState );
	}

	// Update effect state when out of parity with the server
	else if ( m_nOldEffectState != m_EffectState || m_bOldCanFirePortal1 != m_bCanFirePortal1 || m_bOldCanFirePortal2 != m_bCanFirePortal2 )
	{
		DoEffect( m_EffectState );
		m_nOldEffectState = m_EffectState;

		m_bOldCanFirePortal1 = m_bCanFirePortal1;
		m_bOldCanFirePortal2 = m_bCanFirePortal2;
	}

	// Co-op guns are skinned for their team; the view model matches the gun.
	if ( GameRules()->IsMultiplayer() )
	{
		C_BasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( pPlayer )
		{
			int nSkin = ( pPlayer->GetTeamNumber() == TEAM_RED ) ? 2 : 1;
			m_nSkin = nSkin;

			if ( pPlayer->GetViewModel() )
			{
				pPlayer->GetViewModel()->m_nSkin = nSkin;
			}
		}
	}
}

void C_WeaponPortalgun::ClientThink( void )
{
	C_Portal_Player *pPlayer = ToPortalPlayer( GetOwner() );

	if ( pPlayer && dynamic_cast<C_WeaponPortalgun *>( pPlayer->GetActiveWeapon() ) && m_EffectState != EFFECT_NONE )
	{
		m_Parameters[PORTALGUN_GRAVLIGHT].SetVisibleViewModel( false );
		m_Parameters[PORTALGUN_GRAVLIGHT_WORLD].SetVisible3rdPerson( false );

		if ( GameRules()->IsMultiplayer() )
		{
			// Co-op guns show one light tinted with the last fired portal's team color
			Vector colorMagSprites = GetEffectColor( m_iLastFiredPortal );
			m_Parameters[PORTALGUN_PORTALTINTEDLIGHT].SetColor( colorMagSprites );
			m_Parameters[PORTALGUN_PORTALTINTEDLIGHT_WORLD].SetColor( colorMagSprites );
			m_Parameters[PORTALGUN_PORTALTINTEDLIGHT].SetVisibleViewModel();
			m_Parameters[PORTALGUN_PORTALTINTEDLIGHT_WORLD].SetVisible3rdPerson();

			m_Parameters[PORTALGUN_PORTAL1LIGHT].SetVisibleViewModel( false );
			m_Parameters[PORTALGUN_PORTAL1LIGHT_WORLD].SetVisible3rdPerson( false );
			m_Parameters[PORTALGUN_PORTAL2LIGHT].SetVisibleViewModel( false );
			m_Parameters[PORTALGUN_PORTAL2LIGHT_WORLD].SetVisible3rdPerson( false );
		}
		else
		{
			m_Parameters[PORTALGUN_PORTALTINTEDLIGHT].SetVisibleViewModel( false );
			m_Parameters[PORTALGUN_PORTALTINTEDLIGHT_WORLD].SetVisible3rdPerson( false );

			// Turn on and off the correct fired last lights
			m_Parameters[PORTALGUN_PORTAL1LIGHT].SetVisibleViewModel( m_iLastFiredPortal == 1 );
			m_Parameters[PORTALGUN_PORTAL1LIGHT_WORLD].SetVisible3rdPerson( m_iLastFiredPortal == 1 );
			m_Parameters[PORTALGUN_PORTAL2LIGHT].SetVisibleViewModel( m_iLastFiredPortal == 2 );
			m_Parameters[PORTALGUN_PORTAL2LIGHT_WORLD].SetVisible3rdPerson( m_iLastFiredPortal == 2 );
		}
	}

	// Update our effects
	DoEffectIdle();
}

//-----------------------------------------------------------------------------
// Purpose: The color of the last fired portal for this gun's team
//-----------------------------------------------------------------------------
Vector C_WeaponPortalgun::GetEffectColor( int iPalletIndex )
{
	Color color;

	if ( m_iLastFiredPortal == 1 )
	{
		color = UTIL_Portal_Color( 1, GetTeamNumber() );
	}
	else if ( m_iLastFiredPortal == 2 )
	{
		color = UTIL_Portal_Color( 2, GetTeamNumber() );
	}
	else
	{
		color = Color( 128, 128, 128, 255 );
	}

	Vector vColor;
	vColor.x = color.r();
	vColor.y = color.g();
	vColor.z = color.b();

	return vColor;
}

//-----------------------------------------------------------------------------
// Purpose: Plays the cleanser effect for each portal the gun just lost
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::DoCleanseEffect( bool bPortal1Active, bool bPortal2Active )
{
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	// Seen from another player: play it once on the world model
	if ( pLocalPlayer == NULL || GetOwner() != pLocalPlayer )
	{
		ParticleProp()->Create( "portal_weapon_cleanser", PATTACH_POINT_FOLLOW, "muzzle" );
		return;
	}

	C_BaseViewModel *pViewModel = pLocalPlayer->GetViewModel();
	if ( pViewModel == NULL )
		return;

	// First person: one effect per lost portal, tinted with that portal's color
	for ( int iPortal = 1; iPortal <= 2; ++iPortal )
	{
		if ( !( ( iPortal == 1 ) ? bPortal1Active : bPortal2Active ) )
			continue;

		CNewParticleEffect *pEffect = pViewModel->ParticleProp()->Create( "portal_weapon_cleanser", PATTACH_POINT_FOLLOW, "muzzle" );
		if ( pEffect )
		{
			Color color = UTIL_Portal_Color( iPortal, GetTeamNumber() );
			Vector vColor( color.r(), color.g(), color.b() );
			pEffect->SetControlPoint( 2, vColor );
		}
	}
}

extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );

//-----------------------------------------------------------------------------
// Purpose: Gets the complete list of values needed to render an effect from an
//			effect parameter
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::GetEffectParameters( EffectType_t effectID, color32 &color, float &scale, IMaterial **pMaterial, Vector &vecAttachment, bool b3rdPerson )
{
	const float dt = gpGlobals->curtime;

	// Get alpha
	float alpha = m_Parameters[effectID].GetAlpha().Interp( dt );

	// Get scale
	scale = m_Parameters[effectID].GetScale().Interp( dt );

	// Get material
	*pMaterial = (IMaterial *)m_Parameters[effectID].GetMaterial();

	// Setup the color
	color.r = (int)m_Parameters[effectID].GetColor().x;
	color.g = (int)m_Parameters[effectID].GetColor().y;
	color.b = (int)m_Parameters[effectID].GetColor().z;
	color.a = (int)alpha;

	// Setup the attachment
	int		attachment = m_Parameters[effectID].GetAttachment();
	QAngle	angles;

	// Format for first-person
	C_BasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner != NULL )
	{
		C_BaseAnimating *pModel;
		int originalModelIndex = 0;

		if ( b3rdPerson )
		{
			pModel = this;
			originalModelIndex = GetModelIndex();
			SetModelIndex( GetWorldModelIndex() );
		}
		else
		{
			pModel = pOwner->GetViewModel();
		}

		if ( pModel )
		{
			pModel->GetAttachment( attachment, vecAttachment, angles );
		}

		if ( !b3rdPerson )
		{
			::FormatViewModelAttachment( vecAttachment, true );
		}
		else
		{
			SetModelIndex( originalModelIndex );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not an effect is set to display
//-----------------------------------------------------------------------------
bool C_WeaponPortalgun::IsEffectVisible( EffectType_t effectID, bool b3rdPerson )
{
	if ( b3rdPerson )
	{
		return m_Parameters[effectID].IsVisible3rdPerson();
	}
	else
	{
		return m_Parameters[effectID].IsVisibleViewModel();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draws the effect sprite, given an effect parameter ID
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::DrawEffectSprite( EffectType_t effectID, bool b3rdPerson )
{
	color32 color;
	float scale;
	IMaterial *pMaterial;
	Vector	vecAttachment;

	// Don't draw invisible effects
	if ( !IsEffectVisible( effectID, b3rdPerson ) )
		return;

	// Get all of our parameters
	GetEffectParameters( effectID, color, scale, &pMaterial, vecAttachment, b3rdPerson );

	// Don't render fully translucent objects or sprites that were never set up
	if ( color.a <= 0.0f || pMaterial == NULL )
		return;

	// Draw the sprite
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( pMaterial, this );
	DrawSprite( vecAttachment, scale, scale, color );
}

//-----------------------------------------------------------------------------
// Purpose: Render our third-person effects
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::DrawEffects( bool b3rdPerson )
{
	for ( int i = 0; i < NUM_PORTALGUN_PARAMETERS; i++ )
	{
		DrawEffectSprite( (EffectType_t)i, b3rdPerson );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Third-person function call to render world model
//-----------------------------------------------------------------------------
int C_WeaponPortalgun::DrawModel( int flags, const RenderableInstance_t &instance )
{
	int iRetValue = BaseClass::DrawModel( flags, instance );

	if ( iRetValue )
	{
		DrawEffects( true );
	}

	return iRetValue;
}

//-----------------------------------------------------------------------------
// Purpose: First-person function call after viewmodel has been drawn
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::ViewModelDrawn( C_BaseViewModel *pBaseViewModel )
{
	// Render our effects
	DrawEffects( false );

	// Pass this back up
	BaseClass::ViewModelDrawn( pBaseViewModel );
}

//-----------------------------------------------------------------------------
// Purpose: Idle effect (pulsing)
//-----------------------------------------------------------------------------
void C_WeaponPortalgun::DoEffectIdle( void )
{
	StartEffects();

	if ( m_bPulseUp )
	{
		m_fPulse += gpGlobals->frametime;
		if ( m_fPulse > 1.0f )
		{
			m_fPulse = 1.0f;
			m_bPulseUp = !m_bPulseUp;
		}
	}
	else
	{
		m_fPulse -= gpGlobals->frametime;
		if ( m_fPulse < 0.0f )
		{
			m_fPulse = 0.0f;
			m_bPulseUp = !m_bPulseUp;
		}
	}

	// Pulse the tube beam sprites
	Vector colorMagSprites = GetEffectColor( PORTALGUN_MUZZLE_GLOW );
	for ( int i = PORTALGUN_TUBE_BEAM1; i < ( PORTALGUN_TUBE_BEAM1 + NUM_TUBE_BEAM_SPRITES ); i++ )
	{
		m_Parameters[i].GetAlpha().SetAbsolute( cl_portalgun_effects_min_alpha.GetInt() + ( cl_portalgun_effects_max_alpha.GetInt() - cl_portalgun_effects_min_alpha.GetInt() ) * m_fPulse );
		m_Parameters[i].SetColor( colorMagSprites );
	}
}
