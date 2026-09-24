// DWARF declaration skeleton for game/client/portal/c_portal_gamestats.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal/c_portal_gamestats.h:25
typedef CUtlVector<CPortal_StuckPlayer*,CUtlMemory<CPortal_StuckPlayer*, int> > VectorStuckPlayer;

// game/client/portal/c_portal_gamestats.h:29 sizeof=0xd4 (i386)
struct SConfigStats : public BaseStatData
{
public:
	SConfigStats();  // line 30
	char m_TextLanguage[64]; // +0xc  // line 32
	char m_AudioLanguage[64]; // +0x4c  // line 33
	char m_CountryCode[64]; // +0x8c  // line 34
	uint8 m_ReverseMouse; // +0xcc  // line 35
	uint8 m_AllowConsole; // +0xcd  // line 36
	uint8 m_AllowGamepad; // +0xce  // line 37
	uint8 m_OpenMic; // +0xcf  // line 38
	uint8 m_ColorBlind; // +0xd0  // line 39
	uint8 m_Captions; // +0xd1  // line 40
	uint8 m_LowViolence; // +0xd2  // line 41
	static const char *GetStatTableName();  // line 43
	void BuildGamestatDataTable( KeyValues * );  // line 43
};

// game/client/portal/c_portal_gamestats.h:58 sizeof=0x1018 (i386)
struct SSurveyData : public BaseStatData
{
public:
	SSurveyData( uint32, uint32, const char * );  // line 59
	uint32 m_Difficulty; // +0xc  // line 67
	uint32 m_Enjoyment; // +0x10  // line 68
	char m_Essay[4097]; // +0x14  // line 69
	static const char *GetStatTableName();  // line 71
	void BuildGamestatDataTable( KeyValues * );  // line 71
};

// game/client/portal/c_portal_gamestats.h:78
typedef CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*, int> > VectorSurveyData;

// game/client/portal/c_portal_gamestats.h:84 sizeof=0x128 (i386)
struct C_CPortalGameStats : public CBaseGameStats, public CGameEventListener, public CAutoGameSystem, public IGameStatTracker, public IMatchEventsSink
{
public:
	C_CPortalGameStats();  // line 88
	virtual ~C_CPortalGameStats();  // line 93
	virtual void OnEvent( KeyValues * );  // line 98
	virtual void Event_LevelInit();  // line 101
	virtual void Event_LevelShutdown( float );  // line 102
	virtual bool ShouldSendDataOnLevelShutdown();  // line 103
	virtual bool ShouldSendDataOnAppShutdown();  // line 104
	virtual void FireGameEvent( IGameEvent * );  // line 105
	virtual bool UseOldFormat();  // line 106
	virtual bool AddDataForSend( KeyValues *, StatSendType_t );  // line 107
	virtual void SubmitGameStats( KeyValues * );  // line 108
	void AddData();  // line 109
	virtual void OnSessionStart();  // line 110
	virtual void OnSessionEnd();  // line 111
	virtual bool Init();  // line 113
	virtual void Shutdown();  // line 114
	virtual void Event_AddSurveyData( uint32, uint32, const char * );  // line 115
	virtual CUtlVector<IGameStatTracker::IStatContainer*,CUtlMemory<IGameStatTracker::IStatContainer*, int> > *GetStatContainerList();  // line 117
private:
	void GetConfigData();  // line 124
	static CUtlVector<IGameStatTracker::IStatContainer*,CUtlMemory<IGameStatTracker::IStatContainer*, int> > *s_StatLists;  // line 127
	VectorStuckPlayer m_VectorStuckPlayer; // +0x28  // line 129
	SConfigStats m_ConfigData; // +0x3c  // line 130
	VectorSurveyData m_SurveyData; // +0x110  // line 131
	bool m_bSessionActive; // +0x124  // line 132
};

// game/client/portal/c_portal_gamestats.h:93 @0x4e0410 _ZN18C_CPortalGameStatsD0Ev
C_CPortalGameStats::~C_CPortalGameStats()
{
	// inlined CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*, int> >::~CUtlVector() at line 95
	// inlined CUtlVector<CPortal_StuckPlayer*,CUtlMemory<CPortal_StuckPlayer*, int> >::~CUtlVector() at line 95
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 95
	// inlined CGameEventListener::~CGameEventListener() at line 95
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 95
	// inlined CGameEventListener::~CGameEventListener() at line 95
	// inlined CUtlVector<CPortal_StuckPlayer*,CUtlMemory<CPortal_StuckPlayer*, int> >::~CUtlVector() at line 95
}

// game/client/portal/c_portal_gamestats.h:93 @0x4e0720 _ZN18C_CPortalGameStatsD1Ev
C_CPortalGameStats::~C_CPortalGameStats()
{
	// inlined C_CPortalGameStats::~C_CPortalGameStats() at line 95
}

// game/client/portal/c_portal_gamestats.h:102 @0x4df790 _ZN18C_CPortalGameStats19Event_LevelShutdownEf
void C_CPortalGameStats::Event_LevelShutdown( float flElapsed )
{
}

// game/client/portal/c_portal_gamestats.h:103 @0x4df7a0 _ZN18C_CPortalGameStats29ShouldSendDataOnLevelShutdownEv
bool C_CPortalGameStats::ShouldSendDataOnLevelShutdown()
{
}

// game/client/portal/c_portal_gamestats.h:104 @0x4df7b0 _ZN18C_CPortalGameStats27ShouldSendDataOnAppShutdownEv
bool C_CPortalGameStats::ShouldSendDataOnAppShutdown()
{
}

// game/client/portal/c_portal_gamestats.h:105 @0x4df7d0 _ZN18C_CPortalGameStats13FireGameEventEP10IGameEvent
void C_CPortalGameStats::FireGameEvent( IGameEvent *event )
{
}

// game/client/portal/c_portal_gamestats.h:106 @0x4df7e0 _ZN18C_CPortalGameStats12UseOldFormatEv
bool C_CPortalGameStats::UseOldFormat()
{
}

// game/client/portal/c_portal_gamestats.h:117 @0x4df800 _ZN18C_CPortalGameStats20GetStatContainerListEv
CUtlVector<IGameStatTracker::IStatContainer*,CUtlMemory<IGameStatTracker::IStatContainer*, int> > *C_CPortalGameStats::GetStatContainerList()
{
}
