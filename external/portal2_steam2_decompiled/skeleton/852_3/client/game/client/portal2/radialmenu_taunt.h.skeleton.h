// DWARF declaration skeleton for game/client/portal2/radialmenu_taunt.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/radialmenu_taunt.h:13 sizeof=0x8c (i386)
struct TauntStatusData
{
public:
	char szName[16]; // +0x0  // line 14
	char szCommand[32]; // +0x10  // line 15
	char szIcon[64]; // +0x30  // line 16
	int nTextureID; // +0x70  // line 17
	bool bTeamTaunt; // +0x74  // line 18
	char szPosition[16]; // +0x75  // line 20
	bool bUnlocked; // +0x85  // line 21
	bool bOwned; // +0x86  // line 22
	bool bUsed; // +0x87  // line 23
	int nCompletionDelay; // +0x88  // line 24
};

// game/client/portal2/radialmenu_taunt.h:13 (declaration)
~TauntStatusData();

// game/client/portal2/radialmenu_taunt.h:29 sizeof=0x28 (i386)
struct ClientMenuManagerTaunt : public ClientMenuManager
{
public:
	ClientMenuManagerTaunt();  // line 31
	virtual ~ClientMenuManagerTaunt();  // line 40
	void ClearTauntStatusData();  // line 49
	virtual KeyValues *FindMenu( const char * );  // line 51
	virtual void Flush();  // line 52
	void KeyValueBuilder( KeyValues * );  // line 54
	const char *GetNextTauntToEarn();  // line 56
	CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> > *GetTauntData();  // line 58
	bool IsTauntTeam( const char * );  // line 60
	void SetTauntUnlocked( const char * );  // line 62
	void SetTauntOwned( const char * );  // line 63
	void SetTauntUsed( const char * );  // line 64
	void SetTauntDelay( const char *, int );  // line 65
	void SetTauntPosition( const char *, const char * );  // line 66
	void DecreaseOwnershipDelays();  // line 68
	int GetNumTauntsOwned();  // line 70
	int GetNumTauntsPending();  // line 71
	void ClearCustomMenuKeys();  // line 73
	void AddCustomMenuKey( KeyValues * );  // line 74
	KeyValues *GetAllTauntKeys();  // line 75
	void UpdateDefaultTaunts();  // line 76
protected:
	TauntStatusData *GetTauntDataByName( const char * );  // line 80
	KeyValues *m_customMenuKeys; // +0x8  // line 82
	int m_nNumCustomKeys; // +0xc  // line 83
	bool m_bLoadedSaveData; // +0x10  // line 85
	bool m_bMenuStale; // +0x11  // line 86
	CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> > m_TauntData; // +0x14  // line 87
};

// game/client/portal2/radialmenu_taunt.h:40 @0x616030 _ZN22ClientMenuManagerTauntD0Ev
ClientMenuManagerTaunt::~ClientMenuManagerTaunt()
{
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::~CUtlVector() at line 47
	// inlined ClientMenuManager::~ClientMenuManager() at line 47
	// inlined ClientMenuManager::~ClientMenuManager() at line 47
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::~CUtlVector() at line 47
}

// game/client/portal2/radialmenu_taunt.h:40 @0x6161d0 _ZN22ClientMenuManagerTauntD1Ev
ClientMenuManagerTaunt::~ClientMenuManagerTaunt()
{
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::~CUtlVector() at line 47
	// inlined ClientMenuManager::~ClientMenuManager() at line 47
	// inlined ClientMenuManager::~ClientMenuManager() at line 47
	// inlined CUtlVector<TauntStatusData,CUtlMemory<TauntStatusData, int> >::~CUtlVector() at line 47
}
