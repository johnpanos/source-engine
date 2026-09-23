//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy dialect probe (RFC 0006 M0). Unmigrated targets keep exactly
//          C++11; the lambda check is the one the tree's configure has always
//          required.
//
//=============================================================================//

#include <cstdio>
#include <functional>

#if __cplusplus != 201103L
#error "legacy-cxx11 probe: __cplusplus is not 201103L"
#endif

class Probe
{
	static int Value() { return 11; }

public:
	int Run()
	{
		std::function<int()> call = []() { return Probe::Value(); };
		return call();
	}
};

int main()
{
	Probe probe;
	std::printf( "legacy-cxx11-probe ok __cplusplus=%ld\n", static_cast<long>( __cplusplus ) );
	return probe.Run() == 11 ? 0 : 1;
}
