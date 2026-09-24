//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the co-op lobby day-select display
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_MP_LOBBY_SCREEN_H
#define C_MP_LOBBY_SCREEN_H
#ifdef _WIN32
#pragma once
#endif
class C_MP_LobbyScreen : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_MP_LobbyScreen, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_MP_LobbyScreen();
	virtual ~C_MP_LobbyScreen();

	bool IsEnabled( void ) { return m_bEnabled; }

private:
	bool	m_bEnabled;
};

#endif // C_MP_LOBBY_SCREEN_H
