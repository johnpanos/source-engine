
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//===========================================================================//

#include "utlvector.h"
#include "materialsystem/imaterialsystem.h"
#include "imaterialinternal.h"
#include "IHardwareConfigInternal.h"
#include "shadersystem.h"
#include "shaderapi/ishaderutil.h"
#include "shaderapi/ishaderapi.h"
#include "materialsystem/imesh.h"
#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "tier1/tier1.h"
#include "materialsystem/idebugtextureinfo.h"
#include "materialsystem/deformations.h"
#include "render/legacy_shader_provider.h"
#include "vulkan_device.h"
#include "vtf/vtf.h"
#include "pixelwriter.h"
#include "shaderapi/commandbuffer.h"
#include "drawstatefixture.h"

#include <algorithm>
#include <array>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

//-----------------------------------------------------------------------------
// The single native Vulkan presentation context, brought up on SetMode() and
// torn down at shutdown. Owns the real instance/device/queues/swapchain. While
// the material path is still the empty stub (roadmap R32), this proves the
// backend genuinely brings up native Vulkan in-process and presents a frame.
//-----------------------------------------------------------------------------
static render_vulkan::CVulkanContext g_VulkanContext;

//-----------------------------------------------------------------------------
// Unimplemented-entry census.
//
// This backend still answers a large part of IShaderAPI with do-nothing bodies.
// A silent no-op is indistinguishable from correct behavior at the call site, so
// a wrong frame gives no clue which dropped call produced it. Every such body
// records itself here instead, and ReportUnimplementedEntries() prints the
// totals for a real run: that turns "the frame is wrong" into a ranked list of
// behavior the engine asked for and this backend did not perform.
//
// The table is keyed by the address of the __func__ literal, so a call costs a
// short pointer scan and never allocates.
//-----------------------------------------------------------------------------
struct UnimplementedEntry
{
	const char *name;
	uint64_t calls;
};
static UnimplementedEntry g_Unimplemented[256];
static int g_nUnimplemented = 0;

// Why the most recent material pass did not reach the device ("" = it did).
// Reset by RenderPass; reported in draw-state fixtures.
static const char *g_LastDropReason = "";

static void NoteUnimplemented( const char *name )
{
	for ( int i = 0; i < g_nUnimplemented; ++i )
	{
		if ( g_Unimplemented[i].name == name )
		{
			++g_Unimplemented[i].calls;
			return;
		}
	}
	if ( g_nUnimplemented < static_cast<int>( ARRAYSIZE( g_Unimplemented ) ) )
	{
		g_Unimplemented[g_nUnimplemented].name = name;
		g_Unimplemented[g_nUnimplemented].calls = 1;
		++g_nUnimplemented;
	}
}

#define VK_UNIMPLEMENTED() NoteUnimplemented( __func__ )

static void DropDraw( const char *reason )
{
	g_LastDropReason = reason;
	NoteUnimplemented( reason );
}

// Primitive types the engine submitted, indexed by MaterialPrimitiveType_t. Every
// native pipeline rasterizes a triangle list, so anything else here is geometry
// this backend draws as unrelated triangles.
static uint64_t g_PrimitiveTypeCounts[16];

// Base-texture residency at the moment each draw is emitted. A draw that samples
// the default white texture is untextured on screen, so these three totals say
// whether a blank frame is a rasterization problem or a texture-binding one.
// HDR state, owned as the D3D9 backend owns it: the engine enables HDR per map
// (SetHDREnabled) and mat_hdr_level selects full HDR. This backend implements
// HDR_TYPE_INTEGER (16-bit lightmap pages, tone-mapping scale in the shaders).
// The cvar has the D3D9 backend's name, default and flags, so the engine's copy
// and this one are the same setting.
static ConVar mat_hdr_level( "mat_hdr_level", "2", FCVAR_ARCHIVE );
static bool g_bHDREnabled = false;
// cLightScale.x, the linear tone-mapping scale (SetToneMappingScaleLinear).
static Vector g_ToneMappingScale( 1.0f, 1.0f, 1.0f );

static HDRType_t CurrentHDRType()
{
	return ( mat_hdr_level.GetInt() >= 2 && g_bHDREnabled ) ? HDR_TYPE_INTEGER : HDR_TYPE_NONE;
}

static int g_boundTextureHandle = -1;
// The lightmap page bound to sampler 1 for the current pass (-1 = none), and
// whether a BindTexture call is resolving a lightmap standard texture.
static int g_boundLightmapHandle = -1;
static bool g_BindingLightmap = false;
static uint64_t g_DrawsTextured = 0;   // bound a handle whose pixels were uploaded
static uint64_t g_DrawsUnuploaded = 0; // bound a handle that was never filled
static uint64_t g_DrawsUntextured = 0; // bound no texture at all -> default white
// How much of the run went through render targets, so the report shows
// whether offscreen passes are really honored rather than landing on the
// back buffer.
static uint64_t g_DrawsIntoTargets = 0;
static uint64_t g_TargetSwitches = 0;
static uint64_t g_TargetCopies = 0;
static uint64_t g_TargetCopiesDropped = 0;

// Per-texture identity and bind accounting. A draw that samples a texture the
// material system never filled is invisible in any per-call log, so record what
// each handle was created as and how often an unfilled one is actually bound:
// that names the textures whose upload path is missing.
struct TextureRecord
{
	std::string name;
	ImageFormat format = IMAGE_FORMAT_UNKNOWN;
	uint64_t bindsWhileEmpty = 0;
	uint64_t rejectedAsTarget = 0;
	int width = 0;
	int height = 0;
	// CPU copy of mip 0 that TexLock hands to the material system's pixel writer
	// (lightmap pages are written this way, a sub-rectangle at a time); TexUnlock
	// uploads it. Allocated on first lock.
	std::vector<uint8_t> lockSurface;
};
static std::vector<TextureRecord> g_TextureRecords;

static void NoteTextureCreated(
    int handle, const char *debugName, ImageFormat format, int width, int height )
{
	if ( handle < 0 )
		return;
	if ( static_cast<size_t>( handle ) >= g_TextureRecords.size() )
		g_TextureRecords.resize( static_cast<size_t>( handle ) + 1 );
	g_TextureRecords[static_cast<size_t>( handle )].name = debugName ? debugName : "(unnamed)";
	g_TextureRecords[static_cast<size_t>( handle )].format = format;
	g_TextureRecords[static_cast<size_t>( handle )].width = width;
	g_TextureRecords[static_cast<size_t>( handle )].height = height;
	g_TextureRecords[static_cast<size_t>( handle )].lockSurface.clear();
}

// True when the draw about to be emitted samples real material pixels. Both the
// "no texture bound" and the "bound a texture nothing ever filled" cases sample
// something this backend invented, so neither is a drawable surface.
// Whether the pipeline the current pass selected samples $basetexture at all.
// The vertex-color, greenify and constant-color pipelines do not, so a missing
// base texture is irrelevant to them and must not suppress their draws.
static bool g_SamplesBaseTexture = true;
// The viewport the material system last set, reported back by GetViewports.
// Width 0 means "the whole current target", which is what D3D9 resets it to
// whenever the render target changes.
static ShaderViewport_t g_Viewport;

// Blend mode and alpha-test reference the current pass selected (BeginPass).
static render_vulkan::CVulkanContext::DynRasterState g_CurrentRaster;
static float g_CurrentAlphaRef = -1.0f;
// CVulkanContext::kColorSrgb*/kFragment*/kVertex* flags of the pass being drawn.
static int g_CurrentColorFlags = 0;

// D3DRS_STENCIL* render state (IShaderAPI SetStencil*), D3D9's defaults.
struct StencilRenderState
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
static StencilRenderState g_Stencil;

// User clip planes (SetClipPlane/EnableClipPlane), in world space as D3D9 keeps
// them, the D3DRS_CLIPPLANEENABLE mask, and the user clip transform override.
static float g_ClipPlanesWorld[render_vulkan::CVulkanContext::kMaxClipPlanes][4];
static int g_ClipPlanesEnabled = 0;
static bool g_UserClipTransformOverride = false;
static float g_UserClipTransform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
// Set while ClearBuffersObeyStencil / PerformFullScreenStencilOperation draw
// their quad: D3D9 disables D3DRS_CLIPPLANEENABLE around it.
static bool g_ClipPlanesSuppressed = false;
// The clear color as the engine set it (ClearColor3ub/4ub), for the quad clear.
static unsigned char g_ClearColor[4] = { 0, 0, 0, 255 };

// The pixel-shader constant registers (SetPixelShaderConstant), as D3D9 holds
// them; PortalRefract reads c4.
static float g_psConstants[32][4];

// What the current pass's shader is, beyond the pipeline it selects: the
// PortalRefract STAGE combo (-1 for other shaders).
static int g_CurrentPortalStage = -1;
// IShaderAPI::CullMode, D3D9's default CCW.
static MaterialCullMode_t g_DesiredCullMode = MATERIAL_CULLMODE_CCW;

// Hardware skinning state, as CShaderAPIDx8 keeps it: the pose-to-world bone
// matrices (cModel[], 3x4 row-major; bone 0 is also the MODEL matrix) and the
// bone count the current draw blends (SetNumBoneWeights; 0 = not skinned).
enum
{
	kMaxBoneMatrices = 53 // cModel[53] in common_vs_fxc.h
};
static float g_BoneMatrices[kMaxBoneMatrices][12];
static int g_NumBoneWeights = 0;
namespace
{
void CommitModelViewProj();
void CommitViewProj();
} // namespace

static bool HasResidentBaseTexture()
{
	if ( !g_SamplesBaseTexture )
		return true;
	return g_boundTextureHandle >= 0 &&
	       g_VulkanContext.IsManagedTextureUploaded( g_boundTextureHandle );
}

static void NoteDrawTextureResidency()
{
	if ( g_VulkanContext.RenderTarget() >= 0 )
		++g_DrawsIntoTargets;
	if ( g_boundTextureHandle < 0 )
		++g_DrawsUntextured;
	else if ( g_VulkanContext.IsManagedTextureUploaded( g_boundTextureHandle ) )
		++g_DrawsTextured;
	else
	{
		++g_DrawsUnuploaded;
		if ( static_cast<size_t>( g_boundTextureHandle ) < g_TextureRecords.size() )
			++g_TextureRecords[static_cast<size_t>( g_boundTextureHandle )].bindsWhileEmpty;
	}
}

static void NotePrimitiveType( MaterialPrimitiveType_t type )
{
	const int index = static_cast<int>( type );
	if ( index >= 0 && index < static_cast<int>( ARRAYSIZE( g_PrimitiveTypeCounts ) ) )
		++g_PrimitiveTypeCounts[index];
}

static void ReportUnimplementedEntries()
{
	static const char *const kPrimitiveNames[] = { "POINTS", "LINES", "TRIANGLES", "TRIANGLE_STRIP",
	    "LINE_STRIP", "LINE_LOOP", "POLYGON", "QUADS", "INSTANCED_QUADS" };
	for ( int i = 0; i < static_cast<int>( ARRAYSIZE( kPrimitiveNames ) ); ++i )
	{
		if ( g_PrimitiveTypeCounts[i] )
		{
			fprintf( stderr, "[vulkan] primitive %-16s draws=%llu\n", kPrimitiveNames[i],
			    static_cast<unsigned long long>( g_PrimitiveTypeCounts[i] ) );
		}
	}

	fprintf( stderr, "[vulkan] draws textured=%llu unuploaded=%llu untextured=%llu\n",
	    static_cast<unsigned long long>( g_DrawsTextured ),
	    static_cast<unsigned long long>( g_DrawsUnuploaded ),
	    static_cast<unsigned long long>( g_DrawsUntextured ) );

	fprintf( stderr,
	    "[vulkan] render targets: draws=%llu switches=%llu copies=%llu copies-dropped=%llu\n",
	    static_cast<unsigned long long>( g_DrawsIntoTargets ),
	    static_cast<unsigned long long>( g_TargetSwitches ),
	    static_cast<unsigned long long>( g_TargetCopies ),
	    static_cast<unsigned long long>( g_TargetCopiesDropped ) );

	// The textures a draw sampled while still empty, worst first: these are the
	// materials whose pixels never reached this backend.
	std::vector<const TextureRecord *> empties;
	for ( const TextureRecord &rec : g_TextureRecords )
	{
		if ( rec.bindsWhileEmpty )
			empties.push_back( &rec );
	}
	std::sort( empties.begin(), empties.end(),
	    []( const TextureRecord *a, const TextureRecord *b )
	    {
		    return a->bindsWhileEmpty > b->bindsWhileEmpty;
	    } );
	fprintf( stderr, "[vulkan] %zu distinct textures were sampled while empty\n", empties.size() );
	for ( size_t i = 0; i < empties.size() && i < 20; ++i )
	{
		fprintf( stderr, "[vulkan]   empty-sampled binds=%-7llu fmt=%-3d %s\n",
		    static_cast<unsigned long long>( empties[i]->bindsWhileEmpty ),
		    static_cast<int>( empties[i]->format ), empties[i]->name.c_str() );
	}

	for ( const TextureRecord &rec : g_TextureRecords )
	{
		if ( rec.rejectedAsTarget )
			fprintf( stderr, "[vulkan]   rejected-as-target calls=%-7llu fmt=%-3d %s\n",
			    static_cast<unsigned long long>( rec.rejectedAsTarget ),
			    static_cast<int>( rec.format ), rec.name.c_str() );
	}
	fprintf(
	    stderr, "[vulkan] captured frame stream:\n%s", g_VulkanContext.DescribeStream().c_str() );
	auto textureName = []( int handle ) -> const char *
	{
		if ( handle >= 0 && static_cast<size_t>( handle ) < g_TextureRecords.size() )
			return g_TextureRecords[static_cast<size_t>( handle )].name.c_str();
		return "(none)";
	};
	static const char *const kKinds[] = { "draw", "clear", "copy" };
	for ( const auto &r : g_VulkanContext.DescribeStreamRecords() )
	{
		fprintf( stderr,
		    "[vulkan]   %-5s tgt=%-4d sh=%d blend=%d verts=%-6u mod=%.2f,%.2f,%.2f,%.2f "
		    "vcol=%.2f,%.2f,%.2f vp=%.0f,%.0f,%.0f,%.0f uv=[%.2f..%.2f,%.2f..%.2f] "
		    "xf=%.2f,%.2f,%.2f,%.2f/%.2f,%.2f,%.2f,%.2f tex=%s\n",
		    kKinds[r.kind], r.target, r.shaderIndex, r.raster.blend, r.vertexCount, r.modulation[0],
		    r.modulation[1], r.modulation[2], r.modulation[3], r.firstColor[0], r.firstColor[1],
		    r.firstColor[2], r.viewport[0], r.viewport[1], r.viewport[2], r.viewport[3], r.uvMin[0],
		    r.uvMax[0], r.uvMin[1], r.uvMax[1], r.texXform0[0], r.texXform0[1], r.texXform0[2],
		    r.texXform0[3], r.texXform1[0], r.texXform1[1], r.texXform1[2], r.texXform1[3],
		    r.kind == 0 ? textureName( r.texHandle ) : textureName( r.target ) );
	}

	// Most-called first: that ordering is the work list.
	for ( int i = 0; i < g_nUnimplemented; ++i )
	{
		for ( int j = i + 1; j < g_nUnimplemented; ++j )
		{
			if ( g_Unimplemented[j].calls > g_Unimplemented[i].calls )
			{
				UnimplementedEntry tmp = g_Unimplemented[i];
				g_Unimplemented[i] = g_Unimplemented[j];
				g_Unimplemented[j] = tmp;
			}
		}
	}
	for ( int i = 0; i < g_nUnimplemented; ++i )
	{
		fprintf( stderr, "[vulkan] unimplemented %-40s calls=%llu\n", g_Unimplemented[i].name,
		    static_cast<unsigned long long>( g_Unimplemented[i].calls ) );
	}
}

//-----------------------------------------------------------------------------
// The empty mesh
//-----------------------------------------------------------------------------
class CEmptyMesh : public IMesh
{
public:
	CEmptyMesh( bool bIsDynamic );
	virtual ~CEmptyMesh();

	// FIXME: Make this work! Unsupported methods of IIndexBuffer + IVertexBuffer
	virtual bool Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t &desc );
	virtual void Unlock( int nWrittenIndexCount, IndexDesc_t &desc );
	virtual void ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t &desc );
	virtual void ModifyEnd( IndexDesc_t &desc );
	virtual void Spew( int nIndexCount, const IndexDesc_t &desc );
	virtual void ValidateData( int nIndexCount, const IndexDesc_t &desc );
	virtual bool Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc );
	virtual void Unlock( int nVertexCount, VertexDesc_t &desc );
	virtual void Spew( int nVertexCount, const VertexDesc_t &desc );
	virtual void ValidateData( int nVertexCount, const VertexDesc_t &desc );
	virtual bool IsDynamic() const { return m_bIsDynamic; }
	virtual void BeginCastBuffer( VertexFormat_t format ) {}
	virtual void BeginCastBuffer( MaterialIndexFormat_t format ) {}
	virtual void EndCastBuffer() {}
	virtual int GetRoomRemaining() const { return 0; }
	virtual MaterialIndexFormat_t IndexFormat() const { return MATERIAL_INDEX_FORMAT_UNKNOWN; }

	void LockMesh( int numVerts, int numIndices, MeshDesc_t &desc );
	void UnlockMesh( int numVerts, int numIndices, MeshDesc_t &desc );

	void ModifyBeginEx( bool bReadOnly, int firstVertex, int numVerts, int firstIndex,
	    int numIndices, MeshDesc_t &desc );
	void ModifyBegin(
	    int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t &desc );
	void ModifyEnd( MeshDesc_t &desc );

	// returns the # of vertices (static meshes only)
	int VertexCount() const;

	// Where this mesh's vertices and indices come from for the next draw. The
	// world draws a dynamic index list over a static mesh's vertices
	// (GetDynamicMesh with a vertex override), and a batch reuses the indices
	// just built into the dynamic mesh itself (the index override is the mesh).
	void SetSources( IMesh *pVertexOverride, IMesh *pIndexOverride );

	// Sets the primitive type
	void SetPrimitiveType( MaterialPrimitiveType_t type );

	// Draws the entire mesh
	void Draw( int firstIndex, int numIndices );

	void Draw( CPrimList *pPrims, int nPrims );

	// Queue the current geometry (the range recorded by the last Draw) to the
	// native Vulkan dynamic path, with whatever shader/material state is currently
	// selected. Called from CShaderAPIVulkan::RenderPass, i.e. AFTER the material's
	// shader has run BeginPass and set its constants/textures.
	void EmitToNativeQueue();

	// Copy verts and/or indices to a mesh builder. This only works for temp meshes!
	virtual void CopyToMeshBuilder( int iStartVert, // Which vertices to copy.
	    int nVerts,
	    int iStartIndex, // Which indices to copy.
	    int nIndices,
	    int indexOffset, // This is added to each index.
	    CMeshBuilder &builder );

	// Spews the mesh data
	void Spew( int numVerts, int numIndices, const MeshDesc_t &desc );

	void ValidateData( int numVerts, int numIndices, const MeshDesc_t &desc );

	// gets the associated material
	IMaterial *GetMaterial();

	void SetColorMesh( IMesh *pColorMesh, int nVertexOffset ) {}

	virtual int IndexCount() const { return m_numIndices; }

	virtual void SetFlexMesh( IMesh *pMesh, int nVertexOffset ) {}

	virtual void DisableFlexMesh() {}

	virtual void MarkAsDrawn() {}

	virtual unsigned ComputeMemoryUsed() { return 0; }

	virtual VertexFormat_t GetVertexFormat() const { return VERTEX_POSITION; }

	virtual IMesh *GetMesh() { return this; }

private:
	enum
	{
		// Upper bounds on a single lock. A static mesh sizes its storage to what
		// it locks; these only keep a corrupt count from exhausting memory.
		kMaxLockVertices = 4 * 1024 * 1024,
		kMaxLockIndices = 16 * 1024 * 1024
	};

	// Each mesh owns its geometry. Static meshes (world, models) are built once at
	// load and drawn many times, so they must not share storage: when they did,
	// every build overwrote the last and every draw replayed whichever mesh was
	// locked most recently.
	std::vector<unsigned char> m_vertexData;
	// Index list the mesh builder writes into (unsigned short). Draw() uses it to
	// assemble triangles in the authored order.
	std::vector<unsigned short> m_indexData;
	bool m_bIsDynamic;
	// Draw-time sources (SetSources); null means this mesh's own storage.
	CEmptyMesh *m_pVertexSource = nullptr;
	CEmptyMesh *m_pIndexSource = nullptr;
	// Vertices locked into m_vertexData as an interleaved position(vec3) +
	// color(4 bytes) layout with stride kMeshVertexStride, so Draw() can forward
	// real geometry to the native Vulkan dynamic-mesh path.
	int m_numVerts = 0;
	// Number of indices the last lock/unlock recorded (0 = non-indexed).
	int m_numIndices = 0;
	// Index range recorded by the last Draw() call, replayed by EmitToNativeQueue.
	int m_drawFirst = 0;
	int m_drawCount = 0;
	// Topology the mesh builder declared for this geometry. The native pipelines
	// all rasterize a triangle list, so EmitToNativeQueue assembles the triangles
	// this type implies rather than assuming the indices already form one.
	MaterialPrimitiveType_t m_primitiveType = MATERIAL_TRIANGLES;
	// Scratch target for vertex components this bounded layout does not carry, so
	// a mesh builder writing them (with size 0) never corrupts position/color.
	unsigned char m_dummyComponent[64] = { 0 };

public:
	// 12 bytes position + 4 bytes color + 8 bytes texcoord0 + 8 bytes texcoord1
	// + 8 bytes bone weights (two floats) + 4 bytes bone indices + 12 bytes
	// normal + 16 bytes user data (the TANGENT stream, binormal sign in w).
	enum
	{
		kMeshVertexStride = 72,
		kMeshBoneWeightOffset = 32,
		kMeshBoneIndexOffset = 40,
		kMeshNormalOffset = 44,
		kMeshUserDataOffset = 56
	};
};

// Every live mesh this backend created. Overrides arrive as IMesh pointers; this
// is how one is confirmed to be ours before its storage is read, instead of
// trusting a downcast of whatever the caller passed. Constructed on first use
// and never destroyed, because meshes are members of global objects whose
// construction and destruction order relative to this file's globals is not
// fixed.
static std::unordered_set<const IMesh *> &LiveMeshes()
{
	static auto *s_pLive = new std::unordered_set<const IMesh *>();
	return *s_pLive;
}

static CEmptyMesh *AsOwnMesh( IMesh *pMesh )
{
	if ( !pMesh || LiveMeshes().find( pMesh ) == LiveMeshes().end() )
		return nullptr;
	return static_cast<CEmptyMesh *>( pMesh );
}

//-----------------------------------------------------------------------------
// The empty shader shadow
//-----------------------------------------------------------------------------
class CShaderShadowVulkan : public IShaderShadow
{
public:
	CShaderShadowVulkan();
	virtual ~CShaderShadowVulkan();

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
	void VertexShaderVertexFormat(
	    unsigned int nFlags, int nTexCoordCount, int *pTexCoordDimensions, int nUserDataSize );

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
	void CustomTextureOperation( TextureStage_t stage, ShaderTexChannel_t channel, ShaderTexOp_t op,
	    ShaderTexArg_t arg1, ShaderTexArg_t arg2 );

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
	void EnableSRGBWrite( bool bEnable );

	void EnableSRGBRead( Sampler_t stage, bool bEnable );

	virtual void FogMode( ShaderFogMode_t fogMode ) {}

	virtual void DisableFogGammaCorrection( bool bDisable ) {}

	virtual void SetDiffuseMaterialSource( ShaderMaterialSource_t materialSource ) {}

	virtual void SetMorphFormat( MorphFormat_t flags ) {}

	virtual void EnableStencil( bool bEnable ) {}
	virtual void StencilFunc( ShaderStencilFunc_t stencilFunc ) {}
	virtual void StencilPassOp( ShaderStencilOp_t stencilOp ) {}
	virtual void StencilFailOp( ShaderStencilOp_t stencilOp ) {}
	virtual void StencilDepthFailOp( ShaderStencilOp_t stencilOp ) {}
	virtual void StencilReference( int nReference ) {}
	virtual void StencilMask( int nMask ) {}
	virtual void StencilWriteMask( int nMask ) {}

	virtual void ExecuteCommandBuffer( uint8 *pBuf ) {}
	// Alpha to coverage
	void EnableAlphaToCoverage( bool bEnable );

	virtual void SetShadowDepthFiltering( Sampler_t stage ) {}

	virtual void BlendOp( ShaderBlendOp_t blendOp ) {}
	virtual void BlendOpSeparateAlpha( ShaderBlendOp_t blendOp ) {}

	bool m_IsTranslucent;
	bool m_IsAlphaTested;
	bool m_bIsDepthWriteEnabled;
	bool m_bIsDepthTestEnabled = true;
	ShaderDepthFunc_t m_depthFunc = SHADER_DEPTHFUNC_NEAREROREQUAL;
	bool m_bUsesVertexAndPixelShaders;
	// Blend factors recorded during snapshot state (IShaderShadow::BlendFunc);
	// TakeSnapshot turns them, with the depth state, into the native raster state.
	ShaderBlendFactor_t m_blendSrc = SHADER_BLEND_ONE;
	ShaderBlendFactor_t m_blendDst = SHADER_BLEND_ZERO;
	// $alphatest reference [0,1] and comparison recorded by AlphaFunc; applied
	// only when EnableAlphaTest set m_IsAlphaTested. D3D9's default is GEQUAL.
	float m_alphaRef = 0.0f;
	ShaderAlphaFunc_t m_alphaFunc = SHADER_ALPHAFUNC_GEQUAL;
	// IShaderShadow::EnableColorWrites.
	bool m_colorWrites = true;
	// IShaderShadow::EnableCulling ($nocull turns it off); on by default, as in
	// CShaderShadowDX8::SetDefaultState.
	bool m_cullEnable = true;
	// Vertex components the snapshot's vertex shader reads
	// (VertexShaderVertexFormat); the material system sizes its meshes from it.
	VertexFormat_t m_vertexUsage = 0;
	// CVulkanContext::kColorSrgb* inputs/output this snapshot declared sRGB.
	int m_colorFlags = 0;
	// Selected pixel shader recorded during snapshot state (IShaderShadow), so a
	// snapshot can carry which material shader to bind at draw time.
	char m_pixelShaderName[64] = { 0 };
	// Its static combo index (the STAGE of portal_refract_ps2x, for one).
	int m_pixelShaderIndex = 0;
	// Selected vertex shader; screenspaceeffect_vs20 positions in clip space.
	char m_vertexShaderName[64] = { 0 };
};

//-----------------------------------------------------------------------------
// The DX8 implementation of the shader device
//-----------------------------------------------------------------------------
class CShaderDeviceVulkan : public IShaderDevice
{
public:
	CShaderDeviceVulkan() : m_DynamicMesh( true ), m_Mesh( false ) {}

