//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan device bring-up and presentation core for the
//          shaderapivulkan backend (RFC 0001 rank 14/16, roadmap R28/R32).
//
//          The instance, physical-device choice, logical device, queues, memory
//          and completion belong to render.device.vulkan (RFC 0016 K1): this
//          context borrows them through the adapter's host interop
//          (render/device/vulkan/host_device.h). Every buffer and image it uses
//          is allocated there, each submission signals the adapter's timeline,
//          and long-lived resources are released behind its values. This module
//          owns the window surface, the swapchain and its render targets,
//          per-frame command buffers and synchronization, and clean teardown. It clears
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

#include "render/render_gamma_ramp.h"
#include "vulkan_adapter.h"
#include "vulkan_compute.h"
#include "vulkan_debug_utils.h"
#include "vulkan_frame_stats.h"
#include "vulkan_shader_library.h"
#include "vulkan_surface_host.h"
#include "../../render/device/vulkan/host_device.h"
#include "render/device/completion.h"
#include "render/device/device.h"
#include "render/device/encoder.h"
#include "render/legacy/core_passes.h"

#include <vulkan/vulkan.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
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

// A suballocation of render.device.vulkan's allocator.
using VulkanMemory = render::device::vulkan::HostAllocation;

// Requested behavior for a device bring-up. Required behavior that cannot be
// satisfied fails Init() with a diagnostic instead of silently degrading.
struct VulkanContextConfig
{
	// The owner of the device this context borrows (the composition root
	// binds it: NativeVulkanShaderBackend_BindDeviceOwner; RFC 0016 legacy
	// device facade F2). Init adopts the owner's device when the root created
	// it, else has the owner create it from this context's request; Init
	// fails without an owner. The owner, never the context, destroys it.
	render::device::vulkan::IHostDeviceOwner *deviceOwner = nullptr;
	// Reports an error the frame cannot continue after (the shader API's
	// Error). Without one the context logs the message and aborts.
	void ( *fatalError )( const char *message ) = nullptr;
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
	// Wait for vertical blank when presenting. The present mode for either
	// setting is chosen by render.present-policy.v1; RequestVSync changes it.
	bool vsync = true;
	// Game composition keeps scene and render targets linear until render.pass.output.
	bool hdrScene = false;
	// Behave as if the device bound at most this many descriptor sets (0: the
	// device's own maxBoundDescriptorSets). The suites set 4, the Vulkan
	// minimum, to show what such a device draws.
	uint32_t descriptorSetLimit = 0;
	// Object names and command labels for capture tools (vulkan_debug_utils.h).
	DebugLabelPolicy debugLabels = DebugLabelPolicy::Auto;
	// A directory of debug shader variants (regen_material_spv.py --debug-out)
	// used in place of the embedded code they were built from; empty: none.
	std::string shaderDebugDirectory;
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
	// What this context needs of the device, for its owner to create it
	// ahead of Init (the owner's requester). Borrows the host and keeps the
	// request's arrays alive until Init.
	bool PrepareDeviceRequest( IVulkanSurfaceHost &host, const VulkanContextConfig &config,
	    render::device::vulkan::HostDeviceRequest *request, std::string *outError );

	// Idempotent teardown. Waits for the device to go idle first.
	void Shutdown();

	bool IsValid() const { return m_device != VK_NULL_HANDLE; }
	// The live device's identity and capabilities (vulkan_adapter.h); invalid
	// before Init.
	const VulkanAdapterCaps &AdapterCaps() const { return m_adapterCaps; }
	uint32_t MaxSampledTextureDimension() const;
	int MaxAnisotropicLevel() const { return m_maxAnisotropy; }
	void SetAnisotropicLevel( int level );
	// RFC 0011 G5 compute foundation: what the device enabled (queried per
	// device, enabled through the VkPhysicalDeviceFeatures2 chain), its
	// compute resources, and compute work recorded at the start of the next
	// frame's command buffer, before any render pass, on the graphics queue.
	// `serial` is that frame's submission serial: retire what the work reads
	// behind it (Compute().Retire), collected with the managed textures.
	const ComputeCaps &ComputeCapabilities() const { return m_computeCaps; }
	ComputeResources &Compute() { return m_compute; }
	// Compute whose results the CPU reads back (render/gpu_compute.h, RFC 0011
	// G6): `flush` records it at the end of each frame's command buffer, after
	// every render pass, stamped with that frame's serial (valid once
	// CompletedFrameSerial() reaches it). Cleared with an empty function.
	void SetComputeFlush( std::function<void( VkCommandBuffer, uint64_t )> flush )
	{
		m_computeFlush = std::move( flush );
	}
	uint64_t CompletedFrameSerial() const { return m_completedSerial; }

	// A frame (RFC 0016 K3). The frame's commands are recorded into a port
	// encoder of the render core: the legacy frontend's frame graph, or
	// RenderFrame's own encoder. Nothing records into a command buffer of
	// this context any more.
	//
	// PrepareFrame acquires the next swapchain image (recreating the
	// swapchain on OUT_OF_DATE), waits for the frame slot and the image, and
	// makes a requested capture's image. Returns false with *outError on a
	// real error; returns true with *outSkip==true when the frame should be
	// skipped this iteration (e.g. a zero-size / minimized window).
	bool PrepareFrame( bool *outSkip, std::string *outError );
	// The prepared frame's stages, in recording order (the legacy frontend's
	// render::legacy::LegacyFrameStage). Resolve runs only when multisampled
	// and capture only when a back-buffer readback was requested.
	enum FrameStage
	{
		kFrameStageComputeAndUploads = 0,
		kFrameStageScene,
		kFrameStageResolve,
		kFrameStageCapture,
		kFrameStagePresent,
		kFrameStageCount
	};
	bool HasFrameStage( FrameStage stage ) const;
	// Adds one stage of the prepared frame to a port encoder of this
	// context's host device, as host work the adapter records at Submit. The
	// present stage also adds the acquire as a wait and the present semaphore
	// as a signal. The stages share one encoder, in order.
	void AttachFrameStage( FrameStage stage, render::device::CommandEncoder &encoder );
	// After the encoder's submission (or its failure): the frame's
	// bookkeeping, a requested capture's pixels, and the present. Returns
	// false with *outError on a real error.
	bool FinishFrame(
	    const render::device::CompletionToken &token, bool submitted, std::string *outError );
	// A whole frame on an encoder of its own: PrepareFrame, every stage, the
	// port's Submit, FinishFrame. For tools and tests, and for a backend no
	// frame executor was bound to.
	bool RenderFrame( bool *outSkip, std::string *outError );
	// The host device's port (render.device.v2), for frame executors.
	render::device::IRenderDevice2 *Port();

	// RFC 0016 K5: core passes at slots of the stream (core_passes.h). The
	// recorder the composition root bound; null marks no slot.
	void BindCorePassRecorder( render::legacy::ICorePassRecorder *recorder )
	{
		m_corePassRecorder = recorder;
	}
	// Appends a slot record at this point of the stream (a no-op without a
	// recorder).
	// A slot's frame terms, captured when it was marked.
	struct CorePassTerms
	{
		render::material::SurfaceDrawState drawState{};
		float clipPlanes[6][4] = {};
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
		float lightmapScale = 1.0f;
		float outputScale = 1.0f;
		float eye[3] = { 0.0f, 0.0f, 0.0f };
		float envmapScale = 1.0f;
		bool specular = true;
		bool ssbumpNormalized = false;
		render::legacy::CorePassFog fog;
		float time = 0.0f;
		float foliage[2][4] = {};
		bool foliageAvailable = false;
		float waterReflectTintScale = 1.0f;
		bool operator==( const CorePassTerms & ) const = default;
	};
	void SetOpaqueBatching( bool enabled ) { m_opaqueBatching = enabled; }
	void QueueCorePass( uint32_t tag, const CorePassTerms &terms );
	std::uint32_t QueueCoreMesh( const render::legacy::CoreMeshDraw &draw )
	{
		return m_corePassRecorder ? m_corePassRecorder->QueueMesh( draw ) : 0;
	}
	bool CoreMeshesEnabled() const
	{
		return m_corePassRecorder && m_corePassRecorder->AcceptsMeshes();
	}

	// The color the next frame's render pass clears to (linear RGBA, 0..1).
	void SetClearColor( float r, float g, float b, float a );

	// When a capture was requested via RequestCapture() before a frame, its
	// color image is copied to host-visible memory and is available after
	// FinishFrame from GetCapturedPixels().

