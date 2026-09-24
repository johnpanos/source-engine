//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyro aiming for inputsystem: sample rate, screen axes, lifecycle.
//
// CGyroSensor reports rotation about the device's natural axes on its own
// thread. This file ties it to the display: it samples at twice the display
// refresh rate, remaps the axes to the screen as currently rotated, and turns
// the sensor off while the application is in the background.
//
//===========================================================================//

#include "inputsystem.h"
#include "gyro_math.h"
#include "tier0/icommandline.h"
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
static int GyroDisplayRotation( SDL_DisplayID display )
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
	m_bGyroEnabled = bEnable && m_GyroSensor.IsAvailable();
	UpdateGyroSamplePeriod();
	return m_GyroSensor.IsAvailable();
}

bool CInputSystem::GetGyroAccumulators( float &pitch, float &yaw, float &roll )
{
	pitch = yaw = roll = 0.f;
	if ( !m_bGyroEnabled )
		return false;

	UpdateGyroSamplePeriod();

	float device[3];
	m_GyroSensor.ConsumeRotation( device );

	int nRotation = 0;
#if defined( USE_SDL3 )
	nRotation = GyroDisplayRotation( GyroDisplay( m_pLauncherMgr ) );
#endif
	float screen[3];
	gyro::DeviceToScreen( device, nRotation, screen );
	pitch = screen[0];
	yaw = screen[1];
	roll = screen[2];
	return true;
}
