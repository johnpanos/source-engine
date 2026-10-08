//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan (RFC 0016 K1): encoders, submission-time
//			validation and translation into a VkCommandBuffer.
//
//			Encoders record a command list, as the null adapter's do. Submit
//			first validates every encoder in order against the device's usage
//			state (the null adapter's rules, plus what Vulkan needs to avoid
//			undefined behavior: a draw names a pipeline that matches the
//			rendering attachments, bound vertex and index buffers and bind
//			groups of the pipeline's layouts, attachments large enough for the
//			render area). A rejected submission records nothing and leaves the
//			tracked state unchanged. An accepted one is translated on the
//			submitting thread:
//
//			- each transition becomes one vkCmdPipelineBarrier2 from the
//			  resource's tracked usage to the new one (scopes from ScopeOf); a
//			  transition from kUndefined discards the contents (UNDEFINED old
//			  layout) but still waits for the tracked usage;
//			- the port runs commands as if one after another, so a resource
//			  written in a usage and accessed again in it without a transition
//			  (two uploads to one buffer, two passes on one target, two
//			  dispatches on one storage buffer) gets a barrier within that
//			  usage first;
//			- clip Y up and a top-left origin come from a negative-height
//			  viewport; depth 0 to 1 is Vulkan's own;
//			- every submission ends with a barrier that makes device writes
//			  visible to the host, for ReadBuffer.
//
//=============================================================================//

#include "vulkan_device.h"

#include <algorithm>
#include <cstring>
#include <unordered_set>

