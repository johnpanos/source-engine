// DWARF declaration skeleton for game/server/portal2/baseprojectedentity.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xa7e50 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ServerClass::ServerClass() at line 34
}

// game/server/portal2/baseprojectedentity.cpp:13 (declaration)
void GetBaseMap();

// game/server/portal2/baseprojectedentity.cpp:13 @0x706fc0 _ZN20CBaseProjectedEntity14GetDataDescMapEv
datamap_t *CBaseProjectedEntity::GetDataDescMap()
{
}

// game/server/portal2/baseprojectedentity.cpp:13 @0x706fd0 _ZN20CBaseProjectedEntity10GetBaseMapEv
datamap_t *CBaseProjectedEntity::GetBaseMap()
{
}

// game/server/portal2/baseprojectedentity.cpp:13 @0xa7d80 _Z11DataMapInitI20CBaseProjectedEntityEP9datamap_tPT_
datamap_t *DataMapInit<CBaseProjectedEntity>( CBaseProjectedEntity * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 13
	CDatadescGeneratedNameHolder nameHolder;  // line 13
	typedescription_t dataDesc[12];  // line 13
}

// game/server/portal2/baseprojectedentity.cpp:13
datamap_t *g_DataMapHolder;

// game/server/portal2/baseprojectedentity.cpp:13
void CBaseProjectedEntity::m_DataMap;

// game/server/portal2/baseprojectedentity.cpp:34 @0x706fe0 _ZN20CBaseProjectedEntity14GetServerClassEv
ServerClass *CBaseProjectedEntity::GetServerClass()
{
}

// game/server/portal2/baseprojectedentity.cpp:34 @0x706ff0 _ZN20CBaseProjectedEntity40YouForgotToImplementOrDeclareServerClassEv
int CBaseProjectedEntity::YouForgotToImplementOrDeclareServerClass()
{
}

// game/server/portal2/baseprojectedentity.cpp:34 @0xa7710 _Z15ServerClassInitIN22DT_BaseProjectedEntity7ignoredEEiPT_
int ServerClassInit<DT_BaseProjectedEntity::ignored>( DT_BaseProjectedEntity::ignored * )
{
	SendTable &sendTable;  // line 34
	char *const g_pSendTableName;  // line 34
	SendProp g_SendProps[17];  // line 34
}

// game/server/portal2/baseprojectedentity.cpp:34
SendTable g_SendTable;

// game/server/portal2/baseprojectedentity.cpp:34
int g_SendTableInit;

// game/server/portal2/baseprojectedentity.cpp:34
static ServerClass g_CBaseProjectedEntity_ClassReg;

// game/server/portal2/baseprojectedentity.cpp:34
void CBaseProjectedEntity::m_pClassSendTable;

// game/server/portal2/baseprojectedentity.cpp:59 (declaration)
void CBaseProjectedEntity();

// game/server/portal2/baseprojectedentity.cpp:59 @0x707030 _ZN20CBaseProjectedEntityC2Ev
CBaseProjectedEntity::CBaseProjectedEntity()
{
	// inlined CNetworkHandleBase<CPortal_Base2D,CBaseProjectedEntity::NetworkVar_m_hHitPortal>::CNetworkHandleBase() at line 59
	// inlined CNetworkHandleBase<CPortal_Base2D,CBaseProjectedEntity::NetworkVar_m_hSourcePortal>::CNetworkHandleBase() at line 59
	// inlined CNetworkHandleBase<CBaseProjectedEntity,CBaseProjectedEntity::NetworkVar_m_hChildSegment>::CNetworkHandleBase() at line 59
	// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::CNetworkHandleBase() at line 59
}

// game/server/portal2/baseprojectedentity.cpp:59 @0x7075a0 _ZN20CBaseProjectedEntityC1Ev
CBaseProjectedEntity::CBaseProjectedEntity()
{
	// inlined CBaseProjectedEntity::CBaseProjectedEntity() at line 61
}

// game/server/portal2/baseprojectedentity.cpp:63 @0x707000 _ZN20CBaseProjectedEntity5SpawnEv
void CBaseProjectedEntity::Spawn()
{
}

// game/server/portal2/baseprojectedentity.cpp:69 @0x707260 _ZN20CBaseProjectedEntity9OnRestoreEv
void CBaseProjectedEntity::OnRestore()
{
	{
		bool bCreatePlacement;  // line 75
		// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::operator CInfoPlacementHelper*() at line 85
		// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::operator CInfoPlacementHelper*() at line 81
		// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::operator=() at line 78
		// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::Get() at line 75
	}
}

// game/server/portal2/baseprojectedentity.cpp:90 @0x7071b0 _ZN20CBaseProjectedEntity14UpdateOnRemoveEv
void CBaseProjectedEntity::UpdateOnRemove()
{
	// inlined CNetworkHandleBase<CBaseProjectedEntity,CBaseProjectedEntity::NetworkVar_m_hChildSegment>::Get() at line 92
	// inlined CNetworkHandleBase<CInfoPlacementHelper,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>::Get() at line 97
}

// game/server/portal2/baseprojectedentity.cpp:105 @0x7070a0 _ZN20CBaseProjectedEntity20PlacePlacementHelperEP20CInfoPlacementHelper
void CBaseProjectedEntity::PlacePlacementHelper( CInfoPlacementHelper *pHelper )
{
	Vector vecPosition;  // line 107
	{
		Vector vForward;  // line 112
		// inlined Vector::operator*() at line 114
		// inlined Vector::operator-() at line 114
		// inlined Vector::operator VectorByValue&() at line 114
	}
}
