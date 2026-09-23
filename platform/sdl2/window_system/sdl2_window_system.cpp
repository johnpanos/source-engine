//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL2 window/input provider. See sdl2_window_system.h.
//
//			The native decoding mirrors appframework/sdlmgr.cpp: scancodes are
//			reported as HID usages, modifier identity comes from the keycode,
//			button numbers and event timestamps are SDL's own, and wheel
//			detents are passed through unscaled.
//
//=============================================================================//

#include "sdl2_window_system.h"

#include "../../window/window_surfaces.h"

#include <SDL.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <string>
#include <vector>

namespace platform_sdl2
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
	if ( key >= SDLK_a && key <= SDLK_z )
		return SDL_SCANCODE_A + std::uint32_t( key - SDLK_a );
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
	case SDLK_QUOTE:
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
	static_assert( SDL_CONTROLLER_BUTTON_A == 0 && SDL_CONTROLLER_BUTTON_DPAD_RIGHT == 14 );
	if ( button >= std::size( kMap ) )
		return false;
	out = kMap[button];
	return true;
}

bool MapAxis( Uint8 axis, GamepadAxis &out )
{
	switch ( axis )
	{
	case SDL_CONTROLLER_AXIS_LEFTX:
		out = GamepadAxis::LeftX;
		return true;
	case SDL_CONTROLLER_AXIS_LEFTY:
		out = GamepadAxis::LeftY;
		return true;
	case SDL_CONTROLLER_AXIS_RIGHTX:
		out = GamepadAxis::RightX;
		return true;
	case SDL_CONTROLLER_AXIS_RIGHTY:
		out = GamepadAxis::RightY;
		return true;
	case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
		out = GamepadAxis::LeftTrigger;
		return true;
	case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
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
		return SDL_SYSTEM_CURSOR_ARROW;
	case CursorShape::IBeam:
		return SDL_SYSTEM_CURSOR_IBEAM;
	case CursorShape::Wait:
		return SDL_SYSTEM_CURSOR_WAIT;
	case CursorShape::WaitArrow:
		return SDL_SYSTEM_CURSOR_WAITARROW;
	case CursorShape::Crosshair:
		return SDL_SYSTEM_CURSOR_CROSSHAIR;
	case CursorShape::SizeNWSE:
		return SDL_SYSTEM_CURSOR_SIZENWSE;
	case CursorShape::SizeNESW:
		return SDL_SYSTEM_CURSOR_SIZENESW;
	case CursorShape::SizeWE:
		return SDL_SYSTEM_CURSOR_SIZEWE;
	case CursorShape::SizeNS:
		return SDL_SYSTEM_CURSOR_SIZENS;
	case CursorShape::SizeAll:
		return SDL_SYSTEM_CURSOR_SIZEALL;
	case CursorShape::No:
		return SDL_SYSTEM_CURSOR_NO;
	case CursorShape::Hand:
	default:
		return SDL_SYSTEM_CURSOR_HAND;
	}
}

render::RenderExtent PixelExtent( SDL_Window *window )
{
	int width = 0, height = 0;
	SDL_GetWindowSizeInPixels( window, &width, &height );
	return render::RenderExtent{ std::uint32_t( width > 0 ? width : 0 ),
		std::uint32_t( height > 0 ? height : 0 ) };
}

} // namespace

struct Sdl2WindowSystem::State
{
	struct Window
	{
		WindowId id;
		SDL_Window *native = nullptr;
		Uint32 nativeId = 0;
		bool visible = true;
		bool minimized = false;
	};

	struct Pad
	{
		std::uint32_t instance = 0;
		SDL_GameController *controller = nullptr;
	};

	explicit State( Sdl2WindowSystemConfig config ) : config( config ), normalizer( config.policy ) {}

	Window *Find( WindowId id )
	{
		for ( Window &window : windows )
			if ( window.id == id )
				return &window;
		return nullptr;
	}

