//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 virtual memory (platform.virtual-memory.v1) over VirtualAlloc,
//			with bookkeeping that never touches the CRT heap, so an allocator
//			can be built on it (R103: Tier 0's small-block heap).
//
//=============================================================================//

#include "foundation_providers.h"

#include "win32_text.h"

#include <cstdint>
#include <cstring>
#include <new>

namespace platform
{
namespace
{

DWORD ProtectionOf( PageAccess access )
{
	switch ( access )
	{
	case PageAccess::kRead:
		return PAGE_READONLY;
	case PageAccess::kReadWrite:
		return PAGE_READWRITE;
	case PageAccess::kNone:
	default:
		return PAGE_NOACCESS;
	}
}

// A bump arena on reserved address space, committed as it grows. Memory is
// never given back while the owner lives; the owner releases the whole arena.
class CArena
{
public:
	~CArena()
	{
		if ( m_base != nullptr )
		{
			VirtualFree( m_base, 0, MEM_RELEASE );
		}
	}

	void *Allocate( std::size_t bytes )
	{
		bytes = ( bytes + 15 ) & ~std::size_t( 15 );
		if ( m_base == nullptr )
		{
			m_base = static_cast<std::uint8_t *>(
			    VirtualAlloc( nullptr, kReserve, MEM_RESERVE, PAGE_NOACCESS ) );
			if ( m_base == nullptr )
			{
				return nullptr;
			}
		}
		if ( m_used + bytes > kReserve )
		{
			return nullptr;
		}
		while ( m_used + bytes > m_committed )
		{
			if ( VirtualAlloc( m_base + m_committed, kStep, MEM_COMMIT, PAGE_READWRITE ) ==
			     nullptr )
			{
				return nullptr;
			}
			m_committed += kStep;
		}
		void *p = m_base + m_used;
		m_used += bytes;
		return p; // committed pages read zero
	}

private:
	static constexpr std::size_t kReserve = std::size_t( 256 ) << 20;
	static constexpr std::size_t kStep = std::size_t( 64 ) << 10;
	std::uint8_t *m_base = nullptr;
	std::size_t m_used = 0;
	std::size_t m_committed = 0;
};

struct Entry
{
	std::uintptr_t base;
	std::size_t size;
	std::uint8_t *committed; // one bit per page, created on first commit
};

class CWin32VirtualMemory final : public IWin32VirtualMemory
{
public:
	CWin32VirtualMemory()
	{
		SYSTEM_INFO info;
		GetSystemInfo( &info );
		m_page = info.dwPageSize;
		m_granularity = info.dwAllocationGranularity;
		InitializeSRWLock( &m_lock );
	}

	~CWin32VirtualMemory() override
	{
		for ( std::size_t i = 0; i < m_count; ++i )
		{
			VirtualFree( reinterpret_cast<void *>( m_entries[i].base ), 0, MEM_RELEASE );
		}
	}

	std::size_t PageSize() const override { return m_page; }
	std::size_t AllocationGranularity() const override { return m_granularity; }

	MemoryResult Reserve( std::size_t size, MemoryRegion &out ) override
	{
		return ReserveImpl( nullptr, size, out );
	}

	MemoryResult ReserveAt( void *address, std::size_t size, MemoryRegion &out ) override
	{
		if ( address == nullptr )
		{
			return MemoryResult::kInvalidArgument;
		}
		return ReserveImpl( address, size, out );
	}

	MemoryResult Commit( void *address, std::size_t size, PageAccess access ) override
	{
		CExclusive lock( m_lock );
		Entry *entry = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, entry, first ) || !EnsureBitmap( *entry ) )
		{
			return entry == nullptr ? MemoryResult::kInvalidArgument : MemoryResult::kOutOfMemory;
		}
		// MEM_COMMIT zeroes pages that were not committed and keeps the
		// contents of committed ones; it sets the protection of both.
		if ( VirtualAlloc( address, size, MEM_COMMIT, ProtectionOf( access ) ) == nullptr )
		{
			return MemoryResult::kOutOfMemory;
		}
		DWORD old = 0;
		VirtualProtect( address, size, ProtectionOf( access ), &old );
		for ( std::size_t i = 0; i < size / m_page; ++i )
		{
			SetBit( *entry, first + i, true );
		}
		return MemoryResult::kOk;
	}

	MemoryResult Protect( void *address, std::size_t size, PageAccess access ) override
	{
		CExclusive lock( m_lock );
		Entry *entry = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, entry, first ) )
		{
			return MemoryResult::kInvalidArgument;
		}
		for ( std::size_t i = 0; i < size / m_page; ++i )
		{
			if ( !Bit( *entry, first + i ) )
			{
				return MemoryResult::kInvalidArgument;
			}
		}
		DWORD old = 0;
		return VirtualProtect( address, size, ProtectionOf( access ), &old )
		           ? MemoryResult::kOk
		           : MemoryResult::kOutOfMemory;
	}

	MemoryResult Decommit( void *address, std::size_t size ) override
	{
		CExclusive lock( m_lock );
		Entry *entry = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, entry, first ) )
		{
			return MemoryResult::kInvalidArgument;
		}
		if ( !VirtualFree( address, size, MEM_DECOMMIT ) )
		{
			return MemoryResult::kOutOfMemory;
		}
		for ( std::size_t i = 0; i < size / m_page && entry->committed != nullptr; ++i )
		{
			SetBit( *entry, first + i, false );
		}
		return MemoryResult::kOk;
	}

	MemoryResult Release( MemoryRegion region ) override
	{
		CExclusive lock( m_lock );
		const std::size_t i = Find( reinterpret_cast<std::uintptr_t>( region.base ) );
		if ( region.base == nullptr || i == m_count ||
		     m_entries[i].base != reinterpret_cast<std::uintptr_t>( region.base ) ||
		     m_entries[i].size != region.size )
		{
			return MemoryResult::kInvalidArgument;
		}
		VirtualFree( region.base, 0, MEM_RELEASE );
		std::memmove( &m_entries[i], &m_entries[i + 1], ( m_count - i - 1 ) * sizeof( Entry ) );
		--m_count; // the entry's bitmap stays in the arena until teardown
		return MemoryResult::kOk;
	}

