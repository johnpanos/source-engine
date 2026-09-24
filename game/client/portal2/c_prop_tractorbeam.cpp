//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the tractor beam (excursion funnel) projector model:
//			arm/rotation animation and emitter particles.
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

class C_PropTractorBeamProjector : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_PropTractorBeamProjector, C_BaseAnimating );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	virtual void	Spawn( void );
	virtual void	ClientThink( void );
	virtual void	UpdateOnRemove( void );

	static void		RecvProxy_LinearForce( const CRecvProxyData *pData, void *pStruct, void *pOut );
	static void		RecvProxy_Enabled( const CRecvProxyData *pData, void *pStruct, void *pOut );

	float			CalculateRotationPose( void );
	float			CalculateArmaturePose( void );

	virtual void	OnToolStartRecording( void );

protected:

	void			CreateEffect( void );
	void			UpdateEffect( void );
	void			StopEffect( void );

	float			m_flLinearForce;
	Vector			m_vEndPos;
	bool			m_bEffectsActive;
	bool			m_bActivated;
	bool			m_bEnabled;

	// Arm extension animation
	float			m_flArmatureStart;
	float			m_flArmatureTarget;
	float			m_flArmatureDuration;
	float			m_flArmatureStartTime;

	// Rotation (playback rate) animation
	float			m_flRotationStart;
	float			m_flRotationTarget;
	float			m_flRotationDuration;
	float			m_flRotationStartTime;

	CUtlReference<CNewParticleEffect>	m_hEmitterEffect[3];
};

//-----------------------------------------------------------------------------
// Purpose: Spin the projector toward the new force
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::RecvProxy_LinearForce( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PropTractorBeamProjector *pBeam = (C_PropTractorBeamProjector *)pStruct;

	float flLinearForce = pData->m_Value.m_Float;
	if ( flLinearForce == pBeam->m_flLinearForce )
		return;

	pBeam->m_flLinearForce = flLinearForce;

	// Blend the rotation from where it is now to the new speed
	pBeam->m_flRotationStart = pBeam->CalculateRotationPose();
	pBeam->m_flRotationStartTime = gpGlobals->curtime;
	pBeam->m_flRotationDuration = 0.25f;
	pBeam->m_flRotationTarget = pBeam->m_flLinearForce / 120.0f;

	// Extend the arms if we're pushing, retract them if we're pulling
	pBeam->m_flArmatureStart = pBeam->CalculateArmaturePose();
	pBeam->m_flArmatureTarget = ( pBeam->m_flLinearForce > 0.0f ) ? 1.0f : 0.0f;
	pBeam->m_flArmatureDuration = 0.75f;
	pBeam->m_flArmatureStartTime = gpGlobals->curtime;

	pBeam->UpdateEffect();
}

//-----------------------------------------------------------------------------
// Purpose: Wind the projector up or down when it is toggled
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::RecvProxy_Enabled( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PropTractorBeamProjector *pBeam = (C_PropTractorBeamProjector *)pStruct;

	bool bEnabled = ( pData->m_Value.m_Int == 1 );
	if ( pBeam->m_bEnabled == bEnabled )
		return;

	pBeam->m_bEnabled = bEnabled;

	if ( bEnabled )
	{
		pBeam->m_flRotationStart = pBeam->CalculateRotationPose();
		pBeam->m_flRotationStartTime = gpGlobals->curtime;
		pBeam->m_flRotationDuration = 0.25f;
		pBeam->m_flRotationTarget = pBeam->m_flLinearForce / 120.0f;

		pBeam->m_flArmatureStart = pBeam->CalculateArmaturePose();
		pBeam->m_flArmatureTarget = ( pBeam->m_flLinearForce > 0.0f ) ? 1.0f : 0.0f;
		pBeam->m_flArmatureDuration = 0.75f;
		pBeam->m_flArmatureStartTime = gpGlobals->curtime;

		if ( !pBeam->m_bEffectsActive && pBeam->m_bActivated )
		{
			pBeam->CreateEffect();
		}
	}
	else
	{
		// Spin down
		pBeam->m_flRotationStart = pBeam->CalculateRotationPose();
		pBeam->m_flRotationStartTime = gpGlobals->curtime;
		pBeam->m_flRotationDuration = 1.5f;
		pBeam->m_flRotationTarget = 0.0f;

		// Arms go to rest halfway
		pBeam->m_flArmatureStart = pBeam->CalculateArmaturePose();
		pBeam->m_flArmatureTarget = 0.5f;
		pBeam->m_flArmatureDuration = 1.5f;
		pBeam->m_flArmatureStartTime = gpGlobals->curtime;

		pBeam->StopEffect();
	}
}