	Window *FindNative( Uint32 nativeId )
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
		surfaces.SetVisible( window.id, window.visible && !window.minimized );
	}

	void TranslateWindowEvent( const SDL_WindowEvent &e )
	{
		Window *window = FindNative( e.windowID );
		if ( !window )
			return;
		const WindowId id = window->id;
		switch ( e.event )
		{
		case SDL_WINDOWEVENT_SHOWN:
			window->visible = true;
			UpdateVisibility( *window );
			Push( EventType::WindowShown, id );
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			window->visible = false;
			UpdateVisibility( *window );
			Push( EventType::WindowHidden, id );
			break;
		case SDL_WINDOWEVENT_MINIMIZED:
			window->minimized = true;
			UpdateVisibility( *window );
			Push( EventType::WindowMinimized, id );
			break;
		case SDL_WINDOWEVENT_RESTORED:
			window->minimized = false;
			UpdateVisibility( *window );
			Push( EventType::WindowRestored, id );
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			window->minimized = false;
			UpdateVisibility( *window );
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			Push( EventType::WindowFocusGained, id );
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			Push( EventType::WindowFocusLost, id );
			break;
		case SDL_WINDOWEVENT_CLOSE:
			Push( EventType::WindowCloseRequested, id );
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		{
			// The surface learns the new drawable size before either event is seen.
			const render::RenderExtent pixels = PixelExtent( window->native );
			surfaces.SetPixelExtent( id, pixels );
			Event resized;
			resized.type = EventType::WindowResized;
			resized.window = id;
			resized.size = SizeEvent{ e.data1, e.data2 };
			pending.Push( resized );
			Event pixel = resized;
			pixel.type = EventType::WindowPixelSizeChanged;
			pixel.size = SizeEvent{ std::int32_t( pixels.width ), std::int32_t( pixels.height ) };
			pending.Push( pixel );
			break;
		}
		default:
			break;
		}
	}

	// Input for a window this provider does not know (destroyed since the event
	// was queued, or foreign) is dropped. Window 0 means no focused window.
	bool Resolve( Uint32 nativeId, WindowId &out )
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

	void Translate( const SDL_Event &e )
	{
		WindowId window;
		switch ( e.type )
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if ( Resolve( e.key.windowID, window ) )
				normalizer.Key( pending, window, std::uint32_t( e.key.keysym.scancode ),
				    LayoutUsage( e.key.keysym.sym ), e.type == SDL_KEYDOWN, e.key.repeat != 0 );
			break;
		case SDL_TEXTINPUT:
			if ( Resolve( e.text.windowID, window ) )
				normalizer.Text( pending, window,
				    std::string_view( e.text.text, strnlen( e.text.text, sizeof( e.text.text ) ) ) );
			break;
		case SDL_MOUSEMOTION:
			if ( Resolve( e.motion.windowID, window ) )
				normalizer.Motion( pending, window, e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel );
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if ( Resolve( e.button.windowID, window ) )
				normalizer.Button( pending, window, e.button.button, e.type == SDL_MOUSEBUTTONDOWN,
				    e.button.x, e.button.y, e.button.timestamp );
			break;
		case SDL_MOUSEWHEEL:
			if ( Resolve( e.wheel.windowID, window ) )
				normalizer.Wheel( pending, window, e.wheel.x, e.wheel.y );
			break;
		case SDL_WINDOWEVENT:
			TranslateWindowEvent( e.window );
			break;
		case SDL_QUIT:
			Push( EventType::Quit, WindowId{} );
			break;
		case SDL_APP_TERMINATING:
			Push( EventType::AppTerminating, WindowId{} );
			break;
		case SDL_APP_LOWMEMORY:
			Push( EventType::AppLowMemory, WindowId{} );
			break;
		case SDL_APP_WILLENTERBACKGROUND:
			Push( EventType::AppWillEnterBackground, WindowId{} );
			break;
		case SDL_APP_DIDENTERBACKGROUND:
			surfaces.ReleaseAll();
			Push( EventType::AppDidEnterBackground, WindowId{} );
			break;
		case SDL_APP_WILLENTERFOREGROUND:
			surfaces.RestoreAll();
			Push( EventType::AppWillEnterForeground, WindowId{} );
			break;
		case SDL_APP_DIDENTERFOREGROUND:
			Push( EventType::AppDidEnterForeground, WindowId{} );
			break;
		case SDL_CONTROLLERDEVICEADDED:
			OpenPad( e.cdevice.which );
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			ClosePad( std::uint32_t( e.cdevice.which ) );
			break;
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
		{
			Event event;
			if ( !MapButton( e.cbutton.button, event.gamepad.button ) )
				break;
			event.type = e.type == SDL_CONTROLLERBUTTONDOWN ? EventType::GamepadButtonDown
			                                                : EventType::GamepadButtonUp;
			event.gamepad.instance = std::uint32_t( e.cbutton.which );
			pending.Push( event );
			break;
		}
		case SDL_CONTROLLERAXISMOTION:
		{
			Event event;
			if ( !MapAxis( e.caxis.axis, event.gamepad.axis ) )
				break;
			event.type = EventType::GamepadAxis;
			event.gamepad.instance = std::uint32_t( e.caxis.which );
			event.gamepad.value = InputNormalizer::NormalizeAxis( event.gamepad.axis, e.caxis.value );
			pending.Push( event );
			break;
		}
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
		{
			if ( !Resolve( e.tfinger.windowID, window ) )
				break;
			Event event;
			event.type = e.type == SDL_FINGERDOWN ? EventType::TouchDown
			             : e.type == SDL_FINGERUP ? EventType::TouchUp
			                                      : EventType::TouchMotion;
			event.window = window;
			event.touch = TouchEvent{ std::uint64_t( e.tfinger.touchId ),
				std::uint64_t( e.tfinger.fingerId ), e.tfinger.x, e.tfinger.y, e.tfinger.dx,
				e.tfinger.dy, e.tfinger.pressure };
			pending.Push( event );
			break;
		}
		default:
			break;
		}
	}

	void OpenPad( int deviceIndex )
	{
		SDL_GameController *controller = SDL_GameControllerOpen( deviceIndex );
		if ( !controller )
			return;
		const std::uint32_t instance =
		    std::uint32_t( SDL_JoystickInstanceID( SDL_GameControllerGetJoystick( controller ) ) );
		pads.push_back( Pad{ instance, controller } );
		Event event;
		event.type = EventType::GamepadAdded;
		event.gamepad.instance = instance;
		pending.Push( event );
	}

	void ClosePad( std::uint32_t instance )
	{
		for ( std::size_t i = 0; i < pads.size(); ++i )
		{
			if ( pads[i].instance != instance )
				continue;
			SDL_GameControllerClose( pads[i].controller );
			pads.erase( pads.begin() + std::ptrdiff_t( i ) );
			Event event;
			event.type = EventType::GamepadRemoved;
			event.gamepad.instance = instance;
			pending.Push( event );
			return;
		}
	}

	Uint32 SubsystemFlags() const
	{
		return SDL_INIT_VIDEO | ( config.gamepads ? SDL_INIT_GAMECONTROLLER : 0u );
	}

	Sdl2WindowSystemConfig config;
	InputNormalizer normalizer;
	EventFifo pending;
	WindowSurfaces surfaces;
	std::vector<Window> windows;
	std::vector<Pad> pads;
	SDL_Cursor *cursors[std::size_t( CursorShape::Count )] = {};
	std::uint32_t nextId = 1;
	bool connected = false;
};

