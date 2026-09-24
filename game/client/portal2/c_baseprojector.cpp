//========= Portal 2 reconstruction ============================================//
//
// Purpose: Networked base for the hard-light projector and its first segment.
//
// The retained retail Portal 2 client and server binaries name
// DT_BaseProjector, m_hFirstChild, and m_bEnabled. The server table setup
// places the child handle before the enabled flag. Not original Valve source;
// the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_baseprojector.h"
#include "c_baseprojectedentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Source receive-table macros require their declaration layout.
// clang-format off
IMPLEMENT_CLIENTCLASS_DT( C_BaseProjector, DT_BaseProjector, CBaseProjector )
	RecvPropEHandle( RECVINFO( m_hFirstChild ) ),
	RecvPropBool( RECVINFO( m_bEnabled ) ),
END_RECV_TABLE()
