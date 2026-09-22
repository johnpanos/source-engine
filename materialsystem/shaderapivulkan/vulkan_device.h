//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan device bring-up and presentation core for the
//          shaderapivulkan backend (RFC 0001 rank 14/16, roadmap R28/R32).
//
//          This module owns the genuine native Vulkan objects: instance,
//          physical-device selection, logical device and queues, the SDL3
//          window surface, the swapchain and its render targets, per-frame
//          command buffers and synchronization, and clean teardown. It clears
//          and presents a real frame and can read the presented image back for
//          outcome-driven verification.
//
//          It deliberately depends only on <vulkan/vulkan.h>, the SDL3 Vulkan
//          surface entry points, and the C++ standard library so the same core
//          is exercised by both the engine backend and the standalone native
//          smoke test, with no ambient engine state.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_DEVICE_H
#define SHADERAPIVULKAN_VULKAN_DEVICE_H

#ifdef _WIN32
#pragma once
#endif

#include <vulkan/vulkan.h>

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

struct SDL_Window;

namespace render_vulkan
{

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

// A single, coherent native Vulkan presentation context bound to one SDL3
// window. Not copyable; owns all its Vulkan handles and destroys them in
// reverse dependency order at Shutdown()/destruction.
class CVulkanContext
{
public:
	CVulkanContext() = default;
	~CVulkanContext();

	CVulkanContext( const CVulkanContext & ) = delete;
	CVulkanContext &operator=( const CVulkanContext & ) = delete;

	// Bring up every object needed to present to `window`. On failure returns
	// false, fills *outError with a specific reason, and leaves the context
	// fully torn down (safe to destroy, never partially live).
	bool Init( SDL_Window *window, const VulkanContextConfig &config, std::string *outError );

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

	// Ask EndFrame() to capture the rendered color image this frame.
	void RequestCapture() { m_captureRequested = true; }

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
		kDynVertexFloats = 10
	};
	void QueueDynamicTriangles( const float *posColorUvInterleaved, uint32_t vertexCount,
	    const float *lightmapUv = nullptr );
	// Discard the accumulated frame geometry. Called at frame start (ClearBuffers)
	// rather than after Present, so the last frame's geometry stays available for
	// an on-demand screenshot capture (ReadPixels).
	void ClearDynamicQueue()
	{
		FailUnsubmittedQueries();
		m_dynQueued.clear();
		m_dynDrawRecords.clear();
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
		kDynShaderTextured = 3
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
	};
	void SelectDynamicRasterState( const DynRasterState &state ) { m_dynRaster = state; }
	// Distinct states have distinct keys.
	static uint32_t RasterStateKey( const DynRasterState &state );
	// $alphatest: fragments whose alpha is below `ref` are discarded (matching the
	// D3D9 fixed-function GREATEREQUAL alpha test, or GREATER with
	// kFragmentAlphaGreater). A negative `ref` disables it.
	void SelectDynamicAlphaTest( float ref ) { m_dynAlphaRef = ref; }
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
	int CreateManagedTexture( int width, int height, VkFormat format, std::string *outError,
	    VkImageUsageFlags extraUsage = 0 );
	bool UploadManagedTexture(
	    int handle, const uint8_t *data, size_t dataSize, std::string *outError );
	void BindManagedTexture( int handle );
	// The lightmap page the textured pipeline multiplies by (LightmappedGeneric's
	// TEXTURE_LIGHTMAP on sampler 1), sampled at the lightmap coordinates; -1
	// draws without a lightmap.
	void BindManagedLightmap( int handle ) { m_dynLightmapHandle = handle; }
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
		kVertexScreenSpace = 32
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
		kSamplerLinear = 4
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
	void QueueClear( bool color, bool depth );
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

	void GetSwapchainExtent( int &width, int &height ) const
	{
		width = static_cast<int>( m_swapExtent.width );
		height = static_cast<int>( m_swapExtent.height );
	}

	// Count of validation messages of severity WARNING or ERROR observed since
	// Init(). Zero on a clean run when validation is enabled.
	uint32_t ValidationErrorCount() const { return m_validationErrorCount; }

