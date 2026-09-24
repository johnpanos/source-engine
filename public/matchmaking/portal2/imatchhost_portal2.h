//===== Copyright 1996-2009, Valve Corporation, All rights reserved. ======//
//
// Purpose: Portal 2 port: services the matchmaking framework took from the
//          CS:GO-era IServerGameDLL and INetSupport. This engine's
//          IServerGameDLL is frozen and it has no INetSupport, so the Portal 2
//          server game module implements this interface and registers it with
//          IMatchExtensions under IMATCHSERVERHOST_PORTAL2_INTERFACE.
//
//===========================================================================//

#ifndef IMATCHHOST_PORTAL2_H
#define IMATCHHOST_PORTAL2_H
#ifdef _WIN32
#pragma once
#endif

class KeyValues;

// State of the server running in this process (the INetSupport::ServerInfo_t
// fields this engine can answer; network addresses are not available).
struct MatchServerInfoPortal2_t
{
	bool m_bActive;					// a map is running on the local server
	bool m_bDedicated;
	bool m_bInMainMenuBkgnd;		// the map is a main menu background
	char const *m_szServerName;
	char const *m_szMapName;
	int m_numMaxHumanPlayers;
	int m_numHumanPlayers;
};

abstract_class IMatchServerHostPortal2
{
public:
	// CServerGameDLL::ApplyGameSettings: applies a reservation package (named
	// after the game directory) or the "::ExecGameTypeCfg" request.
	virtual void ApplyGameSettings( KeyValues *pKV ) = 0;

	// INetSupport::GetServerInfo for the local server.
	virtual void GetServerInfo( MatchServerInfoPortal2_t *pServerInfo ) = 0;
};

#define IMATCHSERVERHOST_PORTAL2_INTERFACE "IMatchServerHostPortal2_001"

#endif // IMATCHHOST_PORTAL2_H
