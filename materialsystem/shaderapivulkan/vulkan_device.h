//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan device bring-up and presentation core for the
//          shaderapivulkan backend (RFC 0001 rank 14/16, roadmap R28/R32).
//
//          This module owns the genuine native Vulkan objects: instance,
//          physical-device selection, logical device and queues, the window
//          surface, the swapchain and its render targets, per-frame
//          command buffers and synchronization, and clean teardown. It clears
//          and presents a real frame and can read the presented image back for
//          outcome-driven verification.
//
//          It deliberately depends only on <vulkan/vulkan.h> and the C++
//          standard library so the same core is exercised by both the engine
//          backend and the standalone native smoke test, with no ambient engine
//          state. The window is reached only through IVulkanSurfaceHost, which a
//          pair-specific bridge implements (RFC 0001 R16); this core includes no
//          window-system header and never interprets a native window handle.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_DEVICE_H
#define SHADERAPIVULKAN_VULKAN_DEVICE_H

#ifdef _WIN32
#pragma once
#endif

#include "vulkan_frame_stats.h"
#include "vulkan_surface_host.h"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

namespace render_vulkan
{

// An allocator whose value-less construct leaves the element uninitialized, so
// resize() grows a buffer that the caller is about to overwrite without first
// zero-filling it (the frame's vertex stream, written in place per draw).
template <typename T> struct DefaultInitAllocator : std::allocator<T>
{
	template <typename U> struct rebind
	{
		typedef DefaultInitAllocator<U> other;
	};
	DefaultInitAllocator() = default;
	template <typename U> DefaultInitAllocator( const DefaultInitAllocator<U> & ) {}
	template <typename U> void construct( U *p ) { ::new ( static_cast<void *>( p ) ) U; }
	template <typename U, typename... Args> void construct( U *p, Args &&...args )
	{
		::new ( static_cast<void *>( p ) ) U( std::forward<Args>( args )... );
	}
};

// Requested behavior for a device bring-up. Required behavior that cannot be
// satisfied fails Init() with a diagnostic instead of silently degrading.
struct VulkanContextConfig
{
	const char *appName = "Source Native Vulkan";
	// Turn on the Khronos validation layer and a debug messenger. Init() fails
	// if validation is required but the layer/extension are unavailable.
	bool enableValidation = false;
	bool requireValidation = false;
	// Require a discrete GPU; otherwise the best available device is chosen.
	bool requireDiscreteGpu = false;
	// Number of frames that may be recorded/submitted before the oldest must
	// complete. Clamped to [1, kMaxFramesInFlight].
	uint32_t framesInFlight = 2;
	// Prefer mailbox (low-latency) present; falls back to FIFO (always
	// supported) when mailbox is unavailable.
	bool preferMailbox = false;
};

// A single, coherent native Vulkan presentation context bound to one window
// through its surface host. Not copyable; owns all its Vulkan handles and destroys them in
// reverse dependency order at Shutdown()/destruction.
class CVulkanContext
{
public:
	CVulkanContext() = default;
	~CVulkanContext();

	CVulkanContext( const CVulkanContext & ) = delete;
	CVulkanContext &operator=( const CVulkanContext & ) = delete;

	// Bring up every object needed to present to the host's window. The host is
	// borrowed and must outlive Shutdown(). On failure returns false, fills
	// *outError with a specific reason, and leaves the context fully torn down
	// (safe to destroy, never partially live).
	bool Init( IVulkanSurfaceHost &host, const VulkanContextConfig &config, std::string *outError );

	// Idempotent teardown. Waits for the device to go idle first.
	void Shutdown();

	bool IsValid() const { return m_device != VK_NULL_HANDLE; }

	// Begin recording a frame. Acquires the next swapchain image (recreating
	// the swapchain on OUT_OF_DATE), begins the primary command buffer, and
	// opens the clear render pass. Returns false with *outError on a real
	// error; returns true with *outSkip==true when the frame should be skipped
	// this iteration (e.g. a zero-size / minimized window) with no error.
	bool BeginFrame( bool *outSkip, std::string *outError );

	// The color the next BeginFrame's render pass clears to (linear RGBA, 0..1).
	void SetClearColor( float r, float g, float b, float a );

	// Close the render pass and submit + present the acquired image. When a
	// capture was requested via RequestCapture() before this call, the color
	// image is copied to host-visible memory and is available afterwards from
	// GetCapturedPixels(). Returns false with *outError on a real error.
	bool EndFrame( std::string *outError );

	// Recreate the swapchain and its targets for a new drawable size. Safe to
	// call between frames. A zero-size result is retained without error and
	// causes subsequent BeginFrame calls to report a skip.
	bool Resize( int width, int height, std::string *outError );

	// Ask EndFrame() to capture the rendered color image (the back buffer) this frame.
	void RequestCapture() { m_captureRequested = true; }
	// Ask EndFrame() to capture the swapchain image instead: the back buffer as
	// the window presents it, scaled to the drawable. Nothing is captured when
	// the surface does not allow reading its images.
	void RequestPresentedCapture()
	{
		m_captureRequested = true;
		m_capturePresented = true;
	}

	// Bring up a bounded demo pipeline: real shader modules, a graphics
	// pipeline matching the swapchain render pass, and a GPU vertex buffer with
	// an interleaved position+color triangle. This exercises the pipeline /
	// shader / vertex-buffer / draw path the material renderer (R32) will use.
	// Independent of Init(); call it after a successful Init().
	bool InitDemoTriangle( std::string *outError );

	// When enabled, each subsequent frame rasterizes the demo triangle over the
	// clear. Requires a prior successful InitDemoTriangle().
	void SetDrawDemoTriangle( bool enable ) { m_drawDemoTriangle = enable; }
	bool DemoTriangleReady() const { return m_demoPipeline != VK_NULL_HANDLE; }

	// Bring up a bounded textured-quad pipeline: a device-local RGBA texture
	// uploaded through a staging buffer with layout transitions, a sampler, a
	// combined-image-sampler descriptor set, and a pipeline that samples it onto
	// a quad. Exercises the texture / sampler / descriptor path the material
	// renderer (R32) will use. Call after a successful Init().
	bool InitTexturedQuad( std::string *outError );
	void SetDrawTexturedQuad( bool enable ) { m_drawTexturedQuad = enable; }
	bool TexturedQuadReady() const { return m_texQuadPipeline != VK_NULL_HANDLE; }

	// Bring up a bounded indexed + uniform-buffer pipeline: an index buffer
	// (vkCmdDrawIndexed) and a uniform (constant) buffer whose color is bound
	// through a descriptor set and written by the fragment shader. Exercises the
	// index-buffer and shader-constant path the material renderer (R32) leans on
	// most (every material sets shader constants). Call after a successful Init().
	bool InitIndexedUbo( std::string *outError );
	void SetDrawIndexedUbo( bool enable ) { m_drawIndexedUbo = enable; }
	bool IndexedUboReady() const { return m_indexedUboPipeline != VK_NULL_HANDLE; }
	// Update the uniform-buffer color (linear RGBA). Takes effect next frame.
	void SetIndexedUboColor( float r, float g, float b, float a );

	// Bring up a bounded depth-tested pipeline: two overlapping triangles at
	// different depths drawn far-then-near with depth testing on, proving the
	// depth attachment resolves occlusion (the near triangle wins regardless of
	// draw order) -- the capability real 3D scene rendering needs. Call after a
	// successful Init().
	bool InitDemoDepth( std::string *outError );
	void SetDrawDemoDepth( bool enable ) { m_drawDemoDepth = enable; }
	bool DemoDepthReady() const { return m_demoDepthPipeline != VK_NULL_HANDLE; }