	enum
	{
		kMaxFramesInFlight = 3
	};

private:
	bool CreateInstance( std::string *outError );
	bool SetupDebugMessenger( std::string *outError );
	bool CreateSurface( std::string *outError );
	bool PickPhysicalDevice( std::string *outError );
	bool CreateLogicalDevice( std::string *outError );
	bool CreateSwapchain( std::string *outError );
	bool CreateRenderPass( std::string *outError );
	bool CreateFramebuffers( std::string *outError );
	bool CreateCommandResources( std::string *outError );
	bool CreateSyncObjects( std::string *outError );
	bool CreateCaptureImage( std::string *outError );

	void DestroySwapchainObjects();
	bool RecreateSwapchain( std::string *outError );

	bool RecordCapture( VkCommandBuffer cmd, uint32_t imageIndex );
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

	SDL_Window *m_window = nullptr;
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
	VkExtent2D m_swapExtent = { 0, 0 };
	std::vector<VkImage> m_swapImages;
	std::vector<VkImageView> m_swapImageViews;
	std::vector<VkFramebuffer> m_framebuffers;

	// Per-swapchain-image depth attachment, so depth-tested 3D geometry (real
	// scene rendering) has an occlusion buffer. Recreated with the swapchain.
	VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;
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
	float m_dynTexXform0[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float m_dynTexXform1[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	DynRasterState m_dynRaster;
	float m_dynAlphaRef = -1.0f; // $alphatest reference; < 0 disables
	// "$basetexture" material pipeline: a built-in 2-tone texture sampled at the
	// mesh UVs, bound through a descriptor set (its own layout adds the sampler).
	// One textured pipeline per distinct DynRasterState (see RasterStateKey),
	// built from m_texTemplate when a draw first needs it.
	std::map<uint32_t, VkPipeline> m_dynTexPipelines;
	VkPipeline TexturedPipeline( const DynRasterState &state );
	// The fixed-function state every textured pipeline shares; only the blend and
	// depth state vary. Kept (with its shader modules) for pipelines built later.
	struct TexturedPipelineTemplate
	{
		VkPipelineShaderStageCreateInfo stages[2];
		VkVertexInputBindingDescription binding;
		VkVertexInputAttributeDescription attrs[4];
		VkPipelineVertexInputStateCreateInfo vin;
		VkPipelineInputAssemblyStateCreateInfo ia;
		VkPipelineViewportStateCreateInfo vp;
		VkPipelineRasterizationStateCreateInfo rs;
		VkPipelineMultisampleStateCreateInfo ms;
		VkDynamicState dynStates[2];
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
	VkSampler m_samplers[8] = {};
	VkDescriptorSetLayout m_dynTexDescLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_dynTexDescPool = VK_NULL_HANDLE;
	VkDescriptorSet m_dynTexDescSet = VK_NULL_HANDLE;
	// Material-supplied textures (IShaderAPI). The descriptor set points at the
	// bound one, or the built-in 2-tone texture when none is bound.
	enum
	{
		kMaxManagedTexSets = 8192
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
	};
	std::vector<ManagedTexture> m_managedTextures;
	// The managed texture currently bound (BindManagedTexture); captured per draw.
	int m_dynBoundTexHandle = -1;
	int m_dynLightmapHandle = -1;
	int m_dynColorFlags = 0;
	float m_dynOutputScale = 1.0f;
	// Column-major model->projection matrix; identity by default.
	float m_dynTransform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	VkBuffer m_dynVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_dynVertexMemory = VK_NULL_HANDLE;
	void *m_dynMapped = nullptr;
	VkDeviceSize m_dynCapacityBytes = 0;
	std::vector<float> m_dynQueued; // interleaved [x,y,z,r,g,b,u,v] per vertex
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
		int shaderIndex = 0;
		float transform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		float color[4] = { 1, 1, 1, 1 };
		// UnlitGeneric material state captured at this draw (cModulationColor and
		// the two rows of cBaseTextureTransform).
		float modulation[4] = { 1, 1, 1, 1 };
		float texXform0[4] = { 1, 0, 0, 0 };
		float texXform1[4] = { 0, 1, 0, 0 };
		DynRasterState raster;
		float alphaRef = -1.0f; // $alphatest reference; < 0 disables
		int texHandle = -1;     // managed texture bound at this draw (-1 = built-in)
		int lightmapHandle = -1; // lightmap page multiplied in (-1 = none)
		int colorFlags = 0;      // kColorSrgb* inputs/output encoding
		float outputScale = 1.0f; // linear scale before the output encode
	};
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
	void BeginTargetPass( VkCommandBuffer cmd, int target );
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
