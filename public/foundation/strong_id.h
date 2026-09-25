//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Strong identifier vocabulary for strict C++20 first-party targets
//          (RFC 0006 "Results and value types"): one distinct value type per
//          tag instead of an interchangeable integer.
//
//=============================================================================//

#ifndef FOUNDATION_STRONG_ID_H
#define FOUNDATION_STRONG_ID_H

#include <compare>
#include <cstddef>
#include <functional>
#include <type_traits>

namespace foundation
{

// An identifier of the kind named by Tag, represented by the integer Rep.
// Invalid is the reserved "names nothing" value and the default.
//
// Construct explicitly as Id{ n }; there is no implicit conversion from Rep, and
// identifiers of different tags neither convert nor compare. The layout is
// exactly Rep's, so an Id can replace a plain integer member without changing a
// struct's size or alignment.
template <typename Tag, typename Rep = unsigned int, Rep Invalid = Rep{}>
    requires std::is_integral_v<Rep>
struct StrongId
{
	using Representation = Rep;
	static constexpr Rep kInvalid = Invalid;

	Rep value = Invalid;

	constexpr bool IsValid() const noexcept { return value != Invalid; }

	friend constexpr bool operator==( StrongId, StrongId ) noexcept = default;
	friend constexpr auto operator<=>( StrongId, StrongId ) noexcept = default;
};

} // namespace foundation

template <typename Tag, typename Rep, Rep Invalid>
struct std::hash<foundation::StrongId<Tag, Rep, Invalid>>
{
	std::size_t operator()( foundation::StrongId<Tag, Rep, Invalid> id ) const noexcept
	{
		return std::hash<Rep>{}( id.value );
	}
};

#endif // FOUNDATION_STRONG_ID_H
