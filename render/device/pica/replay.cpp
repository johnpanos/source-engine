//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica's replay: one validated encoder's commands in
//			order (pica_device.h, RFC 0026 decision 6). Copies, writes and
//			clears run on the CPU once the GPU has drained; passes and draws
//			go into the open citro3d frame. Every draw sets all the state it
//			uses.
//
//=============================================================================//

#if defined( __3DS__ )

#include "pica_device.h"

#include "render/device/pica_codes.h"

#include <algorithm>
#include <cstring>

namespace render::device::pica
{
namespace
{

// The command words one draw can emit at most, with margin: 96 uniform
// registers, six combiners, three texture units and the fixed function.
constexpr u32 kDrawCommandWords = 4096;

using recording::Command;
using recording::Op;

// The GPU's window y runs up from the framebuffer's last stored row (the
// port's bottom row); the port's viewport y runs down from row 0. Measured in
// Azahar, 2026-10-07 (render.device.v2.pica.raster's viewport checks).
std::uint32_t WindowY( std::uint32_t y, std::uint32_t height, std::uint32_t storedHeight )
{
	return y + height >= storedHeight ? 0 : storedHeight - ( y + height );
}

// PICA float uniforms are stored w z y x.
void WriteUniforms( std::uint32_t first, std::uint32_t count, const std::byte *bytes )
{
	C3D_FVec *registers = C3D_FVUnifWritePtr( GPU_VERTEX_SHADER, int( first ), int( count ) );
	for ( std::uint32_t r = 0; r < count; ++r )
	{
		float value[4];
		std::memcpy( value, bytes + r * kRegisterBytes, sizeof( value ) );
		registers[r].x = value[0];
		registers[r].y = value[1];
		registers[r].z = value[2];
		registers[r].w = value[3];
	}
}

} // namespace

class Replayer
{
public:
	explicit Replayer( PicaDevice &device ) : m_D( device ) {}

	void Run( const std::vector<Command> &commands )
	{
		for ( const Command &command : commands )
			Execute( command );
	}

private:
	void Execute( const Command &command )
	{
		switch ( command.op )
		{
		case Op::kWriteBuffer:
		{
			m_D.BeforeCpuAccess( { command.a } );
			BufferRecord &b = m_D.m_Buffers.at( command.a );
			std::byte *to = b.data + command.copy.destinationOffset;
			std::memcpy( to, command.bytes.data(), command.bytes.size() );
			GSPGPU_FlushDataCache( to, command.bytes.size() );
			break;
		}
		case Op::kCopyBuffer:
		{
			m_D.BeforeCpuAccess( { command.a, command.b } );
			BufferRecord &from = m_D.m_Buffers.at( command.a );
			BufferRecord &to = m_D.m_Buffers.at( command.b );
			std::byte *at = to.data + command.copy.destinationOffset;
			std::memmove( at, from.data + command.copy.sourceOffset, command.copy.size );
			GSPGPU_FlushDataCache( at, command.copy.size );
			break;
		}
		case Op::kClearTexture:
			ClearTexture( command );
			break;
		case Op::kCopyTextureToBuffer:
		{
			m_D.BeforeCpuAccess( { command.a, command.b } );
			const TextureRecord &t = m_D.m_Textures.at( command.a );
			BufferRecord &b = m_D.m_Buffers.at( command.b );
			const TextureBufferCopy &c = command.textureCopy;
			const LevelLayout &level = t.layout.levels[c.mip];
			std::byte *at = b.data + c.bufferOffset;
			CopyOut( t.desc.format, level, t.data + level.offset, c.x, c.y, c.width, c.height, at );
			GSPGPU_FlushDataCache( at, RegionBytes( t.desc.format, c.width, c.height ) );
			break;
		}
		case Op::kCopyBufferToTexture:
		{
			m_D.BeforeCpuAccess( { command.a, command.b } );
			TextureRecord &t = m_D.m_Textures.at( command.b );
			const BufferRecord &b = m_D.m_Buffers.at( command.a );
			const TextureBufferCopy &c = command.textureCopy;
			const LevelLayout &level = t.layout.levels[c.mip];
			if ( m_D.m_Options.sensitivity.texturesUpsideDown )
			{
				const std::size_t row = std::size_t( c.width ) * PortTexelBytes( t.desc.format );
				for ( std::uint32_t y = 0; y < c.height; ++y )
					CopyIn( t.desc.format, level, t.data + level.offset, c.x,
					    c.y + c.height - 1 - y, c.width, 1, b.data + c.bufferOffset + y * row );
			}
			else
				CopyIn( t.desc.format, level, t.data + level.offset, c.x, c.y, c.width, c.height,
				    b.data + c.bufferOffset );
			if ( m_D.m_Options.sensitivity.etc1BytesUnswapped && t.desc.format == Format::kETC1Rgb )
				for ( std::uint64_t at = 0; at + 8 <= level.bytes; at += 8 )
					std::reverse( t.data + level.offset + at, t.data + level.offset + at + 8 );
			GSPGPU_FlushDataCache( t.data + level.offset, std::size_t( level.bytes ) );
			break;
		}
		case Op::kCopyTexture:
			CopyTexture( command );
			break;
		case Op::kBeginRendering:
			BeginRendering( command );
			break;
		case Op::kEndRendering:
			m_Rendering = false;
			break;
		case Op::kSetPipeline:
			m_Pipeline = m_D.m_Pipelines.at( command.a ).get();
			break;
		case Op::kSetBindGroup:
			m_Groups[command.slot] = &m_D.m_BindGroups.at( command.a );
			break;
		case Op::kSetVertexBuffer:
			m_Vertex[command.slot] = { &m_D.m_Buffers.at( command.a ), command.offset, command.a };
			break;
		case Op::kSetIndexBuffer:
			m_Index = { &m_D.m_Buffers.at( command.a ), command.offset, command.a };
			break;
		case Op::kSetViewport:
			m_Viewport = command.viewport;
			break;
		case Op::kSetDrawConstants:
			std::memcpy(
			    m_Constants.data() + command.offset, command.bytes.data(), command.bytes.size() );
			break;
		case Op::kDraw:
		case Op::kDrawIndexed:
			Draw( command );
			break;
		case Op::kTransitionTexture:
		case Op::kTransitionBuffer:
		case Op::kBeginLabel:
		case Op::kEndLabel:
			break;
		// Refused before replay: no compute, timestamps or indirect draws.
		case Op::kDispatch:
		case Op::kWriteTimestamp:
		case Op::kDrawIndexedIndirect:
		case Op::kDrawIndexedIndirectCount:
			break;
		}
	}

