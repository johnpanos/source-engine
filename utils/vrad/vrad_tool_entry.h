//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Direct link-time command-line entry for the VRAD tool product.
//
//=============================================================================//

#ifndef VRAD_TOOL_ENTRY_H
#define VRAD_TOOL_ENTRY_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

#if defined( VRAD_TOOL_ENTRY_EXPORTS )
#define VRAD_TOOL_INTERFACE DLL_EXPORT
#else
#define VRAD_TOOL_INTERFACE DLL_IMPORT
#endif

// This is a private link-time boundary, not an extension-discovery ABI.
VRAD_TOOL_INTERFACE int VRad_RunCommandLine( int argc, char **argv );

#undef VRAD_TOOL_INTERFACE

#endif // VRAD_TOOL_ENTRY_H