	// Methods of IShaderDevice
	virtual int GetCurrentAdapter() const { return 0; }
	// True once the native Vulkan device is up: this is a real rendering backend,
	// so the material system must drive its actual draw path (mesh Draw, shader
	// binding, present) rather than treating it as a headless/null device.
	virtual bool IsUsingGraphics() const { return g_VulkanContext.IsValid(); }
	virtual void SpewDriverInfo() const;
	virtual ImageFormat GetBackBufferFormat() const { return IMAGE_FORMAT_RGB888; }
	virtual void GetBackBufferDimensions( int &width, int &height ) const;
	virtual int StencilBufferBits() const { return g_VulkanContext.StencilBits(); }
	virtual bool IsAAEnabled() const { return false; }
	virtual void Present()
	{
		// Present a real native Vulkan frame: the clear, any material-facing mesh
		// geometry queued this frame (IMesh::Draw), and the selected material
		// shader (BeginPass) are all recorded and submitted here. Material state,
		// textures and the full shader library remain to be wired (roadmap R32).
		if ( !g_VulkanContext.IsValid() )
			return;
		std::string error;
		bool skip = false;
		if ( g_VulkanContext.BeginFrame( &skip, &error ) )
		{
			if ( !skip )
				g_VulkanContext.EndFrame( &error );
		}
		g_VulkanContext.EndStreamFrame();
	}
	virtual void GetWindowSize( int &width, int &height ) const;
	virtual bool AddView( void *hwnd );
	virtual void RemoveView( void *hwnd );
	virtual void SetView( void *hwnd );
	virtual void ReleaseResources();
	virtual void ReacquireResources();
	virtual IMesh *CreateStaticMesh(
	    VertexFormat_t fmt, const char *pTextureBudgetGroup, IMaterial *pMaterial = NULL );
	virtual void DestroyStaticMesh( IMesh *mesh );
	virtual IShaderBuffer *CompileShader(
	    const char *pProgram, size_t nBufLen, const char *pShaderVersion )
	{
		return NULL;
	}
	virtual VertexShaderHandle_t CreateVertexShader( IShaderBuffer *pShaderBuffer )
	{
		return VERTEX_SHADER_HANDLE_INVALID;
	}
	virtual void DestroyVertexShader( VertexShaderHandle_t hShader ) {}
	virtual GeometryShaderHandle_t CreateGeometryShader( IShaderBuffer *pShaderBuffer )
	{
		return GEOMETRY_SHADER_HANDLE_INVALID;
	}
	virtual void DestroyGeometryShader( GeometryShaderHandle_t hShader ) {}
	virtual PixelShaderHandle_t CreatePixelShader( IShaderBuffer *pShaderBuffer )
	{
		return PIXEL_SHADER_HANDLE_INVALID;
	}
	virtual void DestroyPixelShader( PixelShaderHandle_t hShader ) {}
	virtual IVertexBuffer *CreateVertexBuffer(
	    ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pBudgetGroup );
	virtual void DestroyVertexBuffer( IVertexBuffer *pVertexBuffer );
	virtual IIndexBuffer *CreateIndexBuffer( ShaderBufferType_t bufferType,
	    MaterialIndexFormat_t fmt, int nIndexCount, const char *pBudgetGroup );
	virtual void DestroyIndexBuffer( IIndexBuffer *pIndexBuffer );
	virtual IVertexBuffer *GetDynamicVertexBuffer(
	    int streamID, VertexFormat_t vertexFormat, bool bBuffered );
	virtual IIndexBuffer *GetDynamicIndexBuffer( MaterialIndexFormat_t fmt, bool bBuffered );
	virtual void SetHardwareGammaRamp( float fGamma, float fGammaTVRangeMin, float fGammaTVRangeMax,
	    float fGammaTVExponent, bool bTVEnabled )
	{
		VK_UNIMPLEMENTED();
	}
	virtual void EnableNonInteractiveMode(
	    MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo )
	{
		VK_UNIMPLEMENTED();
	}
	virtual void RefreshFrontBufferNonInteractive() {}
	virtual void HandleThreadEvent( uint32 threadEvent ) {}

#ifdef DX_TO_GL_ABSTRACTION
	virtual void DoStartupShaderPreloading( void ) {}
#endif

	virtual char *GetDisplayDeviceName() OVERRIDE { return ""; }

private:
	CEmptyMesh m_Mesh;
	CEmptyMesh m_DynamicMesh;
};

static CShaderDeviceVulkan s_ShaderDeviceEmpty;

// FIXME: Remove; it's for backward compat with the materialsystem only for now
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(
    CShaderDeviceVulkan, IShaderDevice, SHADER_DEVICE_INTERFACE_VERSION, s_ShaderDeviceEmpty )

//-----------------------------------------------------------------------------
// The DX8 implementation of the shader device
//-----------------------------------------------------------------------------
class CShaderDeviceMgrVulkan : public IShaderDeviceMgr
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
	virtual int GetAdapterCount() const;
	virtual void GetAdapterInfo( int adapter, MaterialAdapterInfo_t &info ) const;
	virtual bool GetRecommendedConfigurationInfo(
	    int nAdapter, int nDXLevel, KeyValues *pKeyValues );
	virtual int GetModeCount( int adapter ) const;
	virtual void GetModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter, int mode ) const;
	virtual void GetCurrentModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter ) const;
	virtual bool SetAdapter( int nAdapter, int nFlags );
	virtual CreateInterfaceFn SetMode( void *hWnd, int nAdapter, const ShaderDeviceInfo_t &mode );
	virtual void AddModeChangeCallback( ShaderModeChangeCallbackFunc_t func ) {}
	virtual void RemoveModeChangeCallback( ShaderModeChangeCallbackFunc_t func ) {}
};

static CShaderDeviceMgrVulkan s_ShaderDeviceMgrEmpty;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderDeviceMgrVulkan, IShaderDeviceMgr,
    SHADER_DEVICE_MGR_INTERFACE_VERSION, s_ShaderDeviceMgrEmpty )

//-----------------------------------------------------------------------------
// The DX8 implementation of the shader API
//-----------------------------------------------------------------------------
class CShaderAPIVulkan : public IShaderAPI, public IHardwareConfigInternal, public IDebugTextureInfo
{
public:
	// constructor, destructor
	CShaderAPIVulkan();
	virtual ~CShaderAPIVulkan();

	// IDebugTextureInfo implementation.
public:
	virtual bool IsDebugTextureListFresh( int numFramesAllowed = 1 ) { return false; }
	virtual bool SetDebugTextureRendering( bool bEnable ) { return false; }
	virtual void EnableDebugTextureList( bool bEnable ) {}
	virtual void EnableGetAllTextures( bool bEnable ) {}
	virtual KeyValues *GetDebugTextureList() { return NULL; }
	virtual int GetTextureMemoryUsed( TextureMemoryType eTextureMemory ) { return 0; }

	// Methods of IShaderDynamicAPI
	virtual void GetBackBufferDimensions( int &width, int &height ) const
	{
		s_ShaderDeviceEmpty.GetBackBufferDimensions( width, height );
	}
	virtual void GetCurrentColorCorrection( ShaderColorCorrectionInfo_t *pInfo )
	{
		pInfo->m_bIsEnabled = false;
		pInfo->m_nLookupCount = 0;
		pInfo->m_flDefaultWeight = 0.0f;
	}

	// Methods of IShaderAPI
public:
	virtual void SetViewports( int nCount, const ShaderViewport_t *pViewports );
	virtual int GetViewports( ShaderViewport_t *pViewports, int nMax ) const;
	virtual void ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil,
	    int renderTargetWidth, int renderTargetHeight );
	virtual void ClearColor3ub( unsigned char r, unsigned char g, unsigned char b );
	virtual void ClearColor4ub(
	    unsigned char r, unsigned char g, unsigned char b, unsigned char a );
	virtual void BindVertexShader( VertexShaderHandle_t hVertexShader ) {}
	virtual void BindGeometryShader( GeometryShaderHandle_t hGeometryShader ) {}
	virtual void BindPixelShader( PixelShaderHandle_t hPixelShader ) {}
	virtual void SetRasterState( const ShaderRasterState_t &state ) {}
	virtual void MarkUnusedVertexFields(
	    unsigned int nFlags, int nTexCoordCount, bool *pUnusedTexCoords )
	{
		VK_UNIMPLEMENTED();
	}
	virtual bool OwnGPUResources( bool bEnable ) { return false; }

	virtual bool DoRenderTargetsNeedSeparateDepthBuffer() const;

	// Used to clear the transition table when we know it's become invalid.
	void ClearSnapshots();

	// Sets the mode...
	bool SetMode( void *hwnd, int nAdapter, const ShaderDeviceInfo_t &info )
	{
		// This is the entry point the material system actually calls
		// (CMaterialSystem::SetMode -> g_pShaderAPI->SetMode). Bring up the native
		// Vulkan device/surface/swapchain against the engine's SDL window and the
		// material-facing dynamic-mesh pipelines here.
		if ( g_VulkanContext.IsValid() )
			return true;

		render_vulkan::VulkanContextConfig config;
		config.appName = "Source Engine Native Vulkan";
		config.enableValidation = ( CommandLine()->FindParm( "-vkvalidate" ) != 0 );
		config.framesInFlight = 2;

		std::string error;
		if ( !g_VulkanContext.Init( static_cast<SDL_Window *>( hwnd ), config, &error ) )
		{
			Warning( "[NativeVulkan] IShaderAPI::SetMode bring-up failed: %s\n", error.c_str() );
			return false;
		}
		int w = 0, h = 0;
		g_VulkanContext.GetSwapchainExtent( w, h );
		Msg( "[NativeVulkan] IShaderAPI::SetMode: device '%s' up (%dx%d)\n",
		    g_VulkanContext.DeviceName(), w, h );
		if ( !g_VulkanContext.InitDynamicMesh( &error ) )
			Warning( "[NativeVulkan] dynamic mesh pipelines unavailable: %s\n", error.c_str() );
		return true;
	}

	void ChangeVideoMode( const ShaderDeviceInfo_t &info ) {}

	// Called when the dx support level has changed
	virtual void DXSupportLevelChanged() {}

	virtual void EnableUserClipTransformOverride( bool bEnable );
	virtual void UserClipTransform( const VMatrix &worldToView );

	// Sets the default *dynamic* state
	void SetDefaultState();

	// Returns the snapshot id for the shader state
	StateSnapshot_t TakeSnapshot();

	// Returns true if the state snapshot is transparent
	bool IsTranslucent( StateSnapshot_t id ) const;
	bool IsAlphaTested( StateSnapshot_t id ) const;
	bool UsesVertexAndPixelShaders( StateSnapshot_t id ) const;
	virtual bool IsDepthWriteEnabled( StateSnapshot_t id ) const;

	// Gets the vertex format for a set of snapshot ids
	VertexFormat_t ComputeVertexFormat( int numSnapshots, StateSnapshot_t *pIds ) const;

	// Gets the vertex format for a set of snapshot ids
	VertexFormat_t ComputeVertexUsage( int numSnapshots, StateSnapshot_t *pIds ) const;

	// Begins a rendering pass that uses a state snapshot
	void BeginPass( StateSnapshot_t snapshot );

	// Uses a state snapshot
	void UseSnapshot( StateSnapshot_t snapshot );

	// Use this to get the mesh builder that allows us to modify vertex data
	CMeshBuilder *GetVertexModifyBuilder();

	// Sets the color to modulate by
	void Color3f( float r, float g, float b );
	void Color3fv( float const *pColor );
	void Color4f( float r, float g, float b, float a );
	void Color4fv( float const *pColor );

	// Faster versions of color
	void Color3ub( unsigned char r, unsigned char g, unsigned char b );
	void Color3ubv( unsigned char const *rgb );
	void Color4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a );
	void Color4ubv( unsigned char const *rgba );

	// Sets the lights
	void SetLight( int lightNum, const LightDesc_t &desc );
	void SetLightingOrigin( Vector vLightingOrigin );
	void SetAmbientLight( float r, float g, float b );
	void SetAmbientLightCube( Vector4D cube[6] );

	// Get the lights
	int GetMaxLights( void ) const;
	const LightDesc_t &GetLight( int lightNum ) const;

	// Render state for the ambient light cube (vertex shaders)
	void SetVertexShaderStateAmbientLightCube();
	void SetPixelShaderStateAmbientLightCube( int pshReg, bool bForceToBlack = false ) {}

	float GetAmbientLightCubeLuminance( void ) { return 0.0f; }

	void SetSkinningMatrices();

	// Lightmap texture binding
	void BindLightmap( TextureStage_t stage );
	void BindLightmapAlpha( TextureStage_t stage ) {}
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

	void CopyRenderTargetToTextureEx(
	    ShaderAPITextureHandle_t texID, int nRenderTargetID, Rect_t *pSrcRect, Rect_t *pDstRect );

	void CopyTextureToRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t textureHandle,
	    Rect_t *pSrcRect, Rect_t *pDstRect )
	{
		VK_UNIMPLEMENTED();
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
	IMesh *GetDynamicMesh( IMaterial *pMaterial, int nHWSkinBoneCount, bool buffered,
	    IMesh *pVertexOverride, IMesh *pIndexOverride );
	IMesh *GetDynamicMeshEx( IMaterial *pMaterial, VertexFormat_t fmt, int nHWSkinBoneCount,
	    bool buffered, IMesh *pVertexOverride, IMesh *pIndexOverride );

	IMesh *GetFlexMesh();

	// Renders a single pass of a material
	void RenderPass( int nPass, int nPassCount );
	// Records the pass just rendered as a draw-state fixture.
	void RecordDrawStateFixture( int nPass, int nPassCount );

	// stuff related to matrix stacks
	void MatrixMode( MaterialMatrixMode_t matrixMode );
	void PushMatrix();
	void PopMatrix();
	void LoadMatrix( float *m );
	void LoadBoneMatrix( int boneIndex, const float *m );
	void MultMatrix( float *m );
	void MultMatrixLocal( float *m );
	void GetMatrix( MaterialMatrixMode_t matrixMode, float *dst );
	void LoadIdentity( void );
	void LoadCameraToWorld( void );
	void Ortho( double left, double top, double right, double bottom, double zNear, double zFar );
	void PerspectiveX( double fovx, double aspect, double zNear, double zFar );
	void PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar,
	    double bottom, double top, double left, double right );
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
	void FogColor3fv( float const *rgb );
	void FogColor3ub( unsigned char r, unsigned char g, unsigned char b );
	void FogColor3ubv( unsigned char const *rgb );

	virtual void SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b );
	virtual void SceneFogMode( MaterialFogMode_t fogMode );
	virtual void GetSceneFogColor( unsigned char *rgb );
	virtual MaterialFogMode_t GetSceneFogMode();
	virtual int GetPixelFogCombo();

	void SetHeightClipZ( float z );
	void SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode );

	void SetClipPlane( int index, const float *pPlane );
	void EnableClipPlane( int index, bool bEnable );

	void SetFastClipPlane( const float *pPlane );
	void EnableFastClip( bool bEnable );

	// We use smaller dynamic VBs during level transitions, to free up memory
	virtual int GetCurrentDynamicVBSize( void );
	virtual void DestroyVertexBuffers( bool bExitingLevel = false );

	// Sets the vertex and pixel shaders
	void SetVertexShaderIndex( int vshIndex );
	void SetPixelShaderIndex( int pshIndex );

	// Sets the constant register for vertex and pixel shaders
	void SetVertexShaderConstant(
	    int var, float const *pVec, int numConst = 1, bool bForce = false );
	void SetBooleanVertexShaderConstant(
	    int var, BOOL const *pVec, int numConst = 1, bool bForce = false );
	void SetIntegerVertexShaderConstant(
	    int var, int const *pVec, int numConst = 1, bool bForce = false );
	void SetPixelShaderConstant(
	    int var, float const *pVec, int numConst = 1, bool bForce = false );
	void SetBooleanPixelShaderConstant(
	    int var, BOOL const *pVec, int numBools = 1, bool bForce = false );
	void SetIntegerPixelShaderConstant(
	    int var, int const *pVec, int numIntVecs = 1, bool bForce = false );

	void InvalidateDelayedShaderConstants( void );

	// Gamma<->Linear conversions according to the video hardware we're running on
	float GammaToLinear_HardwareSpecific( float fGamma ) const;
	float LinearToGamma_HardwareSpecific( float fLinear ) const;

	//Set's the linear->gamma conversion textures to use for this hardware for both srgb writes enabled and disabled(identity)
	void SetLinearToGammaConversionTextures( ShaderAPITextureHandle_t hSRGBWriteEnabledTexture,
	    ShaderAPITextureHandle_t hIdentityTexture );

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
	void Bind( IMaterial *pMaterial );

	// Returns the nearest supported format
	ImageFormat GetNearestSupportedFormat( ImageFormat fmt, bool bFilteringRequired = true ) const;
	ImageFormat GetNearestRenderTargetFormat( ImageFormat fmt ) const;

	// Sets the texture state
	void BindTexture( Sampler_t stage, ShaderAPITextureHandle_t textureHandle );

	void SetRenderTarget(
	    ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle )
	{
		SetRenderTargetEx( 0, colorTextureHandle, depthTextureHandle );
	}

	void SetRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t colorTextureHandle,
	    ShaderAPITextureHandle_t depthTextureHandle );

	// Indicates we're going to be modifying this texture
	// TexImage2D, TexSubImage2D, TexWrap, TexMinFilter, and TexMagFilter
	// all use the texture specified by this function.
	void ModifyTexture( ShaderAPITextureHandle_t textureHandle );

	// Texture management methods
	void TexImage2D( int level, int cubeFace, ImageFormat dstFormat, int zOffset, int width,
	    int height, ImageFormat srcFormat, bool bSrcIsTiled, void *imageData );
	void TexSubImage2D( int level, int cubeFace, int xOffset, int yOffset, int zOffset, int width,
	    int height, ImageFormat srcFormat, int srcStride, bool bSrcIsTiled, void *imageData );

	void TexImageFromVTF( IVTFTexture *pVTF, int iVTFFrame );

	bool TexLock( int level, int cubeFaceID, int xOffset, int yOffset, int width, int height,
	    CPixelWriter &writer );
	void TexUnlock();

	// These are bound to the texture, not the texture environment
	void TexMinFilter( ShaderTexFilterMode_t texFilterMode );
	void TexMagFilter( ShaderTexFilterMode_t texFilterMode );
	void TexWrap( ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode );
	void TexSetPriority( int priority );

	ShaderAPITextureHandle_t CreateTexture( int width, int height, int depth,
	    ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags,
	    const char *pDebugName, const char *pTextureGroupName );
	// Create a multi-frame texture (equivalent to calling "CreateTexture" multiple times, but more efficient)
	void CreateTextures( ShaderAPITextureHandle_t *pHandles, int count, int width, int height,
	    int depth, ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags,
	    const char *pDebugName, const char *pTextureGroupName );
	ShaderAPITextureHandle_t CreateDepthTexture(
	    ImageFormat renderFormat, int width, int height, const char *pDebugName, bool bTexture );
	void DeleteTexture( ShaderAPITextureHandle_t textureHandle );
	bool IsTexture( ShaderAPITextureHandle_t textureHandle );
	bool IsTextureResident( ShaderAPITextureHandle_t textureHandle );

	// stuff that isn't to be used from within a shader
	void ClearBuffersObeyStencil( bool bClearColor, bool bClearDepth );
	void ClearBuffersObeyStencilEx( bool bClearColor, bool bClearAlpha, bool bClearDepth );
	void PerformFullScreenStencilOperation( void );
	void ReadPixels(
	    int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat );
	virtual void ReadPixels( Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data,
	    ImageFormat dstFormat, int nDstStride );

	// Selection mode methods
	int SelectionMode( bool selectionMode );
	void SelectionBuffer( unsigned int *pBuffer, int size );
	void ClearSelectionNames();
	void LoadSelectionName( int name );
	void PushSelectionName( int name );
	void PopSelectionName();

	void FlushHardware();
	void ResetRenderState( bool bFullReset = true );

	void SetScissorRect( const int nLeft, const int nTop, const int nRight, const int nBottom,
	    const bool bEnableScissor );

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
	void GetWorldSpaceCameraPosition( float *pPos ) const;

	// Members of IMaterialSystemHardwareConfig
	bool HasDestAlphaBuffer() const;
	bool HasStencilBuffer() const;
	virtual int MaxViewports() const;
	virtual void OverrideStreamOffsetSupport( bool bOverrideEnabled, bool bEnableSupport ) {}
	virtual int GetShadowFilterMode() const;
	int StencilBufferBits() const;
	int GetFrameBufferColorDepth() const;
	int GetSamplerCount() const;
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
	int MaximumAnisotropicLevel() const;
	int MaxTextureWidth() const;
	int MaxTextureHeight() const;
	int MaxTextureAspectRatio() const;
	int GetDXSupportLevel() const;
	const char *GetShaderDLLName() const { return "UNKNOWN"; }
	int TextureMemorySize() const;
	bool SupportsOverbright() const;
	bool SupportsCubeMaps() const;
	bool SupportsMipmappedCubemaps() const;
	bool SupportsNonPow2Textures() const;
	int GetTextureStageCount() const;
	int NumVertexShaderConstants() const;
	int NumBooleanVertexShaderConstants() const;
	int NumIntegerVertexShaderConstants() const;
	int NumPixelShaderConstants() const;
	int MaxNumLights() const;
	bool SupportsHardwareLighting() const;
	int MaxBlendMatrices() const;
	int MaxBlendMatrixIndices() const;
	int MaxVertexShaderBlendMatrices() const;
	int MaxUserClipPlanes() const;
	bool UseFastClipping() const { return false; }
	bool SpecifiesFogColorInLinearSpace() const;
	virtual bool SupportsSRGB() const;
	virtual bool FakeSRGBWrite() const;
	virtual bool CanDoSRGBReadFromRTs() const;
	virtual bool SupportsGLMixedSizeTargets() const;

	const char *GetHWSpecificShaderDLLName() const;
	bool NeedsAAClamp() const { return false; }
	bool SupportsSpheremapping() const;
	virtual int MaxHWMorphBatchCount() const { return 0; }

	// This is the max dx support level supported by the card
	virtual int GetMaxDXSupportLevel() const;

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
	void DisableTextureTransform( TextureStage_t textureStage ) {}
	void SetBumpEnvMatrix(
	    TextureStage_t textureStage, float m00, float m01, float m10, float m11 );

	// Gets the lightmap dimensions
	virtual void GetLightmapDimensions( int *w, int *h );

	virtual void SyncToken( const char *pToken );

	// Setup standard vertex shader constants (that don't change)
	// This needs to be called anytime that overbright changes.
	virtual void SetStandardVertexShaderConstants( float fOverbright ) {}

	// Level of anisotropic filtering
	virtual void SetAnisotropicLevel( int nAnisotropyLevel );

	bool SupportsHDR() const { return false; } // deprecated; GetHDRType is the query
	HDRType_t GetHDRType() const { return CurrentHDRType(); }
	HDRType_t GetHardwareHDRType() const { return HDR_TYPE_INTEGER; }
	virtual bool NeedsATICentroidHack() const { return false; }
	virtual bool SupportsColorOnSecondStream() const { return false; }
	virtual bool SupportsStaticPlusDynamicLighting() const { return false; }
	virtual bool SupportsStreamOffset() const { return false; }
	void SetDefaultDynamicState() {}
	virtual void CommitPixelShaderLighting( int pshReg ) {}

	// Occlusion queries are CVulkanContext query slots; the handle is the slot
	// plus one, since 0 is INVALID_SHADERAPI_OCCLUSION_QUERY_HANDLE.
	ShaderAPIOcclusionQuery_t CreateOcclusionQueryObject( void )
	{
		std::string error;
		const int query = g_VulkanContext.CreateOcclusionQuery( &error );
		if ( query < 0 )
		{
			static bool s_warned = false;
			if ( !s_warned )
				Warning( "shaderapivulkan: no occlusion queries: %s\n", error.c_str() );
			s_warned = true;
			NoteUnimplemented( "occlusion query unavailable on this device" );
			return INVALID_SHADERAPI_OCCLUSION_QUERY_HANDLE;
		}
		return reinterpret_cast<ShaderAPIOcclusionQuery_t>( static_cast<intp>( query ) + 1 );
	}

	static int QuerySlot( ShaderAPIOcclusionQuery_t handle )
	{
		return static_cast<int>( reinterpret_cast<intp>( handle ) ) - 1;
	}

	void DestroyOcclusionQueryObject( ShaderAPIOcclusionQuery_t handle )
	{
		g_VulkanContext.DestroyOcclusionQuery( QuerySlot( handle ) );
	}

	void BeginOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t handle )
	{
		g_VulkanContext.QueueBeginOcclusionQuery( QuerySlot( handle ) );
	}

	void EndOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t handle )
	{
		g_VulkanContext.QueueEndOcclusionQuery( QuerySlot( handle ) );
	}

	int OcclusionQuery_GetNumPixelsRendered( ShaderAPIOcclusionQuery_t handle, bool bFlush )
	{
		const int64_t samples = g_VulkanContext.OcclusionQueryResult( QuerySlot( handle ), bFlush );
		if ( samples == render_vulkan::CVulkanContext::kQueryPending )
			return OCCLUSION_QUERY_RESULT_PENDING;
		if ( samples < 0 )
		{
			// D3D9 can flush a query issued this frame; this backend submits only
			// at present, so such a read (and a query its frame never submitted)
			// reports an error, which the material system treats as no result.
			NoteUnimplemented( "occlusion query result before its frame was submitted" );
			return OCCLUSION_QUERY_RESULT_ERROR;
		}
		return static_cast<int>( MIN( samples, static_cast<int64_t>( INT_MAX ) ) );
	}

	virtual void AcquireThreadOwnership() {}
	virtual void ReleaseThreadOwnership() {}

	virtual bool SupportsBorderColor() const { return false; }
	virtual bool SupportsFetch4() const { return false; }
	virtual bool CanStretchRectFromTextures( void ) const { return false; }
	virtual void EnableBuffer2FramesAhead( bool bEnable ) {}

	virtual void SetPSNearAndFarZ( int pshReg ) {}

	virtual void SetDepthFeatheringPixelShaderConstant( int iConstant, float fDepthBlendScale ) {}

	void SetPixelShaderFogParams( int reg ) {}

	virtual bool InFlashlightMode() const { return false; }

	virtual bool InEditorMode() const { return false; }

	// What fields in the morph do we actually use?
	virtual MorphFormat_t ComputeMorphFormat( int numSnapshots, StateSnapshot_t *pIds ) const
	{
		return 0;
	}

	// Gets the bound morph's vertex format; returns 0 if no morph is bound
	virtual MorphFormat_t GetBoundMorphFormat() { return 0; }

	// Binds a standard texture (white/black/grey, lightmap pages, frame-buffer
	// copies, ...). The material system owns every one of them and resolves the id
	// to a real texture it binds back through BindTexture -- exactly what the D3D9
	// backend defers to. The bound handle is cleared first, so an id that resolves
	// to nothing leaves no previous material's texture in place.
	//
	// A lightmap page bound to sampler 1 is what LightmappedGeneric multiplies by,
	// so that binding is recorded as the draw's lightmap. Bumped lightmaps also
	// need the normal map and three basis samples, which this backend does not
	// implement; their page is sampled at the flat coordinate and reported.
	virtual void BindStandardTexture( Sampler_t stage, StandardTextureId_t id )
	{
		if ( stage == SHADER_SAMPLER0 )
		{
			g_boundTextureHandle = -1;
			g_VulkanContext.BindManagedTexture( -1 );
		}
		const bool lightmap =
		    stage == SHADER_SAMPLER1 &&
		    ( id == TEXTURE_LIGHTMAP || id == TEXTURE_LIGHTMAP_FULLBRIGHT ||
		        id == TEXTURE_LIGHTMAP_BUMPED || id == TEXTURE_LIGHTMAP_BUMPED_FULLBRIGHT );
		if ( stage == SHADER_SAMPLER1 )
		{
			g_boundLightmapHandle = -1;
			g_VulkanContext.BindManagedLightmap( -1 );
		}
		if ( id == TEXTURE_LIGHTMAP_BUMPED || id == TEXTURE_LIGHTMAP_BUMPED_FULLBRIGHT )
			NoteUnimplemented( "lightmap: bumped lightmap sampled as its flat page" );
		g_BindingLightmap = lightmap;
		ShaderUtil()->BindStandardTexture( stage, id );
		g_BindingLightmap = false;
	}

	virtual void BindStandardVertexTexture( VertexTextureSampler_t stage, StandardTextureId_t id )
	{
		VK_UNIMPLEMENTED();
	}

	virtual void GetStandardTextureDimensions( int *pWidth, int *pHeight, StandardTextureId_t id )
	{
		*pWidth = *pHeight = 0;
	}

	virtual void SetFlashlightState( const FlashlightState_t &state, const VMatrix &worldToTexture )
	{
		VK_UNIMPLEMENTED();
	}

	virtual void SetFlashlightStateEx( const FlashlightState_t &state,
	    const VMatrix &worldToTexture, ITexture *pFlashlightDepthTexture )
	{
		VK_UNIMPLEMENTED();
	}

	virtual const FlashlightState_t &GetFlashlightState( VMatrix &worldToTexture ) const
	{
		static FlashlightState_t blah;
		return blah;
	}

	virtual const FlashlightState_t &GetFlashlightStateEx(
	    VMatrix &worldToTexture, ITexture **pFlashlightDepthTexture ) const
	{
		static FlashlightState_t blah;
		return blah;
	}

	virtual void ClearVertexAndPixelShaderRefCounts() {}

	virtual void PurgeUnusedVertexAndPixelShaders() {}

	virtual bool IsAAEnabled() const { return false; }

	virtual int GetVertexTextureCount() const { return 0; }

	virtual int GetMaxVertexTextureDimension() const { return 0; }

	virtual int MaxTextureDepth() const { return 0; }

	// Binds a vertex texture to a particular texture stage in the vertex pipe
	virtual void BindVertexTexture(
	    VertexTextureSampler_t nSampler, ShaderAPITextureHandle_t hTexture )
	{
		VK_UNIMPLEMENTED();
	}

	// Sets morph target factors
	virtual void SetFlexWeights( int nFirstWeight, int nCount, const MorphWeight_t *pWeights ) {}

	// NOTE: Stuff after this is added after shipping HL2.
	ITexture *GetRenderTargetEx( int nRenderTargetID ) { return NULL; }

	// As CShaderAPIDx8::SetToneMappingScaleLinear: without HDR the output scale is
	// 1; in integer HDR it is the engine's exposure.
	void SetToneMappingScaleLinear( const Vector &scale )
	{
		g_ToneMappingScale = scale;
		if ( CurrentHDRType() == HDR_TYPE_NONE )
			g_ToneMappingScale.x = 1.0f;
	}

	const Vector &GetToneMappingScaleLinear( void ) const { return g_ToneMappingScale; }

	// The scale LightmappedGeneric folds into its modulation, as
	// CShaderAPIDx8::GetLightMapScaleFactor defines it per HDR mode: 8-bit LDR
	// lightmaps are stored at 1/2 overbright in gamma space
	// (GammaToLinearFullRange( 2.0 )); integer-HDR pages hold linear light / 16.
	virtual float GetLightMapScaleFactor( void ) const
	{
		return GetHDRType() == HDR_TYPE_INTEGER ? 16.0f : powf( 2.0f, 2.2f );
	}

	// For dealing with device lost in cases where SwapBuffers isn't called all the time (Hammer)
	virtual void HandleDeviceLost() {}

	virtual void EnableLinearColorSpaceFrameBuffer( bool bEnable ) {}

	// Lets the shader know about the full-screen texture so it can
	virtual void SetFullScreenTextureHandle( ShaderAPITextureHandle_t h ) {}

	void SetFloatRenderingParameter( int parm_number, float value ) {}

	void SetIntRenderingParameter( int parm_number, int value ) {}
	void SetVectorRenderingParameter( int parm_number, Vector const &value ) {}

	float GetFloatRenderingParameter( int parm_number ) const { return 0; }

	int GetIntRenderingParameter( int parm_number ) const { return 0; }

	Vector GetVectorRenderingParameter( int parm_number ) const { return Vector( 0, 0, 0 ); }

	// Methods related to stencil
	// D3DRS_STENCIL* render state: recorded here and applied to each draw at
	// RenderPass (ApplyStencilState), as D3D9 applies render state at the draw.
	void SetStencilEnable( bool onoff );
	void SetStencilFailOperation( StencilOperation_t op );
	void SetStencilZFailOperation( StencilOperation_t op );
	void SetStencilPassOperation( StencilOperation_t op );
	void SetStencilCompareFunction( StencilComparisonFunction_t cmpfn );
	void SetStencilReferenceValue( int ref );
	void SetStencilTestMask( uint32 msk );
	void SetStencilWriteMask( uint32 msk );
	void ClearStencilBufferRectangle( int xmin, int ymin, int xmax, int ymax, int value );

	virtual void GetDXLevelDefaults( uint &max_dxlevel, uint &recommended_dxlevel )
	{
		max_dxlevel = recommended_dxlevel = 90;
	}

	virtual void GetMaxToRender(
	    IMesh *pMesh, bool bMaxUntilFlush, int *pMaxVerts, int *pMaxIndices )
	{
		*pMaxVerts = 32768;
		*pMaxIndices = 32768;
	}

	// Returns the max possible vertices + indices to render in a single draw call
	virtual int GetMaxVerticesToRender( IMaterial *pMaterial ) { return 32768; }

	virtual int GetMaxIndicesToRender() { return 32768; }
	virtual int CompareSnapshots( StateSnapshot_t snapshot0, StateSnapshot_t snapshot1 )
	{
		return 0;
	}

	virtual void DisableAllLocalLights() {}

	virtual bool SupportsMSAAMode( int nMSAAMode ) { return false; }

	virtual bool SupportsCSAAMode( int nNumSamples, int nQualityLevel ) { return false; }

	// Hooks for firing PIX events from outside the Material System...
	virtual void BeginPIXEvent( unsigned long color, const char *szName ) {}
	virtual void EndPIXEvent() {}
	virtual void SetPIXMarker( unsigned long color, const char *szName ) {}

	virtual void ComputeVertexDescription(
	    unsigned char *pBuffer, VertexFormat_t vertexFormat, MeshDesc_t &desc ) const
	{
		VK_UNIMPLEMENTED();
	}

	virtual bool SupportsShadowDepthTextures() { return false; }

	virtual bool SupportsFetch4() { return false; }

	virtual int NeedsShaderSRGBConversion( void ) const { return 0; }
	virtual bool UsesSRGBCorrectBlending() const { return false; }

	virtual bool HasFastVertexTextures() const { return false; }

	virtual void SetShadowDepthBiasFactors(
	    float fShadowSlopeScaleDepthBias, float fShadowDepthBias )
	{
		VK_UNIMPLEMENTED();
	}

	virtual void SetDisallowAccess( bool ) {}
	virtual void EnableShaderShaderMutex( bool ) {}
	virtual void ShaderLock() {}
	virtual void ShaderUnlock() {}

	// ------------ New Vertex/Index Buffer interface ----------------------------
	void BindVertexBuffer( int streamID, IVertexBuffer *pVertexBuffer, int nOffsetInBytes,
	    int nFirstVertex, int nVertexCount, VertexFormat_t fmt, int nRepetitions1 )
	{
		VK_UNIMPLEMENTED();
	}
	void BindIndexBuffer( IIndexBuffer *pIndexBuffer, int nOffsetInBytes ) {}
	void Draw( MaterialPrimitiveType_t primitiveType, int firstIndex, int numIndices )
	{
		NotePrimitiveType( primitiveType );
		VK_UNIMPLEMENTED();
	}
	// ------------ End ----------------------------

	virtual int GetVertexBufferCompression( void ) const { return 0; };

	virtual bool ShouldWriteDepthToDestAlpha( void ) const { return false; };
	virtual bool SupportsHDRMode( HDRType_t nHDRMode ) const
	{
		return nHDRMode == HDR_TYPE_NONE || nHDRMode == HDR_TYPE_INTEGER;
	}
	virtual bool IsDX10Card() const { return false; };

	void PushDeformation( const DeformationBase_t *pDeformation ) {}

	virtual void PopDeformation() {}

	int GetNumActiveDeformations() const { return 0; }

	// for shaders to set vertex shader constants. returns a packed state which can be used to set the dynamic combo
	int GetPackedDeformationInformation( int nMaskOfUnderstoodDeformations,
	    float *pConstantValuesOut, int nBufferSize, int nMaximumDeformations,
	    int *pNumDefsOut ) const
	{
		*pNumDefsOut = 0;
		return 0;
	}

	void SetStandardTextureHandle( StandardTextureId_t, ShaderAPITextureHandle_t ) {}

	virtual void ExecuteCommandBuffer( uint8 *pData );
	virtual bool GetHDREnabled( void ) const { return g_bHDREnabled; }
	virtual void SetHDREnabled( bool bEnable ) { g_bHDREnabled = bEnable; }

	virtual void CopyRenderTargetToScratchTexture( ShaderAPITextureHandle_t srcRt,
	    ShaderAPITextureHandle_t dstTex, Rect_t *pSrcRect = NULL, Rect_t *pDstRect = NULL )
	{
		VK_UNIMPLEMENTED();
	}

	// Allows locking and unlocking of very specific surface types.
	virtual void LockRect( void **pOutBits, int *pOutPitch, ShaderAPITextureHandle_t texHandle,
	    int mipmap, int x, int y, int w, int h, bool bWrite, bool bRead )
	{
		VK_UNIMPLEMENTED();
	}

	virtual void UnlockRect( ShaderAPITextureHandle_t texHandle, int mipmap ) {}

	virtual void TexLodClamp( int finest ) {}

	virtual void TexLodBias( float bias ) {}

	virtual void CopyTextureToTexture(
	    ShaderAPITextureHandle_t srcTex, ShaderAPITextureHandle_t dstTex )
	{
		VK_UNIMPLEMENTED();
	}

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

	void EnableAlphaToCoverage() {};
	void DisableAlphaToCoverage() {};

	ImageFormat GetShadowDepthTextureFormat() { return IMAGE_FORMAT_UNKNOWN; };
	ImageFormat GetNullTextureFormat() { return IMAGE_FORMAT_UNKNOWN; };
};

