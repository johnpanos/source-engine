//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Physics benchmark scenes and the parallel-step capability contract
//          (RFC 0013; RFC 0004 performance and determinism). One scene runs
//          per process, against the loaded provider, through public VPhysics
//          interfaces only, so IVP and Box3D receive identical inputs.
//          tools/quality/physics_bench.py runs the scenes per provider and
//          worker count, interleaves repeats, and applies
//          quality/budgets/physics-v1.json.
//
// Output protocol (in addition to the host's checks-v1 records):
//   BENCH <scene> <workers> <bodies> <constraints>
//   BUILD_US <microseconds to build the scene>
//   SAMPLES <microseconds per Simulate(tick)>...
//   METRIC <name> <value>
//   DIGEST <16 hex digits>       bitwise object state after the run
//   MEMORY <rss-start-kb> <rss-built-kb> <rss-end-kb> <peak-kb>
//   UNSUPPORTED <capability>     the provider lacks a requested capability
//
// Workers: 0 creates the environment with IPhysics::CreateEnvironment (what
// the game does today); 1 or more goes through the opt-in parallel-step
// capability.
//
//=============================================================================//

#include "vphysics_conformance.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "tier1/strtools.h"
#include "tier1/utlvector.h"
#include "vcollide.h"
#include "phyfile.h"
#include "vphysics/constraints.h"
#include "vphysics/parallel_step.h"
#include "vphysics/performance.h"
#include "vphysics/step_profile.h"
#include "vstdlib/jobthread.h"

#include <dirent.h>

namespace
{
//-----------------------------------------------------------------------------
// Scene state
//-----------------------------------------------------------------------------
struct Joint_t
{
	IPhysicsObject *pReference;
	IPhysicsObject *pAttached;
	Vector referenceLocal; // the pivot in each body's space at creation
	Vector attachedLocal;
	IPhysicsConstraint *pConstraint;
};

struct Scene_t
{
	Scene_t() : pEnv( NULL ), material( 0 ) {}
	IPhysicsEnvironment *pEnv;
	int material;
	CUtlVector<CPhysCollide *> collides; // owned
	CUtlVector<IPhysicsObject *> statics;
	CUtlVector<IPhysicsObject *> bodies; // dynamic, in creation order
	CUtlVector<Vector> initial;
	CUtlVector<Joint_t> joints;
	CUtlVector<IPhysicsConstraintGroup *> groups;
};

// Deterministic jitter (no libc rand, whose sequence differs per platform).
struct Random_t
{
	unsigned int state;
	float Uniform( float lo, float hi )
	{
		state = state * 1664525u + 1013904223u;
		return lo + ( hi - lo ) * ( ( state >> 8 ) * ( 1.0f / 16777216.0f ) );
	}
};

int EnvironmentCount()
{
	int count = 0;
	while ( s_pPhysics->GetActiveEnvironmentByIndex( count ) )
		count++;
	return count;
}

// The game always installs a collision solver. As the game's ragdoll rules
// do by default (no "selfcollisions" pairs), parts with the same owner do
// not collide; every other pair does.
class CSceneSolver : public IPhysicsCollisionSolver
{
public:
	virtual int ShouldCollide(
	    IPhysicsObject *, IPhysicsObject *, void *pGameData0, void *pGameData1 )
	{
		return !( pGameData0 && pGameData0 == pGameData1 );
	}
	virtual int ShouldSolvePenetration( IPhysicsObject *, IPhysicsObject *, void *, void *, float )
	{
		return 1;
	}
	virtual bool ShouldFreezeObject( IPhysicsObject * ) { return false; }
	virtual int AdditionalCollisionChecksThisTick( int ) { return 0; }
	virtual bool ShouldFreezeContacts( IPhysicsObject **, int ) { return false; }
};
CSceneSolver s_sceneSolver;

// Distinct non-null owner tokens for the solver (never dereferenced).
void *Owner( int index )
{
	return (void *)(intptr_t)( index + 1 );
}

IPhysicsParallelStep *ParallelStep()
{
	return (IPhysicsParallelStep *)s_pPhysics->QueryInterface(
	    VPHYSICS_PARALLEL_STEP_INTERFACE_VERSION );
}

// The host's compute pool, owned here as the engine owns its CmpJob pool:
// the parallel-step capability runs worker tasks on the pool it is given.
IThreadPool *s_pPool = NULL;

IThreadPool *StartPool( int threads, const char *pName )
{
	IThreadPool *pPool = CreateThreadPool();
	ThreadPoolStartParams_t params;
	params.nThreads = threads;
	pPool->Start( params, pName );
	return pPool;
}

void DestroyPool( IThreadPool *pPool )
{
	if ( !pPool )
		return;
	pPool->Stop();
	DestroyThreadPool( pPool );
}

// Sensitivity faults model a provider that breaks the capability contract.
IPhysicsEnvironment *CreateParallel(
    IPhysicsParallelStep *pParallel, int workers, IThreadPool *pPool )
{
	physics_parallelparams_t params;
	params.Defaults();
	params.workerCount = workers;
	params.pThreadPool = pPool;
	int poolLimit = pPool ? pPool->NumThreads() + 1 : 1;
	bool invalid = workers < 1 || workers > pParallel->GetMaxWorkerCount() || workers > poolLimit;
	if ( invalid && FaultIs( "parallel-accepts-invalid" ) )
		params.workerCount = 1;
	return pParallel->CreateParallelEnvironment( params );
}

// Threads in this process (Linux /proc census).
int ThreadCount()
{
	DIR *pDir = opendir( "/proc/self/task" );
	if ( !pDir )
		return -1;
	int count = 0;
	while ( struct dirent *pEntry = readdir( pDir ) )
	{
		if ( pEntry->d_name[0] != '.' )
			count++;
	}
	closedir( pDir );
	return count;
}

// A census that has settled: a joined thread's /proc entry can outlive
// join() briefly, so sample until two readings 5 ms apart agree.
int StableThreadCount()
{
	int last = ThreadCount();
	for ( int i = 0; i < 100; i++ )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		int now = ThreadCount();
		if ( now == last )
			return now;
		last = now;
	}
	return last;
}

IPhysicsStepProfile *StepProfile()
{
	return (IPhysicsStepProfile *)s_pPhysics->QueryInterface(
	    VPHYSICS_STEP_PROFILE_INTERFACE_VERSION );
}

// A profile read, with the "profile-lies" fault modelling a provider whose
// solver time exceeds the whole call.
bool ReadProfile(
    IPhysicsStepProfile *pProfile, const IPhysicsEnvironment *pEnv, physics_stepprofile_t *pOut )
{
	bool ok = pProfile->GetLastSimulate( pEnv, pOut );
	if ( ok && FaultIs( "profile-lies" ) )
		pOut->stepMs = pOut->simulateMs + 1.0f;
	return ok;
}

int WorkerCountOf( IPhysicsParallelStep *pParallel, const IPhysicsEnvironment *pEnv )
{
	if ( FaultIs( "parallel-count-lies" ) )
		return 1;
	return pParallel->GetWorkerCount( pEnv );
}

