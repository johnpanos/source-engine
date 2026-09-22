#include "vphysics_interface.h"
#include "tier0/dbg.h"
#include "utlmap.h"

#include "box3d/base.h"
#include "box3d/constants.h"
#include "box3d_convert.h"
#include "physics_environment.h"
#include "physics_object_hash.h"

// Box3D runs in Source units: its tolerances scale with this length unit.
// It is process-wide and must be set before any hull or world exists, so it
// is set once when the module loads.
//
// Box3D's log reports expected conditions (a convex over the per-hull limits,
// which the collision decoder then splits), so it goes to the developer
// channel rather than stdout.
static void Box3DLog( const char *pMessage )
{
	DevMsg( 2, "Box3D: %s\n", pMessage );
}

static struct CBox3DLengthUnits
{
	CBox3DLengthUnits()
	{
		b3SetLengthUnitsPerMeter( kBox3DInchesPerMeter );
		b3SetLogFcn( Box3DLog );
	}
} s_box3dLengthUnits;

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
	virtual IPhysicsEnvironment *CreateEnvironment()
	{
		IPhysicsEnvironment *pEnvironment = new CPhysicsEnvironmentBox3D();
		m_environments.AddToTail( pEnvironment );
		return pEnvironment;
	}
	virtual void DestroyEnvironment( IPhysicsEnvironment *pEnvironment )
	{
		m_environments.FindAndRemove( pEnvironment );
		delete pEnvironment;
	}
	virtual IPhysicsEnvironment *GetActiveEnvironmentByIndex( int index )
	{
		return m_environments.IsValidIndex( index ) ? m_environments[index] : NULL;
	}
	virtual int GetActiveEnvironmentCount() { return m_environments.Count(); }
	virtual IPhysicsObjectPairHash *CreateObjectPairHash() { return new CObjectPairHashBox3D(); }
	virtual void DestroyObjectPairHash( IPhysicsObjectPairHash *pHash ) { delete pHash; }
	virtual IPhysicsCollisionSet *FindOrCreateCollisionSet( unsigned int id, int maxElementCount )
	{
		int index = m_collisionSets.Find( id );
		if ( index != m_collisionSets.InvalidIndex() )
			return m_collisionSets[index];
		CCollisionSetBox3D *pSet = new CCollisionSetBox3D();
		m_collisionSets.Insert( id, pSet );
		return pSet;
	}
	virtual IPhysicsCollisionSet *FindCollisionSet( unsigned int id )
	{
		int index = m_collisionSets.Find( id );
		return index != m_collisionSets.InvalidIndex() ? m_collisionSets[index] : NULL;
	}
	virtual void DestroyAllCollisionSets()
	{
		for ( int i = m_collisionSets.FirstInorder(); i != m_collisionSets.InvalidIndex(); i = m_collisionSets.NextInorder( i ) )
			delete m_collisionSets[i];
		m_collisionSets.RemoveAll();
	}

private:
	CUtlMap<unsigned int, CCollisionSetBox3D *> m_collisionSets{ DefLessFunc( unsigned int ) };
	CUtlVector<IPhysicsEnvironment *> m_environments;
};

static CPhysicsInterfaceBox3D g_MainDLLInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsInterfaceBox3D, IPhysics, VPHYSICS_INTERFACE_VERSION, g_MainDLLInterface );

#include "physics_collision.h"
static CPhysicsCollisionBox3D g_PhysicsCollision;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsCollisionBox3D, IPhysicsCollision, VPHYSICS_COLLISION_INTERFACE_VERSION, g_PhysicsCollision );

#include "physics_material.h"
CPhysicsSurfacePropsBox3D g_SurfaceDatabase;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsSurfacePropsBox3D, IPhysicsSurfaceProps, VPHYSICS_SURFACEPROPS_INTERFACE_VERSION, g_SurfaceDatabase );

DLL_EXPORT IPhysics *Physics_Create() {
	return &g_MainDLLInterface;
}
