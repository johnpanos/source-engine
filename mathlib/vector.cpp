//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "mathlib/vector.h"

// vec3_origin is defined once, const, in mathlib_base.cpp (mathlib.h declares
// it extern const); a second definition here broke linking mathlib's objects
// directly.