Sdl2WindowSystem::Sdl2WindowSystem( Sdl2WindowSystemConfig config )
    : m_State( std::make_unique<State>( config ) )
{
}

Sdl2WindowSystem::~Sdl2WindowSystem()
{
	Shutdown();
	Disconnect();
}

Expected<void, platform::ProviderError> Sdl2WindowSystem::Connect()
{
	bool expected = false;
	if ( !g_Connected.compare_exchange_strong( expected, true ) )
		return MakeUnexpected( platform::ProviderError{ platform::ProviderErrorCode::ConnectFailed,
		    "another SDL2 window system is already connected in this process" } );
	if ( SDL_InitSubSystem( m_State->SubsystemFlags() ) != 0 )
	{
		std::string detail = std::string( "SDL_InitSubSystem failed: " ) + SDL_GetError();
		g_Connected.store( false );
		return MakeUnexpected(
		    platform::ProviderError{ platform::ProviderErrorCode::ConnectFailed, std::move( detail ) } );
	}
	m_State->connected = true;
	return {};
}

Expected<void, platform::ProviderError> Sdl2WindowSystem::Initialize()
{
	return {};
}

void Sdl2WindowSystem::Shutdown() noexcept
{
	State &s = *m_State;
	for ( State::Window &window : s.windows )
	{
		s.surfaces.Invalidate( window.id );
		SDL_DestroyWindow( window.native );
	}
	s.windows.clear();
	for ( State::Pad &pad : s.pads )
		SDL_GameControllerClose( pad.controller );
	s.pads.clear();
	for ( SDL_Cursor *&cursor : s.cursors )
	{
		if ( cursor )
			SDL_FreeCursor( cursor );
		cursor = nullptr;
	}
	s.pending.Clear();
	s.normalizer.Reset();
	if ( s.connected )
		SDL_FlushEvents( SDL_FIRSTEVENT, SDL_LASTEVENT );
}

