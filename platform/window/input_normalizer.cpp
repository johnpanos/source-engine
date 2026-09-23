//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Window-provider input normalization. See input_normalizer.h.
//
//=============================================================================//

#include "input_normalizer.h"

namespace platform::window
{

namespace
{

constexpr char32_t kReplacement = 0xFFFD;

// Bit 0 is caps lock; bits 1-8 are the HID usages 0xE0-0xE7 (left control,
// shift, alt, GUI, then the right-hand keys).
constexpr std::uint32_t kCapsBit = 1u << 0;
constexpr std::uint32_t kControlBits = ( 1u << 1 ) | ( 1u << 5 );
constexpr std::uint32_t kShiftBits = ( 1u << 2 ) | ( 1u << 6 );
constexpr std::uint32_t kAltBits = ( 1u << 3 ) | ( 1u << 7 );
constexpr std::uint32_t kGuiBits = ( 1u << 4 ) | ( 1u << 8 );

std::uint32_t ModifierKeyBit( std::uint32_t usage ) noexcept
{
	if ( usage == kUsageCapsLock )
		return kCapsBit;
	if ( usage >= kUsageLeftControl && usage <= kUsageRightGui )
		return 1u << ( usage - kUsageLeftControl + 1 );
	return 0;
}

std::int32_t Distance( std::int32_t a, std::int32_t b ) noexcept
{
	return a > b ? a - b : b - a;
}

MouseButton FoldButton( std::uint32_t nativeButton ) noexcept
{
	switch ( nativeButton )
	{
	case 0:
		return MouseButton::None;
	case 1:
		return MouseButton::Left;
	case 2:
		return MouseButton::Middle;
	case 3:
		return MouseButton::Right;
	default:
		// Mice number their extra buttons differently; the launcher has always
		// folded every button above 3 onto 4 and 5 by parity.
		return ( nativeButton & 1u ) ? MouseButton::X2 : MouseButton::X1;
	}
}

Event MakeEvent( EventType type, WindowId window ) noexcept
{
	Event event;
	event.type = type;
	event.window = window;
	return event;
}

} // namespace

EventFifo::EventFifo() : m_Events( kMaxExpansion ) {}

bool EventFifo::Push( const Event &event )
{
	if ( m_Head == m_Tail )
		m_Head = m_Tail = 0;
	if ( m_Tail == m_Events.size() )
	{
		if ( m_Head == 0 )
			return false;
		// Compact in place; capacity never grows.
		std::size_t count = m_Tail - m_Head;
		for ( std::size_t i = 0; i < count; ++i )
			m_Events[i] = m_Events[m_Head + i];
		m_Head = 0;
		m_Tail = count;
	}
	m_Events[m_Tail++] = event;
	return true;
}

std::size_t EventFifo::Drain( std::span<Event> out )
{
	std::size_t count = 0;
	while ( count < out.size() && m_Head != m_Tail )
		out[count++] = m_Events[m_Head++];
	return count;
}

void EventFifo::DropWindow( WindowId window )
{
	std::size_t kept = m_Head;
	for ( std::size_t i = m_Head; i < m_Tail; ++i )
	{
		if ( m_Events[i].window == window )
			continue;
		m_Events[kept++] = m_Events[i];
	}
	m_Tail = kept;
}

void EventFifo::Clear() noexcept
{
	m_Head = m_Tail = 0;
}

InputNormalizer::InputNormalizer( NormalizerPolicy policy ) noexcept : m_Policy( policy ) {}

std::uint32_t InputNormalizer::Modifiers() const noexcept
{
	const std::uint32_t held = m_HeldModifierKeys;
	std::uint32_t mask = 0;
	if ( held & kCapsBit )
		mask |= kModifierCapsLock;
	if ( held & kShiftBits )
		mask |= kModifierShift;
	if ( held & kControlBits )
		mask |= kModifierControl;
	if ( held & kAltBits )
		mask |= kModifierAlt;
	if ( held & kGuiBits )
		mask |= kModifierGui;
	return mask;
}

bool InputNormalizer::Key( EventFifo &out, WindowId window, std::uint32_t usage,
    std::uint32_t layoutUsage, bool pressed, bool repeat )
{
	if ( layoutUsage == 0 )
		layoutUsage = usage;
	if ( const std::uint32_t bit = ModifierKeyBit( layoutUsage ) )
	{
		if ( pressed )
			m_HeldModifierKeys |= bit;
		else
			m_HeldModifierKeys &= ~bit;
	}

	Event event = MakeEvent( pressed ? EventType::KeyDown : EventType::KeyUp, window );
	event.key.usage = usage;
	event.key.layoutUsage = layoutUsage;
	event.key.modifiers = Modifiers();
	event.key.repeat = repeat;
	return out.Push( event );
}

bool InputNormalizer::Text( EventFifo &out, WindowId window, std::string_view utf8 )
{
	std::size_t offset = 0;
	while ( offset < utf8.size() && utf8[offset] != '\0' )
	{
		Event event = MakeEvent( EventType::TextInput, window );
		event.text.codepoint = DecodeUtf8( utf8, offset );
		event.text.modifiers = Modifiers();
		if ( !out.Push( event ) )
			return false;
	}
	return true;
}

bool InputNormalizer::Button( EventFifo &out, WindowId window, std::uint32_t nativeButton,
    bool pressed, std::int32_t x, std::int32_t y, std::uint64_t timestampMs )
{
	const MouseButton button = FoldButton( nativeButton );
	if ( pressed )
		m_Buttons |= MouseButtonMask( button );
	else
		m_Buttons &= ~MouseButtonMask( button );

	std::uint8_t clicks = 1;
	if ( pressed )
	{
		if ( m_HavePress && timestampMs >= m_PressTimeMs &&
		     timestampMs - m_PressTimeMs <= m_Policy.doubleClickMs &&
		     Distance( x, m_PressX ) <= m_Policy.doubleClickDistance &&
		     Distance( y, m_PressY ) <= m_Policy.doubleClickDistance )
		{
			clicks = 2;
			m_HavePress = false;
		}
		else
		{
			m_PressTimeMs = timestampMs;
			m_PressX = x;
			m_PressY = y;
			m_HavePress = true;
		}
	}

	Event event = MakeEvent( pressed ? EventType::MouseButtonDown : EventType::MouseButtonUp, window );
	event.mouse.x = x;
	event.mouse.y = y;
	event.mouse.button = button;
	event.mouse.clicks = clicks;
	event.mouse.buttons = m_Buttons;
	return out.Push( event );
}

bool InputNormalizer::Motion(
    EventFifo &out, WindowId window, std::int32_t x, std::int32_t y, std::int32_t dx, std::int32_t dy )
{
	if ( m_ExpectWarp && window == m_WarpWindow && x == m_WarpX && y == m_WarpY )
	{
		m_ExpectWarp = false;
		return true;
	}

	Event event = MakeEvent( EventType::MouseMotion, window );
	event.mouse.x = x;
	event.mouse.y = y;
	event.mouse.dx = dx;
	event.mouse.dy = dy;
	event.mouse.buttons = m_Buttons;
	return out.Push( event );
}

bool InputNormalizer::Wheel( EventFifo &out, WindowId window, std::int32_t x, std::int32_t y )
{
	if ( x == 0 && y == 0 )
		return true;
	Event event = MakeEvent( EventType::MouseWheel, window );
	event.wheel.x = x;
	event.wheel.y = y;
	return out.Push( event );
}

void InputNormalizer::ExpectWarp( WindowId window, std::int32_t x, std::int32_t y ) noexcept
{
	m_ExpectWarp = true;
	m_WarpWindow = window;
	m_WarpX = x;
	m_WarpY = y;
}

void InputNormalizer::ForgetWindow( WindowId window ) noexcept
{
	if ( m_ExpectWarp && m_WarpWindow == window )
		m_ExpectWarp = false;
}

void InputNormalizer::Reset() noexcept
{
	*this = InputNormalizer( m_Policy );
}

float InputNormalizer::NormalizeAxis( GamepadAxis axis, std::int32_t raw ) noexcept
{
	const bool trigger = axis == GamepadAxis::LeftTrigger || axis == GamepadAxis::RightTrigger;
	float value = static_cast<float>( raw ) / 32767.0f;
	const float low = trigger ? 0.0f : -1.0f;
	if ( value < low )
		value = low;
	if ( value > 1.0f )
		value = 1.0f;
	return value;
}

char32_t DecodeUtf8( std::string_view utf8, std::size_t &offset ) noexcept
{
	const auto byte = [&]( std::size_t i ) -> std::uint32_t
	{ return static_cast<unsigned char>( utf8[i] ); };

	const std::uint32_t lead = byte( offset );
	std::size_t length = 0;
	char32_t value = 0;
	char32_t minimum = 0;
	if ( lead < 0x80 )
	{
		++offset;
		return lead;
	}
	if ( ( lead & 0xE0 ) == 0xC0 )
	{
		length = 2;
		value = lead & 0x1F;
		minimum = 0x80;
	}
	else if ( ( lead & 0xF0 ) == 0xE0 )
	{
		length = 3;
		value = lead & 0x0F;
		minimum = 0x800;
	}
	else if ( ( lead & 0xF8 ) == 0xF0 )
	{
		length = 4;
		value = lead & 0x07;
		minimum = 0x10000;
	}
	else
	{
		++offset;
		return kReplacement;
	}

	if ( offset + length > utf8.size() )
	{
		++offset;
		return kReplacement;
	}
	for ( std::size_t i = 1; i < length; ++i )
	{
		const std::uint32_t next = byte( offset + i );
		if ( ( next & 0xC0 ) != 0x80 )
		{
			++offset;
			return kReplacement;
		}
		value = ( value << 6 ) | ( next & 0x3F );
	}
	if ( value < minimum || value > 0x10FFFF || ( value >= 0xD800 && value <= 0xDFFF ) )
	{
		++offset;
		return kReplacement;
	}
	offset += length;
	return value;
}

} // namespace platform::window
