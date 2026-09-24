//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device gyroscope, sampled on its own thread (Android NDK).
//
// SDL3's Android sensor backend fixes the rate at 60 Hz, stamps samples with
// the poll time rather than the sensor time, and delivers them as SDL events
// whose watchers run under SDL's global watcher lock. Gyro aiming wants a rate
// tied to the display and exact sample times, so this reads the NDK sensor
// queue directly. The same queue carries the gravity sensor (or, on devices
// without one, the accelerometer) that player-space turning needs.
//
//===========================================================================//

#include "gyro_sensor.h"
#include "gyro_math.h"
#include "tier0/dbg.h"

// Only the SDL3 Android profile links the NDK's libandroid (inputsystem/wscript).
#if defined( ANDROID ) && defined( USE_SDL3 )
#include <android/looper.h>
#include <android/sensor.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <atomic>
#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#if defined( ANDROID ) && defined( USE_SDL3 )

namespace
{
const int kLooperIdent = 1;
const int kEventBatch = 16;
// Which way is up changes slowly; 100 Hz is ample for player-space turning.
const int kUpPeriodUs = 10000;
// Low-pass time constant for the raw accelerometer, which also measures the
// hand's own acceleration. The gravity sensor is already filtered.
const double kAccelerometerUpSeconds = 0.15;

// ASensorManager is per package; an app process is named after its package.
void GetPackageName( char *pName, size_t nSize )
{
	pName[0] = '\0';
	FILE *pFile = fopen( "/proc/self/cmdline", "rb" );
	if ( !pFile )
		return;
	size_t nRead = fread( pName, 1, nSize - 1, pFile );
	fclose( pFile );
	pName[nRead] = '\0';
	if ( char *pProcessSuffix = strchr( pName, ':' ) )
		*pProcessSuffix = '\0';
}
} // namespace

struct CGyroSensor::State
{
	ASensorManager *m_pManager = nullptr;
	const ASensor *m_pSensor = nullptr;
	int m_nMinDelayUs = 0;
	// Gravity (or accelerometer) sensor; null when the device has neither.
	const ASensor *m_pUpSensor = nullptr;
	int m_nUpPeriodUs = 0;
	double m_flUpTimeConstant = 0.0;

	pthread_t m_Thread;
	bool m_bThreadStarted = false;

	// Requests from other threads, applied by the sensor thread.
	std::atomic<bool> m_bRunning;
	std::atomic<bool> m_bSuspended;
	std::atomic<int> m_nPeriodUs;
	// The sensor thread's looper, published once it exists. Holds a reference
	// that Shutdown releases after the join, so a late wake is still valid.
	std::atomic<ALooper *> m_pLooper;

	// Integrated rotation in radians. Written by the sensor thread only; each
	// axis is read whole, and a read that splits a sample between axes is
	// completed by the next read, since the totals only accumulate.
	std::atomic<double> m_Total[3];
	// The owner thread's read position.
	double m_Consumed[3];
	std::atomic<uint32_t> m_nSamples;
	// The latest filtered up direction; m_bHaveUp once it is valid. Axes may
	// come from neighbouring samples, which differ by a tiny rotation.
	std::atomic<float> m_Up[3];
	std::atomic<bool> m_bHaveUp;

	State()
	{
		m_bRunning.store( false );
		m_bSuspended.store( false );
		m_nPeriodUs.store( 0 );
		m_pLooper.store( nullptr );
		m_nSamples.store( 0 );
		m_bHaveUp.store( false );
		for ( int i = 0; i < 3; ++i )
		{
			m_Total[i].store( 0.0 );
			m_Consumed[i] = 0.0;
			m_Up[i].store( 0.f );
		}
	}

	void Wake()
	{
		ALooper *pLooper = m_pLooper.load();
		if ( pLooper )
			ALooper_wake( pLooper );
	}

	static void *ThreadMain( void *pState )
	{
		static_cast<State *>( pState )->Run();
		return nullptr;
	}

	void Run();
};

