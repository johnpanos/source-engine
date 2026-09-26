//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the tractor beam (excursion funnel) trigger volume and
//			of the projected segment entity that owns it.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_TRIGGER_TRACTORBEAM_H
#define C_TRIGGER_TRACTORBEAM_H

#ifdef _WIN32
#pragma once
#endif

#include "c_triggers.h"
#include "vphysics_interface.h"
#include "c_baseprojectedentity.h"
#include "particles_new.h"
#include "utlvector.h"

class C_ProjectedTractorBeamEntity;
class C_PaintBlob;
class CMeshBuilder;
class IMaterial;

DECLARE_AUTO_LIST( ITriggerTractorBeamAutoList );

class C_Trigger_TractorBeam : public C_BaseVPhysicsTrigger, public IMotionEvent, public ITriggerTractorBeamAutoList
{
public:
	DECLARE_CLASS( C_Trigger_TractorBeam, C_BaseVPhysicsTrigger );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	virtual C_BaseEntity	*GetEntity( void ) { return this; }

	C_Trigger_TractorBeam();
	virtual ~C_Trigger_TractorBeam();

	virtual void			Spawn( void );
	virtual void			OnNewParticleEffect( const char *pszParticleName, CNewParticleEffect *pNewParticleEffect );
	// Portal 2 port: CS:GO renderable API bridged onto DrawModel( int ).
	PORTAL2_DRAWMODEL_BRIDGE();
	virtual int				DrawModel( int flags, const RenderableInstance_t &instance );
	virtual bool			ShouldDraw( void );
	virtual void			UpdateOnRemove( void );
	virtual void			OnDataChanged( DataUpdateType_t updateType );

	virtual C_BasePlayer	*GetPredictionOwner( void );
	virtual bool			ShouldPredict( void ) { C_BasePlayer *pPredOwner = GetPredictionOwner(); return ( pPredOwner && pPredOwner->IsLocalPlayer() ); }

	virtual void			UpdatePartitionListEntry( void );

	void					OnProjected( void );

	virtual void			StartTouch( C_BaseEntity *pOther );
	virtual void			EndTouch( C_BaseEntity *pOther );

	virtual void			PhysicsSimulate( void );

	virtual IMotionEvent::simresult_e Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );
	void					CalculateFrameMovement( IPhysicsObject *pObject, C_BaseEntity *pEntity, float deltaTime, Vector &linear, AngularImpulse &angular );

	void					UpdateBeam( const Vector &vStartPoint, const Vector &vEndPoint, float flLinearForce );
	void					SetDirection( const Vector &vStart, const Vector &vEnd );
	Vector					GetForceDirection( void ) const;
	int						GetLastUpdateFrame( void ) const;

	// Portal 2 port: translucency type bridged onto IsTransparent()/IsTwoPass().
	PORTAL2_TRANSLUCENCY_BRIDGE();
	virtual RenderableTranslucencyType_t ComputeTranslucencyType( void ) { return RENDERABLE_IS_TRANSLUCENT; }

	// The beam column's world-space box (the trigger has no model to bound it),
	// so the leaf system puts the renderable where DrawModel draws.
	virtual void			GetRenderBounds( Vector &vecMins, Vector &vecMaxs );
	virtual const QAngle&	GetRenderAngles( void );

	float					GetSpeed( void ) { return m_linearForce; }
	float					GetLinearForce( void ) const { return m_linearForce; }

	static void				RecvProxy_Start( const CRecvProxyData *pData, void *pStruct, void *pOut );
	static void				RecvProxy_End( const CRecvProxyData *pData, void *pStruct, void *pOut );

	float					GetLinearLimit( void );
	bool					HasGravityScale( void ) { return ( m_gravityScale != 1.0f ); }
	bool					HasAirDensity( void ) { return ( m_addAirDensity != 0.0f ); }
	bool					HasLinearLimit( void ) { return ( m_linearLimit != 0.0f ); }
	bool					HasLinearScale( void ) { return ( m_linearScale != 1.0f ); }
	bool					HasAngularLimit( void ) { return ( m_angularLimit != 0.0f ); }
	bool					HasAngularScale( void ) { return ( m_angularScale != 1.0f ); }
	bool					HasLinearForce( void ) { return ( m_linearForce != 0.0f ); }

	void					SetProxyEntity( C_ProjectedTractorBeamEntity *pProxy ) { m_hProxyEntity = pProxy; }

	const Vector&			GetStartPoint( void ) const { return m_vStart; }
	const Vector&			GetEndPoint( void ) const { return m_vEnd; }
	bool					IsReversed( void ) { return m_bReversed; }
	bool					IsFromPortal( void ) { return m_bFromPortal; }
	bool					IsToPortal( void ) { return m_bToPortal; }

	virtual void			GetToolRecordingState( KeyValues *msg );
	void					RestoreToToolRecordedState( KeyValues *pKV );
	virtual bool			GetSoundSpatialization( SpatializationInfo_t &info );

	const float&			GetBeamRadius( void ) { return m_flRadius; }

	// Paint blobs currently carried by this beam (shared blob simulation)
	void					RemoveDeadBlobs( void );
	void					RemoveChangedBeamBlobs( void );
	void					RemoveAllBlobsFromBeam( void );

	CUtlVector< C_PaintBlob* >	m_blobs;

