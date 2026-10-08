//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
//
// The Nintendo 3DS shader API: a fullbright renderer for the PICA200.
//
// Derived from the empty shader API. The material system drives it as it
// drives the D3D9 one: meshes keep their vertices and indices in GPU-visible
// linear memory, a draw runs the bound material's shader (so the material's
// own texture bindings and render state reach this API), and each RenderPass
// draws the mesh with the pass's recorded state and the texture bound to
// sampler 0, modulated by the vertex color. Lighting, shader constants and
// every other sampler are ignored: the image is the materials' base textures,
// fullbright. Draws into render targets are skipped (only the back buffer is
// drawn). Textures are uploaded as ETC1, ETC1A4 or RGBA8 at most
// pica_texture_size texels a side. pica_renderer draws through the render
// core's PICA200 device (render.device.pica, RFC 0026), whose conventions
// its own suite proves (render.device.v2.pica).
//
//===========================================================================//

#include <cmath>
#include <cstring>

#include "utlvector.h"
#include "materialsystem/imaterialsystem.h"
#include "IHardwareConfigInternal.h"
#include "shadersystem.h"
#include "shaderapi/ishaderutil.h"
#include "shaderapi/ishaderapi.h"
#include "materialsystem/imesh.h"
#include "tier0/dbg.h"
#include "materialsystem/idebugtextureinfo.h"
#include "materialsystem/deformations.h"
#include "render/legacy/stream_device.h"
#include "render/legacy_shader_provider.h"
#include "imaterialinternal.h"
#include "shaderapi/commandbuffer.h"
#include "bitmap/imageformat.h"
#include "tier0/icommandline.h"
#include "core_copies.h"
#include "pica_renderer.h"
#include "renderparm.h"
#include "pixelwriter.h"
#include "render/legacy/core_mesh_kind.h"
#include "render/legacy/core_passes.h"
#include "render/legacy/material_flag_keys.h"
#include "itextureinternal.h"
#include "texture_group_names.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <malloc.h>

#include "pica_texture.h"
#if defined( PLATFORM_3DS )
extern "C" unsigned int linearSpaceFree( void ); // libctru: GPU-visible linear heap
extern "C" unsigned int __ctru_heap_size; // libctru: the main heap's size

// libctru's (linked into the program, not into a composed module): the system
// tick the harness's guest-time profiler stamps its samples with.
extern "C" unsigned long long svcGetSystemTick( void );
#else
// Elsewhere (RFC 0029: the browser) the device's memory is not the heap's,
// and the profiler's ticks are the 3DS's rate from the platform clock.
static unsigned int linearSpaceFree( void ) { return 0; }
static const unsigned int __ctru_heap_size = 0;
static unsigned long long svcGetSystemTick( void )
{
	return (unsigned long long)( Plat_FloatTime() * 268111856.0 );
}
#endif


//-----------------------------------------------------------------------------
// The empty mesh
//-----------------------------------------------------------------------------
namespace
{
struct CoreCache;
}

class CEmptyMesh : public IMesh
{
public:
	CEmptyMesh( bool bIsDynamic );
	virtual ~CEmptyMesh();

	// FIXME: Make this work! Unsupported methods of IIndexBuffer + IVertexBuffer
	virtual bool Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t& desc );
	virtual void Unlock( int nWrittenIndexCount, IndexDesc_t& desc );
	virtual void ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t& desc );
	virtual void ModifyEnd( IndexDesc_t& desc );
	virtual void Spew( int nIndexCount, const IndexDesc_t & desc );
	virtual void ValidateData( int nIndexCount, const IndexDesc_t &desc );
	virtual bool Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc );
	virtual void Unlock( int nVertexCount, VertexDesc_t &desc );
	virtual void Spew( int nVertexCount, const VertexDesc_t &desc );
	virtual void ValidateData( int nVertexCount, const VertexDesc_t & desc );
	virtual bool IsDynamic() const { return m_bIsDynamic; }
	virtual void BeginCastBuffer( VertexFormat_t format ) {}
	virtual void BeginCastBuffer( MaterialIndexFormat_t format ) {}
	virtual void EndCastBuffer( ) {}
	virtual int GetRoomRemaining() const { return 0; }
	virtual MaterialIndexFormat_t IndexFormat() const { return MATERIAL_INDEX_FORMAT_UNKNOWN; }

	void LockMesh( int numVerts, int numIndices, MeshDesc_t& desc );
	void UnlockMesh( int numVerts, int numIndices, MeshDesc_t& desc );

	void ModifyBeginEx( bool bReadOnly, int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t& desc );
	void ModifyBegin( int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t& desc );
	void ModifyEnd( MeshDesc_t& desc );

	// returns the # of vertices (static meshes only)
	int  VertexCount() const;

	// Sets the primitive type
	void SetPrimitiveType( MaterialPrimitiveType_t type );
	 
	// Draws the entire mesh
	void Draw(int firstIndex, int numIndices);

	void Draw(CPrimList *pPrims, int nPrims);

	// Copy verts and/or indices to a mesh builder. This only works for temp meshes!
	virtual void CopyToMeshBuilder( 
		int iStartVert,		// Which vertices to copy.
		int nVerts, 
		int iStartIndex,	// Which indices to copy.
		int nIndices, 
		int indexOffset,	// This is added to each index.
		CMeshBuilder &builder );

	// Spews the mesh data
	void Spew( int numVerts, int numIndices, const MeshDesc_t & desc );

	void ValidateData( int numVerts, int numIndices, const MeshDesc_t & desc );

	// gets the associated material
	IMaterial* GetMaterial();

	// A static prop's baked vertex lighting (studiorender binds it per draw):
	// its colour mesh's record colours from nVertexOffset, which the render
	// core's baked vertex light point reads (EmitSurfaceToCore).
	void SetColorMesh( IMesh *pColorMesh, int nVertexOffset )
	{
		m_bHasColorMesh = pColorMesh != NULL;
		m_pColorMesh = static_cast<CEmptyMesh *>( pColorMesh );
		m_nColorMeshOffset = nVertexOffset;
	}
	bool m_bHasColorMesh = false;
	CEmptyMesh *m_pColorMesh = nullptr;
	int m_nColorMeshOffset = 0;


	virtual int IndexCount() const
	{
		return m_nIndices;
	}

	virtual void SetFlexMesh( IMesh *pMesh, int nVertexOffset ) {}

	virtual void DisableFlexMesh() {}

	virtual void MarkAsDrawn() {}

	virtual unsigned ComputeMemoryUsed() { return 0; }

	virtual VertexFormat_t GetVertexFormat() const { return m_Format; }
	void SetVertexFormat( VertexFormat_t format ) { m_Format = format; }

	// Draws the current range with the state of the pass in progress.
	void RenderPass();

	virtual IMesh *GetMesh()
	{
		return this;
	}

private:
	// exact: a lock that is not appending sizes the buffer to the count
	// (static meshes are filled once; doubling wasted linear memory).
	bool EnsureVertices( int count, bool exact = false );
	bool EnsureIndices( int count, bool exact = false );
	// A dynamic mesh's storage is CPU memory: every draw copies it into the
	// frame's transient ring, so it never needs GPU-visible linear memory.
	void *AllocStorage( pica::Memory kind, size_t bytes );
	void FreeStorage( void *storage );
	int SourceVertexCount() const { return m_pVertexSource ? m_pVertexSource->m_nVertices : m_nVertices; }
public:
	void SetVertexSource( CEmptyMesh *source ) { m_pVertexSource = source != this ? source : NULL; }
private:
	void DrawRange( int firstIndex, int indexCount );
	// RFC 0026 P3: a VertexLitGeneric draw handed to the render core's model
	// point (reduced model lighting); false leaves it to this backend.
	bool EmitToCore( int firstIndex, int indexCount );
	// Every other bound material's draw handed to its render core point
	// (core_mesh_kind.h): world-space vertices built per draw, as the native
	// Vulkan frontend's; false when the core refuses it.
	bool EmitSurfaceToCore( int firstIndex, int indexCount, render::legacy::CoreMeshKind kind );
	const std::vector<std::uint16_t> *CoreTriangleList( const CEmptyMesh &src, int firstIndex,
		int indexCount, bool cacheable, std::vector<std::uint16_t> &triangles );

	bool m_bIsDynamic;
	// The triangles EmitToCore built from this mesh's contents (CoreCache), valid while
	// m_nContentRevision is unchanged: every Lock and Modify bumps it.
	unsigned m_nContentRevision = 0;
	std::unique_ptr<CoreCache> m_pCoreCache;
	// The mesh whose vertices this one's indices address: a vertex override
	// (GetDynamicMesh's pVertexOverride, as the world's index batches use),
	// else this mesh.
	CEmptyMesh *m_pVertexSource;
	VertexFormat_t m_Format;
	MaterialPrimitiveType_t m_Type;
	// Vertices and indices in linear memory (the GPU reads them in place).
	pica::Vertex *m_pVertices;
	int m_nVertexCapacity;
	int m_nVertices;
	int m_nLockFirstVertex;
	// What the last Lock granted (0 when it failed): Unlock never records
	// more, so no draw reads past the buffer (the memory audit, 2026-10-07).
	int m_nLockedVertices = 0;
	unsigned short *m_pIndices;
	int m_nIndexCapacity;
	int m_nIndices;
	int m_nLockFirstIndex;
	int m_nLockedIndices = 0;
	// Skinning inputs (CPU memory), when the format has bone weights.
	float *m_pBoneWeights;     // 2 per vertex
	// Normals of a format with VERTEX_NORMAL (3 per vertex, CPU memory): the
	// PICA record holds none, and the core's model lighting reads them.
	float *m_pNormals = nullptr;
	int m_nNormalCapacity = 0;
	// The GPU-skinning palette slots of a skinned mesh's vertices (unorm8x4,
	// each a slot x 3; CoreMeshStreams' slots), with the bone each slot
	// holds: built from the bone weights once per content revision, in the
	// mesh's memory, when its bones fit the palette (m_nSkinBones > 0).
	unsigned char *m_pSkinSlots = nullptr;
	unsigned m_nSkinSlotsRevision = ~0u;
	int m_nSkinBones = 0;
	unsigned char m_SkinBones[render::material::kMaxReducedBones] = {};
	bool SkinSlots();
	// Before the vertex streams are rewritten: a new content revision, and a
	// recorded draw reading the in-place memory is submitted first.
	void PrepareVertexWrite()
	{
		++m_nContentRevision;
		pica::PrepareWrite( m_pVertices );
		pica::PrepareWrite( m_pNormals );
		pica::PrepareWrite( m_pBoneWeights );
	}
	// Texture coordinate 0 of a format with more than two components (sprite
	// cards, particles): the builder writes every component, so they go here,
	// 4 per vertex, and the record takes the first two at Unlock/ModifyEnd.
	// Writing them into the record's two floats overran into the next vertex
	// and, past the last, into the neighbouring allocation (the memory audit,
	// 2026-10-07).
	float *m_pWideTexCoords = nullptr;
	int m_nWideTexCoordCapacity = 0;
	// Texture coordinate 1 of a dynamic mesh (a decal's or overlay's lightmap
	// coordinates), every component of it (TexCoordSize( 1 ) per vertex, CPU
	// memory): the record has no room, and the render core's lightmapped
	// points read the first two (EmitSurfaceToCore's world vertices).
	float *m_pTexCoord1 = nullptr;
	int m_nTexCoord1Capacity = 0;
	int m_nModifyFirstVertex = 0;
	int m_nModifyVertexCount = 0;
	bool WideTexCoords() const { return TexCoordSize( 0, m_Format ) > 2; }
	bool EnsureWideTexCoords();
	void CommitWideTexCoords( int first, int count );
	unsigned char *m_pBoneIndices; // 4 per vertex
	int m_nBoneCapacity;
	// The draw in progress.
	int m_nDrawFirst;
	int m_nDrawCount;
	CPrimList *m_pPrims;
	int m_nPrims;
};


// The frontend's recorder the root binds (defined with the core pass slots).
extern render::legacy::ICorePassRecorder *g_CorePassRecorder;

//-----------------------------------------------------------------------------
// PICA state shared by the mesh, shadow and dynamic APIs.
//-----------------------------------------------------------------------------
namespace
{

struct PicaSnapshot
{
	pica::DrawState state;
	VertexFormat_t format;
	int polyOffset = 0; // PolygonOffsetMode_t (EnablePolyOffset), the core's depth bias
};

// Same recorded state (DrawState compared field by field: its padding is not
// initialized) and vertex format.
bool SameSnapshot( const PicaSnapshot &a, const pica::DrawState &s, VertexFormat_t format,
	int polyOffset )
{
	const pica::DrawState &t = a.state;
	return a.format == format && a.polyOffset == polyOffset && t.depthTest == s.depthTest && t.depthWrite == s.depthWrite &&
		t.depthFunc == s.depthFunc && t.blend == s.blend && t.src == s.src && t.dst == s.dst &&
		t.alphaTest == s.alphaTest && t.alphaFunc == s.alphaFunc && t.alphaRef == s.alphaRef &&
		t.cull == s.cull && t.colorWrite == s.colorWrite && t.alphaWrite == s.alphaWrite &&
		memcmp( t.tint, s.tint, sizeof( t.tint ) ) == 0;
}

// One texture handle: the GPU texture and what the material system uploaded.
struct PicaTexture
{
	bool used = false;
	bool renderTarget = false;
	bool depth = false;
	bool lightmap = false; // TEXTURE_GROUP_LIGHTMAP: kept RGBA8 (RGBA4 would band)
	int width = 0;
	int height = 0;
	int mipLevels = 1;
	bool wrapS = true;
	bool wrapT = true;
	// The levels chosen for the GPU, RGBA8 row-major, base first.
	CUtlVector< CUtlVector<unsigned char> > levels;
	// The memory audit: each level's FNV-1a when it arrived, checked at upload
	// (a stray write into the heap between the two shows as texture noise).
	CUtlVector<unsigned int> levelHashes;
	int baseWidth = 0;
	int baseHeight = 0;
	char name[48] = ""; // CreateTextures' debug name (-pica_dump_draws)
	// A cube map (off the 3DS): each face's base level, RGBA8 at cubeSize.
	bool cube = false;
	int cubeSize = 0;
	CUtlVector<unsigned char> cubeFaces[6];
	bool dirty = false;
	pica::Texture gpu;
	// Off the 3DS: the same levels decoded from sRGB when sampled, made on
	// the core's first sRGB import (D3D9 reads sRGB per sampler, so a texture
	// may be read both ways); refreshed with each upload after that.
	pica::Texture gpuSrgb;
	bool wantsSrgb = false;
};

enum MatrixStackId
{
	kStackView,
	kStackProjection,
	kStackModel,
	kStackCount
};

struct Matrix4
{
	float m[16];
};

constexpr int kMaxBones = 53;
constexpr int kStackDepth = 32;

CUtlVector<PicaSnapshot> g_Snapshots;
// The dynamic stencil state (IShaderDynamicAPI::SetStencil*), which the core's
// mesh slots take (DecorateCoreTarget).
struct CoreStencil
{
	bool enable = false;
	StencilOperation_t fail = STENCILOPERATION_KEEP;
	StencilOperation_t depthFail = STENCILOPERATION_KEEP;
	StencilOperation_t pass = STENCILOPERATION_KEEP;
	StencilComparisonFunction_t compare = STENCILCOMPARISONFUNCTION_ALWAYS;
	int reference = 0;
	uint32 testMask = 0xFFFFFFFF;
	uint32 writeMask = 0xFFFFFFFF;
};
CoreStencil g_CoreStencil;
float g_CoreShadowSlopeBias = 0.0f; // SetShadowDepthBiasFactors
float g_CoreShadowBias = 0.0f;
// Set by DecorateCoreDraw for the slot QueueCore marks next: a mesh slot takes
// the bound snapshot's raster state; the frontend's own slots keep theirs.
bool g_CoreMeshSlotPending = false;
// D3D9's dest-alpha depth range without float HDR (m_DestAlphaDepthRange), as
// shaderapivulkan holds it: the depth-alpha copy's encoding.
constexpr float kCoreDestAlphaDepthRange = 192.0f;
CUtlVector<PicaTexture *> g_Textures; // index = handle - 1
ShaderAPITextureHandle_t g_ModifyTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
ShaderAPITextureHandle_t g_BoundTextures[16];
// The lightmap page BindStandardTexture put on sampler 1 (else invalid).
ShaderAPITextureHandle_t g_BoundLightmap = INVALID_SHADERAPI_TEXTURE_HANDLE;
IMaterialInternal *g_pBoundMaterial = NULL;
// Model lighting as studiorender sets it (SetAmbientLightCube, SetLight): the
// render core's model point reads it at each draw (RFC 0026 P3).
float g_AmbientCube[6][3] = {};
constexpr int kPicaMaxLights = 4;
LightDesc_t g_Lights[kPicaMaxLights];
CEmptyMesh *g_pRenderMesh = NULL;
int g_CurrentSnapshot = -1;
// Whether draws reach a target: the back buffer, or a render target
// DrawableTarget admits (SetRenderTargetEx).
bool g_bDrawingToBackBuffer = true;
// R in the low byte, A in the high one (pica::Clear's rgba).
unsigned int g_ClearColor = 0xFF000000;
// The presented frame number.
int g_PicaFrame = 0;
ShaderViewport_t g_Viewport;
Matrix4 g_Stacks[kStackCount][kStackDepth];
int g_StackTop[kStackCount];
int g_CurrentStack = kStackModel;
float g_Bones[kMaxBones][12];
int g_MaxBone = -1;

// EmitToCore's triangles for a static mesh (RFC 0026): they depend only on
// the mesh's contents, so they are built once per content revision and drawn
// range, not every draw. Dynamic meshes are rewritten every frame and never
// cached. The caches of all meshes share kCoreCacheBudget (about 0.5 MB of
// triangles in the intro4 demo); past it, draws build as before. Vertices
// are not cached: at 72 bytes each they filled the budget for 10% of draws.
constexpr std::size_t kCoreCacheBudget = 2u << 20;
std::size_t g_CoreCacheBytes = 0;
struct CoreTriangles
{
	int first = 0;
	int count = 0;
	const CEmptyMesh *source = nullptr;
	unsigned sourceRevision = 0;
	std::vector<std::uint16_t> indices;
};
struct CoreCache
{
	// As an index mesh: triangles per drawn range, of this revision.
	unsigned triangleRevision = ~0u;
	std::vector<CoreTriangles> triangles;
	std::size_t bytes = 0;

