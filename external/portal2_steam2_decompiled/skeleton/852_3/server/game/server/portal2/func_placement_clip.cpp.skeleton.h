// DWARF declaration skeleton for game/server/portal2/func_placement_clip.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xbe7b0 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityFactory<CFuncPlacementClip>::CEntityFactory() at line 30
}

// game/server/portal2/func_placement_clip.cpp:11 sizeof=0x3c0 (i386)
struct CFuncPlacementClip : public CBaseEntity
{
public:
	virtual void Spawn();  // line 16
	virtual bool CreateVPhysics();  // line 17
	void InputEnable( inputdata_t & );  // line 19
	void InputDisable( inputdata_t & );  // line 20
};

// game/server/portal2/func_placement_clip.cpp:11 (declaration)
~CFuncPlacementClip();

// game/server/portal2/func_placement_clip.cpp:11 @0x6b68c0 _ZN18CFuncPlacementClipD0Ev
CFuncPlacementClip::~CFuncPlacementClip()
{
}

// game/server/portal2/func_placement_clip.cpp:11 @0x6b6900 _ZN18CFuncPlacementClipD1Ev
CFuncPlacementClip::~CFuncPlacementClip()
{
}

// game/server/portal2/func_placement_clip.cpp:11 (declaration)
void CFuncPlacementClip();

// game/server/portal2/func_placement_clip.cpp:14 @0x6b66d0 _ZN18CFuncPlacementClip14GetDataDescMapEv
datamap_t *CFuncPlacementClip::GetDataDescMap()
{
}

// game/server/portal2/func_placement_clip.cpp:14 @0x6b66e0 _ZN18CFuncPlacementClip10GetBaseMapEv
datamap_t *CFuncPlacementClip::GetBaseMap()
{
}

// game/server/portal2/func_placement_clip.cpp:16 @0x6b6780 _ZN18CFuncPlacementClip5SpawnEv
void CFuncPlacementClip::Spawn()
{
	// inlined string_t::ToCStr() at line 37
}

// game/server/portal2/func_placement_clip.cpp:17 @0x6b6740 _ZN18CFuncPlacementClip14CreateVPhysicsEv
bool CFuncPlacementClip::CreateVPhysics()
{
	// inlined CBaseEntity::SetSolid() at line 51
}

// game/server/portal2/func_placement_clip.cpp:19 @0x6b66f0 _ZN18CFuncPlacementClip11InputEnableER11inputdata_t
void CFuncPlacementClip::InputEnable( inputdata_t &data )
{
	IPhysicsObject *pPhys;  // line 59
	// inlined CBaseEntity::RemoveSolidFlags() at line 64
	// inlined CBaseEntity::VPhysicsGetObject() at line 59
}

// game/server/portal2/func_placement_clip.cpp:20 @0x6b6830 _ZN18CFuncPlacementClip12InputDisableER11inputdata_t
void CFuncPlacementClip::InputDisable( inputdata_t &data )
{
	IPhysicsObject *pPhys;  // line 69
	// inlined CBaseEntity::AddSolidFlags() at line 74
	// inlined CBaseEntity::VPhysicsGetObject() at line 69
}

// game/server/portal2/func_placement_clip.cpp:25 @0xbe6e0 _Z11DataMapInitI18CFuncPlacementClipEP9datamap_tPT_
datamap_t *DataMapInit<CFuncPlacementClip>( CFuncPlacementClip * )
{
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 25
	CDatadescGeneratedNameHolder nameHolder;  // line 25
	typedescription_t dataDesc[3];  // line 25
}

// game/server/portal2/func_placement_clip.cpp:25
datamap_t *g_DataMapHolder;

// game/server/portal2/func_placement_clip.cpp:30
static CEntityFactory<CFuncPlacementClip> func_placement_clip;

// game/server/util.h:99 @0x6b6920 _ZN14CEntityFactoryI18CFuncPlacementClipE6CreateEPKc
IServerNetworkable *CEntityFactory<CFuncPlacementClip>::Create( const char *pClassName )
{
	CFuncPlacementClip *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CFuncPlacementClip>() at line 101
}

// game/server/util.h:105 @0x6b6880 _ZN14CEntityFactoryI18CFuncPlacementClipE7DestroyEP18IServerNetworkable
void CEntityFactory<CFuncPlacementClip>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6b68b0 _ZN14CEntityFactoryI18CFuncPlacementClipE13GetEntitySizeEv
size_t CEntityFactory<CFuncPlacementClip>::GetEntitySize()
{
}
