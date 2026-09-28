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

#include <cstdio>

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
    std::uint32_t apiVersion, bool bufferDeviceAddress )
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
	VmaAllocator allocator = VK_NULL_HANDLE;
	const VkResult result = vmaCreateAllocator( &info, &allocator );
	if ( result == VK_SUCCESS )
		m_Allocator = allocator;
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
	m_Live = 0;
	m_Bytes = 0;
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
    VkBuffer *buffer, HostAllocation *allocation, void **mapped )
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
		return result;
	}
	*allocation = Handle( made );
	if ( mapped )
		*mapped = madeInfo.pMappedData;
	Count( *allocation, true );
	return VK_SUCCESS;
}

VkResult MemoryAllocator::CreateImage( const VkImageCreateInfo &info, const HostMemory &memory,
    VkImage *image, HostAllocation *allocation )
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
		return result;
	}
	*allocation = Handle( made );
	Count( *allocation, true );
	return VK_SUCCESS;
}

void MemoryAllocator::DestroyBuffer( VkBuffer buffer, HostAllocation allocation )
{
	if ( !m_Allocator || ( buffer == VK_NULL_HANDLE && !allocation ) )
		return;
	if ( allocation )
		Count( allocation, false );
	vmaDestroyBuffer( Vma( m_Allocator ), buffer, Of( allocation ) );
}

void MemoryAllocator::DestroyImage( VkImage image, HostAllocation allocation )
{
	if ( !m_Allocator || ( image == VK_NULL_HANDLE && !allocation ) )
		return;
	if ( allocation )
		Count( allocation, false );
	vmaDestroyImage( Vma( m_Allocator ), image, Of( allocation ) );
}

void MemoryAllocator::Free( HostAllocation allocation )
{
	if ( !m_Allocator || !allocation )
		return;
	Count( allocation, false );
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
