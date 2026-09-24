//========= Portal 2 reconstruction ============================================//
//
// Purpose: Swapping the paint gun and the portal gun between two players.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PAINT_SWAP_GUNS_H
#define PAINT_SWAP_GUNS_H
#ifdef _WIN32
#pragma once
#endif

class CBaseCombatWeapon;
class CPortal_Player;

// Grants the owner of a paint gun the power currently selected on it
void CheckSwapPaintGun( CBaseCombatWeapon *pWeapon );

// Exchanges the active weapons of the two players
void SwapPaintAndPortalGuns( CPortal_Player *pPlayer1, CPortal_Player *pPlayer2 );

// Are the players close enough (directly or through a portal) and looking at each other?
bool CheckSwapProximity( CPortal_Player *pPlayer1, CPortal_Player *pPlayer2 );

#endif // PAINT_SWAP_GUNS_H
