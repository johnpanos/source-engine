//========= Portal 2 reconstruction ============================================//
//
// Purpose: logic_playerproxy for Portal 2: map access to the single player.
//
// Written for the Portal 2 port from map consumers and the retail Portal 2
// server's CLogicPlayerProxy datadesc names (no original source or pseudocode
// exists). The Half-Life 2 proxy in hl2_player.cpp is not built for Portal 2.
// Not original Valve source; the repository's provenance and distribution
// warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal2/portal/portal_player.h"
#include "portal2/portal/weapon_portalgun.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CLogicPlayerProxy : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicPlayerProxy, CLogicalEntity );
	DECLARE_DATADESC();

	virtual void Activate( void );

private:
	CPortal_Player *GetPlayer( void ) const;

	void InputRequestPlayerHealth( inputdata_t &inputdata );
	void InputSetPlayerHealth( inputdata_t &inputdata );
	void InputSetDropEnabled( inputdata_t &inputdata );
	void InputForceVMGrabController( inputdata_t &inputdata );
	void InputForcePhysicsGrabController( inputdata_t &inputdata );
	void InputResetGrabControllerBehavior( inputdata_t &inputdata );
	void InputSetMotionBlurAmount( inputdata_t &inputdata );
	void InputAddPotatosToPortalgun( inputdata_t &inputdata );
	void InputRemovePotatosFromPortalgun( inputdata_t &inputdata );

	void SetPotatos( bool bShowPotatos );

	// Fired by name through CPortal_Player::FirePlayerProxyOutput
	COutputInt m_RequestedPlayerHealth;
	COutputEvent m_PlayerDied;
	COutputEvent m_OnJump;
	COutputEvent m_OnDuck;
	COutputEvent m_OnUnDuck;
	COutputEvent m_OnStartSlowingTime;
	COutputEvent m_OnStopSlowingTime;
	COutputEvent m_OnPrimaryPortalPlaced;
	COutputEvent m_OnSecondaryPortalPlaced;
	COutputEvent m_OnCoopPing;
};

// Source datadesc macros require their declaration layout.
// clang-format off
BEGIN_DATADESC( CLogicPlayerProxy )
	DEFINE_OUTPUT( m_RequestedPlayerHealth, "PlayerHealth" ),
	DEFINE_OUTPUT( m_PlayerDied, "PlayerDied" ),
	DEFINE_OUTPUT( m_OnJump, "OnJump" ),
	DEFINE_OUTPUT( m_OnDuck, "OnDuck" ),
	DEFINE_OUTPUT( m_OnUnDuck, "OnUnDuck" ),
	DEFINE_OUTPUT( m_OnStartSlowingTime, "OnStartSlowingTime" ),
	DEFINE_OUTPUT( m_OnStopSlowingTime, "OnStopSlowingTime" ),
	DEFINE_OUTPUT( m_OnPrimaryPortalPlaced, "OnPrimaryPortalPlaced" ),
	DEFINE_OUTPUT( m_OnSecondaryPortalPlaced, "OnSecondaryPortalPlaced" ),
	DEFINE_OUTPUT( m_OnCoopPing, "OnCoopPing" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "RequestPlayerHealth", InputRequestPlayerHealth ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetPlayerHealth", InputSetPlayerHealth ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetDropEnabled", InputSetDropEnabled ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForceVMGrabController", InputForceVMGrabController ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForcePhysicsGrabController", InputForcePhysicsGrabController ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ResetGrabControllerBehavior", InputResetGrabControllerBehavior ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetMotionBlurAmount", InputSetMotionBlurAmount ),
	DEFINE_INPUTFUNC( FIELD_VOID, "AddPotatosToPortalgun", InputAddPotatosToPortalgun ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemovePotatosFromPortalgun", InputRemovePotatosFromPortalgun ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( logic_playerproxy, CLogicPlayerProxy );
// clang-format on

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::Activate( void )
{
	BaseClass::Activate();

	if ( g_pGameRules->IsMultiplayer() )
	{
		Warning( "Can't use logic player proxy in multiplayer!\n" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: The player is looked up per input; it may not exist yet at Activate
//			and is a new entity after a level transition.
//-----------------------------------------------------------------------------
CPortal_Player *CLogicPlayerProxy::GetPlayer( void ) const
{
	return ToPortalPlayer( UTIL_GetLocalPlayer() );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputRequestPlayerHealth( inputdata_t &inputdata )
{
	CPortal_Player *pPlayer = GetPlayer();
	if ( pPlayer )
	{
		m_RequestedPlayerHealth.Set(
		    pPlayer->GetHealth(), inputdata.pActivator, inputdata.pCaller );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputSetPlayerHealth( inputdata_t &inputdata )
{
	CPortal_Player *pPlayer = GetPlayer();
	if ( pPlayer )
	{
		pPlayer->SetHealth( inputdata.value.Int() );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputSetDropEnabled( inputdata_t &inputdata )
{
	CPortal_Player *pPlayer = GetPlayer();
	if ( pPlayer )
	{
		pPlayer->SetDropEnabled( inputdata.value.Bool() );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputForceVMGrabController( inputdata_t &inputdata )
{
	CPortal_Player *pPlayer = GetPlayer();
	if ( pPlayer )
	{
		pPlayer->SetForcedGrabControllerType( FORCE_GRAB_CONTROLLER_VM );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputForcePhysicsGrabController( inputdata_t &inputdata )
{
	CPortal_Player *pPlayer = GetPlayer();
	if ( pPlayer )
	{
		pPlayer->SetForcedGrabControllerType( FORCE_GRAB_CONTROLLER_PHYSICS );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputResetGrabControllerBehavior( inputdata_t &inputdata )
{
	CPortal_Player *pPlayer = GetPlayer();
	if ( pPlayer )
	{
		pPlayer->SetForcedGrabControllerType( FORCE_GRAB_CONTROLLER_DEFAULT );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputSetMotionBlurAmount( inputdata_t &inputdata )
{
	CPortal_Player *pPlayer = GetPlayer();
	if ( pPlayer )
	{
		pPlayer->SetMotionBlurAmount( inputdata.value.Float() );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputAddPotatosToPortalgun( inputdata_t &inputdata )
{
	SetPotatos( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::InputRemovePotatosFromPortalgun( inputdata_t &inputdata )
{
	SetPotatos( false );
}

//-----------------------------------------------------------------------------
// Purpose: Puts PotatOS on the held portal gun or takes it off. As in the
//          retail server.so, only the active gun changes; the player's
//          PotatOS light (TurnOnPotatos/TurnOffPotatos) is left to scripts.
//-----------------------------------------------------------------------------
void CLogicPlayerProxy::SetPotatos( bool bShowPotatos )
{
	if ( GameRules()->IsMultiplayer() )
	{
		Warning( "Can't use logic player proxy in multiplayer!\n" );
		return;
	}

	CPortal_Player *pPlayer = GetPlayer();
	if ( !pPlayer )
		return;

	CWeaponPortalgun *pPortalGun = dynamic_cast<CWeaponPortalgun *>( pPlayer->GetActiveWeapon() );
	if ( pPortalGun )
	{
		pPortalGun->SetPotatosOnPortalgun( bShowPotatos );
	}
}
