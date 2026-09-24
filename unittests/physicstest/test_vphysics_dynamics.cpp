//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gameplay dynamics ("feel") clauses of the VPhysics provider
//          conformance suite (RFC 0004 / RFC 0005 Q-PHYSICS).
//
//          The contract and API clauses elsewhere prove each method works; these
//          scenes prove a provider moves the Portal weighted cube as IVP does in
//          play: dropping and bouncing, sliding and being pushed, resting on
//          slopes and stacks, being carried by the grab controller, hitting
//          walls and spinning out. Each scene checks physical invariants that
//          hold for any provider, and records trajectory metrics as
//          observations that the runner compares with the IVP oracle.
//
//          The cube is the authored metal_box.phy solid with its keyvalue
//          parameters (40 kg, metalpanel, damping 0.1); the world uses
//          Portal 2's 60 Hz tick, gravity and the game's performance settings
//          (game/server/physics.cpp).
//
//=============================================================================//
#include "vphysics_conformance.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tier1/strtools.h"
#include "tier1/utlvector.h"
#include "vcollide.h"
#include "vcollide_parse.h"
#include "vphysics/friction.h"
#include "vphysics/performance.h"
#include "vphysics/player_controller.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const float kDynTick = 1.0f / 60.0f;		// Portal 2's tick interval
const float kGravity = 600.0f;

// Set DYN_TRACE=1 to print per-tick trajectories while diagnosing a scene.
bool TraceEnabled()
{
	static int s_enabled = -1;
	if ( s_enabled < 0 )
		s_enabled = getenv( "DYN_TRACE" ) ? 1 : 0;
	return s_enabled != 0;
}

void Trace( const char *pFormat, ... )
{
	if ( !TraceEnabled() )
		return;
	va_list args;
	va_start( args, pFormat );
	vprintf( pFormat, args );
	va_end( args );
}

void Tick( IPhysicsEnvironment *pEnv )
{
	if ( !FaultIs( "sim-noop" ) )
		pEnv->Simulate( kDynTick );
}

// The authored cube: solid 0 of the fixture with its parsed parameters.
struct CubeModel_t
{
	const CPhysCollide *pCollide;
	objectparams_t params;
	char surfaceprop[128];
	Vector mins, maxs;		// collision AABB at identity
	bool valid;
};

CubeModel_t LoadCubeModel( const vcollide_t *pFixture )
{
	CubeModel_t model;
	memset( &model, 0, sizeof( model ) );
	if ( !pFixture || pFixture->solidCount < 1 || !pFixture->pKeyValues )
		return model;
	IVPhysicsKeyParser *pParser = s_pCollision->VPhysicsKeyParserCreate( pFixture->pKeyValues );
	while ( pParser && !pParser->Finished() )
	{
		if ( V_stricmp( pParser->GetCurrentBlockName(), "solid" ) )
		{
			pParser->SkipBlock();
			continue;
		}
		solid_t solid;
		memset( &solid, 0, sizeof( solid ) );
		pParser->ParseSolid( &solid, NULL );
		if ( solid.index == 0 )
		{
			model.params = solid.params;
			V_strncpy( model.surfaceprop, solid.surfaceprop, sizeof( model.surfaceprop ) );
			model.valid = true;
		}
	}
	if ( pParser )
		s_pCollision->VPhysicsKeyParserDestroy( pParser );
	model.pCollide = pFixture->solids[0];
	s_pCollision->CollideGetAABB( &model.mins, &model.maxs, model.pCollide, vec3_origin, vec3_angle );
	model.params.pName = "metal_box";
	model.params.enableCollisions = true;
	model.valid &= model.params.mass > 0.0f && model.maxs.z > model.mins.z;
	return model;
}

// Records impacts and scrapes as the game's sound and damage code sees them.
class CFeelEvents : public IPhysicsCollisionEvent
{
public:
	CFeelEvents() { Reset(); }
	void Reset()
	{
		m_impacts = 0;
		m_audibleImpacts = 0;
		m_firstImpactSpeed = -1.0f;
		m_maxVolume = 0.0f;
		m_frictionEvents = 0;
		m_frictionEnergy = 0.0f;
	}
	virtual void PreCollision( vcollisionevent_t *pEvent ) {}
	virtual void PostCollision( vcollisionevent_t *pEvent )
	{
		m_impacts++;
		if ( m_firstImpactSpeed < 0.0f )
			m_firstImpactSpeed = pEvent->collisionSpeed;
		// PhysCollisionSound (game/server/physics.cpp): silent below 70 in/s
		// or within 0.05 s of the pair's last impact; full volume at 320.
		if ( pEvent->deltaCollisionTime >= 0.05f && pEvent->collisionSpeed >= 70.0f )
		{
			m_audibleImpacts++;
			float speed = pEvent->collisionSpeed;
			m_maxVolume = MAX( m_maxVolume, MIN( speed * speed / ( 320.0f * 320.0f ), 1.0f ) );
		}
		if ( TraceEnabled() )
		{
			Vector speed, normal, v0, v1;
			pEvent->pInternalData->GetContactSpeed( speed );
			pEvent->pInternalData->GetSurfaceNormal( normal );
			pEvent->pObjects[0]->GetVelocity( &v0, NULL );
			pEvent->pObjects[1]->GetVelocity( &v1, NULL );
			Trace( "DYN impact speed %.3f contact-speed %.3f %.3f %.3f normal %.3f %.3f %.3f v0 %.3f %.3f %.3f v1 %.3f %.3f %.3f\n",
				pEvent->collisionSpeed, speed.x, speed.y, speed.z, normal.x, normal.y, normal.z, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z );
		}
	}
	virtual void Friction( IPhysicsObject *, float energy, int, int, IPhysicsCollisionData * )
	{
		m_frictionEvents++;
		m_frictionEnergy += energy;
	}
	virtual void StartTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) {}
	virtual void EndTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) {}
	virtual void FluidStartTouch( IPhysicsObject *, IPhysicsFluidController * ) {}
	virtual void FluidEndTouch( IPhysicsObject *, IPhysicsFluidController * ) {}
	virtual void PostSimulationFrame() {}

	int m_impacts;
	int m_audibleImpacts;
	float m_firstImpactSpeed;
	float m_maxVolume;
	int m_frictionEvents;
	float m_frictionEnergy;
};

// A Portal-like world: concrete floor slab (top at z=0), Portal 2 tick and
// the game's performance settings.
struct FeelWorld_t
{
	IPhysicsEnvironment *pEnv;
	CUtlVector<CPhysCollide *> collides;
	IPhysicsObject *pFloor;
	int concrete;
	CFeelEvents events;
};

bool CreateFeelWorld( FeelWorld_t &world )
{
	world.pEnv = s_pPhysics->CreateEnvironment();
	if ( !world.pEnv )
		return false;
	physics_performanceparams_t perf;
	perf.Defaults();
	perf.maxCollisionsPerObjectPerTimestep = 10;
	world.pEnv->SetPerformanceSettings( &perf );
	world.pEnv->SetSimulationTimestep( kDynTick );
	world.pEnv->SetGravity( Vector( 0, 0, -kGravity ) );
	world.pEnv->SetCollisionEventHandler( &world.events );
	world.concrete = s_pProps->GetSurfaceIndex( "concrete" );
	if ( world.concrete < 0 )
		world.concrete = 0;
	CPhysCollide *pFloor = s_pCollision->BBoxToCollide( Vector( -2048, -2048, -64 ), Vector( 2048, 2048, 0 ) );
	world.collides.AddToTail( pFloor );
	objectparams_t params = DefaultParams( 1.0f, NULL );
	world.pFloor = world.pEnv->CreatePolyObjectStatic( pFloor, world.concrete, vec3_origin, vec3_angle, &params );
	return world.pFloor != NULL;
}

