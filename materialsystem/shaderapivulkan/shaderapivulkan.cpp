
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
#include "IHardwareConfigInternal.h"
#include "shadersystem.h"
#include "shaderapi/ishaderutil.h"
#include "shaderapi/ishaderapi.h"
#include "materialsystem/imesh.h"
#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "materialsystem/idebugtextureinfo.h"
#include "materialsystem/deformations.h"
#include "render/legacy_shader_provider.h"
#include "vulkan_device.h"

#include <array>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// The single native Vulkan presentation context, brought up on SetMode() and
// torn down at shutdown. Owns the real instance/device/queues/swapchain. While
// the material path is still the empty stub (roadmap R32), this proves the
// backend genuinely brings up native Vulkan in-process and presents a frame.
//-----------------------------------------------------------------------------
static render_vulkan::CVulkanContext g_VulkanContext;

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

	// Sets the primitive type
	void SetPrimitiveType( MaterialPrimitiveType_t type );

	// Draws the entire mesh
	void Draw( int firstIndex, int numIndices );

	void Draw( CPrimList *pPrims, int nPrims );

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

	virtual int IndexCount() const { return 0; }

	virtual void SetFlexMesh( IMesh *pMesh, int nVertexOffset ) {}

	virtual void DisableFlexMesh() {}

	virtual void MarkAsDrawn() {}

	virtual unsigned ComputeMemoryUsed() { return 0; }

	virtual VertexFormat_t GetVertexFormat() const { return VERTEX_POSITION; }

	virtual IMesh *GetMesh() { return this; }

private:
	enum
	{
		VERTEX_BUFFER_SIZE = 1024 * 1024,
		// Index slots (unsigned short each). Real Source geometry is indexed, so the
		// backend must keep the index buffer, not discard it.
		INDEX_BUFFER_SIZE = 1024 * 1024
	};

	unsigned char *m_pVertexMemory;
	// Index buffer the mesh builder writes into (unsigned short). Draw() uses it to
	// assemble triangles in the authored order; without it, drawing the shared
	// vertices sequentially scrambles the geometry into noise.
	unsigned short *m_pIndexMemory;
	bool m_bIsDynamic;
	// Vertices locked into m_pVertexMemory as an interleaved position(vec3) +
	// color(4 bytes) layout with stride kMeshVertexStride, so Draw() can forward
	// real geometry to the native Vulkan dynamic-mesh path.
	int m_numVerts = 0;
	// Number of indices the last lock/unlock recorded (0 = non-indexed).
	int m_numIndices = 0;
	// Scratch target for vertex components this bounded layout does not carry, so
	// a mesh builder writing them (with size 0) never corrupts position/color.
	unsigned char m_dummyComponent[64] = { 0 };

