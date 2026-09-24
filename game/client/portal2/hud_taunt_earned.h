//========= Portal 2 reconstruction ============================================//
//
// Purpose: HUD notice for gestures (taunts) that were earned but not yet used
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef HUD_TAUNT_EARNED_H
#define HUD_TAUNT_EARNED_H
#ifdef _WIN32
#pragma once
#endif

// Reconstruction note: the 841 DWARF declares CHUDTauntEarned inside
// hud_taunt_earned.cpp (the class starts at line 26 of the .cpp), and no Portal 2
// source includes this header. The element is created by name through
// DECLARE_HUDELEMENT, so the header exports no declarations.

#endif // HUD_TAUNT_EARNED_H
