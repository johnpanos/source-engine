// DWARF declaration skeleton for game/server/portal2/prop_exploding_futbol.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xc9750 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CEntityFactory<CExplodingFutbolCatcher>::CEntityFactory() at line 125
	// inlined CEntityFactory<CPropExplodingFutbolSpawner>::CEntityFactory() at line 184
	// inlined CEntityFactory<CPropExplodingFutbolSocket>::CEntityFactory() at line 200
	// inlined CEntityFactory<CPropExplodingFutbol>::CEntityFactory() at line 286
}

// game/server/portal2/prop_exploding_futbol.cpp:26
char *g_szExplodingFutbolAnimThinkContext;

// game/server/portal2/prop_exploding_futbol.cpp:27
char *g_szExplodingFutbolTimerThinkContext;

// game/server/portal2/prop_exploding_futbol.cpp:28
char *g_szExplodingFutbolKillThinkContext;

// game/server/portal2/prop_exploding_futbol.cpp:30
ConVar exploding_futbol_explosion_debug;

// game/server/portal2/prop_exploding_futbol.cpp:31
ConVar exploding_futbol_use_cooldown_time;

// game/server/portal2/prop_exploding_futbol.cpp:32
ConVar exploding_futbol_explosion_radius;

// game/server/portal2/prop_exploding_futbol.cpp:33
ConVar exploding_futbol_explosion_magnitude;

// game/server/portal2/prop_exploding_futbol.cpp:34
ConVar exploding_futbol_explosion_damage;

// game/server/portal2/prop_exploding_futbol.cpp:35
ConVar exploding_futbol_explosion_damage_falloff;

// game/server/portal2/prop_exploding_futbol.cpp:36
ConVar exploding_futbol_start_color;

// game/server/portal2/prop_exploding_futbol.cpp:37
ConVar exploding_futbol_end_color;

// game/server/portal2/prop_exploding_futbol.cpp:38
ConVar exploding_futbol_flash_start_color;

// game/server/portal2/prop_exploding_futbol.cpp:39
ConVar exploding_futbol_flash_end_color;

// game/server/portal2/prop_exploding_futbol.cpp:40
ConVar exploding_futbol_flash_start_time;

// game/server/portal2/prop_exploding_futbol.cpp:41
ConVar exploding_futbol_flash_duration;

// game/server/portal2/prop_exploding_futbol.cpp:42
ConVar exploding_futbol_hit_breakables;

// game/server/portal2/prop_exploding_futbol.cpp:43
ConVar exploding_futbol_explode_on_fizzle;

// game/server/portal2/prop_exploding_futbol.cpp:46
static const char *g_psxExplodableEntities[6];

// game/server/portal2/prop_exploding_futbol.cpp:63 sizeof=0x4 (i386)
struct CFilterOnlyExplodingFutbol : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity( IHandleEntity *, int );  // line 66
	virtual TraceType_t GetTraceType() const;  // line 72
};

// game/server/portal2/prop_exploding_futbol.cpp:63 (declaration)
void CFilterOnlyExplodingFutbol();

// game/server/portal2/prop_exploding_futbol.cpp:66 @0x6e1f50 _ZN26CFilterOnlyExplodingFutbol15ShouldHitEntityEP13IHandleEntityi
bool CFilterOnlyExplodingFutbol::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	CBaseEntity *pEntity;  // line 68
	// inlined FClassnameIs() at line 69
	// inlined EntityFromEntityHandle() at line 68
}

// game/server/portal2/prop_exploding_futbol.cpp:72 @0x6e1ce0 _ZNK26CFilterOnlyExplodingFutbol12GetTraceTypeEv
TraceType_t CFilterOnlyExplodingFutbol::GetTraceType()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:79 sizeof=0x57c (i386)
struct CExplodingFutbolCatcher : public CBaseAnimating
{
public:
	CExplodingFutbolCatcher();  // line 84
	virtual void Spawn();  // line 86
	virtual void DrawDebugGeometryOverlays();  // line 88
protected:
	virtual void CatchThink();  // line 91
	virtual void CaptureThink();  // line 92
	virtual void CaptureFutbol( CPropExplodingFutbol * );  // line 94
	EHANDLE m_hCaughtFutbol; // +0x514  // line 96
	bool m_bDisableRecaptureOnPlayerGrab; // +0x518  // line 97
	Vector m_vecCatcherBoxMins; // +0x51c  // line 100
	Vector m_vecCatcherBoxMaxs; // +0x528  // line 101
	Vector m_vecCatcherBoxOrigin; // +0x534  // line 102
	Vector m_vecCatcherBoxHalfDiagonal; // +0x540  // line 103
	COutputEvent m_OnFutbolReleased; // +0x54c  // line 105
	COutputEvent m_OnFutbolCaught; // +0x564  // line 106
};

// game/server/portal2/prop_exploding_futbol.cpp:79 (declaration)
~CExplodingFutbolCatcher();

