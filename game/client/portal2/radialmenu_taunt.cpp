//========= Portal 2 reconstruction ============================================//
//
// Purpose: Gesture (taunt) wheel menu manager and per-taunt ownership state
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include "vgui/ISurface.h"
#include "c_portal_player.h"
#include "c_keyvalue_saver.h"
#include "radialmenu_taunt.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Reconstruction note: the 2010 build spells the save file as a literal; the retail
// game rules write it through PORTAL2_MP_SAVE_FILE.
#ifndef PORTAL2_MP_SAVE_FILE
#define PORTAL2_MP_SAVE_FILE "coop_data.txt"
#endif

#define NUM_TAUNT_POSITIONS 8

char g_szPositions[NUM_TAUNT_POSITIONS][16] = {
    "North", "South", "West", "East", "NorthWest", "NorthEast", "SouthWest", "SouthEast" };

// This branch's engine has no split-screen client implementation (see
// CBaseClient::IsSplitScreenUser). Keep one manager for its supported local
// player instead of indexing an unavailable active-slot API.
static ClientMenuManagerTaunt TheClientMenuManagerTaunt;

static void BuildTauntSaveData( KeyValues *pRoot )
{
	if ( KeyValues *pPreviousTaunts = pRoot->FindKey( "taunts" ) )
	{
		pRoot->RemoveSubKey( pPreviousTaunts );
		pPreviousTaunts->deleteThis();
	}

	KeyValues *pTaunts = new KeyValues( "taunts" );
	GetClientMenuManagerTaunt().KeyValueBuilder( pTaunts );
	pRoot->AddSubKey( pTaunts );
}

ClientMenuManagerTaunt &GetClientMenuManagerTaunt( int nSlot )
{
	if ( nSlot > 0 )
	{
		Error(
		    "Portal 2 taunt manager: split-screen slot %d is unsupported by this engine\n", nSlot );
	}

	return TheClientMenuManagerTaunt;
}

void ClientMenuManagerTaunt::ClearTauntStatusData( void )
{
	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pNew = &m_TauntData[i];
		if ( pNew->nTextureID != -1 )
		{
			vgui::surface()->DestroyTextureID( pNew->nTextureID );
			pNew->nTextureID = -1;
		}
	}

	m_TauntData.RemoveAll();
}

KeyValues *ClientMenuManagerTaunt::FindMenu( const char *menuName )
{
	UpdateDefaultTaunts();

	return m_customMenuKeys->FindKey( menuName );
}

void ClientMenuManagerTaunt::Flush( void )
{
	Reset();
	AddMenuFile( "scripts/RadialMenuTaunt.txt" );
	KeyValueSaver().InitKeyValues( PORTAL2_MP_SAVE_FILE, &BuildTauntSaveData );

	ClearTauntStatusData();

	KeyValues *pKVData = m_menuKeys->GetFirstTrueSubKey();
	while ( pKVData )
	{
		int nNew = m_TauntData.AddToTail();
		TauntStatusData *pNew = &m_TauntData[nNew];

		V_strncpy( pNew->szName, pKVData->GetName(), sizeof( pNew->szName ) );
		V_strncpy(
		    pNew->szCommand, pKVData->GetString( "command", "" ), sizeof( pNew->szCommand ) );
		V_strncpy( pNew->szIcon, pKVData->GetString( "icon", "" ), sizeof( pNew->szIcon ) );
		pNew->bTeamTaunt = pKVData->GetBool( "team" );
		pNew->nTextureID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(
		    pNew->nTextureID, VarArgs( "vgui/%s", pNew->szIcon ), true, false );

		V_strncpy( pNew->szPosition, "none", sizeof( pNew->szPosition ) );
		pNew->bUnlocked = false;
		pNew->bOwned = false;
		pNew->bUsed = false;
		pNew->nCompletionDelay = 0;

		pKVData = pKVData->GetNextTrueSubKey();
	}
}

void ClientMenuManagerTaunt::KeyValueBuilder( KeyValues *pKeyValues )
{
	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pTauntData = &m_TauntData[i];

		KeyValues *pNewTaunt = new KeyValues( pTauntData->szName );
		pNewTaunt->SetInt( "unlocked", pTauntData->bUnlocked );
		pNewTaunt->SetInt( "owned", pTauntData->bOwned );
		pNewTaunt->SetInt( "used", pTauntData->bUsed );
		pNewTaunt->SetInt( "delay", pTauntData->nCompletionDelay );
		pNewTaunt->SetString( "position", pTauntData->szPosition );
		pKeyValues->AddSubKey( pNewTaunt );
	}
}

