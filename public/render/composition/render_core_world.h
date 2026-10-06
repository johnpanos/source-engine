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
#include "render/draw_phase.h"
#include "render/world_mesh_upload.h"

class ITexture;
class IMaterial;

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
	// Studio's material classification for accepting one half of a two-pass model.
	bool translucent = false;
	bool hasProxy = false; // selected proxy materials need a live draw-time handoff
};

// Source static props enter as raw Studio files and the material system's
// resolved material descriptors. The core parses the bytes, owns one mesh
// per model and draws accepted instances at the queued world view's slot.
// Pointers are borrowed only for SetStaticProps; no C++ library type crosses
// the engine/core ABI boundary.
struct RenderCoreStaticModel
{
	const char *name;
	const void *mdl;
	unsigned long long mdlBytes;
	const void *vvd;
	unsigned long long vvdBytes;
	const void *vtx;
	unsigned long long vtxBytes;
	const RenderCoreWorldMaterial *materials; // by Studio texture index, skin 0
	unsigned int materialCount;
	// Texture-slot descriptors for each resolved LOD, packed LOD-major. A
	// single level may serve unchanged slots at other levels; a replacement
	// without its resolved descriptor cannot be claimed.
	unsigned int materialLodCount = 1;
	// The host may draw this model as a posed (animated) model, not only as
	// static instances. A posed model keeps a per-frame CPU copy of its
	// skinning geometry and its levels stay resident; a static-only model does
	// not. True unless the host says otherwise, so a host that cannot tell
	// keeps every model posable.
	bool posed = true;
};

struct RenderCoreStaticProp
{
	unsigned int model; // into the models array
	int skin;
	float world[12];         // model to world, three rows of four
	bool castsShadow = true; // authored static-prop and Studio shadow flags
};

// One visible static-prop instance with the LOD selected by the engine for
// this view. The queued view owns the resulting geometry selection.
struct RenderCoreStaticPropDraw
{
	unsigned int prop;
	unsigned int lod;
};

// One Studio draw at the engine's current animation pose. The model index
// names a mesh registered by SetStaticProps; boneToWorld holds boneCount
// contiguous 3x4 row-major matrices. Borrowed only for DrawView.
struct RenderCorePosedModel
{
	unsigned int model;
	unsigned int skin;
	const float *boneToWorld;
	unsigned int boneCount;
	RenderCoreDrawPhase phase = RenderCoreDrawPhase::kAll;
	int body = 0;         // engine-selected Studio body groups, captured with the pose
	unsigned int lod = 0; // the host's selected level, including root-LOD policy
	unsigned long long motionIdentity = 0; // non-reused model instance generation
};

// The world stage's quality settings (RFC 0016 K12); each field has an
// engine r_core_* ConVar and a video option.
struct RenderCoreWorldQuality
{
	// 0 off (no prepass, no GTAO: the term is neutral), then slices x steps
	// set by render.lab.gtao against Cycles: 1 low (2 x 6) and 2 medium
	// (3 x 8) at half resolution, 3 high (5 x 8 at half resolution, every lab
	// check passes), 4 ultra (8 x 8 at full resolution, the lab's own).
	int ambientOcclusion;
	// 0 off (the stage's lights unshadowed), then the shadow atlas: 1 low
	// (2048 texels), 2 medium (4096), 3 high (8192).
	int shadows;
	// Nonzero: the stage's opaque surfaces' depth is drawn before they are
	// lit, so each pixel is shaded once (a performance setting only).
	int depthPrepass;
	// Nonzero: the frame's moving objects (render.dynamic-occlusion boxes)
	// cast shadows over the cached static tiles.
	int shadowMovers;
	// Nonzero: runtime direct light (id Tech's split): the world's lightmap
	// is its indirect layer and every light's direct light is drawn at
	// runtime, shadowed, so moving objects block it; zero draws the bake's
	// total layer. A map without an indirect layer draws its total layer.
	// Read when a stage is set (a map load).
	int runtimeDirect;
	// Enabled product rendercore owns shading exclusively; legacy draws are rejected.
	bool coreOnly = false;
	// Unfinished ordered dynamic handoff. Disabled in the playable composition
	// until queued/capture/resize and image acceptance pass for the whole cohort.
	bool dynamicDraws = false;
	// Nonzero: a stage map's participating media (its fog volumes and
	// controller) are composited over its views (render.pass.volumetric);
	// zero leaves the term out, as render_lab's --no-volumetric.
	int volumetric = 1;
	// Nonzero: render.ssr.v1's glossy reflections over a stage view's lit
	// frame (single-sample, non-temporal, HDR float targets); zero leaves the
	// term out, as render_lab's --no-ssr.
	int ssr = 1;
	// Nonzero: GPU-driven submission (RFC 0016 S3/S4): a stage view's world
	// surfaces are culled and compacted on the GPU into one indirect draw per
	// (material, lightmap page) bucket (render.pass.world's gpuSubmission).
	// 2 also occlusion-culls them against the view's world depth
	// (render.pass.world's gpuOcclusion).
	// Opt-in until RFC 0003's placement measurement selects it.
	int gpuSubmission = 0;
};

