//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The single owner of window-provider input normalization (RFC 0001
//			rank 7, roadmap R14). Providers decode native events into the calls
//			below; this class applies the platform.window.v1 rules and appends
//			normalized events to a bounded FIFO the provider drains in Poll.
//
//			The rules are the SDL2 launcher's existing behavior
//			(appframework/sdlmgr.cpp), expressed without SDL types:
//
//			  - modifier state is tracked from the key the layout makes a key
//			    act as (the SDL keycode, not the scancode), left and right sides
//			    separately, and reported after the transition;
//			    caps lock counts while held, exactly like the launcher;
//			  - native mouse buttons 1/2/3 are left/middle/right; higher buttons
//			    fold onto X1 (even) and X2 (odd);
//			  - a press within the double-click time and distance of the previous
//			    press (of any button) is a double click, which starts a new pair;
//			  - after a warp, the next motion exactly at the warp target in that
//			    window is the synthesized one and is not delivered;
//			  - text is decoded one Unicode scalar value per event, malformed
//			    UTF-8 becoming U+FFFD.
//
//			Portable and deterministic: no native types, clocks or allocation
//			after construction.
//
//=============================================================================//

#ifndef PLATFORM_WINDOW_INPUT_NORMALIZER_H
#define PLATFORM_WINDOW_INPUT_NORMALIZER_H

#include "platform/window/window_events.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace platform::window
{

// Bounded FIFO of normalized events. A provider pulls one native event only when
// the FIFO is empty, so capacity bounds the expansion of a single native event
// (kMaxExpansion) and nothing is ever dropped.
class EventFifo
{
public:
	static constexpr std::size_t kMaxExpansion = 64;

	EventFifo();

	bool IsEmpty() const noexcept { return m_Head == m_Tail; }
	std::size_t Size() const noexcept { return m_Tail - m_Head; }

	// Returns false, adding nothing, when the FIFO is full.
	bool Push( const Event &event );

	// Moves up to out.size() events into 'out' and returns how many.
	std::size_t Drain( std::span<Event> out );

	// The most recently queued event, or null when empty.
	Event *Back() noexcept { return IsEmpty() ? nullptr : &m_Events[m_Tail - 1]; }

	// Removes every queued event that targets 'window'.
	void DropWindow( WindowId window );

	void Clear() noexcept;

private:
	std::vector<Event> m_Events;
	std::size_t m_Head = 0;
	std::size_t m_Tail = 0;
};

struct NormalizerPolicy
{
	// Defaults are the launcher's sdl_double_click_time / sdl_double_click_size.
	std::uint32_t doubleClickMs = 400;
	std::int32_t doubleClickDistance = 2;
};

class InputNormalizer
{
public:
	explicit InputNormalizer( NormalizerPolicy policy = NormalizerPolicy{} ) noexcept;

	// 'usage' is the USB HID usage of the physical key; 'layoutUsage' is the key
	// the active layout makes it act as (see KeyEvent), or 0 for 'usage'. Modifier
	// state follows 'layoutUsage', as the launcher's keycode tracking does.
	bool Key( EventFifo &out, WindowId window, std::uint32_t usage, std::uint32_t layoutUsage,
	    bool pressed, bool repeat );

	bool Text( EventFifo &out, WindowId window, std::string_view utf8 );

	// 'nativeButton' uses the X11/SDL numbering (1 left, 2 middle, 3 right, 4+).
	// 'timestampMs' is the provider's monotonic event time.
	bool Button( EventFifo &out, WindowId window, std::uint32_t nativeButton, bool pressed,
	    std::int32_t x, std::int32_t y, std::uint64_t timestampMs );

	bool Motion( EventFifo &out, WindowId window, std::int32_t x, std::int32_t y,
	    std::int32_t dx, std::int32_t dy );

	bool Wheel( EventFifo &out, WindowId window, std::int32_t x, std::int32_t y );

	// Records a warp so its synthesized motion is not delivered.
	void ExpectWarp( WindowId window, std::int32_t x, std::int32_t y ) noexcept;

	// Forgets per-window state for a destroyed window.
	void ForgetWindow( WindowId window ) noexcept;

	// Returns the normalizer to its initial state (a new provider instance).
	void Reset() noexcept;

	std::uint32_t Modifiers() const noexcept;
	std::uint32_t HeldButtons() const noexcept { return m_Buttons; }

	// Stick axes map [-32768, 32767] onto [-1, 1]; triggers map [0, 32767] onto
	// [0, 1]. Out-of-range input clamps.
	static float NormalizeAxis( GamepadAxis axis, std::int32_t raw ) noexcept;

private:
	NormalizerPolicy m_Policy;
	std::uint32_t m_HeldModifierKeys = 0; // one bit per modifier usage
	std::uint32_t m_Buttons = 0;
	bool m_HavePress = false;
	std::uint64_t m_PressTimeMs = 0;
	std::int32_t m_PressX = 0;
	std::int32_t m_PressY = 0;
	bool m_ExpectWarp = false;
	WindowId m_WarpWindow;
	std::int32_t m_WarpX = 0;
	std::int32_t m_WarpY = 0;
};

// Decodes one Unicode scalar value from 'utf8' at 'offset', advancing it.
// Malformed, overlong, surrogate and out-of-range sequences yield U+FFFD and
// consume one byte.
char32_t DecodeUtf8( std::string_view utf8, std::size_t &offset ) noexcept;

} // namespace platform::window

#endif // PLATFORM_WINDOW_INPUT_NORMALIZER_H
