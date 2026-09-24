//========= Portal 2 port ======================================================//
//
// Purpose: The Portal 2 server game module owns the process's matchmaking
//          framework (matchmaking/, the CS:GO framework and Portal 2 title).
//
// Retail Portal 2 loaded matchmaking as an engine app system; this engine
// does not. The server module initializes before the engine's GameUI and the
// client module (Host_Init: SV_InitGameDLL, EngineVGui()->Init, ClientDLL_Init)
// and shuts down after them, so it creates the framework in DLLInit,
// publishes it through the engine's IMatchFrameworkHost, and withdraws it in
// DLLShutdown. The client borrows it from the engine.
//
// It also implements IMatchServerHostPortal2, the services the framework took
// from the CS:GO-era IServerGameDLL (ApplyGameSettings) and INetSupport
// (GetServerInfo). ApplyGameSettings is reconstructed from the retail Portal 2
// (depot 852) server's CServerGameDLL::ApplyGameSettings: its control flow,
// strings and ConVars. Not original Valve source; the repository's
// provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_matchmaking.h"
#include "engine/imatchframeworkhost.h"
#include "matchmaking/imatchframework.h"
#include "matchmaking/portal2/imatchhost_portal2.h"
#include "tier0/icommandline.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// matchmaking/main.cpp: the framework linked into this module.
extern IMatchFramework *MatchFramework_Portal2();

//-----------------------------------------------------------------------------
// One-time report of a CS:GO engine feature this engine does not provide.
//-----------------------------------------------------------------------------
#define PORTAL2_UNSUPPORTED( feature ) \
	do \
	{ \
		static bool s_bWarned = false; \
		if ( !s_bWarned ) \
		{ \
			s_bWarned = true; \
			DevWarning( "Portal 2: " feature " is not supported by this engine\n" ); \
		} \
	} while ( 0 )

extern ConVar sv_portal_players;
extern ConVar sv_force_transmit_ents;
extern ConVar *sv_cheats;

//-----------------------------------------------------------------------------
// Server host adapter
//-----------------------------------------------------------------------------
class CMatchServerHostPortal2 : public IMatchServerHostPortal2
{
public:
	virtual void ApplyGameSettings( KeyValues *pKV );
	virtual void GetServerInfo( MatchServerInfoPortal2_t *pServerInfo );
};

static CMatchServerHostPortal2 s_MatchServerHost;

void CMatchServerHostPortal2::ApplyGameSettings( KeyValues *pKV )
{
	if ( !pKV )
		return;

	bool bAlreadyLoadingMap = false;

	if ( !V_stricmp( pKV->GetName(), "::ExecGameTypeCfg" ) )
	{
		// The engine's "map" command asks the server to derive the session's
		// settings from the map being loaded.
		if ( !engine )
			return;

		char const *szBspName = pKV->GetString( "map/mapname", "" );
		if ( !szBspName || !*szBspName )
			return;

		// Portal 2 port: retail first inspects the map command's launch options
		// (IVEngineServer::GetLaunchOptions: "changelevel", "changelevel2",
		// "map_background", "reserved", "*mp", "*sp"). This engine's frozen
		// IVEngineServer has no launch options, so the player count is always
		// derived from the map name, which is retail's final fallback.
		PORTAL2_UNSUPPORTED( "map launch option inspection" );

		int numSlots = 1;
		if ( V_stristr( szBspName, "coop" ) )
			numSlots = CommandLine()->FindParm( "-allowspectators" ) ? 3 : 2;

		pKV->SetName( "portal2" );
		pKV->SetInt( "members/numSlots", numSlots );
		DevMsg( "  Adjusting game players %d.\n", numSlots );
		bAlreadyLoadingMap = true;
	}

	if ( !V_stricmp( pKV->GetName(), "portal2" ) )
	{
		// A reservation package from the matchmaking title (named after the
		// game directory), or the adjusted "::ExecGameTypeCfg" request.
		int numSlots = pKV->GetInt( "members/numSlots", 0 );
		if ( numSlots )
		{
			sv_portal_players.SetValue( numSlots );

			static ConVarRef coop_ref( "coop" );
			if ( coop_ref.IsValid() )
			{
				coop_ref.SetValue( numSlots >= 2 ? 1 : 0 );
			}
			DevMsg( "            game players = %d\n", numSlots );
		}

		char const *szMap = pKV->GetString( "game/map", "" );
		if ( szMap && *szMap && !bAlreadyLoadingMap )
		{
			// Development-only ConVars lose that flag in developer mode, so a
			// set flag means a retail session: it starts without cheats.
			if ( sv_force_transmit_ents.IsFlagSet( FCVAR_DEVELOPMENTONLY ) && sv_cheats && sv_cheats->GetInt() )
			{
				sv_cheats->SetValue( 0 );
			}

			char const *szMapCommand = pKV->GetString( "map/mapcommand", "map" );
			if ( !V_stricmp( szMapCommand, "ss_map" ) )
			{
				// Portal 2 port: this engine has no split-screen map command.
				PORTAL2_UNSUPPORTED( "split-screen map loading" );
				szMapCommand = "map";
			}
			DevMsg( "            starting: %s %s...\n", szMapCommand, szMap );
			engine->ServerCommand( CFmtStr( "%s %s reserved\n", szMapCommand, szMap ) );
		}
	}
}