IPhysicsObject *CreateStaticBox( FeelWorld_t &world, const Vector &mins, const Vector &maxs, const Vector &origin,
	const QAngle &angles )
{
	CPhysCollide *pCollide = s_pCollision->BBoxToCollide( mins, maxs );
	world.collides.AddToTail( pCollide );
	objectparams_t params = DefaultParams( 1.0f, NULL );
	return world.pEnv->CreatePolyObjectStatic( pCollide, world.concrete, origin, angles, &params );
}

void DestroyFeelWorld( FeelWorld_t &world )
{
	if ( world.pEnv )
	{
		world.pEnv->SetCollisionEventHandler( NULL );
		s_pPhysics->DestroyEnvironment( world.pEnv );
	}
	for ( int i = 0; i < world.collides.Count(); i++ )
		s_pCollision->DestroyCollide( world.collides[i] );
	world.collides.RemoveAll();
	world.pEnv = NULL;
}

IPhysicsObject *CreateFeelCube( FeelWorld_t &world, const CubeModel_t &model, const Vector &position,
	const QAngle &angles )
{
	objectparams_t params = model.params;
	int material = s_pProps->GetSurfaceIndex( model.surfaceprop );
	IPhysicsObject *pCube = world.pEnv->CreatePolyObject( model.pCollide, material, position, angles, &params );
	if ( pCube )
	{
		pCube->EnableMotion( true );
		pCube->EnableGravity( true );
		// Props turn drag on when their keyvalues give a coefficient.
		pCube->EnableDrag( params.dragCoefficient != 0.0f );
		pCube->SetCallbackFlags( pCube->GetCallbackFlags() | CALLBACK_GLOBAL_COLLISION |
			CALLBACK_GLOBAL_COLLIDE_STATIC | CALLBACK_GLOBAL_FRICTION );
		pCube->Wake();
	}
	return pCube;
}

// Rest height of the cube's origin on the floor (its authored bottom face).
float RestHeight( const CubeModel_t &model )
{
	return -model.mins.z;
}

float SpeedOf( IPhysicsObject *pObject )
{
	return VelocityOf( pObject ).Length();
}

float AngularSpeedOf( IPhysicsObject *pObject )
{
	return AngularVelocityOf( pObject ).Length();
}

// How far the cube's nearest local axis is from world up, in degrees: 0 when
// it lies flat on any face.
float TiltFromFlat( IPhysicsObject *pObject )
{
	float best = 0.0f;
	for ( int axis = 0; axis < 3; axis++ )
		best = MAX( best, fabsf( AxisOf( pObject, axis ).z ) );
	return RAD2DEG( acosf( clamp( best, -1.0f, 1.0f ) ) );
}

// Tracks when a body comes to rest: speed and spin below a threshold for
// ten consecutive ticks.
struct SettleTracker_t
{
	int tick;
	int calm;
	int settledTick;
	void Init() { tick = 0; calm = 0; settledTick = -1; }
	void Update( IPhysicsObject *pObject )
	{
		tick++;
		if ( SpeedOf( pObject ) < 1.0f && AngularSpeedOf( pObject ) < 5.0f )
		{
			if ( ++calm == 10 && settledTick < 0 )
				settledTick = tick - 10;
		}
		else
		{
			calm = 0;
			settledTick = -1;
		}
	}
	float Seconds() const { return settledTick < 0 ? -1.0f : settledTick * kDynTick; }
};

void Obs1( const char *pKey, const char *pTol, float value )
{
	ObsFloats( pKey, pTol, 1, &value );
}

//-----------------------------------------------------------------------------
// Drop and bounce: a flat drop, then a tumbling drop from a tilt.
//-----------------------------------------------------------------------------
void TestDrop( const CubeModel_t &model )
{
	const float restZ = RestHeight( model );
	const float dropHeight = 64.0f;

	// Flat drop.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 0, 0, restZ + dropHeight ), vec3_angle );
		SettleTracker_t settle;
		settle.Init();
		int firstImpactTick = -1, sleepTick = -1;
		float reboundApex = 0.0f, prevVz = 0.0f, maxTilt = 0.0f;
		int rebounds = 0;
		bool landed = false;
		for ( int i = 0; i < 300; i++ )
		{
			Tick( world.pEnv );
			settle.Update( pCube );
			Vector pos = PositionOf( pCube );
			float vz = VelocityOf( pCube ).z;
			if ( !landed && world.events.m_impacts > 0 )
			{
				landed = true;
				firstImpactTick = i;
			}
			if ( landed )
			{
				reboundApex = MAX( reboundApex, pos.z - restZ );
				// A rebound: vertical velocity turns from rising to falling
				// clearly above the rest height.
				if ( prevVz > 0.0f && vz <= 0.0f && pos.z - restZ > 0.25f )
					rebounds++;
				maxTilt = MAX( maxTilt, TiltFromFlat( pCube ) );
			}
			prevVz = vz;
			if ( sleepTick < 0 && pCube->IsAsleep() )
				sleepTick = i;
			Trace( "DYN drop %d z %.3f vz %.3f tilt %.3f\n", i, pos.z, vz, TiltFromFlat( pCube ) );
		}
		Vector rest = PositionOf( pCube );
		Check( TIER_GAMEPLAY, "dynamics.drop-lands", landed && firstImpactTick > 0, "impact tick %d", firstImpactTick );
		// IVP rests objects at its collision distance, about 0.47 units up.
		Check( TIER_GAMEPLAY, "dynamics.drop-rests-flat", Near( rest.z, restZ, 0.75f ) && TiltFromFlat( pCube ) < 1.0f,
			"z %.3f (face %.3f) tilt %.3f", rest.z, restZ, TiltFromFlat( pCube ) );
		// Elasticity products are small (metalpanel on concrete); a flat drop
		// must not bounce higher than a fraction of its drop.
		Check( TIER_GAMEPLAY, "dynamics.drop-rebound-bounded", reboundApex < dropHeight * 0.25f, "apex %.3f", reboundApex );
		Check( TIER_GAMEPLAY, "dynamics.drop-settles", settle.settledTick >= 0 && settle.Seconds() < 2.0f,
			"settled %.3f s", settle.Seconds() );
		Check( TIER_GAMEPLAY, "dynamics.drop-sleeps", sleepTick >= 0, "sleep tick %d", sleepTick );
		// IVP lands a flat cube corner by corner, which leaves it rocking a
		// few degrees and a little off center; Box3D lands it flat.
		Check( TIER_GAMEPLAY, "dynamics.drop-no-drift", rest.AsVector2D().Length() < 4.0f && maxTilt < 10.0f,
			"xy drift %.3f max tilt %.3f", rest.AsVector2D().Length(), maxTilt );
		Obs1( "dynamics.drop.impact-tick", "a3", (float)firstImpactTick );
		Obs1( "dynamics.drop.first-impact-speed", "r0.05", world.events.m_firstImpactSpeed );
		Obs1( "dynamics.drop.rebound-apex", "a0.75", reboundApex );
		Obs1( "dynamics.drop.rebounds", "a1", (float)rebounds );
		Obs1( "dynamics.drop.settle-seconds", "a0.3", settle.Seconds() );
		Obs1( "dynamics.drop.sleep-seconds", "a0.5", sleepTick * kDynTick );
		Obs1( "dynamics.drop.rest-z", "a0.6", rest.z );
		Obs1( "dynamics.drop.audible-impacts", "a1", (float)world.events.m_audibleImpacts );
		Obs1( "dynamics.drop.max-volume", "a0.1", world.events.m_maxVolume );
		Trace( "DYN drop summary impacts %d audible %d friction %d energy %.3f\n", world.events.m_impacts,
			world.events.m_audibleImpacts, world.events.m_frictionEvents, world.events.m_frictionEnergy );
		DestroyFeelWorld( world );
	}

	// Tumbling drop onto an edge: lands flat on a face, near where it fell.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 0, 0, restZ + dropHeight ), QAngle( 25, 0, 15 ) );
		SettleTracker_t settle;
		settle.Init();
		int sleepTick = -1;
		for ( int i = 0; i < 360; i++ )
		{
			Tick( world.pEnv );
			settle.Update( pCube );
			if ( sleepTick < 0 && pCube->IsAsleep() )
				sleepTick = i;
			Vector pos = PositionOf( pCube );
			Trace( "DYN tumble %d pos %.3f %.3f %.3f tilt %.3f w %.3f\n", i, pos.x, pos.y, pos.z, TiltFromFlat( pCube ),
				AngularSpeedOf( pCube ) );
		}
		Vector rest = PositionOf( pCube );
		float travel = rest.AsVector2D().Length();
		Check( TIER_GAMEPLAY, "dynamics.tumble-rests-flat", Near( rest.z, restZ, 0.5f ) && TiltFromFlat( pCube ) < 1.0f,
			"z %.3f tilt %.3f", rest.z, TiltFromFlat( pCube ) );
		Check( TIER_GAMEPLAY, "dynamics.tumble-settles", settle.settledTick >= 0 && settle.Seconds() < 3.0f,
			"settled %.3f s", settle.Seconds() );
		Check( TIER_GAMEPLAY, "dynamics.tumble-travel-bounded", travel < 48.0f, "travel %.3f", travel );
		// A single tumble is chaotic (a small change in the first contact
		// moves the resting place); its travel is bounded above and compared
		// only as a mean over many drops (TestTumbleStatistics).
		Obs1( "dynamics.tumble.settle-seconds", "a0.5", settle.Seconds() );
		Obs1( "dynamics.tumble.sleep-seconds", "a0.75", sleepTick * kDynTick );
		Obs1( "dynamics.tumble.audible-impacts", "a2", (float)world.events.m_audibleImpacts );
		DestroyFeelWorld( world );
	}
}

