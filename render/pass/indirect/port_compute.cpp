//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.indirect (RFC 0016 K12); see
//			public/render/pass/indirect/port_compute.h.
//
//=============================================================================//

#include "render/pass/indirect/port_compute.h"

#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <deque>
#include <map>
#include <mutex>
#include <span>
#include <string_view>
#include <vector>

namespace render::pass::indirect
{

using namespace render::device;

namespace
{

constexpr const char *kSdfSource = "render/pass/indirect/sdf_probe_trace.comp";

struct Buffer
{
	std::size_t bytes = 0;
	bool readback = false;
	std::vector<std::byte> copy; // what Map returns
	bool dirty = false;          // the copy holds bytes the device lacks
	bool bound = false;          // a dispatch wrote it (a readback: the device owns it)
	BufferId device;             // storage the dispatches read and write
	BufferId staging;            // a readback's host copy
	ResourceUsage usage = ResourceUsage::kUndefined;
	std::uint64_t retireAfter = 0; // 0: live
};

struct Program
{
	std::uint32_t bindings = 0;
	std::uint32_t pushBytes = 0;
	BindGroupLayoutId layout;
	PipelineId pipeline;
};

struct Dispatch
{
	std::uint32_t program = 0;
	std::vector<std::uint32_t> buffers;
	std::vector<std::byte> push;
	std::uint32_t groups[3] = { 1, 1, 1 };
};

struct Readback
{
	std::uint32_t buffer = 0;
	std::vector<gpu_compute::ByteRange> ranges; // merged, ascending; empty: all of it
};

struct Submitted
{
	std::uint64_t serial = 0;
	CompletionToken token;
	std::vector<Readback> readbacks; // buffers written in it, and what came home
};

// Sorted, clipped to the buffer and merged.
std::vector<gpu_compute::ByteRange> Merged(
    std::vector<gpu_compute::ByteRange> ranges, std::size_t bytes )
{
	std::vector<gpu_compute::ByteRange> out;
	std::sort( ranges.begin(), ranges.end(),
	    []( const gpu_compute::ByteRange &a, const gpu_compute::ByteRange &b )
	    {
		    return a.offset < b.offset;
	    } );
	for ( gpu_compute::ByteRange range : ranges )
	{
		if ( range.offset >= bytes || range.bytes == 0 )
			continue;
		range.bytes = std::min( range.bytes, bytes - range.offset );
		if ( !out.empty() && range.offset <= out.back().offset + out.back().bytes )
			out.back().bytes =
			    std::max( out.back().bytes, range.offset + range.bytes - out.back().offset );
		else
			out.push_back( range );
	}
	return out;
}

} // namespace

struct PortCompute::State
{
	std::vector<std::uint32_t> spirv;
	mutable std::mutex lock;
	std::map<std::uint32_t, Buffer> buffers;
	std::map<std::uint32_t, Program> programs;
	std::uint32_t nextId = 1;
	std::vector<Dispatch> queued;
	// WrittenRanges since the last submission, per read-back; a buffer
	// written without them comes home whole.
	std::map<std::uint32_t, std::vector<gpu_compute::ByteRange>> hinted;
	std::uint64_t nextSerial = 1;
	std::atomic<std::uint64_t> completed{ 0 };
	// Render sequence only.
	IRenderDevice2 *device = nullptr;
	std::deque<Submitted> inFlight;
	std::map<std::uint64_t, CompletionToken> tokens; // serial -> its submission

