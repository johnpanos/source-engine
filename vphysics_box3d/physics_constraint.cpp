//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D constraints and constraint groups (RFC 0004 C2). See
//          physics_constraint.h for the IVP/Havok -> Box3D mapping.
//
//=============================================================================//
#include "physics_constraint.h"

#include <math.h>
#include <string.h>

#include "box3d/box3d.h"
#include "box3d_convert.h"
#include "physics_environment.h"
#include "physics_object.h"
#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
// Havok N*m (kg*m^2/s^2) -> Box3D torque in Source units (kg*in^2/s^2).
const float kTorqueToBox3D = kBox3DInchesPerMeter * kBox3DInchesPerMeter;
// Havok angular momentum (kg*m^2/s) per Box3D angular impulse (kg*in^2/s).
const float kAngularImpulseToHavok = 1.0f / kTorqueToBox3D;
// hk_Breakable_Constraint treats limits at or above this as unbreakable.
const float kUnbreakableLimit = 1e12f;
// Box3D revolute/twist limits must stay inside (-pi, pi).
const float kMaxJointAngle = 0.99f * M_PI_F;

bool IsBreakable( const constraint_breakableparams_t &constraint )
{
	// Same test as IVP's IsBreakableConstraint (vphysics/physics_constraint.cpp).
	return ( constraint.forceLimit != 0 && constraint.forceLimit < kUnbreakableLimit ) ||
		( constraint.torqueLimit != 0 && constraint.torqueLimit < kUnbreakableLimit ) ||
		( constraint.bodyMassScale[0] != 1.0f && constraint.bodyMassScale[0] != 0.0f ) ||
		( constraint.bodyMassScale[1] != 1.0f && constraint.bodyMassScale[1] != 0.0f );
}

b3Quat QuatFromMatrix( const matrix3x4_t &matrix )
{
	Quaternion q;
	MatrixQuaternion( matrix, q );
	b3Quat out = { { q.x, q.y, q.z }, q.w };
	return b3NormalizeQuat( out );
}

b3Transform TransformFromMatrix( const matrix3x4_t &matrix )
{
	b3Transform out;
	Vector origin;
	MatrixGetColumn( matrix, 3, origin );
	out.p = ToB3( origin );
	out.q = QuatFromMatrix( matrix );
	return out;
}

void MatrixFromTransform( const b3Transform &transform, matrix3x4_t &out )
{
	Quaternion q( transform.q.v.x, transform.q.v.y, transform.q.v.z, transform.q.s );
	QuaternionMatrix( q, FromB3( transform.p ), out );
}

// Rotation whose columns are the given orthonormal basis.
b3Quat QuatFromBasis( const Vector &x, const Vector &y, const Vector &z )
{
	matrix3x4_t matrix;
	MatrixSetColumn( x, 0, matrix );
	MatrixSetColumn( y, 1, matrix );
	MatrixSetColumn( z, 2, matrix );
	MatrixSetColumn( vec3_origin, 3, matrix );
	return QuatFromMatrix( matrix );
}

// Rotation taking frame z to the given unit axis (x, y any perpendicular).
b3Quat QuatWithZ( const Vector &axis )
{
	Vector z = axis;
	VectorNormalize( z );
	Vector x, y;
	VectorVectors( z, x, y );
	// VectorVectors returns right/up with right x up = -forward; make it
	// right-handed with z.
	Vector yy = CrossProduct( z, x );
	return QuatFromBasis( x, yy, z );
}

// Rotation taking frame x to the given unit axis.
b3Quat QuatWithX( const Vector &axis )
{
	Vector x = axis;
	VectorNormalize( x );
	Vector y, z;
	VectorVectors( x, y, z );
	Vector zz = CrossProduct( x, y );
	return QuatFromBasis( x, y, zz );
}

b3Transform BodyTransform( CPhysicsObjectBox3D *pObject )
{
	return b3Body_GetTransform( pObject->GetBody() );
}

// Local frame on an object for a world-space frame.
b3Transform LocalFrame( CPhysicsObjectBox3D *pObject, const b3Transform &world )
{
	return b3MulTransforms( b3InvertTransform( BodyTransform( pObject ) ), world );
}

// Constraint-space rotation that puts constraint axis 'axis' on frame z,
// keeping the cyclic order right-handed: x = e(axis+1), y = e(axis+2).
b3Quat CyclicToZ( int axis )
{
	Vector e[3] = { Vector( 1, 0, 0 ), Vector( 0, 1, 0 ), Vector( 0, 0, 1 ) };
	return QuatFromBasis( e[( axis + 1 ) % 3], e[( axis + 2 ) % 3], e[axis] );
}

b3Quat AxisRotation( int axis, float radians )
{
	b3Vec3 v = b3Vec3_zero;
	( &v.x )[axis] = 1.0f;
	return b3MakeQuatFromAxisAngle( v, radians );
}

float ClampAngle( float radians )
{
	return clamp( radians, -kMaxJointAngle, kMaxJointAngle );
}
}

