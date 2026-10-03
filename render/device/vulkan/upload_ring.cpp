//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan (RFC 0016 K1): the upload ring. Each range
//			records the timeline value of the submission that reads it and is
//			reused only once the timeline reaches that value. A full ring never
//			overwrites and never waits: the device gives that upload its own
//			staging buffer instead (VulkanDevice::StageUpload).
//
//=============================================================================//

#include "vulkan_device.h"

namespace render::device::vulkan
{

void UploadRing::Attach( HostBuffer buffer, std::uint64_t capacity )
{
	std::lock_guard<std::mutex> lock( m_Mutex );
	m_Buffer = buffer;
	m_Capacity = capacity;
	m_Live.clear();
	m_Head = 0;
}

HostBuffer UploadRing::Detach()
{
	std::lock_guard<std::mutex> lock( m_Mutex );
	const HostBuffer buffer = m_Buffer;
	m_Buffer = {};
	m_Capacity = 0;
	m_Live.clear();
	m_Head = 0;
	return buffer;
}

std::optional<std::pair<std::uint64_t, std::uint64_t>> UploadRing::Allocate(
    std::uint64_t size, std::uint64_t completedValue )
{
	std::lock_guard<std::mutex> lock( m_Mutex );
	RetireLocked( completedValue );
	size = ( size + 15u ) & ~std::uint64_t( 15u );
	if ( size == 0 || size >= m_Capacity || m_Buffer.mapped == nullptr )
		return std::nullopt;
	std::uint64_t offset = 0;
	if ( !m_Live.empty() )
	{
		const std::uint64_t tail = m_Live.front().offset;
		if ( m_Head >= tail )
		{
			if ( m_Capacity - m_Head >= size )
				offset = m_Head;
			else if ( tail > size )
				offset = 0;
			else
				return std::nullopt;
		}
		else if ( tail - m_Head > size )
		{
			offset = m_Head;
		}
		else
		{
			return std::nullopt;
		}
	}
	Allocation allocation;
	allocation.offset = offset;
	allocation.size = size;
	allocation.id = ++m_NextId;
	m_Live.push_back( allocation );
	m_Head = offset + size;
	return std::make_pair( offset, allocation.id );
}

UploadRing::Allocation *UploadRing::FindLocked( std::uint64_t id )
{
	if ( m_Live.empty() )
		return nullptr;
	// Successful allocations append consecutive IDs. Retirement removes only
	// a prefix, so the ID's distance from the front is its deque index, even
	// when encoders submit or abandon out of allocation order. Attach/Detach
	// clear the deque but preserve m_NextId, so old IDs cannot name new slots.
	const std::uint64_t index = id - m_Live.front().id;
	return index < m_Live.size() ? &m_Live[index] : nullptr;
}

void UploadRing::Submit( std::uint64_t id, std::uint64_t value )
{
	std::lock_guard<std::mutex> lock( m_Mutex );
	if ( Allocation *allocation = FindLocked( id ) )
	{
		allocation->value = value;
		allocation->submitted = true;
	}
}

void UploadRing::Abandon( std::uint64_t id )
{
	std::lock_guard<std::mutex> lock( m_Mutex );
	if ( Allocation *allocation = FindLocked( id ); allocation && !allocation->submitted )
		allocation->abandoned = true;
}

void UploadRing::Retire( std::uint64_t completedValue )
{
	std::lock_guard<std::mutex> lock( m_Mutex );
	RetireLocked( completedValue );
}

void UploadRing::RetireLocked( std::uint64_t completedValue )
{
	while ( !m_Live.empty() &&
	        ( m_Live.front().abandoned ||
	            ( m_Live.front().submitted && m_Live.front().value <= completedValue ) ) )
		m_Live.pop_front();
	if ( m_Live.empty() )
		m_Head = 0;
}

} // namespace render::device::vulkan
