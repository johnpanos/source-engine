
#include <vulkan/vulkan.h>
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
#include "tier0/threadtools.h"
#include "tier1/tier1.h"
#include "materialsystem/idebugtextureinfo.h"
#include "materialsystem/deformations.h"
#include "render/legacy_shader_provider.h"
#include "render/pbr_material_schema.h"
#include "render/render_display_modes.h"
#include "render/render_sample_count.h"
#include "dxsupport_keyvalues.h"
#include "filesystem.h"
#include "tier1/KeyValues.h"
#include "vulkan_device.h"
#include "vulkan_world_mesh_upload.h"
#include "render/light_set.h"
#include "sdl3/sdl3_vulkan_surface_host.h"
#include "vtf/vtf.h"
#include "pixelwriter.h"
#include "shaderapi/commandbuffer.h"
#include "drawstatefixture.h"
#include "renderparm.h"
#if defined( USE_SDL )
#include "appframework/ilaunchermgr.h"
#endif

#include <algorithm>
#include <array>
#include <climits>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//-----------------------------------------------------------------------------
// The single native Vulkan presentation context, brought up on SetMode() and
// torn down at shutdown. Owns the real instance/device/queues/swapchain. While
// the material path is still the empty stub (roadmap R32), this proves the
// backend genuinely brings up native Vulkan in-process and presents a frame.
//-----------------------------------------------------------------------------
// The window side of the context, supplied by the SDL3-Vulkan bridge. Declared
// first so it is destroyed after the context, which borrows it until Shutdown.
static std::unique_ptr<render_vulkan::IVulkanSurfaceHost> g_VulkanSurfaceHost;
static render_vulkan::CVulkanContext g_VulkanContext;

// A video mode applied by ChangeVideoMode whose mode-change callbacks (the
// engine's window and UI adjustment) have not run yet. As on D3D9
// (CShaderDeviceDx8::ResizeWindow), they run at the next Present on the main
// thread; a resize applied on the render worker commits its UI itself.
static bool g_bPendingModeChangeCallbacks = false;
static void InvokePendingModeChangeCallbacks();

// The DirectX support level this backend's caps claim: 95, shader model 3.0
// class hardware, like the D3D9 device on the same GPUs. Vertex textures stay
// off (HasFastVertexTextures), so the stdshaders keep selecting the ps20b/vs20
// combos the native ports implement. -vkdxlevel 90 restores the earlier caps.
static int NativeCapsDxLevel()
{
	static const int s_nLevel = CommandLine()->ParmValue( "-vkdxlevel", 95 ) == 90 ? 90 : 95;
	return s_nLevel;
}

// The adapter's identity and caps (vulkan_adapter.h): the live device once
// SetMode brought it up; before that, a probe (the material system asks at
// Init, before any window exists). The probe runs once.
static const render_vulkan::VulkanAdapterCaps &CurrentAdapterCaps()
{
	static render_vulkan::VulkanAdapterCaps s_Probe;
	static bool s_bProbed = false;
	if ( g_VulkanContext.IsValid() )
	{
		// A probe that chose another device (one that cannot present to the
		// window) described a different adapter to dxsupport.cfg.
		const render_vulkan::VulkanAdapterCaps &live = g_VulkanContext.AdapterCaps();
		static bool s_bCompared = false;
		if ( s_bProbed && !s_bCompared && s_Probe.valid &&
		     ( s_Probe.vendorId != live.vendorId || s_Probe.deviceId != live.deviceId ) )
			Warning( "[NativeVulkan] the pre-window adapter probe chose '%s', the device is '%s'; "
			         "recommended settings described the probed adapter\n",
			    s_Probe.name.c_str(), live.name.c_str() );
		s_bCompared = true;
		return live;
	}
	if ( !s_bProbed )
	{
		s_bProbed = true;
		std::string error;
		if ( !render_vulkan::ProbeVulkanAdapter( &s_Probe, &error ) )
			Warning( "[NativeVulkan] %s\n", error.c_str() );
	}
	return s_Probe;
}

// Brings the context up against the engine's window. The window reference is
// handed to the pair-specific bridge untouched; nothing here interprets it.
static bool InitVulkanContext(
    void *legacyWindowRef, const render_vulkan::VulkanContextConfig &config, std::string *outError )
{
	std::unique_ptr<render_vulkan::IVulkanSurfaceHost> host =
	    render_vulkan::MakeSdl3LegacySurfaceHost( legacyWindowRef, outError );
	if ( !host )
		return false;
	if ( !g_VulkanContext.Init( *host, config, outError ) )
		return false;
	g_VulkanSurfaceHost = std::move( host );
	// Frame-pacing telemetry (tools/quality/frame_pacing.py): one line per
	// presented frame. Optional, so a sink that cannot be created only warns.
	const char *statsPath = CommandLine()->ParmValue( "-vkframestats", (const char *)NULL );
	std::string statsError;
	if ( statsPath && !g_VulkanContext.OpenFrameStats( statsPath, &statsError ) )
		Warning( "[NativeVulkan] frame stats unavailable: %s\n", statsError.c_str() );
	// -vkpassmerge 0 restores one render pass per view change (A/B, rollback).
	g_VulkanContext.SetPassMerging( CommandLine()->ParmValue( "-vkpassmerge", 1 ) != 0 );
	// The pipeline store lives in the mod directory (the launcher runs from the
	// base directory, and the mod directory is where the engine writes its
	// config on every platform); -vkpipelinecache names another directory, or
	// "none" to build every pipeline on first use without a store.
	const char *gameDir =
	    CommandLine()->ParmValue( "-game", CommandLine()->ParmValue( "-defaultgamedir", "hl2" ) );
	const char *storeDir = CommandLine()->ParmValue( "-vkpipelinecache", gameDir );
	std::string storeError;
	if ( Q_stricmp( storeDir, "none" ) != 0 &&
	     !g_VulkanContext.OpenPipelineStore( storeDir, &storeError ) )
		Warning( "[NativeVulkan] pipeline store unavailable: %s\n", storeError.c_str() );
	return true;
}

// Builds the material pipeline variants earlier runs needed (the pipeline
// store), during bring-up, so the frame that first draws with one does not
// stall on its compile (a portal opening, say).
static void PrewarmVulkanPipelines()
{
	const uint64_t start = render_vulkan::FrameClockMicros();
	const int built = g_VulkanContext.PrewarmPipelines();
	if ( built > 0 )
		Msg( "[NativeVulkan] prewarmed %d pipelines in %.1f ms\n", built,
		    ( render_vulkan::FrameClockMicros() - start ) / 1000.0 );
}

// Labels the frame being built in the frame-stats stream, so a scenario script
// can name its phases (vk_frame_mark fire_blue).
CON_COMMAND( vk_frame_mark, "Label the current frame in the -vkframestats stream" )
{
	for ( int i = 1; i < args.ArgC(); ++i )
		g_VulkanContext.MarkFrame( args[i] );
}

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
// RFC 0011 indirect-light debug view (CVulkanContext::SetIndirectLightView).
static ConVar mat_indirect_view( "mat_indirect_view", "0", FCVAR_CHEAT,
    "Indirect-light debug view: 0 off, 1 indirect diffuse light (irradiance / pi), "
    "2 indirect diffuse radiance. WMSH PBR surfaces show the map's LMAP indirect layer "
    "(black without one), PBRMetalRough models their ambient cube.",
    true, 0.0f, true, 2.0f );
// RFC 0011 render.indirect-policy.v1 for the world (CVulkanContext::SetIndirectPolicy).
static ConVar r_indirect_policy( "r_indirect_policy", "-1", FCVAR_CHEAT,
    "World indirect-light policy: -1 the producer's (BakedPlusDelta while a radiosity change "
    "volume is resident, else Baked), 0 Baked (the lightmap's total layer), 1 BakedPlusDelta "
    "(the total layer plus the producer's change), 2 RuntimeIndirect (the direct layer plus the "
    "producer's indirect; needs LMAP v2 direct and indirect layers)",
    true, -1.0f, true, 2.0f );
static ConVar r_indirect_policy_seed_double( "r_indirect_policy_seed_double", "0", FCVAR_CHEAT,
    "Sensitivity control: the RuntimeIndirect variant reads the total layer (a double count)" );
static ConVar mat_indirect_view_scale( "mat_indirect_view_scale", "1", FCVAR_CHEAT,
    "Linear exposure applied to mat_indirect_view's output.", true, 1.0e-4f, true, 1.0e4f );
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
static int g_boundEnvmapHandle = -1;
static int g_boundRefractNormalHandle = -1;
static int g_boundRefractCubeHandle = -1;
static int g_boundNormalMaskHandle = -1;
static int g_boundPbrNormalHandle = -1;
static int g_boundPbrMraoHandle = -1;
static int g_boundPbrEnvmapHandle = -1;   // sampler 3: PBRMetalRough's $envmap
static int g_boundPbrEmissionHandle = -1; // sampler 2: PBRMetalRough's $emissiontexture
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
	std::string group; // texture budget group (pTextureGroupName)
	ImageFormat format = IMAGE_FORMAT_UNKNOWN;
	uint64_t bindsWhileEmpty = 0;
	uint64_t rejectedAsTarget = 0;
	int priority = 0; // TexSetPriority
	int width = 0;
	int height = 0;
	int mipLevels = 1;
	// What mat_texture_list reports (IDebugTextureInfo), as D3D9's Texture_t
	// keeps it: the bytes of every level and face, and the binds per frame.
	int sizeBytes = 0;
	int lastBoundFrame = -1;
	int timesBoundThisFrame = 0;
	int timesBoundMax = 0;
	// CPU copy of mip 0 that TexLock hands to the material system's pixel writer
	// (lightmap pages are written this way, a sub-rectangle at a time); TexUnlock
	// uploads it. Allocated on first lock.
	std::vector<uint8_t> lockSurface;
};
static std::vector<TextureRecord> g_TextureRecords;

static void NoteTextureCreated( int handle, const char *debugName, ImageFormat format, int width,
    int height, const char *group = nullptr, int mipLevels = 1, bool cube = false )
{
	if ( handle < 0 )
		return;
	if ( static_cast<size_t>( handle ) >= g_TextureRecords.size() )
		g_TextureRecords.resize( static_cast<size_t>( handle ) + 1 );
	TextureRecord &record = g_TextureRecords[static_cast<size_t>( handle )];
	record = TextureRecord();
	record.name = debugName ? debugName : "(unnamed)";
	record.group = group ? group : "";
	record.format = format;
	record.width = width;
	record.height = height;
	record.mipLevels = std::max( 1, mipLevels );
	// D3D9 sums ImageLoader::GetMemRequired over every level (and cube face).
	for ( int level = 0; level < std::max( 1, mipLevels ); ++level )
	{
		record.sizeBytes += ImageLoader::GetMemRequired(
		    std::max( 1, width >> level ), std::max( 1, height >> level ), 1, format, false );
	}
	if ( cube )
		record.sizeBytes *= 6;
}

// IDebugTextureInfo (mat_texture_list, mat_texture_limit): the frame counter
// BeginFrame advances, the bytes of the distinct textures bound this frame, and
// the list EndFrame exports while enabled. As in CShaderAPIDx8.
static int g_CurrentFrameNumber = 0;
static int g_TextureMemoryUsedLastFrame = 0;
static int g_TextureMemoryUsedTotal = 0;
static int g_TextureMemoryUsedPicMip1 = 0;
static int g_TextureMemoryUsedPicMip2 = 0;
static bool g_bEnableDebugTextureList = false;
static bool g_bDebugGetAllTextures = false;
static bool g_bDebugTexturesRendering = false;
static KeyValues *g_pDebugTextureList = nullptr;
static int g_nDebugDataExportFrame = -1;
static ConVar mat_texture_limit( "mat_texture_limit", "-1", 0,
    "If this value is not -1, the material system will limit the amount of texture memory it "
    "uses in a frame. Useful for identifying performance cliffs. The value is in kilobytes." );

// CShaderAPIDx8::WouldBeOverTextureLimit: binding this texture would take the
// frame past mat_texture_limit (then it is not set, as on D3D9).
static bool WouldBeOverTextureLimit( int handle )
{
	if ( mat_texture_limit.GetInt() < 0 || handle < 0 ||
	     static_cast<size_t>( handle ) >= g_TextureRecords.size() )
		return false;
	const TextureRecord &record = g_TextureRecords[static_cast<size_t>( handle )];
	if ( record.lastBoundFrame == g_CurrentFrameNumber )
		return false;
	return g_TextureMemoryUsedLastFrame + record.sizeBytes > mat_texture_limit.GetInt() * 1024;
}

// The per-frame bind accounting CShaderAPIDx8::SetTextureState does.
static void NoteTextureBound( int handle )
{
	if ( handle < 0 || static_cast<size_t>( handle ) >= g_TextureRecords.size() )
		return;
	TextureRecord &record = g_TextureRecords[static_cast<size_t>( handle )];
	if ( record.lastBoundFrame != g_CurrentFrameNumber )
	{
		record.lastBoundFrame = g_CurrentFrameNumber;
		record.timesBoundThisFrame = 0;
		g_TextureMemoryUsedLastFrame += record.sizeBytes;
	}
	if ( !g_bDebugTexturesRendering )
		++record.timesBoundThisFrame;
	record.timesBoundMax = std::max( record.timesBoundMax, record.timesBoundThisFrame );
}

static void AddBufferToTextureList( const char *name, int width, int height )
{
	KeyValues *pSubKey = g_pDebugTextureList->CreateNewKey();
	pSubKey->SetString( "Name", name );
	pSubKey->SetString( "TexGroup", TEXTURE_GROUP_RENDER_TARGET );
	pSubKey->SetInt( "Size", 4 * width * height );
	pSubKey->SetString( "Format", "32 bit buffer (hack)" );
	pSubKey->SetInt( "Width", width );
	pSubKey->SetInt( "Height", height );
	pSubKey->SetInt( "BindsMax", 1 );
	pSubKey->SetInt( "BindsFrame", 1 );
}

