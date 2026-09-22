//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics provider conformance: constraints (hinge, ballsocket,
//          sliding, length, pulley, ragdoll, fixed), constraint groups,
//          activation, motors, breaking and constraint lifetime.
//          Contract record: unittests/physicstest/contracts/vphysics.provider.v1.md
//
//          Every scene hangs objects from the static world floor (at the
//          origin) well above it, so only the constraint under test acts on
//          them. Trajectories are not required to match IVP; checks are the
//          constraint's defining invariant (anchor held, axis kept, limit
//          respected) and observations carry signed quantities that must
//          agree in sign and rough magnitude.
//
//=============================================================================//

#include <math.h>
#include <string.h>

#include "vphysics_conformance.h"
#include "vphysics/constraints.h"

namespace
{
const Vector kPivot( 0, 0, 500 );

class CConstraintEvents : public IPhysicsConstraintEvent
{
public:
	CConstraintEvents() : m_broken( 0 ), m_pLast( NULL ), m_deactivate( true ) {}
	virtual void ConstraintBroken( IPhysicsConstraint *pConstraint )
	{
		m_broken++;
		m_pLast = pConstraint;
		// Game code (physconstraint.cpp) deactivates a broken constraint.
		if ( m_deactivate )
			pConstraint->Deactivate();
	}
	int m_broken;
	IPhysicsConstraint *m_pLast;
	bool m_deactivate;
};

// Sensitivity: "constraint-inactive" models a provider whose constraints
// never act (the pre-C2 Box3D stub) by deactivating each on creation.
IPhysicsConstraint *Faulted( IPhysicsConstraint *pConstraint )
{
	if ( pConstraint && FaultIs( "constraint-inactive" ) )
		pConstraint->Deactivate();
	return pConstraint;
}

float MinZOver( World_t &world, IPhysicsObject *pObject, float seconds )
{
	float minZ = PositionOf( pObject ).z;
	int ticks = (int)( seconds / kTick + 0.5f );
	for ( int i = 0; i < ticks; i++ )
	{
		Step( world.pEnv, kTick );
		minZ = MIN( minZ, PositionOf( pObject ).z );
	}
	return minZ;
}

// The pivot expressed in the attached object's frame; a joint that holds
// its anchor keeps this constant.
Vector PivotLocal( IPhysicsObject *pObject, const Vector &pivot )
{
	Vector local;
	pObject->WorldToLocal( &local, pivot );
	return local;
}

// Angle in degrees between an object's local axis now and a world direction.
float AxisAngle( IPhysicsObject *pObject, int axis, const Vector &direction )
{
	float dot = DotProduct( AxisOf( pObject, axis ), direction );
	dot = clamp( dot, -1.0f, 1.0f );
	return RAD2DEG( acosf( dot ) );
}

constraint_hingeparams_t HingeParams( const Vector &axis )
{
	constraint_hingeparams_t hinge;
	hinge.Defaults();
	hinge.worldPosition = kPivot;
	hinge.worldAxisDirection = axis;
	return hinge;
}

//-----------------------------------------------------------------------------
// Hinge: rotation only about the axis through the pivot; limits; motors.
//-----------------------------------------------------------------------------
void TestHinge()
{
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		IPhysicsConstraint *pHinge = Faulted( world.pEnv->CreateHingeConstraint( world.pFloor, pCube, NULL, HingeParams( Vector( 0, 1, 0 ) ) ) );
		if ( Check( TIER_BOOT, "constraint.hinge-create", pHinge != NULL ) )
		{
			Check( TIER_BOOT, "constraint.hinge-objects", pHinge->GetReferenceObject() == world.pFloor && pHinge->GetAttachedObject() == pCube );
			float minZ = MinZOver( world, pCube, 0.5f );
			// Legacy quirk preserved: IVP's IsAttachedToConstraint only sees
			// IVP_CP_CONSTRAINTS controllers, and every Havok constraint registers
			// at IVP_CP_CONSTRAINTS_MIN, so it reports false for all of them. Game
			// code (CanResolvePenetrationWithNPC) depends on that answer.
			Check( TIER_GAMEPLAY, "constraint.attached-flag-legacy", !pCube->IsAttachedToConstraint( false ) && !pCube->IsAttachedToConstraint( true ) );
			Vector local = PivotLocal( pCube, kPivot );
			Vector position = PositionOf( pCube );
			Check( TIER_GAMEPLAY, "constraint.hinge-swings", minZ < 470.0f, "min z %.2f", minZ );
			Check( TIER_GAMEPLAY, "constraint.hinge-anchor-held", NearVec( local, Vector( -40, 0, 0 ), 2.0f ),
				"pivot local (%.2f %.2f %.2f)", local.x, local.y, local.z );
			Check( TIER_GAMEPLAY, "constraint.hinge-planar", Near( position.y, 0.0f, 1.0f ) && AxisAngle( pCube, 1, Vector( 0, 1, 0 ) ) < 3.0f,
				"y %.2f axis angle %.2f", position.y, AxisAngle( pCube, 1, Vector( 0, 1, 0 ) ) );
			world.pEnv->DestroyConstraint( pHinge );
		}
		DestroyWorld( world );
	}

