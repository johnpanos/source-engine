//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of env_portal_laser. The client draws the whole beam:
//			it re-traces the server's line through portals every frame and
//			places one beam per segment, then puts the sparks at the end.
//
// Reconstructed from the retail Linux client.so (Portal 2 build 2014); the
// 2010 Steam2 builds (external/portal2_steam2_decompiled) drew the beam on the
// server instead. Not original Valve source; the repository's provenance and
// distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_baseanimating.h"
#include "beam_shared.h"
#include "particles_new.h"
#include "portal_util_shared.h"
#include "portalsimulation.h"
#include "c_portal_beam_helper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *LASER_SPARK_EFFECT_NAME = "discouragement_beam_sparks";

class C_PortalLaser : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_PortalLaser, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

	C_PortalLaser();

	virtual void Spawn();
	virtual void Precache();
	virtual void UpdateOnRemove();

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void ClientThink();

private:
	void CreateSparkEffect();
	void StopSparkEffect();

	C_PortalBeamHelper m_beamHelper;
	CHandle< C_BaseEntity > m_hReflector;
	CUtlReference< CNewParticleEffect > m_pSparkEffect;
	Vector m_vStartPoint;
	Vector m_vEndPoint;
	bool m_bLaserOn;
	bool m_bIsLethal;
	bool m_bIsAutoAiming;
	bool m_bShouldSpark;
	bool m_bUseParentDir;
	QAngle m_angParentAngles;
};

IMPLEMENT_CLIENTCLASS_DT( C_PortalLaser, DT_PortalLaser, CPortalLaser )
	RecvPropEHandle( RECVINFO( m_hReflector ) ),
	RecvPropVector( RECVINFO( m_vStartPoint ) ),
	RecvPropVector( RECVINFO( m_vEndPoint ) ),
	RecvPropBool( RECVINFO( m_bLaserOn ) ),
	RecvPropBool( RECVINFO( m_bIsLethal ) ),
	RecvPropBool( RECVINFO( m_bIsAutoAiming ) ),
	RecvPropBool( RECVINFO( m_bShouldSpark ) ),
	RecvPropBool( RECVINFO( m_bUseParentDir ) ),
	RecvPropQAngles( RECVINFO( m_angParentAngles ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_PortalLaser::C_PortalLaser()
{
	m_pSparkEffect = NULL;
	m_vStartPoint = vec3_origin;
	m_vEndPoint = vec3_origin;
	m_bLaserOn = false;
	m_bIsLethal = false;
	m_bIsAutoAiming = false;
	m_bShouldSpark = false;
	m_bUseParentDir = false;
	m_angParentAngles = vec3_angle;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalLaser::Precache()
{
	PrecacheParticleSystem( LASER_SPARK_EFFECT_NAME );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: The first beam is the template for the segments after portals
//-----------------------------------------------------------------------------
void C_PortalLaser::Spawn()
{
	Precache();

	// Reconstruction note: retail reads m_bIsLethal here, but the client spawns
	// entities before their first network update, so this is the default.
	C_Beam *pBeam;
	if ( m_bIsLethal )
	{
		pBeam = C_Beam::BeamCreate( "sprites/laserbeam.vmt", 2.0f );
		pBeam->SetColor( 100, 255, 100 );
	}
	else
	{
		pBeam = C_Beam::BeamCreate( "sprites/purplelaser1.vmt", 32.0f );
	}

	m_beamHelper.Init( pBeam );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalLaser::UpdateOnRemove()
{
	StopSparkEffect();

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalLaser::OnDataChanged( DataUpdateType_t updateType )
{
	if ( !m_bShouldSpark )
	{
		StopSparkEffect();
	}
	else if ( !m_pSparkEffect.IsValid() )
	{
		CreateSparkEffect();
	}

	if ( m_bLaserOn )
	{
		m_beamHelper.TurnOn();
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
	else
	{
		m_beamHelper.TurnOff();
		SetNextClientThink( CLIENT_THINK_NEVER );
	}

	BaseClass::OnDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: Traces the beam through portals every frame
//-----------------------------------------------------------------------------
void C_PortalLaser::ClientThink()
{
	C_BaseEntity *pReflector = m_hReflector.Get();

	CTraceFilterSimpleClassnameList traceFilter( pReflector, COLLISION_GROUP_NONE );
	traceFilter.AddClassnameToIgnore( "projected_wall_entity" );
	traceFilter.AddClassnameToIgnore( "player" );
	traceFilter.AddClassnameToIgnore( "point_laser_target" );

	trace_t tr;
	if ( pReflector == NULL )
	{
		m_beamHelper.UpdatePoints( m_vStartPoint, m_vEndPoint, MASK_SHOT & ~CONTENTS_WINDOW, &traceFilter, &tr );
	}
	else
	{
		// Beams off a reflective cube follow the cube as the client sees it
		Vector vDir;
		if ( m_bUseParentDir )
		{
			AngleVectors( m_angParentAngles, &vDir );
		}
		else
		{
			AngleVectors( pReflector->GetAbsAngles(), &vDir );
		}

		Vector vStart = pReflector->WorldSpaceCenter() + vDir * 22.0f;

		// A cube sticking out of a portal emits from the other side
		CPortalSimulator *pSimulator = CPortalSimulator::GetSimulatorThatOwnsEntity( pReflector );
		if ( pSimulator && pSimulator->EntityIsInPortalHole( pReflector ) )
		{
			const VPlane &portalPlane = pSimulator->GetInternalData().Placement.PortalPlane;
			if ( portalPlane.DistTo( vStart ) < 0.0f && portalPlane.DistTo( pReflector->WorldSpaceCenter() ) > 0.0f )
			{
				const VMatrix &matThisToLinked = pSimulator->GetInternalData().Placement.matThisToLinked;
				vStart = matThisToLinked * vStart;
				vDir = matThisToLinked.ApplyRotation( vDir );
			}
		}

		if ( m_bIsAutoAiming )
		{
			// The server bent the beam onto a target
			vDir = m_vEndPoint - vStart;
			VectorNormalize( vDir );
		}
		else
		{
			UTIL_Portal_Laser_Prevent_Tilting( vDir );
		}

		m_beamHelper.UpdatePointDirection( vStart, vDir, MASK_SHOT & ~CONTENTS_WINDOW, &traceFilter, &tr );
	}

	// Sparks at the end of the last segment, facing along the surface normal
	if ( m_bShouldSpark && m_pSparkEffect.IsValid() )
	{
		m_pSparkEffect->SetControlPoint( 0, tr.endpos );

		Vector vecRight, vecUp;
		VectorVectors( tr.plane.normal, vecRight, vecUp );
		m_pSparkEffect->SetControlPointOrientation( 0, tr.plane.normal, vecRight, vecUp );
	}

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: ClientThink moves the sparks to the end of the beam
//-----------------------------------------------------------------------------
void C_PortalLaser::CreateSparkEffect()
{
	m_pSparkEffect = ParticleProp()->Create( LASER_SPARK_EFFECT_NAME, PATTACH_CUSTOMORIGIN, -1, vec3_origin );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalLaser::StopSparkEffect()
{
	if ( m_pSparkEffect.IsValid() )
	{
		ParticleProp()->StopEmission( m_pSparkEffect );
		m_pSparkEffect = NULL;
	}
}
