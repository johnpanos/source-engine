//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Turns the per-frame two-motor rumble request into commands for a
//          single amplitude-controlled vibrator (a phone's haptic actuator).
//
//===========================================================================//

#include "vibrator_policy.h"

#include <math.h>
#include <stdlib.h>

namespace vibrator
{

static float MotorValue( float flMotor )
{
	// NaN fails the comparison; infinity is not a motor speed either.
	return ( flMotor > 0.f && isfinite( flMotor ) ) ? flMotor : 0.f;
}

float MixMotors( float flLeft, float flRight )
{
	float flL = MotorValue( flLeft );
	float flR = MotorValue( flRight );
	float flEnergy = sqrtf( flL * flL + flR * flR );
	return flEnergy < 1.f ? flEnergy : 1.f;
}

int AmplitudeLevel( float flIntensity )
{
	if ( !( flIntensity > 0.f ) )
		return 0;
	if ( flIntensity >= 1.f )
		return kMaxAmplitude;
	return (int)( flIntensity * kMaxAmplitude + 0.5f );
}

Timing DefaultTiming()
{
	Timing timing;
	timing.m_nLeaseMs = 1000;
	timing.m_nRenewMarginMs = 250;
	timing.m_nMinIntervalMs = 20;
	timing.m_nMinStep = 8;
	timing.m_nSettleMs = 100;
	return timing;
}

CRumbleStream::CRumbleStream( const Timing &timing ) : m_Timing( timing )
{
	Reset();
}

void CRumbleStream::Reset()
{
	m_bPlaying = false;
	m_nLevel = 0;
	m_nSentMs = 0;
	m_nLeaseEndMs = 0;
	m_bDiverged = false;
	m_nDivergedMs = 0;
}

Command CRumbleStream::Play( int nLevel, int64_t nNowMs )
{
	m_bPlaying = true;
	m_nLevel = nLevel;
	m_nSentMs = nNowMs;
	m_nLeaseEndMs = nNowMs + m_Timing.m_nLeaseMs;
	m_bDiverged = false;

	Command command = { Command::PLAY, nLevel, m_Timing.m_nLeaseMs };
	return command;
}

Command CRumbleStream::Update( float flLeft, float flRight, int64_t nNowMs )
{
	const Command none = { Command::NONE, 0, 0 };
	int nLevel = AmplitudeLevel( MixMotors( flLeft, flRight ) );

	// The one-shot has run out (the caller stalled): nothing is playing.
	if ( m_bPlaying && nNowMs >= m_nLeaseEndMs )
		m_bPlaying = false;

	if ( nLevel == 0 )
	{
		if ( !m_bPlaying )
			return none;
		m_bPlaying = false;
		const Command stop = { Command::STOP, 0, 0 };
		return stop;
	}

	// Start, renew before the lease runs out, and restart if the clock went
	// backwards (the lease end can no longer be trusted).
	if ( !m_bPlaying || nNowMs < m_nSentMs || m_nLeaseEndMs - nNowMs <= m_Timing.m_nRenewMarginMs )
		return Play( nLevel, nNowMs );

	if ( nLevel == m_nLevel )
	{
		m_bDiverged = false;
		return none;
	}
	if ( !m_bDiverged )
	{
		m_bDiverged = true;
		m_nDivergedMs = nNowMs;
	}

	if ( nNowMs - m_nSentMs < m_Timing.m_nMinIntervalMs )
		return none;
	if ( abs( nLevel - m_nLevel ) >= m_Timing.m_nMinStep ||
	     nNowMs - m_nDivergedMs >= m_Timing.m_nSettleMs )
		return Play( nLevel, nNowMs );
	return none;
}

} // namespace vibrator