IMPLEMENT_CLIENTCLASS_DT( C_PropTractorBeamProjector, DT_PropTractorBeamProjector, CPropTractorBeamProjector )
	RecvPropFloat( RECVINFO( m_flLinearForce ), 0, C_PropTractorBeamProjector::RecvProxy_LinearForce ),
	RecvPropVector( RECVINFO( m_vEndPos ) ),
	RecvPropInt( RECVINFO( m_bEnabled ), 0, C_PropTractorBeamProjector::RecvProxy_Enabled ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_PropTractorBeamProjector )
	DEFINE_PRED_FIELD( m_flPlaybackRate, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::Spawn( void )
{
	BaseClass::Spawn();

	SetNextClientThink( CLIENT_THINK_ALWAYS );

	m_bEffectsActive = false;
	m_bActivated = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::UpdateOnRemove( void )
{
	// Reconstruction note: the build only stops the emitters here; it does not chain to
	// C_BaseAnimating::UpdateOnRemove().
	StopEffect();
}

//-----------------------------------------------------------------------------
// Purpose: Attach an arm particle to each emitter attachment
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::CreateEffect( void )
{
	StopEffect();

	const char *pAttachmentNames[3] = { "emitter1", "emitter2", "emitter3" };

	for ( int i = 0; i < 3; i++ )
	{
		if ( m_hEmitterEffect[i] )
		{
			ParticleProp()->StopEmission( m_hEmitterEffect[i], false, false );
			m_hEmitterEffect[i] = NULL;
		}

		m_hEmitterEffect[i] = ParticleProp()->Create( "tractor_beam_arm", PATTACH_POINT_FOLLOW, pAttachmentNames[i] );
		if ( m_hEmitterEffect[i] )
		{
			ParticleProp()->AddControlPoint( m_hEmitterEffect[i], 1, this, PATTACH_ABSORIGIN );
			ParticleProp()->AddControlPoint( m_hEmitterEffect[i], 2, this, PATTACH_ABSORIGIN );

			Vector color = ( m_flLinearForce < 0.0f ) ? Vector( 255, 160, 32 ) : Vector( 64, 160, 255 );
			m_hEmitterEffect[i]->SetControlPoint( 1, color );

			Vector vForward;
			AngleVectors( GetAbsAngles(), &vForward );
			Vector vVelocity = vForward * m_flLinearForce;
			m_hEmitterEffect[i]->SetControlPoint( 2, vVelocity );
		}
	}

	m_bEffectsActive = true;
}

//-----------------------------------------------------------------------------
// Purpose: Recolor and re-aim the emitters after a force change
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::UpdateEffect( void )
{
	for ( int i = 0; i < 3; i++ )
	{
		if ( m_hEmitterEffect[i] )
		{
			Vector color = ( m_flLinearForce < 0.0f ) ? Vector( 255, 160, 32 ) : Vector( 64, 160, 255 );
			m_hEmitterEffect[i]->SetControlPoint( 1, color );

			Vector vForward;
			AngleVectors( GetAbsAngles(), &vForward );
			Vector vVelocity = vForward * m_flLinearForce;
			m_hEmitterEffect[i]->SetControlPoint( 2, vVelocity );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::StopEffect( void )
{
	for ( int i = 0; i < 3; i++ )
	{
		if ( m_hEmitterEffect[i] )
		{
			ParticleProp()->StopEmission( m_hEmitterEffect[i], false, false );
			m_hEmitterEffect[i] = NULL;
		}
	}

	m_bEffectsActive = false;
}

//-----------------------------------------------------------------------------
// Purpose: Drive the arm pose and spin rate
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::ClientThink( void )
{
	// Start the emitters once the map has settled
	if ( m_bEnabled && !m_bEffectsActive && m_flLinearForce != 0.0f && gpGlobals->curtime > 0.5f )
	{
		CreateEffect();
		m_bActivated = true;
	}

	float flArmPose = CalculateArmaturePose();
	int nPoseParam = LookupPoseParameter( "reversal" );
	SetPoseParameter( nPoseParam, flArmPose );

	float flAnimSpeed = CalculateRotationPose();
	SetPlaybackRate( flAnimSpeed );

	UpdateClientSideAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: Current spin rate, never faster than the beam's force allows
//-----------------------------------------------------------------------------
float C_PropTractorBeamProjector::CalculateRotationPose( void )
{
	if ( gpGlobals->curtime > m_flRotationStartTime + m_flRotationDuration )
		return m_flRotationTarget;

	float flRotationPose = SimpleSplineRemapVal( gpGlobals->curtime, m_flRotationStartTime, m_flRotationStartTime + m_flRotationDuration, m_flRotationStart, m_flRotationTarget );

	float flForcePerc = m_flLinearForce / 120.0f;
	bool bNegativeForce = ( flForcePerc < 0.0f );
	if ( flForcePerc != 0.0f )
	{
		if ( bNegativeForce )
		{
			if ( flRotationPose < flForcePerc )
				return flForcePerc;
		}
		else if ( flRotationPose > flForcePerc )
		{
			return flForcePerc;
		}
	}

	return flRotationPose;
}

//-----------------------------------------------------------------------------
// Purpose: Current arm extension in [0,1]
//-----------------------------------------------------------------------------
float C_PropTractorBeamProjector::CalculateArmaturePose( void )
{
	if ( gpGlobals->curtime > m_flArmatureStartTime + m_flArmatureDuration )
		return m_flArmatureTarget;

	float flArmPose = SimpleSplineRemapVal( gpGlobals->curtime, m_flArmatureStartTime, m_flArmatureStartTime + m_flArmatureDuration, m_flArmatureStart, m_flArmatureTarget );

	return clamp( flArmPose, 0.0f, 1.0f );
}

//-----------------------------------------------------------------------------
// Purpose: Recreate the emitters so the recording captures them
//-----------------------------------------------------------------------------
void C_PropTractorBeamProjector::OnToolStartRecording( void )
{
	if ( m_bEffectsActive )
	{
		CreateEffect();
	}
}
