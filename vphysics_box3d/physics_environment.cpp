#include "physics_environment.h"
#include "tier0/dbg.h"
#include "jobsystem/worker_backend.h"

// Extern from tier0 or jobsystem
namespace jobsystem {
	extern IWorkerBackend *g_pWorkerBackend;
}

CPhysicsEnvironmentBox3D::CPhysicsEnvironmentBox3D()
{
	m_gravity.Init();
	m_airDensity = 2.0f;
	m_inSimulation = false;
	m_simulationTime = 0.0f;
	m_pScheduler = new CJobSystemBox3DTaskScheduler( nullptr ); // We would pass g_pWorkerBackend when available
}

CPhysicsEnvironmentBox3D::~CPhysicsEnvironmentBox3D()
{
	delete m_pScheduler;
}

void CPhysicsEnvironmentBox3D::SetDebugOverlay( CreateInterfaceFn debugOverlayFactory ) {}
IVPhysicsDebugOverlay *CPhysicsEnvironmentBox3D::GetDebugOverlay( void ) { return nullptr; }

void CPhysicsEnvironmentBox3D::SetGravity( const Vector &gravityVector ) { m_gravity = gravityVector; }
void CPhysicsEnvironmentBox3D::GetGravity( Vector *pGravityVector ) const { *pGravityVector = m_gravity; }

void CPhysicsEnvironmentBox3D::SetAirDensity( float density ) { m_airDensity = density; }
float CPhysicsEnvironmentBox3D::GetAirDensity( void ) const { return m_airDensity; }

#include "physics_object.h"

IPhysicsObject *CPhysicsEnvironmentBox3D::CreatePolyObject( const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, struct objectparams_t *pParams ) { return new CPhysicsObjectBox3D(); }
IPhysicsObject *CPhysicsEnvironmentBox3D::CreatePolyObjectStatic( const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, struct objectparams_t *pParams ) { return new CPhysicsObjectBox3D(); }
IPhysicsObject *CPhysicsEnvironmentBox3D::CreateSphereObject( float radius, int materialIndex, const Vector &position, const QAngle &angles, struct objectparams_t *pParams, bool isStatic ) { return new CPhysicsObjectBox3D(); }
void CPhysicsEnvironmentBox3D::DestroyObject( IPhysicsObject *pObject ) { delete pObject; }

