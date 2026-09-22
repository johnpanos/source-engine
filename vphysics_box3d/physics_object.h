#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include "vphysics_interface.h"
#include "mathlib/mathlib.h"
#include "box3d/id.h"

class CPhysicsEnvironmentBox3D;
class CShadowControllerBox3D;
class CPlayerControllerBox3D;
class CPhysicsFluidControllerBox3D;
struct CPhysCollideBox3D;

// Box3D runs in inches; VPhysics reports rotational inertia, angular impulse
// and torque in IVP's metric units (kg*m^2), so those convert at the API.
const float kInertiaToBox3D = ( 1.0f / 0.0254f ) * ( 1.0f / 0.0254f );

// A VPhysics object backed by one Box3D body. The body origin is the object
// origin (the collide's space), so shapes use collide-space geometry directly
// and Box3D's mass data carries the center of mass.
//
// Kinds: static objects are Box3D static bodies; movable objects are dynamic
// bodies; EnableMotion(false) pins a movable object by making it kinematic
// with zero velocity (IVP's "pinned" core), and re-enabling restores it.
//
// Mass properties follow IVP, not Box3D's shape integration: mass is clamped
// to [VPHYSICS_MIN_MASS, VPHYSICS_MAX_MASS], the diagonal rotational inertia
// is mass * the collide's IVP per-mass inertia * the authored inertia scale,
// clipped below at rotInertiaLimit * |I|, and SetMass rescales it. Damping,
// air drag and velocity limits are applied by the environment before each
// step with IVP's formulas (Box3D's own damping is off).
class CPhysicsObjectBox3D : public IPhysicsObject
{
public:
	CPhysicsObjectBox3D( CPhysicsEnvironmentBox3D *pEnv, const CPhysCollide *pCollide, float sphereRadius,
		int materialIndex, const Vector &position, const QAngle &angles, const objectparams_t *pParams, bool isStatic );
	virtual ~CPhysicsObjectBox3D();

	virtual bool IsStatic() const override { return m_isStatic; }
	virtual bool IsAsleep() const override;
	virtual bool IsTrigger() const override { return m_isTrigger; }
	virtual bool IsFluid() const override { return m_pFluid != NULL; }
	virtual bool IsHinged() const override { return m_hingeAxis >= 0; }
	virtual bool IsCollisionEnabled() const override { return m_collisionEnabled; }
	virtual bool IsGravityEnabled() const override { return !m_isStatic && m_gravityEnabled; }
	virtual bool IsDragEnabled() const override { return !m_isStatic && m_dragEnabled; }
	// Static objects are never pinned, so IVP reports their motion enabled.
	virtual bool IsMotionEnabled() const override { return m_motionEnabled; }
	virtual bool IsMoveable() const override { return !m_isStatic && m_motionEnabled; }
	virtual bool IsAttachedToConstraint( bool bExternalOnly ) const override;

	virtual void EnableCollisions( bool enable ) override;
	virtual void EnableGravity( bool enable ) override;
	virtual void EnableDrag( bool enable ) override;
	virtual void EnableMotion( bool enable ) override;

	virtual void SetGameData( void *pGameData ) override { m_pGameData = pGameData; }
	virtual void *GetGameData( void ) const override { return m_pGameData; }
	virtual void SetGameFlags( unsigned short userFlags ) override { m_gameFlags = userFlags; }
	virtual unsigned short GetGameFlags( void ) const override { return m_gameFlags; }
	virtual void SetGameIndex( unsigned short gameIndex ) override { m_gameIndex = gameIndex; }
	virtual unsigned short GetGameIndex( void ) const override { return m_gameIndex; }

	virtual void SetCallbackFlags( unsigned short callbackflags ) override { m_callbackFlags = callbackflags; }
	virtual unsigned short GetCallbackFlags( void ) const override { return m_callbackFlags; }

	virtual void Wake( void ) override;
	virtual void Sleep( void ) override;

	virtual void RecheckCollisionFilter( void ) override;
	virtual void RecheckContactPoints( void ) override;

	virtual void SetMass( float mass ) override;
	virtual float GetMass( void ) const override { return m_mass; }
	virtual float GetInvMass( void ) const override;
	virtual Vector GetInertia( void ) const override { return m_inertia; }
	virtual Vector GetInvInertia( void ) const override;
	virtual void SetInertia( const Vector &inertia ) override;

	virtual void SetDamping( const float *speed, const float *rot ) override;
	virtual void GetDamping( float *speed, float *rot ) const override;

	virtual void SetDragCoefficient( float *pDrag, float *pAngularDrag ) override;
	virtual void SetBuoyancyRatio( float ratio ) override { m_buoyancyRatio = ratio; }

