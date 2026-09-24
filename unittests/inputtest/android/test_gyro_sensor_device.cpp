//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: On-device test of the Android gyroscope thread
//          (inputsystem/gyro_sensor.cpp), run from adb shell by
//          tools/quality/android_gyro_device_test.sh. It needs a real device
//          with a gyroscope, not the game or the screen: samples must reach
//          the rotation totals at the requested rate, up must be measured,
//          and suspending, turning off and shutting down must stop the
//          sensor. A sensor thread whose queue never wakes it (the looper
//          bug fixed with ALOOPER_PREPARE_ALLOW_NON_CALLBACKS) fails the
//          sample checks. Lay the device still for the magnitude checks.
//
//=============================================================================//

#include "inputsystem/gyro_math.h"
#include "inputsystem/gyro_sensor.h"
#include "testing/conformance_result.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

// gyro_sensor.cpp reports through tier0 (extern "C" there); print instead of
// linking it.
extern "C" void Msg( const char *pMsg, ... )
{
	va_list args;
	va_start( args, pMsg );
	vprintf( pMsg, args );
	va_end( args );
}

extern "C" void Warning( const char *pMsg, ... )
{
	va_list args;
	va_start( args, pMsg );
	printf( "WARNING: " );
	vprintf( pMsg, args );
	va_end( args );
}

namespace
{

unsigned long g_nChecks = 0;
unsigned long g_nFailures = 0;

void Check( bool bCondition, const char *pWhat )
{
	++g_nChecks;
	if ( !bCondition )
		++g_nFailures;
	printf( "%s: %s\n", bCondition ? "ok  " : "FAIL", pWhat );
}

double Now()
{
	timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void Sleep( double flSeconds )
{
	timespec ts;
	ts.tv_sec = static_cast<time_t>( flSeconds );
	ts.tv_nsec = static_cast<long>( ( flSeconds - ts.tv_sec ) * 1e9 );
	nanosleep( &ts, nullptr );
}

double Length( const float v[3] )
{
	return sqrt( double( v[0] ) * v[0] + double( v[1] ) * v[1] + double( v[2] ) * v[2] );
}

// One sensor lifetime: Init, sample, suspend, resume, turn off, Shutdown.
void RunLifetime( int nPass )
{
	printf( "-- lifetime %d\n", nPass );
	CGyroSensor sensor;
	if ( !sensor.Init() )
	{
		Check( false, "the device has a gyroscope and the sensor thread starts" );
		return;
	}
	Check( sensor.IsAvailable(), "the sensor is available after Init" );

	// Off until a period is requested.
	Sleep( 0.3 );
	Check( sensor.SampleCount() == 0, "no samples arrive before a period is requested" );

	// A 120 Hz display asks for 240 Hz.
	const int nPeriodUs = gyro::SamplePeriodUs( 120.f );
	sensor.SetSamplePeriod( nPeriodUs );
	Sleep( 0.3 ); // registration and the first samples
	float rotation[3];
	sensor.ConsumeRotation( rotation );
	const uint32_t nStart = sensor.SampleCount();
	const double flStart = Now();
	Sleep( 1.0 );
	const uint32_t nEnd = sensor.SampleCount();
	const double flRate = ( nEnd - nStart ) / ( Now() - flStart );
	sensor.ConsumeRotation( rotation );
	printf( "   achieved %.1f Hz (asked for %.1f Hz); rotation %.5f %.5f %.5f rad\n", flRate,
	    1e6 / nPeriodUs, rotation[0], rotation[1], rotation[2] );
	Check( nStart > 0, "samples arrive once a period is requested" );
	// Android 12+ caps apps without HIGH_SAMPLING_RATE_SENSORS at 200 Hz.
	Check( flRate > 150.0 && flRate < 400.0, "samples arrive at about the requested rate" );
	Check( isfinite( rotation[0] ) && isfinite( rotation[1] ) && isfinite( rotation[2] ),
	    "the integrated rotation is finite" );
	Check( rotation[0] != 0.f || rotation[1] != 0.f || rotation[2] != 0.f,
	    "the samples reach the rotation totals (a real gyroscope is never exactly still)" );
	Check( Length( rotation ) < 0.2, "a device lying still turns less than 0.2 rad in 1 s" );

	float up[3];
	const bool bHaveUp = sensor.GetUp( up );
	printf( "   up %.3f %.3f %.3f (|up| %.3f)\n", up[0], up[1], up[2], Length( up ) );
	Check( bHaveUp, "which way is up is measured while the sensor runs" );
	Check( !bHaveUp || ( Length( up ) > 7.0 && Length( up ) < 13.0 ),
	    "up has about the magnitude of gravity" );

	// Background: the sensor turns off and its up is forgotten.
	sensor.SetSuspended( true );
	Sleep( 0.2 );
	const uint32_t nSuspended = sensor.SampleCount();
	Sleep( 0.5 );
	Check( sensor.SampleCount() - nSuspended <= 2, "no samples arrive while suspended" );
	Check( !sensor.GetUp( up ), "up is unknown while suspended" );

	// Foreground again.
	sensor.SetSuspended( false );
	Sleep( 0.5 );
	Check( sensor.SampleCount() - nSuspended > 50, "samples resume after the suspension ends" );

	// Turned off by the game.
	sensor.SetSamplePeriod( 0 );
	Sleep( 0.2 );
	const uint32_t nOff = sensor.SampleCount();
	Sleep( 0.5 );
	Check( sensor.SampleCount() - nOff <= 2, "no samples arrive with the period at 0" );

	// Shutdown joins the thread promptly, even with the sensor running.
	sensor.SetSamplePeriod( nPeriodUs );
	Sleep( 0.2 );
	const double flShutdown = Now();
	sensor.Shutdown();
	const double flShutdownSeconds = Now() - flShutdown;
	printf( "   shutdown took %.1f ms\n", flShutdownSeconds * 1e3 );
	Check( !sensor.IsAvailable(), "the sensor is unavailable after Shutdown" );
	Check( flShutdownSeconds < 0.5, "Shutdown joins the sensor thread within 0.5 s" );
	Check( sensor.SampleCount() == 0, "Shutdown releases the sample state" );
}

} // namespace

int main()
{
	// Two lifetimes: a second Init after Shutdown works the same.
	RunLifetime( 1 );
	RunLifetime( 2 );
	return testing::ReportConformance( g_nChecks, g_nFailures );
}
