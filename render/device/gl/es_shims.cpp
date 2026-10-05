//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl: the OpenGL ES 3.1 dialect's entry points and DSA
//			shims (es_shims.h, RFC 0022).
//
//=============================================================================//

#include "es_shims.h"

#include <cstring>
#include <initializer_list>
#include <string>

namespace render::device::gl
{

EsState *&CurrentEs()
{
	thread_local EsState *state = nullptr;
	return state;
}

namespace
{

EsState &Es()
{
	return *CurrentEs();
}

const EsCore &Core()
{
	return CurrentEs()->core;
}

// Binds texture to the edit unit; returns its target.
GLenum BindForEdit( GLuint texture )
{
	EsState &es = Es();
	const GLenum target = es.Target( texture );
	es.core.ActiveTexture( GL_TEXTURE0 + es.editUnit );
	es.core.BindTexture( target, texture );
	return target;
}

// The draw framebuffer bound now, restored by the scope's end.
class DrawFramebufferScope
{
public:
	explicit DrawFramebufferScope( GLuint framebuffer )
	{
		Core().GetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &m_Previous );
		Core().BindFramebuffer( GL_DRAW_FRAMEBUFFER, framebuffer );
	}
	~DrawFramebufferScope() { Core().BindFramebuffer( GL_DRAW_FRAMEBUFFER, GLuint( m_Previous ) ); }
	DrawFramebufferScope( const DrawFramebufferScope & ) = delete;
	DrawFramebufferScope &operator=( const DrawFramebufferScope & ) = delete;

private:
	GLint m_Previous = 0;
};

// Buffers ------------------------------------------------------------------------
// Edits bind GL_COPY_WRITE_BUFFER (reads GL_COPY_READ_BUFFER): no draw state
// reads those points.

void APIENTRY CreateBuffers( GLsizei n, GLuint *buffers )
{
	Core().GenBuffers( n, buffers );
	// A name becomes a buffer object when first bound.
	for ( GLsizei i = 0; i < n; ++i )
		Core().BindBuffer( GL_COPY_WRITE_BUFFER, buffers[i] );
}

void APIENTRY NamedBufferStorage(
    GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags )
{
	Core().BindBuffer( GL_COPY_WRITE_BUFFER, buffer );
	if ( Core().BufferStorage )
		Core().BufferStorage( GL_COPY_WRITE_BUFFER, size, data, flags );
	else
		Core().BufferData( GL_COPY_WRITE_BUFFER, size, data,
		    ( flags & GL_MAP_READ_BIT ) ? GL_DYNAMIC_READ : GL_DYNAMIC_DRAW );
}

void APIENTRY NamedBufferSubData(
    GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data )
{
	Core().BindBuffer( GL_COPY_WRITE_BUFFER, buffer );
	Core().BufferSubData( GL_COPY_WRITE_BUFFER, offset, size, data );
}

void APIENTRY CopyNamedBufferSubData(
    GLuint source, GLuint destination, GLintptr from, GLintptr to, GLsizeiptr size )
{
	Core().BindBuffer( GL_COPY_READ_BUFFER, source );
	Core().BindBuffer( GL_COPY_WRITE_BUFFER, destination );
	Core().CopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, from, to, size );
}

void *APIENTRY MapNamedBufferRange(
    GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access )
{
	Core().BindBuffer( GL_COPY_WRITE_BUFFER, buffer );
	return Core().MapBufferRange( GL_COPY_WRITE_BUFFER, offset, length, access );
}

GLboolean APIENTRY UnmapNamedBuffer( GLuint buffer )
{
	Core().BindBuffer( GL_COPY_WRITE_BUFFER, buffer );
	return Core().UnmapBuffer( GL_COPY_WRITE_BUFFER );
}

void APIENTRY GetNamedBufferSubData( GLuint buffer, GLintptr offset, GLsizeiptr size, void *data )
{
	// ES has no glGetBufferSubData: map for reading.
	Core().BindBuffer( GL_COPY_READ_BUFFER, buffer );
	const void *mapped =
	    Core().MapBufferRange( GL_COPY_READ_BUFFER, offset, size, GL_MAP_READ_BIT );
	if ( !mapped )
		return;
	std::memcpy( data, mapped, static_cast<std::size_t>( size ) );
	Core().UnmapBuffer( GL_COPY_READ_BUFFER );
}

