//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D constraints and constraint groups (RFC 0004 C2).
//
//=============================================================================//
#ifndef PHYSICS_CONSTRAINT_BOX3D_H
#define PHYSICS_CONSTRAINT_BOX3D_H

#include "vphysics_interface.h"
#include "vphysics/constraints.h"
#include "utlvector.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"

class CPhysicsEnvironmentBox3D;
class CPhysicsObjectBox3D;
class CConstraintGroupBox3D;

// Same numbering as IVP's saved constraint types.
enum ConstraintTypeBox3D_t
{
	CONSTRAINT_BOX3D_UNKNOWN = 0,
	CONSTRAINT_BOX3D_RAGDOLL,
	CONSTRAINT_BOX3D_HINGE,
	CONSTRAINT_BOX3D_FIXED,
	CONSTRAINT_BOX3D_BALLSOCKET,
	CONSTRAINT_BOX3D_SLIDING,
	CONSTRAINT_BOX3D_PULLEY,
	CONSTRAINT_BOX3D_LENGTH,
};

//-----------------------------------------------------------------------------
// A constraint owns its authored parameters as object-local joint frames and
// realizes them as Box3D joints only while it simulates: standalone
// constraints while active, grouped ones while their group is active (IVP's
// local constraint systems start inactive until Activate, which is how
// ragdolls are assembled). Box3D joints cannot be switched off, so
// deactivation destroys them and activation rebuilds them from the frames.
//
// Mapping (IVP/Havok -> Box3D):
//   fixed      -> weld joint
//   ballsocket -> spherical joint (free rotation)
//   hinge      -> revolute joint about the NEGATED authored axis: IVP measures
//                 hinge angles and motor speeds clockwise about the axis (the
//                 HL->IVP basis flip), Box3D counter-clockwise
//   sliding    -> prismatic joint along frame A's x axis
//   length     -> distance joint (rope when min < max, rod when equal)
//   ragdoll    -> revolute joint when exactly one axis is free (as IVP converts
//                 it to a hinge); otherwise a spherical joint with Havok's
//                 twist axis (lowest effective inertia), a cone from the widest
//                 swing axis and twist limits. Havok's third "planes" limit has
//                 no Box3D counterpart; the cone bounds that axis instead.
//   pulley     -> solved here each step as a velocity constraint on the rope
//                 length L0 + gear * L1 <= total (Box3D has no pulley joint)
//
// Motor units: IVP hinge/ragdoll torques are Havok N*m; Box3D runs in inches,
// so torques scale by (inches/meter)^2. Linear motor forces are already in
// Source units (kg*in/s^2) after IVP's own conversion cancels.
//
// Breaking follows hk_Breakable_Constraint: a constraint breaks when, for a
// dynamic body, (constraint impulse)/sqrt(mass) reaches forceLimit, or the
// angular impulse (Havok units) reaches DEG2RAD(torqueLimit). The game's
// IPhysicsConstraintEvent::ConstraintBroken handler decides what to do.
//-----------------------------------------------------------------------------
class CConstraintBox3D : public IPhysicsConstraint
{
public:
	CConstraintBox3D( CPhysicsEnvironmentBox3D *pEnv, IPhysicsObject *pReference, IPhysicsObject *pAttached,
		CConstraintGroupBox3D *pGroup, ConstraintTypeBox3D_t type, const constraint_breakableparams_t &breakable );
	virtual ~CConstraintBox3D();

	void InitRagdoll( const constraint_ragdollparams_t &ragdoll );
	void InitHinge( const constraint_limitedhingeparams_t &hinge );
	void InitFixed( const constraint_fixedparams_t &fixed );
	void InitBallsocket( const constraint_ballsocketparams_t &ballsocket );
	void InitSliding( const constraint_slidingparams_t &sliding );
	void InitPulley( const constraint_pulleyparams_t &pulley );
	void InitLength( const constraint_lengthparams_t &length );

	// IPhysicsConstraint
	virtual void Activate( void ) override;
	virtual void Deactivate( void ) override;
	virtual void SetGameData( void *gameData ) override { m_pGameData = gameData; }
	virtual void *GetGameData( void ) const override { return m_pGameData; }
	virtual IPhysicsObject *GetReferenceObject( void ) const override;
	virtual IPhysicsObject *GetAttachedObject( void ) const override;
	virtual void SetLinearMotor( float speed, float maxLinearImpulse ) override;
	virtual void SetAngularMotor( float rotSpeed, float maxAngularImpulse ) override;
	virtual void UpdateRagdollTransforms( const matrix3x4_t &constraintToReference, const matrix3x4_t &constraintToAttached ) override;
	virtual bool GetConstraintTransform( matrix3x4_t *pConstraintToReference, matrix3x4_t *pConstraintToAttached ) const override;
	virtual bool GetConstraintParams( constraint_breakableparams_t *pParams ) const override;
	virtual void OutputDebugInfo() override;

