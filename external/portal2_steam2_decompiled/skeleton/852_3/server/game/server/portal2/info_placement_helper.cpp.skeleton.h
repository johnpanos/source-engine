// DWARF declaration skeleton for game/server/portal2/info_placement_helper.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/info_placement_helper.cpp:0 (declaration)
void GetBaseMap();

// game/server/portal2/info_placement_helper.cpp:0 @0x6b9a10 _ZN20CInfoPlacementHelper14GetDataDescMapEv
datamap_t *CInfoPlacementHelper::GetDataDescMap()
{
}

// game/server/portal2/info_placement_helper.cpp:0 @0x6b9a20 _ZN20CInfoPlacementHelper10GetBaseMapEv
datamap_t *CInfoPlacementHelper::GetBaseMap()
{
}

// None:0 @0xbfeb0 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityFactory<CInfoPlacementHelper>::CEntityFactory() at line 18
	// inlined ServerClass::ServerClass() at line 39
	// inlined CInfoPlacementManager::CInfoPlacementManager() at line 144
}

// game/server/portal2/info_placement_helper.cpp:0
void CInfoPlacementHelper::m_DataMap;

// game/server/portal2/info_placement_helper.cpp:14
ConVar sv_show_placement_help_in_preview;

// game/server/portal2/info_placement_helper.cpp:18
static CEntityFactory<CInfoPlacementHelper> info_placement_helper;

// game/server/portal2/info_placement_helper.cpp:20 @0xbfd80 _Z11DataMapInitI20CInfoPlacementHelperEP9datamap_tPT_
datamap_t *DataMapInit<CInfoPlacementHelper>( CInfoPlacementHelper * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 20
	CDatadescGeneratedNameHolder nameHolder;  // line 20
	typedescription_t dataDesc[13];  // line 20
}

// game/server/portal2/info_placement_helper.cpp:20
datamap_t *g_DataMapHolder;

// game/server/portal2/info_placement_helper.cpp:39 @0x6b9a30 _ZN20CInfoPlacementHelper14GetServerClassEv
ServerClass *CInfoPlacementHelper::GetServerClass()
{
}

// game/server/portal2/info_placement_helper.cpp:39 @0x6b9a40 _ZN20CInfoPlacementHelper40YouForgotToImplementOrDeclareServerClassEv
int CInfoPlacementHelper::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/info_placement_helper.cpp:39 @0xbf9d0 _Z15ServerClassInitIN22DT_InfoPlacementHelper7ignoredEEiPT_
int ServerClassInit<DT_InfoPlacementHelper::ignored>( DT_InfoPlacementHelper::ignored * )
{
	SendTable &sendTable;  // line 39
	char *const g_pSendTableName;  // line 39
	SendProp g_SendProps[10];  // line 39
}

// game/server/portal2/info_placement_helper.cpp:39
SendTable g_SendTable;

// game/server/portal2/info_placement_helper.cpp:39
int g_SendTableInit;

// game/server/portal2/info_placement_helper.cpp:39
static ServerClass g_CInfoPlacementHelper_ClassReg;

// game/server/portal2/info_placement_helper.cpp:39
void CInfoPlacementHelper::m_pClassSendTable;

// game/server/portal2/info_placement_helper.cpp:55 sizeof=0x20 (i386)
struct CInfoPlacementManager : public CAutoGameSystem
{
public:
	virtual void LevelShutdownPostEntity();  // line 58
	void Purge();  // line 63
	void AddPlacementHelper( CInfoPlacementHelper * );  // line 68
	void RemovePlacementHelper( CInfoPlacementHelper * );  // line 77
	CInfoPlacementHelper *FindPlacementHelper( const Vector &, CBasePlayer * );  // line 86
private:
	CUtlVector<CHandle<CInfoPlacementHelper>,CUtlMemory<CHandle<CInfoPlacementHelper>, int> > m_PlacementHelpers; // +0xc  // line 140
};

// game/server/portal2/info_placement_helper.cpp:55 (declaration)
void CInfoPlacementManager();

