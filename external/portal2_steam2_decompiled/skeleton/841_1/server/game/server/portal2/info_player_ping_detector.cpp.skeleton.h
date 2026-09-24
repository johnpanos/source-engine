// DWARF declaration skeleton for game/server/portal2/info_player_ping_detector.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xaafa0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CInfo_Player_Ping_Detector>::CEntityFactory() at line 75
}

// game/server/portal2/info_player_ping_detector.cpp:16 sizeof=0x3fc (i386)
struct CInfo_Player_Ping_Detector : public CPointEntity, public CGameEventListener
{
public:
	CInfo_Player_Ping_Detector();  // line 20
	virtual ~CInfo_Player_Ping_Detector();  // line 21
	virtual void Spawn();  // line 23
	virtual void UpdateOnRemove();  // line 24
	virtual void FireGameEvent( IGameEvent * );  // line 25
	void InputToggle( inputdata_t & );  // line 28
	void InputEnable( inputdata_t & );  // line 29
	void InputDisable( inputdata_t & );  // line 30
protected:
	void Toggle();  // line 35
	void Enable();  // line 36
	void Disable();  // line 37
private:
	string_t m_iszFuncTankName; // +0x3cc  // line 41
	bool m_bEnabled; // +0x3d0  // line 43
	bool m_bDetectedNewPing; // +0x3d1  // line 45
	bool m_bLookAtPlayerPings; // +0x3d2  // line 46
	int m_nTeamToLookAt; // +0x3d4  // line 47
	Vector m_vecPingLocation; // +0x3d8  // line 49
	COutputEvent m_OnPingDetected; // +0x3e4  // line 51
};

// game/server/portal2/info_player_ping_detector.cpp:20 @0x71a0e0 _ZN26CInfo_Player_Ping_DetectorC2Ev
CInfo_Player_Ping_Detector::CInfo_Player_Ping_Detector()
{
	// inlined CPointEntity::CPointEntity() at line 80
	// inlined CGameEventListener::CGameEventListener() at line 80
	// inlined COutputEvent::COutputEvent() at line 80
	// inlined Vector::Vector() at line 82
	// inlined Vector::operator=() at line 82
}

// game/server/portal2/info_player_ping_detector.cpp:20 @0x71a170 _ZN26CInfo_Player_Ping_DetectorC1Ev
CInfo_Player_Ping_Detector::CInfo_Player_Ping_Detector()
{
}

// game/server/portal2/info_player_ping_detector.cpp:21 @0x71a830 _ZN26CInfo_Player_Ping_DetectorD2Ev
CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector()
{
	// inlined COutputEvent::~COutputEvent() at line 89
	// inlined CGameEventListener::~CGameEventListener() at line 89
	// inlined CPointEntity::~CPointEntity() at line 89
	// inlined CPointEntity::~CPointEntity() at line 89
	// inlined CGameEventListener::~CGameEventListener() at line 89
}

// game/server/portal2/info_player_ping_detector.cpp:21 @0x71a950 _ZN26CInfo_Player_Ping_DetectorD0Ev
CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector()
{
	// inlined COutputEvent::~COutputEvent() at line 89
	// inlined CGameEventListener::~CGameEventListener() at line 89
	// inlined CPointEntity::~CPointEntity() at line 89
	// inlined CPointEntity::~CPointEntity() at line 89
	// inlined CGameEventListener::~CGameEventListener() at line 89
}

// game/server/portal2/info_player_ping_detector.cpp:21 @0x71aa70 _ZN26CInfo_Player_Ping_DetectorD1Ev
CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector()
{
	// inlined COutputEvent::~COutputEvent() at line 89
	// inlined CGameEventListener::~CGameEventListener() at line 89
	// inlined CPointEntity::~CPointEntity() at line 89
	// inlined CPointEntity::~CPointEntity() at line 89
	// inlined CGameEventListener::~CGameEventListener() at line 89
}

// game/server/portal2/info_player_ping_detector.cpp:23 @0x71a070 _ZN26CInfo_Player_Ping_Detector5SpawnEv
void CInfo_Player_Ping_Detector::Spawn()
{
	// inlined CGameEventListener::ListenForGameEvent() at line 99
}

// game/server/portal2/info_player_ping_detector.cpp:24 @0x71a060 _ZN26CInfo_Player_Ping_Detector14UpdateOnRemoveEv
void CInfo_Player_Ping_Detector::UpdateOnRemove()
{
}

