#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include "vphysics_interface.h"

class CPhysicsObjectBox3D : public IPhysicsObject
{
public:
	CPhysicsObjectBox3D() {}
	virtual ~CPhysicsObjectBox3D() {}

	virtual bool IsStatic() const override { return false; }
	virtual bool IsAsleep() const override { return false; }
	virtual bool IsTrigger() const override { return false; }
	virtual bool IsFluid() const override { return false; }
	virtual bool IsHinged() const override { return false; }
	virtual bool IsCollisionEnabled() const override { return true; }
	virtual bool IsGravityEnabled() const override { return true; }
	virtual bool IsDragEnabled() const override { return true; }
	virtual bool IsMotionEnabled() const override { return true; }
	virtual bool IsMoveable() const override { return true; }
	virtual bool IsAttachedToConstraint( bool bExternalOnly ) const override { return false; }

	virtual void EnableCollisions( bool enable ) override {}
	virtual void EnableGravity( bool enable ) override {}
	virtual void EnableDrag( bool enable ) override {}
	virtual void EnableMotion( bool enable ) override {}

	virtual void SetGameData( void *pGameData ) override {}
	virtual void *GetGameData( void ) const override { return nullptr; }
	virtual void SetGameFlags( unsigned short userFlags ) override {}
	virtual unsigned short GetGameFlags( void ) const override { return 0; }
	virtual void SetGameIndex( unsigned short gameIndex ) override {}
	virtual unsigned short GetGameIndex( void ) const override { return 0; }

	virtual void SetCallbackFlags( unsigned short callbackflags ) override {}
	virtual unsigned short GetCallbackFlags( void ) const override { return 0; }

	virtual void Wake( void ) override {}
	virtual void Sleep( void ) override {}

	virtual void RecheckCollisionFilter( void ) override {}
	virtual void RecheckContactPoints( void ) override {}

	virtual void SetMass( float mass ) override {}
	virtual float GetMass( void ) const override { return 1.0f; }
	virtual float GetInvMass( void ) const override { return 1.0f; }
	virtual Vector GetInertia( void ) const override { return Vector(1,1,1); }
	virtual Vector GetInvInertia( void ) const override { return Vector(1,1,1); }
	virtual void SetInertia( const Vector &inertia ) override {}

	virtual void SetDamping( const float *speed, const float *rot ) override {}
	virtual void GetDamping( float *speed, float *rot ) const override {}

	virtual void SetDragCoefficient( float *pDrag, float *pAngularDrag ) override {}
	virtual void SetBuoyancyRatio( float ratio ) override {}

	virtual int GetMaterialIndex() const override { return 0; }
	virtual void SetMaterialIndex( int materialIndex ) override {}

	virtual unsigned int GetContents() const override { return 0; }
	virtual void SetContents( unsigned int contents ) override {}

	virtual float GetSphereRadius() const override { return 0.0f; }
	virtual float GetEnergy() const override { return 0.0f; }
	virtual Vector GetMassCenterLocalSpace() const override { return Vector(0,0,0); }

	virtual void SetPosition( const Vector &worldPosition, const QAngle &angles, bool isTeleport ) override {}
	virtual void SetPositionMatrix( const matrix3x4_t &matrix, bool isTeleport ) override {}

	virtual void GetPosition( Vector *worldPosition, QAngle *angles ) const override {}
	virtual void GetPositionMatrix( matrix3x4_t *positionMatrix ) const override {}

	virtual void SetVelocity( const Vector *velocity, const AngularImpulse *angularVelocity ) override {}
	virtual void SetVelocityInstantaneous( const Vector *velocity, const AngularImpulse *angularVelocity ) override {}
	virtual void GetVelocity( Vector *velocity, AngularImpulse *angularVelocity ) const override {}

	virtual void AddVelocity( const Vector *velocity, const AngularImpulse *angularVelocity ) override {}

	virtual void GetVelocityAtPoint( const Vector &worldPosition, Vector *pVelocity ) const override {}
	virtual void GetImplicitVelocity( Vector *velocity, AngularImpulse *angularVelocity ) const override {}

	virtual void LocalToWorld( Vector *worldPosition, const Vector &localPosition ) const override {}
	virtual void WorldToLocal( Vector *localPosition, const Vector &worldPosition ) const override {}

	virtual void LocalToWorldVector( Vector *worldVector, const Vector &localVector ) const override {}
	virtual void WorldToLocalVector( Vector *localVector, const Vector &worldVector ) const override {}

	virtual void ApplyDirectedForce( const Vector &forceVector, const Vector &forcePoint ) {}
	virtual void ApplyForceCenter( const Vector &forceVector ) override {}
	virtual void ApplyForceOffset( const Vector &forceVector, const Vector &worldPosition ) override {}
	virtual void ApplyTorqueCenter( const AngularImpulse &torque ) override {}

	virtual void CalculateForceOffset( const Vector &forceVector, const Vector &worldPosition, Vector *centerForce, AngularImpulse *centerTorque ) const override {}
	virtual void CalculateVelocityOffset( const Vector &forceVector, const Vector &worldPosition, Vector *centerVelocity, AngularImpulse *centerAngularVelocity ) const override {}

	virtual float CalculateLinearDrag( const Vector &unitDirection ) const override { return 0.0f; }
	virtual float CalculateAngularDrag( const Vector &objectSpaceRotationAxis ) const override { return 0.0f; }

	virtual bool GetContactPoint( Vector *contactPoint, IPhysicsObject **contactObject ) const override { return false; }
	virtual void SetShadow( float maxSpeed, float maxAngularSpeed, bool allowPhysicsMovement, bool allowPhysicsRotation ) override {}
	virtual void UpdateShadow( const Vector &targetPosition, const QAngle &targetAngles, bool tempDisableGravity, float timeOffset ) override {}

	virtual int GetShadowPosition( Vector *position, QAngle *angles ) const override { return 0; }
	virtual IPhysicsShadowController *GetShadowController( void ) const override { return nullptr; }
	virtual void RemoveShadowController() override {}
	virtual float ComputeShadowControl( const hlshadowcontrol_params_t &params, float secondsToArrival, float dt ) override { return 0.0f; }

	virtual const CPhysCollide *GetCollide( void ) const override { return nullptr; }
	virtual const char *GetName() const override { return ""; }

	virtual void BecomeTrigger() override {}
	virtual void RemoveTrigger() override {}
	virtual void BecomeHinged( int localAxis ) override {}
	virtual void RemoveHinged() override {}

	virtual IPhysicsFrictionSnapshot *CreateFrictionSnapshot() override { return nullptr; }
	virtual void DestroyFrictionSnapshot( IPhysicsFrictionSnapshot *pSnapshot ) override {}

	virtual void OutputDebugInfo() const override {}
};

#endif // PHYSICS_OBJECT_H