private:
	class CExclusive
	{
	public:
		explicit CExclusive( SRWLOCK &lock ) : m_lock( lock )
		{
			AcquireSRWLockExclusive( &m_lock );
		}
		~CExclusive() { ReleaseSRWLockExclusive( &m_lock ); }

	private:
		SRWLOCK &m_lock;
	};

	MemoryResult ReserveImpl( void *address, std::size_t size, MemoryRegion &out )
	{
		if ( size == 0 || size > SIZE_MAX - m_page )
		{
			return MemoryResult::kInvalidArgument;
		}
		const std::size_t rounded = ( size + m_page - 1 ) / m_page * m_page;
		std::uintptr_t at = reinterpret_cast<std::uintptr_t>( address );
		at -= at % m_granularity;
		CExclusive lock( m_lock );
		if ( !EnsureTableRoom() )
		{
			return MemoryResult::kOutOfMemory;
		}
		void *base =
		    VirtualAlloc( reinterpret_cast<void *>( at ), rounded, MEM_RESERVE, PAGE_NOACCESS );
		if ( base == nullptr )
		{
			return MemoryResult::kOutOfMemory;
		}
		const std::uintptr_t b = reinterpret_cast<std::uintptr_t>( base );
		std::size_t i = 0;
		while ( i < m_count && m_entries[i].base < b )
		{
			++i;
		}
		std::memmove( &m_entries[i + 1], &m_entries[i], ( m_count - i ) * sizeof( Entry ) );
		m_entries[i] = Entry{ b, rounded, nullptr };
		++m_count;
		out.base = base;
		out.size = rounded;
		return MemoryResult::kOk;
	}

	// Index of the last entry whose base <= address (m_count when none).
	std::size_t Find( std::uintptr_t address ) const
	{
		std::size_t lo = 0, hi = m_count;
		while ( lo < hi )
		{
			const std::size_t mid = ( lo + hi ) / 2;
			if ( m_entries[mid].base <= address )
			{
				lo = mid + 1;
			}
			else
			{
				hi = mid;
			}
		}
		return lo == 0 ? m_count : lo - 1;
	}

	bool Locate( void *address, std::size_t size, Entry *&entry, std::size_t &firstPage )
	{
		const std::uintptr_t a = reinterpret_cast<std::uintptr_t>( address );
		if ( address == nullptr || size == 0 || a % m_page != 0 || size % m_page != 0 )
		{
			return false;
		}
		const std::size_t i = Find( a );
		if ( i == m_count || a + size < a || a + size > m_entries[i].base + m_entries[i].size )
		{
			return false;
		}
		entry = &m_entries[i];
		firstPage = ( a - entry->base ) / m_page;
		return true;
	}

	bool EnsureTableRoom()
	{
		if ( m_count < m_capacity )
		{
			return true;
		}
		// Grow by moving the table to a larger block of the arena.
		const std::size_t capacity = m_capacity == 0 ? 256 : m_capacity * 2;
		Entry *grown = static_cast<Entry *>( m_arena.Allocate( capacity * sizeof( Entry ) ) );
		if ( grown == nullptr )
		{
			return false;
		}
		if ( m_count != 0 )
		{
			std::memcpy( grown, m_entries, m_count * sizeof( Entry ) );
		}
		m_entries = grown;
		m_capacity = capacity;
		return true;
	}

	bool EnsureBitmap( Entry &entry )
	{
		if ( entry.committed == nullptr )
		{
			const std::size_t pages = entry.size / m_page;
			entry.committed = static_cast<std::uint8_t *>( m_arena.Allocate( ( pages + 7 ) / 8 ) );
		}
		return entry.committed != nullptr;
	}

	static bool Bit( const Entry &entry, std::size_t page )
	{
		return entry.committed != nullptr && ( entry.committed[page / 8] >> ( page % 8 ) & 1 ) != 0;
	}

	static void SetBit( Entry &entry, std::size_t page, bool on )
	{
		const std::uint8_t mask = static_cast<std::uint8_t>( 1u << ( page % 8 ) );
		entry.committed[page / 8] =
		    on ? entry.committed[page / 8] | mask : entry.committed[page / 8] & ~mask;
	}

	std::size_t m_page = 4096;
	std::size_t m_granularity = 65536;
	SRWLOCK m_lock;
	CArena m_arena;
	Entry *m_entries = nullptr;
	std::size_t m_count = 0;
	std::size_t m_capacity = 0;
};

} // namespace

std::unique_ptr<IWin32VirtualMemory> CreateWin32VirtualMemory()
{
	return std::make_unique<CWin32VirtualMemory>();
}

IWin32VirtualMemory &Win32ProcessVirtualMemory()
{
	// Constructed in static storage on first use; its constructor allocates
	// nothing. Never destroyed: allocators may run during static teardown.
	alignas( CWin32VirtualMemory ) static unsigned char storage[sizeof( CWin32VirtualMemory )];
	static CWin32VirtualMemory *instance = new ( storage ) CWin32VirtualMemory();
	return *instance;
}

} // namespace platform