//-----------------------------------------------------------------------------
// Construction
//-----------------------------------------------------------------------------
CConstraintBox3D::CConstraintBox3D( CPhysicsEnvironmentBox3D *pEnv, IPhysicsObject *pReference, IPhysicsObject *pAttached,
	CConstraintGroupBox3D *pGroup, ConstraintTypeBox3D_t type, const constraint_breakableparams_t &breakable )
	: m_pEnv( pEnv ), m_pReference( ToBox3D( pReference ) ), m_pAttached( ToBox3D( pAttached ) ), m_pGroup( pGroup ),
	  m_pGameData( NULL ), m_type( type ), m_breakable( breakable ), m_active( breakable.isActive ), m_jointCount( 0 ),
	  m_limitEnabled( false ), m_limitLower( 0.0f ), m_limitUpper( 0.0f ), m_motorEnabled( false ), m_motorSpeed( 0.0f ),
	  m_motorMax( 0.0f ), m_coneEnabled( false ), m_coneAngle( 0.0f ), m_twistEnabled( false ), m_totalLength( 0.0f ),
	  m_minLength( 0.0f ), m_gearRatio( 1.0f ), m_isRigid( false ), m_pulleyImpulse( 0.0f ), m_slideFriction( 0.0f ),
	  m_slideVelocity( 0.0f )
{
	m_joints[0] = m_joints[1] = b3_nullJointId;
	m_frameRef = b3Transform_identity;
	m_frameAtt = b3Transform_identity;
	m_ragdoll.Defaults();
	m_objectPosition[0].Init();
	m_objectPosition[1].Init();
	m_pulleyPosition[0].Init();
	m_pulleyPosition[1].Init();
	m_slideAxisRef.Init( 1, 0, 0 );
	m_hingeAxis.Defaults();
	if ( m_pGroup )
		m_pGroup->AddConstraint( this );
}

CConstraintBox3D::~CConstraintBox3D()
{
	DestroyJoints();
	if ( m_pGroup )
		m_pGroup->RemoveConstraint( this );
}

void CConstraintBox3D::InitHinge( const constraint_limitedhingeparams_t &hinge )
{
	m_type = CONSTRAINT_BOX3D_HINGE;
	m_hingeAxis = hinge.hingeAxis;
	// IVP measures the hinge clockwise about the authored axis: build the
	// Box3D frame (counter-clockwise about z) on the negated axis so limits
	// and motor speeds carry over unchanged.
	b3Transform world;
	world.p = ToB3( hinge.worldPosition );
	world.q = QuatWithZ( -hinge.worldAxisDirection );
	m_frameRef = LocalFrame( m_pReference, world );
	m_frameAtt = LocalFrame( m_pAttached, world );

	const constraint_axislimit_t &axis = hinge.hingeAxis;
	if ( axis.minRotation != axis.maxRotation && axis.maxRotation - axis.minRotation < 360.0f )
	{
		m_limitEnabled = true;
		m_limitLower = ClampAngle( DEG2RAD( axis.minRotation ) );
		m_limitUpper = ClampAngle( DEG2RAD( axis.maxRotation ) );
	}
	// IVP enables the hinge motor only with a nonzero torque (friction).
	if ( axis.torque != 0 )
	{
		m_motorEnabled = true;
		m_motorSpeed = DEG2RAD( axis.angularVelocity );
		m_motorMax = DEG2RAD( fabsf( axis.torque ) ) * kTorqueToBox3D;
	}
	SyncJoints();
}

void CConstraintBox3D::InitRagdoll( const constraint_ragdollparams_t &ragdoll )
{
	m_ragdoll = ragdoll;
	int dof = 0, dofIndex = 0;
	for ( int i = 0; i < 3; i++ )
	{
		if ( ragdoll.axes[i].minRotation != ragdoll.axes[i].maxRotation )
		{
			dofIndex = i;
			dof++;
		}
	}

	if ( dof == 1 )
	{
		// IVP converts a single-axis ragdoll joint into a limited hinge about
		// that constraint axis (ConvertRagdollToHinge); ragdoll angles are
		// counter-clockwise, so the Box3D frame uses the axis directly.
		m_type = CONSTRAINT_BOX3D_HINGE;
		b3Quat cyclic = CyclicToZ( dofIndex );
		b3Transform toRef = TransformFromMatrix( ragdoll.constraintToReference );
		b3Transform toAtt = TransformFromMatrix( ragdoll.constraintToAttached );
		m_frameRef.p = toRef.p;
		m_frameRef.q = b3MulQuat( toRef.q, cyclic );
		m_frameAtt.p = toAtt.p;
		m_frameAtt.q = b3MulQuat( toAtt.q, cyclic );

		const constraint_axislimit_t &axis = ragdoll.axes[dofIndex];
		float lower = axis.minRotation, upper = axis.maxRotation;
		if ( ragdoll.useClockwiseRotations )
		{
			lower = -axis.maxRotation;
			upper = -axis.minRotation;
		}
		m_limitEnabled = upper - lower < 360.0f;
		m_limitLower = ClampAngle( DEG2RAD( lower ) );
		m_limitUpper = ClampAngle( DEG2RAD( upper ) );
		// The converted hinge's motor runs clockwise about the axis.
		if ( axis.torque != 0 )
		{
			m_motorEnabled = true;
			m_motorSpeed = -DEG2RAD( axis.angularVelocity );
			m_motorMax = fabsf( axis.torque ) * m_pReference->GetMass() * kTorqueToBox3D;
		}
		m_hingeAxis.minRotation = RAD2DEG( m_limitLower );
		m_hingeAxis.maxRotation = RAD2DEG( m_limitUpper );
		m_hingeAxis.angularVelocity = RAD2DEG( m_motorSpeed );
		m_hingeAxis.torque = RAD2DEG( m_motorMax / kTorqueToBox3D );
		if ( !ragdoll.isActive )
			m_active = false;
		SyncJoints();
		return;
	}

	m_type = CONSTRAINT_BOX3D_RAGDOLL;
	if ( !ragdoll.isActive )
		m_active = false;
	SetupRagdollFit( ragdoll );
	SyncJoints();
}

