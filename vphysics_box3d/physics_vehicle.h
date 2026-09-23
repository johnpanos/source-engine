//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Vehicle controllers on Box3D (RFC 0004): the engine-side
//          controller (gearbox, throttle, steering, boost, handbrake) and the
//          car systems it drives, reproducing IVP's IVP_Car_System_Real_Wheels
//          (wheel objects on joints with suspension/torque/stabilizer
//          actuators), CPhysics_Airboat (raycast pontoons) and
//          CPhysics_Car_System_Raycast_Wheels.
//
//          The controller and the car systems work in IVP's units (meters,
//          newtons, radians) and IVP's body axes (x right, y down, z forward)
//          as the original does, so the tuning constants in vehicle scripts
//          keep their meaning; conversion to Box3D happens at the body.
//
//=============================================================================//
#ifndef PHYSICS_VEHICLE_BOX3D_H
#define PHYSICS_VEHICLE_BOX3D_H

#include "vphysics_interface.h"
#include "vphysics/vehicles.h"

class CPhysicsEnvironmentBox3D;
class CPhysicsObjectBox3D;
class CCarSystemBox3D;

// Saved controller state (versioned). Pointers are the saved objects' old
// addresses; the restore remaps them.
struct CVehicleStateBox3D
{
	int version;
	IPhysicsObject *pBody;
	IPhysicsObject *pWheels[VEHICLE_MAX_WHEEL_COUNT];
	int wheelCount;
	vehicleparams_t vehicleData;
	vehicle_operatingparams_t currentState;
	float wheelRadius;
	float bodyMass;
	float totalWheelMass;
	float gravityLength;
	float torqueScale;
	int vehicleFlags;
	unsigned int tireType;
	unsigned int vehicleType;
	bool occupied;
	bool engineDisable;
	Vector bodyVelocity;
};

class CVehicleControllerBox3D : public IPhysicsVehicleController
{
public:
	CVehicleControllerBox3D( CPhysicsEnvironmentBox3D *pEnv, const vehicleparams_t &params, unsigned int vehicleType,
		IPhysicsGameTrace *pGameTrace );
	virtual ~CVehicleControllerBox3D();

	void InitCarSystem( CPhysicsObjectBox3D *pBody );

	// IPhysicsVehicleController
	virtual void Update( float dt, vehicle_controlparams_t &controls ) override;
	virtual const vehicle_operatingparams_t &GetOperatingParams() override { return m_currentState; }
	virtual const vehicleparams_t &GetVehicleParams() override { return m_vehicleData; }
	virtual vehicleparams_t &GetVehicleParamsForChange() override { return m_vehicleData; }
	virtual float UpdateBooster( float dt ) override;
	virtual int GetWheelCount( void ) override { return m_wheelCount; }
	virtual IPhysicsObject *GetWheel( int index ) override;
	virtual bool GetWheelContactPoint( int index, Vector *pContactPoint, int *pSurfaceProps ) override;
	virtual void SetSpringLength( int wheelIndex, float length ) override;
	virtual void SetWheelFriction( int wheelIndex, float friction ) override;
	virtual void OnVehicleEnter( void ) override;
	virtual void OnVehicleExit( void ) override;
	virtual void SetEngineDisabled( bool bDisable ) override { m_engineDisabled = bDisable; }
	virtual bool IsEngineDisabled( void ) override { return m_engineDisabled; }
	virtual void GetCarSystemDebugData( vehicle_debugcarsystem_t &debugCarSystem ) override;
	virtual void VehicleDataReload() override;

	// Environment hooks.
	void Simulate( float dt );
	void ObjectDestroyed( IPhysicsObject *pObject );
	CPhysicsObjectBox3D *GetBody() const { return m_pBody; }

	// Save/restore.
	void WriteState( CVehicleStateBox3D &state );
	static CVehicleControllerBox3D *CreateFromState( CPhysicsEnvironmentBox3D *pEnv, IPhysicsGameTrace *pGameTrace,
		const CVehicleStateBox3D &state );

private:
	CVehicleControllerBox3D();
	void ResetState();
	void CreateCarSystem();
	void ShutdownCarSystem();
	void InitCarSystemWheels();
	CPhysicsObjectBox3D *CreateWheel( int wheelIndex, const vehicle_axleparams_t &axle );

	void UpdateSteering( const vehicle_controlparams_t &controls, float flDeltaTime, float flSpeed );
	void UpdatePowerslide( const vehicle_controlparams_t &controls, bool bPowerslide, float flSpeed );
	void UpdateEngine( const vehicle_controlparams_t &controls, float flDeltaTime, float flThrottle, float flBrake, bool bHandbrake, bool bPowerslide );
	bool UpdateEngineTurboStart( const vehicle_controlparams_t &controls, float flDeltaTime );
	void UpdateEngineTurboFinish( void );
	void UpdateHandbrake( const vehicle_controlparams_t &controls, float flThrottle, bool bHandbrake, bool bPowerslide );
	void UpdateSkidding( bool bHandbrake );
	void UpdateExtraForces( void );
	void UpdateWheelPositions( void );
	float CalcSteering( float dt, float speed, float steering, bool bAnalog );
	void CalcEngine( float throttle, float brake_val, bool handbrake, float steeringVal, bool torqueBoost );
	void CalcEngineTransmission( float flThrottle );
	bool IsBoosting( void );

	CPhysicsEnvironmentBox3D *m_pEnv;
	IPhysicsGameTrace *m_pGameTrace;
	CCarSystemBox3D *m_pCarSystem;
	CPhysicsObjectBox3D *m_pBody;
	CPhysicsObjectBox3D *m_pWheels[VEHICLE_MAX_WHEEL_COUNT];
	Vector m_wheelPositionLocal[VEHICLE_MAX_WHEEL_COUNT];	// Source units, body space
	Vector m_tracePositionLocal[VEHICLE_MAX_WHEEL_COUNT];
	int m_wheelCount;
	vehicleparams_t m_vehicleData;
	vehicle_operatingparams_t m_currentState;
	float m_wheelRadius;		// meters
	float m_bodyMass;
	float m_totalWheelMass;
	float m_gravityLength;		// m/s^2
	float m_torqueScale;
	int m_vehicleFlags;
	unsigned int m_tireType;
	unsigned int m_vehicleType;
	bool m_traceData;
	bool m_occupied;
	bool m_engineDisabled;
};

#endif // PHYSICS_VEHICLE_BOX3D_H
