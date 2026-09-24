// DWARF declaration skeleton for game/server/portal2/prop_tractorbeam.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/prop_tractorbeam.cpp:0 (declaration)
void GetBaseMap();

// game/server/portal2/prop_tractorbeam.cpp:0 @0x6f5360 _ZN25CPropTractorBeamProjector14GetDataDescMapEv
datamap_t *CPropTractorBeamProjector::GetDataDescMap()
{
}

// game/server/portal2/prop_tractorbeam.cpp:0 @0x6f5370 _ZN25CPropTractorBeamProjector10GetBaseMapEv
datamap_t *CPropTractorBeamProjector::GetBaseMap()
{
}

// None:0 @0xce360 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityFactory<CPropTractorBeamProjector>::CEntityFactory() at line 31
	// inlined ServerClass::ServerClass() at line 33
}

// game/server/portal2/prop_tractorbeam.cpp:0
void CPropTractorBeamProjector::m_DataMap;

// game/server/portal2/prop_tractorbeam.cpp:23 @0xce290 _Z11DataMapInitI25CPropTractorBeamProjectorEP9datamap_tPT_
datamap_t *DataMapInit<CPropTractorBeamProjector>( CPropTractorBeamProjector * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 23
	CDatadescGeneratedNameHolder nameHolder;  // line 23
	typedescription_t dataDesc[3];  // line 23
}

// game/server/portal2/prop_tractorbeam.cpp:23
datamap_t *g_DataMapHolder;

// game/server/portal2/prop_tractorbeam.cpp:31
static CEntityFactory<CPropTractorBeamProjector> prop_tractor_beam;

// game/server/portal2/prop_tractorbeam.cpp:33 @0x6f5380 _ZN25CPropTractorBeamProjector14GetServerClassEv
ServerClass *CPropTractorBeamProjector::GetServerClass()
{
}

// game/server/portal2/prop_tractorbeam.cpp:33 @0x6f5390 _ZN25CPropTractorBeamProjector40YouForgotToImplementOrDeclareServerClassEv
int CPropTractorBeamProjector::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/prop_tractorbeam.cpp:33 @0xce010 _Z15ServerClassInitIN27DT_PropTractorBeamProjector7ignoredEEiPT_
int ServerClassInit<DT_PropTractorBeamProjector::ignored>( DT_PropTractorBeamProjector::ignored * )
{
	SendTable &sendTable;  // line 33
	char *const g_pSendTableName;  // line 33
	SendProp g_SendProps[5];  // line 33
}

// game/server/portal2/prop_tractorbeam.cpp:33
SendTable g_SendTable;

// game/server/portal2/prop_tractorbeam.cpp:33
int g_SendTableInit;

// game/server/portal2/prop_tractorbeam.cpp:33
static ServerClass g_CPropTractorBeamProjector_ClassReg;

// game/server/portal2/prop_tractorbeam.cpp:33
void CPropTractorBeamProjector::m_pClassSendTable;

// game/server/portal2/prop_tractorbeam.cpp:42 @0x6f5400 _ZN25CPropTractorBeamProjector5SpawnEv
void CPropTractorBeamProjector::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 49
}

// game/server/portal2/prop_tractorbeam.cpp:61 @0x6f53c0 _ZN25CPropTractorBeamProjector8PrecacheEv
void CPropTractorBeamProjector::Precache()
{
}

// game/server/portal2/prop_tractorbeam.cpp:73 @0x6f53b0 _ZN25CPropTractorBeamProjector8ActivateEv
void CPropTractorBeamProjector::Activate()
{
}

// game/server/portal2/prop_tractorbeam.cpp:81 @0x6f53a0 _ZN25CPropTractorBeamProjector24CreateNewProjectedEntityEv
CBaseProjectedEntity *CPropTractorBeamProjector::CreateNewProjectedEntity()
{
}

// game/server/portal2/prop_tractorbeam.cpp:89 @0x6f5580 _ZN25CPropTractorBeamProjector19InputSetLinearForceER11inputdata_t
void CPropTractorBeamProjector::InputSetLinearForce( inputdata_t &indata )
{
	// inlined variant_t::Float() at line 92
	// inlined CNetworkVarBase<float,CPropTractorBeamProjector::NetworkVar_m_flLinearForce>::operator=<float>() at line 92
	// inlined CNetworkVarBase<Vector,CPropTractorBeamProjector::NetworkVar_m_vEndPos>::operator=<Vector>() at line 97
}

// game/server/util.h:99 @0x6f57b0 _ZN14CEntityFactoryI25CPropTractorBeamProjectorE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropTractorBeamProjector>::Create( const char *pClassName )
{
	CPropTractorBeamProjector *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPropTractorBeamProjector>() at line 101
}

// game/server/util.h:105 @0x6f5710 _ZN14CEntityFactoryI25CPropTractorBeamProjectorE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropTractorBeamProjector>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/portal2/prop_tractorbeam.cpp:106 @0x6f5470 _ZN25CPropTractorBeamProjector7ProjectEv
void CPropTractorBeamProjector::Project()
{
	// inlined CHandle<CBaseProjectedEntity>::operator->() at line 110
	// inlined CNetworkVarBase<Vector,CPropTractorBeamProjector::NetworkVar_m_vEndPos>::operator=<Vector>() at line 110
}

// game/server/util.h:113 @0x6f5740 _ZN14CEntityFactoryI25CPropTractorBeamProjectorE13GetEntitySizeEv
size_t CEntityFactory<CPropTractorBeamProjector>::GetEntitySize()
{
}
