//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl: submission. Validate checks one encoder's list
//			against the port's rules and the usage state (as the Vulkan
//			adapter does at Submit); Replay runs the accepted lists in order on
//			the GL context.
//
//=============================================================================//

#include "gl_device.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace render::device::gl
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

bool GlDevice::GroupsMatch( const ValidationState &state ) const
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

bool GlDevice::ValidateDraw( const ValidationState &state, bool indexed ) const
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

bool GlDevice::Validate(
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
		// D24's depth has no 32-bit transfer equal to the port's (formats.cpp).
		if ( copy.mip >= t.desc.mipLevels || copy.layer >= t.layers || copy.width == 0 ||
		     copy.height == 0 || t.desc.sampleCount != 1 || t.desc.format == Format::kD24UnormS8 ||
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
			const TextureRecord *t = texture( command.a, ResourceUsage::kCopyDestination );
			if ( !t || IsBlockCompressed( t->desc.format ) || t->desc.sampleCount != 1 )
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
		}
	}
	return true;
}

// Framebuffers ------------------------------------------------------------------

GLuint GlDevice::Framebuffer( const std::vector<std::uint64_t> &colors, std::uint64_t depth )
{
	std::vector<std::uint64_t> key = colors;
	key.push_back( depth );
	if ( const auto found = m_Framebuffers.find( key ); found != m_Framebuffers.end() )
		return found->second;
	const GlApi &gl = Gl();
	GLuint framebuffer = 0;
	gl.CreateFramebuffers( 1, &framebuffer );
	auto attach = [&]( GLenum attachment, const TextureRecord &texture )
	{
		// Mip 0 and layer 0: the port attaches whole 2D textures.
		if ( texture.target == GL_TEXTURE_2D || texture.target == GL_TEXTURE_2D_MULTISAMPLE )
			gl.NamedFramebufferTexture( framebuffer, attachment, texture.name, 0 );
		else
			gl.NamedFramebufferTextureLayer( framebuffer, attachment, texture.name, 0, 0 );
	};
	std::vector<GLenum> buffers;
	for ( std::size_t i = 0; i < colors.size(); ++i )
	{
		const GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>( i );
		attach( attachment, *ExistingTexture( colors[i] ) );
		buffers.push_back( attachment );
	}
	if ( depth )
	{
		const TextureRecord &texture = *ExistingTexture( depth );
		attach(
		    HasStencil( texture.desc.format ) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT,
		    texture );
	}
	if ( buffers.empty() )
	{
		gl.NamedFramebufferDrawBuffers( framebuffer, 1, std::array<GLenum, 1>{ GL_NONE }.data() );
		gl.NamedFramebufferReadBuffer( framebuffer, GL_NONE );
	}
	else
	{
		gl.NamedFramebufferDrawBuffers(
		    framebuffer, static_cast<GLsizei>( buffers.size() ), buffers.data() );
		gl.NamedFramebufferReadBuffer( framebuffer, GL_COLOR_ATTACHMENT0 );
	}
	m_Framebuffers.emplace( std::move( key ), framebuffer );
	return framebuffer;
}

void GlDevice::ForgetFramebuffers( std::uint64_t texture )
{
	for ( auto it = m_Framebuffers.begin(); it != m_Framebuffers.end(); )
	{
		bool uses = false;
		for ( std::uint64_t id : it->first )
			uses |= id == texture;
		if ( !uses )
		{
			++it;
			continue;
		}
		Gl().DeleteFramebuffers( 1, &it->second );
		it = m_Framebuffers.erase( it );
	}
}

// Replay ------------------------------------------------------------------------

namespace
{

float LinearToSrgb( float value )
{
	const float c = std::clamp( value, 0.0f, 1.0f );
	return c <= 0.0031308f ? c * 12.92f : 1.055f * std::pow( c, 1.0f / 2.4f ) - 0.055f;
}

GLenum StencilOperation( StencilOp op )
{
	switch ( op )
	{
	case StencilOp::kKeep:
		return GL_KEEP;
	case StencilOp::kZero:
		return GL_ZERO;
	case StencilOp::kReplace:
		return GL_REPLACE;
	case StencilOp::kIncrementClamp:
		return GL_INCR;
	case StencilOp::kDecrementClamp:
		return GL_DECR;
	case StencilOp::kInvert:
		return GL_INVERT;
	case StencilOp::kIncrementWrap:
		return GL_INCR_WRAP;
	case StencilOp::kDecrementWrap:
		return GL_DECR_WRAP;
	}
	return GL_KEEP;
}

} // namespace

