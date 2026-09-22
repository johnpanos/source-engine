//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance test for explicit and legacy app-system composition.
//
//=============================================================================//

#include "appframework/IAppSystemGroup.h"

#include <stdio.h>
#include <string.h>

namespace
{

int g_Events[16];
int g_EventCount = 0;

void Record( int event )
{
	g_Events[g_EventCount++] = event;
}

class CTestSystem : public CBaseAppSystem<IAppSystem>
{
public:
	CTestSystem( int connectEvent, int initEvent, int shutdownEvent, int disconnectEvent )
	    : m_ConnectEvent( connectEvent ), m_InitEvent( initEvent ),
	      m_ShutdownEvent( shutdownEvent ), m_DisconnectEvent( disconnectEvent )
	{
	}

	virtual bool Connect( CreateInterfaceFn factory )
	{
		Record( m_ConnectEvent );
		return factory( "ExplicitTestSystem001", NULL ) != NULL &&
		       factory( "LegacyTestSystem001", NULL ) != NULL;
	}

	virtual InitReturnVal_t Init()
	{
		Record( m_InitEvent );
		return INIT_OK;
	}

	virtual void Shutdown() { Record( m_ShutdownEvent ); }

	virtual void Disconnect() { Record( m_DisconnectEvent ); }

private:
	int m_ConnectEvent;
	int m_InitEvent;
	int m_ShutdownEvent;
	int m_DisconnectEvent;
};

CTestSystem g_LegacySystem( 2, 5, 7, 10 );

void *LegacyFactory( const char *pName, int *pReturnCode )
{
	const bool found = strcmp( pName, "LegacyTestSystem001" ) == 0;
	if ( pReturnCode )
	{
		*pReturnCode = found ? IFACE_OK : IFACE_FAILED;
	}
	return found ? &g_LegacySystem : NULL;
}

class CTestAppSystemGroup : public CAppSystemGroup
{
public:
	CTestAppSystemGroup() : m_ExplicitSystem( 1, 4, 8, 11 ) {}

	virtual bool Create()
	{
		if ( AddSystem( &m_ExplicitSystem, "ExplicitTestSystem001" ) != &m_ExplicitSystem )
			return false;

		AppSystemFactory_t factory = AddLegacyFactory( LegacyFactory );
		if ( !factory.IsValid() )
			return false;
		return AddSystem( factory, "LegacyTestSystem001" ) == &g_LegacySystem;
	}

	virtual bool PreInit()
	{
		Record( 3 );
		return true;
	}

	virtual int Main()
	{
		Record( 6 );
		return 23;
	}

	virtual void PostShutdown() { Record( 9 ); }

	virtual void Destroy() { Record( 12 ); }

private:
	CTestSystem m_ExplicitSystem;
};

} // namespace

int main()
{
	CTestAppSystemGroup group;
	if ( group.Run() != 23 )
	{
		fprintf( stderr, "appsystemgrouptest: group returned the wrong result\n" );
		return 1;
	}

	if ( g_EventCount != 12 )
	{
		fprintf( stderr, "appsystemgrouptest: expected 12 events, got %d\n", g_EventCount );
		return 1;
	}

	for ( int i = 0; i < g_EventCount; ++i )
	{
		if ( g_Events[i] != i + 1 )
		{
			fprintf( stderr, "appsystemgrouptest: event %d was %d\n", i, g_Events[i] );
			return 1;
		}
	}

	printf( "appsystemgrouptest: explicit and legacy lifecycle order passed\n" );
	return 0;
}
