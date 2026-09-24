//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-controlled time scale. The CS:GO-era engine exposed
//          SetTimescale()/GetTimescale() on IVEngineClient and IVEngineServer
//          so game code (Portal 2's logic_timescale and CGameTimescale) can
//          slow the simulation without enabling cheats. Those interfaces are
//          frozen here, so the engine exposes the same state as its own
//          interface. The host frame time is scaled by this value times the
//          cheat-protected host_timescale.
//
//=============================================================================//

#ifndef IGAMETIMESCALE_H
#define IGAMETIMESCALE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

abstract_class IEngineGameTimescale
{
public:
	// flTimescale must be > 0; 1 is normal speed.
	virtual void SetTimescale( float flTimescale ) = 0;
	virtual float GetTimescale( void ) const = 0;
};

#define VENGINE_GAMETIMESCALE_INTERFACE_VERSION "VEngineGameTimescale001"

#endif // IGAMETIMESCALE_H
