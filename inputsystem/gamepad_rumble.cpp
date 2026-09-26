//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Turns the per-frame two-motor rumble request into commands for a
//          gamepad's rumble (SDL_RumbleGamepad).
//
//===========================================================================//

#include "gamepad_rumble.h"

#include <math.h>
#include <stdlib.h>

namespace gamepadrumble
{

int MotorLevel( float flMotor )
{
	// NaN fails the comparison; infinity is not a motor speed either.
	if ( !( flMotor > 0.f ) || !isfinite( flMotor ) )
		return 0;
	if ( flMotor >= 1.f )
		return kMaxLevel;
	return (int)( flMotor * kMaxLevel + 0.5f );
}

Timing DefaultTiming()
{
	Timing timing;
	timing.m_nLeaseMs = 1000;
	timing.m_nRenewMarginMs = 250;
	timing.m_nMinIntervalMs = 20;
	timing.m_nMinStep = 655;
	return timing;
}

CRumbleLease::CRumbleLease( const Timing &timing ) : m_Timing( timing )
{
	Reset();
}

void CRumbleLease::Reset()
{
	m_bPlaying = false;
	m_nLow = 0;
	m_nHigh = 0;
	m_nSentMs = 0;
	m_nLeaseEndMs = 0;
}

Command CRumbleLease::Play( int nLow, int nHigh, int64_t nNowMs )
{
	m_bPlaying = true;
	m_nLow = nLow;
	m_nHigh = nHigh;
	m_nSentMs = nNowMs;
	m_nLeaseEndMs = nNowMs + m_Timing.m_nLeaseMs;

	Command command = { Command::PLAY, nLow, nHigh, m_Timing.m_nLeaseMs };
	return command;
}

bool CRumbleLease::LevelChanged( int nFrom, int nTo ) const
{
	// Starting or stopping one motor is always a change.
	return ( nFrom == 0 ) != ( nTo == 0 ) || abs( nTo - nFrom ) >= m_Timing.m_nMinStep;
}

Command CRumbleLease::Update( float flLeft, float flRight, int64_t nNowMs )
{
	const Command none = { Command::NONE, 0, 0, 0 };
	const int nLow = MotorLevel( flLeft );
	const int nHigh = MotorLevel( flRight );

	if ( nLow == 0 && nHigh == 0 )
	{
		if ( !m_bPlaying )
			return none;
		Reset();
		const Command stop = { Command::STOP, 0, 0, 0 };
		return stop;
	}

	// A clock that went backwards leaves the lease end unknown.
	if ( !m_bPlaying || nNowMs < m_nSentMs )
		return Play( nLow, nHigh, nNowMs );

	const bool bChanged = LevelChanged( m_nLow, nLow ) || LevelChanged( m_nHigh, nHigh );
	if ( bChanged && nNowMs - m_nSentMs >= m_Timing.m_nMinIntervalMs )
		return Play( nLow, nHigh, nNowMs );

	// Renewal also applies any change that was too small or too soon to send.
	if ( m_nLeaseEndMs - nNowMs < m_Timing.m_nRenewMarginMs )
		return Play( nLow, nHigh, nNowMs );

	return none;
}

} // namespace gamepadrumble
