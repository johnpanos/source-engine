//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl: the EGL surfaceless context (gl_context.h).
//
//=============================================================================//

#include "gl_context.h"

#include <EGL/eglext.h>

#include <cstring>
#include <mutex>

namespace render::device::gl
{

namespace
{

// One surfaceless display per process, initialized while any context lives:
// eglTerminate would end it for every user at once.
std::mutex g_DisplayLock;
EGLDisplay g_Display = EGL_NO_DISPLAY;
int g_DisplayUsers = 0;

bool HasExtension( const char *list, const char *name )
{
	if ( !list )
		return false;
	const std::size_t length = std::strlen( name );
	for ( const char *at = std::strstr( list, name ); at; at = std::strstr( at + 1, name ) )
	{
		const bool starts = at == list || at[-1] == ' ';
		const bool ends = at[length] == '\0' || at[length] == ' ';
		if ( starts && ends )
			return true;
	}
	return false;
}

EGLDisplay AcquireDisplay( std::string &error )
{
	std::lock_guard<std::mutex> lock( g_DisplayLock );
	if ( g_DisplayUsers > 0 )
	{
		++g_DisplayUsers;
		return g_Display;
	}
	const char *client = eglQueryString( EGL_NO_DISPLAY, EGL_EXTENSIONS );
	if ( !HasExtension( client, "EGL_MESA_platform_surfaceless" ) )
	{
		error = "EGL has no EGL_MESA_platform_surfaceless";
		return EGL_NO_DISPLAY;
	}
	EGLDisplay display =
	    eglGetPlatformDisplay( EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr );
	EGLint major = 0, minor = 0;
	if ( display == EGL_NO_DISPLAY || !eglInitialize( display, &major, &minor ) )
	{
		error = "the EGL surfaceless display does not initialize";
		return EGL_NO_DISPLAY;
	}
	g_Display = display;
	g_DisplayUsers = 1;
	return display;
}

void ReleaseDisplay()
{
	std::lock_guard<std::mutex> lock( g_DisplayLock );
	if ( g_DisplayUsers > 0 && --g_DisplayUsers == 0 )
	{
		eglTerminate( g_Display );
		g_Display = EGL_NO_DISPLAY;
	}
}

} // namespace

std::unique_ptr<EglContext> EglContext::Create( bool debug, std::string &error )
{
	std::unique_ptr<EglContext> context( new EglContext );
	context->m_Display = AcquireDisplay( error );
	if ( context->m_Display == EGL_NO_DISPLAY )
		return nullptr;
	const char *extensions = eglQueryString( context->m_Display, EGL_EXTENSIONS );
	if ( !HasExtension( extensions, "EGL_KHR_surfaceless_context" ) ||
	     !HasExtension( extensions, "EGL_KHR_no_config_context" ) || !eglBindAPI( EGL_OPENGL_API ) )
	{
		error = "EGL cannot make a surfaceless, configless OpenGL context";
		return nullptr;
	}
	const bool flushControl = HasExtension( extensions, "EGL_KHR_context_flush_control" );
	// Robust first (a reset is reported, GL_KHR_robustness), then plain.
	for ( const bool robust : { true, false } )
	{
		EGLint attributes[20];
		int count = 0;
		auto put = [&]( EGLint name, EGLint value )
		{
			attributes[count++] = name;
			attributes[count++] = value;
		};
		put( EGL_CONTEXT_MAJOR_VERSION, 4 );
		put( EGL_CONTEXT_MINOR_VERSION, 5 );
		put( EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT );
		if ( debug )
			put( EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE );
		if ( robust )
		{
			put( EGL_CONTEXT_OPENGL_ROBUST_ACCESS, EGL_TRUE );
			put( EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY, EGL_LOSE_CONTEXT_ON_RESET );
		}
		// Switching the context between threads must not flush: the device
		// flushes where its submissions end.
		if ( flushControl )
			put( EGL_CONTEXT_RELEASE_BEHAVIOR_KHR, EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR );
		attributes[count] = EGL_NONE;
		context->m_Context =
		    eglCreateContext( context->m_Display, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attributes );
		if ( context->m_Context != EGL_NO_CONTEXT )
		{
			context->m_Robust = robust;
			break;
		}
	}
	if ( context->m_Context == EGL_NO_CONTEXT )
	{
		error = "no OpenGL 4.5 core context";
		return nullptr;
	}
	ContextScope scope( *context );
	if ( !scope.Ok() )
	{
		error = "the OpenGL context cannot be made current";
		return nullptr;
	}
	if ( const char *missing = context->m_Api.LoadAll(
	         []( const char *name )
	         {
		         return eglGetProcAddress( name );
	         } ) )
	{
		error = std::string( "the OpenGL context lacks " ) + missing;
		return nullptr;
	}
	return context;
}

EglContext::~EglContext()
{
	if ( m_Display == EGL_NO_DISPLAY )
		return;
	if ( m_Context != EGL_NO_CONTEXT )
	{
		if ( eglGetCurrentContext() == m_Context )
			eglMakeCurrent( m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
		eglDestroyContext( m_Display, m_Context );
	}
	ReleaseDisplay();
}

ContextScope::ContextScope( const EglContext &context ) : m_Context( context )
{
	// EGL keeps a current context per client API; the device's is OpenGL.
	if ( eglQueryAPI() != EGL_OPENGL_API )
		eglBindAPI( EGL_OPENGL_API );
	if ( eglGetCurrentContext() == context.Context() )
	{
		m_Ok = true;
		return;
	}
	m_PreviousDisplay = eglGetCurrentDisplay();
	m_PreviousContext = eglGetCurrentContext();
	m_PreviousDraw = eglGetCurrentSurface( EGL_DRAW );
	m_PreviousRead = eglGetCurrentSurface( EGL_READ );
	m_Ok = eglMakeCurrent( context.Display(), EGL_NO_SURFACE, EGL_NO_SURFACE, context.Context() ) ==
	       EGL_TRUE;
	m_Switched = m_Ok;
}

ContextScope::~ContextScope()
{
	if ( !m_Switched )
		return;
	if ( m_PreviousContext != EGL_NO_CONTEXT && m_PreviousDisplay != EGL_NO_DISPLAY )
		eglMakeCurrent( m_PreviousDisplay, m_PreviousDraw, m_PreviousRead, m_PreviousContext );
	else
		eglMakeCurrent( m_Context.Display(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
}

} // namespace render::device::gl