//-----------------------------------------------------------------------------
// Class Factory
//-----------------------------------------------------------------------------

static CShaderAPIVulkan g_ShaderAPIEmpty;
static CShaderShadowVulkan g_ShaderShadow;

// The null provider belongs to shaderapiempty. This module must not export a
// second definition of it: a product links both, and duplicate C entry points
// would leave the dynamic linker to pick one for every caller.
static bool CreateNativeVulkanShaderBackend( render::LegacyShaderServices *services )
{
	if ( !services )
		return false;
	services->manager = &s_ShaderDeviceMgrEmpty;
	services->api = &g_ShaderAPIEmpty;
	services->device = &s_ShaderDeviceEmpty;
	services->shadow = &g_ShaderShadow;
	services->hardware = &g_ShaderAPIEmpty;
	services->debugTextures = &g_ShaderAPIEmpty;
	return true;
}

extern "C" DLL_EXPORT bool NativeVulkanShaderBackend_Create(
    render::LegacyShaderServices *services )
{
	return CreateNativeVulkanShaderBackend( services );
}

extern "C" DLL_EXPORT const render::LegacyShaderProvider *NativeVulkanShaderBackend_Describe()
{
	static const render::LegacyShaderProvider provider = {
	    "native-vulkan", "shaderapivulkan", NativeVulkanShaderBackend_Create };
	return &provider;
}

// Test seam: exposes the native Vulkan context the legacy interfaces drive, so a
// conformance test can request a capture and read back the pixels the material-
// facing IShaderAPI/IShaderDevice path actually rendered. Not for runtime use.
extern "C" DLL_EXPORT render_vulkan::CVulkanContext *ShaderBackend_NativeVulkanContext()
{
	return &g_VulkanContext;
}

// FIXME: Remove; it's for backward compat with the materialsystem only for now
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(
    CShaderAPIVulkan, IShaderAPI, SHADERAPI_INTERFACE_VERSION, g_ShaderAPIEmpty )

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(
    CShaderShadowVulkan, IShaderShadow, SHADERSHADOW_INTERFACE_VERSION, g_ShaderShadow )

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CShaderAPIVulkan, IMaterialSystemHardwareConfig,
    MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, g_ShaderAPIEmpty )

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(
    CShaderAPIVulkan, IDebugTextureInfo, DEBUG_TEXTURE_INFO_VERSION, g_ShaderAPIEmpty )

//-----------------------------------------------------------------------------
// The main GL Shader util interface
//-----------------------------------------------------------------------------
IShaderUtil *g_pShaderUtil;

//-----------------------------------------------------------------------------
// Factory to return from SetMode
//-----------------------------------------------------------------------------
static void *ShaderInterfaceFactory( const char *pInterfaceName, int *pReturnCode )
{
	if ( pReturnCode )
	{
		*pReturnCode = IFACE_OK;
	}
	if ( !Q_stricmp( pInterfaceName, SHADER_DEVICE_INTERFACE_VERSION ) )
		return static_cast<IShaderDevice *>( &s_ShaderDeviceEmpty );
	if ( !Q_stricmp( pInterfaceName, SHADERAPI_INTERFACE_VERSION ) )
		return static_cast<IShaderAPI *>( &g_ShaderAPIEmpty );
	if ( !Q_stricmp( pInterfaceName, SHADERSHADOW_INTERFACE_VERSION ) )
		return static_cast<IShaderShadow *>( &g_ShaderShadow );

	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
//
// CShaderDeviceMgrVulkan
//
//-----------------------------------------------------------------------------
// Links this module's cvars into the engine's list, applying any command-line
// value, as CShaderAPIConVarAccessor does for the D3D9 backend.
class CShaderAPIVulkanConVarAccessor : public IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase( ConCommandBase *pCommand )
	{
		g_pCVar->RegisterConCommand( pCommand );
		const char *pValue = g_pCVar->GetCommandLineValue( pCommand->GetName() );
		if ( pValue && !pCommand->IsCommand() )
			static_cast<ConVar *>( pCommand )->SetValue( pValue );
		return true;
	}
};

bool CShaderDeviceMgrVulkan::Connect( CreateInterfaceFn factory )
{
	ConnectTier1Libraries( &factory, 1 );
	if ( g_pCVar )
	{
		static CShaderAPIVulkanConVarAccessor s_ConVarAccessor;
		ConVar_Register( FCVAR_MATERIAL_SYSTEM_THREAD, &s_ConVarAccessor );
	}

	// So others can access it
	g_pShaderUtil = (IShaderUtil *)factory( SHADER_UTIL_INTERFACE_VERSION, NULL );

	return true;
}

void CShaderDeviceMgrVulkan::Disconnect()
{
	g_pShaderUtil = NULL;
	ConVar_Unregister();
	DisconnectTier1Libraries();
}

void *CShaderDeviceMgrVulkan::QueryInterface( const char *pInterfaceName )
{
	if ( !Q_stricmp( pInterfaceName, SHADER_DEVICE_MGR_INTERFACE_VERSION ) )
		return static_cast<IShaderDeviceMgr *>( this );
	if ( !Q_stricmp( pInterfaceName, MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION ) )
		return static_cast<IMaterialSystemHardwareConfig *>( &g_ShaderAPIEmpty );
	return NULL;
}

InitReturnVal_t CShaderDeviceMgrVulkan::Init()
{
	return INIT_OK;
}

void CShaderDeviceMgrVulkan::Shutdown()
{
	// Tear the native Vulkan device down here, during the engine's ordered
	// shutdown while the SDL window / Wayland connection is still alive. The
	// swapchain references the Wayland surface, so destroying it at process exit
	// (after SDL_Quit) crashes in the Wayland client -- do it now instead. The
	// global context destructor is then a no-op (Shutdown is idempotent).
	g_VulkanContext.Shutdown();
}

// Sets the adapter
bool CShaderDeviceMgrVulkan::SetAdapter( int nAdapter, int nFlags )
{
	return true;
}

// FIXME: Is this a public interface? Might only need to be private to shaderapi
CreateInterfaceFn CShaderDeviceMgrVulkan::SetMode(
    void *hWnd, int nAdapter, const ShaderDeviceInfo_t &mode )
{
	Msg( "[NativeVulkan] Setting mode for adapter %d\n", nAdapter );

	// hWnd is an SDL_Window* on this SDL3 build (see shaderapidx9/winutils.cpp,
	// which casts the same handle to SDL_Window*). Bring up the real native
	// Vulkan device/surface/swapchain against it.
	if ( !g_VulkanContext.IsValid() )
	{
		render_vulkan::VulkanContextConfig config;
		config.appName = "Source Engine Native Vulkan";
		config.enableValidation = ( CommandLine()->FindParm( "-vkvalidate" ) != 0 );
		config.framesInFlight = 2;

		std::string error;
		if ( g_VulkanContext.Init( static_cast<SDL_Window *>( hWnd ), config, &error ) )
		{
			int w = 0, h = 0;
			g_VulkanContext.GetSwapchainExtent( w, h );
			Msg( "[NativeVulkan] device '%s' up: %dx%d, %.0f MiB, validation %s\n",
			    g_VulkanContext.DeviceName(), w, h,
			    double( g_VulkanContext.DeviceLocalMemoryBytes() ) / ( 1024.0 * 1024.0 ),
			    g_VulkanContext.ValidationEnabled() ? "on" : "off" );

			// Bring up the dynamic-mesh pipeline so material-system mesh draws
			// (IMesh::Draw) can rasterize through the backend.
			std::string meshError;
			if ( !g_VulkanContext.InitDynamicMesh( &meshError ) )
				Warning(
				    "[NativeVulkan] dynamic mesh pipeline unavailable: %s\n", meshError.c_str() );
		}
		else
		{
			// Required behavior (a presentable native device) is unavailable:
			// fail loudly rather than silently pretending to be a GPU backend.
			Warning( "[NativeVulkan] device bring-up failed: %s\n", error.c_str() );
		}
	}

	return ShaderInterfaceFactory;
}

// Gets the number of adapters...
int CShaderDeviceMgrVulkan::GetAdapterCount() const
{
	// Advertise the native Vulkan adapter so the material system can select it
	// and drive rendering through this backend. (Integration attempt: the
	// material path is partially implemented; this is honest enumeration of a
	// real device, not a claim of full rendering.)
	return 1;
}

bool CShaderDeviceMgrVulkan::GetRecommendedConfigurationInfo(
    int nAdapter, int nDXLevel, KeyValues *pKeyValues )
{
	return true;
}

// Returns info about each adapter
void CShaderDeviceMgrVulkan::GetAdapterInfo( int adapter, MaterialAdapterInfo_t &info ) const
{
	memset( &info, 0, sizeof( info ) );
	Q_strncpy( info.m_pDriverName, "Native Vulkan", sizeof( info.m_pDriverName ) );
	info.m_nDXSupportLevel = 90;
	info.m_nMaxDXSupportLevel = 90;
	info.m_nDriverVersionHigh = 1;
	info.m_nDriverVersionLow = 0;
}

// Returns the number of modes
int CShaderDeviceMgrVulkan::GetModeCount( int nAdapter ) const
{
	return 1;
}

// Returns mode information..
void CShaderDeviceMgrVulkan::GetModeInfo(
    ShaderDisplayMode_t *pInfo, int nAdapter, int nMode ) const
{
	if ( !pInfo )
		return;
	pInfo->m_nWidth = 1920;
	pInfo->m_nHeight = 1080;
	pInfo->m_Format = IMAGE_FORMAT_BGRA8888;
	pInfo->m_nRefreshRateNumerator = 60;
	pInfo->m_nRefreshRateDenominator = 1;
}

void CShaderDeviceMgrVulkan::GetCurrentModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter ) const
{
	VK_UNIMPLEMENTED();
}

//-----------------------------------------------------------------------------
//
// Shader device empty
//
//-----------------------------------------------------------------------------
void CShaderDeviceVulkan::GetWindowSize( int &width, int &height ) const
{
	if ( g_VulkanContext.IsValid() )
	{
		g_VulkanContext.GetSwapchainExtent( width, height );
		return;
	}
	width = 0;
	height = 0;
}

void CShaderDeviceVulkan::GetBackBufferDimensions( int &width, int &height ) const
{
	if ( g_VulkanContext.IsValid() )
	{
		g_VulkanContext.GetSwapchainExtent( width, height );
		return;
	}
	width = 1024;
	height = 768;
}

// Use this to spew information about the 3D layer
void CShaderDeviceVulkan::SpewDriverInfo() const
{
	if ( g_VulkanContext.IsValid() )
	{
		Msg( "Native Vulkan device: %s (vendor 0x%04x, device 0x%04x, %s)\n",
		    g_VulkanContext.DeviceName(), g_VulkanContext.VendorId(), g_VulkanContext.DeviceId(),
		    g_VulkanContext.IsDiscrete() ? "discrete" : "integrated/other" );
		return;
	}
	Warning( "Native Vulkan device not initialized\n" );
}

// Creates/ destroys a child window
bool CShaderDeviceVulkan::AddView( void *hwnd )
{
	return true;
}

void CShaderDeviceVulkan::RemoveView( void *hwnd )
{
	VK_UNIMPLEMENTED();
}

// Activates a view
void CShaderDeviceVulkan::SetView( void *hwnd )
{
	VK_UNIMPLEMENTED();
}

void CShaderDeviceVulkan::ReleaseResources()
{
	VK_UNIMPLEMENTED();
}

void CShaderDeviceVulkan::ReacquireResources()
{
	VK_UNIMPLEMENTED();
}

// Creates/destroys Mesh
IMesh *CShaderDeviceVulkan::CreateStaticMesh(
    VertexFormat_t fmt, const char *pTextureBudgetGroup, IMaterial *pMaterial )
{
	// A static mesh is built once and drawn many times, so it needs storage of
	// its own; the caller owns it until DestroyStaticMesh.
	return new CEmptyMesh( false );
}

void CShaderDeviceVulkan::DestroyStaticMesh( IMesh *mesh )
{
	CEmptyMesh *own = AsOwnMesh( mesh );
	if ( own && own != &m_Mesh && own != &m_DynamicMesh )
		delete own;
}

// Creates/destroys static vertex + index buffers
IVertexBuffer *CShaderDeviceVulkan::CreateVertexBuffer(
    ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pTextureBudgetGroup )
{
	return ( type == SHADER_BUFFER_TYPE_STATIC || type == SHADER_BUFFER_TYPE_STATIC_TEMP )
	           ? static_cast<IVertexBuffer *>( new CEmptyMesh( false ) )
	           : &m_DynamicMesh;
}

void CShaderDeviceVulkan::DestroyVertexBuffer( IVertexBuffer *pVertexBuffer )
{
	// Only static buffers are allocated per call; the dynamic one is shared.
	if ( pVertexBuffer && pVertexBuffer != &m_DynamicMesh && pVertexBuffer != &m_Mesh )
		DestroyStaticMesh( static_cast<CEmptyMesh *>( pVertexBuffer ) );
}

IIndexBuffer *CShaderDeviceVulkan::CreateIndexBuffer( ShaderBufferType_t bufferType,
    MaterialIndexFormat_t fmt, int nIndexCount, const char *pTextureBudgetGroup )
{
	switch ( bufferType )
	{
	case SHADER_BUFFER_TYPE_STATIC:
	case SHADER_BUFFER_TYPE_STATIC_TEMP:
		return new CEmptyMesh( false );
	default:
		Assert( 0 );
	case SHADER_BUFFER_TYPE_DYNAMIC:
	case SHADER_BUFFER_TYPE_DYNAMIC_TEMP:
		return &m_DynamicMesh;
	}
}

void CShaderDeviceVulkan::DestroyIndexBuffer( IIndexBuffer *pIndexBuffer )
{
	if ( pIndexBuffer && pIndexBuffer != &m_DynamicMesh && pIndexBuffer != &m_Mesh )
		DestroyStaticMesh( static_cast<CEmptyMesh *>( pIndexBuffer ) );
}

IVertexBuffer *CShaderDeviceVulkan::GetDynamicVertexBuffer(
    int streamID, VertexFormat_t vertexFormat, bool bBuffered )
{
	return &m_DynamicMesh;
}

IIndexBuffer *CShaderDeviceVulkan::GetDynamicIndexBuffer(
    MaterialIndexFormat_t fmt, bool bBuffered )
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
	LiveMeshes().insert( this );
}

CEmptyMesh::~CEmptyMesh()
{
	LiveMeshes().erase( this );
}

void CEmptyMesh::SetSources( IMesh *pVertexOverride, IMesh *pIndexOverride )
{
	CEmptyMesh *vertexSource = AsOwnMesh( pVertexOverride );
	CEmptyMesh *indexSource = AsOwnMesh( pIndexOverride );
	if ( ( pVertexOverride && !vertexSource ) || ( pIndexOverride && !indexSource ) )
		NoteUnimplemented( "GetDynamicMesh(foreign override mesh)" );
	m_pVertexSource = ( vertexSource == this ) ? nullptr : vertexSource;
	m_pIndexSource = ( indexSource == this ) ? nullptr : indexSource;
}

bool CEmptyMesh::Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t &desc )
{
	// Hand out real index storage so the mesh builder's authored indices are
	// kept (m_nIndexSize = 1 advances one slot per index). Draw() replays the
	// geometry in this order.
	if ( nMaxIndexCount < 0 )
		nMaxIndexCount = 0;
	if ( nMaxIndexCount > kMaxLockIndices )
		nMaxIndexCount = kMaxLockIndices;
	if ( m_indexData.size() < static_cast<size_t>( nMaxIndexCount ) + 1 )
		m_indexData.resize( static_cast<size_t>( nMaxIndexCount ) + 1 );
	desc.m_pIndices = m_indexData.data();
	desc.m_nIndexSize = 1;
	desc.m_nFirstIndex = 0;
	desc.m_nOffset = 0;
	m_numIndices = nMaxIndexCount;
	return true;
}

void CEmptyMesh::Unlock( int nWrittenIndexCount, IndexDesc_t &desc )
{
	if ( nWrittenIndexCount >= 0 && nWrittenIndexCount <= m_numIndices )
		m_numIndices = nWrittenIndexCount;
}

void CEmptyMesh::ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t &desc )
{
	// Modify the existing list in place; relocking would discard it.
	if ( nFirstIndex < 0 || nIndexCount < 0 ||
	     static_cast<size_t>( nFirstIndex ) + nIndexCount > m_indexData.size() )
	{
		desc.m_pIndices = reinterpret_cast<unsigned short *>( m_dummyComponent );
		desc.m_nIndexSize = 0;
		desc.m_nFirstIndex = 0;
		desc.m_nOffset = 0;
		return;
	}
	desc.m_pIndices = m_indexData.data() + nFirstIndex;
	desc.m_nIndexSize = 1;
	desc.m_nFirstIndex = 0;
	desc.m_nOffset = 0;
}

