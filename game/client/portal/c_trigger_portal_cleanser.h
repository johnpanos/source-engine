//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side fizzler trigger; predicts portal removal for the local gun
//			and drives the field's look (the FizzlerVortex material proxy and the
//			cleanser_scanline particle)
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled) and the retail
// Linux client.so. Not original Valve source; the repository's provenance and
// distribution warning applies.
//
//=============================================================================//
#ifndef C_TRIGGER_PORTAL_CLEANSER_H
#define C_TRIGGER_PORTAL_CLEANSER_H
#ifdef _WIN32
#pragma once
#endif

#include "c_triggers.h"
#include "particles_new.h"

// Reconstruction note: the 2010 builds declare this class inside
// c_trigger_portal_cleanser.cpp. The retail-era shared placement and portalgun code
// includes this header and needs the class, so the declaration lives here.
class C_TriggerPortalCleanser : public C_BaseTrigger
{
public:
	DECLARE_CLASS( C_TriggerPortalCleanser, C_BaseTrigger );
	DECLARE_CLIENTCLASS();

	C_TriggerPortalCleanser();
	virtual ~C_TriggerPortalCleanser();

	bool IsEnabled( void ) const { return !m_bDisabled; }

	virtual void Touch( C_BaseEntity *pOther );
	virtual void UpdatePartitionListEntry( void );
	virtual void OnDataChanged( DataUpdateType_t updateType );

	// The field's state for this frame (FizzlerVortex): $flow_color_intensity,
	// raised briefly when a portal shot hits the field, and $powerup, which ramps
	// over cl_portal_cleanser_powerup_time as the field turns on and back down as
	// it turns off. Also keeps the scanline particle in step.
	void GetCurrentState( float &flIntensity, float &flPowerUp );

	// The vortex object whose position bends the field, or NULL (iObject 0 or 1).
	C_BaseEntity *GetVortexObject( int iObject );

private:
	void UpdateScanline( void );
	void StopScanline( void );

	bool m_bDisabled;
	bool m_bVisible;
	bool m_bUseScanline;
	bool m_bPlayersPassTriggerFilters;
	bool m_bObject1InRange;
	bool m_bObject2InRange;
	CHandle<C_BaseEntity> m_hObject1;
	CHandle<C_BaseEntity> m_hObject2;
	float m_flPortalShotTime;

	float m_flPowerUpTimer;   // seconds into the power-up ramp
	float m_flLastShotTime;   // the last m_flPortalShotTime the pulse started from
	float m_flShotPulseTimer; // seconds into the shot pulse
	float m_flLastUpdateTime; // curtime of the last GetCurrentState

	HPARTICLEFFECT m_hScanlineEffect;
};

typedef C_TriggerPortalCleanser CTriggerPortalCleanser;

#endif // C_TRIGGER_PORTAL_CLEANSER_H