public:
	enum
	{
		kMeshVertexStride = 24
	}; // 12 bytes position + 4 bytes color + 8 bytes texcoord0
};

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
	void EnableSRGBWrite( bool bEnable ) {}

	void EnableSRGBRead( Sampler_t stage, bool bEnable ) {}

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
	bool m_bUsesVertexAndPixelShaders;
	// Blend factors recorded during snapshot state (IShaderShadow::BlendFunc), so
	// TakeSnapshot can classify the D3D9 compositing (opaque/translucent/additive).
	ShaderBlendFactor_t m_blendSrc = SHADER_BLEND_ONE;
	ShaderBlendFactor_t m_blendDst = SHADER_BLEND_ZERO;
	// $alphatest reference [0,1] recorded by AlphaFunc; applied only when
	// EnableAlphaTest set m_IsAlphaTested.
	float m_alphaRef = 0.0f;
	// Selected pixel shader recorded during snapshot state (IShaderShadow), so a
	// snapshot can carry which material shader to bind at draw time.
	char m_pixelShaderName[64] = { 0 };
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
	virtual int StencilBufferBits() const { return 0; }
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
	}
	virtual void EnableNonInteractiveMode(
	    MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo )
	{
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

	virtual void EnableUserClipTransformOverride( bool bEnable ) {}
	virtual void UserClipTransform( const VMatrix &worldToView ) {}

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
	void CopyRenderTargetToTexture( ShaderAPITextureHandle_t texID ) {}

	void CopyRenderTargetToTextureEx(
	    ShaderAPITextureHandle_t texID, int nRenderTargetID, Rect_t *pSrcRect, Rect_t *pDstRect )
	{
	}

	void CopyTextureToRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t textureHandle,
	    Rect_t *pSrcRect, Rect_t *pDstRect )
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
	IMesh *GetDynamicMesh( IMaterial *pMaterial, int nHWSkinBoneCount, bool buffered,
	    IMesh *pVertexOverride, IMesh *pIndexOverride );
	IMesh *GetDynamicMeshEx( IMaterial *pMaterial, VertexFormat_t fmt, int nHWSkinBoneCount,
	    bool buffered, IMesh *pVertexOverride, IMesh *pIndexOverride );

	IMesh *GetFlexMesh();

	// Renders a single pass of a material
	void RenderPass( int nPass, int nPassCount );

	// stuff related to matrix stacks
	void MatrixMode( MaterialMatrixMode_t matrixMode );
	void PushMatrix();
	void PopMatrix();
	void LoadMatrix( float *m );
	void LoadBoneMatrix( int boneIndex, const float *m ) {}
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
	}

	void SetRenderTargetEx( int nRenderTargetID, ShaderAPITextureHandle_t colorTextureHandle,
	    ShaderAPITextureHandle_t depthTextureHandle )
	{
	}

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

	bool SupportsHDR() const { return false; }
	HDRType_t GetHDRType() const { return HDR_TYPE_NONE; }
	HDRType_t GetHardwareHDRType() const { return HDR_TYPE_NONE; }
	virtual bool NeedsATICentroidHack() const { return false; }
	virtual bool SupportsColorOnSecondStream() const { return false; }
	virtual bool SupportsStaticPlusDynamicLighting() const { return false; }
	virtual bool SupportsStreamOffset() const { return false; }
	void SetDefaultDynamicState() {}
	virtual void CommitPixelShaderLighting( int pshReg ) {}

	ShaderAPIOcclusionQuery_t CreateOcclusionQueryObject( void )
	{
		return INVALID_SHADERAPI_OCCLUSION_QUERY_HANDLE;
	}

	void DestroyOcclusionQueryObject( ShaderAPIOcclusionQuery_t handle ) {}

	void BeginOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t handle ) {}

	void EndOcclusionQueryDrawing( ShaderAPIOcclusionQuery_t handle ) {}

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

	// Binds a standard texture
	virtual void BindStandardTexture( Sampler_t stage, StandardTextureId_t id ) {}

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

	virtual void SetFlashlightStateEx( const FlashlightState_t &state,
	    const VMatrix &worldToTexture, ITexture *pFlashlightDepthTexture )
	{
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
	}

	// Sets morph target factors
	virtual void SetFlexWeights( int nFirstWeight, int nCount, const MorphWeight_t *pWeights ) {}

	// NOTE: Stuff after this is added after shipping HL2.
	ITexture *GetRenderTargetEx( int nRenderTargetID ) { return NULL; }

	void SetToneMappingScaleLinear( const Vector &scale ) {}

	const Vector &GetToneMappingScaleLinear( void ) const
	{
		static Vector dummy;
		return dummy;
	}

	virtual float GetLightMapScaleFactor( void ) const { return 1.0; }

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
	void SetStencilEnable( bool onoff ) {}

	void SetStencilFailOperation( StencilOperation_t op ) {}

	void SetStencilZFailOperation( StencilOperation_t op ) {}

	void SetStencilPassOperation( StencilOperation_t op ) {}

	void SetStencilCompareFunction( StencilComparisonFunction_t cmpfn ) {}

	void SetStencilReferenceValue( int ref ) {}

	void SetStencilTestMask( uint32 msk ) {}

	void SetStencilWriteMask( uint32 msk ) {}

	void ClearStencilBufferRectangle( int xmin, int ymin, int xmax, int ymax, int value ) {}

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
	}

	virtual bool SupportsShadowDepthTextures() { return false; }

	virtual bool SupportsFetch4() { return false; }

	virtual int NeedsShaderSRGBConversion( void ) const { return 0; }
	virtual bool UsesSRGBCorrectBlending() const { return false; }

	virtual bool HasFastVertexTextures() const { return false; }

	virtual void SetShadowDepthBiasFactors(
	    float fShadowSlopeScaleDepthBias, float fShadowDepthBias )
	{
	}

	virtual void SetDisallowAccess( bool ) {}
	virtual void EnableShaderShaderMutex( bool ) {}
	virtual void ShaderLock() {}
	virtual void ShaderUnlock() {}

	// ------------ New Vertex/Index Buffer interface ----------------------------
	void BindVertexBuffer( int streamID, IVertexBuffer *pVertexBuffer, int nOffsetInBytes,
	    int nFirstVertex, int nVertexCount, VertexFormat_t fmt, int nRepetitions1 )
	{
	}
	void BindIndexBuffer( IIndexBuffer *pIndexBuffer, int nOffsetInBytes ) {}
	void Draw( MaterialPrimitiveType_t primitiveType, int firstIndex, int numIndices ) {}
	// ------------ End ----------------------------

	virtual int GetVertexBufferCompression( void ) const { return 0; };

	virtual bool ShouldWriteDepthToDestAlpha( void ) const { return false; };
	virtual bool SupportsHDRMode( HDRType_t nHDRMode ) const { return false; };
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

	virtual void ExecuteCommandBuffer( uint8 *pData ) {}
	virtual bool GetHDREnabled( void ) const { return true; }
	virtual void SetHDREnabled( bool bEnable ) {}

	virtual void CopyRenderTargetToScratchTexture( ShaderAPITextureHandle_t srcRt,
	    ShaderAPITextureHandle_t dstTex, Rect_t *pSrcRect = NULL, Rect_t *pDstRect = NULL )
	{
	}

	// Allows locking and unlocking of very specific surface types.
	virtual void LockRect( void **pOutBits, int *pOutPitch, ShaderAPITextureHandle_t texHandle,
	    int mipmap, int x, int y, int w, int h, bool bWrite, bool bRead )
	{
	}

	virtual void UnlockRect( ShaderAPITextureHandle_t texHandle, int mipmap ) {}

	virtual void TexLodClamp( int finest ) {}

	virtual void TexLodBias( float bias ) {}

	virtual void CopyTextureToTexture(
	    ShaderAPITextureHandle_t srcTex, ShaderAPITextureHandle_t dstTex )
	{
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

static bool CreateNullShaderBackend( render::LegacyShaderServices *services )
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

DLL_EXPORT const render::LegacyShaderProvider *NullShaderBackend_Describe()
{
	static const render::LegacyShaderProvider provider = {
	    "null", "shaderapiempty", CreateNullShaderBackend };
	return &provider;
}

extern "C" DLL_EXPORT bool ShaderBackend_Create( render::LegacyShaderServices *services )
{
	return CreateNullShaderBackend( services );
}

extern "C" DLL_EXPORT const render::LegacyShaderProvider *ShaderBackend_Describe()
{
	static const render::LegacyShaderProvider provider = {
	    "native-vulkan", "shaderapivulkan", ShaderBackend_Create };
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
bool CShaderDeviceMgrVulkan::Connect( CreateInterfaceFn factory )
{
	// So others can access it
	g_pShaderUtil = (IShaderUtil *)factory( SHADER_UTIL_INTERFACE_VERSION, NULL );

	return true;
}

void CShaderDeviceMgrVulkan::Disconnect()
{
	g_pShaderUtil = NULL;
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
}

// Activates a view
void CShaderDeviceVulkan::SetView( void *hwnd )
{
}

void CShaderDeviceVulkan::ReleaseResources()
{
}

void CShaderDeviceVulkan::ReacquireResources()
{
}

// Creates/destroys Mesh
IMesh *CShaderDeviceVulkan::CreateStaticMesh(
    VertexFormat_t fmt, const char *pTextureBudgetGroup, IMaterial *pMaterial )
{
	return &m_Mesh;
}

void CShaderDeviceVulkan::DestroyStaticMesh( IMesh *mesh )
{
}

// Creates/destroys static vertex + index buffers
IVertexBuffer *CShaderDeviceVulkan::CreateVertexBuffer(
    ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount, const char *pTextureBudgetGroup )
{
	return ( type == SHADER_BUFFER_TYPE_STATIC || type == SHADER_BUFFER_TYPE_STATIC_TEMP )
	           ? &m_Mesh
	           : &m_DynamicMesh;
}

void CShaderDeviceVulkan::DestroyVertexBuffer( IVertexBuffer *pVertexBuffer )
{
}

IIndexBuffer *CShaderDeviceVulkan::CreateIndexBuffer( ShaderBufferType_t bufferType,
    MaterialIndexFormat_t fmt, int nIndexCount, const char *pTextureBudgetGroup )
{
	switch ( bufferType )
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

void CShaderDeviceVulkan::DestroyIndexBuffer( IIndexBuffer *pIndexBuffer )
{
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
	m_pVertexMemory = new unsigned char[VERTEX_BUFFER_SIZE];
	m_pIndexMemory = new unsigned short[INDEX_BUFFER_SIZE];
}

CEmptyMesh::~CEmptyMesh()
{
	delete[] m_pVertexMemory;
	delete[] m_pIndexMemory;
}

bool CEmptyMesh::Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t &desc )
{
	// Hand out the real index buffer so the mesh builder's authored indices are
	// kept (m_nIndexSize = 1 advances one slot per index). Draw() replays the
	// geometry in this order. Clamp to the buffer; overflow would corrupt memory.
	if ( nMaxIndexCount > INDEX_BUFFER_SIZE )
		nMaxIndexCount = INDEX_BUFFER_SIZE;
	desc.m_pIndices = m_pIndexMemory;
	desc.m_nIndexSize = 1;
	desc.m_nFirstIndex = 0;
	desc.m_nOffset = 0;
	m_numIndices = nMaxIndexCount;
	return true;
}

void CEmptyMesh::Unlock( int nWrittenIndexCount, IndexDesc_t &desc )
{
}

void CEmptyMesh::ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t &desc )
{
	Lock( nIndexCount, false, desc );
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
	m_numVerts = nVertexCount;

	desc.m_pPosition = (float *)( m_pVertexMemory );
	desc.m_pColor = m_pVertexMemory + 12;
	desc.m_VertexSize_Position = kMeshVertexStride;
	desc.m_VertexSize_Color = kMeshVertexStride;

	// Texcoord0 lives at offset 16 (after position+color); other texcoord sets go
	// to the dummy scratch. This lets a textured material sample the mesh UVs.
	desc.m_pNormal = (float *)m_dummyComponent;
	int i;
	for ( i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
	{
		if ( i == 0 )
		{
			desc.m_pTexCoord[i] = (float *)( m_pVertexMemory + 16 );
			desc.m_VertexSize_TexCoord[i] = kMeshVertexStride;
		}
		else
		{
			desc.m_pTexCoord[i] = (float *)m_dummyComponent;
			desc.m_VertexSize_TexCoord[i] = 0;
		}
	}
	desc.m_pBoneWeight = (float *)m_dummyComponent;
	desc.m_pBoneMatrixIndex = (unsigned char *)m_dummyComponent;
	desc.m_pTangentS = (float *)m_dummyComponent;
	desc.m_pTangentT = (float *)m_dummyComponent;
	desc.m_pUserData = (float *)m_dummyComponent;
	desc.m_NumBoneWeights = 2;

	desc.m_VertexSize_BoneWeight = 0;
	desc.m_VertexSize_BoneMatrixIndex = 0;
	desc.m_VertexSize_Normal = 0;
	desc.m_VertexSize_TangentS = 0;
	desc.m_VertexSize_TangentT = 0;
	desc.m_VertexSize_UserData = 0;
	desc.m_ActualVertexSize = kMeshVertexStride;

	desc.m_nFirstVertex = 0;
	desc.m_nOffset = 0;
	return true;
}

void CEmptyMesh::Unlock( int nVertexCount, VertexDesc_t &desc )
{
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
	Lock( numVerts, false, *static_cast<VertexDesc_t *>( &desc ) );
	Lock( numIndices, false, *static_cast<IndexDesc_t *>( &desc ) );
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
	return 0;
}

// Sets the primitive type
void CEmptyMesh::SetPrimitiveType( MaterialPrimitiveType_t type )
{
}

// Draws the entire mesh
void CEmptyMesh::Draw( int firstIndex, int numIndices )
{
	// Forward the locked position+color vertices to the native Vulkan dynamic
	// mesh path as a triangle list. Bounded slice: fixed built-in shading, not
	// material shaders (roadmap R32); indices are treated as a straight list.
	if ( !g_VulkanContext.IsValid() || !g_VulkanContext.DynamicMeshReady() || m_numVerts <= 0 )
		return;

	// Determine the index range to draw. mesh->Draw() passes (-1, 0) meaning "the
	// whole mesh"; an explicit range draws a sub-batch. When the mesh carries no
	// index buffer, fall back to a sequential triangle list over the vertices.
	int first = ( firstIndex > 0 ) ? firstIndex : 0;
	int count = ( numIndices > 0 ) ? numIndices : m_numIndices;

	auto appendVertex = [&]( std::vector<float> &out, int v )
	{
		const unsigned char *base = m_pVertexMemory + static_cast<size_t>( v ) * kMeshVertexStride;
		float pos[3], uv[2];
		memcpy( pos, base, sizeof( pos ) );
		memcpy( uv, base + 16, sizeof( uv ) );
		const unsigned char *col = base + 12;
		out.push_back( pos[0] );
		out.push_back( pos[1] );
		out.push_back( pos[2] );
		out.push_back( col[0] / 255.0f );
		out.push_back( col[1] / 255.0f );
		out.push_back( col[2] / 255.0f );
		out.push_back( uv[0] );
		out.push_back( uv[1] );
	};

	std::vector<float> interleaved;
	if ( count > 0 )
	{
		// Indexed geometry: expand the authored index list into a triangle list, so
		// the shared vertices are assembled into the correct triangles.
		interleaved.reserve( static_cast<size_t>( count ) * 8 );
		for ( int i = 0; i < count; ++i )
		{
			const int idx = static_cast<int>( m_pIndexMemory[first + i] );
			if ( idx >= 0 && idx < m_numVerts )
				appendVertex( interleaved, idx );
		}
		g_VulkanContext.QueueDynamicTriangles(
		    interleaved.data(), static_cast<uint32_t>( interleaved.size() / 8 ) );
	}
	else
	{
		// Non-indexed fallback: draw the vertices in order as a triangle list.
		interleaved.reserve( static_cast<size_t>( m_numVerts ) * 8 );
		for ( int i = 0; i < m_numVerts; ++i )
			appendVertex( interleaved, i );
		g_VulkanContext.QueueDynamicTriangles(
		    interleaved.data(), static_cast<uint32_t>( m_numVerts ) );
	}
}

void CEmptyMesh::Draw( CPrimList *pPrims, int nPrims )
{
}

// Copy verts and/or indices to a mesh builder. This only works for temp meshes!
void CEmptyMesh::CopyToMeshBuilder( int iStartVert, // Which vertices to copy.
    int nVerts,
    int iStartIndex, // Which indices to copy.
    int nIndices,
    int indexOffset, // This is added to each index.
    CMeshBuilder &builder )
{
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
	m_bUsesVertexAndPixelShaders = false;
	m_blendSrc = SHADER_BLEND_ONE;
	m_blendDst = SHADER_BLEND_ZERO;
	m_alphaRef = 0.0f;
}

// Methods related to depth buffering
void CShaderShadowVulkan::DepthFunc( ShaderDepthFunc_t depthFunc )
{
}

void CShaderShadowVulkan::EnableDepthWrites( bool bEnable )
{
	m_bIsDepthWriteEnabled = bEnable;
}

void CShaderShadowVulkan::EnableDepthTest( bool bEnable )
{
}

void CShaderShadowVulkan::EnablePolyOffset( PolygonOffsetMode_t nOffsetMode )
{
}

// Suppresses/activates color writing
void CShaderShadowVulkan::EnableColorWrites( bool bEnable )
{
}

// Suppresses/activates alpha writing
void CShaderShadowVulkan::EnableAlphaWrites( bool bEnable )
{
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
}

void CShaderShadowVulkan::EnableConstantAlpha( bool bEnable )
{
}

void CShaderShadowVulkan::EnableVertexAlpha( bool bEnable )
{
}

void CShaderShadowVulkan::EnableTextureAlpha( TextureStage_t stage, bool bEnable )
{
}

// Alpha testing
void CShaderShadowVulkan::EnableAlphaTest( bool bEnable )
{
	m_IsAlphaTested = bEnable;
}

void CShaderShadowVulkan::AlphaFunc( ShaderAlphaFunc_t alphaFunc, float alphaRef /* [0-1] */ )
{
	m_alphaRef = alphaRef;
}

// Wireframe/filled polygons
void CShaderShadowVulkan::PolyMode( ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode )
{
}

// Back face culling
void CShaderShadowVulkan::EnableCulling( bool bEnable )
{
}

// Alpha to coverage
void CShaderShadowVulkan::EnableAlphaToCoverage( bool bEnable )
{
}

// constant color + transparency
void CShaderShadowVulkan::EnableConstantColor( bool bEnable )
{
}

// Indicates the vertex format for use with a vertex shader
// The flags to pass in here come from the VertexFormatFlags_t enum
// If pTexCoordDimensions is *not* specified, we assume all coordinates
// are 2-dimensional
void CShaderShadowVulkan::VertexShaderVertexFormat(
    unsigned int nFlags, int nTexCoordCount, int *pTexCoordDimensions, int nUserDataSize )
{
}

// Indicates we're going to light the model
void CShaderShadowVulkan::EnableLighting( bool bEnable )
{
}

void CShaderShadowVulkan::EnableSpecular( bool bEnable )
{
}

// Activate/deactivate skinning
void CShaderShadowVulkan::EnableVertexBlend( bool bEnable )
{
}

// per texture unit stuff
void CShaderShadowVulkan::OverbrightValue( TextureStage_t stage, float value )
{
}

void CShaderShadowVulkan::EnableTexture( Sampler_t stage, bool bEnable )
{
}

void CShaderShadowVulkan::EnableCustomPixelPipe( bool bEnable )
{
}

void CShaderShadowVulkan::CustomTextureStages( int stageCount )
{
}

void CShaderShadowVulkan::CustomTextureOperation( TextureStage_t stage, ShaderTexChannel_t channel,
    ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2 )
{
}

void CShaderShadowVulkan::EnableTexGen( TextureStage_t stage, bool bEnable )
{
}

void CShaderShadowVulkan::TexGen( TextureStage_t stage, ShaderTexGenParam_t param )
{
}

// Sets the vertex and pixel shaders
void CShaderShadowVulkan::SetVertexShader( const char *pShaderName, int vshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
}

void CShaderShadowVulkan::EnableBlendingSeparateAlpha( bool bEnable )
{
}
void CShaderShadowVulkan::SetPixelShader( const char *pShaderName, int pshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
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
}
// indicates what per-vertex data we're providing
void CShaderShadowVulkan::DrawFlags( unsigned int drawFlags )
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

CShaderAPIVulkan::CShaderAPIVulkan() : m_Mesh( false )
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
	return false;
}

// Used to clear the transition table when we know it's become invalid.
void CShaderAPIVulkan::ClearSnapshots()
{
}

// Members of IMaterialSystemHardwareConfig
bool CShaderAPIVulkan::HasDestAlphaBuffer() const
{
	return false;
}

bool CShaderAPIVulkan::HasStencilBuffer() const
{
	return false;
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
	return 0;
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

bool CShaderAPIVulkan::SupportsCompressedTextures() const
{
	return false;
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

int CShaderAPIVulkan::MaxUserClipPlanes() const
{
	return 0;
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
}

// Snapshot -> selected pixel shader name, so BeginPass can bind the matching
// native pipeline. The snapshot id keeps the existing flag bits (0..3) and packs
// the table index in the high bits, preserving IsTranslucent()/etc.
static std::vector<std::string> g_snapshotShaders;
// Parallel to g_snapshotShaders: the blend mode (CVulkanContext::kDynBlend*) each
// snapshot composites with, classified from the recorded IShaderShadow blend
// state so BeginPass can select the matching pipeline variant.
static std::vector<int> g_snapshotBlend;
// Parallel to g_snapshotShaders: the $alphatest reference each snapshot applies
// (< 0 when alpha test is disabled).
static std::vector<float> g_snapshotAlphaRef;

// Faithful vertex-shader constant register file. The material system commits its
// standard and shader-specific constants to fixed registers (see
// stdshaders/common_vs_fxc.h); honoring the real register numbers -- rather than
// a bespoke convention -- is what makes the native path a substitutable D3D9
// backend. The registers the UnlitGeneric family consumes:
//   c4-c7   cModelViewProj        (committed from the matrix stack)
//   c37     cModulationColor      ($color * $alpha)
//   c38-c39 cBaseTextureTransform (SHADER_SPECIFIC_CONST_0/1)
// c0-c3 is retained as a legacy alias for the model->projection matrix so the
// direct-interface harnesses that predate the matrix stack keep working.
namespace
{
enum
{
	kVsRegModelViewProj = 4,       // cModelViewProj, 4 registers
	kVsRegModelViewProjLegacy = 0, // legacy c0-c3 alias
	kVsRegModulationColor = 37,    // cModulationColor
	kVsRegBaseTexTransform = 38,   // cBaseTextureTransform[0..1]
	kVsRegCount = 64
};
struct VsConstantFile
{
	float regs[kVsRegCount][4] = {};
	bool written[kVsRegCount] = {};
};
VsConstantFile g_vsConstants;

// Push the constants the native UnlitGeneric pipeline consumes to the context,
// preferring the faithful cModelViewProj (c4) when the material system has set
// it, falling back to the legacy c0 alias otherwise.
void CommitDynamicVsConstants()
{
	if ( g_vsConstants.written[kVsRegModelViewProj] )
		g_VulkanContext.SetDynamicTransform( &g_vsConstants.regs[kVsRegModelViewProj][0] );
	else if ( g_vsConstants.written[kVsRegModelViewProjLegacy] )
		g_VulkanContext.SetDynamicTransform( &g_vsConstants.regs[kVsRegModelViewProjLegacy][0] );

	if ( g_vsConstants.written[kVsRegModulationColor] )
		g_VulkanContext.SetDynamicModulation( &g_vsConstants.regs[kVsRegModulationColor][0] );

	if ( g_vsConstants.written[kVsRegBaseTexTransform] )
		g_VulkanContext.SetDynamicBaseTexTransform( &g_vsConstants.regs[kVsRegBaseTexTransform][0],
		    &g_vsConstants.regs[kVsRegBaseTexTransform + 1][0] );
}

// Classify the recorded blend state into the native compositing mode, matching
// how the D3D9 material shaders configure the fixed-function blender:
//   blending off              -> opaque (src replaces dst)
//   src=ONE,       dst=ONE    -> additive          ($additive)
//   src=SRC_ALPHA, dst=ONE    -> additive (alpha-premultiplied glow)
//   otherwise (e.g. SRC_ALPHA/ONE_MINUS_SRC_ALPHA) -> alpha blend ($translucent)
int ClassifyBlendMode( bool blendEnabled, ShaderBlendFactor_t src, ShaderBlendFactor_t dst )
{
	if ( !blendEnabled )
		return render_vulkan::CVulkanContext::kDynBlendOpaque;
	if ( dst == SHADER_BLEND_ONE && ( src == SHADER_BLEND_ONE || src == SHADER_BLEND_SRC_ALPHA ) )
		return render_vulkan::CVulkanContext::kDynBlendAdditive;
	return render_vulkan::CVulkanContext::kDynBlendAlpha;
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
} // namespace

// Returns the snapshot id for the shader state
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

	const size_t index = g_snapshotShaders.size();
	g_snapshotShaders.push_back( g_ShaderShadow.m_pixelShaderName );
	g_snapshotBlend.push_back( ClassifyBlendMode(
	    g_ShaderShadow.m_IsTranslucent, g_ShaderShadow.m_blendSrc, g_ShaderShadow.m_blendDst ) );
	g_snapshotAlphaRef.push_back(
	    g_ShaderShadow.m_IsAlphaTested ? g_ShaderShadow.m_alphaRef : -1.0f );
	id |= static_cast<StateSnapshot_t>( index << 4 ); // flags occupy bits 0..3
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
	return 0;
}

// Gets the vertex format for a set of snapshot ids
VertexFormat_t CShaderAPIVulkan::ComputeVertexUsage( int numSnapshots, StateSnapshot_t *pIds ) const
{
	return 0;
}

// Uses a state snapshot
void CShaderAPIVulkan::UseSnapshot( StateSnapshot_t snapshot )
{
}

// Sets the color to modulate by
void CShaderAPIVulkan::Color3f( float r, float g, float b )
{
}

void CShaderAPIVulkan::Color3fv( float const *pColor )
{
}

void CShaderAPIVulkan::Color4f( float r, float g, float b, float a )
{
}

void CShaderAPIVulkan::Color4fv( float const *pColor )
{
}

// Faster versions of color
void CShaderAPIVulkan::Color3ub( unsigned char r, unsigned char g, unsigned char b )
{
}

void CShaderAPIVulkan::Color3ubv( unsigned char const *rgb )
{
}

void CShaderAPIVulkan::Color4ub(
    unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
}

void CShaderAPIVulkan::Color4ubv( unsigned char const *rgba )
{
}

// The shade mode
void CShaderAPIVulkan::ShadeMode( ShaderShadeMode_t mode )
{
}

// Binds a particular material to render with
void CShaderAPIVulkan::Bind( IMaterial *pMaterial )
{
}

// Cull mode
void CShaderAPIVulkan::CullMode( MaterialCullMode_t cullMode )
{
}

void CShaderAPIVulkan::ForceDepthFuncEquals( bool bEnable )
{
}

// Forces Z buffering on or off
void CShaderAPIVulkan::OverrideDepthEnable( bool bEnable, bool bDepthEnable )
{
}

void CShaderAPIVulkan::OverrideAlphaWriteEnable( bool bOverrideEnable, bool bAlphaWriteEnable )
{
}

void CShaderAPIVulkan::OverrideColorWriteEnable( bool bOverrideEnable, bool bColorWriteEnable )
{
}

//legacy fast clipping linkage
void CShaderAPIVulkan::SetHeightClipZ( float z )
{
}

void CShaderAPIVulkan::SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode )
{
}

// Sets the lights
void CShaderAPIVulkan::SetLight( int lightNum, const LightDesc_t &desc )
{
}

// Sets lighting origin for the current model
void CShaderAPIVulkan::SetLightingOrigin( Vector vLightingOrigin )
{
}

void CShaderAPIVulkan::SetAmbientLight( float r, float g, float b )
{
}

void CShaderAPIVulkan::SetAmbientLightCube( Vector4D cube[6] )
{
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
}

void CShaderAPIVulkan::SetSkinningMatrices()
{
}

// Lightmap texture binding
void CShaderAPIVulkan::BindLightmap( TextureStage_t stage )
{
}

void CShaderAPIVulkan::BindBumpLightmap( TextureStage_t stage )
{
}

void CShaderAPIVulkan::BindFullbrightLightmap( TextureStage_t stage )
{
}

void CShaderAPIVulkan::BindWhite( TextureStage_t stage )
{
}

void CShaderAPIVulkan::BindBlack( TextureStage_t stage )
{
}

void CShaderAPIVulkan::BindGrey( TextureStage_t stage )
{
}

// Gets the lightmap dimensions
void CShaderAPIVulkan::GetLightmapDimensions( int *w, int *h )
{
	g_pShaderUtil->GetLightmapDimensions( w, h );
}

// Special system flat normal map binding.
void CShaderAPIVulkan::BindFlatNormalMap( TextureStage_t stage )
{
}

void CShaderAPIVulkan::BindNormalizationCubeMap( TextureStage_t stage )
{
}

void CShaderAPIVulkan::BindSignedNormalizationCubeMap( TextureStage_t stage )
{
}

void CShaderAPIVulkan::BindFBTexture( TextureStage_t stage, int textureIndex )
{
}

// Flushes any primitives that are buffered
void CShaderAPIVulkan::FlushBufferedPrimitives()
{
}

// Gets the dynamic mesh; note that you've got to render the mesh
// before calling this function a second time. Clients should *not*
// call DestroyStaticMesh on the mesh returned by this call.
IMesh *CShaderAPIVulkan::GetDynamicMesh( IMaterial *pMaterial, int nHWSkinBoneCount, bool buffered,
    IMesh *pVertexOverride, IMesh *pIndexOverride )
{
	return &m_Mesh;
}

IMesh *CShaderAPIVulkan::GetDynamicMeshEx( IMaterial *pMaterial, VertexFormat_t fmt,
    int nHWSkinBoneCount, bool buffered, IMesh *pVertexOverride, IMesh *pIndexOverride )
{
	return &m_Mesh;
}

IMesh *CShaderAPIVulkan::GetFlexMesh()
{
	return &m_Mesh;
}

// Begins a rendering pass that uses a state snapshot
void CShaderAPIVulkan::BeginPass( StateSnapshot_t snapshot )
{
	// Bind the material shader this snapshot selected: look up its recorded
	// pixel-shader name and route the dynamic-mesh draw to the matching native
	// Vulkan pipeline. This is how a material's chosen shader reaches the GPU.
	const size_t index = static_cast<size_t>( ( snapshot >> 4 ) & 0x7FF );
	if ( index < g_snapshotShaders.size() )
	{
		const std::string &name = g_snapshotShaders[index];
		int shader = render_vulkan::CVulkanContext::kDynShaderPassthrough;
		// Map the bound shader name to a native pipeline. Real Source shader names
		// (e.g. "unlitgeneric_ps20b") are matched by prefix -- UnlitGeneric samples
		// $basetexture with the material transform, which the textured pipeline
		// implements natively. The bounded catalog names remain for the harness.
		if ( name == "greenify" )
			shader = render_vulkan::CVulkanContext::kDynShaderGreenify;
		else if ( name == "constantcolor" )
			shader = render_vulkan::CVulkanContext::kDynShaderConstColor;
		else if ( name == "basetexture" || name == "$basetexture" ||
		          name.compare( 0, 12, "unlitgeneric" ) == 0 )
			shader = render_vulkan::CVulkanContext::kDynShaderTextured;
		g_VulkanContext.SelectDynamicShader( shader );
	}
	// Apply the blend mode this snapshot recorded (opaque/translucent/additive).
	if ( index < g_snapshotBlend.size() )
		g_VulkanContext.SelectDynamicBlend( g_snapshotBlend[index] );
	// Apply the $alphatest reference this snapshot recorded (< 0 = disabled).
	if ( index < g_snapshotAlphaRef.size() )
		g_VulkanContext.SelectDynamicAlphaTest( g_snapshotAlphaRef[index] );
}

// Renders a single pass of a material
void CShaderAPIVulkan::RenderPass( int nPass, int nPassCount )
{
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
}

void CShaderAPIVulkan::PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar,
    double bottom, double top, double left, double right )
{
}

void CShaderAPIVulkan::PickMatrix( int x, int y, int width, int height )
{
}

void CShaderAPIVulkan::Rotate( float angle, float x, float y, float z )
{
}

void CShaderAPIVulkan::Translate( float x, float y, float z )
{
}

void CShaderAPIVulkan::Scale( float x, float y, float z )
{
}

void CShaderAPIVulkan::ScaleXY( float x, float y )
{
}

// Fog methods...
void CShaderAPIVulkan::FogMode( MaterialFogMode_t fogMode )
{
}

void CShaderAPIVulkan::FogStart( float fStart )
{
}

void CShaderAPIVulkan::FogEnd( float fEnd )
{
}

void CShaderAPIVulkan::SetFogZ( float fogZ )
{
}

void CShaderAPIVulkan::FogMaxDensity( float flMaxDensity )
{
}

void CShaderAPIVulkan::GetFogDistances( float *fStart, float *fEnd, float *fFogZ )
{
}

void CShaderAPIVulkan::SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
}

