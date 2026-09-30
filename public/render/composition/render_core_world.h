//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RenderCoreBinding::world (RFC 0016 K5): the BSP world drawn by the
//			render core (render.pass.world), as the engine sees it. Plain
//			structs, C strings and material system types only, and no render
//			namespace, so engine files that hold BSP types (whose global
//			`render` clashes with the core's namespace) can include it.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_RENDER_CORE_WORLD_H
#define RENDER_COMPOSITION_RENDER_CORE_WORLD_H

#include "render/light_set.h"
#include "render/world_mesh_upload.h"

class ITexture;

struct RenderCoreWorldVertex
{
	float position[3];
	float uv[2];
	float lightmapUv[2]; // in the page, offset applied
	unsigned char color[4];
	float normal[3];
	float tangentS[3];
	float tangentT[3];
	float lightmapOffset; // the bumped pages' offset in the page (0 for a flat lightmap)
};

struct RenderCoreWorldSurface
{
	unsigned int material;   // into the materials
	int lightmapPage;        // the material system's lightmap page index
	unsigned int firstIndex; // into the indices
	unsigned int indexCount;
};

// A world stage's surface (RFC 0016 K12): one WMSH meshlet, an index range
// of the world mesh drawn with one material.
struct RenderCoreWorldMeshlet
{
	unsigned int material;   // into the materials
	unsigned int firstIndex; // into the mesh's indices
	unsigned int indexCount;
};

struct RenderCoreWorldMaterial
{
	const char *name;
	const char *shader;
	int variableCount;
	const char *const *keys; // "$basetexture", ...
	const char *const *values;
	// The texture variables' textures, parallel to keys (null where the
	// variable holds no texture).
	ITexture *const *textures;
	// The shader's declared default of each variable, parallel to keys (null
	// where the variable is not a shader parameter).
	const char *const *defaults;
};

struct RenderCoreWorldStats
{
	unsigned int materials;
	unsigned int claimedMaterials;
	unsigned int surfaces;
	unsigned int claimedSurfaces;
	unsigned long long viewsQueued;
	unsigned long long viewsDrawn;
	unsigned long long viewsFailed;
	unsigned long long viewsSkipped; // views of host frames the backend never recorded
	unsigned long long surfacesDrawn;
	// RFC 0014 debug slots: frames hatched (a pixel view or legacy 2),
	// frames tinted (legacy 1) and the top-level views drawn again over the
	// tint.
	unsigned long long debugHatches;
	unsigned long long debugTints;
	unsigned long long debugViewsRedrawn;
	// A world stage's runtime lights (RFC 0016 K12): the frame's light set
	// as last published, and the views queued with their lights clustered.
	unsigned int stageLights;
	unsigned long long stageLitViews;
	char lastFailure[256];
	char gaps[1024];    // "count reason" lines, most frequent first
	char claimed[1024]; // "surfaces material" lines the core draws
};

class IRenderCoreWorld
{
public:
	// Main thread, at level load and shutdown.
	virtual void SetWorld( const RenderCoreWorldVertex *vertices, unsigned int vertexCount,
	    const unsigned int *indices, unsigned int indexCount,
	    const RenderCoreWorldSurface *surfaces, unsigned int surfaceCount,
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) = 0;
	// A BSP2 map's world mesh, drawn as a world stage (RFC 0016 K12, as
	// render_lab draws it: world pbr from the map's lightmap, probes and
	// reflection probes): the validated WMSH lump, its meshlets and their
	// materials. Views then name meshlets. The stage's lighting arrives
	// through StageUpload() before this call and changes through it after.
	// In place of SetWorld; ClearWorld ends both.
	virtual void SetWorldMesh( const void *wmsh, unsigned long long wmshBytes,
	    const RenderCoreWorldMeshlet *meshlets, unsigned int meshletCount,
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) = 0;
	// The world stage's copy of the engine's world mesh uploads: the engine
	// makes every upload it makes to the renderer here too (its lightmap
	// layers, recomposed as moving objects block baked light; the probe
	// volume and its change from the bake; the reflection probes). The mesh
	// upload and DrawBatch are not used. Main thread.
	virtual world_mesh_gpu::IWorldMeshUpload *StageUpload() = 0;
	// The world stage's copy of the frame's light set (RFC 0011
	// render.light-set.v1): the engine publishes each frame's set here as it
	// does to the renderer. Main thread.
	virtual light_set::ILightSetConsumer *StageLights() = 0;
	// Whether a texture has reached the renderer (the core imports it by its
	// material system handle). Main thread.
	virtual bool TextureResident( ITexture *texture ) const = 0;
	virtual void ClearWorld() = 0;
	// Whether the core draws the material's surfaces.
	virtual bool Draws( unsigned int material ) const = 0;
	// Queues a view's visible surfaces that the core draws, with the view's
	// world-to-clip (row-major, column vectors, D3D9 conventions), viewport
	// (x, y, width, height, min and max depth) and host frame, and marks its
	// slot at this point of the frame's stream. False when nothing was queued
	// (no surface, or no backend slots): then the caller draws them itself.
	// A view of a host frame the backend never records counts as skipped.
	// worldToView and viewToClip (the same conventions; either may be null)
	// place a world stage's view lights: the core clusters the frame's
	// runtime lights for the view with them.
	virtual bool DrawView( const unsigned int *surfaces, unsigned int count,
	    const float worldToClip[16], const float viewport[6], unsigned long long hostFrame,
	    const float worldToView[16], const float viewToClip[16] ) = 0;
	// Main thread, at the start of each frame, after the renderer began it
	// (its debug controls are applied). Under a pixel view or
	// cl_render_debug_legacy 2 (RFC 0014) it marks the frame's first slot:
	// the core draws the not-applicable hatch there, and the legacy stream is
	// off from it to the frame's end. Otherwise it marks nothing.
	virtual void BeginFrame() = 0;
	// Main thread, after the frame's last draw and before its present. Under
	// cl_render_debug_legacy 1 (RFC 0014) it marks the frame's last slot, where
	// the core tints magenta what it did not draw. Otherwise it marks nothing.
	virtual void EndFrame() = 0;
	// Views and claimed materials the core failed to draw, so far (never
	// drawn by legacy instead: the caller's policy decides what a failure
	// costs).
	virtual unsigned long long Failures() const = 0;
	virtual void GetStats( RenderCoreWorldStats *out ) const = 0;

protected:
	~IRenderCoreWorld() = default;
};

#endif // RENDER_COMPOSITION_RENDER_CORE_WORLD_H
