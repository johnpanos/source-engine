//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Version 2 sound entries (Portal 2 soundscripts): script handles and
//          operator stacks. A separate, optional interface: ISoundEmitterSystemBase
//          (VSoundEmitter002) keeps its vtable. The sound emitter answers
//          IAppSystem::QueryInterface( SOUNDEMITTERSYSTEM_ENTRIES_INTERFACE_VERSION );
//          callers that get NULL treat every entry as version 1 (no operators).
//
//=============================================================================//

#ifndef ISOUNDEMITTERSYSTEMENTRIES_H
#define ISOUNDEMITTERSYSTEMENTRIES_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class KeyValues;

// Stable identity of a sound entry (a script name), as the Portal 2-era engine
// sends it with networked sounds so the client can run the entry's operator
// stacks. Hash of the lower-cased entry name; not an index, so it survives
// differing load orders.
typedef unsigned int HSOUNDSCRIPTHASH;
#define SOUNDEMITTER_INVALID_HASH ( (HSOUNDSCRIPTHASH)-1 )

#define SOUNDEMITTERSYSTEM_ENTRIES_INTERFACE_VERSION "VSoundEmitterEntries001"

abstract_class ISoundEmitterSystemEntries
{
public:
	// The entry's "operator_stacks" block, or NULL when the entry has none
	// (every version 1 entry). The sound emitter owns the returned keys; they
	// stay valid until the entry is removed or the scripts are reloaded.
	virtual KeyValues *GetOperatorKVByHandle( HSOUNDSCRIPTHASH &handle ) = 0;
	// NULL when no loaded entry has this hash.
	virtual char const *GetSoundNameForHash( HSOUNDSCRIPTHASH hash ) const = 0;
	virtual int GetSoundIndexForHash( HSOUNDSCRIPTHASH hash ) const = 0;
	virtual HSOUNDSCRIPTHASH HashSoundName( char const *pchSndName ) const = 0;
	virtual bool IsValidHash( HSOUNDSCRIPTHASH hash ) const = 0;
	// "soundentry_version" of the entry (1 when absent); 0 for an invalid index.
	virtual int GetSoundEntryVersion( int index ) const = 0;
};

#endif // ISOUNDEMITTERSYSTEMENTRIES_H
