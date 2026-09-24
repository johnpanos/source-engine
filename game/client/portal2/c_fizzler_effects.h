//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the effect entity attached to fizzling objects
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_FIZZLER_EFFECTS_H
#define C_FIZZLER_EFFECTS_H
#ifdef _WIN32
#pragma once
#endif

class C_FizzlerEffects : public C_BaseEntity
{
	DECLARE_CLASS( C_FizzlerEffects, C_BaseEntity );
	DECLARE_CLIENTCLASS();
};

#endif // C_FIZZLER_EFFECTS_H