// Tumbling drops from many orientations. One tumble is chaotic, so only
// the means are compared: how high the cube bounces after its first landing
// and how far it ends from where it fell. Every drop must end flat.
void TestTumbleStatistics( const CubeModel_t &model )
{
	const int kDrops = 12;
	float peakSum = 0.0f, travelSum = 0.0f;
	int flat = 0;
	for ( int k = 0; k < kDrops; k++ )
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		QAngle angles( ( k * 37 ) % 360, ( k * 71 ) % 360, ( k * 53 ) % 360 );
		Vector mins, maxs;
		s_pCollision->CollideGetAABB( &mins, &maxs, model.pCollide, vec3_origin, angles );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 0, 0, -mins.z + 64.0f ), angles );
		float peak = 0.0f;
		bool landed = false;
		for ( int i = 0; i < 240; i++ )
		{
			Tick( world.pEnv );
			landed |= world.events.m_impacts > 0;
			if ( landed )
			{
				// Height of the lowest point above the floor after landing.
				Vector pos;
				QAngle now;
				pCube->GetPosition( &pos, &now );
				Vector boxMins, boxMaxs;
				s_pCollision->CollideGetAABB( &boxMins, &boxMaxs, model.pCollide, pos, now );
				peak = MAX( peak, boxMins.z );
			}
		}
		Vector rest = PositionOf( pCube );
		peakSum += peak;
		travelSum += rest.AsVector2D().Length();
		flat += ( TiltFromFlat( pCube ) < 1.0f && Near( rest.z, RestHeight( model ), 0.75f ) ) ? 1 : 0;
		Trace( "DYN tumble-stats %d peak %.3f travel %.3f tilt %.3f\n", k, peak, rest.AsVector2D().Length(),
			TiltFromFlat( pCube ) );
		DestroyFeelWorld( world );
	}
	Check( TIER_GAMEPLAY, "dynamics.tumbles-end-flat", flat == kDrops, "%d of %d flat", flat, kDrops );
	Obs1( "dynamics.tumble-stats.peak-mean", "a1.5", peakSum / kDrops );
	Obs1( "dynamics.tumble-stats.travel-mean", "a8", travelSum / kDrops );
}

//-----------------------------------------------------------------------------
// Sliding on the floor after a shove, and pushing with a steady force.
//-----------------------------------------------------------------------------
IPhysicsObject *SettledCube( FeelWorld_t &world, const CubeModel_t &model, const Vector &xy )
{
	IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( xy.x, xy.y, RestHeight( model ) + 0.5f ), vec3_angle );
	for ( int i = 0; i < 30; i++ )
		Tick( world.pEnv );
	world.events.Reset();
	return pCube;
}

void TestSlide( const CubeModel_t &model )
{
	FeelWorld_t world;
	CreateFeelWorld( world );
	IPhysicsObject *pCube = SettledCube( world, model, vec3_origin );
	Vector start = PositionOf( pCube );
	Vector shove( 300, 0, 0 );
	pCube->Wake();
	pCube->SetVelocity( &shove, NULL );
	float speeds[3] = { 0, 0, 0 };
	float maxTilt = 0.0f, maxLift = 0.0f;
	int stopTick = -1;
	for ( int i = 0; i < 180; i++ )
	{
		Tick( world.pEnv );
		Vector velocity = VelocityOf( pCube );
		if ( i == 5 || i == 11 || i == 17 )
			speeds[i / 6] = velocity.x;
		maxTilt = MAX( maxTilt, TiltFromFlat( pCube ) );
		maxLift = MAX( maxLift, PositionOf( pCube ).z - start.z );
		if ( stopTick < 0 && velocity.Length() < 1.0f )
			stopTick = i;
		Trace( "DYN slide %d x %.3f vx %.3f tilt %.3f z %.3f\n", i, PositionOf( pCube ).x, velocity.x,
			TiltFromFlat( pCube ), PositionOf( pCube ).z );
	}
	Vector end = PositionOf( pCube );
	float distance = end.x - start.x;
	// Kinetic friction decelerates at mu*g; the slide can be no longer than
	// the frictionless bound and must stop.
	Check( TIER_GAMEPLAY, "dynamics.slide-stops", stopTick > 0 && distance > 1.0f, "stop tick %d distance %.3f",
		stopTick, distance );
	// IVP's contact-point friction curves a slide about 12 units sideways.
	Check( TIER_GAMEPLAY, "dynamics.slide-straight", fabsf( end.y - start.y ) < 20.0f, "sideways %.3f", end.y - start.y );
	// Scrape sounds come from friction events every tick of the slide.
	Check( TIER_GAMEPLAY, "dynamics.slide-scrapes", stopTick > 0 && world.events.m_frictionEvents >= stopTick * 3 / 4,
		"friction events %d over %d ticks", world.events.m_frictionEvents, stopTick );
	Check( TIER_GAMEPLAY, "dynamics.slide-stays-down", maxTilt < 10.0f && maxLift < 1.0f, "tilt %.3f lift %.3f",
		maxTilt, maxLift );
	Obs1( "dynamics.slide.distance", "r0.1", distance );
	Obs1( "dynamics.slide.stop-seconds", "a0.25", stopTick * kDynTick );
	ObsFloats( "dynamics.slide.speed-samples", "a12", 3, speeds );
	Obs1( "dynamics.slide.max-tilt", "a1", maxTilt );
	Trace( "DYN slide summary friction %d energy %.3f impacts %d\n", world.events.m_frictionEvents,
		world.events.m_frictionEnergy, world.events.m_impacts );
	DestroyFeelWorld( world );
}

