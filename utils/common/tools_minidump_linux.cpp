//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Linux host tools use the native core-dump and debugger path.
//
//=============================================================================//

#include "tools_minidump.h"
#include "tier0/dbg.h"

void EnableFullMinidumps( bool bFull )
{
	if ( bFull )
		Warning( "Windows minidump selection is unavailable on Linux; use native core dumps\n" );
}

void SetupDefaultToolsMinidumpHandler()
{
	// Leave the default signal disposition in place so the OS can write a core dump.
}

void SetupToolsMinidumpHandler( ToolsExceptionHandler fn )
{
	if ( fn )
		Error( "Custom Windows minidump handlers are unavailable on Linux\n" );
}
