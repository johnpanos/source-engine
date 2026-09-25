//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL3 window/input provider for the Vulkan compatibility profile.
//          SDL_Window is passed only through the retained launcher/render bridge.
//          This provider never creates a GL context or probes GL entry points.
//
//=============================================================================//

#include <SDL3/SDL.h>

#include "appframework/ilaunchermgr.h"
#include "inputsystem/ButtonCode.h"
#include "tier0/icommandline.h"
#include "tier1/convar.h"
#include "tier1/utllinkedlist.h"

#include "tier0/memdbgon.h"

#if defined( DX_TO_GL_ABSTRACTION )
#error The SDL3 Vulkan launcher provider must not be compiled with the OpenGL launcher ABI.
#endif

namespace
{

ConVar sdl_displayindex( "sdl_displayindex", "0", FCVAR_HIDDEN, "SDL fullscreen display index." );
ConVar sdl_displayindex_fullscreen( "sdl_displayindex_fullscreen", "-1", FCVAR_HIDDEN );

SDL_DisplayID GetDisplay( int index )
{
	int count = 0;
	SDL_DisplayID *displays = SDL_GetDisplays( &count );
	SDL_DisplayID display = SDL_GetPrimaryDisplay();
	if ( displays != NULL && index >= 0 && index < count )
	{
		display = displays[index];
	}
	SDL_free( displays );
	return display;
}

uint ModifierMask( SDL_Keymod modifiers )
{
	uint mask = 0;
	if ( modifiers & SDL_KMOD_CAPS )
		mask |= 1 << eCapsLockKey;
	if ( modifiers & SDL_KMOD_SHIFT )
		mask |= 1 << eShiftKey;
	if ( modifiers & SDL_KMOD_CTRL )
		mask |= 1 << eControlKey;
	if ( modifiers & SDL_KMOD_ALT )
		mask |= 1 << eAltKey;
	if ( modifiers & SDL_KMOD_GUI )
		mask |= 1 << eCommandKey;
	return mask;
}

int ModifierButton( SDL_Keycode key )
{
	switch ( key )
	{
	case SDLK_CAPSLOCK:
		return -KEY_CAPSLOCK;
	case SDLK_RSHIFT:
		return -KEY_RSHIFT;
	case SDLK_LSHIFT:
		return -KEY_LSHIFT;
	case SDLK_RCTRL:
		return -KEY_RCONTROL;
	case SDLK_LCTRL:
		return -KEY_LCONTROL;
	case SDLK_RGUI:
		return -KEY_RWIN;
	case SDLK_LGUI:
		return -KEY_LWIN;
	case SDLK_RALT:
		return -KEY_RALT;
	case SDLK_LALT:
		return -KEY_LALT;
	default:
		return 0;
	}
}

class CSDL3Mgr : public CBaseAppSystem<ILauncherMgr>
{
public:
	~CSDL3Mgr() { Shutdown(); }
	void *QueryInterface( const char *name ) override
	{
		return name && !Q_stricmp( name, SDLMGR_INTERFACE_VERSION ) ? this : NULL;
	}
	InitReturnVal_t Init() override;
	void Shutdown() override;
	bool CreateGameWindow( const char *title, bool windowed, int width, int height ) override;
	void IncWindowRefCount() override
	{
		if ( m_Window )
			++m_nWindowRefs;
	}
	void DecWindowRefCount() override;
	int GetEvents( CCocoaEvent *events, int maximum, bool debugEvents = false ) override;
#if defined( LINUX ) || defined( PLATFORM_BSD )
	int PeekAndRemoveKeyboardEvents(
	    bool *escape, bool *enter, bool *space, bool debugEvents = false ) override;
#endif
	void SetCursorPosition( int x, int y ) override;
	void SetWindowFullScreen( bool fullScreen, int width, int height ) override;
	bool IsWindowFullScreen() override
	{
		return m_Window && ( SDL_GetWindowFlags( m_Window ) & SDL_WINDOW_FULLSCREEN ) != 0;
	}
	void MoveWindow( int x, int y ) override
	{
		if ( m_Window )
			SDL_SetWindowPosition( m_Window, x, y );
	}
	void SizeWindow( int width, int height ) override;
	void PumpWindowsMessageLoop() override;
	void DestroyGameWindow() override { DecWindowRefCount(); }
	void SetApplicationIcon( const char *path ) override;
	void GetMouseDelta( int &x, int &y, bool ignoreNextMouseDelta = false ) override;
	void GetNativeDisplayInfo( int display, uint &width, uint &height, uint &refresh ) override;
	void RenderedSize( uint &width, uint &height, bool set ) override;
	void DisplayedSize( uint &width, uint &height ) override;
	void GetStackCrawl( CStackCrawlParams * ) override {}
	void WaitUntilUserInput( int milliseconds ) override
	{
		// A null event waits without removing it from the SDL queue.
		SDL_WaitEventTimeout( NULL, milliseconds );
	}
	void *GetWindowRef() override { return m_Window; }
	void SetMouseVisible( bool visible ) override;
	void SetMouseCursor( SDL_Cursor *cursor ) override;
	void SetForbidMouseGrab( bool forbid ) override
	{
		m_bForbidMouseGrab = forbid;
		m_bCursorDirty = true;
	}
	void OnFrameRendered() override;
	void SetGammaRamp( const uint16 *, const uint16 *, const uint16 * ) override;
	double GetPrevGLSwapWindowTime() override { return 0.0; }
	float GetWindowDisplayScale() override
	{
		const float scale = m_Window ? SDL_GetWindowDisplayScale( m_Window ) : 0.0f;
		return scale > 0.0f ? scale : 1.0f;
	}

private:
	void PostEvent( const CCocoaEvent &event );
	void HandleEvent( const SDL_Event &event );
	void HandleKey( const SDL_KeyboardEvent &event );
	void MousePosition( CCocoaEvent &event, float x, float y );
	void RenderedToWindowScale( float &scaleX, float &scaleY );