// A steady horizontal force as a fraction of the cube's weight, applied as
// per-tick impulses the way game code pushes props.
void TestPush( const CubeModel_t &model )
{
	const float fractions[] = { 0.1f, 0.2f, 0.4f, 0.7f };
	float moved[ARRAYSIZE( fractions )];
	for ( int f = 0; f < ARRAYSIZE( fractions ); f++ )
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pCube = SettledCube( world, model, vec3_origin );
		Vector start = PositionOf( pCube );
		float impulse = fractions[f] * pCube->GetMass() * kGravity * kDynTick;
		for ( int i = 0; i < 60; i++ )
		{
			pCube->Wake();
			pCube->ApplyForceCenter( Vector( impulse, 0, 0 ) );
			Tick( world.pEnv );
		}
		moved[f] = PositionOf( pCube ).x - start.x;
		Trace( "DYN push %.2f moved %.3f vx %.3f tilt %.3f\n", fractions[f], moved[f], VelocityOf( pCube ).x,
			TiltFromFlat( pCube ) );
		DestroyFeelWorld( world );
	}
	// Below the friction limit IVP's friction springs let the cube give a
	// few units and pull back; above it the push must win.
	bool monotonic = moved[ARRAYSIZE( fractions ) - 1] > 10.0f;
	for ( int f = 1; f < ARRAYSIZE( fractions ); f++ )
		monotonic &= moved[f] >= moved[f - 1] - 5.0f;
	Check( TIER_GAMEPLAY, "dynamics.push-monotonic", monotonic, "moved %.2f %.2f %.2f %.2f", moved[0], moved[1],
		moved[2], moved[3] );
	ObsFloats( "dynamics.push.moved", "a5", ARRAYSIZE( moved ), moved );
}

//-----------------------------------------------------------------------------
// Cube on a ramp: holds on a shallow one, slides down a steep one.
//-----------------------------------------------------------------------------
void TestSlope( const CubeModel_t &model )
{
	const float pitches[] = { 5.0f, 10.0f, 20.0f, 35.0f };
	float travel[ARRAYSIZE( pitches )];
	float tilt[ARRAYSIZE( pitches )];
	for ( int p = 0; p < ARRAYSIZE( pitches ); p++ )
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		// A ramp slab rising toward -x; its top plane passes through the
		// ramp origin at (0, 0, 256).
		QAngle rampAngles( pitches[p], 0, 0 );
		Vector rampOrigin( 0, 0, 256 );
		CreateStaticBox( world, Vector( -512, -256, -32 ), Vector( 512, 256, 0 ), rampOrigin, rampAngles );
		matrix3x4_t ramp;
		AngleMatrix( rampAngles, rampOrigin, ramp );
		Vector up, cubePos;
		MatrixGetColumn( ramp, 2, up );
		cubePos = rampOrigin + up * ( RestHeight( model ) + 0.25f );
		IPhysicsObject *pCube = CreateFeelCube( world, model, cubePos, rampAngles );
		for ( int i = 0; i < 90; i++ )
		{
			Tick( world.pEnv );
			if ( TraceEnabled() && ( i % 10 ) == 0 )
				Trace( "DYN slope %.0f %d pos %.3f %.3f %.3f\n", pitches[p], i, PositionOf( pCube ).x,
					PositionOf( pCube ).y, PositionOf( pCube ).z );
		}
		travel[p] = ( PositionOf( pCube ) - cubePos ).Length();
		// Tilt relative to the ramp: the cube's up axis against the ramp's.
		tilt[p] = RAD2DEG( acosf( clamp( fabsf( DotProduct( AxisOf( pCube, 2 ), up ) ), 0.0f, 1.0f ) ) );
		DestroyFeelWorld( world );
	}
	// 5 degrees is far inside any friction cone on concrete: the cube stays.
	Check( TIER_GAMEPLAY, "dynamics.slope-shallow-holds", travel[0] < 2.0f, "travel %.3f", travel[0] );
	bool steeperSlidesFarther = true;
	for ( int p = 1; p < ARRAYSIZE( pitches ); p++ )
		steeperSlidesFarther &= travel[p] >= travel[p - 1] - 0.5f;
	Check( TIER_GAMEPLAY, "dynamics.slope-monotonic", steeperSlidesFarther, "travel %.2f %.2f %.2f %.2f", travel[0],
		travel[1], travel[2], travel[3] );
	Check( TIER_GAMEPLAY, "dynamics.slope-stays-on-face", tilt[0] < 2.0f && tilt[1] < 2.0f && tilt[2] < 2.0f,
		"tilt %.2f %.2f %.2f", tilt[0], tilt[1], tilt[2] );
	ObsFloats( "dynamics.slope.travel", "a10", ARRAYSIZE( travel ), travel );
	ObsFloats( "dynamics.slope.tilt", "a3", ARRAYSIZE( tilt ), tilt );
}

//-----------------------------------------------------------------------------
// A three-cube stack settles, stays aligned and sleeps.
//-----------------------------------------------------------------------------
void TestStack( const CubeModel_t &model )
{
	FeelWorld_t world;
	CreateFeelWorld( world );
	float height = model.maxs.z - model.mins.z;
	IPhysicsObject *pCubes[3];
	for ( int i = 0; i < 3; i++ )
		pCubes[i] = CreateFeelCube( world, model, Vector( 0, 0, RestHeight( model ) + 0.5f + i * ( height + 1.0f ) ), vec3_angle );
	int sleepTick = -1;
	float maxJitter = 0.0f;
	Vector last = PositionOf( pCubes[2] );
	for ( int i = 0; i < 480; i++ )
	{
		Tick( world.pEnv );
		Vector top = PositionOf( pCubes[2] );
		// Per-tick motion of the top cube once the stack has had a second.
		if ( i >= 60 )
			maxJitter = MAX( maxJitter, ( top - last ).Length() );
		last = top;
		bool allAsleep = pCubes[0]->IsAsleep() && pCubes[1]->IsAsleep() && pCubes[2]->IsAsleep();
		if ( sleepTick < 0 && allAsleep )
			sleepTick = i;
		Trace( "DYN stack %d top %.4f %.4f %.4f asleep %d%d%d\n", i, top.x, top.y, top.z, pCubes[0]->IsAsleep(),
			pCubes[1]->IsAsleep(), pCubes[2]->IsAsleep() );
	}
	Vector top = PositionOf( pCubes[2] );
	float drift = top.AsVector2D().Length();
	float expectedTop = RestHeight( model ) + 2.0f * height;
	// IVP's collision distance stacks up to about 1.5 units over three cubes.
	Check( TIER_GAMEPLAY, "dynamics.stack-stands", Near( top.z, expectedTop, 2.0f ) && drift < 2.0f,
		"top z %.3f (expect %.3f) drift %.3f", top.z, expectedTop, drift );
	Check( TIER_GAMEPLAY, "dynamics.stack-sleeps", sleepTick >= 0, "sleep tick %d", sleepTick );
	Check( TIER_GAMEPLAY, "dynamics.stack-still", maxJitter < 0.05f, "max per-tick motion %.4f", maxJitter );
	Obs1( "dynamics.stack.top-z", "a2", top.z );
	Obs1( "dynamics.stack.drift", "a1", drift );
	Obs1( "dynamics.stack.sleep-seconds", "a1", sleepTick * kDynTick );
	DestroyFeelWorld( world );
}

