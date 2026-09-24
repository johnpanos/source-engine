// DWARF declaration skeleton for game/server/portal2/prop_exploding_futbol.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb4900 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CExplodingFutbolCatcher>::CEntityFactory() at line 123
	// inlined CEntityFactory<CPropExplodingFutbolSpawner>::CEntityFactory() at line 171
	// inlined CEntityFactory<CPropExplodingFutbolSocket>::CEntityFactory() at line 187
	// inlined CEntityFactory<CPropExplodingFutbol>::CEntityFactory() at line 214
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

// game/server/portal2/prop_exploding_futbol.cpp:44
ConVar sv_futbol_funnel_max_correct;

// game/server/portal2/prop_exploding_futbol.cpp:47
static const char *g_psxExplodableEntities[6];

// game/server/portal2/prop_exploding_futbol.cpp:61 sizeof=0x4 (i386)
struct CFilterOnlyExplodingFutbol : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity( IHandleEntity *, int );  // line 64
	virtual TraceType_t GetTraceType() const;  // line 70
};

// game/server/portal2/prop_exploding_futbol.cpp:61 (declaration)
void CFilterOnlyExplodingFutbol();

// game/server/portal2/prop_exploding_futbol.cpp:64 @0x755ab0 _ZN26CFilterOnlyExplodingFutbol15ShouldHitEntityEP13IHandleEntityi
bool CFilterOnlyExplodingFutbol::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	CBaseEntity *pEntity;  // line 66
	// inlined FClassnameIs() at line 67
	// inlined EntityFromEntityHandle() at line 66
}

// game/server/portal2/prop_exploding_futbol.cpp:70 @0x755840 _ZNK26CFilterOnlyExplodingFutbol12GetTraceTypeEv
TraceType_t CFilterOnlyExplodingFutbol::GetTraceType()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:77 sizeof=0x57c (i386)
struct CExplodingFutbolCatcher : public CBaseAnimating
{
public:
	CExplodingFutbolCatcher();  // line 82
	virtual void Spawn();  // line 84
	virtual void DrawDebugGeometryOverlays();  // line 86
protected:
	virtual void CatchThink();  // line 89
	virtual void CaptureThink();  // line 90
	virtual void CaptureFutbol( CPropExplodingFutbol * );  // line 92
	EHANDLE m_hCaughtFutbol; // +0x514  // line 94
	bool m_bDisableRecaptureOnPlayerGrab; // +0x518  // line 95
	Vector m_vecCatcherBoxMins; // +0x51c  // line 98
	Vector m_vecCatcherBoxMaxs; // +0x528  // line 99
	Vector m_vecCatcherBoxOrigin; // +0x534  // line 100
	Vector m_vecCatcherBoxHalfDiagonal; // +0x540  // line 101
	COutputEvent m_OnFutbolReleased; // +0x54c  // line 103
	COutputEvent m_OnFutbolCaught; // +0x564  // line 104
};

// game/server/portal2/prop_exploding_futbol.cpp:77 (declaration)
~CExplodingFutbolCatcher();

// game/server/portal2/prop_exploding_futbol.cpp:77 @0x7559c0 _ZN23CExplodingFutbolCatcherD0Ev
CExplodingFutbolCatcher::~CExplodingFutbolCatcher()
{
	// inlined COutputEvent::~COutputEvent() at line 77
	// inlined COutputEvent::~COutputEvent() at line 77
	// inlined COutputEvent::~COutputEvent() at line 77
}

// game/server/portal2/prop_exploding_futbol.cpp:77 @0x755a40 _ZN23CExplodingFutbolCatcherD1Ev
CExplodingFutbolCatcher::~CExplodingFutbolCatcher()
{
	// inlined COutputEvent::~COutputEvent() at line 77
	// inlined COutputEvent::~COutputEvent() at line 77
	// inlined COutputEvent::~COutputEvent() at line 77
}

// game/server/portal2/prop_exploding_futbol.cpp:80 @0x750830 _ZN23CExplodingFutbolCatcher14GetDataDescMapEv
datamap_t *CExplodingFutbolCatcher::GetDataDescMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:80 @0x750840 _ZN23CExplodingFutbolCatcher10GetBaseMapEv
datamap_t *CExplodingFutbolCatcher::GetBaseMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:82 @0x751540 _ZN23CExplodingFutbolCatcherC2Ev
CExplodingFutbolCatcher::CExplodingFutbolCatcher()
{
	// inlined CHandle<CBaseEntity>::CHandle() at line 912
	// inlined Vector::Vector() at line 912
	// inlined COutputEvent::COutputEvent() at line 912
	// inlined COutputEvent::COutputEvent() at line 912
}

