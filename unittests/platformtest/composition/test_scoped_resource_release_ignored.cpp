//========= Copyright Valve Corporation, All rights reserved. ============//
// Sensitivity: discarding Release()'s handle leaks it; it must be diagnosed.
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
	a.Release();
	return 0;
}