	static void Fill( const TextureRecord &t, std::uint32_t mip, std::uint32_t width,
	    std::uint32_t height, std::uint32_t word )
	{
		const LevelLayout &level = t.layout.levels[mip];
		std::byte *base = t.data + level.offset;
		std::byte bytes[4];
		for ( int i = 0; i < 4; ++i )
			bytes[i] = std::byte( word >> ( 8 * i ) );
		for ( std::uint32_t y = 0; y < height; ++y )
			for ( std::uint32_t x = 0; x < width; ++x )
				std::memcpy(
				    base + std::size_t( TiledIndex( x, y, level.storedWidth ) ) * 4, bytes, 4 );
		GSPGPU_FlushDataCache( base, std::size_t( level.bytes ) );
	}

	static std::uint32_t ClearOf( const TextureRecord &t, const ClearColor &color )
	{
		if ( IsDepthFormat( t.desc.format ) )
			return DepthClearWord( color.r, 0 );
		const float rgba[4] = { color.r, color.g, color.b, color.a };
		return ClearWord( t.desc.format, rgba );
	}

	void ClearTexture( const Command &command )
	{
		m_D.BeforeCpuAccess( { command.a } );
		const TextureRecord &t = m_D.m_Textures.at( command.a );
		const std::uint32_t word = ClearOf( t, command.color );
		const std::uint32_t last =
		    std::min( t.layout.levelCount, command.range.baseMip + command.range.mipCount );
		for ( std::uint32_t mip = command.range.baseMip; mip < last; ++mip )
			Fill( t, mip, t.layout.levels[mip].width, t.layout.levels[mip].height, word );
	}

	void CopyTexture( const Command &command )
	{
		m_D.BeforeCpuAccess( { command.a, command.b } );
		const TextureRecord &from = m_D.m_Textures.at( command.a );
		TextureRecord &to = m_D.m_Textures.at( command.b );
		const TextureBufferCopy &c = command.textureCopy;
		const LevelLayout &source = from.layout.levels[c.mip];
		const LevelLayout &target = to.layout.levels[c.mip];
		CopyBetween( from.desc.format, source, from.data + source.offset, target,
		    to.data + target.offset, c.x, c.y, c.width, c.height );
		GSPGPU_FlushDataCache( to.data + target.offset, std::size_t( target.bytes ) );
	}