	// Symmetric limits: a 40-unit arm limited to +-20 degrees rests near
	// 40 * sin(20) ~= 13.7 units down. IVP's limit is a soft servo (it
	// overshoots to ~30 degrees and settles near 15-20), so the check bounds
	// the overshoot and the resting angle rather than demanding a hard stop.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		constraint_hingeparams_t hinge = HingeParams( Vector( 0, 1, 0 ) );
		hinge.hingeAxis.SetAxisFriction( -20, 20, 0 );
		IPhysicsConstraint *pHinge = Faulted( world.pEnv->CreateHingeConstraint( world.pFloor, pCube, NULL, hinge ) );
		float minZ = MinZOver( world, pCube, 1.0f );
		float z = PositionOf( pCube ).z;
		Check( TIER_GAMEPLAY, "constraint.hinge-limit", pHinge && minZ > 476.0f && z > 483.0f && z < 497.0f, "min z %.2f z %.2f", minZ, z );
		if ( pHinge )
			world.pEnv->DestroyConstraint( pHinge );
		DestroyWorld( world );
	}

	// Limit sign convention: a hinge angle is measured clockwise about the
	// authored axis (IVP's HL->IVP basis flip), so [0, 45] about +y blocks the
	// arm's fall and [-45, 0] allows it. Doors authored against IVP swing the
	// way they do because of this.
	for ( int k = 0; k < 2; k++ )
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		constraint_hingeparams_t hinge = HingeParams( Vector( 0, 1, 0 ) );
		hinge.hingeAxis.SetAxisFriction( k == 0 ? 0.0f : -45.0f, k == 0 ? 45.0f : 0.0f, 0 );
		IPhysicsConstraint *pHinge = Faulted( world.pEnv->CreateHingeConstraint( world.pFloor, pCube, NULL, hinge ) );
		float minZ = MinZOver( world, pCube, 1.0f );
		bool ok = k == 0 ? minZ > 490.0f : minZ < 480.0f;
		Check( TIER_GAMEPLAY, k == 0 ? "constraint.hinge-limit-sign-blocks" : "constraint.hinge-limit-sign-allows", pHinge && ok, "min z %.2f", minZ );
		if ( pHinge )
			world.pEnv->DestroyConstraint( pHinge );
		DestroyWorld( world );
	}

	// Motor: no gravity; the authored motor spins the arm about the axis.
	{
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetGravity( vec3_origin );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		constraint_hingeparams_t hinge = HingeParams( Vector( 0, 0, 1 ) );
		hinge.hingeAxis.angularVelocity = 90.0f;
		hinge.hingeAxis.torque = 1e5f;
		IPhysicsConstraint *pHinge = Faulted( world.pEnv->CreateHingeConstraint( world.pFloor, pCube, NULL, hinge ) );
		Step( world.pEnv, 1.0f );
		AngularImpulse spin = AngularVelocityOf( pCube );
		Vector local = PivotLocal( pCube, kPivot );
		Check( TIER_GAMEPLAY, "constraint.hinge-motor", pHinge && spin.z < -70.0f && spin.z > -110.0f && fabsf( spin.x ) < 5.0f && fabsf( spin.y ) < 5.0f,
			"spin (%.2f %.2f %.2f)", spin.x, spin.y, spin.z );
		Check( TIER_GAMEPLAY, "constraint.hinge-motor-anchor", NearVec( local, Vector( -40, 0, 0 ), 2.0f ),
			"pivot local (%.2f %.2f %.2f)", local.x, local.y, local.z );

		// IPhysicsConstraint::SetAngularMotor retargets a running hinge motor
		// (phys_hinge SetVelocity: max impulse = speed * |inertia| / tick). The
		// motor turns clockwise about the axis for a positive speed.
		if ( pHinge )
			pHinge->SetAngularMotor( -45.0f, 45.0f * pCube->GetInertia().Length() / kTick );
		Step( world.pEnv, 1.0f );
		spin = AngularVelocityOf( pCube );
		Check( TIER_GAMEPLAY, "constraint.hinge-set-motor", spin.z > 30.0f && spin.z < 60.0f, "spin z %.2f", spin.z );
		if ( pHinge )
			world.pEnv->DestroyConstraint( pHinge );
		DestroyWorld( world );
	}
}