// Textures and samplers -------------------------------------------------------------

void APIENTRY CreateTextures( GLenum target, GLsizei n, GLuint *textures )
{
	EsState &es = Es();
	es.core.GenTextures( n, textures );
	es.core.ActiveTexture( GL_TEXTURE0 + es.editUnit );
	for ( GLsizei i = 0; i < n; ++i )
	{
		es.textures[textures[i]] = { target, 0 };
		es.core.BindTexture( target, textures[i] );
	}
}

void APIENTRY DeleteTextures( GLsizei n, const GLuint *textures )
{
	EsState &es = Es();
	for ( GLsizei i = 0; i < n; ++i )
		es.textures.erase( textures[i] );
	es.core.DeleteTextures( n, textures );
}

void APIENTRY CreateSamplers( GLsizei n, GLuint *samplers )
{
	Core().GenSamplers( n, samplers );
}

void APIENTRY BindTextureUnit( GLuint unit, GLuint texture )
{
	EsState &es = Es();
	es.core.ActiveTexture( GL_TEXTURE0 + unit );
	es.core.BindTexture( es.Target( texture ), texture );
}

void Remember( GLuint texture, GLenum internal )
{
	Es().textures[texture].internal = internal;
}

void APIENTRY TextureStorage2D(
    GLuint texture, GLsizei levels, GLenum internal, GLsizei width, GLsizei height )
{
	Remember( texture, internal );
	Core().TexStorage2D( BindForEdit( texture ), levels, internal, width, height );
}

void APIENTRY TextureStorage3D(
    GLuint texture, GLsizei levels, GLenum internal, GLsizei width, GLsizei height, GLsizei depth )
{
	Remember( texture, internal );
	Core().TexStorage3D( BindForEdit( texture ), levels, internal, width, height, depth );
}

void APIENTRY TextureStorage2DMultisample( GLuint texture, GLsizei samples, GLenum internal,
    GLsizei width, GLsizei height, GLboolean fixed )
{
	Remember( texture, internal );
	Core().TexStorage2DMultisample(
	    BindForEdit( texture ), samples, internal, width, height, fixed );
}

void APIENTRY TextureStorage3DMultisample( GLuint texture, GLsizei samples, GLenum internal,
    GLsizei width, GLsizei height, GLsizei depth, GLboolean fixed )
{
	// The device refuses 2D multisample arrays when the entry point is absent.
	Remember( texture, internal );
	const GLenum target = BindForEdit( texture );
	if ( Core().TexStorage3DMultisample )
		Core().TexStorage3DMultisample( target, samples, internal, width, height, depth, fixed );
}

void APIENTRY TextureSubImage2D( GLuint texture, GLint level, GLint x, GLint y, GLsizei width,
    GLsizei height, GLenum format, GLenum type, const void *pixels )
{
	Core().TexSubImage2D(
	    BindForEdit( texture ), level, x, y, width, height, format, type, pixels );
}

// DSA addresses a cube map's faces as layers; ES as face targets.
void APIENTRY TextureSubImage3D( GLuint texture, GLint level, GLint x, GLint y, GLint z,
    GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels )
{
	const GLenum target = BindForEdit( texture );
	if ( target == GL_TEXTURE_CUBE_MAP )
		Core().TexSubImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + GLenum( z ), level, x, y, width,
		    height, format, type, pixels );
	else
		Core().TexSubImage3D( target, level, x, y, z, width, height, depth, format, type, pixels );
}

void APIENTRY CompressedTextureSubImage2D( GLuint texture, GLint level, GLint x, GLint y,
    GLsizei width, GLsizei height, GLenum format, GLsizei bytes, const void *data )
{
	Core().CompressedTexSubImage2D(
	    BindForEdit( texture ), level, x, y, width, height, format, bytes, data );
}

