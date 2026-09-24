//========= Portal 2 port ======================================================//
//
// Purpose: Connects the Portal 2 client or server to a VScript manager.
//
//          The engine's app-system factory provides VSCRIPT_INTERFACE_VERSION
//          when an engine composes the vscript module itself; this engine does
//          not, so the game module loads the vscript provider (libvscript) on
//          its own. A manager the game module loaded is owned by it: connected
//          and initialized here, and shut down, disconnected and unloaded by
//          Portal2_DisconnectScriptManager(). An engine-provided manager stays
//          owned by the engine and is only borrowed.
//
//          Client and server each own one reference to the shared library. The
//          manager is stateless apart from its tier1 connection, so the first
//          of them to disconnect leaves VMs created afterwards without the
//          "developer" ConVar; both disconnect only at module shutdown.
//
//          Header-only on purpose: each game module keeps its own state.
//
//=============================================================================//

#ifndef PORTAL2_VSCRIPT_MODULE_H
#define PORTAL2_VSCRIPT_MODULE_H
#pragma once

#include "tier1/interface.h"
#include "vscript/ivscript.h"

struct Portal2ScriptManagerModule_t
{
	CSysModule *m_pModule = NULL;
	IScriptManager *m_pOwnedManager = NULL;
};

inline Portal2ScriptManagerModule_t g_Portal2ScriptManagerModule;

// Returns the engine's script manager, else the one from the vscript module,
// else NULL (scripting then stays disabled). Call once from the module's init.
inline IScriptManager *Portal2_ConnectScriptManager( CreateInterfaceFn appSystemFactory )
{
	IScriptManager *pManager =
	    (IScriptManager *)appSystemFactory( VSCRIPT_INTERFACE_VERSION, NULL );
	if ( pManager )
		return pManager;

	Portal2ScriptManagerModule_t &module = g_Portal2ScriptManagerModule;
	Assert( !module.m_pModule && !module.m_pOwnedManager );
	module.m_pModule = Sys_LoadModule( "vscript" );
	if ( !module.m_pModule )
	{
		Warning( "Portal 2: VScript is unavailable (the vscript module did not load)\n" );
		return NULL;
	}

	CreateInterfaceFn vscriptFactory = Sys_GetFactory( module.m_pModule );
	pManager =
	    vscriptFactory ? (IScriptManager *)vscriptFactory( VSCRIPT_INTERFACE_VERSION, NULL ) : NULL;
	if ( !pManager )
	{
		Warning( "Portal 2: VScript is unavailable (the vscript module does not export %s)\n",
		    VSCRIPT_INTERFACE_VERSION );
	}
	else if ( !pManager->Connect( appSystemFactory ) )
	{
		Warning( "Portal 2: VScript is unavailable (the script manager failed to connect)\n" );
		pManager = NULL;
	}
	else if ( pManager->Init() != INIT_OK )
	{
		Warning( "Portal 2: VScript is unavailable (the script manager failed to initialize)\n" );
		pManager->Disconnect();
		pManager = NULL;
	}

	if ( !pManager )
	{
		Sys_UnloadModule( module.m_pModule );
		module.m_pModule = NULL;
		return NULL;
	}

	module.m_pOwnedManager = pManager;
	return pManager;
}

// Releases what Portal2_ConnectScriptManager() acquired. Every VM must already
// be destroyed (the VScript game system does that in its shutdown).
inline void Portal2_DisconnectScriptManager()
{
	Portal2ScriptManagerModule_t &module = g_Portal2ScriptManagerModule;
	if ( module.m_pOwnedManager )
	{
		module.m_pOwnedManager->Shutdown();
		module.m_pOwnedManager->Disconnect();
		module.m_pOwnedManager = NULL;
	}
	if ( module.m_pModule )
	{
		Sys_UnloadModule( module.m_pModule );
		module.m_pModule = NULL;
	}
}

#endif // PORTAL2_VSCRIPT_MODULE_H
