//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native conformance suite for the SDL3 window provider
//			(platform.window.v1, RFC 0001 rank 7, roadmap R14). Runs the SAME
//			shared window/input suite as the headless provider. Native input is
//			injected as SDL3 events with SDL_PushEvent, so the provider's real
//			decoding path runs; gamepads are SDL virtual gamepads on SDL's real
//			device path; window size, surfaces, cursor and clipboard use the
//			real video driver.
//
//			The provider is composed through the R06 kernel, run twice (repeat
//			instance) and checked for the one-connected-instance rule. Then the
//			product-behavior clauses the SDL3 launcher (appframework/sdl3mgr.cpp)
//			already had: a flipped wheel reads unflipped, fractional relative
//			motion is carried rather than lost, and a destroyed window's surface
//			outlives it while a presentation holds it.
//
//			SDL_VIDEODRIVER defaults to "offscreen", which needs no display. Run
//			the wayland and x11 profiles only inside an isolated compositor,
//			never on a live desktop session.
//
//=============================================================================//

#include "../../../platform/sdl3/render_surface/sdl3_render_surfaces.h"
#include "../../../platform/sdl3/window_system/sdl3_window_system.h"
#include "../window/window_conformance.h"
#include "platform/composition.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace
{

using namespace platformtest;
using foundation::Expected;
using platform::ApplicationComposition;
using platform::DependencyView;
using platform::ProviderDescriptor;
using platform::ProviderError;
using platform_sdl3::Sdl3WindowSystem;

int g_Checks = 0;
int g_Failures = 0;

void Check( const char *name, bool ok )
{
	++g_Checks;
	if ( !ok )
	{
		++g_Failures;
		std::printf( "FAIL %s: %s\n", name, SDL_GetError() );
	}
}

SDL_Keycode KeycodeFor( std::uint32_t usage )
{
	if ( usage >= SDL_SCANCODE_A && usage <= SDL_SCANCODE_Z )
		return SDLK_A + SDL_Keycode( usage - SDL_SCANCODE_A );
	switch ( usage )
	{
	case kUsageCapsLock:
		return SDLK_CAPSLOCK;
	case kUsageLeftControl:
		return SDLK_LCTRL;
	case kUsageLeftShift:
		return SDLK_LSHIFT;
	case kUsageLeftAlt:
		return SDLK_LALT;
	case kUsageLeftGui:
		return SDLK_LGUI;
	case kUsageRightControl:
		return SDLK_RCTRL;
	case kUsageRightShift:
		return SDLK_RSHIFT;
	case kUsageRightAlt:
		return SDLK_RALT;
	case kUsageRightGui:
		return SDLK_RGUI;
	default:
		return SDL_SCANCODE_TO_KEYCODE( SDL_Scancode( usage ) );
	}
}

class Sdl3Driver final : public IWindowTestDriver
{
public:
	explicit Sdl3Driver( Sdl3WindowSystem &system ) : m_System( system ) {}
	~Sdl3Driver() override
	{
		if ( m_Pad )
			DetachPad( 0 );
	}

	bool Key(
	    WindowId window, std::uint32_t usage, std::uint32_t layoutUsage, bool pressed ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = pressed ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
		e.key.windowID = m_System.NativeWindowId( window );
		e.key.down = pressed;
		e.key.scancode = SDL_Scancode( usage );
		e.key.key = KeycodeFor( layoutUsage ? layoutUsage : usage );
		return Push( e );
	}

	bool Text( WindowId window, std::string_view utf8 ) override
	{
		// The queued event points at this text until the provider polls it.
		m_Texts.emplace_back( utf8 );
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = SDL_EVENT_TEXT_INPUT;
		e.text.windowID = m_System.NativeWindowId( window );
		e.text.text = m_Texts.back().c_str();
		return Push( e );
	}

	bool Button( WindowId window, std::uint32_t button, bool pressed, std::int32_t x,
	    std::int32_t y ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = pressed ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
		e.button.timestamp = SDL_GetTicksNS();
		e.button.windowID = m_System.NativeWindowId( window );
		e.button.button = Uint8( button );
		e.button.down = pressed;
		e.button.clicks = 1;
		e.button.x = float( x );
		e.button.y = float( y );
		return Push( e );
	}

	bool Motion(
	    WindowId window, std::int32_t x, std::int32_t y, std::int32_t dx, std::int32_t dy ) override
	{
		return MotionF( window, float( x ), float( y ), float( dx ), float( dy ) );
	}

	bool MotionF( WindowId window, float x, float y, float dx, float dy )
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = SDL_EVENT_MOUSE_MOTION;
		e.motion.windowID = m_System.NativeWindowId( window );
		e.motion.x = x;
		e.motion.y = y;
		e.motion.xrel = dx;
		e.motion.yrel = dy;
		return Push( e );
	}

	bool Wheel( WindowId window, std::int32_t x, std::int32_t y ) override
	{
		return WheelDirected( window, x, y, false );
	}

	// A flipped ("natural") wheel reports the opposite of the finger's motion.
	bool WheelDirected( WindowId window, std::int32_t x, std::int32_t y, bool flipped )
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = SDL_EVENT_MOUSE_WHEEL;
		e.wheel.windowID = m_System.NativeWindowId( window );
		const std::int32_t sign = flipped ? -1 : 1;
		e.wheel.x = float( sign * x );
		e.wheel.y = float( sign * y );
		e.wheel.integer_x = sign * x;
		e.wheel.integer_y = sign * y;
		e.wheel.direction = flipped ? SDL_MOUSEWHEEL_FLIPPED : SDL_MOUSEWHEEL_NORMAL;
		return Push( e );
	}

	bool Window( WindowId window, NativeWindowEvent event ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.window.windowID = m_System.NativeWindowId( window );
		switch ( event )
		{
		case NativeWindowEvent::Shown:
			e.type = SDL_EVENT_WINDOW_SHOWN;
			break;
		case NativeWindowEvent::Hidden:
			e.type = SDL_EVENT_WINDOW_HIDDEN;
			break;
		case NativeWindowEvent::Minimized:
			e.type = SDL_EVENT_WINDOW_MINIMIZED;
			break;
		case NativeWindowEvent::Restored:
			e.type = SDL_EVENT_WINDOW_RESTORED;
			break;
		case NativeWindowEvent::FocusGained:
			e.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
			break;
		case NativeWindowEvent::FocusLost:
			e.type = SDL_EVENT_WINDOW_FOCUS_LOST;
			break;
		case NativeWindowEvent::CloseRequested:
			e.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
			break;
		}
		return Push( e );
	}

	bool App( NativeAppEvent event ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		switch ( event )
		{
		case NativeAppEvent::Quit:
			e.type = SDL_EVENT_QUIT;
			break;
		case NativeAppEvent::Terminating:
			e.type = SDL_EVENT_TERMINATING;
			break;
		case NativeAppEvent::LowMemory:
			e.type = SDL_EVENT_LOW_MEMORY;
			break;
		case NativeAppEvent::WillEnterBackground:
			e.type = SDL_EVENT_WILL_ENTER_BACKGROUND;
			break;
		case NativeAppEvent::DidEnterBackground:
			e.type = SDL_EVENT_DID_ENTER_BACKGROUND;
			break;
		case NativeAppEvent::WillEnterForeground:
			e.type = SDL_EVENT_WILL_ENTER_FOREGROUND;
			break;
		case NativeAppEvent::DidEnterForeground:
			e.type = SDL_EVENT_DID_ENTER_FOREGROUND;
			break;
		}
		return Push( e );
	}

	// Gamepads are SDL virtual gamepads, so input travels SDL's real device
	// path: the provider sees GAMEPAD_ADDED, opens the gamepad and receives
	// mapped button and axis events.
	bool AttachPad( std::uint32_t &instance ) override
	{
		SDL_VirtualJoystickDesc desc;
		SDL_INIT_INTERFACE( &desc );
		desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
		desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
		desc.nbuttons = SDL_GAMEPAD_BUTTON_DPAD_RIGHT + 1;
		desc.name = "platform.window virtual gamepad";
		const SDL_JoystickID id = SDL_AttachVirtualJoystick( &desc );
		if ( !id )
			return false;
		m_Pad = SDL_OpenJoystick( id );
		if ( !m_Pad )
		{
			SDL_DetachVirtualJoystick( id );
			return false;
		}
		m_PadId = id;
		instance = std::uint32_t( id );
		// A virtual trigger rests at half travel (raw 0); release both before the
		// provider opens the gamepad, so opening reports no axis away from rest.
		SDL_SetJoystickVirtualAxis( m_Pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, -32768 );
		SDL_SetJoystickVirtualAxis( m_Pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, -32768 );
		SDL_UpdateJoysticks();
		return true;
	}

	bool DetachPad( std::uint32_t ) override
	{
		if ( !m_Pad )
			return false;
		SDL_CloseJoystick( m_Pad );
		m_Pad = nullptr;
		return SDL_DetachVirtualJoystick( m_PadId );
	}

	bool PadButton( std::uint32_t, GamepadButton button, bool pressed ) override
	{
		// The virtual gamepad maps button i to SDL_GamepadButton i, the same
		// order as GamepadButton.
		if ( !m_Pad || !SDL_SetJoystickVirtualButton( m_Pad, int( button ), pressed ) )
			return false;
		SDL_UpdateJoysticks();
		if ( button == GamepadButton::Guide && !pressed )
		{
			// SDL holds a released Guide button down for a minimum time (250 ms
			// in SDL 3.4) so a quick tap is still seen; the release follows.
			SDL_Delay( 300 );
			SDL_UpdateJoysticks();
		}
		return true;
	}

	bool PadAxis( std::uint32_t, GamepadAxis axis, std::int32_t raw ) override
	{
		// SDL maps a virtual trigger axis v from the full range onto [0, 32767]
		// as (v + 32768) * 32767 / 65535, truncated; this is its inverse.
		std::int64_t value = raw;
		if ( axis == GamepadAxis::LeftTrigger || axis == GamepadAxis::RightTrigger )
			value = ( std::int64_t( raw ) * 65535 + 32766 ) / 32767 - 32768;
		value = std::clamp<std::int64_t>( value, -32768, 32767 );
		if ( !m_Pad || !SDL_SetJoystickVirtualAxis( m_Pad, int( axis ), Sint16( value ) ) )
			return false;
		SDL_UpdateJoysticks();
		return true;
	}

	bool Touch( NativeTouch kind, WindowId window, std::uint64_t device, std::uint64_t finger,
	    float x, float y, float dx, float dy, float pressure ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = kind == NativeTouch::Down ? SDL_EVENT_FINGER_DOWN
		         : kind == NativeTouch::Up ? SDL_EVENT_FINGER_UP
		                                   : SDL_EVENT_FINGER_MOTION;
		e.tfinger.touchID = SDL_TouchID( device );
		e.tfinger.fingerID = SDL_FingerID( finger );
		e.tfinger.x = x;
		e.tfinger.y = y;
		e.tfinger.dx = dx;
		e.tfinger.dy = dy;
		e.tfinger.pressure = pressure;
		e.tfinger.windowID = m_System.NativeWindowId( window );
		return Push( e );
	}

	void AdvanceMs( std::uint32_t ms ) override { SDL_Delay( ms ); }
	std::uint32_t SettleMs() const override { return m_SettleMs; }
	bool AutoDismissesMessages() const override { return false; }
	std::string LastMessage() const override { return std::string(); }

	std::uint32_t m_SettleMs = 150;