namespace render::device::vulkan
{

// Encoder ----------------------------------------------------------------------

VulkanEncoder::VulkanEncoder( VulkanDevice &device, QueueKind queue )
    : m_Device( device ), m_Queue( queue ), m_Commands( device.TakeCommandList() )
{
}

VulkanEncoder::~VulkanEncoder()
{
	if ( !m_Submitted )
	{
		m_Device.AbandonUploads( *this );
		for ( const auto &payload : m_Computes )
			payload->Aborted();
	}
	m_Device.ReturnCommandList( std::move( m_Commands ) );
}

std::vector<Command> VulkanDevice::TakeCommandList()
{
	std::lock_guard<std::mutex> lock( m_CommandListLock );
	if ( m_CommandLists.empty() )
		return {};
	std::vector<Command> commands = std::move( m_CommandLists.back() );
	m_CommandLists.pop_back();
	return commands;
}

void VulkanDevice::ReturnCommandList( std::vector<Command> commands )
{
	// Trivially destructible commands: clearing keeps the capacity for free.
	commands.clear();
	constexpr std::size_t kKept = 32;
	std::lock_guard<std::mutex> lock( m_CommandListLock );
	if ( m_CommandLists.size() < kKept )
		m_CommandLists.push_back( std::move( commands ) );
}

std::span<const std::byte> VulkanEncoder::KeepBytes( std::span<const std::byte> bytes )
{
	if ( bytes.empty() )
		return {};
	// Chunks never reallocate (each is filled within its reserved capacity),
	// so earlier commands' spans stay valid.
	constexpr std::size_t kChunk = 64 * 1024;
	if ( m_ByteChunks.empty() ||
	     m_ByteChunks.back().capacity() - m_ByteChunks.back().size() < bytes.size() )
	{
		m_ByteChunks.emplace_back();
		m_ByteChunks.back().reserve( std::max( kChunk, bytes.size() ) );
	}
	std::vector<std::byte> &chunk = m_ByteChunks.back();
	const std::size_t at = chunk.size();
	chunk.insert( chunk.end(), bytes.begin(), bytes.end() );
	return std::span<const std::byte>( chunk.data() + at, bytes.size() );
}

void VulkanEncoder::Compute( std::shared_ptr<ComputeInterop> payload )
{
	NotRendering();
	Command command;
	command.op = Op::kComputeInterop;
	command.compute = payload.get();
	if ( payload )
		m_Computes.push_back( std::move( payload ) );
	Push( command );
}

void VulkanEncoder::TransitionTexture(
    TextureId texture, ResourceUsage before, ResourceUsage after, const SubresourceRange &range )
{
	Command command;
	command.op = Op::kTransitionTexture;
	command.a = texture.value;
	command.before = before;
	command.after = after;
	command.range = range;
	Push( command );
}

void VulkanEncoder::TransitionBuffer( BufferId buffer, ResourceUsage before, ResourceUsage after )
{
	Command command;
	command.op = Op::kTransitionBuffer;
	command.a = buffer.value;
	command.before = before;
	command.after = after;
	Push( command );
}

void VulkanEncoder::ClearTexture(
    TextureId texture, const ClearColor &color, const SubresourceRange &range )
{
	NotRendering();
	Command command;
	command.op = Op::kClearTexture;
	command.a = texture.value;
	command.color = color;
	command.range = range;
	Push( command );
}

void VulkanEncoder::WriteBuffer(
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
	Push( command );
}

void VulkanEncoder::CopyBuffer( BufferId source, BufferId destination, const BufferCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyBuffer;
	command.a = source.value;
	command.b = destination.value;
	command.copy = copy;
	Push( command );
}

void VulkanEncoder::CopyTextureToBuffer(
    TextureId source, BufferId destination, const TextureBufferCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyTextureToBuffer;
	command.a = source.value;
	command.b = destination.value;
	command.textureCopy = copy;
	Push( command );
}

void VulkanEncoder::CopyBufferToTexture(
    BufferId source, TextureId destination, const TextureBufferCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyBufferToTexture;
	command.a = source.value;
	command.b = destination.value;
	command.textureCopy = copy;
	Push( command );
}

void VulkanEncoder::CopyTexture(
    TextureId source, TextureId destination, const TextureCopy &copy )
{
	NotRendering();
	Command command;
	command.op = Op::kCopyTexture;
	command.a = source.value;
	command.b = destination.value;
	command.textureCopy = { 0, copy.mip, copy.layer, copy.width, copy.height, copy.x, copy.y };
	Push( command );
}

void VulkanEncoder::BeginRendering( const RenderingDesc &desc )
{
	NotRendering();
	m_Rendering = true;
	Command command;
	command.op = Op::kBeginRendering;
	RenderingPayload &rendering = m_Renderings.emplace_back();
	rendering.colors.assign( desc.colors.begin(), desc.colors.end() );
	rendering.depth = desc.depth;
	command.rendering = &rendering;
	command.width = desc.width;
	command.height = desc.height;
	if ( desc.width == 0 || desc.height == 0 || ( desc.colors.empty() && !desc.depth ) )
		m_Error = true;
	Push( command );
}

void VulkanEncoder::EndRendering()
{
	if ( !m_Rendering )
		m_Error = true;
	m_Rendering = false;
	Command command;
	command.op = Op::kEndRendering;
	Push( command );
}

void VulkanEncoder::SetPipeline( PipelineId pipeline )
{
	Command command;
	command.op = Op::kSetPipeline;
	command.a = pipeline.value;
	Push( command );
}

void VulkanEncoder::SetBindGroup( BindGroupRole role, BindGroupId group )
{
	Command command;
	command.op = Op::kSetBindGroup;
	command.a = group.value;
	command.slot = static_cast<std::uint32_t>( role );
	if ( command.slot >= kMaxBindGroups )
		m_Error = true;
	Push( command );
}

void VulkanEncoder::SetVertexBuffer( std::uint32_t slot, BufferId buffer, std::uint64_t offset )
{
	Command command;
	command.op = Op::kSetVertexBuffer;
	command.a = buffer.value;
	command.slot = slot;
	command.offset = offset;
	Push( command );
}

void VulkanEncoder::SetIndexBuffer( BufferId buffer, std::uint64_t offset, IndexFormat format )
{
	Command command;
	command.op = Op::kSetIndexBuffer;
	command.a = buffer.value;
	command.offset = offset;
	command.indexFormat = format;
	Push( command );
}

void VulkanEncoder::SetViewport( const Viewport &viewport )
{
	Command command;
	command.op = Op::kSetViewport;
	command.viewport = viewport;
	if ( !( viewport.width > 0.0f ) || !( viewport.height > 0.0f ) )
		m_Error = true;
	Push( command );
}

void VulkanEncoder::Draw( std::uint32_t vertexCount, std::uint32_t instanceCount,
    std::uint32_t firstVertex, std::uint32_t firstInstance )
{
	Drawing();
	Command command;
	command.op = Op::kDraw;
	command.params[0] = vertexCount;
	command.params[1] = instanceCount;
	command.params[2] = firstVertex;
	command.params[3] = firstInstance;
	Push( command );
}

void VulkanEncoder::DrawIndexed( std::uint32_t indexCount, std::uint32_t instanceCount,
    std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance )
{
	Drawing();
	Command command;
	command.op = Op::kDrawIndexed;
	command.params[0] = indexCount;
	command.params[1] = instanceCount;
	command.params[2] = firstIndex;
	command.params[3] = firstInstance;
	command.vertexOffset = vertexOffset;
	Push( command );
}

void VulkanEncoder::DrawIndexedIndirect(
    BufferId buffer, std::uint64_t offset, std::uint32_t drawCount, std::uint32_t stride )
{
	Drawing();
	Command command;
	command.op = Op::kDrawIndexedIndirect;
	command.a = buffer.value;
	command.offset = offset;
	command.params[0] = drawCount;
	command.params[1] = stride;
	Push( command );
}

void VulkanEncoder::DrawIndexedIndirectCount( BufferId buffer, std::uint64_t offset,
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
	command.params[0] = maxDrawCount;
	command.params[1] = stride;
	Push( command );
}

void VulkanEncoder::Native( void ( *record )( void *, VkCommandBuffer ), void *user )
{
	// Host work runs outside the port's rendering and outside any section.
	if ( m_Rendering || !record || m_InSection )
		m_Error = true;
	Command command;
	command.op = Op::kNative;
	command.native = record;
	command.nativeUser = user;
	Push( command );
}

void VulkanEncoder::BeginSection()
{
	// A section follows its host work, or the host work's previous section.
	const bool follows = !m_Commands.empty() && ( m_Commands.back().op == Op::kNative ||
	                                                m_Commands.back().op == Op::kSectionEnd );
	if ( m_Rendering || m_InSection || !follows )
		m_Error = true;
	m_InSection = true;
	m_SectionLabels = m_Labels;
	Command command;
	command.op = Op::kSectionBegin;
	Push( command );
}

void VulkanEncoder::EndSection()
{
	// Its labels close inside it: host work runs between sections.
	if ( m_Rendering || !m_InSection || m_Labels != m_SectionLabels )
		m_Error = true;
	m_InSection = false;
	Command command;
	command.op = Op::kSectionEnd;
	Push( command );
}

void VulkanEncoder::AddWait( VkSemaphore semaphore, VkPipelineStageFlags2 stage )
{
	VkSemaphoreSubmitInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	info.semaphore = semaphore;
	info.stageMask = stage;
	m_Waits.push_back( info );
}

void VulkanEncoder::AddSignal( VkSemaphore semaphore )
{
	VkSemaphoreSubmitInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	info.semaphore = semaphore;
	info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	m_Signals.push_back( info );
}

void VulkanEncoder::SetDrawConstants( std::uint32_t offset, std::span<const std::byte> bytes )
{
	Command command;
	command.op = Op::kSetDrawConstants;
	command.offset = offset;
	command.bytes = KeepBytes( bytes );
	Push( command );
}

void VulkanEncoder::Dispatch( std::uint32_t x, std::uint32_t y, std::uint32_t z )
{
	NotRendering();
	Command command;
	command.op = Op::kDispatch;
	command.params[0] = x;
	command.params[1] = y;
	command.params[2] = z;
	Push( command );
}

void VulkanEncoder::BeginLabel( std::string_view label )
{
	++m_Labels;
	Command command;
	command.op = Op::kBeginLabel;
	command.label = &m_LabelText.emplace_back( label );
	Push( command );
}

void VulkanEncoder::WriteTimestamp( BufferId buffer, std::uint64_t offset )
{
	Command command;
	command.op = Op::kWriteTimestamp;
	command.a = buffer.value;
	command.offset = offset;
	Push( command );
}

void VulkanEncoder::BeginOcclusionQuery( BufferId buffer, std::uint64_t offset )
{
	Command command;
	command.op = Op::kBeginOcclusionQuery;
	command.a = buffer.value;
	command.offset = offset;
	Push( command );
}

void VulkanEncoder::EndOcclusionQuery()
{
	Command command;
	command.op = Op::kEndOcclusionQuery;
	Push( command );
}

void VulkanEncoder::EndLabel()
{
	if ( m_Labels == 0 )
		m_Error = true;
	else
		--m_Labels;
	Command command;
	command.op = Op::kEndLabel;
	Push( command );
}

// Uploads ----------------------------------------------------------------------

void VulkanDevice::StageUpload(
    VulkanEncoder &encoder, Command &command, std::span<const std::byte> bytes )
{
	// The ring retires by the graphics timeline; compute-queue uploads take
	// their own staging buffer, released with their submission (S8).
	const bool computeQueue = encoder.Queue() == QueueKind::kCompute;
	if ( const auto allocation = computeQueue
	                                 ? std::nullopt
	                                 : m_Ring.Allocate( bytes.size(), CompletedValue() ) )
	{
		std::memcpy( m_Ring.Data( allocation->first ), bytes.data(), bytes.size() );
		command.ringOffset = allocation->first;
		encoder.RingAllocations().push_back( allocation->second );
		return;
	}
	// The ring is full: this upload gets its own staging buffer, released
	// behind its submission's token. Never an overwrite, never a wait.
	if ( !computeQueue )
		m_DeferredUploads.fetch_add( 1 );
	auto staging = CreateHostBuffer(
	    bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, DeviceOperation::kSubmit );
	if ( !staging )
	{
		encoder.SetError();
		return;
	}
	std::memcpy( staging.Value().mapped, bytes.data(), bytes.size() );
	command.staging = staging.Value().buffer;
	encoder.Staging().push_back( staging.Value() );
}

void VulkanDevice::AbandonUploads( VulkanEncoder &encoder )
{
	for ( std::uint64_t id : encoder.RingAllocations() )
		m_Ring.Abandon( id );
	encoder.RingAllocations().clear();
	for ( HostBuffer &staging : encoder.Staging() )
		DestroyHostBuffer( staging );
	encoder.Staging().clear();
}

// Validation -------------------------------------------------------------------

bool VulkanDevice::GroupsMatch( const ValidationState &state ) const
{
	const PipelineRecord *pipeline = state.pipeline;
	for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
	{
		if ( !pipeline->layoutHasBindings[role] )
			continue;
		const auto group = m_BindGroups.find( state.groups[role] );
		if ( group == m_BindGroups.end() || group->second.released ||
		     group->second.layout != pipeline->layouts[role] )
			return false;
	}
	return true;
}

bool VulkanDevice::ValidateDraw( const ValidationState &state, bool indexed ) const
{
	const PipelineRecord *pipeline = state.pipeline;
	if ( !pipeline || pipeline->kind != PipelineKind::kGraphics || !state.rendering )
		return false;
	if ( pipeline->colorFormats != state.colors || pipeline->depthFormat != state.depth ||
	     pipeline->sampleCount != state.samples )
		return false;
	for ( std::uint32_t slot = 0; slot < pipeline->vertexBuffers; ++slot )
	{
		if ( slot >= 64 || !( state.vertexSlots & ( std::uint64_t( 1 ) << slot ) ) )
			return false;
	}
	if ( indexed && !state.index )
		return false;
	return GroupsMatch( state );
}

bool VulkanDevice::ImportsAtHome( const std::unordered_map<std::uint64_t, ResourceUsage> &states )
{
	// A read-only import has one usage, its home: only a writable one can leave it.
	for ( std::uint64_t id : m_WritableImports )
	{
		const auto found = states.find( id );
		const TextureRecord *texture = LiveTexture( id );
		if ( found != states.end() && texture && found->second != texture->home )
			return false;
	}
	return true;
}

bool VulkanDevice::Validate(
    const std::vector<Command> &commands, std::unordered_map<std::uint64_t, ResourceUsage> &states )
{
	auto state = [&]( std::uint64_t id, ResourceUsage current ) -> ResourceUsage &
	{
		return states.try_emplace( id, current ).first->second;
	};
	auto texture = [&]( std::uint64_t id, ResourceUsage required ) -> TextureRecord *
	{
		TextureRecord *t = LiveTexture( id );
		return t && state( id, t->track.usage ) == required ? t : nullptr;
	};
	auto buffer = [&]( std::uint64_t id, ResourceUsage required ) -> BufferRecord *
	{
		BufferRecord *b = LiveBuffer( id );
		return b && state( id, b->track.usage ) == required ? b : nullptr;
	};
	auto copyFits =
	    []( const TextureRecord &t, const BufferRecord &b, const TextureBufferCopy &copy )
	{
		if ( copy.mip >= t.desc.mipLevels || copy.layer >= t.layers || copy.width == 0 ||
		     copy.height == 0 || t.desc.sampleCount != 1 ||
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
			TextureRecord *t = LiveTexture( command.a );
			if ( !t || !t->desc.usages.Has( command.after ) )
				return false;
			ResourceUsage &current = state( command.a, t->track.usage );
			if ( command.before != ResourceUsage::kUndefined && command.before != current )
				return false;
			current = command.after;
			break;
		}
		case Op::kTransitionBuffer:
		{
			BufferRecord *b = LiveBuffer( command.a );
			if ( !b || !b->desc.usages.Has( command.after ) )
				return false;
			ResourceUsage &current = state( command.a, b->track.usage );
			if ( command.before != ResourceUsage::kUndefined && command.before != current )
				return false;
			current = command.after;
			break;
		}
		case Op::kClearTexture:
		{
			// A block-compressed texture is written by copies only (D19).
			const TextureRecord *t = texture( command.a, ResourceUsage::kCopyDestination );
			if ( !t || IsBlockCompressed( t->desc.format ) )
				return false;
			break;
		}
		case Op::kWriteBuffer:
		{
			BufferRecord *b = buffer( command.a, ResourceUsage::kCopyDestination );
			if ( !b || command.copy.destinationOffset > b->desc.size ||
			     command.copy.size > b->desc.size - command.copy.destinationOffset )
				return false;
			break;
		}
		case Op::kCopyBuffer:
		{
			BufferRecord *src = buffer( command.a, ResourceUsage::kCopySource );
			BufferRecord *dst = buffer( command.b, ResourceUsage::kCopyDestination );
			if ( !src || !dst || command.copy.sourceOffset > src->desc.size ||
			     command.copy.size > src->desc.size - command.copy.sourceOffset ||
			     command.copy.destinationOffset > dst->desc.size ||
			     command.copy.size > dst->desc.size - command.copy.destinationOffset )
				return false;
			break;
		}
		case Op::kCopyTextureToBuffer:
		{
			TextureRecord *t = texture( command.a, ResourceUsage::kCopySource );
			BufferRecord *b = buffer( command.b, ResourceUsage::kCopyDestination );
			if ( !t || !b || !copyFits( *t, *b, command.textureCopy ) )
				return false;
			break;
		}
		case Op::kCopyBufferToTexture:
		{
			BufferRecord *b = buffer( command.a, ResourceUsage::kCopySource );
			TextureRecord *t = texture( command.b, ResourceUsage::kCopyDestination );
			if ( !t || !b || !copyFits( *t, *b, command.textureCopy ) )
				return false;
			break;
		}
		case Op::kCopyTexture:
		{
			TextureRecord *s = texture( command.a, ResourceUsage::kCopySource );
			TextureRecord *d = texture( command.b, ResourceUsage::kCopyDestination );
			const TextureBufferCopy &copy = command.textureCopy;
			auto fits = [&copy]( const TextureRecord &t )
			{
				return copy.mip < t.desc.mipLevels && copy.layer < t.layers &&
				       t.desc.sampleCount == 1 &&
				       CopyRegionAligned( t.desc.format, t.Width( copy.mip ), t.Height( copy.mip ),
				           copy.x, copy.y, copy.width, copy.height, 0 );
			};
			if ( !s || !d || s == d || copy.width == 0 || copy.height == 0 ||
			     s->desc.format != d->desc.format || !fits( *s ) || !fits( *d ) )
				return false;
			break;
		}
		case Op::kBeginRendering:
		{
			v.colors.clear();
			v.depth = Format::kUnknown;
			v.samples = 0;
			auto fits = [&]( const TextureRecord &t )
			{
				if ( t.attachmentView == VK_NULL_HANDLE || command.width > t.desc.width ||
				     command.height > t.desc.height )
					return false;
				if ( v.samples != 0 && v.samples != t.desc.sampleCount )
					return false;
				v.samples = t.desc.sampleCount;
				return true;
			};
			for ( const ColorAttachment &color : command.rendering->colors )
			{
				TextureRecord *t = texture( color.texture.value, ResourceUsage::kColorAttachment );
				if ( !t || !fits( *t ) )
					return false;
				v.colors.push_back( t->desc.format );
				if ( color.resolve.IsValid() )
				{
					TextureRecord *r =
					    texture( color.resolve.value, ResourceUsage::kResolveDestination );
					if ( !r || r->attachmentView == VK_NULL_HANDLE || r->desc.sampleCount != 1 ||
					     t->desc.sampleCount == 1 || r->desc.format != t->desc.format ||
					     command.width > r->desc.width || command.height > r->desc.height )
						return false;
				}
			}
			if ( const auto &depth = command.rendering->depth )
			{
				const std::uint64_t id = depth->texture.value;
				TextureRecord *t = texture( id, ResourceUsage::kDepthWrite );
				if ( !t )
				{
					t = texture( id, ResourceUsage::kDepthRead );
					// A read-only depth attachment cannot be cleared.
					if ( t && depth->load == LoadOp::kClear )
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
			// D43: a query ends inside the rendering scope it began in.
			if ( v.occlusionOpen )
				return false;
			v.rendering = false;
			break;
		case Op::kSetPipeline:
		{
			const auto pipeline = m_Pipelines.find( command.a );
			if ( pipeline == m_Pipelines.end() || pipeline->second.released )
				return false;
			v.pipeline = &pipeline->second;
			v.constants.Bind( pipeline->second.drawConstantBytes );
			break;
		}
		case Op::kSetBindGroup:
		{
			const auto group = m_BindGroups.find( command.a );
			if ( group == m_BindGroups.end() || group->second.released )
				return false;
			// A group whose resources were released would read freed memory.
			for ( const ResourceId &resource : group->second.resources )
			{
				const bool live = resource.kind == ResourceKind::kBuffer
				                      ? LiveBuffer( resource.value ) != nullptr
				                  : resource.kind == ResourceKind::kTexture
				                      ? LiveTexture( resource.value ) != nullptr
				                      : Live( m_Samplers, resource.value );
				if ( !live )
					return false;
			}
			v.groups[command.slot] = command.a;
			break;
		}
		case Op::kSetVertexBuffer:
			if ( !buffer( command.a, ResourceUsage::kVertex ) || command.slot >= 64 ||
			     command.slot >= m_Properties.limits.maxVertexInputBindings )
				return false;
			v.vertexSlots |= std::uint64_t( 1 ) << command.slot;
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
		case Op::kComputeInterop:
			if ( v.rendering || !command.compute )
				return false;
			for ( const auto &image : command.compute->images )
			{
				if ( !LiveTexture( image.id.value ) )
					return false;
				auto found = states.find( image.id.value );
				const auto usage = found == states.end()
				                       ? LiveTexture( image.id.value )->track.usage
				                       : found->second;
				if ( usage != image.usage )
					return false;
			}
			v.pipeline = nullptr;
			v.groups.fill( 0 );
			v.constants.Bind( 0 );
			break;
		case Op::kNative:
		case Op::kSectionBegin:
		case Op::kSectionEnd:
			// Host work leaves nothing bound for the port, and finds each
			// imported texture in its home usage; a section runs between host
			// work (its commands are validated where they are recorded).
			if ( v.rendering || !ImportsAtHome( states ) )
				return false;
			v.pipeline = nullptr;
			v.groups.fill( 0 );
			v.constants.Bind( 0 );
			break;
		case Op::kDraw:
		case Op::kDrawIndexed:
			if ( !ValidateDraw( v, command.op == Op::kDrawIndexed ) || !v.constants.Ready() )
				return false;
			break;
		case Op::kDrawIndexedIndirect:
		case Op::kDrawIndexedIndirectCount:
		{
			// D30/D31: records (and the count) in kIndirect buffers. The
			// capability is checked after validation (kUnsupported).
			const BufferRecord *records = buffer( command.a, ResourceUsage::kIndirect );
			if ( !ValidateDraw( v, true ) || !v.constants.Ready() || !records ||
			     !IndirectRecordsFit(
			         records->desc.size, command.offset, command.params[0], command.params[1] ) )
				return false;
			if ( command.op == Op::kDrawIndexedIndirectCount )
			{
				const std::uint64_t at = command.copy.destinationOffset;
				const BufferRecord *count = buffer( command.b, ResourceUsage::kIndirect );
				if ( !count || at % 4 != 0 || at > count->desc.size ||
				     count->desc.size - at < 4 )
					return false;
			}
			break;
		}
		case Op::kDispatch:
			if ( !v.pipeline || v.pipeline->kind != PipelineKind::kCompute || !GroupsMatch( v ) ||
			     !v.constants.Ready() )
				return false;
			break;
		case Op::kWriteTimestamp:
		{
			// D23: kReadback memory in kCopyDestination, 8-byte aligned.
			const BufferRecord *b = buffer( command.a, ResourceUsage::kCopyDestination );
			if ( !b || b->desc.memory != MemoryKind::kReadback || command.offset % 8 != 0 ||
			     command.offset > b->desc.size || b->desc.size - command.offset < 8 )
				return false;
			break;
		}
		case Op::kBeginOcclusionQuery:
		{
			// D43: inside rendering, none open, kReadback memory in
			// kCopyDestination, 8-byte aligned.
			const BufferRecord *b = buffer( command.a, ResourceUsage::kCopyDestination );
			if ( !v.rendering || v.occlusionOpen || !b || b->desc.memory != MemoryKind::kReadback ||
			     command.offset % 8 != 0 || command.offset > b->desc.size ||
			     b->desc.size - command.offset < 8 )
				return false;
			v.occlusionOpen = true;
			break;
		}
		case Op::kEndOcclusionQuery:
			if ( !v.rendering || !v.occlusionOpen )
				return false;
			v.occlusionOpen = false;
			break;
		case Op::kSetViewport:
		case Op::kBeginLabel:
		case Op::kEndLabel:
			break;
		}
	}
	return true;
}

// Translation ------------------------------------------------------------------

class Translator
{
public:
	Translator( VulkanDevice &device, VkCommandBuffer buffer, bool computeQueue = false )
	    : m_D( device ), m_Cmd( buffer ), m_ComputeQueue( computeQueue )
	{
	}

	bool Encoder( const std::vector<Command> &commands )
	{
		m_Pipeline = nullptr;
		m_Groups.fill( 0 );
		m_Viewport.reset();
		ForgetBuffers();
		for ( std::size_t i = 0; i < commands.size(); ++i )
		{
			if ( commands[i].op == Op::kNative )
				i = Native( commands, i );
			else
				Translate( commands, i );
		}
		return !m_Failed;
	}

	// Host work asks for section `index` of the record being translated:
	// it runs with every earlier section not run yet.
	bool RunSection( VkCommandBuffer cmd, std::uint32_t index )
	{
		if ( cmd != m_Cmd || !m_Commands || index >= m_Sections.size() )
			return false;
		RunSectionsThrough( index + 1 );
		return !m_Failed;
	}

	// D23: the submission's timestamps and the pool they are written to.
	void SetQueries( VkQueryPool queries ) { m_Queries = queries; }
	// D43: the submission's occlusion queries.
	void SetOcclusion( VkQueryPool occlusion ) { m_Occlusion = occlusion; }

	// The framebuffers this translation created, for its submission to own.
	std::vector<VkFramebuffer> TakeFramebuffers() { return std::move( m_Framebuffers ); }

	// Device writes become visible to the host (ReadBuffer) at completion.
	void Finish()
	{
		// The timestamps reach their buffers after every command, outside
		// rendering (Submit checked that each buffer ends in kCopyDestination).
		for ( const PendingTimestamp &timestamp : m_Timestamps )
		{
			const BufferRecord *buffer = m_D.LiveBuffer( timestamp.buffer );
			Access( timestamp.buffer, true );
			vkCmdCopyQueryPoolResults( m_Cmd, m_Queries, timestamp.query, 1, buffer->buffer,
			    timestamp.offset, sizeof( std::uint64_t ),
			    VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT );
		}
		for ( const PendingTimestamp &query : m_OcclusionResults )
		{
			const BufferRecord *buffer = m_D.LiveBuffer( query.buffer );
			Access( query.buffer, true );
			vkCmdCopyQueryPoolResults( m_Cmd, m_Occlusion, query.query, 1, buffer->buffer,
			    query.offset, sizeof( std::uint64_t ),
			    VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT );
		}
		VkMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.memoryBarrierCount = 1;
		dependency.pMemoryBarriers = &barrier;
		m_D.m_Vk.cmdPipelineBarrier2( m_Cmd, &dependency );
	}

	// The submission was accepted: its usage state becomes the device's.
	void Commit()
	{
		for ( const auto &[id, track] : m_Tracks )
		{
			if ( BufferRecord *buffer = m_D.LiveBuffer( id ) )
				buffer->track = track;
			else if ( TextureRecord *texture = m_D.LiveTexture( id ) )
				texture->track = track;
		}
	}

private:
	Track &TrackOf( std::uint64_t id )
	{
		const auto found = m_Tracks.find( id );
		if ( found != m_Tracks.end() )
			return found->second;
		Track initial;
		if ( const BufferRecord *buffer = m_D.LiveBuffer( id ) )
			initial = buffer->track;
		else if ( const TextureRecord *texture = m_D.LiveTexture( id ) )
			initial = texture->track;
		return m_Tracks.emplace( id, initial ).first->second;
	}

	// A compute-only queue cannot name graphics stages or their accesses;
	// there the barrier is the generic all-commands, memory-write to
	// memory-read-and-write one (the cross-queue order itself comes from the
	// submission's semaphore waits, S8).
	void Scopes( const UsageScope &src, const UsageScope &dst, VkPipelineStageFlags2 &srcStages,
	    VkAccessFlags2 &srcAccess, VkPipelineStageFlags2 &dstStages, VkAccessFlags2 &dstAccess ) const
	{
		srcStages = src.stages;
		srcAccess = src.access;
		dstStages = dst.stages;
		dstAccess = dst.access;
		if ( !m_ComputeQueue )
			return;
		srcStages = dstStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		srcAccess = src.access ? VK_ACCESS_2_MEMORY_WRITE_BIT : 0;
		dstAccess = dst.access ? VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT : 0;
	}

	void ImageBarrier( const TextureRecord &texture, const UsageScope &src, const UsageScope &dst,
	    VkImageLayout oldLayout )
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		Scopes( src, dst, barrier.srcStageMask, barrier.srcAccessMask, barrier.dstStageMask,
		    barrier.dstAccessMask );
		barrier.oldLayout = oldLayout;
		barrier.newLayout = dst.layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = texture.image;
		// Usage is tracked per resource, so every transition covers it all.
		barrier.subresourceRange = { BarrierAspects( texture.desc.format ), 0,
		    VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.imageMemoryBarrierCount = 1;
		dependency.pImageMemoryBarriers = &barrier;
		m_D.m_Vk.cmdPipelineBarrier2( m_Cmd, &dependency );
	}

	void BufferBarrier( const BufferRecord &buffer, const UsageScope &src, const UsageScope &dst )
	{
		VkBufferMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		Scopes( src, dst, barrier.srcStageMask, barrier.srcAccessMask, barrier.dstStageMask,
		    barrier.dstAccessMask );
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = buffer.buffer;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;
		VkDependencyInfo dependency{};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.bufferMemoryBarrierCount = 1;
		dependency.pBufferMemoryBarriers = &barrier;
		m_D.m_Vk.cmdPipelineBarrier2( m_Cmd, &dependency );
	}

	void Transition( std::uint64_t id, ResourceUsage before, ResourceUsage after )
	{
		Track &track = TrackOf( id );
		// Wait for whatever the resource was last used as; a transition from
		// kUndefined discards the contents but still orders after that use.
		const UsageScope &src = ScopeOf( track.usage );
		const UsageScope &dst = ScopeOf( after );
		if ( const BufferRecord *buffer = m_D.LiveBuffer( id ) )
		{
			BufferBarrier( *buffer, src, dst );
		}
		else if ( const TextureRecord *texture = m_D.LiveTexture( id ) )
		{
			if ( dst.layout != VK_IMAGE_LAYOUT_UNDEFINED )
				ImageBarrier( *texture, src, dst,
				    before == ResourceUsage::kUndefined ? VK_IMAGE_LAYOUT_UNDEFINED : src.layout );
		}
		track.usage = after;
		track.dirty = false;
	}

	// Host work next: the port's writes to imported textures become visible
	// in their home usage (Validate put each one there).
	void HostBoundary()
	{
		for ( std::uint64_t id : m_D.WritableImports() )
		{
			if ( m_Tracks.find( id ) != m_Tracks.end() )
				Access( id, false );
		}
	}

	// The host may have written any imported texture in its home usage.
	void HostWrote()
	{
		for ( std::uint64_t id : m_D.WritableImports() )
			TrackOf( id ).dirty = true;
	}

	// Orders an access after an unbarriered write in the same usage.
	void Access( std::uint64_t id, bool write )
	{
		Track &track = TrackOf( id );
		if ( track.dirty )
		{
			const UsageScope &scope = ScopeOf( track.usage );
			if ( const BufferRecord *buffer = m_D.LiveBuffer( id ) )
				BufferBarrier( *buffer, scope, scope );
			else if ( const TextureRecord *texture = m_D.LiveTexture( id ) )
				ImageBarrier( *texture, scope, scope, scope.layout );
			track.dirty = false;
		}
		if ( write )
			track.dirty = true;
	}

	// Shader accesses through bind groups: a resource in a write usage
	// (storage write) is written; the others are only read.
	void GroupAccesses( std::vector<std::uint64_t> groups )
	{
		// A pass binds the same few groups draw after draw: each group, then
		// each resource, is accessed once (sorted scratch, no node set).
		std::sort( groups.begin(), groups.end() );
		groups.erase( std::unique( groups.begin(), groups.end() ), groups.end() );
		std::vector<std::uint64_t> &resources = m_GroupScratch;
		resources.clear();
		for ( std::uint64_t groupId : groups )
		{
			const auto group = m_D.m_BindGroups.find( groupId );
			if ( groupId == 0 || group == m_D.m_BindGroups.end() )
				continue;
			for ( const ResourceId &resource : group->second.resources )
			{
				if ( resource.kind != ResourceKind::kSampler )
					resources.push_back( resource.value );
			}
		}
		std::sort( resources.begin(), resources.end() );
		resources.erase( std::unique( resources.begin(), resources.end() ), resources.end() );
		for ( std::uint64_t id : resources )
			Access( id, IsWrite( TrackOf( id ).usage ) );
	}

	void Flush( VkPipelineBindPoint point )
	{
		if ( !m_Pipeline )
			return;
		for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
		{
			if ( !m_Pipeline->layouts[role].IsValid() || !m_Groups[role] || !m_GroupDirty[role] )
				continue;
			const auto group = m_D.m_BindGroups.find( m_Groups[role] );
			if ( group == m_D.m_BindGroups.end() ||
			     group->second.layout != m_Pipeline->layouts[role] )
				continue;
			vkCmdBindDescriptorSets(
			    m_Cmd, point, m_Pipeline->pipelineLayout, role, 1, &group->second.set, 0, nullptr );
			m_GroupDirty[role] = false;
		}
	}

	void ApplyViewport( std::uint32_t width, std::uint32_t height )
	{
		const Viewport viewport = m_Viewport.value_or( Viewport{
		    0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f, 1.0f } );
		// Negative height: clip +Y points up and row 0 is the top row.
		VkViewport flipped{};
		flipped.x = viewport.x;
		flipped.y = viewport.y + viewport.height;
		flipped.width = viewport.width;
		flipped.height = -viewport.height;
		flipped.minDepth = viewport.minDepth;
		flipped.maxDepth = viewport.maxDepth;
		const VulkanAdapterOptions::Sensitivity &broken = m_D.m_Options.sensitivity;
		if ( broken.flipY )
		{
			flipped.y = viewport.y;
			flipped.height = viewport.height;
		}
		if ( broken.glDepthRange )
			flipped.minDepth = 0.5f * ( viewport.minDepth + viewport.maxDepth );
		vkCmdSetViewport( m_Cmd, 0, 1, &flipped );
	}

	bool TimestampBufferTransition( const Command &command )
	{
		if ( command.op != Op::kTransitionBuffer || command.before != ResourceUsage::kUndefined ||
		     command.after != ResourceUsage::kCopyDestination )
			return false;
		const BufferRecord *buffer = m_D.LiveBuffer( command.a );
		return buffer && buffer->desc.memory == MemoryKind::kReadback &&
		       buffer->desc.usages == UsageSet{ ResourceUsage::kCopyDestination };
	}

	void BeginRendering( const std::vector<Command> &commands, std::size_t index )
	{
		const Command &command = commands[index];
		// Everything the pass's draws may write through bind groups, found
		// ahead, since no barrier may sit inside the rendering.
		std::vector<std::uint64_t> groups( m_Groups.begin(), m_Groups.end() );
		// Each distinct group once: a bind repeating its slot's group adds none.
		std::array<std::uint64_t, kMaxBindGroups> slots = m_Groups;
		for ( std::size_t i = index + 1; i < commands.size() && commands[i].op != Op::kEndRendering;
		    ++i )
		{
			if ( commands[i].op == Op::kSetBindGroup && slots[commands[i].slot] != commands[i].a )
			{
				slots[commands[i].slot] = commands[i].a;
				groups.push_back( commands[i].a );
			}
			// Timer chunks opened between draws only receive timestamps;
			// the actual buffer copies occur in Finish. Hoist the transition
			// out of rendering while retaining its previous-use dependency.
			if ( TimestampBufferTransition( commands[i] ) )
				Transition( commands[i].a, commands[i].before, commands[i].after );
		}
		GroupAccesses( std::move( groups ) );

		const RenderingPayload &attachments = *command.rendering;
		if ( !m_D.m_Adapter.dynamicRendering )
		{
			BeginRenderPass( command, attachments );
			return;
		}
		std::vector<VkRenderingAttachmentInfo> colors;
		for ( const ColorAttachment &color : attachments.colors )
		{
			const TextureRecord *texture = m_D.LiveTexture( color.texture.value );
			Access( color.texture.value, true );
			VkRenderingAttachmentInfo info{};
			info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			info.imageView = texture->attachmentView;
			info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			info.loadOp = color.load == LoadOp::kLoad    ? VK_ATTACHMENT_LOAD_OP_LOAD
			              : color.load == LoadOp::kClear ? VK_ATTACHMENT_LOAD_OP_CLEAR
			                                             : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			info.storeOp = color.store == StoreOp::kStore ? VK_ATTACHMENT_STORE_OP_STORE
			                                              : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			info.clearValue.color = {
			    { color.clear.r, color.clear.g, color.clear.b, color.clear.a } };
			if ( color.resolve.IsValid() )
			{
				const TextureRecord *resolve = m_D.LiveTexture( color.resolve.value );
				Access( color.resolve.value, true );
				info.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
				info.resolveImageView = resolve->attachmentView;
				info.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			colors.push_back( info );
		}
		VkRenderingAttachmentInfo depth{};
		VkRenderingAttachmentInfo stencil{};
		const TextureRecord *depthTexture = nullptr;
		if ( const auto &attachment = attachments.depth )
		{
			const std::uint64_t id = attachment->texture.value;
			depthTexture = m_D.LiveTexture( id );
			const bool readOnly = TrackOf( id ).usage == ResourceUsage::kDepthRead;
			Access( id, !readOnly );
			depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depth.imageView = depthTexture->attachmentView;
			depth.imageLayout = ScopeOf( TrackOf( id ).usage ).layout;
			depth.loadOp = attachment->load == LoadOp::kLoad    ? VK_ATTACHMENT_LOAD_OP_LOAD
			               : attachment->load == LoadOp::kClear ? VK_ATTACHMENT_LOAD_OP_CLEAR
			                                                    : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth.storeOp = readOnly ? VK_ATTACHMENT_STORE_OP_NONE
			                : attachment->store == StoreOp::kStore
			                    ? VK_ATTACHMENT_STORE_OP_STORE
			                    : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth.clearValue.depthStencil = { attachment->clearDepth, 0 };
			stencil = depth;
		}
		VkRenderingInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.renderArea = { { 0, 0 }, { command.width, command.height } };
		info.layerCount = 1;
		info.colorAttachmentCount = static_cast<std::uint32_t>( colors.size() );
		info.pColorAttachments = colors.data();
		info.pDepthAttachment = depthTexture ? &depth : nullptr;
		info.pStencilAttachment =
		    depthTexture && HasStencil( depthTexture->desc.format ) ? &stencil : nullptr;
		m_D.m_Vk.cmdBeginRendering( m_Cmd, &info );
		m_Width = command.width;
		m_Height = command.height;
		m_Rendering = true;
		ApplyViewport( m_Width, m_Height );
		const VkRect2D scissor{ { 0, 0 }, { command.width, command.height } };
		vkCmdSetScissor( m_Cmd, 0, 1, &scissor );
	}

	// Progressive enhancement: the same pass as a render pass and a
	// framebuffer on a device without dynamic rendering. Same attachments,
	// layouts, ops and resolves; the framebuffer retires with the submission.
	void BeginRenderPass( const Command &command, const RenderingPayload &attachments )
	{
		using Attachment = VulkanDevice::RenderPassAttachment;
		auto load = []( LoadOp op )
		{
			return op == LoadOp::kLoad    ? VK_ATTACHMENT_LOAD_OP_LOAD
			       : op == LoadOp::kClear ? VK_ATTACHMENT_LOAD_OP_CLEAR
			                              : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		};
		VulkanDevice::RenderPassDesc desc;
		std::vector<VkImageView> views;
		std::vector<VkImageView> resolveViews;
		std::vector<VkClearValue> clears;
		bool anyResolve = false;
		for ( const ColorAttachment &color : attachments.colors )
		{
			const TextureRecord *texture = m_D.LiveTexture( color.texture.value );
			Access( color.texture.value, true );
			desc.colors.push_back( { ToVkFormat( texture->desc.format ),
			    static_cast<VkSampleCountFlagBits>( texture->desc.sampleCount ), load( color.load ),
			    color.store == StoreOp::kStore ? VK_ATTACHMENT_STORE_OP_STORE
			                                   : VK_ATTACHMENT_STORE_OP_DONT_CARE,
			    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } );
			views.push_back( texture->attachmentView );
			VkClearValue clear{};
			clear.color = { { color.clear.r, color.clear.g, color.clear.b, color.clear.a } };
			clears.push_back( clear );
			Attachment resolve;
			if ( color.resolve.IsValid() )
			{
				const TextureRecord *target = m_D.LiveTexture( color.resolve.value );
				Access( color.resolve.value, true );
				resolve = { ToVkFormat( target->desc.format ), VK_SAMPLE_COUNT_1_BIT,
				    VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
				    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
				resolveViews.push_back( target->attachmentView );
				anyResolve = true;
			}
			desc.resolves.push_back( resolve );
		}
		if ( !anyResolve )
			desc.resolves.clear();
		for ( VkImageView view : resolveViews )
		{
			views.push_back( view );
			clears.push_back( VkClearValue{} );
		}
		if ( const auto &attachment = attachments.depth )
		{
			const std::uint64_t id = attachment->texture.value;
			const TextureRecord *texture = m_D.LiveTexture( id );
			const bool readOnly = TrackOf( id ).usage == ResourceUsage::kDepthRead;
			Access( id, !readOnly );
			// Read-only depth stores nothing where the device can say so;
			// else it stores what it read (no write in a read-only layout).
			const VkAttachmentStoreOp store =
			    readOnly ? ( m_D.m_Adapter.storeOpNone ? VK_ATTACHMENT_STORE_OP_NONE_EXT
			                                           : VK_ATTACHMENT_STORE_OP_STORE )
			    : attachment->store == StoreOp::kStore ? VK_ATTACHMENT_STORE_OP_STORE
			                                           : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.depth = Attachment{ ToVkFormat( texture->desc.format ),
			    static_cast<VkSampleCountFlagBits>( texture->desc.sampleCount ),
			    load( attachment->load ), store, ScopeOf( TrackOf( id ).usage ).layout };
			desc.stencil = HasStencil( texture->desc.format );
			views.push_back( texture->attachmentView );
			VkClearValue clear{};
			clear.depthStencil = { attachment->clearDepth, 0 };
			clears.push_back( clear );
		}
		VkRenderPass pass = m_D.RenderPassFor( desc );
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		if ( pass != VK_NULL_HANDLE )
		{
			VkFramebufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.renderPass = pass;
			info.attachmentCount = static_cast<std::uint32_t>( views.size() );
			info.pAttachments = views.data();
			info.width = command.width;
			info.height = command.height;
			info.layers = 1;
			if ( vkCreateFramebuffer( m_D.m_Device, &info, nullptr, &framebuffer ) != VK_SUCCESS )
				framebuffer = VK_NULL_HANDLE;
		}
		if ( framebuffer == VK_NULL_HANDLE )
		{
			m_Failed = true;
			return;
		}
		m_Framebuffers.push_back( framebuffer );
		VkRenderPassBeginInfo begin{};
		begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin.renderPass = pass;
		begin.framebuffer = framebuffer;
		begin.renderArea = { { 0, 0 }, { command.width, command.height } };
		begin.clearValueCount = static_cast<std::uint32_t>( clears.size() );
		begin.pClearValues = clears.data();
		vkCmdBeginRenderPass( m_Cmd, &begin, VK_SUBPASS_CONTENTS_INLINE );
		m_RenderPass = true;
		m_Width = command.width;
		m_Height = command.height;
		m_Rendering = true;
		ApplyViewport( m_Width, m_Height );
		const VkRect2D scissor{ { 0, 0 }, { command.width, command.height } };
		vkCmdSetScissor( m_Cmd, 0, 1, &scissor );
	}

	void ClearTexture( const Command &command )
	{
		const TextureRecord *texture = m_D.LiveTexture( command.a );
		Access( command.a, true );
		const SubresourceRange &r = command.range;
		const std::uint32_t mips = r.baseMip < texture->desc.mipLevels
		                               ? std::min( r.mipCount, texture->desc.mipLevels - r.baseMip )
		                               : 0;
		const std::uint32_t layers = r.baseLayer < texture->layers
		                                 ? std::min( r.layerCount, texture->layers - r.baseLayer )
		                                 : 0;
		if ( mips == 0 || layers == 0 )
			return;
		const VkImageSubresourceRange range{
		    BarrierAspects( texture->desc.format ), r.baseMip, mips, r.baseLayer, layers };
		if ( IsDepthFormat( texture->desc.format ) )
		{
			const VkClearDepthStencilValue value{ std::clamp( command.color.r, 0.0f, 1.0f ), 0 };
			vkCmdClearDepthStencilImage(
			    m_Cmd, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &value, 1, &range );
		}
		else
		{
			VkClearColorValue value{};
			value.float32[0] = command.color.r;
			value.float32[1] = command.color.g;
			value.float32[2] = command.color.b;
			value.float32[3] = command.color.a;
			vkCmdClearColorImage(
			    m_Cmd, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &value, 1, &range );
		}
	}

	void TextureCopy( const Command &command, bool toBuffer )
	{
		const std::uint64_t textureId = toBuffer ? command.a : command.b;
		const std::uint64_t bufferId = toBuffer ? command.b : command.a;
		const TextureRecord *texture = m_D.LiveTexture( textureId );
		const BufferRecord *buffer = m_D.LiveBuffer( bufferId );
		Access( textureId, !toBuffer );
		Access( bufferId, toBuffer );
		const TextureBufferCopy &copy = command.textureCopy;
		VkBufferImageCopy region{};
		region.bufferOffset = copy.bufferOffset;
		region.imageSubresource.aspectMask = CopyAspect( texture->desc.format );
		region.imageSubresource.mipLevel = copy.mip;
		region.imageSubresource.baseArrayLayer = copy.layer;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { std::int32_t( copy.x ), std::int32_t( copy.y ), 0 };
		region.imageExtent = { copy.width, copy.height, 1 };
		if ( toBuffer )
			vkCmdCopyImageToBuffer( m_Cmd, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			    buffer->buffer, 1, &region );
		else
			vkCmdCopyBufferToImage( m_Cmd, buffer->buffer, texture->image,
			    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
	}

	void Translate( const std::vector<Command> &commands, std::size_t index )
	{
		const Command &command = commands[index];
		switch ( command.op )
		{
		case Op::kTransitionTexture:
		case Op::kTransitionBuffer:
			if ( m_Rendering && TimestampBufferTransition( command ) )
				break; // already ordered by BeginRendering
			Transition( command.a, command.before, command.after );
			break;
		case Op::kClearTexture:
			ClearTexture( command );
			break;
		case Op::kWriteBuffer:
		{
			const BufferRecord *buffer = m_D.LiveBuffer( command.a );
			Access( command.a, true );
			VkBufferCopy region{};
			region.srcOffset = command.staging != VK_NULL_HANDLE ? 0 : command.ringOffset;
			region.dstOffset = command.copy.destinationOffset;
			region.size = command.copy.size;
			vkCmdCopyBuffer( m_Cmd,
			    command.staging != VK_NULL_HANDLE ? command.staging : m_D.m_Ring.Buffer(),
			    buffer->buffer, 1, &region );
			break;
		}
		case Op::kCopyBuffer:
		{
			const BufferRecord *source = m_D.LiveBuffer( command.a );
			const BufferRecord *destination = m_D.LiveBuffer( command.b );
			Access( command.a, false );
			Access( command.b, true );
			if ( command.copy.size == 0 )
				break;
			VkBufferCopy region{};
			region.srcOffset = command.copy.sourceOffset;
			region.dstOffset = command.copy.destinationOffset;
			region.size = command.copy.size;
			vkCmdCopyBuffer( m_Cmd, source->buffer, destination->buffer, 1, &region );
			break;
		}
		case Op::kCopyTextureToBuffer:
			TextureCopy( command, true );
			break;
		case Op::kCopyBufferToTexture:
			TextureCopy( command, false );
			break;
		case Op::kCopyTexture:
		{
			const TextureRecord *source = m_D.LiveTexture( command.a );
			const TextureRecord *destination = m_D.LiveTexture( command.b );
			Access( command.a, false );
			Access( command.b, true );
			const TextureBufferCopy &copy = command.textureCopy;
			VkImageCopy region{};
			region.srcSubresource.aspectMask = CopyAspect( source->desc.format );
			region.srcSubresource.mipLevel = copy.mip;
			region.srcSubresource.baseArrayLayer = copy.layer;
			region.srcSubresource.layerCount = 1;
			region.dstSubresource = region.srcSubresource;
			region.srcOffset = { std::int32_t( copy.x ), std::int32_t( copy.y ), 0 };
			region.dstOffset = region.srcOffset;
			region.extent = { copy.width, copy.height, 1 };
			vkCmdCopyImage( m_Cmd, source->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			    destination->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
			break;
		}
		case Op::kBeginRendering:
			BeginRendering( commands, index );
			break;
		case Op::kEndRendering:
			if ( m_RenderPass )
				vkCmdEndRenderPass( m_Cmd );
			else
				m_D.m_Vk.cmdEndRendering( m_Cmd );
			m_RenderPass = false;
			m_Rendering = false;
			break;
		case Op::kSetPipeline:
		{
			// The bound pipeline again changes nothing. A different one with the
			// same layout at the same bind point keeps the bound sets (Vulkan's
			// pipeline layout compatibility), so only a new layout rebinds them.
			const PipelineRecord *next = &m_D.m_Pipelines.find( command.a )->second;
			if ( next == m_Pipeline )
				break;
			const bool keepSets = m_Pipeline && m_Pipeline->kind == next->kind &&
			                      m_Pipeline->pipelineLayout == next->pipelineLayout;
			m_Pipeline = next;
			vkCmdBindPipeline( m_Cmd,
			    m_Pipeline->kind == PipelineKind::kCompute ? VK_PIPELINE_BIND_POINT_COMPUTE
			                                               : VK_PIPELINE_BIND_POINT_GRAPHICS,
			    m_Pipeline->pipeline );
			if ( !keepSets )
				m_GroupDirty.fill( true );
			break;
		}
		case Op::kSetBindGroup:
			// The group already in the slot stays bound (or stays pending).
			if ( m_Groups[command.slot] != command.a )
			{
				m_Groups[command.slot] = command.a;
				m_GroupDirty[command.slot] = true;
			}
			break;
		case Op::kSetVertexBuffer:
		{
			if ( command.slot < m_VertexBound.size() && m_VertexBound[command.slot].valid &&
			     m_VertexBound[command.slot].buffer == command.a &&
			     m_VertexBound[command.slot].offset == command.offset )
				break;
			const VkBuffer buffer = m_D.LiveBuffer( command.a )->buffer;
			const VkDeviceSize offset = command.offset;
			vkCmdBindVertexBuffers( m_Cmd, command.slot, 1, &buffer, &offset );
			if ( command.slot < m_VertexBound.size() )
				m_VertexBound[command.slot] = { true, command.a, command.offset };
			break;
		}
		case Op::kSetIndexBuffer:
			if ( m_IndexBound.valid && m_IndexBound.buffer == command.a &&
			     m_IndexBound.offset == command.offset && m_IndexFormat == command.indexFormat )
				break;
			vkCmdBindIndexBuffer( m_Cmd, m_D.LiveBuffer( command.a )->buffer, command.offset,
			    command.indexFormat == IndexFormat::kUint16 ? VK_INDEX_TYPE_UINT16
			                                                : VK_INDEX_TYPE_UINT32 );
			m_IndexBound = { true, command.a, command.offset };
			m_IndexFormat = command.indexFormat;
			break;
		case Op::kSetViewport:
			m_Viewport = command.viewport;
			if ( m_Rendering )
				ApplyViewport( m_Width, m_Height );
			break;
		case Op::kDraw:
			Flush( VK_PIPELINE_BIND_POINT_GRAPHICS );
			vkCmdDraw(
			    m_Cmd, command.params[0], command.params[1], command.params[2], command.params[3] );
			break;
		case Op::kDrawIndexed:
			Flush( VK_PIPELINE_BIND_POINT_GRAPHICS );
			vkCmdDrawIndexed( m_Cmd, command.params[0], command.params[1], command.params[2],
			    command.vertexOffset, command.params[3] );
			break;
		case Op::kDrawIndexedIndirect:
			Flush( VK_PIPELINE_BIND_POINT_GRAPHICS );
			if ( command.params[0] > 0 )
				vkCmdDrawIndexedIndirect( m_Cmd, m_D.LiveBuffer( command.a )->buffer,
				    command.offset, command.params[0], command.params[1] );
			break;
		case Op::kDrawIndexedIndirectCount:
			Flush( VK_PIPELINE_BIND_POINT_GRAPHICS );
			if ( !m_D.m_Vk.cmdDrawIndexedIndirectCount )
			{
				m_Failed = true;
				break;
			}
			m_D.m_Vk.cmdDrawIndexedIndirectCount( m_Cmd, m_D.LiveBuffer( command.a )->buffer,
			    command.offset, m_D.LiveBuffer( command.b )->buffer, command.copy.destinationOffset,
			    command.params[0], command.params[1] );
			break;
		case Op::kComputeInterop:
		{
			const auto &payload = command.compute;
			for ( const auto &image : payload->images )
				Access( image.id.value, IsWrite( image.usage ) );
			if ( !m_Failed && !payload->Record( m_Cmd ) )
				m_Failed = true;
			HostRebinds();
			break;
		}
		case Op::kNative: // Native()
		case Op::kSectionBegin:
		case Op::kSectionEnd:
			break;
		case Op::kSetDrawConstants:
			vkCmdPushConstants( m_Cmd, m_Pipeline->pipelineLayout,
			    DrawConstantStages( m_Pipeline->kind ), std::uint32_t( command.offset ),
			    std::uint32_t( command.bytes.size() ), command.bytes.data() );
			break;
		case Op::kDispatch:
			GroupAccesses( std::vector<std::uint64_t>( m_Groups.begin(), m_Groups.end() ) );
			Flush( VK_PIPELINE_BIND_POINT_COMPUTE );
			vkCmdDispatch( m_Cmd, command.params[0], command.params[1], command.params[2] );
			break;
		case Op::kBeginLabel:
			if ( m_D.m_Instance->beginLabel )
			{
				VkDebugUtilsLabelEXT label{};
				label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
				label.pLabelName = command.label->c_str();
				m_D.m_Instance->beginLabel( m_Cmd, &label );
			}
			break;
		case Op::kEndLabel:
			if ( m_D.m_Instance->endLabel )
				m_D.m_Instance->endLabel( m_Cmd );
			break;
		case Op::kWriteTimestamp:
			vkCmdWriteTimestamp(
			    m_Cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_Queries, m_NextQuery );
			m_Timestamps.push_back( { m_NextQuery++, command.a, command.offset } );
			break;
		case Op::kBeginOcclusionQuery:
			// D43: exact sample counts (occlusionQueryPrecise, required for
			// the capability).
			vkCmdBeginQuery( m_Cmd, m_Occlusion, m_NextOcclusion, VK_QUERY_CONTROL_PRECISE_BIT );
			m_OcclusionResults.push_back( { m_NextOcclusion, command.a, command.offset } );
			break;
		case Op::kEndOcclusionQuery:
			vkCmdEndQuery( m_Cmd, m_Occlusion, m_NextOcclusion++ );
			break;
		}
	}

	// Host work at `index`, with the sections recorded after it; returns the
	// last command it consumed.
	std::size_t Native( const std::vector<Command> &commands, std::size_t index )
	{
		m_Sections.clear();
		std::size_t next = index + 1;
		while ( next < commands.size() && commands[next].op == Op::kSectionBegin )
		{
			std::size_t end = next + 1;
			while ( commands[end].op != Op::kSectionEnd ) // Validate closed it
				++end;
			m_Sections.push_back( { next + 1, end } );
			next = end + 1;
		}
		m_Commands = &commands;
		m_SectionsRun = 0;
		HostBoundary();
		const Command &command = commands[index];
		command.native( command.nativeUser, m_Cmd );
		// Sections the host work never asked for run after it.
		RunSectionsThrough( m_Sections.size() );
		m_Commands = nullptr;
		HostWrote();
		HostRebinds();
		return next - 1;
	}

	void RunSectionsThrough( std::size_t count )
	{
		for ( ; m_SectionsRun < count; ++m_SectionsRun )
		{
			const auto [begin, end] = m_Sections[m_SectionsRun];
			HostWrote();
			HostRebinds();
			for ( std::size_t i = begin; i < end; ++i )
				Translate( *m_Commands, i );
			HostBoundary();
		}
	}

	// Host work may have bound anything: the port rebinds before its next use.
	void HostRebinds()
	{
		m_Pipeline = nullptr;
		m_Groups.fill( 0 );
		m_GroupDirty.fill( true );
		m_Viewport.reset();
		ForgetBuffers();
	}

	// Host work and a new encoder may bind their own buffers.
	void ForgetBuffers()
	{
		for ( BoundBuffer &bound : m_VertexBound )
			bound.valid = false;
		m_IndexBound.valid = false;
	}

	struct PendingTimestamp
	{
		std::uint32_t query = 0;
		std::uint64_t buffer = 0;
		std::uint64_t offset = 0;
	};

	VulkanDevice &m_D;
	VkCommandBuffer m_Cmd;
	bool m_ComputeQueue = false;
	VkQueryPool m_Queries = VK_NULL_HANDLE;
	std::uint32_t m_NextQuery = 0;
	std::vector<PendingTimestamp> m_Timestamps;
	VkQueryPool m_Occlusion = VK_NULL_HANDLE; // D43
	std::uint32_t m_NextOcclusion = 0;
	std::vector<PendingTimestamp> m_OcclusionResults;
	std::vector<VkFramebuffer> m_Framebuffers;
	bool m_RenderPass = false; // the open pass is a render pass (fallback)
	// The host work being translated and its sections ([first, end) command
	// ranges), for RunSection.
	bool m_Failed = false;
	const std::vector<Command> *m_Commands = nullptr;
	std::vector<std::pair<std::size_t, std::size_t>> m_Sections;
	std::size_t m_SectionsRun = 0;
	std::unordered_map<std::uint64_t, Track> m_Tracks;
	std::vector<std::uint64_t> m_GroupScratch; // GroupAccesses' resources
	const PipelineRecord *m_Pipeline = nullptr;
	std::array<std::uint64_t, kMaxBindGroups> m_Groups{};
	std::array<bool, kMaxBindGroups> m_GroupDirty{};
	// The vertex and index buffers bound by this translation, to skip a bind
	// that repeats them.
	struct BoundBuffer
	{
		bool valid = false;
		std::uint64_t buffer = 0;
		std::uint64_t offset = 0;
	};
	std::array<BoundBuffer, 16> m_VertexBound{};
	BoundBuffer m_IndexBound;
	IndexFormat m_IndexFormat = IndexFormat::kUint16;
	std::optional<Viewport> m_Viewport;
	bool m_Rendering = false;
	std::uint32_t m_Width = 0;
	std::uint32_t m_Height = 0;
};

// Submission -------------------------------------------------------------------

bool VulkanDevice::RunSection( VkCommandBuffer cmd, std::uint32_t index )
{
	return m_Translating && m_Translating->RunSection( cmd, index );
}

DeviceResult<VulkanDevice::CommandContext> VulkanDevice::AcquireContext( QueueKind queue )
{
	const bool compute = queue == QueueKind::kCompute;
	std::vector<CommandContext> &free = compute ? m_Compute.free : m_FreeContexts;
	if ( !free.empty() )
	{
		CommandContext context = std::move( free.back() );
		free.pop_back();
		return context;
	}
	CommandContext context;
	VkCommandPoolCreateInfo pool{};
	pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	pool.queueFamilyIndex = compute ? m_Adapter.computeFamily : m_Adapter.queueFamily;
	VkResult result = vkCreateCommandPool( m_Device, &pool, nullptr, &context.pool );
	if ( result != VK_SUCCESS )
		return Fail( StatusOf( result ), DeviceOperation::kSubmit, result );
	VkCommandBufferAllocateInfo allocate{};
	allocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate.commandPool = context.pool;
	allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate.commandBufferCount = 1;
	result = vkAllocateCommandBuffers( m_Device, &allocate, &context.buffer );
	if ( result != VK_SUCCESS )
	{
		vkDestroyCommandPool( m_Device, context.pool, nullptr );
		return Fail( StatusOf( result ), DeviceOperation::kSubmit, result );
	}
	return context;
}

DeviceResult<CompletionToken> VulkanDevice::Submit(
    QueueKind queue, std::span<CommandEncoder> encoders, const SubmitWaits &waits )
{
	const DeviceOperation op = DeviceOperation::kSubmit;
	// Encoders are consumed whatever the outcome; an unsubmitted backend
	// returns its upload ranges when it is destroyed.
	std::vector<std::unique_ptr<IEncoderBackend>> backends;
	backends.reserve( encoders.size() );
	for ( CommandEncoder &encoder : encoders )
		backends.push_back( encoder.TakeBackend() );

	if ( m_State != DeviceState::kAvailable )
		return Fail( DeviceStatus::kDeviceLost, op );
	const bool compute = queue == QueueKind::kCompute;
	if ( queue != QueueKind::kGraphics && !( compute && AsyncCompute() ) )
		return Fail( DeviceStatus::kUnsupported, op );
	// Waits per queue: the latest value of each timeline (S8: a compute
	// submission may wait for graphics work and the reverse).
	std::uint64_t waitValue = 0;
	std::uint64_t computeWaitValue = 0;
	for ( const CompletionToken &wait : waits.tokens )
	{
		if ( wait.NamesSubmission() && wait.epoch < m_Epoch )
			return Fail( DeviceStatus::kStaleEpoch, op );
		if ( !wait.NamesSubmission() )
			continue;
		const bool waitCompute = wait.queue == QueueKind::kCompute;
		if ( ( wait.queue != QueueKind::kGraphics && !( waitCompute && AsyncCompute() ) ) ||
		     wait.epoch > m_Epoch || wait.value > SubmittedValue( wait.queue ) )
			return Fail( DeviceStatus::kInvalidDescription, op );
		std::uint64_t &into = waitCompute ? computeWaitValue : waitValue;
		into = std::max( into, wait.value );
	}

	std::unordered_map<std::uint64_t, ResourceUsage> states;
	std::vector<VulkanEncoder *> recorded;
	for ( std::unique_ptr<IEncoderBackend> &backend : backends )
	{
		VulkanEncoder *encoder = dynamic_cast<VulkanEncoder *>( backend.get() );
		if ( !encoder || &encoder->Device() != this || encoder->Queue() != queue )
			return Fail( DeviceStatus::kInvalidHandle, op );
		if ( !encoder->Complete() || !Validate( encoder->Commands(), states ) )
			return Fail( DeviceStatus::kInvalidState, op );
		recorded.push_back( encoder );
	}
	// The host resumes after the submission, too.
	if ( !ImportsAtHome( states ) )
		return Fail( DeviceStatus::kInvalidState, op );
	// D23: timestamps need the capability, and each buffer ends the
	// submission in kCopyDestination (their copies run at its end).
	std::uint32_t timestamps = 0;
	std::uint32_t occlusionQueries = 0; // D43
	for ( VulkanEncoder *encoder : recorded )
	{
		for ( const Command &command : encoder->Commands() )
		{
			if ( command.op == Op::kDrawIndexedIndirect &&
			     !m_Facts.capabilities.Has( Capability::kMultiDrawIndirect ) )
				return Fail( DeviceStatus::kUnsupported, op );
			if ( command.op == Op::kDrawIndexedIndirectCount &&
			     !m_Facts.capabilities.Has( Capability::kDrawIndirectCount ) )
				return Fail( DeviceStatus::kUnsupported, op );
			// The compute queue runs transfers and dispatches: no rendering,
			// draws, host work or timestamps (S8).
			if ( compute &&
			     ( command.op == Op::kBeginRendering || command.op == Op::kDraw ||
			         command.op == Op::kDrawIndexed || command.op == Op::kDrawIndexedIndirect ||
			         command.op == Op::kDrawIndexedIndirectCount ||
			         command.op == Op::kNative || command.op == Op::kSectionBegin ||
			         command.op == Op::kComputeInterop || command.op == Op::kWriteTimestamp ||
			         command.op == Op::kBeginOcclusionQuery ) )
				return Fail( DeviceStatus::kUnsupported, op );
			if ( command.op != Op::kWriteTimestamp && command.op != Op::kBeginOcclusionQuery )
				continue;
			const bool timestamp = command.op == Op::kWriteTimestamp;
			if ( !m_Facts.capabilities.Has(
			         timestamp ? Capability::kTimestamps : Capability::kOcclusionQueries ) )
				return Fail( DeviceStatus::kUnsupported, op );
			const auto found = states.find( command.a );
			if ( found == states.end() || found->second != ResourceUsage::kCopyDestination )
				return Fail( DeviceStatus::kInvalidState, op );
			( timestamp ? timestamps : occlusionQueries ) += 1;
		}
	}

	RecycleCompleted();
	auto acquired = AcquireContext( queue );
	if ( !acquired )
		return foundation::MakeUnexpected( acquired.Error() );
	CommandContext context = std::move( acquired ).Value();
	auto giveBack = [&]( VkResult result ) -> foundation::Unexpected<DeviceError>
	{
		if ( result == VK_ERROR_DEVICE_LOST )
			MarkLost();
		for ( VkFramebuffer framebuffer : context.framebuffers )
			vkDestroyFramebuffer( m_Device, framebuffer, nullptr );
		context.framebuffers.clear();
		if ( vkResetCommandPool( m_Device, context.pool, 0 ) == VK_SUCCESS )
		{
			( compute ? m_Compute.free : m_FreeContexts ).push_back( std::move( context ) );
		}
		else
		{
			vkDestroyCommandPool( m_Device, context.pool, nullptr );
			if ( context.queries != VK_NULL_HANDLE )
				vkDestroyQueryPool( m_Device, context.queries, nullptr );
			if ( context.occlusion != VK_NULL_HANDLE )
				vkDestroyQueryPool( m_Device, context.occlusion, nullptr );
		}
		return Fail( StatusOf( result ), op, result );
	};
	if ( timestamps > context.queryCapacity )
	{
		if ( context.queries != VK_NULL_HANDLE )
			vkDestroyQueryPool( m_Device, context.queries, nullptr );
		context.queries = VK_NULL_HANDLE;
		context.queryCapacity = 0;
		std::uint32_t capacity = 64;
		while ( capacity < timestamps )
			capacity *= 2;
		VkQueryPoolCreateInfo queryInfo{};
		queryInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
		queryInfo.queryCount = capacity;
		const VkResult created =
		    vkCreateQueryPool( m_Device, &queryInfo, nullptr, &context.queries );
		if ( created != VK_SUCCESS )
		{
			context.queries = VK_NULL_HANDLE;
			return giveBack( created );
		}
		context.queryCapacity = capacity;
	}
	if ( occlusionQueries > context.occlusionCapacity )
	{
		if ( context.occlusion != VK_NULL_HANDLE )
			vkDestroyQueryPool( m_Device, context.occlusion, nullptr );
		context.occlusion = VK_NULL_HANDLE;
		context.occlusionCapacity = 0;
		std::uint32_t capacity = 64;
		while ( capacity < occlusionQueries )
			capacity *= 2;
		VkQueryPoolCreateInfo queryInfo{};
		queryInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryInfo.queryType = VK_QUERY_TYPE_OCCLUSION;
		queryInfo.queryCount = capacity;
		const VkResult created =
		    vkCreateQueryPool( m_Device, &queryInfo, nullptr, &context.occlusion );
		if ( created != VK_SUCCESS )
		{
			context.occlusion = VK_NULL_HANDLE;
			return giveBack( created );
		}
		context.occlusionCapacity = capacity;
	}

	VkCommandBufferBeginInfo begin{};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkResult result = vkBeginCommandBuffer( context.buffer, &begin );
	if ( result != VK_SUCCESS )
		return giveBack( result );
	Translator translator( *this, context.buffer, compute );
	if ( timestamps > 0 )
	{
		vkCmdResetQueryPool( context.buffer, context.queries, 0, timestamps );
		translator.SetQueries( context.queries );
	}
	if ( occlusionQueries > 0 )
	{
		vkCmdResetQueryPool( context.buffer, context.occlusion, 0, occlusionQueries );
		translator.SetOcclusion( context.occlusion );
	}
	m_Translating = &translator;
	bool translated = true;
	for ( VulkanEncoder *encoder : recorded )
		if ( !translator.Encoder( encoder->Commands() ) )
		{
			translated = false;
			break;
		}
	m_Translating = nullptr;
	context.framebuffers = translator.TakeFramebuffers();
	if ( !translated )
	{
		for ( VulkanEncoder *encoder : recorded )
			for ( const auto &payload : encoder->Computes() )
				payload->Aborted();
		return giveBack( VK_ERROR_INITIALIZATION_FAILED );
	}
	translator.Finish();
	result = vkEndCommandBuffer( context.buffer );
	if ( result != VK_SUCCESS )
		return giveBack( result );

	const std::uint64_t value = SubmittedValue( queue ) + 1;
	VkSemaphoreSubmitInfo signal{};
	signal.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signal.semaphore = compute ? m_Compute.timeline : m_Timeline;
	signal.value = value;
	signal.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	VkSemaphoreSubmitInfo wait{};
	wait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	wait.semaphore = m_Timeline;
	wait.value = waitValue;
	wait.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	VkCommandBufferSubmitInfo commandInfo{};
	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	commandInfo.commandBuffer = context.buffer;
	// The timeline, then the encoders' binary semaphores (host interop).
	std::vector<VkSemaphoreSubmitInfo> waitInfos;
	std::vector<VkSemaphoreSubmitInfo> signalInfos{ signal };
	if ( waitValue > 0 )
		waitInfos.push_back( wait );
	if ( computeWaitValue > 0 )
	{
		VkSemaphoreSubmitInfo computeWait = wait;
		computeWait.semaphore = m_Compute.timeline;
		computeWait.value = computeWaitValue;
		waitInfos.push_back( computeWait );
	}
	if ( m_Holding )
	{
		// Tests only (Hold): the work starts once the host releases the hold.
		VkSemaphoreSubmitInfo hold{};
		hold.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		hold.semaphore = m_Hold;
		hold.value = m_HoldValue + 1;
		hold.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		waitInfos.push_back( hold );
	}
	for ( VulkanEncoder *encoder : recorded )
	{
		waitInfos.insert( waitInfos.end(), encoder->Waits().begin(), encoder->Waits().end() );
		signalInfos.insert(
		    signalInfos.end(), encoder->Signals().begin(), encoder->Signals().end() );
	}
	VkSubmitInfo2 submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submit.waitSemaphoreInfoCount = static_cast<std::uint32_t>( waitInfos.size() );
	submit.pWaitSemaphoreInfos = waitInfos.data();
	submit.commandBufferInfoCount = 1;
	submit.pCommandBufferInfos = &commandInfo;
	submit.signalSemaphoreInfoCount = static_cast<std::uint32_t>( signalInfos.size() );
	submit.pSignalSemaphoreInfos = signalInfos.data();
	{
		// A borrowing host shares the queue (host_device.h).
		std::lock_guard<std::mutex> lock( m_QueueMutex );
		result = m_Vk.queueSubmit2(
		    compute ? m_Compute.queue : m_Queue, 1, &submit, VK_NULL_HANDLE );
	}
	if ( result != VK_SUCCESS )
		return giveBack( result ); // marks the device lost on VK_ERROR_DEVICE_LOST

	( compute ? m_Compute.submitted : m_Submitted ) = value;
	const CompletionToken token{ queue, m_Epoch, value };
	translator.Commit();
	for ( VulkanEncoder *encoder : recorded )
		for ( const auto &payload : encoder->Computes() )
		{
			payload->Submitted( token );
			context.compute.push_back( payload );
		}
	for ( VulkanEncoder *encoder : recorded )
	{
		for ( std::uint64_t allocation : encoder->RingAllocations() )
		{
			if ( m_Options.sensitivity.unsafeUploadReuse )
				m_Ring.Abandon( allocation );
			else
				m_Ring.Submit( allocation, value );
		}
		for ( HostBuffer &staging : encoder->Staging() )
			context.staging.push_back( staging );
		encoder->Staging().clear();
		encoder->MarkSubmitted();
	}
	context.value = value;
	( compute ? m_Compute.inFlight : m_InFlight ).push_back( std::move( context ) );
	return token;
}

} // namespace render::device::vulkan
