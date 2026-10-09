//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL3 window/input provider. See sdl3_window_system.h.
//
//			The native decoding keeps the product's SDL3 launcher
//			(appframework/sdl3mgr.cpp) behavior where the contract leaves room:
//			scancodes are HID usages, modifier identity comes from the keycode,
//			a flipped wheel is reported in the unflipped direction, and text
//			input starts with each window except where it would raise an
//			on-screen keyboard. SDL3 reports pointer positions and motion in
//			fractional window coordinates; positions truncate as the launcher's
//			do, and motion deltas carry their fractions into the next event, so
//			slow relative motion is not lost. Wheel detents use SDL's
//			accumulated whole ticks.
//
//=============================================================================//

#include "sdl3_window_system.h"

#include "../render_surface/sdl3_render_surfaces.h"

#include <SDL3/SDL.h>

#include <atomic>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

namespace platform_sdl3
{

using foundation::Expected;
using foundation::MakeUnexpected;
using foundation::Unexpected;
using namespace platform::window;

namespace
{

// SDL's video and event state is process-global; a second connected provider
// would steal the first one's events.
std::atomic<bool> g_Connected{ false };

Unexpected<WindowError> Fail( WindowStatus status, WindowOperation operation, int native = 0 )
{
	return MakeUnexpected( WindowError{ status, operation, native } );
}

// The key the layout makes this key act as, from its SDL keycode.
std::uint32_t LayoutUsage( SDL_Keycode key )
{
	if ( key >= SDLK_A && key <= SDLK_Z )
		return SDL_SCANCODE_A + std::uint32_t( key - SDLK_A );
	switch ( key )
	{
	case SDLK_EQUALS:
		return SDL_SCANCODE_EQUALS;
	case SDLK_MINUS:
		return SDL_SCANCODE_MINUS;
	case SDLK_LEFTBRACKET:
		return SDL_SCANCODE_LEFTBRACKET;
	case SDLK_RIGHTBRACKET:
		return SDL_SCANCODE_RIGHTBRACKET;
	case SDLK_SEMICOLON:
		return SDL_SCANCODE_SEMICOLON;
	case SDLK_APOSTROPHE:
		return SDL_SCANCODE_APOSTROPHE;
	case SDLK_COMMA:
		return SDL_SCANCODE_COMMA;
	case SDLK_PERIOD:
		return SDL_SCANCODE_PERIOD;
	case SDLK_SLASH:
		return SDL_SCANCODE_SLASH;
	case SDLK_CAPSLOCK:
		return kUsageCapsLock;
	case SDLK_LCTRL:
		return kUsageLeftControl;
	case SDLK_LSHIFT:
		return kUsageLeftShift;
	case SDLK_LALT:
		return kUsageLeftAlt;
	case SDLK_LGUI:
		return kUsageLeftGui;
	case SDLK_RCTRL:
		return kUsageRightControl;
	case SDLK_RSHIFT:
		return kUsageRightShift;
	case SDLK_RALT:
		return kUsageRightAlt;
	case SDLK_RGUI:
		return kUsageRightGui;
	default:
		return 0;
	}
}

bool MapButton( Uint8 button, GamepadButton &out )
{
	static const GamepadButton kMap[] = { GamepadButton::South, GamepadButton::East,
	    GamepadButton::West, GamepadButton::North, GamepadButton::Back, GamepadButton::Guide,
	    GamepadButton::Start, GamepadButton::LeftStick, GamepadButton::RightStick,
	    GamepadButton::LeftShoulder, GamepadButton::RightShoulder, GamepadButton::DpadUp,
	    GamepadButton::DpadDown, GamepadButton::DpadLeft, GamepadButton::DpadRight };
	static_assert( SDL_GAMEPAD_BUTTON_SOUTH == 0 && SDL_GAMEPAD_BUTTON_DPAD_RIGHT == 14 );
	if ( button >= std::size( kMap ) )
		return false;
	out = kMap[button];
	return true;
}

bool MapAxis( Uint8 axis, GamepadAxis &out )
{
	switch ( axis )
	{
	case SDL_GAMEPAD_AXIS_LEFTX:
		out = GamepadAxis::LeftX;
		return true;
	case SDL_GAMEPAD_AXIS_LEFTY:
		out = GamepadAxis::LeftY;
		return true;
	case SDL_GAMEPAD_AXIS_RIGHTX:
		out = GamepadAxis::RightX;
		return true;
	case SDL_GAMEPAD_AXIS_RIGHTY:
		out = GamepadAxis::RightY;
		return true;
	case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
		out = GamepadAxis::LeftTrigger;
		return true;
	case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
		out = GamepadAxis::RightTrigger;
		return true;
	default:
		return false;
	}
}

SDL_SystemCursor SystemCursor( CursorShape shape )
{
	switch ( shape )
	{
	case CursorShape::Arrow:
		return SDL_SYSTEM_CURSOR_DEFAULT;
	case CursorShape::IBeam:
		return SDL_SYSTEM_CURSOR_TEXT;
	case CursorShape::Wait:
		return SDL_SYSTEM_CURSOR_WAIT;
	case CursorShape::WaitArrow:
		return SDL_SYSTEM_CURSOR_PROGRESS;
	case CursorShape::Crosshair:
		return SDL_SYSTEM_CURSOR_CROSSHAIR;
	case CursorShape::SizeNWSE:
		return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
	case CursorShape::SizeNESW:
		return SDL_SYSTEM_CURSOR_NESW_RESIZE;
	case CursorShape::SizeWE:
		return SDL_SYSTEM_CURSOR_EW_RESIZE;
	case CursorShape::SizeNS:
		return SDL_SYSTEM_CURSOR_NS_RESIZE;
	case CursorShape::SizeAll:
		return SDL_SYSTEM_CURSOR_MOVE;
	case CursorShape::No:
		return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
	case CursorShape::Hand:
	default:
		return SDL_SYSTEM_CURSOR_POINTER;
	}
}

std::uint64_t NsToMs( Uint64 ns )
{
	return ns / 1000000u;
}

} // namespace

struct Sdl3WindowSystem::State
{
	struct Window
	{
		WindowId id;
		SDL_Window *native = nullptr;
		SDL_WindowID nativeId = 0;
		render::IRenderSurface *surface = nullptr;
		bool visible = true;
		bool minimized = false;
	};