	SDL_Window *m_Window = NULL;
	SDL_Cursor *m_Cursor = NULL; // Borrowed from the UI cursor owner.
	unsigned m_nWindowRefs = 0;
	bool m_bVideoInitialized = false;
	bool m_bHasFocus = false;
	bool m_bCursorVisible = true;
	bool m_bCursorDirty = true;
	bool m_bVisibilitySetThisFrame = false;
	bool m_bForbidMouseGrab = true;
	// The relative-mode failure was reported; not again until it succeeds
	// (a driver without it, such as the offscreen one, fails every frame).
	bool m_bRelativeModeFailureReported = false;
	bool m_bGammaReported = false;
	float m_MouseDeltaX = 0;
	float m_MouseDeltaY = 0;
	uint m_RenderedWidth = 0;
	uint m_RenderedHeight = 0;
	uint m_ModifierMask = 0;
	int m_MouseButtons = 0;
	CThreadMutex m_EventsMutex;
	CUtlLinkedList<CCocoaEvent, int> m_Events;
};

InitReturnVal_t CSDL3Mgr::Init()
{
	if ( m_bVideoInitialized && m_Window )
		return INIT_OK;
	if ( !m_bVideoInitialized && !SDL_InitSubSystem( SDL_INIT_VIDEO ) )
	{
		Warning( "SDL3 video initialization failed: %s\n", SDL_GetError() );
		return INIT_FAILED;
	}
	m_bVideoInitialized = true;
	m_bForbidMouseGrab =
	    CommandLine()->FindParm( "-nomousegrab" ) || !CommandLine()->FindParm( "-mousegrab" );
	m_Window = SDL_CreateWindow( "", 1280, 720,
	    SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY |
	        SDL_WINDOW_RESIZABLE );
	if ( m_Window == NULL )
	{
		Warning( "SDL3 Vulkan window creation failed: %s\n", SDL_GetError() );
		Shutdown();
		return INIT_FAILED;
	}
	m_nWindowRefs = 1;
	// Where text input raises an on-screen keyboard (Android), it starts only
	// when a text field takes focus (IInputSystem::StartTextInput).
	if ( !SDL_HasScreenKeyboardSupport() && !SDL_StartTextInput( m_Window ) )
	{
		Warning( "SDL3 text input initialization failed: %s\n", SDL_GetError() );
		Shutdown();
		return INIT_FAILED;
	}
	SetAssertDialogParent( m_Window );
	Msg( "RFC0001 window: provider=sdl3 driver=%s\n", SDL_GetCurrentVideoDriver() );
	return INIT_OK;
}

void CSDL3Mgr::Shutdown()
{
	if ( m_Window )
	{
		// Application shutdown follows borrowers' shutdown in the lifecycle graph.
		m_nWindowRefs = 1;
		DecWindowRefCount();
	}
	if ( m_bVideoInitialized )
	{
		SDL_QuitSubSystem( SDL_INIT_VIDEO );
		m_bVideoInitialized = false;
	}
	AUTO_LOCK( m_EventsMutex );
	m_Events.RemoveAll();
	m_MouseDeltaX = m_MouseDeltaY = 0;
	m_RenderedWidth = m_RenderedHeight = m_ModifierMask = 0;
	m_MouseButtons = 0;
	m_Cursor = NULL;
	m_bHasFocus = false;
	m_bCursorVisible = true;
	m_bCursorDirty = true;
	m_bVisibilitySetThisFrame = false;
}

bool CSDL3Mgr::CreateGameWindow( const char *title, bool windowed, int width, int height )
{
	if ( !m_Window && Init() != INIT_OK )
		return false;
	if ( title && !SDL_SetWindowTitle( m_Window, title ) )
		return false;
	if ( width <= 0 || height <= 0 )
		return true; // Startup reserves the hidden window; the mode switch shows it.
	if ( !SDL_SetWindowSize( m_Window, width, height ) ||
	     !SDL_SetWindowFullscreenMode( m_Window, NULL ) ||
	     !SDL_SetWindowFullscreen( m_Window, !windowed ) )
	{
		Warning( "SDL3 window mode failed: %s\n", SDL_GetError() );
		return false;
	}
	sdl_displayindex_fullscreen.SetValue( windowed ? -1 : sdl_displayindex.GetInt() );
	return SDL_ShowWindow( m_Window );
}

void CSDL3Mgr::DecWindowRefCount()
{
	if ( !m_Window || m_nWindowRefs == 0 )
		return;
	if ( --m_nWindowRefs != 0 )
		return;
	SDL_StopTextInput( m_Window );
	SDL_SetWindowRelativeMouseMode( m_Window, false );
	SDL_SetWindowMouseGrab( m_Window, false );
	SetAssertDialogParent( NULL );
	SDL_DestroyWindow( m_Window );
	m_Window = NULL;
}

void CSDL3Mgr::PostEvent( const CCocoaEvent &event )
{
	AUTO_LOCK( m_EventsMutex );
	m_Events.AddToTail( event );
}

int CSDL3Mgr::GetEvents( CCocoaEvent *events, int maximum, bool debugEvents )
{
	if ( debugEvents || events == NULL || maximum <= 0 )
		return 0;
	AUTO_LOCK( m_EventsMutex );
	int count = 0;
	while ( count < maximum && m_Events.Count() )
	{
		const int head = m_Events.Head();
		events[count++] = m_Events[head];
		m_Events.Remove( head );
	}
	return count;
}

#if defined( LINUX ) || defined( PLATFORM_BSD )
int CSDL3Mgr::PeekAndRemoveKeyboardEvents(
    bool *escape, bool *enter, bool *space, bool debugEvents )
{
	if ( debugEvents )
		return 0;
	AUTO_LOCK( m_EventsMutex );
	int count = 0;
	FOR_EACH_LL( m_Events, i )
	{
		CCocoaEvent &event = m_Events[i];
		if ( event.m_EventType != CocoaEvent_KeyDown )
			continue;
		bool *pressed = NULL;
		switch ( event.m_VirtualKeyCode )
		{
		case SDL_SCANCODE_ESCAPE:
			pressed = escape;
			break;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_KP_ENTER:
			pressed = enter;
			break;
		case SDL_SCANCODE_SPACE:
			pressed = space;
			break;
		}
		if ( pressed )
		{
			*pressed = true;
			event.m_EventType = CocoaEvent_Deleted;
			++count;
		}
	}
	return count;
}
#endif

void CSDL3Mgr::SetWindowFullScreen( bool fullScreen, int width, int height )
{
	if ( !m_Window )
		return;
	if ( fullScreen )
	{
		const SDL_DisplayID display = GetDisplay( sdl_displayindex.GetInt() );
		SDL_SetWindowPosition( m_Window, SDL_WINDOWPOS_CENTERED_DISPLAY( display ),
		    SDL_WINDOWPOS_CENTERED_DISPLAY( display ) );
	}
	if ( !SDL_SetWindowFullscreenMode( m_Window, NULL ) ||
	     !SDL_SetWindowFullscreen( m_Window, fullScreen ) )
	{
		Warning( "SDL3 fullscreen transition failed: %s\n", SDL_GetError() );
		return;
	}
	sdl_displayindex_fullscreen.SetValue( fullScreen ? sdl_displayindex.GetInt() : -1 );
	SizeWindow( width, height );
}

void CSDL3Mgr::SizeWindow( int width, int height )
{
	if ( !m_Window || width <= 0 || height <= 0 )
		return;
	// A maximized window keeps its size until it is restored; an explicit size
	// request restores it first.
	if ( SDL_GetWindowFlags( m_Window ) & SDL_WINDOW_MAXIMIZED )
		SDL_RestoreWindow( m_Window );
	if ( !SDL_SetWindowSize( m_Window, width, height ) )
	{
		Warning( "SDL3 window resize failed: %s\n", SDL_GetError() );
		return;
	}
	SDL_ShowWindow( m_Window );
}

void CSDL3Mgr::SetApplicationIcon( const char *path )
{
	if ( !m_Window || !path )
		return;
	SDL_Surface *icon = SDL_LoadBMP( path );
	if ( icon )
	{
		SDL_SetWindowIcon( m_Window, icon );
		SDL_DestroySurface( icon );
	}
}

void CSDL3Mgr::GetMouseDelta( int &x, int &y, bool ignoreNextMouseDelta )
{
	(void)ignoreNextMouseDelta; // Retained launcher behavior; legacy callers pass false.
	x = static_cast<int>( m_MouseDeltaX );
	y = static_cast<int>( m_MouseDeltaY );
	m_MouseDeltaX -= x;
	m_MouseDeltaY -= y;
}

void CSDL3Mgr::GetNativeDisplayInfo( int display, uint &width, uint &height, uint &refresh )
{
	const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(
	    GetDisplay( display < 0 ? sdl_displayindex.GetInt() : display ) );
	width = mode ? mode->w : 0;
	height = mode ? mode->h : 0;
	refresh = mode ? static_cast<uint>( mode->refresh_rate ) : 0;
}

void CSDL3Mgr::RenderedSize( uint &width, uint &height, bool set )
{
	if ( set )
	{
		m_RenderedWidth = width;
		m_RenderedHeight = height;
	}
	else
	{
		width = m_RenderedWidth;
		height = m_RenderedHeight;
	}
}

void CSDL3Mgr::DisplayedSize( uint &width, uint &height )
{
	int w = 0, h = 0;
	if ( m_Window )
		SDL_GetWindowSizeInPixels( m_Window, &w, &h );
	width = w;
	height = h;
}

void CSDL3Mgr::SetMouseVisible( bool visible )
{
	if ( !m_bVisibilitySetThisFrame || visible )
	{
		m_bCursorVisible = visible;
		m_bCursorDirty = true;
		m_bVisibilitySetThisFrame = true;
	}
}

void CSDL3Mgr::SetMouseCursor( SDL_Cursor *cursor )
{
	if ( cursor )
		m_Cursor = cursor;
	else
		SetMouseVisible( false );
	m_bCursorDirty = true;
}

void CSDL3Mgr::OnFrameRendered()
{
	if ( m_Window && m_bCursorDirty )
	{
		const bool capture = m_bHasFocus && !m_bCursorVisible;
		// Wayland FPS control requires relative motion; there is no global warp.
		if ( SDL_SetWindowRelativeMouseMode( m_Window, capture ) )
			m_bRelativeModeFailureReported = false;
		else if ( !m_bRelativeModeFailureReported )
		{
			Warning( "SDL3 relative mouse mode failed: %s\n", SDL_GetError() );
			m_bRelativeModeFailureReported = true;
		}
		SDL_SetWindowMouseGrab( m_Window, capture && !m_bForbidMouseGrab );
		if ( m_bCursorVisible || !m_bHasFocus )
		{
			if ( m_Cursor )
				SDL_SetCursor( m_Cursor );
			SDL_ShowCursor();
		}
		else
			SDL_HideCursor();
		m_bCursorDirty = false;
	}
	m_bVisibilitySetThisFrame = false;
}

void CSDL3Mgr::SetGammaRamp( const uint16 *, const uint16 *, const uint16 * )
{
	// SDL3/Wayland has no window gamma-ramp API. Material gamma must be applied
	// by the rendering provider; the legacy void method cannot report a result.
	if ( !m_bGammaReported )
	{
		Warning( "SDL3 window gamma ramps are unavailable; use material gamma correction.\n" );
		m_bGammaReported = true;
	}
}

// Rendered (back buffer) pixels per window coordinate: SDL reports the mouse in
// window coordinates, while cursor positions are exchanged in rendered pixels.
void CSDL3Mgr::RenderedToWindowScale( float &scaleX, float &scaleY )
{
	int width = 0, height = 0;
	SDL_GetWindowSize( m_Window, &width, &height );
	scaleX = width > 0 && m_RenderedWidth ? static_cast<float>( m_RenderedWidth ) / width : 1.0f;
	scaleY =
	    height > 0 && m_RenderedHeight ? static_cast<float>( m_RenderedHeight ) / height : 1.0f;
}

void CSDL3Mgr::MousePosition( CCocoaEvent &event, float x, float y )
{
	float scaleX, scaleY;
	RenderedToWindowScale( scaleX, scaleY );
	event.m_MousePos[0] = static_cast<int>( x * scaleX );
	event.m_MousePos[1] = static_cast<int>( y * scaleY );
	event.m_MouseButtonFlags = m_MouseButtons;
}

void CSDL3Mgr::SetCursorPosition( int x, int y )
{
	if ( !m_Window )
		return;
	float scaleX, scaleY;
	RenderedToWindowScale( scaleX, scaleY );
	SDL_WarpMouseInWindow( m_Window, x / scaleX, y / scaleY );
}

void CSDL3Mgr::HandleKey( const SDL_KeyboardEvent &key )
{
	CCocoaEvent event = {};
	event.m_EventType = key.down ? CocoaEvent_KeyDown : CocoaEvent_KeyUp;
	event.m_VirtualKeyCode = ModifierButton( key.key );
	if ( event.m_VirtualKeyCode == 0 )
		event.m_VirtualKeyCode = key.scancode;
	if ( CommandLine()->FindParm( "-nonqwerty" ) )
	{
		const char *name = SDL_GetKeyName( key.key );
		if ( name && name[0] && name[1] == '\0' && name[0] >= 'A' && name[0] <= 'Z' )
			event.m_VirtualKeyCode = SDL_SCANCODE_A + name[0] - 'A';
	}
	m_ModifierMask = ModifierMask( key.mod );
	event.m_ModifierKeyMask = m_ModifierMask;
	PostEvent( event );
}

void CSDL3Mgr::HandleEvent( const SDL_Event &input )
{
	CCocoaEvent event = {};
	switch ( input.type )
	{
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:
		HandleKey( input.key );
		return;
	case SDL_EVENT_TEXT_INPUT:
	{
		const char *text = input.text.text;
		while ( text && *text )
		{
			const Uint32 codepoint = SDL_StepUTF8( &text, NULL );
			event.m_EventType = CocoaEvent_KeyDown;
			event.m_VirtualKeyCode = 0;
			event.m_UnicodeKey = static_cast<wchar_t>( codepoint );
			event.m_UnicodeKeyUnmodified = event.m_UnicodeKey;
			event.m_ModifierKeyMask = m_ModifierMask;
			PostEvent( event );
			event.m_EventType = CocoaEvent_KeyUp;
			event.m_UnicodeKey = event.m_UnicodeKeyUnmodified = 0;
			PostEvent( event );
		}
		return;
	}
	case SDL_EVENT_MOUSE_MOTION:
		if ( !m_bHasFocus )
			return;
		m_MouseDeltaX += input.motion.xrel;
		m_MouseDeltaY += input.motion.yrel;
		event.m_EventType = CocoaEvent_MouseMove;
		MousePosition( event, input.motion.x, input.motion.y );
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
	{
		int button = input.button.button;
		if ( button == SDL_BUTTON_MIDDLE )
			button = 3;
		else if ( button == SDL_BUTTON_RIGHT )
			button = 2;
		else if ( button > 3 )
			button = 4 + ( button & 1 );
		if ( button < 1 )
			return;
		event.m_MouseButton = 1 << ( button - 1 );
		if ( input.button.down )
			m_MouseButtons |= event.m_MouseButton;
		else
			m_MouseButtons &= ~event.m_MouseButton;
		event.m_EventType =
		    input.button.down ? CocoaEvent_MouseButtonDown : CocoaEvent_MouseButtonUp;
		event.m_nMouseClickCount = input.button.clicks;
		MousePosition( event, input.button.x, input.button.y );
		break;
	}
	case SDL_EVENT_MOUSE_WHEEL:
		event.m_EventType = CocoaEvent_MouseScroll;
		event.m_MousePos[0] = event.m_MousePos[1] = static_cast<int>( input.wheel.y );
		if ( input.wheel.direction == SDL_MOUSEWHEEL_FLIPPED )
			event.m_MousePos[0] = event.m_MousePos[1] = -event.m_MousePos[0];
		if ( event.m_MousePos[0] == 0 )
			return;
		break;
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	case SDL_EVENT_WINDOW_FOCUS_LOST:
		m_bHasFocus = input.type == SDL_EVENT_WINDOW_FOCUS_GAINED;
		m_bCursorDirty = true;
		m_MouseDeltaX = m_MouseDeltaY = 0;
		m_MouseButtons = 0;
		m_ModifierMask = 0;
		event.m_EventType = CocoaEvent_AppActivate;
		event.m_ModifierKeyMask = m_bHasFocus ? 1 : 0;
		OnFrameRendered();
		break;
	case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
	case SDL_EVENT_QUIT:
		event.m_EventType = CocoaEvent_AppQuit;
		break;
	default:
		return;
	}
	PostEvent( event );
}

void CSDL3Mgr::PumpWindowsMessageLoop()
{
	SDL_Event event;
	// Check the budget before dequeueing, so event 101 remains pending.
	for ( int count = 0; count < 100 && SDL_PollEvent( &event ); ++count )
	{
		SDL_Window *eventWindow = SDL_GetWindowFromEvent( &event );
		if ( eventWindow != NULL && eventWindow != m_Window )
			continue;
		HandleEvent( event );
	}
}

} // namespace

ILauncherMgr *g_pLauncherMgr = NULL;

// Retained factory spelling for the linked launcher composition. Construction
// has no SDL side effects; the application group's Init/Shutdown owns lifetime.
void *CreateSDLMgr()
{
	static CSDL3Mgr manager;
	g_pLauncherMgr = &manager;
	return g_pLauncherMgr;
}