	// Dynamic geometry path used by the material system's mesh interface: a
	// depth-tested position+color pipeline plus a growable host-visible vertex
	// buffer. Callers queue triangles during a frame; the next BeginFrame uploads
	// and draws them, then the queue is cleared. This is how IMesh::Draw reaches
	// the GPU through the backend (roadmap R32 mesh slice).
	bool InitDynamicMesh( std::string *outError );
	bool DynamicMeshReady() const { return m_dynPipeline != VK_NULL_HANDLE; }
	// Append triangle-list vertices as interleaved [x,y,z, r,g,b, u,v] floats,
	// plus [u,v] lightmap coordinates per vertex when the draw has them (zeros
	// otherwise). They are stored as one kDynVertexFloats-wide record.
	enum
	{
		kDynVertexFloats = 18
	};
	// `normalTangent`, when given, holds each vertex's normal (3) and tangent
	// (4, TANGENT/USERDATA with the binormal sign in w), 7 floats per vertex; it
	// follows the lightmap coordinates in the record (zeros otherwise).
	// `vertexAlpha`, when given, holds each vertex color's alpha, 1 float per
	// vertex; it ends the record (1, opaque, otherwise).
	void QueueDynamicTriangles( const float *posColorUvInterleaved, uint32_t vertexCount,
	    const float *lightmapUv = nullptr, const float *normalTangent = nullptr,
	    const float *vertexAlpha = nullptr );
	// The same draw written in place: record a draw with the current state and
	// reserve room for up to `maxVertices` kDynVertexFloats-wide records in the
	// frame's stream; the caller fills them and ends the draw with the count it
	// wrote (0 withdraws the draw). The pointer is valid until the next call
	// that queues geometry. This avoids building the vertices in caller buffers
	// only to copy them here.
	// A draw may also be indexed: `maxIndices` > 0 reserves an index list,
	// relative to the draw's first vertex, returned through *outIndices; the
	// draw then renders the index count EndDynamicDraw is given.
	float *BeginDynamicDraw(
	    uint32_t maxVertices, uint32_t maxIndices = 0, uint32_t **outIndices = nullptr );
	void EndDynamicDraw( uint32_t vertexCount, uint32_t indexCount = 0 );
	// Record a draw with the current state over geometry an earlier draw of the
	// same stream already queued (its vertex and index ranges, as a DrawRange
	// read back after its EndDynamicDraw). Nothing is added to the stream.
	struct DrawRange
	{
		uint32_t firstVertex;
		uint32_t vertexCount;
		uint32_t firstIndex;
		uint32_t indexCount;
	};
	DrawRange LastDrawRange() const;
	void ReuseDynamicDraw( const DrawRange &range );
	// Whether `range` of the stream holds exactly these vertices and indices.
	bool StreamRangeEquals( const DrawRange &range, const float *vertices, uint32_t vertexCount,
	    const uint32_t *indices, uint32_t indexCount ) const;
	// Changes whenever the stream is discarded; a DrawRange is valid only while
	// the epoch it was read in lasts.
	uint64_t StreamEpoch() const { return m_streamEpoch; }
	// Discard the accumulated frame geometry. Called at frame start (ClearBuffers)
	// rather than after Present, so the last frame's geometry stays available for
	// an on-demand screenshot capture (ReadPixels).
	void ClearDynamicQueue()
	{
		FailUnsubmittedQueries();
		++m_streamEpoch;
		m_dynQueued.clear();
		m_dynIndices.clear();
		m_dynDrawRecords.clear();
		m_dynSkinConstants.clear();
		m_dynFramePresented = false;
	}
	// Select which material-shader pipeline the queued geometry uses this frame
	// (0 = vertex-color passthrough, 1 = "greenify", 2 = "constant color"). This
	// is how a bound shader (BeginPass) reaches the draw.
	enum
	{
		kDynShaderPassthrough = 0,
		kDynShaderGreenify = 1,
		kDynShaderConstColor = 2,
		kDynShaderTextured = 3,
		// PortalRefract (portal_refract_vs20 / portal_refract_ps2x), all three
		// stages; see shaders/portal_refract.{vert,frag}.
		kDynShaderPortalRefract = 4,
		// VertexLitGeneric's $phong path (skin_vs20 / skin_ps20b); see
		// shaders/skin.{vert,frag}.
		kDynShaderSkin = 5,
		// RFC 0007 synthetic direct-light BRDF, tested against the headless model.
		kDynShaderPbrDirect = 6
	};
	void SelectDynamicShader( int shaderIndex ) { m_dynShaderIndex = shaderIndex; }
	// Output-merger state of the queued geometry, in the terms of the D3D9 state a
	// material's IShaderShadow selects: blend factors (applied to color and alpha
	// alike, as D3D9 does without separate alpha blending) and the depth test,
	// write and comparison. The defaults are the D3D9 shadow defaults. Only the
	// textured (material) pipeline honors this; one pipeline is built per
	// distinct state, on first use.
	struct DynRasterState
	{
		bool blend = false;
		VkBlendFactor srcFactor = VK_BLEND_FACTOR_ONE;
		VkBlendFactor dstFactor = VK_BLEND_FACTOR_ZERO;
		bool depthTest = true;
		bool depthWrite = true;
		VkCompareOp depthCompare = VK_COMPARE_OP_LESS_OR_EQUAL;
		// IShaderShadow::EnableColorWrites; false leaves the target unchanged (a
		// draw that only feeds an occlusion query, such as dev/lumcompare).
		bool colorWrite = true;
		// D3D9's effective D3DRS_CULLMODE (the shadow state's EnableCulling with the
		// dynamic CullMode). Triangles are wound in D3D screen space, which the
		// flipped draw viewport preserves, so the front face is clockwise.
		VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
		// D3D9's D3DRS_STENCIL* render state (IShaderDynamicAPI SetStencil*): the
		// test and the three operations, applied to front and back faces alike
		// (D3D9 without two-sided stencil). The reference and the masks are
		// per-draw values (SetDynamicStencilValues), not pipeline state.
		bool stencilEnable = false;
		VkCompareOp stencilCompare = VK_COMPARE_OP_ALWAYS;
		VkStencilOp stencilFail = VK_STENCIL_OP_KEEP;
		VkStencilOp stencilDepthFail = VK_STENCIL_OP_KEEP;
		VkStencilOp stencilPass = VK_STENCIL_OP_KEEP;
	};
	void SelectDynamicRasterState( const DynRasterState &state ) { m_dynRaster = state; }
	// Distinct states have distinct keys.
	static uint32_t RasterStateKey( const DynRasterState &state );
	// Its inverse: the state a key was made from (fields a disabled stencil test
	// ignores come back as their defaults).
	static DynRasterState RasterStateFromKey( uint64_t key );
	// $alphatest: fragments whose alpha is below `ref` are discarded (matching the
	// D3D9 fixed-function GREATEREQUAL alpha test, or GREATER with
	// kFragmentAlphaGreater). A negative `ref` disables it.
	void SelectDynamicAlphaTest( float ref ) { m_dynAlphaRef = ref; }
	// D3DRS_STENCILREF, D3DRS_STENCILMASK and D3DRS_STENCILWRITEMASK of the draws
	// that follow.
	void SetDynamicStencilValues( uint32_t reference, uint32_t testMask, uint32_t writeMask )
	{
		m_dynStencilRef = reference;
		m_dynStencilTestMask = testMask;
		m_dynStencilWriteMask = writeMask;
	}
	// Bits of the depth-stencil attachment's stencil aspect (0 when the device
	// offers no depth format with stencil).
	int StencilBits() const { return m_stencilBits; }
	// True when blended sRGB-write draws blend in linear space, as on D3D9.
	bool LinearSpaceSrgbBlending() const { return m_srgbAttachments; }
	// User clip planes of the draws that follow, in D3D clip space (D3D9's
	// SetClipPlane under a vertex shader): a vertex is kept where
	// dot( plane, position ) >= 0. At most kMaxClipPlanes; 0 disables clipping.
	enum
	{
		kMaxClipPlanes = 2,
		// Push-constant bytes of the textured pipeline with clip planes (its 32
		// floats and the planes) and of PortalRefract (48 floats and the planes).
		kTexturedPushBytes = ( 32 + 4 * kMaxClipPlanes ) * 4,
		kPortalPushBytes = ( 48 + 4 * kMaxClipPlanes ) * 4,
		// The skin shader: cViewProj, two texture transform rows, cEyePos and two
		// parameter vectors (36 floats), then the planes.
		kSkinPushBytes = ( 36 + 4 * kMaxClipPlanes ) * 4
	};
	// kMaxClipPlanes when the device can clip (shaderClipDistance and push
	// constants wide enough for the planes), else 0.
	int MaxClipPlanes() const { return m_clipPlanesSupported ? kMaxClipPlanes : 0; }
	// Whether BC1-BC3 (DXT1/DXT3/DXT5) images can be created and sampled: the
	// textureCompressionBC feature, enabled at device creation when supported.
	bool SupportsBlockCompression() const { return m_blockCompression; }
	void SetDynamicClipPlanes( int count, const float ( *planes )[4] )
	{
		m_dynClipPlaneCount = count < 0 ? 0 : ( count > kMaxClipPlanes ? kMaxClipPlanes : count );
		for ( int i = 0; i < m_dynClipPlaneCount; ++i )
			for ( int k = 0; k < 4; ++k )
				m_dynClipPlanes[i][k] = planes[i][k];
	}
	// Set the shader constant the "constant color" material shader reads (linear
	// RGBA), the way a material's pixel-shader constant parameterizes its shader.
	void SetDynamicConstantColor( float r, float g, float b, float a )
	{
		m_dynConstColor[0] = r;
		m_dynConstColor[1] = g;
		m_dynConstColor[2] = b;
		m_dynConstColor[3] = a;
	}
	// Set cModulationColor ($color * $alpha), the modulation the UnlitGeneric
	// material path multiplies the base texture by (linear RGBA). Defaults to
	// white (1,1,1,1) so an unmodulated material is unaffected.
	void SetDynamicModulation( const float *rgba )
	{
		if ( !rgba )
			return;
		for ( int i = 0; i < 4; ++i )
			m_dynModulation[i] = rgba[i];
	}
	void SetDynamicMonitorContrast( float contrast ) { m_dynMonitorContrast = contrast; }
	// Set cBaseTextureTransform (the two float4 rows of the 2x4 affine UV
	// transform, SHADER_SPECIFIC_CONST_0/1). Defaults to identity (row0 =
	// 1,0,0,0 / row1 = 0,1,0,0), i.e. the texture coordinate passes through.
	void SetDynamicBaseTexTransform( const float *row0, const float *row1 )
	{
		for ( int i = 0; i < 4; ++i )
		{
			if ( row0 )
				m_dynTexXform0[i] = row0[i];
			if ( row1 )
				m_dynTexXform1[i] = row1[i];
		}
	}
	// Set the model->projection transform the dynamic vertex shader applies (16
	// floats, column-major), the way a material sets vertex-shader constants
	// c0-c3. Defaults to identity.
	void SetDynamicTransform( const float *m16 );