	struct Pad
	{
		std::uint32_t instance = 0;
		SDL_Gamepad *gamepad = nullptr;
	};

	explicit State( Sdl3WindowSystemConfig config ) : config( config ), normalizer( config.policy )
	{
	}

	Window *Find( WindowId id )
	{
		for ( Window &window : windows )
			if ( window.id == id )
				return &window;
		return nullptr;
	}

	Window *FindNative( SDL_WindowID nativeId )
	{
		for ( Window &window : windows )
			if ( window.nativeId == nativeId )
				return &window;
		return nullptr;
	}

	void Push( EventType type, WindowId window )
	{
		Event event;
		event.type = type;
		event.window = window;
		pending.Push( event );
	}

	void UpdateVisibility( Window &window )
	{
		surfaces.SetVisible( *window.surface, window.visible && !window.minimized );
	}

	void PushSize( EventType type, WindowId id, std::int32_t width, std::int32_t height )
	{
		Event event;
		event.type = type;
		event.window = id;
		event.size = SizeEvent{ width, height };
		pending.Push( event );
	}

	void TranslateWindowEvent( const SDL_WindowEvent &e )
	{
		Window *window = FindNative( e.windowID );
		if ( !window )
			return;
		const WindowId id = window->id;
		switch ( e.type )
		{
		case SDL_EVENT_WINDOW_SHOWN:
			window->visible = true;
			UpdateVisibility( *window );
			Push( EventType::WindowShown, id );
			break;
		case SDL_EVENT_WINDOW_HIDDEN:
			window->visible = false;
			UpdateVisibility( *window );
			Push( EventType::WindowHidden, id );
			break;
		case SDL_EVENT_WINDOW_MINIMIZED:
			window->minimized = true;
			UpdateVisibility( *window );
			Push( EventType::WindowMinimized, id );
			break;
		case SDL_EVENT_WINDOW_RESTORED:
			window->minimized = false;
			UpdateVisibility( *window );
			Push( EventType::WindowRestored, id );
			break;
		case SDL_EVENT_WINDOW_MAXIMIZED:
			window->minimized = false;
			UpdateVisibility( *window );
			break;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			Push( EventType::WindowFocusGained, id );
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			Push( EventType::WindowFocusLost, id );
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			Push( EventType::WindowCloseRequested, id );
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			PushSize( EventType::WindowResized, id, e.data1, e.data2 );
			break;
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			// The surface reads the drawable size from SDL, which has applied it
			// before queueing this event.
			PushSize( EventType::WindowPixelSizeChanged, id, e.data1, e.data2 );
			break;
		default:
			break;
		}
	}

