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

namespace
{

EGLenum EglApi( GlApiKind api )
{
	return api == GlApiKind::kEs31 ? EGL_OPENGL_ES_API : EGL_OPENGL_API;
}

} // namespace

std::unique_ptr<EglContext> EglContext::Create( GlApiKind api, bool debug, std::string &error )
{
	const bool es = api == GlApiKind::kEs31;
	std::unique_ptr<EglContext> context( new EglContext );
	context->m_ApiKind = api;
	context->m_Display = AcquireDisplay( error );
	if ( context->m_Display == EGL_NO_DISPLAY )
		return nullptr;
	const char *extensions = eglQueryString( context->m_Display, EGL_EXTENSIONS );
	if ( !HasExtension( extensions, "EGL_KHR_surfaceless_context" ) ||
	     !HasExtension( extensions, "EGL_KHR_no_config_context" ) || !eglBindAPI( EglApi( api ) ) )
	{
		error = es ? "EGL cannot make a surfaceless, configless OpenGL ES context"
		           : "EGL cannot make a surfaceless, configless OpenGL context";
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
		put( EGL_CONTEXT_MAJOR_VERSION, es ? 3 : 4 );
		put( EGL_CONTEXT_MINOR_VERSION, es ? 1 : 5 );
		if ( !es )
			put( EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT );
		if ( debug )
			put( EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE );
		if ( robust )
		{
			// EGL 1.5 applies these to OpenGL ES contexts too.
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
		error = es ? "no OpenGL ES 3.1 context" : "no OpenGL 4.5 core context";
		return nullptr;
	}
	if ( es )
		context->m_Es = std::make_unique<EsState>();
	ContextScope scope( *context );
	if ( !scope.Ok() )
	{
		error = es ? "the OpenGL ES context cannot be made current"
		           : "the OpenGL context cannot be made current";
		return nullptr;
	}
	const auto load = []( const char *name ) -> void *
	{
		return reinterpret_cast<void *>( eglGetProcAddress( name ) );
	};
	const char *missing =
	    es ? LoadEs( context->m_Api, *context->m_Es, load ) : context->m_Api.LoadAll( load );
	if ( missing )
	{
		error = std::string( es ? "the OpenGL ES context lacks " : "the OpenGL context lacks " ) +
		        missing;
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
		const EGLenum previous = eglQueryAPI();
		eglBindAPI( EglApi( m_ApiKind ) );
		if ( eglGetCurrentContext() == m_Context )
		{
			eglMakeCurrent( m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
			CurrentEs() = nullptr;
		}
		eglDestroyContext( m_Display, m_Context );
		eglBindAPI( previous );
	}
	ReleaseDisplay();
}

ContextScope::ContextScope( const EglContext &context ) : m_Context( context )
{
	m_PreviousEs = CurrentEs();
	CurrentEs() = context.Es();
	// EGL keeps a current context per client API binding: bind the device's.
	m_PreviousApi = eglQueryAPI();
	const EGLenum api = EglApi( context.ApiKind() );
	if ( m_PreviousApi != api )
		eglBindAPI( api );
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
	if ( m_Switched )
	{
		if ( m_PreviousContext != EGL_NO_CONTEXT && m_PreviousDisplay != EGL_NO_DISPLAY )
			eglMakeCurrent( m_PreviousDisplay, m_PreviousDraw, m_PreviousRead, m_PreviousContext );
		else
			eglMakeCurrent( m_Context.Display(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	}
	if ( m_PreviousApi != 0 && m_PreviousApi != eglQueryAPI() )
		eglBindAPI( m_PreviousApi );
	CurrentEs() = m_PreviousEs;
}

} // namespace render::device::gl