// CShaderAPIDx8::ExportTextureList: the live textures' memory totals and, for
// the ones bound this frame (all with EnableGetAllTextures), one key each.
static void ExportTextureList()
{
	if ( !g_bEnableDebugTextureList || !g_VulkanContext.IsValid() )
		return;
	g_nDebugDataExportFrame = g_CurrentFrameNumber;
	if ( g_pDebugTextureList )
		g_pDebugTextureList->deleteThis();
	g_pDebugTextureList = new KeyValues( "TextureList" );
	g_TextureMemoryUsedTotal = 0;
	g_TextureMemoryUsedPicMip1 = 0;
	g_TextureMemoryUsedPicMip2 = 0;
	for ( size_t handle = 0; handle < g_TextureRecords.size(); ++handle )
	{
		TextureRecord &tex = g_TextureRecords[handle];
		if ( tex.name.empty() || !g_VulkanContext.ManagedTextureMipLevels( static_cast<int>( handle ) ) )
			continue;
		g_TextureMemoryUsedTotal += tex.sizeBytes;
		const int levels = static_cast<int>(
		    g_VulkanContext.ManagedTextureMipLevels( static_cast<int>( handle ) ) );
		int numBytes = tex.sizeBytes;
		if ( levels > 1 )
		{
			if ( tex.width > 4 || tex.height > 4 )
			{
				const int topMipSize =
				    ImageLoader::GetMemRequired( tex.width, tex.height, 1, tex.format, false );
				numBytes -= topMipSize;
				g_TextureMemoryUsedPicMip1 += numBytes;
				if ( tex.width > 8 || tex.height > 8 )
				{
					const int ratio = ( tex.width > 8 ? 2 : 1 ) * ( tex.height > 8 ? 2 : 1 );
					numBytes -= topMipSize / ratio;
				}
				g_TextureMemoryUsedPicMip1 += numBytes;
			}
			else
			{
				g_TextureMemoryUsedPicMip1 += numBytes;
				g_TextureMemoryUsedPicMip2 += numBytes;
			}
		}
		else
		{
			g_TextureMemoryUsedPicMip1 += numBytes;
			g_TextureMemoryUsedPicMip2 += numBytes;
		}
		if ( !g_bDebugGetAllTextures && tex.lastBoundFrame != g_CurrentFrameNumber )
			continue;
		if ( tex.lastBoundFrame != g_CurrentFrameNumber )
			tex.timesBoundThisFrame = 0;
		KeyValues *pSubKey = g_pDebugTextureList->CreateNewKey();
		pSubKey->SetString( "Name", tex.name.c_str() );
		pSubKey->SetString( "TexGroup", tex.group.c_str() );
		pSubKey->SetInt( "Size", tex.sizeBytes );
		pSubKey->SetString( "Format", ImageLoader::GetName( tex.format ) );
		pSubKey->SetInt( "Width", tex.width );
		pSubKey->SetInt( "Height", tex.height );
		pSubKey->SetInt( "BindsMax", tex.timesBoundMax );
		pSubKey->SetInt( "BindsFrame", tex.timesBoundThisFrame );
	}
	int width = 0, height = 0;
	g_VulkanContext.GetSwapchainExtent( width, height );
	AddBufferToTextureList( "BACKBUFFER", width, height );
	AddBufferToTextureList( "FRONTBUFFER", width, height );
	AddBufferToTextureList( "DEPTHBUFFER", width, height );
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
static PolygonOffsetMode_t g_CurrentPolyOffset = SHADER_POLYOFFSET_DISABLE;
static float g_CurrentAlphaRef = -1.0f;
// CVulkanContext::kColorSrgb*/kFragment*/kVertex* flags of the pass being drawn.
static int g_CurrentColorFlags = 0;
// The pass's pixel shader is vertexlit_and_unlit_generic (UnlitGeneric,
// VertexLitGeneric), whose modulation is pixel constant c1, not cModulationColor.
static bool g_CurrentModulationInPixelC1 = false;

// What a pass's vertexlit_and_unlit_generic_vs20 static combo says about vertex
// lighting: whether the shader is that one, VERTEXCOLOR (which replaces lighting
// with the vertex color) and HALFLAMBERT. Its dynamic combo (DYNAMIC_LIGHT,
// STATIC_LIGHT) comes with each draw (SetVertexShaderIndex).
struct VertexLitCombo
{
	bool vertexLit = false;
	bool vertexColor = false;
	bool halfLambert = false;
};
static VertexLitCombo g_CurrentVertexLit;

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
// The skin_ps20b static combos of the pass as skin.frag's flags, -1 when the
// pass is not VertexLitGeneric's $phong path.
static int g_CurrentSkinCombos = -1;
// Whether the pass is SolidEnergy's (solidenergy_ps20b), which reads its combos
// from its own constants c10/c11 (solidenergy_dx9_helper.cpp).
static bool g_CurrentSolidEnergy = false;
// Whether the pass is PBRMetalRough's (pbr_metalrough_world_ps). WMSH batches
// select the world pipelines in EmitToNativeQueue; every other mesh draws
// through shaders/model_pbr.frag with skin_vs20's vertex conversion.
static bool g_CurrentPbrModel = false;
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
// Model lighting, as CShaderAPIDx8 keeps it (m_DynamicState): the local lights
// SetLight enabled and the ambient cube. vertexlit_and_unlit_generic_vs20 reads
// them as cLightInfo (c27..) and cAmbientCube (c21..); this backend evaluates
// that vertex lighting where it assembles the vertices (ComputeVertexLighting).
enum
{
	kMaxLocalLights = 4 // MAX_NUM_LIGHTS; ps_2_b hardware lights models with four
};
static LightDesc_t g_LightDescs[kMaxLocalLights];
static bool g_LightEnabled[kMaxLocalLights] = {};
static float g_AmbientCube[6][4] = {};
// SetLightingOrigin: the model's origin, from which the pixel-lit shaders place
// directional lights (CommitPixelShaderLighting).
static float g_LightingOrigin[3] = {};
// The vertex shader's dynamic combo index (SetVertexShaderIndex), which selects
// vertexlit_and_unlit_generic_vs20's DYNAMIC_LIGHT and STATIC_LIGHT.
static int g_VertexShaderDynamicIndex = 0;
// The pixel shader's dynamic combo index (SetPixelShaderIndex), as D3D9's shader
// manager keeps it. -1 is the default state's "no index".
static int g_PixelShaderDynamicIndex = 0;
// Fixed-function dynamic state D3D9 keeps in m_DynamicState: the constant color
// (D3DRS_TEXTUREFACTOR, a D3DCOLOR set by Color*), the shade mode and the
// ambient light (D3DRS_AMBIENT). No DX9 shader reads them; they are held so the
// interface reports what was set.
static unsigned int g_ConstantColor = 0xFFFFFFFF;
static ShaderShadeMode_t g_ShadeMode = SHADER_SMOOTH;
static unsigned int g_AmbientLightColor = 0;
// Fog, as CShaderAPIDx8 keeps it: the scene's mode and color (SceneFogMode,
// SceneFogColor3ub), the mode in effect for the current pass (FogMode, which
// ApplyFogMode derives from the snapshot's IShaderShadow::FogMode), the fog
// range, and the register the pass's shader asked its fog parameters in
// (SetPixelShaderFogParams; D3D9's m_DelayedShaderConstants).
struct FogRenderState
{
	MaterialFogMode_t sceneMode = MATERIAL_FOG_NONE;
	unsigned char sceneColor[3] = { 0, 0, 0 };
	MaterialFogMode_t currentMode = MATERIAL_FOG_NONE;
	float start = 0.0f;
	float end = 0.0f;
	float fogZ = 0.0f;
	float maxDensity = 1.0f;
	// ApplyFogMode's color for the pass (0..1, gamma space) and whether the pass
	// asked for it without gamma correction.
	float pixelColor[3] = { 0.0f, 0.0f, 0.0f };
	bool gammaCorrectionDisabled = false;
	bool srgbWrite = false;
	int delayedParamsRegister = -1;
	// m_DestAlphaDepthRange: 192 without float HDR.
	float destAlphaDepthRange = 192.0f;
};
static FogRenderState g_Fog;
// The registers fog reaches the shaders through.
enum
{
	kVsRegCameraPos = 2,      // cEyePosWaterZ (common_vs_fxc.h)
	kVsRegFogParams = 16,     // cFogParams
	kPsRegLinearFogColor = 29 // g_LinearFogColor (common_ps_fxc.h)
};
// The shadow state's fog mode and fog gamma correction of the pass being drawn
// (BeginPass), which SetPixelShaderFogParams and ApplyFogMode read.
static ShaderFogMode_t g_CurrentShadowFogMode = SHADER_FOGMODE_DISABLED;
static bool g_CurrentDisableFogGammaCorrection = false;
namespace
{
void CommitModelViewProj();
void CommitViewProj();
// Loads identity into the fixed-function texture matrices (MATERIAL_TEXTURE0..).
void ResetTextureMatrices( int count );
// The MODEL matrix of the matrix stack (stored transposed, for row vectors).
const float *ModelMatrix();
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

static size_t SnapshotCount();
static size_t SnapshotCapacity();

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

	fprintf( stderr, "[vulkan] snapshots=%zu of %zu\n", SnapshotCount(), SnapshotCapacity() );
	fprintf( stderr, "[vulkan] fog: scene mode=%d color=%d,%d,%d start=%.1f end=%.1f z=%.1f max=%.2f\n",
	    static_cast<int>( g_Fog.sceneMode ), g_Fog.sceneColor[0], g_Fog.sceneColor[1],
	    g_Fog.sceneColor[2], g_Fog.start, g_Fog.end, g_Fog.fogZ, g_Fog.maxDensity );
	fprintf( stderr, "[vulkan] presents=%llu scaled=%llu\n",
	    static_cast<unsigned long long>( g_VulkanContext.PresentCount() ),
	    static_cast<unsigned long long>( g_VulkanContext.ScaledPresentCount() ) );
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

	// Frees the lock storage (dynamic buffers across level transitions and
	// device resource releases); the next lock allocates it again.
	void ReleaseStorage()
	{
		NoteWrite();
		std::vector<unsigned char>().swap( m_vertexData );
		std::vector<unsigned short>().swap( m_indexData );
		m_numVerts = 0;
		m_numIndices = 0;
		m_drawFirst = 0;
		m_drawCount = 0;
	}

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
	void SetWorldMeshBatch( uint32_t firstIndex, uint32_t indexCount )
	{
		m_worldMeshBatch = true;
		m_worldFirstIndex = firstIndex;
		m_worldIndexCount = indexCount;
		m_worldDrawQueued = false;
	}
	bool WorldMeshDrawQueued() const { return m_worldDrawQueued; }
	bool IsWorldMeshBatch() const { return m_worldMeshBatch; }

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

	// The static-prop color stream (engine/l_studio.cpp CColorMeshData), read as
	// the vertex shader's COLOR1 input (STATIC_LIGHT); nVertexOffset is in bytes of
	// that stream, as D3D9 binds it with a stream offset.
	void SetColorMesh( IMesh *pColorMesh, int nVertexOffset );
	bool HasColorMesh() const { return m_pColorMesh != nullptr; }
	// Vertex v's static lighting color from the bound color mesh, 0..1 per channel.
	bool StaticColor( int v, float rgb[3] ) const;
	// The format the mesh was created with. A color mesh (VERTEX_SPECULAR only)
	// holds tightly packed 4-byte D3DCOLORs, which is how the engine fills it.
	void SetVertexFormat( VertexFormat_t format ) { m_format = format; }
	bool IsColorStream() const { return m_format == VERTEX_SPECULAR; }

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
	// Changes (to a value no mesh has had) whenever this mesh hands out or takes
	// back writable storage, so a conversion of its data can be reused only while
	// the data is unchanged (EmitToNativeQueue).
	uint64_t m_revision = 0;
	void NoteWrite()
	{
		static uint64_t s_revisions = 0;
		m_revision = ++s_revisions;
	}
	// Draw-time sources (SetSources); null means this mesh's own storage.
	CEmptyMesh *m_pVertexSource = nullptr;
	CEmptyMesh *m_pIndexSource = nullptr;
	bool m_worldMeshBatch = false;
	uint32_t m_worldFirstIndex = 0;
	uint32_t m_worldIndexCount = 0;
	bool m_worldDrawQueued = false;
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
	VertexFormat_t m_format = 0;
	CEmptyMesh *m_pColorMesh = nullptr;
	int m_colorMeshOffset = 0;

public:
	// 12 bytes position + 4 bytes color + 8 bytes texcoord0 + 8 bytes texcoord1
	// + 8 bytes bone weights (two floats) + 4 bytes bone indices + 12 bytes
	// normal + 16 bytes user data (the TANGENT stream, binormal sign in w)
	// + 12 bytes each of tangent S and tangent T (brush formats' tangent frame).
	enum
	{
		kMeshVertexStride = 96,
		kMeshBoneWeightOffset = 32,
		kMeshBoneIndexOffset = 40,
		kMeshNormalOffset = 44,
		kMeshUserDataOffset = 56,
		kMeshTangentSOffset = 72,
		kMeshTangentTOffset = 84
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

void CEmptyMesh::SetColorMesh( IMesh *pColorMesh, int nVertexOffset )
{
	m_pColorMesh = pColorMesh ? AsOwnMesh( pColorMesh ) : nullptr;
	m_colorMeshOffset = nVertexOffset;
	if ( pColorMesh && ( !m_pColorMesh || !m_pColorMesh->IsColorStream() ) )
	{
		NoteUnimplemented( "SetColorMesh: color mesh other than a VERTEX_SPECULAR stream" );
		m_pColorMesh = nullptr;
	}
}

bool CEmptyMesh::StaticColor( int v, float rgb[3] ) const
{
	if ( !m_pColorMesh || v < 0 )
		return false;
	const size_t at = static_cast<size_t>( m_colorMeshOffset ) + static_cast<size_t>( v ) * 4;
	if ( at + 4 > m_pColorMesh->m_vertexData.size() )
		return false;
	// D3DCOLOR: blue, green, red, alpha in memory.
	const unsigned char *c = m_pColorMesh->m_vertexData.data() + at;
	rgb[0] = c[2] / 255.0f;
	rgb[1] = c[1] / 255.0f;
	rgb[2] = c[0] / 255.0f;
	return true;
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

	// The pass's fog, as CShaderShadowDX8 records it; ApplyFogMode turns it into
	// the fog color and parameters when the pass begins.
	virtual void FogMode( ShaderFogMode_t fogMode ) { m_fogMode = fogMode; }

	virtual void DisableFogGammaCorrection( bool bDisable )
	{
		m_disableFogGammaCorrection = bDisable;
	}

	// Fixed-function lighting's diffuse source (D3DRS_DIFFUSEMATERIALSOURCE).
	virtual void SetDiffuseMaterialSource( ShaderMaterialSource_t materialSource )
	{
		m_diffuseMaterialSource = materialSource;
	}

	virtual void SetMorphFormat( MorphFormat_t flags ) { m_morphFormat = flags; }

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
	// only when EnableAlphaTest set m_IsAlphaTested. D3D9's default is GEQUAL
	// 0.7 (CShaderShadowDX8::SetDefaultState), which a material that enables
	// the test without a reference ($alphatest, no $alphatestreference) keeps.
	float m_alphaRef = 0.7f;
	ShaderAlphaFunc_t m_alphaFunc = SHADER_ALPHAFUNC_GEQUAL;
	// IShaderShadow::EnableColorWrites.
	bool m_colorWrites = true;
	// IShaderShadow::EnableAlphaWrites; D3D9's default is off.
	bool m_alphaWrites = false;
	PolygonOffsetMode_t m_polyOffset = SHADER_POLYOFFSET_DISABLE;
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
	int m_vertexShaderIndex = 0; // the vertex shader's static combo index
	// Selected vertex shader; screenspaceeffect_vs20 positions in clip space.
	char m_vertexShaderName[64] = { 0 };
	ShaderFogMode_t m_fogMode = SHADER_FOGMODE_DISABLED;
	bool m_disableFogGammaCorrection = false;
	// Samplers the pass enabled (EnableTexture), one bit each. D3D9 sets no
	// texture on a sampler its pass did not enable.
	unsigned int m_enabledSamplers = 0;
	// Fixed-function state, recorded as CShaderShadowDX8 records it. It applies
	// only to a pass with no vertex and pixel shader, which the native
	// pipelines do not draw (RenderPass reports such passes).
	bool m_alphaPipe = false;
	bool m_constantAlpha = false;
	bool m_vertexAlpha = false;
	unsigned int m_textureAlphaStages = 0;
	bool m_constantColor = false;
	bool m_lighting = false;
	bool m_specular = false;
	bool m_vertexBlend = false;
	enum
	{
		kFixedFunctionStages = 8 // SHADER_TEXTURE_STAGE0..7
	};
	float m_overbright[kFixedFunctionStages] = {};
	bool m_customPixelPipe = false;
	int m_customStageCount = 0;
	ShaderTexOp_t m_texOp[kFixedFunctionStages][2] = {};
	ShaderTexArg_t m_texArg[kFixedFunctionStages][2][2] = {};
	unsigned int m_texGenStages = 0;
	ShaderTexGenParam_t m_texGen[kFixedFunctionStages] = {};
	unsigned int m_drawFlags = 0;
	ShaderMaterialSource_t m_diffuseMaterialSource = SHADER_MATERIALSOURCE_MATERIAL;
	MorphFormat_t m_morphFormat = 0;
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
	virtual bool IsAAEnabled() const { return g_VulkanContext.ActiveSampleCount() > 1; }
	virtual void Present()
	{
		// Present a real native Vulkan frame: the clear, any material-facing mesh
		// geometry queued this frame (IMesh::Draw), and the selected material
		// shader (BeginPass) are all recorded and submitted here. Material state,
		// textures and the full shader library remain to be wired (roadmap R32).
		if ( !g_VulkanContext.IsValid() )
			return;
		InvokePendingModeChangeCallbacks();
		g_VulkanContext.SetIndirectLightView(
		    mat_indirect_view.GetInt(), mat_indirect_view_scale.GetFloat() );
		g_VulkanContext.SetIndirectPolicy(
		    r_indirect_policy.GetInt(), r_indirect_policy_seed_double.GetBool() );
		// The engine's RFC 0011 probe-volume switches (lightcache.cpp) also
		// govern per-pixel sampling: 0 off, 1 with visibility, 2 without.
		// r_probevolume 2 keeps the volume to the ambient cube (the path
		// non-PBR model families take).
		static ConVarRef r_probevolume( "r_probevolume" );
		static ConVarRef r_probevolume_visibility( "r_probevolume_visibility" );
		g_VulkanContext.SetProbeVolumeSampling(
		    r_probevolume.IsValid() && r_probevolume.GetInt() != 1                      ? 0
		    : r_probevolume_visibility.IsValid() && !r_probevolume_visibility.GetBool() ? 2
		                                                                                : 1 );
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
	// mat_monitorgamma: SDL3 has no window gamma ramp, so the ramp D3D9 hands to
	// the hardware (render.gamma-ramp.v1) is applied when presenting. Any thread
	// may call this; the next presented frame picks the ramp up.
	virtual void SetHardwareGammaRamp( float fGamma, float fGammaTVRangeMin, float fGammaTVRangeMax,
	    float fGammaTVExponent, bool bTVEnabled )
	{
		render::GammaRampParams params;
		params.gamma = fGamma;
		params.tvRangeMin = fGammaTVRangeMin;
		params.tvRangeMax = fGammaTVRangeMax;
		params.tvExponent = fGammaTVExponent;
		params.tvEnabled = bTVEnabled;
		render::GammaRamp16 ramp;
		render::BuildGammaRamp16( params, ramp );
		g_VulkanContext.PublishGammaRamp( ramp );
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

	void ReleaseDynamicStorage()
	{
		m_Mesh.ReleaseStorage();
		m_DynamicMesh.ReleaseStorage();
	}

private:
	CEmptyMesh m_Mesh;
	CEmptyMesh m_DynamicMesh;
	// ReleaseResources / ReacquireResources pairs; only the outermost pair acts.
	int m_releaseResourcesRefCount = 0;
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
	virtual void AddModeChangeCallback( ShaderModeChangeCallbackFunc_t func );
	virtual void RemoveModeChangeCallback( ShaderModeChangeCallbackFunc_t func );
	void InvokeModeChangeCallbacks();

private:
	void ClampToCapabilities( KeyValues *pKeyValues ) const;

	CUtlVector<ShaderModeChangeCallbackFunc_t> m_ModeChangeCallbacks;
	IFileSystem *m_pFileSystem = NULL;
	KeyValues *m_pDXSupport = NULL;
	bool m_bDXSupportRead = false;
	// The desktop of the display the launcher selected (sdl_displayindex). The
	// launcher owns display selection; false when there is none (tools, tests).
	bool QueryDesktopDisplay( render::DisplayModeFacts *pDesktop ) const;
	void RefreshModeList() const;
	static void ToShaderDisplayMode(
	    const render::DisplayModeFacts &mode, ShaderDisplayMode_t *pInfo );

#if defined( USE_SDL )
	ILauncherMgr *m_pLauncherMgr = NULL;
#endif
	// Rebuilt by GetModeCount, which the engine calls before GetModeInfo.
	mutable std::vector<render::DisplayModeFacts> m_Modes;
	mutable bool m_bWarnedNoDisplay = false;
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

	// IDebugTextureInfo implementation (CShaderAPIDx8's; see ExportTextureList).
public:
	virtual bool IsDebugTextureListFresh( int numFramesAllowed = 1 )
	{
		return g_nDebugDataExportFrame <= g_CurrentFrameNumber &&
		       g_nDebugDataExportFrame >= g_CurrentFrameNumber - numFramesAllowed;
	}
	virtual bool SetDebugTextureRendering( bool bEnable )
	{
		const bool previous = g_bDebugTexturesRendering;
		g_bDebugTexturesRendering = bEnable;
		return previous;
	}
	virtual void EnableDebugTextureList( bool bEnable ) { g_bEnableDebugTextureList = bEnable; }
	virtual void EnableGetAllTextures( bool bEnable ) { g_bDebugGetAllTextures = bEnable; }
	virtual KeyValues *GetDebugTextureList() { return g_pDebugTextureList; }
	virtual int GetTextureMemoryUsed( TextureMemoryType eTextureMemory )
	{
		switch ( eTextureMemory )
		{
		case MEMORY_BOUND_LAST_FRAME:
			return g_TextureMemoryUsedLastFrame;
		case MEMORY_TOTAL_LOADED:
			return g_TextureMemoryUsedTotal;
		case MEMORY_ESTIMATE_PICMIP_1:
			return g_TextureMemoryUsedPicMip1;
		case MEMORY_ESTIMATE_PICMIP_2:
			return g_TextureMemoryUsedPicMip2;
		default:
			return 0;
		}
	}

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
	// As CMeshMgr::MarkUnusedVertexFields: the vertex fields and texture
	// coordinates the next draws' shader reads but their mesh need not supply,
	// which D3D9 feeds from a zero stream. The native vertex records hold zeros
	// for every such field (texture coordinates past the second included).
	virtual void MarkUnusedVertexFields(
	    unsigned int nFlags, int nTexCoordCount, bool *pUnusedTexCoords )
	{
		m_nUnusedVertexFields = nFlags;
		m_nUnusedTextureCoords = 0;
		for ( int i = 0; pUnusedTexCoords && i < nTexCoordCount && i < 32; ++i )
		{
			if ( pUnusedTexCoords[i] )
				m_nUnusedTextureCoords |= 1u << i;
		}
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
		// Vulkan device/surface/swapchain against the engine's window and the
		// material-facing dynamic-mesh pipelines here.
		// The back buffer is the video mode's size, as D3D9's BackBufferWidth/Height;
		// Present scales it to the window's drawable.
		std::string error;
		if ( !g_VulkanContext.SetBackBufferSize(
		         info.m_DisplayMode.m_nWidth, info.m_DisplayMode.m_nHeight, &error ) )
			Warning( "[NativeVulkan] back buffer resize failed: %s\n", error.c_str() );
		g_VulkanContext.RequestVSync( info.m_bWaitForVSync );
		g_VulkanContext.RequestSampleCount( info.m_nAASamples );
		if ( g_VulkanContext.IsValid() )
			return true;

		render_vulkan::VulkanContextConfig config;
		config.appName = "Source Engine Native Vulkan";
		config.enableValidation = ( CommandLine()->FindParm( "-vkvalidate" ) != 0 );
		config.framesInFlight = 2;
		config.vsync = info.m_bWaitForVSync;

		if ( !InitVulkanContext( hwnd, config, &error ) )
		{
			Warning( "[NativeVulkan] IShaderAPI::SetMode bring-up failed: %s\n", error.c_str() );
			return false;
		}
		int w = 0, h = 0, presentW = 0, presentH = 0;
		g_VulkanContext.GetSwapchainExtent( w, h );
		g_VulkanContext.GetPresentExtent( presentW, presentH );
		Msg( "[NativeVulkan] IShaderAPI::SetMode: device '%s' up (back buffer %dx%d, window "
		     "%dx%d)\n",
		    g_VulkanContext.DeviceName(), w, h, presentW, presentH );
		if ( !g_VulkanContext.InitDynamicMesh( &error ) )
			Warning( "[NativeVulkan] dynamic mesh pipelines unavailable: %s\n", error.c_str() );
		else
			PrewarmVulkanPipelines();
		return true;
	}

	// A new video mode (a window resize, mat_setvideomode) sizes the back buffer,
	// as CShaderAPIDx8::ChangeVideoMode does; the window keeps its drawable.
	void ChangeVideoMode( const ShaderDeviceInfo_t &info )
	{
		std::string error;
		if ( !g_VulkanContext.SetBackBufferSize(
		         info.m_DisplayMode.m_nWidth, info.m_DisplayMode.m_nHeight, &error ) )
			Warning( "[NativeVulkan] ChangeVideoMode: %s\n", error.c_str() );
		g_VulkanContext.RequestVSync( info.m_bWaitForVSync );
		g_VulkanContext.RequestSampleCount( info.m_nAASamples );
		if ( !info.m_bResizing )
			g_bPendingModeChangeCallbacks = true;
	}

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
	void SetPixelShaderStateAmbientLightCube( int pshReg, bool bForceToBlack = false );

	float GetAmbientLightCubeLuminance( void ) { return 0.0f; }

	void SetSkinningMatrices();

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
	// The matrix of the current matrix mode, as stored (transposed).
	void GetCurrentMatrix( float *dst );
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
	virtual void CommitPixelShaderLighting( int pshReg );

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

	// As CShaderAPIDx8: the pass's fog parameters into pixel constant reg
	// (cFogEndOverFogRange, water height, max density, 1 / range), remembered so
	// a later ApplyFogMode rewrites them.
	void SetPixelShaderFogParams( int reg );
	void SetPixelShaderFogParams( int reg, ShaderFogMode_t fogMode );
	// CShaderAPIDx8::ApplyFogMode: the pass's fog color (c29) and parameters from
	// its shadow fog mode and the scene fog.
	void ApplyFogMode( ShaderFogMode_t fogMode, bool bSRGBWritesEnabled, bool bDisableFogGammaCorrection );
	void UpdatePixelFogColorConstant();
	MaterialFogMode_t GetPixelFogMode() const;
	void UpdateVertexShaderFogParams();

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

	virtual bool IsAAEnabled() const { return g_VulkanContext.ActiveSampleCount() > 1; }

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
		// D3D9 publishes this same state to c30. Material shaders use z for
		// cubemap energy even when HDR is disabled.
		g_psConstants[30][0] = g_ToneMappingScale.x;
		g_psConstants[30][1] = GetLightMapScaleFactor();
		g_psConstants[30][2] = CurrentHDRType() == HDR_TYPE_INTEGER ? 16.0f : 1.0f;
		g_psConstants[30][3] = powf( g_ToneMappingScale.x, 1.0f / 2.2f );
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

	void SetFloatRenderingParameter( int parm_number, float value )
	{
		if ( parm_number >= 0 && parm_number < MAX_FLOAT_RENDER_PARMS )
			m_FloatRenderingParameters[parm_number] = value;
	}

	void SetIntRenderingParameter( int parm_number, int value )
	{
		if ( parm_number >= 0 && parm_number < MAX_INT_RENDER_PARMS )
			m_IntRenderingParameters[parm_number] = value;
	}
	void SetVectorRenderingParameter( int parm_number, Vector const &value )
	{
		if ( parm_number >= 0 && parm_number < MAX_VECTOR_RENDER_PARMS )
			m_VectorRenderingParameters[parm_number] = value;
	}

	float GetFloatRenderingParameter( int parm_number ) const
	{
		return parm_number >= 0 && parm_number < MAX_FLOAT_RENDER_PARMS
		           ? m_FloatRenderingParameters[parm_number]
		           : 0.0f;
	}

	int GetIntRenderingParameter( int parm_number ) const
	{
		return parm_number >= 0 && parm_number < MAX_INT_RENDER_PARMS
		           ? m_IntRenderingParameters[parm_number]
		           : 0;
	}

	Vector GetVectorRenderingParameter( int parm_number ) const
	{
		return parm_number >= 0 && parm_number < MAX_VECTOR_RENDER_PARMS
		           ? m_VectorRenderingParameters[parm_number]
		           : Vector( 0, 0, 0 );
	}

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

	virtual void DisableAllLocalLights()
	{
		for ( bool &enabled : g_LightEnabled )
			enabled = false;
	}

	// mat_antialias modes the device can honour exactly (render.sample-count.v1).
	virtual bool SupportsMSAAMode( int nMSAAMode )
	{
		return render::IsMsaaModeSupported( nMSAAMode, CurrentAdapterCaps().backBufferSampleMask );
	}

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
	// D3D9 reports DX10-style blending (sRGB writes blended in linear space) on
	// every current GPU; this backend blends so whenever its attachments are sRGB.
	virtual bool UsesSRGBCorrectBlending() const
	{
		return g_VulkanContext.LinearSpaceSrgbBlending();
	}

	virtual bool HasFastVertexTextures() const { return false; }

	virtual void SetShadowDepthBiasFactors(
	    float fShadowSlopeScaleDepthBias, float fShadowDepthBias )
	{
		m_shadowSlopeScaleDepthBias = fShadowSlopeScaleDepthBias;
		m_shadowDepthBias = fShadowDepthBias;
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
	float m_shadowSlopeScaleDepthBias = 0.0f;
	float m_shadowDepthBias = 0.0f;
	void ApplyDepthBiasState( render_vulkan::CVulkanContext::DynRasterState &raster );
	// OverrideDepthEnable: D3D9 forces the Z test on and the Z write to the given
	// value for every draw until the override ends (TransitionTable's
	// PerformShadowStateOverrides). VGUI screens in the world use it to depth-test
	// their $ignorez materials.
	bool m_bOverrideDepthEnable = false;
	bool m_bOverrideDepthWrite = false;
	bool m_bForceDepthFuncEquals = false;
	bool m_bOverrideAlphaWrite = false;
	bool m_bOverriddenAlphaWrite = false;
	bool m_bOverrideColorWrite = false;
	bool m_bOverriddenColorWrite = false;
	unsigned int m_nUnusedVertexFields = 0;
	unsigned int m_nUnusedTextureCoords = 0;
	// SetBooleanVertexShaderConstant and the other non-float banks, as D3D9's
	// m_DesiredState holds them (16 registers each on shader model 3 hardware).
	enum
	{
		kShaderBoolIntRegisters = 16
	};
	BOOL m_vsBoolConstants[kShaderBoolIntRegisters] = {};
	int m_vsIntConstants[kShaderBoolIntRegisters][4] = {};
	BOOL m_psBoolConstants[kShaderBoolIntRegisters] = {};
	int m_psIntConstants[kShaderBoolIntRegisters][4] = {};
	// SetLinearToGammaConversionTextures: the tables D3D9 binds to sampler 15 for
	// shaders that convert to sRGB themselves (NeedsShaderSRGBConversion, which
	// this backend reports false).
	ShaderAPITextureHandle_t m_hLinearToGammaTableTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
	ShaderAPITextureHandle_t m_hLinearToGammaTableIdentityTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
	bool m_bHWMorphingEnabled = false;
	// Fixed-function texture stage state (SetTextureTransformDimension,
	// SetBumpEnvMatrix), identity by default.
	struct TextureStageState
	{
		int transformFlags = 0;
		float bumpEnv[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	};
	TextureStageState m_TextureStages[8];
	void ApplyShadowStateOverrides( render_vulkan::CVulkanContext::DynRasterState &raster ) const;
	float m_FloatRenderingParameters[MAX_FLOAT_RENDER_PARMS];
	int m_IntRenderingParameters[MAX_INT_RENDER_PARMS];
	Vector m_VectorRenderingParameters[MAX_VECTOR_RENDER_PARMS];

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
// Vulkan guarantees formats such as sampled sRGB, but this backend claims a
// semantic feature only once its own path implements and tests it (R32).
static bool DescribeNativeVulkanAdapter( int adapter, render::RenderAdapterInfo *info )
{
	if ( adapter != 0 || !info )
		return false;
	*info = render::RenderAdapterInfo();
	Q_strncpy( info->driverApi, "vulkan", sizeof( info->driverApi ) );
	return true;
}

static bool DrawWorldMaterialBatch( uint32_t firstIndex, uint32_t indexCount )
{
	CEmptyMesh mesh( false );
	mesh.SetWorldMeshBatch( firstIndex, indexCount );
	mesh.Draw( 0, static_cast<int>( indexCount ) );
	return mesh.WorldMeshDrawQueued();
}

static render_vulkan::CVulkanWorldMeshUpload g_WorldMeshUpload(
    g_VulkanContext, DrawWorldMaterialBatch );

// RFC 0011 G2: the engine's per-frame light set. WMSH PBR adds its unbaked
// point and spot lights as direct light (the first kMaxDirectLights of them;
// baked lights are already in the bake, and directional ones have no dynamic
// source today).
class CVulkanLightSetConsumer final : public light_set::ILightSetConsumer
{
public:
	void PublishLightSet( const light_set::Snapshot &snapshot ) override
	{
		using Direct = render_vulkan::CVulkanContext::DirectLight;
		Direct lights[render_vulkan::CVulkanContext::kMaxDirectLights];
		uint32_t count = 0;
		for ( const light_set::RuntimeLight &light : snapshot.lights )
		{
			if ( light.baked || light.shape == light_set::LightShape::Directional ||
			     count == render_vulkan::CVulkanContext::kMaxDirectLights )
				continue;
			Direct &out = lights[count++];
			out = Direct();
			for ( int k = 0; k < 3; ++k )
			{
				out.position[k] = light.position[k];
				out.color[k] = light.color[k];
				out.direction[k] = light.direction[k];
			}
			out.radius = light.radius;
			out.minLight = light.minLight;
			if ( light.shape == light_set::LightShape::Spot )
			{
				out.outerCos = light.outerCos;
				out.innerCos = light.innerCos;
			}
		}
		g_VulkanContext.SetDirectLights( lights, count );
	}
};

static CVulkanLightSetConsumer g_LightSetConsumer;

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
	services->worldMeshUpload = &g_WorldMeshUpload;
	services->lightSetConsumer = &g_LightSetConsumer;
	services->describeAdapter = DescribeNativeVulkanAdapter;
	return true;
}

extern "C" DLL_EXPORT bool NativeVulkanShaderBackend_Create(
    render::LegacyShaderServices *services )
{
	return CreateNativeVulkanShaderBackend( services );
}

extern "C" DLL_EXPORT const render::LegacyShaderProvider *NativeVulkanShaderBackend_Describe()
{
	// Not queued: ComputeVertexDescription is unimplemented, and the fixed mesh
	// layout seeds defaults (white color, unit bone weight) that a replayed
	// queued vertex buffer would overwrite.
	static const render::LegacyShaderProvider provider = {
	    "native-vulkan", "shaderapivulkan", NativeVulkanShaderBackend_Create, false };
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

#if defined( USE_SDL )
	// Optional: application roots without a launcher (tools, tests) get no modes.
	m_pLauncherMgr = (ILauncherMgr *)factory( SDLMGR_INTERFACE_VERSION, NULL );
#endif
	// Optional: without a file system there is no dxsupport.cfg to recommend from.
	m_pFileSystem = (IFileSystem *)factory( FILESYSTEM_INTERFACE_VERSION, NULL );
	return true;
}

void CShaderDeviceMgrVulkan::Disconnect()
{
	g_pShaderUtil = NULL;
#if defined( USE_SDL )
	m_pLauncherMgr = NULL;
#endif
	m_Modes.clear();
	m_pFileSystem = NULL;
	if ( m_pDXSupport )
		m_pDXSupport->deleteThis();
	m_pDXSupport = NULL;
	m_bDXSupportRead = false;
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
	g_VulkanSurfaceHost.reset();
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

	// hWnd is the engine window; the SDL3-Vulkan bridge interprets it. Bring up
	// the real native Vulkan device/surface/swapchain against it, with a back
	// buffer of the mode's size (0 x 0 follows the window).
	std::string sizeError;
	if ( !g_VulkanContext.SetBackBufferSize(
	         mode.m_DisplayMode.m_nWidth, mode.m_DisplayMode.m_nHeight, &sizeError ) )
		Warning( "[NativeVulkan] back buffer resize failed: %s\n", sizeError.c_str() );
	g_VulkanContext.RequestVSync( mode.m_bWaitForVSync );
	g_VulkanContext.RequestSampleCount( mode.m_nAASamples );
	if ( !g_VulkanContext.IsValid() )
	{
		render_vulkan::VulkanContextConfig config;
		config.appName = "Source Engine Native Vulkan";
		config.enableValidation = ( CommandLine()->FindParm( "-vkvalidate" ) != 0 );
		config.framesInFlight = 2;
		config.vsync = mode.m_bWaitForVSync;

		std::string error;
		if ( InitVulkanContext( hWnd, config, &error ) )
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
			else
				PrewarmVulkanPipelines();
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

// dxsupport.cfg (the file the D3D9 device reads) through the shared
// render.dxsupport-policy.v1 owner, for this adapter's identity, then held to
// what this backend supports: a recommendation never names a mode the Video
// options could not apply.
bool CShaderDeviceMgrVulkan::GetRecommendedConfigurationInfo(
    int nAdapter, int nDXLevel, KeyValues *pKeyValues )
{
	const int nLevel = render::ClosestActualDxLevel(
	    nDXLevel != 0 ? nDXLevel : NativeCapsDxLevel(), false, ABSOLUTE_MINIMUM_DXLEVEL );
	if ( nLevel > NativeCapsDxLevel() )
		return false;

	if ( !m_pDXSupport && !m_bDXSupportRead )
	{
		m_bDXSupportRead = true;
		m_pDXSupport =
		    dxsupport::ReadConfig( m_pFileSystem, "dxsupport.cfg", "dxsupport_override.cfg" );
	}
	if ( !m_pDXSupport )
		return true;

	const render_vulkan::VulkanAdapterCaps &caps = CurrentAdapterCaps();
	render::DxSupportQuery query;
	query.dxLevel = nLevel;
	query.maxDxLevel = NativeCapsDxLevel();
	query.vendorId = static_cast<int>( caps.vendorId );
	query.deviceId = static_cast<int>( caps.deviceId );
	query.videoMemoryBytes = g_ShaderAPIEmpty.TextureMemorySize();
	dxsupport::FillHostFacts( &query );
	dxsupport::ApplyRecommendedConfig( m_pDXSupport, query, pKeyValues );
	ClampToCapabilities( pKeyValues );
	return true;
}

void CShaderDeviceMgrVulkan::ClampToCapabilities( KeyValues *pKeyValues ) const
{
	uint32_t sampleMask = 1;
	for ( int samples = 2; samples <= render::kMaxSampleCount; samples *= 2 )
	{
		if ( g_ShaderAPIEmpty.SupportsMSAAMode( samples ) )
			sampleMask |= static_cast<uint32_t>( samples );
	}
	if ( KeyValues *pAA = pKeyValues->FindKey( "ConVar.mat_antialias" ) )
	{
		const int samples = render::ClampSampleCount( pAA->GetInt(), sampleMask );
		if ( samples != pAA->GetInt() && pAA->GetInt() > 1 )
		{
			pKeyValues->SetInt( "ConVar.mat_antialias", samples );
			pKeyValues->SetInt( "ConVar.mat_aaquality", 0 );
		}
	}
	if ( !g_ShaderAPIEmpty.SupportsShadowDepthTextures() )
		pKeyValues->SetInt( "ConVar.r_flashlightdepthtexture", 0 );
	if ( KeyValues *pHDR = pKeyValues->FindKey( "ConVar.mat_hdr_level" ) )
	{
		if ( pHDR->GetInt() > 2 )
			pKeyValues->SetInt( "ConVar.mat_hdr_level", 2 );
	}
}

// Returns info about each adapter
void CShaderDeviceMgrVulkan::GetAdapterInfo( int adapter, MaterialAdapterInfo_t &info ) const
{
	memset( &info, 0, sizeof( info ) );
	const render_vulkan::VulkanAdapterCaps &caps = CurrentAdapterCaps();
	Q_strncpy( info.m_pDriverName, caps.valid ? caps.name.c_str() : "Native Vulkan",
	    sizeof( info.m_pDriverName ) );
	info.m_VendorID = caps.vendorId;
	info.m_DeviceID = caps.deviceId;
	info.m_nDXSupportLevel = NativeCapsDxLevel();
	info.m_nMaxDXSupportLevel = NativeCapsDxLevel();
	info.m_nDriverVersionHigh = caps.driverVersion;
	info.m_nDriverVersionLow = 0;
}

void CShaderDeviceMgrVulkan::AddModeChangeCallback( ShaderModeChangeCallbackFunc_t func )
{
	Assert( func && m_ModeChangeCallbacks.Find( func ) < 0 );
	m_ModeChangeCallbacks.AddToTail( func );
}

void CShaderDeviceMgrVulkan::RemoveModeChangeCallback( ShaderModeChangeCallbackFunc_t func )
{
	m_ModeChangeCallbacks.FindAndRemove( func );
}

void CShaderDeviceMgrVulkan::InvokeModeChangeCallbacks()
{
	for ( int i = 0; i < m_ModeChangeCallbacks.Count(); ++i )
		m_ModeChangeCallbacks[i]();
}

static void InvokePendingModeChangeCallbacks()
{
	if ( !g_bPendingModeChangeCallbacks )
		return;
	g_bPendingModeChangeCallbacks = false;
	if ( ThreadInMainThread() )
		s_ShaderDeviceMgrEmpty.InvokeModeChangeCallbacks();
}

bool CShaderDeviceMgrVulkan::QueryDesktopDisplay( render::DisplayModeFacts *pDesktop ) const
{
	*pDesktop = render::DisplayModeFacts();
#if defined( USE_SDL )
	if ( m_pLauncherMgr )
	{
		uint width = 0, height = 0, refreshHz = 0;
		m_pLauncherMgr->GetNativeDisplayInfo( -1, width, height, refreshHz );
		pDesktop->width = static_cast<int>( width );
		pDesktop->height = static_cast<int>( height );
		pDesktop->refreshNumerator = static_cast<int>( refreshHz );
		pDesktop->refreshDenominator = 1;
	}
#endif
	if ( pDesktop->width > 0 && pDesktop->height > 0 )
		return true;
	if ( !m_bWarnedNoDisplay )
	{
		Warning( "[NativeVulkan] no desktop display to enumerate video modes from\n" );
		m_bWarnedNoDisplay = true;
	}
	return false;
}

// SDL3 fullscreen covers the desktop and presentation scales the back buffer
// to it, so the modes are back-buffer sizes that fit the desktop
// (render.display-modes.v1), all at the desktop refresh rate.
void CShaderDeviceMgrVulkan::RefreshModeList() const
{
	render::DisplayModeFacts desktop;
	std::vector<render::DisplayModeFacts> modes = QueryDesktopDisplay( &desktop )
	                                                  ? render::BuildBackBufferModeList( desktop )
	                                                  : std::vector<render::DisplayModeFacts>();
	if ( !modes.empty() &&
	     ( modes.size() != m_Modes.size() || modes.back().width != m_Modes.back().width ||
	         modes.back().height != m_Modes.back().height ) )
		Msg( "[NativeVulkan] %d video modes for desktop %dx%d@%d (%dx%d .. %dx%d)\n",
		    static_cast<int>( modes.size() ), desktop.width, desktop.height,
		    desktop.refreshNumerator, modes.front().width, modes.front().height, modes.back().width,
		    modes.back().height );
	m_Modes = std::move( modes );
}

void CShaderDeviceMgrVulkan::ToShaderDisplayMode(
    const render::DisplayModeFacts &mode, ShaderDisplayMode_t *pInfo )
{
	pInfo->m_nWidth = mode.width;
	pInfo->m_nHeight = mode.height;
	pInfo->m_Format = IMAGE_FORMAT_BGRA8888;
	pInfo->m_nRefreshRateNumerator = mode.refreshNumerator;
	pInfo->m_nRefreshRateDenominator = mode.refreshDenominator;
}

// Returns the number of modes
int CShaderDeviceMgrVulkan::GetModeCount( int nAdapter ) const
{
	RefreshModeList();
	return static_cast<int>( m_Modes.size() );
}

// Returns mode information..
void CShaderDeviceMgrVulkan::GetModeInfo(
    ShaderDisplayMode_t *pInfo, int nAdapter, int nMode ) const
{
	if ( !pInfo )
		return;
	if ( m_Modes.empty() )
		RefreshModeList();
	if ( nMode < 0 || nMode >= static_cast<int>( m_Modes.size() ) )
	{
		Warning( "[NativeVulkan] GetModeInfo: mode %d of %d requested\n", nMode,
		    static_cast<int>( m_Modes.size() ) );
		ToShaderDisplayMode( render::DisplayModeFacts(), pInfo );
		pInfo->m_nRefreshRateDenominator = 0;
		return;
	}
	ToShaderDisplayMode( m_Modes[nMode], pInfo );
}

// The display's current mode is the desktop's: fullscreen never changes it.
void CShaderDeviceMgrVulkan::GetCurrentModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter ) const
{
	if ( !pInfo )
		return;
	render::DisplayModeFacts desktop;
	QueryDesktopDisplay( &desktop );
	ToShaderDisplayMode( desktop, pInfo );
	if ( desktop.width <= 0 )
		pInfo->m_nRefreshRateDenominator = 0;
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

// As CShaderDeviceDx8: the material system releases everything it created
// through this API (textures, render targets, lightmap pages, standard textures)
// and the dynamic buffers are freed; ReacquireResources has it recreate them in
// the configuration current then (a changed HDR mode reformats the lightmap
// pages). A Vulkan device is never lost, so nothing else is reset. Only the
// outermost of nested pairs acts.
void CShaderDeviceVulkan::ReleaseResources()
{
	if ( m_releaseResourcesRefCount++ != 0 )
	{
		Warning( "ReleaseResources has no effect, now at level %d.\n", m_releaseResourcesRefCount );
		return;
	}
	if ( g_pShaderUtil )
		g_pShaderUtil->ReleaseShaderObjects();
	ReleaseDynamicStorage();
}

void CShaderDeviceVulkan::ReacquireResources()
{
	if ( --m_releaseResourcesRefCount != 0 )
	{
		Warning( "ReacquireResources has no effect, now at level %d.\n", m_releaseResourcesRefCount );
		if ( m_releaseResourcesRefCount < 0 )
			m_releaseResourcesRefCount = 0;
		return;
	}
	if ( g_pShaderUtil )
		g_pShaderUtil->RestoreShaderObjects( ShaderInterfaceFactory );
}

// Creates/destroys Mesh
IMesh *CShaderDeviceVulkan::CreateStaticMesh(
    VertexFormat_t fmt, const char *pTextureBudgetGroup, IMaterial *pMaterial )
{
	// A static mesh is built once and drawn many times, so it needs storage of
	// its own; the caller owns it until DestroyStaticMesh.
	CEmptyMesh *pMesh = new CEmptyMesh( false );
	pMesh->SetVertexFormat( fmt );
	return pMesh;
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
	NoteWrite();
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
	NoteWrite();
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
	NoteWrite();
	if ( nWrittenIndexCount >= 0 && nWrittenIndexCount <= m_numIndices )
		m_numIndices = nWrittenIndexCount;
}

void CEmptyMesh::ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t &desc )
{
	// Modify the existing list in place; relocking would discard it.
	NoteWrite();
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
	NoteWrite();
}

void CEmptyMesh::Spew( int nIndexCount, const IndexDesc_t &desc )
{
}

void CEmptyMesh::ValidateData( int nIndexCount, const IndexDesc_t &desc )
{
}

bool CEmptyMesh::Lock( int nVertexCount, bool bAppend, VertexDesc_t &desc )
{
	NoteWrite();
	if ( IsColorStream() )
	{
		// A static-prop color mesh: one D3DCOLOR per vertex, tightly packed, as
		// D3D9's VERTEX_SPECULAR vertex buffer is. The engine copies baked colors
		// straight to the specular pointer (CModelRender static prop colors).
		nVertexCount =
		    std::max( 0, std::min( nVertexCount, static_cast<int>( kMaxLockVertices ) ) );
		m_numVerts = nVertexCount;
		const size_t colorBytes = ( static_cast<size_t>( nVertexCount ) + 1 ) * 4;
		if ( m_vertexData.size() < colorBytes )
			m_vertexData.resize( colorBytes );
		memset( &desc, 0, sizeof( desc ) );
		desc.m_pPosition = (float *)m_dummyComponent;
		desc.m_pBoneWeight = (float *)m_dummyComponent;
		desc.m_pBoneMatrixIndex = m_dummyComponent;
		desc.m_pNormal = (float *)m_dummyComponent;
		desc.m_pColor = m_dummyComponent;
		desc.m_pSpecular = m_vertexData.data();
		desc.m_VertexSize_Specular = 4;
		for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
			desc.m_pTexCoord[i] = (float *)m_dummyComponent;
		desc.m_pTangentS = (float *)m_dummyComponent;
		desc.m_pTangentT = (float *)m_dummyComponent;
		desc.m_pUserData = (float *)m_dummyComponent;
		desc.m_ActualVertexSize = 4;
		return true;
	}

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
		memset( vertex + kMeshTangentSOffset, 0, 24 );
	}

	desc.m_pPosition = (float *)( vertexMemory );
	desc.m_pColor = vertexMemory + 12;
	desc.m_VertexSize_Position = kMeshVertexStride;
	desc.m_VertexSize_Color = kMeshVertexStride;
	// Model meshes read static lighting from a separate color mesh (SetColorMesh).
	desc.m_pSpecular = m_dummyComponent;
	desc.m_VertexSize_Specular = 0;

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
	desc.m_pTangentS = (float *)( vertexMemory + kMeshTangentSOffset );
	desc.m_pTangentT = (float *)( vertexMemory + kMeshTangentTOffset );
	desc.m_pUserData = (float *)( vertexMemory + kMeshUserDataOffset );
	desc.m_NumBoneWeights = 2;

	desc.m_VertexSize_BoneWeight = kMeshVertexStride;
	desc.m_VertexSize_BoneMatrixIndex = kMeshVertexStride;
	desc.m_VertexSize_Normal = kMeshVertexStride;
	desc.m_VertexSize_TangentS = kMeshVertexStride;
	desc.m_VertexSize_TangentT = kMeshVertexStride;
	desc.m_VertexSize_UserData = kMeshVertexStride;
	desc.m_ActualVertexSize = kMeshVertexStride;

	desc.m_nFirstVertex = 0;
	desc.m_nOffset = 0;
	return true;
}

void CEmptyMesh::Unlock( int nVertexCount, VertexDesc_t &desc )
{
	NoteWrite();
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
	NoteWrite();
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
	NoteWrite();
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
		vdesc.m_pTangentS = (float *)m_dummyComponent;
		vdesc.m_VertexSize_TangentS = 0;
		vdesc.m_pTangentT = (float *)m_dummyComponent;
		vdesc.m_VertexSize_TangentT = 0;
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
		vdesc.m_pTangentS = (float *)( base + kMeshTangentSOffset );
		vdesc.m_pTangentT = (float *)( base + kMeshTangentTOffset );
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
	NoteWrite();
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
static void CommitPassPixelConstants();

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

static void CrossProduct3( const float *a, const float *b, float *out )
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

// common_vs_fxc.h SkinPositionAndNormal's normal: blended through the bones'
// rotations with skinning, else through cModel[0] (the MODEL matrix, stored
// here transposed for row vectors). Not normalized.
static void WorldNormal( const unsigned char *vertex, const float normal[3], float out[3] )
{
	if ( g_NumBoneWeights > 0 )
	{
		float weights[3];
		memcpy( weights, vertex + CEmptyMesh::kMeshBoneWeightOffset, 2 * sizeof( float ) );
		weights[2] = 1.0f - ( weights[0] + weights[1] );
		const unsigned char *indices = vertex + CEmptyMesh::kMeshBoneIndexOffset;
		out[0] = out[1] = out[2] = 0.0f;
		for ( int b = 0; b < 3; ++b )
		{
			const float *m = g_BoneMatrices[indices[b] < kMaxBoneMatrices ? indices[b] : 0];
			for ( int r = 0; r < 3; ++r )
				out[r] += weights[b] * ( m[r * 4] * normal[0] + m[r * 4 + 1] * normal[1] +
				                           m[r * 4 + 2] * normal[2] );
		}
		return;
	}
	const float *m = ModelMatrix();
	for ( int j = 0; j < 3; ++j )
		out[j] = normal[0] * m[j] + normal[1] * m[4 + j] + normal[2] * m[8 + j];
}

// A model-space position through the MODEL matrix (rigid draws).
static void ModelToWorld( float pos[3] )
{
	const float *m = ModelMatrix();
	float world[3];
	for ( int j = 0; j < 3; ++j )
		world[j] = pos[0] * m[j] + pos[1] * m[4 + j] + pos[2] * m[8 + j] + m[12 + j];
	memcpy( pos, world, sizeof( world ) );
}

// One cLightInfo entry, as CShaderAPIDx8::CommitVertexShaderLighting loads it.
struct VertexLightConstants
{
	float color[4]; // w: 1 for a directional light
	float dir[4];   // w: 1 for a spot light
	float pos[3];
	float spot[4]; // exponent, stopdot, stopdot2, 1 / ( stopdot - stopdot2 )
	float atten[3];
};

// The enabled lights sorted as CShaderAPIDx8::SortLights orders them (spot,
// point, directional; stable), with CShaderAPIDx8::SetLight's cone adjustment.
static int BuildVertexLightConstants( VertexLightConstants out[kMaxLocalLights] )
{
	auto typeOrder = []( LightType_t type )
	{
		return type == MATERIAL_LIGHT_SPOT ? 0 : type == MATERIAL_LIGHT_POINT ? 1 : 2;
	};
	int order[kMaxLocalLights];
	int count = 0;
	for ( int i = 0; i < kMaxLocalLights; ++i )
	{
		if ( !g_LightEnabled[i] )
			continue;
		int j = count;
		while ( j > 0 && typeOrder( g_LightDescs[order[j - 1]].m_Type ) >
		                     typeOrder( g_LightDescs[i].m_Type ) )
		{
			order[j] = order[j - 1];
			--j;
		}
		order[j] = i;
		++count;
	}
	for ( int n = 0; n < count; ++n )
	{
		const LightDesc_t &desc = g_LightDescs[order[n]];
		VertexLightConstants &c = out[n];
		const bool directional = desc.m_Type == MATERIAL_LIGHT_DIRECTIONAL;
		const bool spot = desc.m_Type == MATERIAL_LIGHT_SPOT;
		c.color[0] = desc.m_Color.x;
		c.color[1] = desc.m_Color.y;
		c.color[2] = desc.m_Color.z;
		c.color[3] = directional ? 1.0f : 0.0f;
		c.dir[0] = desc.m_Direction.x;
		c.dir[1] = desc.m_Direction.y;
		c.dir[2] = desc.m_Direction.z;
		c.dir[3] = spot ? 1.0f : 0.0f;
		c.pos[0] = desc.m_Position.x;
		c.pos[1] = desc.m_Position.y;
		c.pos[2] = desc.m_Position.z;
		if ( spot )
		{
			float phi = desc.m_Phi > M_PI ? static_cast<float>( M_PI ) : desc.m_Phi;
			float theta = desc.m_Theta;
			if ( theta - phi > -1e-3f )
				theta = phi - 1e-3f;
			const float stopdot = cosf( theta * 0.5f );
			const float stopdot2 = cosf( phi * 0.5f );
			c.spot[0] = desc.m_Falloff;
			c.spot[1] = stopdot;
			c.spot[2] = stopdot2;
			c.spot[3] = stopdot > stopdot2 ? 1.0f / ( stopdot - stopdot2 ) : 0.0f;
		}
		else
		{
			c.spot[0] = 0.0f;
			c.spot[1] = c.spot[2] = c.spot[3] = 1.0f;
		}
		c.atten[0] = desc.m_Attenuation0;
		c.atten[1] = desc.m_Attenuation1;
		c.atten[2] = desc.m_Attenuation2;
	}
	return count;
}

static float Saturate( float x )
{
	return x < 0.0f ? 0.0f : ( x > 1.0f ? 1.0f : x );
}

// common_vs_fxc.h VertexAttenInternal: distance and spot attenuation, 1 for a
// directional light. Also returns the normalized direction to the light.
static float VertexAtten(
    const VertexLightConstants &c, const float worldPos[3], float lightDir[3] = nullptr )
{
	float toLight[3] = { c.pos[0] - worldPos[0], c.pos[1] - worldPos[1], c.pos[2] - worldPos[2] };
	const float distSq =
	    toLight[0] * toLight[0] + toLight[1] * toLight[1] + toLight[2] * toLight[2];
	const float ooDist = 1.0f / sqrtf( distSq );
	float dir[3] = { toLight[0] * ooDist, toLight[1] * ooDist, toLight[2] * ooDist };
	// dst( distSq, ooDist ) = ( 1, dist, distSq, ooDist ).
	const float distanceAtten =
	    1.0f / ( c.atten[0] + c.atten[1] * ( distSq * ooDist ) + c.atten[2] * distSq );
	const float cosTheta = -( c.dir[0] * dir[0] + c.dir[1] * dir[1] + c.dir[2] * dir[2] );
	float spotAtten = ( cosTheta - c.spot[2] ) * c.spot[3];
	// pow( x, 0 ) is 1 for every x (C Annex F), so the point and directional
	// lights' zero exponent skips a powf per light per vertex, bit-exactly.
	spotAtten = c.spot[0] == 0.0f
	                ? 1.0f
	                : Saturate( powf( spotAtten > 0.0001f ? spotAtten : 0.0001f, c.spot[0] ) );
	const float atten = distanceAtten + ( distanceAtten * spotAtten - distanceAtten ) * c.dir[3];
	if ( lightDir )
		memcpy( lightDir, dir, sizeof( dir ) );
	return atten + ( 1.0f - atten ) * c.color[3];
}

// common_vs_fxc.h DoLighting for one vertex: the static color (STATIC_LIGHT),
// then the local lights and the ambient cube (DYNAMIC_LIGHT), in linear light.
static void ComputeVertexLighting( const float worldPos[3], const float normalIn[3],
    const float *staticColor, bool dynamicLight, bool halfLambert,
    const VertexLightConstants *lights, int lightCount, float out[3] )
{
	out[0] = out[1] = out[2] = 0.0f;
	if ( staticColor )
	{
		// GammaToLinear( staticLightingColor * cOverbright )
		for ( int k = 0; k < 3; ++k )
			out[k] += powf( staticColor[k] * 2.0f, 2.2f );
	}
	if ( !dynamicLight )
		return;
	float normal[3] = { normalIn[0], normalIn[1], normalIn[2] };
	const float length =
	    sqrtf( normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] );
	if ( length > 0.0f )
		for ( float &n : normal )
			n /= length;
	for ( int i = 0; i < lightCount; ++i )
	{
		const VertexLightConstants &c = lights[i];
		float lightDir[3];
		const float atten = VertexAtten( c, worldPos, lightDir );
		// CosineTermInternal: the direction to a point or spot light, or the
		// directional light's.
		for ( int k = 0; k < 3; ++k )
			lightDir[k] = lightDir[k] + ( -c.dir[k] - lightDir[k] ) * c.color[3];
		float nDotL = normal[0] * lightDir[0] + normal[1] * lightDir[1] + normal[2] * lightDir[2];
		if ( halfLambert )
		{
			nDotL = nDotL * 0.5f + 0.5f;
			nDotL = nDotL * nDotL;
		}
		else if ( nDotL < 0.0f )
		{
			nDotL = 0.0f;
		}
		for ( int k = 0; k < 3; ++k )
			out[k] += c.color[k] * nDotL * atten;
	}
	// AmbientLight: the cube faces weighted by the squared normal.
	for ( int axis = 0; axis < 3; ++axis )
	{
		const float *face = g_AmbientCube[2 * axis + ( normal[axis] < 0.0f ? 1 : 0 )];
		for ( int k = 0; k < 3; ++k )
			out[k] += normal[axis] * normal[axis] * face[k];
	}
}

// Draws the entire mesh
void CEmptyMesh::Draw( int firstIndex, int numIndices )
{
	const CEmptyMesh &vertices = m_pVertexSource ? *m_pVertexSource : *this;
	const CEmptyMesh &indices = m_pIndexSource ? *m_pIndexSource : *this;
	if ( !g_VulkanContext.IsValid() || !g_VulkanContext.DynamicMeshReady() ||
	     ( !m_worldMeshBatch && vertices.m_numVerts <= 0 ) )
		return;

	render_vulkan::CFrameCostScope cost(
	    g_VulkanContext.CurrentFrameCost(), render_vulkan::kCostMeshDraw );
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
	{
		CommitPassPixelConstants();
		EmitToNativeQueue();
	}
	g_pRenderMesh = nullptr;
}

// Reuse of converted geometry within a frame's stream. Portal views, recursion
// and multipass materials draw the same mesh range several times per frame with
// the same conversion inputs (the converted vertices are in model or world
// space, never view space), so a repeat draw can index the vertices the first
// one queued instead of converting them again. A draw is reused only when every
// input of EmitToNativeQueue's conversion matches exactly: the meshes (by write
// revision), range and topology in the key; the constants in `inputs`; and the
// bone matrices its vertices reference.
struct EmitReuseKey
{
	const void *vertexMesh;
	uint64_t vertexRevision;
	const void *indexMesh;
	uint64_t indexRevision;
	const void *colorMesh;
	uint64_t colorRevision;
	int colorOffset;
	int first;
	int count;
	int numVerts;
	int primitive;
	unsigned flags;
	int skinLightCount;
	int lightCount;

	bool operator==( const EmitReuseKey &other ) const
	{
		return vertexMesh == other.vertexMesh && vertexRevision == other.vertexRevision &&
		       indexMesh == other.indexMesh && indexRevision == other.indexRevision &&
		       colorMesh == other.colorMesh && colorRevision == other.colorRevision &&
		       colorOffset == other.colorOffset && first == other.first && count == other.count &&
		       numVerts == other.numVerts && primitive == other.primitive && flags == other.flags &&
		       skinLightCount == other.skinLightCount && lightCount == other.lightCount;
	}
	uint64_t Hash() const
	{
		uint64_t h = reinterpret_cast<uintptr_t>( vertexMesh ) * 0x9E3779B97F4A7C15ull;
		h ^= vertexRevision + 0x632BE59BD9B4E019ull + ( h << 6 ) + ( h >> 2 );
		h ^= static_cast<uint64_t>( first ) * 0xFF51AFD7ED558CCDull + ( h << 6 ) + ( h >> 2 );
		h ^= static_cast<uint64_t>( count ) + ( h << 6 ) + ( h >> 2 );
		return h;
	}
};

struct EmitReuseEntry
{
	EmitReuseKey key;
	std::vector<float> inputs;
	int maxBone;              // highest bone matrix the vertices read; -1 without skinning
	std::vector<float> bones; // g_BoneMatrices[0..maxBone]
	render_vulkan::CVulkanContext::DrawRange range;
};

// Entries of the current stream epoch; a draw converting fewer indices than
// this is cheaper to convert again than to key.
static std::vector<EmitReuseEntry> s_emitReuse;
static std::unordered_multimap<uint64_t, size_t> s_emitReuseIndex;
static uint64_t s_emitReuseEpoch = 0;
enum
{
	kMinReusedIndices = 96
};

// -vkemitreuseverify: every reused draw is also converted, privately, and the
// result compared with the reused geometry; mismatches are reported by name.
static bool VerifyEmitReuse()
{
	static const bool s_verify = CommandLine()->FindParm( "-vkemitreuseverify" ) != 0;
	return s_verify;
}

static void NoteEmitReuseCheck( bool equal )
{
	static uint64_t s_checked = 0, s_mismatched = 0;
	++s_checked;
	if ( !equal )
		++s_mismatched;
	if ( !equal || s_checked % 1000 == 0 )
		fprintf( stderr, "[vulkan] emit reuse verify: %llu checked, %llu mismatched\n",
		    static_cast<unsigned long long>( s_checked ),
		    static_cast<unsigned long long>( s_mismatched ) );
}

static const float *MonitorTexture2Rows();

void CEmptyMesh::EmitToNativeQueue()
{
	render_vulkan::CFrameCostScope cost( g_VulkanContext.CurrentFrameCost(), render_vulkan::kCostEmit );
	if ( m_worldMeshBatch )
	{
		if ( g_pBoundMaterial &&
		     ( render::pbr::IsMetalRoughShader( g_pBoundMaterial->GetShaderName() ) ||
		         !V_stricmp( g_pBoundMaterial->GetShaderName(), "PBR" ) ) )
		{
			// The material's dynamic pass bound its base, MRAO (sampler 10)
			// and normal (sampler 1). The WMSH pipeline uses those same images at
			// sets 0, 1 and 2, and the map-owned HDR LMAP at set 3.
			float eye[3];
			g_ShaderAPIEmpty.GetWorldSpaceCameraPosition( eye );
			// PBRMetalRough's dynamic state writes c0: transmission, IOR,
			// thickness, and w = 1 for glass (pbr_metalrough_native.cpp).
			const bool metalRough =
			    render::pbr::IsMetalRoughShader( g_pBoundMaterial->GetShaderName() );
			const bool glass = metalRough && g_psConstants[0][3] > 0.5f;
			// The material's optional maps (pbr_metalrough_native.cpp: c3.x
			// its features, c2.x $emissionscale). Translucent batches draw in
			// the engine's back-to-front WMSH pass with the snapshot's blend.
			const int features = metalRough ? static_cast<int>( g_psConstants[3][0] ) : 0;
			render_vulkan::CVulkanContext::PbrWorldMaps maps;
			if ( features & render::pbr::kNativeEmission )
			{
				maps.emission = g_boundPbrEmissionHandle;
				maps.emissionScale = g_psConstants[2][0];
			}
			if ( features & render::pbr::kNativeEnvMap )
				maps.environment = g_boundPbrEnvmapHandle;
			if ( features & render::pbr::kNativeClearCoat )
			{
				maps.clearCoat = g_psConstants[2][1];
				maps.clearCoatRoughness = g_psConstants[2][2];
			}
			if ( glass && ( features & ( render::pbr::kNativeEmission | render::pbr::kNativeEnvMap |
			                               render::pbr::kNativeClearCoat ) ) )
			{
				DropDraw( "draw dropped: WMSH glass with $emissiontexture, $envmap or $clearcoat not "
				          "implemented" );
				return;
			}
			if ( glass )
			{
				render_vulkan::CVulkanContext::PbrGlassParams params;
				params.transmission = g_psConstants[0][0];
				params.ior = g_psConstants[0][1];
				params.thickness = g_psConstants[0][2];
				params.materialKey = reinterpret_cast<uintptr_t>( g_pBoundMaterial );
				if ( !g_VulkanContext.SelectPbrGlassMaterial( g_boundPbrMraoHandle,
				         g_boundPbrNormalHandle, eye, g_CurrentAlphaRef, params ) )
				{
					DropDraw( "draw dropped: WMSH glass images or pipeline unavailable" );
					return;
				}
			}
			else if ( !g_VulkanContext.SelectPbrWorldMaterial( g_boundPbrMraoHandle,
			              g_boundPbrNormalHandle, eye, g_CurrentAlphaRef, maps ) )
			{
				DropDraw( "draw dropped: WMSH PBR images or pipeline unavailable" );
				return;
			}
			static bool s_reportedPbrWorld = false;
			if ( !s_reportedPbrWorld )
			{
				const int mask = g_boundPbrMraoHandle;
				const char *maskName = mask >= 0 && size_t( mask ) < g_TextureRecords.size()
				                           ? g_TextureRecords[size_t( mask )].name.c_str()
				                           : "(none)";
				Msg( "[NativeVulkan] WMSH PBR material %s shader %s: MRAO %s, alpha %.2f, "
				     "eye %.1f %.1f %.1f\n",
				    g_pBoundMaterial->GetName(), g_pBoundMaterial->GetShaderName(), maskName,
				    g_CurrentAlphaRef, eye[0], eye[1], eye[2] );
				s_reportedPbrWorld = true;
			}
		}
		if ( g_VulkanContext.QueueWorldMeshBatch( m_worldFirstIndex, m_worldIndexCount ) )
			m_worldDrawQueued = true;
		else
			DropDraw( "draw dropped: WMSH batch unavailable" );
		return;
	}
	// PBRMetalRough glass has only the WMSH pipeline.
	if ( g_CurrentPbrModel && g_psConstants[0][3] > 0.5f )
	{
		DropDraw( "draw dropped: PBRMetalRough glass on a model not implemented" );
		return;
	}
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

	// Normal and tangent, only for the shaders that read them: PortalRefract
	// (model space; skinned normals and tangents are not blended) and the skin
	// shader (world space, below).
	// PBRMetalRough models take the skin shader's vertex record: world-space
	// position, normal and tangent, and the four lights' attenuation.
	const bool skin = g_CurrentSkinCombos >= 0 || g_CurrentPbrModel;
	const bool envmap =
	    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentLightmappedEnvmap ) != 0;
	const bool refract =
	    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentRefract ) != 0;
	// SolidEnergy: world-space positions and tangent frame, like the skin shader.
	const bool solidEnergy = g_CurrentSolidEnergy;
	const bool solidEnergyModel =
	    solidEnergy && ( static_cast<int>( g_psConstants[11][0] ) & 256 ) != 0; // MODELFORMAT
	const bool wantsTangents =
	    g_CurrentPortalStage >= 0 || skin || envmap || refract || solidEnergy;
	float envContrast = 0.0f, envSaturation = 1.0f, fresnelReflection = 1.0f;
	if ( envmap && g_pBoundMaterial )
	{
		const auto materialFloat =
		    []( IMaterialInternal *material, const char *name, float defaultValue )
		{
			bool found = false;
			IMaterialVar *var = material->FindVar( name, &found, false );
			return found && var ? var->GetFloatValue() : defaultValue;
		};
		envContrast = materialFloat( g_pBoundMaterial, "$envmapcontrast", 0.0f );
		envSaturation = materialFloat( g_pBoundMaterial, "$envmapsaturation", 1.0f );
		fresnelReflection = materialFloat( g_pBoundMaterial, "$fresnelreflection", 1.0f );
	}
	if ( g_CurrentPortalStage >= 0 && g_NumBoneWeights > 0 )
		NoteUnimplemented( "PortalRefract: skinned normal/tangent" );
	// skin_vs20's per-vertex work: the lights' attenuation (GetVertexAttenForLight
	// in SortLights order, 0 for the absent ones) and the world tangent frame.
	VertexLightConstants skinLights[kMaxLocalLights];
	const int skinLightCount = skin ? BuildVertexLightConstants( skinLights ) : 0;
	// vertexlit_and_unlit_generic_vs20 lighting the vertices (not VERTEXCOLOR),
	// feeding a pixel shader that multiplies by it (DIFFUSELIGHTING): the vertex
	// color is the lighting its dynamic combo selects, evaluated once per vertex.
	const bool vertexLighting =
	    g_CurrentVertexLit.vertexLit && !g_CurrentVertexLit.vertexColor &&
	    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentModulateVertexColor );
	const bool dynamicLight = ( g_VertexShaderDynamicIndex / 2 ) % 2 != 0;
	const bool staticLight = ( g_VertexShaderDynamicIndex / 4 ) % 2 != 0;
	// vertexlit_and_unlit_generic's SELFILLUM reads its diffuse term, c1 times
	// the lighting (1 unlit), from the vertex color (demo_dyn_tex.frag).
	const bool selfIllum =
	    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentSelfIllum ) != 0;
	const bool monitor =
	    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentMonitor ) != 0;
	const bool monitorTexture2 =
	    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentMonitorTexture2 ) != 0;
	VertexLightConstants lights[kMaxLocalLights];
	int lightCount = 0;
	if ( vertexLighting )
	{
		if ( ( g_VertexShaderDynamicIndex / 32 ) % 2 )
			NoteUnimplemented( "vertexlit_and_unlit_generic_vs20: LIGHTING_PREVIEW" );
		if ( staticLight && !HasColorMesh() )
			NoteUnimplemented( "STATIC_LIGHT without a color mesh" );
		lightCount = dynamicLight ? BuildVertexLightConstants( lights ) : 0;
	}
	// One mesh vertex as the stream's record (CVulkanContext::QueueDynamicTriangles
	// documents the layout): position, color, uv, lightmap uv, normal/tangent,
	// alpha. It depends only on the vertex and this draw's state, so each vertex
	// an index list reuses is converted once per draw.
	enum
	{
		kRecordFloats = render_vulkan::CVulkanContext::kDynVertexFloats
	};
	int maxBone = -1;
	auto convertVertex = [&]( int v, float *out )
	{
		const unsigned char *base =
		    vertices.m_vertexData.data() + static_cast<size_t>( v ) * kMeshVertexStride;
		if ( g_NumBoneWeights > 0 )
		{
			// The bones SkinPosition and WorldNormal read for this vertex.
			const unsigned char *boneIndices = base + kMeshBoneIndexOffset;
			for ( int b = 0; b < 3; ++b )
				maxBone = std::max(
				    maxBone, boneIndices[b] < kMaxBoneMatrices ? int( boneIndices[b] ) : 0 );
		}
		float pos[3], uv[2];
		memcpy( pos, base, sizeof( pos ) );
		if ( g_NumBoneWeights > 0 )
			SkinPosition( base, pos );
		memcpy( uv, base + 16, sizeof( uv ) );
		memcpy( out + 8, base + 24, 2 * sizeof( float ) ); // lightmap uv
		if ( monitorTexture2 )
		{
			// unlittwotexture_vs20 derives both coordinates from TEXCOORD0.
			// The monitor's second matrix is c50/c51, not the mesh's lightmap UV.
			const float *row0 = MonitorTexture2Rows();
			const float *row1 = row0 + 4;
			out[8] = uv[0] * row0[0] + uv[1] * row0[1] + row0[3];
			out[9] = uv[0] * row1[0] + uv[1] * row1[1] + row1[3];
		}
		if ( wantsTangents )
		{
			float *nt = out + 10;
			memcpy( nt, base + kMeshNormalOffset, sizeof( float ) * 3 );
			memcpy( nt + 3, base + kMeshUserDataOffset, sizeof( float ) * 4 );
			if ( solidEnergy )
			{
				// solidenergy_vs20's frame: a brush's tangent S and T streams, or a
				// model's normal and TANGENT (T, with S = cross( N, T ) * w). The
				// record keeps N, S and the sign making T = cross( N, S ) * sign.
				float normal[3], tangentS[3], tangentT[3];
				WorldNormal( base, nt, normal );
				if ( solidEnergyModel )
				{
					WorldNormal( base, nt + 3, tangentT );
					const float w = nt[6] < 0.0f ? -1.0f : 1.0f;
					CrossProduct3( normal, tangentT, tangentS );
					for ( float &component : tangentS )
						component *= w;
				}
				else
				{
					float objectS[3], objectT[3];
					memcpy( objectS, base + kMeshTangentSOffset, sizeof( objectS ) );
					memcpy( objectT, base + kMeshTangentTOffset, sizeof( objectT ) );
					WorldNormal( base, objectS, tangentS );
					WorldNormal( base, objectT, tangentT );
				}
				float crossNS[3];
				CrossProduct3( normal, tangentS, crossNS );
				const float handedness =
				    crossNS[0] * tangentT[0] + crossNS[1] * tangentT[1] + crossNS[2] * tangentT[2];
				memcpy( nt, normal, sizeof( normal ) );
				memcpy( nt + 3, tangentS, sizeof( tangentS ) );
				nt[6] = handedness < 0.0f ? -1.0f : 1.0f;
			}
			else if ( skin )
			{
				// SkinPositionNormalAndTangentSpace: normal and tangent S through
				// the same rotation (unnormalized; skin.vert normalizes).
				const float normal[3] = { nt[0], nt[1], nt[2] };
				const float tangent[3] = { nt[3], nt[4], nt[5] };
				WorldNormal( base, normal, nt );
				WorldNormal( base, tangent, nt + 3 );
			}
			else if ( envmap )
			{
				float worldPos[3] = { pos[0], pos[1], pos[2] };
				ModelToWorld( worldPos );
				float worldNormal[3];
				WorldNormal( base, nt, worldNormal );
				float eye[3], dotNE = 0.0f, dotNN = 0.0f;
				float eyeLength2 = 0.0f;
				for ( int k = 0; k < 3; ++k )
				{
					eye[k] = g_psConstants[10][k] - worldPos[k];
					dotNE += worldNormal[k] * eye[k];
					dotNN += worldNormal[k] * worldNormal[k];
					eyeLength2 += eye[k] * eye[k];
				}
				for ( int k = 0; k < 3; ++k )
					nt[k] = 2.0f * dotNE * worldNormal[k] - dotNN * eye[k];
				const float eyeDotNormal = dotNE / sqrtf( std::max( eyeLength2 * dotNN, 1e-20f ) );
				const float oneMinusCos = 1.0f - eyeDotNormal;
				const float fresnel = powf( oneMinusCos, 5.0f );
				out[13] = envContrast;
				out[14] = envSaturation;
				out[15] = fresnel * ( 1.0f - fresnelReflection ) + fresnelReflection;
				out[16] = 0.0f;
			}
			else if ( refract )
			{
				float worldPos[3] = { pos[0], pos[1], pos[2] };
				ModelToWorld( worldPos );
				float worldNormal[3], worldTangent[3];
				WorldNormal( base, nt, worldNormal );
				WorldNormal( base, nt + 3, worldTangent );
				float eye[3], eyeLength2 = 0.0f;
				for ( int k = 0; k < 3; ++k )
				{
					eye[k] = g_psConstants[11][k] - worldPos[k];
					eyeLength2 += eye[k] * eye[k];
				}
				const float invEyeLength = 1.0f / sqrtf( std::max( eyeLength2, 1e-20f ) );
				for ( float &component : eye )
					component *= invEyeLength;
				const float sign = nt[6];
				const float binormal[3] = {
				    ( worldNormal[1] * worldTangent[2] - worldNormal[2] * worldTangent[1] ) * sign,
				    ( worldNormal[2] * worldTangent[0] - worldNormal[0] * worldTangent[2] ) * sign,
				    ( worldNormal[0] * worldTangent[1] - worldNormal[1] * worldTangent[0] ) *
				        sign };
				for ( int k = 0; k < 3; ++k )
				{
					nt[k] = worldNormal[k];
					nt[3 + k] = worldTangent[k];
				}
				out[3] =
				    eye[0] * worldTangent[0] + eye[1] * worldTangent[1] + eye[2] * worldTangent[2];
				out[4] = eye[0] * binormal[0] + eye[1] * binormal[1] + eye[2] * binormal[2];
				out[5] =
				    eye[0] * worldNormal[0] + eye[1] * worldNormal[1] + eye[2] * worldNormal[2];
				out[8] = g_psConstants[0][0];
				out[9] = g_psConstants[0][1];
			}
		}
		else
		{
			std::fill( out + 10, out + 17, 0.0f );
		}
		// A D3DCOLOR, as CVertexBuilder::Color4ub stores it (no
		// OPENGL_SWAP_COLORS): bytes B, G, R, A.
		const unsigned char *col = base + 12;
		out[17] = col[3] / 255.0f;
		if ( solidEnergy )
		{
			// World-space position (the push block holds only cViewProj) and the
			// vertex color ($vertexcolor / $vertexalpha).
			if ( g_NumBoneWeights <= 0 )
				ModelToWorld( pos );
			out[3] = col[2] / 255.0f;
			out[4] = col[1] / 255.0f;
			out[5] = col[0] / 255.0f;
		}
		else if ( skin )
		{
			// World-space position (the push block holds only cViewProj), and the
			// four lights' attenuation in the color slot and the alpha.
			if ( g_NumBoneWeights <= 0 )
				ModelToWorld( pos );
			float atten[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			for ( int i = 0; i < skinLightCount; ++i )
				atten[i] = VertexAtten( skinLights[i], pos );
			memcpy( out + 3, atten, 3 * sizeof( float ) );
			out[17] = atten[3];
		}
		else if ( monitor )
		{
			// Carry monitor_ps2x's c3 tint and c2 saturation in the unused
			// vertex-color slots of this unlit material's native vertex record.
			memcpy( out + 3, g_psConstants[3], 3 * sizeof( float ) );
			out[17] = g_psConstants[2][0];
		}
		else if ( envmap )
		{
			// c0 is $envmaptint; c30.z is ENV_MAP_SCALE.
			for ( int k = 0; k < 3; ++k )
				out[3 + k] = g_psConstants[0][k] * g_psConstants[30][2];
		}
		else if ( refract )
		{
			out[17] = g_psConstants[0][2];
		}
		else if ( vertexLighting )
		{
			float worldPos[3] = { pos[0], pos[1], pos[2] };
			if ( g_NumBoneWeights <= 0 )
				ModelToWorld( worldPos );
			float normal[3], worldNormal[3], staticColor[3], lit[3];
			memcpy( normal, base + kMeshNormalOffset, sizeof( normal ) );
			WorldNormal( base, normal, worldNormal );
			const bool hasStatic = staticLight && StaticColor( v, staticColor );
			ComputeVertexLighting( worldPos, worldNormal, hasStatic ? staticColor : nullptr,
			    dynamicLight, g_CurrentVertexLit.halfLambert, lights, lightCount, lit );
			for ( int k = 0; k < 3; ++k )
				out[3 + k] = selfIllum ? lit[k] * g_psConstants[1][k] : lit[k];
		}
		else if ( selfIllum )
		{
			memcpy( out + 3, g_psConstants[1], 3 * sizeof( float ) );
		}
		else
		{
			out[3] = col[2] / 255.0f;
			out[4] = col[1] / 255.0f;
			out[5] = col[0] / 255.0f;
		}
		memcpy( out, pos, sizeof( pos ) );
		out[6] = uv[0];
		out[7] = uv[1];
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
	switch ( m_primitiveType )
	{
	case MATERIAL_POINTS:
	case MATERIAL_LINES:
	case MATERIAL_LINE_STRIP:
	case MATERIAL_LINE_LOOP:
		// No line or point pipeline exists yet, and assembling these as triangles
		// would draw geometry the engine never asked for. Drop the draw instead.
		DropDraw( "draw dropped: line/point topology" );
		return;
	default:
		break;
	}
	// Calls triangle(a, b, c) for each triangle the topology assembles, in draw
	// order. `note` reports rejected triangles to the census (the counting pass
	// does, so each is reported once).
	auto forEachTriangle = [&]( bool note, const std::function<void( int, int, int )> &triangle )
	{
		auto emit = [&]( int a, int b, int c )
		{
			if ( a < 0 || b < 0 || c < 0 || a >= numVerts || b >= numVerts || c >= numVerts )
			{
				if ( note )
					NoteUnimplemented( "triangle dropped: index outside vertex range" );
				return;
			}
			// Strips stitch separate runs together with degenerate (zero-area)
			// triangles. A strip pipeline discards those for free; assembling a
			// list has to drop them explicitly or they become junk geometry.
			if ( a == b || b == c || a == c )
				return;
			triangle( a, b, c );
		};
		switch ( m_primitiveType )
		{
		case MATERIAL_TRIANGLE_STRIP:
			// Every vertex after the first two closes a triangle with its two
			// predecessors, alternating winding so the facing stays consistent.
			for ( int i = 0; i + 2 < elementCount; ++i )
			{
				if ( i & 1 )
					emit( element( i + 1 ), element( i ), element( i + 2 ) );
				else
					emit( element( i ), element( i + 1 ), element( i + 2 ) );
			}
			break;

		case MATERIAL_POLYGON:
			// A single convex polygon, fanned from its first vertex.
			for ( int i = 1; i + 1 < elementCount; ++i )
				emit( element( 0 ), element( i ), element( i + 1 ) );
			break;

		case MATERIAL_QUADS:
		case MATERIAL_INSTANCED_QUADS:
			for ( int i = 0; i + 3 < elementCount; i += 4 )
			{
				emit( element( i ), element( i + 1 ), element( i + 2 ) );
				emit( element( i ), element( i + 2 ), element( i + 3 ) );
			}
			break;

		default:
			for ( int i = 0; i + 2 < elementCount; i += 3 )
				emit( element( i ), element( i + 1 ), element( i + 2 ) );
			break;
		}
	};

	uint32_t triangles = 0;
	forEachTriangle( true,
	    [&]( int, int, int )
	    {
		    ++triangles;
	    } );
	if ( triangles == 0 )
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
	// The draw is indexed: each mesh vertex it uses is converted once, straight
	// into the frame's stream, in first-use order, and the triangles index those
	// records. A per-draw generation stamp marks the vertices already written (and
	// where), so the bookkeeping never needs clearing; it persists across draws.
	static std::vector<uint32_t> s_vertexStamp;
	static std::vector<uint32_t> s_vertexSlot;
	static uint32_t s_generation = 0;
	if ( s_vertexStamp.size() < static_cast<size_t>( numVerts ) )
	{
		s_vertexStamp.resize( static_cast<size_t>( numVerts ), 0 );
		s_vertexSlot.resize( static_cast<size_t>( numVerts ) );
	}
	auto convertDraw =
	    [&]( float *vertexOut, uint32_t *indexOut, uint32_t &written, uint32_t &indexCount )
	{
		if ( ++s_generation == 0 )
		{
			std::fill( s_vertexStamp.begin(), s_vertexStamp.end(), 0u );
			s_generation = 1;
		}
		written = indexCount = 0;
		auto put = [&]( int v )
		{
			if ( s_vertexStamp[static_cast<size_t>( v )] != s_generation )
			{
				convertVertex( v, vertexOut + static_cast<size_t>( written ) * kRecordFloats );
				s_vertexStamp[static_cast<size_t>( v )] = s_generation;
				s_vertexSlot[static_cast<size_t>( v )] = written++;
			}
			indexOut[indexCount++] = s_vertexSlot[static_cast<size_t>( v )];
		};
		forEachTriangle( false,
		    [&]( int a, int b, int c )
		    {
			    put( a );
			    put( b );
			    put( c );
		    } );
	};

	// A repeat of a draw this stream already converted reuses its geometry.
	const uint32_t maxIndices = triangles * 3;
	const bool reusable =
	    !vertices.m_bIsDynamic && !indices.m_bIsDynamic && maxIndices >= kMinReusedIndices;
	EmitReuseKey key;
	std::vector<float> inputs;
	if ( reusable )
	{
		if ( s_emitReuseEpoch != g_VulkanContext.StreamEpoch() )
		{
			s_emitReuse.clear();
			s_emitReuseIndex.clear();
			s_emitReuseEpoch = g_VulkanContext.StreamEpoch();
		}
		memset( &key, 0, sizeof( key ) );
		key.vertexMesh = &vertices;
		key.vertexRevision = vertices.m_revision;
		key.indexMesh = &indices;
		key.indexRevision = indices.m_revision;
		key.colorMesh = vertexLighting && staticLight ? m_pColorMesh : nullptr;
		key.colorRevision = key.colorMesh ? m_pColorMesh->m_revision : 0;
		key.colorOffset = key.colorMesh ? m_colorMeshOffset : 0;
		key.first = first;
		key.count = count;
		key.numVerts = numVerts;
		key.primitive = m_primitiveType;
		key.flags =
		    ( skin ? 1u : 0u ) | ( wantsTangents ? 2u : 0u ) | ( vertexLighting ? 4u : 0u ) |
		    ( selfIllum ? 8u : 0u ) | ( dynamicLight ? 16u : 0u ) | ( staticLight ? 32u : 0u ) |
		    ( g_CurrentVertexLit.halfLambert ? 64u : 0u ) | ( g_NumBoneWeights > 0 ? 128u : 0u ) |
		    ( monitor ? 256u : 0u ) | ( monitorTexture2 ? 512u : 0u ) |
		    ( solidEnergy ? 1024u : 0u ) | ( solidEnergyModel ? 2048u : 0u );
		key.skinLightCount = skinLightCount;
		key.lightCount = lightCount;
		// The constants the conversion reads, beyond the bones.
		if ( g_NumBoneWeights <= 0 && ( skin || vertexLighting || solidEnergy ) )
			inputs.insert( inputs.end(), ModelMatrix(), ModelMatrix() + 16 );
		for ( int i = 0; i < skinLightCount; ++i )
		{
			const float *light = reinterpret_cast<const float *>( &skinLights[i] );
			inputs.insert( inputs.end(), light, light + sizeof( skinLights[i] ) / sizeof( float ) );
		}
		for ( int i = 0; i < lightCount; ++i )
		{
			const float *light = reinterpret_cast<const float *>( &lights[i] );
			inputs.insert( inputs.end(), light, light + sizeof( lights[i] ) / sizeof( float ) );
		}
		if ( vertexLighting && dynamicLight )
			inputs.insert( inputs.end(), &g_AmbientCube[0][0], &g_AmbientCube[0][0] + 6 * 4 );
		if ( selfIllum )
			inputs.insert( inputs.end(), g_psConstants[1], g_psConstants[1] + 3 );
		if ( monitor )
		{
			inputs.push_back( g_psConstants[2][0] );
			inputs.insert( inputs.end(), g_psConstants[3], g_psConstants[3] + 3 );
			if ( monitorTexture2 )
				inputs.insert( inputs.end(), MonitorTexture2Rows(), MonitorTexture2Rows() + 8 );
		}
		const auto candidates = s_emitReuseIndex.equal_range( key.Hash() );
		for ( auto it = candidates.first; it != candidates.second; ++it )
		{
			const EmitReuseEntry &entry = s_emitReuse[it->second];
			if ( !( entry.key == key ) || entry.inputs.size() != inputs.size() ||
			     ( !inputs.empty() && memcmp( entry.inputs.data(), inputs.data(),
			                              inputs.size() * sizeof( float ) ) ) ||
			     ( entry.maxBone >= 0 && memcmp( entry.bones.data(), g_BoneMatrices,
			                                 entry.bones.size() * sizeof( float ) ) ) )
				continue;
			if ( VerifyEmitReuse() )
			{
				// Shadow check: convert anyway, privately, and compare with the
				// geometry being reused.
				std::vector<float> vertexCheck(
				    static_cast<size_t>(
				        std::min( static_cast<uint32_t>( numVerts ), maxIndices ) ) *
				    kRecordFloats );
				std::vector<uint32_t> indexCheck( maxIndices );
				uint32_t checkVertices = 0, checkIndices = 0;
				convertDraw( vertexCheck.data(), indexCheck.data(), checkVertices, checkIndices );
				NoteEmitReuseCheck( g_VulkanContext.StreamRangeEquals( entry.range,
				    vertexCheck.data(), checkVertices, indexCheck.data(), checkIndices ) );
			}
			g_VulkanContext.ReuseDynamicDraw( entry.range );
			g_VulkanContext.CurrentFrameCost().Add( render_vulkan::kCostEmitReuse, 0 );
			if ( g_NumBoneWeights > 0 )
				CommitModelViewProj();
			return;
		}
	}

	uint32_t *indexOut = nullptr;
	float *const vertexOut = g_VulkanContext.BeginDynamicDraw(
	    std::min( static_cast<uint32_t>( numVerts ), maxIndices ), maxIndices, &indexOut );
	uint32_t written = 0, indexCount = 0;
	convertDraw( vertexOut, indexOut, written, indexCount );
	g_VulkanContext.EndDynamicDraw( written, indexCount );
	if ( reusable && written > 0 )
	{
		EmitReuseEntry entry;
		entry.key = key;
		entry.inputs.swap( inputs );
		entry.maxBone = maxBone;
		if ( maxBone >= 0 )
			entry.bones.assign( &g_BoneMatrices[0][0], &g_BoneMatrices[maxBone][0] + 12 );
		entry.range = g_VulkanContext.LastDrawRange();
		s_emitReuseIndex.emplace( key.Hash(), s_emitReuse.size() );
		s_emitReuse.push_back( std::move( entry ) );
	}
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
	m_alphaRef = 0.7f;
	m_alphaFunc = SHADER_ALPHAFUNC_GEQUAL;
	m_colorWrites = true;
	m_alphaWrites = false;
	m_polyOffset = SHADER_POLYOFFSET_DISABLE;
	m_cullEnable = true;
	m_vertexUsage = 0;
	m_colorFlags = 0;
	m_vertexShaderName[0] = '\0';
	// D3D9's default shadow state has no pixel shader: a shader that sets none
	// (WriteZ, a depth-only BufferClearObeyStencil) must not inherit the last one.
	m_pixelShaderName[0] = '\0';
	m_pixelShaderIndex = 0;
	m_vertexShaderIndex = 0;
	m_fogMode = SHADER_FOGMODE_DISABLED;
	m_disableFogGammaCorrection = false;
	m_enabledSamplers = 0;
	m_alphaPipe = false;
	m_constantAlpha = false;
	m_vertexAlpha = false;
	m_textureAlphaStages = 0;
	m_constantColor = false;
	m_lighting = false;
	m_specular = false;
	m_vertexBlend = false;
	for ( float &overbright : m_overbright )
		overbright = 1.0f;
	m_customPixelPipe = false;
	m_customStageCount = 0;
	m_texGenStages = 0;
	for ( ShaderTexGenParam_t &texGen : m_texGen )
		texGen = SHADER_TEXGENPARAM_OBJECT_LINEAR;
	m_drawFlags = 0;
	m_diffuseMaterialSource = SHADER_MATERIALSOURCE_MATERIAL;
	m_morphFormat = 0;
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
	if ( nOffsetMode != SHADER_POLYOFFSET_DISABLE && nOffsetMode != SHADER_POLYOFFSET_DECAL &&
	     nOffsetMode != SHADER_POLYOFFSET_SHADOW_BIAS )
	{
		NoteUnimplemented( "EnablePolyOffset(reserved mode)" );
		m_polyOffset = SHADER_POLYOFFSET_DISABLE;
		return;
	}
	m_polyOffset = nOffsetMode;
}

// Suppresses/activates color writing
void CShaderShadowVulkan::EnableColorWrites( bool bEnable )
{
	m_colorWrites = bEnable;
}

// Suppresses/activates alpha writing
void CShaderShadowVulkan::EnableAlphaWrites( bool bEnable )
{
	m_alphaWrites = bEnable;
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

// A simpler method of dealing with alpha modulation (fixed-function state)
void CShaderShadowVulkan::EnableAlphaPipe( bool bEnable )
{
	m_alphaPipe = bEnable;
}

void CShaderShadowVulkan::EnableConstantAlpha( bool bEnable )
{
	m_constantAlpha = bEnable;
}

void CShaderShadowVulkan::EnableVertexAlpha( bool bEnable )
{
	m_vertexAlpha = bEnable;
}

void CShaderShadowVulkan::EnableTextureAlpha( TextureStage_t stage, bool bEnable )
{
	if ( stage < 0 || stage >= kFixedFunctionStages )
		return;
	if ( bEnable )
		m_textureAlphaStages |= 1u << stage;
	else
		m_textureAlphaStages &= ~( 1u << stage );
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

// constant color + transparency (fixed-function state)
void CShaderShadowVulkan::EnableConstantColor( bool bEnable )
{
	m_constantColor = bEnable;
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

// Indicates we're going to light the model (fixed-function state)
void CShaderShadowVulkan::EnableLighting( bool bEnable )
{
	m_lighting = bEnable;
}

void CShaderShadowVulkan::EnableSpecular( bool bEnable )
{
	m_specular = bEnable;
}

// Activate/deactivate fixed-function skinning. Shader passes skin from the
// bone count (SetNumBoneWeights), as on D3D9.
void CShaderShadowVulkan::EnableVertexBlend( bool bEnable )
{
	m_vertexBlend = bEnable;
}

// per texture unit stuff. As CShaderShadowDX8, the overbright factor is kept
// only on hardware that reports overbright support, which this backend does not.
void CShaderShadowVulkan::OverbrightValue( TextureStage_t stage, float value )
{
	if ( stage >= 0 && stage < kFixedFunctionStages && g_ShaderAPIEmpty.SupportsOverbright() )
		m_overbright[stage] = value;
}

// The samplers a pass reads. Part of the snapshot: D3D9 sets no texture on a
// sampler its current pass did not enable (CShaderAPIDx8::ApplyTextureEnable).
void CShaderShadowVulkan::EnableTexture( Sampler_t stage, bool bEnable )
{
	if ( stage < 0 || stage >= 32 )
		return;
	if ( bEnable )
		m_enabledSamplers |= 1u << stage;
	else
		m_enabledSamplers &= ~( 1u << stage );
}

void CShaderShadowVulkan::EnableCustomPixelPipe( bool bEnable )
{
	m_customPixelPipe = bEnable;
}

void CShaderShadowVulkan::CustomTextureStages( int stageCount )
{
	m_customStageCount = std::max( 0, std::min( stageCount, static_cast<int>( kFixedFunctionStages ) ) );
}

// Fixed-function texture stage operations. They apply only to passes without
// shaders; the stage count bounds them as on D3D9.
void CShaderShadowVulkan::CustomTextureOperation( TextureStage_t stage, ShaderTexChannel_t channel,
    ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2 )
{
	if ( stage < 0 || stage >= kFixedFunctionStages || channel < 0 || channel > 1 )
		return;
	m_texOp[stage][channel] = op;
	m_texArg[stage][channel][0] = arg1;
	m_texArg[stage][channel][1] = arg2;
}

void CShaderShadowVulkan::EnableTexGen( TextureStage_t stage, bool bEnable )
{
	if ( stage < 0 || stage >= kFixedFunctionStages )
		return;
	if ( bEnable )
		m_texGenStages |= 1u << stage;
	else
		m_texGenStages &= ~( 1u << stage );
}

void CShaderShadowVulkan::TexGen( TextureStage_t stage, ShaderTexGenParam_t param )
{
	if ( stage >= 0 && stage < kFixedFunctionStages )
		m_texGen[stage] = param;
}

// Sets the vertex and pixel shaders
void CShaderShadowVulkan::SetVertexShader( const char *pShaderName, int vshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
	Q_strncpy( m_vertexShaderName, pShaderName ? pShaderName : "", sizeof( m_vertexShaderName ) );
	m_vertexShaderIndex = vshIndex;
}

void CShaderShadowVulkan::EnableBlendingSeparateAlpha( bool bEnable )
{
	VK_UNIMPLEMENTED();
}

void CShaderShadowVulkan::DrawFlags( unsigned int drawFlags )
{
	// As CShaderShadowDX8: the vertex components a fixed-function pass reads.
	m_drawFlags = drawFlags;
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
	memset( m_FloatRenderingParameters, 0, sizeof( m_FloatRenderingParameters ) );
	memset( m_IntRenderingParameters, 0, sizeof( m_IntRenderingParameters ) );
	memset( m_VectorRenderingParameters, 0, sizeof( m_VectorRenderingParameters ) );
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

static void ClearSnapshotTables();

// Used to clear the transition table when we know it's become invalid. The
// material system recomputes every live material's snapshots right after, as
// on D3D9 (CTransitionTable::Reset), so the ids start again from zero.
void CShaderAPIVulkan::ClearSnapshots()
{
	ClearSnapshotTables();
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
	return GetDXSupportLevel() >= 95;
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
	return g_VulkanContext.MaxAnisotropicLevel();
}

void CShaderAPIVulkan::SetAnisotropicLevel( int nAnisotropyLevel )
{
	g_VulkanContext.SetAnisotropicLevel( nAnisotropyLevel );
}

int CShaderAPIVulkan::MaxTextureWidth() const
{
	// Before SetMode there is no selected physical device. Keep the content
	// ceiling for that query; after selection report the actual device limit.
	const uint32_t deviceLimit = g_VulkanContext.MaxSampledTextureDimension();
	return static_cast<int>( deviceLimit ? std::min( deviceLimit, 16384U ) : 16384U );
}

int CShaderAPIVulkan::MaxTextureHeight() const
{
	return MaxTextureWidth();
}

int CShaderAPIVulkan::MaxTextureAspectRatio() const
{
	return MaxTextureWidth();
}

// The largest device-local heap, what a D3D9 driver reports (the interface is
// an int of bytes, so larger heaps report INT_MAX).
int CShaderAPIVulkan::TextureMemorySize() const
{
	const uint64_t bytes = CurrentAdapterCaps().largestDeviceLocalHeapBytes;
	return bytes ? static_cast<int>( std::min<uint64_t>( bytes, INT_MAX ) ) : 64 * 1024 * 1024;
}

// As CHardwareConfig::GetDXSupportLevel: the configured level (mat_dxlevel)
// when one is set, never above the caps.
int CShaderAPIVulkan::GetDXSupportLevel() const
{
	const int configured = ShaderUtil() ? ShaderUtil()->GetConfig().dxSupportLevel : 0;
	return configured != 0 ? std::min( configured, NativeCapsDxLevel() ) : NativeCapsDxLevel();
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
	return NativeCapsDxLevel();
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

// Sets the default *dynamic* state before each pass, as
// CShaderAPIDx8::SetDefaultState: identity texture transforms for the first four
// fixed-function stages, the MODEL matrix mode, a white constant color, smooth
// shading, the default shader indices and no unused vertex fields.
void CShaderAPIVulkan::SetDefaultState()
{
	const int stages = std::min( GetTextureStageCount(), 4 );
	ResetTextureMatrices( stages );
	for ( int i = 0; i < stages; ++i )
		m_TextureStages[i].transformFlags = 0;
	MatrixMode( MATERIAL_MODEL );
	Color4ub( 255, 255, 255, 255 );
	ShadeMode( SHADER_SMOOTH );
	SetVertexShaderIndex( -1 );
	SetPixelShaderIndex( 0 );
	MarkUnusedVertexFields( 0, 0, NULL );
}

// Snapshot -> selected pixel shader name, so BeginPass can bind the matching
// native pipeline. The snapshot id keeps the existing flag bits (0..3) and packs
// the table index in the high bits, preserving IsTranslucent()/etc.
static std::vector<std::string> g_snapshotShaders;
// Parallel to g_snapshotShaders: the blend and depth state each snapshot draws
// with, taken from the recorded IShaderShadow state so BeginPass can select the
// matching pipeline.
static std::vector<render_vulkan::CVulkanContext::DynRasterState> g_snapshotRaster;
static std::vector<PolygonOffsetMode_t> g_snapshotPolyOffset;
// Parallel to g_snapshotShaders: the $alphatest reference each snapshot applies
// (< 0 when alpha test is disabled).
static std::vector<float> g_snapshotAlphaRef;
// Parallel to g_snapshotShaders: the vertex usage each snapshot's shader declared.
static std::vector<VertexFormat_t> g_snapshotVertexUsage;
// Parallel to g_snapshotShaders: the kColorSrgb* flags each snapshot declared.
static std::vector<int> g_snapshotColorFlags;
static std::vector<bool> g_snapshotModulationInPixelC1;
static std::vector<VertexLitCombo> g_snapshotVertexLit;
// Parallel to g_snapshotShaders: the fog each snapshot's pass applies
// (IShaderShadow::FogMode, DisableFogGammaCorrection) and the samplers it enabled.
struct SnapshotFogState
{
	ShaderFogMode_t mode;
	bool disableGammaCorrection;
};
static std::vector<SnapshotFogState> g_snapshotFog;
static std::vector<unsigned int> g_snapshotEnabledSamplers;
// Parallel to g_snapshotShaders: SnapshotToneMapType of each snapshot's pass.
static std::vector<int> g_snapshotToneMap;
static int g_CurrentToneMap = 0;
// Parallel to g_snapshotShaders: sprite_ps2x's CONSTANTCOLOR (bit 0) and HDRTYPE
// (bits 1..2) static combos, -1 for other pixel shaders.
static std::vector<int> g_snapshotSpriteCombos;
static int g_CurrentSpriteCombos = -1;
// Parallel to g_snapshotShaders: how each snapshot's pixel shader fogs
// (common_ps_fxc.h CalcPixelFogFactor / FinalOutput) and the registers it reads
// its fog parameters and eye position from.
enum PixelFogMode
{
	kPixelFogNone = 0,     // PIXEL_FOG_TYPE_NONE, or a shader without fog
	kPixelFogCombo,        // the PIXELFOGTYPE dynamic combo (GetPixelFogCombo)
	kPixelFogConstantType, // the type in g_ShaderControls.x (c12.x; ps2b and up)
	kPixelFogDecal         // PIXELFOGTYPE, factor raised to 0.4 (DecalModulate)
};
struct PixelFogInputs
{
	int mode = kPixelFogNone;
	int paramsRegister = 0;
	int eyeRegister = 0;
};
static std::vector<PixelFogInputs> g_snapshotPixelFog;
static PixelFogInputs g_CurrentPixelFog;
// The samplers the pass being drawn enabled (all when no snapshot is current).
static unsigned int g_CurrentEnabledSamplers = ~0u;

// vertexlit_and_unlit_generic_vs20's static combos (fxctmp9/
// vertexlit_and_unlit_generic_vs20.inc): VERTEXCOLOR at stride 192, HALFLAMBERT
// at 768. The vs30 build (hardware with fast vertex textures) is not used here.
static VertexLitCombo SnapshotVertexLitCombo( const CShaderShadowVulkan &shadow )
{
	VertexLitCombo combo;
	combo.vertexLit = !V_stricmp( shadow.m_vertexShaderName, "vertexlit_and_unlit_generic_vs20" );
	if ( combo.vertexLit )
	{
		combo.vertexColor = ( shadow.m_vertexShaderIndex / 192 ) % 2 != 0;
		combo.halfLambert = ( shadow.m_vertexShaderIndex / 768 ) % 2 != 0;
	}
	return combo;
}
// Snapshot ids are 16-bit (StateSnapshot_t is a short): four flag bits and an
// 11-bit table index. As in D3D9's transition table, identical shadow states
// share one snapshot, so the table holds distinct states rather than one entry
// per TakeSnapshot call; appending per call wrapped the index past 2048 and bound
// other materials' blend state.
static const size_t kMaxSnapshots = 0x800;
static std::map<std::string, StateSnapshot_t> g_snapshotIds;

static size_t SnapshotCount()
{
	return g_snapshotShaders.size();
}

static size_t SnapshotCapacity()
{
	return kMaxSnapshots;
}

static void ClearSnapshotTables()
{
	g_snapshotIds.clear();
	g_snapshotShaders.clear();
	g_snapshotRaster.clear();
	g_snapshotPolyOffset.clear();
	g_snapshotAlphaRef.clear();
	g_snapshotVertexUsage.clear();
	g_snapshotColorFlags.clear();
	g_snapshotModulationInPixelC1.clear();
	g_snapshotVertexLit.clear();
	g_snapshotFog.clear();
	g_snapshotEnabledSamplers.clear();
	g_snapshotToneMap.clear();
	g_snapshotSpriteCombos.clear();
	g_snapshotPixelFog.clear();
}

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
	if ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentSky )
	{
		// sky_vs20.fxc reserves SHADER_SPECIFIC_CONST_0 for texture-size values;
		// its base-texture matrix is in SHADER_SPECIFIC_CONST_1/2 (c49/c50).
		g_VulkanContext.SetDynamicBaseTexTransform(
		    &g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_1][0],
		    &g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_2][0] );
	}
	else
	{
		g_VulkanContext.SetDynamicBaseTexTransform( &g_vsConstants.regs[kVsRegBaseTexTransform][0],
		    &g_vsConstants.regs[kVsRegBaseTexTransform + 1][0] );
	}
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
	state.alphaWrite = shadow.m_alphaWrites;
	// Culling on; the dynamic CullMode picks the face when the pass is drawn.
	state.cullMode = shadow.m_cullEnable ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	return state;
}

} // namespace

// D3D9 applies the configured slope and normalized constant bias at the draw,
// not when the shadow snapshot is made. Convert the constant term to Vulkan's
// depth-buffer units; keep the actual factors on the queued draw so later
// config or shadow-bias changes cannot alter an already recorded pass.
void CShaderAPIVulkan::ApplyDepthBiasState( render_vulkan::CVulkanContext::DynRasterState &raster )
{
	static const MaterialSystem_Config_t defaults;
	const MaterialSystem_Config_t &config = ShaderUtil() ? ShaderUtil()->GetConfig() : defaults;
	float slope = 0.0f;
	float normalized = 0.0f;
	if ( g_CurrentPolyOffset == SHADER_POLYOFFSET_DECAL )
	{
		slope = config.m_SlopeScaleDepthBias_Decal != 0.0f
		            ? 1.0f / config.m_SlopeScaleDepthBias_Decal
		            : 0.0f;
		normalized = config.m_DepthBias_Decal != 0.0f ? 1.0f / config.m_DepthBias_Decal : 0.0f;
	}
	else if ( g_CurrentPolyOffset == SHADER_POLYOFFSET_SHADOW_BIAS )
	{
		slope = m_shadowSlopeScaleDepthBias;
		normalized = m_shadowDepthBias;
	}
	else
	{
		slope = config.m_SlopeScaleDepthBias_Normal != 0.0f
		            ? 1.0f / config.m_SlopeScaleDepthBias_Normal
		            : 0.0f;
		normalized = config.m_DepthBias_Normal != 0.0f ? 1.0f / config.m_DepthBias_Normal : 0.0f;
	}
	raster.depthBiasEnable = slope != 0.0f || normalized != 0.0f;
	g_VulkanContext.SetDynamicDepthBias( normalized * g_VulkanContext.DepthBiasUnitScale(), slope );
}

namespace
{

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
	if ( !V_strnicmp( shadow.m_pixelShaderName, "cable_ps20", 10 ) )
		flags |= CVulkanContext::kFragmentCable;
	if ( !V_strnicmp( shadow.m_pixelShaderName, "sky_ps20", 8 ) )
		flags |= CVulkanContext::kFragmentSky;
	if ( !V_strnicmp( shadow.m_pixelShaderName, "monitorscreen_ps20", 18 ) )
	{
		flags |= CVulkanContext::kFragmentMonitor;
		const int texture2Stride = !V_stricmp( shadow.m_pixelShaderName, "monitorscreen_ps20b" )
		                               ? 8
		                               : 2;
		if ( ( shadow.m_pixelShaderIndex / texture2Stride ) % 2 )
			flags |= CVulkanContext::kFragmentMonitorTexture2;
	}
	if ( !V_stricmp( shadow.m_vertexShaderName, "screenspaceeffect_vs20" ) )
		flags |= CVulkanContext::kVertexScreenSpace;
	// bufferclearobeystencil_vs20 passes clip-space positions and the vertex
	// color (DrawClearBufferQuad's), which bufferclearobeystencil_ps2x returns.
	if ( !V_stricmp( shadow.m_vertexShaderName, "bufferclearobeystencil_vs20" ) )
		flags |= CVulkanContext::kVertexScreenSpace | CVulkanContext::kFragmentVertexColor;
	// sprite_ps2x.fxc multiplies the sampled color and alpha by the vertex
	// color when its VERTEXCOLOR combo is selected. The sprite vertex shader
	// gamma-decodes RGB only when SRGB is enabled; its alpha stays linear.
	if ( !V_strnicmp( shadow.m_pixelShaderName, "sprite_ps20", 11 ) &&
	     ( shadow.m_vertexUsage & VERTEX_COLOR ) )
	{
		flags |= CVulkanContext::kFragmentModulateVertexColor |
		         CVulkanContext::kFragmentSpriteVertexAlpha;
		if ( !( shadow.m_colorFlags & CVulkanContext::kColorSrgbReadBase ) )
			flags |= CVulkanContext::kVertexColorNoGammaConvert;
	}
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
	// VertexLitGeneric: the pixel shader's DIFFUSELIGHTING combo multiplies by
	// its color input, which the vertex shader fills with linear vertex lighting
	// (DoLighting; EmitToNativeQueue evaluates it into the vertex color). That
	// color is never gamma converted.
	if ( !V_strnicmp( shadow.m_pixelShaderName, "vertexlit_and_unlit_generic_ps", 30 ) &&
	     ( shadow.m_pixelShaderIndex / 24 ) % 2 )
	{
		flags |= CVulkanContext::kFragmentModulateVertexColor |
		         CVulkanContext::kVertexColorNoGammaConvert;
		// With VERTEXCOLOR as well, albedo is also multiplied by the color.
		if ( ( shadow.m_pixelShaderIndex / 384 ) % 2 )
			NoteUnimplemented( "vertexlit_and_unlit_generic: VERTEXCOLOR with DIFFUSELIGHTING" );
	}
	// Its SELFILLUM static combo ($selfillum; stride 192 in every build) blends
	// the diffuse term toward the tinted albedo by the base alpha (the mask
	// texture is the dynamic c3.w; CommitPassPixelConstants). The pipeline reads
	// the diffuse term, c1 times the lighting, from the vertex color
	// (EmitToNativeQueue), which is linear. SELFILLUM_ENVMAPMASK_ALPHA (stride
	// 1536) needs the env-map mask, and VERTEXCOLOR would also multiply albedo.
	if ( !V_strnicmp( shadow.m_pixelShaderName, "vertexlit_and_unlit_generic_ps", 30 ) )
	{
		const int index = shadow.m_pixelShaderIndex;
		if ( ( index / 1536 ) % 2 )
			NoteUnimplemented( "vertexlit_and_unlit_generic: SELFILLUM_ENVMAPMASK_ALPHA" );
		if ( ( index / 192 ) % 2 )
		{
			if ( ( index / 384 ) % 2 )
				NoteUnimplemented( "vertexlit_and_unlit_generic: SELFILLUM with VERTEXCOLOR" );
			else
				flags |=
				    CVulkanContext::kFragmentSelfIllum | CVulkanContext::kVertexColorNoGammaConvert;
		}
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

void ResetTextureMatrices( int count )
{
	EnsureMatricesInit();
	for ( int i = 0; i < count && MATERIAL_TEXTURE0 + i < NUM_MATRIX_MODES; ++i )
		MatSetIdentity( g_matrices.mat[MATERIAL_TEXTURE0 + i] );
}

bool MatInvert( const float *m, float *out );

// Fast clipping (EnableFastClip / SetFastClipPlane, and the height clip that
// drives them): the plane arrives as Ax+By+Cz=D and is kept as D3D9 keeps it
// (d negated). While enabled, draws use an oblique projection whose near plane
// is the clip plane (CShaderAPIDx8::CommitFastClipPlane).
struct FastClipState
{
	bool enabled = false;
	float plane[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float heightClipZ = 0.0f;
	MaterialHeightClipMode_t heightClipMode = MATERIAL_HEIGHTCLIPMODE_DISABLE;
};
FastClipState g_FastClip;

// The projection the vertex stage draws with: MATERIAL_PROJECTION, or with fast
// clipping on the oblique projection CShaderAPIDx8::CommitFastClipPlane builds.
// User clip planes stay relative to MATERIAL_PROJECTION, as on D3D9.
const float *DrawProjection()
{
	EnsureMatricesInit();
	if ( !g_FastClip.enabled )
		return g_matrices.mat[MATERIAL_PROJECTION];
	static float s_oblique[16];
	float viewToProj[16];
	memcpy( viewToProj, g_matrices.mat[MATERIAL_PROJECTION], sizeof( viewToProj ) );
	// Pull in zNear: the shear moves it out (looking down at water clips).
	viewToProj[3 * 4 + 2] *= 0.5f;
	const float *worldToView =
	    g_UserClipTransformOverride ? g_UserClipTransform : g_matrices.mat[MATERIAL_VIEW];
	float worldToViewInv[16], viewToProjInv[16];
	if ( !MatInvert( worldToView, worldToViewInv ) || !MatInvert( viewToProj, viewToProjInv ) )
	{
		memcpy( s_oblique, viewToProj, sizeof( s_oblique ) );
		return s_oblique;
	}
	// D3DXPlaneNormalize, then the plane through the inverse transposes (row
	// vector times (M^-1)^T).
	float plane[4];
	const float length = sqrtf( g_FastClip.plane[0] * g_FastClip.plane[0] +
	                            g_FastClip.plane[1] * g_FastClip.plane[1] +
	                            g_FastClip.plane[2] * g_FastClip.plane[2] );
	for ( int k = 0; k < 4; ++k )
		plane[k] = length > 0.0f ? g_FastClip.plane[k] / length : g_FastClip.plane[k];
	const auto transformInvTrans = []( const float in[4], const float *inverse, float out[4] )
	{
		for ( int j = 0; j < 4; ++j )
		{
			float sum = 0.0f;
			for ( int k = 0; k < 4; ++k )
				sum += in[k] * inverse[j * 4 + k];
			out[j] = sum;
		}
	};
	float viewPlane[4], clipPlane[4];
	transformInvTrans( plane, worldToViewInv, viewPlane );
	transformInvTrans( viewPlane, viewToProjInv, clipPlane );
	float shear[16];
	MatSetIdentity( shear );
	// A plane with z * w > -0.4 here is behind the camera; D3D9 discards it.
	if ( clipPlane[2] * clipPlane[3] <= -0.4f )
	{
		const float clipLength =
		    sqrtf( clipPlane[0] * clipPlane[0] + clipPlane[1] * clipPlane[1] +
		           clipPlane[2] * clipPlane[2] + clipPlane[3] * clipPlane[3] );
		for ( int row = 0; row < 4; ++row )
			shear[row * 4 + 2] = clipPlane[row] / clipLength;
	}
	MatMul( viewToProj, shear, s_oblique );
	return s_oblique;
}

// Compose model * view * proj and hand it to the dynamic draw path as the MVP.
void CommitModelViewProj()
{
	EnsureMatricesInit();
	float mv[16], mvp[16];
	MatMul( g_matrices.mat[MATERIAL_MODEL], g_matrices.mat[MATERIAL_VIEW], mv );
	MatMul( mv, DrawProjection(), mvp );
	g_VulkanContext.SetDynamicTransform( mvp );
}

const float *ModelMatrix()
{
	EnsureMatricesInit();
	return g_matrices.mat[MATERIAL_MODEL];
}

// view * proj, for world-space (skinned) positions.
void CommitViewProj()
{
	EnsureMatricesInit();
	float vp[16];
	MatMul( g_matrices.mat[MATERIAL_VIEW], DrawProjection(), vp );
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
	MatMul( g_matrices.mat[MATERIAL_VIEW], DrawProjection(), c.viewProj );
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

// The skin shader's registers for the draw: the pixel constants c0..c31 as
// skin_dx9_helper.cpp's dynamic state wrote them, and skin_vs20's cViewProj,
// cBaseTexCoordTransform (SHADER_SPECIFIC_CONST_0/1) and cEyePos. The vertex
// positions arrive in world space (EmitToNativeQueue).
void CommitSkinConstants( const CShaderAPIVulkan &api )
{
	EnsureMatricesInit();
	render_vulkan::CVulkanContext::SkinConstants c;
	memcpy( c.ps, g_psConstants, sizeof( c.ps ) );
	MatMul( g_matrices.mat[MATERIAL_VIEW], DrawProjection(), c.viewProj );
	memcpy( c.texXform0, g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_0],
	    sizeof( c.texXform0 ) );
	memcpy( c.texXform1, g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_0 + 1],
	    sizeof( c.texXform1 ) );
	c.eyePos[3] = 0.0f;
	api.GetWorldSpaceCameraPosition( c.eyePos );
	c.combos = g_CurrentSkinCombos;
	c.numLights = 0;
	for ( bool enabled : g_LightEnabled )
		c.numLights += enabled ? 1 : 0;
	g_VulkanContext.SetDynamicSkinConstants( c );
}
} // namespace

// SolidEnergy's registers for the draw (solidenergy_dx9_helper.cpp's dynamic
// state): its pixel constants c0..c11, c30 (the light scale), and the vertex
// stage's registers the native stages read from the same block: c12/c13 the
// detail 1 transform (SHADER_SPECIFIC_CONST_2/3), c14/c15 the detail 2
// transform (SHADER_SPECIFIC_CONST_6/7), c16 the first vortex and noise scale
// (SHADER_SPECIFIC_CONST_9), c17 the second vortex and normal UV scale
// (SHADER_SPECIFIC_CONST_10). The push block takes cViewProj, the base
// texture transform (SHADER_SPECIFIC_CONST_0/1) and the eye position.
static void CommitSolidEnergyConstants( const CShaderAPIVulkan &api )
{
	EnsureMatricesInit();
	render_vulkan::CVulkanContext::SkinConstants c;
	memcpy( c.ps, g_psConstants, sizeof( c.ps ) );
	const auto copyVs = [&c]( int ps, int vs )
	{
		memcpy( c.ps[ps], g_vsConstants.regs[vs], sizeof( c.ps[ps] ) );
	};
	copyVs( 12, VERTEX_SHADER_SHADER_SPECIFIC_CONST_2 );
	copyVs( 13, VERTEX_SHADER_SHADER_SPECIFIC_CONST_3 );
	copyVs( 14, VERTEX_SHADER_SHADER_SPECIFIC_CONST_6 );
	copyVs( 15, VERTEX_SHADER_SHADER_SPECIFIC_CONST_7 );
	copyVs( 16, VERTEX_SHADER_SHADER_SPECIFIC_CONST_9 );
	copyVs( 17, VERTEX_SHADER_SHADER_SPECIFIC_CONST_10 );
	MatMul( g_matrices.mat[MATERIAL_VIEW], DrawProjection(), c.viewProj );
	memcpy( c.texXform0, g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_0],
	    sizeof( c.texXform0 ) );
	memcpy( c.texXform1, g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_1],
	    sizeof( c.texXform1 ) );
	c.eyePos[3] = 0.0f;
	api.GetWorldSpaceCameraPosition( c.eyePos );
	c.combos = static_cast<int>( g_psConstants[11][0] );
	c.numLights = 0;
	g_VulkanContext.SetDynamicSkinConstants( c );
}

// PBRMetalRough on a model (pbr_metalrough_native.cpp's dynamic state): c2.x
// $emissionscale, c3 the material's feature flags (normal map, emission,
// environment map, as CVulkanContext::kPbrModel*), c4..c9 the ambient cube and
// c20..c25 the lights. skin.vert's registers come from the same vertex state
// as the skin shader's. The environment map's mip count goes to c2.w.
static void CommitPbrModelConstants( const CShaderAPIVulkan &api )
{
	render_vulkan::CVulkanContext::SkinConstants c;
	memcpy( c.ps, g_psConstants, sizeof( c.ps ) );
	EnsureMatricesInit();
	MatMul( g_matrices.mat[MATERIAL_VIEW], DrawProjection(), c.viewProj );
	memcpy( c.texXform0, g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_0],
	    sizeof( c.texXform0 ) );
	memcpy( c.texXform1, g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_0 + 1],
	    sizeof( c.texXform1 ) );
	c.eyePos[3] = 0.0f;
	api.GetWorldSpaceCameraPosition( c.eyePos );
	static_assert( int( render_vulkan::CVulkanContext::kPbrModelNormalMap ) ==
	                       render::pbr::kNativeNormalMap &&
	                   int( render_vulkan::CVulkanContext::kPbrModelEmission ) ==
	                       render::pbr::kNativeEmission &&
	                   int( render_vulkan::CVulkanContext::kPbrModelEnvMap ) ==
	                       render::pbr::kNativeEnvMap &&
	                   int( render_vulkan::CVulkanContext::kPbrModelClearCoat ) ==
	                       render::pbr::kNativeClearCoat,
	    "model_pbr.frag's flags are the material's native feature flags" );
	c.combos = static_cast<int>( g_psConstants[3][0] ) &
	           ( render::pbr::kNativeNormalMap | render::pbr::kNativeEmission |
	               render::pbr::kNativeEnvMap | render::pbr::kNativeClearCoat );
	const int envmap = g_boundPbrEnvmapHandle;
	c.ps[2][3] = envmap >= 0 && size_t( envmap ) < g_TextureRecords.size()
	                 ? static_cast<float>( g_TextureRecords[size_t( envmap )].mipLevels )
	                 : 1.0f;
	c.numLights = 0;
	for ( bool enabled : g_LightEnabled )
		c.numLights += enabled ? 1 : 0;
	g_VulkanContext.SetDynamicSkinConstants( c );
}

