//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device gyroscope on iOS (Core Motion); the CGyroSensor
// contract of gyro_sensor.h, whose Android backend is gyro_sensor.cpp.
//
// Core Motion's device-motion stream carries the bias-corrected rotation rate
// and the filtered gravity vector together, each sample stamped with its own
// time. Its handler runs on a private serial operation queue, which owns the
// integrator and the up filter; requests from the owner thread are applied on
// that queue too, so the owner never waits on it except when Shutdown drains
// it. Reading the gyroscope and accelerometer needs no permission prompt.
//
// Axes match Android's natural-orientation axes: x right and y up in portrait,
// z out of the screen, counter-clockwise positive. Core Motion's gravity
// points toward the ground, so the direction away from it is its negation.
//
//===========================================================================//

#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

#include <atomic>

// objc.h defines BOOL; basetypes.h must not redefine it.
#define DONT_DEFINE_BOOL
#include "gyro_sensor.h"
#include "gyro_math.h"
#include "tier0/dbg.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

namespace
{
// Core Motion timestamps are seconds since boot.
int64_t TimestampNs( NSTimeInterval flSeconds )
{
	return static_cast<int64_t>( flSeconds * 1e9 );
}
} // namespace

struct CGyroSensor::State
{
	CMMotionManager *m_pManager = nil;
	NSOperationQueue *m_pQueue = nil;

	// Requests from other threads, applied on the queue.
	std::atomic<bool> m_bSuspended;
	std::atomic<int> m_nPeriodUs;

	// Queue-owned: the period the stream runs at (0 = stopped).
	int m_nActivePeriodUs = 0;
	gyro::CRateIntegrator m_Integrator;
	// Core Motion's gravity is already filtered.
	gyro::CUpFilter m_UpFilter{ 0.0 };

	// As in gyro_sensor.cpp: totals written by the queue only, each axis read
	// whole; the owner thread keeps its read position.
	std::atomic<double> m_Total[3];
	double m_Consumed[3];
	std::atomic<uint32_t> m_nSamples;
	std::atomic<float> m_Up[3];
	std::atomic<bool> m_bHaveUp;

	State()
	{
		m_bSuspended.store( false );
		m_nPeriodUs.store( 0 );
		m_nSamples.store( 0 );
		m_bHaveUp.store( false );
		for ( int i = 0; i < 3; ++i )
		{
			m_Total[i].store( 0.0 );
			m_Consumed[i] = 0.0;
			m_Up[i].store( 0.f );
		}
	}

	// Queue only.
	void AddSample( CMDeviceMotion *pMotion )
	{
		const CMRotationRate rate = pMotion.rotationRate;
		const float rates[3] = { static_cast<float>( rate.x ), static_cast<float>( rate.y ),
			static_cast<float>( rate.z ) };
		const int64_t nTimestampNs = TimestampNs( pMotion.timestamp );
		double rotation[3] = { 0.0, 0.0, 0.0 };
		m_Integrator.AddSample( nTimestampNs, rates, rotation );
		for ( int i = 0; i < 3; ++i )
		{
			m_Total[i].store( m_Total[i].load( std::memory_order_relaxed ) + rotation[i],
			    std::memory_order_relaxed );
		}
		m_nSamples.fetch_add( 1, std::memory_order_relaxed );

		const CMAcceleration gravity = pMotion.gravity;
		const float away[3] = { static_cast<float>( -gravity.x ), static_cast<float>( -gravity.y ),
			static_cast<float>( -gravity.z ) };
		float up[3];
		m_UpFilter.AddSample( nTimestampNs, away, up );
		for ( int i = 0; i < 3; ++i )
			m_Up[i].store( up[i], std::memory_order_relaxed );
		m_bHaveUp.store( true );
	}