void CGyroSensor::State::Run()
{
	pthread_setname_np( pthread_self(), "GyroSensor" );

	// The queue is polled by ident, without a callback, which a looper accepts
	// only when prepared for it; otherwise the queue is silently left out of
	// the poll and no sample is ever read.
	ALooper *pLooper = ALooper_prepare( ALOOPER_PREPARE_ALLOW_NON_CALLBACKS );
	ALooper_acquire( pLooper );
	ASensorEventQueue *pQueue =
	    ASensorManager_createEventQueue( m_pManager, pLooper, kLooperIdent, nullptr, nullptr );
	// Publish after the queue exists; requests stored before this are read below.
	m_pLooper.store( pLooper );
	if ( !pQueue )
		Warning( "Gyro: could not create a sensor event queue\n" );

	int nActivePeriodUs = 0;
	bool bUpActive = false;
	gyro::CRateIntegrator integrator;
	gyro::CUpFilter upFilter( m_flUpTimeConstant );
	ASensorEvent events[kEventBatch];

	while ( m_bRunning.load() )
	{
		int nWantedUs = ( m_bSuspended.load() || !pQueue ) ? 0 : m_nPeriodUs.load();
		if ( nWantedUs > 0 && nWantedUs < m_nMinDelayUs )
			nWantedUs = m_nMinDelayUs;
		if ( nWantedUs != nActivePeriodUs )
		{
			if ( nActivePeriodUs > 0 )
				ASensorEventQueue_disableSensor( pQueue, m_pSensor );
			if ( bUpActive )
				ASensorEventQueue_disableSensor( pQueue, m_pUpSensor );
			nActivePeriodUs = 0;
			bUpActive = false;
			integrator.Reset();
			upFilter.Reset();
			m_bHaveUp.store( false );
			if ( nWantedUs > 0 )
			{
				// No batching: each sample is reported as soon as it is taken.
				if ( ASensorEventQueue_registerSensor( pQueue, m_pSensor, nWantedUs, 0 ) == 0 )
					nActivePeriodUs = nWantedUs;
				else
					Warning( "Gyro: could not enable the gyroscope at %d us\n", nWantedUs );
				// Without it, turning falls back to the device's own axes.
				if ( m_pUpSensor && ASensorEventQueue_registerSensor(
				                        pQueue, m_pUpSensor, m_nUpPeriodUs, 0 ) == 0 )
					bUpActive = true;
			}
		}

		// Sleeps until a sample arrives or a request wakes the looper.
		ALooper_pollOnce( -1, nullptr, nullptr, nullptr );
		if ( !pQueue )
			continue;

		ssize_t nEvents;
		while ( ( nEvents = ASensorEventQueue_getEvents( pQueue, events, kEventBatch ) ) > 0 )
		{
			double rotation[3] = { 0.0, 0.0, 0.0 };
			uint32_t nGyroSamples = 0;
			bool bUpChanged = false;
			float up[3];
			for ( ssize_t e = 0; e < nEvents; ++e )
			{
				// Timed by the sensor's own timestamps, not by arrival.
				if ( events[e].type == ASENSOR_TYPE_GYROSCOPE )
				{
					integrator.AddSample( events[e].timestamp, events[e].data, rotation );
					++nGyroSamples;
				}
				else if ( m_pUpSensor && events[e].type == ASensor_getType( m_pUpSensor ) )
				{
					upFilter.AddSample( events[e].timestamp, events[e].data, up );
					bUpChanged = true;
				}
			}
			for ( int i = 0; i < 3; ++i )
			{
				m_Total[i].store( m_Total[i].load( std::memory_order_relaxed ) + rotation[i],
				    std::memory_order_relaxed );
			}
			m_nSamples.fetch_add( nGyroSamples, std::memory_order_relaxed );
			if ( bUpChanged )
			{
				for ( int i = 0; i < 3; ++i )
					m_Up[i].store( up[i], std::memory_order_relaxed );
				m_bHaveUp.store( true );
			}
		}
	}

	if ( pQueue )
	{
		if ( nActivePeriodUs > 0 )
			ASensorEventQueue_disableSensor( pQueue, m_pSensor );
		if ( bUpActive )
			ASensorEventQueue_disableSensor( pQueue, m_pUpSensor );
		ASensorManager_destroyEventQueue( m_pManager, pQueue );
	}
}

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

	char szPackage[256];
	GetPackageName( szPackage, sizeof( szPackage ) );
	ASensorManager *pManager = ASensorManager_getInstanceForPackage( szPackage );
	const ASensor *pSensor =
	    pManager ? ASensorManager_getDefaultSensor( pManager, ASENSOR_TYPE_GYROSCOPE ) : nullptr;
	if ( !pSensor )
	{
		Msg( "Gyro: this device has no gyroscope\n" );
		return false;
	}

	State *pState = new State;
	pState->m_pManager = pManager;
	pState->m_pSensor = pSensor;
	pState->m_nMinDelayUs = ASensor_getMinDelay( pSensor );
	pState->m_pUpSensor = ASensorManager_getDefaultSensor( pManager, ASENSOR_TYPE_GRAVITY );
	if ( !pState->m_pUpSensor )
	{
		pState->m_pUpSensor =
		    ASensorManager_getDefaultSensor( pManager, ASENSOR_TYPE_ACCELEROMETER );
		pState->m_flUpTimeConstant = kAccelerometerUpSeconds;
	}
	if ( pState->m_pUpSensor )
	{
		const int nUpMinDelayUs = ASensor_getMinDelay( pState->m_pUpSensor );
		pState->m_nUpPeriodUs = nUpMinDelayUs > kUpPeriodUs ? nUpMinDelayUs : kUpPeriodUs;
	}
	pState->m_bRunning.store( true );
	if ( pthread_create( &pState->m_Thread, nullptr, &State::ThreadMain, pState ) != 0 )
	{
		Warning( "Gyro: could not start the sensor thread\n" );
		delete pState;
		return false;
	}
	pState->m_bThreadStarted = true;
	m_pState = pState;

	Msg( "Gyro: %s (%s), fastest sample period %d us; up from %s\n", ASensor_getName( pSensor ),
	    ASensor_getVendor( pSensor ), pState->m_nMinDelayUs,
	    pState->m_pUpSensor ? ASensor_getName( pState->m_pUpSensor ) : "nothing" );
	return true;
}