static const float *MonitorTexture2Rows()
{
	return &g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_2][0];
}

// skin_ps20b's static combos (fxctmp9/skin_ps20b.inc) as skin.frag's flags, or
// -1 when the pixel shader is not skin_ps20b. The combos skin.frag does not
// implement are reported.
static int SnapshotSkinCombos( const CShaderShadowVulkan &shadow )
{
	if ( V_stricmp( shadow.m_pixelShaderName, "skin_ps20b" ) )
		return -1;
	const int index = shadow.m_pixelShaderIndex;
	const auto combo = [index]( int stride, int count )
	{
		return ( index / stride ) % count != 0;
	};
	int flags = 0;
	flags |= combo( 860160, 2 ) ? 1 : 0;   // FASTPATH_NOBUMP
	flags |= combo( 1280, 2 ) ? 2 : 0;     // LIGHTWARPTEXTURE
	flags |= combo( 2560, 2 ) ? 4 : 0;     // PHONGWARPTEXTURE
	flags |= combo( 160, 2 ) ? 8 : 0;      // SELFILLUM
	flags |= combo( 320, 2 ) ? 16 : 0;     // SELFILLUMFRESNEL
	flags |= combo( 143360, 2 ) ? 32 : 0;  // RIMLIGHT
	flags |= combo( 1720320, 2 ) ? 64 : 0; // BLENDTINTBYBASEALPHA
	if ( combo( 80, 2 ) )
		NoteUnimplemented( "skin_ps20b: CUBEMAP (drawn without the environment map)" );
	if ( combo( 640, 2 ) )
		NoteUnimplemented( "skin_ps20b: FLASHLIGHT" );
	if ( combo( 5120, 2 ) )
		NoteUnimplemented( "skin_ps20b: WRINKLEMAP" );
	if ( combo( 71680, 2 ) )
		NoteUnimplemented( "skin_ps20b: DETAILTEXTURE" );
	return flags;
}