protected:
	void					CreateParticles( void );

	void					DrawColumn( IMaterial *pMaterial, const Vector &vecStart, const Vector &vDir, float flLength, const Vector &vecXAxis, const Vector &vecYAxis, float flRadius, float flAlpha, bool bPinchIn, bool bPinchOut, float flTextureOffset );
	void					DrawColumnSegment( CMeshBuilder &meshBuilder, Vector &vecStart, const Vector &vDir, float flLength, const Vector &vecXAxis, const Vector &vecYAxis, float flRadius, float flAlpha, float flTextureOffset );

	Vector					m_vStart;
	Vector					m_vEnd;
	float					m_linearForce;
	float					m_flRadius;
	bool					m_bReversed;
	bool					m_bFromPortal;
	bool					m_bToPortal;
	bool					m_bDisablePlayerMove;

	IMaterial				*m_pMaterial1;
	IMaterial				*m_pMaterial2;
	IMaterial				*m_pMaterial3;
	bool					m_bRecreateParticles;

	CHandle<C_ProjectedTractorBeamEntity>	m_hProxyEntity;
	CUtlReference<CNewParticleEffect>		m_hCoreEffect;

	IPhysicsMotionController	*m_pController;

	// Motion controller settings (networked from the server's vphysics motion trigger)
	float					m_gravityScale;
	float					m_addAirDensity;
	float					m_linearLimit;
	float					m_linearLimitDelta;
	float					m_linearLimitTime;
	float					m_linearLimitStart;
	float					m_linearLimitStartTime;
	float					m_linearScale;
	float					m_angularLimit;
	float					m_angularScale;
	QAngle					m_linearForceAngles;

	// gpGlobals->framecount of the last UpdateBeam() (retail addition)
	int						m_nLastUpdateFrame;

private:
	float					m_flStartTime;
};

typedef C_Trigger_TractorBeam CTrigger_TractorBeam;


//-----------------------------------------------------------------------------
// Purpose: One segment of a tractor beam; owns the trigger volume for the segment
//-----------------------------------------------------------------------------
class C_ProjectedTractorBeamEntity : public C_BaseProjectedEntity
{
public:
	DECLARE_CLASS( C_ProjectedTractorBeamEntity, C_BaseProjectedEntity );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	virtual void GetProjectionExtents( Vector &outMins, Vector &outMaxs )
	{
		const int nBeamWidth = 2;
		Vector scratchMins( -nBeamWidth, -nBeamWidth, 0 );
		Vector scratchMaxs( nBeamWidth, nBeamWidth, 0 );

		VectorRotate( scratchMins, GetNetworkAngles(), outMins );
		VectorRotate( scratchMaxs, GetNetworkAngles(), outMaxs );
	}

	virtual void OnProjected( void )
	{
		BaseClass::OnProjected();

		C_Trigger_TractorBeam *pBeam = m_hTractorBeamTrigger.Get();
		if ( pBeam )
		{
			pBeam->SetPredictionEligible( GetPredictionEligible() );
			if ( IsPlayerSimulated() )
			{
				pBeam->SetPlayerSimulated( GetSimulatingPlayer() );
			}
			else
			{
				pBeam->UnsetPlayerSimulated();
			}

			// Reconstruction note: the 2010 client called a parameterless UpdateBeam() that read
			// the beam's own networked state. The retail UpdateBeam() takes the endpoints and a
			// signed force (negative reverses the beam), so the segment passes its projected
			// endpoints and the beam's force signed by its networked direction.
			float flLinearForce = pBeam->GetLinearForce();
			pBeam->UpdateBeam( GetStartPoint(), GetEndPoint(), pBeam->IsReversed() ? -flLinearForce : flLinearForce );
		}
	}

private:
	CHandle<C_Trigger_TractorBeam>	m_hTractorBeamTrigger;
};

#endif // C_TRIGGER_TRACTORBEAM_H
