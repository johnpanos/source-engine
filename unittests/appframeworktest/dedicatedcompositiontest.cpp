//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dedicated product lifecycle and frozen IAppSystem lookup contract.
//
//=============================================================================

#include "appframework/dedicated_composition_bridge.h"
#include "appframework/IAppSystemGroup.h"
#include "idedicatedexports.h"
#include "dedicated/exports_service.h"

#include <cstdio>
#include <cstring>

namespace
{

IAppSystem *g_Bridge = nullptr;
int g_Events[32];
int g_EventCount = 0;

void Record( int event )
{
	g_Events[g_EventCount++] = event;
}

void *TestFactory( const char *name, int *returnCode )
{
	void *result = g_Bridge ? g_Bridge->QueryInterface( name ) : nullptr;
	if ( returnCode )
		*returnCode = result ? IFACE_OK : IFACE_FAILED;
	return result;
}

class TestSystem final : public CBaseAppSystem<IAppSystem>
{
public:
	TestSystem( int number, const char *peer ) : m_Number( number ), m_Peer( peer ) {}

	bool Connect( CreateInterfaceFn factory ) override
	{
		Record( m_Number * 10 + 1 );
		return !failConnect && factory( m_Peer, nullptr ) != nullptr;
	}

	InitReturnVal_t Init() override
	{
		Record( m_Number * 10 + 2 );
		return failInit ? INIT_FAILED : INIT_OK;
	}

	void Shutdown() override { Record( m_Number * 10 + 3 ); }
	void Disconnect() override { Record( m_Number * 10 + 4 ); }

	void *QueryInterface( const char *name ) override
	{
		return std::strcmp( name, "TestAlias001" ) == 0 ? this : nullptr;
	}

	bool failConnect = false;
	bool failInit = false;

private:
	int m_Number;
	const char *m_Peer;
};

int g_ServiceLive = 0;
int g_Prints = 0;
int g_Runs = 0;
bool g_FailExportsFactory = false;
IDedicatedExportsService *g_ExportsService = nullptr;
int g_ModuleUnloads = 0;
bool g_ModuleUnloadedAfterTeardown = false;

void UnloadModule( CSysModule *module )
{
	if ( module && g_ServiceLive == 0 && g_ExportsService == nullptr )
		g_ModuleUnloadedAfterTeardown = true;
	++g_ModuleUnloads;
}

class TestExportsService final : public IDedicatedExportsService
{
public:
	TestExportsService() { ++g_ServiceLive; }
	~TestExportsService() override { --g_ServiceLive; }
	void Print( char * ) override { ++g_Prints; }
	void RunServer() override { ++g_Runs; }
};

IDedicatedExportsService *CreateExportsService()
{
	return g_FailExportsFactory ? nullptr : new TestExportsService;
}

void BindExportsService( IDedicatedExportsService *service )
{
	g_ExportsService = service;
}

class TestExportsFacade final : public CBaseAppSystem<IDedicatedExports>
{
public:
	void Sys_Printf( char *text ) override
	{
		if ( g_ExportsService )
			g_ExportsService->Print( text );
	}
	void RunServer() override
	{
		if ( g_ExportsService )
			g_ExportsService->RunServer();
	}
};

class TestProductGroup final : public CAppSystemGroup
{
public:
	explicit TestProductGroup( TestSystem &system ) : m_System( system ) {}

	bool Create() override
	{
		m_Composition = DedicatedComposition_Create( GetFactory() );
		return m_Composition &&
		       AddSystem( DedicatedComposition_LegacyBridge( m_Composition ),
		           "DedicatedCompositionBridge001" ) &&
		       DedicatedComposition_AddSystem( m_Composition, &m_System, "TestFirst001" );
	}

	bool PreInit() override
	{
		m_SawConnectedProvider = GetFactory()( "TestFirst001", nullptr ) == &m_System;
		return m_SawConnectedProvider;
	}

