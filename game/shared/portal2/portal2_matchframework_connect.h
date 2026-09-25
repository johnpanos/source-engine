//========= Portal 2 port ======================================================//
//
// Purpose: Portal 2 port: the client-side connection to the matchmaking
//          framework that the server module publishes through the engine's
//          IMatchFrameworkHost. The one declaration of these functions, shared
//          by portal2_shared_compat.h and the GameUI (which does not include
//          that header). Defined in portal2_shared_compat.cpp.
//
//=============================================================================//

#ifndef PORTAL2_MATCHFRAMEWORK_CONNECT_H
#define PORTAL2_MATCHFRAMEWORK_CONNECT_H
#ifdef _WIN32
#pragma once
#endif

typedef void *( *CreateInterfaceFn )( const char *pName, int *pReturnCode );

// Borrows the matchmaking framework the server module published through the
// engine's IMatchFrameworkHost and sets g_pMatchFramework. The engine
// initializes the server module before its GameUI and the client, so the
// GameUI (CGameUI::Initialize) and the client's Init both connect here; a
// module that cannot connect fails its init. Idempotent.
bool Portal2_ConnectMatchFramework( CreateInterfaceFn engineFactory );
// Stops borrowing the framework (client shutdown; the server outlives it).
void Portal2_DisconnectMatchFramework();

#endif // PORTAL2_MATCHFRAMEWORK_CONNECT_H
