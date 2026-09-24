// DWARF declaration skeleton for game/server/portal2/info_placement_helper.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xaabc0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CInfoPlacementHelper>::CEntityFactory() at line 21
	// inlined ServerClass::ServerClass() at line 42
	// inlined CInfoPlacementManager::CInfoPlacementManager() at line 153
}

// game/server/portal2/info_placement_helper.cpp:14
ConVar sv_show_placement_help_in_preview;

// game/server/portal2/info_placement_helper.cpp:21
static CEntityFactory<CInfoPlacementHelper> info_placement_helper;

// game/server/portal2/info_placement_helper.cpp:23 (declaration)
void GetBaseMap();

// game/server/portal2/info_placement_helper.cpp:23 @0x717f50 _ZN20CInfoPlacementHelper14GetDataDescMapEv
datamap_t *CInfoPlacementHelper::GetDataDescMap()
{
}

// game/server/portal2/info_placement_helper.cpp:23 @0x717f60 _ZN20CInfoPlacementHelper10GetBaseMapEv
datamap_t *CInfoPlacementHelper::GetBaseMap()
{
}

// game/server/portal2/info_placement_helper.cpp:23 @0xaaa90 _Z11DataMapInitI20CInfoPlacementHelperEP9datamap_tPT_
datamap_t *DataMapInit<CInfoPlacementHelper>( CInfoPlacementHelper * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 23
	CDatadescGeneratedNameHolder nameHolder;  // line 23
	typedescription_t dataDesc[13];  // line 23
}

// game/server/portal2/info_placement_helper.cpp:23
datamap_t *g_DataMapHolder;

// game/server/portal2/info_placement_helper.cpp:23
void CInfoPlacementHelper::m_DataMap;

// game/server/portal2/info_placement_helper.cpp:42 @0x717f70 _ZN20CInfoPlacementHelper14GetServerClassEv
ServerClass *CInfoPlacementHelper::GetServerClass()
{
}

// game/server/portal2/info_placement_helper.cpp:42 @0x717f80 _ZN20CInfoPlacementHelper40YouForgotToImplementOrDeclareServerClassEv
int CInfoPlacementHelper::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/info_placement_helper.cpp:42 @0xaa5b0 _Z15ServerClassInitIN22DT_InfoPlacementHelper7ignoredEEiPT_
int ServerClassInit<DT_InfoPlacementHelper::ignored>( DT_InfoPlacementHelper::ignored * )
{
	SendTable &sendTable;  // line 42
	char *const g_pSendTableName;  // line 42
	SendProp g_SendProps[14];  // line 42
}

// game/server/portal2/info_placement_helper.cpp:42
SendTable g_SendTable;

// game/server/portal2/info_placement_helper.cpp:42
int g_SendTableInit;

// game/server/portal2/info_placement_helper.cpp:42
static ServerClass g_CInfoPlacementHelper_ClassReg;

// game/server/portal2/info_placement_helper.cpp:42
void CInfoPlacementHelper::m_pClassSendTable;

// game/server/portal2/info_placement_helper.cpp:64 sizeof=0x20 (i386)
struct CInfoPlacementManager : public CAutoGameSystem
{
public:
	virtual void LevelShutdownPostEntity();  // line 67
	void Purge();  // line 72
	void AddPlacementHelper( CInfoPlacementHelper * );  // line 77
	void RemovePlacementHelper( CInfoPlacementHelper * );  // line 86
	CInfoPlacementHelper *FindPlacementHelper( const Vector &, CBasePlayer * );  // line 95
private:
	CUtlVector<CHandle<CInfoPlacementHelper>,CUtlMemory<CHandle<CInfoPlacementHelper>, int> > m_PlacementHelpers; // +0xc  // line 149
};

// game/server/portal2/info_placement_helper.cpp:64 (declaration)
void CInfoPlacementManager();

// game/server/portal2/info_placement_helper.cpp:64 (declaration)
~CInfoPlacementManager();

