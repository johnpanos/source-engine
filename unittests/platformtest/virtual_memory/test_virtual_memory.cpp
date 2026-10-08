//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for the RFC 0001 virtual-memory
//			capability (PLAT-VMEM-001, Q-FOUNDATION). Runs the shared suite
//			against the deterministic backend with POSIX- and Windows-shaped
//			page and granularity sizes.
//
//			Certifies contract semantics via a fake; NOT evidence of native
//			mapping behavior.
//
//			Build/run: tools/quality/conformance.py check --suite platform.virtual_memory
//
//=============================================================================//

#include "fake_virtual_memory.h"
#include "virtual_memory_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

void RunVariant(
    const char *name, std::size_t page, std::size_t granularity, int &checks, int &failures )
{
	platformtest::CFakeVirtualMemory vm( page, granularity );
	const platformtest::VirtualMemoryReport r = platformtest::RunVirtualMemoryConformance( vm );
	checks += r.checks;
	failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", name, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return;
	}
	std::printf( "ok %s: %d checks passed\n", name, r.checks );
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	RunVariant( "test_virtual_memory[4k/4k]", 4096, 4096, checks, failures );
	RunVariant( "test_virtual_memory[4k/64k]", 4096, 65536, checks, failures );
	RunVariant( "test_virtual_memory[16k/16k]", 16384, 16384, checks, failures );
	return testing::ReportConformance( checks, failures );
}
