//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The dlight a lit burst of sparks carries (render/spark_light.h
//          policy). One owner for both kinds of spark: old-style trail sparks
//          (fx_sparks.cpp) and particle systems drawn with a spark material
//          (particles_new.cpp). Host (main) thread only: the dlight table is
//          engine state.
//
//          Bursts commit their light while the particles simulate; the frame's
//          dlights are written once, after every effect has simulated
//          (SparkLights_Resolve), for the fx_spark_lights bursts most
//          important at the viewer.
//
//=============================================================================//
#ifndef PARTICLE_LIGHT_H
#define PARTICLE_LIGHT_H
#ifdef _WIN32
#pragma once
#endif

#include "render/spark_light.h"
#include "tier1/refcount.h"

// A burst's light at full strength and white color.
struct SparkLightParams_t
{
	int m_Color[3];  // ColorRGBExp32 mantissas, 0..255 (linear)
	int m_nExponent; // ColorRGBExp32 exponent
	float m_flRadius; // base radius; the light reaches further as its sparks spread
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
	int Key() const { return m_nKey; }

	// The burst's light this frame (the last commit before the resolve wins).
	// A dark light asks for none.
	void Commit( const SparkLight::Light_t &light );
	void Release();

private:
	friend void SparkLights_Resolve();

	SparkLightParams_t m_Params;
	SparkLight::Light_t m_Light; // committed this frame
	int m_nKey;                  // 0: no light
	bool m_bHeld;                // holds a dlight
	CParticleDynamicLight *m_pNext; // every configured light
	CParticleDynamicLight *m_pPrev;
};

// One burst's light shared by every emitter of the burst (the big and the
// little sparks of FX_ElectricSpark, say). The first emitter to add a spark in
// a frame starts the burst's frame.
class CSparkBurstLight : public CRefCounted<>
{
public:
	explicit CSparkBurstLight( const SparkLightParams_t &params );

	// Starts the burst at origin at full strength, before its first simulate.
	void Start( const float origin[3] );
	void Add( const float pos[3], float flEmission );
	// Commits the sparks added this frame so far.
	void Commit();
	int Key() const { return m_Light.Key(); }

private:
	SparkLight::CBurst m_Burst;
	CParticleDynamicLight m_Light;
	int m_nFrame;
};

// Lights this frame's bursts: the fx_spark_lights most important at the main
// view hold a dlight, the rest release theirs. Runs once per frame, on the
// host, after every particle effect has simulated.
void SparkLights_Resolve();

// fx_spark_lights_debug 2: logs a live spark ("sparkdbg spark") for the spark
// light fixtures (tools/quality/spark_light_scene.py). nKey 0: a spark no
// burst light carries.
bool SparkLights_LogSparks();
void SparkLights_LogSpark( int nKey, const float pos[3], float flEmission );

#endif // PARTICLE_LIGHT_H
