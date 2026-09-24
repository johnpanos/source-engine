//========= Portal 2 reconstruction ============================================//
//
// Purpose: Excursion funnel volume (trigger_tractorbeam) and the projected
//			entity that carries it through portals.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef TRIGGER_TRACTORBEAM_H
#define TRIGGER_TRACTORBEAM_H

#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"
#include "vphysics_interface.h"
#include "baseprojectedentity.h"
#include "prop_tractorbeam.h"
#include "utlvector.h"

class CProjectedTractorBeamEntity;
class CPaintBlob;
class CSoundPatch;

DECLARE_AUTO_LIST( ITriggerTractorBeamAutoList );

class CTrigger_TractorBeam : public CBaseVPhysicsTrigger, public IMotionEvent, public ITriggerTractorBeamAutoList
{
	DECLARE_CLASS( CTrigger_TractorBeam, CBaseVPhysicsTrigger );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CTrigger_TractorBeam();
	~CTrigger_TractorBeam();

	virtual void	Spawn( void );
	virtual void	Precache( void );
	virtual void	Activate( void );
	virtual void	UpdateOnRemove( void );
	virtual void	StopLoopingSounds( void );
	virtual bool	CreateVPhysics( void );
	virtual void	OnRestore( void );

	//
	// Touch handling
	//
	virtual void	StartTouch( CBaseEntity *pOther );
	virtual void	EndTouch( CBaseEntity *pOther );

	void			InputSetVelocityLimitTime( inputdata_t &inputdata );

	float			GetLinearLimit( void );

	void			SetLinearForce( const Vector &vDir, float flForce )
	{
		QAngle angForce;
		VectorAngles( vDir, angForce );
		m_linearForceAngles = angForce;
		m_linearForce = flForce;
	}

	void			SetLinearForce( float flForce )
	{
		m_linearForce = flForce;
	}

	void			SetAsReversed( bool bReversed ) { m_bReversed = bReversed; }

	void			SetGravityScale( float flScale ) { m_gravityScale = flScale; }
	void			SetAirDensity( float flDensity ) { m_addAirDensity = flDensity; }
	void			SetLinearLimit( float flLimit ) { m_linearLimit = flLimit; }
	void			SetAngularLimt( float flLimit ) { m_angularLimit = flLimit; }
	void			SetProxyEntity( CProjectedTractorBeamEntity *pProxy ) { m_hProxyEntity = pProxy; }
	void			DisablePlayerMovement( bool bDisable ) { m_bDisablePlayerMove = bDisable; }

	bool			HasGravityScale( void ) { return ( m_gravityScale != 1.0f ); }
	bool			HasAirDensity( void ) { return ( m_addAirDensity != 0.0f ); }
	bool			HasLinearLimit( void ) { return ( m_linearLimit != 0.0f ); }
	bool			HasLinearScale( void ) { return ( m_linearScale != 1.0f ); }
	bool			HasAngularLimit( void ) { return ( m_angularLimit != 0.0f ); }
	bool			HasAngularScale( void ) { return ( m_angularScale != 1.0f ); }
	bool			HasLinearForce( void ) { return ( m_linearForce != 0.0f ); }

	void			SetDirection( const Vector &vStart, const Vector &vEnd );

	static CTrigger_TractorBeam *CreateTractorBeam( const Vector &vStart, const Vector &vEnd, CBaseEntity *pOwner );
	void			UpdateBeam( const Vector &vStartPoint, const Vector &vEndPoint, float flLinearForce );

	// IMotionEvent
	virtual IMotionEvent::simresult_e Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );
	void			CalculateFrameMovement( IPhysicsObject *pObject, CBaseEntity *pEntity, float deltaTime, Vector &linear, AngularImpulse &angular );
	void			WakeTouchingObjects( void );

	// Attach an entity that is already inside us to our motion controller
	void			ForceAttachEntity( CBaseEntity *pEntity );

	const Vector	&GetStartPoint( void ) const { return m_vStart.Get(); }
	const Vector	&GetEndPoint( void ) const { return m_vEnd.Get(); }
	Vector			GetForceDirection( void ) const;
	float			GetLinearForce( void ) const { return m_linearForce; }
	bool			IsReversed( void ) { return m_bReversed; }
	bool			IsFromPortal( void ) { return m_bFromPortal; }
	bool			IsToPortal( void ) { return m_bToPortal; }

	void			SetBeamRadius( float flRadius ) { m_flRadius = flRadius; }
	const float		&GetBeamRadius( void ) { return m_flRadius.Get(); }

	int				GetLastUpdateFrame( void ) const;

	// Paint blobs travelling in this beam
	void			RemoveDeadBlobs( void );
	void			RemoveChangedBeamBlobs( void );
	void			RemoveAllBlobsFromBeam( void );

	CUtlVector< CPaintBlob* > m_blobs;