private:
	static bool Push( SDL_Event &e ) { return SDL_PushEvent( &e ); }

	Sdl3WindowSystem &m_System;
	std::deque<std::string> m_Texts;
	SDL_Joystick *m_Pad = nullptr;
	SDL_JoystickID m_PadId = 0;
};

// Drains every pending event of 'type' (after the platform settles).
std::vector<Event> Collect( IEventSource &events, Sdl3Driver &driver, EventType type )
{
	driver.AdvanceMs( 20 );
	std::vector<Event> found;
	Event batch[32];
	for ( std::size_t n; ( n = events.Poll( batch ) ) != 0; )
		for ( std::size_t i = 0; i < n; ++i )
			if ( batch[i].type == type )
				found.push_back( batch[i] );
	return found;
}

class HoldingPresentation final : public render::IRenderSurfaceListener
{
public:
	void OnNativeSurfaceReleasing() override { ++calls; }
	int calls = 0;
};

// The SDL3 launcher's behavior (appframework/sdl3mgr.cpp) that the provider
// keeps, and the surface lifetime the SDL3 bridges rely on.
void ProductClauses( const WindowCapabilities &caps, Sdl3WindowSystem &system )
{
	Sdl3Driver driver( system );
	auto created = caps.windows->Create( WindowDesc{ "sdl3 product clauses", 320, 200 } );
	Check( "sdl3.product.create", !!created );
	if ( !created )
		return;
	const WindowId window = created.Value();
	Collect( *caps.events, driver, EventType::None ); // platform window events

	// A natural-scrolling wheel reads in the contract's direction, as sdl3mgr
	// negates SDL_MOUSEWHEEL_FLIPPED.
	driver.WheelDirected( window, 0, 2, true );
	driver.WheelDirected( window, 0, -1, false );
	const std::vector<Event> wheels = Collect( *caps.events, driver, EventType::MouseWheel );
	Check( "sdl3.product.wheel_flipped_unflipped",
	    wheels.size() == 2 && wheels[0].wheel.y == 2 && wheels[1].wheel.y == -1 );

	// Sub-pixel relative motion is carried, not truncated away: four 0.25 steps
	// make one unit; the positions truncate like the launcher's.
	for ( int i = 0; i < 4; ++i )
		driver.MotionF( window, 10.75f, 20.5f, 0.25f, -0.25f );
	const std::vector<Event> moves = Collect( *caps.events, driver, EventType::MouseMotion );
	int dx = 0, dy = 0;
	bool positions = moves.size() == 4;
	for ( const Event &move : moves )
	{
		dx += move.mouse.dx;
		dy += move.mouse.dy;
		positions = positions && move.mouse.x == 10 && move.mouse.y == 20;
	}
	Check( "sdl3.product.motion_fraction_carried", positions && dx == 1 && dy == -1 );

	// The bridges' endpoint owns the surface: a destroyed window's surface tells
	// its presentation, stays allocated while held, and is reclaimed after.
	render::IRenderSurface *surface = caps.windows->Surface( window );
	platform_sdl3::Sdl3RenderSurfaces &surfaces = system.RenderSurfaces();
	Check( "sdl3.product.bridge_owns_surface",
	    surface && surfaces.Owns( *surface ) && surfaces.NativeWindow( *surface ) != nullptr );
	if ( !surface )
		return;
	HoldingPresentation presentation;
	surface->AttachListener( presentation );
	const std::size_t before = surfaces.AllocatedCount();
	Check( "sdl3.product.destroy", !!caps.windows->Destroy( window ) );
	Event none[4];
	caps.events->Poll( none );
	Check( "sdl3.product.surface_held_after_destroy",
	    presentation.calls == 1 && surfaces.AllocatedCount() == before &&
	        surface->GetStatus() == render::RenderSurfaceStatus::kDestroyed &&
	        surfaces.NativeWindow( *surface ) == nullptr );
	surface->DetachListener( presentation );
	caps.events->Poll( none );
	Check( "sdl3.product.surface_reclaimed", surfaces.AllocatedCount() == before - 1 );
}

