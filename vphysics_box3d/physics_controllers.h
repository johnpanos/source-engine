#ifndef PHYSICS_CONTROLLERS_H
#define PHYSICS_CONTROLLERS_H

#include "vphysics_interface.h"
#include "vphysics/player_controller.h"
#include "vphysics/friction.h"
#include "vphysics/constraints.h"
#include "mathlib/vector.h"
#include "utlvector.h"
#include "box3d/id.h"

class CPhysicsObjectBox3D;

// IVP's shadow steering law (vphysics/physics_shadow.cpp,
// ComputeShadowControllerIVP) applied to a Box3D body: sets the object's
// velocity to close the pose error over secondsToArrival within the max
// speeds, damping the current velocity by dampFactor. Returns the remaining
// seconds to arrival.
float ComputeShadowControlBox3D( CPhysicsObjectBox3D *pObject, const hlshadowcontrol_params_t &params,
	float secondsToArrival, float dt, Vector *pLastImpulse );

// Provider-owned controllers, constraints and contact snapshots for the Box3D
// backend. Controllers are applied by the environment before each Box3D step
// (CPhysicsEnvironmentBox3D::PreStep), in the same place IVP runs its
// simulation controllers.

//-----------------------------------------------------------------------------
// Shadow controller: drives a game-controlled object toward a target pose by
// setting its velocity each step. The steering law is IVP's
// ComputeShadowControllerIVP (vphysics/physics_shadow.cpp): close the error
// over the remaining seconds-to-arrival, clamp to the max speeds, and damp
// the current velocity by dampFactor.
//-----------------------------------------------------------------------------
class CShadowControllerBox3D : public IPhysicsShadowController
{
public:
	CShadowControllerBox3D( CPhysicsObjectBox3D *pObject, bool allowTranslation, bool allowRotation );
	virtual ~CShadowControllerBox3D();

	virtual void Update( const Vector &position, const QAngle &angles, float timeOffset ) override;
	virtual void MaxSpeed( float maxSpeed, float maxAngularSpeed ) override
	{
		m_maxSpeed = maxSpeed;
		m_maxAngularSpeed = maxAngularSpeed;
	}
	virtual void StepUp( float height ) override;
	virtual void SetTeleportDistance( float teleportDistance ) override { m_teleportDistance = teleportDistance; }
	virtual bool AllowsTranslation() override { return m_allowTranslation; }
	virtual bool AllowsRotation() override { return m_allowRotation; }
	virtual void SetPhysicallyControlled( bool isPhysicallyControlled ) override { m_physicallyControlled = isPhysicallyControlled; }
	virtual bool IsPhysicallyControlled() override { return m_physicallyControlled; }
	virtual void GetLastImpulse( Vector *pOut ) override { *pOut = m_lastImpulse; }
	virtual void UseShadowMaterial( bool bUseShadowMaterial ) override {}
	virtual void ObjectMaterialChanged( int materialIndex ) override {}
	virtual float GetTargetPosition( Vector *pPositionOut, QAngle *pAnglesOut ) override
	{
		if ( pPositionOut )
			*pPositionOut = m_targetPosition;
		if ( pAnglesOut )
			*pAnglesOut = m_targetAngles;
		return m_secondsToArrival;
	}
	virtual float GetTeleportDistance( void ) override { return m_teleportDistance; }
	virtual void GetMaxSpeed( float *pMaxSpeedOut, float *pMaxAngularSpeedOut ) override
	{
		if ( pMaxSpeedOut )
			*pMaxSpeedOut = m_maxSpeed;
		if ( pMaxAngularSpeedOut )
			*pMaxAngularSpeedOut = m_maxAngularSpeed;
	}

	// Applies one step of control; called by the environment before stepping.
	void Simulate( float dt );
	bool TempDisablesGravity() const { return m_tempDisableGravity; }
	void SetTempDisableGravity( bool disable ) { m_tempDisableGravity = disable; }

private:
	CPhysicsObjectBox3D *m_pObject;
	Vector m_targetPosition;
	QAngle m_targetAngles;
	Vector m_lastImpulse;
	float m_maxSpeed;
	float m_maxAngularSpeed;
	float m_teleportDistance;
	float m_secondsToArrival;
	float m_dampFactor;
	unsigned short m_savedCallbackFlags;
	float m_savedMass;
	bool m_allowTranslation;
	bool m_allowRotation;
	bool m_physicallyControlled;
	bool m_enabled;
	bool m_tempDisableGravity;
};

//-----------------------------------------------------------------------------
// Player controller: the player's physics shadow, driven toward the game's
// movement result with the same steering law as a shadow controller.
//-----------------------------------------------------------------------------
class CPlayerControllerBox3D : public IPhysicsPlayerController
{
public:
	explicit CPlayerControllerBox3D( CPhysicsObjectBox3D *pObject );
	virtual ~CPlayerControllerBox3D() {}

