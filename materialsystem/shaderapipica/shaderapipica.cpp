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
#include "render/legacy_shader_provider.h"
#include "imaterialinternal.h"
#include "shaderapi/commandbuffer.h"
#include "bitmap/imageformat.h"
#include "tier0/icommandline.h"
#include "pica_renderer.h"
#include "renderparm.h"
#include "pixelwriter.h"
#include "render/legacy/core_passes.h"
#include "render/legacy/material_flag_keys.h"
#include "itextureinternal.h"
#include <string>
#include <vector>
#include <malloc.h>

extern "C" unsigned int linearSpaceFree( void ); // libctru: GPU-visible linear heap
extern "C" unsigned int __ctru_heap_size; // libctru: the main heap's size
#include "pica_texture.h"


//-----------------------------------------------------------------------------
// The empty mesh
//-----------------------------------------------------------------------------
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
	// this backend draws it fullbright, and the draw stays off the render
	// core's model point (RFC 0026 P3: model lighting is the dynamic models').
	void SetColorMesh( IMesh *pColorMesh, int nVertexOffset )
	{
		m_bHasColorMesh = pColorMesh != NULL;
	}
	bool m_bHasColorMesh = false;


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
	void DumpDraw( const float *clip, const pica::DrawState &state, const char *textureName, int textureWidth, int textureHeight, const pica::Vertex *vertices,
		const unsigned short *indices, int count ) const;

	bool m_bIsDynamic;
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
	// Texture coordinate 0 of a format with more than two components (sprite
	// cards, particles): the builder writes every component, so they go here,
	// 4 per vertex, and the record takes the first two at Unlock/ModifyEnd.
	// Writing them into the record's two floats overran into the next vertex
	// and, past the last, into the neighbouring allocation (the memory audit,
	// 2026-10-07).
	float *m_pWideTexCoords = nullptr;
	int m_nWideTexCoordCapacity = 0;
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


