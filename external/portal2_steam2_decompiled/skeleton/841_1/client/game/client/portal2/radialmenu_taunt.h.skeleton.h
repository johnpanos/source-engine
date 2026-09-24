// DWARF declaration skeleton for game/client/portal2/radialmenu_taunt.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/radialmenu_taunt.h:14 sizeof=0x8c (i386)
struct TauntStatusData
{
public:
	char szName[16]; // +0x0  // line 15
	char szCommand[32]; // +0x10  // line 16
	char szIcon[64]; // +0x30  // line 17
	int nTextureID; // +0x70  // line 18
	bool bTeamTaunt; // +0x74  // line 19
	char szPosition[16]; // +0x75  // line 21
	bool bUnlocked; // +0x85  // line 22
	bool bOwned; // +0x86  // line 23
	bool bUsed; // +0x87  // line 24
	int nCompletionDelay; // +0x88  // line 25
};

// game/client/portal2/radialmenu_taunt.h:14 (declaration)
~TauntStatusData();

// game/client/portal2/radialmenu_taunt.h:30 sizeof=0x68 (i386)
struct ClientMenuManagerTaunt : public ClientMenuManager
{
public:
	ClientMenuManagerTaunt();  // line 32
	virtual ~ClientMenuManagerTaunt();  // line 43
	void ClearTauntStatusData();  // line 52
	virtual KeyValues *FindMenu( const char * );  // line 54
	virtual void Flush();  // line 55
	void KeyValueBuilder( KeyValues * );  // line 57
	const char *GetNextTauntToEarn();  // line 59
	CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> > *GetTauntData();  // line 61
	bool IsTauntTeam( const char * );  // line 63
	void SetTauntUnlocked( const char * );  // line 65
	void SetTauntOwned( const char * );  // line 66
	void SetTauntUsed( const char * );  // line 67
	void SetTauntDelay( const char *, int );  // line 68
	void SetTauntPosition( const char *, const char * );  // line 69
	void DecreaseOwnershipDelays();  // line 71
	int GetNumTauntsOwned();  // line 73
	int GetNumTauntsPending();  // line 74
	int GetNumTauntsUnused();  // line 75
	const TauntStatusData *GetUnusedTaunt( int );  // line 76
	void ClearCustomMenuKeys();  // line 78
	void AddCustomMenuKey( KeyValues * );  // line 79
	KeyValues *GetAllTauntKeys();  // line 80
	void UpdateDefaultTaunts();  // line 81
protected:
	TauntStatusData *GetTauntDataByName( const char * );  // line 85
	KeyValues *m_customMenuKeys; // +0x8  // line 87
	int m_nNumCustomKeys; // +0xc  // line 88
	bool m_bLoadedSaveData; // +0x10  // line 90
	bool m_bMenuStale; // +0x11  // line 91
	bool m_bOldPartnerInRange; // +0x12  // line 92
	char m_bOldPartnerForceTauntName[64]; // +0x13  // line 93
	CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> > m_TauntData; // +0x54  // line 94
};

// game/client/portal2/radialmenu_taunt.h:43 @0x622240 _ZN22ClientMenuManagerTauntD0Ev
ClientMenuManagerTaunt::~ClientMenuManagerTaunt()
{
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::~CUtlVector() at line 50
	// inlined ClientMenuManager::~ClientMenuManager() at line 50
	// inlined ClientMenuManager::~ClientMenuManager() at line 50
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::~CUtlVector() at line 50
}

// game/client/portal2/radialmenu_taunt.h:43 @0x6223e0 _ZN22ClientMenuManagerTauntD1Ev
ClientMenuManagerTaunt::~ClientMenuManagerTaunt()
{
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::~CUtlVector() at line 50
	// inlined ClientMenuManager::~ClientMenuManager() at line 50
	// inlined ClientMenuManager::~ClientMenuManager() at line 50
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::~CUtlVector() at line 50
}
