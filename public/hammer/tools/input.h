//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Normalized editor input and tool output values (RFC 0002,
//			hammer.tools; "Input and tool contracts"). A native host (GTK, MFC,
//			a script or a test) converts each native event once into these
//			values; tools never see toolkit events, key state globals or
//			widgets. Tools answer with a ToolResult, and describe what the host
//			should draw (OverlayList), which cursor to show (Cursor) and a
//			status line; they never paint.
//
//			Pointer. Coordinates are the view's pixel space (viewport/camera.h:
//			origin top-left, y down). 'button' is the button that changed for
//			Down/Up; for Move it is the button held (None for a hover).
//
//			Keys. Logical keys; letters, digits and punctuation arrive as
//			Key::Character with the unshifted lower-case ASCII character
//			('x' for Shift+X, with kShift in the modifiers). Text and IME
//			composition are not tool input (presenters own text entry).
//
//			Wheel. 'steps' are notches (fractional for precise devices);
//			positive means away from the user (scroll up).
//
//			Results. Ignored = not relevant to this tool (valid); Handled =
//			consumed; CaptureRequested = the tool began a gesture and wants
//			every pointer event of this view until it releases;
//			CaptureReleased = the gesture ended (with or without an edit);
//			Failed = the requested edit was refused (the message says why) and
//			nothing changed.
//
//			Overlay. A value list of primitives with a style role; the
//			presenter maps roles to colors and line styles. World primitives
//			are in map units; screen primitives are in the pixel space of the
//			view the overlay was built for.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_INPUT_H
#define HAMMER_TOOLS_INPUT_H