//-----------------------------------------------------------------------------
// The Portal 2 grab controller (game/shared/portal2/portal_grabcontroller_shared.cpp):
// the held cube's mass is 1 and its rotational damping 10, drag is off; each
// step the controller drives it with ComputeShadowControl, then slides the
// result along contacts with static or heavy objects (PhysComputeSlideDirection)
// and sets it with SetVelocityInstantaneous.
//-----------------------------------------------------------------------------
class CGrabEmulation : public IMotionEvent
{
public:
	CGrabEmulation() : m_timeToArrive( 0.0f ), m_loadWeight( 40.0f )
	{
		memset( &m_shadow, 0, sizeof( m_shadow ) );
		m_shadow.dampFactor = 1.0f;
		m_shadow.teleportDistance = 0.0f;
		m_shadow.maxSpeed = 1000.0f;
		m_shadow.maxAngular = 3600.0f;
		m_shadow.maxDampSpeed = m_shadow.maxSpeed * 2.0f;
		m_shadow.maxDampAngular = m_shadow.maxAngular;
		m_contactAmount = 0.0f;
	}

	// CGrabController::SetTargetPosition, called by the player each tick.
	void SetTarget( const Vector &position, const QAngle &angles )
	{
		m_shadow.targetPosition = position;
		m_shadow.targetRotation = angles;
		m_timeToArrive = kDynTick;
	}

	static bool InContactWithHeavyObject( IPhysicsObject *pObject, float heavyMass )
	{
		bool contact = false;
		IPhysicsFrictionSnapshot *pSnapshot = pObject->CreateFrictionSnapshot();
		while ( pSnapshot->IsValid() )
		{
			IPhysicsObject *pOther = pSnapshot->GetObject( 1 );
			if ( !pOther->IsMoveable() || pOther->GetMass() > heavyMass )
			{
				contact = true;
				break;
			}
			pSnapshot->NextFrictionData();
		}
		pObject->DestroyFrictionSnapshot( pSnapshot );
		return contact;
	}

	// PhysComputeSlideDirection (game/shared/physics_shared.cpp).
	static void SlideDirection( IPhysicsObject *pPhysics, Vector &velocity, AngularImpulse &angVel, float minMass )
	{
		IPhysicsFrictionSnapshot *pSnapshot = pPhysics->CreateFrictionSnapshot();
		while ( pSnapshot->IsValid() )
		{
			IPhysicsObject *pOther = pSnapshot->GetObject( 1 );
			if ( !pOther->IsMoveable() || pOther->GetMass() > minMass )
			{
				Vector normal;
				pSnapshot->GetSurfaceNormal( normal );
				angVel = normal * DotProduct( angVel, normal );
				float proj = DotProduct( velocity, normal );
				if ( proj > 0.0f )
					velocity -= normal * proj;
			}
			pSnapshot->NextFrictionData();
		}
		pPhysics->DestroyFrictionSnapshot( pSnapshot );
	}

	virtual simresult_e Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime,
		Vector &linear, AngularImpulse &angular )
	{
		hlshadowcontrol_params_t params = m_shadow;
		float target = InContactWithHeavyObject( pObject, m_loadWeight ) ? 0.1f : 1.0f;
		m_contactAmount = target > m_contactAmount ? MIN( target, m_contactAmount + deltaTime * 2.0f )
												   : MAX( target, m_contactAmount - deltaTime * 2.0f );
		params.maxAngular = m_shadow.maxAngular * m_contactAmount * m_contactAmount * m_contactAmount;
		m_timeToArrive = pObject->ComputeShadowControl( params, m_timeToArrive, deltaTime );

		Vector velocity;
		AngularImpulse angVel;
		pObject->GetVelocity( &velocity, &angVel );
		SlideDirection( pObject, velocity, angVel, m_loadWeight );
		pObject->SetVelocityInstantaneous( &velocity, NULL );
		linear.Init();
		angular.Init();
		return SIM_LOCAL_ACCELERATION;
	}

	hlshadowcontrol_params_t m_shadow;
	float m_timeToArrive;
	float m_loadWeight;
	float m_contactAmount;
};

struct HeldCube_t
{
	IPhysicsObject *pCube;
	IPhysicsMotionController *pController;
	CGrabEmulation grab;
};

void Grab( FeelWorld_t &world, HeldCube_t &held, IPhysicsObject *pCube )
{
	held.pCube = pCube;
	held.grab.m_loadWeight = pCube->GetMass();
	held.pController = world.pEnv->CreateMotionController( &held.grab );
	held.pController->AttachObject( pCube, true );
	pCube->Wake();
	float damping = 10.0f;
	pCube->SetMass( 1.0f );
	pCube->SetDamping( NULL, &damping );
	pCube->EnableDrag( false );
	Vector position;
	QAngle angles;
	pCube->GetPosition( &position, &angles );
	held.grab.SetTarget( position, angles );
}

