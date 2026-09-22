//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lightweight precompiled base for the Box3D provider.
//
// This is deliberately NOT vphysics/cbase.h: the shared vcollide text parser
// (vphysics/vcollide_parse.cpp) is reused here for authored keyvalue parsing,
// but the Box3D module must not inherit IVP's headers, allocation macros, or
// C17/FP configuration (RFC 0004). This shim provides only the tier0/mathlib/
// tier1 and public vphysics interface declarations that the parser needs.
//
//=============================================================================//
#ifndef VPHYSICS_BOX3D_CBASE_H
#define VPHYSICS_BOX3D_CBASE_H

#include <stdio.h>

#include "tier0/dbg.h"
#include "mathlib/mathlib.h"
#include "mathlib/vector.h"
#include "utlvector.h"
#include "commonmacros.h"

#include "vphysics_interface.h"

#endif // VPHYSICS_BOX3D_CBASE_H
