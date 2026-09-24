// DWARF declaration skeleton for game/server/portal2/prop_physics_paintable.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xcd5e0 _Z41__static_initialization_and_destruction_0ii
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
	// inlined CEntityFactory<CPropPhysicsPaintable>::CEntityFactory() at line 18
}

// None:0 @0xcd750 _Z41__static_initialization_and_destruction_1ii
__static_initialization_and_destruction_1( int __initialize_p, int __priority )
{
}

// game/server/portal2/prop_physics_paintable.cpp:13 sizeof=0x884 (i386)
struct CPropPhysicsPaintable : public PlayerPickupPaintPowerUser<CPhysicsProp>
{
};

// game/server/portal2/prop_physics_paintable.cpp:13 (declaration)
~CPropPhysicsPaintable();

// game/server/portal2/prop_physics_paintable.cpp:13 @0x6f2e20 _ZN21CPropPhysicsPaintableD0Ev
CPropPhysicsPaintable::~CPropPhysicsPaintable()
{
	// inlined PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser() at line 13
}

// game/server/portal2/prop_physics_paintable.cpp:13 @0x6f3020 _ZN21CPropPhysicsPaintableD1Ev
CPropPhysicsPaintable::~CPropPhysicsPaintable()
{
	// inlined PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser() at line 13
}

// game/server/portal2/prop_physics_paintable.cpp:13 (declaration)
void CPropPhysicsPaintable();

// game/server/portal2/prop_physics_paintable.cpp:18
static CEntityFactory<CPropPhysicsPaintable> prop_physics_paintable;

// game/server/util.h:99 @0x6f3710 _ZN14CEntityFactoryI21CPropPhysicsPaintableE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropPhysicsPaintable>::Create( const char *pClassName )
{
	CPropPhysicsPaintable *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CPropPhysicsPaintable>() at line 101
}

// game/server/util.h:105 @0x6f0220 _ZN14CEntityFactoryI21CPropPhysicsPaintableE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropPhysicsPaintable>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6f0250 _ZN14CEntityFactoryI21CPropPhysicsPaintableE13GetEntitySizeEv
size_t CEntityFactory<CPropPhysicsPaintable>::GetEntitySize()
{
}

// public/tier1/utlvector.h:655 @0x6f0c90 _ZN10CUtlVectorIN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE30ContextSurfacePaintPowerInfo_tE10CUtlMemoryIS5_iEE10GrowVectorEi
void CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,int>::NumAllocated() at line 657
	// inlined CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,int>::Grow() at line 660
	// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::ResetDbgInfo() at line 664
}
