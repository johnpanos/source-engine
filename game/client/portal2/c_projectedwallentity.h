//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the projected light bridge (hard light wall) segment.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_PROJECTEDWALLENTITY_H
#define C_PROJECTEDWALLENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "c_baseprojectedentity.h"
#include "paintable_entity.h"
#include "utlvector.h"

class CPhysCollide;
class CMeshBuilder;
class IMaterial;
class IMaterialVar;
class IMesh;

DECLARE_AUTO_LIST( IProjectedWallEntityAutoList );

enum ProjectedWallSegmentType
{
	BASE_SEGMENT = 0,
	PAINT_END_1_SEGMENT,
	PAINT_END_2_SEGMENT,
	PAINT_MIDDLE_SEGMENT,
	PAINT_SINGLE_SEGMENT,
};

class C_ProjectedWallEntity : public CPaintableEntity<C_BaseProjectedEntity>, public IProjectedWallEntityAutoList
{
public:
	DECLARE_CLASS( C_ProjectedWallEntity, CPaintableEntity<C_BaseProjectedEntity> );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	virtual C_BaseEntity	*GetEntity( void ) { return this; }

	C_ProjectedWallEntity();

	virtual void			UpdateOnRemove( void );
	virtual CollideType_t	GetCollideType( void );
	virtual int				DrawModel( int flags, const RenderableInstance_t &instance );
	virtual bool			ShouldDraw( void ) { return true; }
	virtual void			GetRenderBounds( Vector &vecMins, Vector &vecMaxs );
	virtual const QAngle&	GetRenderAngles( void );
	virtual RenderableTranslucencyType_t ComputeTranslucencyType( void );
	virtual void			ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs );
	virtual void			OnPreDataChanged( DataUpdateType_t type );
	virtual void			OnDataChanged( DataUpdateType_t updateType );
	virtual void			Spawn( void );
	virtual void			ClientThink( void );
	virtual bool			TestCollision( const Ray_t &ray, unsigned int mask, trace_t &trace );
	virtual bool			TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t &tr );

	virtual void			OnProjected( void );
	virtual void			GetProjectionExtents( Vector &outMins, Vector &outMaxs );
	void					GetExtents( Vector &outMins, Vector &outMaxs, float flWidthScale = 1.0f );

	void					ProjectWall( void );

	// Give paint power users touching the bridge the paint on the touched segment
	virtual void			Touch( C_BaseEntity *pOther );

	virtual PaintPowerType	GetPaintPowerAtPoint( const Vector &worldContactPt ) const;
	virtual void			Paint( PaintPowerType type, const Vector &worldContactPt );
	virtual void			CleansePaint( void );

	void					SetPaintPower( int nSegment, PaintPowerType power );

	virtual void			GetToolRecordingState( KeyValues *msg );
	void					RestoreToToolRecordedState( KeyValues *pKV );

	// Push entities out of the bridge (shared with the server)
	void					DisplaceObstructingEntity( C_BaseEntity *pEntity, bool bIgnoreStuck );

private:
	void					DisplaceObstructingEntities( void );
	void					DisplaceObstructingEntity( C_BaseEntity *pEntity, const Vector &vOrigin, const Vector &vWallUp, const Vector &vWallRight, bool bIgnoreStuck );
	void					DrawQuadHelper( CMeshBuilder *meshBuilder, const Vector &vOrigin, const Vector &vRight, const Vector &vUp, float flTextureScaleU, float flTextureScaleV, float flAlpha );

	virtual bool			InitMaterials( void );

	void					ColorWallByPortal( IMaterial *pBody );

	bool					ShouldSpawnParticles( C_Portal_Base2D *pPortal );
	void					SetupWallParticles( void );

	void					PaintWallWithPaint( IMesh *pMesh, CMeshBuilder meshBuilder, ProjectedWallSegmentType type );
	void					PaintWallWithPaintSegment( IMesh *pMesh, CMeshBuilder meshBuilder, ProjectedWallSegmentType type, Vector vecOrigin, Vector vecUp, Vector vecForward, Vector vecRight );

	IMaterial				*m_pBodyMaterial;
	IMaterial				*m_pPaintMaterialMid;
	IMaterial				*m_pPaintMaterialEnd1;
	IMaterial				*m_pPaintMaterialEnd2;
	IMaterial				*m_pPaintMaterialSing;
	IMaterialVar			*m_pPaintColorMid;
	IMaterialVar			*m_pPaintColorEnd1;
	IMaterialVar			*m_pPaintColorEnd2;
	IMaterialVar			*m_pPaintColorSing;
	IMaterial				*m_pSideRailMaterial;

	// World space bounds of the wall
	Vector					m_vWorldSpace_WallMins;
	Vector					m_vWorldSpace_WallMaxs;

public:
	// A wall collision shape and the time it became valid; predicted walls keep a short history
	struct WallCollideableAtTime_t
	{
		Vector vStart, vEnd;
		Vector vWorldMins, vWorldMaxs;
		QAngle qAngles;
		float flTime[2];		// [0] tick time the shape was built, [1] curtime it was built
		CPhysCollide *pCollideable;
		bool bIsPredicted;
	};

private:
	CUtlVector<WallCollideableAtTime_t> m_WallCollideables;

	float					m_flLength;
	float					m_flWidth;
	float					m_flHeight;
	bool					m_bIsHorizontal;
	EHANDLE					m_hColorPortal;

	float					m_flCurDisplayLength;
	float					m_fLastProjectedTime;

	bool					SetPaintSurfaceColor( const PaintPowerType &power );

	int						m_nNumSegments;
	float					m_flSegmentLength;

	CUtlVector<PaintPowerType>	m_PaintPowers;

	// Server bumps this to make clients rebuild the wall particles
	float					m_flParticleUpdateTime;
	float					m_flPrevParticleUpdateTime;

	int						ComputeSegmentIndex( const Vector &vWorldPositionOnWall ) const;
};

typedef C_ProjectedWallEntity CProjectedWallEntity;

#endif // C_PROJECTEDWALLENTITY_H
