//========= Portal 2 reconstruction ============================================//
//
// Purpose: Stationary rocket turret that fires when its tripwire is crossed.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PROP_ROCKETTRIPWIRE_H
#define PROP_ROCKETTRIPWIRE_H
#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

class CInfoPlacementHelper;
class CRocket_Turret_Projectile;

class CPropRocketTripwire : public CBaseAnimating
{
public:
	enum TripwireState_t
	{
		DISABLED = 0,
		ACTIVE,
		FIRING,
	};

	DECLARE_CLASS( CPropRocketTripwire, CBaseAnimating );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CPropRocketTripwire();
	virtual ~CPropRocketTripwire();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void UpdateOnRemove( void );
	virtual Vector EyePosition( void );
	virtual Vector EyeDirection( void );

	void WaitingThink( void );
	void ActiveThink( void );
	void PrepareToFireRocket( void );
	void PreFiringThink( void );
	void FireRocketThink( void );
	void EnableTripwire( void );
	void DisableTripwire( void );

	void InputForceFire( inputdata_t &inputdata );
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

protected:
	void FireRocket( void );
	bool IsAlternativeTripwireTarget( CBaseEntity *pEnt );

	float m_flRocketSpeed;
	float m_flRocketLifetime;
	float m_flTimeLastFired;
	COutputEvent m_OnTripped;
	COutputEvent m_OnRocketExplode;
	CHandle< CInfoPlacementHelper > m_hPlacementHelper;
	CHandle< CRocket_Turret_Projectile > m_hCurRocket;
	bool m_bHasRocketOut;
	TripwireState_t m_State;
	float m_flRocketTimeOfDeath;

	bool m_bDisabled;

	CNetworkVar( int, m_iMuzzleAttachment );
	CNetworkVar( int, m_nBeamEndpointTexture );
	CNetworkVar( bool, m_bDrawBeams );
	CNetworkHandle( CBaseEntity, m_hRocket );
};

#endif // PROP_ROCKETTRIPWIRE_H
