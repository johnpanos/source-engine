//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device: recorded command lists shared by the adapters that
//			replay them; see public/render/device/recording.h.
//
//=============================================================================//

#include "render/device/recording.h"

#include "render/device/validation.h"

#include <algorithm>

namespace render::device::recording
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

// Encoder -----------------------------------------------------------------------

RecordingEncoder::~RecordingEncoder()
{
	if ( !m_Submitted )
		m_Stager.AbandonUploads( m_RingAllocations );
}

void RecordingEncoder::TransitionTexture(
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

void RecordingEncoder::TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after )
{
	Command command;
	command.op = Op::kTransitionBuffer;
	command.a = buffer.value;
	command.before = before;
	command.after = after;
	Push( std::move( command ) );
}

void RecordingEncoder::ClearTexture(
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

void RecordingEncoder::WriteBuffer(
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
		m_Stager.StageUpload( *this, command, bytes );
	Push( std::move( command ) );
}

void RecordingEncoder::CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyBuffer;
	command.a = source.value;
	command.b = destination.value;
	command.copy = copy;
	Push( std::move( command ) );
}

void RecordingEncoder::CopyTextureToBuffer(
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

void RecordingEncoder::CopyBufferToTexture(
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

void RecordingEncoder::CopyTexture( TextureId source, TextureId destination, const TextureCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyTexture;
	command.a = source.value;
	command.b = destination.value;
	command.textureCopy = { 0, copy.mip, copy.layer, copy.width, copy.height, copy.x, copy.y };
	Push( std::move( command ) );
}

void RecordingEncoder::BeginRendering( const RenderingDesc &desc )
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

void RecordingEncoder::EndRendering()
{
	if ( !m_Rendering )
		m_Error = true;
	m_Rendering = false;
	Command command;
	command.op = Op::kEndRendering;
	Push( std::move( command ) );
}

void RecordingEncoder::SetPipeline( PipelineId pipeline )
{
	Command command;
	command.op = Op::kSetPipeline;
	command.a = pipeline.value;
	Push( std::move( command ) );
}

void RecordingEncoder::SetBindGroup( BindGroupRole role, BindGroupId group )
{
	Command command;
	command.op = Op::kSetBindGroup;
	command.a = group.value;
	command.slot = static_cast<std::uint32_t>( role );
	if ( command.slot >= kMaxBindGroups )
		m_Error = true;
	Push( std::move( command ) );
}

void RecordingEncoder::SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset )
{
	Command command;
	command.op = Op::kSetVertexBuffer;
	command.a = buffer.value;
	command.slot = slot;
	command.offset = offset;
	Push( std::move( command ) );
}

void RecordingEncoder::SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat format )
{
	Command command;
	command.op = Op::kSetIndexBuffer;
	command.a = buffer.value;
	command.offset = offset;
	command.indexFormat = format;
	Push( std::move( command ) );
}

void RecordingEncoder::SetViewport( const Viewport &viewport )
{
	Command command;
	command.op = Op::kSetViewport;
	command.viewport = viewport;
	Push( std::move( command ) );
}

void RecordingEncoder::Draw( std::uint32_t vertexCount, std::uint32_t instanceCount,
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

void RecordingEncoder::DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount,
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

// D30/D31: a the records' buffer at offset, b the count's buffer at
// copy.destinationOffset; count the draw count (maximum); first the stride.
void RecordingEncoder::DrawIndexedIndirect(
    BufferId buffer, std::uint64_t offset, std::uint32_t drawCount, std::uint32_t stride )
{
	Drawing();
	Command command;
	command.op = Op::kDrawIndexedIndirect;
	command.a = buffer.value;
	command.offset = offset;
	command.count = drawCount;
	command.first = stride;
	Push( std::move( command ) );
}

void RecordingEncoder::DrawIndexedIndirectCount( BufferId buffer, std::uint64_t offset,
    BufferId countBuffer, std::uint64_t countOffset, std::uint32_t maxDrawCount,
    std::uint32_t stride )
{
	Drawing();
	Command command;
	command.op = Op::kDrawIndexedIndirectCount;
	command.a = buffer.value;
	command.b = countBuffer.value;
	command.offset = offset;
	command.copy.destinationOffset = countOffset;
	command.count = maxDrawCount;
	command.first = stride;
	Push( std::move( command ) );
}

void RecordingEncoder::Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z )
{
	NotRendering();
	Command command;
	command.op = Op::kDispatch;
	command.count = x;
	command.first = y;
	command.firstInstance = z;
	Push( std::move( command ) );
}

void RecordingEncoder::SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes )
{
	Command command;
	command.op = Op::kSetDrawConstants;
	command.offset = offset;
	command.bytes.assign( bytes.begin(), bytes.end() );
	Push( std::move( command ) );
}