// game/server/portal2/info_player_ping_detector.cpp:25 @0x71a1f0 _ZN26CInfo_Player_Ping_Detector13FireGameEventEP10IGameEvent
void CInfo_Player_Ping_Detector::FireGameEvent( IGameEvent *event )
{
	const char *name;  // line 113
	{
		int nPlayerTeam;  // line 116
		int playerId;  // line 117
		Vector vecPingPos;  // line 119
		{
			CBaseEntity *pNewEntity;  // line 139
			{
				CFuncTank *pTank;  // line 142
				// inlined CFuncTank::AimAtTargetPosition() at line 144
			}
			// inlined string_t::ToCStr() at line 140
			// inlined Vector::operator=() at line 136
		}
		{
			int i;  // line 124
			{
				CBasePlayer *pPlayer;  // line 126
				// inlined CBasePlayer::GetUserID() at line 127
			}
		}
	}
}

// game/server/portal2/info_player_ping_detector.cpp:28 @0x71a780 _ZN26CInfo_Player_Ping_Detector11InputToggleER11inputdata_t
void CInfo_Player_Ping_Detector::InputToggle( inputdata_t &inputdata )
{
	// inlined CInfo_Player_Ping_Detector::Toggle() at line 157
}

// game/server/portal2/info_player_ping_detector.cpp:29 @0x71a670 _ZN26CInfo_Player_Ping_Detector11InputEnableER11inputdata_t
void CInfo_Player_Ping_Detector::InputEnable( inputdata_t &inputdata )
{
	// inlined CInfo_Player_Ping_Detector::Enable() at line 165
}

// game/server/portal2/info_player_ping_detector.cpp:30 @0x71a610 _ZN26CInfo_Player_Ping_Detector12InputDisableER11inputdata_t
void CInfo_Player_Ping_Detector::InputDisable( inputdata_t &inputdata )
{
	// inlined CInfo_Player_Ping_Detector::Disable() at line 173
}

// game/server/portal2/info_player_ping_detector.cpp:35 @0x71a6d0 _ZN26CInfo_Player_Ping_Detector6ToggleEv
void CInfo_Player_Ping_Detector::Toggle()
{
	// inlined CInfo_Player_Ping_Detector::Disable() at line 184
	// inlined CInfo_Player_Ping_Detector::Enable() at line 188
}

// game/server/portal2/info_player_ping_detector.cpp:36 @0x71a180 _ZN26CInfo_Player_Ping_Detector6EnableEv
void CInfo_Player_Ping_Detector::Enable()
{
	// inlined CGameEventListener::ListenForGameEvent() at line 199
}

// game/server/portal2/info_player_ping_detector.cpp:37 @0x71a5b0 _ZN26CInfo_Player_Ping_Detector7DisableEv
void CInfo_Player_Ping_Detector::Disable()
{
	// inlined CGameEventListener::StopListeningForAllEvents() at line 208
}

// game/server/portal2/info_player_ping_detector.cpp:53 @0x71a040 _ZN26CInfo_Player_Ping_Detector14GetDataDescMapEv
datamap_t *CInfo_Player_Ping_Detector::GetDataDescMap()
{
}

// game/server/portal2/info_player_ping_detector.cpp:53 @0x71a050 _ZN26CInfo_Player_Ping_Detector10GetBaseMapEv
datamap_t *CInfo_Player_Ping_Detector::GetBaseMap()
{
}

// game/server/portal2/info_player_ping_detector.cpp:57 @0xaae80 _Z11DataMapInitI26CInfo_Player_Ping_DetectorEP9datamap_tPT_
datamap_t *DataMapInit<CInfo_Player_Ping_Detector>( CInfo_Player_Ping_Detector * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 57
	CDatadescGeneratedNameHolder nameHolder;  // line 57
	typedescription_t dataDesc[10];  // line 57
}

// game/server/portal2/info_player_ping_detector.cpp:57
datamap_t *g_DataMapHolder;

// game/server/portal2/info_player_ping_detector.cpp:75
static CEntityFactory<CInfo_Player_Ping_Detector> info_player_ping_detector;

// game/server/util.h:100 @0x71abc0 _ZN14CEntityFactoryI26CInfo_Player_Ping_DetectorE6CreateEPKc
IServerNetworkable *CEntityFactory<CInfo_Player_Ping_Detector>::Create( const char *pClassName )
{
	CInfo_Player_Ping_Detector *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CInfo_Player_Ping_Detector>() at line 102
}

// game/server/util.h:106 @0x71ab80 _ZN14CEntityFactoryI26CInfo_Player_Ping_DetectorE7DestroyEP18IServerNetworkable
void CEntityFactory<CInfo_Player_Ping_Detector>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x71abb0 _ZN14CEntityFactoryI26CInfo_Player_Ping_DetectorE13GetEntitySizeEv
size_t CEntityFactory<CInfo_Player_Ping_Detector>::GetEntitySize()
{
}