	// Input for a window this provider does not know (destroyed since the event
	// was queued, or foreign) is dropped. Window 0 means no focused window.
	bool Resolve( SDL_WindowID nativeId, WindowId &out )
	{
		if ( nativeId == 0 )
		{
			out = WindowId{};
			return true;
		}
		Window *window = FindNative( nativeId );
		if ( !window )
			return false;
		out = window->id;
		return true;
	}

	// Whole units of 'delta' plus the carried fraction; the rest carries on.
	static std::int32_t Carry( float delta, float &remainder )
	{
		const float total = delta + remainder;
		const float whole = std::trunc( total );
		remainder = total - whole;
		return std::int32_t( whole );
	}

	void Translate( const SDL_Event &e )
	{
		WindowId window;
		switch ( e.type )
		{
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			if ( Resolve( e.key.windowID, window ) )
				normalizer.Key( pending, window, std::uint32_t( e.key.scancode ),
				    LayoutUsage( e.key.key ), e.key.down, e.key.repeat );
			break;
		case SDL_EVENT_TEXT_INPUT:
			if ( e.text.text && Resolve( e.text.windowID, window ) )
				normalizer.Text( pending, window, std::string_view( e.text.text ) );
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if ( Resolve( e.motion.windowID, window ) )
				normalizer.Motion( pending, window, std::int32_t( e.motion.x ),
				    std::int32_t( e.motion.y ), Carry( e.motion.xrel, motionRemainderX ),
				    Carry( e.motion.yrel, motionRemainderY ) );
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if ( Resolve( e.button.windowID, window ) )
				normalizer.Button( pending, window, e.button.button, e.button.down,
				    std::int32_t( e.button.x ), std::int32_t( e.button.y ),
				    NsToMs( e.button.timestamp ) );
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			if ( Resolve( e.wheel.windowID, window ) )
			{
				const std::int32_t sign = e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1;
				if ( e.wheel.integer_x || e.wheel.integer_y )
					normalizer.Wheel(
					    pending, window, sign * e.wheel.integer_x, sign * e.wheel.integer_y );
			}
			break;
		case SDL_EVENT_QUIT:
			Push( EventType::Quit, WindowId{} );
			break;
		case SDL_EVENT_TERMINATING:
			Push( EventType::AppTerminating, WindowId{} );
			break;
		case SDL_EVENT_LOW_MEMORY:
			Push( EventType::AppLowMemory, WindowId{} );
			break;
		case SDL_EVENT_WILL_ENTER_BACKGROUND:
			Push( EventType::AppWillEnterBackground, WindowId{} );
			break;
		case SDL_EVENT_DID_ENTER_BACKGROUND:
			surfaces.HandleEvent( e ); // releases every native surface first
			Push( EventType::AppDidEnterBackground, WindowId{} );
			break;
		case SDL_EVENT_WILL_ENTER_FOREGROUND:
			surfaces.HandleEvent( e ); // restores them with a new generation
			Push( EventType::AppWillEnterForeground, WindowId{} );
			break;
		case SDL_EVENT_DID_ENTER_FOREGROUND:
			Push( EventType::AppDidEnterForeground, WindowId{} );
			break;
		case SDL_EVENT_GAMEPAD_ADDED:
			OpenPad( e.gdevice.which );
			break;
		case SDL_EVENT_GAMEPAD_REMOVED:
			ClosePad( std::uint32_t( e.gdevice.which ) );
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
		{
			Event event;
			if ( !MapButton( e.gbutton.button, event.gamepad.button ) )
				break;
			event.type = e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ? EventType::GamepadButtonDown
			                                                     : EventType::GamepadButtonUp;
			event.gamepad.instance = std::uint32_t( e.gbutton.which );
			pending.Push( event );
			break;
		}
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		{
			Event event;
			if ( !MapAxis( e.gaxis.axis, event.gamepad.axis ) )
				break;
			event.type = EventType::GamepadAxis;
			event.gamepad.instance = std::uint32_t( e.gaxis.which );
			event.gamepad.value =
			    InputNormalizer::NormalizeAxis( event.gamepad.axis, e.gaxis.value );
			pending.Push( event );
			break;
		}
		case SDL_EVENT_FINGER_DOWN:
		case SDL_EVENT_FINGER_UP:
		case SDL_EVENT_FINGER_MOTION:
		{
			if ( !Resolve( e.tfinger.windowID, window ) )
				break;
			Event event;
			event.type = e.type == SDL_EVENT_FINGER_DOWN ? EventType::TouchDown
			             : e.type == SDL_EVENT_FINGER_UP ? EventType::TouchUp
			                                             : EventType::TouchMotion;
			event.window = window;
			event.touch =
			    TouchEvent{ std::uint64_t( e.tfinger.touchID ), std::uint64_t( e.tfinger.fingerID ),
			        e.tfinger.x, e.tfinger.y, e.tfinger.dx, e.tfinger.dy, e.tfinger.pressure };
			pending.Push( event );
			break;
		}
		default:
			if ( e.type >= SDL_EVENT_WINDOW_FIRST && e.type <= SDL_EVENT_WINDOW_LAST )
				TranslateWindowEvent( e.window );
			break;
		}
	}

