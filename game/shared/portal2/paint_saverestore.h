//========= Portal 2 reconstruction ============================================//
//
// Purpose: Save/restore block handler for paintmaps and painted projected walls.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PAINT_SAVERESTORE_H
#define PAINT_SAVERESTORE_H
#ifdef _WIN32
#pragma once
#endif

class ISaveRestoreBlockHandler;

ISaveRestoreBlockHandler *GetPaintSaveRestoreBlockHandler();

#endif // PAINT_SAVERESTORE_H
