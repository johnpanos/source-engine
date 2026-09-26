//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Window and input capability contracts (RFC 0001 rank 7, roadmap
//			R14). A window provider exports these capabilities to the
//			composition kernel (public/platform/composition.h); consumers
//			declare which ones they require and which are optional.
//
//			  IWindowSystem  required  windows and their opaque render surfaces
//			  IEventSource   required  normalized events in native order
//			  ICursor        optional  visibility, relative mode, warp, shape
//			  IClipboard     optional  UTF-8 text
//			  IMessageBox    optional  modal error reports
//			  IGamepads      optional  connected-device count and rumble
//
//			A provider that cannot do something omits the capability rather
//			than exporting a no-op; an absent optional capability is a null
//			reference from the composition's dependency view.
//
//			The surfaces are render::IRenderSurface objects (R16's
//			render.presentation.v1); the window system owns them and no native
//			handle is reachable from this header. Every method is called on the
//			composition's owning sequence. Obligations are pinned by
//			platform.window.v1 (unittests/platformtest/contracts/).
//
//=============================================================================//

#ifndef PLATFORM_WINDOW_WINDOW_SYSTEM_H
#define PLATFORM_WINDOW_WINDOW_SYSTEM_H

#include "foundation/error.h"
#include "foundation/expected.h"
#include "platform/window/window_events.h"
#include "render/render_presentation.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace platform::window
{

enum class WindowStatus : std::uint32_t
{
	InvalidArgument,
	UnknownWindow, // never created, or already destroyed
	UnknownDevice,
	Unsupported, // the provider exports the capability but not this request
	NativeFailure,
	TooManyWindows,
};

// Stable operation identifiers for WindowError::operation.
enum class WindowOperation : std::uint32_t
{
	Create,
	Destroy,
	GetSize,
	SetSize,
	SetTitle,
	SetCursorVisible,
	SetRelativeMode,
	Warp,
	SetCursorShape,
	SetClipboard,
	GetClipboard,
	ShowMessage,
	Rumble,
};

// Compact, allocation-free error (the shared foundation payload). 'nativeCode'
// is provider-defined and zero when the provider has none; readable context is
// added by the caller.
using WindowError = foundation::Error<WindowStatus, WindowOperation>;

struct WindowDesc
{
	std::string_view title;
	std::int32_t width = 0; // window coordinates; both must be positive
	std::int32_t height = 0;
	bool resizable = true;
	bool hidden = false;
};

class IWindowSystem
{
public:
	static constexpr std::string_view kCapabilityName = "platform.window-system";

	virtual ~IWindowSystem() = default;

	[[nodiscard]] virtual foundation::Expected<WindowId, WindowError> Create(
	    const WindowDesc &desc ) = 0;

	// Before returning, tells the surface's attached presentation that the native
	// surface is going away, marks the surface kDestroyed and drops every queued
	// event for the window. The surface object stays valid while a presentation
	// remains attached; the window system reclaims it later.
	[[nodiscard]] virtual foundation::Expected<void, WindowError> Destroy( WindowId window ) = 0;

	// Logical size in window coordinates, and drawable size in pixels.
	[[nodiscard]] virtual foundation::Expected<SizeEvent, WindowError> GetSize(
	    WindowId window ) const = 0;
	[[nodiscard]] virtual foundation::Expected<SizeEvent, WindowError> GetPixelSize(
	    WindowId window ) const = 0;

	// Requests a new logical size. Completion is reported by WindowResized, which
	// may arrive after later Poll calls on asynchronous platforms.
	[[nodiscard]] virtual foundation::Expected<void, WindowError> SetSize(
	    WindowId window, std::int32_t width, std::int32_t height ) = 0;
	[[nodiscard]] virtual foundation::Expected<void, WindowError> SetTitle(
	    WindowId window, std::string_view title ) = 0;

	// The window's surface, or null for an unknown window. The surface's drawable
	// extent is updated before the size event that announces it is delivered.
	virtual render::IRenderSurface *Surface( WindowId window ) = 0;
};

class IEventSource
{
public:
	static constexpr std::string_view kCapabilityName = "platform.event-source";

	virtual ~IEventSource() = default;

	// Copies up to out.size() pending events in native order and returns how many
	// were written. Events not copied stay queued for the next call; nothing is
	// dropped or duplicated. An empty span consumes nothing.
	virtual std::size_t Poll( std::span<Event> out ) = 0;

	// Waits up to 'timeoutMs' for an event and returns whether Poll would now
	// return at least one.
	virtual bool Wait( std::uint32_t timeoutMs ) = 0;
};

enum class CursorShape : std::uint32_t
{
	Arrow,
	IBeam,
	Wait,
	WaitArrow,
	Crosshair,
	SizeNWSE,
	SizeNESW,
	SizeWE,
	SizeNS,
	SizeAll,
	No,
	Hand,
	Count,
};

class ICursor
{
public:
	static constexpr std::string_view kCapabilityName = "platform.cursor";

	virtual ~ICursor() = default;

	[[nodiscard]] virtual foundation::Expected<void, WindowError> SetVisible( bool visible ) = 0;
	virtual bool IsVisible() const = 0;

	// Relative mode hides the cursor and reports unbounded motion deltas.
	[[nodiscard]] virtual foundation::Expected<void, WindowError> SetRelativeMode(
	    WindowId window, bool enabled ) = 0;

	// Moves the pointer. The motion event a platform synthesizes for the warp
	// itself is not delivered.
	[[nodiscard]] virtual foundation::Expected<void, WindowError> Warp(
	    WindowId window, std::int32_t x, std::int32_t y ) = 0;

	[[nodiscard]] virtual foundation::Expected<void, WindowError> SetShape( CursorShape shape ) = 0;
};

class IClipboard
{
public:
	static constexpr std::string_view kCapabilityName = "platform.clipboard";

	virtual ~IClipboard() = default;

	// UTF-8 text. An empty clipboard reads back as an empty string.
	[[nodiscard]] virtual foundation::Expected<void, WindowError> SetText(
	    std::string_view utf8 ) = 0;
	[[nodiscard]] virtual foundation::Expected<std::string, WindowError> GetText() = 0;
};

class IMessageBox
{
public:
	static constexpr std::string_view kCapabilityName = "platform.message-box";

	virtual ~IMessageBox() = default;

	// Shows a modal error report and returns after the user dismisses it. An
	// invalid 'parent' shows it unparented; an unknown one is an error.
	[[nodiscard]] virtual foundation::Expected<void, WindowError> ShowError(
	    std::string_view title, std::string_view message, WindowId parent ) = 0;
};

class IGamepads
{
public:
	static constexpr std::string_view kCapabilityName = "platform.gamepads";

	virtual ~IGamepads() = default;

	virtual std::size_t ConnectedCount() const = 0;

	// Intensities in [0, 1]; a zero duration stops rumble.
	[[nodiscard]] virtual foundation::Expected<void, WindowError> Rumble( std::uint32_t instance,
	    float lowFrequency, float highFrequency, std::uint32_t durationMs ) = 0;
};

} // namespace platform::window

#endif // PLATFORM_WINDOW_WINDOW_SYSTEM_H