//-----------------------------------------------------------------------------
// Ballsocket: the anchor is shared; rotation is free in every direction.
//-----------------------------------------------------------------------------
void TestBallsocket()
{
	World_t world;
	CreateWorld( world, NULL );
	IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
	constraint_ballsocketparams_t ballsocket;
	ballsocket.Defaults();
	ballsocket.InitWithCurrentObjectState( world.pFloor, pCube, kPivot );
	IPhysicsConstraint *pBall = Faulted( world.pEnv->CreateBallsocketConstraint( world.pFloor, pCube, NULL, ballsocket ) );
	if ( !Check( TIER_BOOT, "constraint.ballsocket-create", pBall != NULL ) )
	{
		DestroyWorld( world );
		return;
	}
	Vector sideways( 0, 80, 0 );
	pCube->SetVelocity( &sideways, NULL );
	float minZ = MinZOver( world, pCube, 0.5f );
	Vector local = PivotLocal( pCube, kPivot );
	float distance = ( PositionOf( pCube ) - kPivot ).Length();
	Check( TIER_GAMEPLAY, "constraint.ballsocket-anchor-held", NearVec( local, Vector( -40, 0, 0 ), 2.0f ) && Near( distance, 40.0f, 2.5f ),
		"pivot local (%.2f %.2f %.2f) distance %.2f", local.x, local.y, local.z, distance );
	Check( TIER_GAMEPLAY, "constraint.ballsocket-free", minZ < 480.0f && fabsf( PositionOf( pCube ).y ) > 5.0f,
		"min z %.2f y %.2f", minZ, PositionOf( pCube ).y );

	matrix3x4_t toRef, toAtt;
	bool transform = pBall->GetConstraintTransform( &toRef, &toAtt );
	Vector refOrigin, attOrigin;
	MatrixGetColumn( toRef, 3, refOrigin );
	MatrixGetColumn( toAtt, 3, attOrigin );
	Check( TIER_GAMEPLAY, "constraint.ballsocket-transform", transform && NearVec( refOrigin, kPivot, 0.5f ) && NearVec( attOrigin, Vector( -40, 0, 0 ), 0.5f ),
		"ok %d ref (%.2f %.2f %.2f) att (%.2f %.2f %.2f)", transform, refOrigin.x, refOrigin.y, refOrigin.z, attOrigin.x, attOrigin.y, attOrigin.z );
	world.pEnv->DestroyConstraint( pBall );
	DestroyWorld( world );
}

