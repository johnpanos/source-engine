//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the window/input capabilities
//			(platform.window.v1, RFC 0001 rank 7, roadmap R14). Every provider
//			claiming the contracts runs this same predicate: the headless test
//			provider, its deliberately broken variants, and native providers
//			(SDL2 today) driven by a driver that injects native events.
//
//			Checks are named; a failure records the name so the sensitivity
//			suite can require that each seeded defect is caught by its check.
//			Native providers may report platform-generated window events at any
//			time, so input checks look only at the input events they injected.
//
//=============================================================================//

#ifndef PLATFORMTEST_WINDOW_CONFORMANCE_H
#define PLATFORMTEST_WINDOW_CONFORMANCE_H

#include "platform/window/window_system.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace platformtest
{

using namespace platform::window;

struct WindowCapabilities
{
	IWindowSystem *windows = nullptr;
	IEventSource *events = nullptr;
	ICursor *cursor = nullptr;       // optional
	IClipboard *clipboard = nullptr; // optional
	IMessageBox *messageBox = nullptr;
	IGamepads *gamepads = nullptr;
};

enum class NativeWindowEvent
{
	Shown,
	Hidden,
	Minimized,
	Restored,
	FocusGained,
	FocusLost,
	CloseRequested,
};

enum class NativeAppEvent
{
	Quit,
	Terminating,
	LowMemory,
	WillEnterBackground,
	DidEnterBackground,
	WillEnterForeground,
	DidEnterForeground,
};

enum class NativeTouch
{
	Down,
	Up,
	Motion,
};

// Injects native stimuli into the provider's native queue, in call order. Each
// method returns false when the driver could not inject.
class IWindowTestDriver
{
public:
	virtual ~IWindowTestDriver() = default;

	// 'layoutUsage' is the key the layout makes the physical key act as; 0 means
	// the US layout.
	virtual bool Key( WindowId window, std::uint32_t usage, std::uint32_t layoutUsage,
	    bool pressed ) = 0;
	virtual bool Text( WindowId window, std::string_view utf8 ) = 0;
	virtual bool Button( WindowId window, std::uint32_t nativeButton, bool pressed, std::int32_t x,
	    std::int32_t y ) = 0;
	virtual bool Motion( WindowId window, std::int32_t x, std::int32_t y, std::int32_t dx,
	    std::int32_t dy ) = 0;
	virtual bool Wheel( WindowId window, std::int32_t x, std::int32_t y ) = 0;
	virtual bool Window( WindowId window, NativeWindowEvent event ) = 0;
	virtual bool App( NativeAppEvent event ) = 0;
	virtual bool PadButton( std::uint32_t instance, GamepadButton button, bool pressed ) = 0;
	virtual bool PadAxis( std::uint32_t instance, GamepadAxis axis, std::int32_t raw ) = 0;
	virtual bool Touch( NativeTouch kind, WindowId window, std::uint64_t device,
	    std::uint64_t finger, float x, float y, float dx, float dy, float pressure ) = 0;

	// Gamepad hot-plug; false when the platform cannot simulate a device.
	virtual bool PadDevice( std::uint32_t instance, bool added ) = 0;

	// Advances native event time: virtual time for fakes, a real delay natively.
	virtual void AdvanceMs( std::uint32_t ms ) = 0;

	// How long the platform may keep producing its own events after a window
	// operation. Zero for a deterministic provider.
	virtual std::uint32_t SettleMs() const = 0;

	// A message box can be shown and dismissed without a user (fakes only).
	virtual bool AutoDismissesMessages() const = 0;
	virtual std::string LastMessage() const = 0;
};

struct WindowSuiteOptions
{
	std::uint32_t doubleClickMs = 400; // the provider's policy
	std::int32_t doubleClickDistance = 2;
	std::uint32_t resizeTimeoutMs = 3000;
};

struct WindowReport
{
	int checks = 0;
	int failures = 0;
	int skipped = 0;
	std::vector<std::string> failed;
	std::vector<std::string> skips;

	bool Failed( std::string_view name ) const
	{
		for ( const std::string &f : failed )
			if ( f == name )
				return true;
		return false;
	}
};

namespace window_detail
{

inline void Check( WindowReport &r, const char *name, bool ok )
{
	++r.checks;
	if ( ok )
		return;
	++r.failures;
	if ( !r.Failed( name ) )
		r.failed.push_back( name );
	std::printf( "  FAIL %s\n", name );
}

inline void Skip( WindowReport &r, const char *name, const char *reason )
{
	++r.skipped;
	r.skips.push_back( name );
	std::printf( "  skip %s: %s\n", name, reason );
}

inline bool IsInput( EventType type )
{
	switch ( type )
	{
	case EventType::KeyDown:
	case EventType::KeyUp:
	case EventType::TextInput:
	case EventType::MouseMotion:
	case EventType::MouseButtonDown:
	case EventType::MouseButtonUp:
	case EventType::MouseWheel:
	case EventType::GamepadAdded:
	case EventType::GamepadRemoved:
	case EventType::GamepadButtonDown:
	case EventType::GamepadButtonUp:
	case EventType::GamepadAxis:
	case EventType::TouchDown:
	case EventType::TouchUp:
	case EventType::TouchMotion:
		return true;
	default:
		return false;
	}
}

inline bool IsApp( EventType type )
{
	return type >= EventType::Quit && type <= EventType::AppDidEnterForeground;
}

// Records surface callbacks and the surface state seen from inside them.
class ProbeListener final : public render::IRenderSurfaceListener
{
public:
	explicit ProbeListener( render::IRenderSurface &surface ) : m_Surface( surface ) {}

	void OnNativeSurfaceReleasing() override
	{
		++calls;
		statusAtCall = m_Surface.GetStatus();
	}

	int calls = 0;
	render::RenderSurfaceStatus statusAtCall = render::RenderSurfaceStatus::kAvailable;

private:
	render::IRenderSurface &m_Surface;
};

class Suite
{
public:
	Suite( const WindowCapabilities &caps, IWindowTestDriver &driver,
	    const WindowSuiteOptions &options, WindowReport &report )
	    : m_Caps( caps ), m_Windows( *caps.windows ), m_Events( *caps.events ),
	      m_Driver( driver ), m_Options( options ), m_Report( report )
	{
	}

	void Run()
	{
		if ( !CreateWindows() )
			return;
		Poll();
		Keyboard();
		Text();
		Mouse();
		Cursor();
		Clipboard();
		MessageBox();
		Gamepads();
		Touch();
		WindowState();
		Resize();
		Lifecycle();
		Destroy();
	}

private:
	void Check( const char *name, bool ok ) { window_detail::Check( m_Report, name, ok ); }

	// Delivers platform-generated events until the queue stays quiet.
	void Settle()
	{
		Event scratch[16];
		for ( int round = 0; round < 1000; ++round )
		{
			while ( m_Events.Poll( scratch ) != 0 )
			{
			}
			if ( !m_Events.Wait( m_Driver.SettleMs() ) )
				return;
		}
	}

	// Every event currently available, one Poll at a time.
	std::vector<Event> DrainOneByOne()
	{
		std::vector<Event> events;
		Event event;
		for ( int guard = 0; guard < 4096; ++guard )
		{
			if ( m_Events.Poll( std::span<Event>( &event, 1 ) ) == 1 )
			{
				events.push_back( event );
				continue;
			}
			if ( !m_Events.Wait( m_Driver.SettleMs() ) )
				break;
		}
		return events;
	}

	std::vector<Event> Input()
	{
		std::vector<Event> input;
		for ( const Event &event : DrainOneByOne() )
			if ( IsInput( event.type ) )
				input.push_back( event );
		return input;
	}

	std::vector<Event> Matching( EventType type )
	{
		std::vector<Event> found;
		for ( const Event &event : DrainOneByOne() )
			if ( event.type == type )
				found.push_back( event );
		return found;
	}

	bool CreateWindows()
	{
		WindowDesc bad;
		bad.title = "zero";
		auto zero = m_Windows.Create( bad );
		Check( "create.rejects_zero_size", !zero && zero.Error().status == WindowStatus::InvalidArgument &&
		                                       zero.Error().operation == WindowOperation::Create );
		bad.width = -4;
		bad.height = 10;
		auto negative = m_Windows.Create( bad );
		Check( "create.rejects_negative_size",
		    !negative && negative.Error().status == WindowStatus::InvalidArgument );

		WindowDesc desc;
		desc.title = "platform.window A";
		desc.width = 320;
		desc.height = 240;
		auto a = m_Windows.Create( desc );
		desc.title = "platform.window B";
		desc.width = 200;
		desc.height = 100;
		auto b = m_Windows.Create( desc );
		Check( "create.succeeds", a && b );
		if ( !a || !b )
			return false;
		m_A = a.Value();
		m_B = b.Value();
		Check( "create.distinct_ids", m_A.IsValid() && m_B.IsValid() && !( m_A == m_B ) );
		Settle();

		auto size = m_Windows.GetSize( m_A );
		Check( "size.logical", size && size.Value().width == 320 && size.Value().height == 240 );
		auto pixels = m_Windows.GetPixelSize( m_A );
		render::IRenderSurface *surface = m_Windows.Surface( m_A );
		Check( "create.surface_available",
		    surface && surface->GetStatus() == render::RenderSurfaceStatus::kAvailable );
		Check( "create.surface_extent_matches_pixels",
		    surface && pixels && pixels.Value().width > 0 &&
		        surface->GetDrawableExtent().width == std::uint32_t( pixels.Value().width ) &&
		        surface->GetDrawableExtent().height == std::uint32_t( pixels.Value().height ) );
		Check( "create.surfaces_distinct", surface && surface != m_Windows.Surface( m_B ) );

		const WindowId unknown{ 0x7fffff00u };
		auto unknownSize = m_Windows.GetSize( unknown );
		Check( "size.unknown_window",
		    !unknownSize && unknownSize.Error().status == WindowStatus::UnknownWindow );
		Check( "surface.unknown_window_null", m_Windows.Surface( unknown ) == nullptr );
		Check( "title.set", !!m_Windows.SetTitle( m_A, "platform.window A (renamed)" ) );
		auto badTitle = m_Windows.SetTitle( unknown, "x" );
		Check( "title.unknown_window",
		    !badTitle && badTitle.Error().status == WindowStatus::UnknownWindow );

		WindowDesc hidden;
		hidden.title = "platform.window hidden";
		hidden.width = 64;
		hidden.height = 64;
		hidden.hidden = true;
		auto c = m_Windows.Create( hidden );
		Check( "create.hidden", !!c );
		if ( c )
		{
			render::IRenderSurface *hiddenSurface = m_Windows.Surface( c.Value() );
			Check( "create.hidden_zero_extent",
			    hiddenSurface && !hiddenSurface->GetDrawableExtent().IsPresentable() );
			Check( "create.hidden_destroy", !!m_Windows.Destroy( c.Value() ) );
			m_Destroyed.push_back( c.Value() );
		}
		Settle();
		return true;
	}

	void Poll()
	{
		m_Driver.Key( m_A, 0x04, 0, true );
		Event none;
		Check( "poll.empty_span_consumes_nothing", m_Events.Poll( std::span<Event>( &none, 0 ) ) == 0 );
		std::vector<Event> first = Input();
		Check( "poll.empty_span_consumes_nothing",
		    first.size() == 1 && first[0].type == EventType::KeyDown && first[0].key.usage == 0x04 );
		m_Driver.Key( m_A, 0x04, 0, false );
		Input();

		// One native text event expands to several events; a one-slot Poll must
		// deliver every one of them, in order, with nothing lost.
		m_Driver.Text( m_A, "h\xC3\xA9llo" );
		m_Driver.Key( m_A, 0x05, 0, true );
		m_Driver.Key( m_A, 0x05, 0, false );
		std::vector<Event> expanded = Input();
		const char32_t expected[] = { U'h', 0xE9, U'l', U'l', U'o' };
		bool ordered = expanded.size() == 7;
		for ( std::size_t i = 0; ordered && i < 5; ++i )
			ordered = expanded[i].type == EventType::TextInput &&
			          expanded[i].text.codepoint == expected[i] && expanded[i].window == m_A;
		ordered = ordered && expanded[5].type == EventType::KeyDown && expanded[6].type == EventType::KeyUp;
		Check( "poll.capacity_one_no_loss", ordered );

		// Mixed native events keep their order.
		m_Driver.Key( m_A, 0x06, 0, true );
		m_Driver.Button( m_A, 1, true, 5, 5 );
		m_Driver.Motion( m_A, 8, 9, 3, 4 );
		m_Driver.Wheel( m_A, 0, 1 );
		m_Driver.Button( m_A, 1, false, 8, 9 );
		m_Driver.Key( m_A, 0x06, 0, false );
		const EventType order[] = { EventType::KeyDown, EventType::MouseButtonDown,
		    EventType::MouseMotion, EventType::MouseWheel, EventType::MouseButtonUp,
		    EventType::KeyUp };
		std::vector<Event> mixed = Input();
		bool fifo = mixed.size() == 6;
		for ( std::size_t i = 0; fifo && i < 6; ++i )
			fifo = mixed[i].type == order[i];
		Check( "poll.fifo_mixed", fifo );
		m_Driver.AdvanceMs( m_Options.doubleClickMs + 100 );
	}

	std::uint32_t KeyMods( std::uint32_t usage, std::uint32_t layout, bool pressed )
	{
		m_Driver.Key( m_A, usage, layout, pressed );
		std::vector<Event> events = Input();
		if ( events.size() != 1 || events[0].type != ( pressed ? EventType::KeyDown : EventType::KeyUp ) )
			return 0xffffffffu;
		return events[0].key.modifiers;
	}

	void Keyboard()
	{
		m_Driver.Key( m_B, 0x04, 0, true );
		std::vector<Event> down = Input();
		Check( "key.usage_and_window", down.size() == 1 && down[0].type == EventType::KeyDown &&
		                                   down[0].key.usage == 0x04 && down[0].key.layoutUsage == 0x04 &&
		                                   down[0].window == m_B && down[0].key.modifiers == 0 );
		m_Driver.Key( m_B, 0x04, 0, false );
		Input();

		// A French layout puts A on the physical Q key.
		m_Driver.Key( m_A, 0x14, 0x04, true );
		std::vector<Event> layout = Input();
		Check( "key.layout_usage",
		    layout.size() == 1 && layout[0].key.usage == 0x14 && layout[0].key.layoutUsage == 0x04 );
		m_Driver.Key( m_A, 0x14, 0x04, false );
		Input();

		Check( "key.modifier_after_transition",
		    KeyMods( kUsageLeftShift, 0, true ) == kModifierShift &&
		        KeyMods( kUsageLeftShift, 0, false ) == 0 );

		const bool sidesDown = KeyMods( kUsageLeftShift, 0, true ) == kModifierShift &&
		                       KeyMods( kUsageRightShift, 0, true ) == kModifierShift;
		const bool leftUp = KeyMods( kUsageLeftShift, 0, false ) == kModifierShift;
		const bool rightUp = KeyMods( kUsageRightShift, 0, false ) == 0;
		Check( "key.sides_independent", sidesDown && leftUp && rightUp );

		Check( "key.caps_while_held", KeyMods( kUsageCapsLock, 0, true ) == kModifierCapsLock &&
		                                  KeyMods( kUsageCapsLock, 0, false ) == 0 );

		// Caps lock remapped to control acts as control.
		Check( "key.modifier_by_layout",
		    KeyMods( kUsageCapsLock, kUsageLeftControl, true ) == kModifierControl &&
		        KeyMods( kUsageCapsLock, kUsageLeftControl, false ) == 0 );

		const bool all = KeyMods( kUsageRightControl, 0, true ) == kModifierControl &&
		                 KeyMods( kUsageLeftAlt, 0, true ) == ( kModifierControl | kModifierAlt ) &&
		                 KeyMods( kUsageRightGui, 0, true ) ==
		                     ( kModifierControl | kModifierAlt | kModifierGui ) &&
		                 KeyMods( kUsageRightGui, 0, false ) == ( kModifierControl | kModifierAlt ) &&
		                 KeyMods( kUsageLeftAlt, 0, false ) == kModifierControl &&
		                 KeyMods( kUsageRightControl, 0, false ) == 0;
		Check( "key.all_modifiers", all );
	}

	void Text()
	{
		m_Driver.Text( m_A, "a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80" );
		std::vector<Event> text = Input();
		const char32_t expected[] = { U'a', 0xE9, 0x20AC, 0x1F600 };
		bool ok = text.size() == 4;
		for ( std::size_t i = 0; ok && i < 4; ++i )
			ok = text[i].type == EventType::TextInput && text[i].text.codepoint == expected[i];
		Check( "text.codepoints", ok );

		m_Driver.Key( m_A, kUsageLeftShift, 0, true );
		m_Driver.Text( m_A, "Z" );
		m_Driver.Key( m_A, kUsageLeftShift, 0, false );
		std::vector<Event> shifted = Input();
		Check( "text.modifiers", shifted.size() == 3 && shifted[1].type == EventType::TextInput &&
		                             shifted[1].text.codepoint == U'Z' &&
		                             shifted[1].text.modifiers == kModifierShift );

		if ( m_Driver.Text( m_A, "\xC3(" ) )
		{
			std::vector<Event> malformed = Input();
			Check( "text.malformed_replacement",
			    malformed.size() == 2 && malformed[0].text.codepoint == 0xFFFD &&
			        malformed[1].text.codepoint == U'(' );
		}
		else
		{
			window_detail::Skip( m_Report, "text.malformed_replacement",
			    "the platform cannot inject malformed UTF-8" );
		}
	}

	Event PressRelease( std::uint32_t native, std::int32_t x, std::int32_t y )
	{
		m_Driver.Button( m_A, native, true, x, y );
		m_Driver.Button( m_A, native, false, x, y );
		std::vector<Event> events = Input();
		if ( events.size() != 2 )
			return Event{};
		return events[0];
	}

	void Mouse()
	{
		const std::uint32_t natives[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		const MouseButton expected[] = { MouseButton::Left, MouseButton::Middle, MouseButton::Right,
		    MouseButton::X1, MouseButton::X2, MouseButton::X1, MouseButton::X2, MouseButton::X1,
		    MouseButton::X2 };
		bool mapped = true;
		for ( std::size_t i = 0; i < 9; ++i )
		{
			const Event down = PressRelease( natives[i], 10 + std::int32_t( i ) * 20, 10 );
			mapped = mapped && down.type == EventType::MouseButtonDown && down.mouse.button == expected[i] &&
			         down.window == m_A;
		}
		Check( "mouse.button_fold", mapped );
		m_Driver.AdvanceMs( m_Options.doubleClickMs + 100 );

		m_Driver.Button( m_A, 1, true, 30, 30 );
		m_Driver.Button( m_A, 3, true, 30, 30 );
		m_Driver.Motion( m_A, 31, 32, 1, 2 );
		m_Driver.Button( m_A, 1, false, 31, 32 );
		m_Driver.Button( m_A, 3, false, 31, 32 );
		std::vector<Event> held = Input();
		Check( "mouse.held_mask", held.size() == 5 && held[0].mouse.buttons == 1 &&
		                              held[1].mouse.buttons == 3 && held[2].mouse.buttons == 3 &&
		                              held[3].mouse.buttons == 2 && held[4].mouse.buttons == 0 );
		Check( "mouse.motion", held.size() == 5 && held[2].type == EventType::MouseMotion &&
		                           held[2].mouse.x == 31 && held[2].mouse.y == 32 &&
		                           held[2].mouse.dx == 1 && held[2].mouse.dy == 2 );
		m_Driver.AdvanceMs( m_Options.doubleClickMs + 100 );

		// Legacy: a close second press is a double click and starts a new pair.
		const Event c1 = PressRelease( 1, 50, 50 );
		const Event c2 = PressRelease( 1, 51, 51 );
		const Event c3 = PressRelease( 1, 51, 51 );
		Check( "mouse.double_click", c1.mouse.clicks == 1 && c2.mouse.clicks == 2 && c3.mouse.clicks == 1 );
		m_Driver.AdvanceMs( m_Options.doubleClickMs + 100 );

		const Event far1 = PressRelease( 1, 50, 50 );
		const Event far2 = PressRelease( 1, 50 + m_Options.doubleClickDistance + 5, 50 );
		Check( "mouse.double_click_distance", far1.mouse.clicks == 1 && far2.mouse.clicks == 1 );
		m_Driver.AdvanceMs( m_Options.doubleClickMs + 100 );

		PressRelease( 1, 60, 60 );
		m_Driver.AdvanceMs( m_Options.doubleClickMs + 100 );
		const Event late = PressRelease( 1, 60, 60 );
		Check( "mouse.double_click_timeout", late.type == EventType::MouseButtonDown && late.mouse.clicks == 1 );
		m_Driver.AdvanceMs( m_Options.doubleClickMs + 100 );

		m_Driver.Wheel( m_A, 0, 0 );
		m_Driver.Wheel( m_A, 0, -2 );
		m_Driver.Wheel( m_A, 1, 0 );
		std::vector<Event> wheel = Input();
		Check( "mouse.wheel", wheel.size() == 2 && wheel[0].type == EventType::MouseWheel &&
		                          wheel[0].wheel.y == -2 && wheel[0].wheel.x == 0 &&
		                          wheel[1].wheel.x == 1 && wheel[1].wheel.y == 0 );
	}

	void Cursor()
	{
		ICursor *cursor = m_Caps.cursor;
		if ( !cursor )
		{
			window_detail::Skip( m_Report, "cursor.*", "provider exports no cursor capability" );
			return;
		}
		Check( "cursor.hide", !!cursor->SetVisible( false ) && !cursor->IsVisible() );
		Check( "cursor.show", !!cursor->SetVisible( true ) && cursor->IsVisible() );

		const WindowId unknown{ 0x7fffff01u };
		auto badWarp = cursor->Warp( unknown, 1, 1 );
		Check( "cursor.warp_unknown_window",
		    !badWarp && badWarp.Error().status == WindowStatus::UnknownWindow &&
		        badWarp.Error().operation == WindowOperation::Warp );
		Check( "cursor.warp", !!cursor->Warp( m_A, 50, 60 ) );
		bool synthesized = false;
		for ( const Event &event : DrainOneByOne() )
			if ( event.type == EventType::MouseMotion && event.window == m_A && event.mouse.x == 50 &&
			     event.mouse.y == 60 )
				synthesized = true;
		Check( "cursor.warp_motion_suppressed", !synthesized );
		m_Driver.Motion( m_A, 70, 80, 20, 20 );
		std::vector<Event> after = Input();
		Check( "cursor.motion_after_warp",
		    after.size() == 1 && after[0].mouse.x == 70 && after[0].mouse.y == 80 );

		auto badRelative = cursor->SetRelativeMode( unknown, true );
		Check( "cursor.relative_unknown_window",
		    !badRelative && badRelative.Error().status == WindowStatus::UnknownWindow );
		auto relative = cursor->SetRelativeMode( m_A, true );
		if ( relative )
			Check( "cursor.relative_toggle", !!cursor->SetRelativeMode( m_A, false ) );
		else
			Check( "cursor.relative_structured_error",
			    relative.Error().status == WindowStatus::Unsupported ||
			        relative.Error().status == WindowStatus::NativeFailure );

		bool shapes = true;
		for ( std::uint32_t s = 0; s < std::uint32_t( CursorShape::Count ); ++s )
		{
			auto result = cursor->SetShape( CursorShape( s ) );
			shapes = shapes && ( result || result.Error().status == WindowStatus::Unsupported );
		}
		Check( "cursor.shapes", shapes && !!cursor->SetShape( CursorShape::Arrow ) );
		auto badShape = cursor->SetShape( CursorShape::Count );
		Check( "cursor.shape_invalid",
		    !badShape && badShape.Error().status == WindowStatus::InvalidArgument );
		Settle();
	}

	void Clipboard()
	{
		IClipboard *clipboard = m_Caps.clipboard;
		if ( !clipboard )
		{
			window_detail::Skip( m_Report, "clipboard.*", "provider exports no clipboard capability" );
			return;
		}
		const std::string text = "\xCE\xA9mega \xE2\x9C\x93 clipboard";
		Check( "clipboard.set", !!clipboard->SetText( text ) );
		auto read = clipboard->GetText();
		Check( "clipboard.roundtrip_utf8", read && read.Value() == text );
		Check( "clipboard.overwrite", !!clipboard->SetText( "second" ) && clipboard->GetText() &&
		                                  clipboard->GetText().Value() == "second" );
		Check( "clipboard.empty", !!clipboard->SetText( "" ) && clipboard->GetText() &&
		                              clipboard->GetText().Value().empty() );
		Settle();
	}

	void MessageBox()
	{
		IMessageBox *box = m_Caps.messageBox;
		if ( !box )
		{
			window_detail::Skip( m_Report, "message.*", "provider exports no message-box capability" );
			return;
		}
		auto unknown = box->ShowError( "t", "m", WindowId{ 0x7fffff02u } );
		Check( "message.unknown_parent",
		    !unknown && unknown.Error().status == WindowStatus::UnknownWindow &&
		        unknown.Error().operation == WindowOperation::ShowMessage );
		if ( !m_Driver.AutoDismissesMessages() )
		{
			window_detail::Skip( m_Report, "message.shown", "modal display needs a user to dismiss it" );
			return;
		}
		Check( "message.shown", !!box->ShowError( "Title", "Body text", m_A ) &&
		                            m_Driver.LastMessage() == "Title: Body text" );
	}

	void Gamepads()
	{
		bool buttons = true;
		for ( std::uint32_t b = 0; b < std::uint32_t( GamepadButton::Count ); ++b )
		{
			m_Driver.PadButton( 7, GamepadButton( b ), true );
			m_Driver.PadButton( 7, GamepadButton( b ), false );
			std::vector<Event> events = Input();
			buttons = buttons && events.size() == 2 && events[0].type == EventType::GamepadButtonDown &&
			          events[1].type == EventType::GamepadButtonUp && events[0].gamepad.instance == 7 &&
			          events[0].gamepad.button == GamepadButton( b ) && !events[0].window.IsValid();
		}
		Check( "gamepad.buttons", buttons );

		m_Driver.PadAxis( 7, GamepadAxis::LeftX, -32768 );
		m_Driver.PadAxis( 7, GamepadAxis::LeftY, 32767 );
		m_Driver.PadAxis( 7, GamepadAxis::RightX, 0 );
		m_Driver.PadAxis( 7, GamepadAxis::RightTrigger, 32767 );
		m_Driver.PadAxis( 7, GamepadAxis::LeftTrigger, -100 );
		std::vector<Event> axes = Input();
		Check( "gamepad.axes", axes.size() == 5 && axes[0].gamepad.axis == GamepadAxis::LeftX &&
		                           axes[0].gamepad.value == -1.0f && axes[1].gamepad.value == 1.0f &&
		                           axes[2].gamepad.value == 0.0f && axes[3].gamepad.value == 1.0f &&
		                           axes[4].gamepad.axis == GamepadAxis::LeftTrigger &&
		                           axes[4].gamepad.value == 0.0f );

		if ( m_Driver.PadDevice( 3, true ) )
		{
			std::vector<Event> added = Input();
			Check( "gamepad.added", added.size() == 1 && added[0].type == EventType::GamepadAdded &&
			                            added[0].gamepad.instance == 3 );
			if ( m_Caps.gamepads )
				Check( "gamepad.count", m_Caps.gamepads->ConnectedCount() == 1 );
			m_Driver.PadDevice( 3, false );
			std::vector<Event> removed = Input();
			Check( "gamepad.removed", removed.size() == 1 &&
			                              removed[0].type == EventType::GamepadRemoved &&
			                              removed[0].gamepad.instance == 3 );
			if ( m_Caps.gamepads )
				Check( "gamepad.count", m_Caps.gamepads->ConnectedCount() == 0 );
		}
		else
		{
			window_detail::Skip( m_Report, "gamepad.added", "the platform cannot simulate devices" );
		}

		if ( m_Caps.gamepads )
		{
			auto rumble = m_Caps.gamepads->Rumble( 0x7ffffff0u, 0.5f, 0.5f, 10 );
			Check( "gamepad.rumble_unknown_device",
			    !rumble && rumble.Error().status == WindowStatus::UnknownDevice &&
			        rumble.Error().operation == WindowOperation::Rumble );
		}
	}

	void Touch()
	{
		m_Driver.Touch( NativeTouch::Down, m_A, 11, 1, 0.25f, 0.75f, 0.0f, 0.0f, 0.5f );
		m_Driver.Touch( NativeTouch::Motion, m_A, 11, 1, 0.5f, 0.5f, 0.25f, -0.25f, 0.5f );
		m_Driver.Touch( NativeTouch::Up, m_A, 11, 1, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f );
		std::vector<Event> touch = Input();
		Check( "touch.sequence", touch.size() == 3 && touch[0].type == EventType::TouchDown &&
		                             touch[1].type == EventType::TouchMotion &&
		                             touch[2].type == EventType::TouchUp );
		Check( "touch.values", touch.size() == 3 && touch[0].touch.device == 11 &&
		                           touch[0].touch.finger == 1 && touch[0].touch.x == 0.25f &&
		                           touch[0].touch.y == 0.75f && touch[0].touch.pressure == 0.5f &&
		                           touch[1].touch.dx == 0.25f && touch[1].touch.dy == -0.25f );
	}

	bool Announced( WindowId window, NativeWindowEvent native, EventType type )
	{
		m_Driver.Window( window, native );
		for ( const Event &event : DrainOneByOne() )
			if ( event.type == type && event.window == window )
				return true;
		return false;
	}

	void WindowState()
	{
		Check( "window.focus_lost",
		    Announced( m_A, NativeWindowEvent::FocusLost, EventType::WindowFocusLost ) );
		Check( "window.focus_gained",
		    Announced( m_A, NativeWindowEvent::FocusGained, EventType::WindowFocusGained ) );
		Check( "window.close_requested",
		    Announced( m_A, NativeWindowEvent::CloseRequested, EventType::WindowCloseRequested ) );
		render::IRenderSurface *surface = m_Windows.Surface( m_A );
		Check( "window.close_keeps_window",
		    !!m_Windows.GetSize( m_A ) && surface &&
		        surface->GetStatus() == render::RenderSurfaceStatus::kAvailable );

		// Minimized: the surface reports zero area when the event arrives.
		m_Driver.Window( m_A, NativeWindowEvent::Minimized );
		bool zero = false, seen = false;
		Event event;
		for ( int guard = 0; guard < 4096 && !seen; ++guard )
		{
			if ( m_Events.Poll( std::span<Event>( &event, 1 ) ) == 1 )
			{
				if ( event.type == EventType::WindowMinimized && event.window == m_A )
				{
					seen = true;
					zero = surface && !surface->GetDrawableExtent().IsPresentable();
				}
			}
			else if ( !m_Events.Wait( m_Driver.SettleMs() ) )
			{
				break;
			}
		}
		Check( "window.minimized_zero_extent", seen && zero );
		const bool restored =
		    Announced( m_A, NativeWindowEvent::Restored, EventType::WindowRestored );
		Check( "window.restored_extent",
		    restored && surface && surface->GetDrawableExtent().IsPresentable() );
		Check( "window.hidden", Announced( m_B, NativeWindowEvent::Hidden, EventType::WindowHidden ) );
		Check( "window.shown", Announced( m_B, NativeWindowEvent::Shown, EventType::WindowShown ) );
		Settle();
	}

	void Resize()
	{
		auto bad = m_Windows.SetSize( m_A, 0, 10 );
		Check( "resize.rejects_zero",
		    !bad && bad.Error().status == WindowStatus::InvalidArgument &&
		        bad.Error().operation == WindowOperation::SetSize );
		auto unknown = m_Windows.SetSize( WindowId{ 0x7fffff03u }, 10, 10 );
		Check( "resize.unknown_window",
		    !unknown && unknown.Error().status == WindowStatus::UnknownWindow );

		render::IRenderSurface *surface = m_Windows.Surface( m_A );
		Check( "resize.request", !!m_Windows.SetSize( m_A, 400, 300 ) );
		bool resized = false, extentBeforeDelivery = true, pixelEvent = false;
		Event event;
		for ( std::uint32_t waited = 0; waited <= m_Options.resizeTimeoutMs && !( resized && pixelEvent ); )
		{
			if ( m_Events.Poll( std::span<Event>( &event, 1 ) ) == 1 )
			{
				if ( event.window == m_A && event.type == EventType::WindowResized &&
				     event.size.width == 400 && event.size.height == 300 )
					resized = true;
				if ( event.window == m_A && event.type == EventType::WindowPixelSizeChanged )
				{
					pixelEvent = true;
					extentBeforeDelivery = extentBeforeDelivery && surface &&
					                       surface->GetDrawableExtent().width ==
					                           std::uint32_t( event.size.width ) &&
					                       surface->GetDrawableExtent().height ==
					                           std::uint32_t( event.size.height );
				}
				continue;
			}
			m_Events.Wait( 50 );
			waited += 50;
		}
		Check( "resize.event", resized );
		Check( "resize.pixel_event", pixelEvent );
		Check( "resize.extent_before_delivery", pixelEvent && extentBeforeDelivery );
		auto size = m_Windows.GetSize( m_A );
		auto pixels = m_Windows.GetPixelSize( m_A );
		Check( "resize.size", size && size.Value().width == 400 && size.Value().height == 300 );
		Check( "resize.surface_matches_pixels",
		    pixels && surface &&
		        surface->GetDrawableExtent().width == std::uint32_t( pixels.Value().width ) &&
		        surface->GetDrawableExtent().height == std::uint32_t( pixels.Value().height ) );
		Settle();
	}

	void Lifecycle()
	{
		const struct
		{
			NativeAppEvent native;
			EventType type;
		} map[] = {
		    { NativeAppEvent::Quit, EventType::Quit },
		    { NativeAppEvent::Terminating, EventType::AppTerminating },
		    { NativeAppEvent::LowMemory, EventType::AppLowMemory },
		    { NativeAppEvent::WillEnterBackground, EventType::AppWillEnterBackground },
		};
		bool mapped = true;
		for ( const auto &entry : map )
		{
			m_Driver.App( entry.native );
			bool found = false;
			for ( const Event &event : DrainOneByOne() )
				if ( event.type == entry.type && !event.window.IsValid() )
					found = true;
			mapped = mapped && found;
		}
		Check( "app.events", mapped );

		render::IRenderSurface *surface = m_Windows.Surface( m_A );
		if ( !surface )
		{
			Check( "surface.background_release", false );
			return;
		}
		ProbeListener listener( *surface );
		Check( "surface.attach", surface->AttachListener( listener ) );
		ProbeListener other( *surface );
		Check( "surface.single_listener", !surface->AttachListener( other ) );
		const std::uint64_t generation = surface->GetGeneration();

		m_Driver.App( NativeAppEvent::DidEnterBackground );
		bool released = false;
		for ( const Event &event : DrainOneByOne() )
			if ( event.type == EventType::AppDidEnterBackground )
				released = listener.calls == 1 &&
				           surface->GetStatus() == render::RenderSurfaceStatus::kUnavailable;
		Check( "surface.background_release", released );

		m_Driver.App( NativeAppEvent::WillEnterForeground );
		m_Driver.App( NativeAppEvent::DidEnterForeground );
		bool restored = false;
		for ( const Event &event : DrainOneByOne() )
			if ( event.type == EventType::AppWillEnterForeground )
				restored = surface->GetStatus() == render::RenderSurfaceStatus::kAvailable &&
				           surface->GetGeneration() != generation;
		Check( "surface.foreground_restore", restored && listener.calls == 1 );
		surface->DetachListener( listener );
	}

	void Destroy()
	{
		render::IRenderSurface *surface = m_Windows.Surface( m_B );
		if ( !surface )
		{
			Check( "destroy.listener_notified", false );
			return;
		}
		ProbeListener listener( *surface );
		surface->AttachListener( listener );

		// Queued native input for the window must never surface after Destroy.
		m_Driver.Key( m_B, 0x07, 0, true );
		m_Driver.Button( m_B, 1, true, 3, 3 );
		m_Driver.Key( m_A, 0x08, 0, true );
		Check( "destroy.succeeds", !!m_Windows.Destroy( m_B ) );
		m_Destroyed.push_back( m_B );
		Check( "destroy.listener_notified", listener.calls == 1 &&
		                                        listener.statusAtCall !=
		                                            render::RenderSurfaceStatus::kDestroyed );
		Check( "destroy.surface_destroyed",
		    surface->GetStatus() == render::RenderSurfaceStatus::kDestroyed &&
		        !surface->GetDrawableExtent().IsPresentable() );
		Check( "destroy.surface_lookup_null", m_Windows.Surface( m_B ) == nullptr );
		auto size = m_Windows.GetSize( m_B );
		Check( "destroy.size_unknown", !size && size.Error().status == WindowStatus::UnknownWindow );

		bool leaked = false, kept = false;
		for ( const Event &event : DrainOneByOne() )
		{
			for ( WindowId gone : m_Destroyed )
				leaked = leaked || event.window == gone;
			kept = kept || ( event.type == EventType::KeyDown && event.key.usage == 0x08 );
		}
		Check( "destroy.drops_queued_events", !leaked );
		Check( "destroy.keeps_other_windows", kept );
		m_Driver.Key( m_A, 0x08, 0, false );
		Input();

		auto again = m_Windows.Destroy( m_B );
		Check( "destroy.twice_unknown",
		    !again && again.Error().status == WindowStatus::UnknownWindow &&
		        again.Error().operation == WindowOperation::Destroy );
		surface->DetachListener( listener );

		WindowDesc desc;
		desc.title = "platform.window D";
		desc.width = 32;
		desc.height = 32;
		auto d = m_Windows.Create( desc );
		bool fresh = !!d;
		for ( WindowId gone : m_Destroyed )
			fresh = fresh && !( d.Value() == gone );
		Check( "destroy.ids_not_reused", fresh );
		if ( d )
			Check( "destroy.last", !!m_Windows.Destroy( d.Value() ) );
		Check( "destroy.first", !!m_Windows.Destroy( m_A ) );
		Check( "destroy.all_unknown", m_Windows.Surface( m_A ) == nullptr );
		Settle();
	}

	WindowCapabilities m_Caps;
	IWindowSystem &m_Windows;
	IEventSource &m_Events;
	IWindowTestDriver &m_Driver;
	WindowSuiteOptions m_Options;
	WindowReport &m_Report;
	WindowId m_A;
	WindowId m_B;
	std::vector<WindowId> m_Destroyed;
};

} // namespace window_detail

// Runs the whole suite. The capabilities must stay alive and composed for the
// duration of the call; the suite destroys every window it creates.
inline WindowReport RunWindowConformance( const WindowCapabilities &caps,
    IWindowTestDriver &driver, const WindowSuiteOptions &options = WindowSuiteOptions{} )
{
	WindowReport report;
	if ( !caps.windows || !caps.events )
	{
		window_detail::Check( report, "compose.required_capabilities", false );
		return report;
	}
	window_detail::Suite( caps, driver, options, report ).Run();
	return report;
}

} // namespace platformtest

#endif // PLATFORMTEST_WINDOW_CONFORMANCE_H
