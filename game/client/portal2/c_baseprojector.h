//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side name for the base of projectors (light bridge and tractor
//			beam emitters) that own a chain of projected entities.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_BASEPROJECTOR_H
#define C_BASEPROJECTOR_H

#ifdef _WIN32
#pragma once
#endif

#include "c_baseanimating.h"

// The 2010 client builds have no DWARF for this class. The retained retail
// client and server binaries both name DT_BaseProjector, m_hFirstChild and
// m_bEnabled, and the server table setup places the two fields after baseclass.
class C_BaseProjectedEntity;

class C_BaseProjector : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_BaseProjector, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

protected:
	CHandle<C_BaseProjectedEntity> m_hFirstChild;
	bool m_bEnabled;
};

typedef C_BaseProjector CBaseProjector;

#endif // C_BASEPROJECTOR_H