// Returns the snapshot id for the shader state
// The name BeginPass routes a snapshot by: its pixel shader, with the static
// combo where the combo changes what the native pipeline computes
// ("portal_refract#<STAGE>"), or its vertex shader when it has no pixel shader
// (WriteZ, a depth- or stencil-only BufferClearObeyStencil).
static std::string SnapshotShaderRoute( const CShaderShadowVulkan &shadow )
{
	const int skinCombos = SnapshotSkinCombos( shadow );
	if ( skinCombos >= 0 )
		return "skin#" + std::to_string( skinCombos );
	if ( !V_stricmp( shadow.m_pixelShaderName, "solidenergy_ps20b" ) )
		return "solidenergy";
	if ( !V_stricmp( shadow.m_pixelShaderName, "pbr_metalrough_world_ps" ) )
		return "pbr_model";
	if ( !V_strnicmp( shadow.m_pixelShaderName, "portal_refract_ps20b", 20 ) )
		return "portal_refract#" + std::to_string( ( shadow.m_pixelShaderIndex / 4 ) % 3 );
	if ( !V_strnicmp( shadow.m_pixelShaderName, "portal_refract_ps20", 19 ) )
		return "portal_refract#" + std::to_string( shadow.m_pixelShaderIndex % 3 );
	if ( !shadow.m_pixelShaderName[0] )
		return std::string( "vs:" ) + shadow.m_vertexShaderName;
	return shadow.m_pixelShaderName;
}

