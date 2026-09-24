//========= Portal 2 reconstruction ============================================//
//
// Purpose: Basic test bots for the paint build.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PAINT_BOT_TEMP_H
#define PAINT_BOT_TEMP_H
#ifdef _WIN32
#pragma once
#endif

class CBasePlayer;

// Creates a fake client and puts it in the game. pszCustomName may be NULL or empty.
CBasePlayer *BotPutInServer( bool bFrozen, const char *pszCustomName );

// Runs the think for every bot in the game
void Bot_RunAll();

#endif // PAINT_BOT_TEMP_H