	~CoreCache() { g_CoreCacheBytes -= bytes; }
	void Account()
	{
		std::size_t now = 0;
		for ( const CoreTriangles &t : triangles )
			now += t.indices.capacity() * sizeof( std::uint16_t );
		g_CoreCacheBytes = g_CoreCacheBytes - bytes + now;
		bytes = now;
	}
};
bool CoreCacheRoom( std::size_t bytes )
{
	if ( g_CoreCacheBytes + bytes <= kCoreCacheBudget )
		return true;
	static bool s_said = false;
	if ( !s_said )
	{
		s_said = true;
		printf( "pica: core model cache full (%u KB); later meshes build every draw\n",
			unsigned( g_CoreCacheBytes >> 10 ) );
	}
	return false;
}
float g_Modulation[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
#if defined( PLATFORM_3DS )
int g_TextureSizeCap = 128;
#else
// Elsewhere the content's own sizes (RFC 0029: the browser's WebGPU limit).
int g_TextureSizeCap = 4096;
#endif
// Per-present counters of the draw path (printed with the frame stats).
struct DrawPathCounters
{
	unsigned meshDraws = 0;      // IMesh::Draw calls
	unsigned primListDraws = 0;  // CPrimList draws (world surfaces) ...
	unsigned primListEmpty = 0;  // ... dropped: no vertices or prims
	unsigned materialDraws = 0;  // ... that ran a bound material
	unsigned renderPasses = 0;   // RenderPass calls
	unsigned targetSkips = 0;    // passes skipped while drawing to a render target
	unsigned clears = 0;         // back buffer clears
	unsigned overrunDraws = 0;   // refused: a count past the mesh's buffer
	unsigned wideTexCoordLocks = 0; // locks of a 3- or 4-component texcoord 0
	unsigned coreMeshDraws = 0;  // model draws the render core took (EmitToCore)
	unsigned errorMaterialDraws = 0; // draws with the error material, dropped
	unsigned refusedDraws = 0;   // draws the render core refused, dropped
	unsigned targetCopies = 0;   // render-target copies recorded (core_copies.cpp)
	unsigned copiesSkipped = 0;  // ... not done: stretched or moved regions, refusals
};
// Texture path counters since startup.
struct TexturePathCounters
{
	unsigned images = 0;         // TexImage2D calls
	unsigned rejected = 0;       // ... returned early (no texture, face, target, data)
	unsigned convertFailed = 0;  // ... whose source format did not convert
	unsigned uploads = 0;        // GPU textures created
	unsigned uploadFailed = 0;   // ... that failed (size or linear memory)
	unsigned corrupted = 0;      // levels changed between arrival and upload
};
TexturePathCounters g_TextureCounters;
DrawPathCounters g_Counters;
#if defined( PLATFORM_3DS )
int g_UnmippedSizeCap = 256;
#else
int g_UnmippedSizeCap = 4096;
#endif

void Identity( float *m )
{
	memset( m, 0, 16 * sizeof( float ) );
	m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// out = a * b (row-vector convention: v * a * b).
void Mul( const float *a, const float *b, float *out )
{
	float r[16];
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
		{
			float sum = 0.0f;
			for ( int k = 0; k < 4; ++k )
				sum += a[i * 4 + k] * b[k * 4 + j];
			r[i * 4 + j] = sum;
		}
	memcpy( out, r, sizeof( r ) );
}

float *Top( int stack = -1 )
{
	if ( stack < 0 )
		stack = g_CurrentStack;
	return g_Stacks[stack][g_StackTop[stack]].m;
}

void InitStacks()
{
	for ( int i = 0; i < kStackCount; ++i )
	{
		g_StackTop[i] = 0;
		Identity( g_Stacks[i][0].m );
	}
}

// top = m * top (D3DX MultMatrixLocal).
void MultLocal( const float *m )
{
	Mul( m, Top(), Top() );
}

PicaTexture *TextureFor( ShaderAPITextureHandle_t handle )
{
	const int index = int( handle ) - 1;
	if ( index < 0 || index >= g_Textures.Count() || !g_Textures[index] || !g_Textures[index]->used )
		return NULL;
	return g_Textures[index];
}

// The render targets the 3DS draws into (RFC 0026): the portal-plane targets
// of the client's texture portals (`_rt_PortalPlane*`, which the client makes
// only when the device has no stencil), RGBA8 with power-of-two sides. Other
// targets keep being skipped until their own cohort is brought over.
bool DrawableTarget( PicaTexture &texture )
{
#if defined( PLATFORM_3DS )
	if ( !texture.renderTarget || texture.depth || V_strnicmp( texture.name, "_rt_PortalPlane", 15 ) != 0 )
		return false;
#else
	// The full model draws into and samples every colour target.
	if ( !texture.renderTarget || texture.depth )
		return false;
#endif
	if ( !texture.gpu.IsTarget() && !texture.gpu.CreateTarget( texture.width, texture.height ) )
		return false;
	return true;
}

pica::Compare DepthCompare( ShaderDepthFunc_t func )
{
	switch ( func )
	{
	case SHADER_DEPTHFUNC_NEVER: return pica::Compare::kNever;
	case SHADER_DEPTHFUNC_NEARER: return pica::Compare::kLess;
	case SHADER_DEPTHFUNC_EQUAL: return pica::Compare::kEqual;
	case SHADER_DEPTHFUNC_NEAREROREQUAL: return pica::Compare::kLessEqual;
	case SHADER_DEPTHFUNC_FARTHER: return pica::Compare::kGreater;
	case SHADER_DEPTHFUNC_NOTEQUAL: return pica::Compare::kNotEqual;
	case SHADER_DEPTHFUNC_FARTHEROREQUAL: return pica::Compare::kGreaterEqual;
	default: return pica::Compare::kAlways;
	}
}

pica::Compare AlphaCompare( ShaderAlphaFunc_t func )
{
	switch ( func )
	{
	case SHADER_ALPHAFUNC_NEVER: return pica::Compare::kNever;
	case SHADER_ALPHAFUNC_LESS: return pica::Compare::kLess;
	case SHADER_ALPHAFUNC_EQUAL: return pica::Compare::kEqual;
	case SHADER_ALPHAFUNC_LEQUAL: return pica::Compare::kLessEqual;
	case SHADER_ALPHAFUNC_GREATER: return pica::Compare::kGreater;
	case SHADER_ALPHAFUNC_NOTEQUAL: return pica::Compare::kNotEqual;
	case SHADER_ALPHAFUNC_GEQUAL: return pica::Compare::kGreaterEqual;
	default: return pica::Compare::kAlways;
	}
}

pica::Blend BlendFactor( ShaderBlendFactor_t factor )
{
	switch ( factor )
	{
	case SHADER_BLEND_ZERO: return pica::Blend::kZero;
	case SHADER_BLEND_ONE: return pica::Blend::kOne;
	case SHADER_BLEND_DST_COLOR: return pica::Blend::kDstColor;
	case SHADER_BLEND_ONE_MINUS_DST_COLOR: return pica::Blend::kOneMinusDstColor;
	case SHADER_BLEND_SRC_ALPHA: return pica::Blend::kSrcAlpha;
	case SHADER_BLEND_ONE_MINUS_SRC_ALPHA: return pica::Blend::kOneMinusSrcAlpha;
	case SHADER_BLEND_DST_ALPHA: return pica::Blend::kDstAlpha;
	case SHADER_BLEND_ONE_MINUS_DST_ALPHA: return pica::Blend::kOneMinusDstAlpha;
	case SHADER_BLEND_SRC_ALPHA_SATURATE: return pica::Blend::kSrcAlphaSaturate;
	case SHADER_BLEND_SRC_COLOR: return pica::Blend::kSrcColor;
	case SHADER_BLEND_ONE_MINUS_SRC_COLOR: return pica::Blend::kOneMinusSrcColor;
	default: return pica::Blend::kOne;
	}
}

unsigned int LevelHash( const CUtlVector<unsigned char> &level )
{
	unsigned int hash = 2166136261u;
	for ( int i = 0; i < level.Count(); ++i )
		hash = ( hash ^ level[i] ) * 16777619u;
	return hash;
}

// Encodes a texture's chosen levels and uploads them.
void UploadTexture( PicaTexture &texture )
{
	texture.dirty = false;
#if defined( PLATFORM_3DS )
	texture.gpu.Release();
#else
	// Upload refills an image of the same shape in place (the core keeps its id).
	if ( texture.cube )
		texture.gpu.Release();
#endif
	if ( texture.cube )
	{
		const std::uint8_t *faces[6];
		for ( int i = 0; i < 6; ++i )
		{
			if ( texture.cubeFaces[i].Count() != texture.cubeSize * texture.cubeSize * 4 )
				return; // a face has not arrived
			faces[i] = texture.cubeFaces[i].Base();
		}
		if ( texture.gpu.UploadCube( texture.cubeSize, faces ) )
			++g_TextureCounters.uploads;
		else
			++g_TextureCounters.uploadFailed;
		return;
	}
	if ( texture.levels.Count() == 0 || texture.baseWidth < 8 || texture.baseHeight < 8 )
		return;
	for ( int i = 0; i < texture.levels.Count() && i < texture.levelHashes.Count(); ++i )
	{
		if ( LevelHash( texture.levels[i] ) != texture.levelHashes[i] )
		{
			++g_TextureCounters.corrupted;
			printf( "pica: MEMORY CORRUPTION texture %s level %d (%dx%d) changed after it arrived\n",
				texture.name, i, texture.baseWidth >> i, texture.baseHeight >> i );
		}
	}
	const bool mipped = texture.mipLevels > 1;
	bool alpha = false;
	for ( int i = 0; i < texture.levels.Count() && !alpha; ++i )
	{
		const int w = texture.baseWidth >> i, h = texture.baseHeight >> i;
		alpha = pica::HasAlpha( texture.levels[i].Base(), w, h );
	}
	// Block-compressed when mipmapped (world and model textures); RGBA4 for
	// textures the material system updates in place (fonts, UI: half the
	// linear memory of RGBA8, clause D42); RGBA8 for lightmap pages, whose
	// 2x overbright would show 4-bit steps. -pica_texture_rgba8 uploads
	// everything as RGBA8 (isolates the ETC1 encoder and the RGBA4 packing).
#if defined( PLATFORM_3DS )
	static const bool s_ForceRGBA8 = CommandLine()->FindParm( "-pica_texture_rgba8" ) != 0;
#else
	// Other devices take the decoded levels as they are (RGBA8).
	static const bool s_ForceRGBA8 = true;
#endif
	const pica::UploadFormat format =
	    ( s_ForceRGBA8 || ( !mipped && texture.lightmap ) ) ? pica::UploadFormat::kRGBA8
	    : !mipped                                          ? pica::UploadFormat::kRGBA4
	    : alpha ? pica::UploadFormat::kETC1A4
	            : pica::UploadFormat::kETC1;
	CUtlVector<std::vector<std::uint8_t>> encoded;
	const std::uint8_t *levels[16];
	int count = 0;
	for ( int i = 0; i < texture.levels.Count() && count < 16; ++i )
	{
		const int w = texture.baseWidth >> i, h = texture.baseHeight >> i;
		if ( w < 8 || h < 8 )
			break;
		if ( format == pica::UploadFormat::kRGBA8 )
		{
			levels[count++] = texture.levels[i].Base();
			continue;
		}
		std::vector<std::uint8_t> &out = encoded[encoded.AddToTail()];
		if ( format == pica::UploadFormat::kRGBA4 )
		{
			pica::PackRgba4Level( texture.levels[i].Base(), w, h, out );
			levels[count++] = out.data();
			continue;
		}
		if ( !pica::EncodeEtc1Level( format == pica::UploadFormat::kETC1A4, texture.levels[i].Base(), w, h, out ) )
			break;
		levels[count++] = out.data();
	}
	if ( count > 0 && texture.gpu.Upload( format, texture.baseWidth, texture.baseHeight, count, levels ) )
	{
		++g_TextureCounters.uploads;
		texture.gpu.SetWrap( texture.wrapS, texture.wrapT );
	}
	else
		++g_TextureCounters.uploadFailed;
#if !defined( PLATFORM_3DS )
	if ( texture.wantsSrgb && count > 0 &&
	     texture.gpuSrgb.Upload( pica::UploadFormat::kRGBA8Srgb, texture.baseWidth,
	         texture.baseHeight, count, levels ) )
		texture.gpuSrgb.SetWrap( texture.wrapS, texture.wrapT );
	// Off the 3DS the CPU copy stays: the sRGB twin may be asked for later.
	if ( false )
#else
	// Mipmapped levels are not updated in place: drop the CPU copy.
	if ( mipped )
#endif
	{
		texture.levels.Purge();
		texture.levelHashes.Purge();
	}
}

// The size the GPU keeps for a source level of (w, h): powers of two at most
// the cap, at least 8.
void GpuSize( int w, int h, int cap, int &outW, int &outH )
{
	outW = pica::FloorPow2( w < cap ? w : cap );
	outH = pica::FloorPow2( h < cap ? h : cap );
	if ( outW < 8 ) outW = 8;
	if ( outH < 8 ) outH = 8;
}

} // namespace

//-----------------------------------------------------------------------------
// The empty shader shadow
//-----------------------------------------------------------------------------
class CShaderShadowEmpty : public IShaderShadow
{
public:
	CShaderShadowEmpty();
	virtual ~CShaderShadowEmpty();

	// Sets the default *shadow* state
	void SetDefaultState();

	// Methods related to depth buffering
	void DepthFunc( ShaderDepthFunc_t depthFunc );
	void EnableDepthWrites( bool bEnable );
	void EnableDepthTest( bool bEnable );
	void EnablePolyOffset( PolygonOffsetMode_t nOffsetMode );

	// Suppresses/activates color writing 
	void EnableColorWrites( bool bEnable );
	void EnableAlphaWrites( bool bEnable );

	// Methods related to alpha blending
	void EnableBlending( bool bEnable );
	void BlendFunc( ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor );

	// Alpha testing
	void EnableAlphaTest( bool bEnable );
	void AlphaFunc( ShaderAlphaFunc_t alphaFunc, float alphaRef /* [0-1] */ );

	// Wireframe/filled polygons
	void PolyMode( ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode );

	// Back face culling
	void EnableCulling( bool bEnable );
	
	// constant color + transparency
	void EnableConstantColor( bool bEnable );

	// Indicates the vertex format for use with a vertex shader
	// The flags to pass in here come from the VertexFormatFlags_t enum
	// If pTexCoordDimensions is *not* specified, we assume all coordinates
	// are 2-dimensional
	void VertexShaderVertexFormat( unsigned int nFlags, 
		int nTexCoordCount, int* pTexCoordDimensions, int nUserDataSize );
	
	// Indicates we're going to light the model
	void EnableLighting( bool bEnable );
	void EnableSpecular( bool bEnable );

	// vertex blending
	void EnableVertexBlend( bool bEnable );

	// per texture unit stuff
	void OverbrightValue( TextureStage_t stage, float value );
	void EnableTexture( Sampler_t stage, bool bEnable );
	void EnableTexGen( TextureStage_t stage, bool bEnable );
	void TexGen( TextureStage_t stage, ShaderTexGenParam_t param );

	// alternate method of specifying per-texture unit stuff, more flexible and more complicated
	// Can be used to specify different operation per channel (alpha/color)...
	void EnableCustomPixelPipe( bool bEnable );
	void CustomTextureStages( int stageCount );
	void CustomTextureOperation( TextureStage_t stage, ShaderTexChannel_t channel, 
		ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2 );

	// indicates what per-vertex data we're providing
	void DrawFlags( unsigned int drawFlags );

	// A simpler method of dealing with alpha modulation
	void EnableAlphaPipe( bool bEnable );
	void EnableConstantAlpha( bool bEnable );
	void EnableVertexAlpha( bool bEnable );
	void EnableTextureAlpha( TextureStage_t stage, bool bEnable );

	// GR - Separate alpha blending
	void EnableBlendingSeparateAlpha( bool bEnable );
	void BlendFuncSeparateAlpha( ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor );

	// Sets the vertex and pixel shaders
	void SetVertexShader( const char *pFileName, int vshIndex );
	void SetPixelShader( const char *pFileName, int pshIndex );

	// Convert from linear to gamma color space on writes to frame buffer.
	void EnableSRGBWrite( bool bEnable )
	{
	}

	void EnableSRGBRead( Sampler_t stage, bool bEnable )
	{
	}

	virtual void FogMode( ShaderFogMode_t fogMode )
	{
	}

	virtual void DisableFogGammaCorrection( bool bDisable )
	{
	}

	virtual void SetDiffuseMaterialSource( ShaderMaterialSource_t materialSource )
	{
	}

	virtual void SetMorphFormat( MorphFormat_t flags )
	{
	}

	virtual void EnableStencil( bool bEnable )
	{
	}
	virtual void StencilFunc( ShaderStencilFunc_t stencilFunc )
	{
	}
	virtual void StencilPassOp( ShaderStencilOp_t stencilOp )
	{
	}
	virtual void StencilFailOp( ShaderStencilOp_t stencilOp )
	{
	}
	virtual void StencilDepthFailOp( ShaderStencilOp_t stencilOp )
	{
	}
	virtual void StencilReference( int nReference )
	{
	}
	virtual void StencilMask( int nMask )
	{
	}
	virtual void StencilWriteMask( int nMask )
	{
	}

	virtual void ExecuteCommandBuffer( uint8 *pBuf ) 
	{
	}
	// Alpha to coverage
	void EnableAlphaToCoverage( bool bEnable );
	
	virtual void SetShadowDepthFiltering( Sampler_t stage )
	{
	}

	virtual void BlendOp( ShaderBlendOp_t blendOp ) {}
	virtual void BlendOpSeparateAlpha( ShaderBlendOp_t blendOp ) {}

	bool m_IsTranslucent;
	bool m_IsAlphaTested;
	bool m_bIsDepthWriteEnabled;
	bool m_bUsesVertexAndPixelShaders;
	// The PICA state and vertex format the next snapshot records.
	pica::DrawState m_State;
	int m_PolyOffset = SHADER_POLYOFFSET_DISABLE; // EnablePolyOffset's mode
	VertexFormat_t m_VertexFormat;
};


//-----------------------------------------------------------------------------
// The DX8 implementation of the shader device
//-----------------------------------------------------------------------------
class CShaderDeviceEmpty : public render::legacy::ILegacyStreamDevice
{
public:
	CShaderDeviceEmpty() : m_DynamicMesh( true ), m_Mesh( false ) {}

	// The legacy draw stream (render::legacy::ILegacyStreamDevice); the
	// material system's IShaderDevice facade answers everything else (RFC
	// 0016 legacy device facade F4).
	void GetBackBufferDimensions( int &width, int &height ) const;
	virtual void Present( )
	{
		if ( !pica::Initialized() )
			return;
		if ( !pica::InFrame() )
			pica::BeginFrame();
		const double presentStart = Plat_FloatTime();
		pica::EndFrame();
		const double presentEnd = Plat_FloatTime();
		{
			// A slow frame, named with its system ticks: the window to read in
			// a guest-time profile (guest_profile.py --window START END).
			static unsigned long long s_frameTick = 0;
			const unsigned long long tick = svcGetSystemTick();
			if ( s_frameTick && tick - s_frameTick > 268111856ull * 4 / 10 )
				printf( "pica: slow frame %d %.0f ms ticks %08x %08x\n", g_PicaFrame + 1,
					( tick - s_frameTick ) * 1000.0 / 268111856.0, unsigned( s_frameTick ),
					unsigned( tick ) );
			s_frameTick = tick;
		}
		g_bDrawingToBackBuffer = true;
		{
			// Frame time on the guest clock (Plat_FloatTime: the system tick): the
			// performance line every 30 frames, the whole frame and the share
			// spent in EndFrame (submit, GPU wait, present).
			static double s_last = 0, s_sum = 0, s_max = 0, s_present = 0;
			static int s_count = 0;
			if ( s_last )
			{
				const double frame = presentEnd - s_last;
				s_sum += frame;
				s_max = frame > s_max ? frame : s_max;
				s_present += presentEnd - presentStart;
				if ( ++s_count == 30 )
				{
					const double ms = 1000.0;
					printf( "pica: perf frame %d avg %.1f ms (%.1f fps) max %.1f ms end-frame %.1f ms\n",
						g_PicaFrame + 1, s_sum * ms / 30, 30.0 / s_sum,
						s_max * ms, s_present * ms / 30 );
					s_sum = s_max = s_present = 0;
					s_count = 0;
				}
			}
			s_last = presentEnd;
		}
		// A capture the boot harness asks for: -pica_capture <frame> <path>.
		static int s_captureFrame = CommandLine()->ParmValue( "-pica_capture", -1 );
		const int s_frame = ++g_PicaFrame;
		if ( s_frame == s_captureFrame + 1 )
		{
			const char *path = CommandLine()->ParmValue( "-pica_capture_path", "sdmc:/source_pica.ppm" );
			Msg( "pica: capture %s %s\n", path, pica::CaptureTopScreen( path ) ? "ok" : "failed" );
		}
		// Straight to stdout (console.log on the 3DS), not through the engine's
		// spew, which stops reaching stdout once the console exists.
		const pica::Stats &stats = pica::FrameStats();
		if ( s_frame % 120 == 1 )
		{
			// The heap over time (out-of-memory diagnosis): arena = sbrk'd heap,
			// used and free inside it.
			const struct mallinfo heap = mallinfo();
			printf( "pica: frame %d heap arena %u of %u KB used %u KB free %u KB linear free %u KB\n", s_frame,
				(unsigned)( heap.arena / 1024 ), (unsigned)( __ctru_heap_size / 1024 ),
				(unsigned)( heap.uordblks / 1024 ), (unsigned)( heap.fordblks / 1024 ),
				(unsigned)( linearSpaceFree() / 1024 ) );
		}
		if ( s_frame % 120 == 1 )
			printf( "pica: frame %d textures %u KB meshes %u KB submits %u | "
				"mesh draws %u primlist %u empty %u material %u passes %u target skips %u clears %u overrun draws %u wide texcoords %u core meshes %u refused %u error material %u\n", s_frame,
				(unsigned)( stats.textureBytes / 1024 ), (unsigned)( stats.meshBytes / 1024 ),
				(unsigned)stats.submits, g_Counters.meshDraws, g_Counters.primListDraws,
				g_Counters.primListEmpty, g_Counters.materialDraws,
				g_Counters.renderPasses, g_Counters.targetSkips, g_Counters.clears, g_Counters.overrunDraws, g_Counters.wideTexCoordLocks, g_Counters.coreMeshDraws,
				g_Counters.refusedDraws, g_Counters.errorMaterialDraws );
		if ( s_frame % 120 == 1 )
			printf( "pica: target copies %u skipped %u\n", g_Counters.targetCopies,
				g_Counters.copiesSkipped );
		if ( s_frame % 120 == 1 )
			printf( "pica: textures: images %u rejected %u convert failed %u uploads %u failed %u "
				"corrupted %u\n",
				g_TextureCounters.images, g_TextureCounters.rejected, g_TextureCounters.convertFailed,
				g_TextureCounters.uploads, g_TextureCounters.uploadFailed, g_TextureCounters.corrupted );
		if ( s_frame % 120 == 1 && stats.targetSwitches )
			printf( "pica: frame %d target switches %u\n", s_frame, (unsigned)stats.targetSwitches );
		g_Counters = DrawPathCounters();
	}
	virtual bool AddView( void* hwnd );
	virtual void RemoveView( void* hwnd );
	virtual void SetView( void* hwnd );
	virtual void ReleaseResources();
	virtual void ReacquireResources();
	virtual IMesh* CreateStaticMesh( VertexFormat_t fmt, const char *pTextureBudgetGroup, IMaterial * pMaterial = NULL );
	virtual void DestroyStaticMesh( IMesh* mesh );
	virtual IShaderBuffer* CompileShader( const char *pProgram, size_t nBufLen, const char *pShaderVersion ) { return NULL; }
	virtual VertexShaderHandle_t CreateVertexShader( IShaderBuffer* pShaderBuffer ) { return VERTEX_SHADER_HANDLE_INVALID; }
	virtual void DestroyVertexShader( VertexShaderHandle_t hShader ) {}
	virtual GeometryShaderHandle_t CreateGeometryShader( IShaderBuffer* pShaderBuffer ) { return GEOMETRY_SHADER_HANDLE_INVALID; }
	virtual void DestroyGeometryShader( GeometryShaderHandle_t hShader ) {}
	virtual PixelShaderHandle_t CreatePixelShader( IShaderBuffer* pShaderBuffer ) { return PIXEL_SHADER_HANDLE_INVALID; }
	virtual void DestroyPixelShader( PixelShaderHandle_t hShader ) {}
	virtual IVertexBuffer *CreateVertexBuffer( ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pBudgetGroup );
	virtual void DestroyVertexBuffer( IVertexBuffer *pVertexBuffer );
	virtual IIndexBuffer *CreateIndexBuffer( ShaderBufferType_t bufferType, MaterialIndexFormat_t fmt, int nIndexCount, const char *pBudgetGroup );
	virtual void DestroyIndexBuffer( IIndexBuffer *pIndexBuffer );
	virtual IVertexBuffer *GetDynamicVertexBuffer( int streamID, VertexFormat_t vertexFormat, bool bBuffered );
	virtual IIndexBuffer *GetDynamicIndexBuffer( MaterialIndexFormat_t fmt, bool bBuffered );
	virtual void EnableNonInteractiveMode( MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo ) {}
	virtual void RefreshFrontBufferNonInteractive( ) {}
	virtual void HandleThreadEvent( uint32 threadEvent ) {}

private:
	CEmptyMesh m_Mesh;
	CEmptyMesh m_DynamicMesh;
};

static CShaderDeviceEmpty s_ShaderDeviceEmpty;

//-----------------------------------------------------------------------------
// The DX8 implementation of the shader device
//-----------------------------------------------------------------------------
// The DX8 implementation of the shader API
//-----------------------------------------------------------------------------
class CShaderAPIEmpty : public IShaderAPI, public IHardwareConfigInternal, public IDebugTextureInfo
{
public:
	// constructor, destructor
	CShaderAPIEmpty( );
	virtual ~CShaderAPIEmpty();

	// IDebugTextureInfo implementation.
public:

	virtual bool IsDebugTextureListFresh( int numFramesAllowed = 1 ) { return false; }
	virtual bool SetDebugTextureRendering( bool bEnable ) { return false; }
	virtual void EnableDebugTextureList( bool bEnable ) {}
	virtual void EnableGetAllTextures( bool bEnable ) {}
	virtual KeyValues* GetDebugTextureList() { return NULL; }
	virtual int GetTextureMemoryUsed( TextureMemoryType eTextureMemory ) { return 0; }

	// Methods of IShaderDynamicAPI
	virtual void GetBackBufferDimensions( int& width, int& height ) const
	{
		s_ShaderDeviceEmpty.GetBackBufferDimensions( width, height );
	}
	virtual void GetCurrentColorCorrection( ShaderColorCorrectionInfo_t* pInfo )
	{
		pInfo->m_bIsEnabled = false;
		pInfo->m_nLookupCount = 0;
		pInfo->m_flDefaultWeight = 0.0f;
	}


	// Methods of IShaderAPI
public:
	virtual void SetViewports( int nCount, const ShaderViewport_t* pViewports );
	virtual int GetViewports( ShaderViewport_t* pViewports, int nMax ) const;
	virtual void ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil, int renderTargetWidth, int renderTargetHeight );
	virtual void ClearColor3ub( unsigned char r, unsigned char g, unsigned char b );
	virtual void ClearColor4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a );
	virtual void BindVertexShader( VertexShaderHandle_t hVertexShader ) {}
	virtual void BindGeometryShader( GeometryShaderHandle_t hGeometryShader ) {}
	virtual void BindPixelShader( PixelShaderHandle_t hPixelShader ) {}
	virtual void SetRasterState( const ShaderRasterState_t& state ) {}
	virtual void MarkUnusedVertexFields( unsigned int nFlags, int nTexCoordCount, bool *pUnusedTexCoords ) {}
	virtual bool OwnGPUResources( bool bEnable ) { return false; }

	virtual bool DoRenderTargetsNeedSeparateDepthBuffer() const;

	// Used to clear the transition table when we know it's become invalid.
	void ClearSnapshots();

	// Sets the mode...
	// The material system's mode set: bring up the PICA renderer (the device
	// manager's SetMode, which the D3D9 composition uses, does the same).
	bool SetMode( void* hwnd, int nAdapter, const ShaderDeviceInfo_t &info )
	{
		if ( pica::Initialized() )
			return true;
		g_TextureSizeCap = CommandLine()->ParmValue( "-pica_texture_size", g_TextureSizeCap );
		if ( !pica::Init() )
		{
			Warning( "pica: GPU initialization failed\n" );
			return false;
		}
		if ( const int reserveKB = CommandLine()->ParmValue( "-pica_linear_reserve", 0 ) )
			printf( "pica: linear reserve %d KB %s\n", reserveKB,
				pica::ReserveLinear( std::size_t( reserveKB ) * 1024 ) ? "held" : "refused" );
		InitStacks();
		return true;
	}

	void ChangeVideoMode( const ShaderDeviceInfo_t &info )
	{
	}

	// Called when the dx support level has changed
	virtual void DXSupportLevelChanged() {}

	virtual void EnableUserClipTransformOverride( bool bEnable ) {}
	virtual void UserClipTransform( const VMatrix &worldToView ) {}

	// Sets the default *dynamic* state
	void SetDefaultState( );

	// Returns the snapshot id for the shader state
	StateSnapshot_t	 TakeSnapshot( );

	// Returns true if the state snapshot is transparent
	bool IsTranslucent( StateSnapshot_t id ) const;
	bool IsAlphaTested( StateSnapshot_t id ) const;
	bool UsesVertexAndPixelShaders( StateSnapshot_t id ) const;
	virtual bool IsDepthWriteEnabled( StateSnapshot_t id ) const;

	// Gets the vertex format for a set of snapshot ids
	VertexFormat_t ComputeVertexFormat( int numSnapshots, StateSnapshot_t* pIds ) const;

	// Gets the vertex format for a set of snapshot ids
	VertexFormat_t ComputeVertexUsage( int numSnapshots, StateSnapshot_t* pIds ) const;

	// Begins a rendering pass that uses a state snapshot
	void BeginPass( StateSnapshot_t snapshot  );

	// Uses a state snapshot
	void UseSnapshot( StateSnapshot_t snapshot );

	// Use this to get the mesh builder that allows us to modify vertex data
	CMeshBuilder* GetVertexModifyBuilder();

	// Sets the color to modulate by
	void Color3f( float r, float g, float b );
	void Color3fv( float const* pColor );
	void Color4f( float r, float g, float b, float a );
	void Color4fv( float const* pColor );