	void BeginRendering( const Command &command )
	{
		const std::uint64_t colorId = command.colors.empty() ? 0 : command.colors[0].texture.value;
		const std::uint64_t depthId = command.depth ? command.depth->texture.value : 0;
		// The load clears run on the CPU, before this pass's draws.
		if ( ( !command.colors.empty() && command.colors[0].load == LoadOp::kClear ) ||
		     ( command.depth && command.depth->load == LoadOp::kClear ) )
			m_D.BeforeCpuAccess( { colorId, depthId } );
		m_Color = colorId ? &m_D.m_Textures.at( colorId ) : nullptr;
		m_Depth = depthId ? &m_D.m_Textures.at( depthId ) : nullptr;
		for ( std::uint64_t id : { colorId, depthId } )
			if ( id )
				m_D.m_GpuUses.insert( id );
		m_Width = command.width;
		m_Height = command.height;
		if ( m_Color && command.colors[0].load == LoadOp::kClear )
			Fill( *m_Color, 0, m_Width, m_Height, ClearOf( *m_Color, command.colors[0].clear ) );
		if ( m_Depth && command.depth->load == LoadOp::kClear )
			Fill( *m_Depth, 0, m_Width, m_Height, DepthClearWord( command.depth->clearDepth, 0 ) );
		const TextureRecord &any = m_Color ? *m_Color : *m_Depth;
		m_StoredWidth = any.layout.levels[0].storedWidth;
		m_StoredHeight = any.layout.levels[0].storedHeight;
		std::memset( &m_Target, 0, sizeof( m_Target ) );
		C3D_FrameBuf &fb = m_Target.frameBuf;
		C3D_FrameBufAttrib( &fb, u16( m_StoredWidth ), u16( m_StoredHeight ), false );
		C3D_FrameBufColor( &fb, m_Color ? m_Color->data : nullptr, GPU_RB_RGBA8 );
		C3D_FrameBufDepth( &fb, m_Depth ? m_Depth->data : nullptr, GPU_RB_DEPTH24_STENCIL8 );
		m_Viewport = { 0, 0, float( m_Width ), float( m_Height ), 0, 1 };
		m_Rendering = true;
		DrawOn();
	}

	// The open frame draws on this pass's framebuffer.
	void DrawOn()
	{
		m_D.OpenFrame();
		m_D.m_Targets.push_back( m_Target );
		C3D_FrameDrawOn( &m_D.m_Targets.back() );
	}

	const BindGroupEntry *Entry( std::uint32_t group, std::uint32_t binding ) const
	{
		if ( group >= kMaxBindGroups || !m_Groups[group] )
			return nullptr;
		for ( const BindGroupEntry &entry : m_Groups[group]->entries )
			if ( entry.binding == binding )
				return &entry;
		return nullptr;
	}

	// The bytes of a uniform-buffer binding from `offset`, zero beyond it.
	void ReadUniform(
	    const BindGroupEntry *entry, std::uint64_t offset, std::byte *out, std::size_t bytes ) const
	{
		std::memset( out, 0, bytes );
		if ( !entry )
			return;
		const BufferRecord &b = m_D.m_Buffers.at( entry->buffer.value );
		const std::uint64_t start = entry->offset + offset;
		const std::uint64_t end =
		    entry->size ? std::min( b.desc.size, entry->offset + entry->size ) : b.desc.size;
		if ( start >= end )
			return;
		std::memcpy(
		    out, b.data + start, std::size_t( std::min<std::uint64_t>( bytes, end - start ) ) );
	}

	std::uint32_t StageConstant( const CombinerStage &stage ) const
	{
		float value[4] = {};
		switch ( stage.constantKind )
		{
		case ConstantKind::kLiteral:
			return stage.constantColor;
		case ConstantKind::kDrawConstants:
			if ( !m_D.m_Options.sensitivity.dropDrawConstants )
				std::memcpy( value, m_Constants.data() + stage.constantOffset, sizeof( value ) );
			break;
		case ConstantKind::kUniformBuffer:
			ReadUniform( Entry( stage.constantGroup, stage.constantBinding ), stage.constantOffset,
			    reinterpret_cast<std::byte *>( value ), sizeof( value ) );
			break;
		}
		return PackConstant( value );
	}

