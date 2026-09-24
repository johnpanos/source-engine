//========= Portal 2 reconstruction ============================================//
//
// Purpose: Hard light bridge segment projected by prop_wall_projector.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PROJECTEDWALLENTITY_H
#define PROJECTEDWALLENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "baseprojectedentity_shared.h"
#include "paintable_entity.h"
#include "utlvector.h"

// Bridge cross section: WALL_PROJECTOR_THICKNESS across the walking surface and
// WALL_PROJECTOR_HEIGHT through it.
#define WALL_PROJECTOR_THICKNESS	64.0f
#define WALL_PROJECTOR_HEIGHT		0.015625f

class CPhysCollide;
class CInfoPlacementHelper;

DECLARE_AUTO_LIST( IProjectedWallEntityAutoList );

class CProjectedWallEntity : public CPaintableEntity< CBaseProjectedEntity >, public IProjectedWallEntityAutoList
{
	DECLARE_CLASS( CProjectedWallEntity, CPaintableEntity< CBaseProjectedEntity > );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CProjectedWallEntity();
	virtual ~CProjectedWallEntity();

	virtual void	Precache( void );
	virtual void	Spawn( void );
	virtual void	OnRestore( void );
	virtual bool	CreateVPhysics( void );
	virtual bool	ShouldSavePhysics( void ) { return false; }
	virtual void	UpdateOnRemove( void );

	void			ProjectWall( void );

	// Extents used for tracing the projection and for the wall's own sweeps
	virtual void	GetProjectionExtents( Vector &outMins, Vector &outMaxs );
	void			GetExtents( Vector &outMins, Vector &outMaxs, float flWidthScale = 1.0f );

	// Collision is against the custom wall collideable
	virtual bool	TestCollision( const Ray_t &ray, unsigned int mask, trace_t &trace );
	virtual void	ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs );

	virtual void	NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier );

	virtual void	SetSourcePortal( CPortal_Base2D *pPortal );
	virtual void	SetHitPortal( CPortal_Base2D *pPortal );

	bool			IsHorizontal( void ) { return m_bIsHorizontal; }

	// Paint
	virtual void	Touch( CBaseEntity *pOther );

	virtual PaintPowerType GetPaintPowerAtPoint( const Vector &worldContactPt ) const;
	virtual void	Paint( PaintPowerType type, const Vector &worldContactPt );
	virtual void	CleansePaint( void );

	bool			IsWallPainted( const Vector &vecPosition ) const;
	int				GetNumSegments( void ) const;
	float			GetSegmentLength( void ) const;
	PaintPowerType	GetPaintPowerAtSegment( int i ) const;

	static CProjectedWallEntity *CreateNewInstance( void );
	virtual CBaseProjectedEntity *CreateNewProjectedEntity( void );

	virtual void	PlacePlacementHelper( CInfoPlacementHelper *pHelper );

	virtual void	OnProjected( void );

	// Push an entity out of the wall volume (used when something is teleported into it)
	void			DisplaceObstructingEntity( CBaseEntity *pEntity, bool bIgnoreStuck );

private:
	void			CleanupWall( void );
	void			DisplaceObstructingEntities( void );
	void			DisplaceObstructingEntity( CBaseEntity *pEntity, const Vector &vOrigin, const Vector &vWallUp, const Vector &vWallRight, bool bIgnoreStuck );
	void			TraceForWallEndpoint( Vector *pOutEndPoint );
	void			DrawWallBoundsHighlight( void );

	CPhysCollide	*m_pWallCollideable;

	// World space bounds of the wall collideable
	CNetworkVar( Vector, m_vWorldSpace_WallMins );
	CNetworkVar( Vector, m_vWorldSpace_WallMaxs );

	// Portal whose color tints the bridge
	CNetworkVar( CHandle<CPortal_Base2D>, m_hColorPortal );

	CNetworkVar( float, m_flLength );
	CNetworkVar( float, m_flHeight );
	CNetworkVar( float, m_flWidth );

	CNetworkVar( bool, m_bIsHorizontal );

	// The bridge is painted in fixed length segments
	CNetworkVar( int, m_nNumSegments );
	CNetworkVar( float, m_flSegmentLength );

	CNetworkVar( float, m_flParticleUpdateTime );

	CUtlVector< PaintPowerType > m_PaintPowers;

	int				ComputeSegmentIndex( const Vector &vWorldPositionOnWall ) const;
};

#endif // PROJECTEDWALLENTITY_H
