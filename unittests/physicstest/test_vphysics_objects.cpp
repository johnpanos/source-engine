//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics provider conformance: object properties (mass and
//          inertia bounds, damping, drag, energy, force/velocity helpers,
//          hinging, collision/motion toggles, sleep), contact/touch/friction/
//          trigger events, and object lifetime (deletion during simulation,
//          the delete queue, transfer between environments, serialization).
//          Contract record: unittests/physicstest/contracts/vphysics.provider.v1.md
//
//=============================================================================//

#include <math.h>
#include <string.h>

#include "vphysics_conformance.h"
#include "vphysics/friction.h"
#include "tier1/utlvector.h"
#include "bspflags.h"
#include "vphysics/stats.h"

namespace
{
//-----------------------------------------------------------------------------
// Event recorder
//-----------------------------------------------------------------------------
class CEventLog : public IPhysicsCollisionEvent, public IPhysicsObjectEvent
{
public:
	CEventLog() { Reset(); }
	void Reset()
	{
		m_pre = m_post = m_friction = m_startTouch = m_endTouch = 0;
		m_enterTrigger = m_leaveTrigger = m_fluidStart = m_fluidEnd = 0;
		m_wake = m_sleep = m_postFrame = 0;
		m_frictionEnergy = 0.0f;
		m_pTouchA = m_pTouchB = NULL;
		m_pTrigger = m_pTriggered = NULL;
		m_pDeleteInCallback = NULL;
		m_pEnv = NULL;
		m_inSimulationDuringCallback = false;
		m_touchPairs = 0;
		m_pairStarts = 0;
	}

	// IPhysicsCollisionEvent
	virtual void PreCollision( vcollisionevent_t *pEvent )
	{
		m_pre++;
		if ( m_pEnv )
			m_inSimulationDuringCallback |= m_pEnv->IsInSimulation();
	}
	virtual void PostCollision( vcollisionevent_t *pEvent )
	{
		m_post++;
		// Game code deletes entities from impact callbacks (breakables).
		if ( m_pDeleteInCallback && m_pEnv &&
			( pEvent->pObjects[0] == m_pDeleteInCallback || pEvent->pObjects[1] == m_pDeleteInCallback ) )
		{
			m_pEnv->DestroyObject( m_pDeleteInCallback );
			m_pDeleteInCallback = NULL;
		}
	}
	// Sensitivity: "events-silent" models a provider that never reports
	// touch, friction, trigger or wake events.
	static bool Silent() { return FaultIs( "events-silent" ); }

	virtual void Friction( IPhysicsObject *pObject, float energy, int surfaceProps, int surfacePropsHit, IPhysicsCollisionData *pData )
	{
		if ( Silent() )
			return;
		m_friction++;
		m_frictionEnergy += energy;
	}
	virtual void StartTouch( IPhysicsObject *pObject1, IPhysicsObject *pObject2, IPhysicsCollisionData *pTouchData )
	{
		if ( Silent() )
			return;
		m_startTouch++;
		if ( IsWatchedPair( pObject1, pObject2 ) )
		{
			m_touchPairs++;
			m_pairStarts++;
		}
	}
	virtual void EndTouch( IPhysicsObject *pObject1, IPhysicsObject *pObject2, IPhysicsCollisionData *pTouchData )
	{
		if ( Silent() )
			return;
		m_endTouch++;
		if ( IsWatchedPair( pObject1, pObject2 ) )
			m_touchPairs--;
	}
	virtual void FluidStartTouch( IPhysicsObject *pObject, IPhysicsFluidController *pFluid ) { m_fluidStart++; }
	virtual void FluidEndTouch( IPhysicsObject *pObject, IPhysicsFluidController *pFluid ) { m_fluidEnd++; }
	virtual void PostSimulationFrame() { m_postFrame++; }
	virtual void ObjectEnterTrigger( IPhysicsObject *pTrigger, IPhysicsObject *pObject )
	{
		if ( Silent() )
			return;
		m_enterTrigger++;
		m_pTrigger = pTrigger;
		m_pTriggered = pObject;
	}
	virtual void ObjectLeaveTrigger( IPhysicsObject *pTrigger, IPhysicsObject *pObject )
	{
		if ( !Silent() )
			m_leaveTrigger++;
	}

	// IPhysicsObjectEvent
	virtual void ObjectWake( IPhysicsObject *pObject )
	{
		if ( !Silent() )
			m_wake++;
	}
	virtual void ObjectSleep( IPhysicsObject *pObject ) { m_sleep++; }

	bool IsWatchedPair( IPhysicsObject *p0, IPhysicsObject *p1 ) const
	{
		return ( p0 == m_pTouchA && p1 == m_pTouchB ) || ( p0 == m_pTouchB && p1 == m_pTouchA );
	}

