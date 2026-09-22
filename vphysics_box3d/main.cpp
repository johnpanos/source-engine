#include "vphysics_interface.h"
#include "tier0/dbg.h"

#include "physics_environment.h"

class CPhysicsInterfaceBox3D : public IPhysics {
public:
	virtual ~CPhysicsInterfaceBox3D() {}
	virtual bool Connect( CreateInterfaceFn factory ) { return true; }
	virtual void Disconnect() {}
	virtual InitReturnVal_t Init() { return INIT_OK; }
	virtual void Shutdown() {}
	// The app framework resolves sibling interfaces (collision, surfaceprops)
	// through the primary IPhysics AppSystem's QueryInterface, not the module's
	// exported CreateInterface. Delegate to this module's own factory so those
	// interfaces are reachable, matching stock vphysics (vphysics/main.cpp).
	virtual void *QueryInterface( const char *pInterfaceName )
	{
		CreateInterfaceFn factory = Sys_GetFactoryThis();
		return factory ? factory( pInterfaceName, NULL ) : NULL;
	}
	virtual IPhysicsEnvironment *CreateEnvironment() { return new CPhysicsEnvironmentBox3D(); }
	virtual void DestroyEnvironment( IPhysicsEnvironment *pEnvironment ) { delete pEnvironment; }
	virtual IPhysicsEnvironment *GetActiveEnvironmentByIndex( int index ) { return NULL; }
	virtual int GetActiveEnvironmentCount() { return 0; }
	virtual IPhysicsObjectPairHash *CreateObjectPairHash() { return NULL; }
	virtual void DestroyObjectPairHash( IPhysicsObjectPairHash *pHash ) {}
	virtual IPhysicsCollisionSet *FindOrCreateCollisionSet( unsigned int id, int maxElementCount ) { return NULL; }
	virtual IPhysicsCollisionSet *FindCollisionSet( unsigned int id ) { return NULL; }
	virtual void DestroyAllCollisionSets() {}
};

static CPhysicsInterfaceBox3D g_MainDLLInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsInterfaceBox3D, IPhysics, VPHYSICS_INTERFACE_VERSION, g_MainDLLInterface );

#include "physics_collision.h"
static CPhysicsCollisionBox3D g_PhysicsCollision;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsCollisionBox3D, IPhysicsCollision, VPHYSICS_COLLISION_INTERFACE_VERSION, g_PhysicsCollision );

#include "physics_material.h"
static CPhysicsSurfacePropsBox3D g_SurfaceDatabase;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsSurfacePropsBox3D, IPhysicsSurfaceProps, VPHYSICS_SURFACEPROPS_INTERFACE_VERSION, g_SurfaceDatabase );

DLL_EXPORT IPhysics *Physics_Create() {
	return &g_MainDLLInterface;
}
