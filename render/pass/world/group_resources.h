//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Private world-pass resource reuse. Render-sequence owned; no waiting and
// no reuse until the submission covering the last consumer has completed.

#ifndef RENDER_PASS_WORLD_GROUP_RESOURCES_H
#define RENDER_PASS_WORLD_GROUP_RESOURCES_H

#include "render/device/device.h"

#include <algorithm>
#include <vector>

namespace render::pass::world
{

class GroupResources
{
public:
	struct Buffer
	{
		device::BufferId id;
		std::uint64_t size = 0; // zero denotes a borrowed buffer
		device::ResourceUsage usage = device::ResourceUsage::kUndefined;
		device::ResourceUsage before = device::ResourceUsage::kUndefined;
	};
	struct Sampler
	{
		device::SamplerId id;
		bool owned = false; // cache owns ordinary descriptors; caller owns overflow
	};

	device::DeviceResult<Buffer> Acquire( device::IRenderDevice2 &device,
	    std::uint64_t size, device::ResourceUsage usage )
	{
		// Exact sizes preserve descriptor ranges and shader array lengths.
		const auto found = std::find_if( m_Buffers.begin(), m_Buffers.end(),
		    [&]( const Retired &entry )
		    {
			    return entry.buffer.size == size && entry.buffer.usage == usage &&
			           device.IsComplete( entry.after );
		    } );
		if ( found != m_Buffers.end() )
		{
			Buffer buffer = found->buffer;
			m_Bytes -= buffer.size;
			m_Buffers.erase( found );
			return buffer;
		}
		device::BufferDesc desc;
		desc.size = size;
		desc.usages = { device::ResourceUsage::kCopyDestination, usage };
		desc.debugName = "world group buffer";
		auto made = device.CreateBuffer( desc );
		if ( !made )
			return foundation::Unexpected( made.Error() );
		return Buffer{ made.Value(), size, usage };
	}

	void Retire( device::IRenderDevice2 &device, Buffer buffer, device::CompletionToken after )
	{
		if ( !buffer.id.IsValid() || buffer.size == 0 )
			return;
		// Bounds apply to retained idle and pending buffers together, independent
		// of machine RAM. Overflow still draws normally, with fenced destruction.
		constexpr std::uint64_t kMaxBytes = 64u << 20;
		constexpr std::size_t kMaxBuffers = 256;
		if ( buffer.size > kMaxBytes )
		{
			(void)device.Release( buffer.id, after );
			return;
		}
		while ( !m_Buffers.empty() &&
		        ( m_Buffers.size() >= kMaxBuffers || m_Bytes + buffer.size > kMaxBytes ) )
		{
			(void)device.Release( m_Buffers.front().buffer.id, m_Buffers.front().after );
			m_Bytes -= m_Buffers.front().buffer.size;
			m_Buffers.erase( m_Buffers.begin() );
		}
		m_Bytes += buffer.size;
		m_Buffers.push_back( { buffer, after } );
	}

	device::DeviceResult<Sampler> AcquireSampler(
	    device::IRenderDevice2 &device, const device::SamplerDesc &desc )
	{
		for ( const auto &entry : m_Samplers )
			if ( entry.first == desc )
				return Sampler{ entry.second, false };
		auto made = device.CreateSampler( desc );
		if ( !made )
			return foundation::Unexpected( made.Error() );
		constexpr std::size_t kMaxSamplers = 64;
		if ( m_Samplers.size() >= kMaxSamplers )
			return Sampler{ made.Value(), true };
		m_Samplers.emplace_back( desc, made.Value() );
		return Sampler{ made.Value(), false };
	}

	// Caller has drained the device and released all groups borrowing samplers.
	void Release( device::IRenderDevice2 &device )
	{
		for ( const Retired &entry : m_Buffers )
			(void)device.Release( entry.buffer.id, entry.after );
		for ( const auto &entry : m_Samplers )
			(void)device.Release( entry.second, {} );
		*this = GroupResources();
	}

private:
	struct Retired
	{
		Buffer buffer;
		device::CompletionToken after;
	};
	std::vector<Retired> m_Buffers;
	std::uint64_t m_Bytes = 0;
	std::vector<std::pair<device::SamplerDesc, device::SamplerId>> m_Samplers;
};

} // namespace render::pass::world

#endif // RENDER_PASS_WORLD_GROUP_RESOURCES_H