	int m_pre, m_post, m_friction, m_startTouch, m_endTouch;
	int m_enterTrigger, m_leaveTrigger, m_fluidStart, m_fluidEnd;
	int m_wake, m_sleep, m_postFrame;
	int m_touchPairs;
	int m_pairStarts;
	float m_frictionEnergy;
	IPhysicsObject *m_pTouchA, *m_pTouchB;
	IPhysicsObject *m_pTrigger, *m_pTriggered;
	IPhysicsObject *m_pDeleteInCallback;
	IPhysicsEnvironment *m_pEnv;
	bool m_inSimulationDuringCallback;
};

bool Listed( IPhysicsEnvironment *pEnv, IPhysicsObject *pObject )
{
	int count = 0;
	const IPhysicsObject **ppList = pEnv->GetObjectList( &count );
	for ( int i = 0; ppList && i < count; i++ )
	{
		if ( ppList[i] == pObject )
			return true;
	}
	return false;
}

int ObjectCount( IPhysicsEnvironment *pEnv )
{
	int count = 0;
	pEnv->GetObjectList( &count );
	return count;
}

//-----------------------------------------------------------------------------
// Mass, inertia, damping bounds and round trips
//-----------------------------------------------------------------------------
void TestMassProperties()
{
	World_t world;
	CreateWorld( world, NULL );
	IPhysicsEnvironment *pEnv = world.pEnv;

	objectparams_t params = DefaultParams( 0.001f, NULL );
	IPhysicsObject *pTiny = pEnv->CreatePolyObject( world.pCubeCollide, world.material, Vector( 0, 0, 100 ), vec3_angle, &params );
	params = DefaultParams( 1e9f, NULL );
	IPhysicsObject *pHuge = pEnv->CreatePolyObject( world.pCubeCollide, world.material, Vector( 100, 0, 100 ), vec3_angle, &params );
	Check( TIER_GAMEPLAY, "object.mass-create-clamp", pTiny && pHuge && Near( pTiny->GetMass(), VPHYSICS_MIN_MASS, 1e-3f ) &&
		Near( pHuge->GetMass(), VPHYSICS_MAX_MASS, 1.0f ), "tiny %.4f huge %.1f", pTiny ? pTiny->GetMass() : -1.0f, pHuge ? pHuge->GetMass() : -1.0f );

	IPhysicsObject *pCube = CreateCube( world, Vector( 200, 0, 100 ) );
	pCube->SetMass( 0.5f );
	float low = pCube->GetMass();
	pCube->SetMass( 1e9f );
	float high = pCube->GetMass();
	pCube->SetMass( 20.0f );
	Check( TIER_GAMEPLAY, "object.set-mass-clamp", Near( low, 1.0f, 1e-3f ) && Near( high, VPHYSICS_MAX_MASS, 1.0f ) && Near( pCube->GetMass(), 20.0f, 1e-3f ),
		"low %.3f high %.1f", low, high );

	// A pinned (motion-disabled) object keeps its mass setting and stays pinned.
	pCube->EnableMotion( false );
	pCube->SetMass( 30.0f );
	Check( TIER_GAMEPLAY, "object.set-mass-pinned", Near( pCube->GetMass(), 30.0f, 1e-3f ) && !pCube->IsMotionEnabled() && pCube->GetInvMass() == 0.0f,
		"mass %.2f motion %d inv %.4f", pCube->GetMass(), pCube->IsMotionEnabled(), pCube->GetInvMass() );
	pCube->EnableMotion( true );
	Check( TIER_GAMEPLAY, "object.inv-mass", Near( pCube->GetInvMass(), 1.0f / 30.0f, 1e-4f ), "inv %.5f", pCube->GetInvMass() );

	// Inertia: the authored cube's diagonal inertia, then a round trip.
	Vector inertia = pCube->GetInertia();
	ObsVector( "object.cube-inertia", "r0.1", inertia );
	Check( TIER_GAMEPLAY, "object.inertia-positive", inertia.x > 0 && inertia.y > 0 && inertia.z > 0, "(%.1f %.1f %.1f)", inertia.x, inertia.y, inertia.z );
	pCube->SetInertia( Vector( 1000, 2000, 3000 ) );
	Vector got = pCube->GetInertia();
	Vector inv = pCube->GetInvInertia();
	Check( TIER_GAMEPLAY, "object.inertia-roundtrip", NearVec( got, Vector( 1000, 2000, 3000 ), 1.0f ) &&
		Near( inv.x, 1e-3f, 1e-5f ) && Near( inv.z, 1.0f / 3000.0f, 1e-5f ), "(%.1f %.1f %.1f)", got.x, got.y, got.z );
	// IVP reports a static object's inverse mass/inertia from its (never
	// simulated) core: the authored mass, not zero. Game code reads these for
	// world objects (e.g. impact damage), so they are observed exactly.
	float staticInverse[4] = { world.pFloor->GetInvMass(), world.pFloor->GetInvInertia().x, world.pFloor->GetInvInertia().y,
		world.pFloor->GetInvInertia().z };
	ObsFloats( "object.static-inverse", "r0.1", 4, staticInverse );

	float speed = 0.25f, rot = 0.5f;
	pCube->SetDamping( &speed, &rot );
	float gotSpeed = -1, gotRot = -1;
	pCube->GetDamping( &gotSpeed, &gotRot );
	Check( TIER_BOOT, "object.damping-roundtrip", Near( gotSpeed, 0.25f, 1e-4f ) && Near( gotRot, 0.5f, 1e-4f ), "%.3f %.3f", gotSpeed, gotRot );

	// Static objects are never pinned, so IVP reports motion "enabled" while
	// IsMoveable() is false.
	Check( TIER_GAMEPLAY, "object.static-flags", !world.pFloor->IsGravityEnabled() && !world.pFloor->IsDragEnabled() && world.pFloor->IsMotionEnabled() &&
		!world.pFloor->IsMoveable() && world.pFloor->IsCollisionEnabled(),
		"gravity %d drag %d motion %d", world.pFloor->IsGravityEnabled(), world.pFloor->IsDragEnabled(), world.pFloor->IsMotionEnabled() );
	DestroyWorld( world );
}

//-----------------------------------------------------------------------------
// Drag: authored drag coefficient enables air drag on non-static polygon
// objects; the drag basis is the object's orthographic area per mass.
//-----------------------------------------------------------------------------
void TestDrag()
{
	World_t world;
	CreateWorld( world, NULL );
	IPhysicsEnvironment *pEnv = world.pEnv;
	IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 3000 ) );
	// Sensitivity: "drag-off" models a provider without air drag.
	if ( FaultIs( "drag-off" ) )
	{
		float none = 0.0f;
		pCube->SetDragCoefficient( &none, &none );
	}
	Check( TIER_GAMEPLAY, "drag.enabled-by-coefficient", pCube->IsDragEnabled() );

	objectparams_t params = DefaultParams( 50.0f, NULL );
	params.dragCoefficient = 0.0f;
	IPhysicsObject *pNoDrag = pEnv->CreatePolyObject( world.pCubeCollide, world.material, Vector( 300, 0, 3000 ), vec3_angle, &params );
	pNoDrag->EnableMotion( true );
	pNoDrag->EnableGravity( true );
	pNoDrag->Wake();
	Check( TIER_GAMEPLAY, "drag.disabled-without-coefficient", !pNoDrag->IsDragEnabled() );

	params.dragCoefficient = 1.0f;
	IPhysicsObject *pSphere = pEnv->CreateSphereObject( 16.0f, world.material, Vector( 600, 0, 3000 ), vec3_angle, &params, false );
	Check( TIER_GAMEPLAY, "drag.sphere-none", pSphere && !pSphere->IsDragEnabled() && pSphere->CalculateLinearDrag( Vector( 0, 0, 1 ) ) == 0.0f );

	float linear[3] = { pCube->CalculateLinearDrag( Vector( 1, 0, 0 ) ), pCube->CalculateLinearDrag( Vector( 0, 0, 1 ) ),
		pCube->CalculateLinearDrag( Vector( 0.6f, 0, 0.8f ) ) };
	float angular[2] = { pCube->CalculateAngularDrag( Vector( 1, 0, 0 ) ), pCube->CalculateAngularDrag( Vector( 0, 0, 1 ) ) };
	Check( TIER_GAMEPLAY, "drag.linear-basis", linear[0] > 0 && Near( linear[0], linear[1], linear[0] * 0.05f ),
		"x %.6f z %.6f", linear[0], linear[1] );
	Check( TIER_GAMEPLAY, "drag.angular-basis", angular[0] > 0, "x %.8f", angular[0] );
	ObsFloats( "drag.cube-linear", "r0.02", 3, linear );
	ObsFloats( "drag.cube-angular", "r0.02", 2, angular );

	// Coefficient scaling and toggling.
	float drag = 2.0f, angDrag = 2.0f;
	pCube->SetDragCoefficient( &drag, &angDrag );
	float doubled = pCube->CalculateLinearDrag( Vector( 1, 0, 0 ) );
	Check( TIER_GAMEPLAY, "drag.coefficient-scales", Near( doubled, 2.0f * linear[0], linear[0] * 0.05f ), "%.6f vs %.6f", doubled, linear[0] );
	drag = 1.0f;
	angDrag = 1.0f;
	pCube->SetDragCoefficient( &drag, &angDrag );
	pCube->EnableDrag( false );
	Check( TIER_GAMEPLAY, "drag.toggle", !pCube->IsDragEnabled() );
	pCube->EnableDrag( true );

	// Air drag slows a falling object relative to one without drag.
	Step( pEnv, 2.0f );
	float fallDrag = 3000.0f - PositionOf( pCube ).z;
	float fallFree = 3000.0f - PositionOf( pNoDrag ).z;
	Check( TIER_GAMEPLAY, "drag.slows-fall", fallDrag < fallFree - 1.0f && fallDrag > 0.5f * fallFree, "drag %.2f free %.2f", fallDrag, fallFree );
	float fall[2] = { fallDrag, fallFree };
	ObsFloats( "drag.fall-distance", "r0.05", 2, fall );

	// Air density scales drag; zero density removes it.
	pEnv->SetAirDensity( 0.0f );
	Vector velocity( 0, 0, 0 );
	pCube->SetVelocity( &velocity, NULL );
	pNoDrag->SetVelocity( &velocity, NULL );
	pCube->SetPosition( Vector( 0, 0, 3000 ), vec3_angle, true );
	pNoDrag->SetPosition( Vector( 300, 0, 3000 ), vec3_angle, true );
	Step( pEnv, 1.0f );
	float a = PositionOf( pCube ).z, b = PositionOf( pNoDrag ).z;
	Check( TIER_GAMEPLAY, "drag.air-density-zero", Near( a, b, 1.0f ), "drag %.2f free %.2f", a, b );
	pEnv->SetAirDensity( 2.0f );
	DestroyWorld( world );
}