// The tone-mapping scale type a pass's pixel shader ends in (common_ps_fxc.h
// FinalOutput's iTONEMAP_SCALE_TYPE): LINEAR multiplies by cLightScale.x (the
// tone-mapping scale in integer HDR), GAMMA by its 1/2.2 power, NONE by 1.
enum SnapshotToneMap
{
	kToneMapUnknown = 0,
	kToneMapNone,
	kToneMapLinear,
	kToneMapGamma
};

// sprite_ps2x.fxc's CONSTANTCOLOR and HDRTYPE static combos (fxctmp9
// sprite_ps20b.inc strides 16 and 32; sprite_ps20.inc 8 and 16), packed as
// CONSTANTCOLOR | HDRTYPE << 1; -1 for other pixel shaders.
static int SnapshotSpriteCombos( const CShaderShadowVulkan &shadow )
{
	const int index = shadow.m_pixelShaderIndex;
	if ( !V_stricmp( shadow.m_pixelShaderName, "sprite_ps20b" ) )
		return ( ( index / 16 ) % 2 ) | ( ( ( index / 32 ) % 3 ) << 1 );
	if ( !V_stricmp( shadow.m_pixelShaderName, "sprite_ps20" ) )
		return ( ( index / 8 ) % 2 ) | ( ( ( index / 16 ) % 3 ) << 1 );
	return -1;
}

