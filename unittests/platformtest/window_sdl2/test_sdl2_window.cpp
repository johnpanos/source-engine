//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native conformance suite for the SDL2 window provider
//			(platform.window.v1, RFC 0001 rank 7, roadmap R14). Runs the SAME
//			shared window/input suite as the headless provider. Native input is
//			injected as SDL2 events with SDL_PushEvent, so the provider's real
//			decoding path runs; window size, surfaces, cursor and clipboard use
//			the real video driver.
//
//			The provider is composed through the R06 kernel, run twice (repeat
//			instance) and checked for the one-connected-instance rule.
//
//			SDL_VIDEODRIVER defaults to "offscreen", which needs no display. Run
//			the wayland and x11 profiles only inside an isolated compositor,
//			never on a live desktop session.
//
//=============================================================================//

#include "../../../platform/sdl2/window_system/sdl2_window_system.h"
#include "../window/window_conformance.h"
#include "platform/composition.h"
#include "testing/conformance_result.h"

#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace
{

using namespace platformtest;
using foundation::Expected;
using platform::ApplicationComposition;
using platform::DependencyView;
using platform::ProviderDescriptor;
using platform::ProviderError;
using platform_sdl2::Sdl2WindowSystem;

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
		return SDLK_a + SDL_Keycode( usage - SDL_SCANCODE_A );
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

class Sdl2Driver final : public IWindowTestDriver
{
public:
	explicit Sdl2Driver( Sdl2WindowSystem &system ) : m_System( system ) {}

	bool Key( WindowId window, std::uint32_t usage, std::uint32_t layoutUsage, bool pressed ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
		e.key.windowID = m_System.NativeWindowId( window );
		e.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
		e.key.keysym.scancode = SDL_Scancode( usage );
		e.key.keysym.sym = KeycodeFor( layoutUsage ? layoutUsage : usage );
		return Push( e );
	}

	bool Text( WindowId window, std::string_view utf8 ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = SDL_TEXTINPUT;
		e.text.windowID = m_System.NativeWindowId( window );
		std::memcpy( e.text.text, utf8.data(), std::min( utf8.size(), sizeof( e.text.text ) - 1 ) );
		return Push( e );
	}

	bool Button( WindowId window, std::uint32_t button, bool pressed, std::int32_t x,
	    std::int32_t y ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = pressed ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
		e.button.windowID = m_System.NativeWindowId( window );
		e.button.button = Uint8( button );
		e.button.state = pressed ? SDL_PRESSED : SDL_RELEASED;
		e.button.clicks = 1;
		e.button.x = x;
		e.button.y = y;
		return Push( e );
	}

	bool Motion( WindowId window, std::int32_t x, std::int32_t y, std::int32_t dx,
	    std::int32_t dy ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = SDL_MOUSEMOTION;
		e.motion.windowID = m_System.NativeWindowId( window );
		e.motion.x = x;
		e.motion.y = y;
		e.motion.xrel = dx;
		e.motion.yrel = dy;
		return Push( e );
	}

	bool Wheel( WindowId window, std::int32_t x, std::int32_t y ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = SDL_MOUSEWHEEL;
		e.wheel.windowID = m_System.NativeWindowId( window );
		e.wheel.x = x;
		e.wheel.y = y;
		e.wheel.direction = SDL_MOUSEWHEEL_NORMAL;
		return Push( e );
	}

	bool Window( WindowId window, NativeWindowEvent event ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = SDL_WINDOWEVENT;
		e.window.windowID = m_System.NativeWindowId( window );
		switch ( event )
		{
		case NativeWindowEvent::Shown:
			e.window.event = SDL_WINDOWEVENT_SHOWN;
			break;
		case NativeWindowEvent::Hidden:
			e.window.event = SDL_WINDOWEVENT_HIDDEN;
			break;
		case NativeWindowEvent::Minimized:
			e.window.event = SDL_WINDOWEVENT_MINIMIZED;
			break;
		case NativeWindowEvent::Restored:
			e.window.event = SDL_WINDOWEVENT_RESTORED;
			break;
		case NativeWindowEvent::FocusGained:
			e.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
			break;
		case NativeWindowEvent::FocusLost:
			e.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
			break;
		case NativeWindowEvent::CloseRequested:
			e.window.event = SDL_WINDOWEVENT_CLOSE;
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
			e.type = SDL_QUIT;
			break;
		case NativeAppEvent::Terminating:
			e.type = SDL_APP_TERMINATING;
			break;
		case NativeAppEvent::LowMemory:
			e.type = SDL_APP_LOWMEMORY;
			break;
		case NativeAppEvent::WillEnterBackground:
			e.type = SDL_APP_WILLENTERBACKGROUND;
			break;
		case NativeAppEvent::DidEnterBackground:
			e.type = SDL_APP_DIDENTERBACKGROUND;
			break;
		case NativeAppEvent::WillEnterForeground:
			e.type = SDL_APP_WILLENTERFOREGROUND;
			break;
		case NativeAppEvent::DidEnterForeground:
			e.type = SDL_APP_DIDENTERFOREGROUND;
			break;
		}
		return Push( e );
	}

	bool PadButton( std::uint32_t instance, GamepadButton button, bool pressed ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = pressed ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
		e.cbutton.which = SDL_JoystickID( instance );
		e.cbutton.button = Uint8( button ); // same order as SDL_GameControllerButton
		e.cbutton.state = pressed ? SDL_PRESSED : SDL_RELEASED;
		return Push( e );
	}

	bool PadAxis( std::uint32_t instance, GamepadAxis axis, std::int32_t raw ) override
	{
		static const Uint8 kAxes[] = { SDL_CONTROLLER_AXIS_LEFTX, SDL_CONTROLLER_AXIS_LEFTY,
			SDL_CONTROLLER_AXIS_RIGHTX, SDL_CONTROLLER_AXIS_RIGHTY, SDL_CONTROLLER_AXIS_TRIGGERLEFT,
			SDL_CONTROLLER_AXIS_TRIGGERRIGHT };
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = SDL_CONTROLLERAXISMOTION;
		e.caxis.which = SDL_JoystickID( instance );
		e.caxis.axis = kAxes[std::size_t( axis )];
		e.caxis.value = Sint16( raw );
		return Push( e );
	}

	bool Touch( NativeTouch kind, WindowId window, std::uint64_t device, std::uint64_t finger,
	    float x, float y, float dx, float dy, float pressure ) override
	{
		SDL_Event e;
		std::memset( &e, 0, sizeof( e ) );
		e.type = kind == NativeTouch::Down ? SDL_FINGERDOWN
		         : kind == NativeTouch::Up ? SDL_FINGERUP
		                                   : SDL_FINGERMOTION;
		e.tfinger.touchId = SDL_TouchID( device );
		e.tfinger.fingerId = SDL_FingerID( finger );
		e.tfinger.x = x;
		e.tfinger.y = y;
		e.tfinger.dx = dx;
		e.tfinger.dy = dy;
		e.tfinger.pressure = pressure;
		e.tfinger.windowID = m_System.NativeWindowId( window );
		return Push( e );
	}

	// SDL opens a controller from a device index; a pushed event names no real
	// device, so hot-plug needs hardware.
	bool PadDevice( std::uint32_t, bool ) override { return false; }

	void AdvanceMs( std::uint32_t ms ) override { SDL_Delay( ms ); }
	std::uint32_t SettleMs() const override { return m_SettleMs; }
	bool AutoDismissesMessages() const override { return false; }
	std::string LastMessage() const override { return std::string(); }

	std::uint32_t m_SettleMs = 150;

private:
	static bool Push( SDL_Event &e ) { return SDL_PushEvent( &e ) == 1; }

	Sdl2WindowSystem &m_System;
};

struct Observed
{
	Sdl2WindowSystem *system = nullptr;
	int constructed = 0;
};

ProviderDescriptor DescribeSdl2( Observed &observed )
{
	return ProviderDescriptor::Define<Sdl2WindowSystem, IWindowSystem, IEventSource, ICursor,
	    IClipboard, IMessageBox, IGamepads>( "sdl2-window-system", {},
	    [&observed]( const DependencyView & ) -> Expected<std::unique_ptr<Sdl2WindowSystem>, ProviderError>
	    {
		    auto system = std::make_unique<Sdl2WindowSystem>();
		    observed.system = system.get();
		    ++observed.constructed;
		    return system;
	    } );
}

void RunInstance( ApplicationComposition &root, Observed &observed, int run )
{
	Check( "sdl2.start", !!root.Start() );
	WindowCapabilities caps;
	caps.windows = root.Find<IWindowSystem>();
	caps.events = root.Find<IEventSource>();
	caps.cursor = root.Find<ICursor>();
	caps.clipboard = root.Find<IClipboard>();
	caps.messageBox = root.Find<IMessageBox>();
	caps.gamepads = root.Find<IGamepads>();
	if ( !caps.windows || !observed.system )
	{
		Check( "sdl2.published", false );
		return;
	}

	// One connected SDL2 window system per process.
	Sdl2WindowSystem second;
	auto connected = second.Connect();
	Check( "sdl2.single_instance", !connected );

	Sdl2Driver driver( *observed.system );
	const WindowReport report = RunWindowConformance( caps, driver );
	g_Checks += report.checks;
	g_Failures += report.failures;
	std::printf( "%s platform.window[sdl2 %s, run %d]: %d checks, %d failures, %d skips\n",
	    report.failures ? "FAIL" : "ok", SDL_GetCurrentVideoDriver(), run, report.checks,
	    report.failures, report.skipped );
	for ( const std::string &skip : report.skips )
		std::printf( "  recorded skip: %s\n", skip.c_str() );
	Check( "sdl2.stop", !!root.Stop() );
	Check( "sdl2.released_instance", observed.constructed == run );
}

} // namespace

int main()
{
	// Never open windows on a live desktop by accident.
	if ( !std::getenv( "SDL_VIDEODRIVER" ) )
		setenv( "SDL_VIDEODRIVER", "offscreen", 1 );

	SDL_version version;
	SDL_GetVersion( &version );
	std::printf( "SDL %d.%d.%d, video driver request %s\n", version.major, version.minor,
	    version.patch, std::getenv( "SDL_VIDEODRIVER" ) );

	Observed observed;
	{
		ApplicationComposition root;
		Check( "sdl2.add_provider", !!root.AddProvider( DescribeSdl2( observed ) ) );
		RunInstance( root, observed, 1 );
		RunInstance( root, observed, 2 );
	}

	// After the composition is gone the process-wide slot is free again.
	{
		Sdl2WindowSystem again;
		auto connected = again.Connect();
		Check( "sdl2.reconnect_after_teardown", !!connected );
		again.Disconnect();
	}

	std::printf( "%s test_sdl2_window: %d checks, %d failures\n", g_Failures ? "FAIL" : "ok",
	    g_Checks, g_Failures );
	return testing::ReportConformance( g_Checks, g_Failures );
}
