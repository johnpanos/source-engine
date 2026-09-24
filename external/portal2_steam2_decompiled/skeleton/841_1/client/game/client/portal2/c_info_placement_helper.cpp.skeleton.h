// DWARF declaration skeleton for game/client/portal2/c_info_placement_helper.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x73fe0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 16
	// inlined CUtlVector<C_InfoPlacementHelper*,CUtlMemory<C_InfoPlacementHelper*, int> >::CUtlVector() at line 37
}

// game/client/portal2/c_info_placement_helper.cpp:16 @0x51bcd0 _ZN21C_InfoPlacementHelper40YouForgotToImplementOrDeclareClientClassEv
int C_InfoPlacementHelper::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_info_placement_helper.cpp:16 @0x51bcf0 _ZN21C_InfoPlacementHelper14GetClientClassEv
ClientClass *C_InfoPlacementHelper::GetClientClass()
{
}

// game/client/portal2/c_info_placement_helper.cpp:16 @0x73cc0 _Z15ClientClassInitIN22DT_InfoPlacementHelper7ignoredEEiPT_
int ClientClassInit<DT_InfoPlacementHelper::ignored>( DT_InfoPlacementHelper::ignored * )
{
	char *pRecvTableName;  // line 16
	RecvTable &RecvTable;  // line 16
	RecvProp RecvProps[12];  // line 16
}

// game/client/portal2/c_info_placement_helper.cpp:16
RecvTable g_RecvTable;

// game/client/portal2/c_info_placement_helper.cpp:16
int g_RecvTableInit;

// game/client/portal2/c_info_placement_helper.cpp:16 @0x51c1e0 _ZL35_C_InfoPlacementHelper_CreateObjectii
IClientNetworkable *_C_InfoPlacementHelper_CreateObject( int entnum, int serialNum )
{
	C_InfoPlacementHelper *pRet;  // line 16
	// inlined C_InfoPlacementHelper::C_InfoPlacementHelper() at line 16
}

// game/client/portal2/c_info_placement_helper.cpp:16
ClientClass __g_C_InfoPlacementHelperClientClass;

// game/client/portal2/c_info_placement_helper.cpp:16
void C_InfoPlacementHelper::m_pClassRecvTable;

// game/client/portal2/c_info_placement_helper.cpp:37
CUtlVector<C_InfoPlacementHelper*,CUtlMemory<C_InfoPlacementHelper*, int> > s_PlacementHelpers;

// game/client/portal2/c_info_placement_helper.cpp:43 @0x51c010 _Z24UTIL_FindPlacementHelperRK6VectorP12C_BasePlayer
C_InfoPlacementHelper *UTIL_FindPlacementHelper( const Vector &vecEndPoint, C_BasePlayer *pPlayer )
{
	Vector vecEyeDir;  // line 49
	Vector vecEyePos;  // line 49
	C_InfoPlacementHelper *pBestHelper;  // line 52
	float flBestDist;  // line 53
	{
		int i;  // line 56
		{
			C_InfoPlacementHelper *pHelper;  // line 59
			Vector vecTargetDir;  // line 68
			float flTargetDist;  // line 69
			float flToleranceAngle;  // line 71
			float flTargetAngle;  // line 72
			// inlined C_InfoPlacementHelper::IsEnabled() at line 65
			// inlined Vector::operator-() at line 68
		}
	}
}

// game/client/portal2/c_info_placement_helper.cpp:93 (declaration)
void C_InfoPlacementHelper();

// game/client/portal2/c_info_placement_helper.cpp:93 @0x51be30 _ZN21C_InfoPlacementHelperC1Ev
C_InfoPlacementHelper::C_InfoPlacementHelper()
{
}

// game/client/portal2/c_info_placement_helper.cpp:93 @0x51bec0 _ZN21C_InfoPlacementHelperC2Ev
C_InfoPlacementHelper::C_InfoPlacementHelper()
{
}

// game/client/portal2/c_info_placement_helper.cpp:104 @0x51bf50 _ZN21C_InfoPlacementHelper5SpawnEv
void C_InfoPlacementHelper::Spawn()
{
	// inlined CUtlVector<C_InfoPlacementHelper*,CUtlMemory<C_InfoPlacementHelper*, int> >::AddToTail() at line 108
}

// game/client/portal2/c_info_placement_helper.cpp:114 @0x51bd90 _ZN21C_InfoPlacementHelper14UpdateOnRemoveEv
void C_InfoPlacementHelper::UpdateOnRemove()
{
	// inlined CUtlVector<C_InfoPlacementHelper*,CUtlMemory<C_InfoPlacementHelper*, int> >::FindAndFastRemove() at line 118
}

// game/client/portal2/c_info_placement_helper.cpp:124 @0x51bd00 _ZN21C_InfoPlacementHelper21ShouldUseHelperAnglesEv
bool C_InfoPlacementHelper::ShouldUseHelperAngles()
{
}

// game/client/portal2/c_info_placement_helper.cpp:134 (declaration)
void IsEnabled();

// game/client/portal2/c_info_placement_helper.cpp:134 @0x51bd10 _ZN21C_InfoPlacementHelper9IsEnabledEv
bool C_InfoPlacementHelper::IsEnabled()
{
}

// game/client/portal2/c_info_placement_helper.cpp:151 @0x51bd70 _ZN21C_InfoPlacementHelper12BindToPortalEP15C_Portal_Base2D
void C_InfoPlacementHelper::BindToPortal( C_Portal_Base2D *pPortal )
{
}

// public/tier1/utlmemory.h:707 @0x51c390 _ZN10CUtlMemoryIP21C_InfoPlacementHelperiE4GrowEi
void CUtlMemory<C_InfoPlacementHelper*,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<C_InfoPlacementHelper*,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
