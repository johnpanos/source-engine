//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The dlight a lit burst of sparks carries (particle_light.h).
//
//=============================================================================//
#include "cbase.h"
#include "particle_light.h"
#include "dlight.h"
#include "iefx.h"
#include "view.h"
#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar fx_spark_lights( "fx_spark_lights", "4", 0,
    "Most spark bursts that light their surroundings at once, the most important at the view "
    "(0: none)." );
static ConVar fx_spark_lights_debug( "fx_spark_lights_debug", "0", FCVAR_CHEAT,
    "1: draw each spark burst's light (green lit, red asking); 2: also log every live spark and "
    "burst light for the spark light fixtures." );

// The warm white of the spark textures' bright texels (255, 222, 170), as
// linear light mantissas at 30% strength.
static const int kSparkLightColor[3] = { 77, 67, 51 };

// A lit burst's dlight outlives the frame by this much, so the engine keeps its
// slot (and gives it to no other light) until the burst's next resolve.
static const float kSparkLightHold = 0.1f;

// Most bursts one resolve considers; more ask for light only in a crowd of
// bursts, and the rest wait for a later frame.
static const int kMaxCandidates = 64;

static int s_nSerial = 0;
static CParticleDynamicLight *s_pLights = NULL; // every configured light

SparkLightParams_t SparkLightParams( int nExponent, float flRadius )
{
	SparkLightParams_t params = {
	    { kSparkLightColor[0], kSparkLightColor[1], kSparkLightColor[2] }, nExponent, flRadius };
	return params;
}

static SparkLight::Light_t DarkLight()
{
	SparkLight::Light_t light = {};
	return light;
}

// Darkens a burst's dlight now and lets the engine's next decay drop it,
// rather than keep it lit for the rest of its hold.
static void DarkenDlight( int nKey )
{
	dlight_t *pActive[MAX_DLIGHTS];
	const int nActive = effects->CL_GetActiveDLights( pActive );
	for ( int i = 0; i < nActive; ++i )
	{
		if ( pActive[i]->key == nKey )
		{
			pActive[i]->color.r = pActive[i]->color.g = pActive[i]->color.b = 0;
			pActive[i]->die = 0.0f;
			break;
		}
	}
}

CParticleDynamicLight::CParticleDynamicLight()
    : m_nKey( 0 ), m_bHeld( false ), m_pNext( NULL ), m_pPrev( NULL )
{
	m_Params = SparkLightParams( 0, 0.0f );
	m_Light = DarkLight();
}

CParticleDynamicLight::~CParticleDynamicLight()
{
	if ( !m_nKey )
		return;
	if ( m_bHeld )
		DarkenDlight( m_nKey );
	if ( m_pPrev )
		m_pPrev->m_pNext = m_pNext;
	else
		s_pLights = m_pNext;
	if ( m_pNext )
		m_pNext->m_pPrev = m_pPrev;
}

void CParticleDynamicLight::Configure( const SparkLightParams_t &params )
{
	m_Params = params;
	if ( m_nKey )
		return;
	s_nSerial = ( s_nSerial + 1 ) & ( LIGHT_INDEX_SPARK - 1 );
	m_nKey = LIGHT_INDEX_SPARK + s_nSerial;
	m_pPrev = NULL;
	m_pNext = s_pLights;
	if ( s_pLights )
		s_pLights->m_pPrev = this;
	s_pLights = this;
}

void CParticleDynamicLight::Commit( const SparkLight::Light_t &light )
{
	m_Light = light;
}

void CParticleDynamicLight::Release()
{
	m_Light = DarkLight();
}

CSparkBurstLight::CSparkBurstLight( const SparkLightParams_t &params ) : m_nFrame( -1 )
{
	m_Light.Configure( params );
}

void CSparkBurstLight::Start( const float origin[3] )
{
	// The first simulate is a frame away; light the burst where it starts.
	SparkLight::Light_t light = { true, { origin[0], origin[1], origin[2] }, { 1, 1, 1 }, 1.0f, 0.0f };
	m_Light.Commit( light );
}

void CSparkBurstLight::Add( const float pos[3], float flEmission )
{
	if ( m_nFrame != gpGlobals->framecount )
	{
		m_nFrame = gpGlobals->framecount;
		m_Burst.BeginFrame();
	}
	m_Burst.Add( pos, flEmission );
}

void CSparkBurstLight::Commit()
{
	if ( m_nFrame != gpGlobals->framecount )
	{
		// No emitter of the burst has added a spark this frame.
		m_nFrame = gpGlobals->framecount;
		m_Burst.BeginFrame();
	}
	m_Light.Commit( m_Burst.Current() );
}

bool SparkLights_LogSparks()
{
	return fx_spark_lights_debug.GetInt() >= 2;
}

