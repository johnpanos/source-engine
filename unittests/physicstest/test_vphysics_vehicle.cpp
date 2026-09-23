//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics provider conformance: vehicle controllers. HL2's jeep
//          script on the buggy body (real wheels) and the airboat script on
//          the airboat body (raycast pontoons), driven the way
//          CFourWheelVehiclePhysics drives them: parsed through the shared
//          vehicle parser, one Update per tick with the player's controls.
//          Contract record: unittests/physicstest/contracts/vphysics.provider.v1.md
//
//          The game places the axles from model attachments; the fixture
//          derives them from the body's collision bounds instead, the same
//          for every provider.
//
//=============================================================================//

#include <math.h>
#include <string.h>

#include "vphysics_conformance.h"
#include "vphysics/vehicles.h"
#include "vcollide_parse.h"
#include "vcollide.h"
#include "phyfile.h"
#include "cmodel.h"
#include "gametrace.h"
#include "bspflags.h"
#include "saverestore_host.h"

namespace
{
// Objects of one vehicle (same game data) never collide with each other,
// as FVPHYSICS_NO_SELF_COLLISIONS makes the game's solver decide.
class CVehicleSolver : public IPhysicsCollisionSolver
{
public:
	virtual int ShouldCollide( IPhysicsObject *, IPhysicsObject *, void *pGameData0, void *pGameData1 )
	{
		return !( pGameData0 && pGameData0 == pGameData1 );
	}
	virtual int ShouldSolvePenetration( IPhysicsObject *, IPhysicsObject *, void *, void *, float ) { return 1; }
	virtual bool ShouldFreezeObject( IPhysicsObject * ) { return false; }
	virtual int AdditionalCollisionChecksThisTick( int ) { return 0; }
	virtual bool ShouldFreezeContacts( IPhysicsObject **, int ) { return false; }
};

// The game's vehicle traces over a ground slab, optionally with water filling
// everything below z = 0.
class CGroundTrace : public IPhysicsGameTrace
{
public:
	CGroundTrace( CPhysCollide *pGround, bool water ) : m_pGround( pGround ), m_water( water ) {}

