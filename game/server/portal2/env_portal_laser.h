//========= Portal 2 reconstruction ============================================//
//
// Purpose: Thermal discouragement beam (env_portal_laser).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef ENV_PORTAL_LASER_H
#define ENV_PORTAL_LASER_H
#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"
#include "utlvector.h"

class CSprite;
class CSoundPatch;
class CInfoPlacementHelper;

DECLARE_AUTO_LIST( IPortalLaserAutoList );

class CPortalLaser : public CBaseAnimating, public IPortalLaserAutoList
{
public:
	DECLARE_CLASS( CPortalLaser, CBaseAnimating );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CPortalLaser();
	virtual ~CPortalLaser();

	virtual void Spawn();
	virtual void Precache();
	virtual void Activate();
	virtual void UpdateOnRemove();
	virtual bool CreateVPhysics();
	virtual int UpdateTransmitState();

	void InputTurnOn( inputdata_t &inputdata );
	void InputTurnOff( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );

	// Reconstruction note: the retail consumer (portal_player.cpp) queries the
	// closest point on the beam; the 2010 binaries predate this helper.
	Vector ClosestPointOnLineSegment( const Vector &vPos );

	CPortalLaser *m_pChildLaser;

private:
	void RemoveChildLaser();
	void UpdateNextLaser( const Vector &vecStart, const Vector &vecDirection, CBaseEntity *pParent );
	void FireLaser( const Vector &vecStart, const Vector &vecDirection, CBaseEntity *pParent );
	bool ReflectLaserFromEntity( CBaseEntity *pEntity );

	bool InPVS();
	void CreateHelperEntities();
	void CreateSoundProxies();
	void DamageEntity( CBaseEntity *pVictim, float flAmount );
	void TurnOn();
	void TurnOff();
	bool IsOn();

	void TurnOnGlow();
	void TurnOffGlow();

	void TurnOffLaserSound();

	void FireAtPoint( trace_t &tr, bool bImpact );
	void StrikeThink();
	void BeamDamage( trace_t *ptr );

	void UpdateSoundPosition( const Vector &vecStart, const Vector &vecEnd );
	bool StrikeEntitiesAlongLaser( const Vector &vecStart, const Vector &vecEnd, Vector *pVecOut );

	bool ShouldAutoAim( CBaseEntity *pEntity );

	void SetFromReflectedCube( bool bFromReflectedCube ) { m_bFromReflectedCube = bFromReflectedCube; }

public:
	struct PortalLaserInfo_t
	{
		Vector vecStart;
		Vector vecEnd;
		CUtlVector< CBaseEntity* > sortedEntList;
	};

private:
	CBaseEntity *TraceLaser( bool bIsFirstTrace, const Vector &vecStart, const Vector &vecDirection, float &flTotalBeamLength, trace_t &tr, CUtlVector< PortalLaserInfo_t > &infoList, Vector *pVecAutoAimOffset );
	CBaseEntity *GetEntitiesAlongLaser( const Vector &vecStart, const Vector &vecEnd, Vector *pVecOut, CUtlVector< PortalLaserInfo_t > &infoList, bool bIsFirstTrace );
	void DamageEntitiesAlongLaser( const CUtlVector< PortalLaserInfo_t > &infoList, bool bAutoAim );

	Vector m_vecNearestSoundSource[ MAX_PLAYERS ];
	CBaseEntity *m_pSoundProxy[ MAX_PLAYERS ];
	CSoundPatch *m_pAmbientSound[ MAX_PLAYERS ];
	CInfoPlacementHelper *m_pPlacementHelper;
	CSprite *m_pLaserGlow;
	int m_iLaserAttachment;
	string_t m_ModelName;
	bool m_bStartOff;
	bool m_bFromReflectedCube;

	// The client draws the beam: it traces from m_vStartPoint towards
	// m_vEndPoint through portals, or from the reflector when there is one.
	CNetworkHandle( CBaseEntity, m_hReflector );
	CNetworkVector( m_vStartPoint );
	CNetworkVector( m_vEndPoint );
	CNetworkVar( bool, m_bLaserOn );
	CNetworkVar( bool, m_bIsLethal );
	CNetworkVar( bool, m_bIsAutoAiming );
	CNetworkVar( bool, m_bShouldSpark );
	CNetworkVar( bool, m_bUseParentDir );
	CNetworkQAngle( m_angParentAngles );
};

#endif // ENV_PORTAL_LASER_H