void Sdl2WindowSystem::Disconnect() noexcept
{
	if ( !m_State->connected )
		return;
	SDL_QuitSubSystem( m_State->SubsystemFlags() );
	m_State->connected = false;
	g_Connected.store( false );
}

Expected<WindowId, WindowError> Sdl2WindowSystem::Create( const WindowDesc &desc )
{
	State &s = *m_State;
	if ( desc.width <= 0 || desc.height <= 0 )
		return Fail( WindowStatus::InvalidArgument, WindowOperation::Create );
	if ( !s.connected )
		return Fail( WindowStatus::NativeFailure, WindowOperation::Create );
	Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | ( desc.hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN );
	if ( desc.resizable )
		flags |= SDL_WINDOW_RESIZABLE;
	const std::string title( desc.title );
	SDL_Window *native = SDL_CreateWindow( title.c_str(), SDL_WINDOWPOS_UNDEFINED,
	    SDL_WINDOWPOS_UNDEFINED, desc.width, desc.height, flags );
	if ( !native )
		return Fail( WindowStatus::NativeFailure, WindowOperation::Create );

	State::Window window;
	window.id = WindowId{ s.nextId++ };
	window.native = native;
	window.nativeId = SDL_GetWindowID( native );
	window.visible = !desc.hidden;
	s.windows.push_back( window );
	s.surfaces.Add( window.id, PixelExtent( native ), window.visible );
	return window.id;
}

Expected<void, WindowError> Sdl2WindowSystem::Destroy( WindowId id )
{
	State &s = *m_State;
	State::Window *window = s.Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::Destroy );
	// The presentation lets go of the native surface while the window exists.
	s.surfaces.Invalidate( id );
	SDL_DestroyWindow( window->native );
	s.windows.erase( s.windows.begin() + ( window - s.windows.data() ) );
	s.normalizer.ForgetWindow( id );
	s.pending.DropWindow( id );
	return {};
}

Expected<SizeEvent, WindowError> Sdl2WindowSystem::GetSize( WindowId id ) const
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::GetSize );
	SizeEvent size;
	SDL_GetWindowSize( window->native, &size.width, &size.height );
	return size;
}

Expected<SizeEvent, WindowError> Sdl2WindowSystem::GetPixelSize( WindowId id ) const
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::GetSize );
	SizeEvent size;
	SDL_GetWindowSizeInPixels( window->native, &size.width, &size.height );
	return size;
}

Expected<void, WindowError> Sdl2WindowSystem::SetSize(
    WindowId id, std::int32_t width, std::int32_t height )
{
	if ( width <= 0 || height <= 0 )
		return Fail( WindowStatus::InvalidArgument, WindowOperation::SetSize );
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::SetSize );
	SDL_SetWindowSize( window->native, width, height );
	return {};
}

Expected<void, WindowError> Sdl2WindowSystem::SetTitle( WindowId id, std::string_view title )
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::SetTitle );
	SDL_SetWindowTitle( window->native, std::string( title ).c_str() );
	return {};
}

render::IRenderSurface *Sdl2WindowSystem::Surface( WindowId id )
{
	return m_State->surfaces.Find( id );
}

std::size_t Sdl2WindowSystem::Poll( std::span<Event> out )
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

bool Sdl2WindowSystem::Wait( std::uint32_t timeoutMs )
{
	State &s = *m_State;
	const Uint32 start = SDL_GetTicks();
	while ( s.pending.IsEmpty() )
	{
		const Uint32 elapsed = SDL_GetTicks() - start;
		if ( elapsed > timeoutMs )
			return false;
		SDL_Event native;
		if ( !SDL_WaitEventTimeout( &native, int( timeoutMs - elapsed ) ) )
			return !s.pending.IsEmpty();
		s.Translate( native );
	}
	return true;
}

