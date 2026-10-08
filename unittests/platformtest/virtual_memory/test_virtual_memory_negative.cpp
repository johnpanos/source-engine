//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the RFC 0001 virtual-memory conformance suite
//			(PLAT-VMEM-001, Q-FOUNDATION). Feeds the SAME shared predicate broken
//			providers, each violating one clause, and asserts every one is caught
//			while the conforming backend passes.
//
//			Build/run: tools/quality/conformance.py check --suite platform.virtual_memory.sensitivity
//
//=============================================================================//

#include "fake_virtual_memory.h"
#include "virtual_memory_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <cstring>

namespace
{

using platform::MemoryRegion;
using platform::MemoryResult;
using platform::PageAccess;
using platformtest::CFakeVirtualMemory;

// DEFECT: a page size that is not a power of two.
class COddPageSize : public CFakeVirtualMemory
{
public:
	std::size_t PageSize() const override { return 4000; }
};

// DEFECT: recommitted pages keep stale contents.
class CStaleRecommit : public CFakeVirtualMemory
{
public:
	MemoryResult Decommit( void *, std::size_t ) override { return MemoryResult::kOk; }
};

// DEFECT: fresh commits are not zeroed.
class CDirtyCommit : public CFakeVirtualMemory
{
public:
	MemoryResult Commit( void *a, std::size_t s, PageAccess access ) override
	{
		const MemoryResult result = CFakeVirtualMemory::Commit( a, s, access );
		if ( result == MemoryResult::kOk && !m_dirtied )
		{
			m_dirtied = true;
			std::memset( a, 0x11, s );
		}
		return result;
	}

private:
	bool m_dirtied = false;
};

// DEFECT: the base is aligned only to the page, not the granularity.
class CPageAlignedBase : public CFakeVirtualMemory
{
public:
	CPageAlignedBase() : CFakeVirtualMemory( 4096, 65536 ) {}
	std::size_t AllocationGranularity() const override { return 1u << 30; }
};

// DEFECT: the reported size is not rounded up to whole pages.
class CUnroundedSize : public CFakeVirtualMemory
{
public:
	MemoryResult Reserve( std::size_t size, MemoryRegion &out ) override
	{
		const MemoryResult result = CFakeVirtualMemory::Reserve( size, out );
		if ( result == MemoryResult::kOk && size % PageSize() != 0 )
		{
			m_real = out;
			out.size = size;
		}
		return result;
	}
	MemoryResult Release( MemoryRegion region ) override
	{
		if ( region.base == m_real.base && region.base != nullptr )
		{
			region.size = m_real.size;
		}
		return CFakeVirtualMemory::Release( region );
	}

private:
	MemoryRegion m_real;
};

// DEFECT: unaligned commit ranges are accepted (rounded silently).
class CRoundsUnaligned : public CFakeVirtualMemory
{
public:
	MemoryResult Commit( void *a, std::size_t s, PageAccess access ) override
	{
		const std::uintptr_t p = reinterpret_cast<std::uintptr_t>( a );
		const std::size_t page = PageSize();
		if ( a != nullptr && s != 0 && ( p % page != 0 || s % page != 0 ) )
		{
			const std::uintptr_t start = p / page * page;
			const std::size_t size = ( p + s - start + page - 1 ) / page * page;
			CFakeVirtualMemory::Commit( reinterpret_cast<void *>( start ), size, access );
			return MemoryResult::kOk;
		}
		return CFakeVirtualMemory::Commit( a, s, access );
	}
};

// DEFECT: Protect accepts uncommitted pages.
class CProtectsUncommitted : public CFakeVirtualMemory
{
public:
	MemoryResult Protect( void *, std::size_t, PageAccess ) override { return MemoryResult::kOk; }
};

// DEFECT: a second Release reports success.
class CDoubleRelease : public CFakeVirtualMemory
{
public:
	MemoryResult Release( MemoryRegion region ) override
	{
		CFakeVirtualMemory::Release( region );
		return region.base != nullptr ? MemoryResult::kOk : MemoryResult::kInvalidArgument;
	}
};

// DEFECT: Release ignores the size.
class CIgnoresReleaseSize : public CFakeVirtualMemory
{
public:
	MemoryResult Reserve( std::size_t size, MemoryRegion &out ) override
	{
		const MemoryResult result = CFakeVirtualMemory::Reserve( size, out );
		if ( result == MemoryResult::kOk )
		{
			m_sizes[out.base] = out.size;
		}
		return result;
	}
	MemoryResult Release( MemoryRegion region ) override
	{
		auto it = m_sizes.find( region.base );
		if ( it != m_sizes.end() )
		{
			region.size = it->second;
		}
		return CFakeVirtualMemory::Release( region );
	}

private:
	std::map<void *, std::size_t> m_sizes;
};

template <typename T> bool Caught()
{
	T vm;
	return platformtest::RunVirtualMemoryConformance( vm ).failures > 0;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	{
		CFakeVirtualMemory good;
		const platformtest::VirtualMemoryReport r =
		    platformtest::RunVirtualMemoryConformance( good );
		++checks;
		if ( r.failures != 0 )
		{
			std::printf( "FAIL: conforming provider rejected (%d/%d); first: %s (line %d)\n",
			    r.failures, r.checks, r.firstFailure, r.firstFailureLine );
			++failures;
		}
	}
	struct Case
	{
		bool ( *caught )();
		const char *name;
	};
	const Case cases[] = {
	    { Caught<COddPageSize>, "non-power-of-two-page" },
	    { Caught<CStaleRecommit>, "stale-after-recommit" },
	    { Caught<CDirtyCommit>, "unzeroed-commit" },
	    { Caught<CPageAlignedBase>, "base-not-granularity-aligned" },
	    { Caught<CUnroundedSize>, "size-not-page-rounded" },
	    { Caught<CRoundsUnaligned>, "accepts-unaligned-commit" },
	    { Caught<CProtectsUncommitted>, "protects-uncommitted" },
	    { Caught<CDoubleRelease>, "double-release" },
	    { Caught<CIgnoresReleaseSize>, "release-ignores-size" },
	};
	for ( const Case &c : cases )
	{
		++checks;
		if ( !c.caught() )
		{
			std::printf( "FAIL: broken provider '%s' was NOT caught\n", c.name );
			++failures;
		}
	}
	if ( failures == 0 )
	{
		std::printf( "ok test_virtual_memory_negative: all %zu broken providers caught\n",
		    sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
