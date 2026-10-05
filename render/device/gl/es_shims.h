//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl private: the OpenGL ES 3.1 dialect (RFC 0022).
//			ES has no direct state access, so on an ES context the adapter's
//			GlApi members that name desktop DSA calls are filled with shims
//			that bind the object to a scratch point and edit it there. Every
//			shim restores the bindings the replay relies on (the draw
//			framebuffer, the vertex array it set); texture edits use a
//			scratch texture unit, which no draw reads.
//
//			Calls whose ES meaning differs (texture clears, texture readback,
//			base instance, the ring without EXT_buffer_storage) are explicit
//			branches in the adapter, not shims; the shims for them are inert
//			and never called on ES (es_shims.cpp says which).
//
//			The shims find their context's state through CurrentEs(), which
//			ContextScope sets while the context is current on the thread.
//
//=============================================================================//

#ifndef RENDER_DEVICE_GL_ES_SHIMS_H
#define RENDER_DEVICE_GL_ES_SHIMS_H

#include "gl_api.h"

#include <unordered_map>

namespace render::device::gl
{

// ES-only entry points (not in glcorearb.h).
typedef void( APIENTRYP PFNRENDERCLIPCONTROLEXTPROC )( GLenum origin, GLenum depth );
typedef void( APIENTRYP PFNRENDERTEXSTORAGE3DMULTISAMPLEOESPROC )( GLenum target, GLsizei samples,
    GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
    GLboolean fixedsamplelocations );
typedef void( APIENTRYP PFNRENDERBUFFERSTORAGEEXTPROC )(
    GLenum target, GLsizeiptr size, const void *data, GLbitfield flags );

// ES core entry points the shims and the ES branches call. X( member, type,
// names ): the first name found is loaded (core, then OES, then EXT or KHR).
#define RENDER_GLES_REQUIRED( X )                                                                  \
	X( ActiveTexture, PFNGLACTIVETEXTUREPROC, "glActiveTexture" )                                  \
	X( BindBuffer, PFNGLBINDBUFFERPROC, "glBindBuffer" )                                           \
	X( BindFramebuffer, PFNGLBINDFRAMEBUFFERPROC, "glBindFramebuffer" )                            \
	X( BindTexture, PFNGLBINDTEXTUREPROC, "glBindTexture" )                                        \
	X( BindVertexArray, PFNGLBINDVERTEXARRAYPROC, "glBindVertexArray" )                            \
	X( BindVertexBuffer, PFNGLBINDVERTEXBUFFERPROC, "glBindVertexBuffer" )                         \
	X( BlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC, "glBlitFramebuffer" )                            \
	X( BufferData, PFNGLBUFFERDATAPROC, "glBufferData" )                                           \
	X( BufferSubData, PFNGLBUFFERSUBDATAPROC, "glBufferSubData" )                                  \
	X( CheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC, "glCheckFramebufferStatus" )       \
	X( ClearBufferfi, PFNGLCLEARBUFFERFIPROC, "glClearBufferfi" )                                  \
	X( ClearBufferfv, PFNGLCLEARBUFFERFVPROC, "glClearBufferfv" )                                  \
	X( ClipControl, PFNRENDERCLIPCONTROLEXTPROC, "glClipControlEXT" )                              \
	X( CompressedTexSubImage2D, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, "glCompressedTexSubImage2D" )    \
	X( CompressedTexSubImage3D, PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC, "glCompressedTexSubImage3D" )    \
	X( CopyBufferSubData, PFNGLCOPYBUFFERSUBDATAPROC, "glCopyBufferSubData" )                      \
	X( DeleteTextures, PFNGLDELETETEXTURESPROC, "glDeleteTextures" )                               \
	X( DepthRangef, PFNGLDEPTHRANGEFPROC, "glDepthRangef" )                                        \
	X( DrawArraysInstanced, PFNGLDRAWARRAYSINSTANCEDPROC, "glDrawArraysInstanced" )                \
	X( DrawBuffers, PFNGLDRAWBUFFERSPROC, "glDrawBuffers" )                                        \
	X( EnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC, "glEnableVertexAttribArray" )    \
	X( FramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC, "glFramebufferTexture2D" )             \
	X( FramebufferTextureLayer, PFNGLFRAMEBUFFERTEXTURELAYERPROC, "glFramebufferTextureLayer" )    \
	X( GenBuffers, PFNGLGENBUFFERSPROC, "glGenBuffers" )                                           \
	X( GenFramebuffers, PFNGLGENFRAMEBUFFERSPROC, "glGenFramebuffers" )                            \
	X( GenSamplers, PFNGLGENSAMPLERSPROC, "glGenSamplers" )                                        \
	X( GenTextures, PFNGLGENTEXTURESPROC, "glGenTextures" )                                        \
	X( GenVertexArrays, PFNGLGENVERTEXARRAYSPROC, "glGenVertexArrays" )                            \
	X( GetIntegerv, PFNGLGETINTEGERVPROC, "glGetIntegerv" )                                        \
	X( InvalidateFramebuffer, PFNGLINVALIDATEFRAMEBUFFERPROC, "glInvalidateFramebuffer" )          \
	X( MapBufferRange, PFNGLMAPBUFFERRANGEPROC, "glMapBufferRange" )                               \
	X( ReadBuffer, PFNGLREADBUFFERPROC, "glReadBuffer" )                                           \
	X( ReadPixels, PFNGLREADPIXELSPROC, "glReadPixels" )                                           \
	X( Scissor, PFNGLSCISSORPROC, "glScissor" )                                                    \
	X( TexStorage2D, PFNGLTEXSTORAGE2DPROC, "glTexStorage2D" )                                     \
	X( TexStorage2DMultisample, PFNGLTEXSTORAGE2DMULTISAMPLEPROC, "glTexStorage2DMultisample" )    \
	X( TexStorage3D, PFNGLTEXSTORAGE3DPROC, "glTexStorage3D" )                                     \
	X( TexSubImage2D, PFNGLTEXSUBIMAGE2DPROC, "glTexSubImage2D" )                                  \
	X( TexSubImage3D, PFNGLTEXSUBIMAGE3DPROC, "glTexSubImage3D" )                                  \
	X( Uniform1i, PFNGLUNIFORM1IPROC, "glUniform1i" )                                              \
	X( Uniform4i, PFNGLUNIFORM4IPROC, "glUniform4i" )                                              \
	X( UnmapBuffer, PFNGLUNMAPBUFFERPROC, "glUnmapBuffer" )                                        \
	X( VertexAttribBinding, PFNGLVERTEXATTRIBBINDINGPROC, "glVertexAttribBinding" )                \
	X( VertexAttribFormat, PFNGLVERTEXATTRIBFORMATPROC, "glVertexAttribFormat" )                   \
	X( VertexBindingDivisor, PFNGLVERTEXBINDINGDIVISORPROC, "glVertexBindingDivisor" )             \
	X( Viewport, PFNGLVIEWPORTPROC, "glViewport" )                                                 \
	X( BlendEquationSeparatei, PFNGLBLENDEQUATIONSEPARATEIPROC, "glBlendEquationSeparatei",        \
	    "glBlendEquationSeparateiOES", "glBlendEquationSeparateiEXT" )                             \
	X( BlendFuncSeparatei, PFNGLBLENDFUNCSEPARATEIPROC, "glBlendFuncSeparatei",                    \
	    "glBlendFuncSeparateiOES", "glBlendFuncSeparateiEXT" )                                     \
	X( ColorMaski, PFNGLCOLORMASKIPROC, "glColorMaski", "glColorMaskiOES", "glColorMaskiEXT" )     \
	X( Disablei, PFNGLDISABLEIPROC, "glDisablei", "glDisableiOES", "glDisableiEXT" )               \
	X( Enablei, PFNGLENABLEIPROC, "glEnablei", "glEnableiOES", "glEnableiEXT" )                    \
	X( DrawElementsInstancedBaseVertex, PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC,                  \
	    "glDrawElementsInstancedBaseVertex", "glDrawElementsInstancedBaseVertexOES",               \
	    "glDrawElementsInstancedBaseVertexEXT" )

// Optional: absent ones stay nullptr, and the adapter refuses or skips what
// needs them.
#define RENDER_GLES_OPTIONAL( X )                                                                  \
	X( BufferStorage, PFNRENDERBUFFERSTORAGEEXTPROC, "glBufferStorageEXT" )                        \
	X( TexStorage3DMultisample, PFNRENDERTEXSTORAGE3DMULTISAMPLEOESPROC,                           \
	    "glTexStorage3DMultisample", "glTexStorage3DMultisampleOES" )                              \
	X( GetGraphicsResetStatus, PFNGLGETGRAPHICSRESETSTATUSPROC, "glGetGraphicsResetStatus",        \
	    "glGetGraphicsResetStatusEXT", "glGetGraphicsResetStatusKHR" )                             \
	X( DebugMessageCallback, PFNGLDEBUGMESSAGECALLBACKPROC, "glDebugMessageCallback",              \
	    "glDebugMessageCallbackKHR" )                                                              \
	X( DebugMessageControl, PFNGLDEBUGMESSAGECONTROLPROC, "glDebugMessageControl",                 \
	    "glDebugMessageControlKHR" )                                                               \
	X( ObjectLabel, PFNGLOBJECTLABELPROC, "glObjectLabel", "glObjectLabelKHR" )                    \
	X( PushDebugGroup, PFNGLPUSHDEBUGGROUPPROC, "glPushDebugGroup", "glPushDebugGroupKHR" )        \
	X( PopDebugGroup, PFNGLPOPDEBUGGROUPPROC, "glPopDebugGroup", "glPopDebugGroupKHR" )

struct EsCore
{
#define RENDER_GLES_MEMBER( member, type, ... ) type member = nullptr;
	RENDER_GLES_REQUIRED( RENDER_GLES_MEMBER )
	RENDER_GLES_OPTIONAL( RENDER_GLES_MEMBER )
#undef RENDER_GLES_MEMBER
};

// The state of one ES context: its entry points, what each texture name is
// (ES binds by target; DSA names do not), and the scratch objects.
struct EsState
{
	struct Texture
	{
		GLenum target = 0;
		GLenum internal = 0;
	};