void ClientMenuManagerTaunt::KeyValueProcessor( IPlayerLocal *pPlayer )
{
	// Reconstruction note: this is the "taunts" block of the 2010
	// C_PortalMPGameRules::LoadMapCompleteData, which read back what KeyValueBuilder
	// saved. The retail caller passes the signed-in player, whose title-data layout is
	// not recoverable from the 2010 binaries, so the player only gates the load here.
	if ( !pPlayer )
		return;

	KeyValues *pKVData = KeyValueSaver().GetKeyValues( PORTAL2_MP_SAVE_FILE );
	if ( !pKVData )
		return;

	KeyValues *pTaunts = pKVData->FindKey( "taunts" );
	if ( !pTaunts )
		return;

	for ( KeyValues *pTaunt = pTaunts->GetFirstTrueSubKey(); pTaunt;
	    pTaunt = pTaunt->GetNextTrueSubKey() )
	{
		if ( pTaunt->GetInt( "unlocked" ) != 0 )
		{
			SetTauntUnlocked( pTaunt->GetName() );
		}

		if ( pTaunt->GetInt( "owned" ) != 0 )
		{
			SetTauntOwned( pTaunt->GetName() );
		}

		if ( pTaunt->GetInt( "used" ) != 0 )
		{
			SetTauntUsed( pTaunt->GetName() );
		}

		int nDelay = pTaunt->GetInt( "delay" );
		if ( nDelay != 0 )
		{
			SetTauntDelay( pTaunt->GetName(), nDelay );
		}

		SetTauntPosition( pTaunt->GetName(), pTaunt->GetString( "position", "none" ) );
	}

	engine->ClientCmd( VarArgs( "taunts_pending %i", GetNumTauntsPending() ) );
}

const char *ClientMenuManagerTaunt::GetNextTauntToEarn( void )
{
	TauntStatusData *pNext = NULL;
	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pTauntData = &m_TauntData[i];

		// Pending taunts only, preferring team taunts
		if ( pTauntData->bUnlocked && !pTauntData->bOwned && pTauntData->nCompletionDelay == 0 &&
		     ( !pNext || ( !pNext->bTeamTaunt && pTauntData->bTeamTaunt ) ) )
		{
			pNext = pTauntData;
		}
	}

	if ( !pNext )
		return NULL;

	return pNext->szName;
}

bool ClientMenuManagerTaunt::IsTauntTeam( const char *pchTaunt )
{
	TauntStatusData *pTauntData = GetTauntDataByName( pchTaunt );
	if ( !pTauntData )
		return false;

	return pTauntData->bTeamTaunt;
}

void ClientMenuManagerTaunt::SetTauntUnlocked( const char *pchTaunt )
{
	TauntStatusData *pTauntData = GetTauntDataByName( pchTaunt );
	if ( !pTauntData )
		return;

	if ( pTauntData->bUnlocked )
		return;

	pTauntData->bUnlocked = true;

	if ( pTauntData->bTeamTaunt )
	{
		// Team taunts are earned together, so they're pending right away
		pTauntData->nCompletionDelay = 0;
		engine->ClientCmd( VarArgs( "taunts_pending %i", GetNumTauntsPending() ) );
	}
	else
	{
		// Solo taunts become owned a few completions after the last one in line
		int nLargestDelay = 0;
		for ( int j = 0; j < m_TauntData.Count(); ++j )
		{
			TauntStatusData *pOtherTauntData = &m_TauntData[j];
			if ( pOtherTauntData != pTauntData && pOtherTauntData->bUnlocked &&
			     !pOtherTauntData->bOwned )
			{
				nLargestDelay = MAX( nLargestDelay, pOtherTauntData->nCompletionDelay + 3 );
			}
		}

		pTauntData->nCompletionDelay = nLargestDelay;
	}

	KeyValueSaver().MarkKeyValuesDirty( PORTAL2_MP_SAVE_FILE );
	m_bLoadedSaveData = true;
}

void ClientMenuManagerTaunt::SetTauntOwned( const char *pchTaunt )
{
	TauntStatusData *pTauntData = GetTauntDataByName( pchTaunt );
	if ( !pTauntData )
		return;

	if ( pTauntData->bOwned )
		return;

	pTauntData->bOwned = true;

	KeyValueSaver().MarkKeyValuesDirty( PORTAL2_MP_SAVE_FILE );
	m_bLoadedSaveData = true;
}