void APIENTRY CompressedTextureSubImage3D( GLuint texture, GLint level, GLint x, GLint y, GLint z,
    GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei bytes, const void *data )
{
	const GLenum target = BindForEdit( texture );
	if ( target == GL_TEXTURE_CUBE_MAP )
		Core().CompressedTexSubImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + GLenum( z ), level, x, y,
		    width, height, format, bytes, data );
	else
		Core().CompressedTexSubImage3D(
		    target, level, x, y, z, width, height, depth, format, bytes, data );
}

// Framebuffers ------------------------------------------------------------------
// Attachments are edited on GL_READ_FRAMEBUFFER, which only readback and
// blits use, and set it themselves.

void APIENTRY CreateFramebuffers( GLsizei n, GLuint *framebuffers )
{
	Core().GenFramebuffers( n, framebuffers );
	for ( GLsizei i = 0; i < n; ++i )
		Core().BindFramebuffer( GL_READ_FRAMEBUFFER, framebuffers[i] );
}

void Attach( GLenum attachment, GLuint texture, GLint level, GLint layer )
{
	const GLenum target = Es().Target( texture );
	if ( target == GL_TEXTURE_2D || target == GL_TEXTURE_2D_MULTISAMPLE )
		Core().FramebufferTexture2D( GL_READ_FRAMEBUFFER, attachment, target, texture, level );
	else if ( target == GL_TEXTURE_CUBE_MAP )
		Core().FramebufferTexture2D( GL_READ_FRAMEBUFFER, attachment,
		    GL_TEXTURE_CUBE_MAP_POSITIVE_X + GLenum( layer ), texture, level );
	else
		Core().FramebufferTextureLayer( GL_READ_FRAMEBUFFER, attachment, texture, level, layer );
}

void APIENTRY NamedFramebufferTexture(
    GLuint framebuffer, GLenum attachment, GLuint texture, GLint level )
{
	Core().BindFramebuffer( GL_READ_FRAMEBUFFER, framebuffer );
	Attach( attachment, texture, level, 0 );
}

void APIENTRY NamedFramebufferTextureLayer(
    GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer )
{
	Core().BindFramebuffer( GL_READ_FRAMEBUFFER, framebuffer );
	Attach( attachment, texture, level, layer );
}

GLenum APIENTRY CheckNamedFramebufferStatus( GLuint framebuffer, GLenum )
{
	Core().BindFramebuffer( GL_READ_FRAMEBUFFER, framebuffer );
	return Core().CheckFramebufferStatus( GL_READ_FRAMEBUFFER );
}

void APIENTRY NamedFramebufferDrawBuffers( GLuint framebuffer, GLsizei n, const GLenum *buffers )
{
	DrawFramebufferScope scope( framebuffer );
	Core().DrawBuffers( n, buffers );
}

void APIENTRY NamedFramebufferReadBuffer( GLuint framebuffer, GLenum buffer )
{
	Core().BindFramebuffer( GL_READ_FRAMEBUFFER, framebuffer );
	Core().ReadBuffer( buffer );
}

void APIENTRY ClearNamedFramebufferfv(
    GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value )
{
	DrawFramebufferScope scope( framebuffer );
	Core().ClearBufferfv( buffer, drawbuffer, value );
}

void APIENTRY ClearNamedFramebufferfi(
    GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil )
{
	DrawFramebufferScope scope( framebuffer );
	Core().ClearBufferfi( buffer, drawbuffer, depth, stencil );
}

void APIENTRY InvalidateNamedFramebufferData(
    GLuint framebuffer, GLsizei n, const GLenum *attachments )
{
	DrawFramebufferScope scope( framebuffer );
	Core().InvalidateFramebuffer( GL_DRAW_FRAMEBUFFER, n, attachments );
}

void APIENTRY BlitNamedFramebuffer( GLuint source, GLuint destination, GLint sx0, GLint sy0,
    GLint sx1, GLint sy1, GLint dx0, GLint dy0, GLint dx1, GLint dy1, GLbitfield mask,
    GLenum filter )
{
	DrawFramebufferScope scope( destination );
	Core().BindFramebuffer( GL_READ_FRAMEBUFFER, source );
	Core().BlitFramebuffer( sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, filter );
}

// Vertex arrays: the edit leaves the array bound (the replay binds the
// pipeline's array before it sets its buffers, and CreatePipeline is not
// called inside a replay).