	void SetAttributes( const Command &command )
	{
		const PipelineRecord &p = *m_Pipeline;
		const std::int64_t base = command.op == Op::kDrawIndexed ? command.vertexOffset : 0;
		C3D_AttrInfo *attributes = C3D_GetAttrInfo();
		AttrInfo_Init( attributes );
		C3D_BufInfo *buffers = C3D_GetBufInfo();
		BufInfo_Init( buffers );
		for ( std::uint32_t slot = 0; slot < p.vertexBuffers; ++slot )
		{
			const auto &loads = p.slotAttributes[slot];
			if ( loads.empty() )
				continue;
			u64 permutation = 0;
			int entries = 0;
			std::uint32_t at = 0;
			auto pad = [&]( std::uint32_t bytes )
			{
				// Padding entries skip 1 to 4 words (codes 0xC to 0xF).
				for ( std::uint32_t words = bytes / 4; words > 0; )
				{
					const std::uint32_t take = std::min<std::uint32_t>( words, 4u );
					permutation |= u64( 0xB + take ) << ( 4 * entries++ );
					words -= take;
				}
			};
			for ( const AttributeLoad &load : loads )
			{
				pad( load.offset - at );
				const int loader =
				    AttrInfo_AddLoader( attributes, load.location, load.format, load.components );
				permutation |= u64( loader ) << ( 4 * entries++ );
				at = load.offset + load.bytes;
			}
			const VertexSlot &vertex = m_Vertex[slot];
			const std::byte *data =
			    vertex.buffer->data + vertex.offset + base * std::int64_t( p.strides[slot] );
			BufInfo_Add( buffers, data, p.strides[slot], entries, permutation );
		}
		if ( p.vertex.vertexIndexRegister != kNone )
		{
			const int loader =
			    AttrInfo_AddLoader( attributes, p.vertex.vertexIndexRegister, GPU_FLOAT, 1 );
			BufInfo_Add(
			    buffers, Shared().vertexIndices + base, sizeof( float ), 1, u64( loader ) );
		}
	}

	void SetUniforms()
	{
		const PipelineRecord &p = *m_Pipeline;
		std::byte bytes[kFloatUniformRegisters * kRegisterBytes];
		for ( const UniformRange &range : p.vertex.uniforms )
		{
			ReadUniform( Entry( range.group, range.binding ), 0, bytes,
			    std::size_t( range.registerCount ) * kRegisterBytes );
			WriteUniforms( range.firstRegister, range.registerCount, bytes );
		}
		if ( p.vertex.drawConstantRegister != kNone && p.drawConstantBytes > 0 )
		{
			const std::uint32_t registers =
			    ( p.drawConstantBytes + kRegisterBytes - 1 ) / kRegisterBytes;
			std::memset( bytes, 0, std::size_t( registers ) * kRegisterBytes );
			std::memcpy( bytes, m_Constants.data(), p.drawConstantBytes );
			WriteUniforms( p.vertex.drawConstantRegister, registers, bytes );
		}
	}

	void SetCombiners()
	{
		const PipelineRecord &p = *m_Pipeline;
		for ( int i = 0; i < int( kMaxCombinerStages ); ++i )
		{
			C3D_TexEnv *env = C3D_GetTexEnv( i );
			C3D_TexEnvInit( env );
			if ( i == 0 && p.stages.empty() )
			{
				C3D_TexEnvSrc(
				    env, C3D_Both, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR );
				C3D_TexEnvFunc( env, C3D_Both, GPU_REPLACE );
				continue;
			}
			if ( std::size_t( i ) >= p.stages.size() )
				continue; // C3D_TexEnvInit: the previous stage's result, unchanged
			const CombinerStage &s = p.stages[std::size_t( i )];
			C3D_TexEnvSrc( env, C3D_RGB, GPU_TEVSRC( s.rgbSources[0] ),
			    GPU_TEVSRC( s.rgbSources[1] ), GPU_TEVSRC( s.rgbSources[2] ) );
			C3D_TexEnvSrc( env, C3D_Alpha, GPU_TEVSRC( s.alphaSources[0] ),
			    GPU_TEVSRC( s.alphaSources[1] ), GPU_TEVSRC( s.alphaSources[2] ) );
			C3D_TexEnvOpRgb( env, GPU_TEVOP_RGB( s.rgbOperands[0] ),
			    GPU_TEVOP_RGB( s.rgbOperands[1] ), GPU_TEVOP_RGB( s.rgbOperands[2] ) );
			C3D_TexEnvOpAlpha( env, GPU_TEVOP_A( s.alphaOperands[0] ),
			    GPU_TEVOP_A( s.alphaOperands[1] ), GPU_TEVOP_A( s.alphaOperands[2] ) );
			C3D_TexEnvFunc( env, C3D_RGB, GPU_COMBINEFUNC( s.rgbCombine ) );
			C3D_TexEnvFunc( env, C3D_Alpha, GPU_COMBINEFUNC( s.alphaCombine ) );
			C3D_TexEnvScale( env, C3D_RGB, GPU_TEVSCALE( s.rgbScale ) );
			C3D_TexEnvScale( env, C3D_Alpha, GPU_TEVSCALE( s.alphaScale ) );
			C3D_TexEnvColor( env, StageConstant( s ) );
		}
		C3D_TexEnvBufUpdate( C3D_RGB, p.fragment.bufferWrite & 0x0F );
		C3D_TexEnvBufUpdate( C3D_Alpha, p.fragment.bufferWrite >> 4 );
		C3D_TexEnvBufColor( p.fragment.bufferColor );
	}

