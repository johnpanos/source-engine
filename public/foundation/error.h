//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared compact error payload for strict C++20 first-party targets
//          (RFC 0001 "Results and errors", RFC 0006 M1).
//
//=============================================================================//

#ifndef FOUNDATION_ERROR_H
#define FOUNDATION_ERROR_H

#include <cstdint>
#include <type_traits>

namespace foundation
{

// A scoped enum: unscoped enumerations convert to int, scoped ones do not
// (std::is_scoped_enum is C++23).
template <typename T>
concept ScopedEnum = std::is_enum_v<T> && !std::is_convertible_v<T, int>;

// The error half of a domain's Expected<T, E>. Each domain owns its two scoped
// enums: Status, the compact domain code, and Operation, a stable identifier of
// the request that failed. nativeCode carries the provider's own error number
// (errno, an SDK result) and is 0 when the provider has none.
//
// It is a plain value: no allocation, no logging, no ownership and no string.
// Readable context is added at the application boundary, where the input path,
// provider or request is known. It is not an extension ABI; stable C entry
// points use their own sized, versioned result structs.
template <typename Status, typename Operation>
    requires ScopedEnum<Status> && ScopedEnum<Operation>
struct Error
{
	Status status{};
	Operation operation{};
	std::int32_t nativeCode = 0;

	constexpr bool HasNativeCode() const noexcept { return nativeCode != 0; }

	friend constexpr bool operator==( const Error &, const Error & ) noexcept = default;
};

} // namespace foundation

#endif // FOUNDATION_ERROR_H