void TestHeld( const CubeModel_t &model )
{
	// Held still in the air: the cube converges on the target and stays.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 0, 0, 100 ), vec3_angle );
		HeldCube_t held;
		Grab( world, held, pCube );
		const Vector target( 20, 0, 110 );
		const QAngle targetAngles( 0, 30, 0 );
		float maxLateError = 0.0f, maxLateAngle = 0.0f;
		int arriveTick = -1;
		for ( int i = 0; i < 120; i++ )
		{
			held.grab.SetTarget( target, targetAngles );
			Tick( world.pEnv );
			Vector pos;
			QAngle angles;
			pCube->GetPosition( &pos, &angles );
			float error = ( pos - target ).Length();
			if ( arriveTick < 0 && error < 0.5f )
				arriveTick = i;
			if ( i >= 60 )
			{
				maxLateError = MAX( maxLateError, error );
				maxLateAngle = MAX( maxLateAngle, fabsf( AngleDiff( angles.y, targetAngles.y ) ) );
			}
			Trace( "DYN held %d pos %.3f %.3f %.3f yaw %.3f\n", i, pos.x, pos.y, pos.z, angles.y );
		}
		Check( TIER_GAMEPLAY, "dynamics.held-arrives", arriveTick >= 0 && arriveTick < 30, "arrive tick %d", arriveTick );
		Check( TIER_GAMEPLAY, "dynamics.held-steady", maxLateError < 0.25f && maxLateAngle < 1.0f,
			"late error %.3f angle %.3f", maxLateError, maxLateAngle );
		// IVP applies gravity before the grab controller runs, so the
		// controller cancels it and the held cube does not sag.
		Obs1( "dynamics.held.arrive-tick", "a2", (float)arriveTick );
		Obs1( "dynamics.held.late-error", "a0.05", maxLateError );
		world.pEnv->DestroyMotionController( held.pController );
		DestroyFeelWorld( world );
	}

	// Carried while the player walks and turns: the target sweeps a circle
	// at walking speed; the cube lags a steady distance behind it.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 64, 0, 100 ), vec3_angle );
		HeldCube_t held;
		Grab( world, held, pCube );
		const float radius = 64.0f, speed = 175.0f;
		float lagSum = 0.0f, lagMax = 0.0f;
		int lagSamples = 0;
		for ( int i = 0; i < 180; i++ )
		{
			float angle = speed / radius * i * kDynTick;
			Vector target( radius * cosf( angle ), radius * sinf( angle ), 100 );
			held.grab.SetTarget( target, QAngle( 0, RAD2DEG( angle ), 0 ) );
			Tick( world.pEnv );
			float lag = ( PositionOf( pCube ) - target ).Length();
			if ( i >= 30 )
			{
				lagSum += lag;
				lagMax = MAX( lagMax, lag );
				lagSamples++;
			}
		}
		float lagMean = lagSum / MAX( lagSamples, 1 );
		Check( TIER_GAMEPLAY, "dynamics.held-follows", lagMax < 12.0f, "lag mean %.3f max %.3f", lagMean, lagMax );
		Obs1( "dynamics.held.carry-lag-mean", "a0.05", lagMean );
		Obs1( "dynamics.held.carry-lag-max", "a0.1", lagMax );
		world.pEnv->DestroyMotionController( held.pController );
		DestroyFeelWorld( world );
	}

	// Held into a wall: the target sits 12 units inside the wall; the cube
	// must rest against it without buzzing (the grab controller's contact
	// sliding and the provider's contact solve work against each other).
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		CreateStaticBox( world, Vector( 0, -256, 0 ), Vector( 32, 256, 256 ), Vector( 64, 0, 0 ), vec3_angle );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 20, 0, 100 ), vec3_angle );
		HeldCube_t held;
		Grab( world, held, pCube );
		const float faceX = 64.0f - model.maxs.x;
		const Vector target( faceX + 12.0f, 0, 100 );
		float minX = 1e9f, maxX = -1e9f, sumSpeed = 0.0f, maxAngular = 0.0f;
		int samples = 0;
		Vector last = PositionOf( pCube );
		float travel = 0.0f;
		for ( int i = 0; i < 180; i++ )
		{
			held.grab.SetTarget( target, vec3_angle );
			Tick( world.pEnv );
			Vector pos = PositionOf( pCube );
			if ( i >= 60 )
			{
				minX = MIN( minX, pos.x );
				maxX = MAX( maxX, pos.x );
				sumSpeed += SpeedOf( pCube );
				maxAngular = MAX( maxAngular, AngularSpeedOf( pCube ) );
				travel += ( pos - last ).Length();
				samples++;
			}
			last = pos;
			Trace( "DYN wall-hold %d pos %.4f %.4f %.4f v %.3f w %.3f\n", i, pos.x, pos.y, pos.z, SpeedOf( pCube ),
				AngularSpeedOf( pCube ) );
		}
		Vector rest = PositionOf( pCube );
		float gap = faceX - rest.x;
		// Path length over the last two seconds: a cube resting on the wall
		// barely moves; a buzzing one covers ground every tick. IVP buzzes
		// (about 70 units and 340 deg/s); the bounds catch anything worse.
		// Both hold the cube about 2.3 units off the wall: the controller's
		// contact sliding removes the approach once the cube touches it.
		Check( TIER_GAMEPLAY, "dynamics.held-wall-contact", gap > -1.0f && gap < 4.0f, "gap %.3f", gap );
		Check( TIER_GAMEPLAY, "dynamics.held-wall-quiet", travel < 150.0f && maxAngular < 720.0f,
			"travel %.3f spread %.3f angular %.3f mean speed %.3f", travel, maxX - minX, maxAngular,
			sumSpeed / MAX( samples, 1 ) );
		Obs1( "dynamics.held.wall-gap", "a0.5", gap );
		world.pEnv->DestroyMotionController( held.pController );
		DestroyFeelWorld( world );
	}

	// Held down onto the floor: the target is 8 units below the floor.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 0, 0, 60 ), vec3_angle );
		HeldCube_t held;
		Grab( world, held, pCube );
		const Vector target( 0, 0, RestHeight( model ) - 8.0f );
		float travel = 0.0f, maxAngular = 0.0f;
		Vector last = PositionOf( pCube );
		for ( int i = 0; i < 180; i++ )
		{
			held.grab.SetTarget( target, vec3_angle );
			Tick( world.pEnv );
			Vector pos = PositionOf( pCube );
			if ( i >= 60 )
			{
				travel += ( pos - last ).Length();
				maxAngular = MAX( maxAngular, AngularSpeedOf( pCube ) );
			}
			last = pos;
			Trace( "DYN floor-hold %d pos %.4f %.4f %.4f w %.3f\n", i, pos.x, pos.y, pos.z, AngularSpeedOf( pCube ) );
		}
		float gap = PositionOf( pCube ).z - RestHeight( model );
		// IVP holds the cube about 1.5 units up and buzzes (about 9 units,
		// 100 deg/s); the bounds catch anything worse.
		Check( TIER_GAMEPLAY, "dynamics.held-floor-contact", gap > -1.0f && gap < 3.0f, "gap %.3f", gap );
		Check( TIER_GAMEPLAY, "dynamics.held-floor-quiet", travel < 30.0f && maxAngular < 360.0f,
			"travel %.3f angular %.3f", travel, maxAngular );
		world.pEnv->DestroyMotionController( held.pController );
		DestroyFeelWorld( world );
	}

	// Dropped after carrying: released at walking speed, the cube keeps the
	// velocity the controller gave it and gets its mass and damping back.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 0, 0, 100 ), vec3_angle );
		float savedMass = pCube->GetMass();
		float savedRot = 0.0f;
		pCube->GetDamping( NULL, &savedRot );
		HeldCube_t held;
		Grab( world, held, pCube );
		for ( int i = 0; i < 60; i++ )
		{
			held.grab.SetTarget( Vector( i * 175.0f * kDynTick, 0, 100 ), vec3_angle );
			Tick( world.pEnv );
		}
		Vector releaseVelocity = VelocityOf( pCube );
		world.pEnv->DestroyMotionController( held.pController );
		pCube->SetMass( savedMass );
		pCube->SetDamping( NULL, &savedRot );
		for ( int i = 0; i < 6; i++ )
			Tick( world.pEnv );
		Vector after = VelocityOf( pCube );
		Check( TIER_GAMEPLAY, "dynamics.held-release-keeps-speed", Near( releaseVelocity.x, 175.0f, 20.0f ) &&
			after.x > releaseVelocity.x * 0.8f, "release %.3f after %.3f", releaseVelocity.x, after.x );
		Obs1( "dynamics.held.release-vx", "a10", releaseVelocity.x );
		Obs1( "dynamics.held.after-release-vx", "a10", after.x );
		DestroyFeelWorld( world );
	}
}