	// Faster versions of color
	void Color3ub( unsigned char r, unsigned char g, unsigned char b );
	void Color3ubv( unsigned char const* rgb );
	void Color4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a );
	void Color4ubv( unsigned char const* rgba );

	// Sets the lights
	void SetLight( int lightNum, const LightDesc_t& desc );
	void SetLightingOrigin( Vector vLightingOrigin );
	void SetAmbientLight( float r, float g, float b );
	void SetAmbientLightCube( Vector4D cube[6] );

	// Get the lights
	int GetMaxLights( void ) const;
	const LightDesc_t& GetLight( int lightNum ) const;

	// Render state for the ambient light cube (vertex shaders)
	void SetVertexShaderStateAmbientLightCube();
	void SetPixelShaderStateAmbientLightCube( int pshReg, bool bForceToBlack = false )
	{
	}

	float GetAmbientLightCubeLuminance(void)
	{
		return 0.0f;
	}

	void SetSkinningMatrices();

	// Lightmap texture binding
	void BindLightmap( TextureStage_t stage );
	void BindLightmapAlpha( TextureStage_t stage )
	{
	}
	void BindBumpLightmap( TextureStage_t stage );
	void BindFullbrightLightmap( TextureStage_t stage );
	void BindWhite( TextureStage_t stage );
	void BindBlack( TextureStage_t stage );
	void BindGrey( TextureStage_t stage );
	void BindFBTexture( TextureStage_t stage, int textureIdex );
	void CopyRenderTargetToTexture( ShaderAPITextureHandle_t texID )
	{
		CopyRenderTargetToTextureEx( texID, 0, nullptr, nullptr );
	}

	// R91: the current target's region into a render target, in frame order
	// (core_copies.cpp). Copies keep their texels' place (D37): a source and
	// destination rectangle that differ (a stretch or a move) are counted and
	// skipped until a blit does them.
	void CopyRenderTargetToTextureEx( ShaderAPITextureHandle_t texID, int nRenderTargetID, Rect_t *pSrcRect, Rect_t *pDstRect )
	{
		PicaTexture *texture = TextureFor( texID );
		if ( nRenderTargetID != 0 || !texture || !pica::Initialized() )
		{
			++g_Counters.copiesSkipped;
			return;
		}
		const Rect_t *rect = pSrcRect ? pSrcRect : pDstRect;
		if ( pSrcRect && pDstRect &&
			 ( pSrcRect->x != pDstRect->x || pSrcRect->y != pDstRect->y ||
			   pSrcRect->width != pDstRect->width || pSrcRect->height != pDstRect->height ) )
		{
			++g_Counters.copiesSkipped;
			return;
		}
		pica::CopyRect region;
		region.width = rect ? rect->width : texture->width;
		region.height = rect ? rect->height : texture->height;
		region.x = rect ? rect->x : 0;
		region.y = rect ? rect->y : 0;
		// D3D9 PC keeps the opaque scene's depth in destination alpha, so a
		// frame copy carries it (soft particles read _rt_FullFrameDepth); an
		// orthographic capture (the UI) keeps its own alpha.
		const float *projection = Top( kStackProjection );
		pica::CopyDepthAlpha depthAlpha;
		depthAlpha.projection[0] = projection[2 * 4 + 2];
		depthAlpha.projection[1] = projection[3 * 4 + 2];
		depthAlpha.projection[2] = projection[2 * 4 + 3];
		depthAlpha.projection[3] = projection[3 * 4 + 3];
		depthAlpha.range = kCoreDestAlphaDepthRange;
		const bool perspective = projection[2 * 4 + 3] != 0.0f;
		if ( pica::CopyTargetRegion( texture->gpu, region, perspective ? &depthAlpha : nullptr,
		         g_CorePassRecorder ) == pica::CopyResult::kCopied )
			++g_Counters.targetCopies;
		else
			++g_Counters.copiesSkipped;
	}

	void CopyTextureToRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t textureHandle, Rect_t *pSrcRect, Rect_t *pDstRect )
	{
	}

	// Special system flat normal map binding.
	void BindFlatNormalMap( TextureStage_t stage );
	void BindNormalizationCubeMap( TextureStage_t stage );
	void BindSignedNormalizationCubeMap( TextureStage_t stage );

	// Set the number of bone weights
	void SetNumBoneWeights( int numBones );
	void EnableHWMorphing( bool bEnable );

	// Flushes any primitives that are buffered
	void FlushBufferedPrimitives();

	// Gets the dynamic mesh; note that you've got to render the mesh
	// before calling this function a second time. Clients should *not*
	// call DestroyStaticMesh on the mesh returned by this call.
	IMesh* GetDynamicMesh( IMaterial* pMaterial, int nHWSkinBoneCount, bool buffered, IMesh* pVertexOverride, IMesh* pIndexOverride );
	IMesh* GetDynamicMeshEx( IMaterial* pMaterial, VertexFormat_t fmt, int nHWSkinBoneCount, bool buffered, IMesh* pVertexOverride, IMesh* pIndexOverride );

	IMesh* GetFlexMesh();

	// Renders a single pass of a material
	void RenderPass( int nPass, int nPassCount );

	// stuff related to matrix stacks
	void MatrixMode( MaterialMatrixMode_t matrixMode );
	void PushMatrix();
	void PopMatrix();
	void LoadMatrix( float *m );
	void LoadBoneMatrix( int boneIndex, const float *m )
	{
		if ( boneIndex < 0 || boneIndex >= kMaxBones )
			return;
		memcpy( g_Bones[boneIndex], m, 12 * sizeof( float ) );
		if ( boneIndex > g_MaxBone )
			g_MaxBone = boneIndex;
		// As D3D9: bone 0 is the model matrix (the 3x4 transposed).
		if ( boneIndex == 0 )
		{
			float model[16];
			for ( int i = 0; i < 3; ++i )
				for ( int j = 0; j < 4; ++j )
					model[j * 4 + i] = m[i * 4 + j];
			model[3] = model[7] = model[11] = 0.0f;
			model[15] = 1.0f;
			memcpy( Top( kStackModel ), model, sizeof( model ) );
		}
	}
	void MultMatrix( float *m );
	void MultMatrixLocal( float *m );
	void GetMatrix( MaterialMatrixMode_t matrixMode, float *dst );
	void LoadIdentity( void );
	void LoadCameraToWorld( void );
	void Ortho( double left, double top, double right, double bottom, double zNear, double zFar );
	void PerspectiveX( double fovx, double aspect, double zNear, double zFar );
	void PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar, double bottom, double top, double left, double right );
	void PickMatrix( int x, int y, int width, int height );
	void Rotate( float angle, float x, float y, float z );
	void Translate( float x, float y, float z );
	void Scale( float x, float y, float z );
	void ScaleXY( float x, float y );

	// Fog methods...
	void FogMode( MaterialFogMode_t fogMode );
	void FogStart( float fStart );
	void FogEnd( float fEnd );
	void SetFogZ( float fogZ );
	void FogMaxDensity( float flMaxDensity );
	void GetFogDistances( float *fStart, float *fEnd, float *fFogZ );
	void FogColor3f( float r, float g, float b );
	void FogColor3fv( float const* rgb );
	void FogColor3ub( unsigned char r, unsigned char g, unsigned char b );
	void FogColor3ubv( unsigned char const* rgb );

	virtual void SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b );
	virtual void SceneFogMode( MaterialFogMode_t fogMode );
	virtual void GetSceneFogColor( unsigned char *rgb );
	virtual MaterialFogMode_t GetSceneFogMode( );
	virtual int GetPixelFogCombo( );

	void SetHeightClipZ( float z ); 
	void SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode ); 

	void SetClipPlane( int index, const float *pPlane );
	void EnableClipPlane( int index, bool bEnable );

	void SetFastClipPlane( const float *pPlane );
	void EnableFastClip( bool bEnable );
	
	// We use smaller dynamic VBs during level transitions, to free up memory
	virtual int  GetCurrentDynamicVBSize( void );
	virtual void DestroyVertexBuffers( bool bExitingLevel = false );

	// Sets the vertex and pixel shaders
	void SetVertexShaderIndex( int vshIndex );
	void SetPixelShaderIndex( int pshIndex );

	// Sets the constant register for vertex and pixel shaders
	void SetVertexShaderConstant( int var, float const* pVec, int numConst = 1, bool bForce = false );
	void SetBooleanVertexShaderConstant( int var, BOOL const* pVec, int numConst = 1, bool bForce = false );
	void SetIntegerVertexShaderConstant( int var, int const* pVec, int numConst = 1, bool bForce = false );
	void SetPixelShaderConstant( int var, float const* pVec, int numConst = 1, bool bForce = false );
	void SetBooleanPixelShaderConstant( int var, BOOL const* pVec, int numBools = 1, bool bForce = false );
	void SetIntegerPixelShaderConstant( int var, int const* pVec, int numIntVecs = 1, bool bForce = false );

	void InvalidateDelayedShaderConstants( void );

	// Gamma<->Linear conversions according to the video hardware we're running on
	float GammaToLinear_HardwareSpecific( float fGamma ) const;
	float LinearToGamma_HardwareSpecific( float fLinear ) const;

	//Set's the linear->gamma conversion textures to use for this hardware for both srgb writes enabled and disabled(identity)
	void SetLinearToGammaConversionTextures( ShaderAPITextureHandle_t hSRGBWriteEnabledTexture, ShaderAPITextureHandle_t hIdentityTexture );

	// Cull mode
	void CullMode( MaterialCullMode_t cullMode );

	// Force writes only when z matches. . . useful for stenciling things out
	// by rendering the desired Z values ahead of time.
	void ForceDepthFuncEquals( bool bEnable );

	// Forces Z buffering on or off
	void OverrideDepthEnable( bool bEnable, bool bDepthEnable );
	void OverrideAlphaWriteEnable( bool bOverrideEnable, bool bAlphaWriteEnable );
	void OverrideColorWriteEnable( bool bOverrideEnable, bool bColorWriteEnable );

	// Sets the shade mode
	void ShadeMode( ShaderShadeMode_t mode );

	// Binds a particular material to render with
	void Bind( IMaterial* pMaterial );

	// Returns the nearest supported format
	ImageFormat GetNearestSupportedFormat( ImageFormat fmt, bool bFilteringRequired = true ) const;
 	ImageFormat GetNearestRenderTargetFormat( ImageFormat fmt ) const;

	// Sets the texture state
	void BindTexture( Sampler_t stage, ShaderAPITextureHandle_t textureHandle );

	void SetRenderTarget( ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle )
	{
		SetRenderTargetEx( 0, colorTextureHandle, depthTextureHandle );
	}

	void SetRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle )
	{
		// The back buffer and the render targets DrawableTarget admits are
		// drawn (the frame's target follows); views into any other target
		// (monitors, water, post) are skipped. The depth handle is the
		// renderer's own per target size.
		if ( nRenderTargetID != 0 )
			return;
		if ( colorTextureHandle == SHADER_RENDERTARGET_BACKBUFFER )
		{
			g_bDrawingToBackBuffer = true;
			pica::SetTarget( nullptr );
			return;
		}
		PicaTexture *texture = TextureFor( colorTextureHandle );
		g_bDrawingToBackBuffer = texture && pica::Initialized() && DrawableTarget( *texture );
		if ( g_bDrawingToBackBuffer )
			pica::SetTarget( &texture->gpu );
	}

	// Indicates we're going to be modifying this texture
	// TexImage2D, TexSubImage2D, TexWrap, TexMinFilter, and TexMagFilter
	// all use the texture specified by this function.
	void ModifyTexture( ShaderAPITextureHandle_t textureHandle );

	// Texture management methods
	void TexImage2D( int level, int cubeFace, ImageFormat dstFormat, int zOffset, int width, int height, 
							 ImageFormat srcFormat, bool bSrcIsTiled, void *imageData );
	void TexSubImage2D( int level, int cubeFace, int xOffset, int yOffset, int zOffset, int width, int height,
							 ImageFormat srcFormat, int srcStride, bool bSrcIsTiled, void *imageData );

	void TexImageFromVTF( IVTFTexture *pVTF, int iVTFFrame );

	bool TexLock( int level, int cubeFaceID, int xOffset, int yOffset, 
									int width, int height, CPixelWriter& writer );
	void TexUnlock( );
	
	// These are bound to the texture, not the texture environment
	void TexMinFilter( ShaderTexFilterMode_t texFilterMode );
	void TexMagFilter( ShaderTexFilterMode_t texFilterMode );
	void TexWrap( ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode );
	void TexSetPriority( int priority );	

	ShaderAPITextureHandle_t CreateTexture( 
		int width, 
		int height,
		int depth,
		ImageFormat dstImageFormat, 
		int numMipLevels, 
		int numCopies, 
		int flags, 
		const char *pDebugName,
		const char *pTextureGroupName );
	// Create a multi-frame texture (equivalent to calling "CreateTexture" multiple times, but more efficient)
	void CreateTextures( 
		ShaderAPITextureHandle_t *pHandles,
		int count,
		int width, 
		int height,
		int depth,
		ImageFormat dstImageFormat, 
		int numMipLevels, 
		int numCopies, 
		int flags, 
		const char *pDebugName,
		const char *pTextureGroupName );
	ShaderAPITextureHandle_t CreateDepthTexture( ImageFormat renderFormat, int width, int height, const char *pDebugName, bool bTexture );
	void DeleteTexture( ShaderAPITextureHandle_t textureHandle );
	bool IsTexture( ShaderAPITextureHandle_t textureHandle );
	bool IsTextureResident( ShaderAPITextureHandle_t textureHandle );

	// stuff that isn't to be used from within a shader
	void ClearBuffersObeyStencil( bool bClearColor, bool bClearDepth );
	void ClearBuffersObeyStencilEx( bool bClearColor, bool bClearAlpha, bool bClearDepth );
	void PerformFullScreenStencilOperation( void );
	void ReadPixels( int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat );
	virtual void ReadPixels( Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data, ImageFormat dstFormat, int nDstStride );

	// Selection mode methods
	int SelectionMode( bool selectionMode );
	void SelectionBuffer( unsigned int* pBuffer, int size );
	void ClearSelectionNames( );
	void LoadSelectionName( int name );
	void PushSelectionName( int name );
	void PopSelectionName();

	void FlushHardware();
	void ResetRenderState( bool bFullReset = true );

	void SetScissorRect( const int nLeft, const int nTop, const int nRight, const int nBottom, const bool bEnableScissor );

	// Can we download textures?
	virtual bool CanDownloadTextures() const;

	// Board-independent calls, here to unify how shaders set state
	// Implementations should chain back to IShaderUtil->BindTexture(), etc.

	// Use this to begin and end the frame
	void BeginFrame();
	void EndFrame();

	// returns current time
	double CurrentTime() const;

	// Get the current camera position in world space.
	void GetWorldSpaceCameraPosition( float * pPos ) const;

	// Members of IMaterialSystemHardwareConfig
	bool HasDestAlphaBuffer() const;
	bool HasStencilBuffer() const;
	virtual int  MaxViewports() const;
	virtual void OverrideStreamOffsetSupport( bool bOverrideEnabled, bool bEnableSupport ) {}
	virtual int  GetShadowFilterMode() const;
	int  StencilBufferBits() const;
	int	 GetFrameBufferColorDepth() const;
	int  GetSamplerCount() const;
	bool HasSetDeviceGammaRamp() const;
	bool SupportsCompressedTextures() const;
	VertexCompressionType_t SupportsCompressedVertices() const;
	bool SupportsVertexAndPixelShaders() const;
	bool SupportsPixelShaders_1_4() const;
	bool SupportsPixelShaders_2_0() const;
	bool SupportsPixelShaders_2_b() const;
	bool ActuallySupportsPixelShaders_2_b() const;
	bool SupportsStaticControlFlow() const;
	bool SupportsVertexShaders_2_0() const;
	bool SupportsShaderModel_3_0() const;
	int  MaximumAnisotropicLevel() const;
	int  MaxTextureWidth() const;
	int  MaxTextureHeight() const;
	int  MaxTextureAspectRatio() const;
	int  GetDXSupportLevel() const;
	const char *GetShaderDLLName() const
	{
		return "UNKNOWN";
	}
	int	 TextureMemorySize() const;
	bool SupportsOverbright() const;
	bool SupportsCubeMaps() const;
	bool SupportsMipmappedCubemaps() const;
	bool SupportsNonPow2Textures() const;
	int  GetTextureStageCount() const;
	int	 NumVertexShaderConstants() const;
	int	 NumBooleanVertexShaderConstants() const;
	int	 NumIntegerVertexShaderConstants() const;
	int	 NumPixelShaderConstants() const;
	int	 MaxNumLights() const;
	bool SupportsHardwareLighting() const;
	int	 MaxBlendMatrices() const;
	int	 MaxBlendMatrixIndices() const;
	int	 MaxVertexShaderBlendMatrices() const;
	int	 MaxUserClipPlanes() const;
	bool UseFastClipping() const
	{
		return false;
	}
	bool SpecifiesFogColorInLinearSpace() const;
	virtual bool SupportsSRGB() const;
	virtual bool FakeSRGBWrite() const;
	virtual bool CanDoSRGBReadFromRTs() const;
	virtual bool SupportsGLMixedSizeTargets() const;

	const char *GetHWSpecificShaderDLLName() const;
	bool NeedsAAClamp() const
	{
		return false;
	}
	bool SupportsSpheremapping() const;
	virtual int MaxHWMorphBatchCount() const { return 0; }

	// This is the max dx support level supported by the card
	virtual int	 GetMaxDXSupportLevel() const;

	bool ReadPixelsFromFrontBuffer() const;
	bool PreferDynamicTextures() const;
	virtual bool PreferReducedFillrate() const;
	bool HasProjectedBumpEnv() const;
	void ForceHardwareSync( void );
	
	int GetCurrentNumBones( void ) const;
	bool IsHWMorphingEnabled( void ) const;
	int GetCurrentLightCombo( void ) const;
	void GetDX9LightState( LightState_t *state ) const;
	MaterialFogMode_t GetCurrentFogType( void ) const;

	void RecordString( const char *pStr );

	void EvictManagedResources();

	void SetTextureTransformDimension( TextureStage_t textureStage, int dimension, bool projected );
	void DisableTextureTransform( TextureStage_t textureStage )
	{
	}
	void SetBumpEnvMatrix( TextureStage_t textureStage, float m00, float m01, float m10, float m11 );

	// Gets the lightmap dimensions
	virtual void GetLightmapDimensions( int *w, int *h );

	virtual void SyncToken( const char *pToken );

	// Setup standard vertex shader constants (that don't change)
	// This needs to be called anytime that overbright changes.
	virtual void SetStandardVertexShaderConstants( float fOverbright )
	{
	}
	
	// Level of anisotropic filtering
	virtual void SetAnisotropicLevel( int nAnisotropyLevel );

	bool SupportsHDR() const
	{
		return false;
	}
	HDRType_t GetHDRType() const
	{
		return HDR_TYPE_NONE;
	}
	HDRType_t GetHardwareHDRType() const
	{
		return HDR_TYPE_NONE;
	}
	virtual bool NeedsATICentroidHack() const
	{
		return false;
	}
	virtual bool SupportsColorOnSecondStream() const
	{
		return false;
	}
	virtual bool SupportsStaticPlusDynamicLighting() const
	{
		return false;
	}
	virtual bool SupportsStreamOffset() const
	{
		return false;
	}
	void SetDefaultDynamicState()
	{
	}
	virtual void CommitPixelShaderLighting( int pshReg )
	{
	}

	ShaderAPIOcclusionQuery_t CreateOcclusionQueryObject( void )
	{
		return INVALID_SHADERAPI_OCCLUSION_QUERY_HANDLE;
	}

	void DestroyOcclusionQueryObject( ShaderAPIOcclusionQuery_t handle )
	{
	}

	void BeginOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t handle )
	{
	}

	void EndOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t handle )
	{
	}

	int OcclusionQuery_GetNumPixelsRendered( ShaderAPIOcclusionQuery_t handle, bool bFlush )
	{
		return 0;
	}

	virtual void AcquireThreadOwnership() {}
	virtual void ReleaseThreadOwnership() {}

	virtual bool SupportsBorderColor() const { return false; }
	virtual bool SupportsFetch4() const { return false; }
	virtual bool CanStretchRectFromTextures( void ) const { return false; }
	virtual void EnableBuffer2FramesAhead( bool bEnable ) {}

	virtual void SetPSNearAndFarZ( int pshReg ) { }

	virtual void SetDepthFeatheringPixelShaderConstant( int iConstant, float fDepthBlendScale ) {}

	void SetPixelShaderFogParams( int reg )
	{
	}

	virtual bool InFlashlightMode() const
	{
		return false;
	}

	virtual bool InEditorMode() const
	{
		return false;
	}

	// What fields in the morph do we actually use?
	virtual MorphFormat_t ComputeMorphFormat( int numSnapshots, StateSnapshot_t* pIds ) const
	{
		return 0;
	}

	// Gets the bound morph's vertex format; returns 0 if no morph is bound
	virtual MorphFormat_t GetBoundMorphFormat()
	{
		return 0;
	}

	// Binds a standard texture
	virtual void BindStandardTexture( Sampler_t stage, StandardTextureId_t id )
	{
		// The material system binds its own texture (lightmap, white, ...).
		ShaderUtil()->BindStandardTexture( stage, id );
		// A lightmap page on sampler 1 is what a lightmapped surface's core
		// draw reads (CoreMeshDraw::lightmapPage); BindTexture clears it.
		if ( stage == SHADER_SAMPLER1 && ( id == TEXTURE_LIGHTMAP || id == TEXTURE_LIGHTMAP_BUMPED ||
			id == TEXTURE_LIGHTMAP_FULLBRIGHT || id == TEXTURE_LIGHTMAP_BUMPED_FULLBRIGHT ) )
			g_BoundLightmap = g_BoundTextures[1];
	}

	virtual void BindStandardVertexTexture( VertexTextureSampler_t stage, StandardTextureId_t id )
	{
	}

	virtual void GetStandardTextureDimensions( int *pWidth, int *pHeight, StandardTextureId_t id )
	{
		*pWidth = *pHeight = 0;
	}


	virtual void SetFlashlightState( const FlashlightState_t &state, const VMatrix &worldToTexture )
	{
	}

	virtual void SetFlashlightStateEx( const FlashlightState_t &state, const VMatrix &worldToTexture, ITexture *pFlashlightDepthTexture )
	{
	}

	virtual const FlashlightState_t &GetFlashlightState( VMatrix &worldToTexture ) const 
	{
		static FlashlightState_t  blah;
		return blah;
	}

	virtual const FlashlightState_t &GetFlashlightStateEx( VMatrix &worldToTexture, ITexture **pFlashlightDepthTexture ) const 
	{
		static FlashlightState_t  blah;
		return blah;
	}

	virtual void ClearVertexAndPixelShaderRefCounts()
	{
	}

	virtual void PurgeUnusedVertexAndPixelShaders()
	{
	}

	virtual bool IsAAEnabled() const
	{
		return false;
	}

	virtual int GetVertexTextureCount() const
	{
		return 0;
	}

	virtual int GetMaxVertexTextureDimension() const
	{
		return 0;
	}

	virtual int  MaxTextureDepth() const
	{
		return 0;
	}

	// Binds a vertex texture to a particular texture stage in the vertex pipe
	virtual void BindVertexTexture( VertexTextureSampler_t nSampler, ShaderAPITextureHandle_t hTexture )
	{
	}

	// Sets morph target factors
	virtual void SetFlexWeights( int nFirstWeight, int nCount, const MorphWeight_t* pWeights )
	{
	}

	// NOTE: Stuff after this is added after shipping HL2.
	ITexture *GetRenderTargetEx( int nRenderTargetID )
	{
		return NULL;
	}

	void SetToneMappingScaleLinear( const Vector &scale )
	{
	}

	const Vector &GetToneMappingScaleLinear( void ) const
	{
		static Vector dummy;
		return dummy;
	}

	virtual float GetLightMapScaleFactor( void ) const
	{
		return 1.0;
	}


	// For dealing with device lost in cases where SwapBuffers isn't called all the time (Hammer)
	virtual void HandleDeviceLost()
	{
	}

	virtual void EnableLinearColorSpaceFrameBuffer( bool bEnable )
	{
	}

	// Lets the shader know about the full-screen texture so it can 
	virtual void SetFullScreenTextureHandle( ShaderAPITextureHandle_t h )
	{
	}

	// Rendering parameters, stored (the render core's slots read the wind
	// and foliage time from them, RFC 0026 P3).
	void SetFloatRenderingParameter(int parm_number, float value)
	{
		if ( parm_number >= 0 && parm_number < kRenderParams )
			m_FloatParams[parm_number] = value;
	}

	void SetIntRenderingParameter(int parm_number, int value)
	{
		if ( parm_number >= 0 && parm_number < kRenderParams )
			m_IntParams[parm_number] = value;
	}
	void SetVectorRenderingParameter(int parm_number, Vector const &value)
	{
		if ( parm_number >= 0 && parm_number < kRenderParams )
			m_VectorParams[parm_number] = value;
	}

	float GetFloatRenderingParameter(int parm_number) const
	{
		return parm_number >= 0 && parm_number < kRenderParams ? m_FloatParams[parm_number] : 0.0f;
	}

	int GetIntRenderingParameter(int parm_number) const
	{
		return parm_number >= 0 && parm_number < kRenderParams ? m_IntParams[parm_number] : 0;
	}

	Vector GetVectorRenderingParameter(int parm_number) const
	{
		return parm_number >= 0 && parm_number < kRenderParams ? m_VectorParams[parm_number] : Vector( 0, 0, 0 );
	}
	static constexpr int kRenderParams = 64;
	float m_FloatParams[kRenderParams] = {};
	int m_IntParams[kRenderParams] = {};
	Vector m_VectorParams[kRenderParams];

	// Methods related to stencil
	void SetStencilEnable(bool onoff)
	{
		g_CoreStencil.enable = onoff;
	}

	void SetStencilFailOperation(StencilOperation_t op)
	{
		g_CoreStencil.fail = op;
	}

	void SetStencilZFailOperation(StencilOperation_t op)
	{
		g_CoreStencil.depthFail = op;
	}

	void SetStencilPassOperation(StencilOperation_t op)
	{
		g_CoreStencil.pass = op;
	}

	void SetStencilCompareFunction(StencilComparisonFunction_t cmpfn)
	{
		g_CoreStencil.compare = cmpfn;
	}

	void SetStencilReferenceValue(int ref)
	{
		g_CoreStencil.reference = ref;
	}

	void SetStencilTestMask(uint32 msk)
	{
		g_CoreStencil.testMask = msk;
	}

	void SetStencilWriteMask(uint32 msk)
	{
		g_CoreStencil.writeMask = msk;
	}

	void ClearStencilBufferRectangle( int xmin, int ymin, int xmax, int ymax,int value)
	{
	}

	virtual void GetDXLevelDefaults(uint &max_dxlevel,uint &recommended_dxlevel)
	{
		max_dxlevel=recommended_dxlevel=90;
	}

	virtual void GetMaxToRender( IMesh *pMesh, bool bMaxUntilFlush, int *pMaxVerts, int *pMaxIndices )
	{
		*pMaxVerts = 32768;
		*pMaxIndices = 32768;
	}

	// Returns the max possible vertices + indices to render in a single draw call
	virtual int GetMaxVerticesToRender( IMaterial *pMaterial )
	{
		return 32768;
	}

	virtual int GetMaxIndicesToRender( )
	{
		return 32768;
	}
	virtual int CompareSnapshots( StateSnapshot_t snapshot0, StateSnapshot_t snapshot1 ) { return 0; }

	virtual void DisableAllLocalLights() {}

	virtual bool SupportsMSAAMode( int nMSAAMode ) { return false; }

	virtual bool SupportsCSAAMode( int nNumSamples, int nQualityLevel ) { return false; }

	// Hooks for firing PIX events from outside the Material System...
	virtual void BeginPIXEvent( unsigned long color, const char *szName ) {}
	virtual void EndPIXEvent() {}
	virtual void SetPIXMarker( unsigned long color, const char *szName ) {}

	virtual void ComputeVertexDescription( unsigned char* pBuffer, VertexFormat_t vertexFormat, MeshDesc_t& desc ) const {}

	virtual bool SupportsShadowDepthTextures() { return false; }

	virtual bool SupportsFetch4() { return false; }

	virtual int NeedsShaderSRGBConversion(void) const { return 0; }
	virtual bool UsesSRGBCorrectBlending() const { return false; }

	virtual bool HasFastVertexTextures() const { return false; }

	virtual void SetShadowDepthBiasFactors( float fShadowSlopeScaleDepthBias, float fShadowDepthBias )
	{
		g_CoreShadowSlopeBias = fShadowSlopeScaleDepthBias;
		g_CoreShadowBias = fShadowDepthBias;
	}

	virtual void SetDisallowAccess( bool ) {}
	virtual void EnableShaderShaderMutex( bool ) {}
	virtual void ShaderLock() {}
	virtual void ShaderUnlock() {}

// ------------ New Vertex/Index Buffer interface ----------------------------
	void BindVertexBuffer( int streamID, IVertexBuffer *pVertexBuffer, int nOffsetInBytes, int nFirstVertex, int nVertexCount, VertexFormat_t fmt, int nRepetitions1 )
	{
	}
	void BindIndexBuffer( IIndexBuffer *pIndexBuffer, int nOffsetInBytes )
	{
	}
	void Draw( MaterialPrimitiveType_t primitiveType, int firstIndex, int numIndices )
	{
	}
// ------------ End ----------------------------

	virtual int  GetVertexBufferCompression( void ) const { return 0; };

	virtual bool ShouldWriteDepthToDestAlpha( void ) const { return false; };
	virtual bool SupportsHDRMode( HDRType_t nHDRMode ) const { return false; };
	virtual bool IsDX10Card() const { return false; };

	void PushDeformation( const DeformationBase_t *pDeformation )
	{
	}

	virtual void PopDeformation( )
	{
	}

	int GetNumActiveDeformations( ) const
	{
		return 0;
	}

	// for shaders to set vertex shader constants. returns a packed state which can be used to set the dynamic combo
	int GetPackedDeformationInformation( int nMaskOfUnderstoodDeformations,
										 float *pConstantValuesOut,
										 int nBufferSize,
										 int nMaximumDeformations,
										 int *pNumDefsOut ) const
	{
		*pNumDefsOut = 0;
		return 0;
	}

	void SetStandardTextureHandle(StandardTextureId_t,ShaderAPITextureHandle_t)
	{
	}

	// The dynamic-state command buffers of the DX9 shaders: texture bindings
	// matter here; shader constants and indices are walked past.
	virtual void ExecuteCommandBuffer( uint8 *pData )
	{
		for ( ;; )
		{
			int cmd = 0;
			memcpy( &cmd, pData, sizeof( int ) );
			switch ( cmd )
			{
			case CBCMD_END:
				return;
			case CBCMD_JUMP:
				memcpy( &pData, pData + sizeof( int ), sizeof( uint8 * ) );
				break;
			case CBCMD_JSR:
			{
				uint8 *target = NULL;
				memcpy( &target, pData + sizeof( int ), sizeof( uint8 * ) );
				ExecuteCommandBuffer( target );
				pData += sizeof( int ) + sizeof( uint8 * );
				break;
			}
			case CBCMD_SET_PIXEL_SHADER_FLOAT_CONST:
			case CBCMD_SET_VERTEX_SHADER_FLOAT_CONST:
			case CBCMD_SET_VERTEX_SHADER_FLOAT_CONST_REF:
			{
				int count = 0;
				memcpy( &count, pData + 2 * sizeof( int ), sizeof( int ) );
				pData += 3 * sizeof( int ) + ( cmd == CBCMD_SET_VERTEX_SHADER_FLOAT_CONST_REF ?
					sizeof( float * ) : count * 4 * sizeof( float ) );
				break;
			}
			case CBCMD_SETPIXELSHADERFOGPARAMS:
			case CBCMD_STORE_EYE_POS_IN_PSCONST:
			case CBCMD_COMMITPIXELSHADERLIGHTING:
			case CBCMD_SETPIXELSHADERSTATEAMBIENTLIGHTCUBE:
			case CBCMD_SET_PSHINDEX:
			case CBCMD_SET_VSHINDEX:
				pData += 2 * sizeof( int );
				break;
			case CBCMD_SETAMBIENTCUBEDYNAMICSTATEVERTEXSHADER:
				pData += sizeof( int );
				break;
			case CBCMD_SET_DEPTH_FEATHERING_CONST:
				pData += 2 * sizeof( int ) + sizeof( float );
				break;
			case CBCMD_BIND_STANDARD_TEXTURE:
			{
				int sampler = 0, id = 0;
				memcpy( &sampler, pData + sizeof( int ), sizeof( int ) );
				memcpy( &id, pData + 2 * sizeof( int ), sizeof( int ) );
				BindStandardTexture( (Sampler_t)sampler, (StandardTextureId_t)id );
				pData += 3 * sizeof( int );
				break;
			}
			case CBCMD_BIND_SHADERAPI_TEXTURE_HANDLE:
			{
				int sampler = 0;
				ShaderAPITextureHandle_t handle = INVALID_SHADERAPI_TEXTURE_HANDLE;
				memcpy( &sampler, pData + sizeof( int ), sizeof( int ) );
				memcpy( &handle, pData + 2 * sizeof( int ), sizeof( handle ) );
				BindTexture( (Sampler_t)sampler, handle );
				pData += 2 * sizeof( int ) + sizeof( ShaderAPITextureHandle_t );
				break;
			}
			default:
				// Unknown size: stop rather than misread the rest.
				return;
			}
		}
	}
	virtual bool GetHDREnabled( void ) const { return true; }
	virtual void SetHDREnabled( bool bEnable ) {}

	virtual void CopyRenderTargetToScratchTexture( ShaderAPITextureHandle_t srcRt, ShaderAPITextureHandle_t dstTex, Rect_t *pSrcRect = NULL, Rect_t *pDstRect = NULL ) 
	{
	}

	// Allows locking and unlocking of very specific surface types.
	virtual void LockRect( void** pOutBits, int* pOutPitch, ShaderAPITextureHandle_t texHandle, int mipmap, int x, int y, int w, int h, bool bWrite, bool bRead ) 
	{
	}

	virtual void UnlockRect( ShaderAPITextureHandle_t texHandle, int mipmap )
	{
	}

	virtual void TexLodClamp( int finest ) {}

	virtual void TexLodBias( float bias ) {}

	virtual void CopyTextureToTexture( ShaderAPITextureHandle_t srcTex, ShaderAPITextureHandle_t dstTex ) {}
	
	void PrintfVA( char *fmt, va_list vargs ) {}
	void Printf( const char *fmt, ... ) {}
	float Knob( char *knobname, float *setvalue = NULL ) { return 0.0f; };

