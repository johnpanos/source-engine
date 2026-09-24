//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Rumble on the device's own vibrator for inputsystem.
//
// CDeviceVibrator plays amplitude-controlled one-shots; vibrator::CRumbleStream
// decides when to send them from the per-frame rumble request. This file picks
// the output (a rumble-capable gamepad takes precedence over the device) and
// stops the vibrator while the application is in the background.
//
//===========================================================================//

#include "inputsystem.h"
#include "tier0/icommandline.h"
#include "tier0/platform.h"
#include "tier1/convar.h"
#if defined( USE_SDL3 )
#include "SDL.h"
#endif
// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

static ConVar in_device_rumble( "in_device_rumble", "1", FCVAR_ARCHIVE,
    "Play rumble on the device's own vibrator when it has amplitude control and no "
    "rumble-capable gamepad is active (Android)." );

#if defined( USE_SDL3 )
static bool SDLCALL VibratorSDLWatcher( void *userInfo, SDL_Event *event )
{
	CDeviceVibrator *pVibrator = static_cast<CDeviceVibrator *>( userInfo );
	if ( !event || !pVibrator )
		return true;

	switch ( event->type )
	{
	case SDL_EVENT_WILL_ENTER_BACKGROUND:
		pVibrator->SetSuspended( true );
		break;
	case SDL_EVENT_DID_ENTER_FOREGROUND:
		pVibrator->SetSuspended( false );
		break;
	}
	return true;
}
#endif

void CInputSystem::InitializeDeviceVibrator()
{
	if ( m_DeviceVibrator.IsAvailable() )
		ShutdownDeviceVibrator();

	m_VibratorStream.Reset();
	if ( CommandLine()->FindParm( "-nodevicerumble" ) )
		return;
	if ( !m_DeviceVibrator.Init() )
		return;

#if defined( USE_SDL3 )
	m_bVibratorWatchInstalled = SDL_AddEventWatch( VibratorSDLWatcher, &m_DeviceVibrator );
	if ( !m_bVibratorWatchInstalled )
		Warning( "Device vibrator: lifecycle event watch failed: %s\n", SDL_GetError() );
#endif
}

void CInputSystem::ShutdownDeviceVibrator()
{
#if defined( USE_SDL3 )
	// SDL runs watchers under its watcher lock, so none is in flight after this.
	if ( m_bVibratorWatchInstalled )
		SDL_RemoveEventWatch( VibratorSDLWatcher, &m_DeviceVibrator );
#endif
	m_bVibratorWatchInstalled = false;
	m_DeviceVibrator.Shutdown();
	m_VibratorStream.Reset();
}

void CInputSystem::SetDeviceVibratorRumble( float fLeftMotor, float fRightMotor )
{
	if ( !m_DeviceVibrator.IsAvailable() )
		return;

	// The lifecycle watcher already cancelled the vibration.
	if ( m_DeviceVibrator.IsSuspended() )
	{
		m_VibratorStream.Reset();
		return;
	}

	// SetXDeviceRumble drives a gamepad that has rumble; the player holds that.
	const JoystickInfo_t &gamepad = m_pJoystickInfo[0];
	bool bGamepadRumbles = gamepad.m_nDeviceId >= 0 && gamepad.m_pHaptic != NULL;
	if ( bGamepadRumbles || !in_device_rumble.GetBool() )
		fLeftMotor = fRightMotor = 0.f;

	int64_t nNowMs = static_cast<int64_t>( Plat_FloatTime() * 1000.0 );
	vibrator::Command command = m_VibratorStream.Update( fLeftMotor, fRightMotor, nNowMs );
	switch ( command.m_Kind )
	{
	case vibrator::Command::PLAY:
		m_DeviceVibrator.Play( command.m_nAmplitude, command.m_nDurationMs );
		break;
	case vibrator::Command::STOP:
		m_DeviceVibrator.Cancel();
		break;
	case vibrator::Command::NONE:
		break;
	}
}