	// Material-supplied textures (IShaderAPI CreateTexture/TexImage2D/BindTexture).
	// CreateManagedTexture returns a handle >= 0, UploadManagedTexture fills it
	// with tightly-packed 8-bit RGBA, and BindManagedTexture points the textured
	// material shader's sampler at it (handle < 0 restores the built-in texture).
	// `format` may be an uncompressed (R8G8B8A8/B8G8R8A8) or block-compressed
	// (BC1/BC3, i.e. DXT1/DXT5) Vulkan format; upload data must match it.
	// `mipLevels` is clamped to the full chain; each level is uploaded separately.
	// `srgbAlias`, when set, is the sRGB twin of `format`: the image is created
	// mutable between the two and also gets an sRGB view (render targets).
	int CreateManagedTexture( int width, int height, VkFormat format, std::string *outError,
	    VkImageUsageFlags extraUsage = 0, uint32_t mipLevels = 1,
	    VkFormat srgbAlias = VK_FORMAT_UNDEFINED );
	bool UploadManagedTexture( int handle, const uint8_t *data, size_t dataSize,
	    std::string *outError, uint32_t level = 0 );
	// Releases a managed texture (IShaderAPI::DeleteTexture). The handle stops
	// naming it at once: records still referencing it sample the built-in
	// texture, and records rendering into it are dropped. Its Vulkan objects
	// outlive every frame already submitted, which may still read them; the
	// handle is reused only after that.
	void DestroyManagedTexture( int handle );
	// Fills the width x height rectangle at (x, y) of a level with tightly packed
	// data in the image's format, keeping the texels outside it (IShaderAPI
	// TexSubImage2D; VGUI writes its font pages a glyph at a time). A region of
	// a never-filled image leaves the rest zero. Compressed regions must be 4x4
	// block aligned.
	bool UploadManagedTextureRegion( int handle, uint32_t x, uint32_t y, uint32_t width,
	    uint32_t height, const uint8_t *data, size_t dataSize, std::string *outError,
	    uint32_t level = 0 );
	uint32_t ManagedTextureMipLevels( int handle ) const
	{
		return ( handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) )
		           ? m_managedTextures[static_cast<size_t>( handle )].mipLevels
		           : 0;
	}
	void BindManagedTexture( int handle );
	// The lightmap page the textured pipeline multiplies by (LightmappedGeneric's
	// TEXTURE_LIGHTMAP on sampler 1), sampled at the lightmap coordinates; -1
	// draws without a lightmap.
	void BindManagedLightmap( int handle ) { m_dynLightmapHandle = handle; }
	// Textures on samplers 1..15 for shaders that read them as ordinary
	// textures (PortalRefract's noise and color, the skin shader's normal,
	// exponent, warp and self-illumination maps), -1 for none.
	enum
	{
		kMaxSamplers = 16
	};
	void BindManagedSampler( int sampler, int handle )
	{
		if ( sampler >= 1 && sampler < kMaxSamplers )
			m_dynSamplerHandles[sampler] = handle;
	}
	// PortalRefract's constants, in its registers' terms (portal_refract_vs20.fxc
	// and portal_refract_ps2x.fxc). The model and view-projection matrices are
	// laid out like SetDynamicTransform's; `stage` is the STAGE static combo.
	struct PortalConstants
	{
		float model[16];
		float viewProj[16];
		float texXform0[4]; // SHADER_SPECIFIC_CONST_1
		float texXform1[4]; // SHADER_SPECIFIC_CONST_2
		float time = 0.0f;  // SHADER_SPECIFIC_CONST_0.x, already mod 1000
		float openAmount = 0.0f;
		float active = 1.0f; // 1 - $PortalStatic
		float colorScale = 0.0f;
		int stage = 0;
	};
	void SetDynamicPortalConstants( const PortalConstants &constants ) { m_dynPortal = constants; }
	// The skin shader's constants: the pixel shader registers c0..c31 as the
	// material's dynamic state wrote them (skin_dx9_helper.cpp), and the vertex
	// stage's cViewProj, cBaseTexCoordTransform and cEyePos. `combos` holds the
	// skin_ps20b static combos as skin.frag's flags.
	struct SkinConstants
	{
		float ps[32][4];
		float viewProj[16];
		float texXform0[4];
		float texXform1[4];
		float eyePos[4];
		int combos = 0;
		int numLights = 0;
	};
	void SetDynamicSkinConstants( const SkinConstants &constants ) { m_dynSkin = constants; }
	// Synthetic PBR test inputs: N.V, N.L, N.H and V.H. The real material
	// receives these from geometry and lights when the PBR world path arrives.
	void SetDynamicPbrAngles( const float *angles )
	{
		for ( int i = 0; i < 4; ++i )
			m_dynPbrAngles[i] = angles[i];
	}
	bool PbrDirectPipelineSupported() const { return m_pbrDirectReady; }
	// False when the device cannot bind the skin shader's seven descriptor sets
	// or its push block; its draws are then declined.
	bool SkinPipelineSupported() const { return m_skinPipelineLayout != VK_NULL_HANDLE; }
	// False when the device's push constants cannot hold PortalRefract's block;
	// its draws are then declined.
	bool PortalPipelineSupported() const { return m_portalPipelineLayout != VK_NULL_HANDLE; }
	// Which of the textured pipeline's inputs and output are sRGB-encoded, as the
	// material's IShaderShadow EnableSRGBRead/EnableSRGBWrite declared them: an
	// sRGB input is decoded to linear before use, and linear output is encoded.
	// The same flags select the textured pipeline's other D3D9 shader variants:
	// the GREATER alpha test, luminance_compare_ps2x (which reads its c0 from the
	// constant color, SetDynamicConstantColor), and screenspaceeffect_vs20's
	// clip-space positions and untransformed texture coordinates.
	enum
	{
		kColorSrgbReadBase = 1,
		kColorSrgbReadLightmap = 2,
		kColorSrgbWrite = 4,
		kFragmentAlphaGreater = 8,
		kFragmentLuminanceCompare = 16,
		kVertexScreenSpace = 32,
		// PortalRefract decodes sampler 2 (its color ramp) from sRGB.
		kColorSrgbReadSampler2 = 64,
		// The vertex color replaces the base texture (BufferClearObeyStencil).
		kFragmentVertexColor = 128,
		// vertexlit_and_unlit_generic with VERTEXCOLOR ($vertexcolor): the result
		// is multiplied by the vertex color, which the vertex stage converts from
		// gamma to linear unless kVertexColorNoGammaConvert
		// (DONT_GAMMA_CONVERT_VERTEX_COLOR, a material that does not write sRGB).
		kFragmentModulateVertexColor = 256,
		kVertexColorNoGammaConvert = 512,
		// ... and with g_fVertexAlpha ($vertexalpha): alpha times the vertex alpha.
		kFragmentModulateVertexAlpha = 1024,
		// vertexlit_and_unlit_generic with SELFILLUM ($selfillum): the base alpha
		// blends the vertex color (the diffuse term, times c1) toward the
		// modulation (c1 times $selfillumtint); alpha is the modulation's.
		kFragmentSelfIllum = 2048,
		kFragmentCable = 4096,
		kFragmentSky = 8192,
		kFragmentMonitor = 16384,
		kFragmentMonitorTexture2 = 32768,
		// Sprite_DX9's VERTEXCOLOR combo multiplies both RGB and alpha directly.
		kFragmentSpriteVertexAlpha = 65536
	};
	void SelectDynamicColorSpace( int flags ) { m_dynColorFlags = flags; }
	// Linear scale applied to the textured pipeline's color before the sRGB
	// encode: D3D9's FinalOutput LINEAR_LIGHT_SCALE (cLightScale.x), the tone
	// mapping scale in integer HDR. 1 leaves the color unchanged.
	void SetDynamicOutputScale( float scale ) { m_dynOutputScale = scale; }
	// True when this managed texture has had pixel data uploaded into it.
	bool IsManagedTextureUploaded( int handle ) const
	{
		return handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) &&
		       m_managedTextures[static_cast<size_t>( handle )].uploaded;
	}

	// Render targets (IShaderAPI SetRenderTarget and TEXTURE_CREATE_RENDERTARGET).
	// A render-target texture is a managed texture that can also be drawn into:
	// it is created in the swapchain format with its own depth buffer, so the
	// pipelines built for the swapchain pass draw into it unchanged. It starts
	// cleared to opaque black and counts as resident.
	int CreateRenderTargetTexture( int width, int height, std::string *outError );
	bool IsRenderTargetTexture( int handle ) const
	{
		return handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) &&
		       m_managedTextures[static_cast<size_t>( handle )].renderTarget;
	}
	// Direct subsequent records into a render-target texture (-1 = swapchain).
	// Like D3D9's SetRenderTarget this resets the viewport to the whole target.
	void SetRenderTarget( int handle );
	int RenderTarget() const { return m_dynTarget; }
	void GetRenderTargetExtent( int &width, int &height ) const
	{
		uint32_t w = 0, h = 0;
		GetTargetExtent( m_dynTarget, &w, &h );
		width = static_cast<int>( w );
		height = static_cast<int>( h );
	}
	// Identity attached to subsequent records, reported by DescribeStreamRecords.
	void SetRecordTag( int tag ) { m_dynTag = tag; }

	// Per-texture sampler state (IShaderAPI TexWrap/TexMinFilter/TexMagFilter).
	enum
	{
		kSamplerClampU = 1,
		kSamplerClampV = 2,
		kSamplerLinear = 4,
		// D3DSAMP_MIPFILTER: point or linear between mip levels; neither samples
		// level 0 only (D3DTEXF_NONE).
		kSamplerMipPoint = 8,
		kSamplerMipLinear = 16,
		kSamplerStates = 32
	};
	void SetManagedTextureSamplerState( int handle, int samplerState );
	int ManagedTextureSamplerState( int handle ) const
	{
		return ( handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) )
		           ? m_managedTextures[static_cast<size_t>( handle )].samplerState
		           : 0;
	}
	void SetViewport( int x, int y, int width, int height, float minZ, float maxZ );
	void SetScissor( bool enable, int x, int y, int width, int height );
	// Clear the current viewport of the current target to the clear color
	// (SetClearColor) and/or to depth 1.
	void QueueClear( bool color, bool depth, bool stencil = false );
	// Copy the current target into a render-target texture, scaling between the
	// rectangles ({x, y, width, height}; null = the whole image).
	bool QueueCopyToTexture( int dstHandle, const int *srcRect, const int *dstRect );
	// Occlusion queries (IShaderAPI CreateOcclusionQueryObject and friends). Begin
	// and end are stream records like draws, so a query counts exactly the
	// samples the draws between them pass, in engine order. A result exists once
	// the frame holding the query has been submitted; OcclusionQueryResult
	// returns kQueryPending until the GPU has produced it (waiting for it when
	// `wait` is set), and kQueryFailed for a query that cannot produce one: its
	// frame was discarded unsubmitted, it was split across render passes, or it
	// is read with `wait` before its frame was submitted. Creation fails (-1)
	// when the device cannot count samples exactly (occlusionQueryPrecise).
	enum
	{
		kQueryPending = -1,
		kQueryFailed = -2,
		kMaxOcclusionQueries = 1024
	};
	int CreateOcclusionQuery( std::string *outError );
	void DestroyOcclusionQuery( int query );
	void QueueBeginOcclusionQuery( int query );
	void QueueEndOcclusionQuery( int query );
	int64_t OcclusionQueryResult( int query, bool wait );
	// Called once the frame has been presented. The recorded stream stays
	// available for an on-demand capture until the next frame records into it.
	void EndStreamFrame() { m_dynFramePresented = true; }
	// One line per target of the recorded stream (draws, clears, vertices,
	// copies out), in first-use order: what the next capture will replay.
	std::string DescribeStream() const;
	// The recorded stream record by record, for per-draw diagnosis.
	struct StreamRecordInfo
	{
		int kind;
		int target;
		int tag;
		bool clearColor;
		bool clearDepth;
		float clearValue[4];
		int shaderIndex;
		int texHandle;
		DynRasterState raster;
		uint32_t vertexCount;
		float modulation[4];
		float firstColor[3];
		float viewport[6];
		float uvMin[2];
		float uvMax[2];
		float texXform0[4];
		float texXform1[4];
	};
	std::vector<StreamRecordInfo> DescribeStreamRecords() const;

	// The most recent captured frame as tightly-packed 8-bit RGBA, top row
	// first. Empty until a captured EndFrame() completes. *outW/*outH give the
	// captured extent.
	const std::vector<uint8_t> &GetCapturedPixels( int *outW, int *outH ) const;

	// Selected adapter facts, valid after a successful Init().
	const char *DeviceName() const { return m_deviceName.c_str(); }
	uint32_t VendorId() const { return m_vendorId; }
	uint32_t DeviceId() const { return m_deviceId; }
	uint64_t DeviceLocalMemoryBytes() const { return m_deviceLocalMemoryBytes; }
	bool IsDiscrete() const { return m_isDiscrete; }
	bool ValidationEnabled() const { return m_validationEnabled; }

	// The back buffer the engine renders into (D3D9's BackBufferWidth/Height).
	void GetSwapchainExtent( int &width, int &height ) const
	{
		width = static_cast<int>( m_swapExtent.width );
		height = static_cast<int>( m_swapExtent.height );
	}
	// The window's drawable (swapchain) extent the back buffer is presented to.
	void GetPresentExtent( int &width, int &height ) const
	{
		width = static_cast<int>( m_presentExtent.width );
		height = static_cast<int>( m_presentExtent.height );
	}
	// The back buffer's size, as D3D9 takes it from the video mode
	// (D3DPRESENT_PARAMETERS BackBufferWidth/Height), independent of the
	// window's drawable size: Present scales it to the window, as a windowed
	// D3D9 Present stretches the back buffer over the client area. 0 x 0 follows
	// the drawable. Before Init it only records the size; afterwards a changed
	// size recreates the back buffers between frames.
	bool SetBackBufferSize( int width, int height, std::string *outError );
	// Frames presented, and how many of them the present blit had to scale
	// because the back buffer and the drawable differed.
	uint64_t PresentCount() const { return m_presentCount; }
	uint64_t ScaledPresentCount() const { return m_scaledPresentCount; }

	// Count of validation messages of severity WARNING or ERROR observed since
	// Init(). Zero on a clean run when validation is enabled.
	uint32_t ValidationErrorCount() const { return m_validationErrorCount; }

	// Frame-pacing telemetry (vulkan_frame_stats.h). OpenFrameStats writes one
	// JSON line per presented frame to path until CloseFrameStats/Shutdown; it
	// fails without side effects when the file cannot be created. MarkFrame
	// labels the frame being built (a scenario phase such as "fire_blue"), so a
	// measured hitch can be placed in the workload that produced it. Labels are
	// reduced to [A-Za-z0-9_.-].
	// Pipeline store: a VkPipelineCache and the list of material pipeline
	// variants (family and raster-state key) this game has needed, kept in two
	// files under `directory`. Open it after Init and before InitDynamicMesh;
	// PrewarmPipelines then builds every recorded variant up front (at load, not
	// the first frame that needs one), and SavePipelineStore (also run by
	// Shutdown) writes both files back. A missing or unreadable store starts
	// empty; the driver rejects cache data from another device or driver.
	bool OpenPipelineStore( const std::string &directory, std::string *outError );
	int PrewarmPipelines();
	bool SavePipelineStore( std::string *outError );
	bool OpenFrameStats( const char *path, std::string *outError );
	void CloseFrameStats();
	void MarkFrame( const char *label );
	FrameCost &CurrentFrameCost() { return m_frameCost; }

	enum
	{
		kMaxFramesInFlight = 3
	};

