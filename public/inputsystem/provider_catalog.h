//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef INPUTSYSTEM_PROVIDER_CATALOG_H
#define INPUTSYSTEM_PROVIDER_CATALOG_H

class IInputSystem;

// Describes the provider selected by Waf. Configuration can select only this
// already linked provider; native SDK discovery remains private to inputsystem.
struct InputProviderDescriptor
{
	const char *name;
	IInputSystem *( *create )();
};

extern "C" const InputProviderDescriptor *InputSystem_Describe();

#endif // INPUTSYSTEM_PROVIDER_CATALOG_H