	EsCore core;
	std::unordered_map<GLuint, Texture> textures;
	// The texture unit edits bind to: the last one, which no artifact slot uses.
	GLuint editUnit = 0;
	// Scratch framebuffers for readback and texture clears (made on first use).
	GLuint readFramebuffer = 0;
	GLuint clearFramebuffer = 0;
	// Depth readback: compute programs (2D, 2D array) that copy depth texels
	// into a storage buffer (es_shims.cpp DepthCopyProgram).
	GLuint depthCopy2D = 0;
	GLuint depthCopyArray = 0;

	GLenum Target( GLuint name ) const
	{
		const auto found = textures.find( name );
		return found == textures.end() ? GL_TEXTURE_2D : found->second.target;
	}
};

// The state of the ES context current on this thread, or nullptr (a desktop
// context, or none). Set by ContextScope.
EsState *&CurrentEs();

using ProcLoader = void *(*)( const char *name );

// Loads the ES entry points into state.core and fills api: the members ES has
// under the same name directly, the DSA ones with shims. Returns the name of
// the first required entry point missing, or nullptr.
const char *LoadEs( GlApi &api, EsState &state, ProcLoader load );

// The ES depth-copy program for target (GL_TEXTURE_2D or an array target),
// compiled on first use in the current context; 0 if it does not build.
GLuint DepthCopyProgram( const GlApi &gl, EsState &state, bool array );

} // namespace render::device::gl

#endif // RENDER_DEVICE_GL_ES_SHIMS_H
