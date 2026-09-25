//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Move-only ownership of a non-pointer native handle (a file
//          descriptor, an integer or 64-bit API handle) for strict C++20
//          first-party targets (RFC 0006 "Ownership and scope"). Pointer
//          resources use std::unique_ptr with a custom deleter instead.
//
//=============================================================================//

#ifndef FOUNDATION_SCOPED_RESOURCE_H
#define FOUNDATION_SCOPED_RESOURCE_H

#include <concepts>
#include <utility>

namespace foundation
{

// Traits name the handle type, its "owns nothing" value, and how to release it.
// Close must not throw; it runs from destructors and move assignment.
template <typename Traits>
concept ResourceTraits = requires( typename Traits::Handle handle ) {
	{ Traits::Invalid() } noexcept -> std::same_as<typename Traits::Handle>;
	{ Traits::Close( handle ) } noexcept;
} && std::equality_comparable<typename Traits::Handle>;

// Owns at most one handle and closes it exactly once: on destruction, on Reset,
// or when a moved-in handle replaces it. Release() gives the handle up without
// closing it. The layout is exactly the handle's.
template <ResourceTraits Traits> class [[nodiscard]] ScopedResource
{
public:
	using Handle = typename Traits::Handle;

	constexpr ScopedResource() noexcept = default;
	explicit ScopedResource( Handle handle ) noexcept : m_Handle( handle ) {}
	ScopedResource( ScopedResource &&other ) noexcept : m_Handle( other.Release() ) {}
	ScopedResource &operator=( ScopedResource &&other ) noexcept
	{
		if ( this != &other )
			Reset( other.Release() );
		return *this;
	}
	ScopedResource( const ScopedResource & ) = delete;
	ScopedResource &operator=( const ScopedResource & ) = delete;
	~ScopedResource() { Reset(); }

	Handle Get() const noexcept { return m_Handle; }
	bool IsValid() const noexcept { return m_Handle != Traits::Invalid(); }
	explicit operator bool() const noexcept { return IsValid(); }

	// Owns `handle` from now on and closes the previous one. Resetting to the
	// handle already owned keeps it open.
	void Reset( Handle handle = Traits::Invalid() ) noexcept
	{
		const Handle previous = std::exchange( m_Handle, handle );
		if ( previous != Traits::Invalid() && previous != handle )
			Traits::Close( previous );
	}

	// Gives up ownership without closing; the caller now owns the handle.
	[[nodiscard]] Handle Release() noexcept { return std::exchange( m_Handle, Traits::Invalid() ); }

private:
	Handle m_Handle = Traits::Invalid();
};

} // namespace foundation

#endif // FOUNDATION_SCOPED_RESOURCE_H