// game/server/portal2/prop_exploding_futbol.cpp:82 @0x751750 _ZN23CExplodingFutbolCatcherC1Ev
CExplodingFutbolCatcher::CExplodingFutbolCatcher()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:84 @0x750ca0 _ZN23CExplodingFutbolCatcher5SpawnEv
void CExplodingFutbolCatcher::Spawn()
{
	// inlined Vector::operator-() at line 924
	// inlined Vector::operator=() at line 924
	// inlined Vector::operator=() at line 925
}

// game/server/portal2/prop_exploding_futbol.cpp:86 @0x750c30 _ZN23CExplodingFutbolCatcher25DrawDebugGeometryOverlaysEv
void CExplodingFutbolCatcher::DrawDebugGeometryOverlays()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:89 @0x750fe0 _ZN23CExplodingFutbolCatcher10CatchThinkEv
void CExplodingFutbolCatcher::CatchThink()
{
	Ray_t ray;  // line 943
	CFilterOnlyExplodingFutbol filter;  // line 945
	trace_t trace;  // line 946
	// inlined UTIL_TraceRay() at line 947
	// inlined CFilterOnlyExplodingFutbol::CFilterOnlyExplodingFutbol() at line 945
	// inlined Ray_t::Init() at line 944
	{
		CPropExplodingFutbol *pFutbol;  // line 953
		// inlined CPropExplodingFutbol::GetHolder() at line 955
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:90 @0x751460 _ZN23CExplodingFutbolCatcher12CaptureThinkEv
void CExplodingFutbolCatcher::CaptureThink()
{
	CPropExplodingFutbol *pFutbol;  // line 1014
	// inlined CHandle<CBaseEntity>::Set() at line 1023
	// inlined CHandle<CBaseEntity>::Get() at line 1014
}

// game/server/portal2/prop_exploding_futbol.cpp:92 @0x753700 _ZN23CExplodingFutbolCatcher13CaptureFutbolEP20CPropExplodingFutbol
void CExplodingFutbolCatcher::CaptureFutbol( CPropExplodingFutbol *pFutbol )
{
	IPhysicsObject *pPhysicsObject;  // line 983
	CPropExplodingFutbolSpawner *pSpawner;  // line 995
	// inlined CPropExplodingFutbol::GetLastPlayerToHold() at line 1007
	// inlined CPropExplodingFutbol::StopFutbolTimer() at line 1005
	// inlined CPropExplodingFutbol::GetSpawner() at line 995
	// inlined CPropExplodingFutbol::SetHolder() at line 992
	// inlined CBaseEntity::VPhysicsGetObject() at line 983
	// inlined CHandle<CBaseEntity>::Set() at line 980
	{
		CPropIndicatorPanel *pTimerPanel;  // line 998
	}
}

// game/server/util.h:100 @0x755c70 _ZN14CEntityFactoryI23CExplodingFutbolCatcherE6CreateEPKc
IServerNetworkable *CEntityFactory<CExplodingFutbolCatcher>::Create( const char *pClassName )
{
	CExplodingFutbolCatcher *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CExplodingFutbolCatcher>() at line 102
}

// game/server/util.h:100 @0x755cc0 _ZN14CEntityFactoryI27CPropExplodingFutbolSpawnerE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropExplodingFutbolSpawner>::Create( const char *pClassName )
{
	CPropExplodingFutbolSpawner *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropExplodingFutbolSpawner>() at line 102
}

// game/server/util.h:100 @0x755d10 _ZN14CEntityFactoryI26CPropExplodingFutbolSocketE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropExplodingFutbolSocket>::Create( const char *pClassName )
{
	CPropExplodingFutbolSocket *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropExplodingFutbolSocket>() at line 102
}

// game/server/util.h:100 @0x755d80 _ZN14CEntityFactoryI20CPropExplodingFutbolE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropExplodingFutbol>::Create( const char *pClassName )
{
	CPropExplodingFutbol *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropExplodingFutbol>() at line 102
}

// game/server/util.h:106 @0x755850 _ZN14CEntityFactoryI20CPropExplodingFutbolE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropExplodingFutbol>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x755890 _ZN14CEntityFactoryI26CPropExplodingFutbolSocketE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropExplodingFutbolSocket>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x7558d0 _ZN14CEntityFactoryI27CPropExplodingFutbolSpawnerE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropExplodingFutbolSpawner>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:106 @0x755910 _ZN14CEntityFactoryI23CExplodingFutbolCatcherE7DestroyEP18IServerNetworkable
void CEntityFactory<CExplodingFutbolCatcher>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:107 @0xb3d70 _Z11DataMapInitI23CExplodingFutbolCatcherEP9datamap_tPT_
datamap_t *DataMapInit<CExplodingFutbolCatcher>( CExplodingFutbolCatcher * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 121
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 121
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 107
	CDatadescGeneratedNameHolder nameHolder;  // line 107
	typedescription_t dataDesc[10];  // line 107
}

// game/server/portal2/prop_exploding_futbol.cpp:107
datamap_t *g_DataMapHolder;

// game/server/util.h:114 @0x755880 _ZN14CEntityFactoryI20CPropExplodingFutbolE13GetEntitySizeEv
size_t CEntityFactory<CPropExplodingFutbol>::GetEntitySize()
{
}

// game/server/util.h:114 @0x7558c0 _ZN14CEntityFactoryI26CPropExplodingFutbolSocketE13GetEntitySizeEv
size_t CEntityFactory<CPropExplodingFutbolSocket>::GetEntitySize()
{
}

// game/server/util.h:114 @0x755900 _ZN14CEntityFactoryI27CPropExplodingFutbolSpawnerE13GetEntitySizeEv
size_t CEntityFactory<CPropExplodingFutbolSpawner>::GetEntitySize()
{
}

// game/server/util.h:114 @0x755940 _ZN14CEntityFactoryI23CExplodingFutbolCatcherE13GetEntitySizeEv
size_t CEntityFactory<CExplodingFutbolCatcher>::GetEntitySize()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:123
static CEntityFactory<CExplodingFutbolCatcher> exploding_futbol_catcher;

// game/server/portal2/prop_exploding_futbol.cpp:127 sizeof=0x5b8 (i386)
struct CPropExplodingFutbolSpawner : public CExplodingFutbolCatcher
{
public:
	CPropExplodingFutbolSpawner();  // line 132
	virtual void Spawn();  // line 134
	virtual void Precache();  // line 135
	virtual void FutbolDestroyed();  // line 137
	virtual void FutbolGrabbed( CPropExplodingFutbol *, bool );  // line 138
	CPropIndicatorPanel *GetTimerPanel();  // line 140
private:
	virtual void SpawnFutbol();  // line 143
	void InputForceSpawn( inputdata_t & );  // line 145
	COutputEvent m_OnFutbolSpawned; // +0x57c  // line 148
	COutputEvent m_OnFutbolGrabbed; // +0x594  // line 149
	bool m_bHasFutbol; // +0x5ac  // line 151
	bool m_bIsTimed; // +0x5ad  // line 152
	float m_flTimer; // +0x5b0  // line 153
	string_t m_strTimerIndicator; // +0x5b4  // line 154
};

// game/server/portal2/prop_exploding_futbol.cpp:127 (declaration)
~CPropExplodingFutbolSpawner();

// game/server/portal2/prop_exploding_futbol.cpp:127 @0x755f20 _ZN27CPropExplodingFutbolSpawnerD0Ev
CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner()
{
	// inlined COutputEvent::~COutputEvent() at line 127
	// inlined COutputEvent::~COutputEvent() at line 127
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 127
	// inlined COutputEvent::~COutputEvent() at line 127
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 127
}

// game/server/portal2/prop_exploding_futbol.cpp:127 @0x756010 _ZN27CPropExplodingFutbolSpawnerD1Ev
CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner()
{
	// inlined COutputEvent::~COutputEvent() at line 127
	// inlined COutputEvent::~COutputEvent() at line 127
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 127
	// inlined COutputEvent::~COutputEvent() at line 127
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 127
}

// game/server/portal2/prop_exploding_futbol.cpp:130 @0x750850 _ZN27CPropExplodingFutbolSpawner14GetDataDescMapEv
datamap_t *CPropExplodingFutbolSpawner::GetDataDescMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:130 @0x750860 _ZN27CPropExplodingFutbolSpawner10GetBaseMapEv
datamap_t *CPropExplodingFutbolSpawner::GetBaseMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:132 @0x7516a0 _ZN27CPropExplodingFutbolSpawnerC2Ev
CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner()
{
	// inlined COutputEvent::COutputEvent() at line 1035
	// inlined COutputEvent::COutputEvent() at line 1035
	// inlined Vector::Vector() at line 1039
	// inlined Vector::operator=() at line 1039
}

// game/server/portal2/prop_exploding_futbol.cpp:132 @0x751740 _ZN27CPropExplodingFutbolSpawnerC1Ev
CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:134 @0x750dd0 _ZN27CPropExplodingFutbolSpawner5SpawnEv
void CPropExplodingFutbolSpawner::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 1057
}

// game/server/portal2/prop_exploding_futbol.cpp:135 @0x750aa0 _ZN27CPropExplodingFutbolSpawner8PrecacheEv
void CPropExplodingFutbolSpawner::Precache()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:137 @0x750ba0 _ZN27CPropExplodingFutbolSpawner15FutbolDestroyedEv
void CPropExplodingFutbolSpawner::FutbolDestroyed()
{
	CPropIndicatorPanel *pTimerPanel;  // line 1071
}

// game/server/portal2/prop_exploding_futbol.cpp:138 @0x755760 _ZN27CPropExplodingFutbolSpawner13FutbolGrabbedEP20CPropExplodingFutbolb
void CPropExplodingFutbolSpawner::FutbolGrabbed( CPropExplodingFutbol *pFutbol, bool bSamePlayer )
{
	{
		CPropIndicatorPanel *pTimerPanel;  // line 1088
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:140 @0x750ae0 _ZN27CPropExplodingFutbolSpawner13GetTimerPanelEv
CPropIndicatorPanel *CPropExplodingFutbolSpawner::GetTimerPanel()
{
	{
		CBaseEntity *pEnt;  // line 1116
		{
			CPropIndicatorPanel *pTimerPanel;  // line 1122
			// inlined CBaseEntity::GetClassname() at line 1129
		}
	}
	// inlined string_t::operator!=() at line 1113
}

// game/server/portal2/prop_exploding_futbol.cpp:143 @0x751360 _ZN27CPropExplodingFutbolSpawner11SpawnFutbolEv
void CPropExplodingFutbolSpawner::SpawnFutbol()
{
	CPropExplodingFutbol *pFutbol;  // line 1146
	Vector vecFutbolSpawnPoint;  // line 1157
	// inlined CPropExplodingFutbol::SetHolder() at line 1161
}

// game/server/portal2/prop_exploding_futbol.cpp:145 @0x750a40 _ZN27CPropExplodingFutbolSpawner15InputForceSpawnER11inputdata_t
void CPropExplodingFutbolSpawner::InputForceSpawn( inputdata_t &data )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:157 @0xb3c40 _Z11DataMapInitI27CPropExplodingFutbolSpawnerEP9datamap_tPT_
datamap_t *DataMapInit<CPropExplodingFutbolSpawner>( CPropExplodingFutbolSpawner * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 157
	CDatadescGeneratedNameHolder nameHolder;  // line 157
	typedescription_t dataDesc[8];  // line 157
}

// game/server/portal2/prop_exploding_futbol.cpp:157
datamap_t *g_DataMapHolder;

// game/server/portal2/prop_exploding_futbol.cpp:171
static CEntityFactory<CPropExplodingFutbolSpawner> prop_exploding_futbol_spawner;

// game/server/portal2/prop_exploding_futbol.cpp:175 sizeof=0x57c (i386)
struct CPropExplodingFutbolSocket : public CExplodingFutbolCatcher
{
public:
	CPropExplodingFutbolSocket();  // line 179
	virtual void Precache();  // line 181
	virtual void Spawn();  // line 182
	virtual void CaptureFutbol( CPropExplodingFutbol * );  // line 184
};

// game/server/portal2/prop_exploding_futbol.cpp:175 (declaration)
~CPropExplodingFutbolSocket();

// game/server/portal2/prop_exploding_futbol.cpp:175 @0x755950 _ZN26CPropExplodingFutbolSocketD1Ev
CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket()
{
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 175
}

// game/server/portal2/prop_exploding_futbol.cpp:175 @0x7560f0 _ZN26CPropExplodingFutbolSocketD0Ev
CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket()
{
	// inlined CExplodingFutbolCatcher::~CExplodingFutbolCatcher() at line 175
}

// game/server/portal2/prop_exploding_futbol.cpp:179 @0x751620 _ZN26CPropExplodingFutbolSocketC1Ev
CPropExplodingFutbolSocket::CPropExplodingFutbolSocket()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:179 @0x751660 _ZN26CPropExplodingFutbolSocketC2Ev
CPropExplodingFutbolSocket::CPropExplodingFutbolSocket()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:181 @0x750a60 _ZN26CPropExplodingFutbolSocket8PrecacheEv
void CPropExplodingFutbolSocket::Precache()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:182 @0x7512f0 _ZN26CPropExplodingFutbolSocket5SpawnEv
void CPropExplodingFutbolSocket::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 1191
}

// game/server/portal2/prop_exploding_futbol.cpp:184 @0x753db0 _ZN26CPropExplodingFutbolSocket13CaptureFutbolEP20CPropExplodingFutbol
void CPropExplodingFutbolSocket::CaptureFutbol( CPropExplodingFutbol *pFutbol )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:187
static CEntityFactory<CPropExplodingFutbolSocket> prop_exploding_futbol_socket;

// game/server/portal2/prop_exploding_futbol.cpp:191 (declaration)
void GetBaseMap();

// game/server/portal2/prop_exploding_futbol.cpp:191 @0x750870 _ZN20CPropExplodingFutbol14GetDataDescMapEv
datamap_t *CPropExplodingFutbol::GetDataDescMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:191 @0x750880 _ZN20CPropExplodingFutbol10GetBaseMapEv
datamap_t *CPropExplodingFutbol::GetBaseMap()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:191 @0xb4260 _Z11DataMapInitI20CPropExplodingFutbolEP9datamap_tPT_
datamap_t *DataMapInit<CPropExplodingFutbol>( CPropExplodingFutbol * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 212
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 212
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 191
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 212
	CDatadescGeneratedNameHolder nameHolder;  // line 191
	typedescription_t dataDesc[17];  // line 191
}

// game/server/portal2/prop_exploding_futbol.cpp:191
datamap_t *g_DataMapHolder;

// game/server/portal2/prop_exploding_futbol.cpp:191
void CPropExplodingFutbol::m_DataMap;

// game/server/portal2/prop_exploding_futbol.cpp:214
static CEntityFactory<CPropExplodingFutbol> prop_exploding_futbol;

// game/server/portal2/prop_exploding_futbol.cpp:216 (declaration)
void CPropExplodingFutbol();

// game/server/portal2/prop_exploding_futbol.cpp:216 @0x751cc0 _ZN20CPropExplodingFutbolC2Ev
CPropExplodingFutbol::CPropExplodingFutbol()
{
	// inlined CHandle<CPropExplodingFutbolSpawner>::CHandle() at line 227
	// inlined CHandle<CPortal_Player>::CHandle() at line 227
	// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::CUtlVector() at line 227
}

// game/server/portal2/prop_exploding_futbol.cpp:216 @0x751da0 _ZN20CPropExplodingFutbolC1Ev
CPropExplodingFutbol::CPropExplodingFutbol()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:232 @0x750f80 _ZN20CPropExplodingFutbol8PrecacheEv
void CPropExplodingFutbol::Precache()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:243 @0x751940 _ZN20CPropExplodingFutbol5SpawnEv
void CPropExplodingFutbol::Spawn()
{
	// inlined CBaseEntity::AddSpawnFlags() at line 286
	{
		CBaseEntity *pEnt;  // line 253
		// inlined string_t::operator!=() at line 254
		{
			CPropExplodingFutbolSpawner *pSpawner;  // line 260
			// inlined CHandle<CPropExplodingFutbolSpawner>::Set() at line 263
			// inlined CBaseEntity::GetClassname() at line 267
		}
	}
	// inlined CHandle<CPropExplodingFutbolSpawner>::Get() at line 251
	// inlined CBaseEntity::SetModelName() at line 245
}

// game/server/portal2/prop_exploding_futbol.cpp:289 @0x7523f0 _ZN20CPropExplodingFutbol14VPhysicsUpdateEP14IPhysicsObject
void CPropExplodingFutbol::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	Vector vVelocity;  // line 293
	Vector vPosition;  // line 293
	{
		Vector vPropOrigin;  // line 299
		int iPortalCount;  // line 303
		// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::Count() at line 303
		{
			CPortal_Base2D *pFunnelInto;  // line 306
			Vector vPropToFunnelPortal;  // line 307
			float fClosestFunnelPortalDistSqr;  // line 308
			CPortal_Base2D **pPortals;  // line 310
			// inlined CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*, int> >::Base() at line 310
			{
				int i;  // line 311
				{
					CPortal_Base2D *pTempPortal;  // line 313
					{
						Vector vPropToPortal;  // line 320
						float fDistSqr;  // line 326
						// inlined CBaseEntity::GetAbsOrigin() at line 320
						// inlined Vector::operator-() at line 320
						// inlined Vector::Length2DSqr() at line 326
					}
				}
			}
			{
				const float flHeightFromPortal;  // line 338
				const float flVerticalSpeed;  // line 341
				const float flGravity;  // line 342
				float flRoot1;  // line 345
				float flRoot2;  // line 345
				float flTimeToPortal;  // line 347
				Vector vPortalPos;  // line 352
				Vector vPredictedHitPos;  // line 353
				float flMissDist;  // line 357
				// inlined CBaseEntity::GetAbsOrigin() at line 352
				// inlined Vector::operator*() at line 353
				// inlined Vector::operator+() at line 353
				// inlined Vector::operator-() at line 357
				// inlined Vector::Length2D() at line 357
				{
					Vector vVelocityToHitPortal;  // line 361
					// inlined Vector::operator/() at line 361
				}
			}
		}
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:372 (declaration)
void GetHolder();

// game/server/portal2/prop_exploding_futbol.cpp:372 @0x750890 _ZN20CPropExplodingFutbol9GetHolderEv
ExplodingFutbolHolderType_t CPropExplodingFutbol::GetHolder()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:378 (declaration)
void SetHolder( ExplodingFutbolHolderType_t holder );

// game/server/portal2/prop_exploding_futbol.cpp:378 @0x7508a0 _ZN20CPropExplodingFutbol9SetHolderE27ExplodingFutbolHolderType_t
void CPropExplodingFutbol::SetHolder( ExplodingFutbolHolderType_t holder )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:384 (declaration)
void GetLastPlayerToHold();

// game/server/portal2/prop_exploding_futbol.cpp:384 @0x7508c0 _ZN20CPropExplodingFutbol19GetLastPlayerToHoldEv
CPortal_Player *CPropExplodingFutbol::GetLastPlayerToHold()
{
	// inlined CHandle<CPortal_Player>::Get() at line 386
}

// game/server/portal2/prop_exploding_futbol.cpp:390 @0x7512a0 _ZN20CPropExplodingFutbol10SetSpawnerEP27CPropExplodingFutbolSpawner
void CPropExplodingFutbol::SetSpawner( CPropExplodingFutbolSpawner *pSpawner )
{
	// inlined CHandle<CPropExplodingFutbolSpawner>::Set() at line 393
}

// game/server/portal2/prop_exploding_futbol.cpp:398 (declaration)
void GetSpawner();

// game/server/portal2/prop_exploding_futbol.cpp:398 @0x750910 _ZN20CPropExplodingFutbol10GetSpawnerEv
CPropExplodingFutbolSpawner *CPropExplodingFutbol::GetSpawner()
{
	// inlined CHandle<CPropExplodingFutbolSpawner>::operator CPropExplodingFutbolSpawner*() at line 400
}

// game/server/portal2/prop_exploding_futbol.cpp:404 @0x7523e0 _ZN20CPropExplodingFutbol12InputExplodeER11inputdata_t
void CPropExplodingFutbol::InputExplode( inputdata_t &in )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:409 @0x7522d0 _ZN20CPropExplodingFutbol10KillFutbolEv
void CPropExplodingFutbol::KillFutbol()
{
	CTakeDamageInfo info;  // line 412
	// inlined CTakeDamageInfo::SetDamageForce() at line 415
	// inlined Vector::operator*() at line 415
	// inlined CBaseEntity::Forward() at line 415
	// inlined CTakeDamageInfo::SetDamagePosition() at line 414
	// inlined CBaseEntity::GetAbsOrigin() at line 414
}

// game/server/portal2/prop_exploding_futbol.cpp:422 (declaration)
void DestroyFutbol( bool bExplode );

// game/server/portal2/prop_exploding_futbol.cpp:422 @0x753550 _ZN20CPropExplodingFutbol13DestroyFutbolEb
void CPropExplodingFutbol::DestroyFutbol( bool bExplode )
{
	// inlined CHandle<CPropExplodingFutbolSpawner>::Get() at line 433
}

// game/server/portal2/prop_exploding_futbol.cpp:441 @0x7535d0 _ZN20CPropExplodingFutbol12Event_KilledERK15CTakeDamageInfo
void CPropExplodingFutbol::Event_Killed( const CTakeDamageInfo &info )
{
	// inlined CPropExplodingFutbol::DestroyFutbol() at line 445
}

// game/server/portal2/prop_exploding_futbol.cpp:449 @0x7532f0 _ZN20CPropExplodingFutbol13ExplodeFutbolEv
void CPropExplodingFutbol::ExplodeFutbol()
{
	float flExplosionRadius;  // line 451
	PortalRadiusExtensionVector portalRadiusExtensions;  // line 464
	{
		int i;  // line 466
		// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::operator[]() at line 468
	}
	// inlined CBaseEntity::GetAbsAngles() at line 465
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::CUtlVector() at line 464
	// inlined CBaseEntity::GetAbsOrigin() at line 461
	// inlined CBaseEntity::GetAbsAngles() at line 461
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::~CUtlVector() at line 469
	// inlined CBaseEntity::GetOwnerEntity() at line 461
	// inlined ConVar::GetInt() at line 461
	// inlined ConVar::GetFloat() at line 461
	// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::RemoveAll() at line 454
	// inlined ConVar::GetFloat() at line 451
	// inlined CBaseEntity::GetAbsOrigin() at line 465
	// inlined CUtlVector<PortalRadiusExtension_t,CUtlMemory<PortalRadiusExtension_t, int> >::~CUtlVector() at line 469
}

// game/server/portal2/prop_exploding_futbol.cpp:473 @0x751db0 _ZN20CPropExplodingFutbol12DamageEntityEP11CBaseEntityRK6VectorS4_
void CPropExplodingFutbol::DamageEntity( CBaseEntity *pEntity, const Vector &vecCenter, const Vector &vecForward )
{
	Vector vecDamagePos;  // line 475
	{
		float flDist;  // line 480
		float flBaseDamage;  // line 483
		float flPercent;  // line 484
		float flDamage;  // line 485
		CTakeDamageInfo info;  // line 494
		{
			CFmtStr msg;  // line 489
			// inlined CBaseEntity::GetAbsOrigin() at line 491
			// inlined CFmtStrN<256>::CFmtStrN() at line 489
		}
		// inlined CBaseEntity::GetAbsOrigin() at line 480
		// inlined Vector::DistTo() at line 480
		// inlined ConVar::GetFloat() at line 483
		// inlined Lerp<float>() at line 485
		// inlined CTakeDamageInfo::SetDamagePosition() at line 495
	}
	// inlined Vector::operator-() at line 475
	// inlined Vector::operator*() at line 475
	// inlined FClassnameIs() at line 502
	{
		CTakeDamageInfo info;  // line 513
		// inlined Vector::operator VectorByValue&() at line 515
		// inlined Vector::operator-() at line 515
		// inlined CBaseEntity::GetAbsOrigin() at line 515
		// inlined CTakeDamageInfo::SetDamagePosition() at line 514
	}
	{
		CTakeDamageInfo info;  // line 504
		// inlined CTakeDamageInfo::SetDamagePosition() at line 505
		// inlined ConVar::GetFloat() at line 506
		// inlined Vector::operator*() at line 506
		// inlined CTakeDamageInfo::SetDamageForce() at line 506
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:523 @0x7527f0 _ZN20CPropExplodingFutbol11DoExplosionERK6VectorfP14CPortal_Base2D
void CPropExplodingFutbol::DoExplosion( const Vector &vecCenter, float flExplosionRadius, CPortal_Base2D *pPortal )
{
	bool bDebugging;  // line 525
	Color debugColor;  // line 526
	CBaseEntity *pEnts[128];  // line 532
	int nNumFound;  // line 533
	{
		int i;  // line 534
		{
			bool bTestEntity;  // line 542
			int x;  // line 543
			int iExplodedEntitiesCount;  // line 563
			bool bSuccess;  // line 573
			// inlined FClassnameIs() at line 546
			{
				Vector vecRayStart;  // line 578
				Vector vecRayEnd;  // line 579
				CTraceFilterSimpleList traceFilter;  // line 582
				// inlined CTraceFilterSimpleList::~CTraceFilterSimpleList() at line 653
				// inlined CBaseEntity::GetAbsOrigin() at line 579
				{
					int j;  // line 584
				}
				{
					CPortal_Base2D *pInPortal;  // line 609
					Vector vecEndPos;  // line 610
					Ray_t portalRay;  // line 614
					trace_t trace;  // line 616
					bool bPortalTrace;  // line 617
					// inlined CBaseEntity::GetAbsOrigin() at line 634
					// inlined CPortal_Base2D::GetLinkedPortal() at line 609
					// inlined Ray_t::Ray_t() at line 614
					// inlined CBaseEntity::GetAbsOrigin() at line 615
					// inlined Ray_t::Init() at line 615
					{
						Vector vecForward;  // line 623
						// inlined CBaseEntity::GetAbsOrigin() at line 622
						// inlined CBaseEntity::GetAbsOrigin() at line 623
						// inlined Vector::operator-() at line 623
					}
				}
				// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::AddToTail() at line 653
				{
					Color lineColor;  // line 640
					// inlined Color::Color() at line 640
					// inlined Color::Color() at line 644
					// inlined Color::operator=() at line 644
				}
				{
					Ray_t entRay;  // line 592
					trace_t trace;  // line 594
					// inlined Ray_t::Init() at line 593
					// inlined UTIL_TraceRay() at line 595
					// inlined CBaseEntity::Forward() at line 601
					// inlined Vector::operator VectorByValue&() at line 601
				}
				// inlined CTraceFilterSimpleList::~CTraceFilterSimpleList() at line 653
			}
			// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::Count() at line 563
			{
				int j;  // line 564
				// inlined CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*, int> >::operator[]() at line 566
			}
		}
	}
	// inlined UTIL_EntitiesInSphere() at line 533
	// inlined Color::operator=() at line 529
	// inlined Color::Color() at line 529
	// inlined Color::Color() at line 526
}

// game/server/portal2/prop_exploding_futbol.cpp:665 @0x755170 _ZN20CPropExplodingFutbol19ActivateFutbolTimerEf
void CPropExplodingFutbol::ActivateFutbolTimer( float flTimer )
{
	{
		Color futbolColor;  // line 679
		// inlined CBaseEntity::SetRenderColor() at line 680
		// inlined ConVar::GetColor() at line 679
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:687 (declaration)
void StopFutbolTimer();

// game/server/portal2/prop_exploding_futbol.cpp:687 @0x753e10 _ZN20CPropExplodingFutbol15StopFutbolTimerEv
void CPropExplodingFutbol::StopFutbolTimer()
{
	Color futbolColor;  // line 692
	// inlined CBaseEntity::SetRenderColor() at line 693
	// inlined ConVar::GetColor() at line 692
}

// game/server/portal2/prop_exploding_futbol.cpp:697 @0x751770 _ZN20CPropExplodingFutbol15OnPhysGunPickupEP11CBasePlayer15PhysGunPickup_t
void CPropExplodingFutbol::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	CPortal_Player *pHoldingPlayer;  // line 700
	IPhysicsObject *pPhysicsObject;  // line 703
	bool bSamePlayer;  // line 719
	// inlined CHandle<CPortal_Player>::operator=() at line 731
	// inlined CHandle<CPropExplodingFutbolSpawner>::Get() at line 726
	// inlined CPropExplodingFutbol::GetLastPlayerToHold() at line 721
	// inlined CBaseEntity::VPhysicsGetObject() at line 703
	// inlined ToPortalPlayer() at line 700
	{
		CBasePlayer *pOtherPlayer;  // line 735
	}
}

// game/server/portal2/prop_exploding_futbol.cpp:747 @0x750f30 _ZN20CPropExplodingFutbol13OnPhysGunDropEP11CBasePlayer13PhysGunDrop_t
void CPropExplodingFutbol::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:768 @0x750ee0 _ZN20CPropExplodingFutbol12OnTakeDamageERK15CTakeDamageInfo
int CPropExplodingFutbol::OnTakeDamage( const CTakeDamageInfo &info )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:782 @0x753660 _ZN20CPropExplodingFutbol9OnFizzledEv
void CPropExplodingFutbol::OnFizzled()
{
	// inlined CPropExplodingFutbol::DestroyFutbol() at line 786
}

// game/server/portal2/prop_exploding_futbol.cpp:790 @0x750e50 _ZN20CPropExplodingFutbol17VPhysicsCollisionEiP21gamevcollisionevent_t
void CPropExplodingFutbol::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
}

// game/server/portal2/prop_exploding_futbol.cpp:805 @0x7523d0 _ZN20CPropExplodingFutbol9KillThinkEv
void CPropExplodingFutbol::KillThink()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:810 @0x750bd0 _ZN20CPropExplodingFutbol9AnimThinkEv
void CPropExplodingFutbol::AnimThink()
{
}

// game/server/portal2/prop_exploding_futbol.cpp:821 (declaration)
Color ColorLerp( float flPercent, const Color &startColor, const Color &endColor );

// game/server/portal2/prop_exploding_futbol.cpp:821 @0x750960 _Z9ColorLerpfRK5ColorS1_
Color ColorLerp( float flPercent, const Color &startColor, const Color &endColor )
{
	float r;  // line 823
	float g;  // line 824
	float b;  // line 825
	float a;  // line 826
	// inlined Color::r() at line 823
	// inlined Color::g() at line 824
	// inlined Color::b() at line 825
	// inlined Color::a() at line 826
	// inlined Color::Color() at line 828
}

// game/server/portal2/prop_exploding_futbol.cpp:832 @0x754370 _ZN20CPropExplodingFutbol10TimerThinkEv
void CPropExplodingFutbol::TimerThink()
{
	float flDeltaTime;  // line 840
	float flSoundDelay;  // line 845
	string_t strSoundName;  // line 846
	Color startColor;  // line 848
	Color endColor;  // line 848
	Color currentColor;  // line 848
	float flPercent;  // line 849
	// inlined CBaseEntity::SetRenderColor() at line 887
	// inlined Color::r() at line 887
	// inlined Color::g() at line 887
	// inlined Color::b() at line 887
	// inlined Color::operator=() at line 886
	// inlined ColorLerp() at line 886
	// inlined string_t::ToCStr() at line 881
	// inlined Color::operator=() at line 854
	// inlined ConVar::GetColor() at line 854
	// inlined Color::operator=() at line 853
	// inlined ConVar::GetColor() at line 853
	// inlined Color::Color() at line 848
	// inlined Color::Color() at line 848
	// inlined Color::Color() at line 848
	{
		float flCurrentFlashDuration;  // line 863
		// inlined ConVar::GetFloat() at line 861
		// inlined ConVar::GetColor() at line 873
		// inlined Color::operator=() at line 873
		// inlined ConVar::GetColor() at line 874
		// inlined Color::operator=() at line 874
	}
	// inlined CPropExplodingFutbol::StopFutbolTimer() at line 896
}
