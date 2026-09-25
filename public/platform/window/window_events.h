//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Normalized window and input events (RFC 0001 rank 7, roadmap R14).
//			A window provider translates its native events into these values;
//			portable consumers never see an SDL, Win32, Cocoa or Android type.
//
//			Obligations are pinned by platform.window.v1 (see
//			unittests/platformtest/contracts/platform.window.v1.md). The
//			normalization rules preserve what the SDL2 launcher provider
//			(appframework/sdlmgr.cpp) already does: scancodes are USB HID
//			keyboard usages, left and right modifier keys are tracked
//			separately, native mouse buttons above 3 fold onto X1/X2 by parity,
//			double clicks use a time/distance window, and text arrives one
//			Unicode scalar value at a time. Launcher product policy (focus-gated
//			motion, recentring warps, raw-input selection) is not part of the
//			provider and stays with its consumer.
//
//=============================================================================//

#ifndef PLATFORM_WINDOW_WINDOW_EVENTS_H
#define PLATFORM_WINDOW_WINDOW_EVENTS_H

#include "foundation/strong_id.h"

#include <cstdint>

namespace platform::window
{

// Identifies one window of one window system. Zero is never a live window.
// Values are not reused while the window system is alive.
struct WindowIdTag;
using WindowId = foundation::StrongId<WindowIdTag, std::uint32_t>;

enum class EventType : std::uint32_t
{
	None = 0,

	// Application lifecycle. 'window' is invalid.
	Quit,
	AppTerminating,
	AppLowMemory,
	AppWillEnterBackground,
	AppDidEnterBackground,
	AppWillEnterForeground,
	AppDidEnterForeground,

	// Window state. 'size' is set for the two size events.
	WindowShown,
	WindowHidden,
	WindowMinimized,
	WindowRestored,
	WindowResized,          // logical (window-coordinate) size changed
	WindowPixelSizeChanged, // drawable pixel size changed
	WindowFocusGained,
	WindowFocusLost,
	WindowCloseRequested, // the window stays open until the consumer destroys it

	// Keyboard and text. 'key' or 'text' is set.
	KeyDown,
	KeyUp,
	TextInput,

	// Mouse. 'mouse' or 'wheel' is set.
	MouseMotion,
	MouseButtonDown,
	MouseButtonUp,
	MouseWheel,

	// Gamepads. 'gamepad' is set; 'window' is invalid.
	GamepadAdded,
	GamepadRemoved,
	GamepadButtonDown,
	GamepadButtonUp,
	GamepadAxis,

	// Touch. 'touch' is set; 'window' is set when the platform reports one.
	TouchDown,
	TouchUp,
	TouchMotion,
};

// Modifier state bits. The bit positions equal the legacy ECocoaKeyModifier
// shifts, so a legacy adapter copies the mask unchanged.
enum ModifierBits : std::uint32_t
{
	kModifierCapsLock = 1u << 0,
	kModifierShift = 1u << 1,
	kModifierControl = 1u << 2,
	kModifierAlt = 1u << 3,
	kModifierGui = 1u << 4,
};

// USB HID keyboard usages (page 0x07) that carry modifier state.
enum KeyboardUsage : std::uint32_t
{
	kUsageCapsLock = 0x39,
	kUsageLeftControl = 0xE0,
	kUsageLeftShift = 0xE1,
	kUsageLeftAlt = 0xE2,
	kUsageLeftGui = 0xE3,
	kUsageRightControl = 0xE4,
	kUsageRightShift = 0xE5,
	kUsageRightAlt = 0xE6,
	kUsageRightGui = 0xE7,
};

// Portable mouse buttons. The numbering equals the legacy launcher's button
// index, so the held-button mask bit for button b is 1 << (b - 1).
enum class MouseButton : std::uint8_t
{
	None = 0,
	Left = 1,
	Right = 2,
	Middle = 3,
	X1 = 4,
	X2 = 5,
};

constexpr std::uint32_t MouseButtonMask( MouseButton button ) noexcept
{
	return button == MouseButton::None ? 0u : 1u << ( static_cast<std::uint32_t>( button ) - 1u );
}

// Gamepad buttons and axes in the common "standard gamepad" layout: South is
// the bottom face button (A on an Xbox pad, Cross on a PlayStation pad).
enum class GamepadButton : std::uint8_t
{
	South,
	East,
	West,
	North,
	Back,
	Guide,
	Start,
	LeftStick,
	RightStick,
	LeftShoulder,
	RightShoulder,
	DpadUp,
	DpadDown,
	DpadLeft,
	DpadRight,
	Count,
};

enum class GamepadAxis : std::uint8_t
{
	LeftX,
	LeftY,
	RightX,
	RightY,
	LeftTrigger,
	RightTrigger,
	Count,
};

struct KeyEvent
{
	std::uint32_t usage = 0; // USB HID keyboard usage of the physical key; 0 when unknown
	// The key the active layout makes this key act as: for letters and
	// = - [ ] ; ' , . / the US-QWERTY key with the same legend (the launcher's
	// -nonqwerty mapping), for a remapped modifier the modifier it acts as;
	// otherwise equal to 'usage'. Modifier state follows this value.
	std::uint32_t layoutUsage = 0;
	std::uint32_t modifiers = 0; // state after this key's transition
	bool repeat = false;
};

struct TextEvent
{
	char32_t codepoint = 0;      // one Unicode scalar value; U+FFFD for malformed input
	std::uint32_t modifiers = 0; // current modifier state
};

struct MouseEvent
{
	std::int32_t x = 0; // window coordinates
	std::int32_t y = 0;
	std::int32_t dx = 0; // motion only
	std::int32_t dy = 0;
	MouseButton button = MouseButton::None; // button events only
	std::uint8_t clicks = 0;                // button-down: 1, or 2 for a double click
	std::uint32_t buttons = 0;              // held-button mask after this event
};

struct WheelEvent
{
	std::int32_t x = 0; // detents; positive is right
	std::int32_t y = 0; // detents; positive is away from the user
};

struct SizeEvent
{
	std::int32_t width = 0;
	std::int32_t height = 0;
};

struct GamepadEvent
{
	std::uint32_t instance = 0; // stable while the device stays connected
	GamepadButton button = GamepadButton::Count;
	GamepadAxis axis = GamepadAxis::Count;
	float value = 0.0f; // sticks in [-1, 1], triggers in [0, 1]
};

struct TouchEvent
{
	std::uint64_t device = 0;
	std::uint64_t finger = 0;
	float x = 0.0f; // normalized to [0, 1] across the touch surface
	float y = 0.0f;
	float dx = 0.0f;
	float dy = 0.0f;
	float pressure = 0.0f;
};

// One normalized event. Only the member named by 'type' is meaningful; the
// others keep their default values.
struct Event
{
	EventType type = EventType::None;
	WindowId window;
	KeyEvent key;
	TextEvent text;
	MouseEvent mouse;
	WheelEvent wheel;
	SizeEvent size;
	GamepadEvent gamepad;
	TouchEvent touch;
};

} // namespace platform::window

#endif // PLATFORM_WINDOW_WINDOW_EVENTS_H