// NULL with an UNSUPPORTED record when the capability is missing.
IPhysicsEnvironment *CreateEnvironment( int workers, IThreadPool *pPool )
{
	if ( workers <= 0 )
		return s_pPhysics->CreateEnvironment();
	IPhysicsParallelStep *pParallel = ParallelStep();
	if ( !pParallel )
	{
		printf( "UNSUPPORTED %s\n", VPHYSICS_PARALLEL_STEP_INTERFACE_VERSION );
		return NULL;
	}
	if ( workers > pParallel->GetMaxWorkerCount() )
	{
		printf( "UNSUPPORTED workers-%d (max %d)\n", workers, pParallel->GetMaxWorkerCount() );
		return NULL;
	}
	return CreateParallel( pParallel, workers, pPool );
}

bool BeginScene( Scene_t &scene, int workers, IThreadPool *pPool = NULL )
{
	scene.pEnv = CreateEnvironment( workers, pPool ? pPool : s_pPool );
	if ( !scene.pEnv )
		return false;
	physics_performanceparams_t perf;
	perf.Defaults();
	scene.pEnv->SetPerformanceSettings( &perf );
	scene.pEnv->SetSimulationTimestep( kTick );
	scene.pEnv->SetGravity( Vector( 0, 0, -600 ) );
	scene.pEnv->SetCollisionSolver( &s_sceneSolver );
	scene.material = s_pProps->GetSurfaceIndex( "metal" );
	if ( scene.material < 0 )
		scene.material = 0;
	return true;
}

void EndScene( Scene_t &scene )
{
	if ( scene.pEnv )
		s_pPhysics->DestroyEnvironment( scene.pEnv );
	for ( int i = 0; i < scene.collides.Count(); i++ )
		s_pCollision->DestroyCollide( scene.collides[i] );
	scene.pEnv = NULL;
	scene.collides.RemoveAll();
	scene.statics.RemoveAll();
	scene.bodies.RemoveAll();
	scene.initial.RemoveAll();
	scene.joints.RemoveAll();
	scene.groups.RemoveAll();
}

CPhysCollide *BoxCollide( Scene_t &scene, const Vector &mins, const Vector &maxs )
{
	CPhysCollide *pCollide = s_pCollision->BBoxToCollide( mins, maxs );
	scene.collides.AddToTail( pCollide );
	return pCollide;
}

IPhysicsObject *StaticBox( Scene_t &scene, const Vector &mins, const Vector &maxs )
{
	objectparams_t params = DefaultParams( 1.0f, NULL );
	IPhysicsObject *pObject = scene.pEnv->CreatePolyObjectStatic(
	    BoxCollide( scene, mins, maxs ), scene.material, vec3_origin, vec3_angle, &params );
	scene.statics.AddToTail( pObject );
	return pObject;
}

IPhysicsObject *Dynamic( Scene_t &scene, CPhysCollide *pCollide, const Vector &position,
    const QAngle &angles, float mass, void *pOwner )
{
	objectparams_t params = DefaultParams( mass, pOwner );
	IPhysicsObject *pObject =
	    scene.pEnv->CreatePolyObject( pCollide, scene.material, position, angles, &params );
	if ( !pObject )
		return NULL;
	pObject->EnableMotion( true );
	pObject->EnableGravity( true );
	pObject->Wake();
	scene.bodies.AddToTail( pObject );
	scene.initial.AddToTail( position );
	return pObject;
}

// A floor whose top is z=0 and four walls around a square of half-size h.
void Container( Scene_t &scene, float h, float height )
{
	const float t = 32.0f;
	StaticBox( scene, Vector( -h - t, -h - t, -t ), Vector( h + t, h + t, 0 ) );
	StaticBox( scene, Vector( -h - t, -h - t, 0 ), Vector( -h, h + t, height ) );
	StaticBox( scene, Vector( h, -h - t, 0 ), Vector( h + t, h + t, height ) );
	StaticBox( scene, Vector( -h, -h - t, 0 ), Vector( h, -h, height ) );
	StaticBox( scene, Vector( -h, h, 0 ), Vector( h, h + t, height ) );
}

//-----------------------------------------------------------------------------
// Scenes
//-----------------------------------------------------------------------------

// pile: count cubes (the authored Portal cube when given) dropped in about
// four jittered layers into a walled pit. Measures broadphase and contact
// solving at scale, and whether anything escapes.
bool BuildPile( Scene_t &scene, int count, Random_t &random, CPhysCollide *pAuthoredCube )
{
	CPhysCollide *pCube = pAuthoredCube
	                          ? pAuthoredCube
	                          : BoxCollide( scene, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ) );
	Vector mins, maxs;
	s_pCollision->CollideGetAABB( &mins, &maxs, pCube, vec3_origin, vec3_angle );
	Vector size = maxs - mins;
	float extent = MAX( size.x, MAX( size.y, size.z ) );
	float spacing = extent * 1.25f + 4.0f;
	int perRow = MAX( 2, (int)ceilf( sqrtf( count / 4.0f ) ) );
	float half = perRow * spacing * 0.5f;
	int layers = ( count + perRow * perRow - 1 ) / ( perRow * perRow );
	Container( scene, half + 8.0f, ( layers + 2 ) * spacing );
	for ( int i = 0; i < count; i++ )
	{
		int layer = i / ( perRow * perRow );
		int cell = i % ( perRow * perRow );
		Vector position( -half + ( cell % perRow + 0.5f ) * spacing + random.Uniform( -2, 2 ),
		    -half + ( cell / perRow + 0.5f ) * spacing + random.Uniform( -2, 2 ),
		    extent + layer * spacing );
		QAngle angles(
		    random.Uniform( -20, 20 ), random.Uniform( 0, 360 ), random.Uniform( -20, 20 ) );
		if ( !Dynamic( scene, pCube, position, angles, 50.0f, Owner( i ) ) )
			return false;
	}
	return true;
}

// stack: a 2D pyramid with count cubes on its base row, each row resting on
// the one below. A stable solver keeps it standing and asleep.
bool BuildStack( Scene_t &scene, int base )
{
	StaticBox( scene, Vector( -2048, -512, -32 ), Vector( 2048, 512, 0 ) );
	CPhysCollide *pCube = BoxCollide( scene, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ) );
	const float gap = 0.05f;
	for ( int row = 0; row < base; row++ )
	{
		int columns = base - row;
		for ( int c = 0; c < columns; c++ )
		{
			Vector position(
			    ( c - ( columns - 1 ) * 0.5f ) * 32.0f, 0, 16.0f + gap + row * ( 32.0f + gap ) );
			if ( !Dynamic(
			         scene, pCube, position, vec3_angle, 50.0f, Owner( scene.bodies.Count() ) ) )
				return false;
		}
	}
	return true;
}

// A world-aligned constraint space at the pivot (as the suite's ragdoll
// clauses build it).
constraint_ragdollparams_t RagdollJoint(
    IPhysicsObject *pReference, IPhysicsObject *pAttached, const Vector &pivot )
{
	constraint_ragdollparams_t ragdoll;
	ragdoll.Defaults();
	matrix3x4_t constraintToWorld, referenceToWorld, attachedToWorld, worldToReference,
	    worldToAttached;
	AngleMatrix( vec3_angle, pivot, constraintToWorld );
	pReference->GetPositionMatrix( &referenceToWorld );
	pAttached->GetPositionMatrix( &attachedToWorld );
	MatrixInvert( referenceToWorld, worldToReference );
	MatrixInvert( attachedToWorld, worldToAttached );
	ConcatTransforms( worldToReference, constraintToWorld, ragdoll.constraintToReference );
	ConcatTransforms( worldToAttached, constraintToWorld, ragdoll.constraintToAttached );
	return ragdoll;
}