	void OpenPad( SDL_JoystickID id )
	{
		for ( const Pad &pad : pads )
			if ( pad.instance == std::uint32_t( id ) )
				return; // already open (enumerated at connect, then announced)
		SDL_Gamepad *gamepad = SDL_OpenGamepad( id );
		if ( !gamepad )
			return;
		pads.push_back( Pad{ std::uint32_t( id ), gamepad } );
		Event event;
		event.type = EventType::GamepadAdded;
		event.gamepad.instance = std::uint32_t( id );
		pending.Push( event );
	}

	void ClosePad( std::uint32_t instance )
	{
		for ( std::size_t i = 0; i < pads.size(); ++i )
		{
			if ( pads[i].instance != instance )
				continue;
			SDL_CloseGamepad( pads[i].gamepad );
			pads.erase( pads.begin() + std::ptrdiff_t( i ) );
			Event event;
			event.type = EventType::GamepadRemoved;
			event.gamepad.instance = instance;
			pending.Push( event );
			return;
		}
	}

	SDL_InitFlags SubsystemFlags() const
	{
		return SDL_INIT_VIDEO | ( config.gamepads ? SDL_INIT_GAMEPAD : 0u );
	}

	Sdl3WindowSystemConfig config;
	InputNormalizer normalizer;
	EventFifo pending;
	Sdl3RenderSurfaces surfaces;
	std::vector<Window> windows;
	std::vector<Pad> pads;
	SDL_Cursor *cursors[std::size_t( CursorShape::Count )] = {};
	float motionRemainderX = 0.0f;
	float motionRemainderY = 0.0f;
	std::uint32_t nextId = 1;
	bool connected = false;
};

Sdl3WindowSystem::Sdl3WindowSystem( Sdl3WindowSystemConfig config )
    : m_State( std::make_unique<State>( config ) )
{
}

Sdl3WindowSystem::~Sdl3WindowSystem()
{
	Shutdown();
	Disconnect();
}

Expected<void, platform::ProviderError> Sdl3WindowSystem::Connect()
{
	bool expected = false;
	if ( !g_Connected.compare_exchange_strong( expected, true ) )
		return MakeUnexpected( platform::ProviderError{ platform::ProviderErrorCode::ConnectFailed,
		    "another SDL3 window system is already connected in this process" } );
	SDL_SetHint( SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,
	    m_State->config.gamepadsWithoutFocus ? "1" : "0" );
	if ( !SDL_InitSubSystem( m_State->SubsystemFlags() ) )
	{
		std::string detail = std::string( "SDL_InitSubSystem failed: " ) + SDL_GetError();
		g_Connected.store( false );
		return MakeUnexpected( platform::ProviderError{
		    platform::ProviderErrorCode::ConnectFailed, std::move( detail ) } );
	}
	m_State->connected = true;
	return {};
}

Expected<void, platform::ProviderError> Sdl3WindowSystem::Initialize()
{
	return {};
}

void Sdl3WindowSystem::Shutdown() noexcept
{
	State &s = *m_State;
	for ( State::Window &window : s.windows )
	{
		s.surfaces.InvalidateWindow( *window.surface );
		SDL_DestroyWindow( window.native );
	}
	s.windows.clear();
	s.surfaces.Collect();
	for ( State::Pad &pad : s.pads )
		SDL_CloseGamepad( pad.gamepad );
	s.pads.clear();
	for ( SDL_Cursor *&cursor : s.cursors )
	{
		if ( cursor )
			SDL_DestroyCursor( cursor );
		cursor = nullptr;
	}
	s.pending.Clear();
	s.normalizer.Reset();
	s.motionRemainderX = s.motionRemainderY = 0.0f;
	if ( s.connected )
		SDL_FlushEvents( SDL_EVENT_FIRST, SDL_EVENT_LAST );
}

void Sdl3WindowSystem::Disconnect() noexcept
{
	if ( !m_State->connected )
		return;
	SDL_QuitSubSystem( m_State->SubsystemFlags() );
	m_State->connected = false;
	g_Connected.store( false );
}

Expected<WindowId, WindowError> Sdl3WindowSystem::Create( const WindowDesc &desc )
{
	State &s = *m_State;
	if ( desc.width <= 0 || desc.height <= 0 )
		return Fail( WindowStatus::InvalidArgument, WindowOperation::Create );
	if ( !s.connected )
		return Fail( WindowStatus::NativeFailure, WindowOperation::Create );
	SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if ( desc.hidden )
		flags |= SDL_WINDOW_HIDDEN;
	if ( desc.resizable )
		flags |= SDL_WINDOW_RESIZABLE;
	const std::string title( desc.title );
	SDL_Window *native = SDL_CreateWindow( title.c_str(), desc.width, desc.height, flags );
	if ( !native )
		return Fail( WindowStatus::NativeFailure, WindowOperation::Create );
	// SDL2 delivered text by default; SDL3 starts it per window. Where text
	// input raises an on-screen keyboard, the consumer starts it on demand.
	if ( !SDL_HasScreenKeyboardSupport() )
		SDL_StartTextInput( native );

	State::Window window;
	window.id = WindowId{ s.nextId++ };
	window.native = native;
	window.nativeId = SDL_GetWindowID( native );
	window.surface = s.surfaces.Adopt( native );
	window.visible = !desc.hidden;
	s.windows.push_back( window );
	s.UpdateVisibility( s.windows.back() );
	return window.id;
}

Expected<void, WindowError> Sdl3WindowSystem::Destroy( WindowId id )
{
	State &s = *m_State;
	State::Window *window = s.Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::Destroy );
	// The presentation lets go of the native surface while the window exists;
	// the surface object stays until no presentation holds it (Collect).
	s.surfaces.InvalidateWindow( *window->surface );
	SDL_DestroyWindow( window->native );
	s.windows.erase( s.windows.begin() + ( window - s.windows.data() ) );
	s.normalizer.ForgetWindow( id );
	s.pending.DropWindow( id );
	return {};
}

