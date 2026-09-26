//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Engine-private access to the game modules a statically composed product
// bound (public/engine/linked_game_modules.h).
//
//=============================================================================//

#ifndef LINKED_GAME_MODULES_INTERNAL_H
#define LINKED_GAME_MODULES_INTERNAL_H

#include "engine/linked_game_modules.h"

// The bound modules, or NULL when the product loads its game modules by name.
const LinkedGameModules *Engine_GetLinkedGameModules();

// The bound app system implementing interfaceName, or NULL.
IAppSystem *Engine_FindLinkedGameAppSystem( const char *pInterfaceName );

#endif // LINKED_GAME_MODULES_INTERNAL_H
