//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics provider conformance: fluid controllers (buoyancy,
//          damping, current, fluid touch events) and springs.
//          Contract record: unittests/physicstest/contracts/vphysics.provider.v1.md
//
//          The pool is a static "water" volume away from the world floor,
//          surface at z = 0, created the way the game creates BSP fluids
//          (a static object turned into a fluid controller, damping 0.01).
//
//=============================================================================//

#include <math.h>
#include <string.h>

#include "vphysics_conformance.h"
#include "bspflags.h"

namespace
{
const Vector kPoolCenter( 4000, 0, 0 );

class CFluidEvents : public IPhysicsCollisionEvent
{
public:
	CFluidEvents() : m_start( 0 ), m_end( 0 ), m_pFluid( NULL ) {}
	virtual void PreCollision( vcollisionevent_t * ) {}
	virtual void PostCollision( vcollisionevent_t * ) {}
	virtual void Friction( IPhysicsObject *, float, int, int, IPhysicsCollisionData * ) {}
	virtual void StartTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) {}
	virtual void EndTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) {}
	virtual void FluidStartTouch( IPhysicsObject *, IPhysicsFluidController *pFluid ) { m_start++; m_pFluid = pFluid; }
	virtual void FluidEndTouch( IPhysicsObject *, IPhysicsFluidController * ) { m_end++; }
	virtual void PostSimulationFrame() {}
	int m_start, m_end;
	IPhysicsFluidController *m_pFluid;
};

// Sensitivity: "fluid-inert" models a provider whose fluids never act on
// objects; "spring-inert" one whose springs exert no force.
IPhysicsObject *Swimmer( IPhysicsObject *pObject )
{
	if ( pObject && FaultIs( "fluid-inert" ) )
		pObject->SetCallbackFlags( pObject->GetCallbackFlags() & ~CALLBACK_DO_FLUID_SIMULATION );
	return pObject;
}

IPhysicsSpring *Faulted( IPhysicsSpring *pSpring )
{
	if ( pSpring && FaultIs( "spring-inert" ) )
		pSpring->SetSpringConstant( 0.0f );
	return pSpring;
}

struct Pool_t
{
	CPhysCollide *pVolume;
	IPhysicsObject *pWater;
	IPhysicsFluidController *pFluid;
};

bool CreatePool( World_t &world, Pool_t &pool, const Vector &current )
{
	memset( &pool, 0, sizeof( pool ) );
	pool.pVolume = s_pCollision->BBoxToCollide( Vector( -600, -600, -2000 ), Vector( 600, 600, 0 ) );
	int water = s_pProps->GetSurfaceIndex( "water" );
	objectparams_t params = DefaultParams( 1.0f, NULL );
	pool.pWater = world.pEnv->CreatePolyObjectStatic( pool.pVolume, water >= 0 ? water : 0, kPoolCenter, vec3_angle, &params );
	fluidparams_t fluid;
	memset( &fluid, 0, sizeof( fluid ) );
	fluid.surfacePlane.Init( 0, 0, 1, 0 );
	fluid.currentVelocity = current;
	fluid.damping = 0.01f;
	fluid.contents = CONTENTS_WATER;
	pool.pFluid = pool.pWater ? world.pEnv->CreateFluidController( pool.pWater, &fluid ) : NULL;
	return pool.pFluid != NULL;
}

void DestroyPool( World_t &world, Pool_t &pool )
{
	if ( pool.pFluid )
		world.pEnv->DestroyFluidController( pool.pFluid );
	DestroyWorld( world );
	if ( pool.pVolume )
		s_pCollision->DestroyCollide( pool.pVolume );
	memset( &pool, 0, sizeof( pool ) );
}

// A surface property lighter than water, found by density (content differs
// between games).
int FloatingMaterial()
{
	for ( int i = 0; i < s_pProps->SurfacePropCount(); i++ )
	{
		float density = 0;
		s_pProps->GetPhysicsProperties( i, &density, NULL, NULL, NULL );
		if ( density > 100.0f && density < 900.0f )
			return i;
	}
	return -1;
}