	virtual int GetMaterialIndex() const override { return m_materialIndex; }
	virtual void SetMaterialIndex( int materialIndex ) override;

	virtual unsigned int GetContents() const override { return m_contents; }
	virtual void SetContents( unsigned int contents ) override { m_contents = contents; }

	virtual float GetSphereRadius() const override { return m_sphereRadius; }
	virtual float GetEnergy() const override;
	virtual Vector GetMassCenterLocalSpace() const override { return m_massCenter; }

	virtual void SetPosition( const Vector &worldPosition, const QAngle &angles, bool isTeleport ) override;
	virtual void SetPositionMatrix( const matrix3x4_t &matrix, bool isTeleport ) override;
	virtual void GetPosition( Vector *worldPosition, QAngle *angles ) const override;
	virtual void GetPositionMatrix( matrix3x4_t *positionMatrix ) const override;

	virtual void SetVelocity( const Vector *velocity, const AngularImpulse *angularVelocity ) override;
	virtual void SetVelocityInstantaneous( const Vector *velocity, const AngularImpulse *angularVelocity ) override;
	virtual void GetVelocity( Vector *velocity, AngularImpulse *angularVelocity ) const override;
	virtual void AddVelocity( const Vector *velocity, const AngularImpulse *angularVelocity ) override;
	virtual void GetVelocityAtPoint( const Vector &worldPosition, Vector *pVelocity ) const override;
	virtual void GetImplicitVelocity( Vector *velocity, AngularImpulse *angularVelocity ) const override;

	virtual void LocalToWorld( Vector *worldPosition, const Vector &localPosition ) const override;
	virtual void WorldToLocal( Vector *localPosition, const Vector &worldPosition ) const override;
	virtual void LocalToWorldVector( Vector *worldVector, const Vector &localVector ) const override;
	virtual void WorldToLocalVector( Vector *localVector, const Vector &worldVector ) const override;

	virtual void ApplyForceCenter( const Vector &forceVector ) override;
	virtual void ApplyForceOffset( const Vector &forceVector, const Vector &worldPosition ) override;
	virtual void ApplyTorqueCenter( const AngularImpulse &torque ) override;

	virtual void CalculateForceOffset( const Vector &forceVector, const Vector &worldPosition, Vector *centerForce, AngularImpulse *centerTorque ) const override;
	virtual void CalculateVelocityOffset( const Vector &forceVector, const Vector &worldPosition, Vector *centerVelocity, AngularImpulse *centerAngularVelocity ) const override;

	virtual float CalculateLinearDrag( const Vector &unitDirection ) const override;
	virtual float CalculateAngularDrag( const Vector &objectSpaceRotationAxis ) const override;

	virtual bool GetContactPoint( Vector *contactPoint, IPhysicsObject **contactObject ) const override;

	virtual void SetShadow( float maxSpeed, float maxAngularSpeed, bool allowPhysicsMovement, bool allowPhysicsRotation ) override;
	virtual void UpdateShadow( const Vector &targetPosition, const QAngle &targetAngles, bool tempDisableGravity, float timeOffset ) override;
	virtual int GetShadowPosition( Vector *position, QAngle *angles ) const override;
	virtual IPhysicsShadowController *GetShadowController( void ) const override;
	virtual void RemoveShadowController() override;
	virtual float ComputeShadowControl( const hlshadowcontrol_params_t &params, float secondsToArrival, float dt ) override;

	virtual const CPhysCollide *GetCollide( void ) const override { return m_pCollide; }
	virtual const char *GetName() const override { return m_name; }

	virtual void BecomeTrigger() override;
	virtual void RemoveTrigger() override;
	virtual void BecomeHinged( int localAxis ) override;
	virtual void RemoveHinged() override;

	virtual IPhysicsFrictionSnapshot *CreateFrictionSnapshot() override;
	virtual void DestroyFrictionSnapshot( IPhysicsFrictionSnapshot *pSnapshot ) override;

	virtual void OutputDebugInfo() const override;

