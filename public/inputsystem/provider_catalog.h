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

// The definitions export these from their module (DLL_EXPORT); MSVC requires
// the declarations to agree (C2375).
#if defined( _MSC_VER )
#define INPUT_PROVIDER_CATALOG_EXPORT __declspec( dllexport )
#else
#define INPUT_PROVIDER_CATALOG_EXPORT
#endif

extern "C" INPUT_PROVIDER_CATALOG_EXPORT const InputProviderDescriptor *InputSystem_Describe();

#endif // INPUTSYSTEM_PROVIDER_CATALOG_H
