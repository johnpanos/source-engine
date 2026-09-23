//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless window provider for platform.window.v1 (RFC 0001 rank 7,
//			roadmap R14). Deterministic: native events are scripted by the
//			driver and time is virtual. It uses the same portable normalizer
//			and surface registry as native providers, so the shared suite
//			certifies those rules; it is not evidence of any OS behavior.
//
//			Each FakeWindowDefects flag violates exactly one contract rule; the
//			sensitivity suite requires the shared suite to catch every one.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_WINDOW_SYSTEM_H
#define PLATFORMTEST_FAKE_WINDOW_SYSTEM_H

#include "../../../platform/window/input_normalizer.h"
#include "../../../platform/window/window_surfaces.h"
#include "platform/composition.h"
#include "window_conformance.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace platformtest
{

struct FakeWindowDefects
{
	bool acceptZeroSize = false;          // create.rejects_zero_size
	bool emptySpanConsumes = false;       // poll.empty_span_consumes_nothing
	bool dropExpansionOverflow = false;   // poll.capacity_one_no_loss
	bool reverseMixedOrder = false;       // poll.fifo_mixed
	bool modifiersBeforeTransition = false; // key.modifier_after_transition
	bool lastSideClearsModifier = false;  // key.sides_independent
	bool modifiersFromPhysicalKey = false; // key.modifier_by_layout
	bool ignoreLayoutUsage = false;       // key.layout_usage
	bool noReplacementCharacter = false;  // text.malformed_replacement
	bool wrongButtonFold = false;         // mouse.button_fold
	bool tripleDoubleClick = false;       // mouse.double_click
	bool warpNotSuppressed = false;       // cursor.warp_motion_suppressed
	bool clipboardAsciiOnly = false;      // clipboard.roundtrip_utf8
	bool unclampedAxes = false;           // gamepad.axes
	bool closeDestroysWindow = false;     // window.close_keeps_window
	bool minimizeKeepsExtent = false;     // window.minimized_zero_extent
	bool extentAfterSizeEvent = false;    // resize.extent_before_delivery
	bool backgroundKeepsSurface = false;  // surface.background_release
	bool destroySkipsListener = false;    // destroy.listener_notified
	bool deliverDestroyedEvents = false;  // destroy.drops_queued_events
	bool reuseIds = false;                // destroy.ids_not_reused
	bool destroyUnknownSucceeds = false;  // destroy.twice_unknown
	bool failConnect = false;             // composition rollback (not a suite rule)
	bool failInitialize = false;
};

// Lifecycle calls, recorded outside the provider so a test can read them after
// a failed startup has destroyed it.
struct FakeLifecycleProbe
{
	int connects = 0;
	int initializes = 0;
	int shutdowns = 0;
	int disconnects = 0;
};

struct FakeWindowConfig
{
	std::int32_t pixelScale = 1; // drawable pixels per window coordinate
	NormalizerPolicy policy;
	FakeWindowDefects defects;
	FakeLifecycleProbe *probe = nullptr;
};

class FakeWindowSystem final : public platform::IProviderLifecycle,
                               public IWindowSystem,
                               public IEventSource,
                               public ICursor,
                               public IClipboard,
                               public IMessageBox,
                               public IGamepads
{
public:
	// Native events, as a platform would queue them.
	struct Native
	{
		enum class Kind
		{
			Key,
			Text,
			Button,
			Motion,
			Wheel,
			Window,
			Resize,
			App,
			PadButton,
			PadAxis,
			PadDevice,
			Touch,
		};

		Kind kind = Kind::Key;
		WindowId window;
		std::uint32_t a = 0, b = 0; // usage/layout, button, instance, enum values
		bool flag = false;          // pressed / added
		std::int32_t x = 0, y = 0, dx = 0, dy = 0;
		std::uint64_t timeMs = 0;
		std::string text;
		NativeTouch touch = NativeTouch::Down;
		float fx = 0, fy = 0, fdx = 0, fdy = 0, pressure = 0;
		std::uint64_t device = 0, finger = 0;
	};

	explicit FakeWindowSystem( FakeWindowConfig config = FakeWindowConfig{} )
	    : m_Config( config ), m_Normalizer( config.policy )
	{
	}

	// -- Lifecycle --
	foundation::Expected<void, platform::ProviderError> Connect() override
	{
		if ( m_Config.probe )
			++m_Config.probe->connects;
		if ( m_Config.defects.failConnect )
			return foundation::MakeUnexpected(
			    platform::ProviderError{ platform::ProviderErrorCode::ConnectFailed, "no display" } );
		m_Connected = true;
		return {};
	}

	foundation::Expected<void, platform::ProviderError> Initialize() override
	{
		if ( m_Config.probe )
			++m_Config.probe->initializes;
		if ( m_Config.defects.failInitialize )
			return foundation::MakeUnexpected( platform::ProviderError{
			    platform::ProviderErrorCode::InitializationFailed, "no input devices" } );
		return {};
	}

	void Shutdown() noexcept override
	{
		if ( m_Config.probe )
			++m_Config.probe->shutdowns;
		for ( const Window &window : m_Windows )
			m_Surfaces.Invalidate( window.id );
		m_Windows.clear();
		m_Native.clear();
		m_Pending.Clear();
		m_Normalizer.Reset();
		m_Pads.clear();
	}

	void Disconnect() noexcept override
	{
		if ( m_Config.probe )
			++m_Config.probe->disconnects;
		m_Connected = false;
	}

	// -- IWindowSystem --
	foundation::Expected<WindowId, WindowError> Create( const WindowDesc &desc ) override
	{
		if ( ( desc.width <= 0 || desc.height <= 0 ) && !m_Config.defects.acceptZeroSize )
			return Fail( WindowStatus::InvalidArgument, WindowOperation::Create );
		Window window;
		window.id = WindowId{ m_Config.defects.reuseIds && !m_FreedIds.empty() ? m_FreedIds.back()
		                                                                        : m_NextId++ };
		window.width = desc.width;
		window.height = desc.height;
		window.visible = !desc.hidden;
		window.title = std::string( desc.title );
		m_Windows.push_back( window );
		m_Surfaces.Add( window.id, Pixels( window ), window.visible );
		Native shown;
		shown.kind = Native::Kind::Window;
		shown.window = window.id;
		shown.a = std::uint32_t( window.visible ? NativeWindowEvent::Shown : NativeWindowEvent::Hidden );
		m_Native.push_back( shown );
		return window.id;
	}

	foundation::Expected<void, WindowError> Destroy( WindowId id ) override
	{
		Window *window = Find( id );
		if ( !window )
		{
			if ( m_Config.defects.destroyUnknownSucceeds )
				return {};
			return Fail( WindowStatus::UnknownWindow, WindowOperation::Destroy );
		}
		// Defect: the surface is neither told nor invalidated.
		if ( !m_Config.defects.destroySkipsListener )
			m_Surfaces.Invalidate( id );
		m_Windows.erase( m_Windows.begin() + ( window - m_Windows.data() ) );
		m_FreedIds.push_back( id.value );
		m_Normalizer.ForgetWindow( id );
		if ( !m_Config.defects.deliverDestroyedEvents )
		{
			m_Pending.DropWindow( id );
			std::erase_if( m_Native, [&]( const Native &n ) { return n.window == id; } );
		}
		return {};
	}

	foundation::Expected<SizeEvent, WindowError> GetSize( WindowId id ) const override
	{
		const Window *window = Find( id );
		if ( !window )
			return Fail( WindowStatus::UnknownWindow, WindowOperation::GetSize );
		return SizeEvent{ window->width, window->height };
	}

	foundation::Expected<SizeEvent, WindowError> GetPixelSize( WindowId id ) const override
	{
		const Window *window = Find( id );
		if ( !window )
			return Fail( WindowStatus::UnknownWindow, WindowOperation::GetSize );
		const render::RenderExtent pixels = Pixels( *window );
		return SizeEvent{ std::int32_t( pixels.width ), std::int32_t( pixels.height ) };
	}

	foundation::Expected<void, WindowError> SetSize(
	    WindowId id, std::int32_t width, std::int32_t height ) override
	{
		if ( width <= 0 || height <= 0 )
			return Fail( WindowStatus::InvalidArgument, WindowOperation::SetSize );
		if ( !Find( id ) )
			return Fail( WindowStatus::UnknownWindow, WindowOperation::SetSize );
		Native resize;
		resize.kind = Native::Kind::Resize;
		resize.window = id;
		resize.x = width;
		resize.y = height;
		m_Native.push_back( resize );
		return {};
	}

	foundation::Expected<void, WindowError> SetTitle( WindowId id, std::string_view title ) override
	{
		Window *window = Find( id );
		if ( !window )
			return Fail( WindowStatus::UnknownWindow, WindowOperation::SetTitle );
		window->title = std::string( title );
		return {};
	}

	render::IRenderSurface *Surface( WindowId id ) override { return m_Surfaces.Find( id ); }

	// -- IEventSource --
	std::size_t Poll( std::span<Event> out ) override
	{
		m_Surfaces.Collect();
		if ( out.empty() )
		{
			if ( m_Config.defects.emptySpanConsumes && !m_Native.empty() )
				m_Native.erase( m_Native.begin() );
			return 0;
		}
		std::size_t written = m_Pending.Drain( out );
		while ( written < out.size() && !m_Native.empty() )
		{
			if ( m_Config.defects.reverseMixedOrder && m_Native.size() >= 6 )
				std::swap( m_Native[0], m_Native[1] );
			Native native = std::move( m_Native.front() );
			m_Native.erase( m_Native.begin() );
			Translate( native );
			written += m_Pending.Drain( out.subspan( written ) );
			if ( m_Config.defects.dropExpansionOverflow )
				m_Pending.Clear();
		}
		return written;
	}

	bool Wait( std::uint32_t ) override { return !m_Pending.IsEmpty() || !m_Native.empty(); }

	// -- ICursor --
	foundation::Expected<void, WindowError> SetVisible( bool visible ) override
	{
		m_CursorVisible = visible;
		return {};
	}

	bool IsVisible() const override { return m_CursorVisible; }

	foundation::Expected<void, WindowError> SetRelativeMode( WindowId id, bool ) override
	{
		if ( !Find( id ) )
			return Fail( WindowStatus::UnknownWindow, WindowOperation::SetRelativeMode );
		return {};
	}

	foundation::Expected<void, WindowError> Warp( WindowId id, std::int32_t x, std::int32_t y ) override
	{
		if ( !Find( id ) )
			return Fail( WindowStatus::UnknownWindow, WindowOperation::Warp );
		// Like X11, the platform synthesizes a motion event for the warp.
		if ( !m_Config.defects.warpNotSuppressed )
			m_Normalizer.ExpectWarp( id, x, y );
		Native motion;
		motion.kind = Native::Kind::Motion;
		motion.window = id;
		motion.x = x;
		motion.y = y;
		m_Native.push_back( motion );
		return {};
	}

	foundation::Expected<void, WindowError> SetShape( CursorShape shape ) override
	{
		if ( shape >= CursorShape::Count )
			return Fail( WindowStatus::InvalidArgument, WindowOperation::SetCursorShape );
		return {};
	}

	// -- IClipboard --
	foundation::Expected<void, WindowError> SetText( std::string_view utf8 ) override
	{
		m_Clipboard.clear();
		for ( char c : utf8 )
			if ( !m_Config.defects.clipboardAsciiOnly || static_cast<unsigned char>( c ) < 0x80 )
				m_Clipboard.push_back( c );
		return {};
	}

	foundation::Expected<std::string, WindowError> GetText() override { return m_Clipboard; }

	// -- IMessageBox --
	foundation::Expected<void, WindowError> ShowError(
	    std::string_view title, std::string_view message, WindowId parent ) override
	{
		if ( parent.IsValid() && !Find( parent ) )
			return Fail( WindowStatus::UnknownWindow, WindowOperation::ShowMessage );
		lastMessage = std::string( title ) + ": " + std::string( message );
		return {};
	}

	// -- IGamepads --
	std::size_t ConnectedCount() const override { return m_Pads.size(); }

	foundation::Expected<void, WindowError> Rumble(
	    std::uint32_t instance, float low, float high, std::uint32_t ) override
	{
		if ( !( low >= 0.0f && low <= 1.0f && high >= 0.0f && high <= 1.0f ) )
			return Fail( WindowStatus::InvalidArgument, WindowOperation::Rumble );
		for ( std::uint32_t pad : m_Pads )
			if ( pad == instance )
				return {};
		return Fail( WindowStatus::UnknownDevice, WindowOperation::Rumble );
	}

	// -- Test driver endpoint --
	void Inject( Native native )
	{
		native.timeMs = nowMs;
		m_Native.push_back( std::move( native ) );
	}

	bool IsConnected() const { return m_Connected; }
	std::size_t WindowCount() const { return m_Windows.size(); }
	std::size_t SurfaceAllocations() const { return m_Surfaces.AllocatedCount(); }

	std::uint64_t nowMs = 1000;
	std::string lastMessage;

private:
	struct Window
	{
		WindowId id;
		std::int32_t width = 0;
		std::int32_t height = 0;
		bool visible = true;
		bool minimized = false;
		std::string title;
	};

	static foundation::Unexpected<WindowError> Fail( WindowStatus status, WindowOperation operation )
	{
		return foundation::MakeUnexpected( WindowError{ status, operation, 0 } );
	}

	Window *Find( WindowId id )
	{
		for ( Window &window : m_Windows )
			if ( window.id == id )
				return &window;
		return nullptr;
	}

	const Window *Find( WindowId id ) const
	{
		return const_cast<FakeWindowSystem *>( this )->Find( id );
	}

	render::RenderExtent Pixels( const Window &window ) const
	{
		return render::RenderExtent{ std::uint32_t( window.width * m_Config.pixelScale ),
		    std::uint32_t( window.height * m_Config.pixelScale ) };
	}

	void PushWindowEvent( EventType type, WindowId id )
	{
		Event event;
		event.type = type;
		event.window = id;
		m_Pending.Push( event );
	}

	void Translate( const Native &n )
	{
		const FakeWindowDefects &d = m_Config.defects;
		if ( n.window.IsValid() && !Find( n.window ) && !d.deliverDestroyedEvents )
			return; // events for a window destroyed since they were queued
		switch ( n.kind )
		{
		case Native::Kind::Key:
		{
			std::uint32_t layout = d.ignoreLayoutUsage ? 0 : n.b;
			if ( d.modifiersFromPhysicalKey )
				layout = 0;
			const std::uint32_t before = m_Normalizer.Modifiers();
			m_Normalizer.Key( m_Pending, n.window, n.a, layout, n.flag, false );
			if ( Event *last = m_Pending.Back() )
			{
				if ( d.modifiersBeforeTransition )
					last->key.modifiers = before;
				if ( d.lastSideClearsModifier && !n.flag &&
				     ( n.a == kUsageLeftShift || n.a == kUsageRightShift ) )
					last->key.modifiers &= ~kModifierShift;
			}
			break;
		}
		case Native::Kind::Text:
		{
			std::string text;
			for ( std::size_t i = 0; i < n.text.size(); ++i )
			{
				// Defect: a lead byte without its continuation is silently dropped.
				const bool lone = static_cast<unsigned char>( n.text[i] ) >= 0xC0 &&
				                  ( i + 1 == n.text.size() ||
				                      ( static_cast<unsigned char>( n.text[i + 1] ) & 0xC0 ) != 0x80 );
				if ( !( d.noReplacementCharacter && lone ) )
					text.push_back( n.text[i] );
			}
			m_Normalizer.Text( m_Pending, n.window, text );
			break;
		}
		case Native::Kind::Button:
		{
			std::uint32_t button = n.a;
			if ( d.wrongButtonFold && button == 6 )
				button = 5;
			m_Normalizer.Button( m_Pending, n.window, button, n.flag, n.x, n.y, n.timeMs );
			Event *last = m_Pending.Back();
			if ( n.flag && last )
			{
				// Defect: a press right after a double click counts as another double.
				if ( d.tripleDoubleClick && m_LastClicks == 2 && n.timeMs == m_LastPressMs )
					last->mouse.clicks = 2;
				m_LastClicks = last->mouse.clicks;
				m_LastPressMs = n.timeMs;
			}
			break;
		}
		case Native::Kind::Motion:
			m_Normalizer.Motion( m_Pending, n.window, n.x, n.y, n.dx, n.dy );
			break;
		case Native::Kind::Wheel:
			m_Normalizer.Wheel( m_Pending, n.window, n.x, n.y );
			break;
		case Native::Kind::Window:
			TranslateWindow( n );
			break;
		case Native::Kind::Resize:
		{
			Window *window = Find( n.window );
			if ( !window )
				break;
			window->width = n.x;
			window->height = n.y;
			Event resized;
			resized.type = EventType::WindowResized;
			resized.window = n.window;
			resized.size = SizeEvent{ n.x, n.y };
			m_Pending.Push( resized );
			const render::RenderExtent pixels = Pixels( *window );
			Event pixel = resized;
			pixel.type = EventType::WindowPixelSizeChanged;
			pixel.size = SizeEvent{ std::int32_t( pixels.width ), std::int32_t( pixels.height ) };
			if ( !d.extentAfterSizeEvent )
				m_Surfaces.SetPixelExtent( n.window, pixels );
			m_Pending.Push( pixel );
			if ( d.extentAfterSizeEvent )
				m_DeferredExtent = std::make_pair( n.window, pixels );
			break;
		}
		case Native::Kind::App:
			TranslateApp( NativeAppEvent( n.a ) );
			break;
		case Native::Kind::PadButton:
		case Native::Kind::PadAxis:
		case Native::Kind::PadDevice:
		{
			Event event;
			event.gamepad.instance = n.a;
			if ( n.kind == Native::Kind::PadButton )
			{
				event.type = n.flag ? EventType::GamepadButtonDown : EventType::GamepadButtonUp;
				event.gamepad.button = GamepadButton( n.b );
			}
			else if ( n.kind == Native::Kind::PadAxis )
			{
				event.type = EventType::GamepadAxis;
				event.gamepad.axis = GamepadAxis( n.b );
				event.gamepad.value = d.unclampedAxes ? float( n.x ) / 32767.0f
				                                      : InputNormalizer::NormalizeAxis(
				                                            GamepadAxis( n.b ), n.x );
			}
			else
			{
				event.type = n.flag ? EventType::GamepadAdded : EventType::GamepadRemoved;
				if ( n.flag )
					m_Pads.push_back( n.a );
				else
					std::erase( m_Pads, n.a );
			}
			m_Pending.Push( event );
			break;
		}
		case Native::Kind::Touch:
		{
			Event event;
			event.type = n.touch == NativeTouch::Down ? EventType::TouchDown
			             : n.touch == NativeTouch::Up ? EventType::TouchUp
			                                          : EventType::TouchMotion;
			event.window = n.window;
			event.touch = TouchEvent{ n.device, n.finger, n.fx, n.fy, n.fdx, n.fdy, n.pressure };
			m_Pending.Push( event );
			break;
		}
		}
	}

	void TranslateWindow( const Native &n )
	{
		Window *window = Find( n.window );
		if ( !window )
		{
			// Only reachable with deliverDestroyedEvents: report a stale window.
			PushWindowEvent( EventType::WindowShown, n.window );
			return;
		}
		switch ( NativeWindowEvent( n.a ) )
		{
		case NativeWindowEvent::Shown:
			window->visible = true;
			m_Surfaces.SetVisible( n.window, !window->minimized );
			PushWindowEvent( EventType::WindowShown, n.window );
			break;
		case NativeWindowEvent::Hidden:
			window->visible = false;
			m_Surfaces.SetVisible( n.window, false );
			PushWindowEvent( EventType::WindowHidden, n.window );
			break;
		case NativeWindowEvent::Minimized:
			window->minimized = true;
			if ( !m_Config.defects.minimizeKeepsExtent )
				m_Surfaces.SetVisible( n.window, false );
			PushWindowEvent( EventType::WindowMinimized, n.window );
			break;
		case NativeWindowEvent::Restored:
			window->minimized = false;
			m_Surfaces.SetVisible( n.window, window->visible );
			PushWindowEvent( EventType::WindowRestored, n.window );
			break;
		case NativeWindowEvent::FocusGained:
			PushWindowEvent( EventType::WindowFocusGained, n.window );
			break;
		case NativeWindowEvent::FocusLost:
			PushWindowEvent( EventType::WindowFocusLost, n.window );
			break;
		case NativeWindowEvent::CloseRequested:
			PushWindowEvent( EventType::WindowCloseRequested, n.window );
			if ( m_Config.defects.closeDestroysWindow )
				(void)Destroy( n.window );
			break;
		}
	}

	void TranslateApp( NativeAppEvent app )
	{
		EventType type = EventType::Quit;
		switch ( app )
		{
		case NativeAppEvent::Quit:
			type = EventType::Quit;
			break;
		case NativeAppEvent::Terminating:
			type = EventType::AppTerminating;
			break;
		case NativeAppEvent::LowMemory:
			type = EventType::AppLowMemory;
			break;
		case NativeAppEvent::WillEnterBackground:
			type = EventType::AppWillEnterBackground;
			break;
		case NativeAppEvent::DidEnterBackground:
			type = EventType::AppDidEnterBackground;
			if ( !m_Config.defects.backgroundKeepsSurface )
				m_Surfaces.ReleaseAll();
			break;
		case NativeAppEvent::WillEnterForeground:
			type = EventType::AppWillEnterForeground;
			m_Surfaces.RestoreAll();
			break;
		case NativeAppEvent::DidEnterForeground:
			type = EventType::AppDidEnterForeground;
			break;
		}
		PushWindowEvent( type, WindowId{} );
	}

public:
	// Applies a deferred surface extent (the extentAfterSizeEvent defect) once the
	// size event has been delivered.
	void ApplyDeferred()
	{
		if ( m_DeferredExtent.first.IsValid() )
			m_Surfaces.SetPixelExtent( m_DeferredExtent.first, m_DeferredExtent.second );
		m_DeferredExtent = {};
	}

private:
	FakeWindowConfig m_Config;
	InputNormalizer m_Normalizer;
	EventFifo m_Pending;
	WindowSurfaces m_Surfaces;
	std::vector<Window> m_Windows;
	std::vector<Native> m_Native;
	std::vector<std::uint32_t> m_FreedIds;
	std::vector<std::uint32_t> m_Pads;
	std::uint32_t m_NextId = 1;
	bool m_Connected = false;
	bool m_CursorVisible = true;
	std::string m_Clipboard;
	std::uint8_t m_LastClicks = 0;
	std::uint64_t m_LastPressMs = 0;
	std::pair<WindowId, render::RenderExtent> m_DeferredExtent;
};

// Scripts native events into a FakeWindowSystem.
class FakeWindowDriver final : public IWindowTestDriver
{
public:
	explicit FakeWindowDriver( FakeWindowSystem &system ) : m_System( system ) {}

	using Native = FakeWindowSystem::Native;

	bool Key( WindowId window, std::uint32_t usage, std::uint32_t layoutUsage, bool pressed ) override
	{
		Native n;
		n.kind = Native::Kind::Key;
		n.window = window;
		n.a = usage;
		n.b = layoutUsage;
		n.flag = pressed;
		return Push( std::move( n ) );
	}

	bool Text( WindowId window, std::string_view utf8 ) override
	{
		Native n;
		n.kind = Native::Kind::Text;
		n.window = window;
		n.text = std::string( utf8 );
		return Push( std::move( n ) );
	}

	bool Button( WindowId window, std::uint32_t button, bool pressed, std::int32_t x,
	    std::int32_t y ) override
	{
		Native n;
		n.kind = Native::Kind::Button;
		n.window = window;
		n.a = button;
		n.flag = pressed;
		n.x = x;
		n.y = y;
		return Push( std::move( n ) );
	}

	bool Motion( WindowId window, std::int32_t x, std::int32_t y, std::int32_t dx,
	    std::int32_t dy ) override
	{
		Native n;
		n.kind = Native::Kind::Motion;
		n.window = window;
		n.x = x;
		n.y = y;
		n.dx = dx;
		n.dy = dy;
		return Push( std::move( n ) );
	}

	bool Wheel( WindowId window, std::int32_t x, std::int32_t y ) override
	{
		Native n;
		n.kind = Native::Kind::Wheel;
		n.window = window;
		n.x = x;
		n.y = y;
		return Push( std::move( n ) );
	}

	bool Window( WindowId window, NativeWindowEvent event ) override
	{
		Native n;
		n.kind = Native::Kind::Window;
		n.window = window;
		n.a = std::uint32_t( event );
		return Push( std::move( n ) );
	}

	bool App( NativeAppEvent event ) override
	{
		Native n;
		n.kind = Native::Kind::App;
		n.a = std::uint32_t( event );
		return Push( std::move( n ) );
	}

	bool PadButton( std::uint32_t instance, GamepadButton button, bool pressed ) override
	{
		Native n;
		n.kind = Native::Kind::PadButton;
		n.a = instance;
		n.b = std::uint32_t( button );
		n.flag = pressed;
		return Push( std::move( n ) );
	}

	bool PadAxis( std::uint32_t instance, GamepadAxis axis, std::int32_t raw ) override
	{
		Native n;
		n.kind = Native::Kind::PadAxis;
		n.a = instance;
		n.b = std::uint32_t( axis );
		n.x = raw;
		return Push( std::move( n ) );
	}

	bool Touch( NativeTouch kind, WindowId window, std::uint64_t device, std::uint64_t finger,
	    float x, float y, float dx, float dy, float pressure ) override
	{
		Native n;
		n.kind = Native::Kind::Touch;
		n.touch = kind;
		n.window = window;
		n.device = device;
		n.finger = finger;
		n.fx = x;
		n.fy = y;
		n.fdx = dx;
		n.fdy = dy;
		n.pressure = pressure;
		return Push( std::move( n ) );
	}

	bool AttachPad( std::uint32_t &instance ) override
	{
		instance = m_NextPad++;
		return PadDevice( instance, true );
	}

	bool DetachPad( std::uint32_t instance ) override { return PadDevice( instance, false ); }

	void AdvanceMs( std::uint32_t ms ) override { m_System.nowMs += ms; }
	std::uint32_t SettleMs() const override { return 0; }
	bool AutoDismissesMessages() const override { return true; }
	std::string LastMessage() const override { return m_System.lastMessage; }

private:
	bool PadDevice( std::uint32_t instance, bool added )
	{
		Native n;
		n.kind = Native::Kind::PadDevice;
		n.a = instance;
		n.flag = added;
		return Push( std::move( n ) );
	}

	bool Push( Native native )
	{
		m_System.ApplyDeferred();
		m_System.Inject( std::move( native ) );
		return true;
	}

	FakeWindowSystem &m_System;
	std::uint32_t m_NextPad = 3;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_WINDOW_SYSTEM_H
