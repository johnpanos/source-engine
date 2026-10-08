//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Game modules linked into a statically composed product (iOS; Waf
// --static-composition). The composition root binds them once, before the
// engine creates its mod app systems. The engine then takes the game's
// factories and the app systems the game declares from here instead of
// opening client, server and GameUI by name. Products that bind nothing keep
// the desktop game-extension loading.
//
// The game DLL contract is the legacy CreateInterface ABI (versioned
// interfaces scanned by name), so this header is a legacy ABI package
// (architecture/modules.json legacyAbi).
//
//=============================================================================//

#ifndef ENGINE_LINKED_GAME_MODULES_H
#define ENGINE_LINKED_GAME_MODULES_H

#include "tier1/interface.h"

class IAppSystem;

// An app system a game module declares (IClientDLLSharedAppSystems,
// IServerDLLSharedAppSystems), identified by its interface version.
struct LinkedGameAppSystem
{
	const char *interfaceName;
	IAppSystem *system;
};

struct LinkedGameModules
{
	CreateInterfaceFn client;
	CreateInterfaceFn server;
	CreateInterfaceFn gameUI;
	// Every app system the linked game modules declare. Composition fails if
	// a game module declares one that is not here.
	const LinkedGameAppSystem *appSystems;
	int appSystemCount;
};

// The definitions export these from their module (DLL_EXPORT); MSVC requires
// the declarations to agree (C2375).
#if defined( _MSC_VER )
#define LINKED_GAME_MODULES_EXPORT __declspec( dllexport )
#else
#define LINKED_GAME_MODULES_EXPORT
#endif

// Copies the table. Fails if a required factory is missing or the engine
// already bound or loaded its game modules.
extern "C" LINKED_GAME_MODULES_EXPORT bool Engine_BindLinkedGameModules(
    const LinkedGameModules *pModules );

#endif // ENGINE_LINKED_GAME_MODULES_H