// Havok's hk_Ragdoll_Constraint_BP_Builder: the twist axis is the constraint
// axis about which the pair rotates most easily; the wider of the other two
// axes becomes the cone (centered on its limit midpoint); twist limits come
// from the twist axis.
void CConstraintBox3D::SetupRagdollFit( const constraint_ragdollparams_t &ragdoll )
{
	float minRot[3], maxRot[3];
	for ( int i = 0; i < 3; i++ )
	{
		minRot[i] = DEG2RAD( ragdoll.useClockwiseRotations ? -ragdoll.axes[i].maxRotation : ragdoll.axes[i].minRotation );
		maxRot[i] = DEG2RAD( ragdoll.useClockwiseRotations ? -ragdoll.axes[i].minRotation : ragdoll.axes[i].maxRotation );
	}

	b3Transform toRef = TransformFromMatrix( ragdoll.constraintToReference );
	b3Transform toAtt = TransformFromMatrix( ragdoll.constraintToAttached );
	b3Transform refWorld = BodyTransform( m_pReference );
	b3Transform attWorld = BodyTransform( m_pAttached );
	b3Vec3 pivot = b3TransformPoint( refWorld, toRef.p );
	b3Vec3 refArm = b3Sub( pivot, b3Body_GetWorldCenter( m_pReference->GetBody() ) );
	b3Vec3 attArm = b3Sub( pivot, b3Body_GetWorldCenter( m_pAttached->GetBody() ) );
	float refInvMass = DynamicInvMass( m_pReference );
	float attInvMass = DynamicInvMass( m_pAttached );
	b3Matrix3 refInvI = refInvMass > 0 ? b3Body_GetWorldInverseRotationalInertia( m_pReference->GetBody() ) : b3Matrix3{ b3Vec3_zero, b3Vec3_zero, b3Vec3_zero };
	b3Matrix3 attInvI = attInvMass > 0 ? b3Body_GetWorldInverseRotationalInertia( m_pAttached->GetBody() ) : b3Matrix3{ b3Vec3_zero, b3Vec3_zero, b3Vec3_zero };

	int twist = 0;
	float bestInv = -1.0f;
	for ( int i = 0; i < 3; i++ )
	{
		b3Vec3 unit = b3Vec3_zero;
		( &unit.x )[i] = 1.0f;
		b3Vec3 axis = b3RotateVector( refWorld.q, b3RotateVector( toRef.q, unit ) );
		float inv = b3Dot( axis, b3MulMV( refInvI, axis ) ) + b3Dot( axis, b3MulMV( attInvI, axis ) );
		inv += b3LengthSquared( b3Cross( refArm, axis ) ) * refInvMass;
		inv += b3LengthSquared( b3Cross( attArm, axis ) ) * attInvMass;
		if ( inv > bestInv )
		{
			bestInv = inv;
			twist = i;
		}
	}
	int lower = ( twist + 1 ) % 3;
	int upper = ( twist + 2 ) % 3;
	if ( maxRot[lower] - minRot[lower] > maxRot[upper] - minRot[upper] )
	{
		int swap = lower;
		lower = upper;
		upper = swap;
	}

	float coneRange = maxRot[upper] - minRot[upper];
	float coneMid = 0.5f * ( maxRot[upper] + minRot[upper] );
	if ( coneRange <= 0.0f )
		coneRange = 0.2f;	// Havok's no-freedom fallback (+-0.1 cone)
	m_coneEnabled = true;
	m_coneAngle = clamp( 0.5f * coneRange, 0.0f, M_PI_F );
	m_twistEnabled = maxRot[twist] - minRot[twist] < 2.0f * M_PI_F;
	m_limitLower = ClampAngle( minRot[twist] );
	m_limitUpper = ClampAngle( maxRot[twist] );

	b3Quat cyclic = CyclicToZ( twist );
	// Frame A is turned about the cone axis to the cone's midpoint.
	m_frameRef.p = toRef.p;
	m_frameRef.q = b3MulQuat( b3MulQuat( toRef.q, AxisRotation( upper, coneMid ) ), cyclic );
	m_frameAtt.p = toAtt.p;
	m_frameAtt.q = b3MulQuat( toAtt.q, cyclic );

	// Per-axis friction (torque * reference mass, Havok N*m) becomes one
	// motor holding relative rotation, at the mean authored friction.
	float friction = 0.0f;
	int frictionAxes = 0;
	for ( int i = 0; i < 3; i++ )
	{
		if ( ragdoll.axes[i].torque > 0 )
		{
			friction += ragdoll.axes[i].torque;
			frictionAxes++;
		}
	}
	if ( frictionAxes )
	{
		m_motorEnabled = true;
		m_motorSpeed = 0.0f;
		m_motorMax = ( friction / frictionAxes ) * m_pReference->GetMass() * kTorqueToBox3D;
	}
}