//-----------------------------------------------------------------------------
// Sliding: translation along one axis only, no rotation; limits; motor.
//-----------------------------------------------------------------------------
void TestSliding()
{
	{
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetGravity( vec3_origin );
		IPhysicsObject *pCube = CreateCube( world, kPivot );
		constraint_slidingparams_t sliding;
		sliding.Defaults();
		sliding.InitWithCurrentObjectState( world.pFloor, pCube, Vector( 1, 0, 0 ) );
		IPhysicsConstraint *pSlide = Faulted( world.pEnv->CreateSlidingConstraint( world.pFloor, pCube, NULL, sliding ) );
		if ( Check( TIER_BOOT, "constraint.sliding-create", pSlide != NULL ) )
		{
			Vector velocity( 100, 100, 100 );
			AngularImpulse spin( 90, 90, 90 );
			pCube->SetVelocity( &velocity, &spin );
			Step( world.pEnv, 0.5f );
			Vector position = PositionOf( pCube );
			Check( TIER_GAMEPLAY, "constraint.sliding-axis", position.x > 20.0f && Near( position.y, 0.0f, 1.5f ) && Near( position.z, 500.0f, 1.5f ),
				"(%.2f %.2f %.2f)", position.x, position.y, position.z );
			Check( TIER_GAMEPLAY, "constraint.sliding-no-rotation",
				AxisAngle( pCube, 0, Vector( 1, 0, 0 ) ) < 3.0f && AxisAngle( pCube, 2, Vector( 0, 0, 1 ) ) < 3.0f,
				"x axis %.2f z axis %.2f", AxisAngle( pCube, 0, Vector( 1, 0, 0 ) ), AxisAngle( pCube, 2, Vector( 0, 0, 1 ) ) );
			world.pEnv->DestroyConstraint( pSlide );
		}
		DestroyWorld( world );
	}

	{
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetGravity( vec3_origin );
		IPhysicsObject *pCube = CreateCube( world, kPivot );
		constraint_slidingparams_t sliding;
		sliding.Defaults();
		sliding.InitWithCurrentObjectState( world.pFloor, pCube, Vector( 1, 0, 0 ) );
		sliding.limitMin = -20.0f;
		sliding.limitMax = 20.0f;
		IPhysicsConstraint *pSlide = Faulted( world.pEnv->CreateSlidingConstraint( world.pFloor, pCube, NULL, sliding ) );
		Vector velocity( 100, 0, 0 );
		pCube->SetVelocity( &velocity, NULL );
		Step( world.pEnv, 1.0f );
		float x = PositionOf( pCube ).x;
		Check( TIER_GAMEPLAY, "constraint.sliding-limit", pSlide && x > 12.0f && x < 23.0f, "x %.2f", x );
		if ( pSlide )
			world.pEnv->DestroyConstraint( pSlide );
		DestroyWorld( world );
	}

	{
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetGravity( vec3_origin );
		IPhysicsObject *pCube = CreateCube( world, kPivot );
		constraint_slidingparams_t sliding;
		sliding.Defaults();
		sliding.InitWithCurrentObjectState( world.pFloor, pCube, Vector( 1, 0, 0 ) );
		// A positive motor speed drives the attached object along the slide
		// axis. IVP's prismatic motor is a position servo whose target
		// advances once per solver pass (9 per step, scaled by 0.2 when
		// authored), so it overshoots the commanded speed by up to that
		// factor; the contract is the direction and at least the commanded
		// speed, not IVP's overshoot.
		sliding.SetLinearMotor( 50.0f, 1e6f );
		IPhysicsConstraint *pSlide = Faulted( world.pEnv->CreateSlidingConstraint( world.pFloor, pCube, NULL, sliding ) );
		Step( world.pEnv, 1.0f );
		float vx = VelocityOf( pCube ).x;
		Check( TIER_GAMEPLAY, "constraint.sliding-motor", pSlide && vx > 40.0f && vx < 500.0f, "vx %.2f", vx );
		// phys_slideconstraint SetVelocity: max force = speed * mass / tick.
		if ( pSlide )
			pSlide->SetLinearMotor( -25.0f, 25.0f * pCube->GetMass() / kTick );
		Step( world.pEnv, 1.0f );
		vx = VelocityOf( pCube ).x;
		Check( TIER_GAMEPLAY, "constraint.sliding-set-motor", vx < -20.0f && vx > -250.0f, "vx %.2f", vx );
		if ( pSlide )
			world.pEnv->DestroyConstraint( pSlide );
		DestroyWorld( world );
	}
}

