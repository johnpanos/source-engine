//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef PLATFORM_LEGACY_BINDING_H
#define PLATFORM_LEGACY_BINDING_H

#include "foundation/expected.h"

namespace platform
{
enum class LegacyBindingError { Occupied, AlreadyBound };

// Domain adapter owns the slot; composition owns this scope. The slot and the
// provider outlive the scope. Access is on the owning sequence, never concurrent.
template < typename Contract >
class ScopedLegacyBinding
{
public:
	ScopedLegacyBinding() = default;
	~ScopedLegacyBinding() { Reset(); }
	ScopedLegacyBinding( const ScopedLegacyBinding & ) = delete;
	ScopedLegacyBinding &operator=( const ScopedLegacyBinding & ) = delete;

	foundation::Expected< void, LegacyBindingError > Bind( Contract *&slot, Contract &provider )
	{
		if ( m_Slot ) return foundation::MakeUnexpected( LegacyBindingError::AlreadyBound );
		if ( slot ) return foundation::MakeUnexpected( LegacyBindingError::Occupied );
		m_Slot = &slot;
		slot = &provider;
		return {};
	}
	void Reset() noexcept
	{
		if ( m_Slot ) { *m_Slot = nullptr; m_Slot = nullptr; }
	}
private:
	Contract **m_Slot = nullptr;
};
}
#endif