// The game's files as the core reads them (projector cookies,
// materials/<name>.vtf): size returns the file's bytes (0 when absent) and
// read fills exactly that many. Main thread.
struct RenderCoreFileSource
{
	unsigned long long ( *size )( const char *path ) = nullptr;
	bool ( *read )( const char *path, void *out, unsigned long long bytes ) = nullptr;
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
	unsigned long long staticInstancesQueued;
	unsigned long long staticDrawsDrawn;
	// The screen passes' prepass draw lists: builds, reuses and rebuilds
	// after a material or group in them was not ready.
	unsigned long long prepassListBuilds;
	unsigned long long prepassListReuses;
	unsigned long long prepassListRetries;
	unsigned long long posedModelsQueued;
	unsigned long long posedDrawsDrawn;
	// Model geometry (RFC 0016 residency): the published (model, hardware
	// level) blocks, the CPU geometry bytes they share (one copy per block,
	// never one per owner), and the models that keep a per-frame skinning copy
	// because the host may pose them, with those copies' bytes.
	unsigned int modelLevels;
	unsigned long long modelStagingBytes;
	unsigned int modelPoseSources;
	unsigned long long modelPoseBytes;
	unsigned long long dynamicDrawsDrawn;
	unsigned long long dynamicDrawsRefused;
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
	unsigned long long stageLightingBuilds;
	// Views that drew no world geometry and read the frame's stage lighting
	// instead of planning their own (the viewmodel scope).
	unsigned long long stageSharedViews;
	// Nonzero: the world stage draws its lights' direct light at runtime
	// over the lightmap's indirect layer (RenderCoreWorldQuality::runtimeDirect).
	unsigned int stageRuntimeDirect;
	// The stage map's participating media (render.pass.volumetric, RFC 0016
	// K12): nonzero when its entity lump holds a fog volume or controller;
	// stage views the medium was composited over, and those it refused by
	// name (multisampled targets, a target without sampled depth, a pass
	// that does not record).
	unsigned int volumetricMedium;
	unsigned long long volumetricViews;
	unsigned long long volumetricRefused;
	// The frame's projected lights (env_projectedtexture) the stage views
	// light with, and those refused by name (a cookie that is missing, does
	// not decode or differs in size from the first; no file source).
	unsigned int projectorsLit;
	unsigned long long projectorsRefused;
	// Cutout shadow casters (alpha-tested world surfaces through their
	// materials' coverage): draws into the shadow atlas, and surfaces refused
	// by name (an animated cutout; groups not ready).
	unsigned long long cutoutShadowDraws;
	unsigned long long cutoutShadowRefused;
	unsigned long long cutoutShadowNotResident; // static props whose level is off the device
	// Stage views the screen-space reflections traced, and those refused by
	// name (a multisampled, temporal or 8-bit target; a pass that does not
	// record).
	unsigned long long ssrViews;
	unsigned long long ssrRefused;
	char lastFailure[256];
	char lastRefusal[256];
	char gaps[16384];   // bounded scene census: "count reason" lines, most frequent first
	char claimed[1024]; // "surfaces material" lines the core draws
	// GPU-driven submission (r_core_world_gpu_submit): views drawn by it, their
	// indirect draws (one per material and lightmap page), and views that
	// asked for it on a device without the capabilities.
	unsigned long long gpuViews;
	unsigned long long gpuIndirectDraws;
	unsigned long long gpuFallbacks;
	// Of gpuViews, those occlusion-culled (r_core_world_gpu_submit 2), and
	// the depth pyramids built for them.
	unsigned long long gpuOcclusionViews;
	unsigned long long gpuPyramids;
};

// Debug-only snapshot of the latest completed core frame, ABI-safe across
// the engine/core bridge. Inclusive rows must not be added together. CPU is
// command-recording wall time, not game logic or GPU execution; GPU is elapsed
// timestamp time, not per-pixel attribution. Neither includes legacy/present.
struct RenderCoreCostRow
{
	char name[96] = {};
	unsigned int depth = 0;
	double cpuMilliseconds = 0;
	double gpuMilliseconds = 0;
	unsigned long long created = 0, destroyed = 0, bufferBytes = 0;
	bool resourcesSupported = false;
};