void RecordingEncoder::BeginLabel( std::string_view label )
{
	++m_Labels;
	Command command;
	command.op = Op::kBeginLabel;
	command.bytes.assign( reinterpret_cast<const std::byte *>( label.data() ),
	    reinterpret_cast<const std::byte *>( label.data() ) + label.size() );
	Push( std::move( command ) );
}

void RecordingEncoder::EndLabel()
{
	if ( m_Labels == 0 )
		m_Error = true;
	else
		--m_Labels;
	Command command;
	command.op = Op::kEndLabel;
	Push( std::move( command ) );
}

void RecordingEncoder::WriteTimestamp( BufferId buffer, std::uint64_t offset )
{
	Command command;
	command.op = Op::kWriteTimestamp;
	command.a = buffer.value;
	command.offset = offset;
	Push( std::move( command ) );
}

// Validation --------------------------------------------------------------------

std::uint32_t TextureView::Width( std::uint32_t mip ) const
{
	return std::max<std::uint32_t>( 1u, desc.width >> mip );
}

std::uint32_t TextureView::Height( std::uint32_t mip ) const
{
	return std::max<std::uint32_t>( 1u, desc.height >> mip );
}

namespace
{

struct ValidationState
{
	std::optional<PipelineView> pipeline;
	std::array<std::optional<BindGroupLayoutId>, kMaxBindGroups> groups{};
	std::vector<Format> colors;
	Format depth = Format::kUnknown;
	std::uint32_t samples = 0;
	bool rendering = false;
	std::uint32_t vertexSlots = 0;
	bool index = false;
	DrawConstantCoverage constants; // D16
};

bool GroupsMatch( const ValidationState &state )
{
	const PipelineView &pipeline = *state.pipeline;
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		if ( !pipeline.layoutHasBindings[role] )
			continue;
		if ( !state.groups[role] || *state.groups[role] != pipeline.layouts[role] )
			return false;
	}
	return true;
}

bool ValidateDraw( const ValidationState &state, bool indexed )
{
	if ( !state.pipeline || state.pipeline->kind != PipelineKind::kGraphics || !state.rendering )
		return false;
	const PipelineView &pipeline = *state.pipeline;
	if ( !std::equal( pipeline.colorFormats.begin(), pipeline.colorFormats.end(),
	         state.colors.begin(), state.colors.end() ) ||
	     pipeline.depthFormat != state.depth || pipeline.sampleCount != state.samples )
		return false;
	for ( std::uint32_t slot = 0; slot < pipeline.vertexBuffers; ++slot )
	{
		if ( !( state.vertexSlots & ( 1u << slot ) ) )
			return false;
	}
	if ( indexed && !state.index )
		return false;
	return GroupsMatch( state );
}

} // namespace

