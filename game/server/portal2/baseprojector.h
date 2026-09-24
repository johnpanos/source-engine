//========= Portal 2 reconstruction ============================================//
//
// Purpose: Base class for projectors that emit a chain of projected entities
//			(light bridge and tractor beam emitters).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef BASEPROJECTOR_H
#define BASEPROJECTOR_H

#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

class CBaseProjectedEntity;

class CBaseProjector : public CBaseAnimating
{
	DECLARE_CLASS( CBaseProjector, CBaseAnimating );
	DECLARE_DATADESC();

public:
	CBaseProjector();
	virtual ~CBaseProjector();

	virtual void	Spawn( void );
	virtual void	Activate( void );
	virtual void	UpdateOnRemove( void );
	virtual void	OnRestore( void );

	virtual void	Project( void );

	// Each projector type creates its own kind of projected entity
	virtual CBaseProjectedEntity *CreateNewProjectedEntity( void ) = 0;

	void			EnableProjection( bool bEnable );

	// Watch for portals moving into or out of the projection
	void			TestProjectionChangesThink( void );
	static void		TestAllForProjectionChanges( void );

	CNetworkVar( bool, m_bEnabled );

protected:
	void			Cleanup( void );
	virtual int		DrawDebugTextOverlays( void );
	void			DrawBoundsHighlight( void );
	void			TestForPortalChanges( void );

	void			InputEnable( inputdata_t &data );
	void			InputDisable( inputdata_t &data );

	CHandle<CBaseProjectedEntity> m_hFirstChild;
	bool			m_bDisablePlacementHelper;
};

#endif // BASEPROJECTOR_H
