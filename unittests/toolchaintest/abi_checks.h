//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared checks for the frozen-consumer ABI fixtures (toolchain.abi.v1).
//          C++11 so the external-consumer units can include it. Results use
//          the conformance runner's checks-v1 protocol.
//
//=============================================================================//

#ifndef TOOLCHAINTEST_ABI_CHECKS_H
#define TOOLCHAINTEST_ABI_CHECKS_H

#include "boundary_layout.h"

#include <cstdio>
#include <cstring>

namespace toolchaintest
{

extern int g_checks;
extern int g_failures;

inline void Check( bool condition, const char *expression, const char *file, int line )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL %s:%d: %s\n", file, line, expression );
	}
}

#define ABI_CHECK( condition ) ::toolchaintest::Check( ( condition ), #condition, __FILE__, __LINE__ )

// Frozen LP64 layouts of the first-party facade value types. A mismatch is an
// unversioned ABI change of a published boundary, whichever dialect built it.
struct FrozenLayout
{
	const char *name;
	unsigned long size;
	unsigned long align;
	int fieldCount;
	unsigned long offsets[kToolchainMaxFields];
};

static const FrozenLayout kFrozenLp64[] = {
	{ "BatchDesc", 56, 8, 7, { 0, 8, 16, 24, 32, 40, 48 } },
	{ "MapContainerStatus", 16, 8, 3, { 0, 4, 8 } },
	{ "MapLumpInfo", 72, 8, 11, { 0, 4, 8, 12, 16, 24, 32, 40, 48, 52, 53 } },
	{ "MapContainerOpenOptions", 32, 8, 4, { 0, 8, 16, 24 } },
};

inline bool SameLayout( const ToolchainLayoutRecord &a, const ToolchainLayoutRecord &b )
{
	if ( a.size != b.size || a.align != b.align || a.fieldCount != b.fieldCount )
		return false;
	for ( int i = 0; i < a.fieldCount; ++i )
	{
		if ( a.offsets[i] != b.offsets[i] )
			return false;
	}
	return true;
}

// Compares the external-consumer (C++11) and engine (C++20) records of every
// boundary type with each other and with the frozen table.
inline void CheckBoundaryLayouts()
{
	int legacyCount = 0;
	int modernCount = 0;
	const ToolchainLayoutRecord *legacy = ToolchainLayoutsLegacy( &legacyCount );
	const ToolchainLayoutRecord *modern = ToolchainLayoutsCxx20( &modernCount );
	ABI_CHECK( legacyCount > 0 && legacyCount == modernCount );
	ABI_CHECK( legacyCount > 0 && legacy[0].cplusplus == 201103L );
	ABI_CHECK( modernCount > 0 && modern[0].cplusplus >= 202002L );
	for ( int i = 0; i < legacyCount && i < modernCount; ++i )
	{
		ABI_CHECK( std::strcmp( legacy[i].name, modern[i].name ) == 0 );
		const bool same = SameLayout( legacy[i], modern[i] );
		if ( !same )
		{
			std::printf( "FAIL layout of %s differs: C++11 size %lu align %lu, C++20 size %lu "
			             "align %lu\n",
			    legacy[i].name, legacy[i].size, legacy[i].align, modern[i].size, modern[i].align );
		}
		ABI_CHECK( same );
	}
	const int frozenCount = static_cast<int>( sizeof( kFrozenLp64 ) / sizeof( kFrozenLp64[0] ) );
#if defined( __LP64__ )
	for ( int f = 0; f < frozenCount; ++f )
	{
		bool found = false;
		for ( int i = 0; i < modernCount; ++i )
		{
			if ( std::strcmp( modern[i].name, kFrozenLp64[f].name ) != 0 )
				continue;
			found = true;
			ToolchainLayoutRecord frozen = {};
			frozen.name = kFrozenLp64[f].name;
			frozen.size = kFrozenLp64[f].size;
			frozen.align = kFrozenLp64[f].align;
			frozen.fieldCount = kFrozenLp64[f].fieldCount;
			std::memcpy( frozen.offsets, kFrozenLp64[f].offsets, sizeof( frozen.offsets ) );
			const bool same = SameLayout( modern[i], frozen );
			if ( !same )
				std::printf( "FAIL %s no longer matches its frozen LP64 layout\n", frozen.name );
			ABI_CHECK( same );
		}
		ABI_CHECK( found );
	}
#else
	// No frozen table for this data model: an explicit failure, not a skip.
	ABI_CHECK( frozenCount == 0 && "no frozen layout table for this data model" );
#endif
}

inline int Finish()
{
	std::printf( "CONFORMANCE %d %d\n", g_checks, g_failures );
	return g_failures == 0 && g_checks > 0 ? 0 : 1;
}

} // namespace toolchaintest

#endif // TOOLCHAINTEST_ABI_CHECKS_H
