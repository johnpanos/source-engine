//========= Portal 2 reconstruction ============================================//
//
// Purpose: Point push entity that also pushes through portals and moves paint
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PORTAL_POINTPUSH_H
#define PORTAL_POINTPUSH_H
#ifdef _WIN32
#pragma once
#endif

#include "portal_shareddefs.h"

class CPortal_Base2D;

// Spawnflags
#define	SF_PUSH_TEST_LOS			0x0001
#define SF_PUSH_DIRECTIONAL			0x0002
#define SF_PUSH_NO_FALLOFF			0x0004
#define	SF_PUSH_PLAYER				0x0008
#define SF_PUSH_PHYSICS				0x0010

class CPortal_PointPush : public CBaseEntity
{
public:
	DECLARE_CLASS( CPortal_PointPush, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual void Activate( void );

	void PushThink( void );

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

	virtual int UpdateTransmitState( void );

	virtual void DrawDebugGeometryOverlays( void );

private:
	virtual void NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier );

	void PushAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortalFrom, CPortal_Base2D *pPortalTo );
	void PushEntitiesAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortal );
	void PushPaintBlobsAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortal );
	void PushEntity( CBaseEntity *pTarget, const Vector &vecPushPoint );

	void CreatePortalVortexEffect( const Vector &vecOrigin, CPortal_Base2D *pPortal );

	bool TestWithinCone( const Vector &vecOrigin, const Vector &vecEndPos, const QAngle &vecAngles, float flMinAngle );

	CNetworkVar( bool, m_bEnabled );
	CNetworkVar( float, m_flMagnitude );
	CNetworkVar( float, m_flRadius );
	CNetworkVar( float, m_flInnerRadius );		// Inner radius to perform LOS checks from
	CNetworkVar( float, m_flConeOfInfluence );
};

#endif // PORTAL_POINTPUSH_H
