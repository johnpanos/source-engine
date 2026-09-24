// DWARF declaration skeleton for game/server/portal2/baseprojectedentity.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xbd0e0 _Z41__static_initialization_and_destruction_0ii
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
	// inlined ServerClass::ServerClass() at line 31
}

// game/server/portal2/baseprojectedentity.cpp:13 (declaration)
void GetBaseMap();

// game/server/portal2/baseprojectedentity.cpp:13 @0x6ae7d0 _ZN20CBaseProjectedEntity14GetDataDescMapEv
datamap_t *CBaseProjectedEntity::GetDataDescMap()
{
}

// game/server/portal2/baseprojectedentity.cpp:13 @0x6ae7e0 _ZN20CBaseProjectedEntity10GetBaseMapEv
datamap_t *CBaseProjectedEntity::GetBaseMap()
{
}

// game/server/portal2/baseprojectedentity.cpp:13 @0xbd010 _Z11DataMapInitI20CBaseProjectedEntityEP9datamap_tPT_
datamap_t *DataMapInit<CBaseProjectedEntity>( CBaseProjectedEntity * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 13
	CDatadescGeneratedNameHolder nameHolder;  // line 13
	typedescription_t dataDesc[11];  // line 13
}

// game/server/portal2/baseprojectedentity.cpp:13
datamap_t *g_DataMapHolder;

// game/server/portal2/baseprojectedentity.cpp:13
void CBaseProjectedEntity::m_DataMap;

// game/server/portal2/baseprojectedentity.cpp:31 @0x6ae7f0 _ZN20CBaseProjectedEntity14GetServerClassEv
ServerClass *CBaseProjectedEntity::GetServerClass()
{
}

// game/server/portal2/baseprojectedentity.cpp:31 @0x6ae800 _ZN20CBaseProjectedEntity40YouForgotToImplementOrDeclareServerClassEv
int CBaseProjectedEntity::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/baseprojectedentity.cpp:31 @0xbc9a0 _Z15ServerClassInitIN22DT_BaseProjectedEntity7ignoredEEiPT_
int ServerClassInit<DT_BaseProjectedEntity::ignored>( DT_BaseProjectedEntity::ignored * )
{
	SendTable &sendTable;  // line 31
	char *const g_pSendTableName;  // line 31
	SendProp g_SendProps[17];  // line 31
}

// game/server/portal2/baseprojectedentity.cpp:31
SendTable g_SendTable;

// game/server/portal2/baseprojectedentity.cpp:31
int g_SendTableInit;

// game/server/portal2/baseprojectedentity.cpp:31
static ServerClass g_CBaseProjectedEntity_ClassReg;

// game/server/portal2/baseprojectedentity.cpp:31
void CBaseProjectedEntity::m_pClassSendTable;

// game/server/portal2/baseprojectedentity.cpp:56 (declaration)
void CBaseProjectedEntity();

// game/server/portal2/baseprojectedentity.cpp:56 @0x6ae840 _ZN20CBaseProjectedEntityC2Ev
CBaseProjectedEntity::CBaseProjectedEntity()
{
	// inlined CNetworkHandleBase<CPortal_Base2D,CBaseProjectedEntity::NetworkVar_m_hHitPortal>::CNetworkHandleBase() at line 56
	// inlined CNetworkHandleBase<CPortal_Base2D,CBaseProjectedEntity::NetworkVar_m_hSourcePortal>::CNetworkHandleBase() at line 56
	// inlined CNetworkHandleBase<CBaseProjectedEntity,CBaseProjectedEntity::NetworkVar_m_hChildSegment>::CNetworkHandleBase() at line 56
	// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::CNetworkHandleBase() at line 56
}

// game/server/portal2/baseprojectedentity.cpp:56 @0x6aec70 _ZN20CBaseProjectedEntityC1Ev
CBaseProjectedEntity::CBaseProjectedEntity()
{
	// inlined CBaseProjectedEntity::CBaseProjectedEntity() at line 58
}

// game/server/portal2/baseprojectedentity.cpp:60 @0x6ae810 _ZN20CBaseProjectedEntity5SpawnEv
void CBaseProjectedEntity::Spawn()
{
}

// game/server/portal2/baseprojectedentity.cpp:66 @0x6ae9c0 _ZN20CBaseProjectedEntity9OnRestoreEv
void CBaseProjectedEntity::OnRestore()
{
	bool bCreatePlacement;  // line 70
	// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::operator CInfoPlacementHelper*() at line 80
	// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::operator CInfoPlacementHelper*() at line 76
	// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::operator=() at line 73
	// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::Get() at line 70
}

// game/server/portal2/baseprojectedentity.cpp:84 @0x6aebc0 _ZN20CBaseProjectedEntity14UpdateOnRemoveEv
void CBaseProjectedEntity::UpdateOnRemove()
{
	// inlined CNetworkHandleBase<CBaseProjectedEntity,CBaseProjectedEntity::NetworkVar_m_hChildSegment>::Get() at line 86
	// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::Get() at line 91
}

// game/server/portal2/baseprojectedentity.cpp:99 @0x6ae8b0 _ZN20CBaseProjectedEntity20PlacePlacementHelperEP20CInfoPlacementHelper
void CBaseProjectedEntity::PlacePlacementHelper( CInfoPlacementHelper *pHelper )
{
	Vector vecPosition;  // line 101
	{
		Vector vForward;  // line 106
		// inlined Vector::operator*() at line 108
		// inlined Vector::operator-() at line 108
		// inlined Vector::operator VectorByValue&() at line 108
	}
}