private:
	enum
	{
		TRANSLUCENT = 0x1,
		ALPHATESTED = 0x2,
		VERTEX_AND_PIXEL_SHADERS = 0x4,
		DEPTHWRITE = 0x8,
	};

	CEmptyMesh m_Mesh;

public:
	// Runs the bound material's shader over the mesh; its passes call
	// BeginPass/RenderPass, which draw the mesh.
	void DrawMesh( CEmptyMesh *pMesh )
	{
		g_pRenderMesh = pMesh;
		if ( g_pBoundMaterial )
		{
			++g_Counters.materialDraws;
			g_pBoundMaterial->DrawMesh( VERTEX_COMPRESSION_NONE );
		}
		else
		{
			g_CurrentSnapshot = -1;
			pMesh->RenderPass();
		}
		g_pRenderMesh = NULL;
	}

private:
	void EnableAlphaToCoverage() {} ;
	void DisableAlphaToCoverage() {} ;

	ImageFormat GetShadowDepthTextureFormat() { return IMAGE_FORMAT_UNKNOWN; };
	ImageFormat GetNullTextureFormat() { return IMAGE_FORMAT_UNKNOWN; };
};


//-----------------------------------------------------------------------------
// Class Factory
//-----------------------------------------------------------------------------

static CShaderAPIEmpty g_ShaderAPIEmpty;
static CShaderShadowEmpty g_ShaderShadow;



// The PICA adapter draws base textures fullbright: it claims no semantic
// feature (no shader model, render targets or HDR).
static bool DescribePicaAdapter( int adapter, render::RenderAdapterInfo *info )
{
	if ( adapter != 0 || !info )
		return false;
	*info = render::RenderAdapterInfo();
	Q_strncpy( info->name, "PICA200 (Nintendo 3DS, fullbright)", sizeof( info->name ) );
	Q_strncpy( info->driverApi, "pica200", sizeof( info->driverApi ) );
	return true;
}

// RFC 0026 P3: the render core's passes at slots of this stream. The core
// shares this backend's device (the launcher's), so a slot records into the
// frame's own encoder and target, and the core samples this backend's
// textures by their material system handles.
render::legacy::ICorePassRecorder *g_CorePassRecorder = NULL;

class CPicaCoreTextures final : public render::legacy::ICoreTextures
{
public:
	render::device::TextureId Import( int handle, bool srgb ) override
	{
		PicaTexture *texture = TextureFor( ShaderAPITextureHandle_t( handle ) );
		// The PICA200 decodes no sRGB: the reduced model asks for none.
		// Elsewhere an sRGB import reads the texture's sRGB twin.
#if !defined( PLATFORM_3DS )
		if ( texture && srgb && !texture->renderTarget && !texture->cube )
		{
			if ( !texture->wantsSrgb )
			{
				texture->wantsSrgb = true;
				texture->dirty = true; // the next upload makes the twin
			}
			if ( texture->dirty )
				UploadTexture( *texture );
			if ( texture->gpuSrgb.Valid() )
				return render::device::TextureId{ texture->gpuSrgb.Id() };
		}
		srgb = false;
#endif
		if ( !texture || srgb )
		{
			printf( "pica: core import of texture %d refused: %s\n", handle,
				srgb ? "sRGB view" : "no such texture" );
			return render::device::TextureId{};
		}
		if ( texture && texture->dirty )
			UploadTexture( *texture );
#if !defined( PLATFORM_3DS )
		// A render target the core samples before anything drew into it: its
		// image is made now (WebGPU and Vulkan clear it to zero).
		if ( texture && texture->renderTarget && !texture->gpu.Valid() )
			(void)DrawableTarget( *texture );
#endif
		if ( !texture->gpu.Valid() )
			printf( "pica: core import of texture %d (%s) refused: not uploaded (%d levels, %dx%d)\n",
				handle, texture->name, texture->levels.Count(), texture->baseWidth, texture->baseHeight );
		return render::device::TextureId{ texture->gpu.Id() };
	}
	render::device::SamplerDesc Sampler( int handle ) override
	{
		render::device::SamplerDesc desc;
		desc.minFilter = render::device::Filter::kNearest;
		desc.magFilter = render::device::Filter::kLinear;
		desc.mipFilter = render::device::Filter::kLinear;
		const PicaTexture *texture = TextureFor( ShaderAPITextureHandle_t( handle ) );
		const auto mode = []( bool repeat )
		{
			return repeat ? render::device::AddressMode::kRepeat
			              : render::device::AddressMode::kClampToEdge;
		};
		desc.address = mode( !texture || texture->wrapS );
		desc.addressV = mode( !texture || texture->wrapT );
		return desc;
	}
	bool Pending( int handle ) override
	{
		const PicaTexture *texture = TextureFor( ShaderAPITextureHandle_t( handle ) );
#if !defined( PLATFORM_3DS )
		if ( texture && texture->renderTarget && !texture->depth )
			return false; // Import makes its image
#endif
		return texture && !texture->gpu.Valid() && texture->levels.Count() == 0;
	}
};
CPicaCoreTextures g_PicaCoreTextures;

// R91 (the desktop client's move off shaderapivulkan, parts c-e): what a
// draw and its slot take on their way to the core.
//
// DecorateCoreDraw runs on each mesh draw just before it is queued. It marks
// the slot QueueCore opens next as a mesh slot; it will also consume an
// occlusion query's proxy and attach the depth-alpha copy. True: the draw was
// consumed and is not queued.
static bool DecorateCoreDraw( render::legacy::CoreMeshDraw &draw )
{
	g_CoreMeshSlotPending = true;
#if !defined( PLATFORM_3DS )
	// $depthblend reads the frame copy's depth alpha (core_copies), as
	// shaderapivulkan's draws read _rt_FullFrameDepth.
	bool found = false;
	IMaterialVar *depthBlend = g_pBoundMaterial ? g_pBoundMaterial->FindVar( "$depthblend", &found, false ) : nullptr;
	if ( found && depthBlend && depthBlend->GetIntValue() != 0 )
	{
		const ShaderAPITextureHandle_t saved = g_BoundTextures[15];
		g_ShaderAPIEmpty.BindStandardTexture( SHADER_SAMPLER15, TEXTURE_FRAME_BUFFER_FULL_DEPTH );
		draw.depthAlphaHandle = int( g_BoundTextures[15] );
		draw.depthAlphaRange = kCoreDestAlphaDepthRange;
		g_BoundTextures[15] = saved;
	}
#endif
	return false;
}

#if !defined( PLATFORM_3DS )
static render::device::CompareOp CoreCompare( pica::Compare compare )
{
	using render::device::CompareOp;
	switch ( compare )
	{
	case pica::Compare::kNever: return CompareOp::kNever;
	case pica::Compare::kLess: return CompareOp::kLess;
	case pica::Compare::kEqual: return CompareOp::kEqual;
	case pica::Compare::kLessEqual: return CompareOp::kLessEqual;
	case pica::Compare::kGreater: return CompareOp::kGreater;
	case pica::Compare::kNotEqual: return CompareOp::kNotEqual;
	case pica::Compare::kGreaterEqual: return CompareOp::kGreaterEqual;
	case pica::Compare::kAlways: return CompareOp::kAlways;
	}
	return CompareOp::kLessEqual;
}

static render::device::CompareOp CoreStencilCompare( StencilComparisonFunction_t func )
{
	using render::device::CompareOp;
	switch ( func )
	{
	case STENCILCOMPARISONFUNCTION_NEVER: return CompareOp::kNever;
	case STENCILCOMPARISONFUNCTION_LESS: return CompareOp::kLess;
	case STENCILCOMPARISONFUNCTION_EQUAL: return CompareOp::kEqual;
	case STENCILCOMPARISONFUNCTION_LESSEQUAL: return CompareOp::kLessEqual;
	case STENCILCOMPARISONFUNCTION_GREATER: return CompareOp::kGreater;
	case STENCILCOMPARISONFUNCTION_NOTEQUAL: return CompareOp::kNotEqual;
	case STENCILCOMPARISONFUNCTION_GREATEREQUAL: return CompareOp::kGreaterEqual;
	default: return CompareOp::kAlways;
	}
}

static render::device::StencilOp CoreStencilOp( StencilOperation_t op )
{
	using render::device::StencilOp;
	switch ( op )
	{
	case STENCILOPERATION_ZERO: return StencilOp::kZero;
	case STENCILOPERATION_REPLACE: return StencilOp::kReplace;
	case STENCILOPERATION_INCRSAT: return StencilOp::kIncrementClamp;
	case STENCILOPERATION_DECRSAT: return StencilOp::kDecrementClamp;
	case STENCILOPERATION_INVERT: return StencilOp::kInvert;
	case STENCILOPERATION_INCR: return StencilOp::kIncrementWrap;
	case STENCILOPERATION_DECR: return StencilOp::kDecrementWrap;
	default: return StencilOp::kKeep;
	}
}
#endif

// DecorateCoreTarget runs as a slot is marked. A mesh slot takes the bound
// snapshot's raster state (depth test, write and compare, cull, colour and
// alpha writes), the dynamic stencil state and the poly-offset depth bias,
// as shaderapivulkan's slots do (its ApplyDepthBiasState: the material
// system's decal and normal biases, or the shadow bias factors). The 3DS
// keeps the defaults it draws with.
static void DecorateCoreTarget( render::legacy::CorePassTarget &target )
{
	const bool mesh = g_CoreMeshSlotPending;
	g_CoreMeshSlotPending = false;
#if defined( PLATFORM_3DS )
	(void)target;
	(void)mesh;
#else
	if ( !mesh || g_CurrentSnapshot < 0 || g_CurrentSnapshot >= g_Snapshots.Count() )
		return;
	const PicaSnapshot &snapshot = g_Snapshots[g_CurrentSnapshot];
	const pica::DrawState &state = snapshot.state;
	render::material::SurfaceDrawState &out = target.drawState;
	out.overrideDepth = true;
	out.depthTest = state.depthTest;
	out.depthWrite = state.depthWrite;
	out.depthCompare = CoreCompare( state.depthFunc );
	out.cull = state.cull ? render::device::CullMode::kBack : render::device::CullMode::kNone;
	out.colorWrite = std::uint8_t( ( state.colorWrite ? 7 : 0 ) | ( state.alphaWrite ? 8 : 0 ) );
	render::device::StencilState &stencil = out.stencil;
	stencil.enabled = g_CoreStencil.enable;
	stencil.compare = CoreStencilCompare( g_CoreStencil.compare );
	stencil.fail = CoreStencilOp( g_CoreStencil.fail );
	stencil.depthFail = CoreStencilOp( g_CoreStencil.depthFail );
	stencil.pass = CoreStencilOp( g_CoreStencil.pass );
	stencil.reference = std::uint8_t( g_CoreStencil.reference & 255 );
	stencil.readMask = std::uint8_t( g_CoreStencil.testMask & 255 );
	stencil.writeMask = std::uint8_t( g_CoreStencil.writeMask & 255 );
	static const MaterialSystem_Config_t defaults;
	const MaterialSystem_Config_t &config = ShaderUtil() ? ShaderUtil()->GetConfig() : defaults;
	float slope = 0.0f, normalized = 0.0f;
	if ( snapshot.polyOffset == SHADER_POLYOFFSET_DECAL )
	{
		slope = config.m_SlopeScaleDepthBias_Decal != 0.0f ? 1.0f / config.m_SlopeScaleDepthBias_Decal : 0.0f;
		normalized = config.m_DepthBias_Decal != 0.0f ? 1.0f / config.m_DepthBias_Decal : 0.0f;
	}
	else if ( snapshot.polyOffset == SHADER_POLYOFFSET_SHADOW_BIAS )
	{
		slope = g_CoreShadowSlopeBias;
		normalized = g_CoreShadowBias;
	}
	else
	{
		slope = config.m_SlopeScaleDepthBias_Normal != 0.0f ? 1.0f / config.m_SlopeScaleDepthBias_Normal : 0.0f;
		normalized = config.m_DepthBias_Normal != 0.0f ? 1.0f / config.m_DepthBias_Normal : 0.0f;
	}
	// D3D9's normalized bias in depth-buffer units of the D32F depth (2^23,
	// as shaderapivulkan's SetDynamicDepthBias).
	out.depthBiasConstant = normalized * 8388608.0f;
	out.depthBiasSlope = slope;
#endif
}

class CPicaCorePassSlots final : public render::legacy::ICorePassSlots
{
public:
	void MarkSlot( std::uint32_t tag ) override
	{
		if ( !g_CorePassRecorder )
			return;
		pica::CoreSectionTarget section;
		render::device::CommandEncoder *encoder = pica::BeginCoreSection( section );
		if ( !encoder )
			return;
		render::legacy::CorePassTarget target;
		target.device = section.device;
		target.color = render::device::TextureId{ section.color };
		target.depth = render::device::TextureId{ section.depth };
		target.colorFormat = render::device::Format::kRGBA8Unorm;
#if !defined( PLATFORM_3DS )
		// The screen and the render targets are copy sources (pica_renderer),
		// for the full model's scene-color reads.
		target.colorCopySource = true;
#endif
		target.depthFormat = pica::kDepthFormat;
		target.width = section.width;
		target.height = section.height;
		target.textures = &g_PicaCoreTextures;
		target.frame = section.serial;
		target.submitted = render::device::CompletionToken{ render::device::QueueKind::kGraphics,
			section.submittedEpoch, section.submittedValue };
		// Each recording is submitted once and discarded (no capture replays
		// it): a new epoch per recording lets the core release what it kept of
		// the last (a constant epoch kept every recorded view, up to 8192 with
		// their geometry, and a present-counted frame let retired geometry
		// pile up between presents: the 3DS ran out of memory).
		// Narrower still: a slot is recorded once and never replayed, so its
		// stream is discarded as soon as it records. A new epoch per slot lets
		// the core drop each recorded view (a model draw's whole geometry)
		// at the next slot instead of holding a frame of them (~10 MB on the
		// intro4 demo's opening).
		static std::uint64_t s_streamEpoch = 0;
		target.streamEpoch = ++s_streamEpoch;
#if defined( PLATFORM_3DS )
		// LDR, gamma space: the reduced model reads the pages as they are.
		target.lightmapScale = 1.0f;
		target.outputScale = 1.0f;
		target.specular = false;
#else
		// The full model's LDR terms, as shaderapivulkan's LDR path passes
		// them (this shader API reports HDR_TYPE_NONE): lightmap pages scaled
		// by 2^2.2, no tone-mapping scale, the eye (c10), env maps at 1, and
		// specular unless mat_fastspecular is off or mat_fullbright 2. Albedo
		// is read through sRGB twins (CPicaCoreTextures::Import).
		target.lightmapScale = powf( 2.0f, 2.2f );
		target.outputScale = 1.0f;
		{
			float eye[4];
			g_ShaderAPIEmpty.GetWorldSpaceCameraPosition( eye );
			for ( int i = 0; i < 3; ++i )
				target.eye[i] = eye[i];
		}
		target.envmapScale = 1.0f;
		static ConVarRef fastSpecular( "mat_fastspecular" );
		static ConVarRef fullbright( "mat_fullbright" );
		target.specular = ( !fastSpecular.IsValid() || fastSpecular.GetBool() ) &&
		                  ( !fullbright.IsValid() || fullbright.GetInt() != 2 );
#endif
		// The wind and foliage time ($treesway's inputs; the reduced model
		// drops the sway, but the pass reads them).
		const Vector wind = g_ShaderAPIEmpty.GetVectorRenderingParameter( VECTOR_RENDERPARM_WIND_DIRECTION );
		const Vector previousWind =
			g_ShaderAPIEmpty.GetVectorRenderingParameter( VECTOR_RENDERPARM_PREVIOUS_WIND_DIRECTION );
		target.foliage[0][0] = wind.x;
		target.foliage[0][1] = wind.y;
		target.foliage[0][2] = g_ShaderAPIEmpty.GetFloatRenderingParameter( FLOAT_RENDERPARM_FOLIAGE_TIME );
		target.foliage[1][0] = previousWind.x;
		target.foliage[1][1] = previousWind.y;
		target.foliage[1][2] =
			g_ShaderAPIEmpty.GetFloatRenderingParameter( FLOAT_RENDERPARM_PREVIOUS_FOLIAGE_TIME );
		target.foliageAvailable =
			g_ShaderAPIEmpty.GetFloatRenderingParameter( FLOAT_RENDERPARM_FOLIAGE_AVAILABLE ) > 0.0f;
		DecorateCoreTarget( target );
		g_CorePassRecorder->RecordSlot( tag, *encoder, target );
		pica::EndCoreSection();
	}
};
CPicaCorePassSlots g_PicaCorePassSlots;

static void FillLifecycle( render::LegacyShaderServices *services );

static bool CreatePicaShaderBackend( render::LegacyShaderServices *services )
{
	if ( !services )
		return false;
	FillLifecycle( services );
	services->api = &g_ShaderAPIEmpty;
	services->stream = &s_ShaderDeviceEmpty;
	services->shadow = &g_ShaderShadow;
	services->hardware = &g_ShaderAPIEmpty;
	services->debugTextures = &g_ShaderAPIEmpty;
	services->describeAdapter = DescribePicaAdapter;
	services->corePassSlots = &g_PicaCorePassSlots;
	return true;
}

extern "C" DLL_EXPORT void PicaShaderBackend_BindCorePassRecorder(
	render::legacy::ICorePassRecorder *recorder )
{
	g_CorePassRecorder = recorder;
}

extern "C" DLL_EXPORT void PicaShaderBackend_BindDevice( render::device::IRenderDevice2 *device )
{
#if !defined( PLATFORM_3DS )
	// The fixed display (presentation.fixedDisplay) is the window's size the
	// launch names: the browser page's canvas (RFC 0029).
	pica::SetScreenSize( CommandLine()->ParmValue( "-w", 1280 ), CommandLine()->ParmValue( "-h", 720 ) );
#endif
	pica::BindDevice( device );
}

#if !defined( PLATFORM_3DS )
extern "C" DLL_EXPORT void PicaShaderBackend_BindPresenter( pica::Presenter presenter, void *context )
{
	pica::BindPresenter( presenter, context );
}
#endif

DLL_EXPORT const render::LegacyShaderProvider *PicaShaderBackend_Describe()
{
	static const render::LegacyShaderProvider provider = {
	    "pica", "shaderapipica", CreatePicaShaderBackend, false };
	return &provider;
}

// FIXME: Remove; it's for backward compat with the materialsystem only for now
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderAPIEmpty, IShaderAPI, 
									SHADERAPI_INTERFACE_VERSION, g_ShaderAPIEmpty )

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderShadowEmpty, IShaderShadow, 
								SHADERSHADOW_INTERFACE_VERSION, g_ShaderShadow )

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderAPIEmpty, IMaterialSystemHardwareConfig, 
				MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, g_ShaderAPIEmpty )

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderAPIEmpty, IDebugTextureInfo, 
				DEBUG_TEXTURE_INFO_VERSION, g_ShaderAPIEmpty )


//-----------------------------------------------------------------------------
// The main GL Shader util interface
//-----------------------------------------------------------------------------
IShaderUtil* g_pShaderUtil;


//-----------------------------------------------------------------------------
// Factory to return from SetMode
//-----------------------------------------------------------------------------
static void* ShaderInterfaceFactory( const char *pInterfaceName, int *pReturnCode )
{
	if ( pReturnCode )
	{
		*pReturnCode = IFACE_OK;
	}
	if ( !Q_stricmp( pInterfaceName, SHADERAPI_INTERFACE_VERSION ) )
		return static_cast< IShaderAPI* >( &g_ShaderAPIEmpty );
	if ( !Q_stricmp( pInterfaceName, SHADERSHADOW_INTERFACE_VERSION ) )
		return static_cast< IShaderShadow* >( &g_ShaderShadow );

	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;
}


//-----------------------------------------------------------------------------
// The backend's app-system lifecycle and its presentation (the 3DS top
// screen), which the material system's IShaderDeviceMgr and IShaderDevice
// facades forward and answer from (RFC 0016 legacy device facade F4).
//-----------------------------------------------------------------------------
static void FillLifecycle( render::LegacyShaderServices *services )
{
	services->lifecycle.connect = []( void *, CreateInterfaceFn factory )
	{
		// So others can access it
		g_pShaderUtil = (IShaderUtil *)factory( SHADER_UTIL_INTERFACE_VERSION, NULL );
		return true;
	};
	services->lifecycle.disconnect = []( void * )
	{
		g_pShaderUtil = NULL;
	};
	services->lifecycle.init = []( void * )
	{
		return true;
	};
	services->lifecycle.shutdown = []( void * ) {};
	services->lifecycle.setAdapter = []( void *, int, int )
	{
		return true;
	};
	services->lifecycle.setMode = []( void *, void *hWnd, int nAdapter, const ShaderDeviceInfo_t & )
	{
		return g_ShaderAPIEmpty.SetMode( hWnd, nAdapter, ShaderDeviceInfo_t() )
		           ? static_cast<CreateInterfaceFn>( ShaderInterfaceFactory )
		           : static_cast<CreateInterfaceFn>( NULL );
	};
	// The top screen: 400 x 240 at 60 Hz, presented without antialiasing or
	// stencil, its one mode.
	services->presentation.facts = []( void *, render::LegacyPresentationFacts *out )
	{
		*out = render::LegacyPresentationFacts();
		out->presenting = true;
		out->backBufferWidth = out->windowWidth = pica::kScreenWidth;
		out->backBufferHeight = out->windowHeight = pica::kScreenHeight;
	};
	services->presentation.fixedDisplay = []( void *, int *width, int *height, int *refreshHz )
	{
		*width = pica::kScreenWidth;
		*height = pica::kScreenHeight;
		*refreshHz = 60;
		return true;
	};
}

void CShaderDeviceEmpty::GetBackBufferDimensions( int& width, int& height ) const
{
	width = pica::kScreenWidth;
	height = pica::kScreenHeight;
}

// Creates/ destroys a child window
bool CShaderDeviceEmpty::AddView( void* hwnd )
{
	return true;
}

void CShaderDeviceEmpty::RemoveView( void* hwnd )
{
}

// Activates a view
void CShaderDeviceEmpty::SetView( void* hwnd )
{
}

void CShaderDeviceEmpty::ReleaseResources()
{
}

void CShaderDeviceEmpty::ReacquireResources()
{
}

// Creates/destroys Mesh
IMesh* CShaderDeviceEmpty::CreateStaticMesh( VertexFormat_t fmt, const char *pTextureBudgetGroup, IMaterial * pMaterial )
{
	CEmptyMesh *mesh = new CEmptyMesh( false );
	mesh->SetVertexFormat( fmt & ~VERTEX_FORMAT_COMPRESSED );
	return mesh;
}

void CShaderDeviceEmpty::DestroyStaticMesh( IMesh* mesh )
{
	if ( mesh && mesh != &m_Mesh && mesh != &m_DynamicMesh )
		delete static_cast<CEmptyMesh *>( mesh );
}

// Creates/destroys static vertex + index buffers
IVertexBuffer *CShaderDeviceEmpty::CreateVertexBuffer( ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pTextureBudgetGroup )
{
	return ( type == SHADER_BUFFER_TYPE_STATIC || type == SHADER_BUFFER_TYPE_STATIC_TEMP ) ? &m_Mesh : &m_DynamicMesh;
}

void CShaderDeviceEmpty::DestroyVertexBuffer( IVertexBuffer *pVertexBuffer )
{

}

IIndexBuffer *CShaderDeviceEmpty::CreateIndexBuffer( ShaderBufferType_t bufferType, MaterialIndexFormat_t fmt, int nIndexCount, const char *pTextureBudgetGroup )
{
	switch( bufferType )
	{
	case SHADER_BUFFER_TYPE_STATIC:
	case SHADER_BUFFER_TYPE_STATIC_TEMP:
		return &m_Mesh;
	default:
		Assert( 0 );
	case SHADER_BUFFER_TYPE_DYNAMIC:
	case SHADER_BUFFER_TYPE_DYNAMIC_TEMP:
		return &m_DynamicMesh;
	}
}

void CShaderDeviceEmpty::DestroyIndexBuffer( IIndexBuffer *pIndexBuffer )
{

}

IVertexBuffer *CShaderDeviceEmpty::GetDynamicVertexBuffer( int streamID, VertexFormat_t vertexFormat, bool bBuffered )
{
	return &m_DynamicMesh;
}

IIndexBuffer *CShaderDeviceEmpty::GetDynamicIndexBuffer( MaterialIndexFormat_t fmt, bool bBuffered )
{
	return &m_Mesh;
}



//-----------------------------------------------------------------------------
//
// The empty mesh...
//
//-----------------------------------------------------------------------------
CEmptyMesh::CEmptyMesh( bool bIsDynamic ) : m_bIsDynamic( bIsDynamic )
{
	m_Format = VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE( 0, 2 );
	m_Type = MATERIAL_TRIANGLES;
	m_pVertexSource = NULL;
	m_pVertices = NULL;
	m_nVertexCapacity = m_nVertices = m_nLockFirstVertex = 0;
	m_pIndices = NULL;
	m_nIndexCapacity = m_nIndices = m_nLockFirstIndex = 0;
	m_pBoneWeights = NULL;
	m_pBoneIndices = NULL;
	m_nBoneCapacity = 0;
	m_nDrawFirst = m_nDrawCount = 0;
	m_pPrims = NULL;
	m_nPrims = 0;
}

CEmptyMesh::~CEmptyMesh()
{
	FreeStorage( m_pVertices );
	FreeStorage( m_pIndices );
	FreeStorage( m_pBoneWeights );
	delete[] m_pBoneIndices;
	delete[] m_pWideTexCoords;
	FreeStorage( m_pNormals );
	FreeStorage( m_pSkinSlots );
	delete[] m_pTexCoord1;
}

bool CEmptyMesh::EnsureWideTexCoords()
{
	if ( m_nWideTexCoordCapacity >= m_nVertexCapacity )
		return m_pWideTexCoords != NULL;
	float *wide = new float[m_nVertexCapacity * 4];
	memset( wide, 0, m_nVertexCapacity * 4 * sizeof( float ) );
	if ( m_pWideTexCoords )
		memcpy( wide, m_pWideTexCoords, m_nWideTexCoordCapacity * 4 * sizeof( float ) );
	delete[] m_pWideTexCoords;
	m_pWideTexCoords = wide;
	m_nWideTexCoordCapacity = m_nVertexCapacity;
	return true;
}