void TestFluids()
{
	// Controller basics and floating by displacement (no authored volume:
	// Archimedes on the collision volume). A 32-unit 50 kg cube displaces
	// ~537 kg when fully submerged, so it floats ~3 units deep.
	{
		World_t world;
		CreateWorld( world, NULL );
		Pool_t pool;
		CFluidEvents events;
		world.pEnv->SetCollisionEventHandler( &events );
		if ( !Check( TIER_BOOT, "fluid.create", CreatePool( world, pool, vec3_origin ) ) )
		{
			world.pEnv->SetCollisionEventHandler( NULL );
			DestroyPool( world, pool );
			return;
		}
		Vector normal;
		float dist = -1;
		pool.pFluid->GetSurfacePlane( &normal, &dist );
		Check( TIER_BOOT, "fluid.surface-plane", NearVec( normal, Vector( 0, 0, 1 ), 1e-3f ) && Near( dist, 0.0f, 0.01f ),
			"(%.2f %.2f %.2f) %.2f", normal.x, normal.y, normal.z, dist );
		Check( TIER_BOOT, "fluid.density-contents", Near( pool.pFluid->GetDensity(), 1000.0f, 1.0f ) && pool.pFluid->GetContents() == CONTENTS_WATER,
			"density %.1f contents %d", pool.pFluid->GetDensity(), pool.pFluid->GetContents() );
		Check( TIER_GAMEPLAY, "fluid.object-flags", pool.pWater->IsFluid() && pool.pWater->IsTrigger() );

		IPhysicsObject *pFloater = Swimmer( CreateCube( world, kPoolCenter + Vector( 0, 0, 100 ) ) );
		Step( world.pEnv, 5.0f );
		float z = PositionOf( pFloater ).z;
		Check( TIER_GAMEPLAY, "fluid.floats", z > 6.0f && z < 18.0f && fabsf( VelocityOf( pFloater ).z ) < 20.0f,
			"z %.2f vz %.2f", z, VelocityOf( pFloater ).z );
		ObsFloats( "fluid.float-height", "a4", 1, &z );
		Check( TIER_GAMEPLAY, "fluid.start-touch", events.m_start > 0 && events.m_pFluid == pool.pFluid, "start %d", events.m_start );

		// Too heavy for its displacement: sinks, slower than free fall.
		IPhysicsObject *pSinker = Swimmer( CreateCube( world, kPoolCenter + Vector( 200, 0, -40 ), 2000.0f ) );
		Step( world.pEnv, 1.0f );
		float drop = -40.0f - PositionOf( pSinker ).z;
		Check( TIER_GAMEPLAY, "fluid.sinks-damped", drop > 20.0f && drop < 290.0f, "drop %.2f", drop );
		ObsFloats( "fluid.sink-drop", "r0.35", 1, &drop );

		// Leaving the volume (out through the bottom) ends the fluid touch.
		pSinker->SetPosition( kPoolCenter + Vector( 200, 0, -1990 ), vec3_angle, true );
		Vector down( 0, 0, -500 );
		pSinker->SetVelocity( &down, NULL );
		Step( world.pEnv, 0.5f );
		Check( TIER_GAMEPLAY, "fluid.end-touch", events.m_end > 0, "end %d", events.m_end );
		world.pEnv->SetCollisionEventHandler( NULL );
		DestroyPool( world, pool );
	}

	// With an authored volume, buoyancy follows the material's density:
	// lighter-than-water material floats, heavier sinks, whatever the mass.
	{
		World_t world;
		CreateWorld( world, NULL );
		Pool_t pool;
		CreatePool( world, pool, vec3_origin );
		int light = FloatingMaterial();
		objectparams_t params = DefaultParams( 400.0f, NULL );
		params.volume = s_pCollision->CollideVolume( world.pCubeCollide );
		IPhysicsObject *pLight = world.pEnv->CreatePolyObject( world.pCubeCollide, light >= 0 ? light : world.material,
			kPoolCenter + Vector( -200, 0, 60 ), vec3_angle, &params );
		params.mass = 5.0f;
		IPhysicsObject *pHeavy = world.pEnv->CreatePolyObject( world.pCubeCollide, world.material,
			kPoolCenter + Vector( 200, 0, -40 ), vec3_angle, &params );
		Swimmer( pLight )->Wake();
		Swimmer( pHeavy )->Wake();
		Step( world.pEnv, 4.0f );
		float lightZ = PositionOf( pLight ).z, heavyZ = PositionOf( pHeavy ).z;
		Check( TIER_GAMEPLAY, "fluid.material-density", light >= 0 && lightZ > -20.0f && heavyZ < -100.0f,
			"light material %d z %.2f heavy z %.2f", light, lightZ, heavyZ );
		DestroyPool( world, pool );
	}

	// A current carries floating objects along.
	{
		World_t world;
		CreateWorld( world, NULL );
		Pool_t pool;
		CreatePool( world, pool, Vector( 100, 0, 0 ) );
		IPhysicsObject *pFloater = Swimmer( CreateCube( world, kPoolCenter + Vector( 0, 0, 10 ) ) );
		Step( world.pEnv, 3.0f );
		float drift = PositionOf( pFloater ).x - kPoolCenter.x;
		Check( TIER_GAMEPLAY, "fluid.current", drift > 5.0f, "drift %.2f", drift );
		DestroyPool( world, pool );
	}

	// Objects opting out of fluid simulation (and shadows) get no buoyancy.
	{
		World_t world;
		CreateWorld( world, NULL );
		Pool_t pool;
		CreatePool( world, pool, vec3_origin );
		IPhysicsObject *pCube = CreateCube( world, kPoolCenter + Vector( 0, 0, -40 ) );
		pCube->SetCallbackFlags( pCube->GetCallbackFlags() & ~CALLBACK_DO_FLUID_SIMULATION );
		Step( world.pEnv, 1.0f );
		float drop = -40.0f - PositionOf( pCube ).z;
		Check( TIER_GAMEPLAY, "fluid.opt-out", drop > 150.0f, "drop %.2f", drop );
		DestroyPool( world, pool );
	}
}