// A humanoid of 11 boxes: pelvis, torso, head, arms, forearms, thighs,
// calves. Local space stands the pelvis at the origin.
struct Part_t
{
	int parent;
	Vector center;
	Vector half;
	Vector pivot; // joint to the parent (unused for the root)
	float mass;
	float limit[3][2]; // min/max degrees about x, y, z
};

const Part_t kHumanoid[] = {
    { -1, Vector( 0, 0, 0 ), Vector( 6, 8, 4 ), Vector( 0, 0, 0 ), 12,
        { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0, Vector( 0, 0, 14.5f ), Vector( 6, 9, 10 ), Vector( 0, 0, 4.25f ), 20,
        { { -20, 20 }, { -30, 30 }, { -20, 20 } } },
    { 1, Vector( 0, 0, 30.5f ), Vector( 4, 4, 5 ), Vector( 0, 0, 25 ), 5,
        { { -30, 30 }, { -40, 40 }, { -45, 45 } } },
    { 1, Vector( 0, 12.5f, 16 ), Vector( 2.5f, 2.5f, 7 ), Vector( 0, 12.5f, 22.5f ), 3,
        { { -60, 60 }, { -80, 80 }, { -30, 30 } } },
    { 3, Vector( 0, 12.5f, 1.5f ), Vector( 2, 2, 6.5f ), Vector( 0, 12.5f, 8.5f ), 2,
        { { 0, 0 }, { 0, 120 }, { 0, 0 } } },
    { 1, Vector( 0, -12.5f, 16 ), Vector( 2.5f, 2.5f, 7 ), Vector( 0, -12.5f, 22.5f ), 3,
        { { -60, 60 }, { -80, 80 }, { -30, 30 } } },
    { 5, Vector( 0, -12.5f, 1.5f ), Vector( 2, 2, 6.5f ), Vector( 0, -12.5f, 8.5f ), 2,
        { { 0, 0 }, { 0, 120 }, { 0, 0 } } },
    { 0, Vector( 0, 4, -12.5f ), Vector( 3, 3, 8 ), Vector( 0, 4, -4.25f ), 8,
        { { -20, 45 }, { -60, 30 }, { -20, 20 } } },
    { 7, Vector( 0, 4, -30 ), Vector( 2.5f, 2.5f, 8.5f ), Vector( 0, 4, -21 ), 5,
        { { 0, 0 }, { -120, 0 }, { 0, 0 } } },
    { 0, Vector( 0, -4, -12.5f ), Vector( 3, 3, 8 ), Vector( 0, -4, -4.25f ), 8,
        { { -45, 20 }, { -60, 30 }, { -20, 20 } } },
    { 9, Vector( 0, -4, -30 ), Vector( 2.5f, 2.5f, 8.5f ), Vector( 0, -4, -21 ), 5,
        { { 0, 0 }, { -120, 0 }, { 0, 0 } } },
};

// ragdolls: count humanoids (constraint groups, as the game builds ragdolls)
// dropped lying down at random yaws into a pit, about two layers deep.
bool BuildRagdolls( Scene_t &scene, int count, Random_t &random )
{
	const int partCount = ARRAYSIZE( kHumanoid );
	CPhysCollide *pCollides[ARRAYSIZE( kHumanoid )];
	for ( int p = 0; p < partCount; p++ )
		pCollides[p] = BoxCollide( scene, -kHumanoid[p].half, kHumanoid[p].half );
	const float spacing = 88.0f;
	int perRow = MAX( 2, (int)ceilf( sqrtf( count / 2.0f ) ) );
	float half = perRow * spacing * 0.5f;
	int layers = ( count + perRow * perRow - 1 ) / ( perRow * perRow );
	Container( scene, half + 8.0f, layers * 40.0f + 128.0f );
	for ( int r = 0; r < count; r++ )
	{
		int layer = r / ( perRow * perRow );
		int cell = r % ( perRow * perRow );
		Vector origin( -half + ( cell % perRow + 0.5f ) * spacing,
		    -half + ( cell / perRow + 0.5f ) * spacing, 24.0f + layer * 40.0f );
		QAngle angles( 90.0f + random.Uniform( -15, 15 ), random.Uniform( 0, 360 ),
		    random.Uniform( -15, 15 ) );
		matrix3x4_t pose;
		AngleMatrix( angles, origin, pose );
		IPhysicsObject *pParts[ARRAYSIZE( kHumanoid )];
		for ( int p = 0; p < partCount; p++ )
		{
			Vector position;
			VectorTransform( kHumanoid[p].center, pose, position );
			pParts[p] =
			    Dynamic( scene, pCollides[p], position, angles, kHumanoid[p].mass, Owner( r ) );
			if ( !pParts[p] )
				return false;
		}
		constraint_groupparams_t groupParams;
		groupParams.Defaults();
		IPhysicsConstraintGroup *pGroup = scene.pEnv->CreateConstraintGroup( groupParams );
		scene.groups.AddToTail( pGroup );
		for ( int p = 1; p < partCount; p++ )
		{
			const Part_t &part = kHumanoid[p];
			Vector pivot;
			VectorTransform( part.pivot, pose, pivot );
			IPhysicsObject *pReference = pParts[part.parent];
			constraint_ragdollparams_t ragdoll = RagdollJoint( pReference, pParts[p], pivot );
			for ( int axis = 0; axis < 3; axis++ )
				ragdoll.axes[axis].SetAxisFriction( part.limit[axis][0], part.limit[axis][1], 0 );
			Joint_t joint;
			joint.pReference = pReference;
			joint.pAttached = pParts[p];
			pReference->WorldToLocal( &joint.referenceLocal, pivot );
			pParts[p]->WorldToLocal( &joint.attachedLocal, pivot );
			joint.pConstraint =
			    scene.pEnv->CreateRagdollConstraint( pReference, pParts[p], pGroup, ragdoll );
			if ( !joint.pConstraint )
				return false;
			scene.joints.AddToTail( joint );
		}
		if ( pGroup )
			pGroup->Activate();
	}
	return true;
}

// Fast small boxes fired at the object speed limit (k_flMaxVelocity; a tick
// moves them 30 units) at a thin barrier just past x=0. Measures tunneling.
//   projectiles: 6-unit boxes into a 2-unit static wall
//   shards:      2-unit boxes into a 1-unit static wall
//   panes:       6-unit boxes into 1-unit dynamic panes standing on the floor
//                (dynamic-vs-dynamic, which Box3D's continuous collision
//                covers only for bullets)
struct Barrier_t
{
	float projectileHalf;
	float thickness;
	bool dynamicPanes;
};

const Barrier_t kProjectiles = { 3.0f, 2.0f, false };
const Barrier_t kShards = { 1.0f, 1.0f, false };
const Barrier_t kPanes = { 3.0f, 1.0f, true };

const Barrier_t &BarrierOf( const char *pScene )
{
	if ( !V_strcmp( pScene, "shards" ) )
		return kShards;
	if ( !V_strcmp( pScene, "panes" ) )
		return kPanes;
	return kProjectiles;
}

bool IsBarrierScene( const char *pScene )
{
	return !V_strcmp( pScene, "projectiles" ) || !V_strcmp( pScene, "shards" ) ||
	       !V_strcmp( pScene, "panes" );
}

bool BuildBarrier( Scene_t &scene, int count, const Barrier_t &barrier, bool barrierCollides )
{
	int perRow = MAX( 1, (int)ceilf( sqrtf( (float)count ) ) );
	const float pitch = 16.0f;
	float span = perRow * pitch * 0.5f + 32.0f;
	float height = perRow * pitch + 32.0f;
	StaticBox( scene, Vector( -512, -span - 64, -32 ), Vector( 512, span + 64, 0 ) );
	float half = barrier.projectileHalf;
	if ( barrier.dynamicPanes )
	{
		// One pane per column, heavy enough not to be knocked flat at once.
		CPhysCollide *pPane = BoxCollide( scene, Vector( 0, -pitch * 0.5f + 0.5f, 0 ),
		    Vector( barrier.thickness, pitch * 0.5f - 0.5f, height ) );
		for ( int c = 0; c < perRow; c++ )
		{
			Vector position( 0, ( c - ( perRow - 1 ) * 0.5f ) * pitch, 0.05f );
			IPhysicsObject *pObject =
			    Dynamic( scene, pPane, position, vec3_angle, 200.0f, Owner( count + c ) );
			if ( !pObject )
				return false;
			if ( !barrierCollides )
				pObject->EnableCollisions( false );
		}
	}
	else
	{
		IPhysicsObject *pWall =
		    StaticBox( scene, Vector( 0, -span, 0 ), Vector( barrier.thickness, span, height ) );
		if ( !barrierCollides )
			pWall->EnableCollisions( false );
	}
	CPhysCollide *pBox =
	    BoxCollide( scene, Vector( -half, -half, -half ), Vector( half, half, half ) );
	for ( int i = 0; i < count; i++ )
	{
		Vector position( -200.0f, ( i % perRow - ( perRow - 1 ) * 0.5f ) * pitch,
		    16.0f + ( i / perRow ) * pitch );
		IPhysicsObject *pObject = Dynamic( scene, pBox, position, vec3_angle, 2.0f, Owner( i ) );
		if ( !pObject )
			return false;
		Vector velocity( k_flMaxVelocity, 0, 0 );
		pObject->SetVelocity( &velocity, NULL );
	}
	return true;
}

//-----------------------------------------------------------------------------
// Measurement
//-----------------------------------------------------------------------------
void ReadMemory( int *pRssKb, int *pPeakKb )
{
	*pRssKb = *pPeakKb = -1;
	FILE *fp = fopen( "/proc/self/status", "r" );
	if ( !fp )
		return;
	char line[256];
	while ( fgets( line, sizeof( line ), fp ) )
	{
		if ( !strncmp( line, "VmRSS:", 6 ) )
			*pRssKb = atoi( line + 6 );
		else if ( !strncmp( line, "VmHWM:", 6 ) )
			*pPeakKb = atoi( line + 6 );
	}
	fclose( fp );
}

double NowUs()
{
	return std::chrono::duration<double, std::micro>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	    .count();
}

struct State_t
{
	Vector position;
	QAngle angles;
	Vector velocity;
	AngularImpulse angular;
};

State_t StateOf( IPhysicsObject *pObject )
{
	State_t state;
	pObject->GetPosition( &state.position, &state.angles );
	pObject->GetVelocity( &state.velocity, &state.angular );
	return state;
}

// FNV-1a over the bits of every dynamic body's state, in creation order.
unsigned long long Digest( const Scene_t &scene, bool *pFinite )
{
	unsigned long long hash = 1469598103934665603ull;
	*pFinite = true;
	for ( int i = 0; i < scene.bodies.Count(); i++ )
	{
		State_t state = StateOf( scene.bodies[i] );
		const float values[] = { state.position.x, state.position.y, state.position.z,
		    state.angles.x, state.angles.y, state.angles.z, state.velocity.x, state.velocity.y,
		    state.velocity.z, state.angular.x, state.angular.y, state.angular.z };
		for ( int v = 0; v < ARRAYSIZE( values ); v++ )
		{
			if ( !IsFinite( values[v] ) )
				*pFinite = false;
			unsigned int bits;
			memcpy( &bits, &values[v], sizeof( bits ) );
			for ( int b = 0; b < 4; b++ )
			{
				hash ^= ( bits >> ( b * 8 ) ) & 0xff;
				hash *= 1099511628211ull;
			}
		}
	}
	return hash;
}

float JointError( const Joint_t &joint )
{
	Vector fromReference, fromAttached;
	joint.pReference->LocalToWorld( &fromReference, joint.referenceLocal );
	joint.pAttached->LocalToWorld( &fromAttached, joint.attachedLocal );
	return ( fromReference - fromAttached ).Length();
}

void Metric( const char *pName, double value )
{
	printf( "METRIC %s %.6g\n", pName, value );
}

void ReportSceneMetrics( const char *pScene, int count, const Scene_t &scene )
{
	const Barrier_t &barrier = BarrierOf( pScene );
	int projectiles = IsBarrierScene( pScene ) ? count : 0;
	int lost = 0, collapsed = 0, tunneled = 0;
	float maxDrift = 0.0f, maxSpeed = 0.0f;
	for ( int i = 0; i < scene.bodies.Count(); i++ )
	{
		State_t state = StateOf( scene.bodies[i] );
		const Vector &start = scene.initial[i];
		if ( state.position.z < -40.0f )
			lost++;
		float drift = ( state.position - start ).Length2D();
		maxDrift = MAX( maxDrift, drift );
		if ( ( state.position - start ).Length() > 8.0f )
			collapsed++;
		if ( i < projectiles &&
		     state.position.x > barrier.thickness + barrier.projectileHalf + 1.0f )
			tunneled++;
		maxSpeed = MAX( maxSpeed, state.velocity.Length() );
	}
	Metric( "awake_end", scene.pEnv->GetActiveObjectCount() );
	Metric( "max_speed_end", maxSpeed );
	if ( !V_strcmp( pScene, "pile" ) || !V_strcmp( pScene, "ragdolls" ) )
		Metric( "lost", lost );
	if ( !V_strcmp( pScene, "stack" ) )
	{
		Metric( "collapsed", collapsed );
		Metric( "max_drift", maxDrift );
	}
	if ( projectiles )
		Metric( "tunneled", tunneled );
	if ( scene.joints.Count() )
	{
		float maxError = 0.0f, sumError = 0.0f;
		for ( int j = 0; j < scene.joints.Count(); j++ )
		{
			float error = JointError( scene.joints[j] );
			maxError = MAX( maxError, error );
			sumError += error;
		}
		Metric( "joint_error_max", maxError );
		Metric( "joint_error_mean", sumError / scene.joints.Count() );
	}
}

bool BuildScene( Scene_t &scene, const BenchOptions_t &options, CPhysCollide *pAuthoredCube )
{
	Random_t random = { options.seed };
	if ( !V_strcmp( options.pScene, "pile" ) )
		return BuildPile( scene, options.count, random, pAuthoredCube );
	if ( !V_strcmp( options.pScene, "stack" ) )
		return BuildStack( scene, options.count );
	if ( !V_strcmp( options.pScene, "ragdolls" ) )
		return BuildRagdolls( scene, options.count, random );
	if ( IsBarrierScene( options.pScene ) )
		return BuildBarrier(
		    scene, options.count, BarrierOf( options.pScene ), !FaultIs( "bench-tunnel" ) );
	return false;
}

// Host-side faults that a correct provider cannot trigger, used to prove the
// runner's gates detect each failure (tools/quality/physics_bench.py).
void ApplyTickFault( const BenchOptions_t &options, Scene_t &scene, int tick )
{
	if ( tick == options.ticks / 2 && FaultIs( "bench-collapse" ) && scene.bodies.Count() )
	{
		Vector push( 0, 20000, 0 );
		scene.bodies[0]->ApplyForceCenter( push );
	}
	if ( tick == options.ticks / 2 && FaultIs( "bench-joint-break" ) )
	{
		for ( int j = 0; j < scene.joints.Count(); j += 2 )
		{
			if ( scene.joints[j].pConstraint )
				scene.pEnv->DestroyConstraint( scene.joints[j].pConstraint );
			scene.joints[j].pConstraint = NULL;
		}
	}
}

int RunScene( const BenchOptions_t &options, CPhysCollide *pAuthoredCube )
{
	int rssStart, peak;
	ReadMemory( &rssStart, &peak );
	// Threads before the environment exists (the pool, if any, is running).
	int threadsBefore = StableThreadCount();
	Scene_t scene;
	double buildStart = NowUs();
	if ( !BeginScene( scene, options.workers ) )
		return 3;
	bool built = BuildScene( scene, options, pAuthoredCube );
	double buildUs = NowUs() - buildStart;
	char name[128];
	V_snprintf( name, sizeof( name ), "bench.%s.build", options.pScene );
	if ( !Check( TIER_GAMEPLAY, name, built && scene.bodies.Count() > 0, "scene %s count %d",
	         options.pScene, options.count ) )
	{
		EndScene( scene );
		return 0;
	}
	int rssBuilt;
	ReadMemory( &rssBuilt, &peak );
	printf( "BENCH %s %d %d %d\n", options.pScene, options.workers, scene.bodies.Count(),
	    scene.joints.Count() );
	printf( "BUILD_US %.1f\n", buildUs );

	// A worker-count fault: the state diverges only when parallel.
	if ( options.workers > 1 && FaultIs( "parallel-nondeterministic" ) )
	{
		Vector position;
		QAngle angles;
		scene.bodies[0]->GetPosition( &position, &angles );
		position.x += 0.01f;
		scene.bodies[0]->SetPosition( position, angles, true );
	}

	CUtlVector<double> samples;
	samples.EnsureCapacity( options.ticks );
	IPhysicsStepProfile *pProfile = StepProfile();
	double stepMs = 0.0, preMs = 0.0, postMs = 0.0, simulateMs = 0.0;
	for ( int tick = 0; tick < options.ticks; tick++ )
	{
		ApplyTickFault( options, scene, tick );
		double start = NowUs();
		scene.pEnv->Simulate( kTick );
		// A provider whose step got 2 ms slower.
		if ( FaultIs( "bench-slow" ) )
			usleep( 2000 );
		samples.AddToTail( NowUs() - start );
		physics_stepprofile_t profile;
		if ( pProfile && ReadProfile( pProfile, scene.pEnv, &profile ) )
		{
			stepMs += profile.stepMs;
			preMs += profile.preStepMs;
			postMs += profile.postStepMs;
			simulateMs += profile.simulateMs;
		}
	}
	int threadsAfter = StableThreadCount();
	printf( "SAMPLES" );
	for ( int i = 0; i < samples.Count(); i++ )
		printf( " %.1f", samples[i] );
	printf( "\n" );

	bool finite;
	unsigned long long digest = Digest( scene, &finite );
	printf( "DIGEST %016llx\n", digest );
	ReportSceneMetrics( options.pScene, options.count, scene );
	// Where Simulate spent its time (vphysics.step-profile.v1), and whether
	// stepping started threads beyond the host's pool.
	if ( pProfile && simulateMs > 0.0 )
	{
		Metric( "profile_solver_ms", stepMs );
		Metric( "profile_adapter_ms", preMs + postMs );
		Metric( "profile_adapter_fraction", ( preMs + postMs ) / simulateMs );
	}
	Metric( "threads_added", threadsAfter - threadsBefore );
	if ( s_pPool )
	{
		ThreadPoolSchedulingStats_t stats;
		GetThreadPoolSchedulingStats( s_pPool, &stats );
		Metric( "pool_threads", s_pPool->NumThreads() );
		Metric( "pool_forbidden_nested_waits", stats.nForbiddenNestedWaits );
		Metric( "pool_starvation_events", stats.nStarvationEvents );
		Metric( "pool_waited_jobs_run_inline", stats.nWaitedJobsRunInline );
	}
	V_snprintf( name, sizeof( name ), "bench.%s.finite", options.pScene );
	Check( TIER_GAMEPLAY, name, finite, "non-finite body state after %d ticks", options.ticks );
	V_snprintf( name, sizeof( name ), "bench.%s.samples", options.pScene );
	Check( TIER_GAMEPLAY, name, samples.Count() == options.ticks && options.ticks > 0, "%d samples",
	    samples.Count() );

	int rssEnd;
	ReadMemory( &rssEnd, &peak );
	printf( "MEMORY %d %d %d %d\n", rssStart, rssBuilt, rssEnd, peak );
	EndScene( scene );
	return 0;
}

//-----------------------------------------------------------------------------
// vphysics.parallel-step.v1 contract
//-----------------------------------------------------------------------------

// Counts concurrent entries into the game's collision solver.
class CConcurrencyProbe : public IPhysicsCollisionSolver
{
public:
	CConcurrencyProbe()
	    : m_inFlight( 0 ), m_maxInFlight( 0 ), m_calls( 0 ), m_offCaller( 0 ),
	      m_caller( std::this_thread::get_id() )
	{
	}
	virtual int ShouldCollide( IPhysicsObject *, IPhysicsObject *, void *, void * )
	{
		Enter( 0 );
		return 1;
	}
	virtual int ShouldSolvePenetration( IPhysicsObject *, IPhysicsObject *, void *, void *, float )
	{
		return 1;
	}
	virtual bool ShouldFreezeObject( IPhysicsObject * ) { return false; }
	virtual int AdditionalCollisionChecksThisTick( int ) { return 0; }
	virtual bool ShouldFreezeContacts( IPhysicsObject **, int ) { return false; }

	// Holds the entry for holdUs so an overlapping caller is observable.
	void Enter( int holdUs )
	{
		int inFlight = ++m_inFlight;
		int seen = m_maxInFlight.load();
		while ( inFlight > seen && !m_maxInFlight.compare_exchange_weak( seen, inFlight ) )
		{
		}
		m_calls++;
		if ( std::this_thread::get_id() != m_caller )
			m_offCaller++;
		if ( holdUs )
			std::this_thread::sleep_for( std::chrono::microseconds( holdUs ) );
		--m_inFlight;
	}

	// Two threads enter at once: the probe must see the overlap.
	void ForceOverlap()
	{
		std::atomic<int> ready( 0 );
		std::thread other(
		    [&]()
		    {
			    ready++;
			    while ( ready.load() < 2 )
			    {
			    }
			    Enter( 20000 );
		    } );
		ready++;
		while ( ready.load() < 2 )
		{
		}
		Enter( 20000 );
		other.join();
	}

	std::atomic<int> m_inFlight;
	std::atomic<int> m_maxInFlight;
	std::atomic<int> m_calls;
	std::atomic<int> m_offCaller;
	std::thread::id m_caller;
};

// Records whether every collision event arrived on the calling thread.
class CThreadProbeEvents : public IPhysicsCollisionEvent
{
public:
	CThreadProbeEvents() : m_events( 0 ), m_offCaller( 0 ), m_caller( std::this_thread::get_id() )
	{
	}
	void Note()
	{
		m_events++;
		if ( std::this_thread::get_id() != m_caller )
			m_offCaller++;
	}
	virtual void PreCollision( vcollisionevent_t * ) { Note(); }
	virtual void PostCollision( vcollisionevent_t * ) { Note(); }
	virtual void Friction( IPhysicsObject *, float, int, int, IPhysicsCollisionData * ) { Note(); }
	virtual void StartTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * )
	{
		Note();
	}
	virtual void EndTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) { Note(); }
	virtual void FluidStartTouch( IPhysicsObject *, IPhysicsFluidController * ) { Note(); }
	virtual void FluidEndTouch( IPhysicsObject *, IPhysicsFluidController * ) { Note(); }
	virtual void PostSimulationFrame() { Note(); }
	virtual void ObjectEnterTrigger( IPhysicsObject *, IPhysicsObject * ) { Note(); }
	virtual void ObjectLeaveTrigger( IPhysicsObject *, IPhysicsObject * ) { Note(); }
	int m_events;
	int m_offCaller;
	std::thread::id m_caller;
};