//-----------------------------------------------------------------------------
// Energy, force/velocity helpers, torque, point velocity
//-----------------------------------------------------------------------------
void TestDynamicsHelpers()
{
	World_t world;
	CreateWorld( world, NULL );
	world.pEnv->SetGravity( vec3_origin );
	IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 500 ) );
	float drag = 0.0f;
	pCube->SetDragCoefficient( &drag, &drag );
	float noDamping = 0.0f;
	pCube->SetDamping( &noDamping, &noDamping );

	// Energy reflects the simulated velocity (IVP reads the integrated core
	// speed, so a velocity set this tick counts once it has been stepped).
	Vector velocity( 100, 0, 0 );
	pCube->SetVelocity( &velocity, NULL );
	Step( world.pEnv, kTick );
	float energy = pCube->GetEnergy();
	Check( TIER_GAMEPLAY, "object.energy-linear", Near( energy, 0.5f * 50.0f * 100.0f * 100.0f, 2500.0f ), "%.1f", energy );
	ObsFloats( "object.energy-linear", "r0.02", 1, &energy );

	Vector centerForce;
	AngularImpulse centerTorque;
	pCube->CalculateForceOffset( Vector( 0, 100, 0 ), Vector( 16, 0, 500 ), &centerForce, &centerTorque );
	Check( TIER_GAMEPLAY, "object.force-offset", NearVec( centerForce, Vector( 0, 100, 0 ), 0.01f ) && centerTorque.z > 0 &&
		fabsf( centerTorque.x ) < 1.0f && fabsf( centerTorque.y ) < 1.0f,
		"force (%.2f %.2f %.2f) torque (%.2f %.2f %.2f)", centerForce.x, centerForce.y, centerForce.z, centerTorque.x, centerTorque.y, centerTorque.z );
	ObsVector( "object.force-offset-torque", "r0.02", centerTorque );

	Vector centerVelocity;
	AngularImpulse centerAngular;
	pCube->CalculateVelocityOffset( Vector( 0, 100, 0 ), Vector( 16, 0, 500 ), &centerVelocity, &centerAngular );
	Check( TIER_GAMEPLAY, "object.velocity-offset", NearVec( centerVelocity, Vector( 0, 2, 0 ), 0.01f ) && centerAngular.z > 0,
		"velocity (%.3f %.3f %.3f) angular z %.4f", centerVelocity.x, centerVelocity.y, centerVelocity.z, centerAngular.z );
	ObsVector( "object.velocity-offset-angular", "r0.05", centerAngular );

	// Angular impulse about local z: the resulting spin is torque/inertia.
	Vector zero( 0, 0, 0 );
	pCube->SetVelocity( &zero, &zero );
	float inertiaZ = pCube->GetInertia().z;
	pCube->ApplyTorqueCenter( AngularImpulse( 0, 0, inertiaZ * 90.0f ) );
	Step( world.pEnv, kTick );
	AngularImpulse spin = AngularVelocityOf( pCube );
	Check( TIER_GAMEPLAY, "object.torque-impulse", Near( spin.z, 90.0f, 9.0f ) && fabsf( spin.x ) < 1.0f, "spin (%.2f %.2f %.2f)", spin.x, spin.y, spin.z );

	Vector pointVelocity;
	pCube->GetVelocityAtPoint( PositionOf( pCube ) + Vector( 16, 0, 0 ), &pointVelocity );
	// 90 deg/s about z at 16 units: |v| = 16 * pi/2 ~= 25.1, along +y.
	Check( TIER_GAMEPLAY, "object.point-velocity", Near( pointVelocity.y, 25.1f, 3.0f ) && fabsf( pointVelocity.x ) < 3.0f,
		"(%.2f %.2f %.2f)", pointVelocity.x, pointVelocity.y, pointVelocity.z );

	pCube->SetVelocity( &zero, &zero );
	Vector add( 10, 0, 0 );
	pCube->AddVelocity( &add, NULL );
	pCube->AddVelocity( &add, NULL );
	Check( TIER_GAMEPLAY, "object.add-velocity", Near( VelocityOf( pCube ).x, 20.0f, 0.5f ), "vx %.2f", VelocityOf( pCube ).x );
	pCube->SetVelocityInstantaneous( &velocity, &zero );
	Check( TIER_GAMEPLAY, "object.set-velocity-instantaneous", NearVec( VelocityOf( pCube ), velocity, 0.5f ) && !pCube->IsAsleep() );

	// Pinned objects ignore velocity and impulses.
	pCube->EnableMotion( false );
	pCube->ApplyForceCenter( Vector( 0, 0, 1e5f ) );
	pCube->SetVelocity( &velocity, NULL );
	Vector pinned = PositionOf( pCube );
	Step( world.pEnv, 0.2f );
	Check( TIER_GAMEPLAY, "object.pinned-ignores-impulse", NearVec( PositionOf( pCube ), pinned, 0.01f ) );
	pCube->EnableMotion( true );
	DestroyWorld( world );
}