void CMatchServerHostPortal2::GetServerInfo( MatchServerInfoPortal2_t *pServerInfo )
{
	V_memset( pServerInfo, 0, sizeof( *pServerInfo ) );

	// A map is running on this module's server once gpGlobals names it.
	char const *szMapName = gpGlobals ? STRING( gpGlobals->mapname ) : NULL;
	pServerInfo->m_bActive = szMapName && *szMapName;
	pServerInfo->m_bDedicated = engine->IsDedicatedServer();
	pServerInfo->m_bInMainMenuBkgnd = pServerInfo->m_bActive && gpGlobals->eLoadType == MapLoad_Background;

	static ConVarRef hostname( "hostname" );
	pServerInfo->m_szServerName = hostname.IsValid() ? hostname.GetString() : "";
	pServerInfo->m_szMapName = szMapName ? szMapName : "";

	pServerInfo->m_numMaxHumanPlayers = gpGlobals ? gpGlobals->maxClients : 0;
	int numHumans = 0;
	for ( int i = 1; pServerInfo->m_bActive && i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer && pPlayer->IsConnected() && !pPlayer->IsFakeClient() )
			++numHumans;
	}
	pServerInfo->m_numHumanPlayers = numHumans;
}

//-----------------------------------------------------------------------------
// Framework ownership
//-----------------------------------------------------------------------------
static IMatchFrameworkHost *s_pMatchFrameworkHost = NULL;
static bool s_bMatchFrameworkConnected = false;
static bool s_bMatchFrameworkInitialized = false;

bool Portal2_InitMatchFramework( CreateInterfaceFn appSystemFactory, IServerGameDLL *pServerGameDLL )
{
	s_pMatchFrameworkHost = (IMatchFrameworkHost *)appSystemFactory( VENGINE_MATCHFRAMEWORKHOST_INTERFACE_VERSION, NULL );
	if ( !s_pMatchFrameworkHost )
	{
		Warning( "Portal 2: the engine does not provide %s\n", VENGINE_MATCHFRAMEWORKHOST_INTERFACE_VERSION );
		return false;
	}

	IMatchFramework *pFramework = MatchFramework_Portal2();

	// The framework reads g_pMatchFramework while it connects (local players).
	g_pMatchFramework = pFramework;

	if ( !pFramework->Connect( appSystemFactory ) )
	{
		Warning( "Portal 2: the matchmaking framework failed to connect\n" );
		Portal2_ShutdownMatchFramework();
		return false;
	}
	s_bMatchFrameworkConnected = true;

	if ( pFramework->Init() != INIT_OK )
	{
		Warning( "Portal 2: the matchmaking framework failed to initialize\n" );
		Portal2_ShutdownMatchFramework();
		return false;
	}
	s_bMatchFrameworkInitialized = true;

	IMatchExtensions *pExtensions = pFramework->GetMatchExtensions();
	pExtensions->RegisterExtensionInterface( IMATCHSERVERHOST_PORTAL2_INTERFACE, &s_MatchServerHost );
	pExtensions->RegisterExtensionInterface( INTERFACEVERSION_SERVERGAMEDLL, pServerGameDLL );

	if ( !s_pMatchFrameworkHost->PublishMatchFramework( pFramework ) )
	{
		Warning( "Portal 2: another matchmaking framework is already published\n" );
		Portal2_ShutdownMatchFramework();
		return false;
	}
	return true;
}

void Portal2_ShutdownMatchFramework()
{
	IMatchFramework *pFramework = g_pMatchFramework;
	if ( !pFramework )
		return;

	if ( s_pMatchFrameworkHost && s_pMatchFrameworkHost->GetMatchFramework() == pFramework )
	{
		s_pMatchFrameworkHost->WithdrawMatchFramework( pFramework );
	}

	if ( s_bMatchFrameworkInitialized )
	{
		IMatchExtensions *pExtensions = pFramework->GetMatchExtensions();
		if ( pExtensions->GetRegisteredExtensionInterface( INTERFACEVERSION_SERVERGAMEDLL ) )
			pExtensions->UnregisterExtensionInterface( INTERFACEVERSION_SERVERGAMEDLL, pExtensions->GetRegisteredExtensionInterface( INTERFACEVERSION_SERVERGAMEDLL ) );
		if ( pExtensions->GetRegisteredExtensionInterface( IMATCHSERVERHOST_PORTAL2_INTERFACE ) )
			pExtensions->UnregisterExtensionInterface( IMATCHSERVERHOST_PORTAL2_INTERFACE, &s_MatchServerHost );

		pFramework->Shutdown();
		s_bMatchFrameworkInitialized = false;
	}
	if ( s_bMatchFrameworkConnected )
	{
		pFramework->Disconnect();
		s_bMatchFrameworkConnected = false;
	}

	g_pMatchFramework = NULL;
	s_pMatchFrameworkHost = NULL;
}

void Portal2_MatchFrameworkServerFrame()
{
	// A listen server's client runs the framework's frame; a dedicated server
	// has no client, so it runs it here.
	if ( g_pMatchFramework && engine->IsDedicatedServer() )
	{
		g_pMatchFramework->RunFrame();
	}
}