void CConstraintBox3D::InitFixed( const constraint_fixedparams_t &fixed )
{
	m_type = CONSTRAINT_BOX3D_FIXED;
	// attachedRefXform places the attached object in the reference object's
	// space; body frames are object frames, so it is the weld's frame on A.
	m_frameRef = TransformFromMatrix( fixed.attachedRefXform );
	m_frameAtt = b3Transform_identity;
	SyncJoints();
}

void CConstraintBox3D::InitBallsocket( const constraint_ballsocketparams_t &ballsocket )
{
	m_type = CONSTRAINT_BOX3D_BALLSOCKET;
	m_frameRef.p = ToB3( ballsocket.constraintPosition[0] );
	m_frameRef.q = b3Quat_identity;
	m_frameAtt.p = ToB3( ballsocket.constraintPosition[1] );
	// Frames aligned at creation (only the anchor is constrained).
	m_frameAtt.q = b3InvMulQuat( BodyTransform( m_pAttached ).q, BodyTransform( m_pReference ).q );
	SyncJoints();
}

void CConstraintBox3D::InitSliding( const constraint_slidingparams_t &sliding )
{
	m_type = CONSTRAINT_BOX3D_SLIDING;
	m_slideAxisRef = sliding.slideAxisRef;
	VectorNormalize( m_slideAxisRef );
	b3Transform attachedToRef = TransformFromMatrix( sliding.attachedRefXform );
	b3Quat basis = QuatWithX( m_slideAxisRef );
	m_frameRef.p = attachedToRef.p;
	m_frameRef.q = basis;
	m_frameAtt.p = b3Vec3_zero;
	m_frameAtt.q = b3InvMulQuat( attachedToRef.q, basis );

	if ( sliding.limitMin != sliding.limitMax )
	{
		m_limitEnabled = true;
		m_limitLower = MIN( sliding.limitMin, sliding.limitMax );
		m_limitUpper = MAX( sliding.limitMin, sliding.limitMax );
	}
	m_slideFriction = sliding.friction;
	m_slideVelocity = sliding.velocity;
	if ( sliding.friction != 0 )
	{
		m_motorEnabled = true;
		m_motorSpeed = sliding.velocity;
		m_motorMax = fabsf( sliding.friction );
	}
	SyncJoints();
}

void CConstraintBox3D::InitPulley( const constraint_pulleyparams_t &pulley )
{
	m_type = CONSTRAINT_BOX3D_PULLEY;
	for ( int i = 0; i < 2; i++ )
	{
		m_pulleyPosition[i] = pulley.pulleyPosition[i];
		m_objectPosition[i] = pulley.objectPosition[i];
	}
	m_totalLength = pulley.totalLength;
	m_gearRatio = pulley.gearRatio > 0 ? pulley.gearRatio : 1.0f;
	m_isRigid = pulley.isRigid;
	SyncJoints();
}

void CConstraintBox3D::InitLength( const constraint_lengthparams_t &length )
{
	m_type = CONSTRAINT_BOX3D_LENGTH;
	m_objectPosition[0] = length.objectPosition[0];
	m_objectPosition[1] = length.objectPosition[1];
	m_totalLength = length.totalLength;
	m_minLength = length.minLength;
	m_frameRef.p = ToB3( length.objectPosition[0] );
	m_frameRef.q = b3Quat_identity;
	m_frameAtt.p = ToB3( length.objectPosition[1] );
	m_frameAtt.q = b3Quat_identity;
	SyncJoints();
}

//-----------------------------------------------------------------------------
// Box3D realization
//-----------------------------------------------------------------------------
bool CConstraintBox3D::IsSimulating() const
{
	if ( IsInert() )
		return false;
	return m_pGroup ? m_pGroup->IsActive() : m_active;
}

void CConstraintBox3D::SyncJoints()
{
	bool simulating = IsSimulating();
	if ( simulating && !m_jointCount && m_type != CONSTRAINT_BOX3D_PULLEY )
		CreateJoints();
	else if ( !simulating && m_jointCount )
		DestroyJoints();
}

void CConstraintBox3D::AddJoint( b3JointId joint )
{
	if ( !b3Joint_IsValid( joint ) || m_jointCount >= (int)ARRAYSIZE( m_joints ) )
		return;
	m_joints[m_jointCount++] = joint;
	b3Joint_WakeBodies( joint );
}

