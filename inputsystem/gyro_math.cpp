//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyroscope sample integration and screen-axis remapping.
//
//===========================================================================//

#include "gyro_math.h"

#include <math.h>

namespace gyro
{

CRateIntegrator::CRateIntegrator()
{
	Reset();
}

void CRateIntegrator::Reset()
{
	m_bHavePrevious = false;
	m_nPreviousTimestampNs = 0;
	m_flPreviousRate[0] = m_flPreviousRate[1] = m_flPreviousRate[2] = 0.f;
}

void CRateIntegrator::AddSample( int64_t nTimestampNs, const float rate[3], double rotation[3] )
{
	if ( m_bHavePrevious )
	{
		double flSeconds = ( nTimestampNs - m_nPreviousTimestampNs ) * 1e-9;
		if ( flSeconds > 0.0 && flSeconds <= kMaxSampleGapSeconds )
		{
			for ( int i = 0; i < 3; ++i )
				rotation[i] += 0.5 * ( m_flPreviousRate[i] + rate[i] ) * flSeconds;
		}
	}
	for ( int i = 0; i < 3; ++i )
		m_flPreviousRate[i] = rate[i];
	m_nPreviousTimestampNs = nTimestampNs;
	m_bHavePrevious = true;
}

CUpFilter::CUpFilter( double flTimeConstantSeconds ) : m_flTimeConstant( flTimeConstantSeconds )
{
	Reset();
}

void CUpFilter::Reset()
{
	m_bHavePrevious = false;
	m_nPreviousTimestampNs = 0;
	m_flUp[0] = m_flUp[1] = m_flUp[2] = 0.f;
}

void CUpFilter::AddSample( int64_t nTimestampNs, const float sample[3], float up[3] )
{
	double flWeight = 1.0;
	if ( m_bHavePrevious && m_flTimeConstant > 0.0 )
	{
		double flSeconds = ( nTimestampNs - m_nPreviousTimestampNs ) * 1e-9;
		if ( flSeconds <= 0.0 )
			flWeight = 0.0;
		else if ( flSeconds <= kMaxSampleGapSeconds )
			flWeight = 1.0 - exp( -flSeconds / m_flTimeConstant );
	}
	for ( int i = 0; i < 3; ++i )
	{
		m_flUp[i] += static_cast<float>( flWeight * ( sample[i] - m_flUp[i] ) );
		up[i] = m_flUp[i];
	}
	if ( !m_bHavePrevious || nTimestampNs > m_nPreviousTimestampNs )
		m_nPreviousTimestampNs = nTimestampNs;
	m_bHavePrevious = true;
}

int SamplePeriodUs( float flRefreshHz )
{
	const float kSamplesPerRefresh = 2.f;
	const float kFallbackRefreshHz = 60.f;
	if ( !( flRefreshHz >= 1.f ) )
		flRefreshHz = kFallbackRefreshHz;
	return static_cast<int>( 1000000.f / ( kSamplesPerRefresh * flRefreshHz ) + 0.5f );
}

int DisplayRotation( int nDegreesFromPortrait, bool bNaturalLandscape )
{
	// SDL adds 90 degrees for a landscape-natural display (most tablets).
	int nRotation = nDegreesFromPortrait + ( bNaturalLandscape ? 270 : 0 );
	return ( ( nRotation % 360 ) + 360 ) % 360;
}

void DeviceToScreen( const float device[3], int nDisplayRotation, float screen[3] )
{
	// The remap Android documents for Display.getRotation(): screen-right and
	// screen-up expressed in the device's natural axes.
	switch ( nDisplayRotation )
	{
	case 90:
		screen[0] = -device[1];
		screen[1] = device[0];
		break;
	case 180:
		screen[0] = -device[0];
		screen[1] = -device[1];
		break;
	case 270:
		screen[0] = device[1];
		screen[1] = -device[0];
		break;
	default:
		screen[0] = device[0];
		screen[1] = device[1];
		break;
	}
	screen[2] = device[2];
}

} // namespace gyro
