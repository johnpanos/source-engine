//========= Portal 2 reconstruction ============================================//
//
// Purpose: Portal 2 co-op scoreboard (per-team player lists and spectators).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "portalclientscoreboard.h"
#include "c_team.h"
#include "c_playerresource.h"
#include "c_portal_player.h"
#include "portal_mp_gamerules.h"

#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/SectionedListPanel.h>
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog( IViewPort *pViewPort )
    : CClientScoreBoardDialog( pViewPort )
{
	m_pPlayerListT = new SectionedListPanel( this, "PlayerListT" );
	m_pPlayerListCT = new SectionedListPanel( this, "PlayerListCT" );

	m_pPlayerCountLabel_T = new Label( this, "T_PlayerCount", "" );
	m_pScoreLabel_T = new Label( this, "T_Score", "" );
	m_pPingLabel_T = new Label( this, "T_Latency", "" );

	m_pPlayerCountLabel_CT = new Label( this, "CT_PlayerCount", "" );
	m_pScoreLabel_CT = new Label( this, "CT_Score", "" );
	m_pPingLabel_CT = new Label( this, "CT_Latency", "" );

	ListenForGameEvent( "server_spawn" );
	SetDialogVariable( "server", "" );
	SetVisible( false );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Paint background for rounded corners
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::PaintBackground()
{
	// Reconstruction note: both 2010 builds only forward to the base class
	// here; the rounded-corner drawing that m_bgColor was loaded for is not
	// present in either binary.
	BaseClass::PaintBackground();
}

//-----------------------------------------------------------------------------
// Purpose: Paint border for rounded corners
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::PaintBorder()
{
	BaseClass::PaintBorder();
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/scoreboard.res" );

	m_bgColor = GetSchemeColor( "SectionedListPanel.BgColor", GetBgColor(), pScheme );
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color( 40, 40, 40, 160 ) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );

	if ( m_pPlayerListT )
	{
		m_pPlayerListT->SetImageList( m_pImageList, false );
		m_pPlayerListT->SetBgColor( Color( 0, 0, 0, 0 ) );
		m_pPlayerListT->SetBorder( NULL );
		m_pPlayerListT->SetVisible( true );
	}

	if ( m_pPlayerListCT )
	{
		m_pPlayerListCT->SetImageList( m_pImageList, false );
		m_pPlayerListCT->SetBgColor( Color( 0, 0, 0, 0 ) );
		m_pPlayerListCT->SetBorder( NULL );
		m_pPlayerListCT->SetVisible( true );
	}

	// turn off the default player list since we have our own
	if ( m_pPlayerList )
	{
		m_pPlayerList->SetVisible( false );
	}

	if ( m_pPlayerCountLabel_T && m_pScoreLabel_T && m_pPingLabel_T )
	{
		m_pPlayerCountLabel_T->SetFgColor( Color( 255, 64, 64, 255 ) );
		m_pScoreLabel_T->SetFgColor( Color( 255, 64, 64, 255 ) );
		m_pPingLabel_T->SetFgColor( Color( 255, 64, 64, 255 ) );
	}

	if ( m_pPlayerCountLabel_CT && m_pScoreLabel_CT && m_pPingLabel_CT )
	{
		m_pPlayerCountLabel_CT->SetFgColor( Color( 153, 204, 255, 255 ) );
		m_pScoreLabel_CT->SetFgColor( Color( 153, 204, 255, 255 ) );
		m_pPingLabel_CT->SetFgColor( Color( 153, 204, 255, 255 ) );
	}

	SetVisible( false );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Resets the scoreboard panel
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::Reset()
{
	InitPlayerList( m_pPlayerListT, TEAM_RED );
	InitPlayerList( m_pPlayerListCT, TEAM_BLUE );
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CPortalClientScoreBoardDialog::PortalPlayerSortFunc(
    vgui::SectionedListPanel *list, int itemID1, int itemID2 )
{
	KeyValues *it1 = list->GetItemData( itemID1 );
	KeyValues *it2 = list->GetItemData( itemID2 );
	Assert( it1 && it2 );

	// first compare frags
	int v1 = it1->GetInt( "frags" );
	int v2 = it2->GetInt( "frags" );
	if ( v1 > v2 )
		return true;
	else if ( v1 < v2 )
		return false;

	// next compare deaths
	v1 = it1->GetInt( "deaths" );
	v2 = it2->GetInt( "deaths" );
	if ( v1 > v2 )
		return false;
	else if ( v1 < v2 )
		return true;

	// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
	int iPlayerIndex1 = it1->GetInt( "playerIndex" );
	int iPlayerIndex2 = it2->GetInt( "playerIndex" );
	return ( iPlayerIndex1 > iPlayerIndex2 );
}

//-----------------------------------------------------------------------------
// Purpose: Inits the player list in a list panel
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::InitPlayerList(
    vgui::SectionedListPanel *pPlayerList, int teamNumber )
{
	pPlayerList->SetVerticalScrollbar( false );
	pPlayerList->RemoveAll();
	pPlayerList->RemoveAllSections();
	pPlayerList->AddSection( 0, "Players", PortalPlayerSortFunc );
	pPlayerList->SetSectionAlwaysVisible( 0, true );
	pPlayerList->SetSectionFgColor( 0, Color( 255, 255, 255, 255 ) );
	pPlayerList->SetBgColor( Color( 0, 0, 0, 0 ) );
	pPlayerList->SetBorder( NULL );

	// set the section to have the team color
	if ( teamNumber && GameResources() )
	{
		pPlayerList->SetSectionFgColor( 0, GameResources()->GetTeamColor( teamNumber ) );
	}

	pPlayerList->AddColumnToSection( 0, "avatar", "",
	    SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iAvatarWidth );
	pPlayerList->AddColumnToSection( 0, "name", "", 0, m_iNameWidth );
	pPlayerList->AddColumnToSection( 0, "class", "", 0, m_iClassWidth );
	pPlayerList->AddColumnToSection(
	    0, "frags", "", SectionedListPanel::COLUMN_RIGHT, m_iScoreWidth );
	pPlayerList->AddColumnToSection(
	    0, "deaths", "", SectionedListPanel::COLUMN_RIGHT, m_iDeathWidth );
	pPlayerList->AddColumnToSection(
	    0, "ping", "", SectionedListPanel::COLUMN_RIGHT, m_iPingWidth );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the dialog
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::Update()
{
	UpdateTeamInfo();
	UpdatePlayerList();
	UpdateSpectatorList();

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Updates information about teams
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::UpdateTeamInfo()
{
	// update the team sections in the scoreboard
	for ( int teamIndex = TEAM_RED; teamIndex <= TEAM_BLUE; teamIndex++ )
	{
		wchar_t *teamName = NULL;
		C_Team *team = GetGlobalTeam( teamIndex );
		if ( team )
		{
			// choose dialog variables to set depending on team
			const char *pDialogVarTeamScore = NULL;
			const char *pDialogVarTeamPlayerCount = NULL;
			const char *pDialogVarTeamPing = NULL;
			switch ( teamIndex )
			{
			case TEAM_RED:
				teamName = g_pVGuiLocalize->Find( "#P2_Team_Red" );
				pDialogVarTeamScore = "t_teamscore";
				pDialogVarTeamPlayerCount = "t_teamplayercount";
				pDialogVarTeamPing = "t_teamping";
				break;
			case TEAM_BLUE:
				teamName = g_pVGuiLocalize->Find( "#P2_Team_Blue" );
				pDialogVarTeamScore = "ct_teamscore";
				pDialogVarTeamPlayerCount = "ct_teamplayercount";
				pDialogVarTeamPing = "ct_teamping";
				break;
			default:
				Assert( false );
				break;
			}

			// update # of players on each team
			wchar_t name[64];
			wchar_t string1[1024];
			wchar_t wNumPlayers[6];
			V_snwprintf( wNumPlayers, ARRAYSIZE( wNumPlayers ), L"%i", team->Get_Number_Players() );
			if ( !teamName && team )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( team->Get_Name(), name, sizeof( name ) );
				teamName = name;
			}
			if ( team->Get_Number_Players() == 1 )
			{
				g_pVGuiLocalize->ConstructString( string1, sizeof( string1 ),
				    g_pVGuiLocalize->Find( "#P2_ScoreBoard_Player" ), 2, teamName, wNumPlayers );
			}
			else
			{
				g_pVGuiLocalize->ConstructString( string1, sizeof( string1 ),
				    g_pVGuiLocalize->Find( "#P2_ScoreBoard_Players" ), 2, teamName, wNumPlayers );
			}

			// set # of players for team in dialog
			SetDialogVariable( pDialogVarTeamPlayerCount, string1 );

			// set team score in dialog
			SetDialogVariable( pDialogVarTeamScore, team->Get_Score() );

			int pingsum = 0;
			int numcounted = 0;
			for ( int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++ )
			{
				if ( g_PR->IsConnected( playerIndex ) && g_PR->GetTeam( playerIndex ) == teamIndex )
				{
					int ping = g_PR->GetPing( playerIndex );

					if ( ping >= 1 )
					{
						pingsum += ping;
						numcounted++;
					}
				}
			}

			if ( numcounted > 0 )
			{
				int ping = (int)( (float)pingsum / (float)numcounted );
				SetDialogVariable( pDialogVarTeamPing, ping );
			}
			else
			{
				SetDialogVariable( pDialogVarTeamPing, "" );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the player list
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::UpdatePlayerList()
{
	m_pPlayerListT->RemoveAll();
	m_pPlayerListCT->RemoveAll();

	if ( !g_PR )
		return;

	C_Portal_Player *pLocalPlayer = C_Portal_Player::GetLocalPlayer();
	if ( !pLocalPlayer )
		return;

	for ( int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++ )
	{
		if ( g_PR->IsConnected( playerIndex ) )
		{
			SectionedListPanel *pPlayerList = NULL;
			switch ( g_PR->GetTeam( playerIndex ) )
			{
			case TEAM_RED:
				pPlayerList = m_pPlayerListT;
				break;
			case TEAM_BLUE:
				pPlayerList = m_pPlayerListCT;
				break;
			}
			if ( pPlayerList == NULL )
			{
				continue;
			}

			KeyValues *pKeyValues = new KeyValues( "data" );
			GetPlayerScoreInfo( playerIndex, pKeyValues );

			int itemID = pPlayerList->AddItem( 0, pKeyValues );
			Color clr = g_PR->GetTeamColor( g_PR->GetTeam( playerIndex ) );
			pPlayerList->SetItemFgColor( itemID, clr );

			pKeyValues->deleteThis();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the spectator list
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::UpdateSpectatorList()
{
	char szSpectatorList[512] = "";
	int nSpectators = 0;
	for ( int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++ )
	{
		if ( ShouldShowAsSpectator( playerIndex ) )
		{
			if ( nSpectators > 0 )
			{
				Q_strncat( szSpectatorList, ", ", ARRAYSIZE( szSpectatorList ) );
			}

			Q_strncat(
			    szSpectatorList, g_PR->GetPlayerName( playerIndex ), ARRAYSIZE( szSpectatorList ) );
			nSpectators++;
		}
	}

	wchar_t wzSpectators[512] = L"";
	if ( nSpectators > 0 )
	{
		const char *pchFormat =
		    ( 1 == nSpectators ? "#ScoreBoard_Spectator" : "#ScoreBoard_Spectators" );

		wchar_t wzSpectatorCount[16];
		wchar_t wzSpectatorList[1024];
		V_snwprintf( wzSpectatorCount, ARRAYSIZE( wzSpectatorCount ), L"%i", nSpectators );
		g_pVGuiLocalize->ConvertANSIToUnicode(
		    szSpectatorList, wzSpectatorList, sizeof( wzSpectatorList ) );
		g_pVGuiLocalize->ConstructString( wzSpectators, sizeof( wzSpectators ),
		    g_pVGuiLocalize->Find( pchFormat ), 2, wzSpectatorCount, wzSpectatorList );
	}
	SetDialogVariable( "spectators", wzSpectators );
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether the specified player index is a spectator
//-----------------------------------------------------------------------------
bool CPortalClientScoreBoardDialog::ShouldShowAsSpectator( int iPlayerIndex )
{
	// see if player is connected
	if ( g_PR && g_PR->IsConnected( iPlayerIndex ) )
	{
		// spectators show in spectator list
		int iTeam = g_PR->GetTeam( iPlayerIndex );
		if ( TEAM_SPECTATOR == iTeam || TEAM_UNASSIGNED == iTeam )
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Event handler
//-----------------------------------------------------------------------------
void CPortalClientScoreBoardDialog::FireGameEvent( IGameEvent *event )
{
	const char *type = event->GetName();

	if ( 0 == Q_strcmp( type, "server_spawn" ) )
	{
		// set server name in scoreboard
		const char *hostname = event->GetString( "hostname" );
		wchar_t wzHostName[256];
		wchar_t wzServerLabel[256];
		g_pVGuiLocalize->ConvertANSIToUnicode( hostname, wzHostName, sizeof( wzHostName ) );
		g_pVGuiLocalize->ConstructString( wzServerLabel, sizeof( wzServerLabel ),
		    g_pVGuiLocalize->Find( "#Scoreboard_Server" ), 1, wzHostName );
		SetDialogVariable( "server", wzServerLabel );
	}

	if ( IsVisible() )
	{
		Update();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CPortalClientScoreBoardDialog::GetPlayerScoreInfo( int playerIndex, KeyValues *kv )
{
	// Clean up the player name
	const char *oldName = g_PR->GetPlayerName( playerIndex );
	int bufsize = strlen( oldName ) * 2 + 1;
	char *newName = (char *)_alloca( bufsize );
	UTIL_MakeSafeName( oldName, newName, bufsize );
	kv->SetString( "name", newName );

	kv->SetInt( "playerIndex", playerIndex );
	kv->SetInt( "frags", g_PR->GetPlayerScore( playerIndex ) );
	kv->SetInt( "deaths", g_PR->GetDeaths( playerIndex ) );
	kv->SetString( "class", "" );

	UpdatePlayerAvatar( playerIndex, kv );

	if ( g_PR->GetPing( playerIndex ) < 1 )
	{
		if ( g_PR->IsFakePlayer( playerIndex ) )
		{
			kv->SetString( "ping", "BOT" );
		}
		else
		{
			kv->SetString( "ping", "" );
		}
	}
	else
	{
		kv->SetInt( "ping", g_PR->GetPing( playerIndex ) );
	}

	// setup the tracker column
	C_Portal_Player *me = C_Portal_Player::GetLocalPlayer();
	if ( !g_PR || !me )
		return true;

	if ( g_PR->IsHLTV( playerIndex ) )
	{
		// show #spectators in class field
		char numspecs[32];
		Q_snprintf( numspecs, sizeof( numspecs ), "%i Spectators", m_HLTVSpectators );
		kv->SetString( "class", numspecs );
	}
	else if ( !g_PR->IsAlive( playerIndex ) && g_PR->GetTeam( playerIndex ) > TEAM_SPECTATOR )
	{
		kv->SetString( "class", "#P2_Scoreboard_Dead" );
	}

	return true;
}
