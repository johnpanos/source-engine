//===== Copyright 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: Single-player shared memory: one allocation, found by name and
//          entity index, that the client and server game modules of a
//          single-player (in-process listen server) session both map. The
//          Portal 2 shared variables (game/shared/sharedvar.h) use it to read
//          server-authoritative state on the client without networking.
//
//===========================================================================//

#ifndef ISPSHAREDMEMORY_H
#define ISPSHAREDMEMORY_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/basetypes.h"
#include "tier0/platform.h"

abstract_class ISPSharedMemory
{
public:
	// Allocates iSize zeroed bytes the first time; later calls must pass the
	// same size. Returns true if this call created the memory.
	virtual bool	Init( size_t iSize ) = 0;
	virtual uint8 *	Base( void ) = 0;
	virtual size_t	Size( void ) = 0;

	virtual void	AddRef( void ) = 0;
	// Drops one reference; the memory is freed with the last reference.
	virtual void	Release( void ) = 0;
};

//-----------------------------------------------------------------------------
// Engine-owned registry of shared memory spaces. The CS:GO-era engine exposed
// GetSinglePlayerSharedMemorySpace() on IVEngineClient and IVEngineServer;
// those interfaces are frozen here, so the engine exposes the same registry as
// its own interface. Each returned space carries one reference for the caller.
//-----------------------------------------------------------------------------
abstract_class ISPSharedMemoryManager
{
public:
	virtual ISPSharedMemory *GetSharedMemory( const char *szName, int nEntNum ) = 0;
};

#define VENGINE_SPSHAREDMEMORY_INTERFACE_VERSION "VEngineSPSharedMemory001"

#endif // ISPSHAREDMEMORY_H