private:
	void			TractorThink( void );

	IPhysicsMotionController *m_pController;

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_bDisabled );

	CNetworkVar( float, m_gravityScale );
	CNetworkVar( float, m_addAirDensity );
	CNetworkVar( float, m_linearLimit );
	CNetworkVar( float, m_linearLimitDelta );
	CNetworkVar( float, m_linearLimitTime );
	CNetworkVar( float, m_linearLimitStart );
	CNetworkVar( float, m_linearLimitStartTime );
	CNetworkVar( float, m_linearScale );
	CNetworkVar( float, m_angularLimit );
	CNetworkVar( float, m_angularScale );
	CNetworkVar( QAngle, m_linearForceAngles );

	CNetworkHandle( CProjectedTractorBeamEntity, m_hProxyEntity );	// Projected entity that owns this volume

	CNetworkVar( Vector, m_vStart );
	CNetworkVar( Vector, m_vEnd );
	CNetworkVar( float, m_linearForce );
	CNetworkVar( float, m_flRadius );
	CNetworkVar( bool, m_bReversed );
	CNetworkVar( bool, m_bFromPortal );
	CNetworkVar( bool, m_bToPortal );
	CNetworkVar( bool, m_bDisablePlayerMove );

	CSoundPatch		*m_sndPlayerInBeam;
	CSoundPatch		*m_sndAmbient;

	int				m_nLastUpdateFrame;	// gpGlobals->framecount of the last UpdateBeam()
};


//-----------------------------------------------------------------------------
// Purpose: One segment of a tractor beam; owns the trigger volume for the segment
//-----------------------------------------------------------------------------
class CProjectedTractorBeamEntity : public CBaseProjectedEntity
{
	DECLARE_CLASS( CProjectedTractorBeamEntity, CBaseProjectedEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	virtual void	Spawn( void )
	{
		BaseClass::Spawn();

		// Create the volume that does the pushing
		m_hTractorBeamTrigger = CTrigger_TractorBeam::CreateTractorBeam( GetStartPoint(), GetEndPoint(), this );

		SetTransmitState( FL_EDICT_ALWAYS );
	}

	virtual void	UpdateOnRemove( void )
	{
		if ( m_hTractorBeamTrigger.Get() )
		{
			UTIL_Remove( m_hTractorBeamTrigger.Get() );
		}

		BaseClass::UpdateOnRemove();
	}

	virtual void	GetProjectionExtents( Vector &outMins, Vector &outMaxs )
	{
		const int nBeamWidth = 2;
		Vector scratchMins( -nBeamWidth, -nBeamWidth, 0 );
		Vector scratchMaxs( nBeamWidth, nBeamWidth, 0 );

		VectorRotate( scratchMins, GetAbsAngles(), outMins );
		VectorRotate( scratchMaxs, GetAbsAngles(), outMaxs );
	}

	// The force comes from the projector that owns every segment of the beam
	float			GetLinearForce( void )
	{
		// Reconstruction note: body recovered from its inlined copy in the 2010
		// CTrigger_TractorBeam::UpdateBeam(); without a projector the beam pulls at unit force.
		CPropTractorBeamProjector *pProjector = static_cast< CPropTractorBeamProjector* >( GetOwnerEntity() );
		if ( pProjector )
			return pProjector->GetLinearForce();

		return -1.0f;
	}

	static CProjectedTractorBeamEntity *CreateNewInstance( void );
	virtual CBaseProjectedEntity *CreateNewProjectedEntity( void );
	virtual void	OnProjected( void );

private:
	CNetworkHandle( CTrigger_TractorBeam, m_hTractorBeamTrigger );
};

#endif // TRIGGER_TRACTORBEAM_H
