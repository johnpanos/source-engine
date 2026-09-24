// DWARF declaration skeleton for game/server/portal2/indicator_panel.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/indicator_panel.h:8 sizeof=0x3d8 (i386)
struct CLabIndicatorPanel : public CBaseEntity
{
public:
	virtual ~CLabIndicatorPanel();  // line 15
	virtual int UpdateTransmitState();  // line 17
	virtual void SetTransmit( CCheckTransmitInfo *, bool );  // line 18
	virtual void Spawn();  // line 20
	virtual void Precache();  // line 21
	virtual void OnRestore();  // line 22
	void ScreenVisible( bool );  // line 24
	void Disable();  // line 26
	void Enable();  // line 27
	void InputDisable( inputdata_t & );  // line 30
	void InputEnable( inputdata_t & );  // line 31
	void StartTimer( float );  // line 34
	void ResetTimer();  // line 35
	void SetChecked( bool );  // line 36
	void SetPlayerPinged( int );  // line 37
	void SetIsTimer( bool );  // line 38
private:
	void GetControlPanelInfo( int, const char *& );  // line 43
	void GetControlPanelClassName( int, const char *& );  // line 44
	void SpawnControlPanels();  // line 45
	void RestoreControlPanels();  // line 46
	CNetworkVar( bool, m_bEnabled ); // +0x3c0  // line 50
	CNetworkVar( bool, m_bStopped ); // +0x3c1  // line 51
	CNetworkVar( bool, m_bCountingDown ); // +0x3c2  // line 52
	CNetworkVar( bool, m_bIsCountdownTimer ); // +0x3c3  // line 53
	CNetworkVar( bool, m_bIsChecked ); // +0x3c4  // line 54
	CNetworkVar( int, m_iPlayerPinged ); // +0x3c8  // line 55
	CNetworkVar( float, m_flTimerDuration ); // +0x3cc  // line 57
	CNetworkVar( float, m_flTimerStart ); // +0x3d0  // line 58
	CHandle<CVGuiScreen> m_hScreen; // +0x3d4  // line 61
};

// game/server/portal2/indicator_panel.h:8 (declaration)
void CLabIndicatorPanel();

// game/server/portal2/indicator_panel.h:71 sizeof=0x52c (i386)
struct CPropIndicatorPanel : public CBaseAnimating
{
public:
	virtual void Precache();  // line 76
	virtual void Spawn();  // line 77
	virtual void UpdateOnRemove();  // line 79
	void SetTimerDuration( float );  // line 81
	void StartTimer();  // line 82
	void StopTimer();  // line 83
	void ResetTimer();  // line 84
	void InputStart( inputdata_t & );  // line 86
	void InputStop( inputdata_t & );  // line 87
	void InputReset( inputdata_t & );  // line 88
	void InputCheck( inputdata_t & );  // line 89
	void InputUncheck( inputdata_t & );  // line 90
private:
	void CreateIndicatorPanel();  // line 94
	void ToggleIndicatorLights( bool );  // line 95
	void ToggleThink();  // line 96
	bool m_bEnabled; // +0x514  // line 100
	bool m_bStopped; // +0x515  // line 101
	bool m_bCountingDown; // +0x516  // line 102
	bool m_bIsCountdownTimer; // +0x517  // line 103
	bool m_bIsChecked; // +0x518  // line 104
	string_t m_strIndicatorLights; // +0x51c  // line 105
	float m_flTimerDuration; // +0x520  // line 107
	float m_flTimerStart; // +0x524  // line 108
protected:
	CHandle<CLabIndicatorPanel> m_hIndicatorPanel; // +0x528  // line 111
};

// game/server/portal2/indicator_panel.h:71 (declaration)
~CPropIndicatorPanel();

// game/server/portal2/indicator_panel.h:71 @0x6b8e70 _ZN19CPropIndicatorPanelD0Ev
CPropIndicatorPanel::~CPropIndicatorPanel()
{
}

// game/server/portal2/indicator_panel.h:71 @0x6b8eb0 _ZN19CPropIndicatorPanelD1Ev
CPropIndicatorPanel::~CPropIndicatorPanel()
{
}

// game/server/portal2/indicator_panel.h:71 (declaration)
void CPropIndicatorPanel();

// game/server/portal2/indicator_panel.h:121 sizeof=0x568 (i386)
struct CPropTicTacToePanel : public CPropIndicatorPanel, public CGameEventListener
{
public:
	virtual void Spawn();  // line 126
	virtual void FireGameEvent( IGameEvent * );  // line 128
	bool m_bPlayed; // +0x535  // line 130
	COutputEvent m_OnPlayer1Pinged; // +0x538  // line 132
	COutputEvent m_OnPlayer2Pinged; // +0x550  // line 133
};

// game/server/portal2/indicator_panel.h:121 (declaration)
void CPropTicTacToePanel();

// game/server/portal2/indicator_panel.h:121 (declaration)
~CPropTicTacToePanel();

// game/server/portal2/indicator_panel.h:121 @0x6b90a0 _ZN19CPropTicTacToePanelD0Ev
CPropTicTacToePanel::~CPropTicTacToePanel()
{
	// inlined COutputEvent::~COutputEvent() at line 121
	// inlined COutputEvent::~COutputEvent() at line 121
	// inlined CGameEventListener::~CGameEventListener() at line 121
	// inlined CPropIndicatorPanel::~CPropIndicatorPanel() at line 121
	// inlined CPropIndicatorPanel::~CPropIndicatorPanel() at line 121
	// inlined CGameEventListener::~CGameEventListener() at line 121
	// inlined COutputEvent::~COutputEvent() at line 121
}

// game/server/portal2/indicator_panel.h:121 @0x6b91e0 _ZN19CPropTicTacToePanelD1Ev
CPropTicTacToePanel::~CPropTicTacToePanel()
{
	// inlined CPropTicTacToePanel::~CPropTicTacToePanel() at line 121
}
