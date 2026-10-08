//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Win32 virtual memory (platform.virtual-memory.v1) over VirtualAlloc.
//
//=============================================================================//

#include "foundation_providers.h"

#include "win32_text.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

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

class CWin32VirtualMemory final : public IVirtualMemory
{
public:
	CWin32VirtualMemory()
	{
		SYSTEM_INFO info;
		GetSystemInfo( &info );
		m_page = info.dwPageSize;
		m_granularity = info.dwAllocationGranularity;
	}

	~CWin32VirtualMemory() override
	{
		for ( auto &entry : m_regions )
		{
			VirtualFree( reinterpret_cast<void *>( entry.first ), 0, MEM_RELEASE );
		}
	}

	std::size_t PageSize() const override { return m_page; }
	std::size_t AllocationGranularity() const override { return m_granularity; }

	MemoryResult Reserve( std::size_t size, MemoryRegion &out ) override
	{
		if ( size == 0 || size > SIZE_MAX - m_page )
		{
			return MemoryResult::kInvalidArgument;
		}
		const std::size_t rounded = ( size + m_page - 1 ) / m_page * m_page;
		void *base = VirtualAlloc( nullptr, rounded, MEM_RESERVE, PAGE_NOACCESS );
		if ( base == nullptr )
		{
			return MemoryResult::kOutOfMemory;
		}
		Region region;
		region.size = rounded;
		region.committed.assign( rounded / m_page, false );
		std::lock_guard<std::mutex> lock( m_mutex );
		m_regions[Address( base )] = std::move( region );
		out.base = base;
		out.size = rounded;
		return MemoryResult::kOk;
	}

	MemoryResult Commit( void *address, std::size_t size, PageAccess access ) override
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		Region *region = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, region, first ) )
		{
			return MemoryResult::kInvalidArgument;
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
			region->committed[first + i] = true;
		}
		return MemoryResult::kOk;
	}

	MemoryResult Protect( void *address, std::size_t size, PageAccess access ) override
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		Region *region = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, region, first ) )
		{
			return MemoryResult::kInvalidArgument;
		}
		for ( std::size_t i = 0; i < size / m_page; ++i )
		{
			if ( !region->committed[first + i] )
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
		std::lock_guard<std::mutex> lock( m_mutex );
		Region *region = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, region, first ) )
		{
			return MemoryResult::kInvalidArgument;
		}
		if ( !VirtualFree( address, size, MEM_DECOMMIT ) )
		{
			return MemoryResult::kOutOfMemory;
		}
		for ( std::size_t i = 0; i < size / m_page; ++i )
		{
			region->committed[first + i] = false;
		}
		return MemoryResult::kOk;
	}

	MemoryResult Release( MemoryRegion region ) override
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		auto it = m_regions.find( Address( region.base ) );
		if ( region.base == nullptr || it == m_regions.end() || it->second.size != region.size )
		{
			return MemoryResult::kInvalidArgument;
		}
		VirtualFree( region.base, 0, MEM_RELEASE );
		m_regions.erase( it );
		return MemoryResult::kOk;
	}

private:
	struct Region
	{
		std::size_t size = 0;
		std::vector<bool> committed;
	};

	static std::uintptr_t Address( const void *p ) { return reinterpret_cast<std::uintptr_t>( p ); }

	bool Locate( void *address, std::size_t size, Region *&region, std::size_t &firstPage )
	{
		const std::uintptr_t a = Address( address );
		if ( address == nullptr || size == 0 || a % m_page != 0 || size % m_page != 0 )
		{
			return false;
		}
		auto it = m_regions.upper_bound( a );
		if ( it == m_regions.begin() )
		{
			return false;
		}
		--it;
		if ( a + size < a || a + size > it->first + it->second.size )
		{
			return false;
		}
		region = &it->second;
		firstPage = ( a - it->first ) / m_page;
		return true;
	}

	std::size_t m_page = 4096;
	std::size_t m_granularity = 65536;
	std::mutex m_mutex;
	std::map<std::uintptr_t, Region> m_regions;
};

} // namespace

std::unique_ptr<IVirtualMemory> CreateWin32VirtualMemory()
{
	return std::make_unique<CWin32VirtualMemory>();
}

} // namespace platform