void CEmptyMesh::CommitWideTexCoords( int first, int count )
{
	if ( !m_pWideTexCoords || !m_pVertices || first < 0 || count <= 0 ||
		first + count > m_nWideTexCoordCapacity || first + count > m_nVertexCapacity )
		return;
	for ( int i = first; i < first + count; ++i )
	{
		m_pVertices[i].uv[0] = m_pWideTexCoords[i * 4 + 0];
		m_pVertices[i].uv[1] = m_pWideTexCoords[i * 4 + 1];
	}
}

bool CEmptyMesh::Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t& desc )
{
	// A lock that grants indices writes them (Modify's own empty lock does not).
	if ( nMaxIndexCount > 0 )
		++m_nContentRevision;
	const int first = bAppend ? m_nIndices : 0;
	if ( !bAppend )
		m_nIndices = 0;
	static unsigned short s_Bogus[8];
	m_nLockedIndices = 0;
	if ( !EnsureIndices( first + ( nMaxIndexCount > 0 ? nMaxIndexCount : 0 ), !bAppend ) || nMaxIndexCount <= 0 )
	{
		desc.m_pIndices = s_Bogus;
		desc.m_nIndexSize = 0;
		desc.m_nFirstIndex = 0;
		desc.m_nOffset = 0;
		return nMaxIndexCount <= 0;
	}
	pica::PrepareWrite( m_pIndices );
	m_nLockFirstIndex = first;
	m_nLockedIndices = nMaxIndexCount;
	desc.m_pIndices = m_pIndices + first;
	desc.m_nIndexSize = 1;
	desc.m_nFirstIndex = first;
	desc.m_nOffset = first * sizeof( unsigned short );
	return true;
}

void CEmptyMesh::Unlock( int nWrittenIndexCount, IndexDesc_t& desc )
{
	// Never more than the lock granted, inside the buffer.
	if ( nWrittenIndexCount > m_nLockedIndices )
		nWrittenIndexCount = m_nLockedIndices;
	if ( nWrittenIndexCount > m_nIndexCapacity - m_nLockFirstIndex )
		nWrittenIndexCount = m_nIndexCapacity - m_nLockFirstIndex;
	m_nLockedIndices = 0;
	if ( nWrittenIndexCount > 0 && m_pIndices )
	{
		if ( m_nLockFirstIndex + nWrittenIndexCount > m_nIndices )
			m_nIndices = m_nLockFirstIndex + nWrittenIndexCount;
		if ( !m_bIsDynamic )
			pica::FlushLinear( m_pIndices + m_nLockFirstIndex, nWrittenIndexCount * sizeof( unsigned short ) );
	}
}

void CEmptyMesh::ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t& desc )
{
	if ( !bReadOnly )
		++m_nContentRevision;
	if ( !EnsureIndices( nFirstIndex + nIndexCount ) )
	{
		Lock( 0, false, desc );
		return;
	}
	// In place: a recorded draw reading these indices is submitted first.
	if ( !bReadOnly )
		pica::PrepareWrite( m_pIndices );
	m_nLockFirstIndex = nFirstIndex;
	m_nLockedIndices = nIndexCount;
	desc.m_pIndices = m_pIndices + nFirstIndex;
	desc.m_nIndexSize = 1;
	desc.m_nFirstIndex = nFirstIndex;
	desc.m_nOffset = nFirstIndex * sizeof( unsigned short );
}

void CEmptyMesh::ModifyEnd( IndexDesc_t& desc )
{
	if ( m_pIndices && !m_bIsDynamic )
		pica::FlushLinear( m_pIndices, m_nIndices * sizeof( unsigned short ) );
}

void CEmptyMesh::Spew( int nIndexCount, const IndexDesc_t & desc )
{
}

void CEmptyMesh::ValidateData( int nIndexCount, const IndexDesc_t &desc )
{
}

bool CEmptyMesh::Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc )
{
	// Components the record does not hold (normals, tangents, extra
	// texture coordinates, user data) are written to a scratch slot that never
	// advances (size 0).
	static float s_Scratch[32];
	const int first = bAppend ? m_nVertices : 0;
	if ( !bAppend )
		m_nVertices = 0;
	const bool skinned = NumBoneWeights( m_Format ) > 0;
	if ( nVertexCount <= 0 || !EnsureVertices( first + nVertexCount, !bAppend ) )
		nVertexCount = 0;
	m_nLockedVertices = nVertexCount;
	// A lock that grants vertices writes them (Modify's own empty lock does not).
	if ( nVertexCount > 0 )
		PrepareVertexWrite();

	desc.m_pPosition = s_Scratch;
	desc.m_pNormal = s_Scratch;
	desc.m_pColor = (unsigned char *)s_Scratch;
	for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
	{
		desc.m_pTexCoord[i] = s_Scratch;
		desc.m_VertexSize_TexCoord[i] = 0;
	}
	desc.m_pBoneWeight = s_Scratch;
	desc.m_pBoneMatrixIndex = (unsigned char *)s_Scratch;
	desc.m_pTangentS = s_Scratch;
	desc.m_pTangentT = s_Scratch;
	desc.m_pUserData = s_Scratch;
	desc.m_NumBoneWeights = skinned ? 2 : 0;
	desc.m_VertexSize_Position = 0;
	desc.m_VertexSize_BoneWeight = 0;
	desc.m_VertexSize_BoneMatrixIndex = 0;
	desc.m_VertexSize_Normal = 0;
	desc.m_VertexSize_Color = 0;
	desc.m_VertexSize_TangentS = 0;
	desc.m_VertexSize_TangentT = 0;
	desc.m_VertexSize_UserData = 0;
	desc.m_ActualVertexSize = 0;
	desc.m_CompressionType = VERTEX_COMPRESSION_NONE;
	desc.m_nFirstVertex = first;
	desc.m_nOffset = first * sizeof( pica::Vertex );
	m_nLockFirstVertex = first;
	if ( nVertexCount == 0 )
		return true;

	// Defaults for what the format leaves unwritten: white, (0, 0), unskinned.
	for ( int i = first; i < first + nVertexCount; ++i )
	{
		pica::Vertex &v = m_pVertices[i];
		memset( v.pos, 0, sizeof( v.pos ) );
		memset( v.color, 0xFF, sizeof( v.color ) );
		v.uv[0] = v.uv[1] = 0.0f;
	}
	pica::Vertex *base = m_pVertices + first;
	const int stride = sizeof( pica::Vertex );
	desc.m_pPosition = base->pos;
	desc.m_VertexSize_Position = stride;
	if ( m_Format & VERTEX_COLOR )
	{
		desc.m_pColor = base->color;
		desc.m_VertexSize_Color = stride;
	}
	if ( m_pNormals && first + nVertexCount <= m_nNormalCapacity )
	{
		desc.m_pNormal = m_pNormals + first * 3;
		desc.m_VertexSize_Normal = 3 * sizeof( float );
	}
	if ( TexCoordSize( 0, m_Format ) > 2 )
		++g_Counters.wideTexCoordLocks;
	if ( WideTexCoords() && EnsureWideTexCoords() )
	{
		memset( m_pWideTexCoords + first * 4, 0, nVertexCount * 4 * sizeof( float ) );
		desc.m_pTexCoord[0] = m_pWideTexCoords + first * 4;
		desc.m_VertexSize_TexCoord[0] = 4 * sizeof( float );
	}
	else if ( TexCoordSize( 0, m_Format ) > 0 )
	{
		desc.m_pTexCoord[0] = base->uv;
		desc.m_VertexSize_TexCoord[0] = stride;
	}
	if ( m_pTexCoord1 && first + nVertexCount <= m_nTexCoord1Capacity )
	{
		const int size = TexCoordSize( 1, m_Format );
		memset( m_pTexCoord1 + first * size, 0, nVertexCount * size * sizeof( float ) );
		desc.m_pTexCoord[1] = m_pTexCoord1 + first * size;
		desc.m_VertexSize_TexCoord[1] = size * sizeof( float );
	}
	if ( skinned )
	{
		memset( m_pBoneWeights + first * 2, 0, nVertexCount * 2 * sizeof( float ) );
		memset( m_pBoneIndices + first * 4, 0, nVertexCount * 4 );
		desc.m_pBoneWeight = m_pBoneWeights + first * 2;
		desc.m_VertexSize_BoneWeight = 2 * sizeof( float );
		if ( m_Format & VERTEX_BONE_INDEX )
		{
			desc.m_pBoneMatrixIndex = m_pBoneIndices + first * 4;
			desc.m_VertexSize_BoneMatrixIndex = 4;
		}
	}
	desc.m_ActualVertexSize = stride;
	return true;
}

void CEmptyMesh::Unlock( int nVertexCount, VertexDesc_t &desc )
{
	// Never more than the lock granted, inside the buffer.
	if ( nVertexCount > m_nLockedVertices )
		nVertexCount = m_nLockedVertices;
	if ( nVertexCount > m_nVertexCapacity - m_nLockFirstVertex )
		nVertexCount = m_nVertexCapacity - m_nLockFirstVertex;
	m_nLockedVertices = 0;
	if ( WideTexCoords() )
		CommitWideTexCoords( m_nLockFirstVertex, nVertexCount );
	if ( nVertexCount > 0 && m_pVertices )
	{
		if ( m_nLockFirstVertex + nVertexCount > m_nVertices )
			m_nVertices = m_nLockFirstVertex + nVertexCount;
		if ( !m_bIsDynamic )
			pica::FlushLinear( m_pVertices + m_nLockFirstVertex, nVertexCount * sizeof( pica::Vertex ) );
	}
}

void CEmptyMesh::Spew( int nVertexCount, const VertexDesc_t &desc ) 
{
}

void CEmptyMesh::ValidateData( int nVertexCount, const VertexDesc_t & desc )
{
}

void CEmptyMesh::LockMesh( int numVerts, int numIndices, MeshDesc_t& desc )
{
	Lock( numVerts, false, *static_cast<VertexDesc_t*>( &desc ) );
	Lock( numIndices, false, *static_cast<IndexDesc_t*>( &desc ) );
}

void CEmptyMesh::UnlockMesh( int numVerts, int numIndices, MeshDesc_t& desc )
{
	Unlock( numVerts, *static_cast<VertexDesc_t*>( &desc ) );
	Unlock( numIndices, *static_cast<IndexDesc_t*>( &desc ) );
}

void CEmptyMesh::ModifyBeginEx( bool bReadOnly, int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t& desc )
{
	// Point at the existing data in place (no defaults written).
	if ( !bReadOnly )
		PrepareVertexWrite();
	const int savedVertices = m_nVertices, savedIndices = m_nIndices;
	Lock( 0, false, *static_cast<VertexDesc_t*>( &desc ) );
	m_nVertices = savedVertices;
	m_nIndices = savedIndices;
	VertexDesc_t &vdesc = *static_cast<VertexDesc_t*>( &desc );
	if ( m_pVertices && firstVertex + numVerts <= m_nVertexCapacity )
	{
		pica::Vertex *base = m_pVertices + firstVertex;
		const int stride = sizeof( pica::Vertex );
		vdesc.m_pPosition = base->pos;
		vdesc.m_VertexSize_Position = stride;
		if ( m_Format & VERTEX_COLOR )
		{
			vdesc.m_pColor = base->color;
			vdesc.m_VertexSize_Color = stride;
		}
		if ( m_pNormals && firstVertex + numVerts <= m_nNormalCapacity )
		{
			vdesc.m_pNormal = m_pNormals + firstVertex * 3;
			vdesc.m_VertexSize_Normal = 3 * sizeof( float );
		}
		if ( m_pTexCoord1 && firstVertex + numVerts <= m_nTexCoord1Capacity )
		{
			const int size = TexCoordSize( 1, m_Format );
			vdesc.m_pTexCoord[1] = m_pTexCoord1 + firstVertex * size;
			vdesc.m_VertexSize_TexCoord[1] = size * sizeof( float );
		}
		if ( WideTexCoords() && EnsureWideTexCoords() )
		{
			vdesc.m_pTexCoord[0] = m_pWideTexCoords + firstVertex * 4;
			vdesc.m_VertexSize_TexCoord[0] = 4 * sizeof( float );
			m_nModifyFirstVertex = firstVertex;
			m_nModifyVertexCount = numVerts;
		}
		else if ( TexCoordSize( 0, m_Format ) > 0 )
		{
			vdesc.m_pTexCoord[0] = base->uv;
			vdesc.m_VertexSize_TexCoord[0] = stride;
		}
		vdesc.m_nFirstVertex = firstVertex;
		vdesc.m_nOffset = firstVertex * stride;
		vdesc.m_ActualVertexSize = stride;
		m_nLockFirstVertex = firstVertex;
	}
	ModifyBegin( bReadOnly, firstIndex, numIndices, *static_cast<IndexDesc_t*>( &desc ) );
}

void CEmptyMesh::ModifyBegin( int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t& desc )
{
	ModifyBeginEx( false, firstVertex, numVerts, firstIndex, numIndices, desc );
}

void CEmptyMesh::ModifyEnd( MeshDesc_t& desc )
{
	if ( m_nModifyVertexCount > 0 )
		CommitWideTexCoords( m_nModifyFirstVertex, m_nModifyVertexCount );
	m_nModifyVertexCount = 0;
	if ( m_pVertices && !m_bIsDynamic )
		pica::FlushLinear( m_pVertices, m_nVertices * sizeof( pica::Vertex ) );
	ModifyEnd( *static_cast<IndexDesc_t*>( &desc ) );
}

// returns the # of vertices (static meshes only)
int CEmptyMesh::VertexCount() const
{
	return m_nVertices;
}

// Sets the primitive type
void CEmptyMesh::SetPrimitiveType( MaterialPrimitiveType_t type )
{
	m_Type = type;
}

// Draws the entire mesh
void CEmptyMesh::Draw( int firstIndex, int numIndices )
{
	++g_Counters.meshDraws;
	if ( !pica::Initialized() || SourceVertexCount() <= 0 )
		return;
	m_pPrims = NULL;
	m_nPrims = 0;
	m_nDrawFirst = firstIndex > 0 ? firstIndex : 0;
	m_nDrawCount = numIndices > 0 ? numIndices : m_nIndices;
	g_ShaderAPIEmpty.DrawMesh( this );
}

void CEmptyMesh::Draw(CPrimList *pPrims, int nPrims)
{
	++g_Counters.primListDraws;
	if ( !pica::Initialized() || SourceVertexCount() <= 0 || nPrims <= 0 )
	{
		++g_Counters.primListEmpty;
		return;
	}
	m_pPrims = pPrims;
	m_nPrims = nPrims;
	g_ShaderAPIEmpty.DrawMesh( this );
	m_pPrims = NULL;
	m_nPrims = 0;
}

void *CEmptyMesh::AllocStorage( pica::Memory kind, size_t bytes )
{
	// A model's mesh (normals, no lightmap coordinates) keeps its one copy in
	// device memory, which the render core reads in place (EmitToCore's
	// CoreMeshStreams); world meshes, which the core draws from its own
	// data, stay CPU memory.
	const bool inPlace = ( m_Format & VERTEX_NORMAL ) && TexCoordSize( 1, m_Format ) == 0;
	return m_bIsDynamic ? malloc( bytes ) : pica::AllocLinear( kind, bytes, inPlace );
}

void CEmptyMesh::FreeStorage( void *storage )
{
	if ( m_bIsDynamic )
		free( storage );
	else
		pica::FreeLinear( storage );
}

bool CEmptyMesh::EnsureVertices( int count, bool exact )
{
	if ( count <= m_nVertexCapacity )
		return m_pVertices != NULL;
	int capacity = m_nVertexCapacity ? m_nVertexCapacity : 64;
	while ( capacity < count )
		capacity *= 2;
	if ( exact && !m_bIsDynamic )
		capacity = count;
	pica::Vertex *vertices = static_cast<pica::Vertex *>(
		AllocStorage( pica::Memory::kVertices, capacity * sizeof( pica::Vertex ) ) );
	if ( !vertices )
		return false;
	if ( m_pVertices )
	{
		memcpy( vertices, m_pVertices, m_nVertexCapacity * sizeof( pica::Vertex ) );
		FreeStorage( m_pVertices );
	}
	m_pVertices = vertices;
	m_nVertexCapacity = capacity;
	if ( NumBoneWeights( m_Format ) > 0 )
	{
		// The weights with the vertices (the core reads them in place); the
		// bone indices stay CPU memory (the palette slots replace them).
		float *weights = static_cast<float *>(
			AllocStorage( pica::Memory::kVertices, capacity * 2 * sizeof( float ) ) );
		if ( !weights )
			return false;
		unsigned char *indices = new unsigned char[capacity * 4];
		if ( m_pBoneWeights )
		{
			memcpy( weights, m_pBoneWeights, m_nBoneCapacity * 2 * sizeof( float ) );
			memcpy( indices, m_pBoneIndices, m_nBoneCapacity * 4 );
		}
		FreeStorage( m_pBoneWeights );
		delete[] m_pBoneIndices;
		FreeStorage( m_pSkinSlots );
		m_pSkinSlots = NULL;
		m_nSkinSlotsRevision = ~0u;
		m_pBoneWeights = weights;
		m_pBoneIndices = indices;
		m_nBoneCapacity = capacity;
	}
	if ( m_Format & VERTEX_NORMAL )
	{
		float *normals = static_cast<float *>(
			AllocStorage( pica::Memory::kVertices, capacity * 3 * sizeof( float ) ) );
		if ( !normals )
			return false;
		memset( normals, 0, capacity * 3 * sizeof( float ) );
		if ( m_pNormals )
			memcpy( normals, m_pNormals, m_nNormalCapacity * 3 * sizeof( float ) );
		FreeStorage( m_pNormals );
		m_pNormals = normals;
		m_nNormalCapacity = capacity;
	}
	// Dynamic meshes only (decals, overlays): the static world meshes'
	// copy would be megabytes the core never reads (it draws the world
	// from the map's own data).
	if ( m_bIsDynamic && TexCoordSize( 1, m_Format ) >= 2 )
	{
		const int size = TexCoordSize( 1, m_Format );
		float *coords = new float[capacity * size];
		memset( coords, 0, capacity * size * sizeof( float ) );
		if ( m_pTexCoord1 )
			memcpy( coords, m_pTexCoord1, m_nTexCoord1Capacity * size * sizeof( float ) );
		delete[] m_pTexCoord1;
		m_pTexCoord1 = coords;
		m_nTexCoord1Capacity = capacity;
	}
	return true;
}

bool CEmptyMesh::EnsureIndices( int count, bool exact )
{
	if ( count <= m_nIndexCapacity )
		return count == 0 || m_pIndices != NULL;
	int capacity = m_nIndexCapacity ? m_nIndexCapacity : 192;
	while ( capacity < count )
		capacity *= 2;
	if ( exact && !m_bIsDynamic )
		capacity = count;
	unsigned short *indices = static_cast<unsigned short *>(
		AllocStorage( pica::Memory::kIndices, capacity * sizeof( unsigned short ) ) );
	if ( !indices )
		return false;
	if ( m_pIndices )
	{
		memcpy( indices, m_pIndices, m_nIndexCapacity * sizeof( unsigned short ) );
		FreeStorage( m_pIndices );
	}
	m_pIndices = indices;
	m_nIndexCapacity = capacity;
	return true;
}

void CEmptyMesh::RenderPass()
{
	++g_Counters.renderPasses;
	if ( !g_bDrawingToBackBuffer )
	{
		++g_Counters.targetSkips;
		return;
	}
	if ( m_pPrims )
	{
		for ( int i = 0; i < m_nPrims; ++i )
			DrawRange( m_pPrims[i].m_FirstIndex, m_pPrims[i].m_NumIndices );
	}
	else
		DrawRange( m_nDrawFirst, m_nDrawCount );
}

namespace
{

// A material's variables in the core's form, and the raw signature they were
// made from (EmitToCore).
struct MaterialVariables
{
	std::vector<std::uint32_t> signature;
	std::vector<std::string> texts;
	std::vector<render::legacy::CoreMeshVariable> variables;
	std::uint64_t revision = 0; // CoreMeshDraw::materialRevision: new on every rebuild
};

std::uint32_t FloatBits( float value )
{
	std::uint32_t bits;
	memcpy( &bits, &value, sizeof( bits ) );
	return bits;
}

// What the text form depends on, read without formatting.
void SignatureOf( IMaterialInternal *material, std::vector<std::uint32_t> &out )
{
	out.clear();
	IMaterialVar **params = material->GetShaderParams();
	const int count = material->ShaderParamCount();
	out.push_back( std::uint32_t( count ) );
	out.push_back( std::uint32_t( reinterpret_cast<std::uintptr_t>( params ) ) );
	for ( int i = 0; i < count; ++i )
	{
		IMaterialVar *var = params[i];
		if ( !var || !var->IsDefined() )
		{
			out.push_back( 0xFFFFFFFFu );
			continue;
		}
		const MaterialVarType_t type = var->GetType();
		out.push_back( std::uint32_t( type ) );
		switch ( type )
		{
		case MATERIAL_VAR_TYPE_INT:
			out.push_back( std::uint32_t( var->GetIntValue() ) );
			break;
		case MATERIAL_VAR_TYPE_FLOAT:
			out.push_back( FloatBits( var->GetFloatValue() ) );
			break;
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			const int size = var->VectorSize();
			const float *vec = var->GetVecValue();
			out.push_back( std::uint32_t( size ) );
			for ( int j = 0; j < size && j < 4; ++j )
				out.push_back( FloatBits( vec[j] ) );
			break;
		}
		case MATERIAL_VAR_TYPE_TEXTURE:
		{
			ITexture *texture = var->GetTextureValue();
			out.push_back( std::uint32_t( reinterpret_cast<std::uintptr_t>( texture ) ) );
			out.push_back( texture ? std::uint32_t(
				static_cast<ITextureInternal *>( texture )->GetTextureHandle( 0 ) ) : 0u );
			break;
		}
		case MATERIAL_VAR_TYPE_MATRIX:
		{
			const VMatrix &matrix = var->GetMatrixValue();
			for ( int r = 0; r < 4; ++r )
				for ( int c = 0; c < 4; ++c )
					out.push_back( FloatBits( matrix.m[r][c] ) );
			break;
		}
		default:
		{
			// Strings (and the rest): a hash of the text, which for these
			// types is stored, not formatted.
			const char *text = var->GetStringValue();
			std::uint32_t hash = 2166136261u;
			for ( const char *c = text ? text : ""; *c; ++c )
				hash = ( hash ^ std::uint8_t( *c ) ) * 16777619u;
			out.push_back( hash );
			break;
		}
		}
	}
	std::uint32_t flags = 0, bit = 1;
	for ( const auto &flag : RenderLegacyMaterialFlags::Keys )
	{
		if ( material->GetMaterialVarFlag( flag.flag ) )
			flags |= bit;
		bit <<= 1;
	}
	out.push_back( flags );
}

void BuildVariables( IMaterialInternal *material, MaterialVariables &out )
{
	out.texts.clear();
	out.variables.clear();
	IMaterialVar **params = material->GetShaderParams();
	IShader *shader = material->GetShader();
	const int paramCount = material->ShaderParamCount();
	out.texts.reserve( paramCount + 1 );
	for ( int i = 0; i < paramCount; ++i )
	{
		IMaterialVar *var = params[i];
		if ( !var || !var->IsDefined() || !V_strnicmp( var->GetName(), "$flags", 6 ) )
			continue;
		render::legacy::CoreMeshVariable value;
		value.key = var->GetName();
		out.texts.push_back( var->GetStringValue() );
		if ( shader && i < shader->GetNumParams() && !V_stricmp( shader->GetParamName( i ), value.key ) )
			value.defaultValue = shader->GetParamDefault( i );
		if ( var->GetType() == MATERIAL_VAR_TYPE_TEXTURE && var->GetTextureValue() )
		{
			ITextureInternal *texture = static_cast<ITextureInternal *>( var->GetTextureValue() );
			value.textureHandle = int( texture->GetTextureHandle( 0 ) );
		}
		out.variables.push_back( value );
	}
	// The texts are final: point the values at them.
	for ( size_t i = 0; i < out.variables.size(); ++i )
		out.variables[i].value = out.texts[i].c_str();
	for ( const auto &flag : RenderLegacyMaterialFlags::Keys )
		if ( material->GetMaterialVarFlag( flag.flag ) )
			out.variables.push_back( { flag.key, "1", "0", 0 } );
}

const MaterialVariables &VariablesFor( IMaterialInternal *material )
{
	static std::unordered_map<IMaterialInternal *, MaterialVariables> s_cache;
	static std::vector<std::uint32_t> s_signature;
	SignatureOf( material, s_signature );
	auto at = s_cache.find( material );
	if ( at != s_cache.end() && at->second.signature == s_signature )
		return at->second;
	if ( at == s_cache.end() )
	{
		if ( s_cache.size() >= 512 ) // bounded: a reused address rebuilds anyway
			s_cache.clear();
		at = s_cache.emplace( material, MaterialVariables{} ).first;
	}
	static std::uint64_t s_revision = 0;
	at->second.signature = s_signature;
	at->second.revision = ++s_revision;
	BuildVariables( material, at->second );
	return at->second;
}

} // namespace

// The range's triangles for the render core: counterclockwise (legacy
// meshes face clockwise) and 16-bit (meshes are at most 65535 vertices, and
// the PICA200 reads 16-bit indices as they are). A static mesh's come from its
// cache (CoreCache); else they are built into `triangles`. Null for a range
// with no valid triangle.
const std::vector<std::uint16_t> *CEmptyMesh::CoreTriangleList( const CEmptyMesh &src, int firstIndex,
	int indexCount, bool cacheable, std::vector<std::uint16_t> &triangles )
{
	// Triangles, counterclockwise for the core (legacy meshes face clockwise).
	// 16-bit: the mesh is at most 65535 vertices (checked above), and the
	// PICA200 reads 16-bit indices as they are (CoreMeshDraw::indices16).
	const std::vector<std::uint16_t> *drawTriangles = nullptr;
	if ( cacheable )
	{
		CoreCache &cache = *m_pCoreCache;
		if ( cache.triangleRevision != m_nContentRevision )
		{
			cache.triangles.clear();
			cache.triangleRevision = m_nContentRevision;
			cache.Account();
		}
		for ( std::size_t i = 0; i < cache.triangles.size(); ++i )
		{
			CoreTriangles &t = cache.triangles[i];
			if ( t.first != firstIndex || t.count != indexCount )
				continue;
			if ( t.source == &src && t.sourceRevision == src.m_nContentRevision )
				drawTriangles = &t.indices;
			else
			{
				cache.triangles.erase( cache.triangles.begin() + i );
				cache.Account();
			}
			break;
		}
	}
	if ( !drawTriangles )
	{
		const bool indexed = m_nIndices > 0 && indexCount > 0;
		const int count = indexed ? ( firstIndex + indexCount <= m_nIndices ? indexCount : 0 ) : src.m_nVertices;
		auto element = [&]( int i ) { return indexed ? int( m_pIndices[firstIndex + i] ) : i; };
		triangles.reserve( size_t( count > 0 ? count : 0 ) * 3 );
		bool valid = count > 0;
		auto triangle = [&]( int a, int b, int c )
		{
			if ( a < 0 || b < 0 || c < 0 || a >= src.m_nVertices || b >= src.m_nVertices || c >= src.m_nVertices )
			{
				valid = false;
				return;
			}
			if ( a != b && b != c && a != c )
			{
				triangles.push_back( std::uint16_t( a ) );
				triangles.push_back( std::uint16_t( c ) );
				triangles.push_back( std::uint16_t( b ) );
			}
		};
		if ( m_Type == MATERIAL_TRIANGLES )
		{
			if ( count % 3 )
				return nullptr;
			for ( int i = 0; i < count; i += 3 )
				triangle( element( i ), element( i + 1 ), element( i + 2 ) );
		}
		else if ( m_Type == MATERIAL_TRIANGLE_STRIP )
		{
			for ( int i = 0; i + 2 < count; ++i )
				triangle( element( i + ( i & 1 ) ), element( i + 1 - ( i & 1 ) ), element( i + 2 ) );
		}
		else if ( m_Type == MATERIAL_QUADS || m_Type == MATERIAL_INSTANCED_QUADS )
		{
			// Two triangles a quad (VGUI's and the sprites' meshes).
			for ( int i = 0; i + 3 < count; i += 4 )
			{
				triangle( element( i ), element( i + 1 ), element( i + 2 ) );
				triangle( element( i ), element( i + 2 ), element( i + 3 ) );
			}
		}
		else if ( m_Type == MATERIAL_POLYGON )
		{
			for ( int i = 1; i + 1 < count; ++i )
				triangle( element( 0 ), element( i ), element( i + 1 ) );
		}
		else
			return nullptr;
		if ( !valid || triangles.empty() )
			return nullptr;
		drawTriangles = &triangles;
		if ( cacheable && CoreCacheRoom( triangles.size() * sizeof( std::uint16_t ) ) )
		{
			triangles.shrink_to_fit();
			m_pCoreCache->triangles.push_back(
				CoreTriangles{ firstIndex, indexCount, &src, src.m_nContentRevision, std::move( triangles ) } );
			m_pCoreCache->Account();
			drawTriangles = &m_pCoreCache->triangles.back().indices;
		}
	}
	return drawTriangles;
}

