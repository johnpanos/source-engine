//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.gl private: the adapter's own OpenGL 4.5 core context,
//			on an EGL surfaceless display (EGL_MESA_platform_surfaceless), so it
//			needs no window and no presentation. A ContextScope makes it current
//			on the calling thread for one device call and restores whatever was
//			current before.
//
//=============================================================================//

#ifndef RENDER_DEVICE_GL_GL_CONTEXT_H
#define RENDER_DEVICE_GL_GL_CONTEXT_H

#include "gl_api.h"

#include <EGL/egl.h>

#include <memory>
#include <string>

namespace render::device::gl
{

class EglContext
{
public:
	// nullptr, with the reason in error, when EGL or a 4.5 core context is
	// unavailable.
	static std::unique_ptr<EglContext> Create( bool debug, std::string &error );
	~EglContext();
	EglContext( const EglContext & ) = delete;
	EglContext &operator=( const EglContext & ) = delete;

	EGLDisplay Display() const { return m_Display; }
	EGLContext Context() const { return m_Context; }
	// The entry points, loaded when the context was created.
	const GlApi &Api() const { return m_Api; }
	bool Robust() const { return m_Robust; }

private:
	EglContext() = default;

	EGLDisplay m_Display = EGL_NO_DISPLAY;
	EGLContext m_Context = EGL_NO_CONTEXT;
	GlApi m_Api;
	bool m_Robust = false;
};

// Current for one call: makes the context current on this thread unless it
// already is, and restores the previous binding when it goes.
class ContextScope
{
public:
	explicit ContextScope( const EglContext &context );
	~ContextScope();
	ContextScope( const ContextScope & ) = delete;
	ContextScope &operator=( const ContextScope & ) = delete;

	bool Ok() const { return m_Ok; }

private:
	const EglContext &m_Context;
	EGLDisplay m_PreviousDisplay = EGL_NO_DISPLAY;
	EGLContext m_PreviousContext = EGL_NO_CONTEXT;
	EGLSurface m_PreviousDraw = EGL_NO_SURFACE;
	EGLSurface m_PreviousRead = EGL_NO_SURFACE;
	bool m_Switched = false;
	bool m_Ok = false;
};

} // namespace render::device::gl

#endif // RENDER_DEVICE_GL_GL_CONTEXT_H