void CConstraintBox3D::SetupJointBase( b3JointDef &base )
{
	base.userData = this;
	base.bodyIdA = m_pReference->GetBody();
	base.bodyIdB = m_pAttached->GetBody();
	base.localFrameA = m_frameRef;
	base.localFrameB = m_frameAtt;
	// IVP constraints never disable collision between their objects; the
	// game's collision rules (pair hash, ragdoll collision sets) decide.
	base.collideConnected = true;
}

void CConstraintBox3D::CreateJoints()
{
	b3WorldId world = m_pEnv->GetWorld();
	switch ( m_type )
	{
	case CONSTRAINT_BOX3D_FIXED:
	{
		b3WeldJointDef def = b3DefaultWeldJointDef();
		SetupJointBase( def.base );
		AddJoint( b3CreateWeldJoint( world, &def ) );
		break;
	}
	case CONSTRAINT_BOX3D_BALLSOCKET:
	{
		b3SphericalJointDef def = b3DefaultSphericalJointDef();
		SetupJointBase( def.base );
		AddJoint( b3CreateSphericalJoint( world, &def ) );
		break;
	}
	case CONSTRAINT_BOX3D_RAGDOLL:
	{
		b3SphericalJointDef def = b3DefaultSphericalJointDef();
		SetupJointBase( def.base );
		def.enableConeLimit = m_coneEnabled;
		def.coneAngle = m_coneAngle;
		def.enableTwistLimit = m_twistEnabled;
		def.lowerTwistAngle = m_limitLower;
		def.upperTwistAngle = m_limitUpper;
		def.enableMotor = m_motorEnabled;
		def.maxMotorTorque = m_motorMax;
		def.motorVelocity = b3Vec3_zero;
		AddJoint( b3CreateSphericalJoint( world, &def ) );
		break;
	}
	case CONSTRAINT_BOX3D_HINGE:
	{
		b3RevoluteJointDef def = b3DefaultRevoluteJointDef();
		SetupJointBase( def.base );
		def.enableLimit = m_limitEnabled;
		def.lowerAngle = m_limitLower;
		def.upperAngle = m_limitUpper;
		def.enableMotor = m_motorEnabled;
		def.motorSpeed = m_motorSpeed;
		def.maxMotorTorque = m_motorMax;
		AddJoint( b3CreateRevoluteJoint( world, &def ) );
		break;
	}
	case CONSTRAINT_BOX3D_SLIDING:
	{
		b3PrismaticJointDef def = b3DefaultPrismaticJointDef();
		SetupJointBase( def.base );
		def.enableLimit = m_limitEnabled;
		def.lowerTranslation = m_limitLower;
		def.upperTranslation = m_limitUpper;
		def.enableMotor = m_motorEnabled;
		def.motorSpeed = m_motorSpeed;
		def.maxMotorForce = m_motorMax;
		AddJoint( b3CreatePrismaticJoint( world, &def ) );
		break;
	}
	case CONSTRAINT_BOX3D_LENGTH:
	{
		b3DistanceJointDef def = b3DefaultDistanceJointDef();
		SetupJointBase( def.base );
		def.length = m_totalLength;
		if ( m_minLength < m_totalLength )
		{
			// A rope: a zero-stiffness spring leaves only the length range.
			def.enableSpring = true;
			def.hertz = 0.0f;
			def.dampingRatio = 0.0f;
			def.enableLimit = true;
			def.minLength = m_minLength;
			def.maxLength = m_totalLength;
		}
		else
		{
			def.enableSpring = false;
		}
		AddJoint( b3CreateDistanceJoint( world, &def ) );
		break;
	}
	default:
		break;
	}
}

void CConstraintBox3D::DestroyJoints()
{
	for ( int i = 0; i < m_jointCount; i++ )
	{
		if ( b3Joint_IsValid( m_joints[i] ) )
			b3DestroyJoint( m_joints[i], true );
		m_joints[i] = b3_nullJointId;
	}
	m_jointCount = 0;
}

void CConstraintBox3D::ObjectDestroyed()
{
	// The Box3D joints die with the body; drop the handles first so nothing
	// touches them again, then become inert with no objects (as IVP does).
	for ( int i = 0; i < m_jointCount; i++ )
		m_joints[i] = b3_nullJointId;
	m_jointCount = 0;
	DestroyJoints();
	m_pReference = NULL;
	m_pAttached = NULL;
}

float CConstraintBox3D::DynamicInvMass( CPhysicsObjectBox3D *pObject ) const
{
	if ( !pObject || b3Body_GetType( pObject->GetBody() ) != b3_dynamicBody )
		return 0.0f;
	return b3Body_GetInverseMass( pObject->GetBody() );
}

