//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.webgpu: submission. Replay records the accepted
//			lists (validated by recording::Validate) in order into the
//			submission's command buffer.
//
//=============================================================================//

#include "webgpu_device.h"

#include <cstdio>
#include <cstring>
#include <unordered_set>

namespace render::device::webgpu
{

namespace
{

WGPULoadOp LoadOf( LoadOp load )
{
	switch ( load )
	{
	case LoadOp::kLoad:
		return WGPULoadOp_Load;
	case LoadOp::kClear:
		return WGPULoadOp_Clear;
	case LoadOp::kDiscard:
		// WebGPU has no "don't care" load: clearing is the cheap defined one.
		return WGPULoadOp_Clear;
	}
	return WGPULoadOp_Load;
}

std::uint64_t AlignUp( std::uint64_t value, std::uint64_t alignment )
{
	return ( value + alignment - 1 ) / alignment * alignment;
}

} // namespace

// Records one submission's encoders into a command buffer, in order.
class Replay
{
public:
	Replay( WebGpuDevice &device, Submission &submission )
	    : m_D( device ), m_Submission( submission )
	{
	}

	CommandBuffer Run( std::vector<RecordingEncoder *> &encoders );

private:
	struct Binding
	{
		std::uint64_t buffer = 0;
		std::uint64_t offset = 0;
	};

	void Plan( std::vector<RecordingEncoder *> &encoders );
	bool Execute( const Command &command );
	void EndPasses();
	WGPUComputePassEncoder ComputePass();
	void ClearTexture( const Command &command );
	void CopyTextureAndBuffer( const Command &command );
	void UploadDepth( TextureRecord &t, BufferRecord &b, const TextureBufferCopy &copy );
	bool BeginRendering( const Command &command );
	bool BindGroups( bool compute );
	bool PrepareDraw();
	void UseReadback( std::uint64_t id );
	WGPUBuffer Temporary( std::uint64_t size, WGPUBufferUsage usage );

	WebGpuDevice &m_D;
	Submission &m_Submission;
	WGPUCommandEncoder m_Commands = nullptr;
	WGPURenderPassEncoder m_Render = nullptr;
	WGPUComputePassEncoder m_Compute = nullptr;
	const PipelineRecord *m_Pipeline = nullptr;
	bool m_PipelineDirty = true;
	std::array<std::uint64_t, kMaxBindGroups> m_Groups{};
	std::array<Binding, kMaxVertexSlots> m_VertexBuffers{};
	Binding m_Index;
	IndexFormat m_IndexFormat = IndexFormat::kUint16;
	std::array<std::byte, kMaxDrawConstantBytes> m_Constants{};
	bool m_ConstantsDirty = true;

