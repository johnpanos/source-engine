//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared host for the VPhysics provider conformance suite
//          (RFC 0004 / RFC 0005 Q-PHYSICS). The suite is split by clause
//          family across several translation units; this header is the one
//          owner of the reporting protocol, the provider under test and the
//          world fixtures they share. See test_vphysics_conformance.cpp for
//          the output protocol and unittests/physicstest/contracts for the
//          contract record.
//
//=============================================================================//
#ifndef VPHYSICS_CONFORMANCE_H
#define VPHYSICS_CONFORMANCE_H

#include "tier0/dbg.h"
#include "mathlib/mathlib.h"
#include "vphysics_interface.h"

enum Tier_t
{
	TIER_BOOT = 0,
	TIER_GAMEPLAY,
	TIER_COUNT
};

// Records one check result; returns ok.
bool Check( Tier_t tier, const char *pName, bool ok, const char *pFormat = NULL, ... );

// Observations for cross-provider comparison. Tolerances: "x" exact,
// "a<abs>" absolute, "r<frac>" relative.
void ObsFloats( const char *pKey, const char *pTol, int count, const float *pValues );
void ObsVector( const char *pKey, const char *pTol, const Vector &v );
void ObsString( const char *pKey, const char *pValue );

bool Near( float a, float b, float tol );
bool NearVec( const Vector &a, const Vector &b, float tol );
bool IsFiniteVec( const Vector &v );

// Sensitivity: the active --fault name ("" when none).
bool FaultIs( const char *pName );

// Provider under test (collision and surface props are the fault decorators).
extern IPhysics *s_pPhysics;
extern IPhysicsCollision *s_pCollision;
extern IPhysicsSurfaceProps *s_pProps;

// Fixed simulation tick; every Simulate call goes through Step() so the
// "sim-noop" fault can model a provider whose step does not advance.
extern const float kTick;
void Step( IPhysicsEnvironment *pEnv, float seconds );

objectparams_t DefaultParams( float mass, void *pGameData );

// A world with gravity (0,0,-600), a static 1024x1024 floor slab whose top is
// z=0, and a shared 32-unit cube collide.
struct World_t
{
	IPhysicsEnvironment *pEnv;
	CPhysCollide *pFloorCollide;
	CPhysCollide *pCubeCollide;
	IPhysicsObject *pFloor;
	int material;
};

bool CreateWorld( World_t &world, IPhysicsCollisionSolver *pSolver );
IPhysicsObject *CreateCube( World_t &world, const Vector &position, float mass = 50.0f, void *pGameData = NULL );
void DestroyWorld( World_t &world );
Vector PositionOf( IPhysicsObject *pObject );
Vector VelocityOf( IPhysicsObject *pObject );
// Local-space angular velocity in degrees/second, as VPhysics reports it.
AngularImpulse AngularVelocityOf( IPhysicsObject *pObject );
// World-space direction of the object's local axis (0=x, 1=y, 2=z).
Vector AxisOf( IPhysicsObject *pObject, int axis );

// Clause families implemented in their own translation units.
void TestConstraints();
void TestObjectsAndEvents();
void TestFluidsAndSprings();
void TestSaveRestore();
// pFixture: a loaded authored model (may be NULL when none loaded).
void TestPlayerController();
void TestCollideModels( const struct vcollide_t *pFixture );

#endif // VPHYSICS_CONFORMANCE_H