void APIENTRY CreateVertexArrays( GLsizei n, GLuint *arrays )
{
	Core().GenVertexArrays( n, arrays );
	for ( GLsizei i = 0; i < n; ++i )
		Core().BindVertexArray( arrays[i] );
}

void APIENTRY EnableVertexArrayAttrib( GLuint array, GLuint index )
{
	Core().BindVertexArray( array );
	Core().EnableVertexAttribArray( index );
}

void APIENTRY VertexArrayAttribFormat(
    GLuint array, GLuint index, GLint size, GLenum type, GLboolean normalized, GLuint offset )
{
	Core().BindVertexArray( array );
	Core().VertexAttribFormat( index, size, type, normalized, offset );
}

void APIENTRY VertexArrayAttribBinding( GLuint array, GLuint index, GLuint binding )
{
	Core().BindVertexArray( array );
	Core().VertexAttribBinding( index, binding );
}

void APIENTRY VertexArrayBindingDivisor( GLuint array, GLuint binding, GLuint divisor )
{
	Core().BindVertexArray( array );
	Core().VertexBindingDivisor( binding, divisor );
}

void APIENTRY VertexArrayVertexBuffer(
    GLuint array, GLuint binding, GLuint buffer, GLintptr offset, GLsizei stride )
{
	Core().BindVertexArray( array );
	Core().BindVertexBuffer( binding, buffer, offset, stride );
}

void APIENTRY VertexArrayElementBuffer( GLuint array, GLuint buffer )
{
	Core().BindVertexArray( array );
	Core().BindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffer );
}

// Fixed function --------------------------------------------------------------------

void APIENTRY ViewportIndexedf( GLuint, GLfloat x, GLfloat y, GLfloat width, GLfloat height )
{
	// ES viewports are integers; the port's are whole pixels in practice.
	auto round = []( GLfloat value )
	{
		return static_cast<GLint>( value < 0 ? value - 0.5f : value + 0.5f );
	};
	Core().Viewport( round( x ), round( y ), round( width ), round( height ) );
}

void APIENTRY ScissorIndexed( GLuint, GLint x, GLint y, GLsizei width, GLsizei height )
{
	Core().Scissor( x, y, width, height );
}

void APIENTRY DepthRangeIndexed( GLuint, GLdouble nearValue, GLdouble farValue )
{
	Core().DepthRangef( static_cast<GLfloat>( nearValue ), static_cast<GLfloat>( farValue ) );
}

// Draws: the replay offsets per-instance vertex buffers by the first instance
// and sets SPIRV_Cross_BaseInstance (ES has no base instance).
void APIENTRY DrawArraysInstancedBaseInstance(
    GLenum mode, GLint first, GLsizei count, GLsizei instances, GLuint )
{
	Core().DrawArraysInstanced( mode, first, count, instances );
}

void APIENTRY DrawElementsInstancedBaseVertexBaseInstance( GLenum mode, GLsizei count, GLenum type,
    const void *indices, GLsizei instances, GLint baseVertex, GLuint )
{
	Core().DrawElementsInstancedBaseVertex( mode, count, type, indices, instances, baseVertex );
}

GLenum APIENTRY GetGraphicsResetStatus()
{
	return Core().GetGraphicsResetStatus ? Core().GetGraphicsResetStatus() : GL_NO_ERROR;
}

// Debug output: KHR_debug's entry points when present, else nothing.

void APIENTRY DebugMessageCallback( GLDEBUGPROC callback, const void *user )
{
	if ( Core().DebugMessageCallback )
		Core().DebugMessageCallback( callback, user );
}

void APIENTRY DebugMessageControl( GLenum source, GLenum type, GLenum severity, GLsizei count,
    const GLuint *ids, GLboolean enabled )
{
	if ( Core().DebugMessageControl )
		Core().DebugMessageControl( source, type, severity, count, ids, enabled );
}

void APIENTRY ObjectLabel( GLenum identifier, GLuint name, GLsizei length, const GLchar *label )
{
	if ( Core().ObjectLabel )
		Core().ObjectLabel( identifier, name, length, label );
}

