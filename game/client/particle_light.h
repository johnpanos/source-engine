//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The dlight a lit burst of sparks carries (render/spark_light.h
//          policy). One owner for both kinds of spark: old-style trail sparks
//          (fx_sparks.cpp) and particle systems drawn with a spark material
//          (particles_new.cpp). Host (main) thread only: the dlight table is
//          engine state.
//
//=============================================================================//
#ifndef PARTICLE_LIGHT_H
#define PARTICLE_LIGHT_H
#ifdef _WIN32
#pragma once
#endif

#include "render/spark_light.h"

// A burst's light at full strength and white color.
struct SparkLightParams_t
{
	int m_Color[3];  // ColorRGBExp32 mantissas, 0..255 (linear)
	int m_nExponent; // ColorRGBExp32 exponent
	float m_flRadius;
};

// The warm white of the spark textures' bright texels at the chosen strength.
SparkLightParams_t SparkLightParams( int nExponent, float flRadius );

class CParticleDynamicLight
{
public:
	CParticleDynamicLight();
	~CParticleDynamicLight();

	// Gives the burst a light of its own (a key) at these parameters.
	void Configure( const SparkLightParams_t &params );
	bool IsConfigured() const { return m_nKey != 0; }

	// Lights the burst this frame, or releases its light when it is dark. The
	// first light takes a place in the fx_spark_lights budget and a free dlight
	// slot, and is skipped when neither is free.
	void Commit( const SparkLight::Light_t &light );
	void Release();

private:
	SparkLightParams_t m_Params;
	int m_nKey;   // 0: no light
	bool m_bHeld; // holds a place in the budget and a dlight
};

#endif // PARTICLE_LIGHT_H