// game/server/portal2/prop_exploding_futbol.cpp:79 @0x6e1e60 _ZN23CExplodingFutbolCatcherD0Ev
CExplodingFutbolCatcher::~CExplodingFutbolCatcher()
{
	// inlined COutputEvent::~COutputEvent() at line 79
	// inlined COutputEvent::~COutputEvent() at line 79
	// inlined COutputEvent::~COutputEvent() at line 79
}

// game/server/portal2/prop_exploding_futbol.cpp:79 @0x6e1ee0 _ZN23CExplodingFutbolCatcherD1Ev
CExplodingFutbolCatcher::~CExplodingFutbolCatcher()
{
	// inlined COutputEvent::~COutputEvent() at line 79
	// inlined COutputEvent::~COutputEvent() at line 79
	// inlined COutputEvent::~COutputEvent() at line 79
}

// game/server/portal2/prop_exploding_futbol.cpp:82 @0x6de4c0 _ZN23CExplodingFutbolCatcher14GetDataDescMapEv
datamap_t *CExplodingFutbolCatcher::GetDataDescMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:82 @0x6de4d0 _ZN23CExplodingFutbolCatcher10GetBaseMapEv
datamap_t *CExplodingFutbolCatcher::GetBaseMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:84 @0x6df910 _ZN23CExplodingFutbolCatcherC2Ev
CExplodingFutbolCatcher::CExplodingFutbolCatcher()
{
	// inlined CHandle<CBaseEntity>::CHandle() at line 902
	// inlined Vector::Vector() at line 902
	// inlined COutputEvent::COutputEvent() at line 902
	// inlined COutputEvent::COutputEvent() at line 902
}

// game/server/portal2/prop_exploding_futbol.cpp:84 @0x6dfb20 _ZN23CExplodingFutbolCatcherC1Ev
CExplodingFutbolCatcher::CExplodingFutbolCatcher()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:86 @0x6de930 _ZN23CExplodingFutbolCatcher5SpawnEv
void CExplodingFutbolCatcher::Spawn()
{
	// inlined Vector::operator-() at line 914
	// inlined Vector::operator=() at line 914
	// inlined Vector::operator=() at line 915
}

