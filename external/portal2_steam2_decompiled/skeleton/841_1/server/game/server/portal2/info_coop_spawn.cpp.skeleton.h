// DWARF declaration skeleton for game/server/portal2/info_coop_spawn.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xaa460 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CInfo_Coop_Spawn>::CEntityFactory() at line 52
	// inlined CUtlVector<CInfo_Coop_Spawn*,CUtlMemory<CInfo_Coop_Spawn*, int> >::CUtlVector() at line 54
}

// game/server/portal2/info_coop_spawn.cpp:22 sizeof=0x520 (i386)
struct CInfo_Coop_Spawn : public CBaseAnimating
{
public:
	virtual void Activate();  // line 27
	void InputEnable( inputdata_t & );  // line 28
	void InputDisable( inputdata_t & );  // line 29
	void InputSetAsActiveSpawn( inputdata_t & );  // line 30
	virtual void UpdateOnRemove();  // line 31
	bool StringCompare( const char *, const char * );  // line 32
	virtual int DrawDebugTextOverlays();  // line 34
	bool m_bEnabled; // +0x514  // line 36
	int m_iStartingTeam; // +0x518  // line 37
	bool m_bForceGunOnSpawn; // +0x51c  // line 38
};

// game/server/portal2/info_coop_spawn.cpp:22 (declaration)
~CInfo_Coop_Spawn();

// game/server/portal2/info_coop_spawn.cpp:22 @0x717da0 _ZN16CInfo_Coop_SpawnD0Ev
CInfo_Coop_Spawn::~CInfo_Coop_Spawn()
{
}

// game/server/portal2/info_coop_spawn.cpp:22 @0x717de0 _ZN16CInfo_Coop_SpawnD1Ev
CInfo_Coop_Spawn::~CInfo_Coop_Spawn()
{
}

// game/server/portal2/info_coop_spawn.cpp:22 (declaration)
void CInfo_Coop_Spawn();

// game/server/portal2/info_coop_spawn.cpp:25 @0x717850 _ZN16CInfo_Coop_Spawn14GetDataDescMapEv
datamap_t *CInfo_Coop_Spawn::GetDataDescMap()
{
}

// game/server/portal2/info_coop_spawn.cpp:25 @0x717860 _ZN16CInfo_Coop_Spawn10GetBaseMapEv
datamap_t *CInfo_Coop_Spawn::GetBaseMap()
{
}

// game/server/portal2/info_coop_spawn.cpp:27 @0x717b20 _ZN16CInfo_Coop_Spawn8ActivateEv
void CInfo_Coop_Spawn::Activate()
{
	// inlined CUtlVector<CInfo_Coop_Spawn*,CUtlMemory<CInfo_Coop_Spawn*, int> >::AddToTail() at line 58
}

// game/server/portal2/info_coop_spawn.cpp:28 @0x717870 _ZN16CInfo_Coop_Spawn11InputEnableER11inputdata_t
void CInfo_Coop_Spawn::InputEnable( inputdata_t &inputdata )
{
}

// game/server/portal2/info_coop_spawn.cpp:29 @0x717880 _ZN16CInfo_Coop_Spawn12InputDisableER11inputdata_t
void CInfo_Coop_Spawn::InputDisable( inputdata_t &inputdata )
{
}

// game/server/portal2/info_coop_spawn.cpp:30 @0x717c00 _ZN16CInfo_Coop_Spawn21InputSetAsActiveSpawnER11inputdata_t
void CInfo_Coop_Spawn::InputSetAsActiveSpawn( inputdata_t &inputdata )
{
	int iKnownLocations;  // line 79
	CInfo_Coop_Spawn **pKnownLocations;  // line 80
	// inlined CBaseEntity::GetAbsOrigin() at line 104
	{
		int i;  // line 84
		// inlined CBaseEntity::GetAbsOrigin() at line 97
	}
	// inlined CUtlVector<CInfo_Coop_Spawn*,CUtlMemory<CInfo_Coop_Spawn*, int> >::Base() at line 80
	// inlined CUtlVector<CInfo_Coop_Spawn*,CUtlMemory<CInfo_Coop_Spawn*, int> >::Count() at line 79
}

// game/server/portal2/info_coop_spawn.cpp:31 @0x717970 _ZN16CInfo_Coop_Spawn14UpdateOnRemoveEv
void CInfo_Coop_Spawn::UpdateOnRemove()
{
	// inlined CUtlVector<CInfo_Coop_Spawn*,CUtlMemory<CInfo_Coop_Spawn*, int> >::FindAndFastRemove() at line 111
}

// game/server/portal2/info_coop_spawn.cpp:34 @0x717890 _ZN16CInfo_Coop_Spawn21DrawDebugTextOverlaysEv
int CInfo_Coop_Spawn::DrawDebugTextOverlays()
{
	int text_offset;  // line 153
	char tempstr[255];  // line 155
}

// game/server/portal2/info_coop_spawn.cpp:41 @0xaa390 _Z11DataMapInitI16CInfo_Coop_SpawnEP9datamap_tPT_
datamap_t *DataMapInit<CInfo_Coop_Spawn>( CInfo_Coop_Spawn * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 41
	CDatadescGeneratedNameHolder nameHolder;  // line 41
	typedescription_t dataDesc[7];  // line 41
}

// game/server/portal2/info_coop_spawn.cpp:41
datamap_t *g_DataMapHolder;

// game/server/portal2/info_coop_spawn.cpp:52
static CEntityFactory<CInfo_Coop_Spawn> info_coop_spawn;

// game/server/portal2/info_coop_spawn.cpp:54
static CUtlVector<CInfo_Coop_Spawn*,CUtlMemory<CInfo_Coop_Spawn*, int> > s_SpawnLocations;

// game/server/util.h:100 @0x717e00 _ZN14CEntityFactoryI16CInfo_Coop_SpawnE6CreateEPKc
IServerNetworkable *CEntityFactory<CInfo_Coop_Spawn>::Create( const char *pClassName )
{
	CInfo_Coop_Spawn *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CInfo_Coop_Spawn>() at line 102
}

// game/server/util.h:106 @0x717d60 _ZN14CEntityFactoryI16CInfo_Coop_SpawnE7DestroyEP18IServerNetworkable
void CEntityFactory<CInfo_Coop_Spawn>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x717d90 _ZN14CEntityFactoryI16CInfo_Coop_SpawnE13GetEntitySizeEv
size_t CEntityFactory<CInfo_Coop_Spawn>::GetEntitySize()
{
}

// game/server/portal2/info_coop_spawn.cpp:115 @0x717a00 _Z20GetCoopSpawnLocationi
CBaseEntity *GetCoopSpawnLocation( int iTeam )
{
	int iKnownLocations;  // line 117
	CInfo_Coop_Spawn **pKnownLocations;  // line 118
	CInfo_Coop_Spawn **pValidLocations;  // line 119
	int iValidLocations;  // line 120
	CInfo_Coop_Spawn *pValidSpawn;  // line 137
	{
		int i;  // line 121
	}
	// inlined CUtlVector<CInfo_Coop_Spawn*,CUtlMemory<CInfo_Coop_Spawn*, int> >::Base() at line 118
	// inlined CUtlVector<CInfo_Coop_Spawn*,CUtlMemory<CInfo_Coop_Spawn*, int> >::Count() at line 117
}

// public/tier1/utlmemory.h:707 @0x717e70 _ZN10CUtlMemoryIP16CInfo_Coop_SpawniE4GrowEi
void CUtlMemory<CInfo_Coop_Spawn*,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<CInfo_Coop_Spawn*,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
