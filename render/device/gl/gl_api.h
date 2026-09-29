//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl private: the OpenGL 4.5 entry points the adapter
//			calls, loaded through eglGetProcAddress when its context is first
//			current. Nothing outside render/device/gl sees these.
//
//=============================================================================//

#ifndef RENDER_DEVICE_GL_GL_API_H
#define RENDER_DEVICE_GL_GL_API_H

#include <GL/glcorearb.h>

// Block-compressed sRGB formats (EXT_texture_sRGB, not in glcorearb.h).
#ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

namespace render::device::gl
{

// X( member, PFN type, GL name )
#define RENDER_GL_FUNCTIONS( X )                                                                   \
	X( ActiveTexture, PFNGLACTIVETEXTUREPROC, glActiveTexture )                                    \
	X( AttachShader, PFNGLATTACHSHADERPROC, glAttachShader )                                       \
	X( BindBuffer, PFNGLBINDBUFFERPROC, glBindBuffer )                                             \
	X( BindBufferRange, PFNGLBINDBUFFERRANGEPROC, glBindBufferRange )                              \
	X( BindFramebuffer, PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer )                              \
	X( BindImageTexture, PFNGLBINDIMAGETEXTUREPROC, glBindImageTexture )                           \
	X( BindSampler, PFNGLBINDSAMPLERPROC, glBindSampler )                                          \
	X( BindTextureUnit, PFNGLBINDTEXTUREUNITPROC, glBindTextureUnit )                              \
	X( BindVertexArray, PFNGLBINDVERTEXARRAYPROC, glBindVertexArray )                              \
	X( BlendEquationSeparatei, PFNGLBLENDEQUATIONSEPARATEIPROC, glBlendEquationSeparatei )         \
	X( BlendFuncSeparatei, PFNGLBLENDFUNCSEPARATEIPROC, glBlendFuncSeparatei )                     \
	X( BlitNamedFramebuffer, PFNGLBLITNAMEDFRAMEBUFFERPROC, glBlitNamedFramebuffer )               \
	X( CheckNamedFramebufferStatus, PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC,                          \
	    glCheckNamedFramebufferStatus )                                                            \
	X( ClearNamedFramebufferfi, PFNGLCLEARNAMEDFRAMEBUFFERFIPROC, glClearNamedFramebufferfi )      \
	X( ClearNamedFramebufferfv, PFNGLCLEARNAMEDFRAMEBUFFERFVPROC, glClearNamedFramebufferfv )      \
	X( ClearTexSubImage, PFNGLCLEARTEXSUBIMAGEPROC, glClearTexSubImage )                           \
	X( ClientWaitSync, PFNGLCLIENTWAITSYNCPROC, glClientWaitSync )                                 \
	X( ClipControl, PFNGLCLIPCONTROLPROC, glClipControl )                                          \
	X( ColorMaski, PFNGLCOLORMASKIPROC, glColorMaski )                                             \
	X( CompileShader, PFNGLCOMPILESHADERPROC, glCompileShader )                                    \
	X( CompressedTextureSubImage2D, PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC,                          \
	    glCompressedTextureSubImage2D )                                                            \
	X( CompressedTextureSubImage3D, PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC,                          \
	    glCompressedTextureSubImage3D )                                                            \
	X( CopyNamedBufferSubData, PFNGLCOPYNAMEDBUFFERSUBDATAPROC, glCopyNamedBufferSubData )         \
	X( CreateBuffers, PFNGLCREATEBUFFERSPROC, glCreateBuffers )                                    \
	X( CreateFramebuffers, PFNGLCREATEFRAMEBUFFERSPROC, glCreateFramebuffers )                     \
	X( CreateProgram, PFNGLCREATEPROGRAMPROC, glCreateProgram )                                    \
	X( CreateSamplers, PFNGLCREATESAMPLERSPROC, glCreateSamplers )                                 \
	X( CreateShader, PFNGLCREATESHADERPROC, glCreateShader )                                       \
	X( CreateTextures, PFNGLCREATETEXTURESPROC, glCreateTextures )                                 \
	X( CreateVertexArrays, PFNGLCREATEVERTEXARRAYSPROC, glCreateVertexArrays )                     \
	X( CullFace, PFNGLCULLFACEPROC, glCullFace )                                                   \
	X( DebugMessageCallback, PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback )               \
	X( DebugMessageControl, PFNGLDEBUGMESSAGECONTROLPROC, glDebugMessageControl )                  \
	X( DeleteBuffers, PFNGLDELETEBUFFERSPROC, glDeleteBuffers )                                    \
	X( DeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers )                     \
	X( DeleteProgram, PFNGLDELETEPROGRAMPROC, glDeleteProgram )                                    \
	X( DeleteSamplers, PFNGLDELETESAMPLERSPROC, glDeleteSamplers )                                 \
	X( DeleteShader, PFNGLDELETESHADERPROC, glDeleteShader )                                       \
	X( DeleteSync, PFNGLDELETESYNCPROC, glDeleteSync )                                             \
	X( DeleteTextures, PFNGLDELETETEXTURESPROC, glDeleteTextures )                                 \
	X( DeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays )                     \
	X( DepthFunc, PFNGLDEPTHFUNCPROC, glDepthFunc )                                                \
	X( DepthMask, PFNGLDEPTHMASKPROC, glDepthMask )                                                \
	X( DepthRangeIndexed, PFNGLDEPTHRANGEINDEXEDPROC, glDepthRangeIndexed )                        \
	X( DetachShader, PFNGLDETACHSHADERPROC, glDetachShader )                                       \
	X( Disable, PFNGLDISABLEPROC, glDisable )                                                      \
	X( Disablei, PFNGLDISABLEIPROC, glDisablei )                                                   \
	X( DispatchCompute, PFNGLDISPATCHCOMPUTEPROC, glDispatchCompute )                              \
	X( DrawArraysInstancedBaseInstance, PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC,                  \
	    glDrawArraysInstancedBaseInstance )                                                        \
	X( DrawElementsInstancedBaseVertexBaseInstance,                                                \
	    PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC,                                      \
	    glDrawElementsInstancedBaseVertexBaseInstance )                                            \
	X( Enable, PFNGLENABLEPROC, glEnable )                                                         \
	X( Enablei, PFNGLENABLEIPROC, glEnablei )                                                      \
	X( EnableVertexArrayAttrib, PFNGLENABLEVERTEXARRAYATTRIBPROC, glEnableVertexArrayAttrib )      \
	X( FenceSync, PFNGLFENCESYNCPROC, glFenceSync )                                                \
	X( Finish, PFNGLFINISHPROC, glFinish )                                                         \
	X( Flush, PFNGLFLUSHPROC, glFlush )                                                            \
	X( FrontFace, PFNGLFRONTFACEPROC, glFrontFace )                                                \
	X( GetActiveUniformBlockiv, PFNGLGETACTIVEUNIFORMBLOCKIVPROC, glGetActiveUniformBlockiv )      \
	X( GetCompressedTextureSubImage, PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC,                        \
	    glGetCompressedTextureSubImage )                                                           \
	X( GetError, PFNGLGETERRORPROC, glGetError )                                                   \
	X( GetGraphicsResetStatus, PFNGLGETGRAPHICSRESETSTATUSPROC, glGetGraphicsResetStatus )         \
	X( GetIntegerv, PFNGLGETINTEGERVPROC, glGetIntegerv )                                          \
	X( GetNamedBufferSubData, PFNGLGETNAMEDBUFFERSUBDATAPROC, glGetNamedBufferSubData )            \
	X( GetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog )                        \
	X( GetProgramInterfaceiv, PFNGLGETPROGRAMINTERFACEIVPROC, glGetProgramInterfaceiv )            \
	X( GetProgramResourceiv, PFNGLGETPROGRAMRESOURCEIVPROC, glGetProgramResourceiv )               \
	X( GetProgramResourceName, PFNGLGETPROGRAMRESOURCENAMEPROC, glGetProgramResourceName )         \
	X( GetProgramiv, PFNGLGETPROGRAMIVPROC, glGetProgramiv )                                       \
	X( GetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog )                           \
	X( GetShaderiv, PFNGLGETSHADERIVPROC, glGetShaderiv )                                          \
	X( GetString, PFNGLGETSTRINGPROC, glGetString )                                                \
	X( GetStringi, PFNGLGETSTRINGIPROC, glGetStringi )                                             \
	X( GetTextureSubImage, PFNGLGETTEXTURESUBIMAGEPROC, glGetTextureSubImage )                     \
	X( GetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC, glGetUniformBlockIndex )               \
	X( GetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation )                     \
	X( GetUniformiv, PFNGLGETUNIFORMIVPROC, glGetUniformiv )                                       \
	X( InvalidateNamedFramebufferData, PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC,                    \
	    glInvalidateNamedFramebufferData )                                                         \
	X( LinkProgram, PFNGLLINKPROGRAMPROC, glLinkProgram )                                          \
	X( MapNamedBufferRange, PFNGLMAPNAMEDBUFFERRANGEPROC, glMapNamedBufferRange )                  \
	X( MemoryBarrier, PFNGLMEMORYBARRIERPROC, glMemoryBarrier )                                    \
	X( NamedBufferStorage, PFNGLNAMEDBUFFERSTORAGEPROC, glNamedBufferStorage )                     \
	X( NamedBufferSubData, PFNGLNAMEDBUFFERSUBDATAPROC, glNamedBufferSubData )                     \
	X( NamedFramebufferDrawBuffers, PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC,                          \
	    glNamedFramebufferDrawBuffers )                                                            \
	X( NamedFramebufferReadBuffer, PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC,                            \
	    glNamedFramebufferReadBuffer )                                                             \
	X( NamedFramebufferTexture, PFNGLNAMEDFRAMEBUFFERTEXTUREPROC, glNamedFramebufferTexture )      \
	X( NamedFramebufferTextureLayer, PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC,                        \
	    glNamedFramebufferTextureLayer )                                                           \
	X( ObjectLabel, PFNGLOBJECTLABELPROC, glObjectLabel )                                          \
	X( PixelStorei, PFNGLPIXELSTOREIPROC, glPixelStorei )                                          \
	X( PopDebugGroup, PFNGLPOPDEBUGGROUPPROC, glPopDebugGroup )                                    \
	X( ProgramUniform1i, PFNGLPROGRAMUNIFORM1IPROC, glProgramUniform1i )                           \
	X( PushDebugGroup, PFNGLPUSHDEBUGGROUPPROC, glPushDebugGroup )                                 \
	X( SamplerParameterf, PFNGLSAMPLERPARAMETERFPROC, glSamplerParameterf )                        \
	X( SamplerParameteri, PFNGLSAMPLERPARAMETERIPROC, glSamplerParameteri )                        \
	X( ScissorIndexed, PFNGLSCISSORINDEXEDPROC, glScissorIndexed )                                 \
	X( ShaderSource, PFNGLSHADERSOURCEPROC, glShaderSource )                                       \
	X( TextureStorage2D, PFNGLTEXTURESTORAGE2DPROC, glTextureStorage2D )                           \
	X( TextureStorage2DMultisample, PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC,                          \
	    glTextureStorage2DMultisample )                                                            \
	X( TextureStorage3D, PFNGLTEXTURESTORAGE3DPROC, glTextureStorage3D )                           \
	X( TextureStorage3DMultisample, PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC,                          \
	    glTextureStorage3DMultisample )                                                            \
	X( TextureSubImage2D, PFNGLTEXTURESUBIMAGE2DPROC, glTextureSubImage2D )                        \
	X( TextureSubImage3D, PFNGLTEXTURESUBIMAGE3DPROC, glTextureSubImage3D )                        \
	X( UnmapNamedBuffer, PFNGLUNMAPNAMEDBUFFERPROC, glUnmapNamedBuffer )                           \
	X( UseProgram, PFNGLUSEPROGRAMPROC, glUseProgram )                                             \
	X( VertexArrayAttribBinding, PFNGLVERTEXARRAYATTRIBBINDINGPROC, glVertexArrayAttribBinding )   \
	X( VertexArrayAttribFormat, PFNGLVERTEXARRAYATTRIBFORMATPROC, glVertexArrayAttribFormat )      \
	X( VertexArrayBindingDivisor, PFNGLVERTEXARRAYBINDINGDIVISORPROC,                              \
	    glVertexArrayBindingDivisor )                                                              \
	X( VertexArrayElementBuffer, PFNGLVERTEXARRAYELEMENTBUFFERPROC, glVertexArrayElementBuffer )   \
	X( VertexArrayVertexBuffer, PFNGLVERTEXARRAYVERTEXBUFFERPROC, glVertexArrayVertexBuffer )      \
	X( ViewportIndexedf, PFNGLVIEWPORTINDEXEDFPROC, glViewportIndexedf )

struct GlApi
{
#define RENDER_GL_MEMBER( member, type, name ) type member = nullptr;
	RENDER_GL_FUNCTIONS( RENDER_GL_MEMBER )
#undef RENDER_GL_MEMBER

	// Loads every entry point through load(name); the name of the first one
	// missing, or nullptr when all are present.
	template <typename Load> const char *LoadAll( const Load &load )
	{
#define RENDER_GL_LOAD( member, type, name )                                                       \
	member = reinterpret_cast<type>( load( #name ) );                                              \
	if ( !member )                                                                                 \
		return #name;
		RENDER_GL_FUNCTIONS( RENDER_GL_LOAD )
#undef RENDER_GL_LOAD
		return nullptr;
	}
};

} // namespace render::device::gl

#endif // RENDER_DEVICE_GL_GL_API_H
