//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 CommandEncoder (RFC 0016): forwards to the
//			adapter's backend and owns the one-recording-thread diagnostic.
//
//=============================================================================//

#include "render/device/encoder.h"

#include <utility>

namespace render::device
{

CommandEncoder::CommandEncoder( QueueKind queue, std::unique_ptr<IEncoderBackend> backend )
    : m_Queue( queue ), m_Backend( std::move( backend ) )
{
}

CommandEncoder::CommandEncoder( CommandEncoder &&other ) noexcept
    : m_Queue( other.m_Queue ), m_Backend( std::move( other.m_Backend ) ), m_Owner( other.m_Owner ),
      m_HasOwner( other.m_HasOwner ), m_Violations( other.m_Violations ),
      m_LabelObserver( other.m_LabelObserver )
{
}

CommandEncoder &CommandEncoder::operator=( CommandEncoder &&other ) noexcept
{
	if ( this != &other )
	{
		m_Queue = other.m_Queue;
		m_Backend = std::move( other.m_Backend );
		m_Owner = other.m_Owner;
		m_HasOwner = other.m_HasOwner;
		m_Violations = other.m_Violations;
		m_LabelObserver = other.m_LabelObserver;
	}
	return *this;
}

CommandEncoder::~CommandEncoder() = default;

IEncoderBackend *CommandEncoder::Enter()
{
	if ( !m_Backend )
		return nullptr;
	const std::thread::id self = std::this_thread::get_id();
	if ( !m_HasOwner )
	{
		m_Owner = self;
		m_HasOwner = true;
	}
	else if ( m_Owner != self )
		++m_Violations;
	return m_Backend.get();
}

std::unique_ptr<IEncoderBackend> CommandEncoder::TakeBackend()
{
	return std::move( m_Backend );
}

void CommandEncoder::TransitionTexture(
    TextureId texture, ResourceUsage before, ResourceUsage after, const SubresourceRange &range )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->TransitionTexture( texture, before, after, range );
}

void CommandEncoder::TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->TransitionBuffer( buffer, before, after );
}

void CommandEncoder::ClearTexture(
    TextureId texture, const ClearColor &color, const SubresourceRange &range )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->ClearTexture( texture, color, range );
}

void CommandEncoder::WriteBuffer(
    BufferId buffer, std::uint64_t offset, std::span<const std::byte> bytes )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->WriteBuffer( buffer, offset, bytes );
}

void CommandEncoder::CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->CopyBuffer( source, destination, copy );
}

void CommandEncoder::CopyTextureToBuffer(
    TextureId source, BufferId destination, const TextureBufferCopy &copy )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->CopyTextureToBuffer( source, destination, copy );
}

void CommandEncoder::CopyBufferToTexture(
    BufferId source, TextureId destination, const TextureBufferCopy &copy )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->CopyBufferToTexture( source, destination, copy );
}

void CommandEncoder::CopyTexture(
    TextureId source, TextureId destination, const TextureCopy &copy )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->CopyTexture( source, destination, copy );
}

void CommandEncoder::BeginRendering( const RenderingDesc &desc )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->BeginRendering( desc );
}

void CommandEncoder::EndRendering()
{
	if ( IEncoderBackend *backend = Enter() )
		backend->EndRendering();
}

void CommandEncoder::ClearRegion( const render::device::ClearRegion &region )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->ClearRegion( region );
}

void CommandEncoder::SetPipeline( PipelineId pipeline )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->SetPipeline( pipeline );
}

void CommandEncoder::SetBindGroup( BindGroupRole role, BindGroupId group )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->SetBindGroup( role, group );
}

void CommandEncoder::SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->SetVertexBuffer( slot, buffer, offset );
}

void CommandEncoder::SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat format )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->SetIndexBuffer( buffer, offset, format );
}

void CommandEncoder::SetViewport( const Viewport &viewport )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->SetViewport( viewport );
}

void CommandEncoder::Draw( std::uint32_t vertexCount, std::uint32_t instanceCount,
    std::uint32_t firstVertex, std::uint32_t firstInstance )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->Draw( vertexCount, instanceCount, firstVertex, firstInstance );
}

void CommandEncoder::DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount,
    std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->DrawIndexed( indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
}

bool IndirectRecordsFit(
    std::uint64_t bufferSize, std::uint64_t offset, std::uint32_t drawCount, std::uint32_t stride )
{
	if ( offset % 4 != 0 || stride % 4 != 0 || stride < sizeof( DrawIndexedIndirectCommand ) )
		return false;
	if ( drawCount == 0 )
		return offset <= bufferSize;
	const std::uint64_t last = offset + std::uint64_t( drawCount - 1 ) * stride;
	return last >= offset && last <= bufferSize &&
	       bufferSize - last >= sizeof( DrawIndexedIndirectCommand );
}

void CommandEncoder::DrawIndexedIndirect(
    BufferId buffer, std::uint64_t offset, std::uint32_t drawCount, std::uint32_t stride )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->DrawIndexedIndirect( buffer, offset, drawCount, stride );
}

void CommandEncoder::DrawIndexedIndirectCount( BufferId buffer, std::uint64_t offset,
    BufferId countBuffer, std::uint64_t countOffset, std::uint32_t maxDrawCount,
    std::uint32_t stride )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->DrawIndexedIndirectCount(
		    buffer, offset, countBuffer, countOffset, maxDrawCount, stride );
}

void CommandEncoder::Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->Dispatch( x, y, z );
}

void CommandEncoder::SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->SetDrawConstants( offset, bytes );
}

void CommandEncoder::BeginLabel( std::string_view label )
{
	IEncoderBackend *backend = Enter();
	if ( !backend )
		return;
	backend->BeginLabel( label );
	if ( m_LabelObserver )
		m_LabelObserver->OnBeginLabel( *this, label );
}

void CommandEncoder::EndLabel()
{
	IEncoderBackend *backend = Enter();
	if ( !backend )
		return;
	if ( m_LabelObserver )
		m_LabelObserver->OnEndLabel( *this );
	backend->EndLabel();
}

void CommandEncoder::WriteTimestamp( BufferId buffer, std::uint64_t offset )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->WriteTimestamp( buffer, offset );
}

void CommandEncoder::BeginOcclusionQuery( BufferId buffer, std::uint64_t offset )
{
	if ( IEncoderBackend *backend = Enter() )
		backend->BeginOcclusionQuery( buffer, offset );
}

void CommandEncoder::EndOcclusionQuery()
{
	if ( IEncoderBackend *backend = Enter() )
		backend->EndOcclusionQuery();
}

} // namespace render::device