// Which pixel shaders fog, and from which registers (each shader's own
// declarations: g_FogParams and g_EyePos / g_EyePos_SpecExponent).
static PixelFogInputs SnapshotPixelFog( const CShaderShadowVulkan &shadow )
{
	const char *ps = shadow.m_pixelShaderName;
	const auto is = [ps]( const char *prefix ) { return !V_strnicmp( ps, prefix, strlen( prefix ) ); };
	PixelFogInputs fog;
	if ( is( "lightmappedgeneric_ps2" ) )
	{
		fog.mode = kPixelFogCombo;
		fog.paramsRegister = 11;
		fog.eyeRegister = 10;
	}
	else if ( is( "vertexlit_and_unlit_generic_" ) )
	{
		// ps20 selects the type with a combo; ps20b and ps30 read it from c12.x.
		fog.mode = V_stristr( ps, "_ps20b" ) || V_stristr( ps, "_ps30" ) ? kPixelFogConstantType
		                                                                  : kPixelFogCombo;
		fog.paramsRegister = 21;
		fog.eyeRegister = 20;
	}
	else if ( is( "cable_ps2" ) || is( "monitorscreen_ps2" ) || is( "refract_ps2" ) ||
	          is( "unlittwotexture_ps2" ) || is( "sprite_ps2" ) )
	{
		fog.mode = kPixelFogCombo;
		fog.paramsRegister = 12; // PSREG_FOG_PARAMS
		fog.eyeRegister = 11;    // PSREG_EYEPOS_SPEC_EXPONENT
	}
	else if ( is( "decalmodulate_ps2" ) )
	{
		fog.mode = kPixelFogDecal;
		fog.paramsRegister = 12;
		fog.eyeRegister = 11;
	}
	return fog;
}

static int SnapshotToneMapType( const CShaderShadowVulkan &shadow )
{
	const char *ps = shadow.m_pixelShaderName;
	const auto is = [ps]( const char *prefix ) { return !V_strnicmp( ps, prefix, strlen( prefix ) ); };
	if ( !ps[0] || is( "white_ps2" ) || is( "bufferclearobeystencil_ps2" ) ||
	     is( "luminance_compare_ps2" ) )
		return kToneMapNone; // no color, a clear color, or a coverage count
	if ( is( "lightmappedgeneric_ps2" ) || is( "vertexlit_and_unlit_generic_" ) ||
	     is( "skin_ps2" ) || is( "sky_ps2" ) || is( "spritecard_ps2" ) || is( "cable_ps2" ) ||
	     is( "unlittwotexture_ps2" ) )
		return kToneMapLinear;
	if ( is( "shadow_ps2" ) || is( "decalmodulate_ps2" ) || is( "refract_ps2" ) ||
	     is( "monitorscreen_ps2" ) )
		return kToneMapNone;
	// PortalRefract: only stage 2 (the flames) scales; RenderPass knows the stage.
	// SolidEnergy clamps the scale; RenderPass applies it.
	if ( is( "portal_refract_ps2" ) || is( "solidenergy_ps2" ) )
		return kToneMapNone;
	// sprite_ps2x: LINEAR with its SRGB combo, GAMMA without (stride 96 in the
	// ps20b build, 48 in ps20).
	if ( !V_stricmp( ps, "sprite_ps20b" ) )
		return ( shadow.m_pixelShaderIndex / 96 ) % 2 ? kToneMapLinear : kToneMapGamma;
	if ( !V_stricmp( ps, "sprite_ps20" ) )
		return ( shadow.m_pixelShaderIndex / 48 ) % 2 ? kToneMapLinear : kToneMapGamma;
	return kToneMapUnknown;
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
	const int toneMap = SnapshotToneMapType( g_ShaderShadow );
	const int spriteCombos = SnapshotSpriteCombos( g_ShaderShadow );
	char key[176];
	V_snprintf( key, sizeof( key ), "|%d|%llx|%d|%.6g|%llx|%d|%d|%d|%d|%x|%d|%d",
	    static_cast<int>( id ),
	    static_cast<unsigned long long>( render_vulkan::CVulkanContext::RasterStateKey( raster ) ),
	    static_cast<int>( g_ShaderShadow.m_polyOffset ), alphaRef,
	    static_cast<unsigned long long>( g_ShaderShadow.m_vertexUsage ), shaderFlags,
	    g_ShaderShadow.m_vertexShaderIndex, static_cast<int>( g_ShaderShadow.m_fogMode ),
	    g_ShaderShadow.m_disableFogGammaCorrection ? 1 : 0, g_ShaderShadow.m_enabledSamplers,
	    toneMap, spriteCombos );
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
	g_snapshotPolyOffset.push_back( g_ShaderShadow.m_polyOffset );
	g_snapshotAlphaRef.push_back( alphaRef );
	g_snapshotVertexUsage.push_back( g_ShaderShadow.m_vertexUsage );
	g_snapshotColorFlags.push_back( shaderFlags );
	// vertexlit_and_unlit_generic_ps2x/_bump_ps2x: g_DiffuseModulation : c1.
	g_snapshotModulationInPixelC1.push_back(
	    !V_strnicmp( g_ShaderShadow.m_pixelShaderName, "vertexlit_and_unlit_generic_", 28 ) );
	g_snapshotVertexLit.push_back( SnapshotVertexLitCombo( g_ShaderShadow ) );
	g_snapshotFog.push_back(
	    { g_ShaderShadow.m_fogMode, g_ShaderShadow.m_disableFogGammaCorrection } );
	g_snapshotEnabledSamplers.push_back( g_ShaderShadow.m_enabledSamplers );
	g_snapshotToneMap.push_back( toneMap );
	g_snapshotSpriteCombos.push_back( spriteCombos );
	g_snapshotPixelFog.push_back( SnapshotPixelFog( g_ShaderShadow ) );
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

// The constant color, as CShaderAPIDx8 keeps it: a D3DCOLOR (D3DRS_TEXTUREFACTOR)
// with float channels truncated to 0..255. Fixed-function stages read it.
static unsigned int PackConstantColor( int r, int g, int b, int a )
{
	const auto clampByte = []( int v )
	{
		return static_cast<unsigned int>( v < 0 ? 0 : ( v > 255 ? 255 : v ) );
	};
	return ( clampByte( a ) << 24 ) | ( clampByte( r ) << 16 ) | ( clampByte( g ) << 8 ) |
	       clampByte( b );
}

void CShaderAPIVulkan::Color3f( float r, float g, float b )
{
	g_ConstantColor = PackConstantColor(
	    static_cast<int>( r * 255 ), static_cast<int>( g * 255 ), static_cast<int>( b * 255 ), 255 );
}

void CShaderAPIVulkan::Color3fv( float const *pColor )
{
	Color3f( pColor[0], pColor[1], pColor[2] );
}

void CShaderAPIVulkan::Color4f( float r, float g, float b, float a )
{
	g_ConstantColor = PackConstantColor( static_cast<int>( r * 255 ), static_cast<int>( g * 255 ),
	    static_cast<int>( b * 255 ), static_cast<int>( a * 255 ) );
}

void CShaderAPIVulkan::Color4fv( float const *pColor )
{
	Color4f( pColor[0], pColor[1], pColor[2], pColor[3] );
}

void CShaderAPIVulkan::Color3ub( unsigned char r, unsigned char g, unsigned char b )
{
	g_ConstantColor = PackConstantColor( r, g, b, 255 );
}

void CShaderAPIVulkan::Color3ubv( unsigned char const *rgb )
{
	Color3ub( rgb[0], rgb[1], rgb[2] );
}

void CShaderAPIVulkan::Color4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	g_ConstantColor = PackConstantColor( r, g, b, a );
}

void CShaderAPIVulkan::Color4ubv( unsigned char const *rgba )
{
	Color4ub( rgba[0], rgba[1], rgba[2], rgba[3] );
}

// D3DRS_SHADEMODE. The material system selects flat shading for $flat materials;
// D3D9 then takes each triangle's vertex colors from its first vertex. The
// native pipelines interpolate them (RenderPass reports such draws).
void CShaderAPIVulkan::ShadeMode( ShaderShadeMode_t mode )
{
	g_ShadeMode = mode;
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

// Forces the Z comparison to EQUAL for every draw until disabled, keeping each
// snapshot's Z enable and write (CTransitionTable::ForceDepthFuncEquals). It is
// applied where a pass selects its raster state (ApplyShadowStateOverrides).
void CShaderAPIVulkan::ForceDepthFuncEquals( bool bEnable )
{
	m_bForceDepthFuncEquals = bEnable;
}

// Forces Z buffering on or off. BeginPass and RenderPass apply the override
// when they select a pass's raster state, so it needs no flush. The snapshot
// keeps its compare function, as in D3D9.
void CShaderAPIVulkan::OverrideDepthEnable( bool bEnable, bool bDepthEnable )
{
	m_bOverrideDepthEnable = bEnable;
	m_bOverrideDepthWrite = bDepthEnable;
}

void CShaderAPIVulkan::ApplyShadowStateOverrides(
    render_vulkan::CVulkanContext::DynRasterState &raster ) const
{
	if ( m_bForceDepthFuncEquals )
		raster.depthCompare = VK_COMPARE_OP_EQUAL;
	if ( m_bOverrideDepthEnable )
	{
		raster.depthTest = true;
		raster.depthWrite = m_bOverrideDepthWrite;
	}
	if ( m_bOverrideAlphaWrite )
		raster.alphaWrite = m_bOverriddenAlphaWrite;
	if ( m_bOverrideColorWrite )
		raster.colorWrite = m_bOverriddenColorWrite;
}

// Force alpha (or RGB) writes on or off for every draw until the override ends,
// whatever each snapshot enabled (CTransitionTable::OverrideAlphaWriteEnable and
// OverrideColorWriteEnable). Applied with the other shadow-state overrides.
void CShaderAPIVulkan::OverrideAlphaWriteEnable( bool bOverrideEnable, bool bAlphaWriteEnable )
{
	m_bOverrideAlphaWrite = bOverrideEnable;
	m_bOverriddenAlphaWrite = bAlphaWriteEnable;
}

void CShaderAPIVulkan::OverrideColorWriteEnable( bool bOverrideEnable, bool bColorWriteEnable )
{
	m_bOverrideColorWrite = bOverrideEnable;
	m_bOverriddenColorWrite = bColorWriteEnable;
}

// Height clipping (water reflection and refraction views): a fast-clip plane at
// the height, facing up or down (CShaderAPIDx8::UpdateFastClipUserClipPlane).
static void UpdateHeightClipPlane( CShaderAPIVulkan &api )
{
	switch ( g_FastClip.heightClipMode )
	{
	case MATERIAL_HEIGHTCLIPMODE_RENDER_ABOVE_HEIGHT:
	{
		const float plane[4] = { 0.0f, 0.0f, 1.0f, g_FastClip.heightClipZ };
		api.EnableFastClip( true );
		api.SetFastClipPlane( plane );
		break;
	}
	case MATERIAL_HEIGHTCLIPMODE_RENDER_BELOW_HEIGHT:
	{
		const float plane[4] = { 0.0f, 0.0f, -1.0f, -g_FastClip.heightClipZ };
		api.EnableFastClip( true );
		api.SetFastClipPlane( plane );
		break;
	}
	default:
		api.EnableFastClip( false );
		break;
	}
}

void CShaderAPIVulkan::SetHeightClipZ( float z )
{
	if ( z == g_FastClip.heightClipZ )
		return;
	g_FastClip.heightClipZ = z;
	UpdateVertexShaderFogParams();
	UpdateHeightClipPlane( *this );
}

void CShaderAPIVulkan::SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode )
{
	if ( heightClipMode == g_FastClip.heightClipMode )
		return;
	g_FastClip.heightClipMode = heightClipMode;
	UpdateVertexShaderFogParams();
	UpdateHeightClipPlane( *this );
}

// Sets the lights. As CShaderAPIDx8::SetLight: a light of an unknown type or
// MATERIAL_LIGHT_DISABLE is disabled.
void CShaderAPIVulkan::SetLight( int lightNum, const LightDesc_t &desc )
{
	if ( lightNum < 0 || lightNum >= kMaxLocalLights )
		return;
	g_LightDescs[lightNum] = desc;
	g_LightEnabled[lightNum] = desc.m_Type == MATERIAL_LIGHT_POINT ||
	                           desc.m_Type == MATERIAL_LIGHT_DIRECTIONAL ||
	                           desc.m_Type == MATERIAL_LIGHT_SPOT;
}

// Sets lighting origin for the current model
void CShaderAPIVulkan::SetLightingOrigin( Vector vLightingOrigin )
{
	g_LightingOrigin[0] = vLightingOrigin.x;
	g_LightingOrigin[1] = vLightingOrigin.y;
	g_LightingOrigin[2] = vLightingOrigin.z;
}

// As CShaderAPIDx8::SetPixelShaderStateAmbientLightCube: the six cube faces
// into pixel constants pshReg.., black when the shader forces it.
void CShaderAPIVulkan::SetPixelShaderStateAmbientLightCube( int pshReg, bool bForceToBlack )
{
	float cube[6][4] = {};
	if ( !bForceToBlack )
		memcpy( cube, g_AmbientCube, sizeof( cube ) );
	SetPixelShaderConstant( pshReg, cube[0], 6 );
}

// As CShaderAPIDx8::CommitPixelShaderLighting: the sorted lights' colors and
// positions into pixel constants pshReg..pshReg+5, the fourth light spread over
// the w components; a directional light becomes a point 10000 units from the
// lighting origin, against its direction.
void CShaderAPIVulkan::CommitPixelShaderLighting( int pshReg )
{
	const float kFarAway = 10000.0f;
	VertexLightConstants lights[kMaxLocalLights];
	const int count = BuildVertexLightConstants( lights );
	float state[6][4] = {};
	auto position = [&]( const VertexLightConstants &c, float out[3] )
	{
		for ( int k = 0; k < 3; ++k )
			out[k] = c.color[3] > 0.5f ? g_LightingOrigin[k] - c.dir[k] * kFarAway : c.pos[k];
	};
	for ( int i = 0; i < count && i < 3; ++i )
	{
		for ( int k = 0; k < 3; ++k )
			state[2 * i][k] = lights[i].color[k];
		position( lights[i], state[2 * i + 1] );
	}
	if ( count > 3 )
	{
		float pos[3];
		position( lights[3], pos );
		state[0][3] = lights[3].color[0];
		state[1][3] = lights[3].color[1];
		state[2][3] = lights[3].color[2];
		state[3][3] = pos[0];
		state[4][3] = pos[1];
		state[5][3] = pos[2];
	}
	SetPixelShaderConstant( pshReg, state[0], 6 );
}

// D3DRS_AMBIENT, the fixed-function ambient term, kept as D3D9 keeps it. Shader
// passes light from the ambient cube (SetAmbientLightCube).
void CShaderAPIVulkan::SetAmbientLight( float r, float g, float b )
{
	g_AmbientLightColor = PackConstantColor( static_cast<int>( r * 255 ),
	    static_cast<int>( g * 255 ), static_cast<int>( b * 255 ), 255 );
}

void CShaderAPIVulkan::SetAmbientLightCube( Vector4D cube[6] )
{
	for ( int face = 0; face < 6; ++face )
		for ( int k = 0; k < 4; ++k )
			g_AmbientCube[face][k] = cube[face][k];
}

// Get lights
int CShaderAPIVulkan::GetMaxLights( void ) const
{
	return MaxNumLights();
}

const LightDesc_t &CShaderAPIVulkan::GetLight( int lightNum ) const
{
	static LightDesc_t s_Disabled;
	if ( lightNum < 0 || lightNum >= kMaxLocalLights )
		return s_Disabled;
	return g_LightDescs[lightNum];
}

// Render state for the ambient light cube (vertex shaders): D3D9 loads c21..c26
// here; the vertex lighting reads the cube where the draw is assembled.
void CShaderAPIVulkan::SetVertexShaderStateAmbientLightCube()
{
}

// D3D9 loads the bone palette into cModel[] here for the vertex shader. This
// backend skins where it assembles the draw's vertices (SkinPosition), reading
// the palette LoadBoneMatrix keeps, so the matrices are already in place.
void CShaderAPIVulkan::SetSkinningMatrices()
{
}

// Gets the lightmap dimensions
void CShaderAPIVulkan::GetLightmapDimensions( int *w, int *h )
{
	g_pShaderUtil->GetLightmapDimensions( w, h );
}

