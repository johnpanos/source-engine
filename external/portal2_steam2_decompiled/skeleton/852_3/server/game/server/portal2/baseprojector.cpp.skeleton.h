// DWARF declaration skeleton for game/server/portal2/baseprojector.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xbd660 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CUtlVector<CBaseProjector*,CUtlMemory<CBaseProjector*, int> >::CUtlVector() at line 39
}

// game/server/portal2/baseprojector.cpp:25
ConVar sv_projected_entities_use_placement_helper;

// game/server/portal2/baseprojector.cpp:27 (declaration)
void GetBaseMap();

// game/server/portal2/baseprojector.cpp:27 @0x6aedc0 _ZN14CBaseProjector14GetDataDescMapEv
datamap_t *CBaseProjector::GetDataDescMap()
{
}

// game/server/portal2/baseprojector.cpp:27 @0x6aedd0 _ZN14CBaseProjector10GetBaseMapEv
datamap_t *CBaseProjector::GetBaseMap()
{
}

// game/server/portal2/baseprojector.cpp:27 @0xbd360 _Z11DataMapInitI14CBaseProjectorEP9datamap_tPT_
datamap_t *DataMapInit<CBaseProjector>( CBaseProjector * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 37
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 27
	CDatadescGeneratedNameHolder nameHolder;  // line 27
	typedescription_t dataDesc[6];  // line 27
}

// game/server/portal2/baseprojector.cpp:27
datamap_t *g_DataMapHolder;

// game/server/portal2/baseprojector.cpp:27
void CBaseProjector::m_DataMap;

// game/server/portal2/baseprojector.cpp:39
static CUtlVector<CBaseProjector*,CUtlMemory<CBaseProjector*, int> > s_AllProjectors;

// game/server/portal2/baseprojector.cpp:42 (declaration)
void CBaseProjector();

// game/server/portal2/baseprojector.cpp:42 @0x6af180 _ZN14CBaseProjectorC2Ev
CBaseProjector::CBaseProjector()
{
	// inlined CUtlVector<CBaseProjector*,CUtlMemory<CBaseProjector*, int> >::AddToTail() at line 45
	// inlined CHandle<CBaseProjectedEntity>::CHandle() at line 43
	// inlined CNetworkVarBase<bool,CBaseProjector::NetworkVar_m_bEnabled>::CNetworkVarBase() at line 43
}

// game/server/portal2/baseprojector.cpp:42 @0x6af3a0 _ZN14CBaseProjectorC1Ev
CBaseProjector::CBaseProjector()
{
}

// game/server/portal2/baseprojector.cpp:48 (declaration)
~CBaseProjector();

// game/server/portal2/baseprojector.cpp:48 @0x6af480 _ZN14CBaseProjectorD2Ev
CBaseProjector::~CBaseProjector()
{
	// inlined CBaseProjector::Cleanup() at line 50
}

// game/server/portal2/baseprojector.cpp:48 @0x6af6c0 _ZN14CBaseProjectorD0Ev
CBaseProjector::~CBaseProjector()
{
	// inlined CBaseProjector::Cleanup() at line 50
}

// game/server/portal2/baseprojector.cpp:48 @0x6af760 _ZN14CBaseProjectorD1Ev
CBaseProjector::~CBaseProjector()
{
	// inlined CBaseProjector::Cleanup() at line 50
}

// game/server/portal2/baseprojector.cpp:53 @0x6af5f0 _ZN14CBaseProjector14UpdateOnRemoveEv
void CBaseProjector::UpdateOnRemove()
{
	// inlined CUtlVector<CBaseProjector*,CUtlMemory<CBaseProjector*, int> >::FindAndFastRemove() at line 55
	// inlined CBaseProjector::Cleanup() at line 56
}

// game/server/portal2/baseprojector.cpp:62 @0x6af3b0 _ZN14CBaseProjector26TestProjectionChangesThinkEv
void CBaseProjector::TestProjectionChangesThink()
{
	// inlined CBaseProjector::TestAllForProjectionChanges() at line 66
}

// game/server/portal2/baseprojector.cpp:70 @0x6aef30 _ZN14CBaseProjector5SpawnEv
void CBaseProjector::Spawn()
{
}

// game/server/portal2/baseprojector.cpp:77 @0x6aefb0 _ZN14CBaseProjector8ActivateEv
void CBaseProjector::Activate()
{
}

// game/server/portal2/baseprojector.cpp:86 @0x6aeee0 _ZN14CBaseProjector9OnRestoreEv
void CBaseProjector::OnRestore()
{
}

// game/server/portal2/baseprojector.cpp:95 @0x6af000 _ZN14CBaseProjector7ProjectEv
void CBaseProjector::Project()
{
	bool bCreateNew;  // line 97
	// inlined CHandle<CBaseProjectedEntity>::Get() at line 97
	// inlined CHandle<CBaseProjectedEntity>::operator=() at line 100
	// inlined CHandle<CBaseProjectedEntity>::Get() at line 101
	// inlined CBaseEntity::GetAbsAngles() at line 108
	// inlined CHandle<CBaseProjectedEntity>::Get() at line 108
	// inlined CBaseEntity::GetAbsOrigin() at line 108
}

// game/server/portal2/baseprojector.cpp:111 (declaration)
void Cleanup();

// game/server/portal2/baseprojector.cpp:111 @0x6aee80 _ZN14CBaseProjector7CleanupEv
void CBaseProjector::Cleanup()
{
	// inlined CHandle<CBaseProjectedEntity>::Get() at line 113
}

// game/server/portal2/baseprojector.cpp:119 @0x6aee70 _ZN14CBaseProjector21DrawDebugTextOverlaysEv
int CBaseProjector::DrawDebugTextOverlays()
{
	int text_offset;  // line 121
}

// game/server/portal2/baseprojector.cpp:126 (declaration)
void TestAllForProjectionChanges();

// game/server/portal2/baseprojector.cpp:126 @0x6aede0 _ZN14CBaseProjector27TestAllForProjectionChangesEv
void CBaseProjector::TestAllForProjectionChanges()
{
	{
		int i;  // line 128
		{
			CBaseProjectedEntity *pFirstChild;  // line 130
			// inlined CHandle<CBaseProjectedEntity>::Get() at line 130
		}
	}
}

// game/server/portal2/baseprojector.cpp:139 @0x6af890 _ZN14CBaseProjector11InputEnableER11inputdata_t
void CBaseProjector::InputEnable( inputdata_t &data )
{
	// inlined CBaseProjector::EnableProjection() at line 141
}

// game/server/portal2/baseprojector.cpp:145 @0x6af7f0 _ZN14CBaseProjector12InputDisableER11inputdata_t
void CBaseProjector::InputDisable( inputdata_t &data )
{
	// inlined CBaseProjector::EnableProjection() at line 147
}

// game/server/portal2/baseprojector.cpp:151 (declaration)
void EnableProjection( bool bEnable );

// game/server/portal2/baseprojector.cpp:151 @0x6af510 _ZN14CBaseProjector16EnableProjectionEb
void CBaseProjector::EnableProjection( bool bEnable )
{
	// inlined CNetworkVarBase<bool,CBaseProjector::NetworkVar_m_bEnabled>::operator=<bool>() at line 153
	// inlined CBaseProjector::Cleanup() at line 161
}
