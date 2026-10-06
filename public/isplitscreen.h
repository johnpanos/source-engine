//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Local split-screen between the engine and a client DLL. Two optional
//          interfaces beside the frozen ones (IVEngineClient, VClient017 keep
//          their vtables): the engine answers slot queries, the client DLL hears
//          slot changes. A client DLL that never asks sees one local player.
//
//          The contract (slot validity, the active-slot scope, entity ownership)
//          is engine.splitscreen-slots.v1 in
//          unittests/enginetest/contracts/engine.splitscreen-slots.v1.md.
//
//=============================================================================//

#ifndef ISPLITSCREEN_H
#define ISPLITSCREEN_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

#define ENGINE_SPLITSCREEN_INTERFACE_VERSION "VEngineSplitScreen001"

// Exposed by the engine through its interface factory.
abstract_class IEngineSplitScreen
{
public:
	// The slot whose context is current on the calling thread (0 outside any scope).
	virtual int		GetActiveSplitScreenPlayerSlot() = 0;
	// Makes `slot` current on this thread; returns the previous slot.
	virtual int		SetActiveSplitScreenPlayerSlot( int slot ) = 0;
	// Entity index of the local player in `slot`, or -1 if the slot is not in use.
	virtual int		GetSplitScreenPlayer( int nSlot ) = 0;
	// True when more than one local player is in use.
	virtual bool	IsSplitScreenActive() = 0;
	virtual bool	IsValidSplitScreenSlot( int nSlot ) = 0;
	virtual int		FirstValidSplitScreenSlot() = 0;				// -1 == invalid
	virtual int		NextValidSplitScreenSlot( int nPreviousSlot ) = 0;	// -1 == invalid
	// Whether "the local player" can be resolved on this thread (inside an active-slot scope).
	virtual bool	SetLocalPlayerIsResolvable( char const *pchContext, int nLine, bool bResolvable ) = 0;
	virtual bool	IsLocalPlayerResolvable() = 0;
};

#define CLIENT_SPLITSCREEN_INTERFACE_VERSION "IClientSplitScreen001"

// Optionally exported by the client DLL.
abstract_class IClientSplitScreen
{
public:
	virtual void	OnActiveSplitscreenPlayerChanged( int nNewSlot ) = 0;
	// Entering or leaving split-screen, or the number of local players changed.
	virtual void	OnSplitScreenStateChanged() = 0;
};

#define ENGINE_SERVER_SPLITSCREEN_INTERFACE_VERSION "VEngineServerSplitScreen001"

struct edict_t;

// Exposed by the engine to the server DLL: which clients are local split-screen players.
// (IVEngineServer keeps its vtable; entity indices are 1-based client entity indices.)
abstract_class IEngineServerSplitScreen
{
public:
	// True if the client is a non-primary local player riding on another client's connection.
	virtual bool		IsSplitScreenPlayer( int ent_num ) = 0;
	// For a split-screen player: its owner's edict, else NULL.
	virtual edict_t		*GetSplitScreenPlayerAttachToEdict( int ent_num ) = 0;
	// For an owner: how many split-screen players ride on it.
	virtual int			GetNumSplitScreenUsersAttachedToEdict( int ent_num ) = 0;
	// For an owner: the edict of the split-screen player in nSlot (1..), else NULL.
	virtual edict_t		*GetSplitScreenPlayerForEdict( int ent_num, int nSlot ) = 0;
};

#endif // ISPLITSCREEN_H
