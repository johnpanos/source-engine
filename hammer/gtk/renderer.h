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

#include "hammer/formats/material_catalog.h"
#include "hammer/geometry/brush.h"

#include <map>
#include <string>
#include <vector>

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

	// Optional material catalog for textured shading of the 3D view. When set (and
	// a face's material resolves to a decoded base texture), the perspective mesh
	// is drawn textured with world-planar UVs; otherwise the renderer falls back to
	// the flat per-brush fill exactly as before. The catalog is borrowed and must
	// outlive the renderer. Passing nullptr disables texturing. Textures are
	// created lazily during SetScene (a GL context must be current then).
	void SetMaterialCatalog( hammer::formats::MaterialCatalog *catalog ) { m_catalog = catalog; }

	// Uploads a scene's brush geometry to GL buffers, replacing any previous
	// scene. Does NOT move the camera (so live edits keep the current view); call
	// FrameScene() explicitly after a load/new/reset. A GL context must be current.
	void SetScene( const hammer::geometry::WorldScene &scene );

	// The solid id to draw highlighted (a selected brush). Solids with a negative
	// id are drawn as the in-progress "pending" box. Takes effect at the next
	// SetScene. INT_MIN (the default) highlights nothing.
	void SetHighlight( int solidId ) { m_highlightId = solidId; }

	// Highlights a set of selected solids (multi-select). Additive to SetHighlight's
	// single id; both take effect at the next SetScene. Passing an empty vector
	// clears the multi-highlight. Ids not present in the scene are ignored.
	void SetHighlights( const std::vector<int> &ids ) { m_highlights = ids; }

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

	// Sets a 2D view's absolute zoom (pixels per world unit), clamped to the same
	// range as the interactive zooms. No-op for the 3D view. Used by the number-key
	// zoom presets. Pan is unchanged, so the view stays centred where it was.
	void SetOrthoScale( float pixelsPerUnit );

	// World-space coordinates under a viewport pixel, for the status read-out.
	// Only meaningful for 2D views; returns the two in-plane axis values.
	void PixelToWorld(
	    float px, float py, int widthPx, int heightPx, float &outU, float &outV ) const;

	// --- Free-fly navigation for the 3D view (no-op for 2D views) ------------
	// These reproduce Hammer's classic Z / WASD + mouse-look flying. FlyLook
	// rotates the view in place, keeping the eye fixed (deltas in degrees, e.g.
	// mouse pixels * a look speed). FlyMove translates the eye through the world
	// along the current view basis: 'forward' along the look direction, 'right'
	// along screen-right, 'up' along world +Z, in world units.
	void FlyLook( float dYawDeg, float dPitchDeg );
	void FlyMove( float forward, float right, float up );

	// Builds a world-space pick ray for a viewport pixel in the 3D view (origin at
	// the eye, 'outDir' normalized into the scene). Returns false for 2D views, so
	// the caller can fall back to the 2D projection pick. Used for click-to-select.
	bool PixelToRay(
	    float px, float py, int widthPx, int heightPx, float outOrigin[3], float outDir[3] ) const;

	int SolidCount() const { return m_solidCount; }
	int TriangleCount() const { return m_triCount; }

private:
	void ReleaseGl();
	// Derives the 3D eye position and orthonormal view basis (forward = look
	// direction, right = screen-right, up = screen-up) from the orbit camera, the
	// same way RenderPerspective builds its view matrix. Shared by fly + pick.
	void CameraVectors( float eye[3], float forward[3], float right[3], float up[3] ) const;
	void RenderPerspective( int w, int h );
	void RenderOrtho( int w, int h );
	void BuildGrid( int w, int h ); // fills the dynamic grid buffer for 2D
	void AxisIndices( int &uAxis, int &vAxis ) const;

	// Gets or lazily creates the GL texture for a material's base texture, via the
	// catalog. Returns 0 (no texture) when there is no catalog, no material, or no
	// decodable image; the miss is cached. A GL context must be current.
	unsigned int TextureFor( const std::string &material );

	// A run of mesh vertices sharing one GL texture (0 = draw flat/untextured).
	struct MeshRange
	{
		unsigned int texture = 0;
		int firstVertex = 0;
		int vertexCount = 0;
	};

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

	hammer::formats::MaterialCatalog *m_catalog = nullptr;
	std::map<std::string, unsigned int> m_textures; // material name -> GL texture (0 = miss)
	std::vector<MeshRange> m_meshRanges;            // per-texture draw runs over the mesh VBO

	float m_sceneCenter[3] = { 0.0f, 0.0f, 0.0f };
	float m_sceneSize[3] = { 512.0f, 512.0f, 512.0f };
	float m_sceneRadius = 512.0f;
	bool m_haveScene = false;

	ViewMode m_mode = ViewMode::Perspective;
	Camera m_camera;
	Ortho2D m_ortho;
	int m_highlightId = -2147483647; // INT_MIN-ish: highlight nothing by default
	std::vector<int> m_highlights;   // additional selected solid ids (multi-select)
	bool m_initialized = false;
};

} // namespace hammergtk

#endif // HAMMER_GTK_RENDERER_H
