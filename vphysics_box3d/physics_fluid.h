//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D fluid controllers and springs (RFC 0004 D1/C4).
//
//=============================================================================//
#ifndef PHYSICS_FLUID_BOX3D_H
#define PHYSICS_FLUID_BOX3D_H

#include "vphysics_interface.h"
#include "mathlib/mathlib.h"

class CPhysicsObjectBox3D;
class CPhysicsEnvironmentBox3D;

//-----------------------------------------------------------------------------
// A fluid is a trigger object (IVP: a phantom) whose overlapping objects get
// IVP's buoyancy solution each step:
//   buoyancy  = fluid density * object buoyancy ratio * submerged volume * g,
//               along the fluid surface normal at the center of buoyancy
//   damping   = 0.5 * fluid density * damping * A * |v_rel|^2 per submerged
//               surface facing the relative current (IVP's pressure
//               dampening), along the relative current
// Shadow-controlled objects and objects without CALLBACK_DO_FLUID_SIMULATION
// get no buoyancy (IVP's CBuoyancyAttacher). The surface plane and current
// are authored in world space and kept in the fluid object's space, so the
// fluid may move.
//-----------------------------------------------------------------------------
class CPhysicsFluidControllerBox3D : public IPhysicsFluidController
{
public:
	CPhysicsFluidControllerBox3D( CPhysicsObjectBox3D *pFluidObject, const fluidparams_t &params );
	virtual ~CPhysicsFluidControllerBox3D();

	virtual void SetGameData( void *pGameData ) override { m_pGameData = pGameData; }
	virtual void *GetGameData( void ) const override { return m_pGameData; }
	virtual void GetSurfacePlane( Vector *pNormal, float *pDist ) const override;
	virtual float GetDensity() const override { return m_density; }
	virtual void WakeAllSleepingObjects() override;
	virtual int GetContents() const override { return m_contents; }

	CPhysicsObjectBox3D *GetObject() const { return m_pObject; }
	const fluidparams_t &GetParams() const { return m_params; }
	// Applies one step of buoyancy and fluid damping to an overlapping object.
	void ApplyToObject( CPhysicsObjectBox3D *pObject, float dt );

private:
	void GetWorldSurface( Vector *pNormal, float *pDist, Vector *pCurrent ) const;

	CPhysicsObjectBox3D *m_pObject;
	void *m_pGameData;
	fluidparams_t m_params;
	cplane_t m_objectSpacePlane;
	Vector m_objectSpaceCurrent;
	float m_density;		// kg/m^3 (the fluid object's surface property)
	float m_damping;
	int m_contents;
};

//-----------------------------------------------------------------------------
// IVP's actuator spring (IVP_Actuator_Spring), whose force law is unit-free
// in Source units: along the spring, k * (length - natural length) minus
// damping * closing speed, plus "manhattan" damping of the full relative
// velocity by relativeDamping; applied as impulses at the anchors before
// each step. onlyStretch springs only pull.
//-----------------------------------------------------------------------------
class CPhysicsSpringBox3D : public IPhysicsSpring
{
public:
	CPhysicsSpringBox3D( CPhysicsObjectBox3D *pStart, CPhysicsObjectBox3D *pEnd, const springparams_t &params );
	virtual ~CPhysicsSpringBox3D() {}

	virtual void GetEndpoints( Vector *worldPositionStart, Vector *worldPositionEnd ) override;
	virtual void SetSpringConstant( float flSpringConstant ) override { m_constant = flSpringConstant; }
	virtual void SetSpringDamping( float flSpringDamping ) override { m_damping = flSpringDamping; }
	virtual void SetSpringLength( float flSpringLength ) override { m_naturalLength = flSpringLength; }
	virtual IPhysicsObject *GetStartObject( void ) override;
	virtual IPhysicsObject *GetEndObject( void ) override;

	bool Links( IPhysicsObject *pObject ) const;
	// As IVP: a spring whose object is destroyed stops acting and forgets
	// its objects.
	void ObjectDestroyed() { m_pStart = m_pEnd = NULL; }
	void Simulate( float dt );
	void WriteParams( springparams_t &params ) const;

private:
	CPhysicsObjectBox3D *m_pStart;
	CPhysicsObjectBox3D *m_pEnd;
	Vector m_localStart;
	Vector m_localEnd;
	float m_constant;
	float m_naturalLength;
	float m_damping;
	float m_relativeDamping;
	bool m_onlyStretch;
};

#endif // PHYSICS_FLUID_BOX3D_H
