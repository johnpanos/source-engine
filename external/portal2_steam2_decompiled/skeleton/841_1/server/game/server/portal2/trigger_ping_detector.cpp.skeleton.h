// DWARF declaration skeleton for game/server/portal2/trigger_ping_detector.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xba5f0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CTriggerPingDetector>::CEntityFactory() at line 80
}

// game/server/portal2/trigger_ping_detector.cpp:13 sizeof=0x524 (i386)
struct CTriggerPingDetector : public CBaseTrigger, public CGameEventListener
{
public:
	virtual void Spawn();  // line 19
	virtual void FireGameEvent( IGameEvent * );  // line 28
private:
	COutputEvent m_OnOrangePlayerPinged; // +0x4f4  // line 71
	COutputEvent m_OnBluePlayerPinged; // +0x50c  // line 72
};

// game/server/portal2/trigger_ping_detector.cpp:13 (declaration)
void CTriggerPingDetector();

// game/server/portal2/trigger_ping_detector.cpp:13 (declaration)
~CTriggerPingDetector();

// game/server/portal2/trigger_ping_detector.cpp:13 @0x775a00 _ZN20CTriggerPingDetectorD1Ev
CTriggerPingDetector::~CTriggerPingDetector()
{
	// inlined CTriggerPingDetector::~CTriggerPingDetector() at line 13
}

// game/server/portal2/trigger_ping_detector.cpp:13 @0x775b20 _ZN20CTriggerPingDetectorD0Ev
CTriggerPingDetector::~CTriggerPingDetector()
{
	// inlined COutputEvent::~COutputEvent() at line 13
	// inlined COutputEvent::~COutputEvent() at line 13
	// inlined CGameEventListener::~CGameEventListener() at line 13
	// inlined CGameEventListener::~CGameEventListener() at line 13
	// inlined COutputEvent::~COutputEvent() at line 13
}

// game/server/portal2/trigger_ping_detector.cpp:16 @0x775460 _ZN20CTriggerPingDetector14GetDataDescMapEv
datamap_t *CTriggerPingDetector::GetDataDescMap()
{
}

// game/server/portal2/trigger_ping_detector.cpp:16 @0x775470 _ZN20CTriggerPingDetector10GetBaseMapEv
datamap_t *CTriggerPingDetector::GetBaseMap()
{
}

// game/server/portal2/trigger_ping_detector.cpp:19 @0x775800 _ZN20CTriggerPingDetector5SpawnEv
void CTriggerPingDetector::Spawn()
{
	// inlined CBaseEntity::ClearSpawnFlags() at line 21
	// inlined CGameEventListener::ListenForGameEvent() at line 24
}

// game/server/portal2/trigger_ping_detector.cpp:28 @0x7754d0 _ZN20CTriggerPingDetector13FireGameEventEP10IGameEvent
void CTriggerPingDetector::FireGameEvent( IGameEvent *event )
{
	const char *name;  // line 30
	{
		int nPlayerTeam;  // line 33
		int playerId;  // line 34
		Vector vecPingPos;  // line 36
		Ray_t shotRay;  // line 51
		trace_t tr;  // line 53
		{
			int i;  // line 41
			{
				CBasePlayer *pPlayer;  // line 43
				// inlined CBasePlayer::GetUserID() at line 44
			}
		}
		// inlined Ray_t::Init() at line 52
	}
}

// game/server/portal2/trigger_ping_detector.cpp:75 @0xba4d0 _Z11DataMapInitI20CTriggerPingDetectorEP9datamap_tPT_
datamap_t *DataMapInit<CTriggerPingDetector>( CTriggerPingDetector * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 75
	CDatadescGeneratedNameHolder nameHolder;  // line 75
	typedescription_t dataDesc[3];  // line 75
}

// game/server/portal2/trigger_ping_detector.cpp:75
datamap_t *g_DataMapHolder;

// game/server/portal2/trigger_ping_detector.cpp:80
static CEntityFactory<CTriggerPingDetector> trigger_ping_detector;

// game/server/util.h:100 @0x775740 _ZN14CEntityFactoryI20CTriggerPingDetectorE6CreateEPKc
IServerNetworkable *CEntityFactory<CTriggerPingDetector>::Create( const char *pClassName )
{
	CTriggerPingDetector *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CTriggerPingDetector>() at line 102
}

// game/server/util.h:106 @0x775480 _ZN14CEntityFactoryI20CTriggerPingDetectorE7DestroyEP18IServerNetworkable
void CEntityFactory<CTriggerPingDetector>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x7754b0 _ZN14CEntityFactoryI20CTriggerPingDetectorE13GetEntitySizeEv
size_t CEntityFactory<CTriggerPingDetector>::GetEntitySize()
{
}