// Flushes any primitives that are buffered. Every draw is recorded into the
// frame stream as it is made, so there is nothing buffered here; what remains of
// CShaderAPIDx8::FlushBufferedPrimitives is the render context's hook, which
// synchronizes its lazily committed matrices with this API.
void CShaderAPIVulkan::FlushBufferedPrimitives()
{
	if ( ShaderUtil() )
		ShaderUtil()->OnFlushBufferedPrimitives();
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
		g_CurrentSkinCombos = -1;
		if ( !name.compare( 0, 5, "skin#" ) )
		{
			shader = render_vulkan::CVulkanContext::kDynShaderSkin;
			g_CurrentSkinCombos = atoi( name.c_str() + 5 );
		}
		g_CurrentSolidEnergy = name == "solidenergy";
		if ( g_CurrentSolidEnergy )
			shader = render_vulkan::CVulkanContext::kDynShaderSolidEnergy;
		g_CurrentPbrModel = name == "pbr_model";
		if ( g_CurrentPbrModel )
			shader = render_vulkan::CVulkanContext::kDynShaderPbrModel;
		g_SamplesBaseTexture = ( shader == render_vulkan::CVulkanContext::kDynShaderTextured ||
		                         shader == render_vulkan::CVulkanContext::kDynShaderSkin ||
		                         shader == render_vulkan::CVulkanContext::kDynShaderSolidEnergy ||
		                         shader == render_vulkan::CVulkanContext::kDynShaderPbrModel );
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
		g_CurrentPolyOffset = g_snapshotPolyOffset[index];
		ApplyDepthBiasState( g_CurrentRaster );
		render_vulkan::CVulkanContext::DynRasterState raster = g_CurrentRaster;
		ApplyShadowStateOverrides( raster );
		g_VulkanContext.SelectDynamicRasterState( raster );
	}
	g_CurrentColorFlags = index < g_snapshotColorFlags.size() ? g_snapshotColorFlags[index] : 0;
	g_CurrentModulationInPixelC1 =
	    index < g_snapshotModulationInPixelC1.size() && g_snapshotModulationInPixelC1[index];
	g_CurrentVertexLit =
	    index < g_snapshotVertexLit.size() ? g_snapshotVertexLit[index] : VertexLitCombo();
	g_VulkanContext.SelectDynamicColorSpace( g_CurrentColorFlags );
	// The lightmap and samplers 1..15 are bound per pass by the shader's dynamic
	// state.
	g_boundLightmapHandle = -1;
	g_boundEnvmapHandle = -1;
	g_boundRefractNormalHandle = -1;
	g_boundRefractCubeHandle = -1;
	g_boundNormalMaskHandle = -1;
	g_boundPbrNormalHandle = -1;
	g_boundPbrMraoHandle = -1;
	g_boundPbrEnvmapHandle = -1;
	g_boundPbrEmissionHandle = -1;
	g_VulkanContext.BindManagedLightmap( -1 );
	for ( int sampler = 1; sampler < render_vulkan::CVulkanContext::kMaxSamplers; ++sampler )
		g_VulkanContext.BindManagedSampler( sampler, -1 );
	// Apply the $alphatest reference this snapshot recorded (< 0 = disabled).
	if ( index < g_snapshotAlphaRef.size() )
	{
		g_CurrentAlphaRef = g_snapshotAlphaRef[index];
		g_VulkanContext.SelectDynamicAlphaTest( g_snapshotAlphaRef[index] );
	}
	// The pass's fog (CTransitionTable applies it with the shadow state) and the
	// samplers it enabled.
	g_CurrentEnabledSamplers =
	    index < g_snapshotEnabledSamplers.size() ? g_snapshotEnabledSamplers[index] : ~0u;
	g_CurrentToneMap = index < g_snapshotToneMap.size() ? g_snapshotToneMap[index] : 0;
	g_CurrentSpriteCombos =
	    index < g_snapshotSpriteCombos.size() ? g_snapshotSpriteCombos[index] : -1;
	g_CurrentPixelFog =
	    index < g_snapshotPixelFog.size() ? g_snapshotPixelFog[index] : PixelFogInputs();
	if ( index < g_snapshotFog.size() )
	{
		g_CurrentShadowFogMode = g_snapshotFog[index].mode;
		g_CurrentDisableFogGammaCorrection = g_snapshotFog[index].disableGammaCorrection;
	}
	ApplyFogMode( g_CurrentShadowFogMode,
	    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kColorSrgbWrite ) != 0,
	    g_CurrentDisableFogGammaCorrection );
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
// The pass's pixel-shader constants the native pipeline takes as draw state,
// once the shader's dynamic state has written them (just before emitting).
// The pass's pixel fog as its shader computes it (CVulkanContext::DrawFog): the
// fog color from c29, the type from the combo the pass selected or its c12.x,
// the parameters and eye z from the registers the shader declares, and the row
// that takes this draw's vertex positions to world z (skinned positions are
// already in world space).
static void CommitPassFog()
{
	render_vulkan::CVulkanContext::DrawFog fog;
	if ( g_CurrentPixelFog.mode != kPixelFogNone )
	{
		const float fogType = g_CurrentPixelFog.mode == kPixelFogConstantType
		                          ? g_psConstants[12][0]
		                          : ( g_Fog.sceneMode == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ? 1.0f : 0.0f );
		memcpy( fog.color, g_psConstants[kPsRegLinearFogColor], 3 * sizeof( float ) );
		fog.color[3] = fogType;
		memcpy( fog.params, g_psConstants[g_CurrentPixelFog.paramsRegister], sizeof( fog.params ) );
		fog.misc[0] = g_psConstants[g_CurrentPixelFog.eyeRegister][2];
		fog.misc[1] = g_CurrentPixelFog.mode == kPixelFogDecal ? 1.0f : 0.0f;
		if ( g_NumBoneWeights <= 0 )
		{
			const float *model = ModelMatrix();
			fog.worldZ[0] = model[2];
			fog.worldZ[1] = model[6];
			fog.worldZ[2] = model[10];
			fog.worldZ[3] = model[14];
		}
	}
	g_VulkanContext.SetDynamicFog( fog );
}

static void CommitPassPixelConstants()
{
	CommitPassFog();
	// vertexlit_and_unlit_generic's alpha = lerp( alpha, alpha * i.color.a,
	// g_fVertexAlpha ), where g_fVertexAlpha is c12.w ($vertexalpha, 0 or 1).
	int colorFlags = g_CurrentColorFlags;
	if ( ( colorFlags & render_vulkan::CVulkanContext::kFragmentModulateVertexAlpha ) &&
	     g_psConstants[12][3] < 0.5f )
		colorFlags &= ~render_vulkan::CVulkanContext::kFragmentModulateVertexAlpha;
	g_VulkanContext.SelectDynamicColorSpace( colorFlags );
	if ( colorFlags & render_vulkan::CVulkanContext::kFragmentSky )
	{
		// Sky_DX9's sky_ps2x.fxc reads $color from pixel constant c0.
		g_VulkanContext.SetDynamicModulation( g_psConstants[0] );
		return;
	}
	if ( g_CurrentSpriteCombos >= 0 )
	{
		// sprite_ps2x.fxc: sample *= g_Color (c0) with CONSTANTCOLOR, and its RGB
		// *= g_HDRColorScale (c1.x) with HDRTYPE and the dynamic HDRENABLED.
		float modulation[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		if ( g_CurrentSpriteCombos & 1 )
			memcpy( modulation, g_psConstants[0], sizeof( modulation ) );
		if ( ( g_CurrentSpriteCombos >> 1 ) != 0 && ( g_PixelShaderDynamicIndex & 1 ) )
		{
			for ( int k = 0; k < 3; ++k )
				modulation[k] *= g_psConstants[1][0];
		}
		g_VulkanContext.SetDynamicModulation( modulation );
		return;
	}
	if ( colorFlags & render_vulkan::CVulkanContext::kFragmentMonitor )
	{
		// MonitorScreen's c1 contrast is a scalar material parameter. The
		// saturation and tint in c2/c3 travel with the converted vertices.
		g_VulkanContext.SetDynamicMonitorContrast( g_psConstants[1][0] );
		return;
	}
	// Its albedo and alpha are scaled by g_DiffuseModulation (c1): $color and
	// $alpha, and ColorModulate / AlphaModulate (screen fades), linear, written by
	// every pass's dynamic state (SetModulationPixelShaderDynamicState_LinearColorSpace).
	if ( !g_CurrentModulationInPixelC1 )
		return;
	if ( colorFlags & render_vulkan::CVulkanContext::kFragmentSelfIllum )
	{
		// SELFILLUM's blend target is g_SelfIllumTint (c4.rgb) times albedo; the
		// pipeline takes it as c1 * c4 (demo_dyn_tex.frag). c3.w = 1 reads the
		// mask from $selfillummask (sampler 11) instead of the base alpha.
		if ( g_psConstants[3][3] != 0.0f )
			NoteUnimplemented(
			    "vertexlit_and_unlit_generic: $selfillummask (drawn with base alpha)" );
		const float modulation[4] = { g_psConstants[1][0] * g_psConstants[4][0],
		    g_psConstants[1][1] * g_psConstants[4][1], g_psConstants[1][2] * g_psConstants[4][2],
		    g_psConstants[1][3] };
		g_VulkanContext.SetDynamicModulation( modulation );
		return;
	}
	g_VulkanContext.SetDynamicModulation( g_psConstants[1] );
}

static bool NativePipelineImplementsShader( const char *shaderName )
{
	static const char *const kImplemented[] = {
	    "Sky_DX9",
	    "MonitorScreen_DX9",
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
	    "Refract_DX90",
	    // Depth-only, and the stencil-obeying clears (ClearBuffersObeyStencil).
	    "WriteZ_DX9",
	    // Pixel visibility's occlusion-query proxy: writez_vs20 with color, alpha
	    // and depth writes off, counted by the query around it.
	    "Occlusion_DX9",
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
	// SolidEnergy (Portal 2's fizzlers, bridges, beams) has its own shaders on
	// the skin pipeline's layout (shaders/solidenergy.*).
	if ( !V_stricmp( shaderName, "SolidEnergy_dx9" ) )
		return g_VulkanContext.SolidEnergyPipelineSupported();
	// Both the canonical native material and legacy PBR's sampler contract can
	// feed WMSH tangents and the map-scoped HDR lightmap. Ordinary dynamic
	// meshes still need their own PBR pipeline cohort.
	if ( render::pbr::IsMetalRoughShader( shaderName ) || !V_stricmp( shaderName, "PBR" ) )
	{
		if ( g_pRenderMesh && g_pRenderMesh->IsWorldMeshBatch() )
			return g_VulkanContext.PbrWorldPipelineSupported();
		// Other meshes: the canonical material's model pipeline. Legacy PBR
		// keeps its WMSH-only preview bridge.
		return render::pbr::IsMetalRoughShader( shaderName ) && g_CurrentPbrModel &&
		       g_VulkanContext.PbrModelPipelineSupported();
	}
	return false;
}

static bool NativeRefractMaterialSupported( IMaterialInternal *material )
{
	if ( !material || V_stricmp( material->GetShaderName(), "Refract_DX90" ) )
		return true;
	const auto enabled = [material]( const char *name )
	{
		bool found = false;
		IMaterialVar *var = material->FindVar( name, &found, false );
		return found && var && var->GetIntValue() != 0;
	};
	const auto hasTexture = [material]( const char *name )
	{
		bool found = false;
		IMaterialVar *var = material->FindVar( name, &found, false );
		return found && var && var->IsTexture();
	};
	if ( enabled( "$masked" ) || enabled( "$fadeoutonsilhouette" ) ||
	     enabled( "$vertexcolormodulate" ) || hasTexture( "$normalmap2" ) ||
	     hasTexture( "$refracttinttexture" ) )
		return false;
	bool found = false;
	IMaterialVar *saturation = material->FindVar( "$envmapsaturation", &found, false );
	if ( found && saturation )
	{
		float rgb[3];
		saturation->GetVecValue( rgb, 3 );
		for ( float channel : rgb )
			if ( fabsf( channel - 1.0f ) > 0.0001f )
				return false;
	}
	return g_boundEnvmapHandle >= 0 && g_boundRefractNormalHandle >= 0 &&
	       g_VulkanContext.ManagedTextureIsCube( g_boundRefractCubeHandle );
}

void CShaderAPIVulkan::RenderPass( int nPass, int nPassCount )
{
	g_LastDropReason = "";
	// Without a bound material (the conformance fixtures drive the device
	// directly) the snapshot's own pipeline selection stands.
	// screenspace_general is one material shader over many pixel shaders; the
	// pass is implemented when its pixel shader is (dev/lumcompare's).
	// VertexLitGeneric's $phong path needs the skin pipeline, which a device
	// without seven descriptor sets does not get.
	const bool skinUnavailable =
	    g_CurrentSkinCombos >= 0 && !g_VulkanContext.SkinPipelineSupported();
	const bool implemented =
	    !skinUnavailable &&
	    ( !g_pBoundMaterial ||
	        NativePipelineImplementsShader( g_pBoundMaterial->GetShaderName() ) ||
	        ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentLuminanceCompare ) );
	if ( !implemented )
	{
		DropDraw( "draw dropped: material shader not implemented by the native pipeline" );
		NoteDroppedMaterial();
	}
	else if ( !NativeRefractMaterialSupported( g_pBoundMaterial ) )
	{
		DropDraw( "draw dropped: Refract_DX90 material needs an unsupported feature or texture" );
		NoteDroppedMaterial();
	}
	else if ( g_pRenderMesh )
	{
		if ( g_ShadeMode == SHADER_FLAT )
			NoteUnimplemented( "ShadeMode(SHADER_FLAT): vertex colors interpolated smoothly" );
		// LightmappedGeneric ends in FinalOutput( ..., TONEMAP_SCALE_LINEAR ): its
		// color is scaled by the tone-mapping scale, which is 1 without HDR. Other
		// shaders choose their tone-map type per combo (below).
		// D3D9's effective cull mode: the snapshot's culling with the dynamic face.
		render_vulkan::CVulkanContext::DynRasterState raster = g_CurrentRaster;
		ApplyDepthBiasState( raster );
		if ( raster.cullMode != VK_CULL_MODE_NONE )
			raster.cullMode = g_DesiredCullMode == MATERIAL_CULLMODE_CW ? VK_CULL_MODE_FRONT_BIT
			                                                            : VK_CULL_MODE_BACK_BIT;
		ApplyStencilState( raster );
		ApplyShadowStateOverrides( raster );
		g_VulkanContext.SelectDynamicRasterState( raster );
		CommitUserClipPlanes();
		if ( g_CurrentPortalStage >= 0 )
			CommitPortalConstants();
		if ( g_CurrentSkinCombos >= 0 )
			CommitSkinConstants( *this );
		if ( g_CurrentSolidEnergy )
			CommitSolidEnergyConstants( *this );
		if ( g_CurrentPbrModel )
			CommitPbrModelConstants( *this );
		const bool lightmapped = g_boundLightmapHandle >= 0;
		const bool refract =
		    g_pBoundMaterial && !V_stricmp( g_pBoundMaterial->GetShaderName(), "Refract_DX90" );
		if ( refract )
		{
			g_CurrentColorFlags |= render_vulkan::CVulkanContext::kFragmentRefract |
			                       render_vulkan::CVulkanContext::kColorSrgbReadBase;
			bool found = false;
			IMaterialVar *blur = g_pBoundMaterial->FindVar( "$bluramount", &found, false );
			if ( found && blur->GetIntValue() > 0 )
				g_CurrentColorFlags |= render_vulkan::CVulkanContext::kFragmentRefractBlur;
			g_VulkanContext.BindManagedTexture( g_boundEnvmapHandle );
			g_VulkanContext.BindManagedSampler( 1, g_boundRefractNormalHandle );
			g_VulkanContext.BindManagedSampler( 2, g_boundRefractCubeHandle );
			g_VulkanContext.SetDynamicBaseTexTransform(
			    g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_1],
			    g_vsConstants.regs[VERTEX_SHADER_SHADER_SPECIFIC_CONST_2] );
		}
		if ( lightmapped && g_pBoundMaterial &&
		     !V_stricmp( g_pBoundMaterial->GetShaderName(), "LightmappedGeneric" ) &&
		     g_VulkanContext.ManagedTextureIsCube( g_boundEnvmapHandle ) )
		{
			const bool normalAlphaMask =
			    g_pBoundMaterial->GetMaterialVarFlag( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
			if ( !normalAlphaMask || g_boundNormalMaskHandle >= 0 )
			{
				g_CurrentColorFlags |= render_vulkan::CVulkanContext::kFragmentLightmappedEnvmap;
				if ( g_pBoundMaterial->GetMaterialVarFlag( MATERIAL_VAR_BASEALPHAENVMAPMASK ) )
					g_CurrentColorFlags |=
					    render_vulkan::CVulkanContext::kFragmentBaseAlphaEnvmapMask;
				if ( normalAlphaMask )
					g_CurrentColorFlags |=
					    render_vulkan::CVulkanContext::kFragmentNormalAlphaEnvmapMask;
			}
		}
		// FinalOutput( ..., TONEMAP_SCALE_LINEAR ) also ends PortalRefract's
		// stage 2 (the flames), its other stages not scaling, and every output of
		// vertexlit_and_unlit_generic_ps2x (UnlitGeneric and VertexLitGeneric; only
		// its LIGHTING_PREVIEW outputs do not scale).
		// Other shaders scale as their FinalOutput's tone-map type says
		// (SnapshotToneMapType): GAMMA by GAMMA_LIGHT_SCALE, c30.w.
		const bool linearToneScale =
		    lightmapped || g_CurrentPortalStage == 2 || g_CurrentModulationInPixelC1 ||
		    g_CurrentSkinCombos >= 0 || g_CurrentPbrModel ||
		    ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentSky ) ||
		    g_CurrentToneMap == kToneMapLinear;
		float outputScale = 1.0f;
		if ( linearToneScale )
			outputScale = g_ToneMappingScale.x;
		else if ( g_CurrentToneMap == kToneMapGamma )
			outputScale = g_psConstants[30][3];
		g_VulkanContext.SetDynamicOutputScale( outputScale );
		// solidenergy_ps20b scales by saturate( LINEAR_LIGHT_SCALE ).
		if ( g_CurrentSolidEnergy )
			g_VulkanContext.SetDynamicOutputScale( clamp( g_ToneMappingScale.x, 0.0f, 1.0f ) );
		if ( !linearToneScale && g_CurrentToneMap == kToneMapUnknown &&
		     CurrentHDRType() == HDR_TYPE_INTEGER && g_ToneMappingScale.x != 1.0f &&
		     ( raster.colorWrite || raster.alphaWrite ) )
			NoteUnimplemented( "integer HDR: tone-mapping scale of an unclassified pixel shader" );
		CommitPassPixelConstants();
		if ( refract )
		{
			const float tintAndScale[4] = { g_psConstants[1][0], g_psConstants[1][1],
			    g_psConstants[1][2], g_psConstants[5][0] };
			g_VulkanContext.SetDynamicModulation( tintAndScale );
			g_VulkanContext.SelectDynamicAlphaTest( g_psConstants[2][0] );
		}
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

	// Record the actual inputs selected for the glass variants as well as the
	// base texture used by the other textured materials.
	const auto recordSampler = [&]( int stage, int handle )
	{
		const int state = g_VulkanContext.ManagedTextureSamplerState( handle );
		drawstatefixture::Sampler &sampler = draw.samplers[draw.samplerCount++];
		sampler.stage = stage;
		sampler.texture = FixtureTextureName( handle );
		sampler.addressU =
		    ( state & render_vulkan::CVulkanContext::kSamplerClampU ) ? "clamp" : "wrap";
		sampler.addressV =
		    ( state & render_vulkan::CVulkanContext::kSamplerClampV ) ? "clamp" : "wrap";
		sampler.filter =
		    ( state & render_vulkan::CVulkanContext::kSamplerLinear ) ? "linear" : "point";
	};
	if ( g_SamplesBaseTexture )
	{
		if ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentRefract )
		{
			recordSampler( 2, g_boundEnvmapHandle );
			recordSampler( 3, g_boundRefractNormalHandle );
			recordSampler( 4, g_boundRefractCubeHandle );
		}
		else
		{
			recordSampler( 0, g_boundTextureHandle );
			if ( g_CurrentColorFlags & render_vulkan::CVulkanContext::kFragmentLightmappedEnvmap )
			{
				recordSampler( 1, g_boundLightmapHandle );
				recordSampler( 2, g_boundEnvmapHandle );
				if ( g_CurrentColorFlags &
				     render_vulkan::CVulkanContext::kFragmentNormalAlphaEnvmapMask )
					recordSampler( 4, g_boundNormalMaskHandle );
			}
		}
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

// The fixed-function matrix helpers. The render context forwards them here only
// while it validates its matrices (mat_debug matrix validation), then compares
// this API's matrix with its own (TestMatrixSync). Its matrices are VMatrix
// (column vectors); this API keeps their transposes, as LoadMatrix receives
// them. So each helper applies the render context's own VMatrix operation to
// the transpose of the current matrix.
static void TransformCurrentMatrix( CShaderAPIVulkan &api, const std::function<void( VMatrix & )> &op )
{
	float stored[16];
	api.GetCurrentMatrix( stored );
	VMatrix matrix;
	for ( int row = 0; row < 4; ++row )
		for ( int col = 0; col < 4; ++col )
			matrix.m[row][col] = stored[col * 4 + row];
	op( matrix );
	for ( int row = 0; row < 4; ++row )
		for ( int col = 0; col < 4; ++col )
			stored[col * 4 + row] = matrix.m[row][col];
	api.LoadMatrix( stored );
}

void CShaderAPIVulkan::GetCurrentMatrix( float *dst )
{
	memcpy( dst, CurrentMatrix(), 16 * sizeof( float ) );
}

// As CShaderAPIDx8::LoadCameraToWorld: the inverse of the view matrix with its
// translation removed.
void CShaderAPIVulkan::LoadCameraToWorld( void )
{
	EnsureMatricesInit();
	float inverse[16];
	if ( !MatInvert( g_matrices.mat[MATERIAL_VIEW], inverse ) )
		MatSetIdentity( inverse );
	inverse[12] = inverse[13] = inverse[14] = 0.0f;
	LoadMatrix( inverse );
}

void CShaderAPIVulkan::Ortho(
    double left, double top, double right, double bottom, double zNear, double zFar )
{
	TransformCurrentMatrix( *this, [&]( VMatrix &m )
	    { MatrixOrtho( m, left, top, right, bottom, zNear, zFar ); } );
}

void CShaderAPIVulkan::PerspectiveX( double fovx, double aspect, double zNear, double zFar )
{
	TransformCurrentMatrix(
	    *this, [&]( VMatrix &m ) { MatrixPerspectiveX( m, fovx, aspect, zNear, zFar ); } );
}

void CShaderAPIVulkan::PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar,
    double bottom, double top, double left, double right )
{
	TransformCurrentMatrix( *this, [&]( VMatrix &m )
	    { MatrixPerspectiveOffCenterX( m, fovx, aspect, zNear, zFar, bottom, top, left, right ); } );
}

// Maps the pick rectangle of the current viewport to the whole projection.
void CShaderAPIVulkan::PickMatrix( int x, int y, int width, int height )
{
	ShaderViewport_t viewport;
	GetViewports( &viewport, 1 );
	if ( viewport.m_nWidth <= 0 || viewport.m_nHeight <= 0 || width <= 0 || height <= 0 )
		return;
	const float px = 2.0f * static_cast<float>( x - viewport.m_nTopLeftX ) / viewport.m_nWidth - 1;
	const float py = 2.0f * static_cast<float>( y - viewport.m_nTopLeftY ) / viewport.m_nHeight - 1;
	const float pw = 2.0f * static_cast<float>( width ) / viewport.m_nWidth;
	const float ph = 2.0f * static_cast<float>( height ) / viewport.m_nHeight;
	VMatrix pick;
	MatrixSetIdentity( pick );
	pick.m[0][0] = 2.0f / pw;
	pick.m[1][1] = 2.0f / ph;
	pick.m[0][3] = -2.0f * px / pw;
	pick.m[1][3] = -2.0f * py / ph;
	TransformCurrentMatrix( *this, [&]( VMatrix &m )
	    {
		    VMatrix result;
		    MatrixMultiply( m, pick, result );
		    m = result;
	    } );
}

void CShaderAPIVulkan::Rotate( float angle, float x, float y, float z )
{
	TransformCurrentMatrix(
	    *this, [&]( VMatrix &m ) { MatrixRotate( m, Vector( x, y, z ), angle ); } );
}

void CShaderAPIVulkan::Translate( float x, float y, float z )
{
	TransformCurrentMatrix( *this, [&]( VMatrix &m ) { MatrixTranslate( m, Vector( x, y, z ) ); } );
}

void CShaderAPIVulkan::Scale( float x, float y, float z )
{
	TransformCurrentMatrix( *this, [&]( VMatrix &m )
	    {
		    VMatrix scale, result;
		    MatrixBuildScale( scale, x, y, z );
		    MatrixMultiply( m, scale, result );
		    m = result;
	    } );
}

void CShaderAPIVulkan::ScaleXY( float x, float y )
{
	Scale( x, y, 1.0f );
}

// Fog methods, as CShaderAPIDx8 implements them. Every pass fogs in its pixel
// shader on this platform (ShouldUsePixelFogForMode is always true on POSIX):
// the pass's fog color is pixel constant c29 (g_LinearFogColor) and its range
// parameters the register its shader names (SetPixelShaderFogParams).
// The fog mode in effect (m_DynamicState.m_SceneFog), which ApplyFogMode sets
// from the pass's shadow fog mode and the scene fog.
void CShaderAPIVulkan::FogMode( MaterialFogMode_t fogMode )
{
	g_Fog.currentMode = fogMode;
}

void CShaderAPIVulkan::FogStart( float fStart )
{
	if ( fStart == g_Fog.start )
		return;
	g_Fog.start = fStart;
	UpdateVertexShaderFogParams();
}

void CShaderAPIVulkan::FogEnd( float fEnd )
{
	if ( fEnd == g_Fog.end )
		return;
	g_Fog.end = fEnd;
	UpdateVertexShaderFogParams();
}

// The water height for height fog.
void CShaderAPIVulkan::SetFogZ( float fogZ )
{
	if ( fogZ == g_Fog.fogZ )
		return;
	g_Fog.fogZ = fogZ;
	UpdateVertexShaderFogParams();
}

void CShaderAPIVulkan::FogMaxDensity( float flMaxDensity )
{
	if ( flMaxDensity == g_Fog.maxDensity )
		return;
	g_Fog.maxDensity = flMaxDensity;
	UpdateVertexShaderFogParams();
}

void CShaderAPIVulkan::GetFogDistances( float *fStart, float *fEnd, float *fFogZ )
{
	if ( fStart )
		*fStart = g_Fog.start;
	if ( fEnd )
		*fEnd = g_Fog.end;
	if ( fFogZ )
		*fFogZ = g_Fog.fogZ;
}

void CShaderAPIVulkan::SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
	if ( g_Fog.sceneColor[0] == r && g_Fog.sceneColor[1] == g && g_Fog.sceneColor[2] == b )
		return;
	g_Fog.sceneColor[0] = r;
	g_Fog.sceneColor[1] = g;
	g_Fog.sceneColor[2] = b;
	ApplyFogMode( g_CurrentShadowFogMode, g_Fog.srgbWrite, g_CurrentDisableFogGammaCorrection );
}

void CShaderAPIVulkan::SceneFogMode( MaterialFogMode_t fogMode )
{
	if ( g_Fog.sceneMode == fogMode )
		return;
	g_Fog.sceneMode = fogMode;
	// The pass's fog depends on the scene fog mode.
	ApplyFogMode( g_CurrentShadowFogMode, g_Fog.srgbWrite, g_CurrentDisableFogGammaCorrection );
}

void CShaderAPIVulkan::GetSceneFogColor( unsigned char *rgb )
{
	rgb[0] = g_Fog.sceneColor[0];
	rgb[1] = g_Fog.sceneColor[1];
	rgb[2] = g_Fog.sceneColor[2];
}

MaterialFogMode_t CShaderAPIVulkan::GetSceneFogMode()
{
	return g_Fog.sceneMode;
}

// The mode the pixel shaders fog in: the scene's (pixel fog for every mode).
MaterialFogMode_t CShaderAPIVulkan::GetPixelFogMode() const
{
	return g_Fog.sceneMode;
}

// PIXELFOGTYPE: 0 is range fog (or none, through rigged parameters), 1 height.
int CShaderAPIVulkan::GetPixelFogCombo()
{
	if ( g_Fog.sceneMode != MATERIAL_FOG_NONE )
		return g_Fog.sceneMode - 1;
	return MATERIAL_FOG_NONE;
}

// cFogParams and cEyePosWaterZ, which the vertex shaders read.
void CShaderAPIVulkan::UpdateVertexShaderFogParams()
{
	const float ooFogRange = g_Fog.end != g_Fog.start ? 1.0f / ( g_Fog.end - g_Fog.start ) : 1.0f;
	const float fogParams[4] = { ooFogRange * g_Fog.end, 1.0f,
	    1.0f - clamp( g_Fog.maxDensity, 0.0f, 1.0f ), ooFogRange };
	float cameraPos[4];
	GetWorldSpaceCameraPosition( cameraPos );
	cameraPos[3] = g_Fog.fogZ;
	SetVertexShaderConstant( kVsRegFogParams, fogParams, 1 );
	SetVertexShaderConstant( kVsRegCameraPos, cameraPos, 1 );
}

void CShaderAPIVulkan::SetPixelShaderFogParams( int reg )
{
	SetPixelShaderFogParams( reg, g_CurrentShadowFogMode );
}