// game/server/portal2/info_placement_helper.cpp:64 @0x719cb0 _ZN21CInfoPlacementManagerD0Ev
CInfoPlacementManager::~CInfoPlacementManager()
{
	// inlined CUtlVector<CHandle<CInfoPlacementHelper>,CUtlMemory<CHandle<CInfoPlacementHelper>, int> >::~CUtlVector() at line 64
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 64
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 64
}

// game/server/portal2/info_placement_helper.cpp:64 @0x719dc0 _ZN21CInfoPlacementManagerD1Ev
CInfoPlacementManager::~CInfoPlacementManager()
{
	// inlined CUtlVector<CHandle<CInfoPlacementHelper>,CUtlMemory<CHandle<CInfoPlacementHelper>, int> >::~CUtlVector() at line 64
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 64
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 64
}

// game/server/portal2/info_placement_helper.cpp:67 @0x719c50 _ZN21CInfoPlacementManager23LevelShutdownPostEntityEv
void CInfoPlacementManager::LevelShutdownPostEntity()
{
	// inlined CInfoPlacementManager::Purge() at line 69
}

// game/server/util.h:100 @0x719f60 _ZN14CEntityFactoryI20CInfoPlacementHelperE6CreateEPKc
IServerNetworkable *CEntityFactory<CInfoPlacementHelper>::Create( const char *pClassName )
{
	CInfoPlacementHelper *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CInfoPlacementHelper>() at line 102
}

// game/server/util.h:106 @0x719b80 _ZN14CEntityFactoryI20CInfoPlacementHelperE7DestroyEP18IServerNetworkable
void CEntityFactory<CInfoPlacementHelper>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x719bb0 _ZN14CEntityFactoryI20CInfoPlacementHelperE13GetEntitySizeEv
size_t CEntityFactory<CInfoPlacementHelper>::GetEntitySize()
{
}

// game/server/portal2/info_placement_helper.cpp:153
static CInfoPlacementManager g_PlacementManager;

// game/server/portal2/info_placement_helper.cpp:158 @0x718660 _Z24UTIL_FindPlacementHelperRK6VectorP11CBasePlayer
CInfoPlacementHelper *UTIL_FindPlacementHelper( const Vector &vecEndPoint, CBasePlayer *pPlayer )
{
	// inlined CInfoPlacementManager::FindPlacementHelper() at line 160
}

// game/server/portal2/info_placement_helper.cpp:166 (declaration)
void CInfoPlacementHelper();

// game/server/portal2/info_placement_helper.cpp:166 @0x718be0 _ZN20CInfoPlacementHelperC2Ev
CInfoPlacementHelper::CInfoPlacementHelper()
{
	// inlined CPointEntity::CPointEntity() at line 170
	// inlined COutputEvent::COutputEvent() at line 170
	// inlined CEntityOutputTemplate<float,FIELD_FLOAT>::CEntityOutputTemplate() at line 170
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bSnapToHelperAngles>::CNetworkVarBase() at line 170
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bForcePlacement>::CNetworkVarBase() at line 170
	// inlined CNetworkVarBase<float,CInfoPlacementHelper::NetworkVar_m_flDisableTime>::CNetworkVarBase() at line 170
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDeferringToPortal>::CNetworkVarBase() at line 170
	// inlined CEntityOutputTemplate<float,FIELD_FLOAT>::~CEntityOutputTemplate() at line 172
	// inlined COutputEvent::~COutputEvent() at line 172
	// inlined CPointEntity::~CPointEntity() at line 172
}

// game/server/portal2/info_placement_helper.cpp:166 @0x7191d0 _ZN20CInfoPlacementHelperC1Ev
CInfoPlacementHelper::CInfoPlacementHelper()
{
}

// game/server/portal2/info_placement_helper.cpp:177 @0x718000 _ZN20CInfoPlacementHelper5SpawnEv
void CInfoPlacementHelper::Spawn()
{
	// inlined CInfoPlacementManager::AddPlacementHelper() at line 181
}

