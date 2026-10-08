//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for virtual memory (RFC 0001 foundation
//			capability "Virtual memory": page size, reserve, commit, protect,
//			decommit and release).
//
//			Consumers are allocators that reserve address space up front and
//			commit it as they grow (the memory pools, large streaming buffers,
//			guard pages in tests). A native provider wraps mmap/mprotect or
//			VirtualAlloc; the deterministic test provider backs regions with heap
//			memory and tracks page state.
//
//			There is deliberately no executable access: store-targeted builds do
//			not generate code at runtime (AGENTS.md platform rules), and nothing
//			in the engine needs it.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_VIRTUAL_MEMORY_H
#define PLATFORM_CONTRACTS_VIRTUAL_MEMORY_H

// Contract header: standard library only. No tier0/tier1, no native SDK, no
// OS-selection macros. Must compile under linux-headless-core.
#include <cstddef>

namespace platform
{

enum class PageAccess
{
	kNone = 0,
	kRead,
	kReadWrite,
};

enum class MemoryResult
{
	kOk = 0,
	kInvalidArgument, // unaligned, zero-sized, outside a reservation, or not reserved
	kOutOfMemory,     // the platform refused the address space or the backing store
};

// A reservation. `base` is aligned to AllocationGranularity(); `size` is the
// requested size rounded up to a whole number of pages.
struct MemoryRegion
{
	void *base = nullptr;
	std::size_t size = 0;
};

class IVirtualMemory
{
public:
	virtual ~IVirtualMemory() = default;

	// The page size in bytes: a power of two. Commit, Protect and Decommit work
	// on whole pages.
	virtual std::size_t PageSize() const = 0;

	// The alignment of every reservation's base: a power-of-two multiple of
	// PageSize() (64 KiB on Windows, the page size on POSIX).
	virtual std::size_t AllocationGranularity() const = 0;

	// Reserves address space with no access and no backing store. On failure
	// `out` is unchanged. A zero size is kInvalidArgument. Reservations never
	// overlap one another.
	virtual MemoryResult Reserve( std::size_t size, MemoryRegion &out ) = 0;

	// Backs [address, address + size) with memory and gives it `access`. The
	// range must be page-aligned, non-empty and inside one reservation. Pages
	// that were not committed read as zero after Commit; committing an already
	// committed page keeps its contents and changes its access.
	virtual MemoryResult Commit( void *address, std::size_t size, PageAccess access ) = 0;

	// Changes the access of committed pages; their contents are kept. The whole
	// range must be committed, page-aligned and inside one reservation.
	virtual MemoryResult Protect( void *address, std::size_t size, PageAccess access ) = 0;

	// Returns pages to the reserved state: no access, contents discarded, so a
	// later Commit reads zero. Decommitting reserved pages is allowed.
	virtual MemoryResult Decommit( void *address, std::size_t size ) = 0;

	// Releases a whole reservation, committed or not. `region` must be exactly
	// what Reserve returned; anything else, including a second Release, is
	// kInvalidArgument.
	virtual MemoryResult Release( MemoryRegion region ) = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_VIRTUAL_MEMORY_H
