//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: POSIX virtual memory (platform.virtual-memory.v1) over mmap and
//			mprotect. The provider tracks its reservations and page states so
//			it can refuse ranges the contract rejects.
//
//=============================================================================//

#include "foundation_providers.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

#include <sys/mman.h>
#include <unistd.h>

#ifndef MAP_NORESERVE
#define MAP_NORESERVE 0
#endif

namespace platform
{
namespace
{

int ProtectionOf( PageAccess access )
{
	switch ( access )
	{
	case PageAccess::kRead:
		return PROT_READ;
	case PageAccess::kReadWrite:
		return PROT_READ | PROT_WRITE;
	case PageAccess::kNone:
	default:
		return PROT_NONE;
	}
}

class CPosixVirtualMemory final : public IVirtualMemory
{
public:
	CPosixVirtualMemory() : m_page( static_cast<std::size_t>( sysconf( _SC_PAGESIZE ) ) ) {}

	~CPosixVirtualMemory() override
	{
		for ( auto &entry : m_regions )
		{
			munmap( reinterpret_cast<void *>( entry.first ), entry.second.size );
		}
	}

	std::size_t PageSize() const override { return m_page; }
	std::size_t AllocationGranularity() const override { return m_page; }

	MemoryResult Reserve( std::size_t size, MemoryRegion &out ) override
	{
		if ( size == 0 || size > SIZE_MAX - m_page )
		{
			return MemoryResult::kInvalidArgument;
		}
		const std::size_t rounded = ( size + m_page - 1 ) / m_page * m_page;
		void *base =
		    mmap( nullptr, rounded, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0 );
		if ( base == MAP_FAILED )
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
		// Reserved pages were mapped (or remapped by Decommit) as fresh anonymous
		// memory, so they read zero once accessible.
		if ( mprotect( address, size, ProtectionOf( access ) ) != 0 )
		{
			return MemoryResult::kOutOfMemory;
		}
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
		return mprotect( address, size, ProtectionOf( access ) ) == 0 ? MemoryResult::kOk
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
		// A fixed mapping over the range drops the old pages on every kernel.
		void *fresh = mmap( address, size, PROT_NONE,
		    MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_FIXED, -1, 0 );
		if ( fresh == MAP_FAILED )
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
		munmap( region.base, region.size );
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

	const std::size_t m_page;
	std::mutex m_mutex;
	std::map<std::uintptr_t, Region> m_regions;
};

} // namespace

std::unique_ptr<IVirtualMemory> CreatePosixVirtualMemory()
{
	return std::make_unique<CPosixVirtualMemory>();
}

} // namespace platform
