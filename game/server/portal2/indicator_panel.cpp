//========= Portal 2 reconstruction ============================================//
//
// Purpose: Test chamber indicator panels, countdown timers and tic-tac-toe panels
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "indicator_panel.h"
#include "vguiscreen.h"
#include "igameevents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CLabIndicatorPanel )

	DEFINE_FIELD( m_bEnabled, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flTimerDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_bIsCountdownTimer, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bIsChecked, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iPlayerPinged, FIELD_INTEGER ),
	DEFINE_FIELD( m_flTimerStart, FIELD_TIME ),
	DEFINE_FIELD( m_bStopped, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bCountingDown, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hScreen, FIELD_EHANDLE ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CLabIndicatorPanel, DT_LabIndicatorPanel )
	SendPropBool( SENDINFO( m_bEnabled ) ),
	SendPropBool( SENDINFO( m_bStopped ) ),
	SendPropBool( SENDINFO( m_bCountingDown ) ),
	SendPropBool( SENDINFO( m_bIsCountdownTimer ) ),
	SendPropBool( SENDINFO( m_bIsChecked ) ),
	SendPropInt( SENDINFO( m_iPlayerPinged ) ),
	SendPropFloat( SENDINFO( m_flTimerDuration ) ),
	SendPropFloat( SENDINFO( m_flTimerStart ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( vgui_indicator_panel, CLabIndicatorPanel );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CLabIndicatorPanel::~CLabIndicatorPanel()
{
	if ( m_hScreen != NULL )
	{
		DestroyVGuiScreen( m_hScreen.Get() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: The panel itself must be sent so its screen can find it
//-----------------------------------------------------------------------------
int CLabIndicatorPanel::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_FULLCHECK );
}

void CLabIndicatorPanel::SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways )
{
	// Are we already marked for transmission?
	if ( pInfo->m_pTransmitEdict->Get( entindex() ) )
		return;

	BaseClass::SetTransmit( pInfo, bAlways );

	// Force our screen to be sent too.
	if ( m_hScreen != NULL )
	{
		m_hScreen->SetTransmit( pInfo, bAlways );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SpawnControlPanels();

	m_bEnabled = true;
	ScreenVisible( m_bEnabled );

	// Start out idle
	m_bCountingDown = false;
	m_bStopped = false;
	m_flTimerStart = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::Precache( void )
{
	PrecacheVGuiScreen( "indicator_panel" );
}

//-----------------------------------------------------------------------------
// Purpose: Rebuild our screen after a restore
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::OnRestore( void )
{
	BaseClass::OnRestore();

	RestoreControlPanels();

	ScreenVisible( m_bEnabled );
}

//-----------------------------------------------------------------------------
// Purpose: Show or hide the vgui screen
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::ScreenVisible( bool bVisible )
{
	if ( m_hScreen != NULL )
	{
		if ( bVisible )
		{
			m_hScreen->RemoveEffects( EF_NODRAW );
		}
		else
		{
			m_hScreen->AddEffects( EF_NODRAW );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::Disable( void )
{
	if ( !m_bEnabled )
		return;

	m_bEnabled = false;

	ScreenVisible( false );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::Enable( void )
{
	if ( m_bEnabled )
		return;

	m_bEnabled = true;

	ScreenVisible( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::InputDisable( inputdata_t &inputdata )
{
	Disable();
}

void CLabIndicatorPanel::InputEnable( inputdata_t &inputdata )
{
	Enable();
}

void CLabIndicatorPanel::GetControlPanelInfo( int nPanelIndex, const char *&pPanelName )
{
	pPanelName = "indicator_panel";
}

void CLabIndicatorPanel::GetControlPanelClassName( int nPanelIndex, const char *&pPanelName )
{
	pPanelName = "vgui_screen";
}

//-----------------------------------------------------------------------------
// This is called by the base object when it's time to spawn the control panels
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::SpawnControlPanels()
{
	// Our screen type and entity class are fixed, so the lookups are
	// resolved here rather than through GetControlPanelInfo.
	const char *pScreenName = "indicator_panel";
	const char *pScreenClassname = "vgui_screen";

	// The screen is attached directly to us
	CVGuiScreen *pScreen = CreateVGuiScreen( pScreenClassname, pScreenName, this, this, -1 );
	pScreen->ChangeTeam( GetTeamNumber() );
	pScreen->SetActualSize( 32, 32 );
	pScreen->SetActive( true );
	pScreen->MakeVisibleOnlyToTeammates( false );
	pScreen->SetTransparency( true );

	m_hScreen = pScreen;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::RestoreControlPanels( void )
{
	if ( m_hScreen )
	{
		UTIL_RemoveImmediate( m_hScreen );
	}

	SpawnControlPanels();
}

//-----------------------------------------------------------------------------
// Purpose: Begin a countdown of the given length
//-----------------------------------------------------------------------------
void CLabIndicatorPanel::StartTimer( float flDuration )
{
	m_flTimerStart = gpGlobals->curtime;
	m_bStopped = false;
	m_bCountingDown = true;
	m_bIsCountdownTimer = true;
	m_flTimerDuration = flDuration;
}

void CLabIndicatorPanel::ResetTimer( void )
{
	m_bStopped = true;
	m_bCountingDown = false;
	m_bIsCountdownTimer = true;
}

void CLabIndicatorPanel::SetChecked( bool bIsChecked )
{
	m_bIsChecked = bIsChecked;
	m_bIsCountdownTimer = false;
}

void CLabIndicatorPanel::SetPlayerPinged( int iPlayerPinged )
{
	m_iPlayerPinged = iPlayerPinged;
	m_bIsCountdownTimer = false;
}

void CLabIndicatorPanel::SetIsTimer( bool bIsTimer )
{
	m_bIsCountdownTimer = bIsTimer;
}

//=============================================================================
//
// Indicator panel prop
//
//=============================================================================

BEGIN_DATADESC( CPropIndicatorPanel )

	DEFINE_KEYFIELD( m_bEnabled, FIELD_BOOLEAN, "Enabled" ),
	DEFINE_KEYFIELD( m_flTimerDuration, FIELD_FLOAT, "TimerDuration" ),
	DEFINE_KEYFIELD( m_bIsCountdownTimer, FIELD_BOOLEAN, "IsTimer" ),
	DEFINE_KEYFIELD( m_bIsChecked, FIELD_BOOLEAN, "IsChecked" ),
	DEFINE_KEYFIELD( m_strIndicatorLights, FIELD_STRING, "IndicatorLights" ),

	DEFINE_FIELD( m_flTimerStart, FIELD_TIME ),
	DEFINE_FIELD( m_bStopped, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bCountingDown, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hIndicatorPanel, FIELD_EHANDLE ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Check", InputCheck ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Uncheck", InputUncheck ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Start", InputStart ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Stop", InputStop ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Reset", InputReset ),

	DEFINE_THINKFUNC( ToggleThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( prop_indicator_panel, CPropIndicatorPanel );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::Precache( void )
{
	UTIL_PrecacheOther( "vgui_indicator_panel" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::Spawn( void )
{
	Precache();

	AddSolidFlags( FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	CreateIndicatorPanel();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::UpdateOnRemove( void )
{
	UTIL_Remove( m_hIndicatorPanel );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::SetTimerDuration( float flDuration )
{
	m_flTimerDuration = flDuration;
}

//-----------------------------------------------------------------------------
// Purpose: Start the panel counting down and light the indicator lights
//			until the countdown finishes
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::StartTimer( void )
{
	if ( m_hIndicatorPanel )
	{
		m_hIndicatorPanel->StartTimer( m_flTimerDuration );

		// Turn the lights on until we've finished
		ToggleIndicatorLights( true );

		SetThink( &CPropIndicatorPanel::ToggleThink );
		SetNextThink( gpGlobals->curtime + m_flTimerDuration );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::StopTimer( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::ResetTimer( void )
{
	if ( m_hIndicatorPanel )
	{
		m_hIndicatorPanel->ResetTimer();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::InputStart( inputdata_t &indata )
{
	StartTimer();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::InputStop( inputdata_t &indata )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::InputReset( inputdata_t &indata )
{
	ResetTimer();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::InputCheck( inputdata_t &indata )
{
	if ( m_hIndicatorPanel )
	{
		m_hIndicatorPanel->SetChecked( true );
	}

	// Cancel any timer shut-off and light up
	SetThink( NULL );
	ToggleIndicatorLights( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::InputUncheck( inputdata_t &indata )
{
	if ( m_hIndicatorPanel )
	{
		m_hIndicatorPanel->SetChecked( false );
	}

	SetThink( NULL );
	ToggleIndicatorLights( false );
}

//-----------------------------------------------------------------------------
// Purpose: Create and position the networked panel entity
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::CreateIndicatorPanel( void )
{
	m_hIndicatorPanel = static_cast<CLabIndicatorPanel *>( CreateEntityByName( "vgui_indicator_panel" ) );

	if ( m_hIndicatorPanel == NULL )
		return;

	// Orient the screen so it faces out along our forward axis
	QAngle angles = GetAbsAngles();

	VMatrix mat, rotation, tmp;
	MatrixFromAngles( angles, mat );
	MatrixBuildRotationAboutAxis( rotation, Vector( 0, 1, 0 ), 90 );
	MatrixMultiply( mat, rotation, tmp );
	MatrixBuildRotateZ( rotation, 90 );
	MatrixMultiply( tmp, rotation, mat );
	MatrixToAngles( mat, angles );
	m_hIndicatorPanel->SetAbsAngles( angles );

	Vector vecForward, vecRight, vecUp;
	GetVectors( &vecForward, &vecRight, &vecUp );

	// Center the 32x32 screen on our origin, slightly in front of us
	// Reconstruction note: the binary evaluates ( right * 16 - forward ) + up * -16.
	Vector vecOffset = ( vecRight * 16.0f - vecForward ) + vecUp * -16.0f;
	m_hIndicatorPanel->SetAbsOrigin( GetAbsOrigin() + vecOffset );

	// Follow us around
	m_hIndicatorPanel->SetParent( this );

	// Initialize the panel with our keyvalue state
	m_hIndicatorPanel->SetPlayerPinged( -1 );
	m_hIndicatorPanel->SetChecked( m_bIsChecked );
	m_hIndicatorPanel->SetIsTimer( m_bIsCountdownTimer );

	DispatchSpawn( m_hIndicatorPanel );
}

//-----------------------------------------------------------------------------
// Purpose: Set the texture frame of every named indicator light
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::ToggleIndicatorLights( bool bLightsOn )
{
	CBaseEntity *pLight = NULL;
	while ( ( pLight = gEntList.FindEntityByName( pLight, STRING( m_strIndicatorLights ) ) ) != NULL )
	{
		pLight->SetTextureFrameIndex( bLightsOn );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Countdown finished, turn the lights off
//-----------------------------------------------------------------------------
void CPropIndicatorPanel::ToggleThink( void )
{
	ToggleIndicatorLights( false );

	SetThink( NULL );
}

//=============================================================================
//
// Tic-tac-toe panel
//
//=============================================================================

BEGIN_DATADESC( CPropTicTacToePanel )

	DEFINE_FIELD( m_bPlayed, FIELD_BOOLEAN ),

	DEFINE_OUTPUT( m_OnPlayer1Pinged, "OnPlayer1Pinged" ),
	DEFINE_OUTPUT( m_OnPlayer2Pinged, "OnPlayer2Pinged" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( prop_tic_tac_toe_panel, CPropTicTacToePanel );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropTicTacToePanel::Spawn( void )
{
	BaseClass::Spawn();

	ListenForGameEvent( "portal_player_ping" );

	// Nobody has claimed this square yet
	m_hIndicatorPanel->SetPlayerPinged( 0 );

	m_bPlayed = false;
}

//-----------------------------------------------------------------------------
// Purpose: Claim the square for whichever player pings it first
//-----------------------------------------------------------------------------
void CPropTicTacToePanel::FireGameEvent( IGameEvent *event )
{
	if ( m_bPlayed )
		return;

	const char *name = event->GetName();
	if ( !Q_strcmp( name, "portal_player_ping" ) )
	{
		int playerId = event->GetInt( "userid" );
		float flPingX = event->GetFloat( "ping_x" );
		float flPingY = event->GetFloat( "ping_y" );
		float flPingZ = event->GetFloat( "ping_z" );

		// The first connected player is player one, everyone else is player two
		for ( int i = 1; i <= MAX_PLAYERS; ++i )
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
			if ( pPlayer == NULL )
				continue;

			playerId = ( pPlayer->GetUserID() == playerId ) ? 1 : 2;
			break;
		}

		Vector vecMin, vecMax;
		CollisionProp()->WorldSpaceAABB( &vecMin, &vecMax );

		// Our bounds are in the panel's unrotated space, swap them into its facing
		vecMin -= GetAbsOrigin();

		float temp = vecMin.x;
		vecMin.x = vecMin.y;
		vecMin.y = temp;

		vecMax = GetAbsOrigin() - vecMin;
		vecMin += GetAbsOrigin();

		if ( CalcSqrDistanceToAABB( vecMin, vecMax, Vector( flPingX, flPingY, flPingZ ) ) < 2.0f )
		{
			m_hIndicatorPanel->SetPlayerPinged( playerId );
			m_hIndicatorPanel->ScreenVisible( true );

			if ( playerId == 1 )
			{
				m_OnPlayer1Pinged.FireOutput( NULL, NULL );
			}
			else
			{
				m_OnPlayer2Pinged.FireOutput( NULL, NULL );
			}
		}
	}
}
