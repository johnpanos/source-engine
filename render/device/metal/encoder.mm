//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.metal: the upload ring encoders stage into
//			(recording::IUploadStager).
//
//=============================================================================//

#include "metal_device.h"

#include <cstring>

namespace render::device::metal
{

void MetalDevice::StageUpload(
    RecordingEncoder &encoder, Command &command, std::span<const std::byte> bytes )
{
	std::lock_guard<std::mutex> lock( m_RingLock );
	m_Ring.Retire( m_Epoch, m_Completed.load( std::memory_order_acquire ) );
	const std::uint64_t id = ++m_NextAllocation;
	if ( const std::optional<std::uint64_t> offset = m_Ring.Allocate( bytes.size(), id ) )
	{
		// Shared storage: the submission's blit, committed after this write,
		// reads it.
		std::memcpy( static_cast<std::byte *>( [m_RingBuffer contents] ) + *offset, bytes.data(),
		    bytes.size() );
		command.ringOffset = *offset;
		command.fromRing = true;
		encoder.AddRingAllocation( id );
		return;
	}
	command.bytes.assign( bytes.begin(), bytes.end() );
}

void MetalDevice::AbandonUploads( const std::vector<std::uint64_t> &allocations )
{
	std::lock_guard<std::mutex> lock( m_RingLock );
	for ( std::uint64_t id : allocations )
		m_Ring.Abandon( id );
}

} // namespace render::device::metal
