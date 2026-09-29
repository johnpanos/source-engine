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

class ITexture;

struct RenderCoreWorldVertex
{
	float position[3];
	float uv[2];
	float lightmapUv[2]; // in the page, offset applied
	unsigned char color[4];
};

struct RenderCoreWorldSurface
{
	unsigned int material;   // into the materials
	int lightmapPage;        // the material system's lightmap page index
	unsigned int firstIndex; // into the indices
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
	virtual void ClearWorld() = 0;
	// Whether the core draws the material's surfaces.
	virtual bool Draws( unsigned int material ) const = 0;
	// Queues a view's visible surfaces that the core draws, with the view's
	// world-to-clip (row-major, column vectors, D3D9 conventions), viewport
	// (x, y, width, height, min and max depth) and host frame, and marks its
	// slot at this point of the frame's stream. False when nothing was queued
	// (no surface, or no backend slots): then the caller draws them itself.
	// A view of a host frame the backend never records counts as skipped.
	virtual bool DrawView( const unsigned int *surfaces, unsigned int count,
	    const float worldToClip[16], const float viewport[6], unsigned long long hostFrame ) = 0;
	// Views and claimed materials the core failed to draw, so far (never
	// drawn by legacy instead: the caller's policy decides what a failure
	// costs).
	virtual unsigned long long Failures() const = 0;
	virtual void GetStats( RenderCoreWorldStats *out ) const = 0;

protected:
	~IRenderCoreWorld() = default;
};

#endif // RENDER_COMPOSITION_RENDER_CORE_WORLD_H