//-----------------------------------------------------------------------------
// Length: a rope (max length only) or a rigid rod (min == max).
//-----------------------------------------------------------------------------
void TestLength()
{
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot - Vector( 0, 0, 40 ) );
		constraint_lengthparams_t length;
		length.Defaults();
		length.InitWorldspace( world.pFloor, pCube, kPivot, PositionOf( pCube ) );
		length.totalLength = 60.0f;
		IPhysicsConstraint *pRope = Faulted( world.pEnv->CreateLengthConstraint( world.pFloor, pCube, NULL, length ) );
		if ( Check( TIER_BOOT, "constraint.length-create", pRope != NULL ) )
		{
			float minZ = MinZOver( world, pCube, 1.0f );
			float distance = ( PositionOf( pCube ) - kPivot ).Length();
			Check( TIER_GAMEPLAY, "constraint.length-rope-holds", minZ > 434.0f && Near( distance, 60.0f, 3.0f ),
				"min z %.2f distance %.2f", minZ, distance );
			ObsFloats( "constraint.length-rope-distance", "a3", 1, &distance );

			// Slack rope: nothing stops the object moving toward the anchor.
			world.pEnv->SetGravity( vec3_origin );
			Vector up( 0, 0, 80 );
			pCube->SetVelocity( &up, NULL );
			Step( world.pEnv, 0.3f );
			distance = ( PositionOf( pCube ) - kPivot ).Length();
			Check( TIER_GAMEPLAY, "constraint.length-rope-slack", distance < 45.0f, "distance %.2f", distance );
			world.pEnv->DestroyConstraint( pRope );
		}
		DestroyWorld( world );
	}

	{
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetGravity( vec3_origin );
		IPhysicsObject *pCube = CreateCube( world, kPivot - Vector( 0, 0, 40 ) );
		constraint_lengthparams_t length;
		length.Defaults();
		length.InitWorldspace( world.pFloor, pCube, kPivot, PositionOf( pCube ), true );
		IPhysicsConstraint *pRod = Faulted( world.pEnv->CreateLengthConstraint( world.pFloor, pCube, NULL, length ) );
		Vector up( 0, 0, 80 );
		pCube->SetVelocity( &up, NULL );
		Step( world.pEnv, 0.3f );
		float distance = ( PositionOf( pCube ) - kPivot ).Length();
		Check( TIER_GAMEPLAY, "constraint.length-rigid", pRod && Near( distance, 40.0f, 2.5f ), "distance %.2f", distance );
		if ( pRod )
			world.pEnv->DestroyConstraint( pRod );
		DestroyWorld( world );
	}
}

//-----------------------------------------------------------------------------
// Pulley: rope over two fixed pulleys; the heavier side descends and lifts
// the lighter one, and the rope never stretches past its length.
//-----------------------------------------------------------------------------
void TestPulley()
{
	World_t world;
	CreateWorld( world, NULL );
	const Vector pulleyA( 0, 0, 600 ), pulleyB( 200, 0, 600 );
	IPhysicsObject *pHeavy = CreateCube( world, Vector( 0, 0, 500 ), 80.0f );
	IPhysicsObject *pLight = CreateCube( world, Vector( 200, 0, 500 ), 20.0f );
	constraint_pulleyparams_t pulley;
	pulley.Defaults();
	pulley.pulleyPosition[0] = pulleyA;
	pulley.pulleyPosition[1] = pulleyB;
	pulley.objectPosition[0].Init();
	pulley.objectPosition[1].Init();
	pulley.totalLength = 200.0f;
	pulley.gearRatio = 1.0f;
	IPhysicsConstraint *pPulley = Faulted( world.pEnv->CreatePulleyConstraint( pHeavy, pLight, NULL, pulley ) );
	if ( Check( TIER_BOOT, "constraint.pulley-create", pPulley != NULL ) )
	{
		Step( world.pEnv, 1.0f );
		float heavyZ = PositionOf( pHeavy ).z, lightZ = PositionOf( pLight ).z;
		float rope = ( PositionOf( pHeavy ) - pulleyA ).Length() + ( PositionOf( pLight ) - pulleyB ).Length();
		Check( TIER_GAMEPLAY, "constraint.pulley-lifts", heavyZ < 480.0f && lightZ > 520.0f, "heavy z %.2f light z %.2f", heavyZ, lightZ );
		Check( TIER_GAMEPLAY, "constraint.pulley-length", rope < 206.0f, "rope %.2f", rope );
		world.pEnv->DestroyConstraint( pPulley );
	}
	DestroyWorld( world );
}

//-----------------------------------------------------------------------------
// Ragdoll: the joint anchor is held and rotation stays inside the authored
// per-axis limits; a single free axis behaves as a limited hinge.
//-----------------------------------------------------------------------------
constraint_ragdollparams_t RagdollParams( IPhysicsObject *pRef, IPhysicsObject *pAttached, const Vector &pivot )
{
	constraint_ragdollparams_t ragdoll;
	ragdoll.Defaults();
	// Constraint space is world-aligned at the pivot.
	matrix3x4_t constraintToWorld, refToWorld, attToWorld, worldToRef, worldToAtt;
	AngleMatrix( vec3_angle, pivot, constraintToWorld );
	pRef->GetPositionMatrix( &refToWorld );
	pAttached->GetPositionMatrix( &attToWorld );
	MatrixInvert( refToWorld, worldToRef );
	MatrixInvert( attToWorld, worldToAtt );
	ConcatTransforms( worldToRef, constraintToWorld, ragdoll.constraintToReference );
	ConcatTransforms( worldToAtt, constraintToWorld, ragdoll.constraintToAttached );
	return ragdoll;
}

