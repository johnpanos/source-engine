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

// Reconstruction note: neither 2010 client build has DWARF for this header; the only
// client projector recovered (C_PropTractorBeamProjector) derives from C_BaseAnimating
// and the server's CBaseProjector declares no server class, so the client base carries
// no networked state. The shared projection code only needs the type to name the owning
// projector (CBaseProjector *) passed through RecursiveProjection().
class C_BaseProjector : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_BaseProjector, C_BaseAnimating );
};

typedef C_BaseProjector CBaseProjector;

#endif // C_BASEPROJECTOR_H