void CEmptyMesh::ModifyEnd( IndexDesc_t &desc )
{
}

void CEmptyMesh::Spew( int nIndexCount, const IndexDesc_t &desc )
{
}

void CEmptyMesh::ValidateData( int nIndexCount, const IndexDesc_t &desc )
{
}

bool CEmptyMesh::Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc )
{
	// Real interleaved layout so a mesh builder writes coherent vertices that
	// Draw() can forward to the GPU: position (vec3) at offset 0, color (4 bytes)
	// at offset 12, stride kMeshVertexStride. Components this bounded backend
	// does not carry point at a dummy scratch with size 0, so writing them never
	// disturbs position/color.
	if ( nVertexCount < 0 )
		nVertexCount = 0;
	if ( nVertexCount > kMaxLockVertices )
		nVertexCount = kMaxLockVertices;
	m_numVerts = nVertexCount;
	// One spare vertex keeps the pointers valid for a zero-vertex (index-only)
	// lock, which the builder still addresses.
	const size_t bytes = ( static_cast<size_t>( nVertexCount ) + 1 ) * kMeshVertexStride;
	if ( m_vertexData.size() < bytes )
		m_vertexData.resize( bytes );
	unsigned char *const vertexMemory = m_vertexData.data();

	// Seed the color field with opaque white before the builder writes anything.
	// The shader MULTIPLIES by vertex color, and most Source vertex formats -- the
	// lightmapped world format above all -- carry no color at all, so the builder
	// never writes this field. Leaving it as whatever the allocation held means
	// multiplying the entire world by zero, which renders it black no matter how
	// correct its geometry and textures are. White is the identity, and a format
	// that does author colors simply overwrites it.
	// Unskinned vertices likewise get weight 1 on bone 0.
	for ( int v = 0; v < nVertexCount; ++v )
	{
		unsigned char *vertex = vertexMemory + static_cast<size_t>( v ) * kMeshVertexStride;
		memset( vertex + 12, 0xFF, 4 );
		const float weights[2] = { 1.0f, 0.0f };
		memcpy( vertex + kMeshBoneWeightOffset, weights, sizeof( weights ) );
		memset( vertex + kMeshBoneIndexOffset, 0, 4 );
		memset( vertex + kMeshNormalOffset, 0, 28 );
	}

	desc.m_pPosition = (float *)( vertexMemory );
	desc.m_pColor = vertexMemory + 12;
	desc.m_VertexSize_Position = kMeshVertexStride;
	desc.m_VertexSize_Color = kMeshVertexStride;

	// Texcoord0 (base UV) lives at offset 16 and texcoord1 (the lightmap
	// coordinate) at 24; other texcoord sets go to the dummy scratch.
	desc.m_pNormal = (float *)( vertexMemory + kMeshNormalOffset );
	int i;
	for ( i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
	{
		if ( i < 2 )
		{
			desc.m_pTexCoord[i] = (float *)( vertexMemory + 16 + i * 8 );
			desc.m_VertexSize_TexCoord[i] = kMeshVertexStride;
		}
		else
		{
			desc.m_pTexCoord[i] = (float *)m_dummyComponent;
			desc.m_VertexSize_TexCoord[i] = 0;
		}
	}
	// Bone weights and indices: what hardware skinning blends (SkinPosition).
	desc.m_pBoneWeight = (float *)( vertexMemory + kMeshBoneWeightOffset );
	desc.m_pBoneMatrixIndex = vertexMemory + kMeshBoneIndexOffset;
	desc.m_pTangentS = (float *)m_dummyComponent;
	desc.m_pTangentT = (float *)m_dummyComponent;
	desc.m_pUserData = (float *)( vertexMemory + kMeshUserDataOffset );
	desc.m_NumBoneWeights = 2;

	desc.m_VertexSize_BoneWeight = kMeshVertexStride;
	desc.m_VertexSize_BoneMatrixIndex = kMeshVertexStride;
	desc.m_VertexSize_Normal = kMeshVertexStride;
	desc.m_VertexSize_TangentS = 0;
	desc.m_VertexSize_TangentT = 0;
	desc.m_VertexSize_UserData = kMeshVertexStride;
	desc.m_ActualVertexSize = kMeshVertexStride;

	desc.m_nFirstVertex = 0;
	desc.m_nOffset = 0;
	return true;
}

void CEmptyMesh::Unlock( int nVertexCount, VertexDesc_t &desc )
{
	if ( nVertexCount >= 0 && nVertexCount <= m_numVerts )
		m_numVerts = nVertexCount;
}

void CEmptyMesh::Spew( int nVertexCount, const VertexDesc_t &desc )
{
}

void CEmptyMesh::ValidateData( int nVertexCount, const VertexDesc_t &desc )
{
}

void CEmptyMesh::LockMesh( int numVerts, int numIndices, MeshDesc_t &desc )
{
	Lock( numVerts, false, *static_cast<VertexDesc_t *>( &desc ) );
	Lock( numIndices, false, *static_cast<IndexDesc_t *>( &desc ) );
}

void CEmptyMesh::UnlockMesh( int numVerts, int numIndices, MeshDesc_t &desc )
{
	// Record the counts the builder actually wrote, so Draw() replays exactly the
	// authored geometry rather than the (larger) locked capacity.
	if ( numVerts >= 0 )
		m_numVerts = numVerts;
	if ( numIndices >= 0 )
		m_numIndices = numIndices;
}

void CEmptyMesh::ModifyBeginEx( bool bReadOnly, int firstVertex, int numVerts, int firstIndex,
    int numIndices, MeshDesc_t &desc )
{
	// Modify existing geometry in place (e.g. rewriting a static mesh's colors).
	// Relocking would reseed the colors and forget the vertex count.
	VertexDesc_t &vdesc = *static_cast<VertexDesc_t *>( &desc );
	const size_t vertexCapacity = m_vertexData.size() / kMeshVertexStride;
	if ( firstVertex < 0 || numVerts < 0 ||
	     static_cast<size_t>( firstVertex ) + numVerts > vertexCapacity )
	{
		// Out of range: point every component at the scratch with zero stride.
		const int keepVerts = m_numVerts;
		Lock( 0, false, vdesc );
		m_numVerts = keepVerts;
		vdesc.m_pPosition = (float *)m_dummyComponent;
		vdesc.m_pColor = m_dummyComponent;
		vdesc.m_VertexSize_Position = 0;
		vdesc.m_VertexSize_Color = 0;
		vdesc.m_pTexCoord[0] = (float *)m_dummyComponent;
		vdesc.m_VertexSize_TexCoord[0] = 0;
		vdesc.m_pNormal = (float *)m_dummyComponent;
		vdesc.m_VertexSize_Normal = 0;
		vdesc.m_pUserData = (float *)m_dummyComponent;
		vdesc.m_VertexSize_UserData = 0;
	}
	else
	{
		const int keepVerts = m_numVerts;
		Lock( 0, false, vdesc ); // fills the scratch pointers for other components
		m_numVerts = keepVerts;
		unsigned char *base =
		    m_vertexData.data() + static_cast<size_t>( firstVertex ) * kMeshVertexStride;
		vdesc.m_pPosition = (float *)base;
		vdesc.m_pColor = base + 12;
		vdesc.m_pTexCoord[0] = (float *)( base + 16 );
		vdesc.m_pBoneWeight = (float *)( base + kMeshBoneWeightOffset );
		vdesc.m_pBoneMatrixIndex = base + kMeshBoneIndexOffset;
		vdesc.m_pNormal = (float *)( base + kMeshNormalOffset );
		vdesc.m_pUserData = (float *)( base + kMeshUserDataOffset );
	}
	ModifyBegin( bReadOnly, firstIndex, numIndices, *static_cast<IndexDesc_t *>( &desc ) );
}

void CEmptyMesh::ModifyBegin(
    int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t &desc )
{
	ModifyBeginEx( false, firstVertex, numVerts, firstIndex, numIndices, desc );
}

void CEmptyMesh::ModifyEnd( MeshDesc_t &desc )
{
}

// returns the # of vertices (static meshes only)
int CEmptyMesh::VertexCount() const
{
	return m_numVerts;
}

// Sets the primitive type
void CEmptyMesh::SetPrimitiveType( MaterialPrimitiveType_t type )
{
	NotePrimitiveType( type );
	m_primitiveType = type;
}

// The material and mesh the engine is currently drawing, so IMesh::Draw can run
// the real material render path (material -> shader DrawElements -> BeginPass ->
// RenderPass) exactly as the D3D9 backend does, instead of drawing the geometry
// with a fixed default shader. Bind() records the material; Draw() records the
// mesh and kicks the material, whose RenderPass emits the geometry with the
// material's selected shader, modulation, textures and blend state.
static IMaterialInternal *g_pBoundMaterial = nullptr;

// Stable small ids for material names, so each recorded draw can say which
// material produced it without the device knowing about materials.
static std::vector<std::string> g_MaterialTags;

static int MaterialTag( IMaterial *pMaterial )
{
	const char *name = pMaterial ? pMaterial->GetName() : "(no material)";
	for ( size_t i = 0; i < g_MaterialTags.size(); ++i )
	{
		if ( g_MaterialTags[i] == name )
			return static_cast<int>( i );
	}
	if ( g_MaterialTags.size() >= 4096 )
		return -1;
	g_MaterialTags.emplace_back( name );
	return static_cast<int>( g_MaterialTags.size() - 1 );
}

static const char *MaterialTagName( int tag )
{
	return ( tag >= 0 && static_cast<size_t>( tag ) < g_MaterialTags.size() )
	           ? g_MaterialTags[static_cast<size_t>( tag )].c_str()
	           : "?";
}

// Which materials' draws were dropped, by name, for the census report.
static std::vector<std::pair<std::string, uint64_t>> g_DroppedMaterials;

static void NoteDroppedMaterial()
{
	const char *name = g_pBoundMaterial ? g_pBoundMaterial->GetName() : "(no material)";
	for ( auto &entry : g_DroppedMaterials )
	{
		if ( entry.first == name )
		{
			++entry.second;
			return;
		}
	}
	if ( g_DroppedMaterials.size() < 256 )
		g_DroppedMaterials.emplace_back( name, 1 );
}

static void ReportDroppedMaterials()
{
	// The captured frame's records with the material that issued each one.
	for ( const auto &r : g_VulkanContext.DescribeStreamRecords() )
	{
		if ( r.kind == 1 )
			fprintf( stderr,
			    "[vulkan]   frame clear tgt=%d color=%d depth=%d value=%.2f,%.2f,%.2f,%.2f\n",
			    r.target, r.clearColor, r.clearDepth, r.clearValue[0], r.clearValue[1],
			    r.clearValue[2], r.clearValue[3] );
		else
			fprintf( stderr, "[vulkan]   frame %s tgt=%d blend=%d verts=%u material=%s\n",
			    r.kind == 0 ? "draw" : "copy", r.target, r.raster.blend, r.vertexCount,
			    MaterialTagName( r.tag ) );
	}
	for ( const auto &entry : g_DroppedMaterials )
		fprintf( stderr, "[vulkan]   dropped material draws=%-7llu %s\n",
		    static_cast<unsigned long long>( entry.second ), entry.first.c_str() );
}
static CEmptyMesh *g_pRenderMesh = nullptr;

// common_vs_fxc.h SkinPosition with skinning on: always three bones, indices in
// the vertex's first three index bytes, weights w0, w1 and 1 - w0 - w1, each
// bone a pose-to-world 3x4 matrix. Replaces a model-space position with world.
static void SkinPosition( const unsigned char *vertex, float pos[3] )
{
	float weights[3];
	memcpy( weights, vertex + CEmptyMesh::kMeshBoneWeightOffset, 2 * sizeof( float ) );
	weights[2] = 1.0f - ( weights[0] + weights[1] );
	const unsigned char *indices = vertex + CEmptyMesh::kMeshBoneIndexOffset;
	float world[3] = { 0.0f, 0.0f, 0.0f };
	for ( int b = 0; b < 3; ++b )
	{
		const int bone = indices[b] < kMaxBoneMatrices ? indices[b] : 0;
		const float *m = g_BoneMatrices[bone];
		for ( int r = 0; r < 3; ++r )
			world[r] += weights[b] * ( m[r * 4] * pos[0] + m[r * 4 + 1] * pos[1] +
			                             m[r * 4 + 2] * pos[2] + m[r * 4 + 3] );
	}
	pos[0] = world[0];
	pos[1] = world[1];
	pos[2] = world[2];
}

// Draws the entire mesh
void CEmptyMesh::Draw( int firstIndex, int numIndices )
{
	const CEmptyMesh &vertices = m_pVertexSource ? *m_pVertexSource : *this;
	const CEmptyMesh &indices = m_pIndexSource ? *m_pIndexSource : *this;
	{ // VGUIDEBUG
		static std::map<std::string, int> seen;
		const char *mn = g_pBoundMaterial ? g_pBoundMaterial->GetName() : "(none)";
		if ( ++seen[mn] <= 2 )
			fprintf( stderr, "[vguidbg] Draw mat=%s shader=%s verts=%d idx=%d prim=%d\n", mn,
			    g_pBoundMaterial ? g_pBoundMaterial->GetShaderName() : "-", vertices.m_numVerts,
			    indices.m_numIndices, (int)m_primitiveType );
	}
	if ( !g_VulkanContext.IsValid() || !g_VulkanContext.DynamicMeshReady() ||
	     vertices.m_numVerts <= 0 )
		return;

	// Record the index range. mesh->Draw() passes (-1, 0) meaning "the whole mesh".
	m_drawFirst = ( firstIndex > 0 ) ? firstIndex : 0;
	m_drawCount = ( numIndices > 0 ) ? numIndices : indices.m_numIndices;

	// Run the real material path: the bound material's shader executes, selecting
	// its native pipeline and constants via BeginPass, and calls back into
	// RenderPass -> EmitToNativeQueue to draw this geometry with that state. When no
	// material is bound, emit directly with the default (vertex-color) shader.
	g_pRenderMesh = this;
	if ( g_pBoundMaterial )
		g_pBoundMaterial->DrawMesh( VERTEX_COMPRESSION_NONE );
	else
		EmitToNativeQueue();
	g_pRenderMesh = nullptr;
}

void CEmptyMesh::EmitToNativeQueue()
{
	const CEmptyMesh &vertices = m_pVertexSource ? *m_pVertexSource : *this;
	const CEmptyMesh &indices = m_pIndexSource ? *m_pIndexSource : *this;
	const int numVerts = vertices.m_numVerts;
	if ( !g_VulkanContext.IsValid() || !g_VulkanContext.DynamicMeshReady() )
		return;
	// Every way a requested draw can fail to reach the GPU is recorded by name, so
	// the census shows how much of the frame each one costs.
	if ( numVerts <= 0 )
	{
		DropDraw( "draw dropped: mesh has no vertices" );
		return;
	}
	const int first = m_drawFirst;
	int count = m_drawCount;
	// Never read past the indices actually written.
	if ( count > 0 && first + count > indices.m_numIndices )
	{
		NoteUnimplemented( "draw clipped: index range past written indices" );
		count = std::max( 0, indices.m_numIndices - first );
	}
	if ( m_drawCount > 0 && count <= 0 )
	{
		DropDraw( "draw dropped: no indices in range" );
		return;
	}

	std::vector<float> lightmapUv;
	// Normal and tangent, only for the shader that reads them (PortalRefract);
	// skinned normals and tangents are not blended.
	const bool wantsTangents = g_CurrentPortalStage >= 0;
	if ( wantsTangents && g_NumBoneWeights > 0 )
		NoteUnimplemented( "PortalRefract: skinned normal/tangent" );
	std::vector<float> normalTangent;
	std::vector<float> vertexAlpha;
	auto appendVertex = [&]( std::vector<float> &out, int v )
	{
		const unsigned char *base =
		    vertices.m_vertexData.data() + static_cast<size_t>( v ) * kMeshVertexStride;
		float pos[3], uv[2], lightmap[2];
		memcpy( pos, base, sizeof( pos ) );
		if ( g_NumBoneWeights > 0 )
			SkinPosition( base, pos );
		memcpy( uv, base + 16, sizeof( uv ) );
		memcpy( lightmap, base + 24, sizeof( lightmap ) );
		lightmapUv.push_back( lightmap[0] );
		lightmapUv.push_back( lightmap[1] );
		if ( wantsTangents )
		{
			float nt[7];
			memcpy( nt, base + kMeshNormalOffset, sizeof( float ) * 3 );
			memcpy( nt + 3, base + kMeshUserDataOffset, sizeof( float ) * 4 );
			normalTangent.insert( normalTangent.end(), nt, nt + 7 );
		}
		const unsigned char *col = base + 12;
		vertexAlpha.push_back( col[3] / 255.0f );
		out.push_back( pos[0] );
		out.push_back( pos[1] );
		out.push_back( pos[2] );
		out.push_back( col[0] / 255.0f );
		out.push_back( col[1] / 255.0f );
		out.push_back( col[2] / 255.0f );
		out.push_back( uv[0] );
		out.push_back( uv[1] );
	};

	// Element i of this draw, in mesh vertex indices. Indexed geometry reads the
	// authored index list; non-indexed geometry is the vertices in order.
	const int elementCount = ( count > 0 ) ? count : numVerts;
	auto element = [&]( int i ) -> int
	{
		if ( count <= 0 )
			return i;
		return static_cast<int>( indices.m_indexData[static_cast<size_t>( first + i )] );
	};

	std::vector<float> interleaved;
	interleaved.reserve( static_cast<size_t>( elementCount ) * 8 );

	auto appendTriangle = [&]( int a, int b, int c )
	{
		if ( a < 0 || b < 0 || c < 0 || a >= numVerts || b >= numVerts || c >= numVerts )
		{
			NoteUnimplemented( "triangle dropped: index outside vertex range" );
			return;
		}
		// Strips stitch separate runs together with degenerate (zero-area)
		// triangles. A strip pipeline discards those for free; assembling a list
		// has to drop them explicitly or they become junk geometry.
		if ( a == b || b == c || a == c )
			return;
		appendVertex( interleaved, a );
		appendVertex( interleaved, b );
		appendVertex( interleaved, c );
	};

	switch ( m_primitiveType )
	{
	case MATERIAL_TRIANGLE_STRIP:
		// Every vertex after the first two closes a triangle with its two
		// predecessors, alternating winding so the facing stays consistent.
		for ( int i = 0; i + 2 < elementCount; ++i )
		{
			if ( i & 1 )
				appendTriangle( element( i + 1 ), element( i ), element( i + 2 ) );
			else
				appendTriangle( element( i ), element( i + 1 ), element( i + 2 ) );
		}
		break;

	case MATERIAL_POLYGON:
		// A single convex polygon, fanned from its first vertex.
		for ( int i = 1; i + 1 < elementCount; ++i )
			appendTriangle( element( 0 ), element( i ), element( i + 1 ) );
		break;

	case MATERIAL_QUADS:
	case MATERIAL_INSTANCED_QUADS:
		for ( int i = 0; i + 3 < elementCount; i += 4 )
		{
			appendTriangle( element( i ), element( i + 1 ), element( i + 2 ) );
			appendTriangle( element( i ), element( i + 2 ), element( i + 3 ) );
		}
		break;

	case MATERIAL_POINTS:
	case MATERIAL_LINES:
	case MATERIAL_LINE_STRIP:
	case MATERIAL_LINE_LOOP:
		// No line or point pipeline exists yet, and assembling these as triangles
		// would draw geometry the engine never asked for. Drop the draw instead.
		DropDraw( "draw dropped: line/point topology" );
		return;

	default:
		for ( int i = 0; i + 2 < elementCount; i += 3 )
			appendTriangle( element( i ), element( i + 1 ), element( i + 2 ) );
		break;
	}

	if ( interleaved.empty() )
	{
		DropDraw( "draw dropped: no triangles assembled" );
		return;
	}
	NoteDrawTextureResidency();
	// Emitting a draw whose base texture this backend cannot supply does not
	// degrade gracefully: the textured shader samples the default white, and these
	// passes (render-target copies, refraction, the 2D/VGUI overlays that bind a
	// standard texture) are screen-covering, so a single one paints over the
	// entire correct scene behind it. Until render targets and the standard
	// texture set exist, drop the draw instead of covering the frame with it. The
	// residency totals above report how much is being dropped.
	if ( !HasResidentBaseTexture() )
	{
		DropDraw( "draw dropped: textured shader with no base texture" );
		NoteDroppedMaterial();
		return;
	}
	// Skinned positions are already in world space: draw them with view and
	// projection only, as the skinned vertex shaders apply cViewProj.
	if ( g_NumBoneWeights > 0 )
		CommitViewProj();
	g_VulkanContext.QueueDynamicTriangles( interleaved.data(),
	    static_cast<uint32_t>( interleaved.size() / 8 ), lightmapUv.data(),
	    wantsTangents ? normalTangent.data() : nullptr, vertexAlpha.data() );
	if ( g_NumBoneWeights > 0 )
		CommitModelViewProj();
}

void CEmptyMesh::Draw( CPrimList *pPrims, int nPrims )
{
	VK_UNIMPLEMENTED();
}

// Copy verts and/or indices to a mesh builder. This only works for temp meshes!
void CEmptyMesh::CopyToMeshBuilder( int iStartVert, // Which vertices to copy.
    int nVerts,
    int iStartIndex, // Which indices to copy.
    int nIndices,
    int indexOffset, // This is added to each index.
    CMeshBuilder &builder )
{
	VK_UNIMPLEMENTED();
}

// Spews the mesh data
void CEmptyMesh::Spew( int numVerts, int numIndices, const MeshDesc_t &desc )
{
}

void CEmptyMesh::ValidateData( int numVerts, int numIndices, const MeshDesc_t &desc )
{
}

// gets the associated material
IMaterial *CEmptyMesh::GetMaterial()
{
	// umm. this don't work none
	Assert( 0 );
	return 0;
}

// Packs vertex components into a VertexFormat_t exactly as
// CMeshMgr::ComputeVertexFormat does. This backend has no compressed vertices.
static VertexFormat_t NativeVertexFormat( unsigned int flags, int nTexCoordArraySize,
    const int *pTexCoordDimensions, int numBoneWeights, int userDataSize )
{
	VertexFormat_t fmt = flags & ~( VERTEX_FORMAT_USE_EXACT_FORMAT | VERTEX_FORMAT_COMPRESSED );
	if ( numBoneWeights > 0 )
		fmt |= VERTEX_BONEWEIGHT( 2 ); // always exactly two weights
	fmt |= VERTEX_USERDATA_SIZE( userDataSize );
	nTexCoordArraySize =
	    Min( nTexCoordArraySize, static_cast<int>( VERTEX_MAX_TEXTURE_COORDINATES ) );
	for ( int i = 0; i < nTexCoordArraySize; ++i )
	{
		// Without dimensions, the first N coordinates are 2D.
		const int size = pTexCoordDimensions ? pTexCoordDimensions[i] : 2;
		fmt |= VERTEX_TEXCOORD_SIZE( static_cast<TextureStage_t>( i ), size );
	}
	return fmt;
}

//-----------------------------------------------------------------------------
// The shader shadow interface
//-----------------------------------------------------------------------------
CShaderShadowVulkan::CShaderShadowVulkan()
{
	m_IsTranslucent = false;
	m_IsAlphaTested = false;
	m_bIsDepthWriteEnabled = true;
	m_bUsesVertexAndPixelShaders = false;
}

CShaderShadowVulkan::~CShaderShadowVulkan()
{
}

// Sets the default *shadow* state
void CShaderShadowVulkan::SetDefaultState()
{
	m_IsTranslucent = false;
	m_IsAlphaTested = false;
	m_bIsDepthWriteEnabled = true;
	m_bIsDepthTestEnabled = true;
	m_depthFunc = SHADER_DEPTHFUNC_NEAREROREQUAL;
	m_bUsesVertexAndPixelShaders = false;
	m_blendSrc = SHADER_BLEND_ONE;
	m_blendDst = SHADER_BLEND_ZERO;
	m_alphaRef = 0.0f;
	m_alphaFunc = SHADER_ALPHAFUNC_GEQUAL;
	m_colorWrites = true;
	m_cullEnable = true;
	m_vertexUsage = 0;
	m_colorFlags = 0;
	m_vertexShaderName[0] = '\0';
	// D3D9's default shadow state has no pixel shader: a shader that sets none
	// (WriteZ, a depth-only BufferClearObeyStencil) must not inherit the last one.
	m_pixelShaderName[0] = '\0';
	m_pixelShaderIndex = 0;
}

// sRGB decode on the samplers the textured pipeline reads (base on sampler 0,
// lightmap on sampler 1) and encode on output, as D3D9's SRGBTEXTURE sampler
// state and SRGBWRITEENABLE render state apply them.
void CShaderShadowVulkan::EnableSRGBRead( Sampler_t stage, bool bEnable )
{
	int flag = 0;
	if ( stage == SHADER_SAMPLER0 )
		flag = render_vulkan::CVulkanContext::kColorSrgbReadBase;
	else if ( stage == SHADER_SAMPLER1 )
		flag = render_vulkan::CVulkanContext::kColorSrgbReadLightmap;
	else if ( stage == SHADER_SAMPLER2 )
		flag = render_vulkan::CVulkanContext::kColorSrgbReadSampler2;
	m_colorFlags = bEnable ? ( m_colorFlags | flag ) : ( m_colorFlags & ~flag );
}

void CShaderShadowVulkan::EnableSRGBWrite( bool bEnable )
{
	const int flag = render_vulkan::CVulkanContext::kColorSrgbWrite;
	m_colorFlags = bEnable ? ( m_colorFlags | flag ) : ( m_colorFlags & ~flag );
}

// Methods related to depth buffering
void CShaderShadowVulkan::DepthFunc( ShaderDepthFunc_t depthFunc )
{
	m_depthFunc = depthFunc;
}

void CShaderShadowVulkan::EnableDepthWrites( bool bEnable )
{
	m_bIsDepthWriteEnabled = bEnable;
}

void CShaderShadowVulkan::EnableDepthTest( bool bEnable )
{
	m_bIsDepthTestEnabled = bEnable;
}

void CShaderShadowVulkan::EnablePolyOffset( PolygonOffsetMode_t nOffsetMode )
{
	VK_UNIMPLEMENTED();
}

// Suppresses/activates color writing
void CShaderShadowVulkan::EnableColorWrites( bool bEnable )
{
	m_colorWrites = bEnable;
}

// Suppresses/activates alpha writing
void CShaderShadowVulkan::EnableAlphaWrites( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

// Methods related to alpha blending
void CShaderShadowVulkan::EnableBlending( bool bEnable )
{
	m_IsTranslucent = bEnable;
}

void CShaderShadowVulkan::BlendFunc( ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor )
{
	m_blendSrc = srcFactor;
	m_blendDst = dstFactor;
}

// A simpler method of dealing with alpha modulation
void CShaderShadowVulkan::EnableAlphaPipe( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::EnableConstantAlpha( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::EnableVertexAlpha( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::EnableTextureAlpha( TextureStage_t stage, bool bEnable )
{
	VK_UNIMPLEMENTED();
}

// Alpha testing
void CShaderShadowVulkan::EnableAlphaTest( bool bEnable )
{
	m_IsAlphaTested = bEnable;
}

void CShaderShadowVulkan::AlphaFunc( ShaderAlphaFunc_t alphaFunc, float alphaRef /* [0-1] */ )
{
	m_alphaFunc = alphaFunc;
	m_alphaRef = alphaRef;
}

// Wireframe/filled polygons
void CShaderShadowVulkan::PolyMode( ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode )
{
	VK_UNIMPLEMENTED();
}

// Back face culling
void CShaderShadowVulkan::EnableCulling( bool bEnable )
{
	m_cullEnable = bEnable;
}

// Alpha to coverage
void CShaderShadowVulkan::EnableAlphaToCoverage( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

// constant color + transparency
void CShaderShadowVulkan::EnableConstantColor( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

// Indicates the vertex format for use with a vertex shader
// The flags to pass in here come from the VertexFormatFlags_t enum
// If pTexCoordDimensions is *not* specified, we assume all coordinates
// are 2-dimensional
void CShaderShadowVulkan::VertexShaderVertexFormat(
    unsigned int nFlags, int nTexCoordCount, int *pTexCoordDimensions, int nUserDataSize )
{
	// As CShaderShadowDX8: meshes, not shaders, declare bone indices.
	nFlags &= ~VERTEX_BONE_INDEX;
	nFlags |= VERTEX_FORMAT_VERTEX_SHADER;
	m_vertexUsage =
	    NativeVertexFormat( nFlags, nTexCoordCount, pTexCoordDimensions, 0, nUserDataSize );
}

// Indicates we're going to light the model
void CShaderShadowVulkan::EnableLighting( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::EnableSpecular( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

// Activate/deactivate skinning
void CShaderShadowVulkan::EnableVertexBlend( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

// per texture unit stuff
void CShaderShadowVulkan::OverbrightValue( TextureStage_t stage, float value )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::EnableTexture( Sampler_t stage, bool bEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::EnableCustomPixelPipe( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::CustomTextureStages( int stageCount )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::CustomTextureOperation( TextureStage_t stage, ShaderTexChannel_t channel,
    ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2 )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::EnableTexGen( TextureStage_t stage, bool bEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::TexGen( TextureStage_t stage, ShaderTexGenParam_t param )
{
	VK_UNIMPLEMENTED();
}

// Sets the vertex and pixel shaders
void CShaderShadowVulkan::SetVertexShader( const char *pShaderName, int vshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
	Q_strncpy( m_vertexShaderName, pShaderName ? pShaderName : "", sizeof( m_vertexShaderName ) );
}

void CShaderShadowVulkan::EnableBlendingSeparateAlpha( bool bEnable )
{
	VK_UNIMPLEMENTED();
}
void CShaderShadowVulkan::SetPixelShader( const char *pShaderName, int pshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
	m_pixelShaderIndex = pshIndex;
	if ( pShaderName )
	{
		Q_strncpy( m_pixelShaderName, pShaderName, sizeof( m_pixelShaderName ) );
	}
	else
	{
		m_pixelShaderName[0] = '\0';
	}
}

void CShaderShadowVulkan::BlendFuncSeparateAlpha(
    ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor )
{
	VK_UNIMPLEMENTED();
}
// indicates what per-vertex data we're providing
void CShaderShadowVulkan::DrawFlags( unsigned int drawFlags )
{
	VK_UNIMPLEMENTED();
}

//-----------------------------------------------------------------------------
//
// Shader API Empty
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor, destructor
//-----------------------------------------------------------------------------

CShaderAPIVulkan::CShaderAPIVulkan() : m_Mesh( true )
{
}

CShaderAPIVulkan::~CShaderAPIVulkan()
{
}

bool CShaderAPIVulkan::DoRenderTargetsNeedSeparateDepthBuffer() const
{
	return false;
}

// Can we download textures?
bool CShaderAPIVulkan::CanDownloadTextures() const
{
	// True once the device is up: CTexture::Download gates every material texture
	// download on this, so returning false leaves all world/model VTFs unresident
	// (they fall back to the debug texture). This is a real rendering backend.
	return g_VulkanContext.IsValid();
}

// Used to clear the transition table when we know it's become invalid.
void CShaderAPIVulkan::ClearSnapshots()
{
	VK_UNIMPLEMENTED();
}

// Members of IMaterialSystemHardwareConfig
bool CShaderAPIVulkan::HasDestAlphaBuffer() const
{
	return false;
}

bool CShaderAPIVulkan::HasStencilBuffer() const
{
	return g_VulkanContext.StencilBits() > 0;
}

int CShaderAPIVulkan::MaxViewports() const
{
	return 1;
}

int CShaderAPIVulkan::GetShadowFilterMode() const
{
	return 0;
}

int CShaderAPIVulkan::StencilBufferBits() const
{
	return g_VulkanContext.StencilBits();
}

int CShaderAPIVulkan::GetFrameBufferColorDepth() const
{
	return 0;
}

int CShaderAPIVulkan::GetSamplerCount() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 60 ) )
		return 1;
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel >= 60 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 80 ) )
		return 2;
	return 4;
}

bool CShaderAPIVulkan::HasSetDeviceGammaRamp() const
{
	return false;
}

// DXT1/DXT3/DXT5 stay compressed, as on D3D9: the sampler decodes the blocks.
// Reporting false makes the material system decompress them in software, whose
// rounding differs from the hardware decode by up to half a unit in 8 bits.
bool CShaderAPIVulkan::SupportsCompressedTextures() const
{
	return g_VulkanContext.SupportsBlockCompression();
}

VertexCompressionType_t CShaderAPIVulkan::SupportsCompressedVertices() const
{
	return VERTEX_COMPRESSION_NONE;
}

bool CShaderAPIVulkan::SupportsVertexAndPixelShaders() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 80 ) )
		return false;

	return true;
}

bool CShaderAPIVulkan::SupportsPixelShaders_1_4() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 81 ) )
		return false;

	return true;
}

bool CShaderAPIVulkan::SupportsPixelShaders_2_0() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 90 ) )
		return false;

	return true;
}