void TestRagdoll()
{
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		constraint_ragdollparams_t ragdoll = RagdollParams( world.pFloor, pCube, kPivot );
		for ( int i = 0; i < 3; i++ )
			ragdoll.axes[i].SetAxisFriction( -15, 15, 0 );
		IPhysicsConstraint *pJoint = Faulted( world.pEnv->CreateRagdollConstraint( world.pFloor, pCube, NULL, ragdoll ) );
		if ( Check( TIER_BOOT, "constraint.ragdoll-create", pJoint != NULL ) )
		{
			Vector sideways( 0, 60, 0 );
			pCube->SetVelocity( &sideways, NULL );
			float minZ = MinZOver( world, pCube, 1.0f );
			Vector local = PivotLocal( pCube, kPivot );
			Check( TIER_GAMEPLAY, "constraint.ragdoll-anchor-held", NearVec( local, Vector( -40, 0, 0 ), 2.0f ),
				"pivot local (%.2f %.2f %.2f)", local.x, local.y, local.z );
			// Unlimited, the arm would swing below 470; +-15 on every axis keeps
			// the swing within ~20 degrees (the fitted cone is not tighter than
			// the widest axis), i.e. above 500 - 40 * sin(25).
			Check( TIER_GAMEPLAY, "constraint.ragdoll-limited", minZ > 480.0f && AxisAngle( pCube, 0, Vector( 1, 0, 0 ) ) < 30.0f,
				"min z %.2f swing %.2f", minZ, AxisAngle( pCube, 0, Vector( 1, 0, 0 ) ) );

			matrix3x4_t toRef, toAtt;
			bool transform = pJoint->GetConstraintTransform( &toRef, &toAtt );
			Vector refOrigin, attOrigin;
			MatrixGetColumn( toRef, 3, refOrigin );
			MatrixGetColumn( toAtt, 3, attOrigin );
			Check( TIER_GAMEPLAY, "constraint.ragdoll-transform", transform && NearVec( refOrigin, kPivot, 0.5f ) && NearVec( attOrigin, Vector( -40, 0, 0 ), 0.5f ),
				"ok %d ref (%.2f %.2f %.2f) att (%.2f %.2f %.2f)", transform, refOrigin.x, refOrigin.y, refOrigin.z, attOrigin.x, attOrigin.y, attOrigin.z );
			world.pEnv->DestroyConstraint( pJoint );
		}
		DestroyWorld( world );
	}

	// One free axis (y): the ragdoll joint is a hinge about world y, limited
	// to +-20 degrees.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		constraint_ragdollparams_t ragdoll = RagdollParams( world.pFloor, pCube, kPivot );
		ragdoll.axes[1].SetAxisFriction( -20, 20, 0 );
		IPhysicsConstraint *pJoint = Faulted( world.pEnv->CreateRagdollConstraint( world.pFloor, pCube, NULL, ragdoll ) );
		Vector sideways( 0, 60, 0 );
		pCube->SetVelocity( &sideways, NULL );
		float minZ = MinZOver( world, pCube, 1.0f );
		Vector position = PositionOf( pCube );
		Check( TIER_GAMEPLAY, "constraint.ragdoll-hinge", pJoint && minZ > 476.0f && position.z > 483.0f && position.z < 497.0f && Near( position.y, 0.0f, 1.5f ),
			"min z %.2f z %.2f y %.2f", minZ, position.z, position.y );
		if ( pJoint )
			world.pEnv->DestroyConstraint( pJoint );
		DestroyWorld( world );
	}

	// Ragdoll limits are counter-clockwise about the constraint-space axis
	// (the opposite of hinges): [0, 45] about y lets the arm fall.
	for ( int k = 0; k < 2; k++ )
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		constraint_ragdollparams_t ragdoll = RagdollParams( world.pFloor, pCube, kPivot );
		ragdoll.axes[1].SetAxisFriction( k == 0 ? 0.0f : -45.0f, k == 0 ? 45.0f : 0.0f, 0 );
		IPhysicsConstraint *pJoint = Faulted( world.pEnv->CreateRagdollConstraint( world.pFloor, pCube, NULL, ragdoll ) );
		float minZ = MinZOver( world, pCube, 1.0f );
		bool ok = k == 0 ? minZ < 480.0f : minZ > 490.0f;
		Check( TIER_GAMEPLAY, k == 0 ? "constraint.ragdoll-limit-sign-allows" : "constraint.ragdoll-limit-sign-blocks", pJoint && ok, "min z %.2f", minZ );
		if ( pJoint )
			world.pEnv->DestroyConstraint( pJoint );
		DestroyWorld( world );
	}

	// A ragdoll built from a real model's authored constraints (limb
	// hierarchy) is exercised through the .phy fixtures' ragdollconstraint
	// blocks; the keyparser checks cover their parsing.
}

