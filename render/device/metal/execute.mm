//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.metal: submission. Validate checks one encoder's
//			list against the port's rules and the usage state (as the other
//			adapters do at Submit); Replay records the accepted lists in order
//			into the submission's command buffer.
//
//=============================================================================//

#include "metal_device.h"

#include <cstring>

namespace render::device::metal
{

// Validation --------------------------------------------------------------------

struct ValidationState
{
	const PipelineRecord *pipeline = nullptr;
	std::array<std::uint64_t, kMaxBindGroups> groups{};
	std::vector<Format> colors;
	Format depth = Format::kUnknown;
	std::uint32_t samples = 0;
	bool rendering = false;
	std::uint32_t vertexSlots = 0;
	bool index = false;
	DrawConstantCoverage constants; // D16
};

bool MetalDevice::GroupsMatch( const ValidationState &state ) const
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

bool MetalDevice::ValidateDraw( const ValidationState &state, bool indexed ) const
{
	const PipelineRecord *pipeline = state.pipeline;
	if ( !pipeline || pipeline->kind != PipelineKind::kGraphics || !state.rendering )
		return false;
	if ( pipeline->colorFormats != state.colors || pipeline->depthFormat != state.depth ||
	     pipeline->sampleCount != state.samples )
		return false;
	for ( std::uint32_t slot = 0; slot < pipeline->vertexBuffers; ++slot )
	{
		if ( !( state.vertexSlots & ( 1u << slot ) ) )
			return false;
	}
	if ( indexed && !state.index )
		return false;
	return GroupsMatch( state );
}

bool MetalDevice::Validate(
    const std::vector<Command> &commands, std::unordered_map<std::uint64_t, ResourceUsage> &states )
{
	auto state = [&]( std::uint64_t id, ResourceUsage current ) -> ResourceUsage &
	{
		return states.try_emplace( id, current ).first->second;
	};
	auto texture = [&]( std::uint64_t id, ResourceUsage required ) -> TextureRecord *
	{
		TextureRecord *t = LiveTexture( id );
		return t && state( id, t->usage ) == required ? t : nullptr;
	};
	auto buffer = [&]( std::uint64_t id, ResourceUsage required ) -> BufferRecord *
	{
		BufferRecord *b = LiveBuffer( id );
		return b && state( id, b->usage ) == required ? b : nullptr;
	};
	auto copyFits =
	    []( const TextureRecord &t, const BufferRecord &b, const TextureBufferCopy &copy )
	{
		const std::uint32_t layers = t.desc.dimension == TextureDimension::k3D
		                                 ? std::max( 1u, t.desc.depthOrLayers >> copy.mip )
		                                 : t.layers;
		if ( copy.mip >= t.desc.mipLevels || copy.layer >= layers || copy.width == 0 ||
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
			ResourceUsage &current = state( command.a, t->usage );
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
			ResourceUsage &current = state( command.a, b->usage );
			if ( command.before != ResourceUsage::kUndefined && command.before != current )
				return false;
			current = command.after;
			break;
		}
		case Op::kClearTexture:
		{
			// A block-compressed texture is written by copies only (D19).
			// Clears are render passes: no 3D texture (no render target).
			const TextureRecord *t = texture( command.a, ResourceUsage::kCopyDestination );
			if ( !t || IsBlockCompressed( t->desc.format ) || t->desc.sampleCount != 1 ||
			     t->desc.dimension == TextureDimension::k3D )
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
			// A blit copy; Metal copies block-compressed regions too.
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
				if ( t.desc.dimension == TextureDimension::k3D || command.width > t.desc.width ||
				     command.height > t.desc.height )
					return false;
				if ( v.samples != 0 && v.samples != t.desc.sampleCount )
					return false;
				v.samples = t.desc.sampleCount;
				return true;
			};
			if ( command.colors.size() > m_Facts.limits.maxColorAttachments )
				return false;
			for ( const ColorAttachment &color : command.colors )
			{
				TextureRecord *t = texture( color.texture.value, ResourceUsage::kColorAttachment );
				if ( !t || !fits( *t ) )
					return false;
				v.colors.push_back( t->desc.format );
				if ( color.resolve.IsValid() )
				{
					TextureRecord *r =
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
				TextureRecord *t = texture( id, ResourceUsage::kDepthWrite );
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
			// A group whose resources were released would read freed objects.
			for ( const BindGroupEntry &entry : group->second.entries )
			{
				const auto sampler = m_Samplers.find( entry.sampler.value );
				if ( ( entry.buffer.IsValid() && !LiveBuffer( entry.buffer.value ) ) ||
				     ( entry.texture.IsValid() && !LiveTexture( entry.texture.value ) ) ||
				     ( entry.sampler.IsValid() &&
				         ( sampler == m_Samplers.end() || sampler->second.released ) ) )
					return false;
			}
			v.groups[command.slot] = command.a;
			break;
		}
		case Op::kSetVertexBuffer:
			if ( !buffer( command.a, ResourceUsage::kVertex ) || command.slot >= kMaxVertexSlots )
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
			const BufferRecord *b = buffer( command.a, ResourceUsage::kCopyDestination );
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
			// capability is checked after validation (kUnsupported).
			const BufferRecord *records = buffer( command.a, ResourceUsage::kIndirect );
			if ( !ValidateDraw( v, true ) || !v.constants.Ready() || !records ||
			     !IndirectRecordsFit( records->desc.size, command.offset, command.count,
			         command.first ) )
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
		}
	}
	return true;
}

// Replay ------------------------------------------------------------------------

namespace
{

MTLLoadAction LoadActionOf( LoadOp load )
{
	switch ( load )
	{
	case LoadOp::kLoad:
		return MTLLoadActionLoad;
	case LoadOp::kClear:
		return MTLLoadActionClear;
	case LoadOp::kDiscard:
		return MTLLoadActionDontCare;
	}
	return MTLLoadActionLoad;
}

MTLResourceUsage UsageOf( BindingKind kind )
{
	return kind == BindingKind::kStorageBuffer || kind == BindingKind::kStorageTexture
	           ? MTLResourceUsageRead | MTLResourceUsageWrite
	           : MTLResourceUsageRead;
}

} // namespace

// Records one submission's encoders into a command buffer, in order.
class Replay
{
public:
	Replay( MetalDevice &device, id<MTLCommandBuffer> commands )
	    : m_D( device ), m_Commands( commands )
	{
	}