// CShaderAPIDx8::SetPixelShaderFogParams: fog start over range, water height,
// max density and 1 / range; with fog off, parameters that make the range fog
// factor 0 (no fog combo is needed for that case).
void CShaderAPIVulkan::SetPixelShaderFogParams( int reg, ShaderFogMode_t fogMode )
{
	g_Fog.delayedParamsRegister = reg;
	float fogParams[4];
	if ( GetPixelFogMode() != MATERIAL_FOG_NONE && fogMode != SHADER_FOGMODE_DISABLED )
	{
		const float ooFogRange = g_Fog.end != g_Fog.start ? 1.0f / ( g_Fog.end - g_Fog.start ) : 1.0f;
		fogParams[0] = g_Fog.start * ooFogRange;
		fogParams[1] = g_Fog.fogZ;
		fogParams[2] = clamp( g_Fog.maxDensity, 0.0f, 1.0f );
		fogParams[3] = ooFogRange;
		if ( GetPixelFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z )
		{
			// Unused by height fog; 1 keeps the pixel shader math unified.
			fogParams[0] = 0.0f;
			fogParams[2] = 1.0f;
		}
	}
	else
	{
		fogParams[0] = 0.0f;
		fogParams[1] = g_Fog.fogZ;
		fogParams[2] = 1.0f;
		fogParams[3] = 0.0f;
	}
	SetPixelShaderConstant( reg, fogParams, 1 );
}

// CShaderAPIDx8::UpdatePixelFogColorConstant: the fog color in the space the
// pass writes (decoded to linear when it writes sRGB), scaled by the tone-mapping
// scale in integer HDR; w is 1 / the dest-alpha depth range.
void CShaderAPIVulkan::UpdatePixelFogColorConstant()
{
	float fogColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	switch ( GetPixelFogMode() )
	{
	case MATERIAL_FOG_LINEAR:
		for ( int i = 0; i < 3; ++i )
		{
			fogColor[i] = g_Fog.pixelColor[i];
			if ( g_Fog.srgbWrite )
				fogColor[i] = GammaToLinear_HardwareSpecific( fogColor[i] );
		}
		break;
	case MATERIAL_FOG_LINEAR_BELOW_FOG_Z:
		// Water fog has always been in linear space and never sRGB-dependent.
		for ( int i = 0; i < 3; ++i )
			fogColor[i] = GammaToLinear_HardwareSpecific( g_Fog.pixelColor[i] );
		break;
	default:
		break;
	}
	if ( GetPixelFogMode() != MATERIAL_FOG_NONE && !g_Fog.gammaCorrectionDisabled &&
	     CurrentHDRType() == HDR_TYPE_INTEGER )
	{
		for ( int i = 0; i < 3; ++i )
			fogColor[i] *= g_ToneMappingScale.x;
	}
	fogColor[3] = 1.0f / g_Fog.destAlphaDepthRange;
	SetPixelShaderConstant( kPsRegLinearFogColor, fogColor, 1 );
}

// CShaderAPIDx8::ApplyFogMode, run when a pass begins and when the scene fog
// changes: the fog mode in effect, the pass's parameters if its shader asked
// for them, and its fog color (the scene's, or black, grey or white for decals).
void CShaderAPIVulkan::ApplyFogMode(
    ShaderFogMode_t fogMode, bool bSRGBWritesEnabled, bool bDisableFogGammaCorrection )
{
	g_Fog.srgbWrite = bSRGBWritesEnabled;
	if ( fogMode == SHADER_FOGMODE_DISABLED )
	{
		FogMode( MATERIAL_FOG_NONE );
		if ( g_Fog.delayedParamsRegister != -1 )
			SetPixelShaderFogParams( g_Fog.delayedParamsRegister, fogMode );
		return;
	}
	FogMode( g_Fog.sceneMode );
	if ( g_Fog.delayedParamsRegister != -1 )
		SetPixelShaderFogParams( g_Fog.delayedParamsRegister, fogMode );
	bool bShouldGammaCorrect = true;
	unsigned char rgb[3] = { 0, 0, 0 };
	switch ( fogMode )
	{
	case SHADER_FOGMODE_BLACK: // additive decals
		bShouldGammaCorrect = false;
		break;
	case SHADER_FOGMODE_OO_OVERBRIGHT:
	case SHADER_FOGMODE_GREY: // mod2x decals
		rgb[0] = rgb[1] = rgb[2] = 128;
		break;
	case SHADER_FOGMODE_WHITE: // multiplicative decals
		rgb[0] = rgb[1] = rgb[2] = 255;
		bShouldGammaCorrect = false;
		break;
	case SHADER_FOGMODE_FOGCOLOR:
		GetSceneFogColor( rgb );
		break;
	default:
		break;
	}
	g_Fog.gammaCorrectionDisabled = !( bShouldGammaCorrect && !bDisableFogGammaCorrection );
	for ( int i = 0; i < 3; ++i )
		g_Fog.pixelColor[i] = rgb[i] / 255.0f;
	UpdatePixelFogColorConstant();
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
		{
			int nReg = 0;
			ReadCommandField( pCmdBuf, sizeof( int ), &nReg );
			SetPixelShaderFogParams( nReg );
			pCmdBuf += 2 * sizeof( int );
			break;
		}

		case CBCMD_STORE_EYE_POS_IN_PSCONST:
		{
			int nReg = 0;
			ReadCommandField( pCmdBuf, sizeof( int ), &nReg );
			float eye[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			GetWorldSpaceCameraPosition( eye );
			SetPixelShaderConstant( nReg, eye, 1 );
			pCmdBuf += 2 * sizeof( int );
			break;
		}

		case CBCMD_COMMITPIXELSHADERLIGHTING:
		{
			int nReg = 0;
			ReadCommandField( pCmdBuf, sizeof( int ), &nReg );
			CommitPixelShaderLighting( nReg );
			pCmdBuf += 2 * sizeof( int );
			break;
		}

		case CBCMD_SETPIXELSHADERSTATEAMBIENTLIGHTCUBE:
		{
			int nReg = 0;
			ReadCommandField( pCmdBuf, sizeof( int ), &nReg );
			SetPixelShaderStateAmbientLightCube( nReg );
			pCmdBuf += 2 * sizeof( int );
			break;
		}

		case CBCMD_SETAMBIENTCUBEDYNAMICSTATEVERTEXSHADER:
			SetVertexShaderStateAmbientLightCube();
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
		{
			int nIndex = 0;
			ReadCommandField( pCmdBuf, sizeof( int ), &nIndex );
			SetPixelShaderIndex( nIndex );
			pCmdBuf += 2 * sizeof( int );
			break;
		}

		case CBCMD_SET_VSHINDEX:
		{
			int nIndex = 0;
			ReadCommandField( pCmdBuf, sizeof( int ), &nIndex );
			SetVertexShaderIndex( nIndex );
			pCmdBuf += 2 * sizeof( int );
			break;
		}

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
	// Render targets 1..3 are extra color outputs (MRT). The render context
	// disables them on every target change by passing the back buffer, which
	// D3D9 turns into SetRenderTarget( id, NULL ); nothing is drawn to them then.
	// Binding a texture there needs pipelines with more color attachments.
	if ( nRenderTargetID != 0 )
	{
		if ( colorTextureHandle != SHADER_RENDERTARGET_BACKBUFFER &&
		     colorTextureHandle != SHADER_RENDERTARGET_NONE )
			NoteUnimplemented( "SetRenderTargetEx: a texture on render target 1..3 (MRT)" );
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
	g_VertexShaderDynamicIndex = vshIndex;
}

void CShaderAPIVulkan::SetPixelShaderIndex( int pshIndex )
{
	g_PixelShaderDynamicIndex = pshIndex;
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

// The boolean and integer constant banks, held as D3D9 holds them. The native
// pipelines evaluate the shader model 3 static control flow that reads them
// from the same inputs directly (lights, bone counts), so no stage reads them.
void CShaderAPIVulkan::SetBooleanVertexShaderConstant(
    int var, BOOL const *pVec, int numConst, bool bForce )
{
	for ( int i = 0; pVec && i < numConst; ++i )
		if ( var + i >= 0 && var + i < kShaderBoolIntRegisters )
			m_vsBoolConstants[var + i] = pVec[i];
}

void CShaderAPIVulkan::SetIntegerVertexShaderConstant(
    int var, int const *pVec, int numConst, bool bForce )
{
	for ( int i = 0; pVec && i < numConst; ++i )
		if ( var + i >= 0 && var + i < kShaderBoolIntRegisters )
			memcpy( m_vsIntConstants[var + i], pVec + i * 4, sizeof( m_vsIntConstants[0] ) );
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
	for ( int i = 0; pVec && i < numBools; ++i )
		if ( var + i >= 0 && var + i < kShaderBoolIntRegisters )
			m_psBoolConstants[var + i] = pVec[i];
}

void CShaderAPIVulkan::SetIntegerPixelShaderConstant(
    int var, int const *pVec, int numIntVecs, bool bForce )
{
	for ( int i = 0; pVec && i < numIntVecs; ++i )
		if ( var + i >= 0 && var + i < kShaderBoolIntRegisters )
			memcpy( m_psIntConstants[var + i], pVec + i * 4, sizeof( m_psIntConstants[0] ) );
}

// Forgets the register a pass asked its fog parameters in (D3D9's
// m_DelayedShaderConstants.Invalidate), so a later ApplyFogMode does not
// rewrite that register for a pass that no longer uses it.
void CShaderAPIVulkan::InvalidateDelayedShaderConstants( void )
{
	g_Fog.delayedParamsRegister = -1;
}

float CShaderAPIVulkan::GammaToLinear_HardwareSpecific( float fGamma ) const
{
	return SrgbGammaToLinear( fGamma );
}

float CShaderAPIVulkan::LinearToGamma_HardwareSpecific( float fLinear ) const
{
	return SrgbLinearToGamma( fLinear );
}

void CShaderAPIVulkan::SetLinearToGammaConversionTextures(
    ShaderAPITextureHandle_t hSRGBWriteEnabledTexture, ShaderAPITextureHandle_t hIdentityTexture )
{
	m_hLinearToGammaTableTexture = hSRGBWriteEnabledTexture;
	m_hLinearToGammaTableIdentityTexture = hIdentityTexture;
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
	int native = static_cast<int>( textureHandle ) - 1;
	// mat_texture_limit: past the frame's budget D3D9 sets no texture.
	if ( native >= 0 && WouldBeOverTextureLimit( native ) )
		native = -1;
	NoteTextureBound( native );
	// D3D9 sets no texture on a sampler the current pass did not enable
	// (ApplyTextureEnable); measured here before this backend follows it.
	if ( native >= 0 && stage >= 0 && stage < 32 && !( g_CurrentEnabledSamplers & ( 1u << stage ) ) )
		NoteUnimplemented( "BindTexture: sampler the pass did not enable (D3D9 sets no texture)" );
	// Sampler 1 is sampled only when it holds a lightmap page (BindStandardTexture
	// says so); bump, env and detail stages are not sampled yet, so they must not
	// overwrite the base texture either.
	if ( stage == SHADER_SAMPLER1 && g_BindingLightmap )
	{
		g_boundLightmapHandle = native;
		g_VulkanContext.BindManagedLightmap( native );
		return;
	}
	// Samplers 1..15 as ordinary textures, read only by shaders that sample
	// them so (PortalRefract's noise and color ramp, the skin shader's maps).
	if ( stage != SHADER_SAMPLER0 )
		g_VulkanContext.BindManagedSampler( static_cast<int>( stage ), native );
	if ( stage == SHADER_SAMPLER2 )
	{
		g_boundEnvmapHandle = native;
		g_boundPbrEmissionHandle = native;
	}
	if ( stage == SHADER_SAMPLER1 )
		g_boundPbrNormalHandle = native;
	if ( stage == SHADER_SAMPLER10 )
		g_boundPbrMraoHandle = native;
	if ( stage == SHADER_SAMPLER3 )
	{
		g_boundRefractNormalHandle = native;
		g_boundPbrEnvmapHandle = native;
	}
	if ( stage == SHADER_SAMPLER4 )
	{
		g_boundRefractCubeHandle = native;
		g_boundNormalMaskHandle = native;
	}
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
    const void *imageData, int srcStride, std::string *error, uint32_t level = 0, int xOffset = 0,
    int yOffset = 0, uint32_t face = 0 )
{
	// The width x height rectangle at (xOffset, yOffset) of the level; the whole
	// level when it covers it.
	const auto upload = [&]( const uint8_t *data, size_t size )
	{
		return g_VulkanContext.UploadManagedTextureRegion( handle, static_cast<uint32_t>( xOffset ),
		    static_cast<uint32_t>( yOffset ), static_cast<uint32_t>( width ),
		    static_cast<uint32_t>( height ), data, size, error, level, face );
	};
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
		// A sub-rectangle of a larger surface arrives with that surface's row of
		// blocks as its pitch; repack it to the region's own rows.
		const size_t blockRow = blocksX * blockBytes;
		std::vector<uint8_t> packedBlocks;
		if ( srcStride > 0 && static_cast<size_t>( srcStride ) != blockRow )
		{
			packedBlocks.resize( blockRow * blocksY );
			for ( size_t row = 0; row < blocksY; ++row )
				memcpy( &packedBlocks[row * blockRow], src + row * srcStride, blockRow );
			src = packedBlocks.data();
		}
		return upload( src, blockRow * blocksY );
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
		return upload( src, pixels * 8 );
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
		return upload( src, pixels * 4 );

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
	return upload( texels.data(), texels.size() );
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
	         static_cast<uint32_t>( level ), 0, 0, static_cast<uint32_t>( cubeFace ) ) )
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
	if ( level < 0 || g_currentModifyTexture <= 0 || !imageData || width <= 0 || height <= 0 ||
	     xOffset < 0 || yOffset < 0 )
		return;

	// A sub-rectangle keeps the rest of the level: VGUI's font cache writes each
	// glyph into its page this way (CFontTextureCache via ITexture::Download).
	std::string error;
	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	if ( static_cast<uint32_t>( level ) >= g_VulkanContext.ManagedTextureMipLevels( handle ) )
		return;
	if ( !UploadTextureSurface( handle, width, height, srcFormat, imageData, srcStride, &error,
	         static_cast<uint32_t>( level ), xOffset, yOffset,
	         static_cast<uint32_t>( cubeFace ) ) )
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

	// Every cubemap face and mip the VTF has. A texture created smaller than
	// the VTF (mip skipping) starts at the
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
	const int faces = ( pVTF->Flags() & TEXTUREFLAGS_ENVMAP ) ? 6 : 1;
	if ( pVTF->FaceCount() < faces )
	{
		Warning( "[NativeVulkan] TexImageFromVTF: incomplete cubemap\n" );
		return;
	}
	for ( int face = 0; face < faces; ++face )
	{
		for ( uint32_t level = 0;
		    level < levels && firstMip + static_cast<int>( level ) < pVTF->MipCount(); ++level )
		{
			const int mip = firstMip + static_cast<int>( level );
			int mipWidth = 0, mipHeight = 0, mipDepth = 0;
			pVTF->ComputeMipLevelDimensions( mip, &mipWidth, &mipHeight, &mipDepth );
			const unsigned char *bits = pVTF->ImageData( iVTFFrame, face, mip );
			if ( mipWidth <= 0 || mipHeight <= 0 || !bits )
				return;
			std::string error;
			if ( !UploadTextureSurface( handle, mipWidth, mipHeight, pVTF->Format(), bits, 0,
			         &error, level, 0, 0, face ) )
			{
				if ( !error.empty() )
					Warning( "[NativeVulkan] TexImageFromVTF upload failed: %s\n", error.c_str() );
				return;
			}
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
// which is none for a texture of one level. Anisotropy uses the selected
// device's supported sampler level.
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
		mip = CVulkanContext::kSamplerMipLinear;
		break;
	default:
		break;
	}
	if ( g_VulkanContext.ManagedTextureMipLevels( handle ) <= 1 )
		mip = 0;
	const int anisotropic =
	    texFilterMode == SHADER_TEXFILTERMODE_ANISOTROPIC ? CVulkanContext::kSamplerAnisotropic : 0;
	UpdateModifiedTextureSampler( CVulkanContext::kSamplerMipPoint |
	                                  CVulkanContext::kSamplerMipLinear |
	                                  CVulkanContext::kSamplerAnisotropic,
	    mip | anisotropic );
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

// A residency hint for D3D9's managed pool. Native textures are created in
// device-local memory and stay resident until DeleteTexture, so the hint is
// recorded with the texture and changes nothing.
void CShaderAPIVulkan::TexSetPriority( int priority )
{
	const int handle = static_cast<int>( g_currentModifyTexture ) - 1;
	if ( handle >= 0 && static_cast<size_t>( handle ) < g_TextureRecords.size() )
		g_TextureRecords[static_cast<size_t>( handle )].priority = priority;
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
	                             0, static_cast<uint32_t>( std::max( 1, numMipLevels ) ),
	                             VK_FORMAT_UNDEFINED, ( flags & TEXTURE_CREATE_CUBEMAP ) != 0 );
	if ( native < 0 )
	{
		Warning( "[NativeVulkan] CreateTexture failed: %s\n", error.c_str() );
		return 0;
	}
	NoteTextureCreated( native, pDebugName, dstImageFormat, width, height, pTextureGroupName,
	    static_cast<int>( g_VulkanContext.ManagedTextureMipLevels( native ) ),
	    ( flags & TEXTURE_CREATE_CUBEMAP ) != 0 );
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

// Deleting a texture frees its native image once the GPU is done with it
// (CVulkanContext::DestroyManagedTexture). The material system deletes and
// recreates its render targets whenever the back buffer changes size, so this
// must not leak.
void CShaderAPIVulkan::DeleteTexture( ShaderAPITextureHandle_t textureHandle )
{
	const int handle = static_cast<int>( textureHandle ) - 1;
	if ( handle < 0 )
		return;
	g_VulkanContext.DestroyManagedTexture( handle );
	if ( static_cast<size_t>( handle ) < g_TextureRecords.size() )
		g_TextureRecords[static_cast<size_t>( handle )] = TextureRecord();
	if ( g_boundTextureHandle == handle )
		g_boundTextureHandle = -1;
	if ( g_boundLightmapHandle == handle )
		g_boundLightmapHandle = -1;
	if ( g_currentModifyTexture == textureHandle )
		g_currentModifyTexture = 0;
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

// As CShaderAPIDx8::FlushHardware: flush what is buffered, then wait for the
// GPU as the frame sync does. D3D9 also ends and restarts its scene; the native
// frame is recorded and submitted at Present.
void CShaderAPIVulkan::FlushHardware()
{
	FlushBufferedPrimitives();
	ForceHardwareSync();
}

// CShaderAPIDx8::ResetRenderState: return every piece of dynamic render state to
// its default, as after device creation. A full reset also clears the shader
// constants, bones, view and projection; a partial one keeps them.
void CShaderAPIVulkan::ResetRenderState( bool bFullReset )
{
	// Scene fog and the fog state.
	g_Fog = FogRenderState();
	g_CurrentShadowFogMode = SHADER_FOGMODE_DISABLED;
	g_CurrentDisableFogGammaCorrection = false;
	FogStart( 0.0f );
	FogEnd( 0.0f );
	FogMaxDensity( 1.0f );
	// Constant color, ambient, cull and shade mode, morphing and skinning.
	g_ConstantColor = 0xFFFFFFFF;
	g_AmbientLightColor = 0;
	g_DesiredCullMode = MATERIAL_CULLMODE_CCW;
	g_ShadeMode = SHADER_SMOOTH;
	m_bHWMorphingEnabled = false;
	g_NumBoneWeights = 0;
	if ( bFullReset )
		SetAnisotropicLevel( 1 );
	for ( TextureStageState &stage : m_TextureStages )
		stage = TextureStageState();
	// No textures bound, no lights.
	g_boundTextureHandle = -1;
	g_VulkanContext.BindManagedTexture( -1 );
	g_boundLightmapHandle = -1;
	g_VulkanContext.BindManagedLightmap( -1 );
	for ( int sampler = 1; sampler < render_vulkan::CVulkanContext::kMaxSamplers; ++sampler )
		g_VulkanContext.BindManagedSampler( sampler, -1 );
	DisableAllLocalLights();
	// Overrides, stencil, scissor, fast and user clipping.
	m_bOverrideDepthEnable = false;
	m_bForceDepthFuncEquals = false;
	m_bOverrideAlphaWrite = false;
	m_bOverrideColorWrite = false;
	g_Stencil = StencilRenderState();
	SetScissorRect( -1, -1, -1, -1, false );
	EnableFastClip( false );
	g_ClipPlanesEnabled = 0;
	g_UserClipTransformOverride = false;
	// The default pass state and dynamic state.
	SetDefaultState();
	if ( bFullReset )
	{
		memset( g_psConstants, 0, sizeof( g_psConstants ) );
		for ( int bone = 0; bone < kMaxBoneMatrices; ++bone )
		{
			memset( g_BoneMatrices[bone], 0, sizeof( g_BoneMatrices[bone] ) );
			g_BoneMatrices[bone][0] = g_BoneMatrices[bone][5] = g_BoneMatrices[bone][10] = 1.0f;
		}
		MatrixMode( MATERIAL_VIEW );
		LoadIdentity();
		MatrixMode( MATERIAL_PROJECTION );
		LoadIdentity();
		MatrixMode( MATERIAL_MODEL );
	}
	// The viewport follows the whole target again.
	g_Viewport = ShaderViewport_t();
	g_Viewport.m_nWidth = 0;
	g_Viewport.m_nHeight = 0;
	g_VulkanContext.SetViewport( 0, 0, 0, 0, 0.0f, 1.0f );
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

// As CShaderAPIDx8: the flag shaders read back (IsHWMorphingEnabled). The
// material system enables it only on hardware reporting morph batches, which
// this backend does not (MaxHWMorphBatchCount).
void CShaderAPIVulkan::EnableHWMorphing( bool bEnable )
{
	m_bHWMorphingEnabled = bEnable;
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
	++g_CurrentFrameNumber;
	g_TextureMemoryUsedLastFrame = 0;
}

// As CShaderAPIDx8::EndFrame: export the frame's texture list (mat_texture_list).
void CShaderAPIVulkan::EndFrame()
{
	ExportTextureList();
}

// returns the current time in seconds....
double CShaderAPIVulkan::CurrentTime() const
{
	return Sys_FloatTime();
}

// Get the current camera position in world space: as
// CShaderAPIDx8::CacheWorldSpaceCameraPosition, from the view matrix (stored as
// D3D9 holds it, rows for row vectors).
void CShaderAPIVulkan::GetWorldSpaceCameraPosition( float *pPos ) const
{
	EnsureMatricesInit();
	const float *view = g_matrices.mat[MATERIAL_VIEW];
	for ( int i = 0; i < 3; ++i )
		pPos[i] =
		    -( view[12] * view[i * 4] + view[13] * view[i * 4 + 1] + view[14] * view[i * 4 + 2] );
}

// D3D9's frame sync (mat_frame_sync_enable): at the start of a frame, wait until
// the GPU has finished everything submitted before the previous frame, at most
// 200 ms, so the CPU runs no more than one frame ahead. Frames are submitted at
// Present, so that is the second most recent submission.
static ConVar mat_frame_sync_enable( "mat_frame_sync_enable", "1", FCVAR_CHEAT );

void CShaderAPIVulkan::ForceHardwareSync( void )
{
	if ( !mat_frame_sync_enable.GetInt() || !g_VulkanContext.IsValid() )
		return;
	const uint64_t submitted = g_VulkanContext.SubmittedFrameSerial();
	if ( submitted >= 2 )
		g_VulkanContext.WaitForSubmittedFrame( submitted - 1, 200ull * 1000 * 1000 );
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

// The fast-clip plane, Ax+By+Cz=D, kept as D3D9 keeps it (d negated); the
// projection draws use follows it (DrawProjection).
void CShaderAPIVulkan::SetFastClipPlane( const float *pPlane )
{
	if ( !pPlane )
		return;
	g_FastClip.plane[0] = pPlane[0];
	g_FastClip.plane[1] = pPlane[1];
	g_FastClip.plane[2] = pPlane[2];
	g_FastClip.plane[3] = -pPlane[3];
	CommitModelViewProj();
}

void CShaderAPIVulkan::EnableFastClip( bool bEnable )
{
	if ( g_FastClip.enabled == bEnable )
		return;
	g_FastClip.enabled = bEnable;
	CommitModelViewProj();
}

int CShaderAPIVulkan::GetCurrentNumBones( void ) const
{
	return g_NumBoneWeights;
}

bool CShaderAPIVulkan::IsHWMorphingEnabled( void ) const
{
	return m_bHWMorphingEnabled;
}

int CShaderAPIVulkan::GetCurrentLightCombo( void ) const
{
	return 0;
}

// As CShaderAPIDx8::GetDX9LightState: ambient light when any cube face's color
// is nonzero, the enabled local lights, and static vertex lighting when the mesh
// being drawn has a color mesh. The shaders pick their lighting combos from it.
void CShaderAPIVulkan::GetDX9LightState( LightState_t *state ) const
{
	state->m_bAmbientLight = false;
	for ( int face = 0; face < 6; ++face )
		for ( int k = 0; k < 3; ++k )
			state->m_bAmbientLight = state->m_bAmbientLight || g_AmbientCube[face][k] != 0.0f;
	state->m_nNumLights = 0;
	for ( bool enabled : g_LightEnabled )
		state->m_nNumLights += enabled ? 1 : 0;
	state->m_bStaticLightVertex = g_pRenderMesh && g_pRenderMesh->HasColorMesh();
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

// D3D9 frees its dynamic vertex and index buffers across level transitions.
// Here those are the dynamic meshes' CPU storage (the frame stream regrows on
// its own); a later lock allocates what it needs again.
void CShaderAPIVulkan::DestroyVertexBuffers( bool bExitingLevel )
{
	m_Mesh.ReleaseStorage();
	s_ShaderDeviceEmpty.ReleaseDynamicStorage();
}

// D3D9 drops its managed pool from video memory so it is reloaded on demand.
// Native textures have no managed pool: each lives in device-local memory with
// no system copy to reload from, and leaves only through DeleteTexture.
void CShaderAPIVulkan::EvictManagedResources()
{
}

// Fixed-function texture coordinate transforms (D3DTSS_TEXTURETRANSFORMFLAGS)
// and bump-environment matrices, recorded per stage as D3D9 records them.
void CShaderAPIVulkan::SetTextureTransformDimension(
    TextureStage_t textureStage, int dimension, bool projected )
{
	if ( textureStage < 0 || textureStage >= static_cast<int>( ARRAYSIZE( m_TextureStages ) ) )
		return;
	m_TextureStages[textureStage].transformFlags = dimension | ( projected ? 256 : 0 );
}

void CShaderAPIVulkan::SetBumpEnvMatrix(
    TextureStage_t textureStage, float m00, float m01, float m10, float m11 )
{
	if ( textureStage < 0 || textureStage >= static_cast<int>( ARRAYSIZE( m_TextureStages ) ) )
		return;
	float *m = m_TextureStages[textureStage].bumpEnv;
	m[0] = m00;
	m[1] = m01;
	m[2] = m10;
	m[3] = m11;
}

// D3D9 writes the token into its recorded command stream. Here it labels the
// frame being built in the -vkframestats stream, which is that stream.
void CShaderAPIVulkan::SyncToken( const char *pToken )
{
	if ( pToken )
		g_VulkanContext.MarkFrame( pToken );
}
