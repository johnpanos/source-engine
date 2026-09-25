//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The dlight a lit burst of sparks carries (particle_light.h).
//
//=============================================================================//
#include "cbase.h"
#include "particle_light.h"
#include "dlight.h"
#include "iefx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar fx_spark_lights(
    "fx_spark_lights", "4", 0, "Most spark bursts that light their surroundings at once (0: none)." );

// The warm white of the spark textures' bright texels (255, 222, 170), as
// linear light mantissas at 30% strength.
static const int kSparkLightColor[3] = { 77, 67, 51 };

// A lit burst's dlight outlives the frame by this much, so the engine keeps its
// slot (and gives it to no other light) until the burst's next commit.
static const float kSparkLightHold = 0.1f;

static SparkLight::CBudget s_Budget;
static int s_nSerial = 0;

SparkLightParams_t SparkLightParams( int nExponent, float flRadius )
{
	SparkLightParams_t params = {
		{ kSparkLightColor[0], kSparkLightColor[1], kSparkLightColor[2] }, nExponent, flRadius };
	return params;
}

CParticleDynamicLight::CParticleDynamicLight() : m_nKey( 0 ), m_bHeld( false )
{
	m_Params = SparkLightParams( 0, 0.0f );
}

CParticleDynamicLight::~CParticleDynamicLight()
{
	Release();
}

void CParticleDynamicLight::Configure( const SparkLightParams_t &params )
{
	m_Params = params;
	if ( !m_nKey )
	{
		s_nSerial = ( s_nSerial + 1 ) & ( LIGHT_INDEX_SPARK - 1 );
		m_nKey = LIGHT_INDEX_SPARK + s_nSerial;
	}
}

void CParticleDynamicLight::Commit( const SparkLight::Light_t &light )
{
	if ( !m_nKey )
		return;
	if ( !light.m_bLit )
	{
		Release();
		return;
	}
	if ( !m_bHeld )
	{
		// Never take a slot another light holds: with every slot active,
		// CL_AllocDlight would overwrite the first.
		dlight_t *pActive[MAX_DLIGHTS];
		if ( effects->CL_GetActiveDLights( pActive ) >= MAX_DLIGHTS ||
		     !s_Budget.Acquire( fx_spark_lights.GetInt() ) )
			return;
		m_bHeld = true;
	}

	const float flScale = clamp( light.m_flScale, 0.0f, 1.0f );
	dlight_t *dl = effects->CL_AllocDlight( m_nKey );
	dl->origin.Init( light.m_Origin[0], light.m_Origin[1], light.m_Origin[2] );
	dl->color.r = (byte)( m_Params.m_Color[0] * light.m_Color[0] * flScale + 0.5f );
	dl->color.g = (byte)( m_Params.m_Color[1] * light.m_Color[1] * flScale + 0.5f );
	dl->color.b = (byte)( m_Params.m_Color[2] * light.m_Color[2] * flScale + 0.5f );
	dl->color.exponent = m_Params.m_nExponent;
	dl->radius = m_Params.m_flRadius;
	dl->die = gpGlobals->curtime + kSparkLightHold;
}

void CParticleDynamicLight::Release()
{
	if ( !m_bHeld )
		return;
	m_bHeld = false;
	s_Budget.Release();

	// Darken the dlight now and let the engine's next decay drop it, rather
	// than keep it lit for the rest of its hold.
	dlight_t *pActive[MAX_DLIGHTS];
	const int nActive = effects->CL_GetActiveDLights( pActive );
	for ( int i = 0; i < nActive; ++i )
	{
		if ( pActive[i]->key == m_nKey )
		{
			pActive[i]->color.r = pActive[i]->color.g = pActive[i]->color.b = 0;
			pActive[i]->die = 0.0f;
			break;
		}
	}
}
