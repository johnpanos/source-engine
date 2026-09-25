//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Opt-in step profile capability (RFC 0013, vphysics.step-profile.v1).
//
//          Reports where the last IPhysicsEnvironment::Simulate call spent its
//          time: inside the backend's solver step, or in the provider's own
//          serial work before and after it (controllers, event dispatch). It
//          attributes the serial fraction that limits parallel stepping.
//          Reached through IPhysics::QueryInterface; a provider without it
//          (IVP) returns NULL.
//
//          Contract (every claiming provider):
//          - GetLastSimulate returns false and leaves *pProfile untouched for
//            a NULL, foreign or destroyed environment, and true otherwise.
//          - Before the first Simulate every field is zero. Afterwards the
//            times are non-negative wall-clock milliseconds of the last call,
//            and stepMs + preStepMs + postStepMs <= simulateMs.
//          - Reading the profile does not change the simulation.
//          - The solver counts are cumulative since the environment was
//            created, and offCaller <= calls.
//
//=============================================================================//

#ifndef VPHYSICS_STEP_PROFILE_H
#define VPHYSICS_STEP_PROFILE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class IPhysicsEnvironment;

#define VPHYSICS_STEP_PROFILE_INTERFACE_VERSION "VPhysicsStepProfile001"

struct physics_stepprofile_t
{
	int stepCount;      // fixed steps taken by the last Simulate call
	float simulateMs;   // the whole call
	float stepMs;       // inside the backend's solver steps
	float preStepMs;    // provider work before the solver (controllers, springs, fluids)
	float postStepMs;   // provider work after the solver (event dispatch)
	int bodyCount;      // bodies in the environment after the call
	int awakeBodyCount; // bodies awake after the call
	int contactCount;   // contact pairs (overlapping bounds) after the call
	int workerCount;    // workers stepping the environment
	// Calls into the game's IPhysicsCollisionSolver::ShouldCollide since the
	// environment was created, and how many ran on a thread other than the
	// one that called Simulate (RFC 0013 P3 evidence).
	int solverCalls;
	int solverCallsOffCaller;
};

abstract_class IPhysicsStepProfile
{
public:
	virtual bool GetLastSimulate(
	    const IPhysicsEnvironment *pEnvironment, physics_stepprofile_t *pProfile ) const = 0;
};

#endif // VPHYSICS_STEP_PROFILE_H
