// DWARF declaration skeleton for game/server/portal2/trigger_playerteam.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xba830 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CTriggerPlayerTeam>::CEntityFactory() at line 105
}

// game/server/portal2/trigger_playerteam.cpp:16 sizeof=0x550 (i386)
struct CTriggerPlayerTeam : public CBaseTrigger
{
public:
	virtual void Spawn();  // line 21
	virtual void StartTouch( CBaseEntity * );  // line 30
	virtual void EndTouch( CBaseEntity * );  // line 58
private:
	int m_nTargetTeam; // +0x4e8  // line 81
	bool m_bTriggerOnce; // +0x4ec  // line 82
	COutputEvent m_OnStartTouchOrangePlayer; // +0x4f0  // line 84
	COutputEvent m_OnEndTouchOrangePlayer; // +0x508  // line 85
	COutputEvent m_OnStartTouchBluePlayer; // +0x520  // line 86
	COutputEvent m_OnEndTouchBluePlayer; // +0x538  // line 87
};

// game/server/portal2/trigger_playerteam.cpp:16 (declaration)
void CTriggerPlayerTeam();

// game/server/portal2/trigger_playerteam.cpp:16 (declaration)
~CTriggerPlayerTeam();

// game/server/portal2/trigger_playerteam.cpp:16 @0x776260 _ZN18CTriggerPlayerTeamD0Ev
CTriggerPlayerTeam::~CTriggerPlayerTeam()
{
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
	// inlined COutputEvent::~COutputEvent() at line 16
}

// game/server/portal2/trigger_playerteam.cpp:16 @0x776320 _ZN18CTriggerPlayerTeamD1Ev
CTriggerPlayerTeam::~CTriggerPlayerTeam()
{
	// inlined CTriggerPlayerTeam::~CTriggerPlayerTeam() at line 16
}

// game/server/portal2/trigger_playerteam.cpp:19 @0x775c40 _ZN18CTriggerPlayerTeam14GetDataDescMapEv
datamap_t *CTriggerPlayerTeam::GetDataDescMap()
{
}

// game/server/portal2/trigger_playerteam.cpp:19 @0x775c50 _ZN18CTriggerPlayerTeam10GetBaseMapEv
datamap_t *CTriggerPlayerTeam::GetBaseMap()
{
}

// game/server/portal2/trigger_playerteam.cpp:21 @0x775f00 _ZN18CTriggerPlayerTeam5SpawnEv
void CTriggerPlayerTeam::Spawn()
{
	// inlined CBaseEntity::ClearSpawnFlags() at line 23
	// inlined CBaseEntity::AddSpawnFlags() at line 24
}

// game/server/portal2/trigger_playerteam.cpp:30 @0x775d60 _ZN18CTriggerPlayerTeam10StartTouchEP11CBaseEntity
void CTriggerPlayerTeam::StartTouch( CBaseEntity *pOther )
{
}

// game/server/portal2/trigger_playerteam.cpp:58 @0x775ca0 _ZN18CTriggerPlayerTeam8EndTouchEP11CBaseEntity
void CTriggerPlayerTeam::EndTouch( CBaseEntity *pOther )
{
}

// game/server/portal2/trigger_playerteam.cpp:90 @0xba700 _Z11DataMapInitI18CTriggerPlayerTeamEP9datamap_tPT_
datamap_t *DataMapInit<CTriggerPlayerTeam>( CTriggerPlayerTeam * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 90
	CDatadescGeneratedNameHolder nameHolder;  // line 90
	typedescription_t dataDesc[7];  // line 90
}

// game/server/portal2/trigger_playerteam.cpp:90
datamap_t *g_DataMapHolder;

// game/server/util.h:100 @0x775e30 _ZN14CEntityFactoryI18CTriggerPlayerTeamE6CreateEPKc
IServerNetworkable *CEntityFactory<CTriggerPlayerTeam>::Create( const char *pClassName )
{
	CTriggerPlayerTeam *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CTriggerPlayerTeam>() at line 102
}

// game/server/portal2/trigger_playerteam.cpp:105
static CEntityFactory<CTriggerPlayerTeam> trigger_playerteam;

// game/server/util.h:106 @0x775c60 _ZN14CEntityFactoryI18CTriggerPlayerTeamE7DestroyEP18IServerNetworkable
void CEntityFactory<CTriggerPlayerTeam>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x775c90 _ZN14CEntityFactoryI18CTriggerPlayerTeamE13GetEntitySizeEv
size_t CEntityFactory<CTriggerPlayerTeam>::GetEntitySize()
{
}
