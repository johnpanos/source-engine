//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of npc_personality_core: lit mouth, eye flashlight effect.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "particle_property.h"
#include "iefx.h"
#include "dlight.h"
#include "view.h"
#include "functionproxy.h"
#include "imaterialproxydict.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_NPC_Personality_Core : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_NPC_Personality_Core, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

	virtual void	UpdateOnRemove( void );
	virtual void	ControlMouth( CStudioHdr *pStudioHdr );
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	ClientThink( void );

	float			GetMouthAmount( void ) const { return m_flMouthAmount; }

private:
	float			m_flMouthAmount;
	float			m_flInvDim;

	bool			m_bFlashlightEnabled;

	CUtlReference<CNewParticleEffect>	m_pFlashlightEffect;
	dlight_t		*m_pELight;
};

//-----------------------------------------------------------------------------
// Purpose: Stop the flashlight effect and release the eye light.
//-----------------------------------------------------------------------------
void C_NPC_Personality_Core::UpdateOnRemove( void )
{
	if ( m_pFlashlightEffect )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_pFlashlightEffect );
		m_pFlashlightEffect = NULL;
	}

	if ( m_pELight )
	{
		m_pELight->die = gpGlobals->curtime;
		m_pELight = NULL;
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Drive the mouth glow from the speech envelope.
//-----------------------------------------------------------------------------
void C_NPC_Personality_Core::ControlMouth( CStudioHdr *pStudioHdr )
{
	BaseClass::ControlMouth( pStudioHdr );

	if ( !MouthInfo().NeedsEnvelope() || !pStudioHdr )
		return;

	float flOpenAmount = GetMouth()->mouthopen / 64.0f;
	if ( flOpenAmount > 1.0f )
	{
		flOpenAmount = 1.0f;
	}
	else if ( flOpenAmount < 0.2f )
	{
		flOpenAmount = 0.0f;
	}

	float flDarken = MIN( flOpenAmount * 3.0f, 1.0f );

	// Darken quickly while talking, recover slowly
	if ( flDarken > m_flInvDim )
	{
		m_flInvDim = flDarken;
	}
	else
	{
		m_flInvDim *= ExponentialDecay( 0.98f, 0.033f, gpGlobals->frametime );
	}

	m_flMouthAmount = MAX( flOpenAmount, ( 1.0f - m_flInvDim ) * 0.8f );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_NPC_Personality_Core::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
		MouthInfo().ActivateEnvelope();
		m_flMouthAmount = 0.0f;
		m_flInvDim = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Keep the eye flashlight effect and its dynamic light up to date.
//-----------------------------------------------------------------------------
void C_NPC_Personality_Core::ClientThink( void )
{
	if ( !m_bFlashlightEnabled )
	{
		if ( m_pFlashlightEffect )
		{
			ParticleProp()->StopEmissionAndDestroyImmediately( m_pFlashlightEffect );
			m_pFlashlightEffect = NULL;
		}

		if ( m_pELight )
		{
			m_pELight->die = gpGlobals->curtime;
			m_pELight = NULL;
		}
	}
	else
	{
		Vector vecOrigin;
		QAngle angles;
		int iAttachment = LookupAttachment( "eyes" );
		GetAttachment( iAttachment, vecOrigin, angles );

		Vector vForward;
		Vector vRight;
		Vector vUp;
		AngleVectors( angles, &vForward, &vRight, &vUp );

		trace_t tr;
		UTIL_TraceLine( vecOrigin, vecOrigin + vForward * 256.0f, ( MASK_SHOT & ~CONTENTS_HITBOX ), this, COLLISION_GROUP_NONE, &tr );

		if ( !m_pFlashlightEffect )
		{
			m_pFlashlightEffect = ParticleProp()->Create( "flashlight_thirdperson", PATTACH_POINT_FOLLOW, iAttachment );
			ParticleProp()->AddControlPoint( m_pFlashlightEffect, 1, this, PATTACH_CUSTOMORIGIN );
			ParticleProp()->AddControlPoint( m_pFlashlightEffect, 2, this, PATTACH_CUSTOMORIGIN );
			ParticleProp()->AddControlPoint( m_pFlashlightEffect, 3, C_BasePlayer::GetLocalPlayer(), PATTACH_EYES_FOLLOW );
		}

		if ( m_pFlashlightEffect )
		{
			m_pFlashlightEffect->SetControlPoint( 1, vecOrigin );
			m_pFlashlightEffect->SetControlPoint( 2, tr.endpos );
			m_pFlashlightEffect->SetControlPointForwardVector( 1, vForward );
		}

		if ( !m_pELight )
		{
			m_pELight = effects->CL_AllocElight( LIGHT_INDEX_TE_DYNAMIC + entindex() );
		}

		if ( m_pELight )
		{
			m_pELight->die = gpGlobals->curtime + 1e10;
			m_pELight->flags = DLIGHT_NO_WORLD_ILLUMINATION;
			m_pELight->radius = 64.0f;
			m_pELight->style = 0;
			m_pELight->m_pExclusiveLightReceiver = this;
			m_pELight->color.r = 225;
			m_pELight->color.g = 225;
			m_pELight->color.b = 255;
			m_pELight->color.exponent = 0;
			m_pELight->origin = vecOrigin + vForward * 2.0f;

			render->TouchLight( m_pELight );
		}
	}

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

IMPLEMENT_CLIENTCLASS_DT( C_NPC_Personality_Core, DT_NPC_Personality_Core, CNPC_PersonalityCore )
	RecvPropBool( RECVINFO( m_bFlashlightEnabled ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Material proxy that lights the core's mouth while it talks.
//-----------------------------------------------------------------------------
class CLightedMouthProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
};

bool CLightedMouthProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	return CResultProxy::Init( pMaterial, pKeyValues );
}

void CLightedMouthProxy::OnBind( void *pC_BaseEntity )
{
	if ( !pC_BaseEntity )
		return;

	C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );

	C_NPC_Personality_Core *core = dynamic_cast<C_NPC_Personality_Core *>( pEntity );
	if ( core )
	{
		float amt = core->GetMouthAmount();
		SetFloatResult( amt );
	}
	else
	{
		SetFloatResult( 1.0f );
	}
}

EXPOSE_MATERIAL_PROXY( CLightedMouthProxy, LightedMouth );