void CShaderAPIVulkan::SceneFogMode( MaterialFogMode_t fogMode )
{
}

void CShaderAPIVulkan::GetSceneFogColor( unsigned char *rgb )
{
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
}

void CShaderAPIVulkan::FogColor3fv( float const *rgb )
{
}

void CShaderAPIVulkan::FogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
}

void CShaderAPIVulkan::FogColor3ubv( unsigned char const *rgb )
{
}

void CShaderAPIVulkan::SetViewports( int nCount, const ShaderViewport_t *pViewports )
{
}

int CShaderAPIVulkan::GetViewports( ShaderViewport_t *pViewports, int nMax ) const
{
	return 1;
}

// Sets the vertex and pixel shaders
void CShaderAPIVulkan::SetVertexShaderIndex( int vshIndex )
{
}

void CShaderAPIVulkan::SetPixelShaderIndex( int pshIndex )
{
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
}

void CShaderAPIVulkan::SetIntegerVertexShaderConstant(
    int var, int const *pVec, int numConst, bool bForce )
{
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
}

void CShaderAPIVulkan::SetBooleanPixelShaderConstant(
    int var, BOOL const *pVec, int numBools, bool bForce )
{
}

void CShaderAPIVulkan::SetIntegerPixelShaderConstant(
    int var, int const *pVec, int numIntVecs, bool bForce )
{
}