Expected<void, WindowError> Sdl2WindowSystem::SetVisible( bool visible )
{
	if ( SDL_ShowCursor( visible ? SDL_ENABLE : SDL_DISABLE ) < 0 )
		return Fail( WindowStatus::NativeFailure, WindowOperation::SetCursorVisible );
	return {};
}

bool Sdl2WindowSystem::IsVisible() const
{
	return SDL_ShowCursor( SDL_QUERY ) == SDL_ENABLE;
}

Expected<void, WindowError> Sdl2WindowSystem::SetRelativeMode( WindowId id, bool enabled )
{
	if ( !m_State->Find( id ) )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::SetRelativeMode );
	if ( SDL_SetRelativeMouseMode( enabled ? SDL_TRUE : SDL_FALSE ) != 0 )
		return Fail( WindowStatus::Unsupported, WindowOperation::SetRelativeMode );
	return {};
}

Expected<void, WindowError> Sdl2WindowSystem::Warp( WindowId id, std::int32_t x, std::int32_t y )
{
	State::Window *window = m_State->Find( id );
	if ( !window )
		return Fail( WindowStatus::UnknownWindow, WindowOperation::Warp );
	m_State->normalizer.ExpectWarp( id, x, y );
	SDL_WarpMouseInWindow( window->native, x, y );
	return {};
}

Expected<void, WindowError> Sdl2WindowSystem::SetShape( CursorShape shape )
{
	if ( shape >= CursorShape::Count )
		return Fail( WindowStatus::InvalidArgument, WindowOperation::SetCursorShape );
	SDL_Cursor *&cursor = m_State->cursors[std::size_t( shape )];
	if ( !cursor )
		cursor = SDL_CreateSystemCursor( SystemCursor( shape ) );
	if ( !cursor )
		return Fail( WindowStatus::Unsupported, WindowOperation::SetCursorShape );
	SDL_SetCursor( cursor );
	return {};
}

Expected<void, WindowError> Sdl2WindowSystem::SetText( std::string_view utf8 )
{
	if ( SDL_SetClipboardText( std::string( utf8 ).c_str() ) != 0 )
		return Fail( WindowStatus::NativeFailure, WindowOperation::SetClipboard );
	return {};
}

Expected<std::string, WindowError> Sdl2WindowSystem::GetText()
{
	char *text = SDL_GetClipboardText();
	if ( !text )
		return Fail( WindowStatus::NativeFailure, WindowOperation::GetClipboard );
	std::string copy( text );
	SDL_free( text );
	return copy;
}

Expected<void, WindowError> Sdl2WindowSystem::ShowError(
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
	if ( SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, std::string( title ).c_str(),
	         std::string( message ).c_str(), native ) != 0 )
		return Fail( WindowStatus::NativeFailure, WindowOperation::ShowMessage );
	return {};
}

std::size_t Sdl2WindowSystem::ConnectedCount() const
{
	return m_State->pads.size();
}

Expected<void, WindowError> Sdl2WindowSystem::Rumble(
    std::uint32_t instance, float lowFrequency, float highFrequency, std::uint32_t durationMs )
{
	if ( !( lowFrequency >= 0.0f && lowFrequency <= 1.0f && highFrequency >= 0.0f &&
	         highFrequency <= 1.0f ) )
		return Fail( WindowStatus::InvalidArgument, WindowOperation::Rumble );
	for ( const State::Pad &pad : m_State->pads )
	{
		if ( pad.instance != instance )
			continue;
		if ( SDL_GameControllerRumble( pad.controller, Uint16( lowFrequency * 65535.0f ),
		         Uint16( highFrequency * 65535.0f ), durationMs ) != 0 )
			return Fail( WindowStatus::Unsupported, WindowOperation::Rumble );
		return {};
	}
	return Fail( WindowStatus::UnknownDevice, WindowOperation::Rumble );
}

std::uint32_t Sdl2WindowSystem::NativeWindowId( WindowId id ) const
{
	State::Window *window = m_State->Find( id );
	return window ? window->nativeId : 0;
}

} // namespace platform_sdl2