	void Run( std::vector<MetalEncoder *> &encoders )
	{
		for ( MetalEncoder *encoder : encoders )
		{
			m_Pipeline = nullptr;
			m_PipelineId = 0;
			m_Groups.fill( 0 );
			m_VertexBuffers.fill( {} );
			m_Index = {};
			m_Constants.fill( std::byte{ 0 } );
			for ( const Command &command : encoder->Commands() )
				Execute( command );
			End();
		}
	}

private:
	struct Binding
	{
		std::uint64_t buffer = 0;
		std::uint64_t offset = 0;
	};

	NSString *CurrentLabel() const
	{
		if ( m_Labels.empty() )
			return nil;
		const std::string &label = m_Labels.back();
		return [[NSString alloc] initWithBytes:label.data()
		                                length:label.size()
		                              encoding:NSUTF8StringEncoding];
	}

	// Ends whichever encoder is open.
	void End()
	{
		if ( m_Blit )
			[m_Blit endEncoding];
		if ( m_Compute )
			[m_Compute endEncoding];
		if ( m_Render )
			[m_Render endEncoding];
		m_Blit = nil;
		m_Compute = nil;
		m_Render = nil;
		m_Resident.clear();
	}

	id<MTLBlitCommandEncoder> Blit()
	{
		if ( !m_Blit )
		{
			End();
			m_Blit = [m_Commands blitCommandEncoder];
			m_Blit.label = CurrentLabel();
		}
		return m_Blit;
	}

	id<MTLComputeCommandEncoder> Compute()
	{
		if ( !m_Compute )
		{
			End();
			m_Compute = [m_Commands computeCommandEncoder];
			m_Compute.label = CurrentLabel();
		}
		return m_Compute;
	}