bool CShaderAPIVulkan::SupportsPixelShaders_2_b() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 90 ) )
		return false;

	return true;
}

bool CShaderAPIVulkan::ActuallySupportsPixelShaders_2_b() const
{
	return true;
}

bool CShaderAPIVulkan::SupportsShaderModel_3_0() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 95 ) )
		return false;

	return true;
}

bool CShaderAPIVulkan::SupportsStaticControlFlow() const
{
	if ( IsOpenGL() )
		return false;

	return SupportsVertexShaders_2_0();
}

bool CShaderAPIVulkan::SupportsVertexShaders_2_0() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 90 ) )
		return false;

	return true;
}

int CShaderAPIVulkan::MaximumAnisotropicLevel() const
{
	return 0;
}

void CShaderAPIVulkan::SetAnisotropicLevel( int nAnisotropyLevel )
{
	VK_UNIMPLEMENTED();
}

int CShaderAPIVulkan::MaxTextureWidth() const
{
	// Should be big enough to cover all cases
	return 16384;
}

int CShaderAPIVulkan::MaxTextureHeight() const
{
	// Should be big enough to cover all cases
	return 16384;
}

int CShaderAPIVulkan::MaxTextureAspectRatio() const
{
	// Should be big enough to cover all cases
	return 16384;
}

int CShaderAPIVulkan::TextureMemorySize() const
{
	// fake it
	return 64 * 1024 * 1024;
}

int CShaderAPIVulkan::GetDXSupportLevel() const
{
	return 90;
}

bool CShaderAPIVulkan::SupportsOverbright() const
{
	return false;
}

bool CShaderAPIVulkan::SupportsCubeMaps() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 70 ) )
		return false;

	return true;
}

bool CShaderAPIVulkan::SupportsNonPow2Textures() const
{
	return true;
}

bool CShaderAPIVulkan::SupportsMipmappedCubemaps() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 70 ) )
		return false;

	return true;
}

int CShaderAPIVulkan::GetTextureStageCount() const
{
	return 4;
}

int CShaderAPIVulkan::NumVertexShaderConstants() const
{
	return 128;
}

int CShaderAPIVulkan::NumBooleanVertexShaderConstants() const
{
	return 0;
}

int CShaderAPIVulkan::NumIntegerVertexShaderConstants() const
{
	return 0;
}

int CShaderAPIVulkan::NumPixelShaderConstants() const
{
	return 8;
}

int CShaderAPIVulkan::MaxNumLights() const
{
	return 4;
}

bool CShaderAPIVulkan::SupportsSpheremapping() const
{
	return false;
}

// This is the max dx support level supported by the card
int CShaderAPIVulkan::GetMaxDXSupportLevel() const
{
	return 90;
}

bool CShaderAPIVulkan::SupportsHardwareLighting() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 70 ) )
		return false;

	return true;
}

int CShaderAPIVulkan::MaxBlendMatrices() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 70 ) )
	{
		return 1;
	}

	return 0;
}

int CShaderAPIVulkan::MaxBlendMatrixIndices() const
{
	if ( ( ShaderUtil()->GetConfig().dxSupportLevel > 0 ) &&
	     ( ShaderUtil()->GetConfig().dxSupportLevel < 70 ) )
	{
		return 1;
	}

	return 0;
}

int CShaderAPIVulkan::MaxVertexShaderBlendMatrices() const
{
	return 0;
}

// D3D9 user clip planes are clip distances here (see CommitUserClipPlanes). The
// D3D9 backend reports the device's count (6 on DX9 hardware); this backend
// carries two in its push constants, which is what portal recursion at the
// client's default depth (r_portal_stencil_depth 2) pushes. With more planes the
// material system keeps the last two, as it does on any two-plane device.
int CShaderAPIVulkan::MaxUserClipPlanes() const
{
	return g_VulkanContext.MaxClipPlanes();
}

bool CShaderAPIVulkan::SpecifiesFogColorInLinearSpace() const
{
	return false;
}

bool CShaderAPIVulkan::SupportsSRGB() const
{
	return false;
}

bool CShaderAPIVulkan::FakeSRGBWrite() const
{
	return false;
}

bool CShaderAPIVulkan::CanDoSRGBReadFromRTs() const
{
	return true;
}

bool CShaderAPIVulkan::SupportsGLMixedSizeTargets() const
{
	return false;
}

const char *CShaderAPIVulkan::GetHWSpecificShaderDLLName() const
{
	return 0;
}

// Sets the default *dynamic* state
void CShaderAPIVulkan::SetDefaultState()
{
	VK_UNIMPLEMENTED();
}

// Snapshot -> selected pixel shader name, so BeginPass can bind the matching
// native pipeline. The snapshot id keeps the existing flag bits (0..3) and packs
// the table index in the high bits, preserving IsTranslucent()/etc.
static std::vector<std::string> g_snapshotShaders;
// Parallel to g_snapshotShaders: the blend and depth state each snapshot draws
// with, taken from the recorded IShaderShadow state so BeginPass can select the
// matching pipeline.
static std::vector<render_vulkan::CVulkanContext::DynRasterState> g_snapshotRaster;
// Parallel to g_snapshotShaders: the $alphatest reference each snapshot applies
// (< 0 when alpha test is disabled).
static std::vector<float> g_snapshotAlphaRef;
// Parallel to g_snapshotShaders: the vertex usage each snapshot's shader declared.
static std::vector<VertexFormat_t> g_snapshotVertexUsage;
// Parallel to g_snapshotShaders: the kColorSrgb* flags each snapshot declared.
static std::vector<int> g_snapshotColorFlags;
// Snapshot ids are 16-bit (StateSnapshot_t is a short): four flag bits and an
// 11-bit table index. As in D3D9's transition table, identical shadow states
// share one snapshot, so the table holds distinct states rather than one entry
// per TakeSnapshot call; appending per call wrapped the index past 2048 and bound
// other materials' blend state.
static const size_t kMaxSnapshots = 0x800;
static std::map<std::string, StateSnapshot_t> g_snapshotIds;

// Faithful vertex-shader constant register file. The material system commits its
// standard and shader-specific constants to fixed registers (see
// stdshaders/common_vs_fxc.h); honoring the real register numbers -- rather than
// a bespoke convention -- is what makes the native path a substitutable D3D9
// backend. The registers the UnlitGeneric family consumes, named by the one
// authoritative enum (materialsystem/ishadersystem_declarations.h) that the
// shaders' own dynamic state writes through:
//   c4-c7   cModelViewProj        (VERTEX_SHADER_MODELVIEWPROJ)
//   c47     cModulationColor      (VERTEX_SHADER_MODULATION_COLOR)
//   c48-c49 cBaseTextureTransform (VERTEX_SHADER_SHADER_SPECIFIC_CONST_0/1)
// common_vs_fxc.h also defines an older vs_1_1 layout (c37/c38); the DX9
// shaders are vs_2_0 and do not use it.
// c0-c3 is retained as a legacy alias for the model->projection matrix so the
// direct-interface harnesses that predate the matrix stack keep working.
namespace
{
enum
{
	kVsRegModelViewProj = VERTEX_SHADER_MODELVIEWPROJ, // cModelViewProj, 4 registers
	kVsRegModelViewProjLegacy = 0,                     // legacy c0-c3 alias
	kVsRegModulationColor = VERTEX_SHADER_MODULATION_COLOR,
	kVsRegBaseTexTransform = VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, // [0..1]
	kVsRegCount = 64
};
struct VsConstantFile
{
	float regs[kVsRegCount][4] = {};
	bool written[kVsRegCount] = {};

	VsConstantFile()
	{
		// Only the UnlitGeneric family publishes its modulation and base-texture
		// transform through c47/c48-c49. Every other Source shader
		// (LightmappedGeneric, VertexLitGeneric, ...) leaves those registers
		// untouched, and the native textured pipeline multiplies by them
		// unconditionally -- so a zero-initialized register file renders the
		// whole world black and collapses every UV to the origin. Start them at
		// the neutral values instead: modulation white, texture transform
		// identity. A shader that does set them still overrides these.
		regs[kVsRegModulationColor][0] = 1.0f;
		regs[kVsRegModulationColor][1] = 1.0f;
		regs[kVsRegModulationColor][2] = 1.0f;
		regs[kVsRegModulationColor][3] = 1.0f;
		regs[kVsRegBaseTexTransform][0] = 1.0f;
		regs[kVsRegBaseTexTransform + 1][1] = 1.0f;
	}
};
VsConstantFile g_vsConstants;

// A pass begins: return the material registers the native textured pipeline
// reads to their identities. D3D9 constants persist between draws, but a vs_2_0
// shader that reads cModulationColor or cBaseTextureTransform writes them in the
// same pass's dynamic state; one that does not (LightmappedGeneric's fast path
// uses the raw UV) never reads them. The native pipeline reads them for every
// draw, so without this reset a surface inherits the previous material's values
// -- collapsing its UVs to a single texel.
void ResetPassMaterialConstants()
{
	static const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const float identityRows[8] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
	memcpy( g_vsConstants.regs[kVsRegModulationColor], white, sizeof( white ) );
	memcpy( g_vsConstants.regs[kVsRegBaseTexTransform], identityRows, sizeof( identityRows ) );
	g_VulkanContext.SetDynamicModulation( white );
	g_VulkanContext.SetDynamicBaseTexTransform( identityRows, identityRows + 4 );
}

// Push the constants the native UnlitGeneric pipeline consumes to the context,
// preferring the faithful cModelViewProj (c4) when the material system has set
// it, falling back to the legacy c0 alias otherwise.
void CommitDynamicVsConstants()
{
	if ( g_vsConstants.written[kVsRegModelViewProj] )
		g_VulkanContext.SetDynamicTransform( &g_vsConstants.regs[kVsRegModelViewProj][0] );
	else if ( g_vsConstants.written[kVsRegModelViewProjLegacy] )
		g_VulkanContext.SetDynamicTransform( &g_vsConstants.regs[kVsRegModelViewProjLegacy][0] );

	// Always commit these: the register file holds neutral defaults for shaders
	// that never set them, so the draw is modulated by white through an identity
	// texture transform rather than by an unwritten (zero) register.
	g_VulkanContext.SetDynamicModulation( &g_vsConstants.regs[kVsRegModulationColor][0] );
	g_VulkanContext.SetDynamicBaseTexTransform( &g_vsConstants.regs[kVsRegBaseTexTransform][0],
	    &g_vsConstants.regs[kVsRegBaseTexTransform + 1][0] );
}

// The native raster state of a shadow state: the blend factors and depth state
// exactly as the material's IShaderShadow calls set them, which is what the D3D9
// backend hands the fixed-function output merger.
VkBlendFactor NativeBlendFactor( ShaderBlendFactor_t factor )
{
	switch ( factor )
	{
	case SHADER_BLEND_ZERO:
		return VK_BLEND_FACTOR_ZERO;
	case SHADER_BLEND_ONE:
		return VK_BLEND_FACTOR_ONE;
	case SHADER_BLEND_DST_COLOR:
		return VK_BLEND_FACTOR_DST_COLOR;
	case SHADER_BLEND_ONE_MINUS_DST_COLOR:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case SHADER_BLEND_SRC_ALPHA:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case SHADER_BLEND_ONE_MINUS_SRC_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case SHADER_BLEND_DST_ALPHA:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case SHADER_BLEND_ONE_MINUS_DST_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case SHADER_BLEND_SRC_ALPHA_SATURATE:
		return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
	case SHADER_BLEND_SRC_COLOR:
		return VK_BLEND_FACTOR_SRC_COLOR;
	case SHADER_BLEND_ONE_MINUS_SRC_COLOR:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	}
	NoteUnimplemented( "BlendFunc(unknown factor)" );
	return VK_BLEND_FACTOR_ONE;
}

VkCompareOp NativeDepthCompare( ShaderDepthFunc_t func )
{
	switch ( func )
	{
	case SHADER_DEPTHFUNC_NEVER:
		return VK_COMPARE_OP_NEVER;
	case SHADER_DEPTHFUNC_NEARER:
		return VK_COMPARE_OP_LESS;
	case SHADER_DEPTHFUNC_EQUAL:
		return VK_COMPARE_OP_EQUAL;
	case SHADER_DEPTHFUNC_NEAREROREQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case SHADER_DEPTHFUNC_FARTHER:
		return VK_COMPARE_OP_GREATER;
	case SHADER_DEPTHFUNC_NOTEQUAL:
		return VK_COMPARE_OP_NOT_EQUAL;
	case SHADER_DEPTHFUNC_FARTHEROREQUAL:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case SHADER_DEPTHFUNC_ALWAYS:
		return VK_COMPARE_OP_ALWAYS;
	}
	NoteUnimplemented( "DepthFunc(unknown function)" );
	return VK_COMPARE_OP_LESS_OR_EQUAL;
}

render_vulkan::CVulkanContext::DynRasterState SnapshotRasterState(
    const CShaderShadowVulkan &shadow )
{
	render_vulkan::CVulkanContext::DynRasterState state;
	state.blend = shadow.m_IsTranslucent;
	if ( state.blend )
	{
		state.srcFactor = NativeBlendFactor( shadow.m_blendSrc );
		state.dstFactor = NativeBlendFactor( shadow.m_blendDst );
	}
	state.depthTest = shadow.m_bIsDepthTestEnabled;
	state.depthWrite = shadow.m_bIsDepthWriteEnabled;
	state.depthCompare = NativeDepthCompare( shadow.m_depthFunc );
	state.colorWrite = shadow.m_colorWrites;
	// Culling on; the dynamic CullMode picks the face when the pass is drawn.
	state.cullMode = shadow.m_cullEnable ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	return state;
}

// The textured pipeline variant a snapshot selects, beyond its sRGB flags: the
// alpha-test comparison, and the D3D9 shaders it reproduces other than the base
// texture times modulation (see demo_dyn_tex.{vert,frag}).
static int SnapshotShaderFlags( const CShaderShadowVulkan &shadow )
{
	using render_vulkan::CVulkanContext;
	int flags = shadow.m_colorFlags;
	if ( shadow.m_IsAlphaTested )
	{
		if ( shadow.m_alphaFunc == SHADER_ALPHAFUNC_GREATER )
			flags |= CVulkanContext::kFragmentAlphaGreater;
		else if ( shadow.m_alphaFunc != SHADER_ALPHAFUNC_GEQUAL &&
		          shadow.m_alphaFunc != SHADER_ALPHAFUNC_ALWAYS )
			NoteUnimplemented( "alpha test comparison other than GEQUAL/GREATER/ALWAYS" );
	}
	// luminance_compare_ps20 and its ps20b build (screenspace_general picks it).
	if ( !V_strnicmp( shadow.m_pixelShaderName, "luminance_compare_ps20", 22 ) )
		flags |= CVulkanContext::kFragmentLuminanceCompare;
	if ( !V_stricmp( shadow.m_vertexShaderName, "screenspaceeffect_vs20" ) )
		flags |= CVulkanContext::kVertexScreenSpace;
	// bufferclearobeystencil_vs20 passes clip-space positions and the vertex
	// color (DrawClearBufferQuad's), which bufferclearobeystencil_ps2x returns.
	if ( !V_stricmp( shadow.m_vertexShaderName, "bufferclearobeystencil_vs20" ) )
		flags |= CVulkanContext::kVertexScreenSpace | CVulkanContext::kFragmentVertexColor;
	// vertexlit_and_unlit_generic (UnlitGeneric: VGUI, fonts, sprites) with a
	// vertex color stream, which its vertex shader's VERTEXCOLOR combo reads
	// ($vertexcolor or $vertexalpha; vertexlitgeneric_dx9_helper.cpp). The pixel
	// shader's VERTEXCOLOR static combo ($vertexcolor, stride 384 in every
	// ps20/ps20b/ps30 build) multiplies by its color unless DIFFUSELIGHTING
	// (stride 24) makes that color vertex lighting. Its alpha is weighted by
	// g_fVertexAlpha, a dynamic constant (RenderPass). The color is not gamma
	// converted when the material does not write sRGB.
	if ( !V_strnicmp( shadow.m_pixelShaderName, "vertexlit_and_unlit_generic_ps", 30 ) &&
	     ( shadow.m_vertexUsage & VERTEX_COLOR ) )
	{
		const int index = shadow.m_pixelShaderIndex;
		if ( ( index / 384 ) % 2 && !( ( index / 24 ) % 2 ) )
			flags |= CVulkanContext::kFragmentModulateVertexColor;
		if ( !( shadow.m_colorFlags & CVulkanContext::kColorSrgbWrite ) )
			flags |= CVulkanContext::kVertexColorNoGammaConvert;
		flags |= CVulkanContext::kFragmentModulateVertexAlpha;
	}
	return flags;
}

// --- Transform matrix stack -------------------------------------------------
// The material system positions geometry through IShaderAPI's matrix stack
// (MatrixMode/LoadMatrix/MultMatrix/...), NOT by setting cModelViewProj directly
// -- shaderapidx8 composes and commits that constant internally before each draw
// (SetVertexShaderModelViewProjAndModelView). This backend does the same so world
// and model geometry receives its real transform instead of identity.
//
// Convention: CMatRenderContext hands us each matrix already TRANSPOSED
// (`MatrixTranspose(...); g_pShaderAPI->LoadMatrix(...)`), stored row-major. The
// native GLSL vertex shader is column-vector (`gl_Position = mvp * pos`) and reads
// the pushed 16 floats column-major. Working the D3D transpose and the HLSL/GLSL
// packing through end to end, the matrix the GLSL shader needs is exactly
// model_stored * view_stored * proj_stored (standard row-major multiply, laid out
// row-major) -- no extra transpose. Verified against the live Portal render.
struct MatrixStackState
{
	float mat[NUM_MATRIX_MODES][16];
	std::vector<std::array<float, 16>> stack[NUM_MATRIX_MODES];
	MaterialMatrixMode_t mode = MATERIAL_MODEL;
	bool initialized = false;
};
MatrixStackState g_matrices;

void MatSetIdentity( float *m )
{
	for ( int i = 0; i < 16; ++i )
		m[i] = ( i % 5 == 0 ) ? 1.0f : 0.0f;
}

// Row-major multiply: out = a * b (out[i][j] = sum_k a[i][k] * b[k][j]).
void MatMul( const float *a, const float *b, float *out )
{
	float t[16];
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
		{
			float s = 0.0f;
			for ( int k = 0; k < 4; ++k )
				s += a[i * 4 + k] * b[k * 4 + j];
			t[i * 4 + j] = s;
		}
	for ( int i = 0; i < 16; ++i )
		out[i] = t[i];
}

void EnsureMatricesInit()
{
	if ( g_matrices.initialized )
		return;
	for ( int m = 0; m < NUM_MATRIX_MODES; ++m )
		MatSetIdentity( g_matrices.mat[m] );
	g_matrices.initialized = true;
}

float *CurrentMatrix()
{
	EnsureMatricesInit();
	return g_matrices.mat[g_matrices.mode];
}

// Compose model * view * proj and hand it to the dynamic draw path as the MVP.
void CommitModelViewProj()
{
	EnsureMatricesInit();
	float mv[16], mvp[16];
	MatMul( g_matrices.mat[MATERIAL_MODEL], g_matrices.mat[MATERIAL_VIEW], mv );
	MatMul( mv, g_matrices.mat[MATERIAL_PROJECTION], mvp );
	g_VulkanContext.SetDynamicTransform( mvp );
}

// view * proj, for world-space (skinned) positions.
void CommitViewProj()
{
	EnsureMatricesInit();
	float vp[16];
	MatMul( g_matrices.mat[MATERIAL_VIEW], g_matrices.mat[MATERIAL_PROJECTION], vp );
	g_VulkanContext.SetDynamicTransform( vp );
}

// Inverse of a row-major 4x4 matrix by Gauss-Jordan elimination with partial
// pivoting; false when it is singular.
bool MatInvert( const float *m, float *out )
{
	double a[4][8];
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 8; ++j )
			a[i][j] = j < 4 ? m[i * 4 + j] : ( j - 4 == i ? 1.0 : 0.0 );
	for ( int col = 0; col < 4; ++col )
	{
		int pivot = col;
		for ( int row = col + 1; row < 4; ++row )
			if ( fabs( a[row][col] ) > fabs( a[pivot][col] ) )
				pivot = row;
		if ( fabs( a[pivot][col] ) < 1e-20 )
			return false;
		for ( int j = 0; j < 8; ++j )
			std::swap( a[col][j], a[pivot][j] );
		const double inv = 1.0 / a[col][col];
		for ( int j = 0; j < 8; ++j )
			a[col][j] *= inv;
		for ( int row = 0; row < 4; ++row )
		{
			if ( row == col )
				continue;
			const double f = a[row][col];
			for ( int j = 0; j < 8; ++j )
				a[row][j] -= f * a[col][j];
		}
	}
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
			out[i * 4 + j] = static_cast<float>( a[i][j + 4] );
	return true;
}