IPhysicsFluidController *CPhysicsEnvironmentBox3D::CreateFluidController( IPhysicsObject *pFluidObject, fluidparams_t *pParams ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroyFluidController( IPhysicsFluidController * ) {}

IPhysicsSpring *CPhysicsEnvironmentBox3D::CreateSpring( IPhysicsObject *pObjectStart, IPhysicsObject *pObjectEnd, springparams_t *pParams ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroySpring( IPhysicsSpring * ) {}

void CPhysicsEnvironmentBox3D::DestroyConstraint( IPhysicsConstraint * ) {}

IPhysicsConstraintGroup *CPhysicsEnvironmentBox3D::CreateConstraintGroup( const constraint_groupparams_t &pParams ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroyConstraintGroup( IPhysicsConstraintGroup *pGroup ) {}

IPhysicsShadowController *CPhysicsEnvironmentBox3D::CreateShadowController( IPhysicsObject *pObject, bool allowTranslation, bool allowRotation ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroyShadowController( IPhysicsShadowController * ) {}

IPhysicsPlayerController *CPhysicsEnvironmentBox3D::CreatePlayerController( IPhysicsObject *pObject ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroyPlayerController( IPhysicsPlayerController * ) {}

IPhysicsMotionController *CPhysicsEnvironmentBox3D::CreateMotionController( IMotionEvent *pHandler ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroyMotionController( IPhysicsMotionController *pController ) {}

IPhysicsVehicleController *CPhysicsEnvironmentBox3D::CreateVehicleController( IPhysicsObject *pVehicleBodyObject, const vehicleparams_t &params, unsigned int nVehicleType, IPhysicsGameTrace *pGameTrace ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroyVehicleController( IPhysicsVehicleController * ) {}

void CPhysicsEnvironmentBox3D::SetCollisionSolver( IPhysicsCollisionSolver *pSolver ) {}
void CPhysicsEnvironmentBox3D::Simulate( float deltaTime ) 
{
	m_inSimulation = true;
	
	// TODO: Call Box3D world step, utilizing m_pScheduler
	
	m_simulationTime += deltaTime;
	m_inSimulation = false;
}
bool CPhysicsEnvironmentBox3D::IsInSimulation() const { return m_inSimulation; }

float CPhysicsEnvironmentBox3D::GetSimulationTime() const { return m_simulationTime; }
float CPhysicsEnvironmentBox3D::GetNextFrameTime() const { return 0.0f; }

void CPhysicsEnvironmentBox3D::SetCollisionEventHandler( IPhysicsCollisionEvent *pCollisionEvents ) {}
void CPhysicsEnvironmentBox3D::SetObjectEventHandler( IPhysicsObjectEvent *pObjectEvents ) {}
void CPhysicsEnvironmentBox3D::SetConstraintEventHandler( IPhysicsConstraintEvent *pConstraintEvents ) {}

void CPhysicsEnvironmentBox3D::SetQuickDelete( bool bQuick ) {}

int CPhysicsEnvironmentBox3D::GetActiveObjectCount() const { return 0; }
void CPhysicsEnvironmentBox3D::GetActiveObjects( IPhysicsObject **pOutputObjectList ) const {}
const IPhysicsObject **CPhysicsEnvironmentBox3D::GetObjectList( int *pOutputObjectCount ) const { *pOutputObjectCount = 0; return nullptr; }

bool CPhysicsEnvironmentBox3D::TransferObject( IPhysicsObject *pObject, IPhysicsEnvironment *pDestinationEnvironment ) { return false; }

void CPhysicsEnvironmentBox3D::CleanupDeleteList( void ) {}
void CPhysicsEnvironmentBox3D::EnableDeleteQueue( bool enable ) {}

bool CPhysicsEnvironmentBox3D::Save( const physsaveparams_t &params ) { return false; }
void CPhysicsEnvironmentBox3D::PreRestore( const physprerestoreparams_t &params ) {}
bool CPhysicsEnvironmentBox3D::Restore( const physrestoreparams_t &params ) { return false; }
void CPhysicsEnvironmentBox3D::PostRestore() {}

bool CPhysicsEnvironmentBox3D::IsCollisionModelUsed( CPhysCollide *pCollide ) const { return false; }
void CPhysicsEnvironmentBox3D::TraceRay( const Ray_t &ray, unsigned int fMask, IPhysicsTraceFilter *pTraceFilter, trace_t *pTrace ) {}
void CPhysicsEnvironmentBox3D::SweepCollideable( const CPhysCollide *pCollide, const Vector &vecAbsStart, const Vector &vecAbsEnd, const QAngle &vecAngles, unsigned int fMask, IPhysicsTraceFilter *pTraceFilter, trace_t *pTrace ) {}

void CPhysicsEnvironmentBox3D::GetPerformanceSettings( physics_performanceparams_t *pOutput ) const {}
void CPhysicsEnvironmentBox3D::SetPerformanceSettings( const physics_performanceparams_t *pSettings ) {}

void CPhysicsEnvironmentBox3D::ReadStats( physics_stats_t *pOutput ) {}
void CPhysicsEnvironmentBox3D::ClearStats() {}

unsigned int CPhysicsEnvironmentBox3D::GetObjectSerializeSize( IPhysicsObject *pObject ) const { return 0; }
void CPhysicsEnvironmentBox3D::SerializeObjectToBuffer( IPhysicsObject *pObject, unsigned char *pBuffer, unsigned int bufferSize ) {}
IPhysicsObject *CPhysicsEnvironmentBox3D::UnserializeObjectFromBuffer( void *pGameData, unsigned char *pBuffer, unsigned int bufferSize, bool enableCollisions ) { return nullptr; }

void CPhysicsEnvironmentBox3D::EnableConstraintNotify( bool bEnable ) {}
void CPhysicsEnvironmentBox3D::DebugCheckContacts( void ) {}