// game/server/portal2/info_placement_helper.cpp:187 @0x718a10 _ZN20CInfoPlacementHelper9OnRestoreEv
void CInfoPlacementHelper::OnRestore()
{
	// inlined CInfoPlacementManager::AddPlacementHelper() at line 190
}

// game/server/portal2/info_placement_helper.cpp:197 @0x7181e0 _ZN20CInfoPlacementHelper14UpdateOnRemoveEv
void CInfoPlacementHelper::UpdateOnRemove()
{
	// inlined CInfoPlacementManager::RemovePlacementHelper() at line 201
}

// game/server/portal2/info_placement_helper.cpp:207 @0x717ff0 _ZN20CInfoPlacementHelper21ShouldUseHelperAnglesEv
bool CInfoPlacementHelper::ShouldUseHelperAngles()
{
}

// game/server/portal2/info_placement_helper.cpp:215 @0x718330 _ZN20CInfoPlacementHelper11InputEnableER11inputdata_t
void CInfoPlacementHelper::InputEnable( inputdata_t &inputdata )
{
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDisabled>::operator=<bool>() at line 217
}

// game/server/portal2/info_placement_helper.cpp:223 @0x7191e0 _ZN20CInfoPlacementHelper12InputDisableER11inputdata_t
void CInfoPlacementHelper::InputDisable( inputdata_t &inputdata )
{
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDisabled>::operator=<bool>() at line 225
}

// game/server/portal2/info_placement_helper.cpp:231 @0x7184e0 _ZN20CInfoPlacementHelper25DrawDebugGeometryOverlaysEv
void CInfoPlacementHelper::DrawDebugGeometryOverlays()
{
	int g;  // line 235
	// inlined CBaseEntity::GetAbsAngles() at line 236
	// inlined CInfoPlacementHelper::IsEnabled() at line 235
	// inlined CBaseEntity::GetAbsOrigin() at line 233
	// inlined CBaseEntity::GetAbsAngles() at line 233
	// inlined CBaseEntity::GetAbsOrigin() at line 236
}

// game/server/portal2/info_placement_helper.cpp:244 (declaration)
void IsEnabled();

// game/server/portal2/info_placement_helper.cpp:244 @0x717f90 _ZN20CInfoPlacementHelper9IsEnabledEv
bool CInfoPlacementHelper::IsEnabled()
{
}

// game/server/portal2/info_placement_helper.cpp:261 @0x7193b0 _ZN20CInfoPlacementHelper17NotifyPortalEventE13PortalEvent_tP14CPortal_Base2D
void CInfoPlacementHelper::NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier )
{
	float flResetDelay;  // line 278
	// inlined CNetworkVarBase<float,CInfoPlacementHelper::NetworkVar_m_flDisableTime>::operator=<float>() at line 279
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDeferringToPortal>::operator=<bool>() at line 275
	// inlined CHandle<CBaseEntity>::CHandle() at line 274
	// inlined CInfoPlacementHelper::GetTargetOrigin() at line 269
	// inlined CBaseEntity::GetAbsOrigin() at line 269
	// inlined Vector::operator-() at line 269
}

// game/server/portal2/info_placement_helper.cpp:285 @0x7198a0 _ZN20CInfoPlacementHelper12BindToPortalEP14CPortal_Base2D
void CInfoPlacementHelper::BindToPortal( CPortal_Base2D *pPortal )
{
	// inlined CHandle<CBaseEntity>::CHandle() at line 295
	// inlined CNetworkVarBase<bool,CInfoPlacementHelper::NetworkVar_m_bDeferringToPortal>::operator=<bool>() at line 297
}

// public/tier1/utlmemory.h:707 @0x719aa0 _ZN10CUtlMemoryI7CHandleI20CInfoPlacementHelperEiE4GrowEi
void CUtlMemory<CHandle<CInfoPlacementHelper>,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<CHandle<CInfoPlacementHelper>,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