struct Observed
{
	Sdl3WindowSystem *system = nullptr;
	int constructed = 0;
};

ProviderDescriptor DescribeSdl3( Observed &observed )
{
	return ProviderDescriptor::Define<Sdl3WindowSystem, IWindowSystem, IEventSource, ICursor,
	    IClipboard, IMessageBox, IGamepads>( "sdl3-window-system", {},
	    [&observed](
	        const DependencyView & ) -> Expected<std::unique_ptr<Sdl3WindowSystem>, ProviderError>
	    {
		    // A private compositor gives the suite's windows no focus; the suite
		    // judges gamepad normalization, not the focus policy.
		    platform_sdl3::Sdl3WindowSystemConfig config;
		    config.gamepadsWithoutFocus = true;
		    auto system = std::make_unique<Sdl3WindowSystem>( config );
		    observed.system = system.get();
		    ++observed.constructed;
		    return system;
	    } );
}

void RunInstance( ApplicationComposition &root, Observed &observed, int run )
{
	Check( "sdl3.start", !!root.Start() );
	WindowCapabilities caps;
	caps.windows = root.Find<IWindowSystem>();
	caps.events = root.Find<IEventSource>();
	caps.cursor = root.Find<ICursor>();
	caps.clipboard = root.Find<IClipboard>();
	caps.messageBox = root.Find<IMessageBox>();
	caps.gamepads = root.Find<IGamepads>();
	if ( !caps.windows || !observed.system )
	{
		Check( "sdl3.published", false );
		return;
	}

	// One connected SDL2 window system per process.
	Sdl3WindowSystem second;
	auto connected = second.Connect();
	Check( "sdl3.single_instance", !connected );

	Sdl3Driver driver( *observed.system );
	WindowSuiteOptions options;
	options.trace = std::getenv( "PLATFORM_WINDOW_TRACE" ) != nullptr;
	const WindowReport report = RunWindowConformance( caps, driver, options );
	g_Checks += report.checks;
	g_Failures += report.failures;
	std::printf( "%s platform.window[sdl3 %s, run %d]: %d checks, %d failures, %d skips\n",
	    report.failures ? "FAIL" : "ok", SDL_GetCurrentVideoDriver(), run, report.checks,
	    report.failures, report.skipped );
	for ( const std::string &skip : report.skips )
		std::printf( "  recorded skip: %s\n", skip.c_str() );
	ProductClauses( caps, *observed.system );
	Check( "sdl3.stop", !!root.Stop() );
	Check( "sdl3.released_instance", observed.constructed == run );
}

} // namespace

int main()
{
	// Never open windows on a live desktop by accident.
	if ( !std::getenv( "SDL_VIDEODRIVER" ) )
		setenv( "SDL_VIDEODRIVER", "offscreen", 1 );

	const int version = SDL_GetVersion();
	std::printf( "SDL %d.%d.%d, video driver request %s\n", SDL_VERSIONNUM_MAJOR( version ),
	    SDL_VERSIONNUM_MINOR( version ), SDL_VERSIONNUM_MICRO( version ),
	    std::getenv( "SDL_VIDEODRIVER" ) );

	Observed observed;
	{
		ApplicationComposition root;
		Check( "sdl3.add_provider", !!root.AddProvider( DescribeSdl3( observed ) ) );
		RunInstance( root, observed, 1 );
		RunInstance( root, observed, 2 );
	}

	// After the composition is gone the process-wide slot is free again.
	{
		Sdl3WindowSystem again;
		auto connected = again.Connect();
		Check( "sdl3.reconnect_after_teardown", !!connected );
		again.Disconnect();
	}

	std::printf( "%s test_sdl3_window: %d checks, %d failures\n", g_Failures ? "FAIL" : "ok",
	    g_Checks, g_Failures );
	return testing::ReportConformance( g_Checks, g_Failures );
}