//-----------------------------------------------------------------------------
// PICA state shared by the mesh, shadow and dynamic APIs.
//-----------------------------------------------------------------------------
namespace
{

struct PicaSnapshot
{
	pica::DrawState state;
	VertexFormat_t format;
};

// Same recorded state (DrawState compared field by field: its padding is not
// initialized) and vertex format.
bool SameSnapshot( const PicaSnapshot &a, const pica::DrawState &s, VertexFormat_t format )
{
	const pica::DrawState &t = a.state;
	return a.format == format && t.depthTest == s.depthTest && t.depthWrite == s.depthWrite &&
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
	bool dirty = false;
	pica::Texture gpu;
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
CUtlVector<PicaTexture *> g_Textures; // index = handle - 1
ShaderAPITextureHandle_t g_ModifyTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
ShaderAPITextureHandle_t g_BoundTextures[16];
IMaterialInternal *g_pBoundMaterial = NULL;
// Model lighting as studiorender sets it (SetAmbientLightCube, SetLight): the
// render core's model point reads it at each draw (RFC 0026 P3).
float g_AmbientCube[6][3] = {};
constexpr int kPicaMaxLights = 4;
LightDesc_t g_Lights[kPicaMaxLights];
CEmptyMesh *g_pRenderMesh = NULL;
int g_CurrentSnapshot = -1;
bool g_bDrawingToBackBuffer = true;
unsigned int g_ClearColor = 0x000000FF;
// The presented frame number, and the frame whose draws are listed
// (-pica_dump_draws <frame>; each draw's material, size, texture and NDC bounds).
int g_PicaFrame = 0;
int g_PicaDumpFrame = -1;
ShaderViewport_t g_Viewport;
Matrix4 g_Stacks[kStackCount][kStackDepth];
int g_StackTop[kStackCount];
int g_CurrentStack = kStackModel;
float g_Bones[kMaxBones][12];
int g_MaxBone = -1;
float g_Modulation[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
int g_TextureSizeCap = 128;
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
int g_UnmippedSizeCap = 256;

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
	texture.gpu.Release();
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
	// Block-compressed when mipmapped (world and model textures); RGBA8 for
	// textures the material system updates in place (fonts, UI).
	// -pica_texture_rgba8 uploads everything as RGBA8 (isolates the ETC1 encoder).
	static const bool s_ForceRGBA8 = CommandLine()->FindParm( "-pica_texture_rgba8" ) != 0;
	const pica::UploadFormat format =
	    ( !mipped || s_ForceRGBA8 )
	        ? pica::UploadFormat::kRGBA8
	        : ( alpha ? pica::UploadFormat::kETC1A4 : pica::UploadFormat::kETC1 );
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
	// Mipmapped levels are not updated in place: drop the CPU copy.
	if ( mipped )
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
	VertexFormat_t m_VertexFormat;
};


//-----------------------------------------------------------------------------
// The DX8 implementation of the shader device
//-----------------------------------------------------------------------------
class CShaderDeviceEmpty : public IShaderDevice
{
public:
	CShaderDeviceEmpty() : m_DynamicMesh( true ), m_Mesh( false ) {}

	// Methods of IShaderDevice
	virtual int GetCurrentAdapter() const { return 0; }
	virtual bool IsUsingGraphics() const { return true; }
	virtual void SpewDriverInfo() const;
	virtual ImageFormat GetBackBufferFormat() const { return IMAGE_FORMAT_RGB888; }
	virtual void GetBackBufferDimensions( int& width, int& height ) const;
	virtual int  StencilBufferBits() const { return 0; }
	virtual bool IsAAEnabled() const { return false; }
	virtual void Present( )
	{
		if ( !pica::Initialized() )
			return;
		if ( !pica::InFrame() )
			pica::BeginFrame();
		pica::EndFrame();
		g_bDrawingToBackBuffer = true;
		// A capture the boot harness asks for: -pica_capture <frame> <path>.
		static int s_captureFrame = CommandLine()->ParmValue( "-pica_capture", -1 );
		g_PicaDumpFrame = CommandLine()->ParmValue( "-pica_dump_draws", -1 );
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
			printf( "pica: frame %d draws %u triangles %u textures %u KB meshes %u KB rings %u/%u KB ring overflows %u blend refusals %u submits %u | "
				"mesh draws %u primlist %u empty %u material %u passes %u target skips %u clears %u overrun draws %u wide texcoords %u core meshes %u\n", s_frame,
				(unsigned)stats.draws, (unsigned)stats.triangles, (unsigned)( stats.textureBytes / 1024 ), (unsigned)( stats.meshBytes / 1024 ), (unsigned)( stats.ringPeak[0] / 1024 ), (unsigned)( stats.ringPeak[1] / 1024 ),
				(unsigned)stats.ringOverflows, (unsigned)stats.blendRefusals, (unsigned)stats.submits, g_Counters.meshDraws, g_Counters.primListDraws,
				g_Counters.primListEmpty, g_Counters.materialDraws,
				g_Counters.renderPasses, g_Counters.targetSkips, g_Counters.clears, g_Counters.overrunDraws, g_Counters.wideTexCoordLocks, g_Counters.coreMeshDraws );
		if ( s_frame % 120 == 1 )
			printf( "pica: textures: images %u rejected %u convert failed %u uploads %u failed %u "
				"corrupted %u\n",
				g_TextureCounters.images, g_TextureCounters.rejected, g_TextureCounters.convertFailed,
				g_TextureCounters.uploads, g_TextureCounters.uploadFailed, g_TextureCounters.corrupted );
		g_Counters = DrawPathCounters();
	}
	virtual void GetWindowSize( int &width, int &height ) const;
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
	virtual void SetHardwareGammaRamp( float fGamma, float fGammaTVRangeMin, float fGammaTVRangeMax, float fGammaTVExponent, bool bTVEnabled ) {}
	virtual void EnableNonInteractiveMode( MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo ) {}
	virtual void RefreshFrontBufferNonInteractive( ) {}
	virtual void HandleThreadEvent( uint32 threadEvent ) {}

#ifdef DX_TO_GL_ABSTRACTION
	virtual void DoStartupShaderPreloading( void ) {}
#endif

	virtual char *GetDisplayDeviceName() OVERRIDE { return ""; }

private:
	CEmptyMesh m_Mesh;
	CEmptyMesh m_DynamicMesh;
};

static CShaderDeviceEmpty s_ShaderDeviceEmpty;

// FIXME: Remove; it's for backward compat with the materialsystem only for now
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderDeviceEmpty, IShaderDevice, 
								  SHADER_DEVICE_INTERFACE_VERSION, s_ShaderDeviceEmpty )


//-----------------------------------------------------------------------------
// The DX8 implementation of the shader device
//-----------------------------------------------------------------------------
class CShaderDeviceMgrEmpty : public IShaderDeviceMgr
{
public:
	// Methods of IAppSystem
	virtual bool Connect( CreateInterfaceFn factory );
	virtual void Disconnect();
	virtual void *QueryInterface( const char *pInterfaceName );
	virtual InitReturnVal_t Init();
	virtual void Shutdown();

public:
	// Methods of IShaderDeviceMgr
	virtual int	 GetAdapterCount() const;
	virtual void GetAdapterInfo( int adapter, MaterialAdapterInfo_t& info ) const;
	virtual bool GetRecommendedConfigurationInfo( int nAdapter, int nDXLevel, KeyValues *pKeyValues );
	virtual int	 GetModeCount( int adapter ) const;
	virtual void GetModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter, int mode ) const;
	virtual void GetCurrentModeInfo( ShaderDisplayMode_t* pInfo, int nAdapter ) const;
	virtual bool SetAdapter( int nAdapter, int nFlags );
	virtual CreateInterfaceFn SetMode( void *hWnd, int nAdapter, const ShaderDeviceInfo_t& mode );
	virtual void AddModeChangeCallback( ShaderModeChangeCallbackFunc_t func ) {}
	virtual void RemoveModeChangeCallback( ShaderModeChangeCallbackFunc_t func ) {}
};