	void DropDeviceObjects()
	{
		for ( auto &[id, buffer] : buffers )
		{
			buffer.device = BufferId();
			buffer.staging = BufferId();
			buffer.usage = ResourceUsage::kUndefined;
			buffer.dirty = true; // the next device starts from the copies
		}
		for ( auto &[id, program] : programs )
			program.layout = BindGroupLayoutId(), program.pipeline = PipelineId();
		inFlight.clear();
		tokens.clear();
	}
};

PortCompute::PortCompute() : m_State( std::make_unique<State>() )
{
}

PortCompute::~PortCompute() = default;

std::unique_ptr<PortCompute> PortCompute::Create( std::span<const std::uint32_t> spirv )
{
	std::unique_ptr<PortCompute> service( new PortCompute() );
	service->m_State->spirv.assign( spirv.begin(), spirv.end() );
	return service;
}

gpu_compute::Caps PortCompute::Capabilities() const
{
	gpu_compute::Caps caps;
	caps.compute = true;
	return caps;
}

std::uint32_t PortCompute::CreateBuffer( std::size_t bytes, gpu_compute::BufferUse use )
{
	if ( bytes == 0 )
		return 0;
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	Buffer buffer;
	buffer.bytes = bytes;
	buffer.readback = use == gpu_compute::BufferUse::Readback;
	buffer.copy.assign( bytes, std::byte( 0 ) );
	buffer.dirty = true;
	const std::uint32_t id = s.nextId++;
	s.buffers.emplace( id, std::move( buffer ) );
	return id;
}

void *PortCompute::Map( std::uint32_t id )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const auto found = s.buffers.find( id );
	if ( found == s.buffers.end() || found->second.retireAfter != 0 )
		return nullptr;
	Buffer &buffer = found->second;
	// A producer writes an upload through its mapping, and a read-back's
	// first contents before a dispatch owns it.
	if ( !buffer.readback || !buffer.bound )
		buffer.dirty = true;
	return buffer.copy.data();
}

std::uint32_t PortCompute::CreateProgram( const char *name, const gpu_compute::Binding *bindings,
    std::uint32_t count, std::uint32_t pushBytes )
{
	if ( !name || std::string_view( name ) != kSdfProbeTraceProgram || count == 0 ||
	     pushBytes > 128 )
		return 0;
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		if ( bindings[i] != gpu_compute::Binding::Buffer )
			return 0; // scenes need ray query, which the port lacks
	}
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	Program program;
	program.bindings = count;
	program.pushBytes = pushBytes;
	const std::uint32_t id = s.nextId++;
	s.programs.emplace( id, program );
	return id;
}

std::uint64_t PortCompute::QueueDispatch( std::uint32_t program, const std::uint32_t *buffers,
    std::uint32_t count, const void *push, std::uint32_t pushBytes, std::uint32_t groupsX,
    std::uint32_t groupsY, std::uint32_t groupsZ )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const auto found = s.programs.find( program );
	if ( found == s.programs.end() || count != found->second.bindings ||
	     pushBytes != found->second.pushBytes )
		return 0;
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		const auto buffer = s.buffers.find( buffers[i] );
		if ( buffer == s.buffers.end() || buffer->second.retireAfter != 0 )
			return 0;
	}
	Dispatch dispatch;
	dispatch.program = program;
	dispatch.buffers.assign( buffers, buffers + count );
	const std::byte *bytes = static_cast<const std::byte *>( push );
	dispatch.push.assign( bytes, bytes + pushBytes );
	dispatch.groups[0] = groupsX;
	dispatch.groups[1] = groupsY;
	dispatch.groups[2] = groupsZ;
	s.queued.push_back( std::move( dispatch ) );
	return s.nextSerial;
}

void PortCompute::WrittenRanges(
    std::uint32_t buffer, const gpu_compute::ByteRange *ranges, std::uint32_t count )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	std::vector<gpu_compute::ByteRange> &hinted = s.hinted[buffer];
	hinted.insert( hinted.end(), ranges, ranges + count );
}

std::uint64_t PortCompute::CompletedSerial() const
{
	return m_State->completed.load( std::memory_order_acquire );
}

void PortCompute::Retire( std::uint32_t resource, std::uint64_t afterSerial )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( const auto buffer = s.buffers.find( resource ); buffer != s.buffers.end() )
		buffer->second.retireAfter = std::max<std::uint64_t>( afterSerial, 1 );
	else
		s.programs.erase( resource ); // programs are the service's to keep: nothing to wait for
}