// The palette slots of this mesh's skinned vertices (m_pSkinSlots): each
// bone a vertex weighs, in first-use order, gets a slot of the GPU-skinning
// palette (at most kMaxReducedBones); a bone without weight or out of range
// reads slot 0 with its (zero or rounding) weight, as EmitToCore's built
// vertices do. Built once per content revision; false when the bones do not
// fit the palette or there is no memory.
bool CEmptyMesh::SkinSlots()
{
	if ( m_nSkinSlotsRevision == m_nContentRevision )
		return m_nSkinBones > 0;
	m_nSkinSlotsRevision = m_nContentRevision;
	m_nSkinBones = 0;
	if ( !m_pBoneWeights || !m_pBoneIndices || m_nVertices <= 0 || m_nVertices > m_nBoneCapacity )
		return false;
	if ( !m_pSkinSlots )
		m_pSkinSlots = static_cast<unsigned char *>(
			AllocStorage( pica::Memory::kVertices, size_t( m_nBoneCapacity ) * 4 ) );
	if ( !m_pSkinSlots )
		return false;
	pica::PrepareWrite( m_pSkinSlots );
	constexpr int kPalette = int( render::material::kMaxReducedBones );
	signed char slotOf[kMaxBones];
	memset( slotOf, -1, sizeof( slotOf ) );
	int count = 0;
	for ( int i = 0; i < m_nVertices; ++i )
	{
		const float *w = m_pBoneWeights + i * 2;
		const unsigned char *b = m_pBoneIndices + i * 4;
		const float weights[3] = { w[0], w[1], 1.0f - w[0] - w[1] };
		unsigned char *out = m_pSkinSlots + i * 4;
		for ( int k = 0; k < 3; ++k )
		{
			int slot = 0;
			if ( weights[k] > 0.0f && b[k] < kMaxBones )
			{
				if ( slotOf[b[k]] < 0 )
				{
					if ( count == kPalette )
						return false;
					slotOf[b[k]] = (signed char)count;
					m_SkinBones[count++] = b[k];
				}
				slot = slotOf[b[k]];
			}
			out[k] = (unsigned char)( slot * 3 );
		}
		out[3] = 0;
	}
	pica::FlushLinear( m_pSkinSlots, size_t( m_nVertices ) * 4 );
	m_nSkinBones = count;
	return count > 0;
}

// Source's model lighting at the draw (studiorender's SetAmbientLightCube
// and SetLight) for the core's mesh point.
static void FillModelLighting( render::legacy::CoreMeshDraw &draw )
{
	draw.modelLighting = true;
	memcpy( draw.ambientCube, g_AmbientCube, sizeof( draw.ambientCube ) );
	for ( int i = 0; i < kPicaMaxLights && draw.lightCount < render::material::kMaxModelLights; ++i )
	{
		const LightDesc_t &desc = g_Lights[i];
		if ( desc.m_Type == MATERIAL_LIGHT_DISABLE )
			continue;
		render::material::ModelLightDesc &light = draw.lights[draw.lightCount++];
		light.type = desc.m_Type == MATERIAL_LIGHT_SPOT ? render::material::ModelLightType::kSpot
			: desc.m_Type == MATERIAL_LIGHT_DIRECTIONAL ? render::material::ModelLightType::kDirectional
			: render::material::ModelLightType::kPoint;
		light.color[0] = desc.m_Color.x;
		light.color[1] = desc.m_Color.y;
		light.color[2] = desc.m_Color.z;
		light.position[0] = desc.m_Position.x;
		light.position[1] = desc.m_Position.y;
		light.position[2] = desc.m_Position.z;
		light.direction[0] = desc.m_Direction.x;
		light.direction[1] = desc.m_Direction.y;
		light.direction[2] = desc.m_Direction.z;
		light.attenuation[0] = desc.m_Attenuation0;
		light.attenuation[1] = desc.m_Attenuation1;
		light.attenuation[2] = desc.m_Attenuation2;
		light.theta = desc.m_Theta;
		light.phi = desc.m_Phi;
		light.falloff = desc.m_Falloff;
	}
}

// A core draw's view (RFC 0026): the material system's matrices as the
// core's row-major, column-vector ones (FamilyDrawConstants: the transposes
// of the stacks' row-vector matrices) and the current viewport.
static void FillCoreView( render::legacy::CoreMeshDraw &draw, const float *model )
{
	float viewProj[16];
	Mul( Top( kStackView ), Top( kStackProjection ), viewProj );
	const float *view = Top( kStackView );
	const float *projection = Top( kStackProjection );
	for ( int row = 0; row < 4; ++row )
		for ( int col = 0; col < 4; ++col )
		{
			draw.toClip[row * 4 + col] = viewProj[col * 4 + row];
			draw.worldToView[row * 4 + col] = view[col * 4 + row];
			draw.viewToClip[row * 4 + col] = projection[col * 4 + row];
			draw.modelToWorld[row * 4 + col] = model[col * 4 + row];
		}
	int vx = 0, vy = 0, vw = pica::kScreenWidth, vh = pica::kScreenHeight;
	ShaderViewport_t viewport;
	g_ShaderAPIEmpty.GetViewports( &viewport, 1 );
	if ( viewport.m_nWidth > 0 && viewport.m_nHeight > 0 )
	{
		vx = viewport.m_nTopLeftX;
		vy = viewport.m_nTopLeftY;
		vw = viewport.m_nWidth;
		vh = viewport.m_nHeight;
	}
	draw.viewport = { float( vx ), float( vy ), float( vw ), float( vh ), 0.0f, 1.0f };
}

// Queues a core draw at this point of the stream (its slot). The core holds
// each draw's geometry until the recording that reads it is submitted: past
// 1.5 MB or 64 draws, submit and go on, so a frame of draws never holds the
// 3DS's memory at once. False when the core refused it.
static bool QueueCore( const render::legacy::CoreMeshDraw &draw, std::size_t geometryBytes )
{
	const std::uint32_t tag = g_CorePassRecorder->QueueMesh( draw );
	if ( !tag )
		return false;
	g_PicaCorePassSlots.MarkSlot( tag );
	// Each queued draw is also a queued view of the core's (tens of KB of
	// heap until recorded): a frame of many draws flushes by count too.
	static std::size_t s_pendingBytes = 0;
	static unsigned s_pendingDraws = 0;
	s_pendingBytes += geometryBytes;
	if ( s_pendingBytes > ( 3u << 19 ) || ++s_pendingDraws >= 64 )
	{
		pica::FlushRecording();
		s_pendingBytes = 0;
		s_pendingDraws = 0;
	}
	return true;
}

bool CEmptyMesh::EmitToCore( int firstIndex, int indexCount )
{
	static unsigned s_reasons = 0;
	auto skip = [&]( unsigned bit, const char *why )
	{
		if ( !( s_reasons & bit ) )
		{
			s_reasons |= bit;
			printf( "pica: a model draw stays legacy: %s (%s)\n", why,
				g_pBoundMaterial ? g_pBoundMaterial->GetShaderName() : "no material" );
		}
		return false;
	};
	if ( !g_CorePassRecorder || !g_pBoundMaterial ||
		V_stricmp( g_pBoundMaterial->GetShaderName(), "VertexLitGeneric" ) )
		return false;
	if ( !g_CorePassRecorder->AcceptsMeshes() )
		return skip( 1, "the core accepts no meshes" );
	if ( m_bHasColorMesh )
		return false; // a static prop: baked lighting, not the model point's
	const CEmptyMesh &src = m_pVertexSource ? *m_pVertexSource : *this;
	if ( !src.m_pVertices || !src.m_pNormals || src.m_nVertices <= 0 ||
		src.m_nVertices > src.m_nNormalCapacity )
		return skip( 2, "the mesh has no normals" );
	// The memory audit: the PICA's indices are 16-bit, so a larger mesh is
	// not a model draw this backend can hand over.
	static int s_largest = 0;
	if ( src.m_nVertices > 0xFFFF || indexCount > 3 * 0xFFFF )
		return skip( 8, "the mesh is larger than 16-bit indices address" );
	if ( src.m_nVertices > s_largest )
	{
		s_largest = src.m_nVertices;
		if ( s_largest > 4096 )
			printf( "pica: core model draw of %d vertices, %d indices (%s)\n", src.m_nVertices,
				indexCount, g_pBoundMaterial->GetName() );
	}
	// A static mesh keeps the triangles this builds per range (CoreCache).
	const bool cacheable = !m_bIsDynamic && !src.m_bIsDynamic;
	if ( cacheable && !m_pCoreCache )
		m_pCoreCache = std::make_unique<CoreCache>();

	std::vector<std::uint16_t> triangles;
	const std::vector<std::uint16_t> *drawTriangles =
		CoreTriangleList( src, firstIndex, indexCount, cacheable, triangles );
	if ( !drawTriangles )
		return false;

	// World-space positions and normals: skinned by the bones, else by the model.
	const bool skinned = src.m_pBoneWeights && g_MaxBone > 0;
	const float *model = Top( kStackModel );
	// Built once per vertex (each a complete value): default-constructing the
	// array first wrote every vertex twice (a measured memset).
	std::vector<render::material::SurfaceWorldVertex> vertices;

	// The GPU skins (surface_lit_skinned.v.pica) when the draw's bones fit its
	// palette: the vertices stay in bone space and the palette holds each
	// bone's bone-to-world rows; an unskinned mesh is one "bone", its model
	// matrix. The CPU loop below is the fallback (more bones) and the A/B
	// reference (-pica_cpu_skinning).
	static const bool s_CpuSkinning = CommandLine()->FindParm( "-pica_cpu_skinning" ) != 0;
	constexpr int kPalette = int( render::material::kMaxReducedBones );
	float palette[kPalette * 12];
	int paletteCount = 0;
	signed char slotOf[kMaxBones];
	bool gpu = !s_CpuSkinning;
	// The vertices where the mesh keeps them (CoreMeshStreams): nothing is
	// built or copied per draw when the core reads streams and every stream
	// is in the mesh's in-place memory (AllocStorage); a skinned mesh adds its
	// weights and palette slots (SkinSlots), its palette the slots' bones.
	static_assert( sizeof( pica::Vertex ) == 24 && offsetof( pica::Vertex, color ) == 12 &&
		offsetof( pica::Vertex, uv ) == 16, "the record CoreMeshStreams names" );
	render::legacy::CoreMeshStreams streams;
	// -pica_no_mesh_streams: build every draw's vertices (the A/B reference).
	static const bool s_NoMeshStreams = CommandLine()->FindParm( "-pica_no_mesh_streams" ) != 0;
	bool inPlace = gpu && cacheable && !s_NoMeshStreams && g_CorePassRecorder->AcceptsMeshStreams() &&
		pica::DeviceBufferOf( src.m_pVertices, streams.record, streams.recordOffset ) &&
		pica::DeviceBufferOf( src.m_pNormals, streams.normals, streams.normalOffset );
	if ( inPlace && skinned )
	{
		CEmptyMesh &source = const_cast<CEmptyMesh &>( src );
		inPlace = source.SkinSlots() &&
			pica::DeviceBufferOf( src.m_pBoneWeights, streams.weights, streams.weightOffset ) &&
			pica::DeviceBufferOf( src.m_pSkinSlots, streams.slots, streams.slotOffset );
		if ( inPlace )
		{
			paletteCount = src.m_nSkinBones;
			for ( int slot = 0; slot < paletteCount; ++slot )
				memcpy( palette + slot * 12, g_Bones[src.m_SkinBones[slot]], 12 * sizeof( float ) );
		}
	}
	if ( gpu && skinned && !inPlace )
	{
		memset( slotOf, -1, sizeof( slotOf ) );
		for ( int i = 0; i < src.m_nVertices && gpu; ++i )
		{
			const float *w = src.m_pBoneWeights + i * 2;
			const unsigned char *b = src.m_pBoneIndices + i * 4;
			const float weights[3] = { w[0], w[1], 1.0f - w[0] - w[1] };
			for ( int k = 0; k < 3; ++k )
			{
				if ( weights[k] <= 0.0f || b[k] >= kMaxBones || slotOf[b[k]] >= 0 )
					continue;
				if ( paletteCount == kPalette )
				{
					gpu = false;
					break;
				}
				slotOf[b[k]] = (signed char)paletteCount;
				memcpy( palette + paletteCount * 12, g_Bones[b[k]], 12 * sizeof( float ) );
				++paletteCount;
			}
		}
	}
	else if ( gpu && !skinned )
	{
		// The model matrix (row vectors, D3D) as one bone's three rows.
		for ( int r = 0; r < 3; ++r )
		{
			palette[r * 4 + 0] = model[0 + r];
			palette[r * 4 + 1] = model[4 + r];
			palette[r * 4 + 2] = model[8 + r];
			palette[r * 4 + 3] = model[12 + r];
		}
		paletteCount = 1;
	}
	if ( inPlace )
		;
	else if ( gpu && paletteCount > 0 )
	{
		vertices.reserve( src.m_nVertices );
		for ( int i = 0; i < src.m_nVertices; ++i )
		{
			const pica::Vertex &in = src.m_pVertices[i];
			const float *n = src.m_pNormals + i * 3;
			float weight0 = 1.0f, weight1 = 0.0f;
			float offsets[3] = { 0.0f, 0.0f, 0.0f };
			if ( skinned )
			{
				const float *w = src.m_pBoneWeights + i * 2;
				const unsigned char *b = src.m_pBoneIndices + i * 4;
				const float weights[3] = { w[0], w[1], 1.0f - w[0] - w[1] };
				// A bone the CPU path would skip (no weight, out of range)
				// reads slot 0 with its (zero or rounding) weight.
				for ( int k = 0; k < 3; ++k )
					offsets[k] = float( ( weights[k] > 0.0f && b[k] < kMaxBones ? slotOf[b[k]] : 0 ) * 3 );
				weight0 = w[0];
				weight1 = w[1];
			}
			// Every member given (tangentT and lightmapOffset unread here).
			vertices.push_back( render::material::SurfaceWorldVertex{
				{ in.pos[0], in.pos[1], in.pos[2] }, { in.uv[0], in.uv[1] }, { weight0, weight1 },
				{ in.color[2], in.color[1], in.color[0], in.color[3] }, { n[0], n[1], n[2] },
				{ offsets[0], offsets[1], offsets[2] }, { 0.0f, 1.0f, 0.0f }, 0.0f } );
		}
	}
	else
		vertices.resize( src.m_nVertices );
	for ( int i = 0; !( gpu && paletteCount > 0 ) && i < src.m_nVertices; ++i )
	{
		const pica::Vertex &in = src.m_pVertices[i];
		const float *n = src.m_pNormals + i * 3;
		render::material::SurfaceWorldVertex &out = vertices[i];
		float pos[3] = { 0, 0, 0 }, normal[3] = { 0, 0, 0 };
		if ( skinned )
		{
			const float *w = src.m_pBoneWeights + i * 2;
			const unsigned char *b = src.m_pBoneIndices + i * 4;
			const float weights[3] = { w[0], w[1], 1.0f - w[0] - w[1] };
			for ( int k = 0; k < 3; ++k )
			{
				if ( weights[k] <= 0.0f || b[k] >= kMaxBones )
					continue;
				const float *m = g_Bones[b[k]];
				for ( int r = 0; r < 3; ++r )
				{
					pos[r] += weights[k] * ( m[r * 4] * in.pos[0] + m[r * 4 + 1] * in.pos[1] + m[r * 4 + 2] * in.pos[2] + m[r * 4 + 3] );
					normal[r] += weights[k] * ( m[r * 4] * n[0] + m[r * 4 + 1] * n[1] + m[r * 4 + 2] * n[2] );
				}
			}
		}
		else
		{
			for ( int j = 0; j < 3; ++j )
			{
				pos[j] = in.pos[0] * model[j] + in.pos[1] * model[4 + j] + in.pos[2] * model[8 + j] + model[12 + j];
				normal[j] = n[0] * model[j] + n[1] * model[4 + j] + n[2] * model[8 + j];
			}
		}
		const float length = sqrtf( normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] );
		for ( int j = 0; j < 3; ++j )
		{
			out.position[j] = pos[j];
			out.normal[j] = length > 0.0f ? normal[j] / length : ( j == 2 ? 1.0f : 0.0f );
		}
		out.uv[0] = in.uv[0];
		out.uv[1] = in.uv[1];
		out.color[0] = in.color[2];
		out.color[1] = in.color[1];
		out.color[2] = in.color[0];
		out.color[3] = in.color[3];
	}

	// The material's variables, as the core's claim reads them. GetStringValue
	// formats float and vector values with snprintf, which over dozens of
	// parameters per draw was a tenth of the frame (tools/n3ds/guest_profile.py);
	// so each material keeps its text form, rebuilt only when a raw signature
	// of its values (types, bits, texture and string pointers, flags) changes.
	const MaterialVariables &material = VariablesFor( g_pBoundMaterial );
	const std::vector<render::legacy::CoreMeshVariable> &variables = material.variables;

	render::legacy::CoreMeshDraw draw;
	draw.kind = render::legacy::CoreMeshKind::kModelSurface;
	draw.name = g_pBoundMaterial->GetName();
	draw.shader = g_pBoundMaterial->GetShaderName();
	draw.variables = variables.data();
	draw.variableCount = std::uint32_t( variables.size() );
	draw.materialRevision = material.revision;
	if ( gpu && paletteCount > 0 )
	{
		draw.bonePalette = palette;
		draw.boneCount = std::uint32_t( paletteCount );
	}
	if ( inPlace )
	{
		draw.streams = &streams;
		draw.vertexCount = std::uint32_t( src.m_nVertices );
	}
	else
	{
		draw.vertices = vertices.data();
		draw.vertexCount = std::uint32_t( vertices.size() );
	}
	draw.indices16 = drawTriangles->data();
	draw.indexCount = std::uint32_t( drawTriangles->size() );
	draw.mesh = true;
	FillModelLighting( draw );
	FillCoreView( draw, model );
	// The core takes the arrays (no copy); their sizes are counted first.
	// Cached triangles are lent (the core copies them); built ones are taken.
	const std::size_t geometryBytes = vertices.size() * sizeof( render::material::SurfaceWorldVertex ) +
		drawTriangles->size() * sizeof( std::uint16_t );
	if ( !inPlace )
		draw.takeVertices = &vertices;
	if ( drawTriangles == &triangles )
		draw.takeIndices16 = &triangles;
	if ( DecorateCoreDraw( draw ) )
		return true;
	if ( !QueueCore( draw, geometryBytes ) )
		return skip( 4, "QueueMesh refused it" );
	static unsigned s_taken = 0;
	if ( ++s_taken % 50 == 0 && s_taken <= 2000 )
	{
		const struct mallinfo heap = mallinfo();
		printf( "pica: %u core model draws, heap used %u KB, frame %d\n", s_taken,
			(unsigned)( heap.uordblks / 1024 ), g_PicaFrame );
	}
	return true;
}

bool CEmptyMesh::EmitSurfaceToCore( int firstIndex, int indexCount, render::legacy::CoreMeshKind kind )
{
	using render::legacy::CoreMeshKind;
	if ( !g_CorePassRecorder || !g_pBoundMaterial || !g_CorePassRecorder->AcceptsMeshes() )
		return false;
	const CEmptyMesh &src = m_pVertexSource ? *m_pVertexSource : *this;
	if ( !src.m_pVertices || src.m_nVertices <= 0 || src.m_nVertices > 0xFFFF || indexCount > 3 * 0xFFFF )
		return false;
	const bool cacheable = !m_bIsDynamic && !src.m_bIsDynamic;
	if ( cacheable && !m_pCoreCache )
		m_pCoreCache = std::make_unique<CoreCache>();
	std::vector<std::uint16_t> triangles;
	const std::vector<std::uint16_t> *drawTriangles =
		CoreTriangleList( src, firstIndex, indexCount, cacheable, triangles );
	if ( !drawTriangles )
		return false;

	// Only the vertices the range uses (a world mesh's shared source holds up
	// to 65535 of them): [first, last], the triangles rebased onto it.
	int first = 0xFFFF, last = 0;
	for ( std::uint16_t index : *drawTriangles )
	{
		first = std::min<int>( first, index );
		last = std::max<int>( last, index );
	}
	if ( first > 0 )
	{
		if ( drawTriangles != &triangles )
			triangles = *drawTriangles;
		for ( std::uint16_t &index : triangles )
			index = std::uint16_t( index - first );
		drawTriangles = &triangles;
	}

	// World space, as the core's dynamic draws are: skinned by the bones,
	// else by the model matrix (row vectors, D3D).
	const float *model = Top( kStackModel );
	const bool skinned = src.m_pBoneWeights && g_MaxBone > 0;
	const bool normals = src.m_pNormals && src.m_nVertices <= src.m_nNormalCapacity;
	const bool lightmapUv = src.m_pTexCoord1 && src.m_nVertices <= src.m_nTexCoord1Capacity;
	const int lightmapStride = lightmapUv ? TexCoordSize( 1, src.m_Format ) : 0;
	const CEmptyMesh *colors = m_pColorMesh && m_pColorMesh->m_pVertices &&
			m_nColorMeshOffset + src.m_nVertices <= m_pColorMesh->m_nVertices
		? m_pColorMesh
		: nullptr;
	std::vector<render::material::SurfaceWorldVertex> vertices;
	vertices.reserve( last - first + 1 );
	for ( int i = first; i <= last; ++i )
	{
		const pica::Vertex &in = src.m_pVertices[i];
		const float zero[3] = { 0.0f, 0.0f, 1.0f };
		const float *n = normals ? src.m_pNormals + i * 3 : zero;
		float pos[3] = { 0, 0, 0 }, normal[3] = { 0, 0, 0 };
		if ( skinned )
		{
			const float *w = src.m_pBoneWeights + i * 2;
			const unsigned char *b = src.m_pBoneIndices + i * 4;
			const float weights[3] = { w[0], w[1], 1.0f - w[0] - w[1] };
			for ( int k = 0; k < 3; ++k )
			{
				if ( weights[k] <= 0.0f || b[k] >= kMaxBones )
					continue;
				const float *m = g_Bones[b[k]];
				for ( int r = 0; r < 3; ++r )
				{
					pos[r] += weights[k] * ( m[r * 4] * in.pos[0] + m[r * 4 + 1] * in.pos[1] + m[r * 4 + 2] * in.pos[2] + m[r * 4 + 3] );
					normal[r] += weights[k] * ( m[r * 4] * n[0] + m[r * 4 + 1] * n[1] + m[r * 4 + 2] * n[2] );
				}
			}
		}
		else
		{
			for ( int j = 0; j < 3; ++j )
			{
				pos[j] = in.pos[0] * model[j] + in.pos[1] * model[4 + j] + in.pos[2] * model[8 + j] + model[12 + j];
				normal[j] = n[0] * model[j] + n[1] * model[4 + j] + n[2] * model[8 + j];
			}
		}
		const float length = sqrtf( normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] );
		const std::uint8_t *c = colors ? colors->m_pVertices[m_nColorMeshOffset + i].color : in.color;
		render::material::SurfaceWorldVertex out;
		for ( int j = 0; j < 3; ++j )
		{
			out.position[j] = pos[j];
			out.normal[j] = length > 0.0f ? normal[j] / length : ( j == 2 ? 1.0f : 0.0f );
		}
		out.uv[0] = in.uv[0];
		out.uv[1] = in.uv[1];
		if ( lightmapUv )
		{
			out.lightmapUv[0] = src.m_pTexCoord1[i * lightmapStride];
			out.lightmapUv[1] = src.m_pTexCoord1[i * lightmapStride + 1];
		}
		// The record's colour is B G R A.
		out.color[0] = c[2];
		out.color[1] = c[1];
		out.color[2] = c[0];
		out.color[3] = c[3];
		vertices.push_back( out );
	}

	const MaterialVariables &material = VariablesFor( g_pBoundMaterial );
	render::legacy::CoreMeshDraw draw;
	draw.kind = kind;
	draw.name = g_pBoundMaterial->GetName();
	draw.shader = g_pBoundMaterial->GetShaderName();
	draw.variables = material.variables.data();
	draw.variableCount = std::uint32_t( material.variables.size() );
	draw.materialRevision = material.revision;
	draw.vertices = vertices.data();
	draw.vertexCount = std::uint32_t( vertices.size() );
	draw.indices16 = drawTriangles->data();
	draw.indexCount = std::uint32_t( drawTriangles->size() );
	// The model and refraction points are the mesh points (CoreMeshDraw::mesh);
	// a model surface reads Source's model lighting or its baked colours.
	const bool modelSurface = kind == CoreMeshKind::kModelSurface;
	draw.mesh = modelSurface || kind == CoreMeshKind::kTransmission;
	draw.staticVertexLighting = modelSurface && colors;
	if ( modelSurface && !colors )
		FillModelLighting( draw );
	if ( g_BoundLightmap != INVALID_SHADERAPI_TEXTURE_HANDLE && lightmapUv )
	{
		draw.lightmapPage = int( g_BoundLightmap );
		draw.capturedLightmap = true;
	}
	static const float kIdentity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	FillCoreView( draw, kIdentity );
	const std::size_t geometryBytes = vertices.size() * sizeof( vertices[0] ) +
		drawTriangles->size() * sizeof( std::uint16_t );
	draw.takeVertices = &vertices;
	if ( drawTriangles == &triangles )
		draw.takeIndices16 = &triangles;
	if ( DecorateCoreDraw( draw ) )
		return true; // consumed (core_draw_hooks)
	if ( QueueCore( draw, geometryBytes ) )
		return true;
	// Each refused shader named once.
	static std::unordered_map<std::string, bool> s_refused;
	std::string key = std::string( draw.shader ) + " " + draw.name;
	if ( !s_refused[key] )
	{
		s_refused[key] = true;
		printf( "pica: the core refused a %s draw (%s)\n", draw.shader, draw.name );
	}
	return false;
}