// A small pile stepped for 120 ticks; returns the state digest.
struct PileRun_t
{
	int workers;
	IThreadPool *pPool; // NULL: the host pool
	int stopPoolAfter;  // stop pPool after this many ticks (-1: never)
	bool readProfile;   // read the step profile after every tick
};

PileRun_t Pile( int workers )
{
	PileRun_t run = { workers, NULL, -1, false };
	return run;
}

unsigned long long PileDigest( const PileRun_t &run, CPhysCollide *pAuthoredCube, bool *pOk )
{
	BenchOptions_t options;
	memset( &options, 0, sizeof( options ) );
	options.pScene = "pile";
	options.count = 256;
	options.ticks = 120;
	options.workers = run.workers;
	options.seed = 7;
	Scene_t scene;
	*pOk =
	    BeginScene( scene, run.workers, run.pPool ) && BuildScene( scene, options, pAuthoredCube );
	if ( *pOk && run.workers > 1 && FaultIs( "parallel-nondeterministic" ) )
	{
		Vector position;
		QAngle angles;
		scene.bodies[0]->GetPosition( &position, &angles );
		position.x += 0.01f;
		scene.bodies[0]->SetPosition( position, angles, true );
	}
	IPhysicsStepProfile *pProfile = run.readProfile ? StepProfile() : NULL;
	for ( int tick = 0; *pOk && tick < options.ticks; tick++ )
	{
		if ( tick == run.stopPoolAfter && run.pPool )
			run.pPool->Stop();
		scene.pEnv->Simulate( kTick );
		physics_stepprofile_t profile;
		if ( pProfile )
			pProfile->GetLastSimulate( scene.pEnv, &profile );
	}
	bool finite = false;
	unsigned long long digest = *pOk ? Digest( scene, &finite ) : 0;
	*pOk = *pOk && finite;
	EndScene( scene );
	return digest;
}

