// DWARF declaration skeleton for game/server/portal2/func_portalled.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xa9840 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CFunc_Portalled>::CEntityFactory() at line 17
}

// game/server/portal2/func_portalled.cpp:17
static CEntityFactory<CFunc_Portalled> func_portalled;

// game/server/portal2/func_portalled.cpp:19 (declaration)
void GetBaseMap();

// game/server/portal2/func_portalled.cpp:19 @0x7129e0 _ZN15CFunc_Portalled14GetDataDescMapEv
datamap_t *CFunc_Portalled::GetDataDescMap()
{
}

// game/server/portal2/func_portalled.cpp:19 @0x7129f0 _ZN15CFunc_Portalled10GetBaseMapEv
datamap_t *CFunc_Portalled::GetBaseMap()
{
}

// game/server/portal2/func_portalled.cpp:19 @0xa9710 _Z11DataMapInitI15CFunc_PortalledEP9datamap_tPT_
datamap_t *DataMapInit<CFunc_Portalled>( CFunc_Portalled * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 19
	CDatadescGeneratedNameHolder nameHolder;  // line 19
	typedescription_t dataDesc[6];  // line 19
}

// game/server/portal2/func_portalled.cpp:19
datamap_t *g_DataMapHolder;

// game/server/portal2/func_portalled.cpp:19
void CFunc_Portalled::m_DataMap;

// game/server/portal2/func_portalled.cpp:28 @0x712a00 _ZN15CFunc_Portalled24PortalPlacedInsideBoundsEP12CProp_Portal
void CFunc_Portalled::PortalPlacedInsideBounds( CProp_Portal *pPortal )
{
	// inlined CProp_Portal::SetFuncPortalled() at line 30
}

// game/server/portal2/func_portalled.cpp:33 @0x712ad0 _ZN15CFunc_Portalled14OnPrePortalledEP11CBaseEntityb
void CFunc_Portalled::OnPrePortalled( CBaseEntity *pOther, bool bDeparting )
{
}

// game/server/portal2/func_portalled.cpp:52 @0x712a40 _ZN15CFunc_Portalled15OnPostPortalledEP11CBaseEntityb
void CFunc_Portalled::OnPostPortalled( CBaseEntity *pOther, bool bDeparting )
{
}

// game/server/util.h:100 @0x712ba0 _ZN14CEntityFactoryI15CFunc_PortalledE6CreateEPKc
IServerNetworkable *CEntityFactory<CFunc_Portalled>::Create( const char *pClassName )
{
	CFunc_Portalled *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CFunc_Portalled>() at line 102
}

// game/server/util.h:106 @0x712b60 _ZN14CEntityFactoryI15CFunc_PortalledE7DestroyEP18IServerNetworkable
void CEntityFactory<CFunc_Portalled>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x712b90 _ZN14CEntityFactoryI15CFunc_PortalledE13GetEntitySizeEv
size_t CEntityFactory<CFunc_Portalled>::GetEntitySize()
{
}