#include "hammer/viewport/camera.h"
#include "mapgeometry/brush.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace hammer::tools
{

// --- Modifiers ----------------------------------------------------------------------

constexpr std::uint8_t kShift = 1u << 0;
constexpr std::uint8_t kCtrl = 1u << 1;
constexpr std::uint8_t kAlt = 1u << 2;

struct Modifiers
{
	std::uint8_t bits = 0;

	constexpr bool Shift() const { return ( bits & kShift ) != 0; }
	constexpr bool Ctrl() const { return ( bits & kCtrl ) != 0; }
	constexpr bool Alt() const { return ( bits & kAlt ) != 0; }
	constexpr bool Any() const { return bits != 0; }

	friend constexpr bool operator==( const Modifiers &, const Modifiers & ) = default;
};

constexpr Modifiers Mods( std::uint8_t bits = 0 )
{
	return Modifiers{ bits };
}

// --- Pointer ------------------------------------------------------------------------

enum class PointerPhase
{
	Down,
	Move,
	Up,
};

enum class PointerButton
{
	None,
	Left,
	Middle,
	Right,
};

struct PointerEvent
{
	PointerPhase phase = PointerPhase::Move;
	PointerButton button = PointerButton::None;
	Modifiers modifiers;
	double x = 0.0;
	double y = 0.0;
	viewport::ViewKind view = viewport::ViewKind::Top;
	int clickCount = 1;
	std::uint64_t timestampMs = 0;

	viewport::ScreenPoint Point() const { return { x, y }; }
};

// --- Keys ---------------------------------------------------------------------------

enum class Key
{
	None,
	Character, // KeyEvent::character holds the lower-case ASCII character
	Escape,
	Enter,
	Delete,
	Backspace,
	Left,
	Right,
	Up,
	Down,
	PageUp,
	PageDown,
	Home,
	End,
	Tab,
	Space,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
};

enum class KeyPhase
{
	Press,
	Release,
};

struct KeyEvent
{
	Key key = Key::None;
	char character = 0; // Key::Character only: 'a'..'z', '0'..'9', '[', ']', ...
	KeyPhase phase = KeyPhase::Press;
	Modifiers modifiers;
	bool repeat = false; // auto-repeat of a held key

	static KeyEvent Press( Key key, Modifiers modifiers = {} );
	static KeyEvent Release( Key key, Modifiers modifiers = {} );
	// A character key; upper-case letters are folded to lower case.
	static KeyEvent Char(
	    char character, Modifiers modifiers = {}, KeyPhase phase = KeyPhase::Press );

	bool IsPress() const { return phase == KeyPhase::Press; }
	bool IsChar( char c ) const { return key == Key::Character && character == c; }
};

// --- Wheel --------------------------------------------------------------------------

struct WheelEvent
{
	double steps = 0.0; // notches; positive = away from the user
	double x = 0.0;
	double y = 0.0;
	viewport::ViewKind view = viewport::ViewKind::Top;
	Modifiers modifiers;
};

// Why a gesture is abandoned. Every reason cancels: no edit is made and the
// pre-gesture state is restored.
enum class CancelReason
{
	Escape,
	FocusLost,
	CaptureLost,
	ToolSwitched,
	DocumentReplaced,
};

const char *CancelReasonName( CancelReason reason );

// --- Results --------------------------------------------------------------------------

enum class ToolResultKind
{
	Ignored,
	Handled,
	CaptureRequested,
	CaptureReleased,
	Failed,
};

const char *ToolResultName( ToolResultKind kind );

struct ToolResult
{
	ToolResultKind kind = ToolResultKind::Ignored;
	std::string message; // Failed: why; otherwise optional detail

	static ToolResult Ignore() { return {}; }
	static ToolResult Handle() { return { ToolResultKind::Handled, {} }; }
	static ToolResult Capture() { return { ToolResultKind::CaptureRequested, {} }; }
	static ToolResult Release() { return { ToolResultKind::CaptureReleased, {} }; }
	static ToolResult Fail( std::string message )
	{
		return { ToolResultKind::Failed, std::move( message ) };
	}

	bool Is( ToolResultKind k ) const { return kind == k; }
};

// --- Cursor intent ---------------------------------------------------------------------

enum class Cursor
{
	Default,
	Crosshair,
	Move,
	ResizeN,
	ResizeS,
	ResizeE,
	ResizeW,
	ResizeNE,
	ResizeNW,
	ResizeSE,
	ResizeSW,
	Rotate,
	Forbidden,
	Hand,
	Text,
};

const char *CursorName( Cursor cursor );

// --- Overlay ---------------------------------------------------------------------------

enum class OverlayRole
{
	Selection,
	Pending,
	Handle,
	HandleHot,
	Hover,
	Clip,
	Error,
};

enum class OverlayKind
{
	WorldLine,    // world[0] -> world[1]
	WorldBox,     // world[0] = mins, world[1] = maxs
	WorldPolygon, // world = the vertices, closed
	ScreenRect,   // a -> b (corners)
	ScreenHandle, // centered on a, half size 'size' pixels
	ScreenLabel,  // text anchored at a
};

enum class HandleShape
{
	Square,
	Circle,
};

struct OverlayItem
{
	OverlayKind kind = OverlayKind::WorldLine;
	OverlayRole role = OverlayRole::Pending;
	std::vector<mapgeometry::Vec3d> world;
	viewport::ScreenPoint a;
	viewport::ScreenPoint b;
	double size = 0.0;
	HandleShape shape = HandleShape::Square;
	std::string text;
};

struct OverlayList
{
	std::vector<OverlayItem> items;

	void Line( const mapgeometry::Vec3d &from, const mapgeometry::Vec3d &to, OverlayRole role );
	void Box( const mapgeometry::Vec3d &mins, const mapgeometry::Vec3d &maxs, OverlayRole role );
	void Polygon( std::vector<mapgeometry::Vec3d> points, OverlayRole role );
	void Rect( viewport::ScreenPoint a, viewport::ScreenPoint b, OverlayRole role );
	void Handle(
	    viewport::ScreenPoint center, double halfSize, HandleShape shape, OverlayRole role );
	void Label( viewport::ScreenPoint anchor, std::string text, OverlayRole role );
	void Append( const OverlayList &other );

	// The number of items of 'kind' with 'role'.
	std::size_t Count( OverlayKind kind, OverlayRole role ) const;
	// The number of items with 'role' (any kind).
	std::size_t CountRole( OverlayRole role ) const;
	bool Empty() const { return items.empty(); }
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_INPUT_H