void CEmptyMesh::DrawRange( int firstIndex, int indexCount )
{
	const CEmptyMesh &src = m_pVertexSource ? *m_pVertexSource : *this;
	// The memory audit: a count past a buffer would read neighbouring memory
	// as geometry; such a draw is refused and counted.
	if ( src.m_nVertices > src.m_nVertexCapacity || m_nIndices > m_nIndexCapacity )
	{
		++g_Counters.overrunDraws;
		return;
	}
	// Every draw goes to its render core point (core_mesh_kind.h): model
	// surfaces through EmitToCore (the frontend's buffers read in place),
	// everything else with world-space vertices.
	// The error material stands in for a material or model the content lacks:
	// nothing draws it (named once per shader API).
	if ( g_pBoundMaterial && g_pBoundMaterial->IsErrorMaterial() )
	{
		static bool s_said = false;
		if ( !s_said )
		{
			s_said = true;
			printf( "pica: draws with the error material (missing content) are dropped\n" );
		}
		++g_Counters.errorMaterialDraws;
		return;
	}
	const render::legacy::CoreMeshKind kind = g_pBoundMaterial
		? render::legacy::CoreMeshKindFor( g_pBoundMaterial )
		: render::legacy::CoreMeshKind::kSurface;
#if defined( PLATFORM_3DS )
	// The reduced model reads model meshes in place and skins them on the GPU.
	const bool inPlace = kind == render::legacy::CoreMeshKind::kModelSurface;
#else
	// The full model has neither variant: every mesh goes with world-space
	// vertices, skinned here.
	const bool inPlace = false;
#endif
	if ( ( inPlace && EmitToCore( firstIndex, indexCount ) ) ||
		EmitSurfaceToCore( firstIndex, indexCount, kind ) )
	{
		++g_Counters.coreMeshDraws;
		return;
	}
	// The render core draws everything (RFC 0026): a draw it refuses (named
	// once by EmitSurfaceToCore) is dropped and counted, never drawn another way.
	++g_Counters.refusedDraws;
}


// Copy verts and/or indices to a mesh builder. This only works for temp meshes!
void CEmptyMesh::CopyToMeshBuilder( 
	int iStartVert,		// Which vertices to copy.
	int nVerts, 
	int iStartIndex,	// Which indices to copy.
	int nIndices, 
	int indexOffset,	// This is added to each index.
	CMeshBuilder &builder )
{
}

// Spews the mesh data
void CEmptyMesh::Spew( int numVerts, int numIndices, const MeshDesc_t & desc )
{
}

void CEmptyMesh::ValidateData( int numVerts, int numIndices, const MeshDesc_t & desc )
{
}

// gets the associated material
IMaterial* CEmptyMesh::GetMaterial()
{
	// umm. this don't work none
	Assert(0);
	return 0;
}

//-----------------------------------------------------------------------------
// The shader shadow interface
//-----------------------------------------------------------------------------
CShaderShadowEmpty::CShaderShadowEmpty()
{
	m_IsTranslucent = false;
	m_IsAlphaTested = false;
	m_bIsDepthWriteEnabled = true;
	m_bUsesVertexAndPixelShaders = false;
}

CShaderShadowEmpty::~CShaderShadowEmpty()
{
}

// Sets the default *shadow* state
void CShaderShadowEmpty::SetDefaultState()
{
	m_IsTranslucent = false;
	m_IsAlphaTested = false;
	m_bIsDepthWriteEnabled = true;
	m_bUsesVertexAndPixelShaders = false;
	m_State = pica::DrawState();
	m_VertexFormat = 0;
	m_PolyOffset = SHADER_POLYOFFSET_DISABLE;
}

// Methods related to depth buffering
void CShaderShadowEmpty::DepthFunc( ShaderDepthFunc_t depthFunc )
{
	m_State.depthFunc = DepthCompare( depthFunc );
}

void CShaderShadowEmpty::EnableDepthWrites( bool bEnable )
{
	m_bIsDepthWriteEnabled = bEnable;
	m_State.depthWrite = bEnable;
}

void CShaderShadowEmpty::EnableDepthTest( bool bEnable )
{
	m_State.depthTest = bEnable;
}

void CShaderShadowEmpty::EnablePolyOffset( PolygonOffsetMode_t nOffsetMode )
{
	m_PolyOffset = nOffsetMode;
}

// Suppresses/activates color writing 
void CShaderShadowEmpty::EnableColorWrites( bool bEnable )
{
	m_State.colorWrite = bEnable;
}

// Suppresses/activates alpha writing 
void CShaderShadowEmpty::EnableAlphaWrites( bool bEnable )
{
	m_State.alphaWrite = bEnable;
}

// Methods related to alpha blending
void CShaderShadowEmpty::EnableBlending( bool bEnable )
{
	m_IsTranslucent = bEnable;
	m_State.blend = bEnable;
}

void CShaderShadowEmpty::BlendFunc( ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor )
{
	m_State.src = BlendFactor( srcFactor );
	m_State.dst = BlendFactor( dstFactor );
}

// A simpler method of dealing with alpha modulation
void CShaderShadowEmpty::EnableAlphaPipe( bool bEnable )
{
}

void CShaderShadowEmpty::EnableConstantAlpha( bool bEnable )
{
}

void CShaderShadowEmpty::EnableVertexAlpha( bool bEnable )
{
}

void CShaderShadowEmpty::EnableTextureAlpha( TextureStage_t stage, bool bEnable )
{
}


// Alpha testing
void CShaderShadowEmpty::EnableAlphaTest( bool bEnable )
{
	m_IsAlphaTested = bEnable;
	m_State.alphaTest = bEnable;
}

void CShaderShadowEmpty::AlphaFunc( ShaderAlphaFunc_t alphaFunc, float alphaRef /* [0-1] */ )
{
	m_State.alphaFunc = AlphaCompare( alphaFunc );
	const float ref = alphaRef * 255.0f + 0.5f;
	m_State.alphaRef = (unsigned char)( ref < 0.0f ? 0.0f : ( ref > 255.0f ? 255.0f : ref ) );
}


// Wireframe/filled polygons
void CShaderShadowEmpty::PolyMode( ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode )
{
}


// Back face culling
void CShaderShadowEmpty::EnableCulling( bool bEnable )
{
	m_State.cull = bEnable;
}


// Alpha to coverage
void CShaderShadowEmpty::EnableAlphaToCoverage( bool bEnable )
{
}


// constant color + transparency
void CShaderShadowEmpty::EnableConstantColor( bool bEnable )
{
}

// Indicates the vertex format for use with a vertex shader
// The flags to pass in here come from the VertexFormatFlags_t enum
// If pTexCoordDimensions is *not* specified, we assume all coordinates
// are 2-dimensional
void CShaderShadowEmpty::VertexShaderVertexFormat( unsigned int nFlags, 
												   int nTexCoordCount,
												   int* pTexCoordDimensions,
												   int nUserDataSize )
{
	VertexFormat_t format = nFlags;
	for ( int i = 0; i < nTexCoordCount && i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
		format |= VERTEX_TEXCOORD_SIZE( i, pTexCoordDimensions ? pTexCoordDimensions[i] : 2 );
	m_VertexFormat |= format;
}

// Indicates we're going to light the model
void CShaderShadowEmpty::EnableLighting( bool bEnable )
{
}

void CShaderShadowEmpty::EnableSpecular( bool bEnable )
{
}

// Activate/deactivate skinning
void CShaderShadowEmpty::EnableVertexBlend( bool bEnable )
{
}

// per texture unit stuff
void CShaderShadowEmpty::OverbrightValue( TextureStage_t stage, float value )
{
}

void CShaderShadowEmpty::EnableTexture( Sampler_t stage, bool bEnable )
{
}

void CShaderShadowEmpty::EnableCustomPixelPipe( bool bEnable )
{
}

void CShaderShadowEmpty::CustomTextureStages( int stageCount )
{
}

void CShaderShadowEmpty::CustomTextureOperation( TextureStage_t stage, ShaderTexChannel_t channel, 
	ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2 )
{
}

void CShaderShadowEmpty::EnableTexGen( TextureStage_t stage, bool bEnable )
{
}

void CShaderShadowEmpty::TexGen( TextureStage_t stage, ShaderTexGenParam_t param )
{
}

// Sets the vertex and pixel shaders
void CShaderShadowEmpty::SetVertexShader( const char *pShaderName, int vshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
}

void CShaderShadowEmpty::EnableBlendingSeparateAlpha( bool bEnable )
{
}
void CShaderShadowEmpty::SetPixelShader( const char *pShaderName, int pshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
}

void CShaderShadowEmpty::BlendFuncSeparateAlpha( ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor )
{
}
// indicates what per-vertex data we're providing
void CShaderShadowEmpty::DrawFlags( unsigned int drawFlags )
{
}



//-----------------------------------------------------------------------------
//
// Shader API Empty
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor, destructor
//-----------------------------------------------------------------------------

CShaderAPIEmpty::CShaderAPIEmpty()  : m_Mesh( true )
{
}

CShaderAPIEmpty::~CShaderAPIEmpty()
{
}


bool CShaderAPIEmpty::DoRenderTargetsNeedSeparateDepthBuffer() const
{
	return false;
}

// Can we download textures?
bool CShaderAPIEmpty::CanDownloadTextures() const
{
	// TexImageFromVTF/TexImage2D convert and upload to PICA textures.
	return true;
}

// Used to clear the transition table when we know it's become invalid.
void CShaderAPIEmpty::ClearSnapshots()
{
}

// Members of IMaterialSystemHardwareConfig
bool CShaderAPIEmpty::HasDestAlphaBuffer() const
{
	return false;
}

bool CShaderAPIEmpty::HasStencilBuffer() const
{
	return false;
}

int CShaderAPIEmpty::MaxViewports() const
{
	return 1;
}

int CShaderAPIEmpty::GetShadowFilterMode() const
{
	return 0;
}

int CShaderAPIEmpty::StencilBufferBits() const
{
	return 0;
}

int	 CShaderAPIEmpty::GetFrameBufferColorDepth() const
{
	return 0;
}

int  CShaderAPIEmpty::GetSamplerCount() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 60))
		return 1;
	if (( ShaderUtil()->GetConfig().dxSupportLevel >= 60 ) && ( ShaderUtil()->GetConfig().dxSupportLevel < 80 ))
		return 2;
	return 4;
}

bool CShaderAPIEmpty::HasSetDeviceGammaRamp() const
{
	return false;
}

bool CShaderAPIEmpty::SupportsCompressedTextures() const
{
	return false;
}

VertexCompressionType_t CShaderAPIEmpty::SupportsCompressedVertices() const
{
	return VERTEX_COMPRESSION_NONE;
}

bool CShaderAPIEmpty::SupportsVertexAndPixelShaders() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 80))
		return false;

	return true;
}

bool CShaderAPIEmpty::SupportsPixelShaders_1_4() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 81))
		return false;

	return true;
}

bool CShaderAPIEmpty::SupportsPixelShaders_2_0() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 90))
		return false;

	return true;
}

bool CShaderAPIEmpty::SupportsPixelShaders_2_b() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 90))
		return false;

	return true;
}

bool CShaderAPIEmpty::ActuallySupportsPixelShaders_2_b() const
{
	return true;
}

bool CShaderAPIEmpty::SupportsShaderModel_3_0() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
		(ShaderUtil()->GetConfig().dxSupportLevel < 95))
		return false;

	return true;
}

bool CShaderAPIEmpty::SupportsStaticControlFlow() const
{
	if ( IsOpenGL() )
		return false;

	return SupportsVertexShaders_2_0();
}

bool CShaderAPIEmpty::SupportsVertexShaders_2_0() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 90))
		return false;

	return true;
}

int  CShaderAPIEmpty::MaximumAnisotropicLevel() const
{
	return 0;
}

void CShaderAPIEmpty::SetAnisotropicLevel( int nAnisotropyLevel )
{
}

int  CShaderAPIEmpty::MaxTextureWidth() const
{
	// Should be big enough to cover all cases
	return 16384;
}

int  CShaderAPIEmpty::MaxTextureHeight() const
{
	// Should be big enough to cover all cases
	return 16384;
}

int  CShaderAPIEmpty::MaxTextureAspectRatio() const
{
	// Should be big enough to cover all cases
	return 16384;
}


int	 CShaderAPIEmpty::TextureMemorySize() const
{
	// fake it
	return 64 * 1024 * 1024;
}

int  CShaderAPIEmpty::GetDXSupportLevel() const 
{ 
	return 90; 
}

bool CShaderAPIEmpty::SupportsOverbright() const
{
	return false;
}

bool CShaderAPIEmpty::SupportsCubeMaps() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
		return false;

	return true;
}

bool CShaderAPIEmpty::SupportsNonPow2Textures() const
{
	return true;
}

bool CShaderAPIEmpty::SupportsMipmappedCubemaps() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
		return false;

	return true;
}

int  CShaderAPIEmpty::GetTextureStageCount() const
{
	return 4;
}

int	 CShaderAPIEmpty::NumVertexShaderConstants() const
{
	return 128;
}

int	 CShaderAPIEmpty::NumBooleanVertexShaderConstants() const
{
	return 0;
}

int	 CShaderAPIEmpty::NumIntegerVertexShaderConstants() const
{
	return 0;
}

int	 CShaderAPIEmpty::NumPixelShaderConstants() const
{
	return 8;
}

int	 CShaderAPIEmpty::MaxNumLights() const
{
	return 4;
}

bool CShaderAPIEmpty::SupportsSpheremapping() const
{
	return false;
}


// This is the max dx support level supported by the card
int	CShaderAPIEmpty::GetMaxDXSupportLevel() const
{
	return 90;
}

bool CShaderAPIEmpty::SupportsHardwareLighting() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
		return false;

	return true;
}

int	 CShaderAPIEmpty::MaxBlendMatrices() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
	{
		return 1;
	}

	return 0;
}

int	 CShaderAPIEmpty::MaxBlendMatrixIndices() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
	{
		return 1;
	}

	return 0;
}

int	 CShaderAPIEmpty::MaxVertexShaderBlendMatrices() const
{
	return 0;
}

int	CShaderAPIEmpty::MaxUserClipPlanes() const
{
	return 0;
}

bool CShaderAPIEmpty::SpecifiesFogColorInLinearSpace() const
{
	return false;
}

bool CShaderAPIEmpty::SupportsSRGB() const
{
	return false;
}

bool CShaderAPIEmpty::FakeSRGBWrite() const
{
	return false;
}

bool CShaderAPIEmpty::CanDoSRGBReadFromRTs() const
{
	return true;
}

bool CShaderAPIEmpty::SupportsGLMixedSizeTargets() const
{
	return false;
}

const char *CShaderAPIEmpty::GetHWSpecificShaderDLLName() const
{
	return 0;
}

// Sets the default *dynamic* state
void CShaderAPIEmpty::SetDefaultState()
{
}


// Returns the snapshot id for the shader state
StateSnapshot_t	 CShaderAPIEmpty::TakeSnapshot( )
{
	StateSnapshot_t id = 0;
	if (g_ShaderShadow.m_IsTranslucent)
		id |= TRANSLUCENT;
	if (g_ShaderShadow.m_IsAlphaTested)
		id |= ALPHATESTED;
	if (g_ShaderShadow.m_bUsesVertexAndPixelShaders)
		id |= VERTEX_AND_PIXEL_SHADERS;
	if (g_ShaderShadow.m_bIsDepthWriteEnabled)
		id |= DEPTHWRITE;
	// The recorded state's index above the flag bits.
	// StateSnapshot_t is a short: the index has 11 bits above the flags. Equal
	// states share one record (materials record a handful of distinct states).
	int index = -1;
	for ( int i = 0; i < g_Snapshots.Count() && index < 0; ++i )
		if ( SameSnapshot( g_Snapshots[i], g_ShaderShadow.m_State, g_ShaderShadow.m_VertexFormat,
				 g_ShaderShadow.m_PolyOffset ) )
			index = i;
	if ( index < 0 )
	{
		PicaSnapshot snapshot;
		snapshot.state = g_ShaderShadow.m_State;
		snapshot.format = g_ShaderShadow.m_VertexFormat;
		snapshot.polyOffset = g_ShaderShadow.m_PolyOffset;
		index = g_Snapshots.AddToTail( snapshot );
		if ( index > 0x7FF )
			Warning( "pica: %d distinct state snapshots exceed StateSnapshot_t's 11 index bits\n", index + 1 );
	}
	return id | ( StateSnapshot_t( index ) << 4 );
}

// Returns true if the state snapshot is transparent
bool CShaderAPIEmpty::IsTranslucent( StateSnapshot_t id ) const
{
	return (id & TRANSLUCENT) != 0; 
}

bool CShaderAPIEmpty::IsAlphaTested( StateSnapshot_t id ) const
{
	return (id & ALPHATESTED) != 0; 
}

bool CShaderAPIEmpty::IsDepthWriteEnabled( StateSnapshot_t id ) const
{
	return (id & DEPTHWRITE) != 0; 
}

bool CShaderAPIEmpty::UsesVertexAndPixelShaders( StateSnapshot_t id ) const
{
	return (id & VERTEX_AND_PIXEL_SHADERS) != 0; 
}

// Gets the vertex format for a set of snapshot ids
VertexFormat_t CShaderAPIEmpty::ComputeVertexFormat( int numSnapshots, StateSnapshot_t* pIds ) const
{
	VertexFormat_t format = 0;
	for ( int i = 0; i < numSnapshots; ++i )
	{
		const int index = int( pIds[i] >> 4 );
		if ( index >= 0 && index < g_Snapshots.Count() )
			format |= g_Snapshots[index].format;
	}
	return format;
}

// Gets the vertex format for a set of snapshot ids
VertexFormat_t CShaderAPIEmpty::ComputeVertexUsage( int numSnapshots, StateSnapshot_t* pIds ) const
{
	return ComputeVertexFormat( numSnapshots, pIds );
}

// Uses a state snapshot
void CShaderAPIEmpty::UseSnapshot( StateSnapshot_t snapshot )
{
}

// Sets the color to modulate by
void CShaderAPIEmpty::Color3f( float r, float g, float b )
{
	g_Modulation[0] = r, g_Modulation[1] = g, g_Modulation[2] = b, g_Modulation[3] = 1.0f;
}

void CShaderAPIEmpty::Color3fv( float const* pColor )
{
}

void CShaderAPIEmpty::Color4f( float r, float g, float b, float a )
{
	g_Modulation[0] = r, g_Modulation[1] = g, g_Modulation[2] = b, g_Modulation[3] = a;
}

void CShaderAPIEmpty::Color4fv( float const* pColor )
{
}

// Faster versions of color
void CShaderAPIEmpty::Color3ub( unsigned char r, unsigned char g, unsigned char b )
{
}

void CShaderAPIEmpty::Color3ubv( unsigned char const* rgb )
{
}

void CShaderAPIEmpty::Color4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
}

void CShaderAPIEmpty::Color4ubv( unsigned char const* rgba )
{
}

// The shade mode
void CShaderAPIEmpty::ShadeMode( ShaderShadeMode_t mode )
{
}

// Binds a particular material to render with
void CShaderAPIEmpty::Bind( IMaterial* pMaterial )
{
	g_pBoundMaterial = static_cast<IMaterialInternal *>( pMaterial );
}

// Cull mode
void CShaderAPIEmpty::CullMode( MaterialCullMode_t cullMode )
{
}

void CShaderAPIEmpty::ForceDepthFuncEquals( bool bEnable )
{
}

// Forces Z buffering on or off
void CShaderAPIEmpty::OverrideDepthEnable( bool bEnable, bool bDepthEnable )
{
}

void CShaderAPIEmpty::OverrideAlphaWriteEnable( bool bOverrideEnable, bool bAlphaWriteEnable )
{
}

void CShaderAPIEmpty::OverrideColorWriteEnable( bool bOverrideEnable, bool bColorWriteEnable )
{
}

//legacy fast clipping linkage
void CShaderAPIEmpty::SetHeightClipZ( float z )
{
}

void CShaderAPIEmpty::SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode )
{
}

// Sets the lights
void CShaderAPIEmpty::SetLight( int lightNum, const LightDesc_t& desc )
{
	if ( lightNum >= 0 && lightNum < kPicaMaxLights )
		g_Lights[lightNum] = desc;
}

// Sets lighting origin for the current model
void CShaderAPIEmpty::SetLightingOrigin( Vector vLightingOrigin )
{
}

void CShaderAPIEmpty::SetAmbientLight( float r, float g, float b )
{
}

void CShaderAPIEmpty::SetAmbientLightCube( Vector4D cube[6] )
{
	for ( int face = 0; face < 6; ++face )
		for ( int c = 0; c < 3; ++c )
			g_AmbientCube[face][c] = cube[face][c];
}

// Get lights
int CShaderAPIEmpty::GetMaxLights( void ) const
{
	return kPicaMaxLights;
}

const LightDesc_t& CShaderAPIEmpty::GetLight( int lightNum ) const
{
	static LightDesc_t blah;
	return lightNum >= 0 && lightNum < kPicaMaxLights ? g_Lights[lightNum] : blah;
}

// Render state for the ambient light cube (vertex shaders)
void CShaderAPIEmpty::SetVertexShaderStateAmbientLightCube()
{
}

void CShaderAPIEmpty::SetSkinningMatrices()
{
}

// Lightmap texture binding
void CShaderAPIEmpty::BindLightmap( TextureStage_t stage )
{
}

void CShaderAPIEmpty::BindBumpLightmap( TextureStage_t stage )
{
}

void CShaderAPIEmpty::BindFullbrightLightmap( TextureStage_t stage )
{
}

void CShaderAPIEmpty::BindWhite( TextureStage_t stage )
{
}

void CShaderAPIEmpty::BindBlack( TextureStage_t stage )
{
}

void CShaderAPIEmpty::BindGrey( TextureStage_t stage )
{
}

// Gets the lightmap dimensions
void CShaderAPIEmpty::GetLightmapDimensions( int *w, int *h )
{
	g_pShaderUtil->GetLightmapDimensions( w, h );
}

// Special system flat normal map binding.
void CShaderAPIEmpty::BindFlatNormalMap( TextureStage_t stage )
{
}

void CShaderAPIEmpty::BindNormalizationCubeMap( TextureStage_t stage )
{
}

void CShaderAPIEmpty::BindSignedNormalizationCubeMap( TextureStage_t stage )
{
}

void CShaderAPIEmpty::BindFBTexture( TextureStage_t stage, int textureIndex )
{
}

// Flushes any primitives that are buffered
void CShaderAPIEmpty::FlushBufferedPrimitives()
{
}

// Gets the dynamic mesh; note that you've got to render the mesh
// before calling this function a second time. Clients should *not*
// call DestroyStaticMesh on the mesh returned by this call.
IMesh* CShaderAPIEmpty::GetDynamicMesh( IMaterial* pMaterial, int nHWSkinBoneCount, bool buffered, IMesh* pVertexOverride, IMesh* pIndexOverride )
{
	return GetDynamicMeshEx( pMaterial, 0, nHWSkinBoneCount, buffered, pVertexOverride, pIndexOverride );
}

IMesh* CShaderAPIEmpty::GetDynamicMeshEx( IMaterial* pMaterial, VertexFormat_t fmt, int nHWSkinBoneCount, bool buffered, IMesh* pVertexOverride, IMesh* pIndexOverride )
{
	VertexFormat_t format = fmt;
	if ( format == 0 && pMaterial )
		format = static_cast<IMaterialInternal *>( pMaterial )->GetVertexFormat() & ~VERTEX_FORMAT_COMPRESSED;
	if ( format == 0 )
		format = VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE( 0, 2 );
	m_Mesh.SetVertexFormat( format );
	m_Mesh.SetVertexSource( static_cast<CEmptyMesh *>( pVertexOverride ) );
	return &m_Mesh;
}

IMesh* CShaderAPIEmpty::GetFlexMesh()
{
	return &m_Mesh;
}

// Begins a rendering pass that uses a state snapshot
void CShaderAPIEmpty::BeginPass( StateSnapshot_t snapshot  )
{
	g_CurrentSnapshot = int( snapshot >> 4 );
}

// Renders a single pass of a material
void CShaderAPIEmpty::RenderPass( int nPass, int nPassCount )
{
	if ( g_pRenderMesh )
		g_pRenderMesh->RenderPass();
}

// stuff related to matrix stacks
void CShaderAPIEmpty::MatrixMode( MaterialMatrixMode_t matrixMode )
{
	switch ( matrixMode )
	{
	case MATERIAL_VIEW: g_CurrentStack = kStackView; break;
	case MATERIAL_PROJECTION: g_CurrentStack = kStackProjection; break;
	default: g_CurrentStack = kStackModel; break;
	}
}

void CShaderAPIEmpty::PushMatrix()
{
	int &top = g_StackTop[g_CurrentStack];
	if ( top + 1 < kStackDepth )
	{
		g_Stacks[g_CurrentStack][top + 1] = g_Stacks[g_CurrentStack][top];
		++top;
	}
}

void CShaderAPIEmpty::PopMatrix()
{
	if ( g_StackTop[g_CurrentStack] > 0 )
		--g_StackTop[g_CurrentStack];
}

void CShaderAPIEmpty::LoadMatrix( float *m )
{
	memcpy( Top(), m, 16 * sizeof( float ) );
}

void CShaderAPIEmpty::MultMatrix( float *m )
{
	Mul( Top(), m, Top() );
}

void CShaderAPIEmpty::MultMatrixLocal( float *m )
{
	MultLocal( m );
}

void CShaderAPIEmpty::GetMatrix( MaterialMatrixMode_t matrixMode, float *dst )
{
	const int stack = matrixMode == MATERIAL_VIEW ? kStackView :
		( matrixMode == MATERIAL_PROJECTION ? kStackProjection : kStackModel );
	memcpy( dst, Top( stack ), 16 * sizeof( float ) );
}

void CShaderAPIEmpty::LoadIdentity( void )
{
	Identity( Top() );
}

void CShaderAPIEmpty::LoadCameraToWorld( void )
{
	// The inverse of the view's rotation (an orthonormal 3x3: its transpose),
	// with no translation.
	const float *v = Top( kStackView );
	float *m = Top();
	Identity( m );
	for ( int i = 0; i < 3; ++i )
		for ( int j = 0; j < 3; ++j )
			m[i * 4 + j] = v[j * 4 + i];
}

void CShaderAPIEmpty::Ortho( double left, double top, double right, double bottom, double zNear, double zFar )
{
	// D3DXMatrixOrthoOffCenterRH( left, right, top (as bottom), bottom (as top) ).
	const double l = left, r = right, b = top, t = bottom;
	float m[16];
	memset( m, 0, sizeof( m ) );
	m[0] = float( 2.0 / ( r - l ) );
	m[5] = float( 2.0 / ( t - b ) );
	m[10] = float( 1.0 / ( zNear - zFar ) );
	m[12] = float( ( l + r ) / ( l - r ) );
	m[13] = float( ( t + b ) / ( b - t ) );
	m[14] = float( zNear / ( zNear - zFar ) );
	m[15] = 1.0f;
	MultLocal( m );
}

