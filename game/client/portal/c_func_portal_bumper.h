//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side copy of func_portal_bumper for predicted portal placement
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_FUNC_PORTAL_BUMPER_H
#define C_FUNC_PORTAL_BUMPER_H
#ifdef _WIN32
#pragma once
#endif

class C_FuncPortalBumper : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_FuncPortalBumper, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	bool m_bActive;

	bool IsActive( void ) { return m_bActive; }	// is this area currently bumping portals
};

typedef C_FuncPortalBumper CFuncPortalBumper;

#endif // C_FUNC_PORTAL_BUMPER_H
