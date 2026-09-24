//========= Portal 2 reconstruction ============================================//
//
// Purpose: Excursion funnel emitter (prop_tractor_beam).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PROP_TRACTORBEAM_H
#define PROP_TRACTORBEAM_H

#ifdef _WIN32
#pragma once
#endif

#include "baseprojector.h"

class CPropTractorBeamProjector : public CBaseProjector
{
	DECLARE_CLASS( CPropTractorBeamProjector, CBaseProjector );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	virtual void	Spawn( void );
	virtual void	Precache( void );
	virtual void	Activate( void );

	virtual void	Project( void );

	float			GetLinearForce( void ) const { return m_flLinearForce; }

protected:
	virtual CBaseProjectedEntity *CreateNewProjectedEntity( void );
	void			InputSetLinearForce( inputdata_t &indata );

	CNetworkVar( float, m_flLinearForce );		// Negative forces pull toward the emitter
	CNetworkVar( Vector, m_vEndPos );			// End of the first beam segment, for the emitter effects
};

#endif // PROP_TRACTORBEAM_H