void CShaderAPIVulkan::InvalidateDelayedShaderConstants( void )
{
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
	// Point the textured material shader at the bound texture (1-based handle;
	// 0 restores the built-in). Bounded slice: a single sampler stage.
	g_VulkanContext.BindManagedTexture( static_cast<int>( textureHandle ) - 1 );
}

void CShaderAPIVulkan::ClearColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	// Route the material system's clear color to the native Vulkan context; the
	// render pass applies it on the next BeginFrame (driven by Present()).
	g_VulkanContext.SetClearColor( r / 255.0f, g / 255.0f, b / 255.0f, 1.0f );
}

void CShaderAPIVulkan::ClearColor4ub(
    unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
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
void CShaderAPIVulkan::TexImage2D( int level, int cubeFace, ImageFormat dstFormat, int zOffset,
    int width, int height, ImageFormat srcFormat, bool bSrcIsTiled, void *imageData )
{
	// Upload the material's texture data into the native texture selected by
	// ModifyTexture (mip 0). Block-compressed DXT1/DXT5 (Portal's formats) upload
	// their compressed blocks directly to the matching BC image; uncompressed
	// RGBA/BGRA/RGB888 upload to a matching 8-bit image.
	if ( level != 0 || g_currentModifyTexture <= 0 || !imageData || width <= 0 || height <= 0 )
		return;

	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	const uint8_t *src = static_cast<const uint8_t *>( imageData );
	const size_t pixels = static_cast<size_t>( width ) * height;
	std::string error;
	bool ok = true;

	if ( srcFormat == IMAGE_FORMAT_DXT1 || srcFormat == IMAGE_FORMAT_DXT5 )
	{
		// 4x4 block compression: DXT1 = 8 bytes/block, DXT5 = 16 bytes/block.
		const size_t blocksX = ( static_cast<size_t>( width ) + 3 ) / 4;
		const size_t blocksY = ( static_cast<size_t>( height ) + 3 ) / 4;
		const size_t blockBytes = ( srcFormat == IMAGE_FORMAT_DXT1 ) ? 8 : 16;
		ok = g_VulkanContext.UploadManagedTexture(
		    handle, src, blocksX * blocksY * blockBytes, &error );
	}
	else if ( srcFormat == IMAGE_FORMAT_RGBA8888 || srcFormat == IMAGE_FORMAT_BGRA8888 )
	{
		// The image was created with the matching 8-bit format; upload directly.
		ok = g_VulkanContext.UploadManagedTexture( handle, src, pixels * 4, &error );
	}
	else if ( srcFormat == IMAGE_FORMAT_BGRX8888 )
	{
		// Opaque 32-bit BGR + unused X. The image is B8G8R8A8, so the bytes map
		// directly; force the (meaningless) X byte to an opaque alpha.
		std::vector<uint8_t> bgra( pixels * 4 );
		for ( size_t i = 0; i < pixels; ++i )
		{
			bgra[i * 4 + 0] = src[i * 4 + 0];
			bgra[i * 4 + 1] = src[i * 4 + 1];
			bgra[i * 4 + 2] = src[i * 4 + 2];
			bgra[i * 4 + 3] = 255;
		}
		ok = g_VulkanContext.UploadManagedTexture( handle, bgra.data(), bgra.size(), &error );
	}
	else if ( srcFormat == IMAGE_FORMAT_RGB888 || srcFormat == IMAGE_FORMAT_BGR888 )
	{
		// 24-bit color into the R8G8B8A8 image. BGR888 swaps R/B on the way in.
		const bool bgr = ( srcFormat == IMAGE_FORMAT_BGR888 );
		std::vector<uint8_t> rgba( pixels * 4 );
		for ( size_t i = 0; i < pixels; ++i )
		{
			rgba[i * 4 + 0] = src[i * 3 + ( bgr ? 2 : 0 )];
			rgba[i * 4 + 1] = src[i * 3 + 1];
			rgba[i * 4 + 2] = src[i * 3 + ( bgr ? 0 : 2 )];
			rgba[i * 4 + 3] = 255;
		}
		ok = g_VulkanContext.UploadManagedTexture( handle, rgba.data(), rgba.size(), &error );
	}
	else if ( srcFormat == IMAGE_FORMAT_I8 )
	{
		// 8-bit intensity expanded to grayscale RGBA (r = g = b = i, opaque).
		std::vector<uint8_t> rgba( pixels * 4 );
		for ( size_t i = 0; i < pixels; ++i )
		{
			rgba[i * 4 + 0] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = src[i];
			rgba[i * 4 + 3] = 255;
		}
		ok = g_VulkanContext.UploadManagedTexture( handle, rgba.data(), rgba.size(), &error );
	}
	else
	{
		Warning( "[NativeVulkan] TexImage2D: unsupported source format %d (skipped)\n", srcFormat );
		return;
	}

	if ( !ok )
		Warning( "[NativeVulkan] TexImage2D upload failed: %s\n", error.c_str() );
}

void CShaderAPIVulkan::TexSubImage2D( int level, int cubeFace, int xOffset, int yOffset,
    int zOffset, int width, int height, ImageFormat srcFormat, int srcStride, bool bSrcIsTiled,
    void *imageData )
{
}

void CShaderAPIVulkan::TexImageFromVTF( IVTFTexture *pVTF, int iVTFFrame )
{
}

bool CShaderAPIVulkan::TexLock( int level, int cubeFaceID, int xOffset, int yOffset, int width,
    int height, CPixelWriter &writer )
{
	return false;
}

void CShaderAPIVulkan::TexUnlock()
{
}

// These are bound to the texture, not the texture environment
void CShaderAPIVulkan::TexMinFilter( ShaderTexFilterMode_t texFilterMode )
{
}

void CShaderAPIVulkan::TexMagFilter( ShaderTexFilterMode_t texFilterMode )
{
}

void CShaderAPIVulkan::TexWrap( ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode )
{
}

void CShaderAPIVulkan::TexSetPriority( int priority )
{
}

ShaderAPITextureHandle_t CShaderAPIVulkan::CreateTexture( int width, int height, int depth,
    ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags, const char *pDebugName,
    const char *pTextureGroupName )
{
	// Map the Source image format to a Vulkan format. Block-compressed DXT1/DXT5
	// (Portal's texture formats) map to BC1/BC3 and are sampled natively.
	VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
	switch ( dstImageFormat )
	{
	case IMAGE_FORMAT_BGRA8888:
	case IMAGE_FORMAT_BGRX8888:
		vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
		break;
	case IMAGE_FORMAT_DXT1:
		vkFormat = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		break;
	case IMAGE_FORMAT_DXT5:
		vkFormat = VK_FORMAT_BC3_UNORM_BLOCK;
		break;
	default:
		vkFormat = VK_FORMAT_R8G8B8A8_UNORM; // RGBA8888 and RGBA-convertible sources
		break;
	}

	std::string error;
	const int native = g_VulkanContext.CreateManagedTexture( width, height, vkFormat, &error );
	if ( native < 0 )
	{
		Warning( "[NativeVulkan] CreateTexture failed: %s\n", error.c_str() );
		return 0;
	}
	return static_cast<ShaderAPITextureHandle_t>( native + 1 ); // 1-based handle
}

// Create a multi-frame texture (equivalent to calling "CreateTexture" multiple times, but more efficient)
void CShaderAPIVulkan::CreateTextures( ShaderAPITextureHandle_t *pHandles, int count, int width,
    int height, int depth, ImageFormat dstImageFormat, int numMipLevels, int numCopies, int flags,
    const char *pDebugName, const char *pTextureGroupName )
{
	for ( int k = 0; k < count; ++k )
		pHandles[k] = 0;
}

ShaderAPITextureHandle_t CShaderAPIVulkan::CreateDepthTexture(
    ImageFormat renderFormat, int width, int height, const char *pDebugName, bool bTexture )
{
	return 0;
}

void CShaderAPIVulkan::DeleteTexture( ShaderAPITextureHandle_t textureHandle )
{
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
	// Treat a full-frame color clear as the start of a new frame: discard the
	// previous frame's accumulated geometry now (not after Present), so the last
	// rendered frame's geometry stays available for an on-demand screenshot
	// capture (ReadPixels). Smaller render-target clears are left alone.
	if ( bClearColor && renderTargetWidth <= 0 && renderTargetHeight <= 0 )
		g_VulkanContext.ClearDynamicQueue();
}

void CShaderAPIVulkan::ClearBuffersObeyStencil( bool bClearColor, bool bClearDepth )
{
}

void CShaderAPIVulkan::ClearBuffersObeyStencilEx(
    bool bClearColor, bool bClearAlpha, bool bClearDepth )
{
}

void CShaderAPIVulkan::PerformFullScreenStencilOperation( void )
{
}

void CShaderAPIVulkan::SetScissorRect( const int nLeft, const int nTop, const int nRight,
    const int nBottom, const bool bEnableScissor )
{
}

void CShaderAPIVulkan::ReadPixels(
    int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat )
{
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
}

void CShaderAPIVulkan::ResetRenderState( bool bFullReset )
{
}

// Set the number of bone weights
void CShaderAPIVulkan::SetNumBoneWeights( int numBones )
{
}

void CShaderAPIVulkan::EnableHWMorphing( bool bEnable )
{
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
}

void CShaderAPIVulkan::EndFrame()
{
}

// returns the current time in seconds....
double CShaderAPIVulkan::CurrentTime() const
{
	return Sys_FloatTime();
}

// Get the current camera position in world space.
void CShaderAPIVulkan::GetWorldSpaceCameraPosition( float *pPos ) const
{
}

void CShaderAPIVulkan::ForceHardwareSync( void )
{
}

void CShaderAPIVulkan::SetClipPlane( int index, const float *pPlane )
{
}

void CShaderAPIVulkan::EnableClipPlane( int index, bool bEnable )
{
}

void CShaderAPIVulkan::SetFastClipPlane( const float *pPlane )
{
}

void CShaderAPIVulkan::EnableFastClip( bool bEnable )
{
}

int CShaderAPIVulkan::GetCurrentNumBones( void ) const
{
	return 0;
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
}

void CShaderAPIVulkan::EvictManagedResources()
{
}

void CShaderAPIVulkan::SetTextureTransformDimension(
    TextureStage_t textureStage, int dimension, bool projected )
{
}

void CShaderAPIVulkan::SetBumpEnvMatrix(
    TextureStage_t textureStage, float m00, float m01, float m10, float m11 )
{
}

void CShaderAPIVulkan::SyncToken( const char *pToken )
{
}
