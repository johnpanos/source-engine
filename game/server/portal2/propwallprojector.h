//========= Portal 2 reconstruction ============================================//
//
// Purpose: Hard light bridge emitter (prop_wall_projector).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PROPWALLPROJECTOR_H
#define PROPWALLPROJECTOR_H

#ifdef _WIN32
#pragma once
#endif

#include "baseprojector.h"

class CPropWallProjector : public CBaseProjector
{
	DECLARE_CLASS( CPropWallProjector, CBaseProjector );

public:
	virtual void	Spawn( void );
	virtual void	Precache( void );

protected:
	virtual CBaseProjectedEntity *CreateNewProjectedEntity( void );
};

#endif // PROPWALLPROJECTOR_H
