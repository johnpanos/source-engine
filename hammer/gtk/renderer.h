//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: OpenGL 3.3-core renderer for the GTK Hammer desktop shell's viewports
//			(RFC 0002, linux-gtk-desktop). One Renderer drives one viewport in one
//			of the classic Hammer view modes: a shaded 3D camera view, or a 2D
//			orthographic wireframe-with-grid view (top X/Y, front X/Z, side Y/Z).
//
//			It is deliberately free of any GTK/GDK dependency: it draws a
//			hammer::geometry::WorldScene into the currently-bound framebuffer and
//			viewport, so the same code serves each interactive GtkGLArea and the
//			offscreen EGL screenshot path used for automated verification. GL native
//			detail stays confined here; portable editor code deals only in the
//			WorldScene value type. Map geometry is Source's Z-up world space.
//
//=============================================================================//

#ifndef HAMMER_GTK_RENDERER_H
#define HAMMER_GTK_RENDERER_H

#include "hammer/geometry/brush.h"

#include <string>

namespace hammergtk
{

// The classic Hammer viewport kinds.
enum class ViewMode
{
	Perspective, // 3D shaded camera view
	Top,         // 2D orthographic X/Y (looking down -Z)
	Front,       // 2D orthographic X/Z (looking along +Y)
	Side,        // 2D orthographic Y/Z (looking along -X)
};

// Orbit camera for the 3D view, in Source Z-up world space.
struct Camera
{
	float yawDeg = 45.0f;
	float pitchDeg = 30.0f;
	float distance = 1024.0f;
	float target[3] = { 0.0f, 0.0f, 0.0f };
};

// Pan/zoom state for a 2D orthographic view. 'pixelsPerUnit' is the zoom; pan is
// the world-space point at the centre of the viewport (in that view's two axes).
struct Ortho2D
{
	float panU = 0.0f;
	float panV = 0.0f;
	float pixelsPerUnit = 0.25f;
};

class Renderer
{
public:
	Renderer() = default;
	~Renderer();

	Renderer( const Renderer & ) = delete;
	Renderer &operator=( const Renderer & ) = delete;

	// Creates GL programs/buffers. A GL 3.3 context must already be current.
	bool Init( std::string &error );

	void SetViewMode( ViewMode mode ) { m_mode = mode; }
	ViewMode Mode() const { return m_mode; }

	// Uploads a scene's brush geometry to GL buffers, replacing any previous
	// scene. Does NOT move the camera (so live edits keep the current view); call
	// FrameScene() explicitly after a load/new/reset. A GL context must be current.
	void SetScene( const hammer::geometry::WorldScene &scene );

	// The solid id to draw highlighted (a selected brush). Solids with a negative
	// id are drawn as the in-progress "pending" box. Takes effect at the next
	// SetScene. INT_MIN (the default) highlights nothing.
	void SetHighlight( int solidId ) { m_highlightId = solidId; }

	// Draws the current scene into the bound framebuffer at the given pixel size.
	void Render( int widthPx, int heightPx );

	// Frames the whole scene bounds (both the 3D camera and the 2D pan/zoom).
	void FrameScene();

	Camera &Cam() { return m_camera; }

	// Interactive helpers (no GL; safe from input handlers). Drag deltas are in
	// pixels; their meaning depends on the view mode (orbit vs pan).
	void DragBy( float dxPixels, float dyPixels );
	void ZoomBy( float factor );

	// Touchpad two-finger scroll: pans a 2D view or orbits the 3D view. Deltas are
	// scroll units already scaled to a pixel-like magnitude by the caller.
	void PanScroll( float dxUnits, float dyUnits );

	// Figma/Apple-style zoom anchored at a cursor/pinch point (widget pixels), so
	// the world point under that point stays put. 3D falls back to a plain dolly.
	void ZoomAtPixel( float factor, float px, float py, int widthPx, int heightPx );

	// World-space coordinates under a viewport pixel, for the status read-out.
	// Only meaningful for 2D views; returns the two in-plane axis values.
	void PixelToWorld(
	    float px, float py, int widthPx, int heightPx, float &outU, float &outV ) const;

	int SolidCount() const { return m_solidCount; }
	int TriangleCount() const { return m_triCount; }

private:
	void ReleaseGl();
	void RenderPerspective( int w, int h );
	void RenderOrtho( int w, int h );
	void BuildGrid( int w, int h ); // fills the dynamic grid buffer for 2D
	void AxisIndices( int &uAxis, int &vAxis ) const;

	unsigned int m_program = 0;
	unsigned int m_meshVao = 0;
	unsigned int m_meshVbo = 0;
	unsigned int m_lineVao = 0;
	unsigned int m_lineVbo = 0;
	unsigned int m_gridVao = 0;
	unsigned int m_gridVbo = 0;

	int m_meshVertexCount = 0;
	int m_lineVertexCount = 0;
	int m_gridVertexCount = 0;
	int m_triCount = 0;
	int m_solidCount = 0;

	float m_sceneCenter[3] = { 0.0f, 0.0f, 0.0f };
	float m_sceneSize[3] = { 512.0f, 512.0f, 512.0f };
	float m_sceneRadius = 512.0f;
	bool m_haveScene = false;

	ViewMode m_mode = ViewMode::Perspective;
	Camera m_camera;
	Ortho2D m_ortho;
	int m_highlightId = -2147483647; // INT_MIN-ish: highlight nothing by default
	bool m_initialized = false;
};

} // namespace hammergtk

#endif // HAMMER_GTK_RENDERER_H