// game/server/portal2/info_placement_helper.cpp:55 (declaration)
~CInfoPlacementManager();

// game/server/portal2/info_placement_helper.cpp:55 @0x6badc0 _ZN21CInfoPlacementManagerD0Ev
CInfoPlacementManager::~CInfoPlacementManager()
{
	// inlined CUtlVector<CHandle<CInfoPlacementHelper>,CUtlMemory<CHandle<CInfoPlacementHelper>, int> >::~CUtlVector() at line 55
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 55
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 55
}

// game/server/portal2/info_placement_helper.cpp:55 @0x6baed0 _ZN21CInfoPlacementManagerD1Ev
CInfoPlacementManager::~CInfoPlacementManager()
{
	// inlined CUtlVector<CHandle<CInfoPlacementHelper>,CUtlMemory<CHandle<CInfoPlacementHelper>, int> >::~CUtlVector() at line 55
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 55
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 55
}

// game/server/portal2/info_placement_helper.cpp:58 @0x6bac30 _ZN21CInfoPlacementManager23LevelShutdownPostEntityEv
void CInfoPlacementManager::LevelShutdownPostEntity()
{
	// inlined CInfoPlacementManager::Purge() at line 60
}

// game/server/util.h:99 @0x6bafd0 _ZN14CEntityFactoryI20CInfoPlacementHelperE6CreateEPKc
IServerNetworkable *CEntityFactory<CInfoPlacementHelper>::Create( const char *pClassName )
{
	CInfoPlacementHelper *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CInfoPlacementHelper>() at line 101
}

// game/server/util.h:105 @0x6bab60 _ZN14CEntityFactoryI20CInfoPlacementHelperE7DestroyEP18IServerNetworkable
void CEntityFactory<CInfoPlacementHelper>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6bab90 _ZN14CEntityFactoryI20CInfoPlacementHelperE13GetEntitySizeEv
size_t CEntityFactory<CInfoPlacementHelper>::GetEntitySize()
{
}

// game/server/portal2/info_placement_helper.cpp:144
static CInfoPlacementManager g_PlacementManager;

// game/server/portal2/info_placement_helper.cpp:149 @0x6ba070 _Z24UTIL_FindPlacementHelperRK6VectorP11CBasePlayer
CInfoPlacementHelper *UTIL_FindPlacementHelper( const Vector &vecEndPoint, CBasePlayer *pPlayer )
{
	// inlined CInfoPlacementManager::FindPlacementHelper() at line 151
}

// game/server/portal2/info_placement_helper.cpp:157 (declaration)
void CInfoPlacementHelper();

// game/server/portal2/info_placement_helper.cpp:157 @0x6ba8c0 _ZN20CInfoPlacementHelperC2Ev
CInfoPlacementHelper::CInfoPlacementHelper()
{
	// inlined CPointEntity::CPointEntity() at line 161
	// inlined COutputEvent::COutputEvent() at line 161
	// inlined CEntityOutputTemplate<float,FIELD_FLOAT>::CEntityOutputTemplate() at line 161
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bSnapToHelperAngles>::CNetworkVarBase() at line 161
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bForcePlacement>::CNetworkVarBase() at line 161
	// inlined CNetworkVarBase<float,CInfoPlacementHelper::NetworkVar_m_flDisableTime>::CNetworkVarBase() at line 161
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDeferringToPortal>::CNetworkVarBase() at line 161
	// inlined CEntityOutputTemplate<float,FIELD_FLOAT>::~CEntityOutputTemplate() at line 163
	// inlined COutputEvent::~COutputEvent() at line 163
	// inlined CPointEntity::~CPointEntity() at line 163
}

// game/server/portal2/info_placement_helper.cpp:157 @0x6baa70 _ZN20CInfoPlacementHelperC1Ev
CInfoPlacementHelper::CInfoPlacementHelper()
{
}