	int Main() override { return 42; }
	void PostShutdown() override {}
	void Destroy() override
	{
		DedicatedComposition_Destroy( m_Composition );
		m_Composition = nullptr;
		m_Destroyed = true;
	}

	bool Destroyed() const { return m_Destroyed; }
	bool SawConnectedProvider() const { return m_SawConnectedProvider; }

private:
	TestSystem &m_System;
	DedicatedComposition *m_Composition = nullptr;
	bool m_Destroyed = false;
	bool m_SawConnectedProvider = false;
};

bool EventsEqual( const int *expected, int count )
{
	if ( g_EventCount != count )
		return false;
	for ( int index = 0; index < count; ++index )
		if ( g_Events[index] != expected[index] )
			return false;
	return true;
}

bool Check( bool condition, const char *message )
{
	if ( !condition )
		std::fprintf( stderr, "dedicatedcompositiontest: %s\n", message );
	return condition;
}

} // namespace

int main()
{
	TestSystem first( 1, "TestSecond001" );
	TestSystem second( 2, "TestFirst001" );
	DedicatedComposition *composition = DedicatedComposition_Create( TestFactory );
	if ( !Check( composition != nullptr, "could not create composition" ) )
		return 1;
	g_Bridge = DedicatedComposition_LegacyBridge( composition );
	if ( !Check( DedicatedComposition_AddSystem( composition, &first, "TestFirst001" ) &&
	                 DedicatedComposition_AddSystem( composition, &second, "TestSecond001" ) &&
	                 !DedicatedComposition_AddSystem( composition, &first, "TestFirst001" ),
	         "registration and duplicate rejection" ) ||
	     !Check( g_Bridge->QueryInterface( "TestFirst001" ) == nullptr,
	         "unconnected provider must be hidden" ) )
		return 1;

	if ( !Check( g_Bridge->Connect( TestFactory ), DedicatedComposition_Error( composition ) ) ||
	     !Check( g_Bridge->QueryInterface( "TestFirst001" ) == &first &&
	                 g_Bridge->QueryInterface( "TestSecond001" ) == &second &&
	                 g_Bridge->QueryInterface( "TestAlias001" ) == &first,
	         "connected legacy interface lookup" ) ||
	     !Check( g_Bridge->Init() == INIT_OK, DedicatedComposition_Error( composition ) ) )
		return 1;
	g_Bridge->Shutdown();
	g_Bridge->Disconnect();
	constexpr int normal[] = { 11, 21, 12, 22, 23, 13, 24, 14 };
	if ( !Check( EventsEqual( normal, 8 ), "normal lifecycle order" ) ||
	     !Check( g_Bridge->QueryInterface( "TestFirst001" ) == nullptr,
	         "disconnected provider must be hidden" ) )
		return 1;

	g_EventCount = 0;
	second.failConnect = true;
	if ( !Check( !g_Bridge->Connect( TestFactory ), "connect failure was not propagated" ) )
		return 1;
	constexpr int failedConnect[] = { 11, 21, 24, 14 };
	if ( !Check( EventsEqual( failedConnect, 4 ), "connect failure did not roll back" ) ||
	     !Check( g_Bridge->QueryInterface( "TestFirst001" ) == nullptr,
	         "failed connection published a provider" ) )
		return 1;
	second.failConnect = false;

	g_EventCount = 0;
	second.failInit = true;
	if ( !Check( g_Bridge->Connect( TestFactory ) && g_Bridge->Init() == INIT_FAILED,
	         "initialization failure was not propagated" ) )
		return 1;
	constexpr int failedInit[] = { 11, 21, 12, 22, 23, 13, 24, 14 };
	if ( !Check( EventsEqual( failedInit, 8 ), "initialization failure did not roll back" ) ||
	     !Check( g_Bridge->QueryInterface( "TestFirst001" ) == nullptr,
	         "failed initialization published a provider" ) )
		return 1;
	second.failInit = false;

	g_EventCount = 0;
	if ( !Check( g_Bridge->Connect( TestFactory ) && g_Bridge->Init() == INIT_OK,
	         "composition could not restart" ) )
		return 1;
	if ( !Check( DedicatedComposition_Error( composition )[0] == '\0',
	         "successful retry retained a stale error" ) )
		return 1;
	g_Bridge->Shutdown();
	g_Bridge->Disconnect();
	if ( !Check( EventsEqual( normal, 8 ), "restart lifecycle order" ) )
		return 1;
	g_Bridge = nullptr;
	DedicatedComposition_Destroy( composition );

	TestExportsFacade facade;
	composition = DedicatedComposition_Create( TestFactory );
	g_Bridge = DedicatedComposition_LegacyBridge( composition );
	int moduleStorage = 0;
	auto *module = reinterpret_cast<CSysModule *>( &moduleStorage );
	if ( !Check(
	         DedicatedComposition_AddExports( composition, &facade,
	             VENGINE_DEDICATEDEXPORTS_API_VERSION, CreateExportsService, BindExportsService ),
	         "canonical exports registration" ) ||
	     !Check( DedicatedComposition_OwnModule( composition, module, UnloadModule ),
	         "module ownership transfer" ) ||
	     !Check(
	         g_Bridge->Connect( TestFactory ) && g_ServiceLive == 1 && g_ExportsService != nullptr,
	         "composition owned exports service was not connected" ) )
		return 1;
	auto *legacy = static_cast<IDedicatedExports *>(
	    g_Bridge->QueryInterface( VENGINE_DEDICATEDEXPORTS_API_VERSION ) );
	char message[] = "example";
	if ( !Check( legacy == &facade, "preserved exports interface lookup" ) )
		return 1;
	legacy->Sys_Printf( message );
	legacy->RunServer();
	if ( !Check( g_Prints == 1 && g_Runs == 1 && g_Bridge->Init() == INIT_OK,
	         "legacy exports did not forward to canonical service" ) )
		return 1;
	g_Bridge->Shutdown();
	g_Bridge->Disconnect();
	if ( !Check( g_ServiceLive == 0 && g_ExportsService == nullptr &&
	                 g_Bridge->QueryInterface( VENGINE_DEDICATEDEXPORTS_API_VERSION ) == nullptr,
	         "canonical exports were not torn down before the legacy view" ) )
		return 1;
	g_FailExportsFactory = true;
	if ( !Check( !g_Bridge->Connect( TestFactory ) && g_ServiceLive == 0 &&
	                 g_ExportsService == nullptr &&
	                 std::strstr( DedicatedComposition_Error( composition ), "null" ) != nullptr,
	         "canonical construction failure did not roll back" ) )
		return 1;
	g_FailExportsFactory = false;
	if ( !Check( g_Bridge->Connect( TestFactory ) && g_Bridge->Init() == INIT_OK,
	         "canonical service could not restart after construction failure" ) )
		return 1;
	g_Bridge->Shutdown();
	g_Bridge->Disconnect();
	g_Bridge = nullptr;
	DedicatedComposition_Destroy( composition );
	if ( !Check( g_ModuleUnloads == 1 && g_ModuleUnloadedAfterTeardown,
	         "module unloaded before dependent providers were destroyed" ) )
		return 1;

	TestSystem productSystem( 1, "TestFirst001" );
	for ( int mode = 0; mode != 3; ++mode )
	{
		g_EventCount = 0;
		productSystem.failConnect = mode == 1;
		productSystem.failInit = mode == 2;
		TestProductGroup product( productSystem );
		const int result = product.Run();
		if ( !Check( result == ( mode == 0 ? 42 : -1 ) && product.Destroyed() &&
		                 product.SawConnectedProvider() == ( mode != 1 ),
		         "product host did not roll back or publish at the correct phase" ) )
			return 1;
	}
	std::puts( "dedicatedcompositiontest: passed" );
	return 0;
}
