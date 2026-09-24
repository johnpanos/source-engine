//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyro aiming for inputsystem: sample rate, screen axes, lifecycle.
//
// CGyroSensor reports rotation about the device's natural axes on its own
// thread. This file ties it to the display: it samples at twice the display
// refresh rate, remaps the axes to the screen as currently rotated, and turns
// the sensor off while the application is in the background. It also reports,
// once, a running gyroscope that delivers no samples.
//
//===========================================================================//

#include "inputsystem.h"
#include "gyro_math.h"
#include "tier0/icommandline.h"
#include "tier0/platform.h"
#if defined( USE_SDL3 )
#include "SDL.h"
#endif
// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#if defined( USE_SDL3 )
static bool SDLCALL GyroSDLWatcher( void *userInfo, SDL_Event *event )
{
	CGyroSensor *pSensor = static_cast<CGyroSensor *>( userInfo );
	if ( !event || !pSensor )
		return true;

	switch ( event->type )
	{
	case SDL_EVENT_WILL_ENTER_BACKGROUND:
		pSensor->SetSuspended( true );
		break;
	case SDL_EVENT_DID_ENTER_FOREGROUND:
		pSensor->SetSuspended( false );
		break;
	}
	return true;
}

static SDL_DisplayID GyroDisplay( ILauncherMgr *pLauncherMgr )
{
	SDL_Window *window =
	    pLauncherMgr ? static_cast<SDL_Window *>( pLauncherMgr->GetWindowRef() ) : NULL;
	SDL_DisplayID display = window ? SDL_GetDisplayForWindow( window ) : 0;
	return display ? display : SDL_GetPrimaryDisplay();
}

// Degrees the displayed image is rotated from the display's natural orientation.
static int GyroDisplayRotationSDL( SDL_DisplayID display )
{
	int nDegreesFromPortrait;
	switch ( SDL_GetCurrentDisplayOrientation( display ) )
	{
	case SDL_ORIENTATION_LANDSCAPE:
		nDegreesFromPortrait = 90;
		break;
	case SDL_ORIENTATION_PORTRAIT_FLIPPED:
		nDegreesFromPortrait = 180;
		break;
	case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
		nDegreesFromPortrait = 270;
		break;
	case SDL_ORIENTATION_PORTRAIT:
	default:
		nDegreesFromPortrait = 0;
		break;
	}
	return gyro::DisplayRotation( nDegreesFromPortrait,
	    SDL_GetNaturalDisplayOrientation( display ) == SDL_ORIENTATION_LANDSCAPE );
}
#endif

void CInputSystem::InitializeGyro()
{
	if ( m_GyroSensor.IsAvailable() )
		ShutdownGyro();

	if ( CommandLine()->FindParm( "-nogyro" ) )
		return;
	if ( !m_GyroSensor.Init() )
		return;

#if defined( USE_SDL3 )
	m_bGyroWatchInstalled = SDL_AddEventWatch( GyroSDLWatcher, &m_GyroSensor );
	if ( !m_bGyroWatchInstalled )
		Warning( "Gyro: lifecycle event watch failed: %s\n", SDL_GetError() );
#endif
	UpdateGyroSamplePeriod();
}

void CInputSystem::ShutdownGyro()
{
#if defined( USE_SDL3 )
	// SDL runs watchers under its watcher lock, so none is in flight after this
	// and the sensor thread can be joined.
	if ( m_bGyroWatchInstalled )
		SDL_RemoveEventWatch( GyroSDLWatcher, &m_GyroSensor );
#endif
	m_bGyroWatchInstalled = false;
	m_GyroSensor.Shutdown();
	m_bGyroEnabled = false;
}

// Runs on the main thread and only stores a request; the sensor thread
// re-registers the sensor when the period changes (display mode switch).
void CInputSystem::UpdateGyroSamplePeriod()
{
	if ( !m_bGyroEnabled )
	{
		m_GyroSensor.SetSamplePeriod( 0 );
		return;
	}

	float flRefreshHz = 0.f;
#if defined( USE_SDL3 )
	const SDL_DisplayMode *pMode = SDL_GetCurrentDisplayMode( GyroDisplay( m_pLauncherMgr ) );
	if ( pMode )
		flRefreshHz = pMode->refresh_rate;
#endif
	m_GyroSensor.SetSamplePeriod( gyro::SamplePeriodUs( flRefreshHz ) );
}

bool CInputSystem::EnableGyro( bool bEnable )
{
	const bool bWasEnabled = m_bGyroEnabled;
	m_bGyroEnabled = bEnable && m_GyroSensor.IsAvailable();
	if ( m_bGyroEnabled && !bWasEnabled )
	{
		m_flGyroLastPollTime = 0.0;
		m_bGyroStallReported = false;
	}
	UpdateGyroSamplePeriod();
	return m_GyroSensor.IsAvailable();
}

int CInputSystem::GyroDisplayRotation()
{
#if defined( USE_SDL3 )
	return GyroDisplayRotationSDL( GyroDisplay( m_pLauncherMgr ) );
#else
	return 0;
#endif
}

// A sensor that is on but silent would otherwise just look like a device held
// still. Polling pauses (menus, level loads, the background) restart the wait.
void CInputSystem::CheckGyroSamples()
{
	const double kSilentSeconds = 1.0;
	const double kPollGapSeconds = 0.25;

	const double flNow = Plat_FloatTime();
	const uint32 nSamples = m_GyroSensor.SampleCount();
	if ( nSamples != m_nGyroSamplesSeen || flNow - m_flGyroLastPollTime > kPollGapSeconds )
	{
		m_nGyroSamplesSeen = nSamples;
		m_flGyroLastSampleTime = flNow;
	}
	else if ( !m_bGyroStallReported && flNow - m_flGyroLastSampleTime > kSilentSeconds )
	{
		Warning( "Gyro: the gyroscope is on but has delivered no samples for %.0f s\n",
		    kSilentSeconds );
		m_bGyroStallReported = true;
	}
	m_flGyroLastPollTime = flNow;
}

bool CInputSystem::GetGyroAccumulators( float &pitch, float &yaw, float &roll )
{
	pitch = yaw = roll = 0.f;
	if ( !m_bGyroEnabled )
		return false;

	UpdateGyroSamplePeriod();
	CheckGyroSamples();

	float device[3];
	m_GyroSensor.ConsumeRotation( device );

	float screen[3];
	gyro::DeviceToScreen( device, GyroDisplayRotation(), screen );
	pitch = screen[0];
	yaw = screen[1];
	roll = screen[2];
	return true;
}

bool CInputSystem::GetGyroUp( float &right, float &up, float &out )
{
	right = up = out = 0.f;
	float device[3];
	if ( !m_bGyroEnabled || !m_GyroSensor.GetUp( device ) )
		return false;

	// A direction remaps like a rotation axis.
	float screen[3];
	gyro::DeviceToScreen( device, GyroDisplayRotation(), screen );
	right = screen[0];
	up = screen[1];
	out = screen[2];
	return true;
}
