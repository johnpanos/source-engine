// DWARF declaration skeleton for game/server/portal2/propwallprojector.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xc8960 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityFactory<CPropWallProjector>::CEntityFactory() at line 15
}

// game/server/portal2/propwallprojector.cpp:15
static CEntityFactory<CPropWallProjector> prop_wall_projector;

// game/server/portal2/propwallprojector.cpp:19 @0x6dd590 _ZN18CPropWallProjector5SpawnEv
void CPropWallProjector::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 26
}

// game/server/portal2/propwallprojector.cpp:32 @0x6dd570 _ZN18CPropWallProjector8PrecacheEv
void CPropWallProjector::Precache()
{
}

// game/server/portal2/propwallprojector.cpp:38 @0x6dd560 _ZN18CPropWallProjector24CreateNewProjectedEntityEv
CBaseProjectedEntity *CPropWallProjector::CreateNewProjectedEntity()
{
}

// game/server/util.h:99 @0x6dd690 _ZN14CEntityFactoryI18CPropWallProjectorE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropWallProjector>::Create( const char *pClassName )
{
	CPropWallProjector *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPropWallProjector>() at line 101
}

// game/server/util.h:105 @0x6dd5f0 _ZN14CEntityFactoryI18CPropWallProjectorE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropWallProjector>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6dd620 _ZN14CEntityFactoryI18CPropWallProjectorE13GetEntitySizeEv
size_t CEntityFactory<CPropWallProjector>::GetEntitySize()
{
}