Expected<SizeEvent, WindowError> Sdl3WindowSystem::GetSize( WindowId id ) const
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::GetSize );
	SizeEvent size;
	if ( !SDL_GetWindowSize( window->native, &size.width, &size.height ) )
		return Fail( WindowStatus::NativeFailure, WindowOperation::GetSize );
	return size;
}

Expected<SizeEvent, WindowError> Sdl3WindowSystem::GetPixelSize( WindowId id ) const
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::GetSize );
	SizeEvent size;
	if ( !SDL_GetWindowSizeInPixels( window->native, &size.width, &size.height ) )
		return Fail( WindowStatus::NativeFailure, WindowOperation::GetSize );
	return size;
}

Expected<void, WindowError> Sdl3WindowSystem::SetSize(
    WindowId id, std::int32_t width, std::int32_t height )
{
	if ( width <= 0 || height <= 0 )
		return Fail( WindowStatus::InvalidArgument, WindowOperation::SetSize );
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::SetSize );
	if ( !SDL_SetWindowSize( window->native, width, height ) )
		return Fail( WindowStatus::NativeFailure, WindowOperation::SetSize );
	return {};
}

Expected<void, WindowError> Sdl3WindowSystem::SetTitle( WindowId id, std::string_view title )
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::SetTitle );
	if ( !SDL_SetWindowTitle( window->native, std::string( title ).c_str() ) )
		return Fail( WindowStatus::NativeFailure, WindowOperation::SetTitle );
	return {};
}