//-----------------------------------------------------------------------------
// Thrown into a wall, flung through the speed limit, and spun on the floor.
//-----------------------------------------------------------------------------
void TestImpacts( const CubeModel_t &model )
{
	// Head-on into a wall without gravity: rebound speed from restitution.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		world.pEnv->SetGravity( vec3_origin );
		CreateStaticBox( world, Vector( 0, -256, 0 ), Vector( 32, 256, 512 ), Vector( 128, 0, 0 ), vec3_angle );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 0, 0, 200 ), vec3_angle );
		Vector velocity( 600, 0, 0 );
		pCube->SetVelocity( &velocity, NULL );
		float maxPenetration = 0.0f;
		const float faceX = 128.0f - model.maxs.x;
		for ( int i = 0; i < 60; i++ )
		{
			Tick( world.pEnv );
			maxPenetration = MAX( maxPenetration, PositionOf( pCube ).x - faceX );
		}
		Vector after = VelocityOf( pCube );
		Check( TIER_GAMEPLAY, "dynamics.wall-rebounds", after.x < 0.0f && after.x > -600.0f, "vx %.3f", after.x );
		Check( TIER_GAMEPLAY, "dynamics.wall-no-tunnel", maxPenetration < 2.0f, "penetration %.3f", maxPenetration );
		// IVP resolves a flat hit corner by corner and sets the cube
		// spinning (about 100 deg/s); Box3D keeps it square.
		Check( TIER_GAMEPLAY, "dynamics.wall-straight", AngularSpeedOf( pCube ) < 360.0f && fabsf( after.y ) + fabsf( after.z ) < 60.0f,
			"angular %.3f vyz %.3f %.3f", AngularSpeedOf( pCube ), after.y, after.z );
		// Rebound: IVP gives back speed * sqrt(elasticity product).
		Obs1( "dynamics.wall.rebound-vx", "a35", after.x );
		// Box3D's soft contact lets a 600 in/s hit sink about 0.9 units.
		Obs1( "dynamics.wall.penetration", "a1.5", maxPenetration );
		Obs1( "dynamics.wall.first-impact-speed", "r0.05", world.events.m_firstImpactSpeed );
		DestroyFeelWorld( world );
	}

	// Flung faster than the world limit (maxVelocity) toward a thin panel:
	// clamped speed, and no tunneling through 4 units of wall.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		world.pEnv->SetGravity( vec3_origin );
		CreateStaticBox( world, Vector( 0, -256, 0 ), Vector( 4, 256, 512 ), Vector( 512, 0, 0 ), vec3_angle );
		IPhysicsObject *pCube = CreateFeelCube( world, model, Vector( 0, 0, 200 ), vec3_angle );
		Vector velocity( 3000, 0, 0 );
		pCube->SetVelocity( &velocity, NULL );
		Tick( world.pEnv );
		float clamped = VelocityOf( pCube ).x;
		float maxX = PositionOf( pCube ).x;
		for ( int i = 0; i < 60; i++ )
		{
			Tick( world.pEnv );
			maxX = MAX( maxX, PositionOf( pCube ).x );
		}
		float face = 512.0f - model.maxs.x;
		Check( TIER_GAMEPLAY, "dynamics.fling-clamped", clamped <= k_flMaxVelocity + 1.0f, "vx %.3f", clamped );
		Check( TIER_GAMEPLAY, "dynamics.fling-no-tunnel", maxX < face + 2.0f, "max x %.3f face %.3f", maxX, face );
		Obs1( "dynamics.fling.first-tick-vx", "a25", clamped );
		DestroyFeelWorld( world );
	}

	// Spun about the vertical axis on the floor: friction stops it.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pCube = SettledCube( world, model, vec3_origin );
		AngularImpulse spin( 0, 0, 720 );
		pCube->SetVelocity( NULL, &spin );
		int stopTick = -1;
		float spun = 0.0f;
		for ( int i = 0; i < 240; i++ )
		{
			Tick( world.pEnv );
			float w = AngularVelocityOf( pCube ).z;
			spun += w * kDynTick;
			if ( stopTick < 0 && fabsf( w ) < 5.0f )
				stopTick = i;
			Trace( "DYN spin %d wz %.3f tilt %.3f\n", i, w, TiltFromFlat( pCube ) );
		}
		Check( TIER_GAMEPLAY, "dynamics.spin-stops", stopTick > 0, "stop tick %d", stopTick );
		Check( TIER_GAMEPLAY, "dynamics.spin-stays-flat", TiltFromFlat( pCube ) < 1.0f, "tilt %.3f", TiltFromFlat( pCube ) );
		Obs1( "dynamics.spin.stop-seconds", "a0.15", stopTick * kDynTick );
		Obs1( "dynamics.spin.degrees", "r0.2", spun );
		DestroyFeelWorld( world );
	}

	// Dropped onto a resting cube: the lower one is not driven into the floor.
	{
		FeelWorld_t world;
		CreateFeelWorld( world );
		IPhysicsObject *pLower = SettledCube( world, model, vec3_origin );
		float height = model.maxs.z - model.mins.z;
		IPhysicsObject *pUpper = CreateFeelCube( world, model, Vector( 4, 0, RestHeight( model ) + height + 96.0f ), vec3_angle );
		float minLowerZ = PositionOf( pLower ).z;
		for ( int i = 0; i < 240; i++ )
		{
			Tick( world.pEnv );
			minLowerZ = MIN( minLowerZ, PositionOf( pLower ).z );
		}
		float sink = RestHeight( model ) - minLowerZ;
		float upperZ = PositionOf( pUpper ).z;
		Check( TIER_GAMEPLAY, "dynamics.cube-on-cube-no-sink", sink < 1.0f, "sink %.3f", sink );
		Obs1( "dynamics.cube-on-cube.sink", "a0.6", sink );
		Obs1( "dynamics.cube-on-cube.upper-z", "a2", upperZ );
		DestroyFeelWorld( world );
	}
}