// The enabled user clip planes in clip space, as CShaderAPIDx8::
// CommitUserClipPlanes computes them for a vertex shader: each world plane is
// transformed by the inverse transpose of worldToView * projection, where
// worldToView is the view matrix unless a user clip transform overrides it. The
// stored matrices are D3D's (row vectors), so a clip-space plane p' satisfies
// p' . clip == p . world. Suppressed around the stencil-obeying quad clears, as
// D3D9 disables D3DRS_CLIPPLANEENABLE there.
void CommitUserClipPlanes()
{
	float planes[render_vulkan::CVulkanContext::kMaxClipPlanes][4];
	int count = 0;
	if ( !g_ClipPlanesSuppressed && g_ClipPlanesEnabled )
	{
		EnsureMatricesInit();
		float worldToProjection[16], inverse[16];
		MatMul( g_UserClipTransformOverride ? g_UserClipTransform : g_matrices.mat[MATERIAL_VIEW],
		    g_matrices.mat[MATERIAL_PROJECTION], worldToProjection );
		if ( MatInvert( worldToProjection, inverse ) )
		{
			for ( int i = 0; i < render_vulkan::CVulkanContext::kMaxClipPlanes; ++i )
			{
				if ( !( g_ClipPlanesEnabled & ( 1 << i ) ) )
					continue;
				// p'_j = sum_k p_k * inverse[j][k]  (p * (M^-1)^T)
				for ( int j = 0; j < 4; ++j )
				{
					float sum = 0.0f;
					for ( int k = 0; k < 4; ++k )
						sum += g_ClipPlanesWorld[i][k] * inverse[j * 4 + k];
					planes[count][j] = sum;
				}
				++count;
			}
		}
		else
			NoteUnimplemented( "user clip planes: singular world-to-projection transform" );
	}
	g_VulkanContext.SetDynamicClipPlanes( count, planes );
}

VkStencilOp NativeStencilOp( StencilOperation_t op )
{
	switch ( op )
	{
	case STENCILOPERATION_ZERO:
		return VK_STENCIL_OP_ZERO;
	case STENCILOPERATION_REPLACE:
		return VK_STENCIL_OP_REPLACE;
	case STENCILOPERATION_INCRSAT:
		return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case STENCILOPERATION_DECRSAT:
		return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case STENCILOPERATION_INVERT:
		return VK_STENCIL_OP_INVERT;
	case STENCILOPERATION_INCR:
		return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case STENCILOPERATION_DECR:
		return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	default:
		return VK_STENCIL_OP_KEEP;
	}
}

VkCompareOp NativeStencilCompare( StencilComparisonFunction_t func )
{
	switch ( func )
	{
	case STENCILCOMPARISONFUNCTION_NEVER:
		return VK_COMPARE_OP_NEVER;
	case STENCILCOMPARISONFUNCTION_LESS:
		return VK_COMPARE_OP_LESS;
	case STENCILCOMPARISONFUNCTION_EQUAL:
		return VK_COMPARE_OP_EQUAL;
	case STENCILCOMPARISONFUNCTION_LESSEQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case STENCILCOMPARISONFUNCTION_GREATER:
		return VK_COMPARE_OP_GREATER;
	case STENCILCOMPARISONFUNCTION_NOTEQUAL:
		return VK_COMPARE_OP_NOT_EQUAL;
	case STENCILCOMPARISONFUNCTION_GREATEREQUAL:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	default:
		return VK_COMPARE_OP_ALWAYS;
	}
}

// D3D9's stencil render state onto a draw's raster state and dynamic values. The
// reference is compared as D3D does: ref & mask against stencil & mask, with the
// comparison "ref OP stencil", which is Vulkan's order too.
void ApplyStencilState( render_vulkan::CVulkanContext::DynRasterState &raster )
{
	raster.stencilEnable = g_Stencil.enable;
	if ( g_Stencil.enable )
	{
		raster.stencilCompare = NativeStencilCompare( g_Stencil.compare );
		raster.stencilFail = NativeStencilOp( g_Stencil.fail );
		raster.stencilDepthFail = NativeStencilOp( g_Stencil.depthFail );
		raster.stencilPass = NativeStencilOp( g_Stencil.pass );
	}
	g_VulkanContext.SetDynamicStencilValues( static_cast<uint32_t>( g_Stencil.reference ) & 0xFF,
	    g_Stencil.testMask & 0xFF, g_Stencil.writeMask & 0xFF );
}

// PortalRefract's registers (portal_refract_helper.cpp's dynamic state) for the
// draw: the model and view-projection from the matrix stack, as cModel[0] and
// cViewProj are committed on D3D9; time and the texture transform from the
// vertex constants it writes (SHADER_SPECIFIC_CONST_0..2); open amount, active
// and color scale from pixel constant c4.
void CommitPortalConstants()
{
	EnsureMatricesInit();
	render_vulkan::CVulkanContext::PortalConstants c;
	memcpy( c.model, g_matrices.mat[MATERIAL_MODEL], sizeof( c.model ) );
	MatMul( g_matrices.mat[MATERIAL_VIEW], g_matrices.mat[MATERIAL_PROJECTION], c.viewProj );
	memcpy( c.texXform0, g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_1],
	    sizeof( c.texXform0 ) );
	memcpy( c.texXform1, g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_2],
	    sizeof( c.texXform1 ) );
	c.time = g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_0][0];
	c.openAmount = g_psConstants[4][0];
	c.active = g_psConstants[4][1];
	c.colorScale = g_psConstants[4][2];
	c.stage = g_CurrentPortalStage;
	g_VulkanContext.SetDynamicPortalConstants( c );
}
} // namespace

// Returns the snapshot id for the shader state
// The name BeginPass routes a snapshot by: its pixel shader, with the static
// combo where the combo changes what the native pipeline computes
// ("portal_refract#<STAGE>"), or its vertex shader when it has no pixel shader
// (WriteZ, a depth- or stencil-only BufferClearObeyStencil).
static std::string SnapshotShaderRoute( const CShaderShadowVulkan &shadow )
{
	if ( !V_strnicmp( shadow.m_pixelShaderName, "portal_refract_ps20b", 20 ) )
		return "portal_refract#" + std::to_string( ( shadow.m_pixelShaderIndex / 4 ) % 3 );
	if ( !V_strnicmp( shadow.m_pixelShaderName, "portal_refract_ps20", 19 ) )
		return "portal_refract#" + std::to_string( shadow.m_pixelShaderIndex % 3 );
	if ( !shadow.m_pixelShaderName[0] )
		return std::string( "vs:" ) + shadow.m_vertexShaderName;
	return shadow.m_pixelShaderName;
}

StateSnapshot_t CShaderAPIVulkan::TakeSnapshot()
{
	StateSnapshot_t id = 0;
	if ( g_ShaderShadow.m_IsTranslucent )
		id |= TRANSLUCENT;
	if ( g_ShaderShadow.m_IsAlphaTested )
		id |= ALPHATESTED;
	if ( g_ShaderShadow.m_bUsesVertexAndPixelShaders )
		id |= VERTEX_AND_PIXEL_SHADERS;
	if ( g_ShaderShadow.m_bIsDepthWriteEnabled )
		id |= DEPTHWRITE;

	const render_vulkan::CVulkanContext::DynRasterState raster =
	    SnapshotRasterState( g_ShaderShadow );
	// D3D9 holds the alpha reference as an integer 0..255, truncating (see
	// shadershadowdx8.cpp AlphaFunc); the same reference is applied here.
	// ALWAYS passes every fragment, as no test.
	const float alphaRef =
	    g_ShaderShadow.m_IsAlphaTested && g_ShaderShadow.m_alphaFunc != SHADER_ALPHAFUNC_ALWAYS
	        ? static_cast<int>( g_ShaderShadow.m_alphaRef * 255 ) / 255.0f
	        : -1.0f;
	const int shaderFlags = SnapshotShaderFlags( g_ShaderShadow );
	char key[128];
	V_snprintf( key, sizeof( key ), "|%d|%u|%.6g|%llx|%d", static_cast<int>( id ),
	    render_vulkan::CVulkanContext::RasterStateKey( raster ), alphaRef,
	    static_cast<unsigned long long>( g_ShaderShadow.m_vertexUsage ), shaderFlags );
	const std::string stateKey = SnapshotShaderRoute( g_ShaderShadow ) + key;
	const auto existing = g_snapshotIds.find( stateKey );
	if ( existing != g_snapshotIds.end() )
		return existing->second;

	const size_t index = g_snapshotShaders.size();
	if ( index >= kMaxSnapshots )
	{
		// No silent aliasing: an id that cannot hold the index would bind another
		// state's shader and blend.
		Error( "shaderapivulkan: more than %d distinct shadow states\n",
		    static_cast<int>( kMaxSnapshots ) );
	}
	g_snapshotShaders.push_back( SnapshotShaderRoute( g_ShaderShadow ) );
	g_snapshotRaster.push_back( raster );
	g_snapshotAlphaRef.push_back( alphaRef );
	g_snapshotVertexUsage.push_back( g_ShaderShadow.m_vertexUsage );
	g_snapshotColorFlags.push_back( shaderFlags );
	// Flags occupy bits 0..3; the index fits the remaining 11 bits of the short.
	id = static_cast<StateSnapshot_t>( id | static_cast<int>( index << 4 ) );
	g_snapshotIds[stateKey] = id;
	return id;
}

// Returns true if the state snapshot is transparent
bool CShaderAPIVulkan::IsTranslucent( StateSnapshot_t id ) const
{
	return ( id & TRANSLUCENT ) != 0;
}

bool CShaderAPIVulkan::IsAlphaTested( StateSnapshot_t id ) const
{
	return ( id & ALPHATESTED ) != 0;
}

bool CShaderAPIVulkan::IsDepthWriteEnabled( StateSnapshot_t id ) const
{
	return ( id & DEPTHWRITE ) != 0;
}

bool CShaderAPIVulkan::UsesVertexAndPixelShaders( StateSnapshot_t id ) const
{
	return ( id & VERTEX_AND_PIXEL_SHADERS ) != 0;
}

// Gets the vertex format for a set of snapshot ids
VertexFormat_t CShaderAPIVulkan::ComputeVertexFormat(
    int numSnapshots, StateSnapshot_t *pIds ) const
{
	return ComputeVertexUsage( numSnapshots, pIds );
}

// The union of the vertex components a material's passes read, merged as
// CShaderAPIDx8::ComputeVertexUsage merges them: flags OR together, and each
// texture coordinate takes the widest size any pass declares.
VertexFormat_t CShaderAPIVulkan::ComputeVertexUsage( int numSnapshots, StateSnapshot_t *pIds ) const
{
	int flags = 0;
	int numBones = 0;
	int userDataSize = 0;
	int texCoordSize[VERTEX_MAX_TEXTURE_COORDINATES] = {};
	for ( int i = 0; i < numSnapshots; ++i )
	{
		const size_t index = static_cast<size_t>( ( pIds[i] >> 4 ) & 0x7FF );
		if ( index >= g_snapshotVertexUsage.size() )
			continue;
		const VertexFormat_t fmt = g_snapshotVertexUsage[index];
		flags |= VertexFlags( fmt );
		numBones = Max( numBones, NumBoneWeights( fmt ) );
		userDataSize = Max( userDataSize, UserDataSize( fmt ) );
		for ( int j = 0; j < VERTEX_MAX_TEXTURE_COORDINATES; ++j )
			texCoordSize[j] = Max( texCoordSize[j], TexCoordSize( j, fmt ) );
	}
	return NativeVertexFormat(
	    flags, VERTEX_MAX_TEXTURE_COORDINATES, texCoordSize, numBones, userDataSize );
}

// Uses a state snapshot
void CShaderAPIVulkan::UseSnapshot( StateSnapshot_t snapshot )
{
	VK_UNIMPLEMENTED();
}

// Sets the color to modulate by
void CShaderAPIVulkan::Color3f( float r, float g, float b )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Color3fv( float const *pColor )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Color4f( float r, float g, float b, float a )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Color4fv( float const *pColor )
{
	VK_UNIMPLEMENTED();
}

