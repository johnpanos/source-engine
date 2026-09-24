//========= Portal 2 reconstruction ============================================//
//
// Purpose: Rocket fired by the rocket tripwire turret.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef ROCKETPROJECTILE_H
#define ROCKETPROJECTILE_H
#ifdef _WIN32
#pragma once
#endif

#include "basecombatcharacter.h"
#include "smoke_trail.h"

class CSoundPatch;

class CRocket_Turret_Projectile : public CBaseCombatCharacter
{
	DECLARE_CLASS( CRocket_Turret_Projectile, CBaseCombatCharacter );
public:
	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void UpdateOnRemove( void );

	virtual void NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params );
	virtual void SetLauncher( EHANDLE hLauncher );
	virtual void CreateSmokeTrail( void );

	virtual void MissileTouch( CBaseEntity *pOther );
	void Explode( void );

protected:
	virtual void DoExplosion( void );
	virtual void CreateSounds( void );
	virtual void StopLoopingSounds( void );

	EHANDLE m_hLauncher;
	CSoundPatch *m_pAmbientSound;
	CHandle< RocketTrail > m_hRocketTrail;

	DECLARE_DATADESC();
};

#endif // ROCKETPROJECTILE_H
