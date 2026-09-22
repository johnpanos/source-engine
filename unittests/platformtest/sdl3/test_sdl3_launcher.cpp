//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native SDL3 launcher outcomes: Vulkan-only window, event delivery,
//          no dropped event at pump budget, partial failure, repeated lifetime.
//
//=============================================================================//

#include <SDL3/SDL.h>
#include "appframework/ilaunchermgr.h"
#include "tier0/icommandline.h"
#include <cstdio>
#include <cstring>

extern void *CreateSDLMgr();

namespace
{
int checks = 0;
int failures = 0;

void Check( bool passed, const char *description, int line )
{
	++checks;
	if ( !passed )
	{
		++failures;
		std::fprintf( stderr, "FAIL line %d: %s (SDL: %s)\n", line, description, SDL_GetError() );
	}
}
#define CHECK( condition ) Check( ( condition ), #condition, __LINE__ )

void Push( SDL_Event &event )
{
	CHECK( SDL_PushEvent( &event ) );
}

void DiscardPending( ILauncherMgr &manager )
{
	SDL_FlushEvents( SDL_EVENT_FIRST, SDL_EVENT_LAST );
	CCocoaEvent events[128];
	while ( manager.GetEvents( events, 128 ) )
	{
	}
}

void CheckEvents( ILauncherMgr &manager )
{
	SDL_Window *window = static_cast<SDL_Window *>( manager.GetWindowRef() );
	const SDL_WindowID windowId = SDL_GetWindowID( window );
	DiscardPending( manager );
	SDL_Event input = {};
	input.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
	input.window.windowID = windowId;
	Push( input );
	manager.PumpWindowsMessageLoop();
	CCocoaEvent events[128] = {};
	CHECK( manager.GetEvents( events, 128 ) == 1 );
	CHECK( events[0].m_EventType == CocoaEvent_AppActivate );
	CHECK( events[0].m_ModifierKeyMask == 1 );

	input = {};
	input.type = SDL_EVENT_KEY_DOWN;
	input.key.windowID = windowId;
	input.key.scancode = SDL_SCANCODE_W;
	input.key.key = SDLK_W;
	input.key.mod = SDL_KMOD_SHIFT;
	input.key.down = true;
	Push( input );
	input.type = SDL_EVENT_KEY_UP;
	input.key.down = false;
	input.key.mod = 0;
	Push( input );
	input = {};
	input.type = SDL_EVENT_TEXT_INPUT;
	input.text.windowID = windowId;
	input.text.text = "\xc3\xa9\xf0\x9f\x9a\xaa";
	Push( input );
	manager.PumpWindowsMessageLoop();
	CHECK( manager.GetEvents( events, 128, true ) == 0 );
	CHECK( manager.GetEvents( events, 128 ) == 6 );
	CHECK( events[0].m_EventType == CocoaEvent_KeyDown );
	CHECK( events[0].m_VirtualKeyCode == SDL_SCANCODE_W );
	CHECK( events[0].m_ModifierKeyMask == ( 1 << eShiftKey ) );
	CHECK( events[1].m_EventType == CocoaEvent_KeyUp );
	CHECK( events[1].m_ModifierKeyMask == 0 );
	CHECK( events[2].m_UnicodeKey == 0xe9 );
	CHECK( events[3].m_EventType == CocoaEvent_KeyUp );
	CHECK( events[4].m_UnicodeKey == 0x1f6aa );
	CHECK( events[5].m_EventType == CocoaEvent_KeyUp );

	input = {};
	input.type = SDL_EVENT_MOUSE_MOTION;
	input.motion.windowID = windowId;
	input.motion.x = 100;
	input.motion.y = 80;
	input.motion.xrel = 2.5f;
	input.motion.yrel = 3.25f;
	Push( input );
	Push( input );
	input = {};
	input.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
	input.button.windowID = windowId;
	input.button.button = SDL_BUTTON_RIGHT;
	input.button.down = true;
	input.button.clicks = 2;
	Push( input );
	manager.PumpWindowsMessageLoop();
	CHECK( manager.GetEvents( events, 128 ) == 3 );
	CHECK( events[0].m_EventType == CocoaEvent_MouseMove );
	CHECK( events[0].m_MousePos[0] == 100 );
	CHECK( events[2].m_EventType == CocoaEvent_MouseButtonDown );
	CHECK( events[2].m_MouseButton == COCOABUTTON_RIGHT );
	CHECK( events[2].m_MouseButtonFlags == COCOABUTTON_RIGHT );
	CHECK( events[2].m_nMouseClickCount == 2 );
	int x = 0, y = 0;
	manager.GetMouseDelta( x, y );
	CHECK( x == 5 && y == 6 );
	manager.GetMouseDelta( x, y );
	CHECK( x == 0 && y == 0 );

	DiscardPending( manager );
	input = {};
	input.type = SDL_EVENT_KEY_DOWN;
	input.key.windowID = windowId;
	input.key.scancode = SDL_SCANCODE_SPACE;
	input.key.key = SDLK_SPACE;
	input.key.down = true;
	for ( int i = 0; i < 101; ++i )
		Push( input );
	manager.PumpWindowsMessageLoop();
	CHECK( manager.GetEvents( events, 128 ) == 100 );
	manager.PumpWindowsMessageLoop();
	CHECK( manager.GetEvents( events, 128 ) == 1 );
	CHECK( events[0].m_VirtualKeyCode == SDL_SCANCODE_SPACE );

	Push( input );
	manager.PumpWindowsMessageLoop();
	bool escape = false, enter = false, space = false;
	CHECK( manager.PeekAndRemoveKeyboardEvents( &escape, &enter, &space ) == 1 );
	CHECK( !escape && !enter && space );
	CHECK( manager.GetEvents( events, 128 ) == 1 );
	CHECK( events[0].m_EventType == CocoaEvent_Deleted );

	input = {};
	input.type = SDL_EVENT_WINDOW_FOCUS_LOST;
	input.window.windowID = windowId;
	Push( input );
	input = {};
	input.type = SDL_EVENT_MOUSE_MOTION;
	input.motion.windowID = windowId;
	input.motion.xrel = 100;
	Push( input );
	input = {};
	input.type = SDL_EVENT_QUIT;
	Push( input );
	manager.PumpWindowsMessageLoop();
	CHECK( manager.GetEvents( events, 128 ) == 2 );
	CHECK( events[0].m_EventType == CocoaEvent_AppActivate );
	CHECK( events[0].m_ModifierKeyMask == 0 );
	CHECK( events[1].m_EventType == CocoaEvent_AppQuit );
	manager.GetMouseDelta( x, y );
	CHECK( x == 0 && y == 0 );
}
} // namespace

