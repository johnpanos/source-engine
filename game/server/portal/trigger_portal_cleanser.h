//========= Portal 2 reconstruction ============================================//
//
// Purpose: Fizzler trigger that removes portals and dissolves objects that touch it
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef TRIGGER_PORTAL_CLEANSER_H
#define TRIGGER_PORTAL_CLEANSER_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"

class CTriggerPortalCleanser : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerPortalCleanser, CBaseTrigger );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void	Spawn( void );
	virtual void Precache( void );
	virtual void Activate( void );
	virtual void	Touch( CBaseEntity *pOther );

	virtual void	Enable( void );
	virtual void Disable( void );

	// CBaseTrigger's Enable and Disable are not virtual here; the inputs route
	// through these so the cleanser's own state changes run.
	virtual void InputEnable( inputdata_t &inputdata );
	virtual void InputDisable( inputdata_t &inputdata );
	virtual void InputToggle( inputdata_t &inputdata );

	virtual int		UpdateTransmitState( void )
	{
		return SetTransmitState( FL_EDICT_PVSCHECK );
	}

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_bDisabled );

	bool			IsEnabled( void ) const { return !m_bDisabled; }

	// Reconstruction note: the retail-era consumers (weapon_portalgun_shared.cpp and
	// prop_weightedcube.cpp) call these two, which the 2010 builds do not have yet. The
	// 2010 Touch dissolved objects inline; the retail code factors that out into the static
	// helper so other entities can fizzle an object with or without a cleanser (pFizzler may
	// be NULL). Their definitions belong to the Portal 2 trigger_portal_cleanser.cpp.
	void			SetPortalShot( void );
	static void		FizzleBaseAnimating( CTriggerPortalCleanser *pFizzler, CBaseAnimating *pBaseAnimating );

private:

	void			FizzleTouchingPortals( void );
	void			InputFizzleTouchingPortals( inputdata_t &data ) { FizzleTouchingPortals(); }

	// Retail: finds the two nearest vortex objects (cubes, turrets...) within
	// sv_portal_cleanser_vortex_distance; the client bends the field around them.
	void SearchThink( void );
	void PlayerPassesTriggerFiltersThink( void );
	void PlayActivateSound( void );
	void PlayDeactivateSound( void );

	// Outputs
	COutputEvent	m_OnDissolve;
	COutputEvent	m_OnFizzle;
	COutputEvent	m_OnDissolveBox;

	CNetworkVar( bool, m_bVisible );     // "Visible": draw the field (effects/fizzler)
	CNetworkVar( bool, m_bUseScanline ); // "UseScanline": the client's cleanser_scanline particle
	CNetworkVar(
	    float, m_flPortalShotTime ); // curtime of the last portal shot this cleanser stopped
	CNetworkVar( bool, m_bObject1InRange );
	CNetworkVar( bool, m_bObject2InRange );
	CNetworkHandle( CBaseEntity, m_hObject1 ); // nearest vortex object
	CNetworkHandle( CBaseEntity, m_hObject2 ); // second nearest
	CNetworkVar( bool, m_bPlayersPassTriggerFilters );
};

#endif // TRIGGER_PORTAL_CLEANSER_H