	void Execute( const Command &command )
	{
		switch ( command.op )
		{
		case Op::kTransitionTexture:
		case Op::kTransitionBuffer:
			// Automatic hazard tracking orders the accesses.
			break;
		case Op::kClearTexture:
			ClearTexture( command );
			break;
		case Op::kWriteBuffer:
		{
			id<MTLBuffer> destination = m_D.ExistingBuffer( command.a )->buffer;
			id<MTLBuffer> source = m_D.Ring();
			std::uint64_t from = command.ringOffset;
			if ( !command.fromRing )
			{
				// The ring was full: the bytes travel in their own buffer, which
				// the command buffer keeps alive.
				source = [m_D.Native() newBufferWithBytes:command.bytes.data()
				                                   length:command.bytes.size()
				                                  options:MTLResourceStorageModeShared];
				from = 0;
			}
			[Blit() copyFromBuffer:source
			          sourceOffset:from
			              toBuffer:destination
			     destinationOffset:command.copy.destinationOffset
			                  size:command.copy.size];
			break;
		}
		case Op::kCopyBuffer:
			[Blit() copyFromBuffer:m_D.ExistingBuffer( command.a )->buffer
			          sourceOffset:command.copy.sourceOffset
			              toBuffer:m_D.ExistingBuffer( command.b )->buffer
			     destinationOffset:command.copy.destinationOffset
			                  size:command.copy.size];
			break;
		case Op::kCopyTextureToBuffer:
		case Op::kCopyBufferToTexture:
			CopyTextureAndBuffer( command );
			break;
		case Op::kCopyTexture:
		{
			const TextureRecord &s = *m_D.ExistingTexture( command.a );
			const TextureRecord &d = *m_D.ExistingTexture( command.b );
			const TextureBufferCopy &copy = command.textureCopy;
			const bool volume = s.desc.dimension == TextureDimension::k3D;
			[Blit() copyFromTexture:s.texture
			            sourceSlice:volume ? 0 : copy.layer
			            sourceLevel:copy.mip
			           sourceOrigin:MTLOriginMake( copy.x, copy.y, volume ? copy.layer : 0 )
			             sourceSize:MTLSizeMake( copy.width, copy.height, 1 )
			              toTexture:d.texture
			       destinationSlice:volume ? 0 : copy.layer
			       destinationLevel:copy.mip
			      destinationOrigin:MTLOriginMake( copy.x, copy.y, volume ? copy.layer : 0 )];
			break;
		}
		case Op::kBeginRendering:
			BeginRendering( command );
			break;
		case Op::kEndRendering:
			End();
			break;
		case Op::kSetPipeline:
			m_Pipeline = m_D.ExistingPipeline( command.a );
			m_PipelineId = command.a;
			m_StateDirty = true;
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
				[m_Render setViewport:( MTLViewport ){ command.viewport.x, command.viewport.y,
				                          command.viewport.width, command.viewport.height,
				                          command.viewport.minDepth, command.viewport.maxDepth }];
			break;
		case Op::kSetDrawConstants:
			std::memcpy( m_Constants.data() + command.offset, command.bytes.data(),
			    command.bytes.size() );
			break;
		case Op::kDraw:
		case Op::kDrawIndexed:
			Draw( command );
			break;
		case Op::kDrawIndexedIndirect:
			DrawIndirect( command );
			break;
		case Op::kDispatch:
			Dispatch( command );
			break;
		case Op::kBeginLabel:
			m_Labels.emplace_back(
			    reinterpret_cast<const char *>( command.bytes.data() ), command.bytes.size() );
			break;
		case Op::kEndLabel:
			if ( !m_Labels.empty() )
				m_Labels.pop_back();
			break;
		case Op::kWriteTimestamp:
		case Op::kDrawIndexedIndirectCount:
			break; // refused at Submit (unclaimed capabilities)
		}
	}