int main( int argc, char **argv )
{
	CommandLine()->CreateCmdLine( argc, argv );
	ILauncherMgr *manager = static_cast<ILauncherMgr *>( CreateSDLMgr() );
	CHECK( manager != NULL );
	CHECK( manager->GetWindowRef() == NULL );
	CHECK( manager->Init() == INIT_OK );
	SDL_Window *window = static_cast<SDL_Window *>( manager->GetWindowRef() );
	if ( window == NULL )
	{
		std::fprintf( stderr, "Native Vulkan-capable SDL3 display is required.\n" );
		return 1;
	}
	CHECK( std::strcmp( SDL_GetCurrentVideoDriver(), "wayland" ) == 0 );
	CHECK( ( SDL_GetWindowFlags( window ) & SDL_WINDOW_VULKAN ) != 0 );
	CHECK( ( SDL_GetWindowFlags( window ) & SDL_WINDOW_OPENGL ) == 0 );
	CHECK( manager->Init() == INIT_OK );
	CHECK( manager->GetWindowRef() == window );
	CheckEvents( *manager );

	manager->IncWindowRefCount();
	manager->DestroyGameWindow();
	CHECK( manager->GetWindowRef() == window );
	manager->DestroyGameWindow();
	CHECK( manager->GetWindowRef() == NULL );
	CHECK( manager->CreateGameWindow( "SDL3 lifecycle fixture", true, 640, 480 ) );
	CHECK( manager->GetWindowRef() != NULL );
	manager->Shutdown();
	CHECK( manager->GetWindowRef() == NULL );
	CHECK( ( SDL_WasInit( SDL_INIT_VIDEO ) & SDL_INIT_VIDEO ) == 0 );

	CHECK( SDL_SetHintWithPriority(
	    SDL_HINT_VIDEO_DRIVER, "source_fixture_missing_driver", SDL_HINT_OVERRIDE ) );
	CHECK( manager->Init() == INIT_FAILED );
	CHECK( manager->GetWindowRef() == NULL );
	CHECK( ( SDL_WasInit( SDL_INIT_VIDEO ) & SDL_INIT_VIDEO ) == 0 );
	CHECK( SDL_SetHintWithPriority( SDL_HINT_VIDEO_DRIVER, "wayland", SDL_HINT_OVERRIDE ) );
	CHECK( manager->Init() == INIT_OK );
	CHECK( manager->GetWindowRef() != NULL );
	manager->Shutdown();
	manager->Shutdown();
	CHECK( manager->GetWindowRef() == NULL );
	CHECK( ( SDL_WasInit( SDL_INIT_VIDEO ) & SDL_INIT_VIDEO ) == 0 );
	std::printf( "CONFORMANCE %d %d\n", checks, failures );
	return failures == 0 ? 0 : 1;
}