render::IRenderSurface *Sdl3WindowSystem::Surface( WindowId id )
{
	State::Window *window = m_State->Find( id );
	return window ? window->surface : nullptr;
}

std::size_t Sdl3WindowSystem::Poll( std::span<Event> out )
{
	State &s = *m_State;
	s.surfaces.Collect();
	if ( out.empty() )
		return 0;
	std::size_t written = s.pending.Drain( out );
	SDL_Event native;
	// Pull a native event only when every normalized one has been delivered, so
	// the bounded FIFO never overflows.
	while ( written < out.size() && s.pending.IsEmpty() && SDL_PollEvent( &native ) )
	{
		s.Translate( native );
		written += s.pending.Drain( out.subspan( written ) );
	}
	return written;
}

bool Sdl3WindowSystem::Wait( std::uint32_t timeoutMs )
{
	State &s = *m_State;
	const Uint64 start = SDL_GetTicks();
	while ( s.pending.IsEmpty() )
	{
		const Uint64 elapsed = SDL_GetTicks() - start;
		if ( elapsed > timeoutMs )
			return false;
		SDL_Event native;
		if ( !SDL_WaitEventTimeout( &native, Sint32( timeoutMs - elapsed ) ) )
			return !s.pending.IsEmpty();
		s.Translate( native );
	}
	return true;
}

Expected<void, WindowError> Sdl3WindowSystem::SetVisible( bool visible )
{
	if ( !( visible ? SDL_ShowCursor() : SDL_HideCursor() ) )
		return Fail( WindowStatus::NativeFailure, WindowOperation::SetCursorVisible );
	return {};
}

bool Sdl3WindowSystem::IsVisible() const
{
	return SDL_CursorVisible();
}