void APIENTRY PushDebugGroup( GLenum source, GLuint id, GLsizei length, const GLchar *message )
{
	if ( Core().PushDebugGroup )
		Core().PushDebugGroup( source, id, length, message );
}

void APIENTRY PopDebugGroup()
{
	if ( Core().PopDebugGroup )
		Core().PopDebugGroup();
}

// Never called on ES: the adapter claims no timestamps, no block compression
// and clears and reads textures back through its own ES branches.

void APIENTRY CreateQueries( GLenum, GLsizei n, GLuint *ids )
{
	for ( GLsizei i = 0; i < n; ++i )
		ids[i] = 0;
}

void APIENTRY QueryCounter( GLuint, GLenum )
{
}

void APIENTRY GetQueryBufferObjectui64v( GLuint, GLuint, GLenum, GLintptr )
{
}

void APIENTRY ClearTexSubImage(
    GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void * )
{
}

void APIENTRY GetTextureSubImage(
    GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void * )
{
}

void APIENTRY GetCompressedTextureSubImage(
    GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, void * )
{
}

template <typename T>
bool LoadFirst( T &member, ProcLoader load, std::initializer_list<const char *> names )
{
	for ( const char *name : names )
	{
		if ( void *found = load( name ) )
		{
			member = reinterpret_cast<T>( found );
			return true;
		}
	}
	return false;
}

} // namespace