void CShaderAPIEmpty::PerspectiveX( double fovx, double aspect, double zNear, double zFar )
{
	// D3DXMatrixPerspectiveRH.
	const double width = 2.0 * zNear * tan( fovx * M_PI / 360.0 );
	const double height = width / aspect;
	float m[16];
	memset( m, 0, sizeof( m ) );
	m[0] = float( 2.0 * zNear / width );
	m[5] = float( 2.0 * zNear / height );
	m[10] = float( zFar / ( zNear - zFar ) );
	m[11] = -1.0f;
	m[14] = float( zNear * zFar / ( zNear - zFar ) );
	MultLocal( m );
}

void CShaderAPIEmpty::PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar, double bottom, double top, double left, double right )
{
}

void CShaderAPIEmpty::PickMatrix( int x, int y, int width, int height )
{
}

void CShaderAPIEmpty::Rotate( float angle, float x, float y, float z )
{
	const float length = sqrtf( x * x + y * y + z * z );
	if ( length <= 0.0f )
		return;
	x /= length, y /= length, z /= length;
	const float a = float( M_PI ) * angle / 180.0f;
	const float c = cosf( a ), s = sinf( a ), t = 1.0f - c;
	float m[16];
	Identity( m );
	m[0] = t * x * x + c, m[1] = t * x * y + s * z, m[2] = t * x * z - s * y;
	m[4] = t * x * y - s * z, m[5] = t * y * y + c, m[6] = t * y * z + s * x;
	m[8] = t * x * z + s * y, m[9] = t * y * z - s * x, m[10] = t * z * z + c;
	MultLocal( m );
}

void CShaderAPIEmpty::Translate( float x, float y, float z )
{
	float m[16];
	Identity( m );
	m[12] = x, m[13] = y, m[14] = z;
	MultLocal( m );
}

void CShaderAPIEmpty::Scale( float x, float y, float z )
{
	float m[16];
	Identity( m );
	m[0] = x, m[5] = y, m[10] = z;
	MultLocal( m );
}

void CShaderAPIEmpty::ScaleXY( float x, float y )
{
	float m[16];
	Identity( m );
	m[0] = x, m[5] = y;
	MultLocal( m );
}

// Fog methods...
void CShaderAPIEmpty::FogMode( MaterialFogMode_t fogMode )
{
}

void CShaderAPIEmpty::FogStart( float fStart )
{
}

void CShaderAPIEmpty::FogEnd( float fEnd )
{
}

void CShaderAPIEmpty::SetFogZ( float fogZ )
{
}
	
void CShaderAPIEmpty::FogMaxDensity( float flMaxDensity )
{
}

void CShaderAPIEmpty::GetFogDistances( float *fStart, float *fEnd, float *fFogZ )
{
}


void CShaderAPIEmpty::SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
}


void CShaderAPIEmpty::SceneFogMode( MaterialFogMode_t fogMode )
{
}

void CShaderAPIEmpty::GetSceneFogColor( unsigned char *rgb )
{
}

MaterialFogMode_t CShaderAPIEmpty::GetSceneFogMode( )
{
	return MATERIAL_FOG_NONE;
}

int CShaderAPIEmpty::GetPixelFogCombo( )
{
	return 0;
}

void CShaderAPIEmpty::FogColor3f( float r, float g, float b )
{
}

void CShaderAPIEmpty::FogColor3fv( float const* rgb )
{
}

void CShaderAPIEmpty::FogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
}

void CShaderAPIEmpty::FogColor3ubv( unsigned char const* rgb )
{
}

void CShaderAPIEmpty::SetViewports( int nCount, const ShaderViewport_t* pViewports )
{
	if ( nCount <= 0 || !pViewports )
		return;
	g_Viewport = pViewports[0];
	if ( g_bDrawingToBackBuffer && pica::Initialized() )
		pica::SetViewport( g_Viewport.m_nTopLeftX, g_Viewport.m_nTopLeftY, g_Viewport.m_nWidth,
			g_Viewport.m_nHeight );
}

int CShaderAPIEmpty::GetViewports( ShaderViewport_t* pViewports, int nMax ) const
{
	if ( pViewports && nMax >= 1 )
	{
		pViewports[0] = g_Viewport;
		if ( g_Viewport.m_nWidth <= 0 || g_Viewport.m_nHeight <= 0 )
			pViewports[0].Init( 0, 0, pica::kScreenWidth, pica::kScreenHeight );
	}
	return 1;
}

// Sets the vertex and pixel shaders
void CShaderAPIEmpty::SetVertexShaderIndex( int vshIndex )
{
}

void CShaderAPIEmpty::SetPixelShaderIndex( int pshIndex )
{
}

// Sets the constant registers for vertex and pixel shaders
void CShaderAPIEmpty::SetVertexShaderConstant( int var, float const* pVec, int numConst, bool bForce )
{
}

void CShaderAPIEmpty::SetBooleanVertexShaderConstant( int var, BOOL const* pVec, int numConst, bool bForce )
{
}

void CShaderAPIEmpty::SetIntegerVertexShaderConstant( int var, int const* pVec, int numConst, bool bForce )
{
}

void CShaderAPIEmpty::SetPixelShaderConstant( int var, float const* pVec, int numConst, bool bForce )
{
}

void CShaderAPIEmpty::SetBooleanPixelShaderConstant( int var, BOOL const* pVec, int numBools, bool bForce )
{
}

void CShaderAPIEmpty::SetIntegerPixelShaderConstant( int var, int const* pVec, int numIntVecs, bool bForce )
{
}

void CShaderAPIEmpty::InvalidateDelayedShaderConstants( void )
{
}

float CShaderAPIEmpty::GammaToLinear_HardwareSpecific( float fGamma ) const
{
	return 0.0f;
}

float CShaderAPIEmpty::LinearToGamma_HardwareSpecific( float fLinear ) const
{
	return 0.0f;
}

void CShaderAPIEmpty::SetLinearToGammaConversionTextures( ShaderAPITextureHandle_t hSRGBWriteEnabledTexture, ShaderAPITextureHandle_t hIdentityTexture )
{

}


// Returns the nearest supported format
ImageFormat CShaderAPIEmpty::GetNearestSupportedFormat( ImageFormat fmt, bool bFilteringRequired /* = true */ ) const
{
	// Uploads arrive as RGBA8888 (the material system decodes DXT and friends)
	// and are encoded for the PICA here.
	return IMAGE_FORMAT_RGBA8888;
}

ImageFormat CShaderAPIEmpty::GetNearestRenderTargetFormat( ImageFormat fmt ) const
{
	return IMAGE_FORMAT_RGBA8888;
}

// Sets the texture state
void CShaderAPIEmpty::BindTexture( Sampler_t stage, ShaderAPITextureHandle_t textureHandle )
{
	if ( stage >= 0 && stage < 16 )
		g_BoundTextures[stage] = textureHandle;
	if ( stage == SHADER_SAMPLER1 )
		g_BoundLightmap = INVALID_SHADERAPI_TEXTURE_HANDLE;
}

void CShaderAPIEmpty::ClearColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	g_ClearColor = unsigned( r ) | ( unsigned( g ) << 8 ) | ( unsigned( b ) << 16 ) | 0xFF000000u;
}

void CShaderAPIEmpty::ClearColor4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	g_ClearColor = unsigned( r ) | ( unsigned( g ) << 8 ) | ( unsigned( b ) << 16 ) | ( unsigned( a ) << 24 );
}

// Indicates we're going to be modifying this texture
// TexImage2D, TexSubImage2D, TexWrap, TexMinFilter, and TexMagFilter
// all use the texture specified by this function.
void CShaderAPIEmpty::ModifyTexture( ShaderAPITextureHandle_t textureHandle )
{
	g_ModifyTexture = textureHandle;
}

// Texture management methods
void CShaderAPIEmpty::TexImage2D( int level, int cubeFace, ImageFormat dstFormat, int zOffset, int width, int height, 
						 ImageFormat srcFormat, bool bSrcIsTiled, void *imageData )
{
	++g_TextureCounters.images;
	PicaTexture *texture = TextureFor( g_ModifyTexture );
	if ( texture && texture->cube )
	{
		// Each face's base level (smaller levels are not kept), square, at
		// most the size cap a side.
		if ( level == 0 && cubeFace == 0 )
			printf( "pica: cube %s face images %dx%d src format %d data %d\n", texture->name,
				width, height, (int)srcFormat, imageData != NULL );
		if ( level != 0 || cubeFace < 0 || cubeFace > 5 || !imageData || width != height )
		{
			++g_TextureCounters.rejected;
			return;
		}
		CUtlVector<unsigned char> rgba;
		rgba.SetCount( width * height * 4 );
		if ( !ImageLoader::ConvertImageFormat( (const unsigned char *)imageData, srcFormat,
				rgba.Base(), IMAGE_FORMAT_RGBA8888, width, height ) )
		{
			++g_TextureCounters.convertFailed;
			return;
		}
		const int size = Min( width, g_TextureSizeCap );
		if ( texture->cubeSize != size )
		{
			for ( CUtlVector<unsigned char> &face : texture->cubeFaces )
				face.Purge();
			texture->cubeSize = size;
		}
		CUtlVector<unsigned char> &out = texture->cubeFaces[cubeFace];
		out.SetCount( size * size * 4 );
		if ( size == width )
			memcpy( out.Base(), rgba.Base(), rgba.Count() );
		else
			pica::Resample( rgba.Base(), width, height, out.Base(), size, size );
		texture->dirty = true;
		return;
	}
	if ( !texture || cubeFace != 0 || zOffset != 0 || width <= 0 || height <= 0 ||
		texture->renderTarget || texture->depth )
	{
		++g_TextureCounters.rejected;
		return;
	}
	// No data (a lightmap page, filled later by TexSubImage2D): the level is
	// allocated at the GPU size and cleared, so the updates have a home and
	// the render core can sample the page (RFC 0026 P3).
	if ( !imageData )
	{
		if ( level != 0 || texture->mipLevels > 1 )
		{
			++g_TextureCounters.rejected;
			return;
		}
		int gpuW, gpuH;
		GpuSize( width, height, g_UnmippedSizeCap, gpuW, gpuH );
		texture->levels.Purge();
		texture->levelHashes.Purge();
		texture->baseWidth = gpuW;
		texture->baseHeight = gpuH;
		CUtlVector<unsigned char> &out = texture->levels[texture->levels.AddToTail()];
		out.SetCount( gpuW * gpuH * 4 );
		memset( out.Base(), 0, out.Count() );
		texture->levelHashes.AddToTail( LevelHash( out ) );
		texture->dirty = true;
		return;
	}
	const bool mipped = texture->mipLevels > 1;
	const int cap = mipped ? g_TextureSizeCap : g_UnmippedSizeCap;
	// Keep the first level that fits the cap as the GPU's base (smaller mips
	// follow it); a texture with no fitting level is resampled.
	if ( mipped && ( width > cap || height > cap ) && level + 1 < texture->mipLevels )
		return;
	int gpuW, gpuH;
	GpuSize( width, height, cap, gpuW, gpuH );
	CUtlVector<unsigned char> rgba;
	rgba.SetCount( width * height * 4 );
	if ( !ImageLoader::ConvertImageFormat( (const unsigned char *)imageData, srcFormat, rgba.Base(),
			IMAGE_FORMAT_RGBA8888, width, height ) )
	{
		++g_TextureCounters.convertFailed;
		return;
	}
	// -pica_dump_texture <debug name>: the converted RGBA of each level the
	// backend receives, as sdmc:/source-engine/texdump_<level>.ppm.
	static const char *s_DumpName = CommandLine()->ParmValue( "-pica_dump_texture", (const char *)NULL );
	if ( s_DumpName && !V_stricmp( s_DumpName, texture->name ) )
	{
		char path[96];
		V_snprintf( path, sizeof( path ), "sdmc:/source-engine/texdump_%d.ppm", level );
		if ( FILE *f = fopen( path, "wb" ) )
		{
			fprintf( f, "P6\n%d %d\n255\n", width, height );
			for ( int i = 0; i < width * height; ++i )
				fwrite( &rgba[i * 4], 1, 3, f );
			fclose( f );
		}
		printf( "pica: dumped %s level %d %dx%d src format %d\n", texture->name, level, width, height, (int)srcFormat );
	}
	if ( texture->levels.Count() == 0 || !mipped )
	{
		texture->levels.Purge();
		texture->levelHashes.Purge();
		texture->baseWidth = gpuW;
		texture->baseHeight = gpuH;
	}
	const int index = texture->levels.Count();
	const int wantW = texture->baseWidth >> index, wantH = texture->baseHeight >> index;
	if ( wantW < 8 || wantH < 8 )
		return;
	CUtlVector<unsigned char> &out = texture->levels[texture->levels.AddToTail()];
	out.SetCount( wantW * wantH * 4 );
	if ( wantW == width && wantH == height )
		memcpy( out.Base(), rgba.Base(), rgba.Count() );
	else
		pica::Resample( rgba.Base(), width, height, out.Base(), wantW, wantH );
	texture->levelHashes.AddToTail( LevelHash( out ) );
	texture->dirty = true;
}

void CShaderAPIEmpty::TexSubImage2D( int level, int cubeFace, int xOffset, int yOffset, int zOffset, int width, int height,
						 ImageFormat srcFormat, int srcStride, bool bSrcIsTiled, void *imageData )
{
	PicaTexture *texture = TextureFor( g_ModifyTexture );
	if ( !texture || level != 0 || cubeFace != 0 || !imageData || texture->levels.Count() == 0 ||
		texture->mipLevels > 1 )
		return;
	// Unmipped textures keep their RGBA copy at the GPU size: scale the
	// sub-rectangle into it (fonts and UI pages are usually at full size).
	CUtlVector<unsigned char> rgba;
	rgba.SetCount( width * height * 4 );
	if ( !ImageLoader::ConvertImageFormat( (const unsigned char *)imageData, srcFormat, rgba.Base(),
			IMAGE_FORMAT_RGBA8888, width, height, srcStride, 0 ) )
		return;
	unsigned char *dst = texture->levels[0].Base();
	const int dw = texture->baseWidth, dh = texture->baseHeight;
	for ( int y = 0; y < height; ++y )
	{
		const int ty = ( yOffset + y ) * dh / texture->height;
		if ( ty < 0 || ty >= dh )
			continue;
		for ( int x = 0; x < width; ++x )
		{
			const int tx = ( xOffset + x ) * dw / texture->width;
			if ( tx < 0 || tx >= dw )
				continue;
			memcpy( dst + ( ty * dw + tx ) * 4, rgba.Base() + ( y * width + x ) * 4, 4 );
		}
	}
	if ( texture->levelHashes.Count() > 0 )
		texture->levelHashes[0] = LevelHash( texture->levels[0] );
	texture->dirty = true;
}

void CShaderAPIEmpty::TexImageFromVTF( IVTFTexture *pVTF, int iVTFFrame )
{
	// The material system's texture upload: every mip of the frame (face 0;
	// cube and volume textures keep their first face/slice) through TexImage2D,
	// which keeps the levels that fit the PICA and encodes them.
	if ( !pVTF )
		return;
#if !defined( PLATFORM_3DS )
	// The full model's cube maps: each face's base level (TexImage2D's cube path).
	PicaTexture *target = TextureFor( g_ModifyTexture );
	if ( target && target->cube )
	{
		int width = 0, height = 0, depth = 0;
		pVTF->ComputeMipLevelDimensions( 0, &width, &height, &depth );
		for ( int face = 0; face < 6; ++face )
		{
			unsigned char *pData = pVTF->ImageData( iVTFFrame, face, 0 );
			if ( pData )
				TexImage2D( 0, face, IMAGE_FORMAT_RGBA8888, 0, width, height, pVTF->Format(), false, pData );
		}
		return;
	}
#endif
	for ( int level = 0; level < pVTF->MipCount(); ++level )
	{
		int width = 0, height = 0, depth = 0;
		pVTF->ComputeMipLevelDimensions( level, &width, &height, &depth );
		unsigned char *pData = pVTF->ImageData( iVTFFrame, 0, level );
		if ( pData )
			TexImage2D( level, 0, IMAGE_FORMAT_RGBA8888, 0, width, height, pVTF->Format(), false, pData );
	}
}

// TexLock/TexUnlock: the material system writes lightmap pages through a
// pixel writer (cmatlightmaps.cpp). The writer gets a scratch RGBA8 rectangle;
// TexUnlock puts it into the texture's level as TexSubImage2D does, making the
// level first when the texture has none (RFC 0026 P3: the render core samples
// the pages).
namespace
{
CUtlVector<unsigned char> g_TexLockPixels;
ShaderAPITextureHandle_t g_TexLockTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
int g_TexLockRect[4] = {}; // x, y, width, height
}

bool CShaderAPIEmpty::TexLock( int level, int cubeFaceID, int xOffset, int yOffset, 
								int width, int height, CPixelWriter& writer )
{
	PicaTexture *texture = TextureFor( g_ModifyTexture );
	if ( !texture || level != 0 || cubeFaceID != 0 || width <= 0 || height <= 0 ||
		texture->mipLevels > 1 || texture->renderTarget || texture->depth )
		return false;
	if ( texture->levels.Count() == 0 )
		TexImage2D( 0, 0, IMAGE_FORMAT_RGBA8888, 0, texture->width, texture->height,
			IMAGE_FORMAT_RGBA8888, false, NULL );
	if ( texture->levels.Count() == 0 )
		return false;
	g_TexLockPixels.SetCount( width * height * 4 );
	memset( g_TexLockPixels.Base(), 0, g_TexLockPixels.Count() );
	g_TexLockTexture = g_ModifyTexture;
	g_TexLockRect[0] = xOffset;
	g_TexLockRect[1] = yOffset;
	g_TexLockRect[2] = width;
	g_TexLockRect[3] = height;
	writer.SetPixelMemory( IMAGE_FORMAT_RGBA8888, g_TexLockPixels.Base(), width * 4 );
	return true;
}

void CShaderAPIEmpty::TexUnlock( )
{
	if ( g_TexLockTexture == INVALID_SHADERAPI_TEXTURE_HANDLE )
		return;
	const ShaderAPITextureHandle_t modify = g_ModifyTexture;
	g_ModifyTexture = g_TexLockTexture;
	TexSubImage2D( 0, 0, g_TexLockRect[0], g_TexLockRect[1], 0, g_TexLockRect[2], g_TexLockRect[3],
		IMAGE_FORMAT_RGBA8888, g_TexLockRect[2] * 4, false, g_TexLockPixels.Base() );
	g_ModifyTexture = modify;
	g_TexLockTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
}


// These are bound to the texture, not the texture environment
void CShaderAPIEmpty::TexMinFilter( ShaderTexFilterMode_t texFilterMode )
{
}

void CShaderAPIEmpty::TexMagFilter( ShaderTexFilterMode_t texFilterMode )
{
}

void CShaderAPIEmpty::TexWrap( ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode )
{
	PicaTexture *texture = TextureFor( g_ModifyTexture );
	if ( !texture )
		return;
	const bool wrap = wrapMode == SHADER_TEXWRAPMODE_REPEAT;
	if ( coord == SHADER_TEXCOORD_S )
		texture->wrapS = wrap;
	else if ( coord == SHADER_TEXCOORD_T )
		texture->wrapT = wrap;
	texture->gpu.SetWrap( texture->wrapS, texture->wrapT );
}

void CShaderAPIEmpty::TexSetPriority( int priority )
{
}

ShaderAPITextureHandle_t CShaderAPIEmpty::CreateTexture( 
	int width, 
	int height,
	int depth,
	ImageFormat dstImageFormat, 
	int numMipLevels, 
	int numCopies, 
	int flags, 
	const char *pDebugName,
	const char *pTextureGroupName )
{
	ShaderAPITextureHandle_t handle = 0;
	CreateTextures( &handle, 1, width, height, depth, dstImageFormat, numMipLevels, numCopies, flags,
		pDebugName, pTextureGroupName );
	return handle;
}

// Create a multi-frame texture (equivalent to calling "CreateTexture" multiple times, but more efficient)
void CShaderAPIEmpty::CreateTextures( 
							ShaderAPITextureHandle_t *pHandles,
							int count,
							int width, 
							int height,
							int depth,
							ImageFormat dstImageFormat, 
							int numMipLevels, 
							int numCopies, 
							int flags, 
							const char *pDebugName,
							const char *pTextureGroupName )
{
	for ( int k = 0; k < count; ++k )
	{
		PicaTexture *texture = new PicaTexture;
		texture->used = true;
		texture->width = width;
		texture->height = height;
		texture->mipLevels = numMipLevels > 0 ? numMipLevels : 1;
		texture->renderTarget = ( flags & TEXTURE_CREATE_RENDERTARGET ) != 0;
		texture->depth = ( flags & TEXTURE_CREATE_DEPTHBUFFER ) != 0;
		texture->lightmap =
			pTextureGroupName && V_strcmp( pTextureGroupName, TEXTURE_GROUP_LIGHTMAP ) == 0;
#if !defined( PLATFORM_3DS )
		// The full model samples cube maps as cubes (the 3DS keeps face 0).
		texture->cube = ( flags & TEXTURE_CREATE_CUBEMAP ) != 0;
#endif
		V_strncpy( texture->name, pDebugName ? pDebugName : "", sizeof( texture->name ) );
		pHandles[k] = ShaderAPITextureHandle_t( g_Textures.AddToTail( texture ) + 1 );
	}
}


ShaderAPITextureHandle_t CShaderAPIEmpty::CreateDepthTexture( ImageFormat renderFormat, int width, int height, const char *pDebugName, bool bTexture )
{
	ShaderAPITextureHandle_t handle = 0;
	CreateTextures( &handle, 1, width, height, 1, renderFormat, 1, 1, TEXTURE_CREATE_DEPTHBUFFER,
		pDebugName, NULL );
	return handle;
}

void CShaderAPIEmpty::DeleteTexture( ShaderAPITextureHandle_t textureHandle )
{
	PicaTexture *texture = TextureFor( textureHandle );
	if ( !texture )
		return;
	for ( int i = 0; i < 16; ++i )
		if ( g_BoundTextures[i] == textureHandle )
			g_BoundTextures[i] = INVALID_SHADERAPI_TEXTURE_HANDLE;
	g_Textures[int( textureHandle ) - 1] = NULL;
	delete texture;
}

bool CShaderAPIEmpty::IsTexture( ShaderAPITextureHandle_t textureHandle )
{
	return TextureFor( textureHandle ) != NULL;
}

bool CShaderAPIEmpty::IsTextureResident( ShaderAPITextureHandle_t textureHandle )
{
	PicaTexture *texture = TextureFor( textureHandle );
	return texture && texture->gpu.Valid();
}

// stuff that isn't to be used from within a shader
void CShaderAPIEmpty::ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil, int renderTargetWidth, int renderTargetHeight )
{
	if ( g_bDrawingToBackBuffer && pica::Initialized() )
	{
		++g_Counters.clears;
		pica::Clear( bClearColor, bClearDepth, g_ClearColor );
	}
}

void CShaderAPIEmpty::ClearBuffersObeyStencil( bool bClearColor, bool bClearDepth )
{
}

void CShaderAPIEmpty::ClearBuffersObeyStencilEx( bool bClearColor, bool bClearAlpha, bool bClearDepth )
{
}

void CShaderAPIEmpty::PerformFullScreenStencilOperation( void )
{
}

void CShaderAPIEmpty::SetScissorRect( const int nLeft, const int nTop, const int nRight, const int nBottom, const bool bEnableScissor )
{
}

void CShaderAPIEmpty::ReadPixels( int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat )
{
}

void CShaderAPIEmpty::ReadPixels( Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data, ImageFormat dstFormat, int nDstStride )
{
}

void CShaderAPIEmpty::FlushHardware()
{
}

void CShaderAPIEmpty::ResetRenderState( bool bFullReset )
{
}

// Set the number of bone weights
void CShaderAPIEmpty::SetNumBoneWeights( int numBones )
{
}

void CShaderAPIEmpty::EnableHWMorphing( bool bEnable )
{
}

// Selection mode methods
int CShaderAPIEmpty::SelectionMode( bool selectionMode )
{
	return 0;
}

void CShaderAPIEmpty::SelectionBuffer( unsigned int* pBuffer, int size )
{
}

void CShaderAPIEmpty::ClearSelectionNames( )
{
}

void CShaderAPIEmpty::LoadSelectionName( int name )
{
}

void CShaderAPIEmpty::PushSelectionName( int name )
{
}

void CShaderAPIEmpty::PopSelectionName()
{
}


// Use this to get the mesh builder that allows us to modify vertex data
CMeshBuilder* CShaderAPIEmpty::GetVertexModifyBuilder()
{
	return 0;
}

// Board-independent calls, here to unify how shaders set state
// Implementations should chain back to IShaderUtil->BindTexture(), etc.

// Use this to begin and end the frame
void CShaderAPIEmpty::BeginFrame()
{
}

void CShaderAPIEmpty::EndFrame()
{
}

// returns the current time in seconds....
double CShaderAPIEmpty::CurrentTime() const
{
	return Sys_FloatTime();
}

// Get the current camera position in world space.
void CShaderAPIEmpty::GetWorldSpaceCameraPosition( float * pPos ) const
{
	// The view matrix's inverse translation (row vectors, D3D), as
	// shaderapivulkan computes it: the eye in world space.
	const float *view = Top( kStackView );
	for ( int i = 0; i < 3; ++i )
		pPos[i] = -( view[12] * view[i * 4] + view[13] * view[i * 4 + 1] + view[14] * view[i * 4 + 2] );
}

void CShaderAPIEmpty::ForceHardwareSync( void )
{
}

void CShaderAPIEmpty::SetClipPlane( int index, const float *pPlane )
{
}

void CShaderAPIEmpty::EnableClipPlane( int index, bool bEnable )
{
}

void CShaderAPIEmpty::SetFastClipPlane( const float *pPlane )
{
}

void CShaderAPIEmpty::EnableFastClip( bool bEnable )
{
}

int CShaderAPIEmpty::GetCurrentNumBones( void ) const
{
	return 0;
}

bool CShaderAPIEmpty::IsHWMorphingEnabled( void ) const
{
	return false;
}

int CShaderAPIEmpty::GetCurrentLightCombo( void ) const
{
	return 0;
}

void CShaderAPIEmpty::GetDX9LightState( LightState_t *state ) const
{
	state->m_nNumLights = 0;
	state->m_bAmbientLight = false;
	state->m_bStaticLightVertex = false;
	state->m_bStaticLightTexel = false;
}

MaterialFogMode_t CShaderAPIEmpty::GetCurrentFogType( void ) const
{
	return MATERIAL_FOG_NONE;
}

void CShaderAPIEmpty::RecordString( const char *pStr )
{
}

bool CShaderAPIEmpty::ReadPixelsFromFrontBuffer() const
{
	return true;
}

bool CShaderAPIEmpty::PreferDynamicTextures() const
{
	return false;
}

bool CShaderAPIEmpty::PreferReducedFillrate() const
{ 
	return false; 
}

bool CShaderAPIEmpty::HasProjectedBumpEnv() const
{
	return true;
}

int  CShaderAPIEmpty::GetCurrentDynamicVBSize( void )
{
	return 0;
}

void CShaderAPIEmpty::DestroyVertexBuffers( bool bExitingLevel )
{
}

void CShaderAPIEmpty::EvictManagedResources()
{
}

void CShaderAPIEmpty::SetTextureTransformDimension( TextureStage_t textureStage, int dimension, bool projected )
{
}

void CShaderAPIEmpty::SetBumpEnvMatrix( TextureStage_t textureStage, float m00, float m01, float m10, float m11 )
{
}

void CShaderAPIEmpty::SyncToken( const char *pToken )
{
}
