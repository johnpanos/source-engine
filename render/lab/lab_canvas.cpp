//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's canvas (RFC 0016 K11); see lab_canvas.h.
//
//=============================================================================//

#include "lab_canvas.h"

#include "lab_support.h"

#include <cstring>

namespace render::lab
{

using namespace render::device;

std::optional<std::string> Canvas::Create( IRenderDevice2 &device, std::uint32_t width,
    std::uint32_t height, std::unique_ptr<Canvas> &out )
{
	std::unique_ptr<Canvas> canvas( new Canvas( device ) );
	canvas->m_Width = width;
	canvas->m_Height = height;
	TextureDesc color;
	color.format = kCanvasColor;
	color.width = width;
	color.height = height;
	color.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
	color.debugName = "render_lab.canvas.color";
	TextureDesc depth = color;
	depth.format = kCanvasDepth;
	depth.usages = { ResourceUsage::kDepthWrite };
	depth.debugName = "render_lab.canvas.depth";
	BufferDesc readback;
	readback.size = std::uint64_t( width ) * height * 8;
	readback.usages = { ResourceUsage::kCopyDestination };
	readback.memory = MemoryKind::kReadback;
	auto colorId = device.CreateTexture( color );
	auto depthId = device.CreateTexture( depth );
	auto readbackId = device.CreateBuffer( readback );
	canvas->m_ColorDesc = color;
	if ( colorId )
		canvas->m_Color = colorId.Value();
	if ( depthId )
		canvas->m_Depth = depthId.Value();
	if ( readbackId )
		canvas->m_Readback = readbackId.Value();
	if ( !colorId || !depthId || !readbackId )
		return std::string( "the canvas's targets were refused" );
	out = std::move( canvas );
	return std::nullopt;
}

Canvas::~Canvas()
{
	(void)m_Device.WaitIdle();
	for ( BufferId buffer : m_Buffers )
		(void)m_Device.Release( buffer, CompletionToken() );
	if ( m_Readback.IsValid() )
		(void)m_Device.Release( m_Readback, CompletionToken() );
	if ( m_Color.IsValid() )
		(void)m_Device.Release( m_Color, CompletionToken() );
	if ( m_Depth.IsValid() )
		(void)m_Device.Release( m_Depth, CompletionToken() );
}

BufferId Canvas::Vertices( std::span<const std::byte> bytes )
{
	BufferDesc desc;
	desc.size = bytes.size();
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
	auto buffer = m_Device.CreateBuffer( desc );
	if ( !buffer )
		return BufferId();
	m_Buffers.push_back( buffer.Value() );
	m_Writes.push_back( { buffer.Value(), std::vector<std::byte>( bytes.begin(), bytes.end() ) } );
	return buffer.Value();
}

std::optional<std::string> Canvas::Render( resources::TextureCache &textures,
    material::GroupResidency &groups, std::span<const CanvasDraw> draws, const ClearColor &clear,
    CanvasImage *out )
{
	auto encoded = m_Device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoded )
		return std::string( "no encoder" );
	CommandEncoder &encoder = encoded.Value();
	textures.RecordUploads( encoder );
	groups.RecordUploads( encoder );
	for ( const PendingWrite &write : m_Writes )
	{
		encoder.TransitionBuffer(
		    write.buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( write.buffer, 0, write.bytes );
		encoder.TransitionBuffer(
		    write.buffer, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
	}
	m_Writes.clear();
	encoder.TransitionTexture(
	    m_Color, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	encoder.TransitionTexture( m_Depth, ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
	const ColorAttachment attachments[] = {
	    { m_Color, LoadOp::kClear, StoreOp::kStore, clear, {} } };
	RenderingDesc rendering;
	rendering.colors = attachments;
	rendering.depth = DepthAttachment{ m_Depth, LoadOp::kClear, StoreOp::kStore, 1.0f };
	rendering.width = m_Width;
	rendering.height = m_Height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport( { 0, 0, float( m_Width ), float( m_Height ), 0, 1 } );
	for ( const CanvasDraw &draw : draws )
	{
		encoder.SetPipeline( draw.pipeline );
		for ( std::size_t role = 0; role < draw.groups.size(); ++role )
		{
			if ( draw.groups[role].IsValid() )
				encoder.SetBindGroup( BindGroupRole( role ), draw.groups[role] );
		}
		encoder.SetVertexBuffer( 0, draw.vertices, 0 );
		if ( !draw.constants.empty() )
			encoder.SetDrawConstants( 0, draw.constants );
		encoder.Draw( draw.vertexCount, 1, 0, 0 );
	}
	encoder.EndRendering();
	encoder.TransitionTexture(
	    m_Color, ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
	if ( out )
	{
		encoder.TransitionBuffer(
		    m_Readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.CopyTextureToBuffer( m_Color, m_Readback, { 0, 0, 0, m_Width, m_Height } );
	}
	auto token = m_Device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	if ( !token )
		return std::string( "the canvas frame was refused at submission" );
	(void)m_Device.WaitIdle();
	textures.Retire( token.Value() );
	groups.Retire( token.Value() );
	if ( !out )
		return std::nullopt;
	std::vector<std::byte> pixels( std::size_t( m_Width ) * m_Height * 8 );
	if ( !m_Device.ReadBuffer( m_Readback, 0, pixels ) )
		return std::string( "the canvas did not read back" );
	out->width = m_Width;
	out->height = m_Height;
	out->rgba.resize( std::size_t( m_Width ) * m_Height * 4 );
	for ( std::size_t i = 0; i < out->rgba.size(); ++i )
	{
		std::uint16_t half;
		std::memcpy( &half, pixels.data() + i * 2, sizeof( half ) );
		out->rgba[i] = HalfToFloat( half );
	}
	return std::nullopt;
}

} // namespace render::lab