// Callers must stop calling SetSuspended (remove its event watch) first.
void CGyroSensor::Shutdown()
{
	if ( !m_pState )
		return;

	m_pState->m_bRunning.store( false );
	m_pState->Wake();
	if ( m_pState->m_bThreadStarted )
		pthread_join( m_pState->m_Thread, nullptr );
	if ( ALooper *pLooper = m_pState->m_pLooper.exchange( nullptr ) )
		ALooper_release( pLooper );

	delete m_pState;
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
	m_pState->Wake();
}

void CGyroSensor::SetSuspended( bool bSuspended )
{
	if ( !m_pState || m_pState->m_bSuspended.exchange( bSuspended ) == bSuspended )
		return;
	m_pState->Wake();
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

#else // !( ANDROID && USE_SDL3 )

// No gyroscope backend on this platform.
struct CGyroSensor::State
{
};

CGyroSensor::CGyroSensor() : m_pState( nullptr )
{
}

CGyroSensor::~CGyroSensor()
{
}

bool CGyroSensor::Init()
{
	return false;
}

void CGyroSensor::Shutdown()
{
}

bool CGyroSensor::IsAvailable() const
{
	return false;
}

void CGyroSensor::SetSamplePeriod( int )
{
}

void CGyroSensor::SetSuspended( bool )
{
}

void CGyroSensor::ConsumeRotation( float rotation[3] )
{
	rotation[0] = rotation[1] = rotation[2] = 0.f;
}

bool CGyroSensor::GetUp( float up[3] ) const
{
	up[0] = up[1] = up[2] = 0.f;
	return false;
}

uint32_t CGyroSensor::SampleCount() const
{
	return 0;
}

#endif // ANDROID && USE_SDL3