	// Queue only: starts, retimes or stops the stream to match the requests.
	void Apply()
	{
		const int nWantedUs = m_bSuspended.load() ? 0 : m_nPeriodUs.load();
		if ( nWantedUs == m_nActivePeriodUs )
			return;
		if ( m_nActivePeriodUs > 0 )
			[m_pManager stopDeviceMotionUpdates];
		m_nActivePeriodUs = 0;
		m_Integrator.Reset();
		m_UpFilter.Reset();
		m_bHaveUp.store( false );
		if ( nWantedUs <= 0 )
			return;

		// Core Motion raises an interval below the hardware minimum to it.
		m_pManager.deviceMotionUpdateInterval = nWantedUs * 1e-6;
		State *pState = this;
		[m_pManager startDeviceMotionUpdatesToQueue:m_pQueue
		                                withHandler:^( CMDeviceMotion *pMotion, NSError *pError ) {
			                                if ( pMotion && !pError )
				                                pState->AddSample( pMotion );
		                                }];
		m_nActivePeriodUs = nWantedUs;
	}

	// Any thread.
	void Request()
	{
		State *pState = this;
		[m_pQueue addOperationWithBlock:^{
			pState->Apply();
		}];
	}
};

CGyroSensor::CGyroSensor() : m_pState( nullptr )
{
}

CGyroSensor::~CGyroSensor()
{
	Shutdown();
}

bool CGyroSensor::Init()
{
	if ( m_pState )
		return true;

	CMMotionManager *pManager = [[CMMotionManager alloc] init];
	if ( !pManager.deviceMotionAvailable )
	{
		[pManager release];
		Msg( "Gyro: this device has no gyroscope\n" );
		return false;
	}

	State *pState = new State;
	pState->m_pManager = pManager;
	pState->m_pQueue = [[NSOperationQueue alloc] init];
	pState->m_pQueue.maxConcurrentOperationCount = 1;
	pState->m_pQueue.name = @"GyroSensor";
	pState->m_pQueue.qualityOfService = NSQualityOfServiceUserInteractive;
	m_pState = pState;

	Msg( "Gyro: Core Motion device motion; up from gravity\n" );
	return true;
}

// Callers must stop calling SetSuspended (remove its event watch) first.
void CGyroSensor::Shutdown()
{
	if ( !m_pState )
		return;

	State *pState = m_pState;
	[pState->m_pQueue addOperationWithBlock:^{
		if ( pState->m_nActivePeriodUs > 0 )
			[pState->m_pManager stopDeviceMotionUpdates];
		pState->m_nActivePeriodUs = 0;
	}];
	// Drains the stop and any handler already queued before the stop.
	[pState->m_pQueue waitUntilAllOperationsAreFinished];
	[pState->m_pQueue release];
	[pState->m_pManager release];

	delete pState;
	m_pState = nullptr;
}

bool CGyroSensor::IsAvailable() const
{
	return m_pState != nullptr;
}

void CGyroSensor::SetSamplePeriod( int nPeriodUs )
{
	if ( !m_pState || m_pState->m_nPeriodUs.exchange( nPeriodUs ) == nPeriodUs )
		return;
	m_pState->Request();
}

void CGyroSensor::SetSuspended( bool bSuspended )
{
	if ( !m_pState || m_pState->m_bSuspended.exchange( bSuspended ) == bSuspended )
		return;
	m_pState->Request();
}

void CGyroSensor::ConsumeRotation( float rotation[3] )
{
	for ( int i = 0; i < 3; ++i )
	{
		rotation[i] = 0.f;
		if ( !m_pState )
			continue;
		double flTotal = m_pState->m_Total[i].load( std::memory_order_relaxed );
		rotation[i] = static_cast<float>( flTotal - m_pState->m_Consumed[i] );
		m_pState->m_Consumed[i] = flTotal;
	}
}

bool CGyroSensor::GetUp( float up[3] ) const
{
	up[0] = up[1] = up[2] = 0.f;
	if ( !m_pState || !m_pState->m_bHaveUp.load() )
		return false;
	for ( int i = 0; i < 3; ++i )
		up[i] = m_pState->m_Up[i].load( std::memory_order_relaxed );
	return true;
}

uint32_t CGyroSensor::SampleCount() const
{
	return m_pState ? m_pState->m_nSamples.load( std::memory_order_relaxed ) : 0;
}
