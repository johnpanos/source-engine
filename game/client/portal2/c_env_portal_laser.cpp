//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of env_portal_laser: impact sparks.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_baseanimating.h"
#include "particles_new.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *LASER_SPARK_EFFECT_NAME = "discouragement_beam_sparks";

class C_PortalLaser : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_PortalLaser, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

	C_PortalLaser();

	virtual void Precache();
	virtual void Spawn();
	virtual void UpdateOnRemove();

	virtual void OnPreDataChanged( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );

private:
	void CreateSparkEffect();
	void UpdateSparkEffect();

	CUtlReference< CNewParticleEffect > m_pSparkEffect;
	Vector m_vecLaserEndPos;
	Vector m_vecOldLaserEndPos;
	Vector m_vecLaserImpactNormal;
	bool m_bShouldSpark;
	bool m_bOldShouldSpark;
};

IMPLEMENT_CLIENTCLASS_DT( C_PortalLaser, DT_PortalLaser, CPortalLaser )
	RecvPropVector( RECVINFO( m_vecLaserEndPos ) ),
	RecvPropVector( RECVINFO( m_vecLaserImpactNormal ) ),
	RecvPropBool( RECVINFO( m_bShouldSpark ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_PortalLaser::C_PortalLaser()
{
	m_pSparkEffect = NULL;
	m_vecLaserEndPos = vec3_origin;
	m_vecOldLaserEndPos = vec3_origin;
	m_vecLaserImpactNormal = vec3_origin;
	m_bShouldSpark = false;
	m_bOldShouldSpark = false;
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
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalLaser::Spawn()
{
	Precache();

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalLaser::UpdateOnRemove()
{
	if ( m_pSparkEffect )
	{
		ParticleProp()->StopEmission( m_pSparkEffect );
		m_pSparkEffect = NULL;
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalLaser::OnPreDataChanged( DataUpdateType_t updateType )
{
	m_vecOldLaserEndPos = m_vecLaserEndPos;
	m_bOldShouldSpark = m_bShouldSpark;

	BaseClass::OnPreDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalLaser::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		CreateSparkEffect();
	}

	// Move the sparks with the end of the beam
	if ( m_vecOldLaserEndPos != m_vecLaserEndPos )
	{
		UpdateSparkEffect();
	}

	if ( m_bShouldSpark != m_bOldShouldSpark )
	{
		if ( m_bShouldSpark )
		{
			CreateSparkEffect();

			if ( m_pSparkEffect.IsValid() )
			{
				m_pSparkEffect->StartEmission();
			}
		}
		else
		{
			if ( m_pSparkEffect.IsValid() )
			{
				m_pSparkEffect->StopEmission();
			}
		}
	}

	BaseClass::OnDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: The spark system is created stopped and toggled by m_bShouldSpark
//-----------------------------------------------------------------------------
void C_PortalLaser::CreateSparkEffect()
{
	if ( !m_pSparkEffect.IsValid() )
	{
		m_pSparkEffect = ParticleProp()->Create( LASER_SPARK_EFFECT_NAME, PATTACH_CUSTOMORIGIN, -1, vec3_origin );
		m_pSparkEffect->StopEmission();
	}

	UpdateSparkEffect();
}

//-----------------------------------------------------------------------------
// Purpose: Place the sparks at the impact point, facing along the surface normal
//-----------------------------------------------------------------------------
void C_PortalLaser::UpdateSparkEffect()
{
	if ( m_pSparkEffect.IsValid() )
	{
		m_pSparkEffect->SetControlPoint( 0, m_vecLaserEndPos );

		Vector vecRight, vecUp;
		VectorVectors( m_vecLaserImpactNormal, vecRight, vecUp );
		m_pSparkEffect->SetControlPointOrientation( 0, m_vecLaserImpactNormal, vecRight, vecUp );
	}
}