// Runs one submission's encoders on the context, in order.
class Replay
{
public:
	Replay( GlDevice &device, CompletionToken token )
	    : m_D( device ), m_Gl( device.Gl() ), m_Token( token )
	{
	}

	void Run( std::vector<GlEncoder *> &encoders )
	{
		PrepareConstants( encoders );
		for ( GlEncoder *encoder : encoders )
		{
			m_Pipeline = nullptr;
			m_Groups.fill( 0 );
			m_VertexBuffers.fill( {} );
			m_Index = {};
			for ( const Command &command : encoder->Commands() )
				Execute( command );
		}
		if ( m_Framebuffer )
			m_Gl.BindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
		if ( const GLenum error = m_Gl.GetError(); error != GL_NO_ERROR )
		{
			std::fprintf( stderr, "render.device.gl: submission %llu left GL error 0x%x\n",
			    static_cast<unsigned long long>( m_Token.value ), error );
			m_D.CountMessage();
		}
	}

private:
	struct Binding
	{
		std::uint64_t buffer = 0;
		std::uint64_t offset = 0;
	};

	// The draw constants of every draw and dispatch, in one buffer for the
	// submission (D16: each draw sees the block as it was set before it).
	void PrepareConstants( std::vector<GlEncoder *> &encoders )
	{
		const std::uint32_t alignment = std::max( m_D.m_Facts.limits.uniformBufferAlignment, 16u );
		m_Stride = ( kMaxDrawConstantBytes + alignment - 1 ) / alignment * alignment;
		std::vector<std::byte> data;
		for ( GlEncoder *encoder : encoders )
		{
			std::array<std::byte, kMaxDrawConstantBytes> block{};
			std::uint32_t bytes = 0;
			for ( const Command &command : encoder->Commands() )
			{
				if ( command.op == Op::kSetPipeline )
					bytes = m_D.m_Pipelines.find( command.a )->second.drawConstantBytes;
				else if ( command.op == Op::kSetDrawConstants )
					std::memcpy(
					    block.data() + command.offset, command.bytes.data(), command.bytes.size() );
				else if ( ( command.op == Op::kDraw || command.op == Op::kDrawIndexed ||
				              command.op == Op::kDispatch ) &&
				          bytes > 0 )
				{
					const std::size_t at = data.size();
					data.resize( at + m_Stride );
					std::memcpy( data.data() + at, block.data(), kMaxDrawConstantBytes );
				}
			}
		}
		if ( data.empty() )
			return;
		m_Gl.CreateBuffers( 1, &m_Constants );
		m_Gl.NamedBufferStorage(
		    m_Constants, static_cast<GLsizeiptr>( data.size() ), data.data(), 0 );
		m_D.m_Transients.push_back( { m_Constants, m_Token.value } );
	}

