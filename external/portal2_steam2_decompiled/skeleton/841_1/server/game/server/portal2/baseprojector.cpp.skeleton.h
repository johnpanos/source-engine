// DWARF declaration skeleton for game/server/portal2/baseprojector.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xa8330 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CUtlVector<CBaseProjector*,CUtlMemory<CBaseProjector*, int> >::CUtlVector() at line 41
}

// game/server/portal2/baseprojector.cpp:25
ConVar sv_projected_entities_use_placement_helper;

// game/server/portal2/baseprojector.cpp:27 (declaration)
void GetBaseMap();

// game/server/portal2/baseprojector.cpp:27 @0x7076f0 _ZN14CBaseProjector14GetDataDescMapEv
datamap_t *CBaseProjector::GetDataDescMap()
{
}

// game/server/portal2/baseprojector.cpp:27 @0x707700 _ZN14CBaseProjector10GetBaseMapEv
datamap_t *CBaseProjector::GetBaseMap()
{
}

// game/server/portal2/baseprojector.cpp:27 @0xa8030 _Z11DataMapInitI14CBaseProjectorEP9datamap_tPT_
datamap_t *DataMapInit<CBaseProjector>( CBaseProjector * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 39
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 27
	CDatadescGeneratedNameHolder nameHolder;  // line 27
	typedescription_t dataDesc[7];  // line 27
}

// game/server/portal2/baseprojector.cpp:27
datamap_t *g_DataMapHolder;

// game/server/portal2/baseprojector.cpp:27
void CBaseProjector::m_DataMap;

// game/server/portal2/baseprojector.cpp:41
static CUtlVector<CBaseProjector*,CUtlMemory<CBaseProjector*, int> > s_AllProjectors;

// game/server/portal2/baseprojector.cpp:44 (declaration)
void CBaseProjector();

// game/server/portal2/baseprojector.cpp:44 @0x707ae0 _ZN14CBaseProjectorC2Ev
CBaseProjector::CBaseProjector()
{
	// inlined CUtlVector<CBaseProjector*,CUtlMemory<CBaseProjector*, int> >::AddToTail() at line 47
	// inlined CHandle<CBaseProjectedEntity>::CHandle() at line 45
	// inlined CNetworkVarBase<bool,CBaseProjector::NetworkVar_m_bEnabled>::CNetworkVarBase() at line 45
}

// game/server/portal2/baseprojector.cpp:44 @0x707e10 _ZN14CBaseProjectorC1Ev
CBaseProjector::CBaseProjector()
{
}

// game/server/portal2/baseprojector.cpp:50 (declaration)
~CBaseProjector();

// game/server/portal2/baseprojector.cpp:50 @0x7081f0 _ZN14CBaseProjectorD0Ev
CBaseProjector::~CBaseProjector()
{
	// inlined CBaseProjector::Cleanup() at line 52
}

// game/server/portal2/baseprojector.cpp:50 @0x708290 _ZN14CBaseProjectorD1Ev
CBaseProjector::~CBaseProjector()
{
	// inlined CBaseProjector::Cleanup() at line 52
}

// game/server/portal2/baseprojector.cpp:50 @0x708320 _ZN14CBaseProjectorD2Ev
CBaseProjector::~CBaseProjector()
{
	// inlined CBaseProjector::Cleanup() at line 52
}

// game/server/portal2/baseprojector.cpp:55 @0x708120 _ZN14CBaseProjector14UpdateOnRemoveEv
void CBaseProjector::UpdateOnRemove()
{
	// inlined CUtlVector<CBaseProjector*,CUtlMemory<CBaseProjector*, int> >::FindAndFastRemove() at line 57
	// inlined CBaseProjector::Cleanup() at line 58
}

// game/server/portal2/baseprojector.cpp:64 @0x707e20 _ZN14CBaseProjector26TestProjectionChangesThinkEv
void CBaseProjector::TestProjectionChangesThink()
{
	// inlined CBaseProjector::TestAllForProjectionChanges() at line 68
}

// game/server/portal2/baseprojector.cpp:72 @0x707880 _ZN14CBaseProjector5SpawnEv
void CBaseProjector::Spawn()
{
}

// game/server/portal2/baseprojector.cpp:79 @0x707900 _ZN14CBaseProjector8ActivateEv
void CBaseProjector::Activate()
{
}

// game/server/portal2/baseprojector.cpp:88 @0x707830 _ZN14CBaseProjector9OnRestoreEv
void CBaseProjector::OnRestore()
{
}

// game/server/portal2/baseprojector.cpp:97 @0x707950 _ZN14CBaseProjector7ProjectEv
void CBaseProjector::Project()
{
	bool bCreateNew;  // line 99
	// inlined CHandle<CBaseProjectedEntity>::Get() at line 99
	// inlined CHandle<CBaseProjectedEntity>::operator=() at line 102
	// inlined CHandle<CBaseProjectedEntity>::Get() at line 103
	// inlined CBaseEntity::GetAbsAngles() at line 110
	// inlined CHandle<CBaseProjectedEntity>::Get() at line 110
	// inlined CBaseEntity::GetAbsOrigin() at line 110
}

// game/server/portal2/baseprojector.cpp:113 (declaration)
void Cleanup();

// game/server/portal2/baseprojector.cpp:113 @0x7077b0 _ZN14CBaseProjector7CleanupEv
void CBaseProjector::Cleanup()
{
	// inlined CHandle<CBaseProjectedEntity>::Get() at line 115
	// inlined CHandle<CBaseProjectedEntity>::operator=() at line 118
}

// game/server/portal2/baseprojector.cpp:122 @0x7077a0 _ZN14CBaseProjector21DrawDebugTextOverlaysEv
int CBaseProjector::DrawDebugTextOverlays()
{
	int text_offset;  // line 124
}

// game/server/portal2/baseprojector.cpp:129 (declaration)
void TestAllForProjectionChanges();

// game/server/portal2/baseprojector.cpp:129 @0x707710 _ZN14CBaseProjector27TestAllForProjectionChangesEv
void CBaseProjector::TestAllForProjectionChanges()
{
	{
		int i;  // line 131
		{
			CBaseProjectedEntity *pFirstChild;  // line 133
			// inlined CHandle<CBaseProjectedEntity>::Get() at line 133
		}
	}
}

// game/server/portal2/baseprojector.cpp:142 @0x7083b0 _ZN14CBaseProjector11InputEnableER11inputdata_t
void CBaseProjector::InputEnable( inputdata_t &data )
{
	// inlined CBaseProjector::EnableProjection() at line 144
}

// game/server/portal2/baseprojector.cpp:148 @0x708110 _ZN14CBaseProjector12InputDisableER11inputdata_t
void CBaseProjector::InputDisable( inputdata_t &data )
{
}

// game/server/portal2/baseprojector.cpp:154 (declaration)
void EnableProjection( bool bEnable );

// game/server/portal2/baseprojector.cpp:154 @0x707ef0 _ZN14CBaseProjector16EnableProjectionEb
void CBaseProjector::EnableProjection( bool bEnable )
{
	// inlined CNetworkVarBase<bool,CBaseProjector::NetworkVar_m_bEnabled>::operator=<bool>() at line 156
	// inlined CBaseProjector::Cleanup() at line 164
}
