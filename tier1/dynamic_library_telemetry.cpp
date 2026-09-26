//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy bridge from a platform loader provider's load-site events to
//          the Tier 0 module-load telemetry stream (RFC 0001 rank 4, R07-SYS).
//
//=============================================================================//

#include "tier1/dynamic_library_telemetry.h"
#include "tier0/module_load_telemetry_internal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{

// The provider's structured status as the stream's text; the stream has no
// status field of its own.
const char *StatusText( platform::DynamicLibraryStatus status )
{
	switch ( status )
	{
	case platform::DynamicLibraryStatus::kOk:
		return "";
	case platform::DynamicLibraryStatus::kInvalidArgument:
		return "invalid argument";
	case platform::DynamicLibraryStatus::kNotFound:
		return "not found";
	case platform::DynamicLibraryStatus::kSymbolNotFound:
		return "symbol not found";
	case platform::DynamicLibraryStatus::kUnsupportedNoLoad:
		return "no-load unsupported";
	case platform::DynamicLibraryStatus::kProviderError:
		return "provider error";
	}
	return "unknown status";
}

// The stream keys a record by module pointer; one IDynamicLibrary is one load.
CSysModule *RecordKey( const platform::IDynamicLibrary *library )
{
	return reinterpret_cast<CSysModule *>( const_cast<platform::IDynamicLibrary *>( library ) );
}

int ProviderResult( const platform::DynamicLibraryError &result )
{
	if ( result.IsOk() )
		return 0;
	return result.providerCode != 0 ? result.providerCode : -1;
}

} // namespace

void CModuleLoadTelemetryObserver::OnLoad( const char *path, const char *resolvedPath,
    const platform::IDynamicLibrary *library, const platform::DynamicLibraryError &result )
{
	Sys_RecordModuleLoad( library != nullptr ? RecordKey( library ) : nullptr, path,
	    resolvedPath != nullptr ? resolvedPath : path, ProviderResult( result ),
	    StatusText( result.status ) );
}

void CModuleLoadTelemetryObserver::OnFindSymbol( const platform::IDynamicLibrary *library,
    const char *name, const void *, const platform::DynamicLibraryError &result )
{
	Sys_RecordModuleEntryPoint( RecordKey( library ), nullptr, name, result.IsOk(),
	    ProviderResult( result ), StatusText( result.status ) );
}

void CModuleLoadTelemetryObserver::OnUnload( const platform::IDynamicLibrary *library )
{
	Sys_RecordModuleUnload( RecordKey( library ), true, 0, "" );
}
