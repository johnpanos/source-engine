// DWARF declaration skeleton for game/server/portal2/propwallprojector.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb35e0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CPropWallProjector>::CEntityFactory() at line 15
}

// game/server/portal2/propwallprojector.cpp:15
static CEntityFactory<CPropWallProjector> prop_wall_projector;

// game/server/portal2/propwallprojector.cpp:19 @0x74f330 _ZN18CPropWallProjector5SpawnEv
void CPropWallProjector::Spawn()
{
	// inlined CBaseEntity::SetSolid() at line 26
}

// game/server/portal2/propwallprojector.cpp:32 @0x74f310 _ZN18CPropWallProjector8PrecacheEv
void CPropWallProjector::Precache()
{
}

// game/server/portal2/propwallprojector.cpp:38 @0x74f300 _ZN18CPropWallProjector24CreateNewProjectedEntityEv
CBaseProjectedEntity *CPropWallProjector::CreateNewProjectedEntity()
{
}

// game/server/util.h:100 @0x74f430 _ZN14CEntityFactoryI18CPropWallProjectorE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropWallProjector>::Create( const char *pClassName )
{
	CPropWallProjector *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropWallProjector>() at line 102
}

// game/server/util.h:106 @0x74f390 _ZN14CEntityFactoryI18CPropWallProjectorE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropWallProjector>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x74f3c0 _ZN14CEntityFactoryI18CPropWallProjectorE13GetEntitySizeEv
size_t CEntityFactory<CPropWallProjector>::GetEntitySize()
{
}
