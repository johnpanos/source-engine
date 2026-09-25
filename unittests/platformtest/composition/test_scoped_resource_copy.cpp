//========= Copyright Valve Corporation, All rights reserved. ============//
// Sensitivity: a scoped resource must not be copyable (two owners would close twice).
#include "foundation/scoped_resource.h"
struct Traits
{
	using Handle = int;
	static constexpr int Invalid() noexcept { return -1; }
	static void Close( int ) noexcept {}
};
int main()
{
	foundation::ScopedResource<Traits> a( 1 );
	foundation::ScopedResource<Traits> b = a;
	return b.Get();
}
