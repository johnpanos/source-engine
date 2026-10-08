//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the RFC 0001 virtual-memory capability
//			(platform::IVirtualMemory). Every provider that claims the contract
//			runs THIS predicate. It only reads committed pages and only writes
//			read-write pages, so it never faults on a native provider.
//
//=============================================================================//

#ifndef PLATFORMTEST_VIRTUAL_MEMORY_CONFORMANCE_H
#define PLATFORMTEST_VIRTUAL_MEMORY_CONFORMANCE_H

#include "platform/contracts/virtual_memory.h"

#include <cstdint>
#include <cstdio>

namespace platformtest
{

struct VirtualMemoryReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define VM_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

inline bool IsPowerOfTwo( std::size_t v )
{
	return v != 0 && ( v & ( v - 1 ) ) == 0;
}

// True when every byte of a page equals `value`.
inline bool PageIs( const unsigned char *page, std::size_t pageSize, unsigned char value )
{
	for ( std::size_t i = 0; i < pageSize; ++i )
	{
		if ( page[i] != value )
		{
			return false;
		}
	}
	return true;
}

inline void FillPage( unsigned char *page, std::size_t pageSize, unsigned char value )
{
	for ( std::size_t i = 0; i < pageSize; ++i )
	{
		page[i] = value;
	}
}

inline VirtualMemoryReport RunVirtualMemoryConformance( platform::IVirtualMemory &vm )
{
	using platform::MemoryRegion;
	using platform::MemoryResult;
	using platform::PageAccess;

	VirtualMemoryReport r;
	const std::size_t page = vm.PageSize();
	const std::size_t granularity = vm.AllocationGranularity();
	VM_CHECK( r, IsPowerOfTwo( page ) );
	VM_CHECK( r, IsPowerOfTwo( granularity ) && granularity % page == 0 );
	if ( !IsPowerOfTwo( page ) || page < 64 )
	{
		return r; // nothing below is meaningful with a bogus page size
	}

	// Zero-sized reservations are refused and leave `out` unchanged.
	{
		MemoryRegion out;
		out.size = 77;
		VM_CHECK( r, vm.Reserve( 0, out ) == MemoryResult::kInvalidArgument );
		VM_CHECK( r, out.size == 77 && out.base == nullptr );
	}

	// Three pages and a byte round up to four pages on a granularity boundary.
	MemoryRegion region;
	VM_CHECK( r, vm.Reserve( 3 * page + 1, region ) == MemoryResult::kOk );
	if ( region.base == nullptr )
	{
		r.Record( false, "reservation failed", __LINE__ );
		return r;
	}
	VM_CHECK( r, region.size == 4 * page );
	VM_CHECK( r, reinterpret_cast<std::uintptr_t>( region.base ) % granularity == 0 );
	unsigned char *base = static_cast<unsigned char *>( region.base );

	// Malformed commit ranges.
	VM_CHECK(
	    r, vm.Commit( base + 1, page, PageAccess::kReadWrite ) == MemoryResult::kInvalidArgument );
	VM_CHECK(
	    r, vm.Commit( base, page + 1, PageAccess::kReadWrite ) == MemoryResult::kInvalidArgument );
	VM_CHECK( r, vm.Commit( base, 0, PageAccess::kReadWrite ) == MemoryResult::kInvalidArgument );
	VM_CHECK( r, vm.Commit( base + 3 * page, 2 * page, PageAccess::kReadWrite ) ==
	                 MemoryResult::kInvalidArgument );
	VM_CHECK(
	    r, vm.Commit( nullptr, page, PageAccess::kReadWrite ) == MemoryResult::kInvalidArgument );

	// Protecting reserved but uncommitted pages is refused.
	VM_CHECK( r, vm.Protect( base, page, PageAccess::kRead ) == MemoryResult::kInvalidArgument );

	// Fresh commits read zero and hold what is written.
	VM_CHECK( r, vm.Commit( base, 2 * page, PageAccess::kReadWrite ) == MemoryResult::kOk );
	VM_CHECK( r, PageIs( base, page, 0 ) && PageIs( base + page, page, 0 ) );
	FillPage( base, page, 0x5a );
	FillPage( base + page, page, 0xa5 );
	VM_CHECK( r, PageIs( base, page, 0x5a ) && PageIs( base + page, page, 0xa5 ) );

	// Protect keeps contents, both ways.
	VM_CHECK( r, vm.Protect( base, 2 * page, PageAccess::kRead ) == MemoryResult::kOk );
	VM_CHECK( r, PageIs( base, page, 0x5a ) && PageIs( base + page, page, 0xa5 ) );
	VM_CHECK( r, vm.Protect( base, 2 * page, PageAccess::kReadWrite ) == MemoryResult::kOk );
	FillPage( base + page, page, 0x3c );
	VM_CHECK( r, PageIs( base + page, page, 0x3c ) );

	// A range that is only partly committed cannot be protected.
	VM_CHECK( r,
	    vm.Protect( base + page, 2 * page, PageAccess::kRead ) == MemoryResult::kInvalidArgument );
	VM_CHECK( r, PageIs( base + page, page, 0x3c ) );

	// Recommitting committed pages keeps their contents.
	VM_CHECK( r, vm.Commit( base, page, PageAccess::kReadWrite ) == MemoryResult::kOk );
	VM_CHECK( r, PageIs( base, page, 0x5a ) );

	// Decommit discards: a later commit reads zero; neighbours are untouched.
	VM_CHECK( r, vm.Decommit( base, page ) == MemoryResult::kOk );
	VM_CHECK( r, vm.Commit( base, page, PageAccess::kReadWrite ) == MemoryResult::kOk );
	VM_CHECK( r, PageIs( base, page, 0 ) );
	VM_CHECK( r, PageIs( base + page, page, 0x3c ) );

	// Decommitting pages that were never committed is allowed; malformed is not.
	VM_CHECK( r, vm.Decommit( base + 3 * page, page ) == MemoryResult::kOk );
	VM_CHECK( r, vm.Decommit( base + 1, page ) == MemoryResult::kInvalidArgument );

	// A second reservation does not overlap the first.
	MemoryRegion other;
	VM_CHECK( r, vm.Reserve( page, other ) == MemoryResult::kOk );
	{
		const std::uintptr_t a0 = reinterpret_cast<std::uintptr_t>( region.base );
		const std::uintptr_t b0 = reinterpret_cast<std::uintptr_t>( other.base );
		VM_CHECK( r, other.base != nullptr && ( b0 + other.size <= a0 || a0 + region.size <= b0 ) );
	}

	// Release takes exactly what Reserve returned, once.
	{
		MemoryRegion wrongSize = region;
		wrongSize.size = page;
		VM_CHECK( r, vm.Release( wrongSize ) == MemoryResult::kInvalidArgument );
		VM_CHECK( r, vm.Release( MemoryRegion() ) == MemoryResult::kInvalidArgument );
	}
	VM_CHECK( r, vm.Release( region ) == MemoryResult::kOk );
	VM_CHECK( r, vm.Release( region ) == MemoryResult::kInvalidArgument );
	VM_CHECK(
	    r, vm.Commit( base, page, PageAccess::kReadWrite ) == MemoryResult::kInvalidArgument );
	VM_CHECK( r, vm.Release( other ) == MemoryResult::kOk );

	return r;
}

} // namespace platformtest

#endif // PLATFORMTEST_VIRTUAL_MEMORY_CONFORMANCE_H
