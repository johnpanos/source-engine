//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef LAUNCHER_MAIN_STATIC_COMPOSITION_H
#define LAUNCHER_MAIN_STATIC_COMPOSITION_H

struct GamePlatformServices;

// Binds the linked game modules to the engine; see static_composition.cpp.
bool StaticComposition_BindGame();

// Hands the root's platform services to the linked client, server and GameUI
// (public/game/game_platform_services.h), before the engine starts them.
bool StaticComposition_BindPlatformServices( const GamePlatformServices &services );

#endif // LAUNCHER_MAIN_STATIC_COMPOSITION_H