//-----------------------------------------------------------------------------
// Hinged objects, collision and motion toggles, explicit sleep
//-----------------------------------------------------------------------------
void TestObjectModes()
{
	// BecomeHinged: rotation only about the local axis; translation free.
	{
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetGravity( vec3_origin );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 500 ) );
		pCube->BecomeHinged( 2 );
		Check( TIER_GAMEPLAY, "object.hinged-flag", pCube->IsHinged() );
		// Torques about every axis: a hinged object only turns about its
		// hinge axis (IVP pins the others with near-infinite inertia).
		Vector inertia = pCube->GetInertia();
		Vector velocity( 50, 0, 0 );
		pCube->SetVelocity( &velocity, NULL );
		pCube->ApplyTorqueCenter( AngularImpulse( inertia.z * 90.0f, inertia.z * 90.0f, inertia.z * 90.0f ) );
		Step( world.pEnv, 0.5f );
		// Allow wobble on the locked axes: IVP pins them with a huge inertia.
		float tilt = RAD2DEG( acosf( clamp( DotProduct( AxisOf( pCube, 2 ), Vector( 0, 0, 1 ) ), -1.0f, 1.0f ) ) );
		float yaw = RAD2DEG( acosf( clamp( DotProduct( AxisOf( pCube, 0 ), Vector( 1, 0, 0 ) ), -1.0f, 1.0f ) ) );
		Check( TIER_GAMEPLAY, "object.hinged-axis-only", tilt < 5.0f && yaw > 20.0f && PositionOf( pCube ).x > 10.0f,
			"tilt %.2f yaw %.2f x %.2f", tilt, yaw, PositionOf( pCube ).x );
		pCube->RemoveHinged();
		Check( TIER_GAMEPLAY, "object.unhinged", !pCube->IsHinged() );
		DestroyWorld( world );
	}

	// Collisions off: falls through the floor; back on: stays out.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 100 ) );
		pCube->EnableCollisions( false );
		Check( TIER_GAMEPLAY, "object.collisions-disabled-flag", !pCube->IsCollisionEnabled() );
		Step( world.pEnv, 1.0f );
		Check( TIER_GAMEPLAY, "object.collisions-disabled-falls", PositionOf( pCube ).z < -50.0f, "z %.2f", PositionOf( pCube ).z );
		pCube->EnableCollisions( true );
		pCube->SetPosition( Vector( 0, 0, 100 ), vec3_angle, true );
		pCube->SetVelocity( &vec3_origin, &vec3_origin );
		Step( world.pEnv, 2.0f );
		Check( TIER_GAMEPLAY, "object.collisions-reenabled", pCube->IsCollisionEnabled() && PositionOf( pCube ).z > 14.0f,
			"z %.2f", PositionOf( pCube ).z );
		DestroyWorld( world );
	}

	// Motion off mid-flight stops the object; on again it falls again.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 1000 ) );
		Step( world.pEnv, 0.3f );
		pCube->EnableMotion( false );
		Vector frozen = PositionOf( pCube );
		Step( world.pEnv, 0.3f );
		// IVP finishes the current sub-step interpolation after pinning.
		Check( TIER_GAMEPLAY, "object.motion-disable-stops", NearVec( PositionOf( pCube ), frozen, 0.1f ) && !pCube->IsMoveable(),
			"z %.3f from %.3f moveable %d", PositionOf( pCube ).z, frozen.z, pCube->IsMoveable() );
		pCube->EnableMotion( true );
		pCube->Wake();
		Step( world.pEnv, 0.3f );
		Check( TIER_GAMEPLAY, "object.motion-enable-resumes", PositionOf( pCube ).z < frozen.z - 5.0f, "z %.2f from %.2f", PositionOf( pCube ).z, frozen.z );
		DestroyWorld( world );
	}

	// Explicit sleep holds an object in mid-air until woken.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 1000 ) );
		pCube->Sleep();
		Step( world.pEnv, 0.3f );
		Check( TIER_GAMEPLAY, "object.sleep-holds", pCube->IsAsleep() && Near( PositionOf( pCube ).z, 1000.0f, 0.01f ), "asleep %d z %.2f",
			pCube->IsAsleep(), PositionOf( pCube ).z );
		pCube->Wake();
		Step( world.pEnv, 0.3f );
		Check( TIER_GAMEPLAY, "object.wake-resumes", !pCube->IsAsleep() && PositionOf( pCube ).z < 990.0f, "z %.2f", PositionOf( pCube ).z );
		DestroyWorld( world );
	}

	// Resting contact: contact point and friction snapshot data.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 40 ) );
		Step( world.pEnv, 1.5f );
		Vector contact;
		IPhysicsObject *pOther = NULL;
		bool touching = pCube->GetContactPoint( &contact, &pOther );
		Check( TIER_GAMEPLAY, "contacts.contact-point", touching && pOther == world.pFloor && Near( contact.z, 0.0f, 1.0f ),
			"touching %d z %.2f", touching, contact.z );
		IPhysicsFrictionSnapshot *pSnapshot = pCube->CreateFrictionSnapshot();
		float normalForce = 0.0f;
		Vector normal( 0, 0, 0 );
		int material = -1;
		for ( ; pSnapshot->IsValid(); pSnapshot->NextFrictionData() )
		{
			normalForce += pSnapshot->GetNormalForce();
			pSnapshot->GetSurfaceNormal( normal );
			material = pSnapshot->GetMaterial( 1 );
		}
		pCube->DestroyFrictionSnapshot( pSnapshot );
		// Resting weight: m * g = 50 * 600.
		Check( TIER_GAMEPLAY, "contacts.normal-force", Near( normalForce, 30000.0f, 9000.0f ), "%.1f", normalForce );
		Check( TIER_GAMEPLAY, "contacts.surface-normal", normal.z < -0.9f, "(%.2f %.2f %.2f)", normal.x, normal.y, normal.z );
		Check( TIER_GAMEPLAY, "contacts.material", material == world.material, "%d", material );
		DestroyWorld( world );
	}

	// Game-animated props (doors, droppers) are shadows that neither
	// translate nor rotate from physics: IVP gives them ~1e14 inertia and
	// maximum mass. Driven by their animation, with a dynamic object
	// landing on them, everything stays finite.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pDoor = CreateCube( world, Vector( 0, 0, 100 ) );
		pDoor->SetShadow( 1e4f, 1e4f, false, false );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 200 ) );
		for ( int i = 0; i < 60; i++ )
		{
			pDoor->UpdateShadow( Vector( 0, 0, 100 + i * 0.5f ), QAngle( 0, i * 3.0f, i * 1.0f ), false, kTick );
			Step( world.pEnv, kTick );
		}
		Vector doorPosition = PositionOf( pDoor ), cubePosition = PositionOf( pCube );
		QAngle doorAngles;
		pDoor->GetPosition( NULL, &doorAngles );
		Check( TIER_GAMEPLAY, "shadow.rigid-animated-finite", IsFiniteVec( doorPosition ) && IsFiniteVec( cubePosition ) &&
			IsFiniteVec( VelocityOf( pCube ) ) && fabsf( doorAngles.y ) < 1e6f && cubePosition.z > 100.0f,
			"door (%.2f %.2f %.2f) cube (%.2f %.2f %.2f)", doorPosition.x, doorPosition.y, doorPosition.z, cubePosition.x, cubePosition.y, cubePosition.z );
		DestroyWorld( world );
	}

	// Shadow targets follow teleports.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 500 ) );
		pCube->SetShadow( 1e4f, 1e4f, true, true );
		pCube->SetPosition( Vector( 50, 60, 70 ), vec3_angle, true );
		Vector target;
		pCube->GetShadowController()->GetTargetPosition( &target, NULL );
		Check( TIER_GAMEPLAY, "shadow.teleport-updates-target", NearVec( target, Vector( 50, 60, 70 ), 0.01f ),
			"(%.2f %.2f %.2f)", target.x, target.y, target.z );
		DestroyWorld( world );
	}
}