	// Each mip and layer in the range is cleared by an empty render pass.
	void ClearTexture( const Command &command )
	{
		End();
		const TextureRecord &t = *m_D.ExistingTexture( command.a );
		const bool depth = IsDepthFormat( t.desc.format );
		const SubresourceRange &r = command.range;
		const std::uint32_t mips = std::min( r.baseMip + r.mipCount, t.desc.mipLevels );
		const std::uint32_t layers = std::min( r.baseLayer + r.layerCount, t.layers );
		for ( std::uint32_t mip = r.baseMip; mip < mips; ++mip )
		{
			for ( std::uint32_t layer = r.baseLayer; layer < layers; ++layer )
			{
				MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
				if ( depth )
				{
					pass.depthAttachment.texture = t.texture;
					pass.depthAttachment.level = mip;
					pass.depthAttachment.slice = layer;
					pass.depthAttachment.loadAction = MTLLoadActionClear;
					pass.depthAttachment.storeAction = MTLStoreActionStore;
					pass.depthAttachment.clearDepth = command.color.r;
					if ( HasStencil( t.desc.format ) )
					{
						pass.stencilAttachment.texture = t.texture;
						pass.stencilAttachment.level = mip;
						pass.stencilAttachment.slice = layer;
						pass.stencilAttachment.loadAction = MTLLoadActionClear;
						pass.stencilAttachment.storeAction = MTLStoreActionStore;
						pass.stencilAttachment.clearStencil = 0;
					}
				}
				else
				{
					// The clear color is linear; an sRGB attachment encodes it.
					MTLRenderPassColorAttachmentDescriptor *color = pass.colorAttachments[0];
					color.texture = t.texture;
					color.level = mip;
					color.slice = layer;
					color.loadAction = MTLLoadActionClear;
					color.storeAction = MTLStoreActionStore;
					color.clearColor = MTLClearColorMake(
					    command.color.r, command.color.g, command.color.b, command.color.a );
				}
				id<MTLRenderCommandEncoder> clear =
				    [m_Commands renderCommandEncoderWithDescriptor:pass];
				clear.label = CurrentLabel();
				[clear endEncoding];
			}
		}
	}

	void CopyTextureAndBuffer( const Command &command )
	{
		const bool toBuffer = command.op == Op::kCopyTextureToBuffer;
		const TextureRecord &t = *m_D.ExistingTexture( toBuffer ? command.a : command.b );
		const BufferRecord &b = *m_D.ExistingBuffer( toBuffer ? command.b : command.a );
		const TextureBufferCopy &copy = command.textureCopy;
		const std::uint64_t row = RegionBytes( t.desc.format, copy.width, 1 );
		const std::uint64_t image = RegionBytes( t.desc.format, copy.width, copy.height );
		const bool volume = t.desc.dimension == TextureDimension::k3D;
		const NSUInteger slice = volume ? 0 : copy.layer;
		const MTLOrigin origin = MTLOriginMake( copy.x, copy.y, volume ? copy.layer : 0 );
		const MTLSize size = MTLSizeMake( copy.width, copy.height, 1 );
		// A depth-stencil copy moves the depth aspect: the port's 32-bit floats.
		const MTLBlitOption options =
		    HasStencil( t.desc.format ) ? MTLBlitOptionDepthFromDepthStencil : MTLBlitOptionNone;
		if ( toBuffer )
			[Blit() copyFromTexture:t.texture
			                 sourceSlice:slice
			                 sourceLevel:copy.mip
			                sourceOrigin:origin
			                  sourceSize:size
			                    toBuffer:b.buffer
			           destinationOffset:copy.bufferOffset
			      destinationBytesPerRow:row
			    destinationBytesPerImage:image
			                     options:options];
		else
			[Blit() copyFromBuffer:b.buffer
			               sourceOffset:copy.bufferOffset
			          sourceBytesPerRow:row
			        sourceBytesPerImage:image
			                 sourceSize:size
			                  toTexture:t.texture
			           destinationSlice:slice
			           destinationLevel:copy.mip
			          destinationOrigin:origin
			                    options:options];
	}