private:
	// Frame-pacing telemetry state: the cost of the frame being built, the sink,
	// frame boundaries (FrameClockMicros), pending marks, and per-slot GPU
	// timestamps (begin/end of each slot's command buffer) with the stats frame
	// each slot last submitted and the newest GPU duration read back.
	FrameCost m_frameCost;
	FILE *m_frameStatsFile = nullptr;
	uint64_t m_statsFrame = 0;
	uint64_t m_frameBeginUs = 0;
	uint64_t m_prevFrameBeginUs = 0;
	uint64_t m_prevFrameEndUs = 0;
	uint64_t m_recordBeginUs = 0;
	uint64_t m_frameBeginCpuUs = 0;
	uint64_t m_prevFrameBeginCpuUs = 0;
	std::string m_frameMarks;
	VkQueryPool m_timestampPool = VK_NULL_HANDLE;
	double m_timestampPeriodNs = 0.0;
	uint64_t m_timestampMask = 0;
	uint64_t m_slotStatsFrame[kMaxFramesInFlight] = {};
	uint64_t m_gpuResultFrame = 0;
	uint64_t m_gpuResultUs = 0;
	void CreateTimestampPool();
	void ReadSlotGpuTime( uint32_t slot );
	void WriteFrameStats( uint64_t endUs );

	bool CreateInstance( std::string *outError );
	bool SetupDebugMessenger( std::string *outError );
	bool CreateSurface( std::string *outError );
	bool PickPhysicalDevice( std::string *outError );
	bool CreateLogicalDevice( std::string *outError );
	// `oldSwapchain` hands the presentation over (VkSwapchainCreateInfoKHR), so a
	// rebuild never leaves the window without a presentable image.
	bool CreateSwapchain( std::string *outError, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE );
	bool CreateRenderPass( std::string *outError );
	bool CreateFramebuffers( std::string *outError );
	bool CreateCommandResources( std::string *outError );
	bool CreateSyncObjects( std::string *outError );
	bool CreateCaptureImage( VkExtent2D extent, std::string *outError );

	void DestroySwapchainObjects();
	bool RecreateSwapchain( std::string *outError );
	// Whether the surface's extent or transform differs from the ones the
	// swapchain was built against; a SUBOPTIMAL result rebuilds only then.
	bool SurfaceChangedSinceSwapchain();
	// Keep m_surface bound to the window's current native surface, replacing it
	// (and the swapchain) when the platform swapped or lost it. `outReady` is
	// false while the platform has no surface at all (a backgrounded activity).
	bool EnsureSurfaceCurrent( bool *outReady, std::string *outError );

	bool RecordCapture( VkCommandBuffer cmd, uint32_t imageIndex );
	// Scale the back buffer (resting in `backBufferLayout`) into the acquired
	// swapchain image and leave that image ready to present.
	void RecordPresentBlit(
	    VkCommandBuffer cmd, uint32_t imageIndex, VkImageLayout backBufferLayout, bool capture );
	bool ResolveCapturedPixels( std::string *outError );

	uint32_t FindMemoryType( uint32_t typeBits, VkMemoryPropertyFlags props, bool *found ) const;
	bool CreateShaderModule(
	    const uint32_t *code, size_t sizeBytes, VkShaderModule *outModule, std::string *outError );
	bool CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
	    VkBuffer *outBuffer, VkDeviceMemory *outMemory, std::string *outError );
	void DestroyDemoTriangle();

	bool BeginSingleTimeCommands( VkCommandBuffer *outCmd, std::string *outError );
	bool EndSingleTimeCommands( VkCommandBuffer cmd, std::string *outError );
	void DestroyTexturedQuad();
	void DestroyIndexedUbo();
	void DestroyDemoDepth();
	void DestroyDynamicMesh();
	bool CreateDepthResources( std::string *outError );

	IVulkanSurfaceHost *m_host = nullptr;
	VulkanContextConfig m_config;

	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	uint32_t m_graphicsQueueFamily = UINT32_MAX;
	uint32_t m_presentQueueFamily = UINT32_MAX;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkFormat m_swapFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR m_swapColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
	// m_swapImages are the back buffers the engine renders into, one per
	// swapchain image, at m_swapExtent (the video mode's size). The swapchain's
	// own images (m_presentImages, at the drawable's m_presentExtent) only
	// receive the scaled back buffer at present.
	VkExtent2D m_swapExtent = { 0, 0 };
	VkExtent2D m_presentExtent = { 0, 0 };
	// The window's drawable size when the swapchain was built; a different size
	// at the next frame rebuilds it (Wayland reports no OUT_OF_DATE on resize).
	int m_presentDrawable[2] = { 0, 0 };
	// The surface's reported extent and transform when the swapchain was built.
	// With an identity pre-transform on a rotated Android display every present
	// is SUBOPTIMAL; only a change of these rebuilds, not every such frame.
	VkExtent2D m_swapSurfaceExtent = { 0, 0 };
	VkSurfaceTransformFlagBitsKHR m_swapSurfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	// The host's native-surface generation m_surface was created from. Android
	// destroys and recreates the native window (backgrounding, some display
	// swaps); a surface of the old one can no longer present.
	uint64_t m_surfaceGeneration = 0;
	bool m_surfaceLost = false;
	VkExtent2D m_requestedBackBuffer = { 0, 0 };
	VkFilter m_presentFilter = VK_FILTER_LINEAR;
	bool m_capturePresented = false;
	uint64_t m_presentCount = 0;
	uint64_t m_scaledPresentCount = 0;
	bool m_presentCapturable = false;
	std::vector<VkImage> m_presentImages;
	std::vector<VkDeviceMemory> m_backBufferMemories;
	std::vector<VkImage> m_swapImages;
	std::vector<VkImageView> m_swapImageViews;
	std::vector<VkFramebuffer> m_framebuffers;

	// Per-swapchain-image depth attachment, so depth-tested 3D geometry (real
	// scene rendering) has an occlusion buffer. Recreated with the swapchain.
	// The depth-stencil format, chosen at device pick: D24S8 as D3D9 asks for,
	// else D32S8, else depth only (no stencil bits).
	VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;
	VkImageAspectFlags m_depthAspects = VK_IMAGE_ASPECT_DEPTH_BIT;
	int m_stencilBits = 0;
	bool m_clipPlanesSupported = false;
	bool m_blockCompression = false;
	std::vector<VkImage> m_depthImages;
	std::vector<VkDeviceMemory> m_depthMemories;
	std::vector<VkImageView> m_depthViews;

	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	// Render-pass-compatible siblings of m_renderPass (same formats, samples and
	// dependencies), so every pipeline built against m_renderPass draws in them:
	// re-entering the swapchain image after a render-target pass (load, not
	// clear), and drawing into a render-target texture (sampled-layout in/out).
	VkRenderPass m_renderPassLoad = VK_NULL_HANDLE;
	VkRenderPass m_renderPassTarget = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_commandBuffers;

	uint32_t m_framesInFlight = 2;
	uint32_t m_currentFrame = 0;
	std::vector<VkSemaphore> m_imageAvailable;
	std::vector<VkSemaphore> m_renderFinished;
	std::vector<VkFence> m_inFlight;
	// Maps each swapchain image to the in-flight fence currently using it, so a
	// freshly acquired image is not recorded into while a prior submission that
	// targeted it is still executing.
	std::vector<VkFence> m_imagesInFlight;

	// Host-visible linear image used to copy the rendered color image out for
	// verification. Allocated lazily on the first capture request.
	VkImage m_captureImage = VK_NULL_HANDLE;
	VkDeviceMemory m_captureMemory = VK_NULL_HANDLE;
	VkExtent2D m_captureExtent = { 0, 0 };
	bool m_captureRequested = false;
	bool m_capturePending = false;
	std::vector<uint8_t> m_capturedPixels;
	int m_capturedWidth = 0;
	int m_capturedHeight = 0;

	// Bounded demo pipeline (R32 draw-path proof), optional.
	VkPipelineLayout m_demoPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_demoPipeline = VK_NULL_HANDLE;
	VkBuffer m_demoVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_demoVertexMemory = VK_NULL_HANDLE;
	uint32_t m_demoVertexCount = 0;
	bool m_drawDemoTriangle = false;

	// Bounded textured-quad pipeline (R32 texture-path proof), optional.
	VkImage m_texImage = VK_NULL_HANDLE;
	VkDeviceMemory m_texMemory = VK_NULL_HANDLE;
	VkImageView m_texView = VK_NULL_HANDLE;
	VkSampler m_texSampler = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_texDescLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_texDescPool = VK_NULL_HANDLE;
	VkDescriptorSet m_texDescSet = VK_NULL_HANDLE;
	VkPipelineLayout m_texQuadPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_texQuadPipeline = VK_NULL_HANDLE;
	VkBuffer m_texQuadVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_texQuadVertexMemory = VK_NULL_HANDLE;
	uint32_t m_texQuadVertexCount = 0;
	bool m_drawTexturedQuad = false;

	// Bounded indexed + uniform-buffer pipeline (R32 index/constant proof).
	VkBuffer m_iuVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_iuVertexMemory = VK_NULL_HANDLE;
	VkBuffer m_iuIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_iuIndexMemory = VK_NULL_HANDLE;
	VkBuffer m_iuUniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_iuUniformMemory = VK_NULL_HANDLE;
	void *m_iuUniformMapped = nullptr;
	VkDescriptorSetLayout m_iuDescLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_iuDescPool = VK_NULL_HANDLE;
	VkDescriptorSet m_iuDescSet = VK_NULL_HANDLE;
	VkPipelineLayout m_iuPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_indexedUboPipeline = VK_NULL_HANDLE;
	uint32_t m_iuIndexCount = 0;
	float m_iuColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	bool m_drawIndexedUbo = false;

	// Bounded depth-test demo pipeline (R32 depth/occlusion proof).
	VkBuffer m_depthDemoVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_depthDemoVertexMemory = VK_NULL_HANDLE;
	VkPipelineLayout m_demoDepthPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_demoDepthPipeline = VK_NULL_HANDLE;
	uint32_t m_depthDemoVertexCount = 0;
	bool m_drawDemoDepth = false;

	// Dynamic geometry path (material-system mesh draws).
	VkPipelineLayout m_dynPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_dynPipeline = VK_NULL_HANDLE;      // vertex-color passthrough
	VkPipeline m_dynPipelineGreen = VK_NULL_HANDLE; // "greenify" material shader
	VkPipeline m_dynPipelineConst = VK_NULL_HANDLE; // "constant color" material shader
	int m_dynShaderIndex = 0;
	float m_dynConstColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	// UnlitGeneric material state: cModulationColor ($color * $alpha) and the two
	// rows of cBaseTextureTransform. Defaults leave the material unmodulated and
	// the UV coordinate untransformed.
	float m_dynModulation[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float m_dynMonitorContrast = 0.0f;
	float m_dynTexXform0[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float m_dynTexXform1[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	float m_dynPbrAngles[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	DynRasterState m_dynRaster;
	float m_dynAlphaRef = -1.0f; // $alphatest reference; < 0 disables
	// "$basetexture" material pipeline: a built-in 2-tone texture sampled at the
	// mesh UVs, bound through a descriptor set (its own layout adds the sampler).
	// One textured pipeline per distinct DynRasterState (see RasterStateKey),
	// built from m_texTemplate when a draw first needs it.
	// Keyed by RasterStateKey, with bit 32 set for pipelines of the sRGB passes.
	std::map<uint64_t, VkPipeline> m_dynTexPipelines;
	VkPipeline TexturedPipeline( const DynRasterState &state, bool srgbPass = false );
	// The pipeline store (OpenPipelineStore): the cache every material pipeline
	// is built through, the variants built this session, and the files.
	enum PipelineFamily
	{
		kPipelineTextured = 0,
		kPipelinePortal = 1,
		kPipelineSkin = 2,
		kPipelinePbrDirect = 3,
		kPipelineFamilies
	};
	VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
	std::vector<std::pair<int, uint64_t>> m_pipelineVariants;
	std::string m_pipelineStoreDirectory;
	void NotePipelineVariant( int family, uint64_t key )
	{
		m_pipelineVariants.push_back( std::make_pair( family, key ) );
	}

	// PortalRefract pipelines, one per raster state, built on first use.
	std::map<uint64_t, VkPipeline> m_portalPipelines;
	VkPipeline PortalPipeline( const DynRasterState &state, bool srgbPass = false );
	VkPipelineLayout m_portalPipelineLayout = VK_NULL_HANDLE;
	// The skin shader: six sampler sets (s0, s1, s2, s3, s7, s14) and its pixel
	// shader constants in a uniform buffer per frame in flight, written only
	// after that frame's fence has signaled, bound at a per-draw dynamic offset.
	std::map<uint64_t, VkPipeline> m_skinPipelines;
	VkPipeline SkinPipeline( const DynRasterState &state, bool srgbPass = false );
	std::map<uint64_t, VkPipeline> m_pbrDirectPipelines;
	VkPipeline PbrDirectPipeline( const DynRasterState &state, bool srgbPass = false );
	VkPipelineLayout m_pbrDirectPipelineLayout = VK_NULL_HANDLE;
	VkShaderModule m_pbrDirectFrag = VK_NULL_HANDLE;
	int m_pbrSplitSumHandle = -1;
	bool m_pbrDirectReady = false;
	bool InitPbrDirectPipeline( std::string *outError );
	void DestroyPbrDirectPipeline();
	VkPipelineLayout m_skinPipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_skinUboLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_skinUboPool = VK_NULL_HANDLE;
	VkShaderModule m_skinVert = VK_NULL_HANDLE;
	VkShaderModule m_skinFrag = VK_NULL_HANDLE;
	VkVertexInputAttributeDescription m_skinAttrs[7] = {};
	VkPipelineVertexInputStateCreateInfo m_skinVin = {};
	struct SkinUniformBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		void *mapped = nullptr;
		VkDeviceSize capacity = 0;
		VkDescriptorSet set = VK_NULL_HANDLE;
	};
	std::vector<SkinUniformBuffer> m_skinUbos; // one per frame in flight
	VkDeviceSize m_uboAlignment = 256;
	bool InitSkinPipeline( std::string *outError );
	// Copies this frame's skin constants into the frame's uniform buffer (grown
	// as needed) and returns each draw's offset in `offsets`; false when the
	// buffer cannot be provided.
	bool UploadSkinConstants( std::vector<uint32_t> *offsets );
	void DestroySkinPipeline();
	VkShaderModule m_portalVert = VK_NULL_HANDLE;
	VkShaderModule m_portalFrag = VK_NULL_HANDLE;
	bool m_portalPushSupported = false;
	bool InitPortalPipeline( std::string *outError );
	VkPipeline BuildMaterialPipeline( const DynRasterState &state, VkShaderModule vert,
	    VkShaderModule frag, VkPipelineLayout layout,
	    const VkPipelineVertexInputStateCreateInfo *vertexInput, VkRenderPass renderPass );
	// D3D9 blends a draw that writes sRGB (SRGBWRITEENABLE) in linear space: the
	// destination is decoded, blended and encoded again. Such draws render
	// through sRGB-format views of the same attachments (mutable-format swapchain
	// images and render targets), in these render passes, so the hardware does
	// exactly that. Without VK_KHR_swapchain_mutable_format they blend encoded
	// values, and the census reports it.
	bool m_srgbAttachments = false;
	VkFormat m_swapFormatSrgb = VK_FORMAT_UNDEFINED;
	std::vector<VkImageView> m_swapImageViewsSrgb;
	std::vector<VkFramebuffer> m_framebuffersSrgb;
	VkRenderPass m_renderPassLoadSrgb = VK_NULL_HANDLE;
	VkRenderPass m_renderPassTargetSrgb = VK_NULL_HANDLE;
	// PortalRefract's vertex input: the textured one plus normal and tangent.
	VkVertexInputAttributeDescription m_portalAttrs[6] = {};
	VkPipelineVertexInputStateCreateInfo m_portalVin = {};
	// The fixed-function state every textured pipeline shares; only the blend and
	// depth state vary. Kept (with its shader modules) for pipelines built later.
	struct TexturedPipelineTemplate
	{
		VkPipelineShaderStageCreateInfo stages[2];
		VkVertexInputBindingDescription binding;
		VkVertexInputAttributeDescription attrs[5];
		VkPipelineVertexInputStateCreateInfo vin;
		VkPipelineInputAssemblyStateCreateInfo ia;
		VkPipelineViewportStateCreateInfo vp;
		VkPipelineRasterizationStateCreateInfo rs;
		VkPipelineMultisampleStateCreateInfo ms;
		VkDynamicState dynStates[5];
		VkPipelineDynamicStateCreateInfo dyn;
	};
	TexturedPipelineTemplate m_texTemplate = {};
	VkPipelineLayout m_dynTexPipelineLayout = VK_NULL_HANDLE;
	VkImage m_dynTexImage = VK_NULL_HANDLE;
	VkDeviceMemory m_dynTexMemory = VK_NULL_HANDLE;
	VkImageView m_dynTexView = VK_NULL_HANDLE;
	VkSampler m_dynTexSampler = VK_NULL_HANDLE;
	// Sampler per addressing/filter combination (kSamplerClampU | kSamplerClampV |
	// kSamplerLinear). D3D9 sampler state is per texture here, as Source sets it
	// when each texture is created.
	VkSampler m_samplers[kSamplerStates] = {};
	VkDescriptorSetLayout m_dynTexDescLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_dynTexDescPool = VK_NULL_HANDLE;
	VkDescriptorSet m_dynTexDescSet = VK_NULL_HANDLE;
	// Material-supplied textures (IShaderAPI). The descriptor set points at the
	// bound one, or the built-in 2-tone texture when none is bound.
	enum
	{
		kMaxManagedTexSets = 16384
	};
	struct ManagedTexture
	{
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		// Per-texture descriptor set, so each draw can bind its own texture.
		VkDescriptorSet descSet = VK_NULL_HANDLE;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mipLevels = 1;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		// False until pixel data has actually been uploaded. Sampling an image
		// that was created but never filled yields undefined contents.
		bool uploaded = false;
		// Render-target textures own a depth buffer and a framebuffer for
		// m_renderPassTarget; their color image rests in SHADER_READ_ONLY.
		bool renderTarget = false;
		// Sampler state (kSampler* bits); 0 is D3D9's default: wrap, point.
		int samplerState = 0;
		VkImage depthImage = VK_NULL_HANDLE;
		VkDeviceMemory depthMemory = VK_NULL_HANDLE;
		VkImageView depthView = VK_NULL_HANDLE;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		// sRGB view of an 8-bit or BC image: a render target's sRGB attachment
		// (framebufferSrgb, m_renderPassTargetSrgb), and for sampling the set that
		// decodes each texel before filtering, as D3D9's SRGBTEXTURE does.
		VkImageView srgbView = VK_NULL_HANDLE;
		VkFramebuffer framebufferSrgb = VK_NULL_HANDLE;
		VkDescriptorSet descSetSrgb = VK_NULL_HANDLE;
	};
	std::vector<ManagedTexture> m_managedTextures;
	// Deleted textures awaiting the completion of the submission that may still
	// use them (`afterSerial`, a value of m_submitSerial), and handles free again.
	struct RetiredTexture
	{
		ManagedTexture texture;
		int handle;
		uint64_t afterSerial;
	};
	std::vector<RetiredTexture> m_retiredTextures;
	std::vector<int> m_freeTextureHandles;
	// Frame submissions: the count so far, the serial each frame slot last
	// submitted, and the newest serial known complete.
	uint64_t m_submitSerial = 0;
	uint64_t m_slotSerial[kMaxFramesInFlight] = {};
	uint64_t m_completedSerial = 0;
	uint32_t m_liveTextureSets = 0;
	void ReleaseManagedTextureObjects( ManagedTexture &t );
	void RetireCompletedTextures();
	// The managed texture currently bound (BindManagedTexture); captured per draw.
	int m_dynBoundTexHandle = -1;
	int m_dynLightmapHandle = -1;
	int m_dynColorFlags = 0;
	float m_dynOutputScale = 1.0f;
	uint32_t m_dynStencilRef = 0;
	uint32_t m_dynStencilTestMask = 0xFF;
	uint32_t m_dynStencilWriteMask = 0xFF;
	int m_dynClipPlaneCount = 0;
	float m_dynClipPlanes[kMaxClipPlanes][4] = {};
	int m_dynSamplerHandles[kMaxSamplers] = {
	    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	PortalConstants m_dynPortal;
	SkinConstants m_dynSkin;
	// The skin constants of this frame's skin draws (DynDraw::skin indexes them).
	std::vector<SkinConstants> m_dynSkinConstants;
	// Column-major model->projection matrix; identity by default.
	float m_dynTransform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	// A persistently mapped host-visible buffer the frame's stream is copied
	// into. Each frame slot has its own: a slot's fence has signalled when its
	// frame begins, so its buffers are free to rewrite or regrow, while the
	// previous frame may still be reading the other slot's.
	struct StreamBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		void *mapped = nullptr;
		VkDeviceSize capacity = 0;
	};
	StreamBuffer m_dynVertexStreams[kMaxFramesInFlight];
	StreamBuffer m_dynIndexStreams[kMaxFramesInFlight];
	// Texel uploads small enough to defer (a font glyph, a lightmap patch): the
	// texels are copied here and the copy is recorded at the start of the next
	// frame's command buffer, ahead of every draw that frame replays, instead of
	// each upload submitting a copy and waiting for the queue to go idle. All of
	// a frame's draws replay at present, so they see the same texels either way.
	// Larger uploads (level load) stay synchronous, after the pending ones.
	struct PendingUpload
	{
		int handle;
		uint32_t x, y, width, height, level;
		// The image's contents outside the region survive (a mip chain level, a
		// part of an uploaded image); a never-filled image's rest is cleared.
		bool preserve;
		bool clearRest;
		size_t offset;
		size_t size;
	};
	enum : size_t
	{
		// An upload up to this size is deferred; a bigger one runs at once.
		kDeferredUploadMaxBytes = 256 * 1024,
		// Texels held back at most; beyond this the pending ones run at once.
		kPendingUploadCapBytes = 8 * 1024 * 1024
	};
	std::vector<PendingUpload> m_pendingUploads;
	std::vector<uint8_t> m_pendingUploadData;
	StreamBuffer m_uploadStreams[kMaxFramesInFlight];
	void RecordTextureUpload( VkCommandBuffer cmd, VkImage image, const PendingUpload &upload,
	    VkBuffer staging, VkDeviceSize offset );
	// Records every pending upload into `cmd`, staging from `stream`.
	bool RecordPendingUploads( VkCommandBuffer cmd, StreamBuffer &stream );
	// Runs the pending uploads now, in their own submission (before a
	// synchronous upload, so the order of uploads is kept).
	bool FlushPendingUploads( std::string *outError );
	bool EnsureStreamBuffer( StreamBuffer &stream, VkDeviceSize bytes, VkBufferUsageFlags usage );
	void DestroyStreamBuffer( StreamBuffer &stream );
	// kDynVertexFloats per vertex (QueueDynamicTriangles documents the record).
	std::vector<float, DefaultInitAllocator<float>> m_dynQueued;
	// Index lists of indexed draws, each relative to its draw's first vertex.
	std::vector<uint32_t, DefaultInitAllocator<uint32_t>> m_dynIndices;
	uint64_t m_streamEpoch = 1;
	// Each IMesh::Draw becomes one record capturing the state current at that
	// draw (transform, shader, constant color), so the many objects the engine
	// draws in a frame each render with their own state instead of all collapsing
	// to the last-set state.
	enum
	{
		kRecordDraw = 0,
		kRecordClear = 1,
		kRecordCopy = 2,
		kRecordQueryBegin = 3,
		kRecordQueryEnd = 4
	};
	struct DynDraw
	{
		// Draws, clears and render-target copies share one ordered stream, since
		// their relative order across render targets is what the frame means.
		int kind = kRecordDraw;
		int target = -1; // render-target texture handle; -1 = swapchain image
		int tag = -1;    // caller-defined identity (diagnostics only)
		// x, y, width, height, minZ, maxZ; width <= 0 means the whole target.
		float viewport[6] = { 0, 0, 0, 0, 0, 1 };
		bool scissorEnabled = false;
		int scissor[4] = { 0, 0, 0, 0 }; // x, y, width, height
		bool clearColor = false;
		bool clearDepth = false;
		float clearValue[4] = { 0, 0, 0, 1 };
		int copyDst = -1;
		// Occlusion query slot of a begin/end record, and which issue of that
		// query the begin record is.
		int query = -1;
		uint64_t querySerial = 0;
		int copySrcRect[4] = { 0, 0, 0, 0 }; // width <= 0 means the whole image
		int copyDstRect[4] = { 0, 0, 0, 0 };
		uint32_t firstVertex = 0;
		uint32_t vertexCount = 0;
		// Indexed draws only (indexCount > 0): the range of m_dynIndices drawn.
		uint32_t firstIndex = 0;
		uint32_t indexCount = 0;
		int shaderIndex = 0;
		float transform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		float color[4] = { 1, 1, 1, 1 };
		// UnlitGeneric material state captured at this draw (cModulationColor and
		// the two rows of cBaseTextureTransform).
		float modulation[4] = { 1, 1, 1, 1 };
		float texXform0[4] = { 1, 0, 0, 0 };
		float texXform1[4] = { 0, 1, 0, 0 };
		float pbrAngles[4] = { 1, 1, 1, 1 };
		float monitorContrast = 0.0f;
		DynRasterState raster;
		float alphaRef = -1.0f;   // $alphatest reference; < 0 disables
		int texHandle = -1;       // managed texture bound at this draw (-1 = built-in)
		int lightmapHandle = -1;  // lightmap page multiplied in (-1 = none)
		int colorFlags = 0;       // kColorSrgb* inputs/output encoding
		float outputScale = 1.0f; // linear scale before the output encode
		bool clearStencil = false;
		uint32_t stencilRef = 0;
		uint32_t stencilTestMask = 0xFF;
		uint32_t stencilWriteMask = 0xFF;
		int clipPlaneCount = 0;
		float clipPlanes[kMaxClipPlanes][4] = {};
		int samplerHandles[kMaxSamplers] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1 }; // samplers 1..15 ([0] unused)
		PortalConstants portal;
		int skin = -1; // index into m_dynSkinConstants
	};
	// A draw record carrying the state current now, before its geometry.
	DynDraw &AppendDrawRecord();
	std::vector<DynDraw> m_dynDrawRecords;
	// Target/viewport/scissor state captured by each record.
	int m_dynTarget = -1;
	int m_dynTag = -1;
	float m_dynViewport[6] = { 0, 0, 0, 0, 0, 1 };
	bool m_dynScissorEnabled = false;
	int m_dynScissor[4] = { 0, 0, 0, 0 };
	// Set by EndStreamFrame; the next record discards the presented frame.
	bool m_dynFramePresented = false;
	DynDraw &AppendRecord( int kind );

	// Occlusion queries: one pool slot per query object. `issued` counts begins
	// recorded; `submitted` is the issue whose frame was last submitted, so a
	// result is readable only when the two agree.
	struct OcclusionQuerySlot
	{
		bool live = false;
		uint64_t issued = 0;
		uint64_t submitted = 0;
		bool failed = false; // the latest issue cannot produce a result
	};
	VkQueryPool m_queryPool = VK_NULL_HANDLE;
	std::vector<OcclusionQuerySlot> m_querySlots;
	bool m_preciseOcclusion = false;
	// Issues replayed into the frame being recorded; marked submitted by EndFrame.
	std::vector<std::pair<int, uint64_t>> m_replayedQueries;
	void FailUnsubmittedQueries();
	void GetTargetExtent( int target, uint32_t *outW, uint32_t *outH ) const;
	void BeginTargetPass( VkCommandBuffer cmd, int target, bool srgb = false );
	void RecordTargetCopy( VkCommandBuffer cmd, int srcTarget, const DynDraw &copy );

	// Per-frame acquisition state, valid between BeginFrame and EndFrame.
	uint32_t m_acquiredImage = 0;
	bool m_frameOpen = false;

	VkClearColorValue m_clearColor = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	// Selected adapter facts.
	std::string m_deviceName = "unknown";
	uint32_t m_vendorId = 0;
	uint32_t m_deviceId = 0;
	uint64_t m_deviceLocalMemoryBytes = 0;
	bool m_isDiscrete = false;
	bool m_validationEnabled = false;
	uint32_t m_validationErrorCount = 0;

	// Debug-messenger entry points, resolved from the instance when validation
	// is enabled.
	PFN_vkCreateDebugUtilsMessengerEXT m_pfnCreateDebugMessenger = nullptr;
	PFN_vkDestroyDebugUtilsMessengerEXT m_pfnDestroyDebugMessenger = nullptr;

	friend VkBool32 VulkanDebugCallbackTrampoline( VkDebugUtilsMessageSeverityFlagBitsEXT,
	    VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT *, void * );
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_DEVICE_H
