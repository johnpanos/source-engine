//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef SND_DMA_H
#define SND_DMA_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"

class ConVar;
class QAngle;
struct channel_t;

extern bool snd_initialized;

bool SND_IsInGame( void );

//-----------------------------------------------------------------------------
// Shared with the sound operator system (snd_op_sys) and the mixer
// (snd_mixgroups.cpp).
//-----------------------------------------------------------------------------
extern ConVar snd_showstart;
extern ConVar snd_obscured_gain_db;
extern ConVar snd_refdb;
extern ConVar snd_refdist;
extern int g_snd_trace_count;

// convert sound db level to approximate sound source radius,
// used only for determining how much of sound is obscured by world

#define SND_RADIUS_MAX		(20.0 * 12.0)	// max sound source radius
#define SND_RADIUS_MIN		(2.0 * 12.0)	// min sound source radius

#define SND_DB_MAX				140.0	// max db of any sound source
#define SND_DB_MED				90.0	// db at which compression curve changes
#define SND_DB_MIN				60.0	// min db of any sound source

inline float dB_To_Radius ( float db )
{
	return SND_RADIUS_MIN + (SND_RADIUS_MAX - SND_RADIUS_MIN) * (db - SND_DB_MIN) / (SND_DB_MAX - SND_DB_MIN);
}

#define MASK_BLOCK_AUDIO ( CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW )

#define SNDLVL_TO_DIST_MULT( sndlvl ) ( sndlvl ? ((pow( 10.0f, snd_refdb.GetFloat() / 20 ) / pow( 10.0f, (float)sndlvl / 20 )) / snd_refdist.GetFloat()) : 0 )
#define DIST_MULT_TO_SNDLVL( dist_mult ) (soundlevel_t)(int)( dist_mult ? ( 20 * log10( pow( 10.0f, snd_refdb.GetFloat() / 20 ) / (dist_mult * snd_refdist.GetFloat()) ) ) : 0 )

float SND_GetGainFromMult( float gain, float dist_mult, vec_t dist );
float SND_GetDspMix( channel_t *pchannel, int idist );
float SND_GetDspMix( channel_t *pchannel, int idist, float flSndlvl );
bool SND_ChannelOkToTrace( channel_t *ch );
float SND_GetFacingDirection( channel_t *pChannel, const QAngle &source_angles );
float SND_FadeToNewGain( channel_t *ch, float gain_new );
void ConvertListenerVectorTo2D( Vector *pvforward, Vector *pvright );
void ChannelSetVolTargets( channel_t *pch, int *pvolumes, int ivol_offset, int cvol );
void ChannelSetVolTargets( channel_t *pch, float *pvolumes, int ivol_offset, int cvol );
void ChannelUpdateVolXfade( channel_t *pch );
float dB_To_Gain( float dB );

// Engine side of a device speaker pan: fills per-speaker gains (0..1) for a
// sound in direction sourceDir (normalized, listener relative).
void Device_SpatializeChannel( int nSlot, float volumes[], const Vector &sourceDir, float mono, float flRearToStereoScale );

channel_t *S_FindChannelByGuid( int guid );
channel_t *S_FindChannelByScriptHash( unsigned int nHandle );
void S_StopChannel( channel_t *pChannel );
float S_GetElapsedTime( const channel_t *pChannel );
float S_SoundDuration( channel_t *pChannel );
float S_GetDashboarMusicMixValue();
const char *S_GetLevelNameShort();

#endif // SND_DMA_H
