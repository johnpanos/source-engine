//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side copy of func_portal_bumper for predicted portal placement
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_func_portal_bumper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_FuncPortalBumper, DT_FuncPortalBumper, CFuncPortalBumper )
	RecvPropBool( RECVINFO( m_bActive ) ),
END_RECV_TABLE()
