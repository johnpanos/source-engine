// DWARF declaration skeleton for game/client/portal2/c_baseprojectedentity.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x73540 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined ClientClass::ClientClass() at line 15
	// inlined PredMapInit<C_BaseProjectedEntity>() at line 35
	// inlined CUtlVector<C_BaseProjectedEntity*,CUtlMemory<C_BaseProjectedEntity*, int> >::CUtlVector() at line 52
}

// game/client/portal2/c_baseprojectedentity.cpp:15 @0x5200d0 _ZN21C_BaseProjectedEntity40YouForgotToImplementOrDeclareClientClassEv
int C_BaseProjectedEntity::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_baseprojectedentity.cpp:15 @0x5200f0 _ZN21C_BaseProjectedEntity14GetClientClassEv
ClientClass *C_BaseProjectedEntity::GetClientClass()
{
}

// game/client/portal2/c_baseprojectedentity.cpp:15 @0x73160 _Z15ClientClassInitIN22DT_BaseProjectedEntity7ignoredEEiPT_
int ClientClassInit<DT_BaseProjectedEntity::ignored>( DT_BaseProjectedEntity::ignored * )
{
	char *pRecvTableName;  // line 15
	RecvTable &RecvTable;  // line 15
	RecvProp RecvProps[15];  // line 15
}

// game/client/portal2/c_baseprojectedentity.cpp:15
RecvTable g_RecvTable;

// game/client/portal2/c_baseprojectedentity.cpp:15
int g_RecvTableInit;

// game/client/portal2/c_baseprojectedentity.cpp:15 @0x520560 _ZL35_C_BaseProjectedEntity_CreateObjectii
IClientNetworkable *_C_BaseProjectedEntity_CreateObject( int entnum, int serialNum )
{
	C_BaseProjectedEntity *pRet;  // line 15
	// inlined C_BaseProjectedEntity::C_BaseProjectedEntity() at line 15
}

// game/client/portal2/c_baseprojectedentity.cpp:15
ClientClass __g_C_BaseProjectedEntityClientClass;

// game/client/portal2/c_baseprojectedentity.cpp:15
void C_BaseProjectedEntity::m_pClassRecvTable;

// game/client/portal2/c_baseprojectedentity.cpp:35 @0x520100 _ZN21C_BaseProjectedEntity14GetPredDescMapEv
datamap_t *C_BaseProjectedEntity::GetPredDescMap()
{
}

// game/client/portal2/c_baseprojectedentity.cpp:35 (declaration)
datamap_t *PredMapInit<C_BaseProjectedEntity>( C_BaseProjectedEntity * );

// game/client/portal2/c_baseprojectedentity.cpp:35 @0x520110 _Z11PredMapInitI21C_BaseProjectedEntityEP9datamap_tPT_
datamap_t *PredMapInit<C_BaseProjectedEntity>( C_BaseProjectedEntity * )
{
}

// game/client/portal2/c_baseprojectedentity.cpp:35
datamap_t *g_PredMapHolder;

// game/client/portal2/c_baseprojectedentity.cpp:35
void C_BaseProjectedEntity::m_PredMap;

// game/client/portal2/c_baseprojectedentity.cpp:52
static CUtlVector<C_BaseProjectedEntity*,CUtlMemory<C_BaseProjectedEntity*, int> > s_AllProjectedEntities;

// game/client/portal2/c_baseprojectedentity.cpp:53 (declaration)
void C_BaseProjectedEntity();

// game/client/portal2/c_baseprojectedentity.cpp:53 @0x520430 _ZN21C_BaseProjectedEntityC2Ev
C_BaseProjectedEntity::C_BaseProjectedEntity()
{
	// inlined CUtlVector<C_BaseProjectedEntity*,CUtlMemory<C_BaseProjectedEntity*, int> >::AddToTail() at line 55
	// inlined CHandle<C_BaseEntity>::CHandle() at line 53
	// inlined CHandle<C_BaseProjectedEntity>::CHandle() at line 53
	// inlined CHandle<C_Portal_Base2D>::CHandle() at line 53
	// inlined CHandle<C_Portal_Base2D>::CHandle() at line 53
}

// game/client/portal2/c_baseprojectedentity.cpp:53 @0x520550 _ZN21C_BaseProjectedEntityC1Ev
C_BaseProjectedEntity::C_BaseProjectedEntity()
{
}

// game/client/portal2/c_baseprojectedentity.cpp:58 (declaration)
~C_BaseProjectedEntity();

// game/client/portal2/c_baseprojectedentity.cpp:58 @0x520360 _ZN21C_BaseProjectedEntityD2Ev
C_BaseProjectedEntity::~C_BaseProjectedEntity()
{
	// inlined CUtlVector<C_BaseProjectedEntity*,CUtlMemory<C_BaseProjectedEntity*, int> >::FindAndFastRemove() at line 60
}

// game/client/portal2/c_baseprojectedentity.cpp:58 @0x5205c0 _ZN21C_BaseProjectedEntityD0Ev
C_BaseProjectedEntity::~C_BaseProjectedEntity()
{
	// inlined CUtlVector<C_BaseProjectedEntity*,CUtlMemory<C_BaseProjectedEntity*, int> >::FindAndFastRemove() at line 60
}

// game/client/portal2/c_baseprojectedentity.cpp:58 @0x520680 _ZN21C_BaseProjectedEntityD1Ev
C_BaseProjectedEntity::~C_BaseProjectedEntity()
{
	// inlined CUtlVector<C_BaseProjectedEntity*,CUtlMemory<C_BaseProjectedEntity*, int> >::FindAndFastRemove() at line 60
}

// game/client/portal2/c_baseprojectedentity.cpp:63 @0x5202a0 _ZN21C_BaseProjectedEntity18GetPredictionOwnerEv
C_BasePlayer *C_BaseProjectedEntity::GetPredictionOwner()
{
	C_Portal_Base2D *pSourcePortal;  // line 65
	{
		C_Prop_Portal *pSourcePropPortal;  // line 68
		{
			C_BaseEntity *pFiredBy;  // line 71
			// inlined CHandle<C_BaseEntity>::Get() at line 71
		}
	}
	// inlined CHandle<C_Portal_Base2D>::operator C_Portal_Base2D*() at line 65
}

// game/client/portal2/c_baseprojectedentity.cpp:82 @0x5201a0 _ZN21C_BaseProjectedEntity14PostDataUpdateE16DataUpdateType_t
void C_BaseProjectedEntity::PostDataUpdate( DataUpdateType_t updateType )
{
	C_Portal_Base2D *pSourcePortal;  // line 86
	// inlined CHandle<C_Portal_Base2D>::Get() at line 86
	// inlined C_BaseEntity::GetSimulatingPlayer() at line 93
}

// game/client/portal2/c_baseprojectedentity.cpp:113 @0x520140 _ZN21C_BaseProjectedEntity27TestAllForProjectionChangesEv
void C_BaseProjectedEntity::TestAllForProjectionChanges()
{
	{
		int i;  // line 116
	}
}

// public/tier1/utlmemory.h:707 @0x520800 _ZN10CUtlMemoryIP21C_BaseProjectedEntityiE4GrowEi
void CUtlMemory<C_BaseProjectedEntity*,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<C_BaseProjectedEntity*,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
