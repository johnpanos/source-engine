//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: OpenGL 3.3-core renderer for the GTK Hammer desktop shell's viewports
//			(RFC 0002, linux-gtk-desktop). It draws what the headless editor
//			presents and owns no editor state of its own:
//
//			  * geometry: a hammer::viewport::RenderSnapshot (solids with their
//			    faces and displacements, point-entity markers, selection flags);
//			  * the view: a hammer::viewport::Camera2D or Camera3D, the cameras the
//			    EditorWorkspace owns and its CameraController moves, so what is
//			    drawn and what a pointer event means come from one camera;
//			  * decoration: the workspace's grid lines (2D) and the active tool's
//			    tools::OverlayList (pending boxes, handles, marquees, clip lines).
//
//			It is free of any GTK/GDK dependency: it draws into the currently
//			bound framebuffer, so the same code serves each interactive GtkGLArea
//			and the offscreen EGL screenshot path. Cameras are in logical pixels;
//			the framebuffer size may differ (HiDPI). GL detail stays confined here.
//			Map geometry is Source's Z-up world space. Overlay labels are not drawn
//			(no GL text); the host shows tool status in its status bar.
//
//=============================================================================//

#ifndef HAMMER_GTK_RENDERER_H
#define HAMMER_GTK_RENDERER_H

#include "hammer/formats/material_catalog.h"
#include "hammer/tools/input.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/extraction.h"
#include "hammer/viewport/grid.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace hammergtk
{

class Renderer
{
public:
	Renderer() = default;
	~Renderer();

	Renderer( const Renderer & ) = delete;
	Renderer &operator=( const Renderer & ) = delete;

	// Creates GL programs/buffers. A GL 3.3 context must already be current.
	bool Init( std::string &error );

	// Optional material catalog for textured shading of the 3D view. When set (and
	// a face's material resolves to a decoded base texture), faces are drawn
	// textured with world-planar UVs; otherwise flat. Borrowed; it must outlive
	// the renderer. Textures are created lazily in SetSnapshot.
	void SetMaterialCatalog( hammer::formats::MaterialCatalog *catalog ) { m_catalog = catalog; }

	// Uploads a snapshot's geometry, replacing the previous one. Selected
	// solids, faces and entities are drawn highlighted. A GL context must be
	// current.
	void SetSnapshot( const hammer::viewport::RenderSnapshot &snapshot );

	// Draws a 2D wireframe view: grid, solid and entity edges, then the overlay.
	// 'fbWidth'/'fbHeight' are the framebuffer's pixels; the camera's viewport
	// is in logical pixels.
	void Render2D( const hammer::viewport::Camera2D &camera,
	    const std::vector<hammer::viewport::GridLine> &grid,
	    const hammer::tools::OverlayList &overlay, int fbWidth, int fbHeight );

	// Draws the shaded 3D view, then the overlay.
	void Render3D( const hammer::viewport::Camera3D &camera,
	    const hammer::tools::OverlayList &overlay, int fbWidth, int fbHeight );

	int SolidCount() const { return m_solidCount; }
	int EntityCount() const { return m_entityCount; }
	int TriangleCount() const { return m_triCount; } // solid and displacement triangles

private:
	void ReleaseGl();
	void DrawOverlay( const hammer::tools::OverlayList &overlay, const float worldMvp[16],
	    const float screenMvp[16], bool depthTest );
	void DrawDynamic( const std::vector<float> &vertices, unsigned int mode, const float mvp[16] );

	// Gets or lazily creates the GL texture for a material's base texture, via the
	// catalog. Returns 0 when there is no catalog, no material, or no decodable
	// image; the miss is cached. A GL context must be current.
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
	unsigned int m_dynVao = 0;
	unsigned int m_dynVbo = 0;
	int m_meshVertexCount = 0;
	int m_lineVertexCount = 0;
	int m_triCount = 0;
	int m_solidCount = 0;
	int m_entityCount = 0;

	hammer::formats::MaterialCatalog *m_catalog = nullptr;
	std::map<std::string, unsigned int> m_textures; // material name -> GL texture (0 = miss)
	std::vector<MeshRange> m_meshRanges;            // per-texture draw runs over the mesh VBO

	std::optional<hammer::scene::Box> m_bounds; // of the snapshot, for the 3D depth range
	bool m_initialized = false;
};

} // namespace hammergtk

#endif // HAMMER_GTK_RENDERER_H