void SparkLights_LogSpark( int nKey, const float pos[3], float flEmission )
{
	Msg( "sparkdbg spark %d %d %.2f %.2f %.2f %.4f\n", gpGlobals->framecount, nKey, pos[0], pos[1],
	    pos[2], flEmission );
}

void SparkLights_Resolve()
{
	CParticleDynamicLight *pLights[kMaxCandidates];
	SparkLight::Candidate_t candidates[kMaxCandidates];
	int nCandidates = 0;
	for ( CParticleDynamicLight *pLight = s_pLights; pLight; pLight = pLight->m_pNext )
	{
		const SparkLight::Light_t &light = pLight->m_Light;
		if ( !light.m_bLit || nCandidates == kMaxCandidates )
		{
			if ( pLight->m_bHeld )
			{
				pLight->m_bHeld = false;
				DarkenDlight( pLight->Key() );
			}
			continue;
		}
		SparkLight::Candidate_t &candidate = candidates[nCandidates];
		for ( int k = 0; k < 3; ++k )
			candidate.m_Origin[k] = light.m_Origin[k];
		candidate.m_flRadius = SparkLight::LightRadius( pLight->m_Params.m_flRadius, light.m_flSpread );
		const float flScale = clamp( light.m_flScale, 0.0f, 1.0f );
		float flBrightest = 0.0f;
		for ( int k = 0; k < 3; ++k )
			flBrightest = MAX( flBrightest, pLight->m_Params.m_Color[k] * light.m_Color[k] );
		candidate.m_flStrength = flScale * TexLightToLinear( (int)flBrightest, pLight->m_Params.m_nExponent );
		candidate.m_bWasLit = pLight->m_bHeld;
		pLights[nCandidates++] = pLight;
	}

	const Vector &vecView = MainViewOrigin();
	const float view[3] = { vecView.x, vecView.y, vecView.z };
	bool bLit[kMaxCandidates];
	SparkLight::SelectLit( candidates, nCandidates, fx_spark_lights.GetInt(), view, bLit );

	const int nDebug = fx_spark_lights_debug.GetInt();
	if ( nDebug >= 2 )
		Msg( "sparkdbg view %d %.2f %.2f %.2f\n", gpGlobals->framecount, view[0], view[1], view[2] );
	for ( int i = 0; i < nCandidates; ++i )
	{
		CParticleDynamicLight *pLight = pLights[i];
		const SparkLight::Candidate_t &candidate = candidates[i];
		bool bHolds = bLit[i];
		if ( bHolds && !pLight->m_bHeld )
		{
			// Never take a slot another light holds: with every slot active,
			// CL_AllocDlight would overwrite the first.
			dlight_t *pActive[MAX_DLIGHTS];
			bHolds = effects->CL_GetActiveDLights( pActive ) < MAX_DLIGHTS;
		}
		if ( !bHolds )
		{
			if ( pLight->m_bHeld )
			{
				pLight->m_bHeld = false;
				DarkenDlight( pLight->Key() );
			}
		}
		else
		{
			pLight->m_bHeld = true;
			const SparkLight::Light_t &light = pLight->m_Light;
			const SparkLightParams_t &params = pLight->m_Params;
			const float flScale = clamp( light.m_flScale, 0.0f, 1.0f );
			dlight_t *dl = effects->CL_AllocDlight( pLight->Key() );
			dl->origin.Init( light.m_Origin[0], light.m_Origin[1], light.m_Origin[2] );
			dl->color.r = (byte)( params.m_Color[0] * light.m_Color[0] * flScale + 0.5f );
			dl->color.g = (byte)( params.m_Color[1] * light.m_Color[1] * flScale + 0.5f );
			dl->color.b = (byte)( params.m_Color[2] * light.m_Color[2] * flScale + 0.5f );
			dl->color.exponent = params.m_nExponent;
			dl->radius = candidate.m_flRadius;
			dl->die = gpGlobals->curtime + kSparkLightHold;
		}

		if ( nDebug >= 1 )
		{
			const Vector vecOrigin( candidate.m_Origin[0], candidate.m_Origin[1], candidate.m_Origin[2] );
			NDebugOverlay::Cross3D( vecOrigin, 8.0f, bHolds ? 0 : 255, bHolds ? 255 : 0, 0, true, 0.0f );
			if ( bHolds )
				NDebugOverlay::Sphere( vecOrigin, candidate.m_flRadius, 0, 160, 0, false, 0.0f );
		}
		if ( nDebug >= 2 )
		{
			Msg( "sparkdbg light %d %d %d %.2f %.2f %.2f %.2f %.5f %.2f\n", gpGlobals->framecount,
			    pLight->Key(), bHolds ? 1 : 0, candidate.m_Origin[0], candidate.m_Origin[1],
			    candidate.m_Origin[2], candidate.m_flRadius, candidate.m_flStrength,
			    pLight->m_Light.m_flSpread );
		}
	}
}