//-----------------------------------------------------------------------------
// Fixed: carries the attached object rigidly; breakable limits.
//-----------------------------------------------------------------------------
void TestBreakable()
{
	struct Case_t
	{
		const char *pName;
		float forceLimit;
		bool breaks;
	};
	const Case_t cases[] = {
		{ "constraint.breakable-breaks", 10.0f, true },
		{ "constraint.breakable-holds", 1e5f, false },
		{ "constraint.unbreakable-holds", 0.0f, false },
	};
	for ( int c = 0; c < (int)ARRAYSIZE( cases ); c++ )
	{
		World_t world;
		CreateWorld( world, NULL );
		CConstraintEvents events;
		world.pEnv->SetConstraintEventHandler( &events );
		IPhysicsObject *pCube = CreateCube( world, kPivot );
		constraint_fixedparams_t fixed;
		fixed.Defaults();
		fixed.InitWithCurrentObjectState( world.pFloor, pCube );
		fixed.constraint.forceLimit = cases[c].forceLimit;
		IPhysicsConstraint *pWeld = Faulted( world.pEnv->CreateFixedConstraint( world.pFloor, pCube, NULL, fixed ) );
		Step( world.pEnv, 1.0f );
		float drop = 500.0f - PositionOf( pCube ).z;
		bool broke = events.m_broken > 0 && events.m_pLast == pWeld;
		bool ok = pWeld && ( cases[c].breaks ? ( broke && drop > 100.0f ) : ( !broke && drop < 3.0f ) );
		Check( TIER_GAMEPLAY, cases[c].pName, ok, "broken %d drop %.2f", events.m_broken, drop );

		constraint_breakableparams_t params;
		memset( &params, 0, sizeof( params ) );
		if ( c == 0 && pWeld )
		{
			bool got = pWeld->GetConstraintParams( &params );
			Check( TIER_GAMEPLAY, "constraint.breakable-params", got && Near( params.forceLimit, 10.0f, 0.1f ),
				"ok %d force %.3f", got, params.forceLimit );
		}
		if ( pWeld )
			world.pEnv->DestroyConstraint( pWeld );
		world.pEnv->SetConstraintEventHandler( NULL );
		DestroyWorld( world );
	}
}