void ClientMenuManagerTaunt::SetTauntOwned( const char *pchTaunt, bool bAwardSilently )
{
	// Reconstruction note: the retail MPTauntEarned handler awards a taunt with this one
	// call. The steps are the 2010 handler's: "new" earns the next pending taunt, then it
	// is unlocked, owned and put in an empty wheel slot. A silent award is marked used so
	// the "taunt earned" notice (which lists owned, unused taunts) skips it.
	char szTaunt[32];
	V_strncpy( szTaunt, pchTaunt, sizeof( szTaunt ) );

	if ( V_strcmp( szTaunt, "new" ) == 0 )
	{
		const char *pchNext = GetNextTauntToEarn();
		if ( !pchNext )
			return;

		V_strncpy( szTaunt, pchNext, sizeof( szTaunt ) );
	}

	SetTauntUnlocked( szTaunt );
	SetTauntOwned( szTaunt );
	SetTauntPosition( szTaunt, "empty" );

	if ( bAwardSilently )
	{
		SetTauntUsed( szTaunt );
	}
}

void ClientMenuManagerTaunt::SetTauntUsed( const char *pchTaunt )
{
	TauntStatusData *pTauntData = GetTauntDataByName( pchTaunt );
	if ( !pTauntData )
		return;

	if ( pTauntData->bUsed )
		return;

	m_bMenuStale = true;
	pTauntData->bUsed = true;

	KeyValueSaver().MarkKeyValuesDirty( PORTAL2_MP_SAVE_FILE );
	m_bLoadedSaveData = true;
}

void ClientMenuManagerTaunt::SetTauntDelay( const char *pchTaunt, int nDelay )
{
	TauntStatusData *pTauntData = GetTauntDataByName( pchTaunt );
	if ( !pTauntData )
		return;

	if ( pTauntData->nCompletionDelay == nDelay )
		return;

	pTauntData->nCompletionDelay = nDelay;

	KeyValueSaver().MarkKeyValuesDirty( PORTAL2_MP_SAVE_FILE );
	m_bLoadedSaveData = true;
}

void ClientMenuManagerTaunt::SetTauntPosition( const char *pchTaunt, const char *pchPosition )
{
	TauntStatusData *pTauntData = GetTauntDataByName( pchTaunt );
	if ( !pTauntData )
		return;

	bool bMoveToEmpty = ( V_strcmp( pchPosition, "empty" ) == 0 );

	if ( V_strcmp( pTauntData->szPosition, pchPosition ) == 0 )
		return;

	if ( bMoveToEmpty )
	{
		// Only taunts that aren't on the wheel yet get moved into an empty slot
		if ( V_strcmp( pTauntData->szPosition, "none" ) != 0 )
			return;

		pchPosition = "none";

		for ( int nPos = 0; nPos < NUM_TAUNT_POSITIONS; ++nPos )
		{
			bool bNotTaken = true;
			for ( int j = 0; j < m_TauntData.Count(); ++j )
			{
				if ( V_strcmp( m_TauntData[j].szPosition, g_szPositions[nPos] ) == 0 )
				{
					bNotTaken = false;
					break;
				}
			}

			if ( bNotTaken )
			{
				pchPosition = g_szPositions[nPos];
				break;
			}
		}
	}
	else
	{
		// Bump whatever taunt was in the requested slot off the wheel
		for ( int j = 0; j < m_TauntData.Count(); ++j )
		{
			if ( V_strcmp( m_TauntData[j].szPosition, pchPosition ) == 0 )
			{
				V_strncpy( m_TauntData[j].szPosition, "none", sizeof( m_TauntData[j].szPosition ) );
				break;
			}
		}
	}

	V_strncpy( pTauntData->szPosition, pchPosition, sizeof( pTauntData->szPosition ) );

	KeyValueSaver().MarkKeyValuesDirty( PORTAL2_MP_SAVE_FILE );
	m_bMenuStale = true;
	m_bLoadedSaveData = true;
}