	// Provider internals.
	ConstraintTypeBox3D_t GetType() const { return m_type; }
	CConstraintGroupBox3D *GetGroup() const { return m_pGroup; }
	bool Links( IPhysicsObject *pObject ) const
	{
		return pObject && ( pObject == (IPhysicsObject *)m_pReference || pObject == (IPhysicsObject *)m_pAttached );
	}
	bool IsInert() const { return m_pReference == NULL; }
	bool IsSimulating() const;
	// Rebuilds or tears down the Box3D joints to match IsSimulating().
	void SyncJoints();
	// Called when a linked object is destroyed: its joints die with the body,
	// and IVP leaves an inert constraint with no objects.
	void ObjectDestroyed();
	// The group this constraint belongs to is going away.
	void GroupDestroyed() { m_pGroup = NULL; SyncJoints(); }
	// Per step, before the Box3D step (pulley rope solve).
	void PreStep( float dt );
	// Per step, after the Box3D step: true when the breaking limit was reached.
	bool CheckBreak( float dt ) const;

	// Authored parameters for persistence (IVP's WriteTo* equivalents).
	void WriteRagdoll( constraint_ragdollparams_t &ragdoll ) const;
	void WriteHinge( constraint_hingeparams_t &hinge ) const;
	void WriteFixed( constraint_fixedparams_t &fixed ) const;
	void WriteBallsocket( constraint_ballsocketparams_t &ballsocket ) const;
	void WriteSliding( constraint_slidingparams_t &sliding ) const;
	void WritePulley( constraint_pulleyparams_t &pulley ) const;
	void WriteLength( constraint_lengthparams_t &length ) const;

private:
	void SetupJointBase( struct b3JointDef &base );
	void CreateJoints();
	void DestroyJoints();
	void AddJoint( b3JointId joint );
	void SetupRagdollFit( const constraint_ragdollparams_t &ragdoll );
	void ReadBreakable( constraint_breakableparams_t &params ) const;
	float DynamicInvMass( CPhysicsObjectBox3D *pObject ) const;

	CPhysicsEnvironmentBox3D *m_pEnv;
	CPhysicsObjectBox3D *m_pReference;
	CPhysicsObjectBox3D *m_pAttached;
	CConstraintGroupBox3D *m_pGroup;
	void *m_pGameData;
	ConstraintTypeBox3D_t m_type;
	constraint_breakableparams_t m_breakable;
	bool m_active;

	// Box3D realization.
	b3JointId m_joints[2];
	int m_jointCount;

	// Object-local joint frames (reference = A, attached = B).
	b3Transform m_frameRef;
	b3Transform m_frameAtt;

	// Limits (radians or inches, Box3D convention) and motor.
	bool m_limitEnabled;
	float m_limitLower;
	float m_limitUpper;
	bool m_motorEnabled;
	float m_motorSpeed;		// rad/s or in/s, Box3D convention
	float m_motorMax;		// Box3D torque (kg*in^2/s^2) or force (kg*in/s^2)
	// Spherical (ragdoll) fit.
	bool m_coneEnabled;
	float m_coneAngle;
	bool m_twistEnabled;

	// Authored ragdoll data kept for GetConstraintTransform / persistence.
	constraint_ragdollparams_t m_ragdoll;
	// Authored linear (length/pulley) data.
	Vector m_objectPosition[2];
	Vector m_pulleyPosition[2];
	float m_totalLength;
	float m_minLength;
	float m_gearRatio;
	bool m_isRigid;
	// Last pulley impulse (for breaking), Source units.
	float m_pulleyImpulse;
	// Sliding: authored axis in reference space and limits (Source units).
	Vector m_slideAxisRef;
	float m_slideFriction;
	float m_slideVelocity;
	// Hinge: authored limits/friction in degrees (IVP convention).
	constraint_axislimit_t m_hingeAxis;
};

//-----------------------------------------------------------------------------
// Constraint group: a set of constraints (a ragdoll) that simulate together
// once activated. IVP's error reporting for groups is compiled out
// (hk_Local_Constraint_System::report_square_error), so a group is never in
// an error state; the parameters round-trip.
//-----------------------------------------------------------------------------
class CConstraintGroupBox3D : public IPhysicsConstraintGroup
{
public:
	explicit CConstraintGroupBox3D( const constraint_groupparams_t &params ) : m_params( params ), m_active( false ) {}
	virtual ~CConstraintGroupBox3D();

	virtual void Activate() override;
	virtual bool IsInErrorState() override { return false; }
	virtual void ClearErrorState() override {}
	virtual void GetErrorParams( constraint_groupparams_t *pParams ) override
	{
		if ( pParams )
			*pParams = m_params;
	}
	virtual void SetErrorParams( const constraint_groupparams_t &params ) override
	{
		m_params.minErrorTicks = params.minErrorTicks;
		m_params.errorTolerance = params.errorTolerance;
	}
	virtual void SolvePenetration( IPhysicsObject *pObj0, IPhysicsObject *pObj1 ) override {}

	bool IsActive() const { return m_active; }
	const constraint_groupparams_t &GetParams() const { return m_params; }
	void AddConstraint( CConstraintBox3D *pConstraint ) { m_constraints.AddToTail( pConstraint ); }
	void RemoveConstraint( CConstraintBox3D *pConstraint ) { m_constraints.FindAndRemove( pConstraint ); }

private:
	constraint_groupparams_t m_params;
	bool m_active;
	CUtlVector<CConstraintBox3D *> m_constraints;
};

#endif // PHYSICS_CONSTRAINT_BOX3D_H