	virtual void Update( const Vector &position, const Vector &velocity, float secondsToArrival, bool onground, IPhysicsObject *ground ) override;
	virtual void SetEventHandler( IPhysicsPlayerControllerEvent *handler ) override { m_pHandler = handler; }
	virtual bool IsInContact( void ) override;
	virtual void MaxSpeed( const Vector &maxVelocity ) override { m_maxVelocity = maxVelocity; }
	virtual void SetObject( IPhysicsObject *pObject ) override;
	virtual int GetShadowPosition( Vector *position, QAngle *angles ) override;
	virtual void StepUp( float height ) override;
	virtual void Jump() override {}
	virtual void GetShadowVelocity( Vector *velocity ) override;
	virtual IPhysicsObject *GetObject() override;
	virtual void GetLastImpulse( Vector *pOut ) override { *pOut = m_lastImpulse; }
	virtual void SetPushMassLimit( float maxPushMass ) override { m_pushMassLimit = maxPushMass; }
	virtual void SetPushSpeedLimit( float maxPushSpeed ) override { m_pushSpeedLimit = maxPushSpeed; }
	virtual float GetPushMassLimit() override { return m_pushMassLimit; }
	virtual float GetPushSpeedLimit() override { return m_pushSpeedLimit; }
	virtual bool WasFrozen() override { return false; }

	void Simulate( float dt );

private:
	CPhysicsObjectBox3D *m_pObject;
	IPhysicsPlayerControllerEvent *m_pHandler;
	Vector m_targetPosition;
	Vector m_targetVelocity;
	Vector m_lastImpulse;
	Vector m_maxVelocity;
	float m_secondsToArrival;
	float m_pushMassLimit;
	float m_pushSpeedLimit;
	bool m_enabled;
};

//-----------------------------------------------------------------------------
// Motion controller: calls its IMotionEvent handler for each attached awake
// object every step and applies the returned force or acceleration.
//-----------------------------------------------------------------------------
class CMotionControllerBox3D : public IPhysicsMotionController
{
public:
	explicit CMotionControllerBox3D( IMotionEvent *pHandler ) : m_pHandler( pHandler ), m_priority( MEDIUM_PRIORITY ) {}
	virtual ~CMotionControllerBox3D() {}

	virtual void SetEventHandler( IMotionEvent *handler ) override { m_pHandler = handler; }
	virtual void AttachObject( IPhysicsObject *pObject, bool checkIfAlreadyAttached ) override
	{
		if ( checkIfAlreadyAttached && m_objects.Find( pObject ) != m_objects.InvalidIndex() )
			return;
		m_objects.AddToTail( pObject );
	}
	virtual void DetachObject( IPhysicsObject *pObject ) override { m_objects.FindAndRemove( pObject ); }
	virtual int CountObjects( void ) override { return m_objects.Count(); }
	virtual void GetObjects( IPhysicsObject **pObjectList ) override
	{
		for ( int i = 0; i < m_objects.Count(); i++ )
			pObjectList[i] = m_objects[i];
	}
	virtual void ClearObjects( void ) override { m_objects.RemoveAll(); }
	virtual void WakeObjects( void ) override
	{
		for ( int i = 0; i < m_objects.Count(); i++ )
			m_objects[i]->Wake();
	}
	virtual void SetPriority( priority_t priority ) override { m_priority = priority; }

	void Simulate( float dt );
	// Drops a destroyed object so the controller never touches it again.
	void ObjectDestroyed( IPhysicsObject *pObject ) { m_objects.FindAndRemove( pObject ); }

private:
	IMotionEvent *m_pHandler;
	priority_t m_priority;
	CUtlVector<IPhysicsObject *> m_objects;
};

//-----------------------------------------------------------------------------
// Friction snapshot: iterates the object's current touching contacts, one
// entry per manifold, captured when the snapshot is created.
//-----------------------------------------------------------------------------
class CFrictionSnapshotBox3D : public IPhysicsFrictionSnapshot
{
public:
	explicit CFrictionSnapshotBox3D( CPhysicsObjectBox3D *pObject );
	virtual ~CFrictionSnapshotBox3D() {}

	virtual bool IsValid() override { return m_index < m_entries.Count(); }
	virtual IPhysicsObject *GetObject( int index ) override;
	virtual int GetMaterial( int index ) override;
	virtual void GetContactPoint( Vector &out ) override { out = Current().point; }
	virtual void GetSurfaceNormal( Vector &out ) override { out = Current().normal; }
	virtual float GetNormalForce() override { return Current().normalForce; }
	virtual float GetEnergyAbsorbed() override { return 0.0f; }
	virtual void RecomputeFriction() override {}
	virtual void ClearFrictionForce() override {}
	virtual void MarkContactForDelete() override {}
	virtual void DeleteAllMarkedContacts( bool wakeObjects ) override {}
	virtual void NextFrictionData() override { m_index++; }
	virtual float GetFrictionCoefficient() override { return Current().friction; }

private:
	struct Entry_t
	{
		CPhysicsObjectBox3D *pOther;
		Vector point;
		Vector normal;		// points from this object toward the other
		float normalForce;
		float friction;
	};
	const Entry_t &Current() const
	{
		static Entry_t s_empty = { NULL, vec3_origin, vec3_origin, 0.0f, 0.0f };
		return m_index < m_entries.Count() ? m_entries[m_index] : s_empty;
	}

	CPhysicsObjectBox3D *m_pObject;
	CUtlVector<Entry_t> m_entries;
	int m_index;
};

#endif // PHYSICS_CONTROLLERS_H