bool PortCompute::Flush( IRenderDevice2 &device )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( s.device != &device )
	{
		// A new device: the old one's objects went with it.
		s.DropDeviceObjects();
		s.device = &device;
	}
	(void)device.Poll();

	// What completed: its read-backs come home, then the serial is published.
	std::uint64_t completed = s.completed.load( std::memory_order_relaxed );
	while ( !s.inFlight.empty() && device.IsComplete( s.inFlight.front().token ) )
	{
		const Submitted &done = s.inFlight.front();
		for ( const Readback &readback : done.readbacks )
		{
			const auto found = s.buffers.find( readback.buffer );
			if ( found == s.buffers.end() || !found->second.staging.IsValid() )
				continue;
			Buffer &buffer = found->second;
			if ( readback.ranges.empty() )
				(void)device.ReadBuffer( buffer.staging, 0, buffer.copy );
			for ( const gpu_compute::ByteRange &range : readback.ranges )
				(void)device.ReadBuffer( buffer.staging, range.offset,
				    std::span( buffer.copy ).subspan( range.offset, range.bytes ) );
		}
		completed = done.serial;
		s.inFlight.pop_front();
	}
	s.completed.store( completed, std::memory_order_release );

	// Retired buffers go once CompletedSerial() reaches their serial (the
	// contract: until then a producer may still read its mapping, and a
	// submission may still use the device buffers).
	for ( auto it = s.buffers.begin(); it != s.buffers.end(); )
	{
		Buffer &buffer = it->second;
		if ( buffer.retireAfter == 0 || buffer.retireAfter > completed )
		{
			++it;
			continue;
		}
		const auto token = s.tokens.find( buffer.retireAfter );
		const CompletionToken after = token != s.tokens.end() ? token->second : CompletionToken();
		for ( BufferId id : { buffer.device, buffer.staging } )
		{
			if ( id.IsValid() )
				(void)device.Release( id, after );
		}
		it = s.buffers.erase( it );
	}
	while ( !s.tokens.empty() && s.tokens.begin()->first + 64 < s.nextSerial )
		s.tokens.erase( s.tokens.begin() );
	if ( s.queued.empty() )
		return true;

	// The programs this submission uses, made on first use.
	std::vector<Dispatch> work;
	work.swap( s.queued );
	std::map<std::uint32_t, std::vector<gpu_compute::ByteRange>> hinted;
	hinted.swap( s.hinted );
	const std::uint64_t serial = s.nextSerial++;
	for ( const Dispatch &dispatch : work )
	{
		Program &program = s.programs[dispatch.program];
		if ( program.pipeline.IsValid() )
			continue;
		const ShaderStageSet compute{ ShaderStage::kCompute };
		std::vector<BindingDesc> bindings;
		for ( std::uint32_t b = 0; b < program.bindings; ++b )
			bindings.push_back( { b, BindingKind::kStorageBuffer, 1, compute } );
		auto layout = device.CreateBindGroupLayout( { BindGroupRole::kFrame, bindings } );
		if ( !layout )
			return false;
		program.layout = layout.Value();
		shaderlib::PipelineRecipe recipe =
		    shaderlib::CoreRecipe( { kSdfSource }, PipelineKind::kCompute );
		recipe.layouts = { program.layout, {}, {}, {} };
		recipe.debugName = kSdfSource;
		shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
		if ( !s.spirv.empty() &&
		     !artifacts.ReplaceSpirv( kSdfSource, s.spirv, device.Facts().artifactFormat ) )
			return false;
		auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
		if ( !resolved )
			return false;
		PipelineDesc desc = resolved.Value().Desc();
		desc.drawConstantBytes = program.pushBytes;
		auto pipeline = device.CreatePipeline( desc );
		if ( !pipeline )
			return false;
		program.pipeline = pipeline.Value();
	}

	auto encoded = device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoded )
		return false;
	CommandEncoder &encoder = encoded.Value();
	encoder.BeginLabel( "indirect producers" );
	std::vector<ResourceId> transient; // this submission's bind groups
	auto ensure = [&]( Buffer &buffer ) -> bool
	{
		if ( !buffer.device.IsValid() )
		{
			BufferDesc desc;
			desc.size = buffer.bytes;
			desc.usages = { ResourceUsage::kStorageRead, ResourceUsage::kStorageWrite,
			    ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
			desc.debugName = "indirect producer buffer";
			auto made = device.CreateBuffer( desc );
			if ( !made )
				return false;
			buffer.device = made.Value();
			buffer.usage = ResourceUsage::kUndefined;
		}
		if ( buffer.readback && !buffer.staging.IsValid() )
		{
			BufferDesc desc;
			desc.size = buffer.bytes;
			desc.usages = { ResourceUsage::kCopyDestination };
			desc.memory = MemoryKind::kReadback;
			desc.debugName = "indirect producer read-back";
			auto made = device.CreateBuffer( desc );
			if ( !made )
				return false;
			buffer.staging = made.Value();
		}
		if ( buffer.dirty )
		{
			encoder.TransitionBuffer(
			    buffer.device, buffer.usage, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( buffer.device, 0, buffer.copy );
			buffer.usage = ResourceUsage::kCopyDestination;
			buffer.dirty = false;
		}
		return true;
	};
	std::vector<std::uint32_t> written;
	bool ok = true;
	for ( const Dispatch &dispatch : work )
	{
		const Program &program = s.programs[dispatch.program];
		std::vector<BindGroupEntry> entries;
		for ( std::uint32_t b = 0; b < dispatch.buffers.size() && ok; ++b )
		{
			Buffer &buffer = s.buffers[dispatch.buffers[b]];
			ok = ensure( buffer );
			// Each dispatch's writes are visible to the next (read and
			// write storage throughout).
			encoder.TransitionBuffer( buffer.device, buffer.usage, ResourceUsage::kStorageWrite );
			buffer.usage = ResourceUsage::kStorageWrite;
			entries.push_back( { b, buffer.device, 0, 0, {}, {} } );
			if ( buffer.readback )
			{
				buffer.bound = true;
				if ( std::find( written.begin(), written.end(), dispatch.buffers[b] ) ==
				     written.end() )
					written.push_back( dispatch.buffers[b] );
			}
		}
		if ( !ok )
			break;
		auto group = device.CreateBindGroup( { program.layout, entries } );
		if ( !group )
		{
			ok = false;
			break;
		}
		transient.push_back( group.Value() );
		encoder.SetPipeline( program.pipeline );
		encoder.SetBindGroup( BindGroupRole::kFrame, group.Value() );
		if ( !dispatch.push.empty() )
			encoder.SetDrawConstants( 0, dispatch.push );
		encoder.Dispatch( dispatch.groups[0], dispatch.groups[1], dispatch.groups[2] );
	}
	// The read-backs: every buffer a dispatch wrote, copied home: the ranges
	// its producer said the dispatches write, else all of it. The host copy
	// holds the rest already.
	std::vector<Readback> readbacks;
	for ( const std::uint32_t id : written )
	{
		if ( !ok )
			break;
		Buffer &buffer = s.buffers[id];
		Readback readback{ id, {} };
		if ( const auto found = hinted.find( id ); found != hinted.end() )
		{
			readback.ranges = Merged( found->second, buffer.bytes );
			if ( readback.ranges.empty() )
				continue; // it wrote nothing
		}
		encoder.TransitionBuffer( buffer.device, buffer.usage, ResourceUsage::kCopySource );
		buffer.usage = ResourceUsage::kCopySource;
		encoder.TransitionBuffer(
		    buffer.staging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		if ( readback.ranges.empty() )
		{
			BufferCopy copy;
			copy.size = buffer.bytes;
			encoder.CopyBuffer( buffer.device, buffer.staging, copy );
		}
		for ( const gpu_compute::ByteRange &range : readback.ranges )
		{
			BufferCopy copy;
			copy.sourceOffset = range.offset;
			copy.destinationOffset = range.offset;
			copy.size = range.bytes;
			encoder.CopyBuffer( buffer.device, buffer.staging, copy );
		}
		readbacks.push_back( std::move( readback ) );
	}
	encoder.EndLabel();
	CommandEncoder encoders[] = { std::move( encoded ).Value() };
	auto token = device.Submit( QueueKind::kGraphics, encoders, {} );
	const CompletionToken after = token ? token.Value() : CompletionToken();
	for ( ResourceId id : transient )
		(void)device.Release( id, after );
	if ( !ok || !token )
	{
		// Nothing of this submission reads back: its serial never completes
		// with data, so the producers' in-flight limit holds them; the next
		// Flush starts from the copies.
		s.DropDeviceObjects();
		return false;
	}
	s.tokens[serial] = token.Value();
	s.inFlight.push_back( { serial, token.Value(), std::move( readbacks ) } );
	return true;
}

void PortCompute::ReleaseDevice( IRenderDevice2 &device )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( s.device != &device )
		return;
	for ( auto &[id, buffer] : s.buffers )
	{
		for ( BufferId made : { buffer.device, buffer.staging } )
		{
			if ( made.IsValid() )
				(void)device.Release( made, CompletionToken() );
		}
	}
	for ( auto &[id, program] : s.programs )
	{
		if ( program.pipeline.IsValid() )
			(void)device.Release( program.pipeline, CompletionToken() );
		if ( program.layout.IsValid() )
			(void)device.Release( program.layout, CompletionToken() );
	}
	(void)device.Poll();
	s.DropDeviceObjects();
	s.device = nullptr;
}

} // namespace render::pass::indirect