//-----------------------------------------------------------------------------
// The player walks into the cube and keeps walking. Game movement stops the
// player at the cube; CBasePlayer::PostThinkVPhysics then leads the physics
// shadow's target by half of the blocked step (m_touchedPhysObject), and the
// player controller (SetupVPhysicsShadow: 85 kg "player" hull, push limits
// 350 kg and 50 in/s) pushes the cube with it. Movement is emulated along x:
// the player origin advances at walking speed until it meets the cube's face.
//-----------------------------------------------------------------------------
void TestPlayerPush( const CubeModel_t &model )
{
	FeelWorld_t world;
	CreateFeelWorld( world );
	IPhysicsObject *pCube = SettledCube( world, model, Vector( 0, 0, 0 ) );
	CPhysCollide *pHull = s_pCollision->BBoxToCollide( Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
	world.collides.AddToTail( pHull );
	objectparams_t params = DefaultParams( 85.0f, NULL );
	params.inertia = 1e24f;
	params.dragCoefficient = 0;
	int playerMaterial = s_pProps->GetSurfaceIndex( "player" );
	Vector origin( model.mins.x - 16.0f - 24.0f, 0, 0 );
	IPhysicsObject *pPlayer = world.pEnv->CreatePolyObject( pHull, playerMaterial >= 0 ? playerMaterial : 0, origin, vec3_angle, &params );
	pPlayer->SetCallbackFlags( CALLBACK_GLOBAL_COLLISION | CALLBACK_SHADOW_COLLISION );
	IPhysicsPlayerController *pController = world.pEnv->CreatePlayerController( pPlayer );
	pController->SetPushMassLimit( 350.0f );
	pController->SetPushSpeedLimit( 50.0f );
	Vector cubeStart = PositionOf( pCube );
	const Vector wishVel( 175, 0, 0 );
	const float kDistEpsilon = 0.03125f;
	int contactTick = -1, blockedTicks = 0;
	float speedSum = 0.0f, maxTilt = 0.0f;
	int speedSamples = 0;
	for ( int i = 0; i < 120; i++ )
	{
		// Game movement: walk until the player's box meets the cube.
		Vector cubeMins, cubeMaxs, cubePos;
		QAngle cubeAngles;
		pCube->GetPosition( &cubePos, &cubeAngles );
		s_pCollision->CollideGetAABB( &cubeMins, &cubeMaxs, model.pCollide, cubePos, cubeAngles );
		Vector old = origin;
		float wanted = old.x + wishVel.x * kDynTick;
		float limit = cubeMins.x - 16.0f - kDistEpsilon;
		bool touched = wanted > limit;
		origin.x = touched ? MAX( old.x, limit ) : wanted;
		Vector target = origin;
		if ( touched )
		{
			// PostThinkVPhysics: halfway to where the player wanted to be.
			target = origin * 0.5f + ( old + wishVel * kDynTick ) * 0.5f;
			blockedTicks++;
		}
		if ( !FaultIs( "player-inert" ) )
			pController->Update( target, wishVel, kDynTick, true, NULL );
		Tick( world.pEnv );
		if ( contactTick < 0 && PositionOf( pCube ).x > cubeStart.x + 0.5f )
			contactTick = i;
		if ( contactTick >= 0 && i >= contactTick + 30 )
		{
			speedSum += VelocityOf( pCube ).x;
			speedSamples++;
		}
		maxTilt = MAX( maxTilt, TiltFromFlat( pCube ) );
		Trace( "DYN player-push %d origin %.3f shadow %.3f z %.3f cube %.3f vx %.3f tilt %.3f shadow-v %.3f %.3f\n", i,
			origin.x, PositionOf( pPlayer ).x, PositionOf( pPlayer ).z, PositionOf( pCube ).x, VelocityOf( pCube ).x,
			TiltFromFlat( pCube ), VelocityOf( pPlayer ).x, VelocityOf( pPlayer ).z );
	}
	float pushed = PositionOf( pCube ).x - cubeStart.x;
	float cubeSpeed = speedSum / MAX( speedSamples, 1 );
	Check( TIER_GAMEPLAY, "dynamics.player-push-moves", contactTick >= 0 && pushed > 16.0f, "contact tick %d pushed %.3f",
		contactTick, pushed );
	Check( TIER_GAMEPLAY, "dynamics.player-push-upright", maxTilt < 10.0f, "max tilt %.3f", maxTilt );
	Obs1( "dynamics.player-push.distance", "a8", pushed );
	Obs1( "dynamics.player-push.cube-speed", "a4", cubeSpeed );
	Obs1( "dynamics.player-push.blocked-ticks", "a10", (float)blockedTicks );
	world.pEnv->DestroyPlayerController( pController );
	DestroyFeelWorld( world );
}

// The player walks across the top of the cube. The cube is lighter than
// twice the player, so it is not rideable ground (CBasePlayer::
// IsRideablePhysics) and the controller gets no ground object; the player's
// feet must not drag the cube along.
void TestPlayerWalksOnCube( const CubeModel_t &model )
{
	FeelWorld_t world;
	CreateFeelWorld( world );
	IPhysicsObject *pCube = SettledCube( world, model, Vector( 0, 0, 0 ) );
	CPhysCollide *pHull = s_pCollision->BBoxToCollide( Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
	world.collides.AddToTail( pHull );
	objectparams_t params = DefaultParams( 85.0f, NULL );
	params.inertia = 1e24f;
	params.dragCoefficient = 0;
	int playerMaterial = s_pProps->GetSurfaceIndex( "player" );
	float top = PositionOf( pCube ).z + model.maxs.z;
	Vector origin( -12, 0, top );
	IPhysicsObject *pPlayer = world.pEnv->CreatePolyObject( pHull, playerMaterial >= 0 ? playerMaterial : 0, origin, vec3_angle, &params );
	pPlayer->SetCallbackFlags( CALLBACK_GLOBAL_COLLISION | CALLBACK_SHADOW_COLLISION );
	IPhysicsPlayerController *pController = world.pEnv->CreatePlayerController( pPlayer );
	pController->SetPushMassLimit( 350.0f );
	pController->SetPushSpeedLimit( 50.0f );
	// Stand still for a moment, then walk 24 units across the top.
	for ( int i = 0; i < 20; i++ )
	{
		if ( !FaultIs( "player-inert" ) )
			pController->Update( origin, vec3_origin, kDynTick, true, NULL );
		Tick( world.pEnv );
	}
	Vector cubeStart = PositionOf( pCube );
	const Vector walk( 150, 0, 0 );
	float maxCubeSpeed = 0.0f;
	for ( int i = 0; i < 10; i++ )
	{
		origin += walk * kDynTick;
		if ( !FaultIs( "player-inert" ) )
			pController->Update( origin, walk, kDynTick, true, NULL );
		Tick( world.pEnv );
		maxCubeSpeed = MAX( maxCubeSpeed, VelocityOf( pCube ).AsVector2D().Length() );
		Trace( "DYN walk-on-cube %d player %.3f %.3f cube %.3f vx %.3f\n", i, PositionOf( pPlayer ).x, PositionOf( pPlayer ).z,
			PositionOf( pCube ).x, VelocityOf( pCube ).x );
	}
	float dragged = ( PositionOf( pCube ) - cubeStart ).AsVector2D().Length();
	float playerZ = PositionOf( pPlayer ).z;
	Check( TIER_GAMEPLAY, "dynamics.walk-on-cube-supports", playerZ > top - 2.0f, "player z %.3f top %.3f", playerZ, top );
	// IVP's shadow hops on its collision distance and jostles the cube about
	// 3 units (up to 40 in/s); Box3D leaves it in place. Either is fine;
	// dragging it along with the player is not.
	Check( TIER_GAMEPLAY, "dynamics.walk-on-cube-not-dragged", dragged < 8.0f, "dragged %.3f max speed %.3f", dragged,
		maxCubeSpeed );
	world.pEnv->DestroyPlayerController( pController );
	DestroyFeelWorld( world );
}
}

void TestDynamics( const vcollide_t *pCubeFixture )
{
	CubeModel_t model = LoadCubeModel( pCubeFixture );
	if ( !Check( TIER_GAMEPLAY, "dynamics.cube-model", model.valid, "mass %.2f surfaceprop %s", model.params.mass,
			 model.surfaceprop ) )
		return;
	Obs1( "dynamics.cube-model.rest-height", "a0.01", RestHeight( model ) );
	TestDrop( model );
	TestTumbleStatistics( model );
	TestSlide( model );
	TestPush( model );
	TestSlope( model );
	TestStack( model );
	TestHeld( model );
	TestImpacts( model );
	TestPlayerPush( model );
	TestPlayerWalksOnCube( model );
}