	// Provider internals.
	IPhysicsShadowController *EnsureShadowController( bool allowTranslation, bool allowRotation );
	CShadowControllerBox3D *GetShadow() const { return m_pShadow; }
	// Detaches the shadow controller without destroying it (TransferObject)
	// and attaches an existing one.
	CShadowControllerBox3D *DetachShadowController();
	void AttachShadowController( CShadowControllerBox3D *pShadow );
	void SetPlayerController( CPlayerControllerBox3D *pController ) { m_pPlayerController = pController; }
	CPlayerControllerBox3D *GetPlayerController() const { return m_pPlayerController; }
	void SetFluidController( CPhysicsFluidControllerBox3D *pFluid ) { m_pFluid = pFluid; }
	CPhysicsFluidControllerBox3D *GetFluidController() const { return m_pFluid; }
	b3BodyId GetBody() const { return m_body; }
	CPhysicsEnvironmentBox3D *GetEnvironment() const { return m_pEnv; }
	float GetFriction() const { return m_friction; }
	float GetVolume() const { return m_volume; }
	float GetBuoyancyRatio() const { return m_buoyancyRatio; }
	// Sets linear/angular (world, radians) velocity directly, as controllers do.
	void SetWorldVelocity( const Vector &linear, const Vector &angularRadians );
	void GetWorldVelocity( Vector *linear, Vector *angularRadians ) const;
	void TeleportTo( const Vector &position, const QAngle &angles );
	bool WasAwake() const { return m_wasAwake; }
	void SetWasAwake( bool awake ) { m_wasAwake = awake; }
	// Box3D reports contacts after the step; IVP's PreCollision runs before
	// the impulse. The environment captures velocities before stepping and
	// has the colliding objects report them while PreCollision runs.
	void CapturePreStepVelocity();
	void ReportPreStepVelocity( bool report ) { m_reportPreStep = report; }
	Vector GetPreStepVelocity() const { return m_preStepLinear; }
	// IVP's per-step damping and air drag (CDragController), applied by the
	// environment before each step.
	void ApplyDampingAndDrag( float dt, float airDensity );
	bool HasTouchedDynamic() const { return m_hasTouchedDynamic; }
	void SetTouchedDynamic() { m_hasTouchedDynamic = true; }
	bool IsAsleepSinceCreation() const { return m_asleepSinceCreation; }
	void SetAsleepSinceCreation( bool asleep ) { m_asleepSinceCreation = asleep; }
	float GetDragCoefficient() const { return m_dragCoefficient; }
	float GetAngularDragCoefficient() const { return m_angDragCoefficient; }
	float GetRotInertiaLimit() const { return m_rotInertiaLimit; }
	float GetInertiaScale() const { return m_inertiaScale; }
	bool IsMarkedForDelete() const { return ( m_callbackFlags & CALLBACK_MARKED_FOR_DELETE ) != 0; }
	// Fluid buoyancy uses the object's shapes.
	int GetShapes( b3ShapeId *pShapes, int capacity ) const;

private:
	void CreateShapes();
	void DestroyShapes();
	void ComputeInitialInertia();
	void ApplyMassProperties();
	void ApplyBodyType();
	void ApplyFilter();
	void RecomputeDragBases();
	float GetDragInDirection( const Vector &worldVelocity ) const;
	float GetAngularDragInDirection( const Vector &localAngularRadians ) const;
	void ClampVelocity();

	CPhysicsEnvironmentBox3D *m_pEnv;
	const CPhysCollide *m_pCollide;
	b3BodyId m_body;
	CShadowControllerBox3D *m_pShadow;
	CPlayerControllerBox3D *m_pPlayerController;
	CPhysicsFluidControllerBox3D *m_pFluid;
	void *m_pGameData;
	char m_name[64];
	Vector m_massCenter;
	Vector m_inertia;			// IVP units (kg*m^2), object axes, diagonal
	Vector m_dragBasis;			// per IVP: projected area per mass (m^2/kg), object axes
	Vector m_angDragBasis;
	float m_mass;
	float m_inertiaScale;
	float m_rotInertiaLimit;
	float m_sphereRadius;
	float m_speedDamping;
	float m_rotDamping;
	float m_dragCoefficient;
	float m_angDragCoefficient;
	float m_volume;
	float m_buoyancyRatio;
	float m_friction;
	float m_restitution;
	int m_materialIndex;
	int m_hingeAxis;
	unsigned int m_contents;
	unsigned short m_callbackFlags;
	unsigned short m_gameFlags;
	unsigned short m_gameIndex;
	bool m_isStatic;
	bool m_isTrigger;
	bool m_collisionEnabled;
	bool m_gravityEnabled;
	bool m_dragEnabled;
	bool m_motionEnabled;
	bool m_shadowTempGravityDisable;
	bool m_wasAwake;
	bool m_reportPreStep;
	bool m_hasTouchedDynamic;
	bool m_asleepSinceCreation;
	Vector m_preStepLinear;
	Vector m_preStepAngular;	// world, radians/second
};

inline CPhysicsObjectBox3D *ToBox3D( IPhysicsObject *pObject ) { return static_cast<CPhysicsObjectBox3D *>( pObject ); }

#endif // PHYSICS_OBJECT_H