	void BeginRendering( const Command &command )
	{
		End();
		MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
		for ( std::size_t i = 0; i < command.colors.size(); ++i )
		{
			const ColorAttachment &attachment = command.colors[i];
			MTLRenderPassColorAttachmentDescriptor *color = pass.colorAttachments[i];
			color.texture = m_D.ExistingTexture( attachment.texture.value )->texture;
			color.loadAction = LoadActionOf( attachment.load );
			color.clearColor = MTLClearColorMake(
			    attachment.clear.r, attachment.clear.g, attachment.clear.b, attachment.clear.a );
			const bool store = attachment.store == StoreOp::kStore;
			if ( attachment.resolve.IsValid() )
			{
				color.resolveTexture = m_D.ExistingTexture( attachment.resolve.value )->texture;
				color.storeAction = store ? MTLStoreActionStoreAndMultisampleResolve
				                          : MTLStoreActionMultisampleResolve;
			}
			else
			{
				color.storeAction = store ? MTLStoreActionStore : MTLStoreActionDontCare;
			}
		}
		if ( command.depth )
		{
			const TextureRecord &t = *m_D.ExistingTexture( command.depth->texture.value );
			const MTLStoreAction store = command.depth->store == StoreOp::kStore
			                                 ? MTLStoreActionStore
			                                 : MTLStoreActionDontCare;
			pass.depthAttachment.texture = t.texture;
			pass.depthAttachment.loadAction = LoadActionOf( command.depth->load );
			pass.depthAttachment.storeAction = store;
			pass.depthAttachment.clearDepth = command.depth->clearDepth;
			if ( HasStencil( t.desc.format ) )
			{
				pass.stencilAttachment.texture = t.texture;
				pass.stencilAttachment.loadAction = LoadActionOf( command.depth->load );
				pass.stencilAttachment.storeAction = store;
				pass.stencilAttachment.clearStencil = 0;
			}
		}
		pass.renderTargetWidth = command.width;
		pass.renderTargetHeight = command.height;
		m_Render = [m_Commands renderCommandEncoderWithDescriptor:pass];
		m_Render.label = CurrentLabel();
		// The render area: attachments may be larger than the pass.
		[m_Render setViewport:( MTLViewport ){ 0.0, 0.0, double( command.width ),
		                          double( command.height ), 0.0, 1.0 }];
		[m_Render setScissorRect:( MTLScissorRect ){ 0, 0, command.width, command.height }];
		m_StateDirty = true;
	}

	// Binds the groups a pipeline stage declares, as its argument buffers,
	// and declares the resources they name resident for this encoder.
	template <typename Bind, typename Use>
	void BindGroups( std::uint32_t stage, Bind bind, Use use )
	{
		const PipelineRecord &p = *m_Pipeline;
		for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
		{
			const StageGroup &view = p.groups[stage][role];
			if ( !view.encoder )
				continue;
			BindGroupRecord &group = *m_D.ExistingBindGroup( m_Groups[role] );
			bind( m_D.ArgumentBuffer( group, m_PipelineId, stage, view ), role );
			const std::uint64_t key = ( m_Groups[role] << 2 ) | stage;
			if ( !m_Resident.insert( key ).second )
				continue;
			for ( const BindGroupEntry &entry : group.entries )
			{
				BindingKind kind = BindingKind::kUniformBuffer;
				for ( const BindingDesc &binding : group.bindings )
				{
					if ( binding.binding == entry.binding )
						kind = binding.kind;
				}
				if ( entry.buffer.IsValid() )
					use( m_D.ExistingBuffer( entry.buffer.value )->buffer, UsageOf( kind ) );
				else if ( entry.texture.IsValid() )
					use( m_D.ExistingTexture( entry.texture.value )->texture, UsageOf( kind ) );
			}
		}
	}

	void PrepareDraw()
	{
		const PipelineRecord &p = *m_Pipeline;
		if ( m_StateDirty )
		{
			[m_Render setRenderPipelineState:p.render];
			[m_Render setDepthStencilState:p.depthStencil];
			[m_Render setCullMode:p.cull];
			[m_Render setFrontFacingWinding:p.winding];
			[m_Render setDepthBias:p.depthBiasConstant slopeScale:p.depthBiasSlope clamp:0.0f];
			[m_Render setStencilReferenceValue:p.stencilReference];
			m_StateDirty = false;
		}
		for ( std::uint32_t slot = 0; slot < p.vertexBuffers; ++slot )
			[m_Render setVertexBuffer:m_D.ExistingBuffer( m_VertexBuffers[slot].buffer )->buffer
			                   offset:m_VertexBuffers[slot].offset
			                  atIndex:kVertexBufferBase + slot];
		id<MTLRenderCommandEncoder> render = m_Render;
		auto use = [render]( id<MTLResource> resource, MTLResourceUsage usage )
		{
			[render useResource:resource
			              usage:usage
			             stages:MTLRenderStageVertex | MTLRenderStageFragment];
		};
		BindGroups(
		    kStageVertex,
		    [render]( id<MTLBuffer> buffer, std::uint32_t role )
		    { [render setVertexBuffer:buffer offset:0 atIndex:role]; },
		    use );
		BindGroups(
		    kStageFragment,
		    [render]( id<MTLBuffer> buffer, std::uint32_t role )
		    { [render setFragmentBuffer:buffer offset:0 atIndex:role]; },
		    use );
		if ( p.drawConstantBytes > 0 )
		{
			[m_Render setVertexBytes:m_Constants.data()
			                  length:p.drawConstantBytes
			                 atIndex:kDrawConstantsSlot];
			[m_Render setFragmentBytes:m_Constants.data()
			                    length:p.drawConstantBytes
			                   atIndex:kDrawConstantsSlot];
		}
	}

