//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy bridge from a platform loader provider's load-site events to
//          the Tier 0 module-load telemetry stream (RFC 0001 rank 4, R07-SYS).
//
//=============================================================================//

#ifndef TIER1_DYNAMIC_LIBRARY_TELEMETRY_H
#define TIER1_DYNAMIC_LIBRARY_TELEMETRY_H

#ifdef _WIN32
#pragma once
#endif

#include "platform/contracts/dynamic_library.h"

// Records every load, symbol lookup and unload a loader provider reports as the
// same LOAD / ENTRY_POINT / UNLOAD events Sys_LoadModule emits, under the
// caller's CScopedModuleLoadRequest context. Each IDynamicLibrary is its own
// load record, so duplicate loads keep distinct load IDs. Stateless; one
// instance may serve any number of loaders.
class CModuleLoadTelemetryObserver final : public platform::IDynamicLibraryObserver
{
public:
	void OnLoad( const char *path, const char *resolvedPath,
	    const platform::IDynamicLibrary *library,
	    const platform::DynamicLibraryError &result ) override;
	void OnFindSymbol( const platform::IDynamicLibrary *library, const char *name,
	    const void *symbol, const platform::DynamicLibraryError &result ) override;
	void OnUnload( const platform::IDynamicLibrary *library ) override;
};

#endif // TIER1_DYNAMIC_LIBRARY_TELEMETRY_H
