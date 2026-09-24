//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the effect entity attached to fizzling objects
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_fizzler_effects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( env_fizzler_effects, C_FizzlerEffects );

IMPLEMENT_CLIENTCLASS_DT( C_FizzlerEffects, DT_FizzlerEffects, FizzlerEffects )
END_RECV_TABLE()
