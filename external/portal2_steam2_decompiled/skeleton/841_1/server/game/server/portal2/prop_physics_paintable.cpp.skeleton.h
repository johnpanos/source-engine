// DWARF declaration skeleton for game/server/portal2/prop_physics_paintable.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xb8490 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CEntityFactory<CPropPhysicsPaintable>::CEntityFactory() at line 18
}

// None:0 @0xb8570 _Z41__static_initialization_and_destruction_1ii
__static_initialization_and_destruction_1( int __initialize_p, int __priority )
{
}

// game/server/portal2/prop_physics_paintable.cpp:13 sizeof=0x884 (i386)
struct CPropPhysicsPaintable : public PlayerPickupPaintPowerUser<CPhysicsProp>
{
};

// game/server/portal2/prop_physics_paintable.cpp:13 (declaration)
~CPropPhysicsPaintable();

// game/server/portal2/prop_physics_paintable.cpp:13 @0x76a730 _ZN21CPropPhysicsPaintableD0Ev
CPropPhysicsPaintable::~CPropPhysicsPaintable()
{
	// inlined PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser() at line 13
}

// game/server/portal2/prop_physics_paintable.cpp:13 @0x76a880 _ZN21CPropPhysicsPaintableD1Ev
CPropPhysicsPaintable::~CPropPhysicsPaintable()
{
	// inlined PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser() at line 13
}

// game/server/portal2/prop_physics_paintable.cpp:13 (declaration)
void CPropPhysicsPaintable();

// game/server/portal2/prop_physics_paintable.cpp:18
static CEntityFactory<CPropPhysicsPaintable> prop_physics_paintable;

// game/server/util.h:100 @0x76b020 _ZN14CEntityFactoryI21CPropPhysicsPaintableE6CreateEPKc
IServerNetworkable *CEntityFactory<CPropPhysicsPaintable>::Create( const char *pClassName )
{
	CPropPhysicsPaintable *pEnt;  // line 102
	// inlined _CreateEntityTemplate<CPropPhysicsPaintable>() at line 102
}

// game/server/util.h:106 @0x767320 _ZN14CEntityFactoryI21CPropPhysicsPaintableE7DestroyEP18IServerNetworkable
void CEntityFactory<CPropPhysicsPaintable>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:114 @0x767350 _ZN14CEntityFactoryI21CPropPhysicsPaintableE13GetEntitySizeEv
size_t CEntityFactory<CPropPhysicsPaintable>::GetEntitySize()
{
}