// Faster versions of color
void CShaderAPIVulkan::Color3ub( unsigned char r, unsigned char g, unsigned char b )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Color3ubv( unsigned char const *rgb )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Color4ub(
    unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Color4ubv( unsigned char const *rgba )
{
	VK_UNIMPLEMENTED();
}

// The shade mode
void CShaderAPIVulkan::ShadeMode( ShaderShadeMode_t mode )
{
	VK_UNIMPLEMENTED();
}

// Binds a particular material to render with
void CShaderAPIVulkan::Bind( IMaterial *pMaterial )
{
	// Record the material so IMesh::Draw can run its shader through the real
	// material path (see CEmptyMesh::Draw / RenderPass).
	g_pBoundMaterial = static_cast<IMaterialInternal *>( pMaterial );
	g_VulkanContext.SetRecordTag( MaterialTag( pMaterial ) );
}

// Cull mode
// The dynamic cull mode (CShaderAPIDx8::CullMode): CCW culls back faces, CW
// (mirrored views) culls front faces. It applies only to snapshots that enable
// culling, when each pass is drawn.
void CShaderAPIVulkan::CullMode( MaterialCullMode_t cullMode )
{
	if ( cullMode != MATERIAL_CULLMODE_CCW && cullMode != MATERIAL_CULLMODE_CW )
	{
		Warning( "CullMode: invalid cullMode\n" );
		return;
	}
	g_DesiredCullMode = cullMode;
}

void CShaderAPIVulkan::ForceDepthFuncEquals( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

// Forces Z buffering on or off
void CShaderAPIVulkan::OverrideDepthEnable( bool bEnable, bool bDepthEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::OverrideAlphaWriteEnable( bool bOverrideEnable, bool bAlphaWriteEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::OverrideColorWriteEnable( bool bOverrideEnable, bool bColorWriteEnable )
{
	VK_UNIMPLEMENTED();
}

//legacy fast clipping linkage
void CShaderAPIVulkan::SetHeightClipZ( float z )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode )
{
	VK_UNIMPLEMENTED();
}

// Sets the lights
void CShaderAPIVulkan::SetLight( int lightNum, const LightDesc_t &desc )
{
	VK_UNIMPLEMENTED();
}

// Sets lighting origin for the current model
void CShaderAPIVulkan::SetLightingOrigin( Vector vLightingOrigin )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetAmbientLight( float r, float g, float b )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetAmbientLightCube( Vector4D cube[6] )
{
	VK_UNIMPLEMENTED();
}

// Get lights
int CShaderAPIVulkan::GetMaxLights( void ) const
{
	return 0;
}

const LightDesc_t &CShaderAPIVulkan::GetLight( int lightNum ) const
{
	static LightDesc_t blah;
	return blah;
}

// Render state for the ambient light cube (vertex shaders)
void CShaderAPIVulkan::SetVertexShaderStateAmbientLightCube()
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetSkinningMatrices()
{
	VK_UNIMPLEMENTED();
}

// Lightmap texture binding
void CShaderAPIVulkan::BindLightmap( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::BindBumpLightmap( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::BindFullbrightLightmap( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::BindWhite( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::BindBlack( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::BindGrey( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

// Gets the lightmap dimensions
void CShaderAPIVulkan::GetLightmapDimensions( int *w, int *h )
{
	g_pShaderUtil->GetLightmapDimensions( w, h );
}

// Special system flat normal map binding.
void CShaderAPIVulkan::BindFlatNormalMap( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::BindNormalizationCubeMap( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::BindSignedNormalizationCubeMap( TextureStage_t stage )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::BindFBTexture( TextureStage_t stage, int textureIndex )
{
	VK_UNIMPLEMENTED();
}

// Flushes any primitives that are buffered
void CShaderAPIVulkan::FlushBufferedPrimitives()
{
	VK_UNIMPLEMENTED();
}

// Gets the dynamic mesh; note that you've got to render the mesh
// before calling this function a second time. Clients should *not*
// call DestroyStaticMesh on the mesh returned by this call.
IMesh *CShaderAPIVulkan::GetDynamicMesh( IMaterial *pMaterial, int nHWSkinBoneCount, bool buffered,
    IMesh *pVertexOverride, IMesh *pIndexOverride )
{
	return GetDynamicMeshEx(
	    pMaterial, 0, nHWSkinBoneCount, buffered, pVertexOverride, pIndexOverride );
}

IMesh *CShaderAPIVulkan::GetDynamicMeshEx( IMaterial *pMaterial, VertexFormat_t fmt,
    int nHWSkinBoneCount, bool buffered, IMesh *pVertexOverride, IMesh *pIndexOverride )
{
	// As in D3D9's mesh manager, the one dynamic mesh draws either its own
	// geometry or an override's: the world's index lists are built here over a
	// static mesh's vertices, and a batch passes this mesh back as its own index
	// override to keep the indices it just built.
	m_Mesh.SetSources( pVertexOverride, pIndexOverride );
	return &m_Mesh;
}

IMesh *CShaderAPIVulkan::GetFlexMesh()
{
	return &m_Mesh;
}

// Begins a rendering pass that uses a state snapshot
void CShaderAPIVulkan::BeginPass( StateSnapshot_t snapshot )
{
	ResetPassMaterialConstants();

	// Bind the material shader this snapshot selected: look up its recorded
	// pixel-shader name and route the dynamic-mesh draw to the matching native
	// Vulkan pipeline. This is how a material's chosen shader reaches the GPU.
	const size_t index = static_cast<size_t>( ( snapshot >> 4 ) & 0x7FF );
	if ( index < g_snapshotShaders.size() )
	{
		const std::string &name = g_snapshotShaders[index];
		// Map the bound shader to a native pipeline. The test-catalog names keep
		// their bespoke pipelines; every real Source material shader
		// (LightmappedGeneric, VertexLitGeneric/vertexlit_and_unlit_generic, World,
		// UnlitGeneric, ...) samples a base texture, so it routes to the textured
		// pipeline, which samples $basetexture (bound to sampler0) with the material
		// transform and modulation. Lightmaps/bump/env are not yet applied.
		int shader = render_vulkan::CVulkanContext::kDynShaderTextured;
		if ( name == "greenify" )
			shader = render_vulkan::CVulkanContext::kDynShaderGreenify;
		else if ( name == "constantcolor" )
			shader = render_vulkan::CVulkanContext::kDynShaderConstColor;
		else if ( name == "vertexcolor" || name == "vertex_passthrough" )
			shader = render_vulkan::CVulkanContext::kDynShaderPassthrough;
		g_CurrentPortalStage = -1;
		if ( !name.compare( 0, 15, "portal_refract#" ) )
		{
			shader = render_vulkan::CVulkanContext::kDynShaderPortalRefract;
			g_CurrentPortalStage = atoi( name.c_str() + 15 );
		}
		g_SamplesBaseTexture = ( shader == render_vulkan::CVulkanContext::kDynShaderTextured );
		// Shaders that sample nothing on sampler 0: WriteZ and the quad clears draw
		// depth, stencil or their vertex color; PortalRefract samples the frame
		// copy only in stage 0.
		if ( name == "vs:writez_vs20" || name == "vs:bufferclearobeystencil_vs20" ||
		     !name.compare( 0, 24, "bufferclearobeystencil_p" ) || g_CurrentPortalStage > 0 )
			g_SamplesBaseTexture = false;
		else if ( g_CurrentPortalStage == 0 )
			g_SamplesBaseTexture = true;
		g_VulkanContext.SelectDynamicShader( shader );
	}
	// Apply the blend and depth state this snapshot recorded.
	if ( index < g_snapshotRaster.size() )
	{
		g_CurrentRaster = g_snapshotRaster[index];
		g_VulkanContext.SelectDynamicRasterState( g_snapshotRaster[index] );
	}
	g_CurrentColorFlags = index < g_snapshotColorFlags.size() ? g_snapshotColorFlags[index] : 0;
	g_VulkanContext.SelectDynamicColorSpace( g_CurrentColorFlags );
	// The lightmap and samplers 1-2 are bound per pass by the shader's dynamic state.
	g_boundLightmapHandle = -1;
	g_VulkanContext.BindManagedLightmap( -1 );
	g_VulkanContext.BindManagedSampler( 1, -1 );
	g_VulkanContext.BindManagedSampler( 2, -1 );
	// Apply the $alphatest reference this snapshot recorded (< 0 = disabled).
	if ( index < g_snapshotAlphaRef.size() )
	{
		g_CurrentAlphaRef = g_snapshotAlphaRef[index];
		g_VulkanContext.SelectDynamicAlphaTest( g_snapshotAlphaRef[index] );
	}
}

// Renders a single pass of a material. The material's shader has, by now, run
// BeginPass (selecting the native pipeline + blend + alpha) and set its dynamic
// constants and textures. Emit the current mesh's geometry with that state.
// Material shaders whose output the native textured pipeline reproduces: the
// base texture at the mesh UVs times cModulationColor, composited with the
// material's blend and depth state. Their lightmap, detail and env-map terms are
// the tracked gaps. Every other shader samples or writes what this pipeline
// cannot express -- screen-space post-processing (Engine_Post, MotionBlur, the
// bloom downsample and blur), depth-only passes (WriteZ), refraction and portal
// surfaces addressed by screen position -- so drawing it as a base texture would
// paint the wrong image, often over the whole frame. Its draws are declined by
// name instead (census, dropped-material report and draw-state fixture).
static bool NativePipelineImplementsShader( const char *shaderName )
{
	static const char *const kImplemented[] = {
	    "LightmappedGeneric",
	    "WorldVertexTransition",
	    "VertexLitGeneric",
	    "UnlitGeneric",
	    "UnlitTwoTexture_DX9",
	    "Sprite_DX9",
	    "Spritecard",
	    "Cable_DX9",
	    "Shadow",
	    "DecalModulate",
	    // Depth-only, and the stencil-obeying clears (ClearBuffersObeyStencil).
	    "WriteZ_DX9",
	    "BufferClearObeyStencil_DX9",
	};
	for ( const char *name : kImplemented )
	{
		if ( V_stricmp( name, shaderName ) == 0 )
			return true;
	}
	// PortalRefract has its own pipeline (shaders/portal_refract.*), which a
	// device with too few push-constant bytes or clip distances does not get.
	if ( !V_stricmp( shaderName, "PortalRefract_dx9" ) )
		return g_VulkanContext.PortalPipelineSupported();
	return false;
}

void CShaderAPIVulkan::RenderPass( int nPass, int nPassCount )
{
	g_LastDropReason = "";
	// Without a bound material (the conformance fixtures drive the device
	// directly) the snapshot's own pipeline selection stands.
	// screenspace_general is one material shader over many pixel shaders; the
	// pass is implemented when its pixel shader is (dev/lumcompare's).
	const bool implemented =
	    !g_pBoundMaterial || NativePipelineImplementsShader( g_pBoundMaterial->GetShaderName() ) ||
	    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentLuminanceCompare );
	if ( !implemented )
	{
		DropDraw( "draw dropped: material shader not implemented by the native pipeline" );
		NoteDroppedMaterial();
	}
	else if ( g_pRenderMesh )
	{
		// LightmappedGeneric ends in FinalOutput( ..., TONEMAP_SCALE_LINEAR ): its
		// color is scaled by the tone-mapping scale, which is 1 without HDR. Other
		// shaders choose their tone-map type per combo and are not scaled yet.
		// D3D9's effective cull mode: the snapshot's culling with the dynamic face.
		render_vulkan::CVulkanContext::DynRasterState raster = g_CurrentRaster;
		if ( raster.cullMode != VK_CULL_MODE_NONE )
			raster.cullMode = g_DesiredCullMode == MATERIAL_CULLMODE_CW ? VK_CULL_MODE_FRONT_BIT
			                                                            : VK_CULL_MODE_BACK_BIT;
		ApplyStencilState( raster );
		g_VulkanContext.SelectDynamicRasterState( raster );
		CommitUserClipPlanes();
		if ( g_CurrentPortalStage >= 0 )
			CommitPortalConstants();
		const bool lightmapped = g_boundLightmapHandle >= 0;
		// FinalOutput( ..., TONEMAP_SCALE_LINEAR ) also ends PortalRefract's
		// stage 2 (the flames); its other stages do not scale.
		g_VulkanContext.SetDynamicOutputScale(
		    ( lightmapped || g_CurrentPortalStage == 2 ) ? g_ToneMappingScale.x : 1.0f );
		if ( !lightmapped && CurrentHDRType() == HDR_TYPE_INTEGER && g_ToneMappingScale.x != 1.0f )
			NoteUnimplemented( "integer HDR: tone-mapping scale on unlit/model shaders" );
		// vertexlit_and_unlit_generic's alpha = lerp( alpha, alpha * i.color.a,
		// g_fVertexAlpha ), where g_fVertexAlpha is c12.w ($vertexalpha, 0 or 1).
		int colorFlags = g_CurrentColorFlags;
		if ( ( colorFlags & render_vulkan::CVulkanContext::kFragmentModulateVertexAlpha ) &&
		     g_psConstants[12][3] < 0.5f )
			colorFlags &= ~render_vulkan::CVulkanContext::kFragmentModulateVertexAlpha;
		g_VulkanContext.SelectDynamicColorSpace( colorFlags );
		g_pRenderMesh->EmitToNativeQueue();
	}
	if ( drawstatefixture::Instance().Enabled() )
		RecordDrawStateFixture( nPass, nPassCount );
}

// Draw-state fixture (drawstatefixture.h) of the pass just rendered: what this
// backend applied, in the terms the D3D9 reference records, so the two can be
// compared field by field.
static const char *FixtureTextureName( int handle )
{
	if ( handle < 0 )
		return "(built-in white)";
	if ( static_cast<size_t>( handle ) < g_TextureRecords.size() )
		return g_TextureRecords[static_cast<size_t>( handle )].name.c_str();
	return "(unknown)";
}

static const char *FixtureBlendName( VkBlendFactor factor )
{
	switch ( factor )
	{
	case VK_BLEND_FACTOR_ZERO:
		return "zero";
	case VK_BLEND_FACTOR_ONE:
		return "one";
	case VK_BLEND_FACTOR_SRC_COLOR:
		return "src_color";
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
		return "one_minus_src_color";
	case VK_BLEND_FACTOR_DST_COLOR:
		return "dst_color";
	case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
		return "one_minus_dst_color";
	case VK_BLEND_FACTOR_SRC_ALPHA:
		return "src_alpha";
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		return "one_minus_src_alpha";
	case VK_BLEND_FACTOR_DST_ALPHA:
		return "dst_alpha";
	case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		return "one_minus_dst_alpha";
	case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
		return "src_alpha_saturate";
	default:
		return "other";
	}
}

void CShaderAPIVulkan::RecordDrawStateFixture( int nPass, int nPassCount )
{
	drawstatefixture::Draw draw;
	draw.material = g_pBoundMaterial ? g_pBoundMaterial->GetName() : "";
	draw.shader = g_pBoundMaterial ? g_pBoundMaterial->GetShaderName() : "";
	draw.pass = nPass;
	draw.passCount = nPassCount;
	const int target = g_VulkanContext.RenderTarget();
	if ( target >= 0 )
		draw.target = FixtureTextureName( target );
	ShaderViewport_t viewport;
	GetViewports( &viewport, 1 );
	if ( g_Viewport.m_nWidth <= 0 || g_Viewport.m_nHeight <= 0 )
		g_VulkanContext.GetRenderTargetExtent( viewport.m_nWidth, viewport.m_nHeight );
	draw.viewport[0] = viewport.m_nTopLeftX;
	draw.viewport[1] = viewport.m_nTopLeftY;
	draw.viewport[2] = viewport.m_nWidth;
	draw.viewport[3] = viewport.m_nHeight;
	draw.depthRange[0] = viewport.m_flMinZ;
	draw.depthRange[1] = viewport.m_flMaxZ;

	// The native pipelines sample one texture, sampler 0, and only the textured
	// pipeline samples it at all.
	if ( g_SamplesBaseTexture )
	{
		const int state = g_VulkanContext.ManagedTextureSamplerState( g_boundTextureHandle );
		drawstatefixture::Sampler &sampler = draw.samplers[draw.samplerCount++];
		sampler.stage = 0;
		sampler.texture = FixtureTextureName( g_boundTextureHandle );
		sampler.addressU =
		    ( state & render_vulkan::CVulkanContext::kSamplerClampU ) ? "clamp" : "wrap";
		sampler.addressV =
		    ( state & render_vulkan::CVulkanContext::kSamplerClampV ) ? "clamp" : "wrap";
		sampler.filter =
		    ( state & render_vulkan::CVulkanContext::kSamplerLinear ) ? "linear" : "point";
	}

	// The blend and depth state of the pipeline actually bound. Only the textured
	// pipeline honors it; the others draw opaque with the default depth state.
	render_vulkan::CVulkanContext::DynRasterState raster;
	if ( g_SamplesBaseTexture )
		raster = g_CurrentRaster;
	draw.blend = raster.blend;
	draw.srcBlend = FixtureBlendName( raster.srcFactor );
	draw.dstBlend = FixtureBlendName( raster.dstFactor );
	draw.depthTest = raster.depthTest;
	draw.depthWrite = raster.depthWrite;
	draw.alphaTestRef = g_CurrentAlphaRef;
	memcpy( draw.modulation, g_vsConstants.regs[kVsRegModulationColor], sizeof( draw.modulation ) );
	memcpy( draw.baseTextureTransform, g_vsConstants.regs[kVsRegBaseTexTransform],
	    sizeof( draw.baseTextureTransform ) );
	draw.submitted = g_LastDropReason[0] == 0;
	draw.dropReason = g_LastDropReason;
	drawstatefixture::Instance().Record( draw );
}

// stuff related to matrix stacks. These drive the model/view/projection matrices
// the material system positions geometry with; the composed MVP is committed to
// the native dynamic draw path (see the matrix helpers above).
void CShaderAPIVulkan::MatrixMode( MaterialMatrixMode_t matrixMode )
{
	EnsureMatricesInit();
	if ( matrixMode >= 0 && matrixMode < NUM_MATRIX_MODES )
		g_matrices.mode = matrixMode;
}

void CShaderAPIVulkan::PushMatrix()
{
	float *cur = CurrentMatrix();
	std::array<float, 16> saved;
	for ( int i = 0; i < 16; ++i )
		saved[i] = cur[i];
	g_matrices.stack[g_matrices.mode].push_back( saved );
}

void CShaderAPIVulkan::PopMatrix()
{
	std::vector<std::array<float, 16>> &st = g_matrices.stack[g_matrices.mode];
	if ( !st.empty() )
	{
		float *cur = CurrentMatrix();
		const std::array<float, 16> &top = st.back();
		for ( int i = 0; i < 16; ++i )
			cur[i] = top[i];
		st.pop_back();
		CommitModelViewProj();
	}
}

void CShaderAPIVulkan::LoadMatrix( float *m )
{
	if ( !m )
		return;
	float *cur = CurrentMatrix();
	for ( int i = 0; i < 16; ++i )
		cur[i] = m[i];
	CommitModelViewProj();
}

void CShaderAPIVulkan::MultMatrix( float *m )
{
	if ( !m )
		return;
	float *cur = CurrentMatrix();
	MatMul( cur, m, cur ); // top = top * m
	CommitModelViewProj();
}

void CShaderAPIVulkan::MultMatrixLocal( float *m )
{
	if ( !m )
		return;
	float *cur = CurrentMatrix();
	MatMul( m, cur, cur ); // top = m * top
	CommitModelViewProj();
}

void CShaderAPIVulkan::GetMatrix( MaterialMatrixMode_t matrixMode, float *dst )
{
	if ( !dst || matrixMode < 0 || matrixMode >= NUM_MATRIX_MODES )
		return;
	EnsureMatricesInit();
	for ( int i = 0; i < 16; ++i )
		dst[i] = g_matrices.mat[matrixMode][i];
}

void CShaderAPIVulkan::LoadIdentity( void )
{
	MatSetIdentity( CurrentMatrix() );
	CommitModelViewProj();
}

void CShaderAPIVulkan::LoadCameraToWorld( void )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Ortho(
    double left, double top, double right, double bottom, double zNear, double zFar )
{
	// Build a D3D-style orthographic matrix (stored transposed to match the
	// convention LoadMatrix receives) and post-multiply it onto the current matrix.
	const float rl = static_cast<float>( right - left );
	const float tb = static_cast<float>( top - bottom );
	const float fn = static_cast<float>( zFar - zNear );
	if ( rl == 0.0f || tb == 0.0f || fn == 0.0f )
		return;
	float o[16];
	MatSetIdentity( o );
	o[0] = 2.0f / rl;
	o[5] = 2.0f / tb;
	o[10] = 1.0f / fn;
	o[3] = static_cast<float>( -( right + left ) / ( right - left ) );
	o[7] = static_cast<float>( -( top + bottom ) / ( top - bottom ) );
	o[11] = static_cast<float>( -zNear / ( zFar - zNear ) );
	float *cur = CurrentMatrix();
	MatMul( cur, o, cur );
	CommitModelViewProj();
}

void CShaderAPIVulkan::PerspectiveX( double fovx, double aspect, double zNear, double zFar )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar,
    double bottom, double top, double left, double right )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::PickMatrix( int x, int y, int width, int height )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Rotate( float angle, float x, float y, float z )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Translate( float x, float y, float z )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::Scale( float x, float y, float z )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::ScaleXY( float x, float y )
{
	VK_UNIMPLEMENTED();
}

// Fog methods...
void CShaderAPIVulkan::FogMode( MaterialFogMode_t fogMode )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::FogStart( float fStart )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::FogEnd( float fEnd )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetFogZ( float fogZ )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::FogMaxDensity( float flMaxDensity )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::GetFogDistances( float *fStart, float *fEnd, float *fFogZ )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SceneFogMode( MaterialFogMode_t fogMode )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::GetSceneFogColor( unsigned char *rgb )
{
	VK_UNIMPLEMENTED();
}

MaterialFogMode_t CShaderAPIVulkan::GetSceneFogMode()
{
	return MATERIAL_FOG_NONE;
}

int CShaderAPIVulkan::GetPixelFogCombo()
{
	return 0;
}

void CShaderAPIVulkan::FogColor3f( float r, float g, float b )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::FogColor3fv( float const *rgb )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::FogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::FogColor3ubv( unsigned char const *rgb )
{
	VK_UNIMPLEMENTED();
}

// Shaders that precompute their per-draw state (LightmappedGeneric, the world's
// shader, above all) hand it over as a command buffer instead of individual
// calls: base-texture binds, constants and shader indices all arrive here. The
// format is CCommandBufferBuilder's (shaderapi/commandbuffer.h); this walks it the
// way the D3D9 backend does. An empty body silently discarded the texture of
// every world surface.
template <typename T> static void ReadCommandField( const uint8 *pCmd, size_t offset, T *out )
{
	memcpy( out, pCmd + offset, sizeof( T ) );
}

void CShaderAPIVulkan::ExecuteCommandBuffer( uint8 *pCmdBuf )
{
	for ( ;; )
	{
		int nCmd = 0;
		ReadCommandField( pCmdBuf, 0, &nCmd );
		switch ( nCmd )
		{
		case CBCMD_END:
			return;

		case CBCMD_JUMP:
		{
			uint8 *target = nullptr;
			ReadCommandField( pCmdBuf, sizeof( int ), &target );
			pCmdBuf = target;
			break;
		}

		case CBCMD_JSR:
		{
			uint8 *target = nullptr;
			ReadCommandField( pCmdBuf, sizeof( int ), &target );
			ExecuteCommandBuffer( target );
			pCmdBuf += sizeof( int ) + sizeof( uint8 * );
			break;
		}

		case CBCMD_SET_PIXEL_SHADER_FLOAT_CONST:
		case CBCMD_SET_VERTEX_SHADER_FLOAT_CONST:
		{
			int nStartConst = 0, nNumConsts = 0;
			ReadCommandField( pCmdBuf, sizeof( int ), &nStartConst );
			ReadCommandField( pCmdBuf, 2 * sizeof( int ), &nNumConsts );
			const float *pValues = reinterpret_cast<const float *>( pCmdBuf + 3 * sizeof( int ) );
			if ( nCmd == CBCMD_SET_PIXEL_SHADER_FLOAT_CONST )
				SetPixelShaderConstant( nStartConst, pValues, nNumConsts, false );
			else
				SetVertexShaderConstant( nStartConst, pValues, nNumConsts, false );
			pCmdBuf += nNumConsts * 4 * sizeof( float ) + 3 * sizeof( int );
			break;
		}

		case CBCMD_SETPIXELSHADERFOGPARAMS:
		case CBCMD_STORE_EYE_POS_IN_PSCONST:
		case CBCMD_COMMITPIXELSHADERLIGHTING:
		case CBCMD_SETPIXELSHADERSTATEAMBIENTLIGHTCUBE:
			// Fog, eye position and lighting constants: the native pipelines do
			// not consume them yet.
			NoteUnimplemented( "ExecuteCommandBuffer(fog/eye/lighting constant)" );
			pCmdBuf += 2 * sizeof( int );
			break;

		case CBCMD_SETAMBIENTCUBEDYNAMICSTATEVERTEXSHADER:
			NoteUnimplemented( "ExecuteCommandBuffer(vertex ambient cube)" );
			pCmdBuf += sizeof( int );
			break;

		case CBCMD_SET_DEPTH_FEATHERING_CONST:
			NoteUnimplemented( "ExecuteCommandBuffer(depth feathering)" );
			pCmdBuf += 2 * sizeof( int ) + sizeof( float );
			break;

		case CBCMD_BIND_STANDARD_TEXTURE:
		{
			int nSampler = 0, nTextureID = 0;
			ReadCommandField( pCmdBuf, sizeof( int ), &nSampler );
			ReadCommandField( pCmdBuf, 2 * sizeof( int ), &nTextureID );
			BindStandardTexture( static_cast<Sampler_t>( nSampler ),
			    static_cast<StandardTextureId_t>( nTextureID ) );
			pCmdBuf += 3 * sizeof( int );
			break;
		}

		case CBCMD_BIND_SHADERAPI_TEXTURE_HANDLE:
		{
			int nSampler = 0;
			ShaderAPITextureHandle_t hTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
			ReadCommandField( pCmdBuf, sizeof( int ), &nSampler );
			ReadCommandField( pCmdBuf, 2 * sizeof( int ), &hTexture );
			BindTexture( static_cast<Sampler_t>( nSampler ), hTexture );
			pCmdBuf += 2 * sizeof( int ) + sizeof( ShaderAPITextureHandle_t );
			break;
		}

		case CBCMD_SET_PSHINDEX:
		case CBCMD_SET_VSHINDEX:
			// Static/dynamic combo selection; the native pipelines have no combos.
			pCmdBuf += 2 * sizeof( int );
			break;

		default:
			// The command's size is unknown, so the rest of the buffer cannot be
			// walked. Stop rather than misread it, and say so.
			NoteUnimplemented( "ExecuteCommandBuffer(unknown command)" );
			return;
		}
	}
}

void CShaderAPIVulkan::SetViewports( int nCount, const ShaderViewport_t *pViewports )
{
	if ( nCount <= 0 || !pViewports )
		return;
	// Only one viewport exists without multiple render targets.
	g_Viewport = pViewports[0];
	g_VulkanContext.SetViewport( g_Viewport.m_nTopLeftX, g_Viewport.m_nTopLeftY,
	    g_Viewport.m_nWidth, g_Viewport.m_nHeight, g_Viewport.m_flMinZ, g_Viewport.m_flMaxZ );
}

int CShaderAPIVulkan::GetViewports( ShaderViewport_t *pViewports, int nMax ) const
{
	if ( pViewports && nMax >= 1 )
	{
		pViewports[0] = g_Viewport;
		if ( g_Viewport.m_nWidth <= 0 || g_Viewport.m_nHeight <= 0 )
		{
			int width = 0, height = 0;
			g_VulkanContext.GetSwapchainExtent( width, height );
			pViewports[0].Init( 0, 0, width, height );
		}
	}
	return 1;
}

void CShaderAPIVulkan::SetRenderTargetEx( int nRenderTargetID,
    ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle )
{
	// Only render target 0 exists; multiple simultaneous render targets need
	// pipelines with more than one color attachment.
	if ( nRenderTargetID != 0 )
	{
		VK_UNIMPLEMENTED();
		return;
	}
	// Each render-target texture carries its own depth buffer, and the back
	// buffer carries the swapchain's, so the depth handle selects nothing extra.
	int target = -1;
	if ( colorTextureHandle != SHADER_RENDERTARGET_BACKBUFFER )
	{
		target = static_cast<int>( colorTextureHandle ) - 1; // 1-based handles
		if ( !g_VulkanContext.IsRenderTargetTexture( target ) )
		{
			// A texture created without TEXTURE_CREATE_RENDERTARGET cannot be
			// drawn into. Say so instead of drawing its pass onto the back buffer.
			NoteUnimplemented( "SetRenderTargetEx(non-render-target texture)" );
			if ( target >= 0 && static_cast<size_t>( target ) < g_TextureRecords.size() )
				++g_TextureRecords[static_cast<size_t>( target )].rejectedAsTarget;
			target = -1;
		}
	}
	if ( target != g_VulkanContext.RenderTarget() )
		++g_TargetSwitches;
	g_VulkanContext.SetRenderTarget( target );
	g_Viewport.m_nWidth = 0;
	g_Viewport.m_nHeight = 0;
}

void CShaderAPIVulkan::CopyRenderTargetToTextureEx(
    ShaderAPITextureHandle_t texID, int nRenderTargetID, Rect_t *pSrcRect, Rect_t *pDstRect )
{
	if ( nRenderTargetID != 0 )
	{
		VK_UNIMPLEMENTED();
		return;
	}
	int src[4] = { 0, 0, 0, 0 };
	int dst[4] = { 0, 0, 0, 0 };
	if ( pSrcRect )
	{
		src[0] = pSrcRect->x;
		src[1] = pSrcRect->y;
		src[2] = pSrcRect->width;
		src[3] = pSrcRect->height;
	}
	if ( pDstRect )
	{
		dst[0] = pDstRect->x;
		dst[1] = pDstRect->y;
		dst[2] = pDstRect->width;
		dst[3] = pDstRect->height;
	}
	if ( g_VulkanContext.QueueCopyToTexture( static_cast<int>( texID ) - 1, src, dst ) )
		++g_TargetCopies;
	else
		++g_TargetCopiesDropped;
}

// Sets the vertex and pixel shaders
void CShaderAPIVulkan::SetVertexShaderIndex( int vshIndex )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetPixelShaderIndex( int pshIndex )
{
	VK_UNIMPLEMENTED();
}

// Sets the constant registers for vertex and pixel shaders
void CShaderAPIVulkan::SetVertexShaderConstant(
    int var, float const *pVec, int numConst, bool bForce )
{
	// Store into the faithful vertex-shader constant register file, then derive the
	// native UnlitGeneric material state (cModelViewProj, cModulationColor,
	// cBaseTextureTransform) from the real Source register numbers. This makes the
	// backend honor the D3D9 constant contract rather than a bespoke convention.
	if ( !pVec || numConst <= 0 )
		return;
	for ( int i = 0; i < numConst; ++i )
	{
		const int reg = var + i;
		if ( reg < 0 || reg >= kVsRegCount )
			continue;
		g_vsConstants.regs[reg][0] = pVec[i * 4 + 0];
		g_vsConstants.regs[reg][1] = pVec[i * 4 + 1];
		g_vsConstants.regs[reg][2] = pVec[i * 4 + 2];
		g_vsConstants.regs[reg][3] = pVec[i * 4 + 3];
		g_vsConstants.written[reg] = true;
	}
	CommitDynamicVsConstants();
}

void CShaderAPIVulkan::SetBooleanVertexShaderConstant(
    int var, BOOL const *pVec, int numConst, bool bForce )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetIntegerVertexShaderConstant(
    int var, int const *pVec, int numConst, bool bForce )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetPixelShaderConstant(
    int var, float const *pVec, int numConst, bool bForce )
{
	// Route the material's pixel-shader constant (register 'var', RGBA float4) to
	// the native Vulkan constant-color material shader. Bounded slice: only the
	// first float4 of register 0 is consumed; full constant-register banks are
	// later work (roadmap R32).
	if ( pVec && numConst > 0 && var == 0 )
	{
		g_VulkanContext.SetDynamicConstantColor( pVec[0], pVec[1], pVec[2], pVec[3] );
	}
	for ( int i = 0; pVec && i < numConst && var + i < 32; ++i )
	{
		if ( var + i >= 0 )
			memcpy( g_psConstants[var + i], pVec + i * 4, sizeof( g_psConstants[0] ) );
	}
}

void CShaderAPIVulkan::SetBooleanPixelShaderConstant(
    int var, BOOL const *pVec, int numBools, bool bForce )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetIntegerPixelShaderConstant(
    int var, int const *pVec, int numIntVecs, bool bForce )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::InvalidateDelayedShaderConstants( void )
{
	VK_UNIMPLEMENTED();
}

float CShaderAPIVulkan::GammaToLinear_HardwareSpecific( float fGamma ) const
{
	return 0.0f;
}

float CShaderAPIVulkan::LinearToGamma_HardwareSpecific( float fLinear ) const
{
	return 0.0f;
}

void CShaderAPIVulkan::SetLinearToGammaConversionTextures(
    ShaderAPITextureHandle_t hSRGBWriteEnabledTexture, ShaderAPITextureHandle_t hIdentityTexture )
{
	VK_UNIMPLEMENTED();
}

// Returns the nearest supported format
ImageFormat CShaderAPIVulkan::GetNearestSupportedFormat(
    ImageFormat fmt, bool bFilteringRequired /* = true */ ) const
{
	return fmt;
}

ImageFormat CShaderAPIVulkan::GetNearestRenderTargetFormat( ImageFormat fmt ) const
{
	return fmt;
}

// Sets the texture state
// The texture the material system is currently modifying (ModifyTexture), used
// by TexImage2D. 0 means none. Handles are 1-based over the native texture table
// so 0 stays "invalid", matching Source's convention.
static ShaderAPITextureHandle_t g_currentModifyTexture = 0;

void CShaderAPIVulkan::BindTexture( Sampler_t stage, ShaderAPITextureHandle_t textureHandle )
{
	// 1-based handle; 0 restores the built-in.
	const int native = static_cast<int>( textureHandle ) - 1;
	// Sampler 1 is sampled only when it holds a lightmap page (BindStandardTexture
	// says so); bump, env and detail stages are not sampled yet, so they must not
	// overwrite the base texture either.
	if ( stage == SHADER_SAMPLER1 && g_BindingLightmap )
	{
		g_boundLightmapHandle = native;
		g_VulkanContext.BindManagedLightmap( native );
		return;
	}
	// Samplers 1 and 2 as ordinary textures, read only by shaders that sample
	// them so (PortalRefract's noise and color ramp).
	if ( stage == SHADER_SAMPLER1 || stage == SHADER_SAMPLER2 )
		g_VulkanContext.BindManagedSampler( static_cast<int>( stage ), native );
	if ( stage != SHADER_SAMPLER0 )
		return;
	g_boundTextureHandle = native;
	g_VulkanContext.BindManagedTexture( native );
}

void CShaderAPIVulkan::ClearColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	g_ClearColor[0] = r;
	g_ClearColor[1] = g;
	g_ClearColor[2] = b;
	g_ClearColor[3] = 255;
	// Route the material system's clear color to the native Vulkan context; the
	// render pass applies it on the next BeginFrame (driven by Present()).
	g_VulkanContext.SetClearColor( r / 255.0f, g / 255.0f, b / 255.0f, 1.0f );
}

void CShaderAPIVulkan::ClearColor4ub(
    unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	g_ClearColor[0] = r;
	g_ClearColor[1] = g;
	g_ClearColor[2] = b;
	g_ClearColor[3] = a;
	g_VulkanContext.SetClearColor( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f );
}

// Indicates we're going to be modifying this texture
// TexImage2D, TexSubImage2D, TexWrap, TexMinFilter, and TexMagFilter
// all use the texture specified by this function.
void CShaderAPIVulkan::ModifyTexture( ShaderAPITextureHandle_t textureHandle )
{
	g_currentModifyTexture = textureHandle;
}

// Texture management methods
// One authoritative upload path for a texture surface, shared by TexImage2D and
// TexSubImage2D: the material system uses BOTH to deliver VTF pixels, so the
// conversion and upload rules must live in a single place. `srcStride` is the
// source row pitch in bytes; 0 means tightly packed. Block-compressed DXT1/3/5
// (Portal's dominant formats) upload their blocks directly to the matching BC
// image; uncompressed formats are converted to the 8-bit image they were created
// with. Returns false when the source format is not supported yet.
static bool UploadTextureSurface( int handle, int width, int height, ImageFormat srcFormat,
    const void *imageData, int srcStride, std::string *error, uint32_t level = 0 )
{
	const uint8_t *src = static_cast<const uint8_t *>( imageData );
	const size_t pixels = static_cast<size_t>( width ) * height;
	if ( srcFormat == IMAGE_FORMAT_DXT1 || srcFormat == IMAGE_FORMAT_DXT1_ONEBITALPHA ||
	     srcFormat == IMAGE_FORMAT_DXT3 || srcFormat == IMAGE_FORMAT_DXT5 )
	{
		// 4x4 block compression: DXT1 = 8 bytes/block, DXT3/DXT5 = 16 bytes/block.
		const size_t blocksX = ( static_cast<size_t>( width ) + 3 ) / 4;
		const size_t blocksY = ( static_cast<size_t>( height ) + 3 ) / 4;
		const size_t blockBytes =
		    ( srcFormat == IMAGE_FORMAT_DXT1 || srcFormat == IMAGE_FORMAT_DXT1_ONEBITALPHA ) ? 8
		                                                                                     : 16;
		return g_VulkanContext.UploadManagedTexture(
		    handle, src, blocksX * blocksY * blockBytes, error, level );
	}

	// 16-bit integer texels (integer-HDR lightmap pages) go only into the
	// R16G16B16A16 image created for them; there is no 8-bit conversion.
	if ( srcFormat == IMAGE_FORMAT_RGBA16161616 )
	{
		const bool image16 =
		    static_cast<size_t>( handle ) < g_TextureRecords.size() &&
		    g_TextureRecords[static_cast<size_t>( handle )].format == IMAGE_FORMAT_RGBA16161616;
		if ( !image16 || ( srcStride > 0 && srcStride != width * 8 ) )
		{
			NoteUnimplemented( "upload: RGBA16161616 into another format or padded rows" );
			return false;
		}
		return g_VulkanContext.UploadManagedTexture( handle, src, pixels * 8, error, level );
	}

	int srcBpp = 0;
	switch ( srcFormat )
	{
	case IMAGE_FORMAT_RGBA8888:
	case IMAGE_FORMAT_BGRA8888:
	case IMAGE_FORMAT_BGRX8888:
		srcBpp = 4;
		break;
	case IMAGE_FORMAT_RGB888:
	case IMAGE_FORMAT_BGR888:
		srcBpp = 3;
		break;
	case IMAGE_FORMAT_I8:
		srcBpp = 1;
		break;
	default:
		return false;
	}

	// A padded source pitch is repacked so the conversions below can index rows
	// tightly.
	std::vector<uint8_t> packed;
	const size_t tightRow = static_cast<size_t>( width ) * srcBpp;
	if ( srcStride > 0 && static_cast<size_t>( srcStride ) != tightRow )
	{
		packed.resize( tightRow * static_cast<size_t>( height ) );
		for ( int y = 0; y < height; ++y )
			memcpy( &packed[static_cast<size_t>( y ) * tightRow],
			    src + static_cast<size_t>( y ) * srcStride, tightRow );
		src = packed.data();
	}

	// The image stores B8G8R8A8 when it was created from a BGR-ordered format and
	// R8G8B8A8 otherwise (CreateTexture). The material system may deliver either
	// order into either image, so the source order never implies the image's.
	const ImageFormat imageFormat = ( static_cast<size_t>( handle ) < g_TextureRecords.size() )
	                                    ? g_TextureRecords[static_cast<size_t>( handle )].format
	                                    : IMAGE_FORMAT_UNKNOWN;
	const bool imageIsBgra =
	    imageFormat == IMAGE_FORMAT_BGRA8888 || imageFormat == IMAGE_FORMAT_BGRX8888;
	if ( ( srcFormat == IMAGE_FORMAT_RGBA8888 && !imageIsBgra ) ||
	     ( srcFormat == IMAGE_FORMAT_BGRA8888 && imageIsBgra ) )
		return g_VulkanContext.UploadManagedTexture( handle, src, pixels * 4, error, level );

	// Otherwise convert to RGBA, then to the image's order.
	std::vector<uint8_t> texels( pixels * 4 );
	for ( size_t i = 0; i < pixels; ++i )
	{
		uint8_t *d = &texels[i * 4];
		switch ( srcFormat )
		{
		case IMAGE_FORMAT_RGBA8888:
			d[0] = src[i * 4 + 0];
			d[1] = src[i * 4 + 1];
			d[2] = src[i * 4 + 2];
			d[3] = src[i * 4 + 3];
			break;
		case IMAGE_FORMAT_BGRA8888:
		case IMAGE_FORMAT_BGRX8888:
			d[0] = src[i * 4 + 2];
			d[1] = src[i * 4 + 1];
			d[2] = src[i * 4 + 0];
			// BGRX's X byte is meaningless; the texel is opaque.
			d[3] = ( srcFormat == IMAGE_FORMAT_BGRX8888 ) ? 255 : src[i * 4 + 3];
			break;
		case IMAGE_FORMAT_RGB888:
			d[0] = src[i * 3 + 0];
			d[1] = src[i * 3 + 1];
			d[2] = src[i * 3 + 2];
			d[3] = 255;
			break;
		case IMAGE_FORMAT_BGR888:
			d[0] = src[i * 3 + 2];
			d[1] = src[i * 3 + 1];
			d[2] = src[i * 3 + 0];
			d[3] = 255;
			break;
		default: // IMAGE_FORMAT_I8: intensity as grayscale
			d[0] = d[1] = d[2] = src[i];
			d[3] = 255;
			break;
		}
		if ( imageIsBgra )
			std::swap( d[0], d[2] );
	}
	return g_VulkanContext.UploadManagedTexture(
	    handle, texels.data(), texels.size(), error, level );
}

void CShaderAPIVulkan::TexImage2D( int level, int cubeFace, ImageFormat dstFormat, int zOffset,
    int width, int height, ImageFormat srcFormat, bool bSrcIsTiled, void *imageData )
{
	// Upload the material's texture data into a level of the native texture
	// selected by ModifyTexture.
	if ( level < 0 || g_currentModifyTexture <= 0 || !imageData || width <= 0 || height <= 0 )
		return;

	std::string error;
	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	if ( static_cast<uint32_t>( level ) >= g_VulkanContext.ManagedTextureMipLevels( handle ) )
		return;
	if ( !UploadTextureSurface( handle, width, height, srcFormat, imageData, 0, &error,
	         static_cast<uint32_t>( level ) ) )
	{
		if ( error.empty() )
			Warning(
			    "[NativeVulkan] TexImage2D: unsupported source format %d (skipped)\n", srcFormat );
		else
			Warning( "[NativeVulkan] TexImage2D upload failed: %s\n", error.c_str() );
		return;
	}
}

// The material system delivers most VTF pixels through this entry, not
// TexImage2D. Leaving it empty created the texture objects but never filled
// them, so every world surface sampled an empty image and rendered black.
void CShaderAPIVulkan::TexSubImage2D( int level, int cubeFace, int xOffset, int yOffset,
    int zOffset, int width, int height, ImageFormat srcFormat, int srcStride, bool bSrcIsTiled,
    void *imageData )
{
	if ( level < 0 || g_currentModifyTexture <= 0 || !imageData || width <= 0 || height <= 0 )
		return;
	// Only a full-level update can be applied; partial sub-rectangle updates are
	// not yet supported.
	if ( xOffset != 0 || yOffset != 0 )
		return;

	std::string error;
	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	if ( static_cast<uint32_t>( level ) >= g_VulkanContext.ManagedTextureMipLevels( handle ) )
		return;
	if ( !UploadTextureSurface( handle, width, height, srcFormat, imageData, srcStride, &error,
	         static_cast<uint32_t>( level ) ) )
	{
		if ( !error.empty() )
			Warning( "[NativeVulkan] TexSubImage2D upload failed: %s\n", error.c_str() );
		return;
	}
}

// The material system uploads every VTF-backed texture through THIS entry
// (CTexture::WriteDataToShaderAPITexture calls it once per frame of the
// texture), not through TexImage2D. While it was an empty stub the backend
// created ~900 texture objects and filled almost none of them, so every world
// surface sampled an empty image and the map rendered black.
void CShaderAPIVulkan::TexImageFromVTF( IVTFTexture *pVTF, int iVTFFrame )
{
	if ( !pVTF || g_currentModifyTexture <= 0 )
		return;

	// Face 0, every level the texture has, as D3D9's LoadTextureFromVTF loads
	// them. A texture created smaller than the VTF (mip skipping) starts at the
	// VTF level whose size matches its level 0.
	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	const int textureWidth = static_cast<size_t>( handle ) < g_TextureRecords.size()
	                             ? g_TextureRecords[static_cast<size_t>( handle )].width
	                             : 0;
	int firstMip = 0;
	for ( ; firstMip + 1 < pVTF->MipCount(); ++firstMip )
	{
		int w = 0, h = 0, d = 0;
		pVTF->ComputeMipLevelDimensions( firstMip, &w, &h, &d );
		if ( w <= textureWidth )
			break;
	}
	const uint32_t levels = g_VulkanContext.ManagedTextureMipLevels( handle );
	for ( uint32_t level = 0;
	    level < levels && firstMip + static_cast<int>( level ) < pVTF->MipCount(); ++level )
	{
		const int mip = firstMip + static_cast<int>( level );
		int mipWidth = 0, mipHeight = 0, mipDepth = 0;
		pVTF->ComputeMipLevelDimensions( mip, &mipWidth, &mipHeight, &mipDepth );
		const unsigned char *bits = pVTF->ImageData( iVTFFrame, 0, mip );
		if ( mipWidth <= 0 || mipHeight <= 0 || !bits )
			return;
		std::string error;
		if ( !UploadTextureSurface(
		         handle, mipWidth, mipHeight, pVTF->Format(), bits, 0, &error, level ) )
		{
			if ( !error.empty() )
				Warning( "[NativeVulkan] TexImageFromVTF upload failed: %s\n", error.c_str() );
			return;
		}
	}
}

// The texture TexLock locked (0-based), or -1.
static int g_lockedTexture = -1;

// Locks a rectangle of mip 0 of the texture selected by ModifyTexture for the
// material system's pixel writer, as CShaderAPIDx8::TexLock does. The writer
// addresses a CPU copy of the surface in the image's own layout (8-bit color or
// 16-bit integer); other formats and mip levels are refused, like D3D9 refuses
// levels it did not create.
bool CShaderAPIVulkan::TexLock( int level, int cubeFaceID, int xOffset, int yOffset, int width,
    int height, CPixelWriter &writer )
{
	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	if ( g_lockedTexture >= 0 || handle < 0 ||
	     static_cast<size_t>( handle ) >= g_TextureRecords.size() )
		return false;
	TextureRecord &record = g_TextureRecords[static_cast<size_t>( handle )];
	int texelBytes = 0;
	if ( record.format == IMAGE_FORMAT_RGBA8888 || record.format == IMAGE_FORMAT_BGRA8888 ||
	     record.format == IMAGE_FORMAT_BGRX8888 )
		texelBytes = 4;
	else if ( record.format == IMAGE_FORMAT_RGBA16161616 )
		texelBytes = 8;
	if ( level != 0 || cubeFaceID != 0 || texelBytes == 0 )
	{
		NoteUnimplemented( "TexLock(mip > 0, cube face or unsupported format)" );
		return false;
	}
	if ( xOffset < 0 || yOffset < 0 || width <= 0 || height <= 0 ||
	     xOffset + width > record.width || yOffset + height > record.height )
		return false;
	const size_t pitch = static_cast<size_t>( record.width ) * texelBytes;
	if ( record.lockSurface.empty() )
		record.lockSurface.assign( pitch * static_cast<size_t>( record.height ), 0 );
	writer.SetPixelMemory( record.format,
	    &record.lockSurface[static_cast<size_t>( yOffset ) * pitch +
	                        static_cast<size_t>( xOffset ) * texelBytes],
	    static_cast<int>( pitch ) );
	g_lockedTexture = handle;
	return true;
}

void CShaderAPIVulkan::TexUnlock()
{
	if ( g_lockedTexture < 0 )
		return;
	const TextureRecord &record = g_TextureRecords[static_cast<size_t>( g_lockedTexture )];
	std::string error;
	// The copy is already in the image's layout, so it uploads as that format.
	if ( !UploadTextureSurface( g_lockedTexture, record.width, record.height, record.format,
	         record.lockSurface.data(), 0, &error ) &&
	     !error.empty() )
		Warning( "[NativeVulkan] TexUnlock upload failed: %s\n", error.c_str() );
	g_lockedTexture = -1;
}

// These are bound to the texture, not the texture environment
// Sampler state of the texture selected by ModifyTexture. Vulkan samplers have
// one filter for minification and magnification here, so linear filtering is
// used when either direction asks for it.
static void UpdateModifiedTextureSampler( int clearBits, int setBits )
{
	if ( g_currentModifyTexture <= 0 )
		return;
	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	const int state =
	    ( g_VulkanContext.ManagedTextureSamplerState( handle ) & ~clearBits ) | setBits;
	g_VulkanContext.SetManagedTextureSamplerState( handle, state );
}

static bool IsLinearFilter( ShaderTexFilterMode_t mode )
{
	return mode != SHADER_TEXFILTERMODE_NEAREST &&
	       mode != SHADER_TEXFILTERMODE_NEAREST_MIPMAP_NEAREST &&
	       mode != SHADER_TEXFILTERMODE_NEAREST_MIPMAP_LINEAR;
}

// As CShaderAPIDx8::TexMinFilter: the minification filter and the mip filter,
// which is none for a texture of one level. Anisotropic filtering is sampled as
// trilinear.
void CShaderAPIVulkan::TexMinFilter( ShaderTexFilterMode_t texFilterMode )
{
	using render_vulkan::CVulkanContext;
	if ( IsLinearFilter( texFilterMode ) )
		UpdateModifiedTextureSampler( 0, CVulkanContext::kSamplerLinear );
	if ( g_currentModifyTexture <= 0 )
		return;
	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	int mip = 0;
	switch ( texFilterMode )
	{
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_NEAREST:
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_NEAREST:
		mip = CVulkanContext::kSamplerMipPoint;
		break;
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_LINEAR:
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR:
		mip = CVulkanContext::kSamplerMipLinear;
		break;
	case SHADER_TEXFILTERMODE_ANISOTROPIC:
		NoteUnimplemented( "anisotropic filtering (sampled trilinear)" );
		mip = CVulkanContext::kSamplerMipLinear;
		break;
	default:
		break;
	}
	if ( g_VulkanContext.ManagedTextureMipLevels( handle ) <= 1 )
		mip = 0;
	UpdateModifiedTextureSampler(
	    CVulkanContext::kSamplerMipPoint | CVulkanContext::kSamplerMipLinear, mip );
}

void CShaderAPIVulkan::TexMagFilter( ShaderTexFilterMode_t texFilterMode )
{
	if ( IsLinearFilter( texFilterMode ) )
		UpdateModifiedTextureSampler( 0, render_vulkan::CVulkanContext::kSamplerLinear );
}

void CShaderAPIVulkan::TexWrap( ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode )
{
	int bit = 0;
	if ( coord == SHADER_TEXCOORD_S )
		bit = render_vulkan::CVulkanContext::kSamplerClampU;
	else if ( coord == SHADER_TEXCOORD_T )
		bit = render_vulkan::CVulkanContext::kSamplerClampV;
	else
		return; // 2D images have no third coordinate
	// Border addressing is approximated by clamping to the edge.
	if ( wrapMode == SHADER_TEXWRAPMODE_REPEAT )
		UpdateModifiedTextureSampler( bit, 0 );
	else
		UpdateModifiedTextureSampler( 0, bit );
}

void CShaderAPIVulkan::TexSetPriority( int priority )
{
	VK_UNIMPLEMENTED();
}

ShaderAPITextureHandle_t CShaderAPIVulkan::CreateTexture( int width, int height, int depth,
    ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags, const char *pDebugName,
    const char *pTextureGroupName )
{
	// Map the Source image format to a Vulkan format. Block-compressed DXT1/3/5
	// (Portal's texture formats) map to BC1/BC2/BC3 and are sampled natively.
	VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
	switch ( dstImageFormat )
	{
	case IMAGE_FORMAT_BGRA8888:
	case IMAGE_FORMAT_BGRX8888:
		vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
		break;
	case IMAGE_FORMAT_DXT1:
	case IMAGE_FORMAT_DXT1_ONEBITALPHA:
		vkFormat = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		break;
	case IMAGE_FORMAT_DXT3:
		vkFormat = VK_FORMAT_BC2_UNORM_BLOCK;
		break;
	case IMAGE_FORMAT_DXT5:
		vkFormat = VK_FORMAT_BC3_UNORM_BLOCK;
		break;
	case IMAGE_FORMAT_RGBA16161616:
		// Integer-HDR lightmap pages; the same memory order as D3DFMT_A16B16G16R16.
		vkFormat = VK_FORMAT_R16G16B16A16_UNORM;
		break;
	default:
		vkFormat = VK_FORMAT_R8G8B8A8_UNORM; // RGBA8888 and RGBA-convertible sources
		break;
	}

	std::string error;
	// Render targets are drawn into, so they need an attachment-capable image,
	// a depth buffer and a framebuffer rather than a sampled-only texture.
	const int native = ( flags & TEXTURE_CREATE_RENDERTARGET )
	                       ? g_VulkanContext.CreateRenderTargetTexture( width, height, &error )
	                       : g_VulkanContext.CreateManagedTexture( width, height, vkFormat, &error,
	                             0, static_cast<uint32_t>( std::max( 1, numMipLevels ) ) );
	if ( native < 0 )
	{
		Warning( "[NativeVulkan] CreateTexture failed: %s\n", error.c_str() );
		return 0;
	}
	NoteTextureCreated( native, pDebugName, dstImageFormat, width, height );
	return static_cast<ShaderAPITextureHandle_t>( native + 1 ); // 1-based handle
}

// Create a multi-frame texture (equivalent to calling "CreateTexture" multiple times, but more efficient)
void CShaderAPIVulkan::CreateTextures( ShaderAPITextureHandle_t *pHandles, int count, int width,
    int height, int depth, ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags,
    const char *pDebugName, const char *pTextureGroupName )
{
	// The material system's texture manager creates most textures (all world/model
	// VTFs) through this batch call, not the single CreateTexture -- so it must
	// create real native textures, or nothing samples and the world renders with
	// the built-in fallback. Create one managed texture per handle.
	for ( int k = 0; k < count; ++k )
		pHandles[k] = CreateTexture( width, height, depth, dstImageFormat, numMipLevels, numCopies,
		    flags, pDebugName, pTextureGroupName );
}

ShaderAPITextureHandle_t CShaderAPIVulkan::CreateDepthTexture(
    ImageFormat renderFormat, int width, int height, const char *pDebugName, bool bTexture )
{
	return 0;
}

void CShaderAPIVulkan::DeleteTexture( ShaderAPITextureHandle_t textureHandle )
{
	VK_UNIMPLEMENTED();
}

bool CShaderAPIVulkan::IsTexture( ShaderAPITextureHandle_t textureHandle )
{
	return true;
}

bool CShaderAPIVulkan::IsTextureResident( ShaderAPITextureHandle_t textureHandle )
{
	return false;
}

// stuff that isn't to be used from within a shader
void CShaderAPIVulkan::ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil,
    int renderTargetWidth, int renderTargetHeight )
{
	// A clear is part of the frame's ordered stream: it applies to whichever
	// target is current when it is issued, bounded by the viewport as in D3D9.
	// Frame boundaries come from Present, not from clears -- the engine clears
	// render targets mid-frame and passes real dimensions for the back buffer.
	g_VulkanContext.QueueClear( bClearColor, bClearDepth, bClearStencil );
}

// As CShaderAPIDx8: a full-screen quad through BufferClearObeyStencil
// (DrawClearBufferQuad), so the clear obeys the current stencil test, drawn with
// user clip planes disabled because the quad is in altered world space.
void CShaderAPIVulkan::ClearBuffersObeyStencil( bool bClearColor, bool bClearDepth )
{
	ClearBuffersObeyStencilEx( bClearColor, bClearColor, bClearDepth );
}

void CShaderAPIVulkan::ClearBuffersObeyStencilEx(
    bool bClearColor, bool bClearAlpha, bool bClearDepth )
{
	if ( !bClearColor && !bClearAlpha && !bClearDepth )
		return;
	g_ClipPlanesSuppressed = true;
	ShaderUtil()->DrawClearBufferQuad( g_ClearColor[0], g_ClearColor[1], g_ClearColor[2],
	    g_ClearColor[3], bClearColor, bClearAlpha, bClearDepth );
	g_ClipPlanesSuppressed = false;
}

void CShaderAPIVulkan::PerformFullScreenStencilOperation( void )
{
	g_ClipPlanesSuppressed = true;
	ShaderUtil()->DrawClearBufferQuad( 0, 0, 0, 0, false, false, false );
	g_ClipPlanesSuppressed = false;
}

void CShaderAPIVulkan::SetStencilEnable( bool onoff )
{
	g_Stencil.enable = onoff;
}

void CShaderAPIVulkan::SetStencilFailOperation( StencilOperation_t op )
{
	g_Stencil.fail = op;
}

void CShaderAPIVulkan::SetStencilZFailOperation( StencilOperation_t op )
{
	g_Stencil.depthFail = op;
}

void CShaderAPIVulkan::SetStencilPassOperation( StencilOperation_t op )
{
	g_Stencil.pass = op;
}

void CShaderAPIVulkan::SetStencilCompareFunction( StencilComparisonFunction_t cmpfn )
{
	g_Stencil.compare = cmpfn;
}

void CShaderAPIVulkan::SetStencilReferenceValue( int ref )
{
	g_Stencil.reference = ref;
}

void CShaderAPIVulkan::SetStencilTestMask( uint32 msk )
{
	g_Stencil.testMask = msk;
}

void CShaderAPIVulkan::SetStencilWriteMask( uint32 msk )
{
	g_Stencil.writeMask = msk;
}

void CShaderAPIVulkan::ClearStencilBufferRectangle(
    int xmin, int ymin, int xmax, int ymax, int value )
{
	NoteUnimplemented( "ClearStencilBufferRectangle" );
}

void CShaderAPIVulkan::SetScissorRect( const int nLeft, const int nTop, const int nRight,
    const int nBottom, const bool bEnableScissor )
{
	g_VulkanContext.SetScissor( bEnableScissor, nLeft, nTop, nRight - nLeft, nBottom - nTop );
}

void CShaderAPIVulkan::ReadPixels(
    int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat )
{
	drawstatefixture::Instance().WriteFrame( "screenshot" );
	// Copy the most recently presented frame (captured by CVulkanContext) into
	// the caller's buffer, converting to the requested format. This is what the
	// engine's +screenshot path reads; without it every capture is blank.
	if ( !data || width <= 0 || height <= 0 || !g_VulkanContext.IsValid() )
		return;
	// Render the geometry queued for this frame and capture it on demand (only
	// here, not every frame -- a full-frame GPU copy per frame stalls the loop).
	{
		std::string err;
		bool skip = false;
		g_VulkanContext.RequestCapture();
		if ( g_VulkanContext.BeginFrame( &skip, &err ) && !skip )
			g_VulkanContext.EndFrame( &err );
	}
	int cw = 0, ch = 0;
	const std::vector<uint8_t> &px = g_VulkanContext.GetCapturedPixels( &cw, &ch );
	if ( px.empty() || cw <= 0 || ch <= 0 )
		return;

	const int dstBpp =
	    ( dstFormat == IMAGE_FORMAT_RGB888 || dstFormat == IMAGE_FORMAT_BGR888 ) ? 3 : 4;
	for ( int row = 0; row < height; ++row )
	{
		const int sy = y + row;
		for ( int col = 0; col < width; ++col )
		{
			const int sx = x + col;
			unsigned char *d = data + ( static_cast<size_t>( row ) * width + col ) * dstBpp;
			if ( sx < 0 || sx >= cw || sy < 0 || sy >= ch )
			{
				for ( int i = 0; i < dstBpp; ++i )
					d[i] = 0;
				continue;
			}
			const uint8_t *s = &px[( static_cast<size_t>( sy ) * cw + sx ) * 4]; // RGBA
			switch ( dstFormat )
			{
			case IMAGE_FORMAT_RGBA8888:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				break;
			case IMAGE_FORMAT_BGRA8888:
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				d[3] = s[3];
				break;
			case IMAGE_FORMAT_BGR888:
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				break;
			default: // RGB888 and other 3-byte requests
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				break;
			}
		}
	}
}

void CShaderAPIVulkan::ReadPixels(
    Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data, ImageFormat dstFormat, int nDstStride )
{
	// A screenshot is the one point in a real run where the frame is inspected, so
	// report what this backend was asked for and did not do while producing it.
	ReportUnimplementedEntries();
	ReportDroppedMaterials();
	drawstatefixture::Instance().WriteFrame( "screenshot" );

	// The engine's +screenshot path calls THIS overload (a source rect in the back
	// buffer -> a destination rect in `data` at `nDstStride`). Without it the
	// screenshot buffer stays uninitialized and reads back as noise. Copy the most
	// recently presented frame (captured on demand by CVulkanContext), converting
	// to the requested format.
	if ( !data || !pSrcRect || !pDstRect || !g_VulkanContext.IsValid() )
		return;
	{
		std::string err;
		bool skip = false;
		g_VulkanContext.RequestCapture();
		if ( g_VulkanContext.BeginFrame( &skip, &err ) && !skip )
			g_VulkanContext.EndFrame( &err );
	}
	int cw = 0, ch = 0;
	const std::vector<uint8_t> &px = g_VulkanContext.GetCapturedPixels( &cw, &ch );
	if ( px.empty() || cw <= 0 || ch <= 0 )
		return;

	const int dstBpp =
	    ( dstFormat == IMAGE_FORMAT_RGB888 || dstFormat == IMAGE_FORMAT_BGR888 ) ? 3 : 4;
	const int copyW = ( pDstRect->width < pSrcRect->width ) ? pDstRect->width : pSrcRect->width;
	const int copyH = ( pDstRect->height < pSrcRect->height ) ? pDstRect->height : pSrcRect->height;
	if ( nDstStride <= 0 )
		nDstStride = pDstRect->width * dstBpp;

	for ( int row = 0; row < copyH; ++row )
	{
		const int sy = pSrcRect->y + row;
		unsigned char *dRow =
		    data + static_cast<size_t>( pDstRect->y + row ) * nDstStride + pDstRect->x * dstBpp;
		for ( int col = 0; col < copyW; ++col )
		{
			const int sx = pSrcRect->x + col;
			unsigned char *d = dRow + static_cast<size_t>( col ) * dstBpp;
			if ( sx < 0 || sx >= cw || sy < 0 || sy >= ch )
			{
				for ( int i = 0; i < dstBpp; ++i )
					d[i] = 0;
				continue;
			}
			const uint8_t *s = &px[( static_cast<size_t>( sy ) * cw + sx ) * 4]; // RGBA
			switch ( dstFormat )
			{
			case IMAGE_FORMAT_RGBA8888:
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				break;
			case IMAGE_FORMAT_BGRA8888:
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				d[3] = s[3];
				break;
			case IMAGE_FORMAT_BGR888:
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				break;
			default: // RGB888 and other 3-byte requests
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				break;
			}
		}
	}
}

void CShaderAPIVulkan::FlushHardware()
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::ResetRenderState( bool bFullReset )
{
	VK_UNIMPLEMENTED();
}

// Set the number of bone weights
void CShaderAPIVulkan::SetNumBoneWeights( int numBones )
{
	g_NumBoneWeights = numBones;
}

// As CShaderAPIDx8::LoadBoneMatrix: store cModel[boneIndex], and bone 0 is also
// the MODEL matrix (loaded transposed, leaving MATERIAL_MODEL the matrix mode).
void CShaderAPIVulkan::LoadBoneMatrix( int boneIndex, const float *m )
{
	if ( !m || boneIndex < 0 || boneIndex >= kMaxBoneMatrices )
	{
		NoteUnimplemented( "LoadBoneMatrix: bone index outside cModel[53]" );
		return;
	}
	memcpy( g_BoneMatrices[boneIndex], m, sizeof( g_BoneMatrices[boneIndex] ) );
	if ( boneIndex == 0 )
	{
		MatrixMode( MATERIAL_MODEL );
		VMatrix transposed;
		transposed.Init( *reinterpret_cast<const matrix3x4_t *>( m ) );
		MatrixTranspose( transposed, transposed );
		LoadMatrix( transposed.Base() );
	}
}

void CShaderAPIVulkan::EnableHWMorphing( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

// Selection mode methods
int CShaderAPIVulkan::SelectionMode( bool selectionMode )
{
	return 0;
}

void CShaderAPIVulkan::SelectionBuffer( unsigned int *pBuffer, int size )
{
}

void CShaderAPIVulkan::ClearSelectionNames()
{
}

void CShaderAPIVulkan::LoadSelectionName( int name )
{
}

void CShaderAPIVulkan::PushSelectionName( int name )
{
}

void CShaderAPIVulkan::PopSelectionName()
{
}

// Use this to get the mesh builder that allows us to modify vertex data
CMeshBuilder *CShaderAPIVulkan::GetVertexModifyBuilder()
{
	return 0;
}

// Board-independent calls, here to unify how shaders set state
// Implementations should chain back to IShaderUtil->BindTexture(), etc.

// Use this to begin and end the frame
void CShaderAPIVulkan::BeginFrame()
{
	drawstatefixture::Instance().Configure( "vulkan-native" );
	drawstatefixture::Instance().BeginFrame();
}

void CShaderAPIVulkan::EndFrame()
{
	VK_UNIMPLEMENTED();
}

// returns the current time in seconds....
double CShaderAPIVulkan::CurrentTime() const
{
	return Sys_FloatTime();
}

// Get the current camera position in world space.
void CShaderAPIVulkan::GetWorldSpaceCameraPosition( float *pPos ) const
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::ForceHardwareSync( void )
{
	VK_UNIMPLEMENTED();
}

// The plane arrives in world space as Ax+By+Cz=D and is kept as D3D9 keeps it
// (Ax+By+Cz-D>=0); it is moved to clip space per draw (CommitUserClipPlanes).
void CShaderAPIVulkan::SetClipPlane( int index, const float *pPlane )
{
	if ( index < 0 || index >= render_vulkan::CVulkanContext::kMaxClipPlanes || !pPlane )
		return;
	g_ClipPlanesWorld[index][0] = pPlane[0];
	g_ClipPlanesWorld[index][1] = pPlane[1];
	g_ClipPlanesWorld[index][2] = pPlane[2];
	g_ClipPlanesWorld[index][3] = -pPlane[3];
}

void CShaderAPIVulkan::EnableClipPlane( int index, bool bEnable )
{
	if ( index < 0 || index >= render_vulkan::CVulkanContext::kMaxClipPlanes )
		return;
	if ( bEnable )
		g_ClipPlanesEnabled |= 1 << index;
	else
		g_ClipPlanesEnabled &= ~( 1 << index );
}

void CShaderAPIVulkan::EnableUserClipTransformOverride( bool bEnable )
{
	g_UserClipTransformOverride = bEnable;
}

// D3D9 keeps the transform as a D3D matrix (VMatrixToD3DXMatrix transposes).
void CShaderAPIVulkan::UserClipTransform( const VMatrix &worldToView )
{
	for ( int row = 0; row < 4; ++row )
		for ( int col = 0; col < 4; ++col )
			g_UserClipTransform[row * 4 + col] = worldToView.m[col][row];
}

void CShaderAPIVulkan::SetFastClipPlane( const float *pPlane )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::EnableFastClip( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

int CShaderAPIVulkan::GetCurrentNumBones( void ) const
{
	return g_NumBoneWeights;
}

bool CShaderAPIVulkan::IsHWMorphingEnabled( void ) const
{
	return false;
}

int CShaderAPIVulkan::GetCurrentLightCombo( void ) const
{
	return 0;
}

void CShaderAPIVulkan::GetDX9LightState( LightState_t *state ) const
{
	state->m_nNumLights = 0;
	state->m_bAmbientLight = false;
	state->m_bStaticLightVertex = false;
	state->m_bStaticLightTexel = false;
}

MaterialFogMode_t CShaderAPIVulkan::GetCurrentFogType( void ) const
{
	return MATERIAL_FOG_NONE;
}

void CShaderAPIVulkan::RecordString( const char *pStr )
{
}

bool CShaderAPIVulkan::ReadPixelsFromFrontBuffer() const
{
	return true;
}

bool CShaderAPIVulkan::PreferDynamicTextures() const
{
	return false;
}

bool CShaderAPIVulkan::PreferReducedFillrate() const
{
	return false;
}

bool CShaderAPIVulkan::HasProjectedBumpEnv() const
{
	return true;
}

int CShaderAPIVulkan::GetCurrentDynamicVBSize( void )
{
	return 0;
}

void CShaderAPIVulkan::DestroyVertexBuffers( bool bExitingLevel )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::EvictManagedResources()
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetTextureTransformDimension(
    TextureStage_t textureStage, int dimension, bool projected )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SetBumpEnvMatrix(
    TextureStage_t textureStage, float m00, float m01, float m10, float m11 )
{
	VK_UNIMPLEMENTED();
}

void CShaderAPIVulkan::SyncToken( const char *pToken )
{
	VK_UNIMPLEMENTED();
}