	void SetTextures()
	{
		const PipelineRecord &p = *m_Pipeline;
		for ( std::size_t unit = 0; unit < kMaxTextureUnits; ++unit )
		{
			if ( unit >= p.fragment.textures.size() )
			{
				C3D_TexBind( int( unit ), &Shared().placeholder );
				continue;
			}
			const TextureUnit &u = p.fragment.textures[unit];
			const BindGroupEntry *texture = Entry( u.textureGroup, u.textureBinding );
			const BindGroupEntry *sampler = Entry( u.samplerGroup, u.samplerBinding );
			if ( !texture || !sampler )
			{
				C3D_TexBind( int( unit ), &Shared().placeholder );
				continue;
			}
			const TextureRecord &t = m_D.m_Textures.at( texture->texture.value );
			m_D.m_GpuUses.insert( texture->texture.value );
			const SamplerDesc &s = m_D.m_Samplers.at( sampler->sampler.value ).desc;
			C3D_Tex &tex = Shared().units[unit];
			tex = t.sampled;
			const GPU_TEXTURE_WRAP_PARAM wrap =
			    s.address == AddressMode::kClampToEdge      ? GPU_CLAMP_TO_EDGE
			    : s.address == AddressMode::kMirroredRepeat ? GPU_MIRRORED_REPEAT
			                                                : GPU_REPEAT;
			const auto filter = []( Filter f )
			{
				return f == Filter::kLinear ? GPU_LINEAR : GPU_NEAREST;
			};
			tex.param = GPU_TEXTURE_MODE( GPU_TEX_2D ) |
			            GPU_TEXTURE_MAG_FILTER( filter( s.magFilter ) ) |
			            GPU_TEXTURE_MIN_FILTER( filter( s.minFilter ) ) |
			            GPU_TEXTURE_MIP_FILTER( filter( s.mipFilter ) ) |
			            GPU_TEXTURE_WRAP_S( wrap ) | GPU_TEXTURE_WRAP_T( wrap );
			C3D_TexBind( int( unit ), &tex );
		}
	}