void ClientMenuManagerTaunt::SetTauntLocked( const char *pchTaunt )
{
	// Reconstruction note: locking is retail-only (mp_lock_taunt); it reverses what
	// SetTauntUnlocked/SetTauntOwned/SetTauntPosition record for one taunt.
	TauntStatusData *pTauntData = GetTauntDataByName( pchTaunt );
	if ( !pTauntData )
		return;

	if ( !pTauntData->bUnlocked && !pTauntData->bOwned &&
	     V_strcmp( pTauntData->szPosition, "none" ) == 0 )
		return;

	pTauntData->bUnlocked = false;
	pTauntData->bOwned = false;
	pTauntData->bUsed = false;
	pTauntData->nCompletionDelay = 0;
	V_strncpy( pTauntData->szPosition, "none", sizeof( pTauntData->szPosition ) );

	KeyValueSaver().MarkKeyValuesDirty( PORTAL2_MP_SAVE_FILE );
	m_bMenuStale = true;
	m_bLoadedSaveData = true;
}

void ClientMenuManagerTaunt::SetAllTauntsLocked( void )
{
	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		SetTauntLocked( m_TauntData[i].szName );
	}
}

void ClientMenuManagerTaunt::DecreaseOwnershipDelays( void )
{
	bool bChange = false;
	bool bPending = false;

	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pTauntData = &m_TauntData[i];

		if ( pTauntData->bOwned || pTauntData->nCompletionDelay <= 0 )
			continue;

		pTauntData->nCompletionDelay--;
		bChange = true;

		if ( pTauntData->nCompletionDelay == 0 )
		{
			if ( pTauntData->bTeamTaunt )
			{
				// Team taunts still have to be performed together
				bPending = true;
			}
			else
			{
				SetTauntOwned( pTauntData->szName );
				SetTauntPosition( pTauntData->szName, "empty" );
				bChange = true;
			}
		}
	}

	if ( bChange )
	{
		if ( bPending )
		{
			engine->ClientCmd( VarArgs( "taunts_pending %i", GetNumTauntsPending() ) );
		}

		KeyValueSaver().MarkKeyValuesDirty( PORTAL2_MP_SAVE_FILE );
	}
}

int ClientMenuManagerTaunt::GetNumTauntsOwned( void )
{
	int nNumTauntsOwned = 0;

	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pTauntData = &m_TauntData[i];
		if ( pTauntData->bOwned )
		{
			nNumTauntsOwned++;
		}
	}

	return nNumTauntsOwned;
}

int ClientMenuManagerTaunt::GetNumTauntsPending( void )
{
	int nNumTauntsPending = 0;

	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pTauntData = &m_TauntData[i];
		if ( pTauntData->bUnlocked && !pTauntData->bOwned && pTauntData->nCompletionDelay == 0 )
		{
			nNumTauntsPending++;
		}
	}

	return nNumTauntsPending;
}

int ClientMenuManagerTaunt::GetNumTauntsUnused( void )
{
	int nNumTauntsUnused = 0;

	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pTauntData = &m_TauntData[i];
		if ( pTauntData->bUnlocked && pTauntData->bOwned && !pTauntData->bUsed )
		{
			nNumTauntsUnused++;
		}
	}

	return nNumTauntsUnused;
}

const TauntStatusData *ClientMenuManagerTaunt::GetUnusedTaunt( int nIndex )
{
	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pTauntData = &m_TauntData[i];
		if ( pTauntData->bUnlocked && pTauntData->bOwned && !pTauntData->bUsed )
		{
			if ( nIndex == 0 )
				return pTauntData;

			nIndex--;
		}
	}

	return NULL;
}

void ClientMenuManagerTaunt::ClearCustomMenuKeys( void )
{
	if ( m_customMenuKeys )
	{
		m_customMenuKeys->deleteThis();
	}

	m_customMenuKeys = new KeyValues( "ClientMenu" );
	m_nNumCustomKeys = 0;
}

void ClientMenuManagerTaunt::AddCustomMenuKey( KeyValues *pKeys )
{
	KeyValues *pDefaultKey = m_customMenuKeys->FindKey( "Default", true );
	pDefaultKey->AddSubKey( pKeys );
	m_nNumCustomKeys++;
}

KeyValues *ClientMenuManagerTaunt::GetAllTauntKeys( void )
{
	return m_menuKeys;
}