// game/server/portal2/prop_exploding_futbol.cpp:88 @0x6de8c0 _ZN23CExplodingFutbolCatcher25DrawDebugGeometryOverlaysEv
void CExplodingFutbolCatcher::DrawDebugGeometryOverlays()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:91 @0x6df010 _ZN23CExplodingFutbolCatcher10CatchThinkEv
void CExplodingFutbolCatcher::CatchThink()
{
	Ray_t ray;  // line 933
	CFilterOnlyExplodingFutbol filter;  // line 935
	trace_t trace;  // line 936
	// inlined UTIL_TraceRay() at line 937
	// inlined CFilterOnlyExplodingFutbol::CFilterOnlyExplodingFutbol() at line 935
	// inlined Ray_t::Init() at line 934
	{
		CPropExplodingFutbol *pFutbol;  // line 943
		// inlined CPropExplodingFutbol::GetHolder() at line 945
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:92 @0x6dfe70 _ZN23CExplodingFutbolCatcher12CaptureThinkEv
void CExplodingFutbolCatcher::CaptureThink()
{
	CPropExplodingFutbol *pFutbol;  // line 1004
	// inlined CHandle<CBaseEntity>::Set() at line 1013
	// inlined CHandle<CBaseEntity>::Get() at line 1004
}

// game/server/portal2/prop_exploding_futbol.cpp:94 @0x6e19d0 _ZN23CExplodingFutbolCatcher13CaptureFutbolEP20CPropExplodingFutbol
void CExplodingFutbolCatcher::CaptureFutbol( CPropExplodingFutbol *pFutbol )
{
	IPhysicsObject *pPhysicsObject;  // line 973
	CPropExplodingFutbolSpawner *pSpawner;  // line 985
	// inlined CPropExplodingFutbol::GetLastPlayerToHold() at line 997
	// inlined CPropExplodingFutbol::StopFutbolTimer() at line 995
	// inlined CPropExplodingFutbol::GetSpawner() at line 985
	// inlined CPropExplodingFutbol::SetHolder() at line 982
	// inlined CBaseEntity::VPhysicsGetObject() at line 973
	// inlined CHandle<CBaseEntity>::Set() at line 970
	{
		CPropIndicatorPanel *pTimerPanel;  // line 988
	}
}

// game/server/util.h:99 @0x6e2110 _ZN14CEntityFactoryI23CExplodingFutbolCatcherE6CreateEPKc
IServerNetworkable *CEntityFactory<CExplodingFutbolCatcher>::Create( const char *pClassName )
{
	CExplodingFutbolCatcher *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CExplodingFutbolCatcher>() at line 101
}

// game/server/util.h:99 @0x6e2160 _ZN14CEntityFactoryI27CPropExplodingFutbolSpawnerE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropExplodingFutbolSpawner>::Create( const char *pClassName )
{
	CPropExplodingFutbolSpawner *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPropExplodingFutbolSpawner>() at line 101
}

// game/server/util.h:99 @0x6e21b0 _ZN14CEntityFactoryI26CPropExplodingFutbolSocketE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropExplodingFutbolSocket>::Create( const char *pClassName )
{
	CPropExplodingFutbolSocket *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPropExplodingFutbolSocket>() at line 101
}

// game/server/util.h:99 @0x6e2220 _ZN14CEntityFactoryI20CPropExplodingFutbolE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropExplodingFutbol>::Create( const char *pClassName )
{
	CPropExplodingFutbol *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPropExplodingFutbol>() at line 101
}

// game/server/util.h:105 @0x6e1cf0 _ZN14CEntityFactoryI20CPropExplodingFutbolE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropExplodingFutbol>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:105 @0x6e1d30 _ZN14CEntityFactoryI26CPropExplodingFutbolSocketE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropExplodingFutbolSocket>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:105 @0x6e1d70 _ZN14CEntityFactoryI27CPropExplodingFutbolSpawnerE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropExplodingFutbolSpawner>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:105 @0x6e1db0 _ZN14CEntityFactoryI23CExplodingFutbolCatcherE7DestroyEP18IServerNetworkable
void CEntityFactory<CExplodingFutbolCatcher>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:109 @0xc90d0 _Z11DataMapInitI23CExplodingFutbolCatcherEP9datamap_tPT_
datamap_t *DataMapInit<CExplodingFutbolCatcher>( CExplodingFutbolCatcher * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 123
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 123
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 109
	CDatadescGeneratedNameHolder nameHolder;  // line 109
	typedescription_t dataDesc[10];  // line 109
}

// game/server/portal2/prop_exploding_futbol.cpp:109
datamap_t *g_DataMapHolder;

// game/server/util.h:113 @0x6e1d20 _ZN14CEntityFactoryI20CPropExplodingFutbolE13GetEntitySizeEv
size_t CEntityFactory<CPropExplodingFutbol>::GetEntitySize()
{
}

// game/server/util.h:113 @0x6e1d60 _ZN14CEntityFactoryI26CPropExplodingFutbolSocketE13GetEntitySizeEv
size_t CEntityFactory<CPropExplodingFutbolSocket>::GetEntitySize()
{
}

// game/server/util.h:113 @0x6e1da0 _ZN14CEntityFactoryI27CPropExplodingFutbolSpawnerE13GetEntitySizeEv
size_t CEntityFactory<CPropExplodingFutbolSpawner>::GetEntitySize()
{
}

// game/server/util.h:113 @0x6e1de0 _ZN14CEntityFactoryI23CExplodingFutbolCatcherE13GetEntitySizeEv
size_t CEntityFactory<CExplodingFutbolCatcher>::GetEntitySize()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:125
static CEntityFactory<CExplodingFutbolCatcher> exploding_futbol_catcher;

// game/server/portal2/prop_exploding_futbol.cpp:127
enum ExplodingFutbolHolderType_t
{
	EXPLODING_FUTBOL_HELD_BY_NONE = 0,
	EXPLODING_FUTBOL_HELD_BY_PLAYER = 1,
	EXPLODING_FUTBOL_HELD_BY_SPAWNER = 2,
	EXPLODING_FUTBOL_HELD_BY_CATCHER = 3,
	EXPLODING_FUTBOL_HELD_BY_COUNT = 4,
};

// game/server/portal2/prop_exploding_futbol.cpp:140 sizeof=0x5b8 (i386)
struct CPropExplodingFutbolSpawner : public CExplodingFutbolCatcher
{
public:
	CPropExplodingFutbolSpawner();  // line 145
	virtual void Spawn();  // line 147
	virtual void Precache();  // line 148
	virtual void FutbolDestroyed();  // line 150
	virtual void FutbolGrabbed( CPropExplodingFutbol *, bool );  // line 151
	CPropIndicatorPanel *GetTimerPanel();  // line 153
private:
	virtual void SpawnFutbol();  // line 156
	void InputForceSpawn( inputdata_t & );  // line 158
	COutputEvent m_OnFutbolSpawned; // +0x57c  // line 161
	COutputEvent m_OnFutbolGrabbed; // +0x594  // line 162
	bool m_bHasFutbol; // +0x5ac  // line 164
	bool m_bIsTimed; // +0x5ad  // line 165
	float m_flTimer; // +0x5b0  // line 166
	string_t m_strTimerIndicator; // +0x5b4  // line 167
};

// game/server/portal2/prop_exploding_futbol.cpp:140 (declaration)
~CPropExplodingFutbolSpawner();

// game/server/portal2/prop_exploding_futbol.cpp:140 @0x6e23c0 _ZN27CPropExplodingFutbolSpawnerD0Ev
CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner()
{
	// inlined COutputEvent::~COutputEvent() at line 140
	// inlined COutputEvent::~COutputEvent() at line 140
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 140
	// inlined COutputEvent::~COutputEvent() at line 140
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 140
}

// game/server/portal2/prop_exploding_futbol.cpp:140 @0x6e24b0 _ZN27CPropExplodingFutbolSpawnerD1Ev
CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner()
{
	// inlined COutputEvent::~COutputEvent() at line 140
	// inlined COutputEvent::~COutputEvent() at line 140
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 140
	// inlined COutputEvent::~COutputEvent() at line 140
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 140
}

// game/server/portal2/prop_exploding_futbol.cpp:143 @0x6de4e0 _ZN27CPropExplodingFutbolSpawner14GetDataDescMapEv
datamap_t *CPropExplodingFutbolSpawner::GetDataDescMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:143 @0x6de4f0 _ZN27CPropExplodingFutbolSpawner10GetBaseMapEv
datamap_t *CPropExplodingFutbolSpawner::GetBaseMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:145 @0x6dfa70 _ZN27CPropExplodingFutbolSpawnerC2Ev
CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner()
{
	// inlined COutputEvent::COutputEvent() at line 1025
	// inlined COutputEvent::COutputEvent() at line 1025
	// inlined Vector::Vector() at line 1029
	// inlined Vector::operator=() at line 1029
}

// game/server/portal2/prop_exploding_futbol.cpp:145 @0x6dfb10 _ZN27CPropExplodingFutbolSpawnerC1Ev
CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:147 @0x6dec60 _ZN27CPropExplodingFutbolSpawner5SpawnEv
void CPropExplodingFutbolSpawner::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 1047
}

// game/server/portal2/prop_exploding_futbol.cpp:148 @0x6de730 _ZN27CPropExplodingFutbolSpawner8PrecacheEv
void CPropExplodingFutbolSpawner::Precache()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:150 @0x6de830 _ZN27CPropExplodingFutbolSpawner15FutbolDestroyedEv
void CPropExplodingFutbolSpawner::FutbolDestroyed()
{
	CPropIndicatorPanel *pTimerPanel;  // line 1061
}

// game/server/portal2/prop_exploding_futbol.cpp:151 @0x6e17c0 _ZN27CPropExplodingFutbolSpawner13FutbolGrabbedEP20CPropExplodingFutbolb
void CPropExplodingFutbolSpawner::FutbolGrabbed( CPropExplodingFutbol *pFutbol, bool bSamePlayer )
{
	{
		CPropIndicatorPanel *pTimerPanel;  // line 1078
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:153 @0x6de770 _ZN27CPropExplodingFutbolSpawner13GetTimerPanelEv
CPropIndicatorPanel *CPropExplodingFutbolSpawner::GetTimerPanel()
{
	{
		CBaseEntity *pEnt;  // line 1106
		{
			CPropIndicatorPanel *pTimerPanel;  // line 1112
			// inlined CBaseEntity::GetClassname() at line 1119
		}
	}
	// inlined string_t::operator!=() at line 1103
}

// game/server/portal2/prop_exploding_futbol.cpp:156 @0x6dece0 _ZN27CPropExplodingFutbolSpawner11SpawnFutbolEv
void CPropExplodingFutbolSpawner::SpawnFutbol()
{
	CPropExplodingFutbol *pFutbol;  // line 1136
	Vector vecFutbolSpawnPoint;  // line 1147
	// inlined CPropExplodingFutbol::SetHolder() at line 1151
}

// game/server/portal2/prop_exploding_futbol.cpp:158 @0x6de6d0 _ZN27CPropExplodingFutbolSpawner15InputForceSpawnER11inputdata_t
void CPropExplodingFutbolSpawner::InputForceSpawn( inputdata_t &data )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:170 @0xc8fa0 _Z11DataMapInitI27CPropExplodingFutbolSpawnerEP9datamap_tPT_
datamap_t *DataMapInit<CPropExplodingFutbolSpawner>( CPropExplodingFutbolSpawner * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 170
	CDatadescGeneratedNameHolder nameHolder;  // line 170
	typedescription_t dataDesc[8];  // line 170
}

// game/server/portal2/prop_exploding_futbol.cpp:170
datamap_t *g_DataMapHolder;

// game/server/portal2/prop_exploding_futbol.cpp:184
static CEntityFactory<CPropExplodingFutbolSpawner> prop_exploding_futbol_spawner;

// game/server/portal2/prop_exploding_futbol.cpp:188 sizeof=0x57c (i386)
struct CPropExplodingFutbolSocket : public CExplodingFutbolCatcher
{
public:
	CPropExplodingFutbolSocket();  // line 192
	virtual void Precache();  // line 194
	virtual void Spawn();  // line 195
	virtual void CaptureFutbol( CPropExplodingFutbol * );  // line 197
};

// game/server/portal2/prop_exploding_futbol.cpp:188 (declaration)
~CPropExplodingFutbolSocket();

// game/server/portal2/prop_exploding_futbol.cpp:188 @0x6e1df0 _ZN26CPropExplodingFutbolSocketD1Ev
CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket()
{
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 188
}

// game/server/portal2/prop_exploding_futbol.cpp:188 @0x6e2590 _ZN26CPropExplodingFutbolSocketD0Ev
CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket()
{
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 188
}

// game/server/portal2/prop_exploding_futbol.cpp:192 @0x6df9f0 _ZN26CPropExplodingFutbolSocketC1Ev
CPropExplodingFutbolSocket::CPropExplodingFutbolSocket()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:192 @0x6dfa30 _ZN26CPropExplodingFutbolSocketC2Ev
CPropExplodingFutbolSocket::CPropExplodingFutbolSocket()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:194 @0x6de6f0 _ZN26CPropExplodingFutbolSocket8PrecacheEv
void CPropExplodingFutbolSocket::Precache()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:195 @0x6dea60 _ZN26CPropExplodingFutbolSocket5SpawnEv
void CPropExplodingFutbolSocket::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 1181
}

// game/server/portal2/prop_exploding_futbol.cpp:197 @0x6e1c80 _ZN26CPropExplodingFutbolSocket13CaptureFutbolEP20CPropExplodingFutbol
void CPropExplodingFutbolSocket::CaptureFutbol( CPropExplodingFutbol *pFutbol )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:200
static CEntityFactory<CPropExplodingFutbolSocket> prop_exploding_futbol_socket;

// game/server/portal2/prop_exploding_futbol.cpp:204 sizeof=0x7a4 (i386)
struct CPropExplodingFutbol : public CPhysicsProp
{
public:
	CPropExplodingFutbol();  // line 209
	virtual void Precache();  // line 211
	virtual void Spawn();  // line 212
	virtual void Event_Killed( const CTakeDamageInfo & );  // line 214
	ExplodingFutbolHolderType_t GetHolder();  // line 217
	void SetHolder( ExplodingFutbolHolderType_t );  // line 218
	CPortal_Player *GetLastPlayerToHold();  // line 219
	virtual void SetSpawner( CPropExplodingFutbolSpawner * );  // line 220
	CPropExplodingFutbolSpawner *GetSpawner();  // line 221
	void KillThink();  // line 223
	void AnimThink();  // line 224
	void TimerThink();  // line 225
	void ActivateFutbolTimer( float );  // line 227
	void StopFutbolTimer();  // line 228
	virtual void OnPhysGunPickup( CBasePlayer *, PhysGunPickup_t );  // line 231
	virtual void OnPhysGunDrop( CBasePlayer *, PhysGunDrop_t );  // line 232
	virtual int OnTakeDamage( const CTakeDamageInfo & );  // line 234
	virtual void OnFizzled();  // line 235
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 236
private:
	void KillFutbol();  // line 239
	void DestroyFutbol( bool );  // line 240
	void ExplodeFutbol();  // line 241
	void DoExplosion( const Vector &, float, CPortal_Base2D * );  // line 242
	void DamageEntity( CBaseEntity *, const Vector &, const Vector & );  // line 243
	void InputExplode( inputdata_t & );  // line 245
	ExplodingFutbolHolderType_t m_Holder; // +0x768  // line 247
	CHandle<CPropExplodingFutbolSpawner> m_hSpawner; // +0x76c  // line 248
	string_t m_strSpawnerName; // +0x770  // line 249
	CHandle<CPortal_Player> m_hLastHeldByPlayer; // +0x774  // line 250
	CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> > m_ExplodedEntities; // +0x778  // line 251
	bool m_bShouldRespawn; // +0x78c  // line 253
	bool m_bTimerActive; // +0x78d  // line 254
	bool m_bExplodeOnTouch; // +0x78e  // line 255
	float m_flExplosionTimer; // +0x790  // line 256
	float m_flTotalTimer; // +0x794  // line 257
	float m_flLastTickTime; // +0x798  // line 258
	float m_flLastTimerSoundTime; // +0x79c  // line 259
	float m_flLastFlashTime; // +0x7a0  // line 260
};

// game/server/portal2/prop_exploding_futbol.cpp:204 (declaration)
~CPropExplodingFutbol();

// game/server/portal2/prop_exploding_futbol.cpp:204 @0x6e1fd0 _ZN20CPropExplodingFutbolD1Ev
CPropExplodingFutbol::~CPropExplodingFutbol()
{
	// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::~CUtlVector() at line 204
}

// game/server/portal2/prop_exploding_futbol.cpp:204 @0x6e2270 _ZN20CPropExplodingFutbolD0Ev
CPropExplodingFutbol::~CPropExplodingFutbol()
{
	// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::~CUtlVector() at line 204
}

// game/server/portal2/prop_exploding_futbol.cpp:207 @0x6de500 _ZN20CPropExplodingFutbol14GetDataDescMapEv
datamap_t *CPropExplodingFutbol::GetDataDescMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:207 @0x6de510 _ZN20CPropExplodingFutbol10GetBaseMapEv
datamap_t *CPropExplodingFutbol::GetBaseMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:209 @0x6dfd80 _ZN20CPropExplodingFutbolC2Ev
CPropExplodingFutbol::CPropExplodingFutbol()
{
	// inlined CHandle<CPropExplodingFutbolSpawner>::CHandle() at line 298
	// inlined CHandle<CPortal_Player>::CHandle() at line 298
	// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::CUtlVector() at line 298
}

// game/server/portal2/prop_exploding_futbol.cpp:209 @0x6dfe60 _ZN20CPropExplodingFutbolC1Ev
CPropExplodingFutbol::CPropExplodingFutbol()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:211 @0x6dec00 _ZN20CPropExplodingFutbol8PrecacheEv
void CPropExplodingFutbol::Precache()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:212 @0x6dede0 _ZN20CPropExplodingFutbol5SpawnEv
void CPropExplodingFutbol::Spawn()
{
	// inlined CBaseEntity::AddSpawnFlags() at line 358
	{
		CBaseEntity *pEnt;  // line 325
		// inlined string_t::operator!=() at line 326
		{
			CPropExplodingFutbolSpawner *pSpawner;  // line 332
			// inlined CHandle<CPropExplodingFutbolSpawner>::Set() at line 335
			// inlined CBaseEntity::GetClassname() at line 339
		}
	}
	// inlined CHandle<CPropExplodingFutbolSpawner>::Get() at line 323
}

// game/server/portal2/prop_exploding_futbol.cpp:214 @0x6e18a0 _ZN20CPropExplodingFutbol12Event_KilledERK15CTakeDamageInfo
void CPropExplodingFutbol::Event_Killed( const CTakeDamageInfo &info )
{
	// inlined CPropExplodingFutbol::DestroyFutbol() at line 435
}

// game/server/portal2/prop_exploding_futbol.cpp:217 @0x6de520 _ZN20CPropExplodingFutbol9GetHolderEv
ExplodingFutbolHolderType_t CPropExplodingFutbol::GetHolder()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:218 @0x6de530 _ZN20CPropExplodingFutbol9SetHolderE27ExplodingFutbolHolderType_t
void CPropExplodingFutbol::SetHolder( ExplodingFutbolHolderType_t holder )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:219 @0x6de550 _ZN20CPropExplodingFutbol19GetLastPlayerToHoldEv
CPortal_Player *CPropExplodingFutbol::GetLastPlayerToHold()
{
	// inlined CHandle<CPortal_Player>::Get() at line 376
}

// game/server/portal2/prop_exploding_futbol.cpp:220 @0x6dfd30 _ZN20CPropExplodingFutbol10SetSpawnerEP27CPropExplodingFutbolSpawner
void CPropExplodingFutbol::SetSpawner( CPropExplodingFutbolSpawner *pSpawner )
{
	// inlined CHandle<CPropExplodingFutbolSpawner>::Set() at line 383
}

// game/server/portal2/prop_exploding_futbol.cpp:221 @0x6de5a0 _ZN20CPropExplodingFutbol10GetSpawnerEv
CPropExplodingFutbolSpawner *CPropExplodingFutbol::GetSpawner()
{
	// inlined CHandle<CPropExplodingFutbolSpawner>::operator CPropExplodingFutbolSpawner*() at line 390
}

// game/server/portal2/prop_exploding_futbol.cpp:223 @0x6df8f0 _ZN20CPropExplodingFutbol9KillThinkEv
void CPropExplodingFutbol::KillThink()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:224 @0x6de860 _ZN20CPropExplodingFutbol9AnimThinkEv
void CPropExplodingFutbol::AnimThink()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:225 @0x6e0e60 _ZN20CPropExplodingFutbol10TimerThinkEv
void CPropExplodingFutbol::TimerThink()
{
	float flDeltaTime;  // line 830
	float flSoundDelay;  // line 835
	string_t strSoundName;  // line 836
	Color startColor;  // line 838
	Color endColor;  // line 838
	Color currentColor;  // line 838
	float flPercent;  // line 839
	// inlined CBaseEntity::SetRenderColor() at line 877
	// inlined Color::r() at line 877
	// inlined Color::g() at line 877
	// inlined Color::b() at line 877
	// inlined Color::operator=() at line 876
	// inlined ColorLerp() at line 876
	// inlined string_t::ToCStr() at line 871
	// inlined Color::operator=() at line 844
	// inlined ConVar::GetColor() at line 844
	// inlined Color::operator=() at line 843
	// inlined ConVar::GetColor() at line 843
	// inlined Color::Color() at line 838
	// inlined Color::Color() at line 838
	// inlined Color::Color() at line 838
	{
		float flCurrentFlashDuration;  // line 853
		// inlined ConVar::GetFloat() at line 851
		// inlined ConVar::GetColor() at line 863
		// inlined Color::operator=() at line 863
		// inlined ConVar::GetColor() at line 864
		// inlined Color::operator=() at line 864
	}
	// inlined CPropExplodingFutbol::StopFutbolTimer() at line 886
}

// game/server/portal2/prop_exploding_futbol.cpp:227 @0x6e15d0 _ZN20CPropExplodingFutbol19ActivateFutbolTimerEf
void CPropExplodingFutbol::ActivateFutbolTimer( float flTimer )
{
	{
		Color futbolColor;  // line 669
		// inlined CBaseEntity::SetRenderColor() at line 670
		// inlined ConVar::GetColor() at line 669
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:228 @0x6e1470 _ZN20CPropExplodingFutbol15StopFutbolTimerEv
void CPropExplodingFutbol::StopFutbolTimer()
{
	Color futbolColor;  // line 682
	// inlined CBaseEntity::SetRenderColor() at line 683
	// inlined ConVar::GetColor() at line 682
}

// game/server/portal2/prop_exploding_futbol.cpp:231 @0x6dfb40 _ZN20CPropExplodingFutbol15OnPhysGunPickupEP11CBasePlayer15PhysGunPickup_t
void CPropExplodingFutbol::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	CPortal_Player *pHoldingPlayer;  // line 690
	IPhysicsObject *pPhysicsObject;  // line 693
	bool bSamePlayer;  // line 709
	// inlined CHandle<CPortal_Player>::operator=() at line 721
	// inlined CHandle<CPropExplodingFutbolSpawner>::Get() at line 716
	// inlined CPropExplodingFutbol::GetLastPlayerToHold() at line 711
	// inlined CBaseEntity::VPhysicsGetObject() at line 693
	// inlined ToPortalPlayer() at line 690
	{
		CBasePlayer *pOtherPlayer;  // line 725
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:232 @0x6debb0 _ZN20CPropExplodingFutbol13OnPhysGunDropEP11CBasePlayer13PhysGunDrop_t
void CPropExplodingFutbol::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:234 @0x6deb60 _ZN20CPropExplodingFutbol12OnTakeDamageERK15CTakeDamageInfo
int CPropExplodingFutbol::OnTakeDamage( const CTakeDamageInfo &info )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:235 @0x6e1930 _ZN20CPropExplodingFutbol9OnFizzledEv
void CPropExplodingFutbol::OnFizzled()
{
	// inlined CPropExplodingFutbol::DestroyFutbol() at line 776
}

// game/server/portal2/prop_exploding_futbol.cpp:236 @0x6dead0 _ZN20CPropExplodingFutbol17VPhysicsCollisionEiP21gamevcollisionevent_t
void CPropExplodingFutbol::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:239 @0x6df7f0 _ZN20CPropExplodingFutbol10KillFutbolEv
void CPropExplodingFutbol::KillFutbol()
{
	CTakeDamageInfo info;  // line 402
	// inlined CTakeDamageInfo::SetDamageForce() at line 405
	// inlined Vector::operator*() at line 405
	// inlined CBaseEntity::Forward() at line 405
	// inlined CTakeDamageInfo::SetDamagePosition() at line 404
	// inlined CBaseEntity::GetAbsOrigin() at line 404
}

// game/server/portal2/prop_exploding_futbol.cpp:240 @0x6e0de0 _ZN20CPropExplodingFutbol13DestroyFutbolEb
void CPropExplodingFutbol::DestroyFutbol( bool bExplode )
{
	// inlined CHandle<CPropExplodingFutbolSpawner>::Get() at line 423
}

// game/server/portal2/prop_exploding_futbol.cpp:241 @0x6e0b80 _ZN20CPropExplodingFutbol13ExplodeFutbolEv
void CPropExplodingFutbol::ExplodeFutbol()
{
	float flExplosionRadius;  // line 441
	PortalRadiusExtensionVector portalRadiusExtensions;  // line 454
	{
		int i;  // line 456
		// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::operator[]() at line 458
	}
	// inlined CBaseEntity::GetAbsAngles() at line 455
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::CUtlVector() at line 454
	// inlined CBaseEntity::GetAbsOrigin() at line 451
	// inlined CBaseEntity::GetAbsAngles() at line 451
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::~CUtlVector() at line 459
	// inlined CBaseEntity::GetOwnerEntity() at line 451
	// inlined ConVar::GetInt() at line 451
	// inlined ConVar::GetFloat() at line 451
	// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::RemoveAll() at line 444
	// inlined ConVar::GetFloat() at line 441
	// inlined CBaseEntity::GetAbsOrigin() at line 455
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::~CUtlVector() at line 459
}

// game/server/portal2/prop_exploding_futbol.cpp:242 @0x6dff50 _ZN20CPropExplodingFutbol11DoExplosionERK6VectorfP14CPortal_Base2D
void CPropExplodingFutbol::DoExplosion( const Vector &vecCenter, float flExplosionRadius, CPortal_Base2D *pPortal )
{
	bool bDebugging;  // line 515
	Color debugColor;  // line 516
	CBaseEntity *pEnts[128];  // line 522
	int nNumFound;  // line 523
	{
		int i;  // line 524
		{
			bool bTestEntity;  // line 532
			int x;  // line 533
			int iExplodedEntitiesCount;  // line 553
			bool bSuccess;  // line 563
			// inlined FClassnameIs() at line 536
			{
				Vector vecRayStart;  // line 568
				Vector vecRayEnd;  // line 569
				CTraceFilterSimpleList traceFilter;  // line 572
				// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::AddToTail() at line 643
				// inlined CTraceFilterSimpleList::~CTraceFilterSimpleList() at line 643
				// inlined CBaseEntity::GetAbsOrigin() at line 569
				{
					int j;  // line 574
				}
				{
					CPortal_Base2D *pInPortal;  // line 599
					Vector vecEndPos;  // line 600
					Ray_t portalRay;  // line 604
					trace_t trace;  // line 606
					bool bPortalTrace;  // line 607
					// inlined CBaseEntity::GetAbsOrigin() at line 624
					// inlined CPortal_Base2D::GetLinkedPortal() at line 599
					// inlined Ray_t::Ray_t() at line 604
					// inlined CBaseEntity::GetAbsOrigin() at line 605
					// inlined Ray_t::Init() at line 605
					{
						Vector vecForward;  // line 613
						// inlined CBaseEntity::GetAbsOrigin() at line 612
						// inlined CBaseEntity::GetAbsOrigin() at line 613
						// inlined Vector::operator-() at line 613
					}
				}
				{
					Color lineColor;  // line 630
					// inlined Color::Color() at line 630
					// inlined Color::Color() at line 634
					// inlined Color::operator=() at line 634
				}
				{
					Ray_t entRay;  // line 582
					trace_t trace;  // line 584
					// inlined Vector::operator VectorByValue&() at line 591
					// inlined CBaseEntity::Forward() at line 591
					// inlined UTIL_TraceRay() at line 585
					// inlined Ray_t::Init() at line 583
				}
				// inlined CTraceFilterSimpleList::~CTraceFilterSimpleList() at line 643
			}
			// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::Count() at line 553
			{
				int j;  // line 554
				// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::operator[]() at line 556
			}
		}
	}
	// inlined UTIL_EntitiesInSphere() at line 523
	// inlined Color::operator=() at line 519
	// inlined Color::Color() at line 519
	// inlined Color::Color() at line 516
}

// game/server/portal2/prop_exploding_futbol.cpp:243 @0x6df2d0 _ZN20CPropExplodingFutbol12DamageEntityEP11CBaseEntityRK6VectorS4_
void CPropExplodingFutbol::DamageEntity( CBaseEntity *pEntity, const Vector &vecCenter, const Vector &vecForward )
{
	Vector vecDamagePos;  // line 465
	{
		float flDist;  // line 470
		float flBaseDamage;  // line 473
		float flPercent;  // line 474
		float flDamage;  // line 475
		CTakeDamageInfo info;  // line 484
		{
			CFmtStr msg;  // line 479
			// inlined CBaseEntity::GetAbsOrigin() at line 481
			// inlined CFmtStrN<256>::CFmtStrN() at line 479
		}
		// inlined CBaseEntity::GetAbsOrigin() at line 470
		// inlined Vector::DistTo() at line 470
		// inlined ConVar::GetFloat() at line 473
		// inlined Lerp<float>() at line 475
		// inlined CTakeDamageInfo::SetDamagePosition() at line 485
	}
	// inlined Vector::operator-() at line 465
	// inlined Vector::operator*() at line 465
	// inlined FClassnameIs() at line 492
	{
		CTakeDamageInfo info;  // line 503
		// inlined Vector::operator VectorByValue&() at line 505
		// inlined Vector::operator-() at line 505
		// inlined CBaseEntity::GetAbsOrigin() at line 505
		// inlined CTakeDamageInfo::SetDamagePosition() at line 504
	}
	{
		CTakeDamageInfo info;  // line 494
		// inlined CTakeDamageInfo::SetDamagePosition() at line 495
		// inlined ConVar::GetFloat() at line 496
		// inlined Vector::operator*() at line 496
		// inlined CTakeDamageInfo::SetDamageForce() at line 496
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:245 @0x6df900 _ZN20CPropExplodingFutbol12InputExplodeER11inputdata_t
void CPropExplodingFutbol::InputExplode( inputdata_t &in )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:263 @0xc93b0 _Z11DataMapInitI20CPropExplodingFutbolEP9datamap_tPT_
datamap_t *DataMapInit<CPropExplodingFutbol>( CPropExplodingFutbol * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 284
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 284
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 284
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 263
	CDatadescGeneratedNameHolder nameHolder;  // line 263
	typedescription_t dataDesc[17];  // line 263
}

// game/server/portal2/prop_exploding_futbol.cpp:263
datamap_t *g_DataMapHolder;

// game/server/portal2/prop_exploding_futbol.cpp:286
static CEntityFactory<CPropExplodingFutbol> prop_exploding_futbol;

// game/server/portal2/prop_exploding_futbol.cpp:811 (declaration)
Color ColorLerp( float flPercent, const Color &startColor, const Color &endColor );

// game/server/portal2/prop_exploding_futbol.cpp:811 @0x6de5f0 _Z9ColorLerpfRK5ColorS1_
Color ColorLerp( float flPercent, const Color &startColor, const Color &endColor )
{
	float r;  // line 813
	float g;  // line 814
	float b;  // line 815
	float a;  // line 816
	// inlined Color::r() at line 813
	// inlined Color::g() at line 814
	// inlined Color::b() at line 815
	// inlined Color::a() at line 816
	// inlined Color::Color() at line 818
}
