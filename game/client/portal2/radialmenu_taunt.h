//========= Portal 2 reconstruction ============================================//
//
// Purpose: Gesture (taunt) wheel menu manager and per-taunt ownership state
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef RADIALMENU_TAUNT_H
#define RADIALMENU_TAUNT_H
#ifdef _WIN32
#pragma once
#endif

#include "radialmenu.h"

struct TauntStatusData
{
	char szName[16];
	char szCommand[32];
	char szIcon[64];
	int nTextureID;
	bool bTeamTaunt;

	char szPosition[16];
	bool bUnlocked;
	bool bOwned;
	bool bUsed;
	int nCompletionDelay;
};

class IPlayerLocal;

class ClientMenuManagerTaunt : public ClientMenuManager
{
public:
	ClientMenuManagerTaunt()
	{
		m_customMenuKeys = NULL;
		ClearCustomMenuKeys();

		m_bLoadedSaveData = false;
		m_bMenuStale = true;
		m_bOldPartnerInRange = false;
		m_bOldPartnerForceTauntName[0] = '\0';
	}

	virtual ~ClientMenuManagerTaunt()
	{
		if ( m_customMenuKeys )
		{
			m_customMenuKeys->deleteThis();
		}
		m_customMenuKeys = NULL;
	}

	void ClearTauntStatusData( void );

	virtual KeyValues *FindMenu( const char *menuName );
	virtual void Flush( void );

	void KeyValueBuilder( KeyValues *pKeyValues );
	// Reconstruction note: the retail game rules hand the loaded player data to the
	// manager here; the 2010 build did the same work inline in LoadMapCompleteData.
	void KeyValueProcessor( IPlayerLocal *pPlayer );

	const char *GetNextTauntToEarn( void );

	CUtlVector<TauntStatusData> *GetTauntData( void ) { return &m_TauntData; }

	bool IsTauntTeam( const char *pchTaunt );

	void SetTauntUnlocked( const char *pchTaunt );
	void SetTauntOwned( const char *pchTaunt );
	void SetTauntOwned( const char *pchTaunt, bool bAwardSilently );
	void SetTauntUsed( const char *pchTaunt );
	void SetTauntDelay( const char *pchTaunt, int nDelay );
	void SetTauntPosition( const char *pchTaunt, const char *pchPosition );
	void SetTauntLocked( const char *pchTaunt );
	void SetAllTauntsLocked( void );

	void DecreaseOwnershipDelays( void );

	int GetNumTauntsOwned( void );
	int GetNumTauntsPending( void );
	int GetNumTauntsUnused( void );
	const TauntStatusData *GetUnusedTaunt( int nIndex );

	void ClearCustomMenuKeys( void );
	void AddCustomMenuKey( KeyValues *pKeys );
	KeyValues *GetAllTauntKeys( void );
	void UpdateDefaultTaunts( void );

protected:
	TauntStatusData *GetTauntDataByName( const char *pchTaunt );

	KeyValues *m_customMenuKeys;
	int m_nNumCustomKeys;

	bool m_bLoadedSaveData;
	bool m_bMenuStale;
	bool m_bOldPartnerInRange;
	char m_bOldPartnerForceTauntName[64];
	CUtlVector<TauntStatusData> m_TauntData;
};

// Reconstruction note: the 2010 build had one global manager (TheClientMenuManagerTaunt);
// the retail radial menu and game rules address one manager per split-screen slot.
// A negative slot selects the active split-screen slot.
ClientMenuManagerTaunt &GetClientMenuManagerTaunt( int nSlot = -1 );

#endif // RADIALMENU_TAUNT_H
