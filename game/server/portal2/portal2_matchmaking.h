//========= Portal 2 port ======================================================//
//
// Purpose: The server game module's ownership of the matchmaking framework
//          (see portal2_matchmaking.cpp).
//
//=============================================================================//

#ifndef PORTAL2_MATCHMAKING_H
#define PORTAL2_MATCHMAKING_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"

class IServerGameDLL;

// Creates, connects and initializes the framework, registers this module's
// matchmaking extensions and publishes it to the engine. Sets
// g_pMatchFramework. On failure everything done is undone and false returned.
bool Portal2_InitMatchFramework( CreateInterfaceFn appSystemFactory, IServerGameDLL *pServerGameDLL );

// Withdraws, shuts down and disconnects the framework; clears g_pMatchFramework.
void Portal2_ShutdownMatchFramework();

// Runs the framework's frame on a dedicated server (the client runs it otherwise).
void Portal2_MatchFrameworkServerFrame();

#endif // PORTAL2_MATCHMAKING_H