//-----------------------------------------------------------------------------
// Touch, friction (scrape) and trigger events
//-----------------------------------------------------------------------------
void TestTouchEvents()
{
	// Two dynamic objects: StartTouch when they come into contact, EndTouch
	// when separated. Static contacts only report with TOUCH_STATIC.
	{
		CEventLog log;
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetCollisionEventHandler( &log );
		IPhysicsObject *pBottom = CreateCube( world, Vector( 0, 0, 16 ) );
		IPhysicsObject *pTop = CreateCube( world, Vector( 0, 0, 80 ) );
		log.m_pTouchA = pBottom;
		log.m_pTouchB = pTop;
		Step( world.pEnv, 1.5f );
		Check( TIER_GAMEPLAY, "events.start-touch", log.m_touchPairs > 0, "pairs %d start %d", log.m_touchPairs, log.m_startTouch );
		Check( TIER_GAMEPLAY, "events.post-simulation-frame", log.m_postFrame > 0 );
		pTop->SetPosition( Vector( 500, 0, 16 ), vec3_angle, true );
		pTop->Wake();
		Step( world.pEnv, 0.3f );
		Check( TIER_GAMEPLAY, "events.end-touch", log.m_touchPairs == 0 && log.m_endTouch > 0, "pairs %d end %d", log.m_touchPairs, log.m_endTouch );
		world.pEnv->SetCollisionEventHandler( NULL );
		DestroyWorld( world );
	}

	{
		CEventLog log;
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetCollisionEventHandler( &log );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 16 ) );
		log.m_pTouchA = pCube;
		log.m_pTouchB = world.pFloor;
		Step( world.pEnv, 1.0f );
		int withoutStatic = log.m_pairStarts;
		pCube->SetCallbackFlags( pCube->GetCallbackFlags() | CALLBACK_GLOBAL_TOUCH_STATIC );
		pCube->SetPosition( Vector( 0, 0, 60 ), vec3_angle, true );
		pCube->Wake();
		Step( world.pEnv, 1.0f );
		Check( TIER_GAMEPLAY, "events.touch-static-gated", withoutStatic == 0 && log.m_pairStarts > 0,
			"without %d with %d", withoutStatic, log.m_pairStarts );

		// Friction (scrape) events while sliding over the floor.
		log.m_friction = 0;
		Vector slide( 400, 0, 0 );
		pCube->SetVelocity( &slide, NULL );
		Step( world.pEnv, 0.5f );
		Check( TIER_GAMEPLAY, "events.friction-scrape", log.m_friction > 0 && log.m_frictionEnergy > 0, "count %d energy %.2f",
			log.m_friction, log.m_frictionEnergy );
		world.pEnv->SetCollisionEventHandler( NULL );
		DestroyWorld( world );
	}

	// Sleep and wake events.
	{
		CEventLog log;
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetObjectEventHandler( &log );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 16 ) );
		Step( world.pEnv, 4.0f );
		int sleeps = log.m_sleep;
		log.m_wake = 0;
		Vector kick( 0, 0, 200 );
		pCube->SetVelocity( &kick, NULL );
		Step( world.pEnv, kTick );
		Check( TIER_GAMEPLAY, "events.wake", sleeps > 0 && log.m_wake > 0, "sleeps %d wakes %d", sleeps, log.m_wake );
		world.pEnv->SetObjectEventHandler( NULL );
		DestroyWorld( world );
	}

	// Triggers: objects pass through and the trigger reports enter/leave.
	{
		CEventLog log;
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetCollisionEventHandler( &log );
		CPhysCollide *pVolume = s_pCollision->BBoxToCollide( Vector( -64, -64, -32 ), Vector( 64, 64, 32 ) );
		objectparams_t params = DefaultParams( 1.0f, NULL );
		IPhysicsObject *pTrigger = world.pEnv->CreatePolyObjectStatic( pVolume, world.material, Vector( 0, 0, 300 ), vec3_angle, &params );
		pTrigger->BecomeTrigger();
		Check( TIER_GAMEPLAY, "trigger.flag", pTrigger->IsTrigger() && !pTrigger->IsFluid() );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 500 ) );
		Step( world.pEnv, 0.9f );
		float midZ = PositionOf( pCube ).z;
		Step( world.pEnv, 0.6f );
		Check( TIER_GAMEPLAY, "trigger.passes-through", PositionOf( pCube ).z < 200.0f, "z %.2f (mid %.2f)", PositionOf( pCube ).z, midZ );
		Check( TIER_GAMEPLAY, "trigger.enter", log.m_enterTrigger > 0 && log.m_pTrigger == pTrigger && log.m_pTriggered == pCube,
			"enter %d", log.m_enterTrigger );
		Check( TIER_GAMEPLAY, "trigger.leave", log.m_leaveTrigger > 0, "leave %d", log.m_leaveTrigger );
		pTrigger->RemoveTrigger();
		Check( TIER_GAMEPLAY, "trigger.remove", !pTrigger->IsTrigger() );
		world.pEnv->SetCollisionEventHandler( NULL );
		DestroyWorld( world );
		s_pCollision->DestroyCollide( pVolume );
	}
}