// Simulate called from inside a pool job (RFC 0013: a wait runs only its own
// tasks, so this cannot deadlock even when every other thread is busy).
struct NestedRun_t
{
	PileRun_t pile;
	CPhysCollide *pCube;
	unsigned long long digest;
	bool ok;
	bool onPoolThread;
	std::thread::id host;
};

void RunNested( NestedRun_t *pRun )
{
	pRun->onPoolThread = std::this_thread::get_id() != pRun->host;
	pRun->digest = PileDigest( pRun->pile, pRun->pCube, &pRun->ok );
}

// vphysics.step-profile.v1 clauses (skipped with a record when absent).
void RunStepProfileContract( CPhysCollide *pAuthoredCube, int workers )
{
	IPhysicsStepProfile *pProfile = StepProfile();
	if ( !pProfile )
	{
		printf( "UNSUPPORTED %s\n", VPHYSICS_STEP_PROFILE_INTERFACE_VERSION );
		return;
	}
	physics_stepprofile_t profile;
	memset( &profile, 0x5a, sizeof( profile ) );
	physics_stepprofile_t sentinel = profile;
	int notAnEnvironment = 0;
	bool rejects =
	    !pProfile->GetLastSimulate( NULL, &profile ) &&
	    !pProfile->GetLastSimulate( (const IPhysicsEnvironment *)&notAnEnvironment, &profile );
	Check( TIER_GAMEPLAY, "profile.rejects-foreign",
	    rejects && !memcmp( &profile, &sentinel, sizeof( profile ) ) );

	Scene_t scene;
	BenchOptions_t options;
	memset( &options, 0, sizeof( options ) );
	options.pScene = "pile";
	options.count = 256;
	options.seed = 3;
	bool ok = BeginScene( scene, workers );
	physics_stepprofile_t zero;
	memset( &zero, 0, sizeof( zero ) );
	ok = ok && ReadProfile( pProfile, scene.pEnv, &profile );
	Check( TIER_GAMEPLAY, "profile.zero-before-first",
	    ok && !memcmp( &profile, &zero, sizeof( zero ) ) );
	ok = ok && BuildScene( scene, options, pAuthoredCube );
	bool consistent = ok;
	int bodies = 0;
	for ( int tick = 0; ok && tick < 60; tick++ )
	{
		scene.pEnv->Simulate( kTick );
		ok = ReadProfile( pProfile, scene.pEnv, &profile );
		consistent =
		    consistent && ok && profile.stepCount == 1 && profile.stepMs >= 0.0f &&
		    profile.solverCallsOffCaller <= profile.solverCalls && profile.preStepMs >= 0.0f &&
		    profile.postStepMs >= 0.0f &&
		    profile.stepMs + profile.preStepMs + profile.postStepMs <= profile.simulateMs + 0.01f &&
		    profile.workerCount == workers;
		bodies = profile.bodyCount;
	}
	Check( TIER_GAMEPLAY, "profile.consistent", consistent && bodies >= 256,
	    "last: steps %d, simulate %.3f ms, solver %.3f, pre %.3f, post %.3f, workers %d, bodies %d",
	    profile.stepCount, profile.simulateMs, profile.stepMs, profile.preStepMs,
	    profile.postStepMs, profile.workerCount, bodies );
	EndScene( scene );

	bool okPlain, okRead;
	PileRun_t read = Pile( 1 );
	read.readProfile = true;
	unsigned long long plain = PileDigest( Pile( 1 ), pAuthoredCube, &okPlain );
	unsigned long long withReads = PileDigest( read, pAuthoredCube, &okRead );
	Check( TIER_GAMEPLAY, "profile.read-harmless", okPlain && okRead && plain == withReads );
}