	MTLIndexType IndexType() const
	{
		return m_IndexFormat == IndexFormat::kUint32 ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
	}

	void Draw( const Command &command )
	{
		PrepareDraw();
		const PipelineRecord &p = *m_Pipeline;
		if ( command.op == Op::kDraw )
		{
			[m_Render drawPrimitives:p.primitive
			             vertexStart:command.first
			             vertexCount:command.count
			           instanceCount:command.instances
			            baseInstance:command.firstInstance];
			return;
		}
		const std::uint64_t size = m_IndexFormat == IndexFormat::kUint32 ? 4 : 2;
		[m_Render drawIndexedPrimitives:p.primitive
		                     indexCount:command.count
		                      indexType:IndexType()
		                    indexBuffer:m_D.ExistingBuffer( m_Index.buffer )->buffer
		              indexBufferOffset:m_Index.offset + std::uint64_t( command.first ) * size
		                  instanceCount:command.instances
		                     baseVertex:command.vertexOffset
		                   baseInstance:command.firstInstance];
	}

	// D30: Metal's indexed indirect arguments have the port record's layout
	// (MTLDrawIndexedPrimitivesIndirectArguments), one draw per record.
	void DrawIndirect( const Command &command )
	{
		PrepareDraw();
		const PipelineRecord &p = *m_Pipeline;
		id<MTLBuffer> records = m_D.ExistingBuffer( command.a )->buffer;
		for ( std::uint32_t i = 0; i < command.count; ++i )
			[m_Render drawIndexedPrimitives:p.primitive
			                      indexType:IndexType()
			                    indexBuffer:m_D.ExistingBuffer( m_Index.buffer )->buffer
			              indexBufferOffset:m_Index.offset
			                 indirectBuffer:records
			           indirectBufferOffset:command.offset + std::uint64_t( i ) * command.first];
	}

	void Dispatch( const Command &command )
	{
		id<MTLComputeCommandEncoder> compute = Compute();
		const PipelineRecord &p = *m_Pipeline;
		[compute setComputePipelineState:p.compute];
		BindGroups(
		    kStageCompute,
		    [compute]( id<MTLBuffer> buffer, std::uint32_t role )
		    { [compute setBuffer:buffer offset:0 atIndex:role]; },
		    [compute]( id<MTLResource> resource, MTLResourceUsage usage )
		    { [compute useResource:resource usage:usage]; } );
		if ( p.drawConstantBytes > 0 )
			[compute setBytes:m_Constants.data()
			           length:p.drawConstantBytes
			          atIndex:kDrawConstantsSlot];
		[compute dispatchThreadgroups:MTLSizeMake( command.count, command.first,
		                                  command.firstInstance )
		        threadsPerThreadgroup:p.threadgroup];
	}

	MetalDevice &m_D;
	id<MTLCommandBuffer> m_Commands;
	id<MTLBlitCommandEncoder> m_Blit = nil;
	id<MTLComputeCommandEncoder> m_Compute = nil;
	id<MTLRenderCommandEncoder> m_Render = nil;
	std::unordered_set<std::uint64_t> m_Resident; // (group << 2 | stage) per encoder
	std::vector<std::string> m_Labels;
	const PipelineRecord *m_Pipeline = nullptr;
	std::uint64_t m_PipelineId = 0;
	bool m_StateDirty = true;
	std::array<std::uint64_t, kMaxBindGroups> m_Groups{};
	std::array<Binding, kMaxVertexSlots> m_VertexBuffers{};
	Binding m_Index;
	IndexFormat m_IndexFormat = IndexFormat::kUint16;
	std::array<std::byte, kMaxDrawConstantBytes> m_Constants{};
};

void MetalDevice::Execute( id<MTLCommandBuffer> commands, std::vector<MetalEncoder *> &encoders )
{
	Replay replay( *this, commands );
	replay.Run( encoders );
}

} // namespace render::device::metal
