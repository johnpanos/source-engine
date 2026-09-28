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
      m_Violations( other.m_Violations )
{
}

CommandEncoder &CommandEncoder::operator=( CommandEncoder &&other ) noexcept
{
	if ( this != &other )
	{
		m_Queue = other.m_Queue;
		m_Backend = std::move( other.m_Backend );
		m_Owner = other.m_Owner;
		m_Violations = other.m_Violations;
	}
	return *this;
}

CommandEncoder::~CommandEncoder() = default;

IEncoderBackend *CommandEncoder::Enter()
{
	if ( !m_Backend )
		return nullptr;
	const std::thread::id self = std::this_thread::get_id();
	if ( m_Owner == std::thread::id() )
		m_Owner = self;
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
	if ( IEncoderBackend *backend = Enter() )
		backend->BeginLabel( label );
}

void CommandEncoder::EndLabel()
{
	if ( IEncoderBackend *backend = Enter() )
		backend->EndLabel();
}

} // namespace render::device