// game/server/portal2/info_placement_helper.cpp:168 @0x6ba420 _ZN20CInfoPlacementHelper5SpawnEv
void CInfoPlacementHelper::Spawn()
{
	// inlined CInfoPlacementManager::AddPlacementHelper() at line 172
}

// game/server/portal2/info_placement_helper.cpp:178 @0x6b9ac0 _ZN20CInfoPlacementHelper9OnRestoreEv
void CInfoPlacementHelper::OnRestore()
{
	// inlined CInfoPlacementManager::AddPlacementHelper() at line 181
}

// game/server/portal2/info_placement_helper.cpp:188 @0x6b9c90 _ZN20CInfoPlacementHelper14UpdateOnRemoveEv
void CInfoPlacementHelper::UpdateOnRemove()
{
	// inlined CInfoPlacementManager::RemovePlacementHelper() at line 192
}

// game/server/portal2/info_placement_helper.cpp:198 @0x6b9ab0 _ZN20CInfoPlacementHelper21ShouldUseHelperAnglesEv
bool CInfoPlacementHelper::ShouldUseHelperAngles()
{
}

// game/server/portal2/info_placement_helper.cpp:206 @0x6b9e90 _ZN20CInfoPlacementHelper11InputEnableER11inputdata_t
void CInfoPlacementHelper::InputEnable( inputdata_t &inputdata )
{
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDisabled>::operator=<bool>() at line 208
}

// game/server/portal2/info_placement_helper.cpp:214 @0x6ba600 _ZN20CInfoPlacementHelper12InputDisableER11inputdata_t
void CInfoPlacementHelper::InputDisable( inputdata_t &inputdata )
{
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDisabled>::operator=<bool>() at line 216
}

// game/server/portal2/info_placement_helper.cpp:222 @0x6b9ef0 _ZN20CInfoPlacementHelper25DrawDebugGeometryOverlaysEv
void CInfoPlacementHelper::DrawDebugGeometryOverlays()
{
	int g;  // line 226
	// inlined CBaseEntity::GetAbsAngles() at line 227
	// inlined CInfoPlacementHelper::IsEnabled() at line 226
	// inlined CBaseEntity::GetAbsOrigin() at line 224
	// inlined CBaseEntity::GetAbsAngles() at line 224
	// inlined CBaseEntity::GetAbsOrigin() at line 227
}

// game/server/portal2/info_placement_helper.cpp:235 (declaration)
void IsEnabled();

// game/server/portal2/info_placement_helper.cpp:235 @0x6b9a50 _ZN20CInfoPlacementHelper9IsEnabledEv
bool CInfoPlacementHelper::IsEnabled()
{
}

// game/server/portal2/info_placement_helper.cpp:252 @0x6ba660 _ZN20CInfoPlacementHelper17NotifyPortalEventE13PortalEvent_tP14CPortal_Base2D
void CInfoPlacementHelper::NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier )
{
	float flResetDelay;  // line 269
	// inlined CNetworkVarBase<float,CInfoPlacementHelper::NetworkVar_m_flDisableTime>::operator=<float>() at line 270
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDeferringToPortal>::operator=<bool>() at line 266
	// inlined CHandle<CBaseEntity>::CHandle() at line 265
	// inlined CInfoPlacementHelper::GetTargetOrigin() at line 260
	// inlined CBaseEntity::GetAbsOrigin() at line 260
	// inlined Vector::operator-() at line 260
}

// game/server/portal2/info_placement_helper.cpp:276 @0x6b9de0 _ZN20CInfoPlacementHelper12BindToPortalEP14CPortal_Base2D
void CInfoPlacementHelper::BindToPortal( CPortal_Base2D *pPortal )
{
	// inlined CHandle<CBaseEntity>::CHandle() at line 286
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDeferringToPortal>::operator=<bool>() at line 288
}

// public/tier1/utlmemory.h:707 @0x6baa80 _ZN10CUtlMemoryI7CHandleI20CInfoPlacementHelperEiE4GrowEi
void CUtlMemory<CHandle<CInfoPlacementHelper>,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<CHandle<CInfoPlacementHelper>,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