	// Recreate the swapchain and its targets for a new drawable size. Safe to
	// call between frames. A zero-size result is retained without error and
	// causes subsequent BeginFrame calls to report a skip.
	bool Resize( int width, int height, std::string *outError );

	// Ask EndFrame() to capture the rendered color image (the back buffer) this frame.
	void RequestCapture( bool linearHdr = false )
	{
		m_captureRequested = true;
		m_captureHdrRequested = linearHdr;
		m_capturedPixels.clear();
		m_capturedHdrPixels.clear();
	}
	// Ask EndFrame() to capture the swapchain image instead: the back buffer as
	// the window presents it, scaled to the drawable. Nothing is captured when
	// the surface does not allow reading its images.
	void RequestPresentedCapture()
	{
		m_captureRequested = true;
		m_capturePresented = true;
	}

	// The frame stream's device resources (samplers, descriptor pool, depth
	// copies); mesh draws reach the core, not this device.
	bool InitDynamicMesh( std::string *outError );
	bool DynamicMeshReady() const { return m_dynamicResourcesReady; }
	// Discard the accumulated frame geometry. Called at frame start (ClearBuffers)
	// rather than after Present, so the last frame's geometry stays available for
	// an on-demand screenshot capture (ReadPixels).
	void ClearDynamicQueue()
	{
		FailUnsubmittedQueries();
		++m_streamEpoch;
		m_dynDrawRecords.clear();
		m_queuedOcclusionQuery = -1;
		m_corePassTerms.clear();
		m_queueCoreOnly = false;
		m_queueCustomEffects = false;
		m_queueLegacyHud = false;
		m_frameLabels.clear();
		m_dynFramePresented = false;
		m_sceneCaptureCurrent = false;
		m_sceneCapturesQueued = 0;
	}
	// Volume textures (the material system's colour-correction lookups and the
	// white volume) are available once the dynamic resources hold the white
	// volume, as they were before the post pipeline was deleted (e32f58361).
	bool VolumeTexturesSupported() const { return m_whiteVolumeHandle >= 0; }
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
		bool depthBiasEnable = false;
		// D3D9 controls RGB and alpha writes independently. Both are off for a
		// draw that only feeds an occlusion query, such as dev/lumcompare.
		bool colorWrite = true;
		bool alphaWrite = false;
		// D3D9's effective D3DRS_CULLMODE (the shadow state's EnableCulling with the
		// dynamic CullMode). Triangles are wound in D3D screen space, which the
		// flipped draw viewport preserves, so the front face is clockwise.
		VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
		// D3D9's D3DRS_STENCIL* render state (IShaderDynamicAPI SetStencil*): the
		// test and the three operations, applied to front and back faces alike
		// (D3D9 without two-sided stencil). The reference and the masks are
		// per-draw values (SetDynamicStencilValues), not pipeline state.
		// False builds the pipeline with its fragment stage's alpha test
		// compiled out (specialization constant kAlphaTest, a Metal function
		// constant under MoltenVK), for draws whose alpha reference is off. A
		// fragment stage that may discard costs a tiled GPU its hidden-surface
		// removal for every draw that uses it.
		bool alphaTest = true;
		// Legacy gamma-valued UI/post outputs into the linear scene.
		bool decodeOutput = false;
		// >= 0 builds the pipeline with the fragment stage's static combos
		// compiled in (specialization constant kSpecCombos, constant_id 1), so
		// the compiler drops the paths the material does not use; -1 leaves
		// them to the push constants. Set for lightmapped and skinned draws,
		// whose uber-shaders' register pressure dominated a tiled GPU's frame.
		int specCombos = -1;
		bool stencilEnable = false;
		VkCompareOp stencilCompare = VK_COMPARE_OP_ALWAYS;
		VkStencilOp stencilFail = VK_STENCIL_OP_KEEP;
		VkStencilOp stencilDepthFail = VK_STENCIL_OP_KEEP;
		VkStencilOp stencilPass = VK_STENCIL_OP_KEEP;
		// D3D9's D3DFILL_WIREFRAME (IShaderShadow::PolyMode with
		// SHADER_POLYMODE_LINE): polygons rasterize as their edges. Drawn only on
		// devices with fillModeNonSolid (WireframeSupported).
		bool wireframe = false;
	};

	// The depth bias of the core's draws (ApplyDepthBiasState): D3D9 supplies
	// a normalized bias; the port's constant factor is in depth buffer units
	// (the D32 float scale is approximate near depth 0.5).
	void SetDynamicDepthBias( float normalized, float slopeFactor )
	{
		m_dynDepthBiasConstant =
		    normalized *
		    ( m_depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ? 8388608.0f : 16777216.0f );
		m_dynDepthBiasSlope = slopeFactor;
	}
	float DynamicDepthBiasConstant() const { return m_dynDepthBiasConstant; }
	float DynamicDepthBiasSlope() const { return m_dynDepthBiasSlope; }
	// Bits of the depth-stencil attachment's stencil aspect (0 when the device
	// offers no depth format with stencil).
	int StencilBits() const { return m_stencilBits; }
	// True when blended sRGB-write draws blend in linear space, as on D3D9.
	bool LinearSpaceSrgbBlending() const { return m_srgbAttachments; }
	// Render-pass merging (on by default): records whose result is the same
	// through either view of a target stay in the open pass, and the frame's
	// first pass opens in the view its first color draw needs. Off restores the
	// earlier pass per view change (-vkpassmerge 0), for A/B runs and rollback.
	void SetPassMerging( bool enable ) { m_passMerging = enable; }
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
	};
	// kMaxClipPlanes when the device can clip (shaderClipDistance and push
	// constants wide enough for the planes), else 0.
	int MaxClipPlanes() const { return m_clipPlanesSupported ? kMaxClipPlanes : 0; }
	// Whether BC1-BC3 (DXT1/DXT3/DXT5) images can be created and sampled: the
	// textureCompressionBC feature, enabled at device creation when supported.
	bool SupportsBlockCompression() const { return m_blockCompression; }

	// Material-supplied textures (IShaderAPI CreateTexture/TexImage2D/BindTexture).
	// CreateManagedTexture returns a handle >= 0, UploadManagedTexture fills it
	// with tightly-packed 8-bit RGBA, and BindManagedTexture points the textured
	// material shader's sampler at it (handle < 0 restores the built-in texture).
	// `format` may be an uncompressed (R8G8B8A8/B8G8R8A8) or block-compressed
	// (BC1/BC3, i.e. DXT1/DXT5) Vulkan format; upload data must match it.
	// `mipLevels` is clamped to the full chain; each level is uploaded separately.
	// The selected device's format/usage extent and mip limits are checked before
	// any image or memory is allocated.
	// `srgbAlias`, when set, is the sRGB twin of `format`: the image is created
	// mutable between the two and also gets an sRGB view (render targets).
	// `depth` > 1 makes a volume (3D) texture, one level, not a cube; its
	// uploads cover every slice at once.
	// `cubeCount` > 1 makes a cube array of that many cubes (layer 6n + face).
	int CreateManagedTexture( int width, int height, VkFormat format, std::string *outError,
	    VkImageUsageFlags extraUsage = 0, uint32_t mipLevels = 1,
	    VkFormat srgbAlias = VK_FORMAT_UNDEFINED, bool cube = false, uint32_t depth = 1,
	    uint32_t cubeCount = 1 );
	// The selected device's largest volume texture edge.
	uint32_t MaxVolumeTextureDimension() const { return m_maxImageDimension3D; }
	bool UploadManagedTexture( int handle, const uint8_t *data, size_t dataSize,
	    std::string *outError, uint32_t level = 0, uint32_t face = 0 );
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
	    uint32_t level = 0, uint32_t face = 0 );
	uint32_t ManagedTextureMipLevels( int handle ) const
	{
		return ( handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) )
		           ? m_managedTextures[static_cast<size_t>( handle )].mipLevels
		           : 0;
	}
	bool ManagedTextureIsVolume( int handle ) const
	{
		return handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) &&
		       m_managedTextures[static_cast<size_t>( handle )].depth > 1;
	}
	// Textures on samplers 1..15 for shaders that read them as ordinary
	// textures (PortalRefract's noise and color, the skin shader's normal,
	// exponent, warp and self-illumination maps), -1 for none.
	enum
	{
		kMaxSamplers = 16
	};
	// A draw's pixel fog (common_ps_fxc.h FinalOutput's BlendPixelFog), in the
	// D3D9 registers' terms: the fog color (g_LinearFogColor, c29) with the pixel
	// fog type in w (0 range, 1 height, -1 the shader does not fog); the pass's
	// fog parameters (fog start over range, water height, max density, 1 / range);
	// the row giving a vertex's world z from its record position (dot with
	// (x, y, z, 1)); and the eye's world z in misc.x, with misc.y 1 for
	// DecalModulate's pow( factor, 0.4 ). The textured and world pipelines read it
	// from an instance-rate vertex stream (vertex binding 1), so it takes no
	// push-constant space.
	struct DrawFog
	{
		float color[4] = { 0.0f, 0.0f, 0.0f, -1.0f };
		float params[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
		float worldZ[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
		float misc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		bool operator==( const DrawFog &other ) const
		{
			return std::memcmp( this, &other, sizeof( DrawFog ) ) == 0;
		}
	};
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
		// LightmappedGeneric's second base texture (s7) and detail (s12).
		kColorSrgbReadSampler7 = 4194304,
		kColorSrgbReadSampler12 = 8388608,
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
		kFragmentSpriteVertexAlpha = 65536,
		kFragmentLightmappedEnvmap = 131072,
		kFragmentRefract = 262144,
		kFragmentNormalAlphaEnvmapMask = 2097152,
	};

	// Render targets (IShaderAPI SetRenderTarget and TEXTURE_CREATE_RENDERTARGET).
	// A render-target texture is a managed texture that can also be drawn into:
	// it is created in the swapchain format with its own depth buffer, so the
	// pipelines built for the swapchain pass draw into it unchanged. It starts
	// cleared to opaque black and counts as resident.
	//
	// deferStorage allocates its images on first use instead (many of the
	// material system's full-frame targets are never drawn into: at 4K each
	// costs ~96 MiB with its depth). Until then it samples a shared opaque
	// black image, a clear to opaque black leaves it as it is, and anything
	// else that records against it, copies into it or imports it allocates
	// the storage first; failing to is fatal (VulkanContextConfig::fatalError).
	int CreateRenderTargetTexture(
	    int width, int height, std::string *outError, bool deferStorage = false );
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
		kSamplerAnisotropic = 32,
		kSamplerStates = 64
	};
	void SetManagedTextureSamplerState( int handle, int samplerState );
	int ManagedTextureSamplerState( int handle ) const
	{
		return ( handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) )
		           ? m_managedTextures[static_cast<size_t>( handle )].samplerState
		           : 0;
	}
	void SetViewport( int x, int y, int width, int height, float minZ, float maxZ );
	// Clear the current viewport of the current target to the clear color
	// (SetClearColor) and/or to depth 1.
	void QueueClear( bool color, bool depth, bool stencil = false );
	// A frame copy's alpha as D3D9 PC holds it (WRITE_DEPTH_TO_DESTALPHA): the
	// source's projected z over the dest-alpha depth range, from its depth.
	struct DepthToAlpha
	{
		// The draw projection's z column (row-vector matrices): projected z =
		// view z * zScale + zOffset, w = view z * wScale + wOffset.
		float projection[4]; // zScale, zOffset, wScale, wOffset
		float invRange;      // 1 / the dest-alpha depth range
	};
	// Copy the current target into a render-target texture, scaling between the
	// rectangles ({x, y, width, height}; null = the whole image). With
	// `depthToAlpha` the copy's alpha then holds the source's depth.
	bool QueueCopyToTexture( int dstHandle, const int *srcRect, const int *dstRect,
	    const DepthToAlpha *depthToAlpha = nullptr );
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
		float viewport[6];
	};
	std::vector<StreamRecordInfo> DescribeStreamRecords() const;

	// The most recent captured frame as tightly-packed 8-bit RGBA, top row
	// first. Empty until a captured EndFrame() completes. *outW/*outH give the
	// captured extent.
	const std::vector<uint8_t> &GetCapturedPixels( int *outW, int *outH ) const;
	const std::vector<uint16_t> &GetCapturedHdrPixels( int *outW, int *outH ) const
	{
		*outW = m_capturedWidth;
		*outH = m_capturedHeight;
		return m_capturedHdrPixels;
	}

	// Selected adapter facts, valid after a successful Init().
	const char *DeviceName() const { return m_deviceName.c_str(); }
	uint32_t VendorId() const { return m_vendorId; }
	uint64_t DeviceLocalMemoryBytes() const { return m_deviceLocalMemoryBytes; }
	bool IsDiscrete() const { return m_isDiscrete; }
	bool ValidationEnabled() const { return m_validationEnabled; }

	// VK_EXT_debug_utils names and labels; inert unless the config's policy
	// wants them. Its label stack follows the frame command buffers, so it is
	// used on the thread that records frames.
	VulkanDebugUtils &DebugUtils() { return m_debugUtils; }
	// Names a managed texture's image and views (Source's texture name); a
	// render pass into it is labelled with the name.
	void NameManagedTexture( int handle, const char *name );
	// A label at the current point of the frame's record stream (IShaderAPI's
	// PIX events), emitted when the records replay; dropped while labels are off.
	enum class FrameLabelOp : uint8_t
	{
		Push,
		Pop,
		Insert,
	};
	void QueueFrameLabel( FrameLabelOp op, const char *name = nullptr, uint32_t argb = 0 );

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
	// mat_vsync. Recorded here and applied by the next BeginFrame, which
	// recreates the swapchain when the selected present mode changes; call it
	// on the thread that owns the device.
	void RequestVSync( bool vsync ) { m_requestedVSync = vsync; }
	bool VSyncRequested() const { return m_requestedVSync; }
	// mat_hdr_output (RFC 0016 "Output", render.output.v1; the swapchain side
	// follows render.presentation.v1 "Dynamic range"). Asks for an
	// extended-linear half-float swapchain, taken by the next swapchain build
	// when the surface offers one, the window can show it, the back buffer
	// has an sRGB view and a core-pass recorder records the frame's output.
	// Otherwise the build says why and presents as before. Applied like
	// RequestVSync.
	void RequestExtendedOutput( bool extended ) { m_requestedExtendedOutput = extended; }
	// Whether the current swapchain is extended-linear.
	bool ExtendedOutput() const { return m_extendedOutput; }
	bool HasCorePassRecorder() const { return m_corePassRecorder != nullptr; }
	bool HdrScene() const { return m_config.hdrScene; }
	bool OutputPassRecorded() const { return m_outputSectionRecorded; }
	// 0 for either takes it from the display (ResolvedHdrSettings).
	void SetHdrSettings( float exposure, float peakNits )
	{
		m_hdrExposure = exposure;
		m_hdrPeakNits = peakNits;
	}
	// The exposure and peak the output pass uses: the user's values, or for a
	// 0 the system's SDR white over the 203 cd/m^2 reference, and SDR white
	// times the display's headroom. Without a report: exposure 1, 1000 nits.
	// Sets *outFromDisplay when the display supplied the peak.
	void ResolvedHdrSettings(
	    float *outExposure, float *outPeakNits, bool *outFromDisplay = nullptr ) const;
	// The present mode of the current swapchain, and a count of swapchains
	// created since Init (each resize, mode or present-mode change adds one).
	VkPresentModeKHR PresentMode() const { return m_presentMode; }
	// The present modes the surface offered when the swapchain was created.
	const std::vector<VkPresentModeKHR> &SurfacePresentModes() const
	{
		return m_surfacePresentModes;
	}
	uint64_t SwapchainGeneration() const { return m_swapchainGeneration; }
	// mat_antialias: a sample count for the back buffer, applied by the next
	// BeginFrame, clamped to what the device supports (render.sample-count.v1);
	// 0 and 1 mean no multisampling. Call it on the thread that owns the device.
	void RequestSampleCount( int samples ) { m_requestedSamples = samples; }
	int ActiveSampleCount() const { return m_activeSamples; }
	static constexpr uint64_t kAcquireTimeoutNs = 1000000000ull;
	// Frame submissions so far (each EndFrame that submitted adds one), and a
	// wait of at most timeoutNs for the submission with that serial to complete
	// on the GPU: D3D9's frame sync query (IShaderAPI::ForceHardwareSync). True
	// once it has completed, including one that completed long ago; false on a
	// timeout, a serial not yet submitted, or no device.
	uint64_t SubmittedFrameSerial() const { return m_submitSerial; }
	bool WaitForSubmittedFrame( uint64_t serial, uint64_t timeoutNs );
	// Frames presented, and how many of them the present blit had to scale
	// because the back buffer and the drawable differed. The longest run of
	// consecutive scaled presents bounds how long the back buffer took to follow
	// a changed drawable (a queued resize settles first; a synchronous one never
	// scales).
	uint64_t PresentCount() const { return m_presentCount; }
	uint64_t ScaledPresentCount() const { return m_scaledPresentCount; }
	uint64_t LongestScaledPresentRun() const { return m_longestScaledPresentRun; }

	// The monitor gamma ramp (render.gamma-ramp.v1), applied to the presented
	// image as D3D9's hardware ramp is; back-buffer reads (captures, ReadPixels)
	// stay unchanged. Callable from any thread: the next frame to present picks
	// the newest ramp up. An 8-bit identity ramp presents by plain blit.
	void PublishGammaRamp( const render::GammaRamp16 &ramp );
	// Whether presents currently apply a ramp, and how many frames did.
	bool GammaPresentActive() const { return m_gammaActive; }
	uint64_t GammaPresentCount() const { return m_gammaPresentCount; }

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
	bool SavePipelineStore( std::string *outError );
	bool OpenFrameStats( const char *path, std::string *outError );
	// -vkgputimers: GPU time per render pass, target copy, scene capture and
	// present in the frame-stats stream ("gpu_passes"). Timestamps are taken
	// at pass boundaries, outside any render pass, so no pass (or Metal render
	// encoder under MoltenVK) is split to time it. False when the queue has no
	// timestamps.
	bool EnableGpuTimers( std::string *outError );
	void CloseFrameStats();
	void MarkFrame( const char *label );
	// -vkrenderdocframes: capture the listed stats frames under RenderDoc.
	// `trigger` (RenderDoc's TriggerCapture) runs right after frame N-1's
	// present, so the capture holds exactly stats frame N, whose stats line
	// then carries "renderdoc_capture":true. `frames` must be ascending.
	void SetRenderDocFrames( std::vector<uint64_t> frames, bool ( *trigger )() );
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
	int m_statsPresentMode = -1; // the present mode the stream last recorded
	uint64_t m_frameBeginUs = 0;
	uint64_t m_prevFrameBeginUs = 0;
	uint64_t m_prevFrameEndUs = 0;
	uint64_t m_recordBeginUs = 0;
	uint64_t m_frameBeginCpuUs = 0;
	uint64_t m_prevFrameBeginCpuUs = 0;
	std::string m_frameMarks;
	std::vector<uint64_t> m_renderDocFrames; // ascending, not yet captured
	bool ( *m_renderDocTrigger )() = nullptr;
	bool m_renderDocArmed = false; // the frame being built is being captured
	VkQueryPool m_timestampPool = VK_NULL_HANDLE;
	double m_timestampPeriodNs = 0.0;
	uint64_t m_timestampMask = 0;
	uint64_t m_slotStatsFrame[kMaxFramesInFlight] = {};
	uint64_t m_gpuResultFrame = 0;
	uint64_t m_gpuResultUs = 0;
	// Begin to the end of rendering, before the present blit or gamma pass
	// that waits for the acquired swapchain image: on a display that only
	// presents in FIFO (iOS, tvOS), m_gpuResultUs includes that vsync wait.
	uint64_t m_gpuRenderUs = 0;
	// Per frame slot: begin, end of rendering, end of the frame's commands.
	static constexpr uint32_t kTimestampsPerFrame = 3;
	// -vkgputimers: one timestamp per boundary; the segment after a mark is
	// labeled by the mark (EnableGpuTimers).
	static constexpr uint32_t kMaxGpuTimerMarks = 128;
	VkQueryPool m_gpuTimerPool = VK_NULL_HANDLE;
	std::vector<std::string> m_gpuTimerLabels[kMaxFramesInFlight];
	// The latest resolved frame's time per label: label, segments, microseconds.
	struct GpuTimerTotal
	{
		std::string label;
		uint32_t count;
		uint64_t us;
	};
	std::vector<GpuTimerTotal> m_gpuTimerResult;
	// Every 120th resolved frame: its segments in order, "label",us pairs
	// ("gpu_sequence" in the stats), which the per-label totals cannot show.
	std::string m_gpuTimerSequence;
	// With the timers: where the frame's back-buffer depth and stencil reads
	// end, of how many records ("depth_end" in the stats).
	size_t m_statsDepthEnd[3] = {};
	// Actual indexed/non-indexed draws issued from the legacy record stream
	// in the most recently submitted frame (RFC 0016 K9 runtime census).
	size_t m_statsLegacyStreamDraws = 0;
	size_t m_statsLegacyProgramDraws = 0;
	void GpuTimerMark( VkCommandBuffer cmd, std::string label );
	std::string GpuTimerTargetLabel( const char *kind, int target, bool srgb ) const;
	void CreateTimestampPool();
	void ReadSlotGpuTime( uint32_t slot );
	void WriteFrameStats( uint64_t endUs );

	// The adapter (render.device.vulkan) creates the instance, the device and
	// its queues; the context names its surface, extensions and features.
	bool CreateDevice( std::string *outError );
	void DescribeDevice( VkPhysicalDevice physical, uint32_t graphicsFamily,
	    render::device::vulkan::HostDeviceFeatures *out );
	bool CreateSurface( std::string *outError );
	VkSampler CreateManagedSampler( int state, int anisotropy ) const;
	// `oldSwapchain` hands the presentation over (VkSwapchainCreateInfoKHR), so a
	// rebuild never leaves the window without a presentable image.
	bool CreateSwapchain( std::string *outError, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE );
	bool CreateBackBuffers( std::string *outError );
	bool CreateRenderPass( std::string *outError );
	bool CreateAttachmentPass( VkAttachmentLoadOp loadOp, VkImageLayout colorInitial,
	    VkImageLayout colorFinal, VkImageLayout depthInitial, VkRenderPass *outPass,
	    VkFormat colorFormat, VkSampleCountFlagBits samples, std::string *outError,
	    int keep = kKeepDepthStencil );
	// Multisampled back buffer (mat_antialias): one color and one depth/stencil
	// image of m_activeSamples samples at the back buffer's size, drawn through
	// their own passes and resolved into the frame's back buffer (m_swapImages)
	// before anything reads it. Render targets stay single-sampled, as on D3D9.
	bool ApplySampleCount( std::string *outError );
	bool CreateMsaaTargets( int samples, std::string *outError );
	void DestroyMsaaTargets();
	void DestroyMsaaPasses();
	// Resolves the multisampled color into m_swapImages[imageIndex] and leaves
	// that image in COLOR_ATTACHMENT_OPTIMAL (the layout a single-sampled back
	// buffer rests in between passes). Outside any render pass.
	void ResolveBackBuffer( VkCommandBuffer cmd, uint32_t imageIndex );
	bool CreateFramebuffers( std::string *outError );
	bool CreateCommandResources( std::string *outError );
	bool CreateSyncObjects( std::string *outError );
	bool GrowRenderFinished( size_t count, std::string *outError );
	bool CreateCaptureImage( VkExtent2D extent, std::string *outError );

	void DestroySwapchainObjects();
	void DestroyBackBuffers();
	// Core passes (RFC 0016 K5). The back buffers, their depth and the
	// multisampled targets imported as port textures on first use (homes:
	// color attachment and depth write, the layouts they rest in between
	// passes), and released with them.
	render::legacy::CorePassTarget CorePassTargetFor( int target );
	void ReleaseCorePassImports( bool msaaOnly );
	// The scene stage's sections: one per slot record, in stream order.
	void RecordCorePassSections( render::device::CommandEncoder &encoder );
	// The frame's output (RFC 0016 "Output") on an extended-linear swapchain:
	// the back buffer through its sRGB view (linear values) and the acquired
	// swapchain image, imported as port textures (home kColorAttachment) and
	// handed to the recorder as the present stage's section.
	render::legacy::CoreOutputTargets CoreOutputTargetsFor();
	void RecordOutputSection( render::device::CommandEncoder &encoder );
	void RecordPresentOutput(
	    VkCommandBuffer cmd, uint32_t imageIndex, VkImageLayout backBufferLayout );
	void ReleaseOutputImports( bool swapchainToo );
	std::vector<render::device::TextureId> m_outputScene;  // per back buffer
	std::vector<render::device::TextureId> m_outputTarget; // per swapchain image
	bool m_outputSectionRecorded = false; // this frame's present runs the output
	bool m_outputFallbackLogged = false;
	render::legacy::ICorePassRecorder *m_corePassRecorder = nullptr;
	// Managed textures imported for core passes (ICoreTextures), by handle:
	// the image imported and its port texture.
	class CoreTextures final : public render::legacy::ICoreTextures
	{
	public:
		explicit CoreTextures( CVulkanContext &context ) : m_context( context ) {}
		render::device::TextureId Import( int handle, bool srgb ) override
		{
			return m_context.ImportManagedTexture( handle - 1, srgb );
		}
		render::device::SamplerDesc Sampler( int handle ) override
		{
			return m_context.ManagedTextureSampler( handle - 1 );
		}

		// Frozen-path: a texture the material system made and has not filled
		// (a picmip reload) is pending for the core, not a failed import.
		bool Pending( int handle ) override
		{
			const int index = handle - 1;
			if ( index < 0 || index >= int( m_context.m_managedTextures.size() ) )
				return false;
			const auto &texture = m_context.m_managedTextures[index];
			return !texture.uploaded && !texture.renderTarget && !texture.storagePending;
		}

		std::uint64_t ContentRevision( int handle ) override
		{
			const int index = handle - 1;
			if ( index < 0 || index >= int( m_context.m_managedTextures.size() ) )
				return 0;
			const auto &texture = m_context.m_managedTextures[index];
			return texture.uploaded && !texture.renderTarget ? texture.contentRevision : 0;
		}

		std::optional<render::legacy::ICoreTextures::MipInfo> MipDescription( int handle ) override
		{
			const int index = handle - 1;
			if ( index < 0 || index >= int( m_context.m_managedTextures.size() ) )
				return std::nullopt;
			const auto &texture = m_context.m_managedTextures[index];
			if ( !texture.uploaded || texture.renderTarget || texture.layers != 1 || texture.depth != 1 ||
			     !texture.width || !texture.height || !texture.mipLevels )
				return std::nullopt;
			return render::legacy::ICoreTextures::MipInfo{
			    texture.width, texture.height, texture.mipLevels };
		}

	private:
		CVulkanContext &m_context;
	};
	struct CoreTextureImport
	{
		VkImage image = VK_NULL_HANDLE;
		render::device::TextureId id[2]; // unorm, sRGB
		render::device::TextureId depth; // render target's paired depth/stencil
	};
	render::device::TextureId ImportManagedTexture( int handle, bool srgb );
	render::device::SamplerDesc ManagedTextureSampler( int handle ) const;
	// Releases a managed texture's import behind the frame being recorded.
	void ReleaseManagedImport( int handle );
	CoreTextures m_coreTextures{ *this };
	std::vector<CoreTextureImport> m_coreTextureImports;
	std::vector<render::device::TextureId> m_coreColor;     // per back buffer
	std::vector<render::device::TextureId> m_coreColorSrgb; // its sRGB view
	std::vector<render::device::TextureId> m_coreDepth;
	render::device::TextureId m_coreMsColor;
	render::device::TextureId m_coreMsColorSrgb;
	render::device::TextureId m_coreMsDepth;
	uint64_t m_corePassesRun = 0;
	bool RecreateSwapchain( std::string *outError );
	bool RecreateBackBuffers( std::string *outError );
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
	// The same, drawing the back buffer through the gamma ramp instead of
	// blitting it. False (nothing recorded) when the pass is unavailable.
	bool RecordPresentGamma(
	    VkCommandBuffer cmd, uint32_t imageIndex, VkImageLayout backBufferLayout, bool capture );
	// Counts a present, scaled when the back buffer and the drawable differ.
	void NoteSuboptimal();
	void NotePresent( bool scaled );
	// Copies the presented swapchain image (in `layout`, last written at
	// `srcStage`/`srcAccess`) for capture if requested, then transitions it to
	// PRESENT_SRC.
	void RecordPresentedCaptureAndRelease( VkCommandBuffer cmd, uint32_t imageIndex,
	    VkImageLayout layout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccess,
	    bool capture );
	// Picks up a newly published ramp (frame thread).
	void ApplyPublishedGammaRamp();
	// Device-lifetime and swapchain-lifetime objects of the gamma pass.
	bool EnsurePresentGamma( std::string *outError );
	bool EnsurePresentGammaTargets( std::string *outError );
	void DestroyPresentGammaTargets();
	void DestroyPresentGamma();
	bool ResolveCapturedPixels( std::string *outError );

	// The vertex attributes a pipeline declares, less those its vertex stage
	// does not read (Vulkan reports each as WARNING-Shader-OutputNotConsumed).
	// CreateShaderModule records each vertex module's input locations from its
	// SPIR-V. `storage` holds the filtered copy until the pipeline is created.
	struct ConsumedVertexInput
	{
		VkPipelineVertexInputStateCreateInfo info = {};
		std::vector<VkVertexInputAttributeDescription> attributes;
	};
	const VkPipelineVertexInputStateCreateInfo *FilterVertexInput(
	    const VkPipelineVertexInputStateCreateInfo *input, VkShaderModule vertex,
	    ConsumedVertexInput *storage ) const;
	std::map<VkShaderModule, uint64_t> m_vertexInputLocations;
	// Each module's index name (material_spv_index.h), kept while debug labels
	// are on, to name the pipelines built from it.
	std::map<VkShaderModule, const char *> m_moduleNames;
	bool CreateShaderModule(
	    const uint32_t *code, size_t sizeBytes, VkShaderModule *outModule, std::string *outError );
	bool CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
	    VkBuffer *outBuffer, VulkanMemory *outMemory, std::string *outError );
	// Memory through render.device.vulkan: an image bound to memory with the
	// given properties (the first matching type, as before), and the
	// allocation's mapping and freeing. FreeMemory is for memory whose buffer
	// or image no submitted work still uses.
	bool CreateImage( const VkImageCreateInfo &info, VkMemoryPropertyFlags props, VkImage *outImage,
	    VulkanMemory *outMemory, const char *what, std::string *outError );
	VkResult MapMemory( VulkanMemory memory, void **data );
	void UnmapMemory( VulkanMemory memory );
	void FreeMemory( VulkanMemory memory );

	bool BeginSingleTimeCommands( VkCommandBuffer *outCmd, std::string *outError );
	// Submits cmd signaling the next value of the adapter's timeline (outValue)
	// and never waits: the command buffer is freed behind that value, and the
	// caller releases what the work reads behind it too (or waits for it, for
	// a readback).
	bool EndSingleTimeCommands(
	    VkCommandBuffer cmd, std::string *outError, uint64_t *outValue = nullptr );
	void DestroyDynamicMesh();
	bool CreateDepthResources( std::string *outError );

	IVulkanSurfaceHost *m_host = nullptr;
	VulkanContextConfig m_config;

	// The device this context borrows (render.device.vulkan's host interop);
	// the handles below are its, cached.
	render::device::vulkan::IHostDevice *m_hostDevice = nullptr;
	// This context had the owner create the device (no root created it ahead
	// of Init: a GPU suite's host), so it releases it at Shutdown; otherwise
	// the root releases it after this context shut down.
	bool m_releaseDevice = false;
	std::vector<const char *> m_deviceExtensions;   // DescribeDevice's, until creation
	std::vector<const char *> m_instanceExtensions; // the host's, until creation
	bool m_toolingInfo = false;
	std::string m_createError;
	uint64_t m_adapterAllocations = 0; // the adapter's own (its upload ring)
	VkInstance m_instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	uint32_t m_graphicsQueueFamily = UINT32_MAX;
	uint32_t m_presentQueueFamily = UINT32_MAX;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	// The back buffers', render targets' and passes' color format (8-bit)
	// and the swapchain's own format and color space: the same unless the
	// swapchain is extended-linear (RequestExtendedOutput).
	VkFormat m_swapFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR m_swapColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkFormat m_presentFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR m_presentColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	// The swapchain is PASS_THROUGH and the surface declares the output's own
	// image description (IVulkanSurfaceHost::PrepareOutputDescription).
	bool m_outputDescription = false;
	bool m_outputDescriptionRefused = false; // attaching failed on this window
	bool m_presentDescriptionDirty = false;  // rebuild the swapchain for it
	bool m_requestedExtendedOutput = false;
	bool m_swapchainExtendedRequest = false; // the request the swapchain was built for
	bool m_extendedOutput = false;
	float m_hdrExposure = 1.0f;
	float m_hdrPeakNits = 1000.0f;
	VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
	VulkanAdapterCaps m_adapterCaps;
	int m_requestedSamples = 1;
	int m_activeSamples = 1;
	uint64_t m_resolveCount = 0;
	VkImage m_msColor = VK_NULL_HANDLE;
	VkImage m_msDepth = VK_NULL_HANDLE;
	VulkanMemory m_msColorMemory = VK_NULL_HANDLE;
	VulkanMemory m_msDepthMemory = VK_NULL_HANDLE;
	VkImageView m_msColorView = VK_NULL_HANDLE;
	VkImageView m_msColorViewSrgb = VK_NULL_HANDLE;
	VkImageView m_msDepthView = VK_NULL_HANDLE;
	// The multisampled depth read by the depth-to-alpha pass (depth aspect only),
	// when the depth format can be sampled.
	VkImageView m_msDepthSampleView = VK_NULL_HANDLE;
	VkDescriptorSet m_msDepthSampleSet = VK_NULL_HANDLE;
	VkFramebuffer m_msFramebuffer = VK_NULL_HANDLE;
	VkFramebuffer m_msFramebufferSrgb = VK_NULL_HANDLE;
	VkRenderPass m_msPassClear = VK_NULL_HANDLE;
	VkRenderPass m_msPassLoad = VK_NULL_HANDLE;
	VkRenderPass m_msPassClearSrgb = VK_NULL_HANDLE;
	VkRenderPass m_msPassLoadSrgb = VK_NULL_HANDLE;
	int m_msPassSamples = 0; // the sample count the m_msPass* objects were built for
	VkExtent2D m_msExtent = { 0, 0 };
	bool m_requestedVSync = true;
	std::vector<VkPresentModeKHR> m_surfacePresentModes;
	bool m_swapchainVSync = true; // the vsync request m_presentMode was selected for
	uint64_t m_swapchainGeneration = 0;
	uint64_t m_acquireTimeouts = 0;
	bool m_acquireTimedOut = false;
	// An acquire or present reported VK_SUBOPTIMAL_KHR with the surface
	// unchanged (NoteSuboptimal). On Wayland that is new dmabuf feedback (a
	// scanout tranche for a fullscreen surface): the rebuilt swapchain takes
	// the modifiers the display can scan out. Rebuilt is cleared by the next
	// clean acquire, so a persistently suboptimal surface rebuilds once.
	bool m_suboptimalPending = false;
	bool m_suboptimalRebuilt = false;
	uint32_t m_suboptimalRebuilds = 0;
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
	uint64_t m_scaledPresentRun = 0;
	uint64_t m_longestScaledPresentRun = 0;
	bool m_presentCapturable = false;
	std::vector<VkImage> m_presentImages;

	// Monitor gamma. PublishGammaRamp writes m_publishedRamp under the mutex and
	// then bumps the revision (release); the frame thread compares it (acquire)
	// and copies the ramp under the mutex.
	std::mutex m_gammaMutex;
	render::GammaRamp16 m_publishedRamp = {};
	std::atomic<uint64_t> m_publishedRampRevision{ 0 };
	uint64_t m_appliedRampRevision = 0;
	render::GammaRamp16 m_activeRamp = {};
	bool m_gammaActive = false;
	// How the last frame reached its swapchain image (logged on change):
	// 'output' and 'gamma' are passes that write it directly, 'blit' copies
	// the back buffer.
	const char *m_presentPath = nullptr;
	// The frame being recorded turned the legacy stream off (RFC 0014,
	// render::legacy::kCorePassLegacyOff): it presents without the ramp.
	bool m_frameLegacyOff = false;
	bool m_queueCustomEffects = false; // product custom effects; off in diagnostics
	bool m_queueCoreOnly = false;    // frame-ordered slot, before vertex conversion
	bool m_queueLegacyHud = false;   // top-level HUD stage, scoped to this frame
	bool m_gammaUnavailable = false; // the pass failed to build; presents blit
	uint64_t m_gammaPresentCount = 0;
	VkFormat m_gammaFormat = VK_FORMAT_UNDEFINED; // what the pass was built for
	VkRenderPass m_gammaRenderPass = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_gammaSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout m_gammaPipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_gammaPipeline = VK_NULL_HANDLE;
	// Depth into a frame copy's alpha (DepthToAlpha): a fullscreen pass in a
	// render-target texture's pass writing alpha only.
	bool EnsureDepthToAlpha( std::string *outError );
	bool EnsureMsDepthSampleSet();
	void DestroyDepthToAlpha();
	VkPipelineLayout m_depthToAlphaLayout = VK_NULL_HANDLE;
	VkPipeline m_depthToAlphaPipeline = VK_NULL_HANDLE;
	VkPipeline m_depthToAlphaMsPipeline = VK_NULL_HANDLE; // reads multisampled depth
	bool m_depthToAlphaUnavailable = false; // the pass failed to build
	uint32_t m_lastFrameDepthToAlpha = 0;
	uint32_t m_lastFrameDepthToAlphaSkipped = 0;
	VkSampler m_gammaSamplerNearest = VK_NULL_HANDLE;
	VkSampler m_gammaSamplerLinear = VK_NULL_HANDLE;
	VkDescriptorPool m_gammaDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet m_gammaSets[kMaxFramesInFlight] = {};
	VkBuffer m_gammaRampBuffers[kMaxFramesInFlight] = {};
	VulkanMemory m_gammaRampMemories[kMaxFramesInFlight] = {};
	float *m_gammaRampMapped[kMaxFramesInFlight] = {};
	std::vector<VkImageView> m_presentImageViews;
	std::vector<VkFramebuffer> m_presentFramebuffers;
	std::vector<VulkanMemory> m_backBufferMemories;
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
	std::vector<VulkanMemory> m_depthMemories;
	std::vector<VkImageView> m_depthViews;

	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	// Render-pass-compatible siblings of m_renderPass (same formats, samples and
	// dependencies), so every pipeline built against m_renderPass draws in them:
	// re-entering the swapchain image after a render-target pass (load, not
	// clear), and drawing into a render-target texture (sampled-layout in/out).
	VkRenderPass m_renderPassLoad = VK_NULL_HANDLE;
	VkRenderPass m_renderPassTarget = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;

	uint32_t m_framesInFlight = 2;
	uint32_t m_descriptorSetLimit = 0;
	uint32_t m_currentFrame = 0;
	std::vector<VkSemaphore> m_imageAvailable;
	std::vector<VkSemaphore> m_renderFinished; // per swapchain image (GrowRenderFinished)
	// Completion is the adapter's timeline (render.device.vulkan): each frame
	// slot's last submission signaled m_slotValue, and each swapchain image's
	// last submission m_imageValue (0: none), so a freshly acquired image is not
	// recorded into while a prior submission that targeted it still executes.
	uint64_t m_slotValue[kMaxFramesInFlight] = {};
	std::vector<uint64_t> m_imageValue;
	// Frame serials in flight with the timeline value each signals; the oldest
	// whose value completed advances m_completedSerial (UpdateCompletedSerial).
	std::vector<std::pair<uint64_t, uint64_t>> m_serialValues;
	void UpdateCompletedSerial();
	// One-time command buffers submitted and the timeline value that frees each.
	std::vector<std::pair<uint64_t, VkCommandBuffer>> m_oneTimeCommands;
	// Releases a buffer and its memory once the submission that signals value
	// completes (render.device.vulkan's release queue).
	void ReleaseBufferAfter( uint64_t value, VkBuffer buffer, VulkanMemory memory );
	// Objects the frame being recorded may still use, destroyed once the
	// submission with frame serial afterSerial completes (as m_retiredTextures).
	struct RetiredObject
	{
		uint64_t afterSerial;
		void ( *destroy )( CVulkanContext *context, uint64_t a, uint64_t b );
		uint64_t a;
		uint64_t b;
	};
	std::vector<RetiredObject> m_retiredObjects;

	// Host-visible linear image used to copy the rendered color image out for
	// verification. Allocated lazily on the first capture request.
	VkImage m_captureImage = VK_NULL_HANDLE;
	VkImage m_captureOutputImage = VK_NULL_HANDLE;
	VulkanMemory m_captureOutputMemory = VK_NULL_HANDLE;
	render::device::TextureId m_captureOutput;
	bool m_captureOutputRecorded = false;
	VulkanMemory m_captureMemory = VK_NULL_HANDLE;
	VkExtent2D m_captureExtent = { 0, 0 };
	bool m_captureRequested = false;
	bool m_captureHdrRequested = false;
	VkFormat m_captureFormat = VK_FORMAT_UNDEFINED;
	bool m_capturePending = false;
	std::vector<uint8_t> m_capturedPixels;
	std::vector<uint16_t> m_capturedHdrPixels;
	int m_capturedWidth = 0;
	int m_capturedHeight = 0;

	// Dynamic geometry path (material-system mesh draws).
	bool m_dynamicResourcesReady = false; // InitDynamicMesh ran
	float m_dynDepthBiasConstant = 0.0f;
	float m_dynDepthBiasSlope = 0.0f;
	// "$basetexture" material pipeline: a built-in 2-tone texture sampled at the
	// mesh UVs, bound through a descriptor set (its own layout adds the sampler).
	// Scene capture (vulkan_scene_capture.cpp): managed textures the size of the
	// back buffer. Color is the swapchain format with its sRGB view and a full
	// mip chain; depth is the depth format, sampled through a depth-only view.
	int m_sceneColorHandle = -1;
	int m_sceneDepthHandle = -1;
	// The depth format can be copied from the attachments and sampled.
	bool m_sceneDepthUsable = false;
	bool m_sceneDepthEnabled = true;
	// Queue-side state: whether the target still holds what the last capture
	// copied, which target and glass material it served, and captures queued.
	bool m_sceneCaptureCurrent = false;
	int m_sceneCaptureTarget = -1;
	uint32_t m_sceneCapturesQueued = 0;
	uint32_t m_lastFrameSceneCaptures = 0;
	uint32_t m_lastFrameSceneDepthCaptures = 0;
	// Whether the latest replayed capture copied depth (ReadSceneDepth).
	bool m_sceneDepthCaptured = false;
	bool EnsureSceneCapture( std::string *outError );
	void DestroySceneCapture();
	void NoteSceneChanged() { m_sceneCaptureCurrent = false; }
	// Records the copy into the capture images; returns whether depth was copied.
	// Copies `target`'s depth (width x height from the origin) into the capture's
	// depth image; false when there is no single-sampled depth to copy.
	bool RecordSceneDepthCopy( VkCommandBuffer cmd, int target, uint32_t width, uint32_t height );
	// The pipeline store (OpenPipelineStore): the cache every material pipeline
	// is built through, the variants built this session, and the files.
	enum PipelineFamily
	{
	};
	VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
	std::vector<std::pair<int, uint64_t>> m_pipelineVariants;
	std::string m_pipelineStoreDirectory;

	// PortalRefract pipelines, one per raster state, built on first use.
	int m_whiteVolumeHandle = -1;
	uint32_t m_maxImageDimension3D = 0;
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
	// Back-buffer load passes by view (sRGB or not) and by which of depth and
	// stencil they load and store (kKeepDepth | kKeepStencil). The entry for
	// both is m_renderPassLoad or m_renderPassLoadSrgb; the others serve the
	// back buffer after the frame's last depth or stencil use.
	VkRenderPass m_renderPassLoadKeep[2][4] = {};
	VkRenderPass m_renderPassTargetSrgb = VK_NULL_HANDLE;
	// The frame's clearing pass over the sRGB view (m_passMerging).
	VkRenderPass m_renderPassClearSrgb = VK_NULL_HANDLE;
	bool m_passMerging = true;
	bool m_opaqueBatching = true;
	std::uint64_t m_opaqueCandidates = 0;
	std::uint64_t m_opaqueBatches = 0;
	std::uint64_t m_opaqueFollowers = 0;
	VkSampler m_dynTexSampler = VK_NULL_HANDLE;
	// Sampler per addressing/filter combination, including anisotropy.
	VkSampler m_samplers[kSamplerStates] = {};
	int m_maxAnisotropy = 1;
	int m_anisotropyLevel = 1;
	VkDescriptorSetLayout m_dynTexDescLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_dynTexDescPool = VK_NULL_HANDLE;
	// 1x1 opaque black: what a deferred render target samples before it has
	// storage (a new target reads opaque black).
	int m_unrenderedTargetHandle = -1;
	// Material-supplied textures (IShaderAPI). The descriptor set points at the
	// bound one, or the built-in 2-tone texture when none is bound.
	enum
	{
		kMaxManagedTexSets = 16384
	};
	struct ManagedTexture
	{
		VkImage image = VK_NULL_HANDLE;
		VulkanMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		// Per-texture descriptor set, so each draw can bind its own texture.
		VkDescriptorSet descSet = VK_NULL_HANDLE;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mipLevels = 1;
		uint32_t layers = 1;
		uint32_t depth = 1; // > 1: a volume texture
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		// False until pixel data has actually been uploaded. Sampling an image
		// that was created but never filled yields undefined contents.
		bool uploaded = false;
		uint64_t contentRevision = 0;
		// Render-target textures own a depth buffer and a framebuffer for
		// m_renderPassTarget; their color image rests in SHADER_READ_ONLY.
		bool renderTarget = false;
		// Sampler state (kSampler* bits); 0 is D3D9's default: wrap, point.
		int samplerState = 0;
		VkImage depthImage = VK_NULL_HANDLE;
		VulkanMemory depthMemory = VK_NULL_HANDLE;
		VkImageView depthView = VK_NULL_HANDLE;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		// sRGB view of an 8-bit or BC image: a render target's sRGB attachment
		// (framebufferSrgb, m_renderPassTargetSrgb), and for sampling the set that
		// decodes each texel before filtering, as D3D9's SRGBTEXTURE does.
		VkImageView srgbView = VK_NULL_HANDLE;
		VkFramebuffer framebufferSrgb = VK_NULL_HANDLE;
		VkDescriptorSet descSetSrgb = VK_NULL_HANDLE;
		// NameManagedTexture's name, kept only while debug labels are on.
		std::string debugName;
		// A deferred render target without storage yet: view and descriptor
		// sets are borrowed from m_unrenderedTargetHandle (not owned), and
		// image, depth and framebuffers are null.
		bool storagePending = false;
	};
	std::vector<ManagedTexture> m_managedTextures;
	uint64_t m_nextTextureContentRevision = 1;
	ComputeCaps m_computeCaps;
	DeviceFeatureChain m_featureChain;
	ComputeResources m_compute;
	std::vector<std::function<void( VkCommandBuffer, uint64_t )>> m_computeWork;
	std::function<void( VkCommandBuffer, uint64_t )> m_computeFlush;
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
	// Stores a fully built managed texture in a free handle.
	int StoreManagedTexture( const ManagedTexture &texture );
	// Deferred render targets (CreateRenderTargetTexture's deferStorage).
	bool IsStoragePending( int handle ) const
	{
		return handle >= 0 && handle < static_cast<int>( m_managedTextures.size() ) &&
		       m_managedTextures[static_cast<size_t>( handle )].storagePending;
	}
	bool MaterializeRenderTarget( int handle, std::string *outError );
	void EnsureRenderTargetStorage( int handle );
	// Depth image, framebuffers and the initial clear for a render-target
	// texture whose color image and views are already in `t` (the eager body
	// shared by CreateRenderTargetTexture and MaterializeRenderTarget).
	bool BuildRenderTargetStorage( ManagedTexture &t, std::string *outError );
	void RetireCompletedTextures();
	// A persistently mapped host-visible buffer the frame's stream is copied
	// into. Each frame slot has its own: a slot's fence has signalled when its
	// frame begins, so its buffers are free to rewrite or regrow, while the
	// previous frame may still be reading the other slot's.
	struct StreamBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VulkanMemory memory = VK_NULL_HANDLE;
		void *mapped = nullptr;
		VkDeviceSize capacity = 0;
	};
	// The frame's distinct DrawFog records (vertex binding 1, per instance).
	// Texel uploads small enough to defer (a font glyph, a lightmap patch): the
	// texels are copied here and the copy is recorded at the start of the next
	// frame's command buffer, ahead of every draw that frame replays, instead of
	// each upload submitting a copy and waiting for the queue to go idle. All of
	// a frame's draws replay at present, so they see the same texels either way.
	// Larger uploads (level load) stay synchronous, after the pending ones.
	struct PendingUpload
	{
		int handle;
		uint32_t x, y, width, height, level, face;
		uint32_t depth; // slices, all of a volume texture's
		// The image's contents outside the region survive (a mip chain level, a
		// part of an uploaded image); a never-filled image's rest is cleared.
		bool preserve;
		bool clearRest;
		size_t offset;
		size_t size;
	};
	enum : size_t
	{
		// An upload up to this size is deferred; a bigger one runs at once,
		// waiting for the GPU. 4 MB keeps a frame's lightmap-page update off
		// that wait (13.5 ms on an Apple TV 4K at 1080p, a missed refresh).
		kDeferredUploadMaxBytes = 4 * 1024 * 1024,
		// Texels held back at most; beyond this the pending ones run at once.
		kPendingUploadCapBytes = 8 * 1024 * 1024
	};
	std::vector<PendingUpload> m_pendingUploads;
	std::vector<uint8_t> m_pendingUploadData;
	StreamBuffer m_uploadStreams[kMaxFramesInFlight];
	void RecordTextureUpload( VkCommandBuffer cmd, VkImage image, const PendingUpload &upload,
	    VkBuffer staging, VkDeviceSize offset );
	// Uploads of one image level that keep the rest of it and do not overlap:
	// one pair of barriers and one copy for all of them.
	void RecordTextureUploadRun( VkCommandBuffer cmd, VkImage image, const PendingUpload *uploads,
	    size_t count, VkBuffer staging );
	// Records every pending upload into `cmd`, staging from `stream`.
	bool RecordPendingUploads( VkCommandBuffer cmd, StreamBuffer &stream );
	// Runs the pending uploads now, in their own submission (before a
	// synchronous upload, so the order of uploads is kept).
	bool FlushPendingUploads( std::string *outError );
	bool EnsureStreamBuffer( StreamBuffer &stream, VkDeviceSize bytes, VkBufferUsageFlags usage );
	void DestroyStreamBuffer( StreamBuffer &stream );
	void ReleaseStreamBufferAfter( uint64_t value, StreamBuffer &stream );
	// Counts the frame streams discarded; core passes key per-stream state by it.
	uint64_t m_streamEpoch = 1;
	// The ordered stream of what the legacy interface still issues between the
	// core's passes: clears, render-target copies, occlusion query markers and
	// the core passes' slots.
	enum
	{
		kRecordClear = 1,
		kRecordCopy = 2,
		kRecordQueryBegin = 3,
		kRecordQueryEnd = 4,
		// A core pass's slot (RFC 0016 K5, core_passes.h): the replay closes
		// its pass and runs the slot's section of the scene record there.
		kRecordCorePass = 6
	};
	struct DynDraw
	{
		// Clears, copies and slots share one ordered stream, since
		// their relative order across render targets is what the frame means.
		int kind = kRecordClear;
		int target = -1; // render-target texture handle; -1 = swapchain image
		int tag = -1;    // caller-defined identity (diagnostics only)
		// x, y, width, height, minZ, maxZ; width <= 0 means the whole target.
		float viewport[6] = { 0, 0, 0, 0, 0, 1 };
		bool clearColor = false;
		bool clearDepth = false;
		float clearValue[4] = { 0, 0, 0, 1 };
		int copyDst = -1;
		bool corePortalCopy = false;   // the preceding snapshot consumed by a retained effect
		uint32_t corePass = 0;      // kRecordCorePass: the slot's tag
		uint32_t corePassTerms = 0; // and its terms (m_corePassTerms)
		// Occlusion query slot of a begin/end record, and which issue of that
		// query the begin record is.
		int query = -1;
		uint64_t querySerial = 0;
		int copySrcRect[4] = { 0, 0, 0, 0 }; // width <= 0 means the whole image
		int copyDstRect[4] = { 0, 0, 0, 0 };
		// A copy whose alpha then takes the source's depth (DepthToAlpha).
		bool copyDepthToAlpha = false;
		DepthToAlpha copyDepth = {};
		bool clearStencil = false;
	};
	std::vector<DynDraw> m_dynDrawRecords;
	std::vector<CorePassTerms> m_corePassTerms; // the kRecordCorePass records' terms
	// QueueFrameLabel's labels, each before the record at `record`.
	struct FrameLabel
	{
		size_t record;
		FrameLabelOp op;
		uint32_t argb;
		std::string name;
	};
	std::vector<FrameLabel> m_frameLabels;
	void ReplayFrameLabels( size_t *cursor, size_t throughRecord );
	// Target/viewport/scissor state captured by each record.
	int m_dynTarget = -1;
	int m_dynTag = -1;
	float m_dynViewport[6] = { 0, 0, 0, 0, 0, 1 };
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
	int m_queuedOcclusionQuery = -1;
	bool m_preciseOcclusion = false;
	bool m_fillModeNonSolid = false;
	// Issues replayed into the frame being recorded; marked submitted by EndFrame.
	std::vector<std::pair<int, uint64_t>> m_replayedQueries;
	void FailUnsubmittedQueries();
	void GetTargetExtent( int target, uint32_t *outW, uint32_t *outH ) const;
	// Which of the back buffer's depth and stencil a pass loads and stores.
	// Fewer (single-sampled back buffer only) for passes after the frame's last
	// use: on a tiled GPU each kept plane is a full-screen load and store.
	enum
	{
		kKeepNone = 0,
		kKeepDepth = 1,
		kKeepStencil = 2,
		kKeepDepthStencil = kKeepDepth | kKeepStencil
	};
	void BeginTargetPass(
	    VkCommandBuffer cmd, int target, bool srgb = false, int keep = kKeepDepthStencil );
	void RecordTargetCopy( VkCommandBuffer cmd, int srcTarget, const DynDraw &copy );
	void RecordDepthToAlpha( VkCommandBuffer cmd, int srcTarget, const DynDraw &copy );
	bool RecordWantsSrgb( const DynDraw &r ) const;
	// Which of the back buffer's depth and stencil a record may read
	// (kKeepDepth | kKeepStencil). Unknown kinds count as reading both.
	static bool SkipLegacyRecord( const DynDraw &record, bool legacyOff, bool legacyHud );
	static int RecordBackBufferDepthReads( const DynDraw &r );
	bool RecordViewAgnostic( const DynDraw &r ) const;
	bool FirstPassWantsSrgb() const;

	// Per-frame acquisition state, valid between BeginFrame and EndFrame.
	uint32_t m_acquiredImage = 0;
	bool m_frameOpen = false;
	// The prepared frame's capture (PrepareFrame decides it), and when its
	// recording ended (the submit cost runs from there to FinishFrame).
	bool m_frameCapture = false;
	bool m_frameCaptureHdr = false;
	bool m_frameCapturePresented = false;
	int64_t m_recordEndUs = 0;
	FrameCost m_lastFrameCost;
	// Records the prepared frame's stages (AttachFrameStage's host work):
	// all of them, or one.
	void RecordFrameCommands( VkCommandBuffer cmd );
	void RecordFrameStage( FrameStage stage, VkCommandBuffer cmd );
	void RecordFrameComputeAndUploads( VkCommandBuffer cmd );
	void RecordFrameScene( VkCommandBuffer cmd );
	void RecordFramePresent( VkCommandBuffer cmd );

	VkClearColorValue m_clearColor = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	// Selected adapter facts.
	std::string m_deviceName = "unknown";
	uint32_t m_vendorId = 0;
	uint32_t m_deviceId = 0;
	uint64_t m_deviceLocalMemoryBytes = 0;
	bool m_isDiscrete = false;
	bool m_validationEnabled = false;
	uint32_t m_validationErrorCount = 0;

	// VK_EXT_debug_utils was enabled on the instance (for validation, or for
	// names and labels when the policy is not Off).
	bool m_debugUtilsExtension = false;
	// VK_KHR_shader_non_semantic_info is enabled (debug shader variants).
	bool m_nonSemanticInfo = false;
	VulkanDebugUtils m_debugUtils;
	VulkanShaderLibrary m_shaderLibrary;
	void SetupDebugTools( bool toolingInfo );

	// Debug-messenger entry points, resolved from the instance when validation
	// is enabled.

	friend VkBool32 VulkanDebugCallbackTrampoline( VkDebugUtilsMessageSeverityFlagBitsEXT,
	    VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT *, void * );
};

// Formats whose samples the hardware decodes from sRGB on every view.
bool IsSrgbFormat( VkFormat format );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_DEVICE_H
