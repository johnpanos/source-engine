//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan (RFC 0016 K1, decision 4): the adapter's one
//			memory allocator, over the pinned Vulkan Memory Allocator
//			(quality/toolchain/vulkan-memory-allocator.json). This is the only
//			translation unit that sees VMA; everything else holds opaque
//			HostAllocation handles.
//
//			Memory types: an allocation takes the first memory type, in the
//			device's order, that has every required property, as a per-resource
//			vkAllocateMemory choosing the first matching type did (VMA's cost
//			ties go to the lowest index). Preferred properties break ties
//			without making a type ineligible.
//
//=============================================================================//

#include "vulkan_device.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <map>

// VMA's internal consistency checks report instead of aborting a product.
#define VMA_ASSERT( expression ) ( (void)0 )
#define VMA_HEAVY_ASSERT( expression ) ( (void)0 )
// Entry points through vkGetInstanceProcAddr/vkGetDeviceProcAddr: a loader
// that exports only Vulkan 1.0 or 1.1 symbols (Android's) still links, and
// VMA uses the newer entry points the device offers.
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#elif defined( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#include <vk_mem_alloc.h>
#if defined( __clang__ )
#pragma clang diagnostic pop
#elif defined( __GNUC__ )
#pragma GCC diagnostic pop
#endif