//-----------------------------------------------------------------------------
// Lifetime: deletion inside simulation callbacks, the delete queue,
// transfer between environments, serialization.
//-----------------------------------------------------------------------------
void TestLifetime()
{
	// Game code destroys objects from collision callbacks; the provider must
	// report being in simulation there and defer the deletion safely.
	{
		CEventLog log;
		World_t world;
		CreateWorld( world, NULL );
		log.m_pEnv = world.pEnv;
		world.pEnv->SetCollisionEventHandler( &log );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 100 ) );
		log.m_pDeleteInCallback = pCube;
		Step( world.pEnv, 1.5f );
		Check( TIER_GAMEPLAY, "lifetime.in-simulation-callbacks", log.m_inSimulationDuringCallback && log.m_pre > 0 );
		Check( TIER_GAMEPLAY, "lifetime.delete-in-callback", log.m_pDeleteInCallback == NULL && !Listed( world.pEnv, pCube ) &&
			ObjectCount( world.pEnv ) == 1, "count %d", ObjectCount( world.pEnv ) );
		world.pEnv->SetCollisionEventHandler( NULL );
		DestroyWorld( world );
	}

	// The delete queue defers destruction until CleanupDeleteList.
	{
		World_t world;
		CreateWorld( world, NULL );
		CPhysCollide *pOwnCollide = s_pCollision->BBoxToCollide( Vector( -8, -8, -8 ), Vector( 8, 8, 8 ) );
		objectparams_t params = DefaultParams( 10.0f, NULL );
		IPhysicsObject *pObject = world.pEnv->CreatePolyObject( pOwnCollide, world.material, Vector( 0, 0, 100 ), vec3_angle, &params );
		world.pEnv->EnableDeleteQueue( true );
		world.pEnv->DestroyObject( pObject );
		bool unlisted = !Listed( world.pEnv, pObject );
		bool stillUsed = world.pEnv->IsCollisionModelUsed( pOwnCollide );
		world.pEnv->CleanupDeleteList();
		bool released = !world.pEnv->IsCollisionModelUsed( pOwnCollide );
		Check( TIER_GAMEPLAY, "lifetime.delete-queue", unlisted && stillUsed && released,
			"unlisted %d queued-in-use %d released %d", unlisted, stillUsed, released );
		world.pEnv->EnableDeleteQueue( false );
		DestroyWorld( world );
		s_pCollision->DestroyCollide( pOwnCollide );
	}

	// TransferObject moves a live object (state, game data, shadow) into
	// another environment (Portal's simulators do this).
	{
		World_t source, destination;
		CreateWorld( source, NULL );
		CreateWorld( destination, NULL );
		int gameData = 0;
		IPhysicsObject *pCube = CreateCube( source, Vector( 0, 0, 300 ), 50.0f, &gameData );
		pCube->SetGameFlags( 0x42 );
		pCube->SetShadow( 1e4f, 1e4f, true, true );
		IPhysicsShadowController *pShadow = pCube->GetShadowController();
		Vector velocity( 30, 0, 0 );
		pCube->SetVelocity( &velocity, NULL );
		Step( source.pEnv, 0.1f );
		Vector before = PositionOf( pCube );
		bool moved = source.pEnv->TransferObject( pCube, destination.pEnv );
		Check( TIER_GAMEPLAY, "lifetime.transfer", moved && !Listed( source.pEnv, pCube ) && Listed( destination.pEnv, pCube ),
			"moved %d", moved );
		Check( TIER_GAMEPLAY, "lifetime.transfer-state", NearVec( PositionOf( pCube ), before, 0.01f ) && pCube->GetGameData() == &gameData &&
			pCube->GetGameFlags() == 0x42 && Near( pCube->GetMass(), 50.0f, 0.01f ) && pCube->GetShadowController() == pShadow,
			"pos (%.2f %.2f %.2f) shadow %d", PositionOf( pCube ).x, PositionOf( pCube ).y, PositionOf( pCube ).z, pCube->GetShadowController() == pShadow );
		pCube->RemoveShadowController();
		Step( destination.pEnv, 1.5f );
		Check( TIER_GAMEPLAY, "lifetime.transfer-simulates", PositionOf( pCube ).z < 30.0f && PositionOf( pCube ).z > 10.0f,
			"z %.2f", PositionOf( pCube ).z );
		Check( TIER_GAMEPLAY, "lifetime.transfer-foreign", !source.pEnv->TransferObject( pCube, destination.pEnv ) );
		DestroyWorld( source );
		DestroyWorld( destination );
	}

	// Serialize/unserialize an object (Portal's shadow clones).
	{
		World_t source, destination;
		CreateWorld( source, NULL );
		CreateWorld( destination, NULL );
		IPhysicsObject *pCube = CreateCube( source, Vector( 10, 20, 300 ), 40.0f );
		pCube->SetGameFlags( 0x17 );
		pCube->SetContents( CONTENTS_SOLID | CONTENTS_MONSTER );
		Vector velocity( 0, 25, 0 );
		pCube->SetVelocity( &velocity, NULL );
		unsigned int size = source.pEnv->GetObjectSerializeSize( pCube );
		CUtlVector<unsigned char> buffer;
		buffer.SetCount( size ? size : 1 );
		source.pEnv->SerializeObjectToBuffer( pCube, buffer.Base(), size );
		int gameData = 0;
		IPhysicsObject *pClone = size ? destination.pEnv->UnserializeObjectFromBuffer( &gameData, buffer.Base(), size, true ) : NULL;
		Check( TIER_GAMEPLAY, "lifetime.serialize", size > 0 && pClone != NULL && Listed( destination.pEnv, pClone ), "size %u", size );
		if ( pClone )
		{
			Check( TIER_GAMEPLAY, "lifetime.serialize-state", NearVec( PositionOf( pClone ), Vector( 10, 20, 300 ), 0.01f ) &&
				Near( pClone->GetMass(), 40.0f, 0.01f ) && pClone->GetGameData() == &gameData && pClone->GetGameFlags() == 0x17 &&
				pClone->GetContents() == ( CONTENTS_SOLID | CONTENTS_MONSTER ) && pClone->GetCollide() == pCube->GetCollide() &&
				NearVec( VelocityOf( pClone ), velocity, 0.5f ) && pClone->IsCollisionEnabled(),
				"pos (%.2f %.2f %.2f) mass %.2f", PositionOf( pClone ).x, PositionOf( pClone ).y, PositionOf( pClone ).z, pClone->GetMass() );
		}
		DestroyWorld( source );
		DestroyWorld( destination );
	}
}
}

