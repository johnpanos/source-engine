//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Records the layout of every facade boundary type in the dialect this
//          unit is compiled with. TOOLCHAIN_LAYOUT_FN names the entry point.
//          TOOLCHAIN_SEED_DIALECT_LAYOUT adds a deliberately dialect-dependent
//          type so the sensitivity suite proves the comparison has teeth.
//
//=============================================================================//

#include "boundary_layout.h"

#include "jobsystem/parallel_batch.h"
#include "jobsystem/worker_backend.h"
#include "mapcontainer/map_container.h"

#include <cstddef>
#include <functional>
#include <string>

#ifndef TOOLCHAIN_LAYOUT_FN
#error "define TOOLCHAIN_LAYOUT_FN to the unit's layout entry point"
#endif

#if defined( TOOLCHAIN_SEED_DIALECT_LAYOUT )
struct SeededDialectDependent
{
	int legacy;
#if __cplusplus >= 202002L
	int addedOnlyInCxx20;
#endif
};
#endif

#define LAYOUT_RECORD( T, N, ... ) { #T, sizeof( T ), alignof( T ), N, { __VA_ARGS__ }, __cplusplus }
#define OFFSET( T, F ) offsetof( T, F )

namespace
{
using jobsystem::BatchDesc;
using mapcontainer::MapContainerOpenOptions;
using mapcontainer::MapContainerStatus;
using mapcontainer::MapLumpInfo;
typedef std::function<void( int )> WorkerBody;

const ToolchainLayoutRecord g_records[] = {
	LAYOUT_RECORD( BatchDesc, 7, OFFSET( BatchDesc, name ), OFFSET( BatchDesc, context ),
	    OFFSET( BatchDesc, count ), OFFSET( BatchDesc, process ), OFFSET( BatchDesc, begin ),
	    OFFSET( BatchDesc, end ), OFFSET( BatchDesc, maxParticipants ) ),
	LAYOUT_RECORD( MapContainerStatus, 3, OFFSET( MapContainerStatus, code ),
	    OFFSET( MapContainerStatus, fourcc ), OFFSET( MapContainerStatus, offset ) ),
	LAYOUT_RECORD( MapLumpInfo, 11, OFFSET( MapLumpInfo, fourcc ), OFFSET( MapLumpInfo, version ),
	    OFFSET( MapLumpInfo, flags ), OFFSET( MapLumpInfo, alignment ), OFFSET( MapLumpInfo, offset ),
	    OFFSET( MapLumpInfo, storedSize ), OFFSET( MapLumpInfo, uncompressedSize ),
	    OFFSET( MapLumpInfo, legacyOrigin ), OFFSET( MapLumpInfo, legacyUncompressedSize ),
	    OFFSET( MapLumpInfo, hasHash ), OFFSET( MapLumpInfo, hash ) ),
	LAYOUT_RECORD( MapContainerOpenOptions, 4, OFFSET( MapContainerOpenOptions, verifyContent ),
	    OFFSET( MapContainerOpenOptions, pKnownFourCCs ),
	    OFFSET( MapContainerOpenOptions, nKnownFourCCs ),
	    OFFSET( MapContainerOpenOptions, allowUnknownRequired ) ),
	// Standard types reachable through a facade (IWorkerBackend::ParallelFor)
	// and the libstdc++ dual-ABI witness: no member offsets, size/alignment only.
	LAYOUT_RECORD( WorkerBody, 0, 0 ),
	LAYOUT_RECORD( std::string, 0, 0 ),
#if defined( TOOLCHAIN_SEED_DIALECT_LAYOUT )
	LAYOUT_RECORD( SeededDialectDependent, 1, OFFSET( SeededDialectDependent, legacy ) ),
#endif
};
} // namespace

const ToolchainLayoutRecord *TOOLCHAIN_LAYOUT_FN( int *pCount )
{
	*pCount = static_cast<int>( sizeof( g_records ) / sizeof( g_records[0] ) );
	return g_records;
}