	void Execute( const Command &command )
	{
		switch ( command.op )
		{
		case Op::kTransitionTexture:
		case Op::kTransitionBuffer:
			// Writes through storage bindings are incoherent in GL; everything
			// else GL orders itself.
			if ( command.before == ResourceUsage::kStorageWrite )
				m_Gl.MemoryBarrier( GL_ALL_BARRIER_BITS );
			break;
		case Op::kClearTexture:
			ClearTexture( command );
			break;
		case Op::kWriteBuffer:
		{
			const BufferRecord *b = m_D.ExistingBuffer( command.a );
			if ( command.fromRing )
				m_Gl.CopyNamedBufferSubData( m_D.m_RingBuffer, b->name,
				    static_cast<GLintptr>( command.ringOffset ),
				    static_cast<GLintptr>( command.copy.destinationOffset ),
				    static_cast<GLsizeiptr>( command.copy.size ) );
			else
				m_Gl.NamedBufferSubData( b->name,
				    static_cast<GLintptr>( command.copy.destinationOffset ),
				    static_cast<GLsizeiptr>( command.copy.size ), command.bytes.data() );
			break;
		}
		case Op::kCopyBuffer:
			m_Gl.CopyNamedBufferSubData( m_D.ExistingBuffer( command.a )->name,
			    m_D.ExistingBuffer( command.b )->name,
			    static_cast<GLintptr>( command.copy.sourceOffset ),
			    static_cast<GLintptr>( command.copy.destinationOffset ),
			    static_cast<GLsizeiptr>( command.copy.size ) );
			break;
		case Op::kCopyTextureToBuffer:
			CopyTextureToBuffer( command );
			break;
		case Op::kCopyBufferToTexture:
			CopyBufferToTexture( command );
			break;
		case Op::kBeginRendering:
			BeginRendering( command );
			break;
		case Op::kEndRendering:
			EndRendering();
			break;
		case Op::kSetPipeline:
			m_Pipeline = &m_D.m_Pipelines.find( command.a )->second;
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
			m_Gl.ViewportIndexedf( 0, command.viewport.x, command.viewport.y,
			    command.viewport.width, command.viewport.height );
			m_Gl.DepthRangeIndexed( 0, command.viewport.minDepth, command.viewport.maxDepth );
			break;
		case Op::kDraw:
		case Op::kDrawIndexed:
			Draw( command );
			break;
		case Op::kDispatch:
			Dispatch( command );
			break;
		case Op::kSetDrawConstants:
			break;
		case Op::kBeginLabel:
			m_Gl.PushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0,
			    static_cast<GLsizei>( command.bytes.size() ),
			    reinterpret_cast<const GLchar *>( command.bytes.data() ) );
			break;
		case Op::kEndLabel:
			m_Gl.PopDebugGroup();
			break;
		case Op::kWriteTimestamp:
		{
			// The GPU writes the result into the buffer when it is available.
			const BufferRecord &b = *m_D.ExistingBuffer( command.a );
			GLuint query = 0;
			m_Gl.CreateQueries( GL_TIMESTAMP, 1, &query );
			m_Gl.QueryCounter( query, GL_TIMESTAMP );
			m_Gl.GetQueryBufferObjectui64v(
			    query, b.name, GL_QUERY_RESULT, static_cast<GLintptr>( command.offset ) );
			m_D.m_Transients.push_back( { query, m_Token.value, true } );
			break;
		}
		}
	}

	void ClearTexture( const Command &command )
	{
		const TextureRecord &t = *m_D.ExistingTexture( command.a );
		const Format format = t.desc.format;
		GLenum dataFormat = GL_RGBA;
		GLenum type = GL_FLOAT;
		float color[4] = { command.color.r, command.color.g, command.color.b, command.color.a };
		struct DepthStencil
		{
			float depth;
			std::uint32_t stencil;
		} depthStencil{ command.color.r, 0 };
		const void *data = color;
		if ( IsDepthFormat( format ) )
		{
			if ( HasStencil( format ) )
			{
				dataFormat = GL_DEPTH_STENCIL;
				type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
				data = &depthStencil;
			}
			else
			{
				dataFormat = GL_DEPTH_COMPONENT;
			}
		}
		else if ( format == Format::kRGBA8Srgb || format == Format::kBGRA8Srgb )
		{
			// A texture clear writes texels as given: the port's clear color
			// is linear, as an attachment clear's.
			for ( int i = 0; i < 3; ++i )
				color[i] = LinearToSrgb( color[i] );
		}
		const SubresourceRange &r = command.range;
		const std::uint32_t mips = std::min( r.baseMip + r.mipCount, t.desc.mipLevels );
		for ( std::uint32_t mip = r.baseMip; mip < mips; ++mip )
		{
			const auto width = static_cast<GLsizei>( t.Width( mip ) );
			const auto height = static_cast<GLsizei>( t.Height( mip ) );
			if ( t.desc.dimension == TextureDimension::k3D )
			{
				const auto depth =
				    static_cast<GLsizei>( std::max( 1u, t.desc.depthOrLayers >> mip ) );
				m_Gl.ClearTexSubImage( t.name, static_cast<GLint>( mip ), 0, 0, 0, width, height,
				    depth, dataFormat, type, data );
				continue;
			}
			const std::uint32_t layers = std::min( r.baseLayer + r.layerCount, t.layers );
			if ( layers > r.baseLayer )
				m_Gl.ClearTexSubImage( t.name, static_cast<GLint>( mip ), 0, 0,
				    static_cast<GLint>( r.baseLayer ), width, height,
				    static_cast<GLsizei>( layers - r.baseLayer ), dataFormat, type, data );
		}
	}

	void CopyTextureToBuffer( const Command &command )
	{
		const TextureRecord &t = *m_D.ExistingTexture( command.a );
		const BufferRecord &b = *m_D.ExistingBuffer( command.b );
		const TextureBufferCopy &copy = command.textureCopy;
		const GlFormat format = FormatOf( t.desc.format );
		const auto bytes =
		    static_cast<GLsizei>( RegionBytes( t.desc.format, copy.width, copy.height ) );
		void *offset = reinterpret_cast<void *>( static_cast<std::uintptr_t>( copy.bufferOffset ) );
		m_Gl.BindBuffer( GL_PIXEL_PACK_BUFFER, b.name );
		const auto x = static_cast<GLint>( copy.x );
		const auto y = static_cast<GLint>( copy.y );
		if ( format.compressed )
			m_Gl.GetCompressedTextureSubImage( t.name, static_cast<GLint>( copy.mip ), x, y,
			    static_cast<GLint>( copy.layer ), static_cast<GLsizei>( copy.width ),
			    static_cast<GLsizei>( copy.height ), 1, bytes, offset );
		else
			m_Gl.GetTextureSubImage( t.name, static_cast<GLint>( copy.mip ), x, y,
			    static_cast<GLint>( copy.layer ), static_cast<GLsizei>( copy.width ),
			    static_cast<GLsizei>( copy.height ), 1, format.format, format.type, bytes, offset );
		m_Gl.BindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
	}

	void CopyBufferToTexture( const Command &command )
	{
		const BufferRecord &b = *m_D.ExistingBuffer( command.a );
		const TextureRecord &t = *m_D.ExistingTexture( command.b );
		const TextureBufferCopy &copy = command.textureCopy;
		const GlFormat format = FormatOf( t.desc.format );
		const auto bytes =
		    static_cast<GLsizei>( RegionBytes( t.desc.format, copy.width, copy.height ) );
		const void *offset =
		    reinterpret_cast<const void *>( static_cast<std::uintptr_t>( copy.bufferOffset ) );
		const auto mip = static_cast<GLint>( copy.mip );
		const auto width = static_cast<GLsizei>( copy.width );
		const auto height = static_cast<GLsizei>( copy.height );
		const auto x = static_cast<GLint>( copy.x );
		const auto y = static_cast<GLint>( copy.y );
		m_Gl.BindBuffer( GL_PIXEL_UNPACK_BUFFER, b.name );
		if ( t.target == GL_TEXTURE_2D )
		{
			if ( format.compressed )
				m_Gl.CompressedTextureSubImage2D(
				    t.name, mip, x, y, width, height, format.internal, bytes, offset );
			else
				m_Gl.TextureSubImage2D(
				    t.name, mip, x, y, width, height, format.format, format.type, offset );
		}
		else
		{
			const auto layer = static_cast<GLint>( copy.layer );
			if ( format.compressed )
				m_Gl.CompressedTextureSubImage3D(
				    t.name, mip, x, y, layer, width, height, 1, format.internal, bytes, offset );
			else
				m_Gl.TextureSubImage3D( t.name, mip, x, y, layer, width, height, 1, format.format,
				    format.type, offset );
		}
		m_Gl.BindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
	}

	// Clears and blits obey masks and the scissor: open them all, and apply
	// the pipeline's state again at the next draw.
	void OpenMasks( std::size_t colors )
	{
		for ( std::size_t i = 0; i < colors; ++i )
			m_Gl.ColorMaski( static_cast<GLuint>( i ), GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
		m_Gl.DepthMask( GL_TRUE );
		m_StateDirty = true;
	}

	void BeginRendering( const Command &command )
	{
		m_Rendering = &command;
		std::vector<std::uint64_t> colors;
		for ( const ColorAttachment &color : command.colors )
			colors.push_back( color.texture.value );
		const std::uint64_t depth = command.depth ? command.depth->texture.value : 0;
		m_Framebuffer = m_D.Framebuffer( colors, depth );
		m_Gl.BindFramebuffer( GL_DRAW_FRAMEBUFFER, m_Framebuffer );
		// The render area: attachments may be larger than the pass.
		m_Gl.Enable( GL_SCISSOR_TEST );
		m_Gl.ScissorIndexed( 0, 0, 0, static_cast<GLsizei>( command.width ),
		    static_cast<GLsizei>( command.height ) );
		OpenMasks( command.colors.size() );
		std::vector<GLenum> discard;
		for ( std::size_t i = 0; i < command.colors.size(); ++i )
		{
			const ColorAttachment &color = command.colors[i];
			if ( color.load == LoadOp::kClear )
			{
				const float value[4] = {
				    color.clear.r, color.clear.g, color.clear.b, color.clear.a };
				m_Gl.ClearNamedFramebufferfv(
				    m_Framebuffer, GL_COLOR, static_cast<GLint>( i ), value );
			}
			else if ( color.load == LoadOp::kDiscard )
			{
				discard.push_back( GL_COLOR_ATTACHMENT0 + static_cast<GLenum>( i ) );
			}
		}
		if ( command.depth )
		{
			const bool stencil = HasStencil( m_D.ExistingTexture( depth )->desc.format );
			if ( command.depth->load == LoadOp::kClear )
			{
				if ( stencil )
					m_Gl.ClearNamedFramebufferfi(
					    m_Framebuffer, GL_DEPTH_STENCIL, 0, command.depth->clearDepth, 0 );
				else
					m_Gl.ClearNamedFramebufferfv(
					    m_Framebuffer, GL_DEPTH, 0, &command.depth->clearDepth );
			}
			else if ( command.depth->load == LoadOp::kDiscard )
			{
				discard.push_back( stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT );
			}
		}
		if ( !discard.empty() )
			m_Gl.InvalidateNamedFramebufferData(
			    m_Framebuffer, static_cast<GLsizei>( discard.size() ), discard.data() );
		m_Gl.ViewportIndexedf( 0, 0.0f, 0.0f, static_cast<float>( command.width ),
		    static_cast<float>( command.height ) );
		m_Gl.DepthRangeIndexed( 0, 0.0, 1.0 );
	}

	void EndRendering()
	{
		const Command &command = *m_Rendering;
		m_Gl.Disable( GL_SCISSOR_TEST );
		std::vector<GLenum> discard;
		for ( std::size_t i = 0; i < command.colors.size(); ++i )
		{
			const ColorAttachment &color = command.colors[i];
			if ( color.resolve.IsValid() )
			{
				const GLuint source = m_D.Framebuffer( { color.texture.value }, 0 );
				const GLuint target = m_D.Framebuffer( { color.resolve.value }, 0 );
				const auto width = static_cast<GLint>( command.width );
				const auto height = static_cast<GLint>( command.height );
				m_Gl.BlitNamedFramebuffer( source, target, 0, 0, width, height, 0, 0, width, height,
				    GL_COLOR_BUFFER_BIT, GL_NEAREST );
			}
			if ( color.store == StoreOp::kDiscard )
				discard.push_back( GL_COLOR_ATTACHMENT0 + static_cast<GLenum>( i ) );
		}
		if ( command.depth && command.depth->store == StoreOp::kDiscard )
			discard.push_back(
			    HasStencil( m_D.ExistingTexture( command.depth->texture.value )->desc.format )
			        ? GL_DEPTH_STENCIL_ATTACHMENT
			        : GL_DEPTH_ATTACHMENT );
		if ( !discard.empty() )
			m_Gl.InvalidateNamedFramebufferData(
			    m_Framebuffer, static_cast<GLsizei>( discard.size() ), discard.data() );
		m_Rendering = nullptr;
	}

	void ApplyState()
	{
		const PipelineRecord &p = *m_Pipeline;
		m_Gl.UseProgram( p.program->name );
		m_Gl.BindVertexArray( p.vertexArray );
		if ( p.raster.cull == CullMode::kNone )
		{
			m_Gl.Disable( GL_CULL_FACE );
		}
		else
		{
			m_Gl.Enable( GL_CULL_FACE );
			m_Gl.CullFace( p.raster.cull == CullMode::kBack ? GL_BACK : GL_FRONT );
		}
		// Facing is judged in clip space (Y up), where glClipControl's
		// upper-left origin keeps GL's own winding.
		m_Gl.FrontFace( p.raster.frontCounterClockwise ? GL_CCW : GL_CW );
		// GL and the port agree: no depth writes without the depth test.
		if ( p.depthStencil.depthTest )
			m_Gl.Enable( GL_DEPTH_TEST );
		else
			m_Gl.Disable( GL_DEPTH_TEST );
		m_Gl.DepthMask( p.depthStencil.depthWrite ? GL_TRUE : GL_FALSE );
		m_Gl.DepthFunc( CompareFunction( p.depthStencil.compare ) );
		const StencilState &stencil = p.depthStencil.stencil;
		if ( stencil.enabled )
			m_Gl.Enable( GL_STENCIL_TEST );
		else
			m_Gl.Disable( GL_STENCIL_TEST );
		m_Gl.StencilFunc( CompareFunction( stencil.compare ), stencil.reference, stencil.readMask );
		m_Gl.StencilMask( stencil.writeMask );
		m_Gl.StencilOp( StencilOperation( stencil.fail ), StencilOperation( stencil.depthFail ),
		    StencilOperation( stencil.pass ) );
		for ( std::size_t i = 0; i < p.colorFormats.size(); ++i )
		{
			const auto index = static_cast<GLuint>( i );
			BlendMode mode = p.blends.empty() ? BlendMode::kOpaque : p.blends[i];
			if ( mode == BlendMode::kTransmittance &&
			     m_D.m_Options.sensitivity.transmittanceAsPremultiplied )
				mode = BlendMode::kPremultiplied;
			if ( mode == BlendMode::kOpaque )
			{
				m_Gl.Disablei( GL_BLEND, index );
			}
			else
			{
				m_Gl.Enablei( GL_BLEND, index );
				m_Gl.BlendEquationSeparatei( index, GL_FUNC_ADD, GL_FUNC_ADD );
				switch ( mode )
				{
				case BlendMode::kAlpha:
					m_Gl.BlendFuncSeparatei( index, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
					    GL_ONE_MINUS_SRC_ALPHA );
					break;
				case BlendMode::kPremultiplied:
					m_Gl.BlendFuncSeparatei(
					    index, GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
					break;
				case BlendMode::kTransmittance:
					m_Gl.BlendFuncSeparatei( index, GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_ONE );
					break;
				case BlendMode::kAdditive:
				case BlendMode::kOpaque:
					m_Gl.BlendFuncSeparatei( index, GL_ONE, GL_ONE, GL_ONE, GL_ONE );
					break;
				}
			}
			const std::uint8_t mask =
			    p.writeMasks.empty() || m_D.m_Options.sensitivity.ignoreColorWriteMasks
			        ? kColorWriteAll
			        : p.writeMasks[i];
			m_Gl.ColorMaski( index, ( mask & kColorWriteRed ) != 0,
			    ( mask & kColorWriteGreen ) != 0, ( mask & kColorWriteBlue ) != 0,
			    ( mask & kColorWriteAlpha ) != 0 );
		}
		m_StateDirty = false;
	}

	// Binds the groups the program reads to their flat slots; true when a
	// storage binding may be written.
	bool BindGroups()
	{
		const PipelineRecord &p = *m_Pipeline;
		bool storage = false;
		for ( std::uint32_t role = 0; role < kMaxBindGroups; ++role )
		{
			if ( !p.layoutHasBindings[role] )
				continue;
			const BindGroupRecord &group = m_D.m_BindGroups.find( m_Groups[role] )->second;
			for ( const BindGroupEntry &entry : group.entries )
			{
				const BindingDesc *binding = nullptr;
				for ( const BindingDesc &candidate : group.bindings )
				{
					if ( candidate.binding == entry.binding )
						binding = &candidate;
				}
				const GLuint slot = FlatSlot( role, entry.binding );
				switch ( binding->kind )
				{
				case BindingKind::kUniformBuffer:
				case BindingKind::kStorageBuffer:
				{
					const BufferRecord &b = *m_D.ExistingBuffer( entry.buffer.value );
					const std::uint64_t size = entry.size ? entry.size : b.desc.size - entry.offset;
					const bool uniform = binding->kind == BindingKind::kUniformBuffer;
					m_Gl.BindBufferRange( uniform ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER,
					    slot, b.name, static_cast<GLintptr>( entry.offset ),
					    static_cast<GLsizeiptr>( size ) );
					storage |= !uniform;
					break;
				}
				case BindingKind::kSampledTexture:
					m_Gl.BindTextureUnit( slot, m_D.ExistingTexture( entry.texture.value )->name );
					break;
				case BindingKind::kStorageTexture:
				{
					const TextureRecord &t = *m_D.ExistingTexture( entry.texture.value );
					m_Gl.BindImageTexture( slot, t.name, 0, t.target != GL_TEXTURE_2D, 0,
					    GL_READ_WRITE, FormatOf( t.desc.format ).internal );
					storage = true;
					break;
				}
				case BindingKind::kSampler:
					break;
				}
			}
		}
		// Each combined sampler takes its texture's unit and the sampler it
		// was built with.
		for ( const Program::Combined &combined : p.program->combined )
		{
			GLuint sampler = 0;
			if ( combined.sampler && combined.samplerGroup < kMaxBindGroups )
			{
				const auto group = m_D.m_BindGroups.find( m_Groups[combined.samplerGroup] );
				if ( group != m_D.m_BindGroups.end() )
				{
					for ( const BindGroupEntry &entry : group->second.entries )
					{
						if ( entry.binding == combined.samplerBinding && entry.sampler.IsValid() )
							sampler = m_D.m_Samplers.find( entry.sampler.value )->second.name;
					}
				}
			}
			m_Gl.BindSampler( static_cast<GLuint>( combined.unit ), sampler );
		}
		return storage;
	}

	void BindConstants()
	{
		if ( m_Pipeline->drawConstantBytes == 0 )
			return;
		m_Gl.BindBufferRange( GL_UNIFORM_BUFFER, kDrawConstantsSlot, m_Constants,
		    static_cast<GLintptr>( m_NextConstants * m_Stride ), kMaxDrawConstantBytes );
		++m_NextConstants;
	}

	void Draw( const Command &command )
	{
		if ( m_StateDirty )
			ApplyState();
		const PipelineRecord &p = *m_Pipeline;
		for ( std::uint32_t slot = 0; slot < p.vertexBuffers; ++slot )
		{
			const Binding &binding = m_VertexBuffers[slot];
			m_Gl.VertexArrayVertexBuffer( p.vertexArray, slot,
			    m_D.ExistingBuffer( binding.buffer )->name, static_cast<GLintptr>( binding.offset ),
			    static_cast<GLsizei>( p.strides[slot] ) );
		}
		const bool storage = BindGroups();
		BindConstants();
		if ( p.program->baseInstance >= 0 )
			m_Gl.ProgramUniform1i( p.program->name, p.program->baseInstance,
			    static_cast<GLint>( command.firstInstance ) );
		if ( command.op == Op::kDraw )
		{
			m_Gl.DrawArraysInstancedBaseInstance( p.topology, static_cast<GLint>( command.first ),
			    static_cast<GLsizei>( command.count ), static_cast<GLsizei>( command.instances ),
			    command.firstInstance );
		}
		else
		{
			m_Gl.VertexArrayElementBuffer(
			    p.vertexArray, m_D.ExistingBuffer( m_Index.buffer )->name );
			const bool wide = m_IndexFormat == IndexFormat::kUint32;
			const std::uint64_t offset =
			    m_Index.offset + std::uint64_t( command.first ) * ( wide ? 4 : 2 );
			m_Gl.DrawElementsInstancedBaseVertexBaseInstance( p.topology,
			    static_cast<GLsizei>( command.count ), wide ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
			    reinterpret_cast<const void *>( static_cast<std::uintptr_t>( offset ) ),
			    static_cast<GLsizei>( command.instances ), command.vertexOffset,
			    command.firstInstance );
		}
		if ( storage )
			m_Gl.MemoryBarrier( GL_ALL_BARRIER_BITS );
	}

	void Dispatch( const Command &command )
	{
		m_Gl.UseProgram( m_Pipeline->program->name );
		m_StateDirty = true;
		const bool storage = BindGroups();
		BindConstants();
		m_Gl.DispatchCompute( command.count, command.first, command.firstInstance );
		// Commands in one usage run in order: a later dispatch reads this one's
		// storage writes.
		if ( storage )
			m_Gl.MemoryBarrier( GL_ALL_BARRIER_BITS );
	}

	GlDevice &m_D;
	const GlApi &m_Gl;
	CompletionToken m_Token;
	GLuint m_Constants = 0;
	std::uint32_t m_Stride = kMaxDrawConstantBytes;
	std::size_t m_NextConstants = 0;
	const PipelineRecord *m_Pipeline = nullptr;
	bool m_StateDirty = true;
	std::array<std::uint64_t, kMaxBindGroups> m_Groups{};
	std::array<Binding, kMaxVertexSlots> m_VertexBuffers{};
	Binding m_Index;
	IndexFormat m_IndexFormat = IndexFormat::kUint16;
	const Command *m_Rendering = nullptr;
	GLuint m_Framebuffer = 0;
};

void GlDevice::Execute( std::vector<GlEncoder *> &encoders, CompletionToken token )
{
	Replay replay( *this, token );
	replay.Run( encoders );
}

} // namespace render::device::gl