Expected<void, WindowError> Sdl3WindowSystem::SetRelativeMode( WindowId id, bool enabled )
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::SetRelativeMode );
	if ( !SDL_SetWindowRelativeMouseMode( window->native, enabled ) )
		return Fail( WindowStatus::Unsupported, WindowOperation::SetRelativeMode );
	m_State->motionRemainderX = m_State->motionRemainderY = 0.0f;
	return {};
}

Expected<void, WindowError> Sdl3WindowSystem::Warp( WindowId id, std::int32_t x, std::int32_t y )
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::Warp );
	m_State->normalizer.ExpectWarp( id, x, y );
	SDL_WarpMouseInWindow( window->native, float( x ), float( y ) );
	return {};
}

Expected<void, WindowError> Sdl3WindowSystem::SetShape( CursorShape shape )
{
	if ( shape >= CursorShape::Count )
		return Fail( WindowStatus::InvalidArgument, WindowOperation::SetCursorShape );
	SDL_Cursor *&cursor = m_State->cursors[std::size_t( shape )];
	if ( !cursor )
		cursor = SDL_CreateSystemCursor( SystemCursor( shape ) );
	if ( !cursor || !SDL_SetCursor( cursor ) )
		return Fail( WindowStatus::Unsupported, WindowOperation::SetCursorShape );
	return {};
}

Expected<void, WindowError> Sdl3WindowSystem::SetText( std::string_view utf8 )
{
	if ( !SDL_SetClipboardText( std::string( utf8 ).c_str() ) )
		return Fail( WindowStatus::NativeFailure, WindowOperation::SetClipboard );
	return {};
}

Expected<std::string, WindowError> Sdl3WindowSystem::GetText()
{
	char *text = SDL_GetClipboardText();
	if ( !text )
		return Fail( WindowStatus::NativeFailure, WindowOperation::GetClipboard );
	std::string copy( text );
	SDL_free( text );
	return copy;
}

Expected<void, WindowError> Sdl3WindowSystem::ShowError(
    std::string_view title, std::string_view message, WindowId parent )
{
	SDL_Window *native = nullptr;
	if ( parent.IsValid() )
	{
		State::Window *window = m_State->Find( parent );
		if ( !window )
			return Fail( WindowStatus::UnknownWindow, WindowOperation::ShowMessage );
		native = window->native;
	}
	if ( !SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, std::string( title ).c_str(),
	         std::string( message ).c_str(), native ) )
		return Fail( WindowStatus::NativeFailure, WindowOperation::ShowMessage );
	return {};
}

std::size_t Sdl3WindowSystem::ConnectedCount() const
{
	return m_State->pads.size();
}

Expected<void, WindowError> Sdl3WindowSystem::Rumble(
    std::uint32_t instance, float lowFrequency, float highFrequency, std::uint32_t durationMs )
{
	if ( !( lowFrequency >= 0.0f && lowFrequency <= 1.0f && highFrequency >= 0.0f &&
	         highFrequency <= 1.0f ) )
		return Fail( WindowStatus::InvalidArgument, WindowOperation::Rumble );
	for ( const State::Pad &pad : m_State->pads )
	{
		if ( pad.instance != instance )
			continue;
		if ( !SDL_RumbleGamepad( pad.gamepad, Uint16( lowFrequency * 65535.0f ),
		         Uint16( highFrequency * 65535.0f ), durationMs ) )
			return Fail( WindowStatus::Unsupported, WindowOperation::Rumble );
		return {};
	}
	return Fail( WindowStatus::UnknownDevice, WindowOperation::Rumble );
}

std::uint32_t Sdl3WindowSystem::NativeWindowId( WindowId id ) const
{
	State::Window *window = m_State->Find( id );
	return window ? window->nativeId : 0;
}

Sdl3RenderSurfaces &Sdl3WindowSystem::RenderSurfaces()
{
	return m_State->surfaces;
}

} // namespace platform_sdl3
