//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Opt-in shape inertia capability (RFC 0013, vphysics.shape-inertia.v1).
//
//          The legacy VPhysics inertia model (IVP's) keeps one inertia value
//          per object axis: it drops the products of inertia, so a body whose
//          object axes are not its principal axes spins as if they were, and
//          it raises every axis to at least rotInertiaLimit times the inertia
//          vector's length, so slender bodies turn like fat ones. Its
//          per-axis values are IVP's approximation, not the solid's inertia.
//          Rotation, and with it the gyroscopic torque, then follows that
//          simplified tensor.
//
//          A provider with this capability can instead give an environment's
//          objects the full inertia tensor of their collision shapes (uniform
//          density, about the object's mass center, products of inertia
//          included, no rotInertiaLimit), and its solver rotates them with
//          it. Reached through IPhysics::QueryInterface; a provider without it
//          (IVP) returns NULL, and every environment keeps the legacy model.
//          Nothing in IPhysics, IPhysicsEnvironment or IPhysicsObject changes.
//
//          Contract (every claiming provider):
//          - An environment starts with PHYSICS_INERTIA_LEGACY, which is
//            exactly the legacy behavior, and GetInertiaTensor reports its
//            objects' tensors as diagonal, equal to GetInertia.
//          - SetInertiaModel returns false and changes nothing for a NULL or
//            foreign environment, an unknown model, or an environment that
//            already holds an object. The model is a composition choice made
//            before the first object, not a runtime toggle.
//          - An object keeps the model of the environment that created it,
//            including through TransferObject; an object restored from a save
//            takes the model of the environment it is restored into.
//          - Under PHYSICS_INERTIA_SHAPE, an object with a collision model has
//            the tensor of its solid scaled to its mass and objectparams_t
//            inertia scale; rotInertiaLimit is not applied. A sphere's tensor
//            is 2/5 m r^2 on each axis. The solver's rotation uses the tensor:
//            torque-free motion conserves the world angular momentum it
//            implies.
//          - The per-axis IPhysicsObject API keeps its meaning under both
//            models: GetInertia is the tensor's diagonal in object axes;
//            GetInvInertia is the inverse tensor's diagonal; SetInertia sets
//            the diagonal and keeps the axes' coupling (the tensor becomes
//            D^1/2 C D^1/2, C the unit-diagonal coupling), so saving
//            GetInertia and setting it back restores the tensor; SetMass
//            scales the tensor. GetEnergy, CalculateVelocityOffset and
//            force-mode motion controllers use the full tensor.
//          - GetInertiaTensor returns false and leaves the output untouched
//            for a NULL or foreign object.
//
//=============================================================================//

#ifndef VPHYSICS_SHAPE_INERTIA_H
#define VPHYSICS_SHAPE_INERTIA_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class IPhysicsEnvironment;
class IPhysicsObject;

#define VPHYSICS_SHAPE_INERTIA_INTERFACE_VERSION "VPhysicsShapeInertia001"

enum physics_inertia_model_t
{
	PHYSICS_INERTIA_LEGACY = 0, // per-axis inertia, rotInertiaLimit applied (IVP)
	PHYSICS_INERTIA_SHAPE = 1,  // the collision solid's full tensor
};

abstract_class IPhysicsShapeInertia
{
public:
	virtual bool SetInertiaModel(
	    IPhysicsEnvironment * pEnvironment, physics_inertia_model_t model ) = 0;

	// PHYSICS_INERTIA_LEGACY for a NULL or foreign environment.
	virtual physics_inertia_model_t GetInertiaModel( const IPhysicsEnvironment *pEnvironment )
	    const = 0;

	// The object's inertia tensor about its mass center in object axes, in
	// the units of IPhysicsObject::GetInertia (kg*m^2); tensor[row][column].
	virtual bool GetInertiaTensor( const IPhysicsObject *pObject, float tensor[3][3] ) const = 0;
};

#endif // VPHYSICS_SHAPE_INERTIA_H
