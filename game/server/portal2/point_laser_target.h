//========= Portal 2 reconstruction ============================================//
//
// Purpose: Laser targets, catchers and relays struck by env_portal_laser.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef POINT_LASER_TARGET_H
#define POINT_LASER_TARGET_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "baseanimating.h"

//-----------------------------------------------------------------------------
// Interface for things that want to know when a laser target is powered
//-----------------------------------------------------------------------------
abstract_class ICatcher
{
public:
	virtual void OnPowered() = 0;
	virtual void OnUnPowered() = 0;
};

//-----------------------------------------------------------------------------
// Default (do nothing) catcher
//-----------------------------------------------------------------------------
class CCatcher : public ICatcher
{
public:
	virtual void OnPowered() {}
	virtual void OnUnPowered() {}
};

//-----------------------------------------------------------------------------
// point_laser_target
//-----------------------------------------------------------------------------
class CPortalLaserTarget : public CBaseEntity
{
public:
	DECLARE_CLASS( CPortalLaserTarget, CBaseEntity );
	DECLARE_DATADESC();

	CPortalLaserTarget();

	virtual void Spawn();

	virtual int OnTakeDamage( const CTakeDamageInfo &info );

	bool IsTerminalPoint() const { return m_bTerminalPoint; }

	void SetCatcher( CCatcher *pCatcher ) { m_pCatcher = pCatcher; }

	bool IsPowered();

protected:

	void DisableThink();

	COutputEvent m_OnPowered;
	COutputEvent m_OnUnpowered;

	bool m_bPowered;
	bool m_bTerminalPoint;
	CCatcher *m_pCatcher;
	string_t m_ModelName;
};

//-----------------------------------------------------------------------------
// prop_laser_catcher
//-----------------------------------------------------------------------------
class CLaserCatcher : public CBaseAnimating, public CCatcher
{
public:
	DECLARE_CLASS( CLaserCatcher, CBaseAnimating );
	DECLARE_DATADESC();

	CLaserCatcher();

	virtual void Precache();
	virtual void Spawn();
	virtual bool CreateVPhysics();
	virtual void Activate();
	virtual void AnimateThink();
	virtual void UpdateOnRemove();

	virtual void OnPowered();
	virtual void OnUnPowered();

	virtual bool PoweringDown();
	virtual bool PoweringUp();

	virtual void CreateHelperEntities();
	virtual const char *GetCatcherModelName();
	virtual bool IsTerminalPoint();

	virtual float SpinUp( float flSpinRate );
	virtual float SpinDown( float flSpinRate );
	virtual void OnRestore();

protected:
	enum
	{
		POWER_STATE_IDLE = 0,
		POWER_STATE_POWERING_DOWN,
		POWER_STATE_POWERING_UP,
	};

	CPortalLaserTarget *m_pCatcherLaserTarget;
	COutputEvent m_OnPowered;
	COutputEvent m_OnUnpowered;

	int m_iPowerState;

private:

	int m_IdleSequence;
	int m_PowerOnSequence;

	int m_iTargetAttachment;
};

//-----------------------------------------------------------------------------
// prop_laser_relay
//-----------------------------------------------------------------------------
class CLaserRelay : public CLaserCatcher
{
	DECLARE_CLASS( CLaserRelay, CLaserCatcher );
	DECLARE_DATADESC();

private:
	virtual const char *GetCatcherModelName();
	virtual bool IsTerminalPoint();
};

#endif // POINT_LASER_TARGET_H
