//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// Purpose: Defines a group of app systems that all have the same lifetime
// that need to be connected/initialized, etc. in a well-defined order
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef DEDICATED_H
#define DEDICATED_H

#ifdef _WIN32
#pragma once
#endif

#include "appframework/tier3app.h"
#include "appframework/dedicated_composition_bridge.h"

//-----------------------------------------------------------------------------
// Forward declarations 
//-----------------------------------------------------------------------------
class IDedicatedServerAPI;
class IDedicatedExports;


//-----------------------------------------------------------------------------
// Singleton interfaces 
//-----------------------------------------------------------------------------
extern IDedicatedServerAPI *engine;
IDedicatedExports *Dedicated_GetExports();


extern char g_szEXEName[ MAX_PATH ];


//-----------------------------------------------------------------------------
// Inner loop: initialize, shutdown main systems, load steam to 
//-----------------------------------------------------------------------------
#ifdef POSIX
#define DEDICATED_BASECLASS CTier2SteamApp
#else
#define DEDICATED_BASECLASS CVguiSteamApp
#endif

class CDedicatedAppSystemGroup : public DEDICATED_BASECLASS
{
	typedef DEDICATED_BASECLASS BaseClass;

public:
	// Methods of IApplication
	virtual bool Create( );
	virtual bool PreInit( );
	virtual int Main( );
	virtual void PostShutdown();
	virtual void Destroy();

	// Only the compatibility bridge is registered with CAppSystemGroup. Product
	// systems are registered with ApplicationComposition, which drives lifecycle.
	bool AddComposedSystem( IAppSystem *system, const char *interfaceName )
	{
		return DedicatedComposition_AddSystem( m_Composition, system, interfaceName );
	}

	bool AddComposedSystem( AppModule_t module, const char *interfaceName )
	{
		if ( !AddComposedSystem( BaseClass::CreateSystem( module, interfaceName ), interfaceName ) )
			return false;
		CSysModule *handle = BaseClass::ReleaseModule( module );
		if ( DedicatedComposition_OwnModule( m_Composition, handle, Sys_UnloadModule ) )
			return true;
		if ( handle )
			Sys_UnloadModule( handle );
		return false;
	}

	AppModule_t LoadPhysicsModule( const char *moduleName )
	{
		return BaseClass::LoadModule( moduleName );
	}

	void *FindSystem( const char *pInterfaceName )
	{
		return CSteamAppSystemGroup::FindSystem( pInterfaceName );
	}

private:
	DedicatedComposition *m_Composition = nullptr;
};



#endif // DEDICATED_H