	virtual void VehicleTraceRay( const Ray_t &ray, void *pVehicle, trace_t *pTrace )
	{
		s_pCollision->TraceBox( ray, m_pGround, vec3_origin, vec3_angle, pTrace );
		if ( pTrace->fraction < 1.0f )
		{
			pTrace->contents = CONTENTS_SOLID;
			pTrace->surface.surfaceProps = (unsigned short)MAX( s_pProps->GetSurfaceIndex( "concrete" ), 0 );
		}
	}
	virtual void VehicleTraceRayWithWater( const Ray_t &ray, void *pVehicle, trace_t *pTrace )
	{
		VehicleTraceRay( ray, pVehicle, pTrace );
		if ( !m_water )
			return;
		Vector start = ray.m_Start + ray.m_StartOffset;
		float dz = ray.m_Delta.z;
		if ( start.z < 0.0f )
		{
			// Starting under water: report where the ray leaves it.
			pTrace->startsolid = true;
			pTrace->fractionleftsolid = dz > 0.0f ? MIN( -start.z / dz, 1.0f ) : 1.0f;
			return;
		}
		if ( dz < 0.0f && start.z + dz < 0.0f )
		{
			float fraction = -start.z / dz;
			if ( fraction < pTrace->fraction )
			{
				pTrace->fraction = fraction;
				pTrace->endpos = start + ray.m_Delta * fraction;
				pTrace->plane.normal.Init( 0, 0, 1 );
				pTrace->contents = CONTENTS_WATER;
				pTrace->surface.surfaceProps = (unsigned short)MAX( s_pProps->GetSurfaceIndex( "water" ), 0 );
			}
		}
	}
	virtual bool VehiclePointInWater( const Vector &point ) { return m_water && point.z < 0.0f; }

private:
	CPhysCollide *m_pGround;
	bool m_water;
};

struct Vehicle_t
{
	vcollide_t collide;
	vehicleparams_t params;
	objectparams_t bodyParams;
	Vector mins, maxs;
	bool loaded;
};

bool LoadVehicle( const VehicleFixture_t &fixture, Vehicle_t &vehicle )
{
	memset( &vehicle, 0, sizeof( vehicle ) );
	char *pScript = ReadFixtureFile( fixture.pScript, NULL );
	int size = 0;
	char *pModel = ReadFixtureFile( fixture.pModel, &size );
	if ( !pScript || !pModel || size <= (int)sizeof( phyheader_t ) )
		return false;

	IVPhysicsKeyParser *pParser = s_pCollision->VPhysicsKeyParserCreate( pScript );
	bool parsed = false;
	while ( !pParser->Finished() )
	{
		if ( !V_stricmp( pParser->GetCurrentBlockName(), "vehicle" ) )
		{
			pParser->ParseVehicle( &vehicle.params, NULL );
			parsed = true;
		}
		else
		{
			pParser->SkipBlock();
		}
	}
	s_pCollision->VPhysicsKeyParserDestroy( pParser );

	phyheader_t header;
	memcpy( &header, pModel, sizeof( header ) );
	s_pCollision->VCollideLoad( &vehicle.collide, header.solidCount, pModel + sizeof( header ), size - (int)sizeof( header ) );
	if ( !parsed || vehicle.collide.solidCount < 1 )
		return false;
	s_pCollision->CollideGetAABB( &vehicle.mins, &vehicle.maxs, vehicle.collide.solids[0], vec3_origin, vec3_angle );

	// CFourWheelVehiclePhysics::CalcWheelData from the body's bounds: axles
	// at 20% and 80% of the length, wheels just inside the sides, hanging
	// below the body with a quarter-radius of travel.
	Vector size3 = vehicle.maxs - vehicle.mins;
	for ( int i = 0; i < vehicle.params.axleCount && i < 2; i++ )
	{
		vehicle_axleparams_t &axle = vehicle.params.axles[i];
		float radius = axle.wheels.radius;
		float y = vehicle.mins.y + size3.y * ( i == 0 ? 0.8f : 0.2f );
		axle.offset.Init( ( vehicle.mins.x + vehicle.maxs.x ) * 0.5f, y, vehicle.mins.z + radius * 0.3f );
		axle.wheelOffset.Init( size3.x * 0.45f, 0, 0 );
		axle.wheels.springAdditionalLength = radius * 0.25f;
		axle.raytraceCenterOffset.Init( axle.offset.x, y, vehicle.mins.z + 4.0f );
		axle.raytraceOffset.Init( size3.x * 0.4f, 0, 0 );
	}
	vehicle.bodyParams = DefaultParams( vehicle.params.body.massOverride > 0 ? vehicle.params.body.massOverride : 1000.0f, NULL );
	vehicle.loaded = true;
	return true;
}

// Sensitivity: "vehicle-stub" models the former provider that created no
// vehicle controller.
IPhysicsVehicleController *Faulted( IPhysicsEnvironment *pEnv, IPhysicsVehicleController *pController )
{
	// The hidden controller stays attached to its body (destroying it first
	// is unsafe on IVP) and goes with the body.
	if ( pController && FaultIs( "vehicle-stub" ) )
		return NULL;
	return pController;
}

struct Car_t
{
	IPhysicsObject *pBody;
	IPhysicsVehicleController *pController;
	int gameData;
};

bool CreateCar( IPhysicsEnvironment *pEnv, Vehicle_t &vehicle, unsigned int type, const Vector &position, IPhysicsGameTrace *pTrace,
	int material, Car_t &car )
{
	car.gameData = 1;
	objectparams_t params = vehicle.bodyParams;
	params.pGameData = &car.gameData;
	Vector massCenter = vehicle.params.body.massCenterOverride;
	if ( massCenter != vec3_origin )
		params.massCenterOverride = &massCenter;
	car.pBody = pEnv->CreatePolyObject( vehicle.collide.solids[0], material, position, vec3_angle, &params );
	car.pController = car.pBody ? Faulted( pEnv, pEnv->CreateVehicleController( car.pBody, vehicle.params, type, pTrace ) ) : NULL;
	return car.pController != NULL;
}

// In the game's order: body, then controller.
void DestroyCar( IPhysicsEnvironment *pEnv, Car_t &car )
{
	if ( car.pBody )
		pEnv->DestroyObject( car.pBody );
	if ( car.pController )
		pEnv->DestroyVehicleController( car.pController );
	car.pController = NULL;
	car.pBody = NULL;
}

void Drive( IPhysicsEnvironment *pEnv, Car_t &car, vehicle_controlparams_t &controls, float seconds )
{
	int ticks = (int)( seconds / kTick + 0.5f );
	for ( int i = 0; i < ticks; i++ )
	{
		car.pController->Update( kTick, controls );
		Step( pEnv, kTick );
	}
}

vehicle_controlparams_t Controls( float throttle, float steering, float brake )
{
	vehicle_controlparams_t controls;
	memset( &controls, 0, sizeof( controls ) );
	controls.throttle = throttle;
	controls.steering = steering;
	controls.brake = brake;
	controls.bHasBrakePedal = true;
	return controls;
}

int ObjectCount( IPhysicsEnvironment *pEnv )
{
	int count = 0;
	pEnv->GetObjectList( &count );
	return count;
}

float Yaw( IPhysicsObject *pObject )
{
	QAngle angles;
	pObject->GetPosition( NULL, &angles );
	return angles.y;
}

// Where a body with the fixture's axles rests its wheels on z = 0.
float RestHeight( const Vehicle_t &vehicle )
{
	float lowest = FLT_MAX;
	for ( int i = 0; i < vehicle.params.axleCount; i++ )
		lowest = MIN( lowest, vehicle.params.axles[i].offset.z - vehicle.params.axles[i].wheels.radius );
	return 1.0f - lowest;
}

void TestCar( Vehicle_t &vehicle )
{
	CVehicleSolver solver;
	World_t world;
	if ( !CreateWorld( world, &solver ) )
		return;
	CPhysCollide *pGround = s_pCollision->BBoxToCollide( Vector( -12000, -12000, -32 ), Vector( 12000, 12000, 0 ) );
	objectparams_t groundParams = DefaultParams( 1.0f, NULL );
	IPhysicsObject *pGroundObject = world.pEnv->CreatePolyObjectStatic( pGround, world.material, vec3_origin, vec3_angle, &groundParams );
	CGroundTrace trace( pGround, false );
	int before = ObjectCount( world.pEnv );

	Car_t car;
	memset( &car, 0, sizeof( car ) );
	Vector start( 0, -4000, RestHeight( vehicle ) );
	if ( !Check( TIER_GAMEPLAY, "vehicle.car-create", CreateCar( world.pEnv, vehicle, VEHICLE_TYPE_CAR_WHEELS, start, &trace, world.material, car ) ) )
	{
		DestroyCar( world.pEnv, car );
		DestroyWorld( world );
		return;
	}
	IPhysicsVehicleController *pCar = car.pController;
	int wheels = pCar->GetWheelCount();
	bool wheelsValid = wheels == vehicle.params.axleCount * vehicle.params.wheelsPerAxle;
	for ( int i = 0; i < wheels; i++ )
	{
		IPhysicsObject *pWheel = pCar->GetWheel( i );
		wheelsValid = wheelsValid && pWheel && ( pWheel->GetCallbackFlags() & CALLBACK_IS_VEHICLE_WHEEL ) &&
			pWheel->GetGameData() == car.pBody->GetGameData() && pWheel->IsCollisionEnabled();
	}
	Check( TIER_GAMEPLAY, "vehicle.car-wheels", wheelsValid && ObjectCount( world.pEnv ) == before + 1 + wheels, "wheels %d objects %d",
		wheels, ObjectCount( world.pEnv ) - before );
	// Script speeds (mph) are converted to game units per second.
	float maxSpeed = pCar->GetVehicleParams().engine.maxSpeed;
	Check( TIER_GAMEPLAY, "vehicle.params-converted", Near( maxSpeed, 35.0f * 0.44707f / 0.0254f, 0.5f ), "max speed %.2f", maxSpeed );
	printf( "OBS vehicle.car-max-speed r0.001 %.4f\n", maxSpeed );

	// Parked: settles on its wheels.
	vehicle_controlparams_t idle = Controls( 0, 0, 0 );
	Drive( world.pEnv, car, idle, 2.0f );
	Vector parked = PositionOf( car.pBody );
	Vector up = AxisOf( car.pBody, 2 );
	const vehicle_operatingparams_t &state = pCar->GetOperatingParams();
	Check( TIER_GAMEPLAY, "vehicle.car-settles", up.z > 0.95f && state.wheelsInContact == wheels && IsFiniteVec( parked ) &&
		fabsf( VelocityOf( car.pBody ).Length() ) < 10.0f, "up %.3f contact %d height %.2f", up.z, state.wheelsInContact, parked.z );
	float heights[1] = { parked.z };
	ObsFloats( "vehicle.car-ride-height", "a4", 1, heights );
	Vector contact;
	int surface = -1;
	bool touching = pCar->GetWheelContactPoint( 0, &contact, &surface );
	Check( TIER_GAMEPLAY, "vehicle.wheel-contact", touching && Near( contact.z, 0.0f, 2.0f ) && surface >= 0, "z %.2f", contact.z );

	// Throttle drives it forward (the body's +y) and shifts up.
	vehicle_controlparams_t gas = Controls( 1, 0, 0 );
	Vector forward = AxisOf( car.pBody, 1 );
	Drive( world.pEnv, car, gas, 3.0f );
	float driven = DotProduct( PositionOf( car.pBody ) - parked, forward );
	float speed = state.speed;
	Check( TIER_GAMEPLAY, "vehicle.car-drives", driven > 150.0f && speed > 150.0f && state.gear > 0 && state.engineRPM > 0,
		"moved %.1f speed %.1f gear %d rpm %.0f", driven, speed, state.gear, state.engineRPM );
	printf( "OBS vehicle.car-gear a1 %d\n", state.gear );
	float drive[2] = { driven, speed };
	ObsFloats( "vehicle.car-drive", "r0.35", 2, drive );

	// Steering turns it; full lock one way changes the heading.
	float yaw = Yaw( car.pBody );
	vehicle_controlparams_t turn = Controls( 0.5f, 1, 0 );
	Drive( world.pEnv, car, turn, 1.0f );
	float turned = AngleDiff( Yaw( car.pBody ), yaw );
	Check( TIER_GAMEPLAY, "vehicle.car-steers", fabsf( turned ) > 5.0f && fabsf( state.steeringAngle ) > 1.0f, "turned %.1f steer %.1f",
		turned, state.steeringAngle );
	printf( "OBS vehicle.car-turn-sign x %d\n", turned > 0 ? 1 : -1 );

	// Braking stops it.
	vehicle_controlparams_t brake = Controls( 0, 0, 1 );
	float before_brake = fabsf( state.speed );
	Drive( world.pEnv, car, brake, 2.5f );
	Check( TIER_GAMEPLAY, "vehicle.car-brakes", fabsf( state.speed ) < 0.35f * before_brake, "speed %.1f from %.1f", state.speed, before_brake );

	// Torque boost engages and starts its recharge timer.
	vehicle_controlparams_t boost = Controls( 1, 0, 0 );
	boost.boost = 1;
	Drive( world.pEnv, car, boost, kTick );
	bool boosting = state.isTorqueBoosting && state.boostDelay > 0;
	float delay = pCar->UpdateBooster( 0.5f );
	Check( TIER_GAMEPLAY, "vehicle.car-boost", boosting && delay > 0 && delay < state.boostDelay + 0.6f, "torque %d delay %.2f",
		state.isTorqueBoosting, delay );

	// Occupancy and engine switches.
	pCar->OnVehicleEnter();
	pCar->SetEngineDisabled( true );
	bool disabled = pCar->IsEngineDisabled();
	pCar->OnVehicleExit();
	Check( TIER_GAMEPLAY, "vehicle.enter-exit", disabled && !pCar->IsEngineDisabled() );

	// The game's teardown: the entity removes its body first, which shuts
	// the car system down with its wheels (IVP's object listener), then
	// destroys the controller. (Destroying the controller first leaves IVP
	// a dangling listener on the body, so it is not exercised.)
	int withCar = ObjectCount( world.pEnv );
	world.pEnv->DestroyObject( car.pBody );
	car.pBody = NULL;
	int afterBody = ObjectCount( world.pEnv );
	Step( world.pEnv, kTick );
	world.pEnv->DestroyVehicleController( car.pController );
	car.pController = NULL;
	Check( TIER_GAMEPLAY, "vehicle.body-removes-wheels", afterBody == withCar - 1 - wheels && ObjectCount( world.pEnv ) == afterBody,
		"%d -> %d -> %d", withCar, afterBody, ObjectCount( world.pEnv ) );
	world.pEnv->DestroyObject( pGroundObject );
	DestroyWorld( world );
}

// Save and restore a driving car the way the game does: the body and its
// wheels as objects, then the controller, into a fresh level.
void TestCarSaveRestore( Vehicle_t &vehicle )
{
	CVehicleSolver solver;
	World_t world;
	if ( !CreateWorld( world, &solver ) )
		return;
	CPhysCollide *pGround = s_pCollision->BBoxToCollide( Vector( -12000, -12000, -32 ), Vector( 12000, 12000, 0 ) );
	objectparams_t groundParams = DefaultParams( 1.0f, NULL );
	IPhysicsObject *pGroundObject = world.pEnv->CreatePolyObjectStatic( pGround, world.material, vec3_origin, vec3_angle, &groundParams );
	CGroundTrace trace( pGround, false );
	Car_t car;
	memset( &car, 0, sizeof( car ) );
	if ( !CreateCar( world.pEnv, vehicle, VEHICLE_TYPE_CAR_WHEELS, Vector( 0, -4000, RestHeight( vehicle ) ), &trace, world.material, car ) )
	{
		Check( TIER_GAMEPLAY, "vehicle.save-car", false );
		DestroyCar( world.pEnv, car );
		world.pEnv->DestroyObject( pGroundObject );
		DestroyWorld( world );
		return;
	}
	vehicle_controlparams_t idle = Controls( 0, 0, 0 ), gas = Controls( 1, 0, 0 );
	Drive( world.pEnv, car, idle, 1.0f );
	Drive( world.pEnv, car, gas, 1.0f );
	int wheels = car.pController->GetWheelCount();
	float speed = car.pController->GetOperatingParams().speed;

	CSaveRestoreBuffer buffer;
	CHostSave save( &buffer );
	CUtlVector<void *> saved;
	CUtlVector<PhysInterfaceId_t> types;
	saved.AddToTail( car.pBody );
	types.AddToTail( PIID_IPHYSICSOBJECT );
	for ( int i = 0; i < wheels; i++ )
	{
		saved.AddToTail( car.pController->GetWheel( i ) );
		types.AddToTail( PIID_IPHYSICSOBJECT );
	}
	saved.AddToTail( car.pController );
	types.AddToTail( PIID_IPHYSICSVEHICLECONTROLLER );
	bool written = true;
	for ( int i = 0; i < saved.Count(); i++ )
	{
		physsaveparams_t params = { &save, saved[i], types[i] };
		written &= world.pEnv->Save( params );
	}
	Check( TIER_GAMEPLAY, "vehicle.save-write", written && buffer.Errors() == 0 );
	IPhysicsObject *pOldGround = pGroundObject, *pOldFloor = world.pFloor;
	DestroyCar( world.pEnv, car );
	world.pEnv->DestroyObject( pGroundObject );
	DestroyWorld( world );

	World_t restored;
	CreateWorld( restored, &solver );
	pGroundObject = restored.pEnv->CreatePolyObjectStatic( pGround, restored.material, vec3_origin, vec3_angle, &groundParams );
	physprerestoreparams_t pre;
	pre.recreatedObjectCount = 2;
	pre.recreatedObjectList[0].pOldObject = pOldGround;
	pre.recreatedObjectList[0].pNewObject = pGroundObject;
	pre.recreatedObjectList[1].pOldObject = pOldFloor;
	pre.recreatedObjectList[1].pNewObject = restored.pFloor;
	restored.pEnv->PreRestore( pre );
	CHostRestore restore( &buffer );
	CUtlVector<void *> created;
	bool read = true;
	for ( int i = 0; i < saved.Count(); i++ )
	{
		void *pNew = NULL;
		physrestoreparams_t params;
		memset( &params, 0, sizeof( params ) );
		params.pRestore = &restore;
		params.ppObject = &pNew;
		params.type = types[i];
		params.pGameData = &car.gameData;
		params.pName = "vehicle";
		params.pCollisionModel = i == 0 ? vehicle.collide.solids[0] : NULL;
		params.pEnvironment = restored.pEnv;
		params.pGameTrace = &trace;
		read &= restored.pEnv->Restore( params ) && pNew;
		created.AddToTail( pNew );
	}
	restored.pEnv->PostRestore();
	Check( TIER_GAMEPLAY, "vehicle.restore-read", read && buffer.Errors() == 0 && restore.GetReadPos() == buffer.Size() );
	if ( read )
	{
		Car_t back;
		back.gameData = car.gameData;
		back.pBody = (IPhysicsObject *)created[0];
		back.pController = (IPhysicsVehicleController *)created[created.Count() - 1];
		bool linked = back.pController->GetWheelCount() == wheels;
		for ( int i = 0; i < wheels && linked; i++ )
			linked = back.pController->GetWheel( i ) == created[i + 1];
		Check( TIER_GAMEPLAY, "vehicle.restore-links", linked );
		Vector restoredVelocity = VelocityOf( back.pBody );
		Vector start = PositionOf( back.pBody );
		Vector forward = AxisOf( back.pBody, 1 );
		Drive( restored.pEnv, back, gas, 1.0f );
		float driven = DotProduct( PositionOf( back.pBody ) - start, forward );
		Check( TIER_GAMEPLAY, "vehicle.restore-drives", restoredVelocity.Length() > 0.5f * fabsf( speed ) && driven > 100.0f,
			"velocity %.1f (saved speed %.1f) moved %.1f", restoredVelocity.Length(), speed, driven );
		DestroyCar( restored.pEnv, back );
	}
	restored.pEnv->DestroyObject( pGroundObject );
	DestroyWorld( restored );
}

void TestAirboat( Vehicle_t &vehicle )
{
	for ( int pass = 0; pass < 2; pass++ )
	{
		bool water = pass == 1;
		CVehicleSolver solver;
		World_t world;
		if ( !CreateWorld( world, &solver ) )
			return;
		// Water fills z < 0 over a floor 256 units down; the land pass uses a
		// dry floor at z = 0.
		float floorTop = water ? -256.0f : 0.0f;
		CPhysCollide *pGround = s_pCollision->BBoxToCollide( Vector( -12000, -12000, floorTop - 32 ), Vector( 12000, 12000, floorTop ) );
		objectparams_t groundParams = DefaultParams( 1.0f, NULL );
		IPhysicsObject *pGroundObject = world.pEnv->CreatePolyObjectStatic( pGround, world.material, vec3_origin, vec3_angle, &groundParams );
		CGroundTrace trace( pGround, water );
		Car_t boat;
		memset( &boat, 0, sizeof( boat ) );
		// The pontoon raycasts start 4 units into the hull bottom.
		Vector start( 0, -4000, water ? -vehicle.mins.z - 12.0f : -vehicle.mins.z + 2.0f );
		// Normally the game moves the default floor out of the way; the
		// water pass sinks it with the pool bottom.
		if ( water )
			world.pFloor->SetPosition( Vector( 0, 0, -512 ), vec3_angle, true );
		const char *pName = water ? "vehicle.airboat-water" : "vehicle.airboat-land";
		char name[128];
		V_snprintf( name, sizeof( name ), "%s-create", pName );
		if ( Check( TIER_GAMEPLAY, name, CreateCar( world.pEnv, vehicle, VEHICLE_TYPE_AIRBOAT_RAYCAST, start, &trace, world.material, boat ) ) )
		{
			IPhysicsVehicleController *pBoat = boat.pController;
			vehicle_controlparams_t idle = Controls( 0, 0, 0 );
			Drive( world.pEnv, boat, idle, 3.0f );
			Vector rest = PositionOf( boat.pBody );
			float upZ = AxisOf( boat.pBody, 2 ).z;
			V_snprintf( name, sizeof( name ), "%s-rests", pName );
			// Floating: the hull bottom stays near the surface; on land the
			// pontoons hold it at the ground.
			float bottom = rest.z + vehicle.mins.z;
			bool rests = upZ > 0.9f && IsFiniteVec( rest ) && bottom > ( water ? -40.0f : -4.0f ) && bottom < 30.0f;
			Check( TIER_GAMEPLAY, name, rests, "bottom %.2f up %.3f", bottom, upZ );
			V_snprintf( name, sizeof( name ), "%s-rest-height", pName );
			ObsFloats( name, "a6", 1, &bottom );

			pBoat->OnVehicleEnter();
			Vector forward = AxisOf( boat.pBody, 1 );
			vehicle_controlparams_t gas = Controls( 1, 0, 0 );
			Drive( world.pEnv, boat, gas, 3.0f );
			float driven = DotProduct( PositionOf( boat.pBody ) - rest, forward );
			V_snprintf( name, sizeof( name ), "%s-drives", pName );
			Check( TIER_GAMEPLAY, name, driven > 100.0f, "moved %.1f speed %.1f", driven, pBoat->GetOperatingParams().speed );
			V_snprintf( name, sizeof( name ), "%s-distance", pName );
			ObsFloats( name, "r0.35", 1, &driven );

			float yaw = Yaw( boat.pBody );
			vehicle_controlparams_t turn = Controls( 1, 1, 0 );
			Drive( world.pEnv, boat, turn, 1.5f );
			float turned = AngleDiff( Yaw( boat.pBody ), yaw );
			V_snprintf( name, sizeof( name ), "%s-steers", pName );
			Check( TIER_GAMEPLAY, name, fabsf( turned ) > 5.0f, "turned %.1f", turned );
			printf( "OBS %s-turn-sign x %d\n", pName, turned > 0 ? 1 : -1 );
			pBoat->OnVehicleExit();

			vehicle_debugcarsystem_t debug;
			pBoat->GetCarSystemDebugData( debug );
			V_snprintf( name, sizeof( name ), "%s-debug-rays", pName );
			Check( TIER_GAMEPLAY, name, debug.vecWheelRaycasts[0][0] != debug.vecWheelRaycasts[0][1] );
		}
		DestroyCar( world.pEnv, boat );
		world.pEnv->DestroyObject( pGroundObject );
		DestroyWorld( world );
	}
}
}

void TestVehicles( const VehicleFixture_t *pFixtures, int count )
{
	bool car = false, airboat = false;
	for ( int i = 0; i < count; i++ )
	{
		Vehicle_t vehicle;
		bool loaded = LoadVehicle( pFixtures[i], vehicle );
		char name[128];
		V_snprintf( name, sizeof( name ), "vehicle.%s-fixture", pFixtures[i].pKind );
		if ( !Check( TIER_BOOT, name, loaded, "%s %s", pFixtures[i].pScript, pFixtures[i].pModel ) )
			continue;
		if ( !V_strcmp( pFixtures[i].pKind, "car" ) )
		{
			car = true;
			TestCar( vehicle );
			TestCarSaveRestore( vehicle );
		}
		else if ( !V_strcmp( pFixtures[i].pKind, "airboat" ) )
		{
			airboat = true;
			TestAirboat( vehicle );
		}
		s_pCollision->VCollideUnload( &vehicle.collide );
	}
	// Vehicle coverage is required, not optional.
	Check( TIER_BOOT, "vehicle.fixtures-present", car && airboat );
}