	// The submission's uploads and draw constants, written by the queue
	// before the command buffer runs.
	std::vector<std::byte> m_Uploads;
	std::uint64_t m_UploadCursor = 0;
	WGPUBuffer m_UploadBuffer = nullptr;
	std::vector<std::byte> m_ConstantBytes;
	std::uint64_t m_ConstantCursor = 0;
	std::uint64_t m_ConstantOffset = 0;
	std::uint64_t m_ConstantAlignment = 256;
	WGPUBuffer m_ConstantBuffer = nullptr;
	std::map<std::pair<std::uint64_t, const GroupLayout *>, WGPUBindGroup> m_ConstantGroups;
	std::vector<std::uint64_t> m_Readbacks;
	struct RegionWrite
	{
		WGPUBuffer buffer;
		std::array<std::uint32_t, 4> words;
	};
	std::vector<RegionWrite> m_RegionWrites;
	std::unordered_set<std::uint64_t> m_ReadbackSet;
};

WGPUBuffer Replay::Temporary( std::uint64_t size, WGPUBufferUsage usage )
{
	WGPUBufferDescriptor descriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
	descriptor.usage = usage;
	descriptor.size = AlignUp( std::max<std::uint64_t>( size, 4 ), 4 );
	m_Submission.buffers.emplace_back( wgpuDeviceCreateBuffer( m_D.Native(), &descriptor ) );
	return m_Submission.buffers.back().Get();
}

void Replay::UseReadback( std::uint64_t id )
{
	BufferRecord *b = m_D.ExistingBuffer( id );
	if ( b && b->desc.memory == MemoryKind::kReadback && m_ReadbackSet.insert( id ).second )
		m_Readbacks.push_back( id );
}

// Sizes the submission's upload and constants buffers, and finds the
// readback buffers it names.
void Replay::Plan( std::vector<RecordingEncoder *> &encoders )
{
	std::uint64_t uploads = 0;
	std::uint64_t draws = 0;
	for ( RecordingEncoder *encoder : encoders )
	{
		for ( const Command &command : encoder->Commands() )
		{
			switch ( command.op )
			{
			case Op::kWriteBuffer:
				uploads += AlignUp( command.bytes.size(), 4 );
				UseReadback( command.a );
				break;
			case Op::kCopyBuffer:
				UseReadback( command.a );
				UseReadback( command.b );
				break;
			case Op::kCopyTextureToBuffer:
				UseReadback( command.b );
				break;
			case Op::kCopyBufferToTexture:
			case Op::kSetVertexBuffer:
			case Op::kSetIndexBuffer:
			case Op::kDrawIndexedIndirect:
				UseReadback( command.a );
				break;
			case Op::kSetBindGroup:
				if ( BindGroupRecord *group = m_D.ExistingBindGroup( command.a ) )
				{
					for ( const BindGroupEntry &entry : group->entries )
					{
						if ( entry.buffer.IsValid() )
							UseReadback( entry.buffer.value );
					}
				}
				break;
			case Op::kDraw:
			case Op::kDrawIndexed:
			case Op::kDispatch:
				++draws;
				break;
			default:
				break;
			}
		}
	}
	if ( uploads )
	{
		m_Uploads.resize( uploads );
		m_UploadBuffer = Temporary( uploads, WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst );
	}
	m_ConstantAlignment =
	    std::max<std::uint64_t>( m_D.NativeLimits().minUniformBufferOffsetAlignment, 4 );
	if ( draws )
	{
		const std::uint64_t size = draws * m_ConstantAlignment + kMaxDrawConstantBytes;
		m_ConstantBytes.resize( size );
		m_ConstantBuffer = Temporary( size, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst );
	}
	m_Submission.readbacks = m_Readbacks;
}

CommandBuffer Replay::Run( std::vector<RecordingEncoder *> &encoders )
{
	Plan( encoders );
	WGPUCommandEncoderDescriptor descriptor = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
	m_Commands = wgpuDeviceCreateCommandEncoder( m_D.Native(), &descriptor );
	bool ok = true;
	for ( RecordingEncoder *encoder : encoders )
	{
		m_Pipeline = nullptr;
		m_PipelineDirty = true;
		m_Groups.fill( 0 );
		m_VertexBuffers.fill( {} );
		m_Index = {};
		m_Constants.fill( std::byte{ 0 } );
		m_ConstantsDirty = true;
		for ( const Command &command : encoder->Commands() )
		{
			if ( !Execute( command ) )
			{
				ok = false;
				break;
			}
		}
		EndPasses();
		if ( !ok )
			break;
	}
	// Each readback buffer the submission named lands in its MapRead copy.
	for ( std::uint64_t id : m_Readbacks )
	{
		BufferRecord *b = m_D.ExistingBuffer( id );
		wgpuCommandEncoderCopyBufferToBuffer(
		    m_Commands, b->buffer.Get(), 0, b->readback.Get(), 0, b->allocated );
	}
	CommandBuffer commands( wgpuCommandEncoderFinish( m_Commands, nullptr ) );
	wgpuCommandEncoderRelease( m_Commands );
	if ( !ok )
		return {};
	const WGPUQueue queue = wgpuDeviceGetQueue( m_D.Native() );
	if ( m_UploadBuffer && m_UploadCursor )
		wgpuQueueWriteBuffer( queue, m_UploadBuffer, 0, m_Uploads.data(), m_UploadCursor );
	for ( const RegionWrite &write : m_RegionWrites )
		wgpuQueueWriteBuffer( queue, write.buffer, 0, write.words.data(), sizeof( write.words ) );
	if ( m_ConstantBuffer && m_ConstantCursor )
		wgpuQueueWriteBuffer( queue, m_ConstantBuffer, 0, m_ConstantBytes.data(),
		    m_ConstantCursor + kMaxDrawConstantBytes );
	wgpuQueueRelease( queue );
	return commands;
}

void Replay::EndPasses()
{
	if ( m_Render )
	{
		wgpuRenderPassEncoderEnd( m_Render );
		wgpuRenderPassEncoderRelease( m_Render );
		m_Render = nullptr;
	}
	if ( m_Compute )
	{
		wgpuComputePassEncoderEnd( m_Compute );
		wgpuComputePassEncoderRelease( m_Compute );
		m_Compute = nullptr;
	}
}

WGPUComputePassEncoder Replay::ComputePass()
{
	if ( !m_Compute )
	{
		EndPasses();
		WGPUComputePassDescriptor descriptor = WGPU_COMPUTE_PASS_DESCRIPTOR_INIT;
		m_Compute = wgpuCommandEncoderBeginComputePass( m_Commands, &descriptor );
		m_PipelineDirty = true;
	}
	return m_Compute;
}

bool Replay::Execute( const Command &command )
{
	switch ( command.op )
	{
	case Op::kTransitionTexture:
	case Op::kTransitionBuffer:
		// WebGPU orders the accesses itself.
		break;
	case Op::kClearTexture:
		ClearTexture( command );
		break;
	case Op::kWriteBuffer:
	{
		EndPasses();
		// Whole words: a write that ends at the buffer's end (the only partial
		// word Inexpressible lets through) fills its last word with zeros, in
		// the allocation's padding.
		const std::size_t size = AlignUp( command.bytes.size(), 4 );
		std::memcpy(
		    m_Uploads.data() + m_UploadCursor, command.bytes.data(), command.bytes.size() );
		std::memset( m_Uploads.data() + m_UploadCursor + command.bytes.size(), 0,
		    size - command.bytes.size() );
		wgpuCommandEncoderCopyBufferToBuffer( m_Commands, m_UploadBuffer, m_UploadCursor,
		    m_D.ExistingBuffer( command.a )->buffer.Get(), command.copy.destinationOffset, size );
		m_UploadCursor += size;
		break;
	}
	case Op::kCopyBuffer:
		EndPasses();
		wgpuCommandEncoderCopyBufferToBuffer( m_Commands,
		    m_D.ExistingBuffer( command.a )->buffer.Get(), command.copy.sourceOffset,
		    m_D.ExistingBuffer( command.b )->buffer.Get(), command.copy.destinationOffset,
		    command.copy.size );
		break;
	case Op::kCopyTextureToBuffer:
	case Op::kCopyBufferToTexture:
		CopyTextureAndBuffer( command );
		break;
	case Op::kCopyTexture:
	{
		EndPasses();
		TextureRecord &s = *m_D.ExistingTexture( command.a );
		TextureRecord &d = *m_D.ExistingTexture( command.b );
		const TextureBufferCopy &copy = command.textureCopy;
		const FormatBlock block = BlockOf( s.desc.format );
		WGPUTexelCopyTextureInfo source = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
		source.texture = s.texture.Get();
		source.mipLevel = copy.mip;
		source.origin = { copy.x, copy.y, copy.layer };
		WGPUTexelCopyTextureInfo destination = source;
		destination.texture = d.texture.Get();
		const WGPUExtent3D size{ static_cast<std::uint32_t>( AlignUp( copy.width, block.width ) ),
		    static_cast<std::uint32_t>( AlignUp( copy.height, block.height ) ), 1 };
		wgpuCommandEncoderCopyTextureToTexture( m_Commands, &source, &destination, &size );
		break;
	}
	case Op::kBeginRendering:
		return BeginRendering( command );
	case Op::kEndRendering:
		EndPasses();
		break;
	case Op::kSetPipeline:
		m_Pipeline = m_D.ExistingPipeline( command.a );
		m_PipelineDirty = true;
		break;
	case Op::kSetBindGroup:
		m_Groups[command.slot] = command.a;
		break;
	case Op::kSetVertexBuffer:
		m_VertexBuffers[command.slot] = { command.a, command.offset };
		break;
	case Op::kSetIndexBuffer:
		m_Index = { command.a, command.offset };
		m_IndexFormat = command.indexFormat;
		break;
	case Op::kSetViewport:
		if ( m_Render )
			wgpuRenderPassEncoderSetViewport( m_Render, command.viewport.x, command.viewport.y,
			    command.viewport.width, command.viewport.height, command.viewport.minDepth,
			    command.viewport.maxDepth );
		break;
	case Op::kSetDrawConstants:
		std::memcpy(
		    m_Constants.data() + command.offset, command.bytes.data(), command.bytes.size() );
		m_ConstantsDirty = true;
		break;
	case Op::kDraw:
		if ( !PrepareDraw() )
			return false;
		wgpuRenderPassEncoderDraw(
		    m_Render, command.count, command.instances, command.first, command.firstInstance );
		break;
	case Op::kDrawIndexed:
		if ( !PrepareDraw() )
			return false;
		wgpuRenderPassEncoderDrawIndexed( m_Render, command.count, command.instances, command.first,
		    command.vertexOffset, command.firstInstance );
		break;
	case Op::kDrawIndexedIndirect:
	{
		// D30: one draw per record; the records have WebGPU's layout.
		if ( !PrepareDraw() )
			return false;
		const WGPUBuffer records = m_D.ExistingBuffer( command.a )->buffer.Get();
		for ( std::uint32_t i = 0; i < command.count; ++i )
			wgpuRenderPassEncoderDrawIndexedIndirect(
			    m_Render, records, command.offset + std::uint64_t( i ) * command.first );
		break;
	}
	case Op::kDispatch:
	{
		const WGPUComputePassEncoder compute = ComputePass();
		if ( m_PipelineDirty )
		{
			wgpuComputePassEncoderSetPipeline( compute, m_Pipeline->compute.Get() );
			m_PipelineDirty = false;
		}
		if ( !BindGroups( true ) )
			return false;
		wgpuComputePassEncoderDispatchWorkgroups(
		    compute, command.count, command.first, command.firstInstance );
		break;
	}
	case Op::kBeginLabel:
	case Op::kEndLabel:
		break;
	case Op::kWriteTimestamp:
	case Op::kBeginOcclusionQuery:
	case Op::kEndOcclusionQuery:
	case Op::kDrawIndexedIndirectCount:
	case Op::kClearRegion:
		break; // refused at Submit (unclaimed capabilities)
	}
	return true;
}

// Each mip and layer in the range is cleared by an empty render pass.
void Replay::ClearTexture( const Command &command )
{
	EndPasses();
	TextureRecord &t = *m_D.ExistingTexture( command.a );
	const bool depth = IsDepthFormat( t.desc.format );
	const SubresourceRange &r = command.range;
	const std::uint32_t mips = std::min( r.baseMip + r.mipCount, t.desc.mipLevels );
	const std::uint32_t layers = std::min( r.baseLayer + r.layerCount,
	    t.desc.dimension == TextureDimension::k3D ? t.desc.depthOrLayers : t.layers );
	for ( std::uint32_t mip = r.baseMip; mip < mips; ++mip )
	{
		for ( std::uint32_t layer = r.baseLayer; layer < layers; ++layer )
		{
			const bool volume = t.desc.dimension == TextureDimension::k3D;
			WGPURenderPassDescriptor pass = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
			WGPURenderPassColorAttachment color = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
			WGPURenderPassDepthStencilAttachment depthStencil =
			    WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
			const WGPUTextureView view = m_D.AttachmentView( t, mip, volume ? 0 : layer );
			if ( depth )
			{
				depthStencil.view = view;
				depthStencil.depthLoadOp = WGPULoadOp_Clear;
				depthStencil.depthStoreOp = WGPUStoreOp_Store;
				depthStencil.depthClearValue = command.color.r;
				if ( HasStencil( t.desc.format ) )
				{
					depthStencil.stencilLoadOp = WGPULoadOp_Clear;
					depthStencil.stencilStoreOp = WGPUStoreOp_Store;
					depthStencil.stencilClearValue = 0;
				}
				pass.depthStencilAttachment = &depthStencil;
			}
			else
			{
				// The clear color is linear; an sRGB attachment encodes it.
				color.view = view;
				color.depthSlice = volume ? layer : WGPU_DEPTH_SLICE_UNDEFINED;
				color.loadOp = WGPULoadOp_Clear;
				color.storeOp = WGPUStoreOp_Store;
				color.clearValue = {
				    command.color.r, command.color.g, command.color.b, command.color.a };
				pass.colorAttachmentCount = 1;
				pass.colorAttachments = &color;
			}
			const WGPURenderPassEncoder clear =
			    wgpuCommandEncoderBeginRenderPass( m_Commands, &pass );
			wgpuRenderPassEncoderEnd( clear );
			wgpuRenderPassEncoderRelease( clear );
		}
	}
}

// A tightly packed region: rows of a multiple of 256 bytes copy directly;
// others go through a padded buffer of the submission, row by row.
void Replay::CopyTextureAndBuffer( const Command &command )
{
	EndPasses();
	const bool toBuffer = command.op == Op::kCopyTextureToBuffer;
	TextureRecord &t = *m_D.ExistingTexture( toBuffer ? command.a : command.b );
	BufferRecord &b = *m_D.ExistingBuffer( toBuffer ? command.b : command.a );
	const TextureBufferCopy &copy = command.textureCopy;
	const FormatBlock block = BlockOf( t.desc.format );
	if ( !toBuffer && IsDepthFormat( t.desc.format ) )
	{
		UploadDepth( t, b, copy );
		return;
	}
	const std::uint64_t row = RegionBytes( t.desc.format, copy.width, 1 );
	const std::uint32_t rows =
	    static_cast<std::uint32_t>( AlignUp( copy.height, block.height ) / block.height );
	WGPUTexelCopyTextureInfo texture = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
	texture.texture = t.texture.Get();
	texture.mipLevel = copy.mip;
	texture.origin = { copy.x, copy.y, copy.layer };
	if ( IsDepthFormat( t.desc.format ) )
		texture.aspect = WGPUTextureAspect_DepthOnly;
	const WGPUExtent3D size{ static_cast<std::uint32_t>( AlignUp( copy.width, block.width ) ),
	    static_cast<std::uint32_t>( AlignUp( copy.height, block.height ) ), 1 };
	WGPUTexelCopyBufferInfo buffer = WGPU_TEXEL_COPY_BUFFER_INFO_INIT;
	if ( row % kCopyRowAlignment == 0 )
	{
		buffer.buffer = b.buffer.Get();
		buffer.layout.offset = copy.bufferOffset;
		buffer.layout.bytesPerRow = static_cast<std::uint32_t>( row );
		buffer.layout.rowsPerImage = rows;
		if ( toBuffer )
			wgpuCommandEncoderCopyTextureToBuffer( m_Commands, &texture, &buffer, &size );
		else
			wgpuCommandEncoderCopyBufferToTexture( m_Commands, &buffer, &texture, &size );
		return;
	}
	const std::uint64_t padded = AlignUp( row, kCopyRowAlignment );
	const WGPUBuffer staging =
	    Temporary( padded * rows, WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst );
	buffer.buffer = staging;
	buffer.layout.bytesPerRow = static_cast<std::uint32_t>( padded );
	buffer.layout.rowsPerImage = rows;
	if ( toBuffer )
	{
		wgpuCommandEncoderCopyTextureToBuffer( m_Commands, &texture, &buffer, &size );
		for ( std::uint32_t r = 0; r < rows; ++r )
			wgpuCommandEncoderCopyBufferToBuffer(
			    m_Commands, staging, r * padded, b.buffer.Get(), copy.bufferOffset + r * row, row );
	}
	else
	{
		for ( std::uint32_t r = 0; r < rows; ++r )
			wgpuCommandEncoderCopyBufferToBuffer(
			    m_Commands, b.buffer.Get(), copy.bufferOffset + r * row, staging, r * padded, row );
		wgpuCommandEncoderCopyBufferToTexture( m_Commands, &buffer, &texture, &size );
	}
}

// WebGPU copies no buffer into a depth texture: the region is drawn, each
// texel's depth read from the buffer's 32-bit floats (DepthUploadFor).
void Replay::UploadDepth( TextureRecord &t, BufferRecord &b, const TextureBufferCopy &copy )
{
	const WGPUTextureFormat format = TextureFormatOf( t.desc.format );
	const WebGpuDevice::DepthUpload *upload = m_D.DepthUploadFor( format );
	const std::uint32_t region[4] = {
	    static_cast<std::uint32_t>( copy.bufferOffset / 4 ), copy.width, copy.x, copy.y };
	const WGPUBuffer uniform =
	    Temporary( sizeof( region ), WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst );
	m_RegionWrites.push_back( { uniform, { region[0], region[1], region[2], region[3] } } );
	WGPUBindGroupEntry entries[2] = { WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT };
	entries[0].binding = 0;
	entries[0].buffer = b.buffer.Get();
	entries[0].size = b.allocated;
	entries[1].binding = 1;
	entries[1].buffer = uniform;
	entries[1].size = sizeof( region );
	WGPUBindGroupDescriptor group = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
	group.layout = upload->layout.Get();
	group.entryCount = 2;
	group.entries = entries;
	m_Submission.groups.emplace_back( wgpuDeviceCreateBindGroup( m_D.Native(), &group ) );
	WGPURenderPassDepthStencilAttachment depth = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
	depth.view = m_D.AttachmentView( t, copy.mip, copy.layer );
	depth.depthLoadOp = WGPULoadOp_Load;
	depth.depthStoreOp = WGPUStoreOp_Store;
	depth.depthClearValue = 0.0f; // unused, but a browser refuses webgpu.h's NaN default
	if ( HasStencil( t.desc.format ) )
	{
		depth.stencilLoadOp = WGPULoadOp_Load;
		depth.stencilStoreOp = WGPUStoreOp_Store;
	}
	WGPURenderPassDescriptor pass = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
	pass.depthStencilAttachment = &depth;
	const WGPURenderPassEncoder encoder = wgpuCommandEncoderBeginRenderPass( m_Commands, &pass );
	wgpuRenderPassEncoderSetPipeline( encoder, upload->pipeline.Get() );
	wgpuRenderPassEncoderSetBindGroup( encoder, 0, m_Submission.groups.back().Get(), 0, nullptr );
	wgpuRenderPassEncoderSetViewport( encoder, float( copy.x ), float( copy.y ),
	    float( copy.width ), float( copy.height ), 0.0f, 1.0f );
	wgpuRenderPassEncoderSetScissorRect( encoder, copy.x, copy.y, copy.width, copy.height );
	wgpuRenderPassEncoderDraw( encoder, 3, 1, 0, 0 );
	wgpuRenderPassEncoderEnd( encoder );
	wgpuRenderPassEncoderRelease( encoder );
}

bool Replay::BeginRendering( const Command &command )
{
	EndPasses();
	std::vector<WGPURenderPassColorAttachment> colors;
	for ( const ColorAttachment &attachment : command.colors )
	{
		TextureRecord &t = *m_D.ExistingTexture( attachment.texture.value );
		WGPURenderPassColorAttachment color = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
		color.view = m_D.AttachmentView( t, 0, 0 );
		color.loadOp = LoadOf( attachment.load );
		color.storeOp =
		    attachment.store == StoreOp::kStore ? WGPUStoreOp_Store : WGPUStoreOp_Discard;
		color.clearValue = {
		    attachment.clear.r, attachment.clear.g, attachment.clear.b, attachment.clear.a };
		if ( attachment.resolve.IsValid() )
			color.resolveTarget =
			    m_D.AttachmentView( *m_D.ExistingTexture( attachment.resolve.value ), 0, 0 );
		colors.push_back( color );
	}
	WGPURenderPassDepthStencilAttachment depth = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
	WGPURenderPassDescriptor pass = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
	pass.colorAttachmentCount = colors.size();
	pass.colorAttachments = colors.data();
	if ( command.depth )
	{
		TextureRecord &t = *m_D.ExistingTexture( command.depth->texture.value );
		const WGPUStoreOp store =
		    command.depth->store == StoreOp::kStore ? WGPUStoreOp_Store : WGPUStoreOp_Discard;
		depth.view = m_D.AttachmentView( t, 0, 0 );
		depth.depthLoadOp = LoadOf( command.depth->load );
		depth.depthStoreOp = store;
		depth.depthClearValue = command.depth->clearDepth;
		if ( HasStencil( t.desc.format ) )
		{
			depth.stencilLoadOp = LoadOf( command.depth->load );
			depth.stencilStoreOp = store;
			depth.stencilClearValue = 0;
		}
		pass.depthStencilAttachment = &depth;
	}
	m_Render = wgpuCommandEncoderBeginRenderPass( m_Commands, &pass );
	// The render area: attachments may be larger than the pass.
	wgpuRenderPassEncoderSetViewport(
	    m_Render, 0.0f, 0.0f, float( command.width ), float( command.height ), 0.0f, 1.0f );
	wgpuRenderPassEncoderSetScissorRect( m_Render, 0, 0, command.width, command.height );
	m_PipelineDirty = true;
	return true;
}

// Binds the pipeline layout's groups: the port's, built for the layout; the
// empty group for an unused role; group 3 with the draw constants at their
// dynamic offset.
bool Replay::BindGroups( bool compute )
{
	const PipelineRecord &p = *m_Pipeline;
	for ( std::uint32_t g = 0; g < p.groupCount; ++g )
	{
		const GroupLayout &layout = *p.groups[g];
		WGPUBindGroup group = nullptr;
		std::uint32_t offsets = 0;
		std::uint32_t offset = 0;
		if ( layout.drawConstants )
		{
			// Constants changed since the last draw take the next slot.
			if ( m_ConstantsDirty )
			{
				m_ConstantOffset = m_ConstantCursor;
				m_ConstantCursor += m_ConstantAlignment;
				std::memcpy( m_ConstantBytes.data() + m_ConstantOffset, m_Constants.data(),
				    kMaxDrawConstantBytes );
				m_ConstantsDirty = false;
			}
			BindGroupRecord *port = m_Groups[g] ? m_D.ExistingBindGroup( m_Groups[g] ) : nullptr;
			const auto key = std::make_pair( m_Groups[g], &layout );
			if ( const auto found = m_ConstantGroups.find( key ); found != m_ConstantGroups.end() )
				group = found->second;
			else
				group = m_ConstantGroups[key] =
				    m_D.ConstantsGroup( port, layout, m_ConstantBuffer, m_Submission );
			offsets = 1;
			offset = static_cast<std::uint32_t>( m_ConstantOffset );
		}
		else if ( layout.entries.empty() )
		{
			group = m_D.EmptyGroup();
		}
		else if ( BindGroupRecord *port = m_D.ExistingBindGroup( m_Groups[g] ) )
		{
			group = m_D.BuiltGroup( *port, layout );
		}
		if ( !group )
		{
			// Name what is missing: no group bound, or the first binding the
			// WGSL uses that the bound group lacks (or holds a released resource).
			const BindGroupRecord *port = m_D.ExistingBindGroup( m_Groups[g] );
			std::uint32_t missing = ~0u;
			for ( const BindingLine &line : layout.entries )
			{
				if ( !port || std::none_of( port->entries.begin(), port->entries.end(),
				                  [&]( const BindGroupEntry &e ) { return e.binding == line.source; } ) )
				{
					missing = line.binding;
					break;
				}
			}
			std::fprintf( stderr,
			    "render.device.webgpu: group %u lacks a binding its pipeline's WGSL uses (%s %u, "
			    "pipeline %s)\n",
			    g, port ? "binding" : "no group bound; first binding", missing,
			    m_Pipeline ? m_Pipeline->name.c_str() : "?" );
			return false;
		}
		if ( compute )
			wgpuComputePassEncoderSetBindGroup( m_Compute, g, group, offsets, &offset );
		else
			wgpuRenderPassEncoderSetBindGroup( m_Render, g, group, offsets, &offset );
	}
	return true;
}

bool Replay::PrepareDraw()
{
	const PipelineRecord &p = *m_Pipeline;
	if ( m_PipelineDirty )
	{
		wgpuRenderPassEncoderSetPipeline( m_Render, p.render.Get() );
		wgpuRenderPassEncoderSetStencilReference( m_Render, p.stencilReference );
		m_PipelineDirty = false;
	}
	for ( std::uint32_t slot = 0; slot < p.vertexBuffers; ++slot )
	{
		const BufferRecord &b = *m_D.ExistingBuffer( m_VertexBuffers[slot].buffer );
		wgpuRenderPassEncoderSetVertexBuffer( m_Render, slot, b.buffer.Get(),
		    m_VertexBuffers[slot].offset, b.allocated - m_VertexBuffers[slot].offset );
	}
	if ( m_Index.buffer )
	{
		const BufferRecord &b = *m_D.ExistingBuffer( m_Index.buffer );
		wgpuRenderPassEncoderSetIndexBuffer( m_Render, b.buffer.Get(),
		    m_IndexFormat == IndexFormat::kUint32 ? WGPUIndexFormat_Uint32 : WGPUIndexFormat_Uint16,
		    m_Index.offset, b.allocated - m_Index.offset );
	}
	return BindGroups( false );
}

CommandBuffer WebGpuDevice::Execute(
    std::vector<RecordingEncoder *> &encoders, Submission &submission )
{
	Replay replay( *this, submission );
	return replay.Run( encoders );
}

} // namespace render::device::webgpu