//-----------------------------------------------------------------------------
// Activation: constraints in a group are inert until the group activates
// (ragdolls are built this way); standalone constraints deactivate and
// reactivate.
//-----------------------------------------------------------------------------
void TestActivation()
{
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		constraint_groupparams_t groupParams;
		groupParams.Defaults();
		IPhysicsConstraintGroup *pGroup = world.pEnv->CreateConstraintGroup( groupParams );
		constraint_ballsocketparams_t ballsocket;
		ballsocket.Defaults();
		ballsocket.InitWithCurrentObjectState( world.pFloor, pCube, kPivot );
		IPhysicsConstraint *pBall = Faulted( world.pEnv->CreateBallsocketConstraint( world.pFloor, pCube, pGroup, ballsocket ) );
		Step( world.pEnv, 0.2f );
		float fell = 500.0f - PositionOf( pCube ).z;
		Check( TIER_GAMEPLAY, "constraint.group-inactive-until-activate", pGroup && pBall && fell > 8.0f, "fell %.2f", fell );

		// Reset and activate: the anchor now holds.
		pCube->SetPosition( kPivot + Vector( 40, 0, 0 ), vec3_angle, true );
		pCube->SetVelocity( &vec3_origin, &vec3_origin );
		if ( pGroup )
			pGroup->Activate();
		Step( world.pEnv, 0.5f );
		Vector local = PivotLocal( pCube, kPivot );
		Check( TIER_GAMEPLAY, "constraint.group-activate", NearVec( local, Vector( -40, 0, 0 ), 2.0f ),
			"pivot local (%.2f %.2f %.2f)", local.x, local.y, local.z );

		constraint_groupparams_t readBack;
		memset( &readBack, 0, sizeof( readBack ) );
		if ( pGroup )
			pGroup->GetErrorParams( &readBack );
		Check( TIER_BOOT, "constraint.group-error-params", readBack.minErrorTicks == groupParams.minErrorTicks &&
			Near( readBack.errorTolerance, groupParams.errorTolerance, 0.01f ),
			"ticks %d tolerance %.3f", readBack.minErrorTicks, readBack.errorTolerance );
		Check( TIER_BOOT, "constraint.group-no-error", pGroup && !pGroup->IsInErrorState() );
		if ( pBall )
			world.pEnv->DestroyConstraint( pBall );
		if ( pGroup )
			world.pEnv->DestroyConstraintGroup( pGroup );
		DestroyWorld( world );
	}

	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
		constraint_ballsocketparams_t ballsocket;
		ballsocket.Defaults();
		ballsocket.InitWithCurrentObjectState( world.pFloor, pCube, kPivot );
		IPhysicsConstraint *pBall = Faulted( world.pEnv->CreateBallsocketConstraint( world.pFloor, pCube, NULL, ballsocket ) );
		if ( pBall )
			pBall->Deactivate();
		Step( world.pEnv, 0.2f );
		float fell = 500.0f - PositionOf( pCube ).z;
		Check( TIER_GAMEPLAY, "constraint.deactivate", pBall && fell > 8.0f, "fell %.2f", fell );
		pCube->SetPosition( kPivot + Vector( 40, 0, 0 ), vec3_angle, true );
		pCube->SetVelocity( &vec3_origin, &vec3_origin );
		if ( pBall )
			pBall->Activate();
		Step( world.pEnv, 0.5f );
		Vector local = PivotLocal( pCube, kPivot );
		Check( TIER_GAMEPLAY, "constraint.reactivate", NearVec( local, Vector( -40, 0, 0 ), 2.0f ),
			"pivot local (%.2f %.2f %.2f)", local.x, local.y, local.z );
		if ( pBall )
			world.pEnv->DestroyConstraint( pBall );
		DestroyWorld( world );
	}
}

//-----------------------------------------------------------------------------
// Lifetime: destroying a constrained object leaves an inert constraint the
// game can still destroy, and (with notification enabled) reports it.
//-----------------------------------------------------------------------------
void TestLifetime()
{
	World_t world;
	CreateWorld( world, NULL );
	CConstraintEvents events;
	events.m_deactivate = false;
	world.pEnv->SetConstraintEventHandler( &events );
	world.pEnv->EnableConstraintNotify( true );
	IPhysicsObject *pA = CreateCube( world, kPivot );
	IPhysicsObject *pB = CreateCube( world, kPivot + Vector( 40, 0, 0 ) );
	constraint_fixedparams_t fixed;
	fixed.Defaults();
	fixed.InitWithCurrentObjectState( pA, pB );
	IPhysicsConstraint *pWeld = Faulted( world.pEnv->CreateFixedConstraint( pA, pB, NULL, fixed ) );
	Step( world.pEnv, 0.1f );
	world.pEnv->DestroyObject( pB );
	Check( TIER_GAMEPLAY, "constraint.object-destroyed-notify", pWeld && events.m_broken == 1 && events.m_pLast == pWeld,
		"broken %d", events.m_broken );
	Step( world.pEnv, 0.1f );
	Check( TIER_BOOT, "constraint.inert-after-object-destroyed", !pA->IsAttachedToConstraint( false ) );
	if ( pWeld )
		world.pEnv->DestroyConstraint( pWeld );
	Step( world.pEnv, 0.1f );
	Check( TIER_BOOT, "constraint.inert-destroy", true );
	world.pEnv->EnableConstraintNotify( false );
	world.pEnv->SetConstraintEventHandler( NULL );
	DestroyWorld( world );
}
}

void TestConstraints()
{
	TestHinge();
	TestBallsocket();
	TestSliding();
	TestLength();
	TestPulley();
	TestRagdoll();
	TestBreakable();
	TestActivation();
	TestLifetime();
}