void ClientMenuManagerTaunt::UpdateDefaultTaunts( void )
{
	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPortalPlayer();
	C_Portal_Player *pPartnerPlayer = ( pPlayer ? pPlayer->HasTauntPartnerInRange() : NULL );
	bool bTeamAccept = ( pPartnerPlayer != NULL );

	if ( m_bOldPartnerInRange != bTeamAccept ||
	     ( bTeamAccept &&
	         V_strcmp( m_bOldPartnerForceTauntName, pPartnerPlayer->GetTauntForceName() ) != 0 ) )
	{
		// A partner came or went (or changed which team taunt they're offering)
		m_bOldPartnerInRange = bTeamAccept;
		V_strncpy( m_bOldPartnerForceTauntName,
		    ( bTeamAccept ? pPartnerPlayer->GetTauntForceName() : "" ),
		    sizeof( m_bOldPartnerForceTauntName ) );
		m_bMenuStale = true;
	}
	else if ( !m_bMenuStale )
	{
		return;
	}

	if ( !m_bLoadedSaveData )
	{
		// No save data yet, start out with the defaults
		m_bLoadedSaveData = true;
		KeyValueSaver().MarkKeyValuesDirty( PORTAL2_MP_SAVE_FILE );

		// The first taunt is always available
		for ( int i = 0; i < m_TauntData.Count(); ++i )
		{
			TauntStatusData *pTauntData = &m_TauntData[i];
			pTauntData->bUnlocked = true;
			pTauntData->bOwned = true;
			pTauntData->bUsed = true;
			pTauntData->nCompletionDelay = 0;
			V_strncpy( pTauntData->szPosition, g_szPositions[0], sizeof( pTauntData->szPosition ) );
			break;
		}

		// The first team taunt is waiting to be performed together
		for ( int i = 0; i < m_TauntData.Count(); ++i )
		{
			TauntStatusData *pTauntData = &m_TauntData[i];
			if ( !pTauntData->bTeamTaunt )
				continue;

			pTauntData->bUnlocked = true;
			pTauntData->bOwned = false;
			pTauntData->bUsed = false;
			engine->ClientCmd( VarArgs( "taunts_pending %i", GetNumTauntsPending() ) );
			break;
		}
	}

	ClearCustomMenuKeys();

	bool bPositionsFilled[NUM_TAUNT_POSITIONS];
	V_memset( bPositionsFilled, 0, sizeof( bPositionsFilled ) );

	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		TauntStatusData *pTauntData = &m_TauntData[i];

		// The team taunt our partner is offering goes in the middle
		bool bIsReadyTeamTaunt =
		    ( bTeamAccept && V_strcmp( pPartnerPlayer->GetTauntForceName(),
		                         pTauntData->szCommand + V_strlen( "taunt " ) ) == 0 );

		if ( V_strcmp( pTauntData->szPosition, "none" ) == 0 && !bIsReadyTeamTaunt )
			continue;

		KeyValues *pNewTaunt;
		if ( bIsReadyTeamTaunt )
		{
			pNewTaunt = new KeyValues( "Center" );
			bTeamAccept = false;
		}
		else
		{
			pNewTaunt = new KeyValues( pTauntData->szPosition );
		}

		pNewTaunt->SetString( "command", pTauntData->szCommand );
		pNewTaunt->SetString( "icon", pTauntData->szIcon );
		AddCustomMenuKey( pNewTaunt );

		for ( int nPosition = 0; nPosition < NUM_TAUNT_POSITIONS; ++nPosition )
		{
			if ( !bPositionsFilled[nPosition] &&
			     V_strcmp( g_szPositions[nPosition], pTauntData->szPosition ) == 0 )
			{
				bPositionsFilled[nPosition] = true;
				break;
			}
		}
	}

	// Fill the rest of the wheel with blanks
	for ( int nPosition = 0; nPosition < NUM_TAUNT_POSITIONS; ++nPosition )
	{
		if ( bPositionsFilled[nPosition] )
			continue;

		KeyValues *pNewTaunt = new KeyValues( g_szPositions[nPosition] );
		pNewTaunt->SetString( "command", "" );
		pNewTaunt->SetString( "icon", "hud/commands/empty" );
		pNewTaunt->SetBool( "new", false );
		AddCustomMenuKey( pNewTaunt );
	}

	m_bMenuStale = false;
}

TauntStatusData *ClientMenuManagerTaunt::GetTauntDataByName( const char *pchTaunt )
{
	for ( int i = 0; i < m_TauntData.Count(); ++i )
	{
		if ( V_strcmp( m_TauntData[i].szName, pchTaunt ) == 0 )
		{
			return &m_TauntData[i];
		}
	}

	return NULL;
}
