//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the co-op lobby day-select display
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_mp_lobby_screen.h"
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT( C_MP_LobbyScreen, DT_MP_LobbyScreen, CMP_LobbyScreen )
	RecvPropBool( RECVINFO( m_bEnabled ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Constructor 
//-----------------------------------------------------------------------------
C_MP_LobbyScreen::C_MP_LobbyScreen()
{
}

C_MP_LobbyScreen::~C_MP_LobbyScreen()
{
}