namespace
{
// Game collision rules that can turn one pair off (the NPC solver's
// penetration mode).
class CPairRule : public IPhysicsCollisionSolver
{
public:
	CPairRule() : m_pA( NULL ), m_pB( NULL ) {}
	virtual int ShouldCollide( IPhysicsObject *p0, IPhysicsObject *p1, void *, void * )
	{
		return !( ( p0 == m_pA && p1 == m_pB ) || ( p0 == m_pB && p1 == m_pA ) );
	}
	virtual int ShouldSolvePenetration( IPhysicsObject *, IPhysicsObject *, void *, void *, float ) { return 1; }
	virtual bool ShouldFreezeObject( IPhysicsObject * ) { return false; }
	virtual int AdditionalCollisionChecksThisTick( int ) { return 0; }
	virtual bool ShouldFreezeContacts( IPhysicsObject **, int ) { return false; }
	IPhysicsObject *m_pA;
	IPhysicsObject *m_pB;
};

class CTouchCounter : public IPhysicsCollisionEvent
{
public:
	CTouchCounter() : m_start( 0 ), m_end( 0 ) {}
	virtual void PreCollision( vcollisionevent_t * ) {}
	virtual void PostCollision( vcollisionevent_t * ) {}
	virtual void Friction( IPhysicsObject *, float, int, int, IPhysicsCollisionData * ) {}
	virtual void StartTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) { m_start++; }
	virtual void EndTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) { m_end++; }
	virtual void FluidStartTouch( IPhysicsObject *, IPhysicsFluidController * ) {}
	virtual void FluidEndTouch( IPhysicsObject *, IPhysicsFluidController * ) {}
	virtual void PostSimulationFrame() {}
	int m_start, m_end;
};

