//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl: encoders record CPU command lists (any thread),
//			and the upload ring they stage into.
//
//=============================================================================//

#include "gl_device.h"

#include <cstring>

namespace render::device::gl
{

// Upload ring ---------------------------------------------------------------------

std::optional<std::uint64_t> UploadRing::Allocate( std::uint64_t size, std::uint64_t id )
{
	size = ( size + 15u ) & ~std::uint64_t( 15u );
	if ( size == 0 || size >= m_Capacity )
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
	m_Live.push_back( { offset, size, id, {}, false, false } );
	m_Head = offset + size;
	return offset;
}

void UploadRing::Submit( std::uint64_t id, CompletionToken token )
{
	for ( Allocation &allocation : m_Live )
	{
		if ( allocation.id == id )
		{
			allocation.token = token;
			allocation.submitted = true;
		}
	}
}

void UploadRing::Abandon( std::uint64_t id )
{
	for ( Allocation &allocation : m_Live )
	{
		if ( allocation.id == id && !allocation.submitted )
			allocation.abandoned = true;
	}
}

void UploadRing::Retire( std::uint32_t epoch, std::uint64_t completed )
{
	while ( !m_Live.empty() )
	{
		const Allocation &front = m_Live.front();
		const bool done =
		    front.abandoned ||
		    ( front.submitted && ( front.token.epoch < epoch || front.token.value <= completed ) );
		if ( !done )
			break;
		m_Live.pop_front();
	}
	if ( m_Live.empty() )
		m_Head = 0;
}

void GlDevice::StageUpload( GlEncoder &encoder, Command &command, std::span<const std::byte> bytes )
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

// Encoder -----------------------------------------------------------------------

GlEncoder::~GlEncoder()
{
	if ( !m_Submitted )
		m_Device.AbandonUploads( m_RingAllocations );
}

void GlEncoder::TransitionTexture(
    TextureId texture, ResourceUsage before, ResourceUsage after, const SubresourceRange &range )
{
	Command command;
	command.op = Op::kTransitionTexture;
	command.a = texture.value;
	command.before = before;
	command.after = after;
	command.range = range;
	Push( std::move( command ) );
}

void GlEncoder::TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after )
{
	Command command;
	command.op = Op::kTransitionBuffer;
	command.a = buffer.value;
	command.before = before;
	command.after = after;
	Push( std::move( command ) );
}

void GlEncoder::ClearTexture(
    TextureId texture, const ClearColor &color, const SubresourceRange &range )
{
	NotRendering();
	Command command;
	command.op = Op::kClearTexture;
	command.a = texture.value;
	command.color = color;
	command.range = range;
	Push( std::move( command ) );
}

void GlEncoder::WriteBuffer(
    BufferId buffer, std::uint64_t offset, std::span<const std::byte> bytes )
{
	NotRendering();
	Command command;
	command.op = Op::kWriteBuffer;
	command.a = buffer.value;
	command.copy.destinationOffset = offset;
	command.copy.size = bytes.size();
	if ( bytes.empty() )
		m_Error = true;
	else
		m_Device.StageUpload( *this, command, bytes );
	Push( std::move( command ) );
}

void GlEncoder::CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyBuffer;
	command.a = source.value;
	command.b = destination.value;
	command.copy = copy;
	Push( std::move( command ) );
}

void GlEncoder::CopyTextureToBuffer(
    TextureId source, BufferId destination, const TextureBufferCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyTextureToBuffer;
	command.a = source.value;
	command.b = destination.value;
	command.textureCopy = copy;
	Push( std::move( command ) );
}

void GlEncoder::CopyBufferToTexture(
    BufferId source, TextureId destination, const TextureBufferCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyBufferToTexture;
	command.a = source.value;
	command.b = destination.value;
	command.textureCopy = copy;
	Push( std::move( command ) );
}

void GlEncoder::BeginRendering( const RenderingDesc &desc )
{
	NotRendering();
	m_Rendering = true;
	Command command;
	command.op = Op::kBeginRendering;
	command.colors.assign( desc.colors.begin(), desc.colors.end() );
	command.depth = desc.depth;
	command.width = desc.width;
	command.height = desc.height;
	if ( desc.width == 0 || desc.height == 0 || ( desc.colors.empty() && !desc.depth ) )
		m_Error = true;
	Push( std::move( command ) );
}

void GlEncoder::EndRendering()
{
	if ( !m_Rendering )
		m_Error = true;
	m_Rendering = false;
	Command command;
	command.op = Op::kEndRendering;
	Push( std::move( command ) );
}

void GlEncoder::SetPipeline( PipelineId pipeline )
{
	Command command;
	command.op = Op::kSetPipeline;
	command.a = pipeline.value;
	Push( std::move( command ) );
}

void GlEncoder::SetBindGroup( BindGroupRole role, BindGroupId group )
{
	Command command;
	command.op = Op::kSetBindGroup;
	command.a = group.value;
	command.slot = static_cast<std::uint32_t>( role );
	if ( command.slot >= kMaxBindGroups )
		m_Error = true;
	Push( std::move( command ) );
}

void GlEncoder::SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset )
{
	Command command;
	command.op = Op::kSetVertexBuffer;
	command.a = buffer.value;
	command.slot = slot;
	command.offset = offset;
	Push( std::move( command ) );
}

void GlEncoder::SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat format )
{
	Command command;
	command.op = Op::kSetIndexBuffer;
	command.a = buffer.value;
	command.offset = offset;
	command.indexFormat = format;
	Push( std::move( command ) );
}

void GlEncoder::SetViewport( const Viewport &viewport )
{
	Command command;
	command.op = Op::kSetViewport;
	command.viewport = viewport;
	Push( std::move( command ) );
}

void GlEncoder::Draw( std::uint32_t vertexCount, std::uint32_t instanceCount,
    std::uint32_t firstVertex, std::uint32_t firstInstance )
{
	Drawing();
	Command command;
	command.op = Op::kDraw;
	command.count = vertexCount;
	command.instances = instanceCount;
	command.first = firstVertex;
	command.firstInstance = firstInstance;
	Push( std::move( command ) );
}

void GlEncoder::DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount,
    std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance )
{
	Drawing();
	Command command;
	command.op = Op::kDrawIndexed;
	command.count = indexCount;
	command.instances = instanceCount;
	command.first = firstIndex;
	command.vertexOffset = vertexOffset;
	command.firstInstance = firstInstance;
	Push( std::move( command ) );
}

void GlEncoder::Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z )
{
	NotRendering();
	Command command;
	command.op = Op::kDispatch;
	command.count = x;
	command.first = y;
	command.firstInstance = z;
	Push( std::move( command ) );
}

void GlEncoder::SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes )
{
	Command command;
	command.op = Op::kSetDrawConstants;
	command.offset = offset;
	command.bytes.assign( bytes.begin(), bytes.end() );
	Push( std::move( command ) );
}

void GlEncoder::BeginLabel( std::string_view label )
{
	++m_Labels;
	Command command;
	command.op = Op::kBeginLabel;
	command.bytes.assign( reinterpret_cast<const std::byte *>( label.data() ),
	    reinterpret_cast<const std::byte *>( label.data() ) + label.size() );
	Push( std::move( command ) );
}

void GlEncoder::EndLabel()
{
	if ( m_Labels == 0 )
		m_Error = true;
	else
		--m_Labels;
	Command command;
	command.op = Op::kEndLabel;
	Push( std::move( command ) );
}

} // namespace render::device::gl
