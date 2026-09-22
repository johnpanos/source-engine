//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Linux/Android touch implementation for inputsystem
//
//===========================================================================//

/* For force feedback testing. */
#include "inputsystem.h"
#include "tier1/convar.h"
#include "tier0/icommandline.h"
#include "SDL.h"
#include "SDL_touch.h"
// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Handle the events coming from the Touch SDL subsystem.
//-----------------------------------------------------------------------------
#if defined( USE_SDL3 )
bool SDLCALL TouchSDLWatcher( void *userInfo, SDL_Event *event )
#else
int SDLCALL TouchSDLWatcher( void *userInfo, SDL_Event *event )
#endif
{
	CInputSystem *pInputSystem = (CInputSystem *)userInfo;

	if( !event || !pInputSystem ) return 1;

#if defined( USE_SDL3 )
	switch ( event->type )
	{
	case SDL_FINGERDOWN:
		pInputSystem->FingerEventSDL3( IE_FingerDown, event->tfinger.fingerID, event->tfinger.x,
		    event->tfinger.y, event->tfinger.dx, event->tfinger.dy );
		break;
	case SDL_FINGERUP:
	case SDL_EVENT_FINGER_CANCELED:
		pInputSystem->FingerEventSDL3( IE_FingerUp, event->tfinger.fingerID, event->tfinger.x,
		    event->tfinger.y, event->tfinger.dx, event->tfinger.dy );
		break;
	case SDL_FINGERMOTION:
		pInputSystem->FingerEventSDL3( IE_FingerMotion, event->tfinger.fingerID, event->tfinger.x,
		    event->tfinger.y, event->tfinger.dx, event->tfinger.dy );
		break;
	}

#else
	switch ( event->type ) {
	case SDL_FINGERDOWN:
		pInputSystem->FingerEvent( IE_FingerDown, event->tfinger.fingerId, event->tfinger.x, event->tfinger.y, event->tfinger.dx, event->tfinger.dy );
		break;
	case SDL_FINGERUP:
		pInputSystem->FingerEvent( IE_FingerUp, event->tfinger.fingerId, event->tfinger.x, event->tfinger.y, event->tfinger.dx, event->tfinger.dy );
		break;
	case SDL_FINGERMOTION:
		pInputSystem->FingerEvent( IE_FingerMotion ,event->tfinger.fingerId, event->tfinger.x, event->tfinger.y, event->tfinger.dx, event->tfinger.dy );
		break;
	}

#endif

	return 1;
}

//-----------------------------------------------------------------------------
// Initialize all joysticks
//-----------------------------------------------------------------------------
void CInputSystem::InitializeTouch( void )
{
	if ( m_bTouchInitialized )
		ShutdownTouch();

	// abort startup if user requests no touch
	if ( CommandLine()->FindParm("-notouch") ) return;

	memset( m_touchAccumX, 0, sizeof(m_touchAccumX) );
	memset( m_touchAccumY, 0, sizeof(m_touchAccumY) );

#if defined( USE_SDL3 )
	memset( m_touchFingerUsed, 0, sizeof( m_touchFingerUsed ) );
#endif
#if defined( USE_SDL3 )
	m_bTouchInitialized = SDL_AddEventWatch( TouchSDLWatcher, this );
	if ( !m_bTouchInitialized )
		Warning( "Touch event watch failed: %s\n", SDL_GetError() );
#else
	m_bTouchInitialized = true;
	SDL_AddEventWatch(TouchSDLWatcher, this);
#endif
}

void CInputSystem::ShutdownTouch()
{
	if ( !m_bTouchInitialized )
		return;

	SDL_DelEventWatch( TouchSDLWatcher, this );
	m_bTouchInitialized = false;
}

bool CInputSystem::GetTouchAccumulators( int fingerId, float &dx, float &dy )
{
	if ( fingerId < 0 || fingerId >= TOUCH_FINGER_MAX_COUNT )
	{
		dx = dy = 0.f;
		return false;
	}
	dx = m_touchAccumX[fingerId];
	dy = m_touchAccumY[fingerId];

	m_touchAccumX[fingerId] = m_touchAccumY[fingerId] = 0.f;

	return true;
}

void CInputSystem::FingerEvent(int eventType, int fingerId, float x, float y, float dx, float dy)
{
	if ( fingerId < 0 || fingerId >= TOUCH_FINGER_MAX_COUNT )
		return;

	if( eventType == IE_FingerUp )
	{
		m_touchAccumX[fingerId] = 0.f;
		m_touchAccumY[fingerId] = 0.f;
	}
	else
	{
		m_touchAccumX[fingerId] += dx;
		m_touchAccumY[fingerId] += dy;
	}

	int _x,_y;
	memcpy( &_x, &x, sizeof(float) );
	memcpy( &_y, &y, sizeof(float) );
	PostEvent(eventType, m_nLastSampleTick, fingerId, _x, _y);
}

#if defined( USE_SDL3 )
void CInputSystem::FingerEventSDL3(
    int eventType, uint64 fingerId, float x, float y, float dx, float dy )
{
	int slot = -1;
	int available = -1;
	for ( int i = 0; i < TOUCH_FINGER_MAX_COUNT; ++i )
	{
		if ( m_touchFingerUsed[i] && m_touchFingerIds[i] == fingerId )
			slot = i;
		else if ( !m_touchFingerUsed[i] && available < 0 )
			available = i;
	}
	if ( slot < 0 && eventType == IE_FingerDown && available >= 0 )
	{
		slot = available;
		m_touchFingerUsed[slot] = true;
		m_touchFingerIds[slot] = fingerId;
	}
	if ( slot < 0 )
		return;
	FingerEvent( eventType, slot, x, y, dx, dy );
	if ( eventType == IE_FingerUp )
		m_touchFingerUsed[slot] = false;
}
#endif