int RunParallelContract( CPhysCollide *pAuthoredCube )
{
	IPhysicsParallelStep *pParallel = ParallelStep();
	if ( !pParallel )
	{
		printf( "UNSUPPORTED %s\n", VPHYSICS_PARALLEL_STEP_INTERFACE_VERSION );
		return 3;
	}
	int maxWorkers = pParallel->GetMaxWorkerCount();
	Check( TIER_GAMEPLAY, "parallel.max-workers", maxWorkers >= 1, "max %d", maxWorkers );
	int poolThreads = s_pPool ? s_pPool->NumThreads() : 0;
	int parallelWorkers = MIN( 4, MIN( maxWorkers, poolThreads + 1 ) );
	Check( TIER_BOOT, "parallel.host-pool", parallelWorkers > 1, "pool threads %d, max workers %d",
	    poolThreads, maxWorkers );

	int before = EnvironmentCount();
	IPhysicsEnvironment *pZero = CreateParallel( pParallel, 0, s_pPool );
	Check( TIER_GAMEPLAY, "parallel.rejects-zero", !pZero && EnvironmentCount() == before );
	if ( pZero )
		s_pPhysics->DestroyEnvironment( pZero );
	IPhysicsEnvironment *pOver = CreateParallel( pParallel, maxWorkers + 1, s_pPool );
	Check( TIER_GAMEPLAY, "parallel.rejects-over-max", !pOver && EnvironmentCount() == before );
	if ( pOver )
		s_pPhysics->DestroyEnvironment( pOver );

	IPhysicsEnvironment *pSerial = CreateParallel( pParallel, 1, s_pPool );
	IPhysicsEnvironment *pWide = CreateParallel( pParallel, parallelWorkers, s_pPool );
	IPhysicsEnvironment *pPlain = s_pPhysics->CreateEnvironment();
	Check( TIER_GAMEPLAY, "parallel.creates",
	    pSerial && pWide && pPlain && EnvironmentCount() == before + 3 );
	Check( TIER_GAMEPLAY, "parallel.count-created",
	    WorkerCountOf( pParallel, pSerial ) == 1 &&
	        WorkerCountOf( pParallel, pWide ) == parallelWorkers,
	    "serial %d, wide %d (expected %d)", WorkerCountOf( pParallel, pSerial ),
	    WorkerCountOf( pParallel, pWide ), parallelWorkers );
	Check( TIER_GAMEPLAY, "parallel.count-plain", WorkerCountOf( pParallel, pPlain ) == 1 );
	int notAnEnvironment = 0;
	Check( TIER_GAMEPLAY, "parallel.count-foreign",
	    pParallel->GetWorkerCount( NULL ) == 0 &&
	        pParallel->GetWorkerCount( (const IPhysicsEnvironment *)&notAnEnvironment ) == 0 );
	s_pPhysics->DestroyEnvironment( pSerial );
	s_pPhysics->DestroyEnvironment( pWide );
	s_pPhysics->DestroyEnvironment( pPlain );
	Check( TIER_GAMEPLAY, "parallel.destroy",
	    EnvironmentCount() == before && pParallel->GetWorkerCount( pWide ) == 0 );

	// The pool is the worker budget: more than one worker needs a pool, and at
	// most its threads plus the calling thread.
	{
		IPhysicsEnvironment *pNoPool = CreateParallel( pParallel, 2, NULL );
		Check( TIER_GAMEPLAY, "parallel.requires-pool", !pNoPool && EnvironmentCount() == before );
		if ( pNoPool )
			s_pPhysics->DestroyEnvironment( pNoPool );
		IPhysicsEnvironment *pOverPool = CreateParallel( pParallel, poolThreads + 2, s_pPool );
		Check( TIER_GAMEPLAY, "parallel.rejects-over-pool",
		    !pOverPool && EnvironmentCount() == before, "%d workers on a %d-thread pool",
		    poolThreads + 2, poolThreads );
		if ( pOverPool )
			s_pPhysics->DestroyEnvironment( pOverPool );
	}

	// Stepping starts no threads beyond the host pool. The census can see a
	// new thread (negative control).
	{
		std::atomic<bool> release( false );
		int before = StableThreadCount();
		std::thread extra(
		    [&]()
		    {
			    while ( !release.load() )
				    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		    } );
		int during = StableThreadCount();
		release = true;
		extra.join();
		Check( TIER_GAMEPLAY, "parallel.census-detects-thread", before > 0 && during == before + 1,
		    "before %d, with one extra thread %d", before, during );

		before = StableThreadCount();
		bool ok;
		PileDigest( Pile( parallelWorkers ), pAuthoredCube, &ok );
		release = false;
		std::thread *pPrivate = NULL;
		// A provider that started a thread of its own while stepping.
		if ( FaultIs( "parallel-private-threads" ) )
			pPrivate = new std::thread(
			    [&]()
			    {
				    while ( !release.load() )
					    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
			    } );
		int after = StableThreadCount();
		release = true;
		if ( pPrivate )
		{
			pPrivate->join();
			delete pPrivate;
		}
		Check( TIER_GAMEPLAY, "parallel.no-provider-threads", ok && before > 0 && after <= before,
		    "threads before %d, after stepping %d workers %d", before, parallelWorkers, after );
	}

	// Simulate from inside a pool job: the job's own wait must run the tasks
	// it needs, with only one other pool thread free.
	{
		bool okSerial;
		unsigned long long serial = PileDigest( Pile( 1 ), pAuthoredCube, &okSerial );
		IThreadPool *pNestedPool = StartPool( 2, "PhysNested" );
		NestedRun_t run;
		run.pile = Pile( 3 );
		run.pile.pPool = pNestedPool;
		run.pCube = pAuthoredCube;
		run.digest = 0;
		run.ok = false;
		run.onPoolThread = false;
		run.host = std::this_thread::get_id();
		CJob *pJob = pNestedPool->QueueCall( RunNested, &run );
		// Poll rather than wait: a wait would run the job on this thread.
		double deadline = NowUs() + ( FaultIs( "parallel-nested-timeout" ) ? 0.0 : 60e6 );
		while ( pJob && !pJob->IsFinished() && NowUs() < deadline )
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		bool finished = pJob && pJob->IsFinished();
		Check( TIER_GAMEPLAY, "parallel.nested-simulate",
		    finished && run.ok && run.onPoolThread && run.digest == serial && okSerial,
		    "finished %d, on pool thread %d, digest %016llx (serial %016llx)", finished,
		    run.onPoolThread, run.digest, serial );
		ThreadPoolSchedulingStats_t stats;
		GetThreadPoolSchedulingStats( pNestedPool, &stats );
		Check( TIER_GAMEPLAY, "parallel.nested-stats",
		    finished && stats.nForbiddenNestedWaits == 0 && stats.nStarvationEvents == 0,
		    "forbidden nested waits %d, starvation events %d", stats.nForbiddenNestedWaits,
		    stats.nStarvationEvents );
		if ( finished )
		{
			pJob->Release();
			DestroyPool( pNestedPool );
		}
		// An unfinished job keeps its pool: tearing it down would hang here
		// instead of reporting.
	}

	// A pool stopped while its environment lives: tasks then run on the
	// waiting thread, and the result is unchanged.
	{
		bool okSerial, okStopped;
		unsigned long long serial = PileDigest( Pile( 1 ), pAuthoredCube, &okSerial );
		IThreadPool *pStopping = StartPool( 3, "PhysStopping" );
		PileRun_t run = Pile( 4 );
		run.pPool = pStopping;
		run.stopPoolAfter = 30;
		unsigned long long stopped = PileDigest( run, pAuthoredCube, &okStopped );
		DestroyPool( pStopping );
		Check( TIER_GAMEPLAY, "parallel.pool-stopped", okSerial && okStopped && stopped == serial,
		    "digest %016llx (serial %016llx)", stopped, serial );
	}

	// The oracle can see concurrent entry (negative control of the probe).
	{
		CConcurrencyProbe probe;
		probe.ForceOverlap();
		Check( TIER_GAMEPLAY, "parallel.probe-detects-overlap", probe.m_maxInFlight.load() == 2 );
	}

	// The game's solver is never entered concurrently, and events stay on the
	// calling thread, while a pile is stepped on several workers.
	{
		BenchOptions_t options;
		memset( &options, 0, sizeof( options ) );
		options.pScene = "pile";
		options.count = 512;
		options.seed = 11;
		Scene_t scene;
		CConcurrencyProbe probe;
		CThreadProbeEvents events;
		bool ok = BeginScene( scene, parallelWorkers );
		if ( ok )
		{
			scene.pEnv->SetCollisionSolver( &probe );
			scene.pEnv->SetCollisionEventHandler( &events );
			ok = BuildScene( scene, options, pAuthoredCube );
		}
		for ( int tick = 0; ok && tick < 90; tick++ )
			scene.pEnv->Simulate( kTick );
		// A provider that entered the solver from two threads at once.
		if ( FaultIs( "parallel-solver-concurrent" ) )
			probe.ForceOverlap();
		Check( TIER_GAMEPLAY, "parallel.solver-serialized",
		    ok && probe.m_calls.load() > 0 && probe.m_maxInFlight.load() == 1,
		    "calls %d, max in flight %d", probe.m_calls.load(), probe.m_maxInFlight.load() );
		Check( TIER_GAMEPLAY, "parallel.events-on-caller",
		    ok && events.m_events > 0 && events.m_offCaller == 0,
		    "events %d, off the calling thread %d", events.m_events, events.m_offCaller );
		// The step profile's solver counts agree with the probe's own.
		IPhysicsStepProfile *pProfile = StepProfile();
		physics_stepprofile_t profile;
		memset( &profile, 0, sizeof( profile ) );
		if ( pProfile && scene.pEnv )
			ReadProfile( pProfile, scene.pEnv, &profile );
		Check( TIER_GAMEPLAY, "profile.solver-counts",
		    !pProfile || ( profile.solverCalls == probe.m_calls.load() &&
		                     profile.solverCallsOffCaller == probe.m_offCaller.load() ),
		    "profile %d calls, %d off the caller; probe %d, %d", profile.solverCalls,
		    profile.solverCallsOffCaller, probe.m_calls.load(), probe.m_offCaller.load() );
		Metric( "solver_calls", probe.m_calls.load() );
		Metric( "solver_calls_off_caller", probe.m_offCaller.load() );
		ThreadPoolSchedulingStats_t stats;
		GetThreadPoolSchedulingStats( s_pPool, &stats );
		Check( TIER_GAMEPLAY, "parallel.pool-stats",
		    stats.nForbiddenNestedWaits == 0 && stats.nStarvationEvents == 0,
		    "forbidden nested waits %d, starvation events %d", stats.nForbiddenNestedWaits,
		    stats.nStarvationEvents );
		if ( scene.pEnv )
		{
			scene.pEnv->SetCollisionSolver( NULL );
			scene.pEnv->SetCollisionEventHandler( NULL );
		}
		EndScene( scene );
	}

	// Worker-count invariance, when claimed: plain, 1 and N workers agree
	// bitwise on a pile after 120 ticks.
	Metric( "invariance_claimed", pParallel->IsWorkerCountInvariant() ? 1 : 0 );
	if ( pParallel->IsWorkerCountInvariant() )
	{
		bool okPlain, okSerial, okWide;
		unsigned long long plain = PileDigest( Pile( 0 ), pAuthoredCube, &okPlain );
		unsigned long long serial = PileDigest( Pile( 1 ), pAuthoredCube, &okSerial );
		unsigned long long wide = PileDigest( Pile( parallelWorkers ), pAuthoredCube, &okWide );
		Check( TIER_GAMEPLAY, "parallel.worker-invariant",
		    okPlain && okSerial && okWide && plain == serial && serial == wide,
		    "plain %016llx, 1 worker %016llx, %d workers %016llx", plain, serial, parallelWorkers,
		    wide );
	}
	RunStepProfileContract( pAuthoredCube, parallelWorkers );
	return 0;
}

