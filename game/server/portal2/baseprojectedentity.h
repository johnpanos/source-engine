//========= Portal 2 reconstruction ============================================//
//
// Purpose: Base class for entities projected by a projector (light bridges and
//			tractor beams) that can redirect through linked portals.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef BASEPROJECTEDENTITY_H
#define BASEPROJECTEDENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "portal_base2d.h"

class CBaseProjector;
class CInfoPlacementHelper;

class CBaseProjectedEntity : public CBaseEntity
{
	DECLARE_CLASS( CBaseProjectedEntity, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CBaseProjectedEntity();

	virtual void	Spawn( void );

	virtual void	FindProjectedEndpoints( void );

	virtual void	SetHitPortal( CPortal_Base2D *pPortal );
	virtual CPortal_Base2D *GetHitPortal( void );

	virtual void	SetSourcePortal( CPortal_Base2D *pPortal );
	virtual CPortal_Base2D *GetSourcePortal( void );

	virtual bool	DidRedirectionPortalMove( CPortal_Base2D *pPortal );

	virtual const Vector &GetStartPoint( void ) const { return m_vecStartPoint.Get(); }
	virtual const Vector &GetEndPoint( void ) const { return m_vecEndPoint.Get(); }
	virtual Vector	GetLengthVector( void ) { return m_vecEndPoint.Get() - m_vecStartPoint.Get(); }
	virtual void	GetProjectionExtents( Vector &outMins, Vector &outMaxs );

	// Project from the given origin, spawning or re-projecting child segments through
	// every linked portal the projection hits.
	void			RecursiveProjection( bool bShouldSpawn, CBaseProjector *pParentProjector, CPortal_Base2D *pExitPortal, const Vector &vProjectOrigin, const QAngle &qProjectAngles, int iRemainingProjections, bool bDisablePlacementHelper );
	virtual bool	IsHittingPortal( Vector *pOutOrigin, QAngle *pOutAngles, CPortal_Base2D **pOutPortal );
	virtual void	TestForProjectionChanges( void );

	virtual void	UpdateOnRemove( void );
	virtual void	OnRestore( void );

	// Every projected entity type spawns more of its own kind through portals
	virtual CBaseProjectedEntity *CreateNewProjectedEntity( void ) = 0;
	virtual void	PlacePlacementHelper( CInfoPlacementHelper *pHelper );

	// Reconstruction note: the retail shared projection code calls OnPreProjected() before a
	// segment is (re)positioned; the 2010 builds have no such hook, so the base does nothing.
	virtual void	OnPreProjected( void ) {}
	virtual void	OnProjected( void );

	void			TestForReflectPaint( void );

protected:
	CNetworkHandle( CPortal_Base2D, m_hHitPortal );				// Portal the end of this segment enters
	CNetworkHandle( CPortal_Base2D, m_hSourcePortal );			// Portal this segment exits from
	CNetworkVector( m_vecSourcePortalCenter );
	CNetworkVector( m_vecSourcePortalRemoteCenter );
	CNetworkQAngle( m_vecSourcePortalAngle );
	CNetworkQAngle( m_vecSourcePortalRemoteAngle );
	CNetworkVector( m_vecStartPoint );
	CNetworkVector( m_vecEndPoint );

	CNetworkHandle( CBaseProjectedEntity, m_hChildSegment );	// Next segment on the far side of m_hHitPortal
	CNetworkHandle( CInfoPlacementHelper, m_hPlacementHelper );

	bool			m_bCreatePlacementHelper;

private:
	CNetworkVar( int, m_iMaxRemainingRecursions );
};

#endif // BASEPROJECTEDENTITY_H