//-----------------------------------------------------------------------------
// Pulley: velocity constraint on L0 + gear * L1 <= total, with Baumgarte
// feedback for the stretch accumulated during the previous step.
//-----------------------------------------------------------------------------
void CConstraintBox3D::PreStep( float dt )
{
	m_pulleyImpulse = 0.0f;
	if ( m_type != CONSTRAINT_BOX3D_PULLEY || !IsSimulating() || dt <= 0 )
		return;

	CPhysicsObjectBox3D *pObjects[2] = { m_pReference, m_pAttached };
	float scale[2] = { 1.0f, m_gearRatio };
	Vector point[2], direction[2];
	float length = 0.0f;
	for ( int i = 0; i < 2; i++ )
	{
		pObjects[i]->LocalToWorld( &point[i], m_objectPosition[i] );
		direction[i] = point[i] - m_pulleyPosition[i];
		float segment = VectorNormalize( direction[i] );
		length += scale[i] * segment;
	}
	float error = length - m_totalLength;
	if ( !m_isRigid && error < 0 )
		return;

	float cdot = 0.0f, k = 0.0f;
	for ( int i = 0; i < 2; i++ )
	{
		Vector velocity;
		pObjects[i]->GetVelocityAtPoint( point[i], &velocity );
		cdot += scale[i] * DotProduct( direction[i], velocity );
		float invMass = DynamicInvMass( pObjects[i] );
		if ( invMass <= 0 )
			continue;
		b3BodyId body = pObjects[i]->GetBody();
		b3Vec3 arm = b3Sub( ToB3( point[i] ), b3Body_GetWorldCenter( body ) );
		b3Vec3 rn = b3Cross( arm, ToB3( direction[i] ) );
		b3Matrix3 invI = b3Body_GetWorldInverseRotationalInertia( body );
		k += scale[i] * scale[i] * ( invMass + b3Dot( rn, b3MulMV( invI, rn ) ) );
	}
	if ( k <= 0 )
		return;

	float bias = 0.2f * error / dt;
	float lambda = -( cdot + bias ) / k;
	if ( !m_isRigid && lambda > 0 )
		lambda = 0;		// a rope only pulls
	if ( lambda == 0 )
		return;
	m_pulleyImpulse = fabsf( lambda );
	for ( int i = 0; i < 2; i++ )
	{
		if ( DynamicInvMass( pObjects[i] ) <= 0 )
			continue;
		Vector impulse = direction[i] * ( lambda * scale[i] );
		b3Body_ApplyLinearImpulse( pObjects[i]->GetBody(), ToB3( impulse ), ToB3( point[i] ), true );
	}
}