CPhysCollide *LoadAuthoredCube( const char *pPath, vcollide_t *pCollide )
{
	memset( pCollide, 0, sizeof( *pCollide ) );
	if ( !pPath )
		return NULL;
	int size = 0;
	char *pData = ReadFixtureFile( pPath, &size );
	if ( !pData || size <= (int)sizeof( phyheader_t ) )
	{
		free( pData );
		return NULL;
	}
	phyheader_t header;
	memcpy( &header, pData, sizeof( header ) );
	s_pCollision->VCollideLoad(
	    pCollide, header.solidCount, pData + sizeof( header ), size - (int)sizeof( header ) );
	free( pData );
	return pCollide->solidCount > 0 ? pCollide->solids[0] : NULL;
}
}

int RunBench( const BenchOptions_t &options )
{
	// The host's pool: workers - 1 threads unless given (the contract needs 3).
	bool contract = !V_strcmp( options.pScene, "contract" );
	int poolThreads =
	    options.poolThreads >= 0 ? options.poolThreads : ( contract ? 3 : options.workers - 1 );
	if ( poolThreads > 0 )
		s_pPool = StartPool( poolThreads, "PhysBench" );
	vcollide_t cube;
	CPhysCollide *pAuthoredCube = LoadAuthoredCube( options.pCubePath, &cube );
	if ( options.pCubePath )
		Check( TIER_BOOT, "bench.authored-cube", pAuthoredCube != NULL, "%s", options.pCubePath );
	int status;
	if ( contract )
		status = RunParallelContract( pAuthoredCube );
	else
		status = RunScene( options, pAuthoredCube );
	if ( cube.solidCount )
		s_pCollision->VCollideUnload( &cube );
	DestroyPool( s_pPool );
	s_pPool = NULL;
	return status;
}
