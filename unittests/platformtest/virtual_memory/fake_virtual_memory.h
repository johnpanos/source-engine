//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic backend for platform::IVirtualMemory. Each
//			reservation is an aligned heap block with a per-page state table, so
//			the suite can read and write committed pages. Access to uncommitted
//			or read-only pages is not trapped (nothing in the shared suite
//			faults). This is a CONFORMING provider: the positive subject.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_VIRTUAL_MEMORY_H
#define PLATFORMTEST_FAKE_VIRTUAL_MEMORY_H

#include "platform/contracts/virtual_memory.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>

namespace platformtest
{

class CFakeVirtualMemory : public platform::IVirtualMemory
{
public:
	explicit CFakeVirtualMemory( std::size_t pageSize = 4096, std::size_t granularity = 65536 )
	    : m_pageSize( pageSize ), m_granularity( granularity )
	{
	}

	~CFakeVirtualMemory() override
	{
		for ( auto &entry : m_regions )
		{
			std::free( entry.second.block );
		}
	}

	std::size_t PageSize() const override { return m_pageSize; }
	std::size_t AllocationGranularity() const override { return m_granularity; }

	platform::MemoryResult Reserve( std::size_t size, platform::MemoryRegion &out ) override
	{
		if ( size == 0 )
		{
			return platform::MemoryResult::kInvalidArgument;
		}
		const std::size_t rounded = ( size + m_pageSize - 1 ) / m_pageSize * m_pageSize;
		void *block = std::aligned_alloc(
		    m_granularity, ( rounded + m_granularity - 1 ) / m_granularity * m_granularity );
		if ( block == nullptr )
		{
			return platform::MemoryResult::kOutOfMemory;
		}
		Region region;
		region.block = block;
		region.size = rounded;
		region.committed.assign( rounded / m_pageSize, false );
		m_regions[Address( block )] = region;
		out.base = block;
		out.size = rounded;
		return platform::MemoryResult::kOk;
	}

	platform::MemoryResult Commit( void *address, std::size_t size, platform::PageAccess ) override
	{
		Region *region = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, region, first ) )
		{
			return platform::MemoryResult::kInvalidArgument;
		}
		for ( std::size_t i = 0; i < size / m_pageSize; ++i )
		{
			if ( !region->committed[first + i] )
			{
				std::memset( static_cast<char *>( address ) + i * m_pageSize, 0, m_pageSize );
				region->committed[first + i] = true;
			}
		}
		return platform::MemoryResult::kOk;
	}

	platform::MemoryResult Protect( void *address, std::size_t size, platform::PageAccess ) override
	{
		Region *region = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, region, first ) )
		{
			return platform::MemoryResult::kInvalidArgument;
		}
		for ( std::size_t i = 0; i < size / m_pageSize; ++i )
		{
			if ( !region->committed[first + i] )
			{
				return platform::MemoryResult::kInvalidArgument;
			}
		}
		return platform::MemoryResult::kOk;
	}

	platform::MemoryResult Decommit( void *address, std::size_t size ) override
	{
		Region *region = nullptr;
		std::size_t first = 0;
		if ( !Locate( address, size, region, first ) )
		{
			return platform::MemoryResult::kInvalidArgument;
		}
		for ( std::size_t i = 0; i < size / m_pageSize; ++i )
		{
			region->committed[first + i] = false;
			// Scribble so a provider that forgets to zero on recommit is visible.
			std::memset( static_cast<char *>( address ) + i * m_pageSize, 0xcd, m_pageSize );
		}
		return platform::MemoryResult::kOk;
	}

	platform::MemoryResult Release( platform::MemoryRegion region ) override
	{
		auto it = m_regions.find( Address( region.base ) );
		if ( it == m_regions.end() || it->second.size != region.size )
		{
			return platform::MemoryResult::kInvalidArgument;
		}
		std::free( it->second.block );
		m_regions.erase( it );
		return platform::MemoryResult::kOk;
	}

private:
	struct Region
	{
		void *block = nullptr;
		std::size_t size = 0;
		std::vector<bool> committed;
	};

	static std::uintptr_t Address( const void *p ) { return reinterpret_cast<std::uintptr_t>( p ); }

	// Finds the reservation holding a page-aligned, non-empty range.
	bool Locate( void *address, std::size_t size, Region *&region, std::size_t &firstPage )
	{
		const std::uintptr_t a = Address( address );
		if ( address == nullptr || size == 0 || a % m_pageSize != 0 || size % m_pageSize != 0 )
		{
			return false;
		}
		auto it = m_regions.upper_bound( a );
		if ( it == m_regions.begin() )
		{
			return false;
		}
		--it;
		const std::uintptr_t base = it->first;
		if ( a + size < a || a + size > base + it->second.size )
		{
			return false;
		}
		region = &it->second;
		firstPage = ( a - base ) / m_pageSize;
		return true;
	}

	std::size_t m_pageSize;
	std::size_t m_granularity;
	std::map<std::uintptr_t, Region> m_regions;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_VIRTUAL_MEMORY_H