namespace render::device::vulkan
{

namespace
{

VmaAllocator Vma( void *allocator )
{
	return static_cast<VmaAllocator>( allocator );
}

VmaAllocation Of( HostAllocation allocation )
{
	return reinterpret_cast<VmaAllocation>( allocation );
}

HostAllocation Handle( VmaAllocation allocation )
{
	return reinterpret_cast<HostAllocation>( allocation );
}

double Seconds()
{
	return std::chrono::duration<double>( std::chrono::steady_clock::now().time_since_epoch() )
	    .count();
}

double Mib( std::uint64_t bytes )
{
	return double( bytes ) / ( 1024.0 * 1024.0 );
}

// Unlabelled allocations (the frozen backend's, through the host device)
// group by shape: same format, extent, mips, layers, samples and role.
std::string ImageKey( const VkImageCreateInfo &info )
{
	const char *role = "texture";
	if ( info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT )
		role = "depth target";
	else if ( info.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT )
		role = "color target";
	else if ( info.usage & VK_IMAGE_USAGE_STORAGE_BIT )
		role = "storage image";
	char key[160];
	std::snprintf( key, sizeof( key ), "image %s fmt %d %ux%ux%u mips %u layers %u x%d", role,
	    int( info.format ), info.extent.width, info.extent.height, info.extent.depth,
	    info.mipLevels, info.arrayLayers, int( info.samples ) );
	return key;
}

std::string BufferKey( const VkBufferCreateInfo &info, const HostMemory &memory )
{
	char key[96];
	std::snprintf( key, sizeof( key ), "buffer usage 0x%x %s", unsigned( info.usage ),
	    ( memory.required & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) ? "host" : "device" );
	return key;
}

VmaAllocationCreateInfo CreateInfo( const HostMemory &memory )
{
	VmaAllocationCreateInfo info{};
	info.usage = VMA_MEMORY_USAGE_UNKNOWN;
	info.requiredFlags = memory.required;
	info.preferredFlags = memory.preferred;
	if ( memory.mapped )
		info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
	return info;
}

} // namespace

MemoryAllocator::~MemoryAllocator()
{
	Destroy();
}

VkResult MemoryAllocator::Create( VkInstance instance, VkPhysicalDevice physical, VkDevice device,
    std::uint32_t apiVersion, bool bufferDeviceAddress, bool memoryBudget )
{
	Destroy();
	VmaVulkanFunctions functions{};
	functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	functions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
	VmaAllocatorCreateInfo info{};
	info.pVulkanFunctions = &functions;
	info.instance = instance;
	info.physicalDevice = physical;
	info.device = device;
	info.vulkanApiVersion = apiVersion;
	if ( bufferDeviceAddress )
		info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	if ( memoryBudget )
		info.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	VmaAllocator allocator = VK_NULL_HANDLE;
	const VkResult result = vmaCreateAllocator( &info, &allocator );
	if ( result == VK_SUCCESS )
	{
		m_Allocator = allocator;
		m_MemoryBudgetEnabled = memoryBudget;
	}
	if ( const char *report = std::getenv( "SOURCE_VK_MEMORY_REPORT" ) )
		m_ReportSeconds = std::max( 1.0, std::atof( report ) );
	return result;
}

void MemoryAllocator::Destroy()
{
	if ( !m_Allocator )
		return;
	const std::uint64_t live = m_Live.load();
	if ( live != 0 )
		std::fprintf( stderr,
		    "render.device.vulkan: %llu allocations (%llu bytes) still live when the allocator is "
		    "destroyed\n",
		    static_cast<unsigned long long>( live ),
		    static_cast<unsigned long long>( m_Bytes.load() ) );
	vmaDestroyAllocator( Vma( m_Allocator ) );
	m_Allocator = nullptr;
	m_MemoryBudgetEnabled = false;
	{
		std::lock_guard<std::mutex> lock( m_TrackMutex );
		m_Tracked.clear();
	}
	m_Live = 0;
	m_Bytes = 0;
}

void MemoryAllocator::Track( HostAllocation allocation, std::string key )
{
	VmaAllocationInfo info{};
	vmaGetAllocationInfo( Vma( m_Allocator ), Of( allocation ), &info );
	std::lock_guard<std::mutex> lock( m_TrackMutex );
	m_Tracked[allocation] = Tracked{ std::move( key ), info.size };
}

void MemoryAllocator::Untrack( HostAllocation allocation )
{
	if ( m_ReportSeconds <= 0.0 )
		return;
	std::lock_guard<std::mutex> lock( m_TrackMutex );
	m_Tracked.erase( allocation );
}

void MemoryAllocator::Label( HostAllocation allocation, std::string_view label )
{
	if ( m_ReportSeconds <= 0.0 || !allocation || label.empty() )
		return;
	std::lock_guard<std::mutex> lock( m_TrackMutex );
	const auto found = m_Tracked.find( allocation );
	if ( found != m_Tracked.end() )
		found->second.key = "core: " + std::string( label );
}

void MemoryAllocator::ReportFailure(
    std::string_view label, const char *what, VkResult result, std::uint64_t bytes )
{
	char size[32] = "";
	if ( bytes )
		std::snprintf( size, sizeof( size ), ", %.1f MiB", Mib( bytes ) );
	if ( label.empty() )
		std::fprintf( stderr,
		    "render.device.vulkan: allocation failed (%d): %s%s; %.1f MiB live in %llu "
		    "allocations\n",
		    int( result ), what, size, Mib( m_Bytes.load() ),
		    static_cast<unsigned long long>( m_Live.load() ) );
	else
		std::fprintf( stderr,
		    "render.device.vulkan: allocation failed (%d): %s '%.*s'%s; %.1f MiB live in %llu "
		    "allocations\n",
		    int( result ), what, int( label.size() ), label.data(), size, Mib( m_Bytes.load() ),
		    static_cast<unsigned long long>( m_Live.load() ) );
	if ( m_ReportSeconds > 0.0 && !m_FailureReported )
	{
		m_FailureReported = true;
		Report( "allocation failure" );
	}
}

void MemoryAllocator::MaybeReport()
{
	const std::uint64_t bytes = m_Bytes.load();
	const std::uint64_t moved =
	    bytes > m_ReportedBytes ? bytes - m_ReportedBytes : m_ReportedBytes - bytes;
	const double now = Seconds();
	if ( moved < ( 32ull << 20 ) || now - m_LastReport < m_ReportSeconds )
		return;
	Report( "interval" );
}

void MemoryAllocator::Report( const char *reason )
{
	if ( !m_Allocator )
		return;
	struct Group
	{
		std::uint64_t bytes = 0;
		std::uint64_t count = 0;
	};
	std::map<std::string, Group> groups;
	{
		std::lock_guard<std::mutex> lock( m_TrackMutex );
		m_ReportedBytes = m_Bytes.load();
		m_LastReport = Seconds();
		for ( const auto &[allocation, tracked] : m_Tracked )
		{
			Group &group = groups[tracked.key];
			group.bytes += tracked.size;
			++group.count;
		}
	}
	std::vector<std::pair<std::string, Group>> sorted( groups.begin(), groups.end() );
	std::sort( sorted.begin(), sorted.end(),
	    []( const auto &a, const auto &b )
	    {
		    return a.second.bytes > b.second.bytes;
	    } );
	std::fprintf( stderr,
	    "render.device.vulkan memory report (%s): %.1f MiB live in %llu allocations, %zu groups\n",
	    reason, Mib( m_Bytes.load() ), static_cast<unsigned long long>( m_Live.load() ),
	    sorted.size() );
	const VkPhysicalDeviceMemoryProperties *properties = nullptr;
	vmaGetMemoryProperties( Vma( m_Allocator ), &properties );
	VmaBudget budgets[VK_MAX_MEMORY_HEAPS] = {};
	vmaGetHeapBudgets( Vma( m_Allocator ), budgets );
	for ( std::uint32_t heap = 0; properties && heap < properties->memoryHeapCount; ++heap )
		std::fprintf( stderr,
		    "  heap %u%s: %.1f MiB size, VMA blocks %.1f MiB, allocations %.1f MiB, process "
		    "usage %.1f MiB, budget %.1f MiB\n",
		    heap,
		    ( properties->memoryHeaps[heap].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ) ? " device"
		                                                                              : " host",
		    Mib( properties->memoryHeaps[heap].size ), Mib( budgets[heap].statistics.blockBytes ),
		    Mib( budgets[heap].statistics.allocationBytes ), Mib( budgets[heap].usage ),
		    Mib( budgets[heap].budget ) );
	const std::size_t shown = std::min<std::size_t>( sorted.size(), 80 );
	for ( std::size_t i = 0; i < shown; ++i )
		std::fprintf( stderr, "  %9.1f MiB %6llu  %s\n", Mib( sorted[i].second.bytes ),
		    static_cast<unsigned long long>( sorted[i].second.count ), sorted[i].first.c_str() );
}

MemoryBudgetSnapshot MemoryAllocator::ReadBudget( std::uint32_t epoch ) const
{
	MemoryBudgetSnapshot result;
	if ( !m_Allocator )
		return result;
	const VkPhysicalDeviceMemoryProperties *properties = nullptr;
	vmaGetMemoryProperties( Vma( m_Allocator ), &properties );
	if ( !properties )
		return result;
	VmaBudget budgets[VK_MAX_MEMORY_HEAPS] = {};
	vmaGetHeapBudgets( Vma( m_Allocator ), budgets );
	result.supported = true;
	result.epoch = epoch;
	result.heaps.reserve( properties->memoryHeapCount );
	for ( std::uint32_t heap = 0; heap < properties->memoryHeapCount; ++heap )
	{
		HeapMemoryBudget entry;
		entry.heap = heap;
		entry.deviceLocal =
		    ( properties->memoryHeaps[heap].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ) != 0;
		entry.usageKnown = true;
		entry.usageEstimated = !m_MemoryBudgetEnabled;
		entry.budgetKnown = m_MemoryBudgetEnabled;
		entry.usageBytes = budgets[heap].usage;
		if ( entry.budgetKnown )
			entry.budgetBytes = budgets[heap].budget;
		result.heaps.push_back( entry );
	}
	return result;
}

void MemoryAllocator::Count( HostAllocation allocation, bool add )
{
	VmaAllocationInfo info{};
	vmaGetAllocationInfo( Vma( m_Allocator ), Of( allocation ), &info );
	if ( add )
	{
		m_Live.fetch_add( 1 );
		m_Bytes.fetch_add( info.size );
	}
	else
	{
		m_Live.fetch_sub( 1 );
		m_Bytes.fetch_sub( info.size );
	}
}

VkResult MemoryAllocator::CreateBuffer( const VkBufferCreateInfo &info, const HostMemory &memory,
    VkBuffer *buffer, HostAllocation *allocation, void **mapped, std::string_view label )
{
	*buffer = VK_NULL_HANDLE;
	*allocation = nullptr;
	if ( !m_Allocator )
		return VK_ERROR_INITIALIZATION_FAILED;
	const VmaAllocationCreateInfo create = CreateInfo( memory );
	VmaAllocation made = VK_NULL_HANDLE;
	VmaAllocationInfo madeInfo{};
	const VkResult result =
	    vmaCreateBuffer( Vma( m_Allocator ), &info, &create, buffer, &made, &madeInfo );
	if ( result != VK_SUCCESS )
	{
		*buffer = VK_NULL_HANDLE;
		ReportFailure( label, BufferKey( info, memory ).c_str(), result, info.size );
		return result;
	}
	*allocation = Handle( made );
	if ( mapped )
		*mapped = madeInfo.pMappedData;
	Count( *allocation, true );
	if ( m_ReportSeconds > 0.0 )
	{
		Track( *allocation, BufferKey( info, memory ) );
		MaybeReport();
	}
	return VK_SUCCESS;
}

VkResult MemoryAllocator::CreateImage( const VkImageCreateInfo &info, const HostMemory &memory,
    VkImage *image, HostAllocation *allocation, std::string_view label )
{
	*image = VK_NULL_HANDLE;
	*allocation = nullptr;
	if ( !m_Allocator )
		return VK_ERROR_INITIALIZATION_FAILED;
	const VmaAllocationCreateInfo create = CreateInfo( memory );
	VmaAllocation made = VK_NULL_HANDLE;
	const VkResult result =
	    vmaCreateImage( Vma( m_Allocator ), &info, &create, image, &made, nullptr );
	if ( result != VK_SUCCESS )
	{
		*image = VK_NULL_HANDLE;
		ReportFailure( label, ImageKey( info ).c_str(), result, 0 );
		return result;
	}
	*allocation = Handle( made );
	Count( *allocation, true );
	if ( m_ReportSeconds > 0.0 )
	{
		Track( *allocation, ImageKey( info ) );
		MaybeReport();
	}
	return VK_SUCCESS;
}

void MemoryAllocator::DestroyBuffer( VkBuffer buffer, HostAllocation allocation )
{
	if ( !m_Allocator || ( buffer == VK_NULL_HANDLE && !allocation ) )
		return;
	if ( allocation )
	{
		Count( allocation, false );
		Untrack( allocation );
	}
	vmaDestroyBuffer( Vma( m_Allocator ), buffer, Of( allocation ) );
}

void MemoryAllocator::DestroyImage( VkImage image, HostAllocation allocation )
{
	if ( !m_Allocator || ( image == VK_NULL_HANDLE && !allocation ) )
		return;
	if ( allocation )
	{
		Count( allocation, false );
		Untrack( allocation );
	}
	vmaDestroyImage( Vma( m_Allocator ), image, Of( allocation ) );
}

void MemoryAllocator::Free( HostAllocation allocation )
{
	if ( !m_Allocator || !allocation )
		return;
	Count( allocation, false );
	Untrack( allocation );
	vmaFreeMemory( Vma( m_Allocator ), Of( allocation ) );
}

VkResult MemoryAllocator::Map( HostAllocation allocation, void **data )
{
	*data = nullptr;
	if ( !m_Allocator || !allocation )
		return VK_ERROR_MEMORY_MAP_FAILED;
	return vmaMapMemory( Vma( m_Allocator ), Of( allocation ), data );
}

void MemoryAllocator::Unmap( HostAllocation allocation )
{
	if ( m_Allocator && allocation )
		vmaUnmapMemory( Vma( m_Allocator ), Of( allocation ) );
}

VkResult MemoryAllocator::Invalidate(
    HostAllocation allocation, VkDeviceSize offset, VkDeviceSize size )
{
	if ( !m_Allocator || !allocation )
		return VK_ERROR_MEMORY_MAP_FAILED;
	return vmaInvalidateAllocation( Vma( m_Allocator ), Of( allocation ), offset, size );
}

VkMemoryPropertyFlags MemoryAllocator::Properties( HostAllocation allocation ) const
{
	if ( !m_Allocator || !allocation )
		return 0;
	VkMemoryPropertyFlags flags = 0;
	vmaGetAllocationMemoryProperties( Vma( m_Allocator ), Of( allocation ), &flags );
	return flags;
}

} // namespace render::device::vulkan