// A cube resting on the floor; then optionally the rules drop the pair and/or
// the game deletes their contact through the cube's friction snapshot, as
// CPhysicsNPCSolver does. Returns the cube's height a second later.
float ContactDeletion( bool ruleOff, bool deleteContact, int *pStarts = NULL, int *pEnds = NULL )
{
	CPairRule rule;
	World_t world;
	if ( !CreateWorld( world, &rule ) )
		return 0.0f;
	CTouchCounter touches;
	world.pEnv->SetCollisionEventHandler( &touches );
	IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 17 ) );
	pCube->SetCallbackFlags( pCube->GetCallbackFlags() | CALLBACK_GLOBAL_TOUCH_STATIC );
	Step( world.pEnv, 1.0f );
	touches.m_start = touches.m_end = 0;
	if ( ruleOff )
	{
		rule.m_pA = pCube;
		rule.m_pB = world.pFloor;
	}
	if ( deleteContact )
	{
		IPhysicsFrictionSnapshot *pSnapshot = pCube->CreateFrictionSnapshot();
		while ( pSnapshot->IsValid() )
		{
			if ( pSnapshot->GetObject( 1 ) == world.pFloor )
				pSnapshot->MarkContactForDelete();
			pSnapshot->NextFrictionData();
		}
		pSnapshot->DeleteAllMarkedContacts( true );
		pCube->DestroyFrictionSnapshot( pSnapshot );
	}
	// The deletion wakes only the partner (here the static floor).
	pCube->Wake();
	Step( world.pEnv, 1.0f );
	float z = PositionOf( pCube ).z;
	if ( pStarts )
		*pStarts = touches.m_start;
	if ( pEnds )
		*pEnds = touches.m_end;
	world.pEnv->SetCollisionEventHandler( NULL );
	world.pEnv->DestroyObject( pCube );
	DestroyWorld( world );
	return z;
}

void TestContactDeletion()
{
	// A rule change alone leaves the existing contact in place; deleting the
	// contact applies it; a deleted contact the rules still allow returns.
	float ruleOnly = ContactDeletion( true, false );
	int starts = 0, ends = 0;
	float deleted = ContactDeletion( true, true );
	float restored = ContactDeletion( false, true, &starts, &ends );

	Check( TIER_GAMEPLAY, "contacts.rule-change-keeps-contact", Near( ruleOnly, 16.0f, 2.0f ), "z %.2f", ruleOnly );
	Check( TIER_GAMEPLAY, "contacts.delete-applies-rules", deleted < -40.0f, "z %.2f", deleted );
	// The deleted contact ends and begins again (IVP reports each contact
	// point; the count may differ).
	Check( TIER_GAMEPLAY, "contacts.deleted-contact-returns", Near( restored, 16.0f, 2.0f ) && starts > 0 && ends > 0,
		"z %.2f starts %d ends %d", restored, starts, ends );
}

// Simulation statistics: counters accumulate as objects collide and reset
// on ClearStats.
void TestStats()
{
	World_t world;
	if ( !CreateWorld( world, NULL ) )
		return;
	world.pEnv->ClearStats();
	physics_stats_t cleared;
	memset( &cleared, 0xFF, sizeof( cleared ) );
	world.pEnv->ReadStats( &cleared );
	IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 100 ) );
	Step( world.pEnv, 2.0f );
	physics_stats_t stats;
	memset( &stats, 0, sizeof( stats ) );
	world.pEnv->ReadStats( &stats );
	Check( TIER_GAMEPLAY, "stats.counts-collisions", cleared.impactCounter == 0 && cleared.collisionPairsCreated == 0 &&
		stats.impactCounter > 0 && stats.collisionPairsCreated > 0, "cleared %d/%d then impacts %d pairs %d", cleared.impactCounter,
		cleared.collisionPairsCreated, stats.impactCounter, stats.collisionPairsCreated );
	world.pEnv->ClearStats();
	world.pEnv->ReadStats( &stats );
	Check( TIER_GAMEPLAY, "stats.clear", stats.impactCounter == 0 && stats.collisionPairsCreated == 0 );
	// Debug hook: callable in any state.
	world.pEnv->DebugCheckContacts();
	world.pEnv->DestroyObject( pCube );
	DestroyWorld( world );
}
}

void TestObjectsAndEvents()
{
	TestStats();
	TestContactDeletion();
	TestMassProperties();
	TestDrag();
	TestDynamicsHelpers();
	TestObjectModes();
	TestTouchEvents();
	TestLifetime();
}