static CShaderDeviceMgrEmpty s_ShaderDeviceMgrEmpty;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderDeviceMgrEmpty, IShaderDeviceMgr, 
								  SHADER_DEVICE_MGR_INTERFACE_VERSION, s_ShaderDeviceMgrEmpty )


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
	}

	void CopyRenderTargetToTextureEx( ShaderAPITextureHandle_t texID, int nRenderTargetID, Rect_t *pSrcRect, Rect_t *pDstRect )
	{
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
		// Only the back buffer is drawn; views into render targets (monitors,
		// portals, water, post) are skipped.
		if ( nRenderTargetID == 0 )
			g_bDrawingToBackBuffer = colorTextureHandle == SHADER_RENDERTARGET_BACKBUFFER;
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
	}

	void SetStencilFailOperation(StencilOperation_t op)
	{
	}

	void SetStencilZFailOperation(StencilOperation_t op)
	{
	}

	void SetStencilPassOperation(StencilOperation_t op)
	{
	}

	void SetStencilCompareFunction(StencilComparisonFunction_t cmpfn)
	{
	}

	void SetStencilReferenceValue(int ref)
	{
	}

	void SetStencilTestMask(uint32 msk)
	{
	}

	void SetStencilWriteMask(uint32 msk)
	{
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

	virtual void SetShadowDepthBiasFactors( float fShadowSlopeScaleDepthBias, float fShadowDepthBias ) {}

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
		if ( !texture || srgb )
		{
			printf( "pica: core import of texture %d refused: %s\n", handle,
				srgb ? "sRGB view" : "no such texture" );
			return render::device::TextureId{};
		}
		if ( texture->dirty )
			UploadTexture( *texture );
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
		desc.address = !texture || ( texture->wrapS && texture->wrapT )
			? render::device::AddressMode::kRepeat
			: render::device::AddressMode::kClampToEdge;
		return desc;
	}
	bool Pending( int handle ) override
	{
		const PicaTexture *texture = TextureFor( ShaderAPITextureHandle_t( handle ) );
		return texture && !texture->gpu.Valid() && texture->levels.Count() == 0;
	}
};
CPicaCoreTextures g_PicaCoreTextures;

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
		target.depthFormat = render::device::Format::kD24UnormS8;
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
		target.streamEpoch = target.frame;
		// LDR, gamma space: the reduced model reads the pages as they are.
		target.lightmapScale = 1.0f;
		target.outputScale = 1.0f;
		target.specular = false;
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
		g_CorePassRecorder->RecordSlot( tag, *encoder, target );
		pica::EndCoreSection();
	}
};
CPicaCorePassSlots g_PicaCorePassSlots;

