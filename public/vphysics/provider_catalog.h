//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef VPHYSICS_PROVIDER_CATALOG_H
#define VPHYSICS_PROVIDER_CATALOG_H

class IPhysics;

// A linked physics provider. A composition root offers the providers its
// product linked and selects one by name (-physics). Each provider has its own
// entry point, so a statically composed product can link both.
struct PhysicsProviderDescriptor
{
	// The -physics name, which is also the provider's module name.
	const char *name;
	IPhysics *( *create )();
};

extern "C" const PhysicsProviderDescriptor *PhysicsIVP_Describe();
extern "C" const PhysicsProviderDescriptor *PhysicsBox3D_Describe();

#endif // VPHYSICS_PROVIDER_CATALOG_H
