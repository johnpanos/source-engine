//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#if !defined( CVAR_H )
#define CVAR_H
#ifdef _WIN32
#pragma once
#endif

#include "vstdlib/vstdlib.h"
#include "icvar.h"


//-----------------------------------------------------------------------------
// Returns the linked CVar app-system instance.
//-----------------------------------------------------------------------------
VSTDLIB_INTERFACE ICvar *VStdLib_GetICVar();

// Legacy extension-ABI factory. Linked application composition must use the
// typed instance accessor above.
VSTDLIB_INTERFACE CreateInterfaceFn VStdLib_GetICVarFactory();


#endif // CVAR_H