struct RenderCoreResourceSample
{
	unsigned long long frame = 0;
	unsigned long long created = 0, destroyed = 0, released = 0;
	unsigned long long bufferBytes = 0, live = 0, pending = 0;
	unsigned long long buffers = 0, textures = 0, groups = 0, other = 0;
	bool supported = false;
};

struct RenderCoreCostReport
{
	static constexpr unsigned int kCapacity = 64;
	RenderCoreCostRow rows[kCapacity];
	RenderCoreResourceSample resources;
	RenderCoreResourceSample history[kCapacity];
	unsigned int historyCount = 0;
	unsigned int count = 0;
	unsigned int omitted = 0;
	unsigned int dropped = 0;
	unsigned long long frame = 0;
	unsigned long long currentFrame = 0;
	bool available = false; // a device has attempted to record timers
	bool supported = false;
	RenderCoreCostReport() = default;
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
	// `entities`: the map's entity lump, whose authored lights (light,
	// light_spot, light_rect, light_environment) light the stage when the
	// frame's light set has no world lights (a map compiled without vrad).
	virtual void SetWorldMesh( const void *wmsh, unsigned long long wmshBytes,
	    const RenderCoreWorldMeshlet *meshlets, unsigned int meshletCount,
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount,
	    const char *entities ) = 0;
	// Main thread, after SetWorldMesh at level load. Unsupported models and
	// materials remain on studiorender; DrawsStaticProp decides per instance
	// and selected LOD.
	virtual void SetStaticProps( const RenderCoreStaticModel *models, unsigned int modelCount,
	    const RenderCoreStaticProp *props, unsigned int propCount ) = 0;
	virtual bool DrawsStaticProp( unsigned int prop, unsigned int lod = 0 ) const = 0;
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
	// runtime lights for the view with them; worldToView's x axis also
	// orients the water point's reflection. waterZOffset is the height the
	// view moves its water surfaces by (the client's waterZAdjust, 0 but for
	// an eye within r_eyewaterepsilon of a water plane).
	virtual bool DrawView( const unsigned int *surfaces, unsigned int count,
	    const float worldToClip[16], const float viewport[6], unsigned long long hostFrame,
	    const float worldToView[16], const float viewToClip[16], float waterZOffset,
	    const RenderCoreStaticPropDraw *staticProps, unsigned int staticPropCount,
	    const RenderCorePosedModel *posedModels, unsigned int posedModelCount ) = 0;
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
	virtual void SelectTemporalView( unsigned long long identity ) = 0;
	virtual void ResetTemporalHistory() = 0;
	virtual bool CaptureTemporalInputs( const char *prefix, bool afterReset ) = 0;
	virtual void CommitTemporalFrame( bool submitted ) = 0;
	virtual bool TemporalEnabled() const = 0;
	virtual void TemporalJitter( float *x, float *y ) const = 0;
	virtual bool ReconstructTemporal( int x, int y, int renderWidth, int renderHeight,
	    int outputWidth, int outputHeight, float deltaMilliseconds ) = 0;
	// Views and claimed materials the core failed to draw, so far (never
	// drawn by legacy instead: the caller's policy decides what a failure
	// costs).
	virtual unsigned long long Failures() const = 0;
	virtual IMaterial *NeutralMaterial( const char * ) { return nullptr; }

	virtual void GetStats( RenderCoreWorldStats *out ) const = 0;
	// RFC 0014 D4 (cl_render_debug_gpu_timers): whether the stage times its
	// labeled GPU sections (shadows, prepass, lit world, GTAO), from its next
	// slot. Main thread.
	virtual void SetGpuTimers( bool enabled ) = 0;
	// The stage's quality settings, from its next view. Main thread.
	virtual void SetQuality( const RenderCoreWorldQuality &quality ) = 0;
	// The game's files (projector cookies). Main thread, before the views
	// that need them; without one the frame's projectors are refused.
	virtual void SetFileSource( const RenderCoreFileSource & ) {}
	// The sections' GPU time since the last call, over the frames read (the
	// return value; 0 when none was), as "depth ms-per-frame
	// count-per-frame name" lines in `out`. Main thread.
	virtual unsigned int TakeGpuTimes( char *out, unsigned int size ) = 0;
	// Main thread: nonblocking, non-destructive snapshot, independent of the
	// console report. The core owns measurement; the host owns presentation.
	virtual void ReadCosts( RenderCoreCostReport *out ) = 0;

protected:
	~IRenderCoreWorld() = default;
};

#endif // RENDER_COMPOSITION_RENDER_CORE_WORLD_H