static bool CreatePicaShaderBackend( render::LegacyShaderServices *services )
{
	if ( !services )
		return false;
	services->manager = &s_ShaderDeviceMgrEmpty;
	services->api = &g_ShaderAPIEmpty;
	services->device = &s_ShaderDeviceEmpty;
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
	pica::BindDevice( device );
}

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
	if ( !Q_stricmp( pInterfaceName, SHADER_DEVICE_INTERFACE_VERSION ) )
		return static_cast< IShaderDevice* >( &s_ShaderDeviceEmpty );
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
//
// CShaderDeviceMgrEmpty
//
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrEmpty::Connect( CreateInterfaceFn factory )
{
	// So others can access it
	g_pShaderUtil = (IShaderUtil*)factory( SHADER_UTIL_INTERFACE_VERSION, NULL );

	return true;
}

void CShaderDeviceMgrEmpty::Disconnect()
{
	g_pShaderUtil = NULL;
}

void *CShaderDeviceMgrEmpty::QueryInterface( const char *pInterfaceName )
{
	if ( !Q_stricmp( pInterfaceName, SHADER_DEVICE_MGR_INTERFACE_VERSION ) )
		return static_cast< IShaderDeviceMgr* >( this );
	if ( !Q_stricmp( pInterfaceName, MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION ) )
		return static_cast< IMaterialSystemHardwareConfig* >( &g_ShaderAPIEmpty );
	return NULL;
}

InitReturnVal_t CShaderDeviceMgrEmpty::Init()
{
	return INIT_OK;
}

void CShaderDeviceMgrEmpty::Shutdown()
{

}

// Sets the adapter
bool CShaderDeviceMgrEmpty::SetAdapter( int nAdapter, int nFlags )
{
	return true;
}

// FIXME: Is this a public interface? Might only need to be private to shaderapi
CreateInterfaceFn CShaderDeviceMgrEmpty::SetMode( void *hWnd, int nAdapter, const ShaderDeviceInfo_t& mode ) 
{
	if ( !g_ShaderAPIEmpty.SetMode( hWnd, nAdapter, ShaderDeviceInfo_t() ) )
		return NULL;
	return ShaderInterfaceFactory;
}

// Gets the number of adapters...
// The null backend advertises one software adapter so the render.contracts
// provider around this manager can describe it and select a profile for it.
int	 CShaderDeviceMgrEmpty::GetAdapterCount() const
{
	return 1;
}

bool CShaderDeviceMgrEmpty::GetRecommendedConfigurationInfo( int nAdapter, int nDXLevel, KeyValues *pKeyValues ) 
{
	return true;
}

// Returns info about each adapter
void CShaderDeviceMgrEmpty::GetAdapterInfo( int adapter, MaterialAdapterInfo_t& info ) const
{
	memset( &info, 0, sizeof( info ) );
	Q_strncpy( info.m_pDriverName, "PICA200 (Nintendo 3DS, fullbright)", sizeof( info.m_pDriverName ) );
	info.m_nDXSupportLevel = 90;
}

// Returns the number of modes
int	 CShaderDeviceMgrEmpty::GetModeCount( int nAdapter ) const
{
	return 1;
}