const char *LoadEs( GlApi &api, EsState &state, ProcLoader load )
{
	EsCore &core = state.core;
#define RENDER_GLES_LOAD_REQUIRED( member, type, ... )                                             \
	if ( !LoadFirst( core.member, load, { __VA_ARGS__ } ) )                                        \
		return std::initializer_list<const char *>{ __VA_ARGS__ }.begin()[0];
	RENDER_GLES_REQUIRED( RENDER_GLES_LOAD_REQUIRED )
#undef RENDER_GLES_LOAD_REQUIRED
#define RENDER_GLES_LOAD_OPTIONAL( member, type, ... )                                             \
	(void)LoadFirst( core.member, load, { __VA_ARGS__ } );
	RENDER_GLES_OPTIONAL( RENDER_GLES_LOAD_OPTIONAL )
#undef RENDER_GLES_LOAD_OPTIONAL

	// The members ES has under the same name and meaning.
#define RENDER_GLES_DIRECT( member )                                                               \
	if ( !LoadFirst( api.member, load, { "gl" #member } ) )                                        \
		return "gl" #member;
	RENDER_GLES_DIRECT( AttachShader )
	RENDER_GLES_DIRECT( BindBufferRange )
	RENDER_GLES_DIRECT( BindImageTexture )
	RENDER_GLES_DIRECT( BindSampler )
	RENDER_GLES_DIRECT( ClientWaitSync )
	RENDER_GLES_DIRECT( CompileShader )
	RENDER_GLES_DIRECT( CreateProgram )
	RENDER_GLES_DIRECT( CreateShader )
	RENDER_GLES_DIRECT( CullFace )
	RENDER_GLES_DIRECT( DeleteBuffers )
	RENDER_GLES_DIRECT( DeleteQueries )
	RENDER_GLES_DIRECT( DeleteFramebuffers )
	RENDER_GLES_DIRECT( DeleteProgram )
	RENDER_GLES_DIRECT( DeleteSamplers )
	RENDER_GLES_DIRECT( DeleteShader )
	RENDER_GLES_DIRECT( DeleteSync )
	RENDER_GLES_DIRECT( DeleteVertexArrays )
	RENDER_GLES_DIRECT( StencilFunc )
	RENDER_GLES_DIRECT( StencilMask )
	RENDER_GLES_DIRECT( StencilOp )
	RENDER_GLES_DIRECT( PolygonOffset )
	RENDER_GLES_DIRECT( DepthFunc )
	RENDER_GLES_DIRECT( DepthMask )
	RENDER_GLES_DIRECT( DetachShader )
	RENDER_GLES_DIRECT( Disable )
	RENDER_GLES_DIRECT( DispatchCompute )
	RENDER_GLES_DIRECT( Enable )
	RENDER_GLES_DIRECT( FenceSync )
	RENDER_GLES_DIRECT( Finish )
	RENDER_GLES_DIRECT( Flush )
	RENDER_GLES_DIRECT( FrontFace )
	RENDER_GLES_DIRECT( GetActiveUniformBlockiv )
	RENDER_GLES_DIRECT( GetError )
	RENDER_GLES_DIRECT( GetIntegerv )
	RENDER_GLES_DIRECT( GetQueryiv )
	RENDER_GLES_DIRECT( GetProgramInfoLog )
	RENDER_GLES_DIRECT( GetProgramInterfaceiv )
	RENDER_GLES_DIRECT( GetProgramResourceiv )
	RENDER_GLES_DIRECT( GetProgramResourceName )
	RENDER_GLES_DIRECT( GetProgramiv )
	RENDER_GLES_DIRECT( GetShaderInfoLog )
	RENDER_GLES_DIRECT( GetShaderiv )
	RENDER_GLES_DIRECT( GetString )
	RENDER_GLES_DIRECT( GetStringi )
	RENDER_GLES_DIRECT( GetUniformBlockIndex )
	RENDER_GLES_DIRECT( GetUniformLocation )
	RENDER_GLES_DIRECT( GetUniformiv )
	RENDER_GLES_DIRECT( LinkProgram )
	RENDER_GLES_DIRECT( MemoryBarrier )
	RENDER_GLES_DIRECT( PixelStorei )
	RENDER_GLES_DIRECT( ProgramUniform1i )
	RENDER_GLES_DIRECT( SamplerParameterf )
	RENDER_GLES_DIRECT( SamplerParameteri )
	RENDER_GLES_DIRECT( ShaderSource )
	RENDER_GLES_DIRECT( UseProgram )
#undef RENDER_GLES_DIRECT
	api.ActiveTexture = core.ActiveTexture;
	api.BindBuffer = core.BindBuffer;
	api.BindFramebuffer = core.BindFramebuffer;
	api.BindVertexArray = core.BindVertexArray;
	api.ClipControl = core.ClipControl;
	api.BlendEquationSeparatei = core.BlendEquationSeparatei;
	api.BlendFuncSeparatei = core.BlendFuncSeparatei;
	api.ColorMaski = core.ColorMaski;
	api.Disablei = core.Disablei;
	api.Enablei = core.Enablei;

	// Shims for the DSA members.
	api.BindTextureUnit = &BindTextureUnit;
	api.BlitNamedFramebuffer = &BlitNamedFramebuffer;
	api.CheckNamedFramebufferStatus = &CheckNamedFramebufferStatus;
	api.ClearNamedFramebufferfi = &ClearNamedFramebufferfi;
	api.ClearNamedFramebufferfv = &ClearNamedFramebufferfv;
	api.ClearTexSubImage = &ClearTexSubImage;
	api.CompressedTextureSubImage2D = &CompressedTextureSubImage2D;
	api.CompressedTextureSubImage3D = &CompressedTextureSubImage3D;
	api.CopyNamedBufferSubData = &CopyNamedBufferSubData;
	api.CreateBuffers = &CreateBuffers;
	api.CreateQueries = &CreateQueries;
	api.CreateFramebuffers = &CreateFramebuffers;
	api.CreateSamplers = &CreateSamplers;
	api.CreateTextures = &CreateTextures;
	api.CreateVertexArrays = &CreateVertexArrays;
	api.DebugMessageCallback = &DebugMessageCallback;
	api.DebugMessageControl = &DebugMessageControl;
	api.DeleteTextures = &DeleteTextures;
	api.DepthRangeIndexed = &DepthRangeIndexed;
	api.DrawArraysInstancedBaseInstance = &DrawArraysInstancedBaseInstance;
	api.DrawElementsInstancedBaseVertexBaseInstance = &DrawElementsInstancedBaseVertexBaseInstance;
	api.EnableVertexArrayAttrib = &EnableVertexArrayAttrib;
	api.GetCompressedTextureSubImage = &GetCompressedTextureSubImage;
	api.GetGraphicsResetStatus = &GetGraphicsResetStatus;
	api.GetNamedBufferSubData = &GetNamedBufferSubData;
	api.GetQueryBufferObjectui64v = &GetQueryBufferObjectui64v;
	api.GetTextureSubImage = &GetTextureSubImage;
	api.InvalidateNamedFramebufferData = &InvalidateNamedFramebufferData;
	api.MapNamedBufferRange = &MapNamedBufferRange;
	api.NamedBufferStorage = &NamedBufferStorage;
	api.NamedBufferSubData = &NamedBufferSubData;
	api.NamedFramebufferDrawBuffers = &NamedFramebufferDrawBuffers;
	api.NamedFramebufferReadBuffer = &NamedFramebufferReadBuffer;
	api.NamedFramebufferTexture = &NamedFramebufferTexture;
	api.NamedFramebufferTextureLayer = &NamedFramebufferTextureLayer;
	api.ObjectLabel = &ObjectLabel;
	api.PopDebugGroup = &PopDebugGroup;
	api.PushDebugGroup = &PushDebugGroup;
	api.QueryCounter = &QueryCounter;
	api.ScissorIndexed = &ScissorIndexed;
	api.TextureStorage2D = &TextureStorage2D;
	api.TextureStorage2DMultisample = &TextureStorage2DMultisample;
	api.TextureStorage3D = &TextureStorage3D;
	api.TextureStorage3DMultisample = &TextureStorage3DMultisample;
	api.TextureSubImage2D = &TextureSubImage2D;
	api.TextureSubImage3D = &TextureSubImage3D;
	api.UnmapNamedBuffer = &UnmapNamedBuffer;
	api.VertexArrayAttribBinding = &VertexArrayAttribBinding;
	api.VertexArrayAttribFormat = &VertexArrayAttribFormat;
	api.VertexArrayBindingDivisor = &VertexArrayBindingDivisor;
	api.VertexArrayElementBuffer = &VertexArrayElementBuffer;
	api.VertexArrayVertexBuffer = &VertexArrayVertexBuffer;
	api.ViewportIndexedf = &ViewportIndexedf;
	return nullptr;
}

namespace
{

// Copies depth texels of a mip (and layer) into a storage buffer as the
// port's 32-bit floats: ES ReadPixels cannot read depth.
const char *kDepthCopySource = R"glsl(#version 310 es
layout(local_size_x = 8, local_size_y = 8) in;
uniform highp SAMPLER source;
layout(std430, binding = 0) writeonly buffer Out { highp float values[]; };
uniform ivec4 region; // x, y, width, height
uniform ivec4 where;  // first word, mip, layer, unused
void main()
{
	ivec2 p = ivec2(gl_GlobalInvocationID.xy);
	if (p.x >= region.z || p.y >= region.w)
		return;
	values[where.x + p.y * region.z + p.x] = texelFetch(source, COORD, where.y).r;
}
)glsl";

} // namespace

GLuint DepthCopyProgram( const GlApi &gl, EsState &state, bool array )
{
	GLuint &program = array ? state.depthCopyArray : state.depthCopy2D;
	if ( program )
		return program;
	std::string source = kDepthCopySource;
	const auto replace = [&]( const char *from, const char *to )
	{
		source.replace( source.find( from ), std::strlen( from ), to );
	};
	replace( "SAMPLER", array ? "sampler2DArray" : "sampler2D" );
	replace( "COORD", array ? "ivec3(region.xy + p, where.z)" : "region.xy + p" );
	const GLuint shader = gl.CreateShader( GL_COMPUTE_SHADER );
	const GLchar *text = source.c_str();
	gl.ShaderSource( shader, 1, &text, nullptr );
	gl.CompileShader( shader );
	GLint compiled = GL_FALSE;
	gl.GetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
	const GLuint built = gl.CreateProgram();
	gl.AttachShader( built, shader );
	if ( compiled )
		gl.LinkProgram( built );
	gl.DetachShader( built, shader );
	gl.DeleteShader( shader );
	GLint linked = GL_FALSE;
	if ( compiled )
		gl.GetProgramiv( built, GL_LINK_STATUS, &linked );
	if ( !linked )
	{
		gl.DeleteProgram( built );
		return 0;
	}
	program = built;
	return program;
}

} // namespace render::device::gl
