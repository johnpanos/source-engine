//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of an entity projected from a projector (light bridge and
//			tractor beam segments), redirected through linked portals.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_BASEPROJECTEDENTITY_H
#define C_BASEPROJECTEDENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "c_baseplayer.h"
#include "c_portal_base2d.h"

class C_BaseProjector;

// The shared projection code names the owning projector with its server spelling.
typedef C_BaseProjector CBaseProjector;

class C_BaseProjectedEntity : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_BaseProjectedEntity, C_BaseEntity );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_BaseProjectedEntity();
	virtual ~C_BaseProjectedEntity();

	virtual const Vector&	GetStartPoint( void ) const { return m_vecStartPoint; }
	virtual const Vector&	GetEndPoint( void ) const { return m_vecEndPoint; }
	virtual Vector			GetLengthVector( void ) { return m_vecEndPoint - m_vecStartPoint; }

	void					RecursiveProjection( bool bShouldSpawn, CBaseProjector *pParentProjector, C_Portal_Base2D *pExitPortal, const Vector &vProjectOrigin, const QAngle &qProjectAngles, int iRemainingProjections, bool bDisablePlacementHelper );
	virtual bool			IsHittingPortal( Vector *pOutOrigin, QAngle *pOutAngles, C_Portal_Base2D **pOutPortal );
	virtual void			TestForProjectionChanges( void );

	virtual bool			DidRedirectionPortalMove( C_Portal_Base2D *pPortal );
	virtual void			FindProjectedEndpoints( void );

	virtual void			SetHitPortal( C_Portal_Base2D *pPortal );
	virtual C_Portal_Base2D	*GetHitPortal( void );

	virtual void			SetSourcePortal( C_Portal_Base2D *pPortal );
	virtual C_Portal_Base2D	*GetSourcePortal( void );

	virtual void			GetProjectionExtents( Vector &outMins, Vector &outMaxs );
	virtual void			OnProjected( void );

	// Retail addition used by the shared projection code (reflection paint redirection)
	void					TestForReflectPaint( void );

	virtual void			OnDataChanged( DataUpdateType_t updateType );
	virtual void			PostDataUpdate( DataUpdateType_t updateType );

	virtual C_BasePlayer	*GetPredictionOwner( void );
	virtual bool			ShouldPredict( void ) { C_BasePlayer *pPredOwner = GetPredictionOwner(); return ( pPredOwner && pPredOwner->IsLocalPlayer() ); }

	static void				TestAllForProjectionChanges( void );

	CHandle<C_Portal_Base2D>		m_hHitPortal;
	CHandle<C_Portal_Base2D>		m_hSourcePortal;
	Vector							m_vecSourcePortalCenter;
	Vector							m_vecSourcePortalRemoteCenter;
	QAngle							m_vecSourcePortalAngle;
	QAngle							m_vecSourcePortalRemoteAngle;
	Vector							m_vecStartPoint;
	Vector							m_vecEndPoint;
	CHandle<C_BaseProjectedEntity>	m_hChildSegment;
	CHandle<C_BaseEntity>			m_hPlacementHelper;

	int								m_iMaxRemainingRecursions;
	bool							m_bCreatePlacementHelper;

	// Projection state captured by OnProjected() so derived classes can tell whether a
	// network update really moved the projection (retail addition; not in the 2010 layout).
	struct PreDataChanged_t
	{
		Vector	vStartPoint;
		Vector	vEndPoint;
		QAngle	qAngles;
	};
	PreDataChanged_t				PreDataChanged;
};

#endif // C_BASEPROJECTEDENTITY_H
