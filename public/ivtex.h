//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef IVTEX_H
#define IVTEX_H

#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "appframework/IAppSystem.h"


class IVTex : public IAppSystem
{
public:
	// For use by command-line tools
	virtual int VTex( int argc, char **argv ) = 0;

	// For use by engine
	virtual int VTex( CreateInterfaceFn filesystemFactory, const char *pGameDir, int argc, char **argv ) = 0;
};

// Normal command-line entry point for tool products that link vtex_dll as a
// build dependency. This is not an extension-discovery ABI: the executable
// imports the symbol directly and never resolves a module name or interface
// string at runtime.
#if defined( VTEX_TOOL_ENTRY_EXPORTS )
#define VTEX_TOOL_INTERFACE DLL_EXPORT
#else
#define VTEX_TOOL_INTERFACE DLL_IMPORT
#endif

VTEX_TOOL_INTERFACE int VTex_RunCommandLine( int argc, char **argv );

#undef VTEX_TOOL_INTERFACE

#define IVTEX_VERSION_STRING "VTEX_003"


#endif // IVTEX_H
