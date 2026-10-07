//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl: the upload ring encoders stage into
//			(recording::IUploadStager).
//
//=============================================================================//

#include "gl_device.h"

#include <cstring>

namespace render::device::gl
{

void GlDevice::StageUpload( RecordingEncoder &encoder, Command &command, std::span<const std::byte> bytes )
{
	std::lock_guard<std::mutex> lock( m_RingLock );
	m_Ring.Retire( m_Epoch, m_Completed.load( std::memory_order_acquire ) );
	const std::uint64_t id = ++m_NextAllocation;
	if ( m_RingData )
	{
		if ( const std::optional<std::uint64_t> offset = m_Ring.Allocate( bytes.size(), id ) )
		{
			// Coherent and persistently mapped: GL commands issued after this
			// write (the submission's copy) see it.
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

void GlDevice::AbandonUploads( const std::vector<std::uint64_t> &allocations )
{
	std::lock_guard<std::mutex> lock( m_RingLock );
	for ( std::uint64_t id : allocations )
		m_Ring.Abandon( id );
}

std::uint64_t GlDevice::DeferredUploads() const
{
	std::lock_guard<std::mutex> lock( m_RingLock );
	return m_DeferredUploads;
}

} // namespace render::device::gl