// Returns mode information..
void CShaderDeviceMgrEmpty::GetModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter, int nMode ) const
{
	pInfo->m_nWidth = pica::kScreenWidth;
	pInfo->m_nHeight = pica::kScreenHeight;
	pInfo->m_Format = IMAGE_FORMAT_RGBA8888;
	pInfo->m_nRefreshRateNumerator = 60;
	pInfo->m_nRefreshRateDenominator = 1;
}

void CShaderDeviceMgrEmpty::GetCurrentModeInfo( ShaderDisplayMode_t* pInfo, int nAdapter ) const
{
	GetModeInfo( pInfo, nAdapter, 0 );
}


//-----------------------------------------------------------------------------
//
// Shader device empty
//
//-----------------------------------------------------------------------------
void CShaderDeviceEmpty::GetWindowSize( int &width, int &height ) const
{
	width = pica::kScreenWidth;
	height = pica::kScreenHeight;
}

void CShaderDeviceEmpty::GetBackBufferDimensions( int& width, int& height ) const
{
	width = pica::kScreenWidth;
	height = pica::kScreenHeight;
}

// Use this to spew information about the 3D layer 
void CShaderDeviceEmpty::SpewDriverInfo() const
{
	Warning("Empty shader\n");
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
	delete[] m_pBoneWeights;
	delete[] m_pBoneIndices;
	delete[] m_pWideTexCoords;
	delete[] m_pNormals;
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
	if ( m_pVertices )
		pica::PrepareWrite( m_pVertices );

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
	return m_bIsDynamic ? malloc( bytes ) : pica::AllocLinear( kind, bytes );
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
		float *weights = new float[capacity * 2];
		unsigned char *indices = new unsigned char[capacity * 4];
		if ( m_pBoneWeights )
		{
			memcpy( weights, m_pBoneWeights, m_nBoneCapacity * 2 * sizeof( float ) );
			memcpy( indices, m_pBoneIndices, m_nBoneCapacity * 4 );
		}
		delete[] m_pBoneWeights;
		delete[] m_pBoneIndices;
		m_pBoneWeights = weights;
		m_pBoneIndices = indices;
		m_nBoneCapacity = capacity;
	}
	if ( m_Format & VERTEX_NORMAL )
	{
		float *normals = new float[capacity * 3];
		memset( normals, 0, capacity * 3 * sizeof( float ) );
		if ( m_pNormals )
			memcpy( normals, m_pNormals, m_nNormalCapacity * 3 * sizeof( float ) );
		delete[] m_pNormals;
		m_pNormals = normals;
		m_nNormalCapacity = capacity;
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
	// Triangles, counterclockwise for the core (legacy meshes face clockwise).
	std::vector<std::uint32_t> triangles;
	const bool indexed = m_nIndices > 0 && indexCount > 0;
	const int count = indexed ? ( firstIndex + indexCount <= m_nIndices ? indexCount : 0 ) : src.m_nVertices;
	auto element = [&]( int i ) { return indexed ? int( m_pIndices[firstIndex + i] ) : i; };
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
			triangles.push_back( std::uint32_t( a ) );
			triangles.push_back( std::uint32_t( c ) );
			triangles.push_back( std::uint32_t( b ) );
		}
	};
	if ( m_Type == MATERIAL_TRIANGLES )
	{
		if ( count % 3 )
			return false;
		for ( int i = 0; i < count; i += 3 )
			triangle( element( i ), element( i + 1 ), element( i + 2 ) );
	}
	else if ( m_Type == MATERIAL_TRIANGLE_STRIP )
	{
		for ( int i = 0; i + 2 < count; ++i )
			triangle( element( i + ( i & 1 ) ), element( i + 1 - ( i & 1 ) ), element( i + 2 ) );
	}
	else
		return false;
	if ( !valid || triangles.empty() )
		return false;

	// World-space positions and normals: skinned by the bones, else by the model.
	const bool skinned = src.m_pBoneWeights && g_MaxBone > 0;
	const float *model = Top( kStackModel );
	std::vector<render::material::SurfaceWorldVertex> vertices( src.m_nVertices );
	for ( int i = 0; i < src.m_nVertices; ++i )
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

	// The material's variables, as the core's claim reads them.
	std::vector<render::legacy::CoreMeshVariable> variables;
	std::vector<std::string> texts;
	IMaterialVar **params = g_pBoundMaterial->GetShaderParams();
	IShader *shader = g_pBoundMaterial->GetShader();
	const int paramCount = g_pBoundMaterial->ShaderParamCount();
	texts.reserve( paramCount + 1 );
	for ( int i = 0; i < paramCount; ++i )
	{
		IMaterialVar *var = params[i];
		if ( !var || !var->IsDefined() || !V_strnicmp( var->GetName(), "$flags", 6 ) )
			continue;
		render::legacy::CoreMeshVariable value;
		value.key = var->GetName();
		texts.push_back( var->GetStringValue() );
		value.value = texts.back().c_str();
		if ( shader && i < shader->GetNumParams() && !V_stricmp( shader->GetParamName( i ), value.key ) )
			value.defaultValue = shader->GetParamDefault( i );
		if ( var->GetType() == MATERIAL_VAR_TYPE_TEXTURE && var->GetTextureValue() )
		{
			ITextureInternal *texture = static_cast<ITextureInternal *>( var->GetTextureValue() );
			value.textureHandle = int( texture->GetTextureHandle( 0 ) );
		}
		variables.push_back( value );
	}
	for ( const auto &flag : RenderLegacyMaterialFlags::Keys )
		if ( g_pBoundMaterial->GetMaterialVarFlag( flag.flag ) )
			variables.push_back( { flag.key, "1", "0", 0 } );

	render::legacy::CoreMeshDraw draw;
	draw.kind = render::legacy::CoreMeshKind::kModelSurface;
	draw.name = g_pBoundMaterial->GetName();
	draw.shader = g_pBoundMaterial->GetShaderName();
	draw.variables = variables.data();
	draw.variableCount = std::uint32_t( variables.size() );
	draw.vertices = vertices.data();
	draw.vertexCount = std::uint32_t( vertices.size() );
	draw.indices = triangles.data();
	draw.indexCount = std::uint32_t( triangles.size() );
	draw.mesh = true;
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
	// Row-major, column vectors (FamilyDrawConstants): the transposes of the
	// stacks' row-vector matrices; positions are already world space.
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
	const std::uint32_t tag = g_CorePassRecorder->QueueMesh( draw );
	if ( !tag )
		return skip( 4, "QueueMesh refused it" );
	g_PicaCorePassSlots.MarkSlot( tag );
	static unsigned s_taken = 0;
	if ( ++s_taken % 50 == 0 && s_taken <= 2000 )
	{
		const struct mallinfo heap = mallinfo();
		printf( "pica: %u core model draws, heap used %u KB, frame %d\n", s_taken,
			(unsigned)( heap.uordblks / 1024 ), g_PicaFrame );
	}
	return true;
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
	if ( EmitToCore( firstIndex, indexCount ) )
	{
		++g_Counters.coreMeshDraws;
		return;
	}
	pica::Primitive primitive = pica::Primitive::kTriangles;
	switch ( m_Type )
	{
	case MATERIAL_TRIANGLES:
	case MATERIAL_QUADS:
	case MATERIAL_POLYGON:
	case MATERIAL_INSTANCED_QUADS:
		break;
	case MATERIAL_TRIANGLE_STRIP:
		primitive = pica::Primitive::kTriangleStrip;
		break;
	default:
		return; // lines and points: the PICA draws triangles only
	}

	const PicaSnapshot *snapshot = ( g_CurrentSnapshot >= 0 && g_CurrentSnapshot < g_Snapshots.Count() ) ?
		&g_Snapshots[g_CurrentSnapshot] : NULL;
	pica::DrawState state = snapshot ? snapshot->state : pica::DrawState();
	memcpy( state.tint, g_Modulation, sizeof( state.tint ) );
	PicaTexture *texture = TextureFor( g_BoundTextures[0] );
	if ( texture && texture->dirty )
		UploadTexture( *texture );

	// clip = (model) * view * projection.
	float viewProj[16], clip[16];
	Mul( Top( kStackView ), Top( kStackProjection ), viewProj );

	const pica::Vertex *vertices = src.m_pVertices;
	const bool skinned = src.m_pBoneWeights && g_MaxBone > 0;
	if ( skinned || src.m_bIsDynamic )
	{
		// Transient copy: the dynamic mesh is rewritten by the next draw, and
		// skinning writes world-space positions.
		pica::Vertex *copy = static_cast<pica::Vertex *>(
			pica::AllocTransient( pica::Memory::kVertices, src.m_nVertices * sizeof( pica::Vertex ) ) );
		if ( !copy )
			return;
		memcpy( copy, src.m_pVertices, src.m_nVertices * sizeof( pica::Vertex ) );
		if ( skinned )
		{
			for ( int i = 0; i < src.m_nVertices; ++i )
			{
				const float *w = src.m_pBoneWeights + i * 2;
				const unsigned char *b = src.m_pBoneIndices + i * 4;
				const float weights[3] = { w[0], w[1], 1.0f - w[0] - w[1] };
				const float *p = src.m_pVertices[i].pos;
				float out[3] = { 0.0f, 0.0f, 0.0f };
				for ( int k = 0; k < 3; ++k )
				{
					if ( weights[k] <= 0.0f || b[k] >= kMaxBones )
						continue;
					const float *m = g_Bones[b[k]];
					for ( int r = 0; r < 3; ++r )
						out[r] += weights[k] * ( m[r * 4 + 0] * p[0] + m[r * 4 + 1] * p[1] + m[r * 4 + 2] * p[2] + m[r * 4 + 3] );
				}
				memcpy( copy[i].pos, out, sizeof( out ) );
			}
			memcpy( clip, viewProj, sizeof( clip ) );
		}
		pica::FlushLinear( copy, src.m_nVertices * sizeof( pica::Vertex ) );
		vertices = copy;
	}
	if ( !skinned )
		Mul( Top( kStackModel ), viewProj, clip );

	const unsigned short *indices = NULL;
	int count = 0;
	if ( m_nIndices > 0 && indexCount > 0 )
	{
		if ( firstIndex + indexCount > m_nIndices )
			indexCount = m_nIndices - firstIndex;
		if ( indexCount <= 0 )
			return;
		if ( m_bIsDynamic )
		{
			unsigned short *copy = static_cast<unsigned short *>(
				pica::AllocTransient( pica::Memory::kIndices, indexCount * sizeof( unsigned short ) ) );
			if ( !copy )
				return;
			memcpy( copy, m_pIndices + firstIndex, indexCount * sizeof( unsigned short ) );
			pica::FlushLinear( copy, indexCount * sizeof( unsigned short ) );
			indices = copy;
		}
		else
			indices = m_pIndices + firstIndex;
		count = indexCount;
	}
	else if ( m_Type == MATERIAL_QUADS || m_Type == MATERIAL_POLYGON )
	{
		// Unindexed quads and polygons: generate triangles.
		const bool quads = m_Type == MATERIAL_QUADS;
		const int triangles = quads ? ( src.m_nVertices / 4 ) * 2 : src.m_nVertices - 2;
		if ( triangles <= 0 )
			return;
		unsigned short *generated = static_cast<unsigned short *>(
			pica::AllocTransient( pica::Memory::kIndices, triangles * 3 * sizeof( unsigned short ) ) );
		if ( !generated )
			return;
		int n = 0;
		if ( quads )
			for ( int q = 0; q + 3 < src.m_nVertices; q += 4 )
			{
				const unsigned short quad[6] = { (unsigned short)q, (unsigned short)( q + 1 ), (unsigned short)( q + 2 ),
					(unsigned short)q, (unsigned short)( q + 2 ), (unsigned short)( q + 3 ) };
				memcpy( generated + n, quad, sizeof( quad ) );
				n += 6;
			}
		else
			for ( int t = 1; t + 1 < src.m_nVertices; ++t )
			{
				generated[n++] = 0;
				generated[n++] = (unsigned short)t;
				generated[n++] = (unsigned short)( t + 1 );
			}
		pica::FlushLinear( generated, n * sizeof( unsigned short ) );
		indices = generated;
		count = n;
	}
	if ( g_PicaFrame == g_PicaDumpFrame )
		DumpDraw( clip, state, texture ? texture->name : "", texture ? texture->width : 0, texture ? texture->height : 0, vertices, indices, count );
	pica::Draw( clip, state, texture ? &texture->gpu : NULL, vertices, src.m_nVertices, indices, count, primitive );
}

