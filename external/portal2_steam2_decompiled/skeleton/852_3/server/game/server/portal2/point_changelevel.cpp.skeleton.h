// DWARF declaration skeleton for game/server/portal2/point_changelevel.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xc4640 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityFactory<CInfoLandmark>::CEntityFactory() at line 33
	// inlined CEntityFactory<CInfoLandmark>::CEntityFactory() at line 34
	// inlined CEntityFactory<CPointChangelevel>::CEntityFactory() at line 61
}

// game/server/portal2/point_changelevel.cpp:15
static char st_szOriginMap[32];

// game/server/portal2/point_changelevel.cpp:16
static char st_szDestinationMap[32];

// game/server/portal2/point_changelevel.cpp:22 sizeof=0x3c0 (i386)
struct CInfoLandmark : public CPointEntity
{
public:
	virtual void Activate();  // line 25
};

// game/server/portal2/point_changelevel.cpp:22 (declaration)
void CInfoLandmark();

// game/server/portal2/point_changelevel.cpp:22 (declaration)
~CInfoLandmark();

// game/server/portal2/point_changelevel.cpp:22 @0x6d3030 _ZN13CInfoLandmarkD0Ev
CInfoLandmark::~CInfoLandmark()
{
	// inlined CPointEntity::~CPointEntity() at line 22
}

// game/server/portal2/point_changelevel.cpp:22 @0x6d3080 _ZN13CInfoLandmarkD1Ev
CInfoLandmark::~CInfoLandmark()
{
	// inlined CPointEntity::~CPointEntity() at line 22
}

// game/server/portal2/point_changelevel.cpp:25 @0x6d2fb0 _ZN13CInfoLandmark8ActivateEv
void CInfoLandmark::Activate()
{
	// inlined CNetworkVarBase<string_t,CBaseEntity::NetworkVar_m_iName>::operator=<string_t>() at line 28
}

// game/server/portal2/point_changelevel.cpp:33
static CEntityFactory<CInfoLandmark> info_landmark_entry;

// game/server/portal2/point_changelevel.cpp:34
static CEntityFactory<CInfoLandmark> info_landmark_exit;

// game/server/portal2/point_changelevel.cpp:40 sizeof=0x3d8 (i386)
struct CPointChangelevel : public CPointEntity
{
private:
	void InputChangeLevel( inputdata_t & );  // line 46
	COutputEvent m_OnChangeLevel; // +0x3c0  // line 49
};

// game/server/portal2/point_changelevel.cpp:40 (declaration)
void CPointChangelevel();

// game/server/portal2/point_changelevel.cpp:40 (declaration)
~CPointChangelevel();

// game/server/portal2/point_changelevel.cpp:40 @0x6d30b0 _ZN17CPointChangelevelD0Ev
CPointChangelevel::~CPointChangelevel()
{
	// inlined COutputEvent::~COutputEvent() at line 40
	// inlined CPointEntity::~CPointEntity() at line 40
	// inlined CPointEntity::~CPointEntity() at line 40
}

// game/server/portal2/point_changelevel.cpp:40 @0x6d3130 _ZN17CPointChangelevelD1Ev
CPointChangelevel::~CPointChangelevel()
{
	// inlined COutputEvent::~COutputEvent() at line 40
	// inlined CPointEntity::~CPointEntity() at line 40
	// inlined CPointEntity::~CPointEntity() at line 40
}

// game/server/portal2/point_changelevel.cpp:43 @0x6d2cf0 _ZN17CPointChangelevel14GetDataDescMapEv
datamap_t *CPointChangelevel::GetDataDescMap()
{
}

// game/server/portal2/point_changelevel.cpp:43 @0x6d2d00 _ZN17CPointChangelevel10GetBaseMapEv
datamap_t *CPointChangelevel::GetBaseMap()
{
}

// game/server/portal2/point_changelevel.cpp:46 @0x6d2d40 _ZN17CPointChangelevel16InputChangeLevelER11inputdata_t
void CPointChangelevel::InputChangeLevel( inputdata_t &inputdata )
{
	// inlined string_t::ToCStr() at line 72
	// inlined variant_t::String() at line 73
}

// game/server/portal2/point_changelevel.cpp:53 @0xc4520 _Z11DataMapInitI17CPointChangelevelEP9datamap_tPT_
datamap_t *DataMapInit<CPointChangelevel>( CPointChangelevel * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 53
	CDatadescGeneratedNameHolder nameHolder;  // line 53
	typedescription_t dataDesc[3];  // line 53
}

// game/server/portal2/point_changelevel.cpp:53
datamap_t *g_DataMapHolder;

// game/server/portal2/point_changelevel.cpp:61
static CEntityFactory<CPointChangelevel> point_changelevel;

// game/server/portal2/point_changelevel.cpp:83 @0x6d2d10 _Z30ChangeLevel_DestinationMapNamev
const char *ChangeLevel_DestinationMapName()
{
}

// game/server/portal2/point_changelevel.cpp:88 @0x6d2d20 _Z25ChangeLevel_OriginMapNamev
const char *ChangeLevel_OriginMapName()
{
}

// game/server/portal2/point_changelevel.cpp:93 @0x6d2d30 _Z27ChangeLevel_GetLandmarkNamev
const char *ChangeLevel_GetLandmarkName()
{
}

// game/server/util.h:99 @0x6d2eb0 _ZN14CEntityFactoryI17CPointChangelevelE6CreateEPKc
IServerNetworkable *CEntityFactory<CPointChangelevel>::Create( const char *pClassName )
{
	CPointChangelevel *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPointChangelevel>() at line 101
}

// game/server/util.h:99 @0x6d2f40 _ZN14CEntityFactoryI13CInfoLandmarkE6CreateEPKc
IServerNetworkable *CEntityFactory<CInfoLandmark>::Create( const char *pClassName )
{
	CInfoLandmark *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CInfoLandmark>() at line 101
}

// game/server/util.h:105 @0x6d2e30 _ZN14CEntityFactoryI17CPointChangelevelE7DestroyEP18IServerNetworkable
void CEntityFactory<CPointChangelevel>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:105 @0x6d2e70 _ZN14CEntityFactoryI13CInfoLandmarkE7DestroyEP18IServerNetworkable
void CEntityFactory<CInfoLandmark>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6d2e60 _ZN14CEntityFactoryI17CPointChangelevelE13GetEntitySizeEv
size_t CEntityFactory<CPointChangelevel>::GetEntitySize()
{
}

// game/server/util.h:113 @0x6d2ea0 _ZN14CEntityFactoryI13CInfoLandmarkE13GetEntitySizeEv
size_t CEntityFactory<CInfoLandmark>::GetEntitySize()
{
}