bool Validate( const std::vector<Command> &commands,
    std::unordered_map<std::uint64_t, ResourceUsage> &states, const IRecordedResources &resources )
{
	auto state = [&]( std::uint64_t id, ResourceUsage current ) -> ResourceUsage &
	{ return states.try_emplace( id, current ).first->second; };
	auto texture = [&]( std::uint64_t id, ResourceUsage required ) -> std::optional<TextureView>
	{
		std::optional<TextureView> t = resources.Texture( id );
		if ( !t || state( id, t->usage ) != required )
			return std::nullopt;
		return t;
	};
	auto buffer = [&]( std::uint64_t id, ResourceUsage required ) -> std::optional<BufferView>
	{
		std::optional<BufferView> b = resources.Buffer( id );
		if ( !b || state( id, b->usage ) != required )
			return std::nullopt;
		return b;
	};
	auto copyFits = [&]( const TextureView &t, const BufferView &b, const TextureBufferCopy &copy )
	{
		if ( copy.mip >= t.desc.mipLevels || copy.layer >= t.layers || copy.width == 0 ||
		     copy.height == 0 || t.desc.sampleCount != 1 || !resources.CanCopyWithBuffer( t ) ||
		     !CopyRegionAligned( t.desc.format, t.Width( copy.mip ), t.Height( copy.mip ), copy.x,
		         copy.y, copy.width, copy.height, copy.bufferOffset ) )
			return false;
		const std::uint64_t bytes = RegionBytes( t.desc.format, copy.width, copy.height );
		return copy.bufferOffset <= b.desc.size && bytes <= b.desc.size - copy.bufferOffset;
	};

	ValidationState v;
	for ( const Command &command : commands )
	{
		switch ( command.op )
		{
		case Op::kTransitionTexture:
		{
			const std::optional<TextureView> t = resources.Texture( command.a );
			if ( !t || !t->desc.usages.Has( command.after ) )
				return false;
			ResourceUsage &current = state( command.a, t->usage );
			if ( command.before != ResourceUsage::kUndefined && command.before != current )
				return false;
			current = command.after;
			break;
		}
		case Op::kTransitionBuffer:
		{
			const std::optional<BufferView> b = resources.Buffer( command.a );
			if ( !b || !b->desc.usages.Has( command.after ) )
				return false;
			ResourceUsage &current = state( command.a, b->usage );
			if ( command.before != ResourceUsage::kUndefined && command.before != current )
				return false;
			current = command.after;
			break;
		}
		case Op::kClearTexture:
		{
			// A block-compressed texture is written by copies only (D19).
			const std::optional<TextureView> t =
			    texture( command.a, ResourceUsage::kCopyDestination );
			if ( !t || IsBlockCompressed( t->desc.format ) || t->desc.sampleCount != 1 ||
			     !resources.CanClear( *t ) )
				return false;
			break;
		}
		case Op::kWriteBuffer:
		{
			const std::optional<BufferView> b = buffer( command.a, ResourceUsage::kCopyDestination );
			if ( !b || command.copy.destinationOffset > b->desc.size ||
			     command.copy.size > b->desc.size - command.copy.destinationOffset )
				return false;
			break;
		}
		case Op::kCopyBuffer:
		{
			const std::optional<BufferView> src = buffer( command.a, ResourceUsage::kCopySource );
			const std::optional<BufferView> dst =
			    buffer( command.b, ResourceUsage::kCopyDestination );
			if ( !src || !dst || command.copy.sourceOffset > src->desc.size ||
			     command.copy.size > src->desc.size - command.copy.sourceOffset ||
			     command.copy.destinationOffset > dst->desc.size ||
			     command.copy.size > dst->desc.size - command.copy.destinationOffset )
				return false;
			break;
		}
		case Op::kCopyTextureToBuffer:
		{
			const std::optional<TextureView> t = texture( command.a, ResourceUsage::kCopySource );
			const std::optional<BufferView> b =
			    buffer( command.b, ResourceUsage::kCopyDestination );
			if ( !t || !b || !copyFits( *t, *b, command.textureCopy ) )
				return false;
			break;
		}
		case Op::kCopyBufferToTexture:
		{
			const std::optional<BufferView> b = buffer( command.a, ResourceUsage::kCopySource );
			const std::optional<TextureView> t =
			    texture( command.b, ResourceUsage::kCopyDestination );
			if ( !t || !b || !copyFits( *t, *b, command.textureCopy ) )
				return false;
			break;
		}
		case Op::kCopyTexture:
		{
			const std::optional<TextureView> s = texture( command.a, ResourceUsage::kCopySource );
			const std::optional<TextureView> d =
			    texture( command.b, ResourceUsage::kCopyDestination );
			const TextureBufferCopy &copy = command.textureCopy;
			auto fits = [&]( const TextureView &t )
			{
				return copy.mip < t.desc.mipLevels && copy.layer < t.layers &&
				       t.desc.sampleCount == 1 && resources.CanCopyTexture( t ) &&
				       CopyRegionAligned( t.desc.format, t.Width( copy.mip ), t.Height( copy.mip ),
				           copy.x, copy.y, copy.width, copy.height, 0 );
			};
			if ( !s || !d || command.a == command.b || copy.width == 0 || copy.height == 0 ||
			     s->desc.format != d->desc.format || !fits( *s ) || !fits( *d ) )
				return false;
			break;
		}
		case Op::kBeginRendering:
		{
			v.colors.clear();
			v.depth = Format::kUnknown;
			v.samples = 0;
			auto fits = [&]( const TextureView &t )
			{
				if ( t.desc.dimension == TextureDimension::k3D || command.width > t.desc.width ||
				     command.height > t.desc.height )
					return false;
				if ( v.samples != 0 && v.samples != t.desc.sampleCount )
					return false;
				v.samples = t.desc.sampleCount;
				return true;
			};
			if ( command.colors.size() > resources.MaxColorAttachments() )
				return false;
			for ( const ColorAttachment &color : command.colors )
			{
				const std::optional<TextureView> t =
				    texture( color.texture.value, ResourceUsage::kColorAttachment );
				if ( !t || !fits( *t ) )
					return false;
				v.colors.push_back( t->desc.format );
				if ( color.resolve.IsValid() )
				{
					const std::optional<TextureView> r =
					    texture( color.resolve.value, ResourceUsage::kResolveDestination );
					if ( !r || r->desc.sampleCount != 1 || t->desc.sampleCount == 1 ||
					     r->desc.format != t->desc.format || command.width > r->desc.width ||
					     command.height > r->desc.height ||
					     r->desc.dimension == TextureDimension::k3D )
						return false;
				}
			}
			if ( command.depth )
			{
				const std::uint64_t id = command.depth->texture.value;
				std::optional<TextureView> t = texture( id, ResourceUsage::kDepthWrite );
				if ( !t )
				{
					t = texture( id, ResourceUsage::kDepthRead );
					// A read-only depth attachment cannot be cleared.
					if ( t && command.depth->load == LoadOp::kClear )
						return false;
				}
				if ( !t || !fits( *t ) )
					return false;
				v.depth = t->desc.format;
			}
			v.rendering = true;
			break;
		}
		case Op::kEndRendering:
			v.rendering = false;
			break;
		case Op::kSetPipeline:
		{
			v.pipeline = resources.Pipeline( command.a );
			if ( !v.pipeline )
				return false;
			v.constants.Bind( v.pipeline->drawConstantBytes );
			break;
		}
		case Op::kSetBindGroup:
		{
			const std::optional<BindGroupLayoutId> layout = resources.BindGroup( command.a );
			if ( !layout )
				return false;
			v.groups[command.slot] = layout;
			break;
		}
		case Op::kSetVertexBuffer:
			if ( !buffer( command.a, ResourceUsage::kVertex ) ||
			     command.slot >= resources.MaxVertexSlots() )
				return false;
			v.vertexSlots |= 1u << command.slot;
			break;
		case Op::kSetIndexBuffer:
			if ( !buffer( command.a, ResourceUsage::kIndex ) )
				return false;
			v.index = true;
			break;
		case Op::kSetDrawConstants:
			if ( !v.constants.Write( std::uint32_t( command.offset ), command.bytes.size() ) )
				return false;
			break;
		case Op::kDraw:
		case Op::kDrawIndexed:
			if ( !ValidateDraw( v, command.op == Op::kDrawIndexed ) || !v.constants.Ready() )
				return false;
			break;
		case Op::kDispatch:
			if ( !v.pipeline || v.pipeline->kind != PipelineKind::kCompute || !GroupsMatch( v ) ||
			     !v.constants.Ready() )
				return false;
			break;
		case Op::kWriteTimestamp:
		{
			// D23: kReadback memory in kCopyDestination, 8-byte aligned.
			const std::optional<BufferView> b = buffer( command.a, ResourceUsage::kCopyDestination );
			if ( !b || b->desc.memory != MemoryKind::kReadback || command.offset % 8 != 0 ||
			     command.offset > b->desc.size || b->desc.size - command.offset < 8 )
				return false;
			break;
		}
		case Op::kSetViewport:
		case Op::kBeginLabel:
		case Op::kEndLabel:
			break;
		case Op::kDrawIndexedIndirect:
		case Op::kDrawIndexedIndirectCount:
		{
			// D30/D31: records (and the count) in kIndirect buffers. The
			// capability is checked after validation (CheckSubmission).
			const std::optional<BufferView> records = buffer( command.a, ResourceUsage::kIndirect );
			if ( !ValidateDraw( v, true ) || !v.constants.Ready() || !records ||
			     !IndirectRecordsFit( records->desc.size, command.offset, command.count,
			         command.first ) )
				return false;
			if ( command.op == Op::kDrawIndexedIndirectCount )
			{
				const std::uint64_t at = command.copy.destinationOffset;
				const std::optional<BufferView> count =
				    buffer( command.b, ResourceUsage::kIndirect );
				if ( !count || at % 4 != 0 || at > count->desc.size || count->desc.size - at < 4 )
					return false;
			}
			break;
		}
		}
	}
	return true;
}

std::optional<DeviceStatus> CheckSubmission( std::span<const RecordingEncoder *const> encoders,
    CapabilitySet capabilities, const std::unordered_map<std::uint64_t, ResourceUsage> &states )
{
	for ( const RecordingEncoder *encoder : encoders )
	{
		for ( const Command &command : encoder->Commands() )
		{
			// D30/D31: refused where unclaimed.
			if ( ( command.op == Op::kDrawIndexedIndirect &&
			         !capabilities.Has( Capability::kMultiDrawIndirect ) ) ||
			     ( command.op == Op::kDrawIndexedIndirectCount &&
			         !capabilities.Has( Capability::kDrawIndirectCount ) ) )
				return DeviceStatus::kUnsupported;
			if ( command.op != Op::kWriteTimestamp )
				continue;
			if ( !capabilities.Has( Capability::kTimestamps ) )
				return DeviceStatus::kUnsupported;
			const auto found = states.find( command.a );
			if ( found == states.end() || found->second != ResourceUsage::kCopyDestination )
				return DeviceStatus::kInvalidState;
		}
	}
	return std::nullopt;
}

} // namespace render::device::recording