// One line per draw of the -pica_dump_draws frame: what it draws and where
// its vertices land in normalized device coordinates (row-vector clip).
void CEmptyMesh::DumpDraw( const float *clip, const pica::DrawState &state, const char *textureName, int textureWidth, int textureHeight, const pica::Vertex *vertices,
	const unsigned short *indices, int count ) const
{
	const CEmptyMesh &src = m_pVertexSource ? *m_pVertexSource : *this;
	float lo[2] = { 1e30f, 1e30f }, hi[2] = { -1e30f, -1e30f };
	int behind = 0;
	const int n = indices ? count : src.m_nVertices;
	for ( int k = 0; k < n; ++k )
	{
		const int i = indices ? indices[k] : k;
		if ( i >= src.m_nVertices )
			continue;
		const float *p = vertices[i].pos;
		float c[4];
		for ( int j = 0; j < 4; ++j )
			c[j] = p[0] * clip[j] + p[1] * clip[4 + j] + p[2] * clip[8 + j] + clip[12 + j];
		if ( c[3] <= 1e-6f )
		{
			++behind;
			continue;
		}
		for ( int j = 0; j < 2; ++j )
		{
			lo[j] = Min( lo[j], c[j] / c[3] );
			hi[j] = Max( hi[j], c[j] / c[3] );
		}
	}
	printf( "pica draw: %s prims %d tex %s %dx%d ndc x %.2f..%.2f y %.2f..%.2f behind %d color %02x%02x%02x%02x blend %d %d/%d atest %d/%d snap %d\n",
		g_pBoundMaterial ? g_pBoundMaterial->GetName() : "(none)", indices ? count / 3 : src.m_nVertices,
		textureName, textureWidth, textureHeight, lo[0], hi[0], lo[1], hi[1], behind,
		src.m_nVertices ? vertices[0].color[0] : 0, src.m_nVertices ? vertices[0].color[1] : 0,
		src.m_nVertices ? vertices[0].color[2] : 0, src.m_nVertices ? vertices[0].color[3] : 0,
		state.blend, int( state.src ), int( state.dst ), state.alphaTest, state.alphaRef, g_CurrentSnapshot );
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
		if ( SameSnapshot( g_Snapshots[i], g_ShaderShadow.m_State, g_ShaderShadow.m_VertexFormat ) )
			index = i;
	if ( index < 0 )
	{
		PicaSnapshot snapshot;
		snapshot.state = g_ShaderShadow.m_State;
		snapshot.format = g_ShaderShadow.m_VertexFormat;
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
}

void CShaderAPIEmpty::ClearColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	g_ClearColor = ( unsigned( r ) << 24 ) | ( unsigned( g ) << 16 ) | ( unsigned( b ) << 8 ) | 0xFF;
}

void CShaderAPIEmpty::ClearColor4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	g_ClearColor = ( unsigned( r ) << 24 ) | ( unsigned( g ) << 16 ) | ( unsigned( b ) << 8 ) | a;
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