	void SetFixedFunction()
	{
		const PipelineRecord &p = *m_Pipeline;
		const FragmentProgram &f = p.fragment;
		C3D_AlphaTest( f.alphaTest != kNone,
		    GPU_TESTFUNC( f.alphaTest != kNone ? f.alphaTest : test::kAlways ), f.alphaReference );
		// PVS1 writes clip z = -d w; the GPU stores z/w * scale + offset.
		const float minDepth = std::clamp( m_Viewport.minDepth, 0.0f, 1.0f );
		const float maxDepth = std::clamp( m_Viewport.maxDepth, 0.0f, 1.0f );
		const float depthSign = m_D.m_Options.sensitivity.depthNotNegated ? 1.0f : -1.0f;
		C3D_DepthMap( true, depthSign * ( maxDepth - minDepth ), minDepth );
		const DepthStencilState &ds = p.depthStencil;
		const bool depth = m_Depth != nullptr;
		const std::uint8_t colorMask =
		    m_D.m_Options.sensitivity.ignoreColorWriteMasks ? kColorWriteAll : p.writeMask;
		const int mask =
		    ( m_Color ? colorMask : 0 ) | ( depth && ds.depthWrite ? GPU_WRITE_DEPTH : 0 );
		C3D_DepthTest( depth && ( ds.depthTest || ds.depthWrite ),
		    GPU_TESTFUNC( ds.depthTest ? TestFunction( ds.compare ) : test::kAlways ),
		    GPU_WRITEMASK( mask ) );
		const StencilState &st = ds.stencil;
		C3D_StencilTest( depth && st.enabled, GPU_TESTFUNC( TestFunction( st.compare ) ),
		    st.reference, st.readMask, st.writeMask );
		C3D_StencilOp( GPU_STENCILOP( StencilOperation( st.fail ) ),
		    GPU_STENCILOP( StencilOperation( st.depthFail ) ),
		    GPU_STENCILOP( StencilOperation( st.pass ) ) );
		const BlendFactors blend =
		    m_D.m_Options.sensitivity.blendAsOpaque ? BlendFactors{} : p.blend;
		C3D_AlphaBlend( GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_BLENDFACTOR( blend.colorSource ),
		    GPU_BLENDFACTOR( blend.colorDestination ), GPU_BLENDFACTOR( blend.alphaSource ),
		    GPU_BLENDFACTOR( blend.alphaDestination ) );
		RasterState raster = p.raster;
		if ( m_D.m_Options.sensitivity.windingReversed )
			raster.frontCounterClockwise = !raster.frontCounterClockwise;
		C3D_CullFace( GPU_CULLMODE( render::device::pica_format::CullMode( raster ) ) );
		const auto x = std::uint32_t( std::max( 0.0f, m_Viewport.x ) );
		const auto y = std::uint32_t( std::max( 0.0f, m_Viewport.y ) );
		const auto w = std::uint32_t( std::max( 0.0f, m_Viewport.width ) );
		const auto h = std::uint32_t( std::max( 0.0f, m_Viewport.height ) );
		C3D_SetViewport( x, WindowY( y, h, m_StoredHeight ), w, h );
		// The render area: pixels outside it are not the pass's to touch,
		// though its attachments may be stored larger (whole tiles).
		const std::uint32_t areaTop = WindowY( 0, m_Height, m_StoredHeight );
		C3D_SetScissor( GPU_SCISSOR_NORMAL, 0, areaTop, m_Width, areaTop + m_Height );
	}

	void Draw( const Command &command )
	{
		if ( !m_Pipeline || command.count == 0 )
			return;
		// A frame whose command buffer is nearly full is submitted first: the
		// live offset (C3D_GetCmdBufUsage lagged a game frame into overflow),
		// with room for the largest draw's state (every uniform register).
		u32 *buffer = nullptr;
		u32 words = 0, used = 0;
		GPUCMD_GetBuffer( &buffer, &words, &used );
		if ( buffer && words - used < kDrawCommandWords )
		{
			m_D.Drain();
			DrawOn();
		}
		BindProgram( *m_Pipeline );
		for ( std::uint32_t slot = 0; slot < m_Pipeline->vertexBuffers; ++slot )
			if ( m_Vertex[slot].buffer )
				m_D.m_GpuUses.insert( m_Vertex[slot].id );
		if ( command.op == Op::kDrawIndexed )
			m_D.m_GpuUses.insert( m_Index.id );
		SetAttributes( command );
		SetUniforms();
		SetCombiners();
		SetTextures();
		SetFixedFunction();
		if ( command.op == Op::kDraw )
		{
			C3D_DrawArrays( m_Pipeline->topology, int( command.first ), int( command.count ) );
			return;
		}
		const std::byte *indices =
		    m_Index.buffer->data + m_Index.offset + std::size_t( command.first ) * 2;
		C3D_DrawElements( m_Pipeline->topology, int( command.count ), C3D_UNSIGNED_SHORT, indices );
	}

	struct VertexSlot
	{
		BufferRecord *buffer = nullptr;
		std::uint64_t offset = 0;
		std::uint64_t id = 0;
	};

	PicaDevice &m_D;
	PipelineRecord *m_Pipeline = nullptr;
	std::array<BindGroupRecord *, kMaxBindGroups> m_Groups{};
	std::array<VertexSlot, kInputRegisters> m_Vertex{};
	VertexSlot m_Index;
	Viewport m_Viewport;
	std::array<std::byte, kMaxDrawConstantBytes> m_Constants{};
	bool m_Rendering = false;
	TextureRecord *m_Color = nullptr;
	TextureRecord *m_Depth = nullptr;
	std::uint32_t m_Width = 0;
	std::uint32_t m_Height = 0;
	std::uint32_t m_StoredWidth = 0;
	std::uint32_t m_StoredHeight = 0;
	C3D_RenderTarget m_Target{};
};

void ReplayCommands( PicaDevice &device, const std::vector<Command> &commands )
{
	Replayer( device ).Run( commands );
}

} // namespace render::device::pica

#endif // __3DS__