//-----------------------------------------------------------------------------
// Breaking (hk_Breakable_Constraint semantics)
//-----------------------------------------------------------------------------
bool CConstraintBox3D::CheckBreak( float dt ) const
{
	if ( !IsSimulating() || !IsBreakable( m_breakable ) )
		return false;

	float linearImpulse = m_pulleyImpulse;
	float angularImpulse = 0.0f;
	for ( int i = 0; i < m_jointCount; i++ )
	{
		if ( !b3Joint_IsValid( m_joints[i] ) )
			continue;
		linearImpulse += b3Length( b3Joint_GetConstraintForce( m_joints[i] ) ) * dt;
		angularImpulse += b3Length( b3Joint_GetConstraintTorque( m_joints[i] ) ) * dt;
	}

	CPhysicsObjectBox3D *pObjects[2] = { m_pReference, m_pAttached };
	if ( m_breakable.forceLimit > 0 && m_breakable.forceLimit < kUnbreakableLimit )
	{
		for ( int i = 0; i < 2; i++ )
		{
			float invMass = DynamicInvMass( pObjects[i] );
			if ( invMass <= 0 )
				continue;
			// sqrt(m) * |dv| with dv = impulse / m.
			if ( linearImpulse * sqrtf( invMass ) >= m_breakable.forceLimit )
				return true;
		}
	}
	if ( m_breakable.torqueLimit > 0 && m_breakable.torqueLimit < kUnbreakableLimit )
	{
		if ( angularImpulse * kAngularImpulseToHavok >= DEG2RAD( m_breakable.torqueLimit ) )
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// IPhysicsConstraint
//-----------------------------------------------------------------------------
IPhysicsObject *CConstraintBox3D::GetReferenceObject( void ) const
{
	return m_pReference;
}

IPhysicsObject *CConstraintBox3D::GetAttachedObject( void ) const
{
	return m_pAttached;
}

void CConstraintBox3D::Activate( void )
{
	// As IVP: a grouped constraint follows its group.
	if ( m_pGroup )
		return;
	m_active = true;
	SyncJoints();
}

void CConstraintBox3D::Deactivate( void )
{
	if ( m_pGroup )
		return;
	m_active = false;
	SyncJoints();
}

void CConstraintBox3D::SetLinearMotor( float speed, float maxLinearImpulse )
{
	if ( m_type != CONSTRAINT_BOX3D_SLIDING )
		return;
	m_slideVelocity = speed;
	m_slideFriction = maxLinearImpulse;
	m_motorEnabled = true;
	m_motorSpeed = speed;
	m_motorMax = fabsf( maxLinearImpulse );
	for ( int i = 0; i < m_jointCount; i++ )
	{
		b3PrismaticJoint_EnableMotor( m_joints[i], true );
		b3PrismaticJoint_SetMotorSpeed( m_joints[i], m_motorSpeed );
		b3PrismaticJoint_SetMaxMotorForce( m_joints[i], m_motorMax );
		b3Joint_WakeBodies( m_joints[i] );
	}
}

void CConstraintBox3D::SetAngularMotor( float rotSpeed, float maxAngularImpulse )
{
	if ( m_type == CONSTRAINT_BOX3D_RAGDOLL && rotSpeed == 0 )
	{
		// IVP: ragdoll joint friction update.
		m_motorEnabled = maxAngularImpulse != 0;
		m_motorSpeed = 0.0f;
		m_motorMax = DEG2RAD( fabsf( maxAngularImpulse ) ) * kTorqueToBox3D;
		for ( int i = 0; i < m_jointCount; i++ )
		{
			b3SphericalJoint_EnableMotor( m_joints[i], m_motorEnabled );
			b3SphericalJoint_SetMaxMotorTorque( m_joints[i], m_motorMax );
			b3SphericalJoint_SetMotorVelocity( m_joints[i], b3Vec3_zero );
		}
	}
	else if ( m_type == CONSTRAINT_BOX3D_HINGE )
	{
		m_motorEnabled = true;
		m_motorSpeed = DEG2RAD( rotSpeed );
		m_motorMax = DEG2RAD( fabsf( maxAngularImpulse ) ) * kTorqueToBox3D;
		m_hingeAxis.angularVelocity = rotSpeed;
		m_hingeAxis.torque = fabsf( maxAngularImpulse );
		for ( int i = 0; i < m_jointCount; i++ )
		{
			b3RevoluteJoint_EnableMotor( m_joints[i], true );
			b3RevoluteJoint_SetMotorSpeed( m_joints[i], m_motorSpeed );
			b3RevoluteJoint_SetMaxMotorTorque( m_joints[i], m_motorMax );
			b3Joint_WakeBodies( m_joints[i] );
		}
	}
}

void CConstraintBox3D::UpdateRagdollTransforms( const matrix3x4_t &constraintToReference, const matrix3x4_t &constraintToAttached )
{
	if ( m_type != CONSTRAINT_BOX3D_RAGDOLL || IsInert() )
		return;
	m_ragdoll.constraintToReference = constraintToReference;
	m_ragdoll.constraintToAttached = constraintToAttached;
	SetupRagdollFit( m_ragdoll );
	for ( int i = 0; i < m_jointCount; i++ )
	{
		b3Joint_SetLocalFrameA( m_joints[i], m_frameRef );
		b3Joint_SetLocalFrameB( m_joints[i], m_frameAtt );
	}
}

bool CConstraintBox3D::GetConstraintTransform( matrix3x4_t *pConstraintToReference, matrix3x4_t *pConstraintToAttached ) const
{
	// Same coverage as IVP: ragdoll, ballsocket and fixed constraints.
	switch ( m_type )
	{
	case CONSTRAINT_BOX3D_RAGDOLL:
		if ( pConstraintToReference )
			*pConstraintToReference = m_ragdoll.constraintToReference;
		if ( pConstraintToAttached )
			*pConstraintToAttached = m_ragdoll.constraintToAttached;
		return true;
	case CONSTRAINT_BOX3D_BALLSOCKET:
		if ( pConstraintToReference )
			AngleMatrix( vec3_angle, FromB3( m_frameRef.p ), *pConstraintToReference );
		if ( pConstraintToAttached )
			AngleMatrix( vec3_angle, FromB3( m_frameAtt.p ), *pConstraintToAttached );
		return true;
	case CONSTRAINT_BOX3D_FIXED:
		if ( pConstraintToReference )
			MatrixFromTransform( m_frameRef, *pConstraintToReference );
		if ( pConstraintToAttached )
			MatrixFromTransform( m_frameAtt, *pConstraintToAttached );
		return true;
	default:
		return false;
	}
}

void CConstraintBox3D::ReadBreakable( constraint_breakableparams_t &params ) const
{
	// As IVP's ReadBreakableConstraint: breakable constraints report their
	// limits (strength normalized to 1), others the defaults.
	if ( IsBreakable( m_breakable ) )
	{
		params = m_breakable;
		params.strength = 1.0f;
		params.bodyMassScale[0] = m_breakable.bodyMassScale[0] > 0 ? m_breakable.bodyMassScale[0] : 1.0f;
		params.bodyMassScale[1] = m_breakable.bodyMassScale[1] > 0 ? m_breakable.bodyMassScale[1] : 1.0f;
		if ( params.forceLimit <= 0 )
			params.forceLimit = kUnbreakableLimit;
		if ( params.torqueLimit <= 0 )
			params.torqueLimit = RAD2DEG( kUnbreakableLimit );
	}
	else
	{
		params.Defaults();
	}
	if ( !m_pGroup )
		params.isActive = m_active;
}

bool CConstraintBox3D::GetConstraintParams( constraint_breakableparams_t *pParams ) const
{
	if ( !pParams || IsInert() || m_type == CONSTRAINT_BOX3D_UNKNOWN )
		return false;
	ReadBreakable( *pParams );
	return true;
}

void CConstraintBox3D::OutputDebugInfo()
{
	static const char *s_types[] = { "unknown", "ragdoll", "hinge", "fixed", "ballsocket", "sliding", "pulley", "length" };
	Msg( "Box3D constraint %s: %s, %d joint(s)%s\n", s_types[m_type], IsSimulating() ? "active" : "inactive", m_jointCount,
		m_pGroup ? " (grouped)" : "" );
}

//-----------------------------------------------------------------------------
// Persistence: authored parameters from the stored frames
//-----------------------------------------------------------------------------
void CConstraintBox3D::WriteRagdoll( constraint_ragdollparams_t &ragdoll ) const
{
	ragdoll = m_ragdoll;
	ReadBreakable( ragdoll.constraint );
	ragdoll.childIndex = -1;
	ragdoll.parentIndex = -1;
	ragdoll.isActive = true;
}

void CConstraintBox3D::WriteHinge( constraint_hingeparams_t &hinge ) const
{
	hinge.Defaults();
	ReadBreakable( hinge.constraint );
	if ( IsInert() )
		return;
	// The joint frame's z is the negated IVP axis.
	b3Transform world = b3MulTransforms( BodyTransform( m_pReference ), m_frameRef );
	hinge.worldPosition = FromB3( world.p );
	hinge.worldAxisDirection = -FromB3( b3RotateVector( world.q, b3Vec3_axisZ ) );
	hinge.hingeAxis.SetAxisFriction( 0, 0, 0 );
	if ( m_limitEnabled )
	{
		hinge.hingeAxis.minRotation = RAD2DEG( m_limitLower );
		hinge.hingeAxis.maxRotation = RAD2DEG( m_limitUpper );
	}
	if ( m_motorEnabled )
	{
		hinge.hingeAxis.angularVelocity = RAD2DEG( m_motorSpeed );
		hinge.hingeAxis.torque = RAD2DEG( m_motorMax / kTorqueToBox3D );
	}
}

void CConstraintBox3D::WriteFixed( constraint_fixedparams_t &fixed ) const
{
	fixed.Defaults();
	ReadBreakable( fixed.constraint );
	MatrixFromTransform( m_frameRef, fixed.attachedRefXform );
}

void CConstraintBox3D::WriteBallsocket( constraint_ballsocketparams_t &ballsocket ) const
{
	ballsocket.Defaults();
	ReadBreakable( ballsocket.constraint );
	ballsocket.constraintPosition[0] = FromB3( m_frameRef.p );
	ballsocket.constraintPosition[1] = FromB3( m_frameAtt.p );
}

void CConstraintBox3D::WriteSliding( constraint_slidingparams_t &sliding ) const
{
	sliding.Defaults();
	ReadBreakable( sliding.constraint );
	// attachedRefXform = frameRef * inverse(frameAtt) (frameAtt has no offset).
	b3Transform attachedToRef = b3MulTransforms( m_frameRef, b3InvertTransform( m_frameAtt ) );
	MatrixFromTransform( attachedToRef, sliding.attachedRefXform );
	sliding.slideAxisRef = m_slideAxisRef;
	if ( m_limitEnabled )
	{
		sliding.limitMin = m_limitLower;
		sliding.limitMax = m_limitUpper;
	}
	if ( m_motorEnabled )
	{
		sliding.friction = m_slideFriction;
		sliding.velocity = m_slideVelocity;
	}
}

void CConstraintBox3D::WritePulley( constraint_pulleyparams_t &pulley ) const
{
	pulley.Defaults();
	ReadBreakable( pulley.constraint );
	for ( int i = 0; i < 2; i++ )
	{
		pulley.pulleyPosition[i] = m_pulleyPosition[i];
		pulley.objectPosition[i] = m_objectPosition[i];
	}
	pulley.totalLength = m_totalLength;
	pulley.gearRatio = m_gearRatio;
	pulley.isRigid = m_isRigid;
}

void CConstraintBox3D::WriteLength( constraint_lengthparams_t &length ) const
{
	length.Defaults();
	ReadBreakable( length.constraint );
	length.objectPosition[0] = m_objectPosition[0];
	length.objectPosition[1] = m_objectPosition[1];
	length.totalLength = m_totalLength;
	length.minLength = m_minLength;
}

//-----------------------------------------------------------------------------
// Constraint group
//-----------------------------------------------------------------------------
CConstraintGroupBox3D::~CConstraintGroupBox3D()
{
	CUtlVector<CConstraintBox3D *> members;
	members.CopyArray( m_constraints.Base(), m_constraints.Count() );
	m_constraints.RemoveAll();
	for ( int i = 0; i < members.Count(); i++ )
		members[i]->GroupDestroyed();
}

void CConstraintGroupBox3D::Activate()
{
	if ( m_active )
		return;
	m_active = true;
	for ( int i = 0; i < m_constraints.Count(); i++ )
		m_constraints[i]->SyncJoints();
}