//-----------------------------------------------------------------------------
// Springs
//-----------------------------------------------------------------------------
void TestSprings()
{
	// Hanging from the static floor: equilibrium stretch = m g / k.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 400 ) );
		springparams_t spring;
		memset( &spring, 0, sizeof( spring ) );
		spring.constant = 1000.0f;
		spring.naturalLength = 100.0f;
		spring.damping = 100.0f;
		spring.relativeDamping = 10.0f;
		spring.startPosition = Vector( 0, 0, 500 );
		spring.endPosition = Vector( 0, 0, 400 );
		spring.useLocalPositions = false;
		IPhysicsSpring *pSpring = Faulted( world.pEnv->CreateSpring( world.pFloor, pCube, &spring ) );
		if ( Check( TIER_BOOT, "spring.create", pSpring != NULL ) )
		{
			Check( TIER_BOOT, "spring.objects", pSpring->GetStartObject() == world.pFloor && pSpring->GetEndObject() == pCube );
			Vector start, end;
			pSpring->GetEndpoints( &start, &end );
			Check( TIER_BOOT, "spring.endpoints", NearVec( start, Vector( 0, 0, 500 ), 0.01f ) && NearVec( end, Vector( 0, 0, 400 ), 0.01f ) );
			Step( world.pEnv, 6.0f );
			pSpring->GetEndpoints( &start, &end );
			float length = ( start - end ).Length();
			// 50 * 600 / 1000 = 30 units of stretch.
			Check( TIER_GAMEPLAY, "spring.equilibrium", Near( length, 130.0f, 6.0f ) && VelocityOf( pCube ).Length() < 10.0f,
				"length %.2f speed %.2f", length, VelocityOf( pCube ).Length() );
			ObsFloats( "spring.equilibrium-length", "a3", 1, &length );

			pSpring->SetSpringLength( 150.0f );
			Step( world.pEnv, 6.0f );
			pSpring->GetEndpoints( &start, &end );
			length = ( start - end ).Length();
			Check( TIER_GAMEPLAY, "spring.set-length", Near( length, 180.0f, 6.0f ), "length %.2f", length );
			world.pEnv->DestroySpring( pSpring );
		}
		DestroyWorld( world );
	}

	// Compressed onlyStretch springs push nothing; ordinary springs push apart.
	for ( int k = 0; k < 2; k++ )
	{
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetGravity( vec3_origin );
		IPhysicsObject *pA = CreateCube( world, Vector( 0, 0, 500 ) );
		IPhysicsObject *pB = CreateCube( world, Vector( 40, 0, 500 ) );
		springparams_t spring;
		memset( &spring, 0, sizeof( spring ) );
		spring.constant = 2000.0f;
		spring.naturalLength = 100.0f;
		spring.startPosition = vec3_origin;
		spring.endPosition = vec3_origin;
		spring.useLocalPositions = true;
		spring.onlyStretch = k == 0;
		IPhysicsSpring *pSpring = Faulted( world.pEnv->CreateSpring( pA, pB, &spring ) );
		Step( world.pEnv, 0.3f );
		float distance = ( PositionOf( pB ) - PositionOf( pA ) ).Length();
		bool ok = k == 0 ? Near( distance, 40.0f, 1.0f ) : distance > 60.0f;
		Check( TIER_GAMEPLAY, k == 0 ? "spring.only-stretch" : "spring.pushes", pSpring && ok, "distance %.2f", distance );
		if ( pSpring )
			world.pEnv->DestroySpring( pSpring );
		DestroyWorld( world );
	}
}
}

void TestFluidsAndSprings()
{
	TestFluids();
	TestSprings();
}
