//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.d3d12: uploads staged into the adapter's upload-heap
//			ring for the shared recording encoder (recording::IUploadStager).
//
//=============================================================================//

#include "d3d12_device.h"

#include <cstring>

namespace render::device::d3d12
{

void D3d12Device::StageUpload(
    RecordingEncoder &encoder, Command &command, std::span<const std::byte> bytes )
{
	std::lock_guard<std::mutex> lock( m_RingLock );
	m_RingState.Retire( m_Epoch, m_Completed.load( std::memory_order_acquire ) );
	const std::uint64_t id = ++m_NextAllocation;
	if ( m_RingData )
	{
		if ( const std::optional<std::uint64_t> offset = m_RingState.Allocate( bytes.size(), id ) )
		{
			// The upload heap is CPU-coherent: the submission's copy, recorded
			// after this write, reads it.
			std::memcpy( m_RingData + *offset, bytes.data(), bytes.size() );
			command.ringOffset = *offset;
			command.fromRing = true;
			encoder.AddRingAllocation( id );
			return;
		}
	}
	++m_DeferredUploads;
	command.bytes.assign( bytes.begin(), bytes.end() );
}

void D3d12Device::AbandonUploads( const std::vector<std::uint64_t> &allocations )
{
	std::lock_guard<std::mutex> lock( m_RingLock );
	for ( std::uint64_t id : allocations )
		m_RingState.Abandon( id );
}

std::uint64_t D3d12Device::DeferredUploads() const
{
	std::lock_guard<std::mutex> lock( m_RingLock );
	return m_DeferredUploads;
}

} // namespace render::device::d3d12
