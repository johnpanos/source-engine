//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Per-material pixel conformance. Renders real Source materials through
//          the real material system and the selected render provider, and
//          reports the pixels each backend produced for fixed inputs.
//
//          Families (-family):
//
//          lightmap (default): LightmappedGeneric's lightmap term. Each case
//          draws one full-viewport quad with a solid base texture and a lightmap
//          allocated, packed and bound exactly as a compiled map's lightmaps are
//          (Begin/EndLightmapAllocation, UpdateLightmap with linear float texels,
//          BindLightmapPage). The lightmap's left and right halves hold different
//          values and one pixel is read from each half, so the result also shows
//          that the lightmap is addressed by its own texture coordinates.
//
//          exposure: the measurement integer-HDR auto-exposure is built on
//          (CLuminanceHistogramSystem, game/client/viewpostprocess.cpp). The frame
//          is filled with regions of known color, copied to _rt_FullFrameFB, and
//          the real dev/lumcompare material (screenspace_general with
//          luminance_compare_ps20) is drawn once per luminance range of the
//          client's histogram, each draw bracketed by an occlusion query. The
//          query counts are the histogram the client turns into an exposure.
//
//          skinning: model placement. A model material ($model UnlitGeneric)
//          draws a small quad from a static mesh with bone weights and indices,
//          as studiorender builds and draws models: rigid models through the
//          MODEL matrix that LoadBoneMatrix( 0 ) loads, skinned ones through the
//          bone matrices (SetNumBoneWeights, LoadBoneMatrix). Each case places
//          the quad in one third of the frame; which third it covers is the
//          measurement. One case winds the quad the other way: model materials
//          cull back faces, so it must not appear at all.
//
//          portal: stencil portal recursion with the real Portal materials
//          (material_pixel_portal.cpp).
//
//          modellight: VertexLitGeneric model lighting, the ambient cube, local
//          lights and static vertex lighting (material_pixel_modellight.cpp).
//
//          pbr-fallback: a PBRMetalRough VMT follows $fallbackmaterial to a
//          distinct UnlitGeneric VMT and draws its green procedural texture.
//
//          This program measures; it does not judge. It writes the inputs and
//          the measured pixels as source-material-pixels/v1 JSON, and
//          tools/quality/material_pixel_conformance.py applies the oracle:
//          backend-independent properties on every backend, and agreement with
//          the D3D9 reference capture within a stated tolerance.
//
//          Run inside a staged game runtime (the driver stages one):
//            material_pixel_conformance -game portal -renderer <id>
//                -hdr <none|integer> [-family <lightmap|exposure|skinning|portal|
//                                         modellight|cable|pbr-fallback>]
//                -out <file.json>
//
//=============================================================================//

#include "appframework/AppFramework.h"
#include "appframework/ilaunchermgr.h"
#include "appframework/linked_systems.h"
#include "appframework/window_provider.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/materialsystem_config.h"
#include "material_pixel_modellight.h"
#include "material_pixel_portal.h"
#include "pixelwriter.h"
#include "render/builtin_shader_provider.h"
#include "render/legacy_shader_provider.h"
#include "tier0/icommandline.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "vtf/vtf.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace
{

const int kWindowSize = 256;
// The linear tone-mapping scale the modellight family draws with in integer HDR.
const float kModelLightToneScale = 0.75f;
// A case's lightmap is kLightmapSize square: the left half holds one value and
// the right half another. With the one-texel border every map lightmap has, the
// quarter points of the quad land on texel centers 2 and 5, inside each half.
const int kLightmapSize = 8;

struct LightmapCase
{
	const char *name;
	unsigned char base[3]; // $basetexture texel (sRGB-encoded, as authored)
	float left[3];         // lightmap, linear light as vrad writes it
	float right[3];
};

// Chosen to expose the lightmap term's structure: zero light, a ramp through and
// beyond 1.0 (lightmaps are overbright), channel independence, and the base
// texture as a factor.
const LightmapCase kLightmapCases[] = {
    { "black_lightmap", { 255, 255, 255 }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { "ramp_low", { 255, 255, 255 }, { 0.0625f, 0.0625f, 0.0625f }, { 0.125f, 0.125f, 0.125f } },
    { "ramp_mid", { 255, 255, 255 }, { 0.25f, 0.25f, 0.25f }, { 0.5f, 0.5f, 0.5f } },
    { "ramp_high", { 255, 255, 255 }, { 1.0f, 1.0f, 1.0f }, { 2.0f, 2.0f, 2.0f } },
    { "channels", { 255, 255, 255 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { "base_gray", { 128, 128, 128 }, { 0.25f, 0.25f, 0.25f }, { 1.0f, 1.0f, 1.0f } },
    { "base_color", { 255, 128, 64 }, { 0.5f, 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } },
};
const int kLightmapCaseCount = sizeof( kLightmapCases ) / sizeof( kLightmapCases[0] );

// Exposure frame: four columns by two rows, top row first. Each color's linear
// luminance (the NTSC weights luminance_compare_ps2x.fxc applies after the sRGB
// read) lies well inside one histogram range, clear of the range edges.
struct ExposureRegion
{
	const char *name;
	unsigned char color[3]; // written to the back buffer as-is (sRGB-encoded)
};
const ExposureRegion kExposureRegions[] = {
    { "black", { 0, 0, 0 } },
    { "gray64", { 64, 64, 64 } },
    { "gray128", { 128, 128, 128 } },
    { "gray190", { 190, 190, 190 } },
    { "white", { 255, 255, 255 } },
    { "red", { 255, 0, 0 } },
    { "green", { 0, 255, 0 } },
    { "blue", { 0, 0, 255 } },
};
const int kExposureRegionCount = sizeof( kExposureRegions ) / sizeof( kExposureRegions[0] );
const int kExposureColumns = 4;
const int kExposureRows = 2;
// The client's default histogram (mat_tonemap_algorithm 1): N_LUMINANCE_RANGES_NEW
// ranges, the last of which counts every pixel (it calibrates the query counts).
const int kLuminanceRanges = 17;
// The client's defaults for the measured region, mat_exposure_center_region_x/y.
const float kExposureCenterRegionX = 0.9f;
const float kExposureCenterRegionY = 0.85f;
// Frames to wait for query results before the run is declared incomplete.
const int kMaxQueryPollFrames = 60;
// Skinning cases. Bones translate the quad along clip-space x; the frame is read
// at the centers of its thirds. Bone 0 always holds kSkinDecoy, the placement a
// backend that ignored skinning would draw every skinned case at.
struct SkinningCase
{
	const char *name;
	int numBoneWeights;     // SetNumBoneWeights; 0 draws rigidly through MODEL
	float bone0;            // bone 0 translation (also the MODEL matrix)
	float bones[3];         // bones 1..3 translations
	float weights[2];       // stored weights; the third is 1 - w0 - w1
	unsigned char index[3]; // bone indices of the three weights
	int expectedThird;      // 0 left, 1 middle, 2 right, -1 culled (the oracle decides)
	bool backFacing;        // wound counter-clockwise on screen: culled by default
};
const float kSkinLeft = -2.0f / 3.0f, kSkinMiddle = 0.0f, kSkinRight = 2.0f / 3.0f;
const float kSkinDecoy = kSkinRight;
const SkinningCase kSkinningCases[] = {
    // Rigid: studiorender loads bone 0 and draws without bone weights.
    { "rigid_bone0", 0, kSkinLeft, { 0, 0, 0 }, { 1, 0 }, { 0, 0, 0 }, 0 },
    { "one_bone", 2, kSkinDecoy, { kSkinLeft, 0, 0 }, { 1, 0 }, { 1, 0, 0 }, 0 },
    { "implicit_third_weight", 2, kSkinDecoy, { 0, kSkinMiddle, 0 }, { 0, 0 }, { 1, 0, 2 }, 1 },
    { "blend_half", 2, kSkinDecoy, { kSkinLeft, 0, 0 }, { 0.5f, 0.5f }, { 1, 0, 0 }, 1 },
    { "high_index", 2, kSkinDecoy, { kSkinMiddle, 0, kSkinRight }, { 1, 0 }, { 3, 0, 0 }, 2 },
    { "rigid_back_facing", 0, kSkinLeft, { 0, 0, 0 }, { 1, 0 }, { 0, 0, 0 }, -1, true },
};
const int kSkinningCaseCount = sizeof( kSkinningCases ) / sizeof( kSkinningCases[0] );
const float kSkinHalfWidth = 0.25f;

// What IMatRenderContext::OcclusionQuery_GetNumPixelsRendered returns before a
// query has a result (the shader API's OCCLUSION_QUERY_RESULT_PENDING).
const int kQueryPending = -1;

// Fills a procedural texture with one color, or with m_Bottom below its middle
// row when m_Split is set; changed per case before Download().
class CSolidColorRegenerator : public ITextureRegenerator
{
public:
	unsigned char m_Color[4] = { 255, 255, 255, 255 };
	unsigned char m_Bottom[4] = { 255, 255, 255, 255 };
	bool m_Split = false;

	void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTF, Rect_t *pRect ) override
	{
		// The backend picks the storage format (D3D9 hands back BGRA8888), so write
		// through the pixel writer rather than assuming a byte order.
		for ( int mip = 0; mip < pVTF->MipCount(); ++mip )
		{
			int width = 0, height = 0, depth = 0;
			pVTF->ComputeMipLevelDimensions( mip, &width, &height, &depth );
			CPixelWriter writer;
			writer.SetPixelMemory(
			    pVTF->Format(), pVTF->ImageData( 0, 0, mip ), pVTF->RowSizeInBytes( mip ) );
			for ( int y = 0; y < height * depth; ++y )
			{
				// Row 0 is the top of the texture (texture coordinate v = 0).
				const unsigned char *c = ( m_Split && y >= height / 2 ) ? m_Bottom : m_Color;
				writer.Seek( 0, y );
				for ( int x = 0; x < width; ++x )
					writer.WritePixel( c[0], c[1], c[2], c[3] );
			}
		}
	}

	void Release() override {}
};

class CMaterialPixelApp : public CSteamAppSystemGroup
{
public:
	bool Create() override;
	bool PreInit() override;
	int Main() override;
	void PostShutdown() override;
	void Destroy() override {}
	// Shared with the portal family (material_pixel_portal.cpp).
	void WriteClearProbe( FILE *out );

private:
	bool RunLightmapCases( FILE *out );
	bool RunExposureCases( FILE *out );
	bool RunSkinningCases( FILE *out );
	bool RunCableCases( FILE *out );
	bool RunSkyCases( FILE *out );
	bool RunPbrFallbackCases( FILE *out );
	bool RenderCase( IMaterial *pMaterial, int sortId, const int offset[2], int lightmapPageId,
	    const float ( *points )[2], int pointCount, unsigned char ( *pixels )[3],
	    float toneScale = 1.0f );
	void DrawLightmappedQuad(
	    IMaterial *pMaterial, int sortId, const int offset[2], const int pageSize[2] );
	bool ReadPixel( float fx, float fy, unsigned char rgb[3] );
};

bool CMaterialPixelApp::Create()
{
	const WindowProviderDescriptor *window = WindowProvider_Describe();
	if ( !AddSystem( window->create(), SDLMGR_INTERFACE_VERSION ) ||
	     !AddSystem( MaterialSystem_Create(), MATERIAL_SYSTEM_INTERFACE_VERSION ) )
		return false;
	IMaterialSystem *pMaterialSystem =
	    (IMaterialSystem *)FindSystem( MATERIAL_SYSTEM_INTERFACE_VERSION );

	// The same render catalog the product's composition root offers.
	const render::LegacyShaderProvider *catalog[] = {
#if defined( LINKED_DX9_BACKEND )
	    Dx9ShaderBackend_Describe(),
#endif
#if defined( LINKED_NATIVE_VULKAN_BACKEND )
	    NativeVulkanShaderBackend_Describe(),
#endif
	};
	const char *requested = CommandLine()->ParmValue( "-renderer", "" );
	const render::LegacyShaderProvider *selected = NULL;
	for ( const render::LegacyShaderProvider *provider : catalog )
	{
		if ( !Q_stricmp( requested, provider->id ) )
			selected = provider;
	}
	if ( !selected || !MaterialSystem_BindShaderProvider( pMaterialSystem, selected ) )
	{
		Warning( "material pixel conformance: render provider '%s' is not linked\n", requested );
		return false;
	}
	return MaterialSystem_BindBuiltinShaderProvider(
	    pMaterialSystem, StandardShaderLibrary_Describe() );
}

bool CMaterialPixelApp::PreInit()
{
	CreateInterfaceFn factory = GetFactory();
	ConnectTier1Libraries( &factory, 1 );
	ConVar_Register();
	ConnectTier2Libraries( &factory, 1 );
	if ( !g_pFullFileSystem || !g_pMaterialSystem )
		return false;
	// Before Init, as the engine connects the material system (gl_shader.cpp):
	// the full-screen texture is what D3D9 back-buffer readback resolves through.
	g_pMaterialSystem->SetAdapter( 0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE );
	return SetupSearchPaths( NULL, false, false );
}

void CMaterialPixelApp::PostShutdown()
{
	DisconnectTier2Libraries();
	ConVar_Unregister();
	DisconnectTier1Libraries();
}

// RunPortalCases (material_pixel_portal.cpp) writes the shared probe through this.
static CMaterialPixelApp *s_pApp = nullptr;
static void WriteClearProbeThunk( FILE *out )
{
	s_pApp->WriteClearProbe( out );
}

int CMaterialPixelApp::Main()
{
	s_pApp = this;
	const char *outPath = CommandLine()->ParmValue( "-out", "" );
	const char *hdr = CommandLine()->ParmValue( "-hdr", "none" );
	const bool integerHdr = !Q_stricmp( hdr, "integer" );
	const char *family = CommandLine()->ParmValue( "-family", "lightmap" );
	const bool exposure = !Q_stricmp( family, "exposure" );
	const bool skinning = !Q_stricmp( family, "skinning" );
	const bool portal = !Q_stricmp( family, "portal" );
	const bool modelLight = !Q_stricmp( family, "modellight" );
	const bool cable = !Q_stricmp( family, "cable" );
	const bool sky = !Q_stricmp( family, "sky" );
	const bool pbrFallback = !Q_stricmp( family, "pbr-fallback" );
	if ( !outPath[0] || ( !integerHdr && Q_stricmp( hdr, "none" ) ) ||
	     ( !exposure && !skinning && !portal && !modelLight && !cable && !sky && !pbrFallback &&
	         Q_stricmp( family, "lightmap" ) ) )
	{
		Warning(
		    "material pixel conformance: need -out <file>, -hdr <none|integer> and "
		    "-family <lightmap|exposure|skinning|portal|modellight|cable|sky|pbr-fallback>\n" );
		return 2;
	}

	g_pMaterialSystem->ModInit();
	ILauncherMgr *pLauncher = (ILauncherMgr *)FindSystem( SDLMGR_INTERFACE_VERSION );
	if ( !pLauncher || !pLauncher->CreateGameWindow(
	                       "material pixel conformance", true, kWindowSize, kWindowSize ) )
	{
		Warning( "material pixel conformance: no window\n" );
		return 3;
	}
	// The back buffer is the window's drawable size in pixels, as the engine's
	// resize path sets it. On a scaled display that differs from the requested
	// window size, and a mismatched swapchain reports the device lost.
	int pixelWidth = kWindowSize, pixelHeight = kWindowSize;
	SDL_PumpEvents();
	if ( !SDL_GetWindowSizeInPixels(
	         static_cast<SDL_Window *>( pLauncher->GetWindowRef() ), &pixelWidth, &pixelHeight ) ||
	     pixelWidth <= 0 || pixelHeight <= 0 )
	{
		Warning( "material pixel conformance: no drawable size: %s\n", SDL_GetError() );
		return 3;
	}
	MaterialSystem_Config_t config = g_pMaterialSystem->GetCurrentConfigForVideoCard();
	config.m_VideoMode.m_Width = pixelWidth;
	config.m_VideoMode.m_Height = pixelHeight;
	config.m_VideoMode.m_RefreshRate = 60;
	config.SetFlag( MATSYS_VIDCFG_FLAGS_WINDOWED, true );
	if ( !g_pMaterialSystem->SetMode( pLauncher->GetWindowRef(), config ) )
	{
		Warning( "material pixel conformance: SetMode failed\n" );
		return 3;
	}
	// Portal frames are compared pixel for pixel, so both backends rasterize one
	// sample per pixel and filter textures the same way. SetMode applies the
	// dxsupport level's defaults (4x MSAA and mat_trilinear 1 on D3D9 here; the
	// native backend reads no dxsupport.cfg), so the sample count and the texture
	// filter are pinned after it.
	if ( portal || modelLight )
	{
		MaterialSystem_Config_t pinned = g_pMaterialSystem->GetCurrentConfigForVideoCard();
		pinned.m_nAASamples = 0;
		pinned.m_nForceAnisotropicLevel = 1;
		pinned.SetFlag( MATSYS_VIDCFG_FLAGS_FORCE_TRILINEAR, true );
		g_pMaterialSystem->OverrideConfig( pinned, false );
	}

	// The HDR mode decides the lightmap page format and the lightmap scale, so it
	// is chosen before lightmaps are allocated, the way map load orders it.
	if ( ConVar *pHdrLevel = g_pCVar->FindVar( "mat_hdr_level" ) )
		pHdrLevel->SetValue( integerHdr ? 2 : 0 );
	// Readback must return this frame's pixels before the case is recorded, so the
	// render context runs on this thread rather than queued to a render thread.
	ConVar *pQueueMode = g_pCVar->FindVar( "mat_queue_mode" );
	if ( !pQueueMode )
	{
		Warning( "material pixel conformance: mat_queue_mode is not registered\n" );
		return 3;
	}
	pQueueMode->SetValue( 0 );
	g_pMaterialSystemHardwareConfig->SetHDREnabled( integerHdr );

	FILE *out = fopen( outPath, "w" );
	if ( !out )
	{
		Warning( "material pixel conformance: cannot write %s\n", outPath );
		return 2;
	}
	const bool ok = exposure      ? RunExposureCases( out )
	                : skinning    ? RunSkinningCases( out )
	                : cable       ? RunCableCases( out )
	                : sky         ? RunSkyCases( out )
	                : pbrFallback ? RunPbrFallbackCases( out )
	                : portal      ? RunPortalCases( out, outPath, WriteClearProbeThunk )
	                : modelLight
	                    // Integer HDR scales FinalOutput's linear light (the game's
	                    // tone-mapping scale); without HDR it is 1.
	                    ? RunModelLightCases( out, outPath, WriteClearProbeThunk,
	                          integerHdr ? kModelLightToneScale : 1.0f )
	                    : RunLightmapCases( out );
	fclose( out );
	pLauncher->DestroyGameWindow();
	return ok ? 0 : 1;
}

struct CableCase
{
	const char *name;
	unsigned char normal[3];
	unsigned char base[3];
};

static const CableCase kCableCases[] = {
    { "front_facing_normal", { 128, 128, 255 }, { 255, 128, 64 } },
    { "side_facing_normal", { 255, 128, 128 }, { 255, 128, 64 } },
    { "back_facing_normal", { 128, 128, 0 }, { 255, 128, 64 } },
    { "diagonal_normal", { 128, 204, 230 }, { 64, 192, 255 } },
};

bool CMaterialPixelApp::RunPbrFallbackCases( FILE *out )
{
	static CSolidColorRegenerator s_Regenerators[3];
	const char *textureNames[3] = { "conformance/pbr_primary_red", "conformance/pbr_primary_mrao",
	    "conformance/pbr_fallback_green" };
	const unsigned char colors[3][3] = { { 255, 0, 0 }, { 0, 255, 255 }, { 0, 255, 0 } };
	for ( int i = 0; i < 3; ++i )
	{
		ITexture *pTexture = g_pMaterialSystem->CreateProceduralTexture( textureNames[i],
		    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
		    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
		        TEXTUREFLAGS_SINGLECOPY );
		if ( !pTexture )
			return false;
		for ( int channel = 0; channel < 3; ++channel )
			s_Regenerators[i].m_Color[channel] = colors[i][channel];
		pTexture->SetTextureRegenerator( &s_Regenerators[i] );
		pTexture->Download();
	}

	IMaterial *pMaterial =
	    g_pMaterialSystem->FindMaterial( "conformance/pbr_case", TEXTURE_GROUP_OTHER );
	if ( !pMaterial )
		return false;
	pMaterial->IncrementReferenceCount();
	g_pMaterialSystem->CacheUsedMaterials();
	const char *shader = pMaterial->GetShaderName();
	if ( !shader )
		shader = "";

	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"pbr-fallback\"," );
	WriteClearProbe( out );
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	unsigned char center[3] = {};
	unsigned char outside[3] = {};
	g_pMaterialSystem->BeginFrame( 0 );
	{
		int width = 0, height = 0;
		g_pMaterialSystem->GetBackBufferDimensions( width, height );
		pRenderContext->Viewport( 0, 0, width, height );
		pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
		pRenderContext->ClearBuffers( true, true );
		pRenderContext->DrawScreenSpaceRectangle(
		    pMaterial, width / 4, height / 4, width / 2, height / 2, 0, 0, 3, 3, 4, 4 );
		ReadPixel( 0.5f, 0.5f, center );
		ReadPixel( 0.1f, 0.1f, outside );
	}
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();
	IMaterial *pMissingReference = g_pMaterialSystem->FindMaterial(
	    "conformance/pbr_missing_reference", TEXTURE_GROUP_OTHER, false );
	IMaterial *pMissingMrao = g_pMaterialSystem->FindMaterial(
	    "conformance/pbr_missing_mrao", TEXTURE_GROUP_OTHER, false );
	IMaterial *pMissingBase = g_pMaterialSystem->FindMaterial(
	    "conformance/pbr_missing_base", TEXTURE_GROUP_OTHER, false );
	IMaterial *pMissingTarget = g_pMaterialSystem->FindMaterial(
	    "conformance/pbr_missing_target", TEXTURE_GROUP_OTHER, false );
	IMaterial *pSelf =
	    g_pMaterialSystem->FindMaterial( "conformance/pbr_self", TEXTURE_GROUP_OTHER, false );
	IMaterial *pTraversal =
	    g_pMaterialSystem->FindMaterial( "conformance/pbr_traversal", TEXTURE_GROUP_OTHER, false );
	IMaterial *pNestedTraversal = g_pMaterialSystem->FindMaterial(
	    "conformance/pbr_nested_traversal", TEXTURE_GROUP_OTHER, false );
	IMaterial *pUnsupported = g_pMaterialSystem->FindMaterial(
	    "conformance/pbr_unsupported", TEXTURE_GROUP_OTHER, false );
	IMaterial *pCycle =
	    g_pMaterialSystem->FindMaterial( "conformance/pbr_cycle", TEXTURE_GROUP_OTHER, false );
	IMaterial *pPbrTarget = g_pMaterialSystem->FindMaterial(
	    "conformance/pbr_pbr_target_case", TEXTURE_GROUP_OTHER, false );
	fprintf( out,
	    "\"shader\":\"%s\",\"error_material\":%s,\"pixels\":{\"center\":[%u,%u,%u],"
	    "\"outside\":[%u,%u,%u]},\"invalid\":{\"missing_reference_rejected\":%s,"
	    "\"missing_mrao_rejected\":%s,\"missing_base_rejected\":%s,"
	    "\"missing_target_rejected\":%s,"
	    "\"self_rejected\":%s,\"traversal_rejected\":%s,\"nested_traversal_rejected\":%s,"
	    "\"unsupported_rejected\":%s,\"cycle_rejected\":%s,"
	    "\"pbr_target_rejected\":%s}}\n",
	    shader, pMaterial->IsErrorMaterial() ? "true" : "false", center[0], center[1], center[2],
	    outside[0], outside[1], outside[2],
	    pMissingReference && pMissingReference->IsErrorMaterial() ? "true" : "false",
	    pMissingMrao && pMissingMrao->IsErrorMaterial() ? "true" : "false",
	    pMissingBase && pMissingBase->IsErrorMaterial() ? "true" : "false",
	    pMissingTarget && pMissingTarget->IsErrorMaterial() ? "true" : "false",
	    pSelf && pSelf->IsErrorMaterial() ? "true" : "false",
	    pTraversal && pTraversal->IsErrorMaterial() ? "true" : "false",
	    pNestedTraversal && pNestedTraversal->IsErrorMaterial() ? "true" : "false",
	    pUnsupported && pUnsupported->IsErrorMaterial() ? "true" : "false",
	    pCycle && pCycle->IsErrorMaterial() ? "true" : "false",
	    pPbrTarget && pPbrTarget->IsErrorMaterial() ? "true" : "false" );
	pMaterial->DecrementReferenceCount();
	return true;
}

bool CMaterialPixelApp::RunCableCases( FILE *out )
{
	static CSolidColorRegenerator s_BaseRegenerator;
	static CSolidColorRegenerator s_NormalRegenerator;
	ITexture *pBase = g_pMaterialSystem->CreateProceduralTexture( "conformance/cable_base",
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	ITexture *pNormal = g_pMaterialSystem->CreateProceduralTexture( "conformance/cable_normal",
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	if ( !pBase || !pNormal )
		return false;
	pBase->SetTextureRegenerator( &s_BaseRegenerator );
	pNormal->SetTextureRegenerator( &s_NormalRegenerator );

	KeyValues *pKeys = new KeyValues( "Cable" );
	pKeys->SetString( "$basetexture", "conformance/cable_base" );
	pKeys->SetString( "$bumpmap", "conformance/cable_normal" );
	IMaterial *pMaterial = g_pMaterialSystem->CreateMaterial( "conformance/cable", pKeys );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
		return false;
	pMaterial->IncrementReferenceCount();
	g_pMaterialSystem->CacheUsedMaterials();
	if ( pMaterial->GetVertexFormat() == 0 )
	{
		Warning( "material pixel conformance: %s has no vertex format after precache\n",
		    pMaterial->GetName() );
		return false;
	}

	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"cable\"," );
	WriteClearProbe( out );
	fprintf( out, "\"cases\":[" );
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	bool ok = true;
	for ( int i = 0; i < ARRAYSIZE( kCableCases ); ++i )
	{
		const CableCase &c = kCableCases[i];
		for ( int k = 0; k < 3; ++k )
		{
			s_BaseRegenerator.m_Color[k] = c.base[k];
			s_NormalRegenerator.m_Color[k] = c.normal[k];
		}
		pBase->Download();
		pNormal->Download();

		IMesh *pMesh = pRenderContext->CreateStaticMesh(
		    pMaterial->GetVertexFormat(), TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS, pMaterial );
		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, 4, 6 );
		const float corners[4][2] = {
		    { -0.75f, 0.75f }, { 0.75f, 0.75f }, { 0.75f, -0.75f }, { -0.75f, -0.75f } };
		for ( int v = 0; v < 4; ++v )
		{
			meshBuilder.Position3f( corners[v][0], corners[v][1], 0.5f );
			meshBuilder.Color4ub( 255, 255, 255, 255 );
			meshBuilder.TexCoord2f( 0, 0.5f * ( corners[v][0] / 0.75f + 1.0f ),
			    0.5f * ( 1.0f - corners[v][1] / 0.75f ) );
			meshBuilder.TexCoord2f( 1, 0.5f * ( corners[v][0] / 0.75f + 1.0f ),
			    0.5f * ( 1.0f - corners[v][1] / 0.75f ) );
			meshBuilder.TangentS3f( 1.0f, 0.0f, 0.0f );
			meshBuilder.TangentT3f( 0.0f, 1.0f, 0.0f );
			meshBuilder.AdvanceVertex();
		}
		for ( unsigned short index : { 0, 1, 2, 0, 2, 3 } )
			meshBuilder.FastIndex( index );
		meshBuilder.End();

		unsigned char pixel[3] = {};
		g_pMaterialSystem->BeginFrame( 0 );
		{
			int width = 0, height = 0;
			g_pMaterialSystem->GetBackBufferDimensions( width, height );
			pRenderContext->Viewport( 0, 0, width, height );
			pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
			pRenderContext->ClearBuffers( true, true );
			for ( int mode = MATERIAL_VIEW; mode <= MATERIAL_PROJECTION; ++mode )
			{
				pRenderContext->MatrixMode( static_cast<MaterialMatrixMode_t>( mode ) );
				pRenderContext->LoadIdentity();
			}
			pRenderContext->Bind( pMaterial );
			pMesh->Draw();
			ok = ReadPixel( 0.5f, 0.5f, pixel ) && ok;
		}
		g_pMaterialSystem->EndFrame();
		g_pMaterialSystem->SwapBuffers();
		pRenderContext->DestroyStaticMesh( pMesh );
		fprintf( out,
		    "%s{\"name\":\"%s\",\"normal\":[%u,%u,%u],\"base\":[%u,%u,%u],"
		    "\"pixels\":[%u,%u,%u]}",
		    i ? "," : "", c.name, c.normal[0], c.normal[1], c.normal[2], c.base[0], c.base[1],
		    c.base[2], pixel[0], pixel[1], pixel[2] );
	}
	fprintf( out, "]}\n" );
	pMaterial->DecrementReferenceCount();
	return ok;
}

struct SkyCase
{
	const char *name;
	unsigned char baseTop[3];
	unsigned char baseBottom[3];
	unsigned char tint[3];
};

static const SkyCase kSkyCases[] = {
    { "untinted_sky", { 192, 128, 64 }, { 32, 160, 224 }, { 255, 255, 255 } },
    { "colored_sky", { 64, 192, 255 }, { 255, 48, 80 }, { 128, 192, 64 } },
};

bool CMaterialPixelApp::RunSkyCases( FILE *out )
{
	static CSolidColorRegenerator s_BaseRegenerator;
	ITexture *pBase = g_pMaterialSystem->CreateProceduralTexture( "conformance/sky_base",
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	if ( !pBase )
		return false;
	s_BaseRegenerator.m_Split = true;
	pBase->SetTextureRegenerator( &s_BaseRegenerator );

	IMaterial *materials[ARRAYSIZE( kSkyCases )] = {};
	for ( int i = 0; i < ARRAYSIZE( kSkyCases ); ++i )
	{
		char tint[96];
		V_snprintf( tint, sizeof( tint ), "[%.8f %.8f %.8f]", kSkyCases[i].tint[0] / 255.0f,
		    kSkyCases[i].tint[1] / 255.0f, kSkyCases[i].tint[2] / 255.0f );
		KeyValues *pKeys = new KeyValues( "Sky" );
		pKeys->SetString( "$basetexture", "conformance/sky_base" );
		pKeys->SetString( "$color", tint );
		pKeys->SetString(
		    "$basetexturetransform", "center .5 .5 scale 1 1 rotate 0 translate 0 -.25" );
		char name[64];
		V_snprintf( name, sizeof( name ), "conformance/sky_%s", kSkyCases[i].name );
		materials[i] = g_pMaterialSystem->CreateMaterial( name, pKeys );
		if ( !materials[i] || materials[i]->IsErrorMaterial() )
			return false;
		materials[i]->IncrementReferenceCount();
	}
	g_pMaterialSystem->CacheUsedMaterials();
	for ( IMaterial *pMaterial : materials )
	{
		if ( pMaterial->GetVertexFormat() == 0 )
		{
			Warning( "material pixel conformance: %s has no vertex format after precache\n",
			    pMaterial->GetName() );
			return false;
		}
	}

	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"sky\"," );
	WriteClearProbe( out );
	fprintf( out, "\"cases\":[" );
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	bool ok = true;
	for ( int i = 0; i < ARRAYSIZE( kSkyCases ); ++i )
	{
		const SkyCase &c = kSkyCases[i];
		for ( int k = 0; k < 3; ++k )
		{
			s_BaseRegenerator.m_Color[k] = c.baseTop[k];
			s_BaseRegenerator.m_Bottom[k] = c.baseBottom[k];
		}
		pBase->Download();
		IMaterial *pMaterial = materials[i];
		IMesh *pMesh = pRenderContext->CreateStaticMesh(
		    pMaterial->GetVertexFormat(), TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS, pMaterial );
		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, 4, 6 );
		const float corners[4][2] = {
		    { -0.75f, 0.75f }, { 0.75f, 0.75f }, { 0.75f, -0.75f }, { -0.75f, -0.75f } };
		for ( int v = 0; v < 4; ++v )
		{
			meshBuilder.Position3f( corners[v][0], corners[v][1], 0.5f );
			meshBuilder.TexCoord2f( 0, 0.5f * ( corners[v][0] / 0.75f + 1.0f ),
			    0.5f * ( 1.0f - corners[v][1] / 0.75f ) );
			meshBuilder.AdvanceVertex();
		}
		for ( unsigned short index : { 0, 1, 2, 0, 2, 3 } )
			meshBuilder.FastIndex( index );
		meshBuilder.End();

		unsigned char pixels[2][3] = {};
		g_pMaterialSystem->BeginFrame( 0 );
		{
			int width = 0, height = 0;
			g_pMaterialSystem->GetBackBufferDimensions( width, height );
			pRenderContext->Viewport( 0, 0, width, height );
			pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
			pRenderContext->ClearBuffers( true, true );
			for ( int mode = MATERIAL_VIEW; mode <= MATERIAL_PROJECTION; ++mode )
			{
				pRenderContext->MatrixMode( static_cast<MaterialMatrixMode_t>( mode ) );
				pRenderContext->LoadIdentity();
			}
			pRenderContext->Bind( pMaterial );
			pMesh->Draw();
			ok = ReadPixel( 0.25f, 0.5f, pixels[0] ) && ok;
			ok = ReadPixel( 0.75f, 0.5f, pixels[1] ) && ok;
		}
		g_pMaterialSystem->EndFrame();
		g_pMaterialSystem->SwapBuffers();
		pRenderContext->DestroyStaticMesh( pMesh );
		fprintf( out,
		    "%s{\"name\":\"%s\",\"base_top\":[%u,%u,%u],"
		    "\"base_bottom\":[%u,%u,%u],\"tint\":[%u,%u,%u],"
		    "\"pixels\":[[%u,%u,%u],[%u,%u,%u]]}",
		    i ? "," : "", c.name, c.baseTop[0], c.baseTop[1], c.baseTop[2], c.baseBottom[0],
		    c.baseBottom[1], c.baseBottom[2], c.tint[0], c.tint[1], c.tint[2], pixels[0][0],
		    pixels[0][1], pixels[0][2], pixels[1][0], pixels[1][1], pixels[1][2] );
	}
	fprintf( out, "]}\n" );
	for ( IMaterial *pMaterial : materials )
		pMaterial->DecrementReferenceCount();
	return ok;
}

void CMaterialPixelApp::DrawLightmappedQuad(
    IMaterial *pMaterial, int sortId, const int offset[2], const int pageSize[2] )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	for ( int mode = MATERIAL_VIEW; mode <= MATERIAL_PROJECTION; ++mode )
	{
		pRenderContext->MatrixMode( static_cast<MaterialMatrixMode_t>( mode ) );
		pRenderContext->LoadIdentity();
	}
	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->LoadIdentity();

	MaterialSystem_SortInfo_t *pSortInfo =
	    new MaterialSystem_SortInfo_t[g_pMaterialSystem->GetNumSortIDs()];
	g_pMaterialSystem->GetSortInfo( pSortInfo );
	pRenderContext->BindLightmapPage( pSortInfo[sortId].lightmapPageID );
	delete[] pSortInfo;
	pRenderContext->Bind( pMaterial );

	IMesh *pMesh = pRenderContext->GetDynamicMesh();
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
	// Clip-space corners (identity transforms), base UV across the quad, and the
	// lightmap UV over the lightmap's interior (its one-texel border excluded).
	const float corners[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for ( const float *st : corners )
	{
		meshBuilder.Position3f( st[0] * 2.0f - 1.0f, 1.0f - st[1] * 2.0f, 0.5f );
		meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0, st[0], st[1] );
		const float s = ( st[0] * ( kLightmapSize - 2 ) + 1 + offset[0] ) / pageSize[0];
		const float t = ( st[1] * ( kLightmapSize - 2 ) + 1 + offset[1] ) / pageSize[1];
		meshBuilder.TexCoord2f( 1, s, t );
		meshBuilder.TangentS3f( 1.0f, 0.0f, 0.0f );
		meshBuilder.TangentT3f( 0.0f, 1.0f, 0.0f );
		meshBuilder.AdvanceVertex();
	}
	meshBuilder.End();
	pMesh->Draw();
}

bool CMaterialPixelApp::ReadPixel( float fx, float fy, unsigned char rgb[3] )
{
	int width = 0, height = 0;
	g_pMaterialSystem->GetBackBufferDimensions( width, height );
	if ( width <= 0 || height <= 0 )
		return false;
	unsigned char rgba[4] = { 0, 0, 0, 0 };
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->ReadPixels( static_cast<int>( fx * width ), static_cast<int>( fy * height ), 1,
	    1, rgba, IMAGE_FORMAT_RGBA8888 );
	rgb[0] = rgba[0];
	rgb[1] = rgba[1];
	rgb[2] = rgba[2];
	return true;
}

// Writes the renderer, the HDR type the backend reports, and the readback
// self-check: a cleared, undrawn frame must read back as the clear color.
// Without it a backend whose readback returns zeros would "pass" every case
// that expects black.
void CMaterialPixelApp::WriteClearProbe( FILE *out )
{
	fprintf( out, "\"renderer\":\"%s\",\"hdr_type\":%d,",
	    CommandLine()->ParmValue( "-renderer", "" ),
	    static_cast<int>( g_pMaterialSystemHardwareConfig->GetHDRType() ) );
	unsigned char probe[3] = { 0, 0, 0 };
	g_pMaterialSystem->BeginFrame( 0 );
	{
		CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
		pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
		pRenderContext->ClearBuffers( true, true );
		ReadPixel( 0.5f, 0.5f, probe );
	}
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();
	fprintf( out, "\"clear_probe\":{\"clear\":[255,0,255],\"pixel\":[%d,%d,%d]},", probe[0],
	    probe[1], probe[2] );
}

bool CMaterialPixelApp::RunLightmapCases( FILE *out )
{
	static CSolidColorRegenerator s_BaseRegenerator;
	ITexture *pBase = g_pMaterialSystem->CreateProceduralTexture( "conformance/lightmap_base",
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	if ( !pBase )
		return false;
	pBase->SetTextureRegenerator( &s_BaseRegenerator );

	KeyValues *pKeys = new KeyValues( "LightmappedGeneric" );
	pKeys->SetString( "$basetexture", "conformance/lightmap_base" );
	IMaterial *pMaterial = g_pMaterialSystem->CreateMaterial( "conformance/lightmapped", pKeys );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
		return false;
	pMaterial->IncrementReferenceCount();
	// Precache it as map load does, which builds its shader render state (vertex
	// format, snapshots); an uncached material draws with no vertex format.
	g_pMaterialSystem->CacheUsedMaterials();
	if ( pMaterial->GetVertexFormat() == 0 )
	{
		Warning( "material pixel conformance: %s has no vertex format after precache\n",
		    pMaterial->GetName() );
		return false;
	}

	// One lightmap per case, allocated and packed the way map load does it.
	int sortIds[kLightmapCaseCount];
	int offsets[kLightmapCaseCount][2];
	g_pMaterialSystem->BeginLightmapAllocation();
	for ( int i = 0; i < kLightmapCaseCount; ++i )
		sortIds[i] = g_pMaterialSystem->AllocateLightmap(
		    kLightmapSize, kLightmapSize, offsets[i], pMaterial );
	g_pMaterialSystem->EndLightmapAllocation();

	std::vector<MaterialSystem_SortInfo_t> sortInfo( g_pMaterialSystem->GetNumSortIDs() );
	g_pMaterialSystem->GetSortInfo( sortInfo.data() );
	for ( int i = 0; i < kLightmapCaseCount; ++i )
	{
		float texels[kLightmapSize * kLightmapSize * 4];
		for ( int y = 0; y < kLightmapSize; ++y )
		{
			for ( int x = 0; x < kLightmapSize; ++x )
			{
				const float *value =
				    x < kLightmapSize / 2 ? kLightmapCases[i].left : kLightmapCases[i].right;
				float *texel = &texels[( y * kLightmapSize + x ) * 4];
				texel[0] = value[0];
				texel[1] = value[1];
				texel[2] = value[2];
				texel[3] = 1.0f;
			}
		}
		int size[2] = { kLightmapSize, kLightmapSize };
		g_pMaterialSystem->UpdateLightmap(
		    sortInfo[sortIds[i]].lightmapPageID, size, offsets[i], texels, NULL, NULL, NULL );
	}

	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"lightmap\"," );
	WriteClearProbe( out );
	fprintf( out, "\"cases\":[" );

	bool ok = true;
	const float kHalves[2][2] = { { 0.25f, 0.5f }, { 0.75f, 0.5f } };
	for ( int i = 0; i < kLightmapCaseCount; ++i )
	{
		const LightmapCase &c = kLightmapCases[i];
		s_BaseRegenerator.m_Split = false;
		for ( int k = 0; k < 3; ++k )
			s_BaseRegenerator.m_Color[k] = c.base[k];
		pBase->Download();

		unsigned char px[2][3] = {};
		ok = RenderCase( pMaterial, sortIds[i], offsets[i], sortInfo[sortIds[i]].lightmapPageID,
		         kHalves, 2, px ) &&
		     ok;
		fprintf( out,
		    "%s{\"name\":\"%s\",\"base\":[%d,%d,%d],"
		    "\"lightmap\":[[%g,%g,%g],[%g,%g,%g]],\"pixels\":[[%d,%d,%d],[%d,%d,%d]]}",
		    i ? "," : "", c.name, c.base[0], c.base[1], c.base[2], c.left[0], c.left[1], c.left[2],
		    c.right[0], c.right[1], c.right[2], px[0][0], px[0][1], px[0][2], px[1][0], px[1][1],
		    px[1][2] );
	}
	fprintf( out, "]," );

	// Orientation: texture row 0 (v = 0) sits on the quad's clip-space top edge
	// (y = +1), which D3D9 puts at the top of the frame and of readback. A red top
	// half and a blue bottom half, lit by a unit lightmap (ramp_high's left half),
	// are read at a quarter and three quarters of the height.
	const unsigned char top[3] = { 255, 0, 0 }, bottom[3] = { 0, 0, 255 };
	s_BaseRegenerator.m_Split = true;
	for ( int k = 0; k < 3; ++k )
	{
		s_BaseRegenerator.m_Color[k] = top[k];
		s_BaseRegenerator.m_Bottom[k] = bottom[k];
	}
	pBase->Download();
	const int unitCase = 3; // ramp_high: lightmap 1.0 on the left half
	const float kRows[2][2] = { { 0.25f, 0.25f }, { 0.25f, 0.75f } };
	unsigned char rows[2][3] = {};
	ok = RenderCase( pMaterial, sortIds[unitCase], offsets[unitCase],
	         sortInfo[sortIds[unitCase]].lightmapPageID, kRows, 2, rows ) &&
	     ok;
	fprintf( out,
	    "\"orientation\":{\"top_texel\":[%d,%d,%d],\"bottom_texel\":[%d,%d,%d],"
	    "\"pixels\":[[%d,%d,%d],[%d,%d,%d]]}",
	    top[0], top[1], top[2], bottom[0], bottom[1], bottom[2], rows[0][0], rows[0][1], rows[0][2],
	    rows[1][0], rows[1][1], rows[1][2] );
	s_BaseRegenerator.m_Split = false;

	// Tone-mapping scale: ramp_mid (white base, lightmap 0.25 | 0.5) drawn with the
	// linear tone-mapping scale at 2. D3D9 multiplies LightmappedGeneric's output
	// by it in integer HDR and forces it to 1 without HDR.
	const float toneScale = 2.0f;
	const int rampMid = 2;
	for ( int k = 0; k < 3; ++k )
		s_BaseRegenerator.m_Color[k] = kLightmapCases[rampMid].base[k];
	pBase->Download();
	unsigned char toned[2][3] = {};
	ok = RenderCase( pMaterial, sortIds[rampMid], offsets[rampMid],
	         sortInfo[sortIds[rampMid]].lightmapPageID, kHalves, 2, toned, toneScale ) &&
	     ok;
	fprintf( out,
	    ",\"tone_scale\":{\"case\":\"%s\",\"scale\":%g,"
	    "\"pixels\":[[%d,%d,%d],[%d,%d,%d]]}}\n",
	    kLightmapCases[rampMid].name, toneScale, toned[0][0], toned[0][1], toned[0][2], toned[1][0],
	    toned[1][1], toned[1][2] );
	pMaterial->DecrementReferenceCount();
	return ok;
}

// The client's histogram measurement, step for step (CHistogram_entry_t::IssueQuery
// and DoPreBloomTonemapping in game/client/viewpostprocess.cpp): the back buffer
// is copied to _rt_FullFrameFB (UpdateScreenEffectTexture), then for each
// luminance range dev/lumcompare is drawn over the center of the viewport with
// $C0_X/$C0_Y as the range and $C0_Z as the scale, inside an occlusion query.
// The shader keeps only pixels whose luminance is in range, so each query counts
// the pixels of one histogram bar.
bool CMaterialPixelApp::RunExposureCases( FILE *out )
{
	// The engine creates _rt_FullFrameFB at startup (matsys_interface.cpp
	// CreateFullFrameFBTexture); dev/lumcompare names it as its base texture.
	g_pMaterialSystem->BeginRenderTargetAllocation();
	ITexture *pFrameBuffer = g_pMaterialSystem->CreateNamedRenderTargetTextureEx2(
	    "_rt_FullFrameFB", 1, 1, RT_SIZE_FULL_FRAME_BUFFER,
	    g_pMaterialSystem->GetBackBufferFormat(), MATERIAL_RT_DEPTH_SHARED,
	    TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, CREATERENDERTARGETFLAGS_HDR );
	g_pMaterialSystem->EndRenderTargetAllocation();
	if ( !pFrameBuffer || pFrameBuffer->IsError() )
	{
		Warning( "material pixel conformance: cannot create _rt_FullFrameFB\n" );
		return false;
	}
	pFrameBuffer->IncrementReferenceCount();
	IMaterial *pMaterial = g_pMaterialSystem->FindMaterial( "dev/lumcompare", TEXTURE_GROUP_OTHER );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
	{
		Warning( "material pixel conformance: dev/lumcompare is not in the runtime content\n" );
		return false;
	}
	pMaterial->IncrementReferenceCount();
	g_pMaterialSystem->CacheUsedMaterials();
	IMaterialVar *pMin = pMaterial->FindVar( "$C0_X", NULL );
	IMaterialVar *pMax = pMaterial->FindVar( "$C0_Y", NULL );
	IMaterialVar *pScale = pMaterial->FindVar( "$C0_Z", NULL );

	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"exposure\"," );
	WriteClearProbe( out );

	int width = 0, height = 0;
	g_pMaterialSystem->GetBackBufferDimensions( width, height );
	fprintf( out, "\"frame\":[%d,%d],\"regions\":[", width, height );

	// The client's ranges (UpdateLuminanceRanges, algorithm 1): even steps with
	// more resolution at the dark end, and a last range that counts every pixel.
	// A range ending at 1.0 is tested up to 10000, so overbright pixels count.
	float rangeMin[kLuminanceRanges], rangeMax[kLuminanceRanges];
	for ( int i = 0; i < kLuminanceRanges; ++i )
	{
		if ( i == kLuminanceRanges - 1 )
		{
			rangeMin[i] = 0.0f;
			rangeMax[i] = 100000.0f;
			continue;
		}
		rangeMin[i] = powf( float( i ) / float( kLuminanceRanges - 1 ), 1.5f );
		rangeMax[i] = powf( float( i + 1 ) / float( kLuminanceRanges - 1 ), 1.5f );
		if ( rangeMax[i] == 1.0f )
			rangeMax[i] = 10000.0f;
	}
	// The measured rectangle, as IssueQuery derives it from the viewport.
	const int skipX = static_cast<int>( width * ( 0.5f * ( 1.0f - kExposureCenterRegionX ) ) );
	const int skipY = static_cast<int>( height * ( 0.5f * ( 1.0f - kExposureCenterRegionY ) ) );
	const int query[4] = { skipX, skipY, width - 2 * skipX, height - 2 * skipY };

	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	OcclusionQueryObjectHandle_t queries[kLuminanceRanges];
	for ( int i = 0; i < kLuminanceRanges; ++i )
		queries[i] = pRenderContext->CreateOcclusionQueryObject();

	bool ok = true;
	g_pMaterialSystem->BeginFrame( 0 );
	pRenderContext->Viewport( 0, 0, width, height );
	pRenderContext->ClearColor4ub( 0, 0, 0, 255 );
	pRenderContext->ClearBuffers( true, true );
	int rects[kExposureRegionCount][4];
	for ( int r = 0; r < kExposureRegionCount; ++r )
	{
		const int column = r % kExposureColumns, row = r / kExposureColumns;
		rects[r][0] = width * column / kExposureColumns;
		rects[r][1] = height * row / kExposureRows;
		rects[r][2] = width * ( column + 1 ) / kExposureColumns - rects[r][0];
		rects[r][3] = height * ( row + 1 ) / kExposureRows - rects[r][1];
		const unsigned char *c = kExposureRegions[r].color;
		pRenderContext->Viewport( rects[r][0], rects[r][1], rects[r][2], rects[r][3] );
		pRenderContext->ClearColor4ub( c[0], c[1], c[2], 255 );
		pRenderContext->ClearBuffers( true, false );
	}
	pRenderContext->Viewport( 0, 0, width, height );

	Rect_t whole = { 0, 0, width, height };
	pRenderContext->CopyRenderTargetToTextureEx( pFrameBuffer, 0, &whole, NULL );
	// Integer HDR measures the frame as displayed: the client passes a scale of 1
	// (only float HDR passes the tone-mapping scale).
	const float scale = 1.0f;
	for ( int i = 0; i < kLuminanceRanges; ++i )
	{
		pMin->SetFloatValue( rangeMin[i] );
		pMax->SetFloatValue( rangeMax[i] );
		pScale->SetFloatValue( scale );
		pRenderContext->BeginOcclusionQueryDrawing( queries[i] );
		pRenderContext->DrawScreenSpaceRectangle( pMaterial, query[0], query[1], query[2], query[3],
		    query[0], query[1], query[0] + query[2] - 1, query[1] + query[3] - 1, width, height );
		pRenderContext->EndOcclusionQueryDrawing( queries[i] );
	}
	// Read after the luminance draws: dev/lumcompare disables color writes, so
	// the frame still holds exactly the regions it was cleared to.
	for ( int r = 0; r < kExposureRegionCount; ++r )
	{
		unsigned char pixel[3] = {};
		ok = ReadPixel( ( rects[r][0] + 0.5f * rects[r][2] ) / width,
		         ( rects[r][1] + 0.5f * rects[r][3] ) / height, pixel ) &&
		     ok;
		const unsigned char *c = kExposureRegions[r].color;
		fprintf( out,
		    "%s{\"name\":\"%s\",\"color\":[%d,%d,%d],\"rect\":[%d,%d,%d,%d],"
		    "\"pixel\":[%d,%d,%d]}",
		    r ? "," : "", kExposureRegions[r].name, c[0], c[1], c[2], rects[r][0], rects[r][1],
		    rects[r][2], rects[r][3], pixel[0], pixel[1], pixel[2] );
	}
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();

	// Results arrive frames later, as the client reads them; poll across frames.
	int counts[kLuminanceRanges];
	for ( int i = 0; i < kLuminanceRanges; ++i )
		counts[i] = kQueryPending;
	int frames = 0;
	for ( ; frames < kMaxQueryPollFrames; ++frames )
	{
		bool pending = false;
		for ( int i = 0; i < kLuminanceRanges; ++i )
		{
			if ( counts[i] == kQueryPending )
				counts[i] = pRenderContext->OcclusionQuery_GetNumPixelsRendered( queries[i] );
			pending = pending || counts[i] == kQueryPending;
		}
		if ( !pending )
			break;
		g_pMaterialSystem->BeginFrame( 0 );
		pRenderContext->ClearBuffers( true, true );
		g_pMaterialSystem->EndFrame();
		g_pMaterialSystem->SwapBuffers();
	}

	// An occlusion query counts samples: with a multisampled back buffer every
	// pixel counts once per sample, which the client calibrates out with its
	// last range. The configured sample count is recorded next to the counts.
	fprintf( out,
	    "],\"query_rect\":[%d,%d,%d,%d],\"scale\":%g,\"aa_samples\":%d,\"poll_frames\":%d,"
	    "\"ranges\":[",
	    query[0], query[1], query[2], query[3], scale,
	    g_pMaterialSystem->GetCurrentConfigForVideoCard().m_nAASamples, frames );
	for ( int i = 0; i < kLuminanceRanges; ++i )
	{
		fprintf( out, "%s{\"min\":%.9g,\"max\":%.9g,\"pixels\":%d}", i ? "," : "", rangeMin[i],
		    rangeMax[i], counts[i] );
		pRenderContext->DestroyOcclusionQueryObject( queries[i] );
	}
	fprintf( out, "]}\n" );
	pMaterial->DecrementReferenceCount();
	pFrameBuffer->DecrementReferenceCount();
	return ok;
}

// A translation along x as the 3x4 pose-to-world matrix LoadBoneMatrix takes.
static matrix3x4_t SkinTranslation( float x )
{
	return matrix3x4_t( 1, 0, 0, x, 0, 1, 0, 0, 0, 0, 1, 0 );
}

bool CMaterialPixelApp::RunSkinningCases( FILE *out )
{
	static CSolidColorRegenerator s_SkinRegenerator;
	const unsigned char color[3] = { 40, 200, 90 };
	for ( int k = 0; k < 3; ++k )
		s_SkinRegenerator.m_Color[k] = color[k];
	ITexture *pBase = g_pMaterialSystem->CreateProceduralTexture( "conformance/skinning_base",
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	if ( !pBase )
		return false;
	pBase->SetTextureRegenerator( &s_SkinRegenerator );
	pBase->Download();

	KeyValues *pKeys = new KeyValues( "UnlitGeneric" );
	pKeys->SetString( "$basetexture", "conformance/skinning_base" );
	pKeys->SetInt( "$model", 1 );
	IMaterial *pMaterial = g_pMaterialSystem->CreateMaterial( "conformance/skinned", pKeys );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
		return false;
	pMaterial->IncrementReferenceCount();
	g_pMaterialSystem->CacheUsedMaterials();
	if ( pMaterial->GetVertexFormat() == 0 )
	{
		Warning( "material pixel conformance: %s has no vertex format after precache\n",
		    pMaterial->GetName() );
		return false;
	}

	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"skinning\"," );
	WriteClearProbe( out );
	fprintf( out, "\"color\":[%d,%d,%d],\"cases\":[", color[0], color[1], color[2] );

	// The model mesh format: the material's, plus two bone weights and the bone
	// indices, uncompressed (studiorendercontext.cpp R_StudioCreateStaticMeshes).
	const VertexFormat_t format = ( pMaterial->GetVertexFormat() & ~VERTEX_FORMAT_COMPRESSED ) |
	                              VERTEX_BONEWEIGHT( 2 ) | VERTEX_BONE_INDEX;
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	bool ok = true;
	for ( int i = 0; i < kSkinningCaseCount; ++i )
	{
		const SkinningCase &c = kSkinningCases[i];
		IMesh *pMesh = pRenderContext->CreateStaticMesh(
		    format, TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS, pMaterial );
		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, 4, 6 );
		const float corners[4][2] = { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };
		for ( int v = 0; v < 4; ++v )
		{
			meshBuilder.Position3f(
			    corners[v][0] * kSkinHalfWidth, corners[v][1] * kSkinHalfWidth, 0.5f );
			meshBuilder.Normal3f( 0.0f, 0.0f, -1.0f );
			meshBuilder.Color4ub( 255, 255, 255, 255 );
			meshBuilder.TexCoord2f( 0, 0.5f * ( corners[v][0] + 1 ), 0.5f * ( 1 - corners[v][1] ) );
			meshBuilder.BoneWeight( 0, c.weights[0] );
			meshBuilder.BoneWeight( 1, c.weights[1] );
			for ( int b = 0; b < 3; ++b )
				meshBuilder.BoneMatrix( b, c.index[b] );
			meshBuilder.BoneMatrix( 3, 0 );
			meshBuilder.AdvanceVertex();
		}
		// Clockwise on screen (the front face); reversed for a back-facing case.
		const unsigned short front[6] = { 0, 1, 2, 0, 2, 3 };
		const unsigned short back[6] = { 0, 2, 1, 0, 3, 2 };
		for ( unsigned short index : c.backFacing ? back : front )
			meshBuilder.FastIndex( index );
		meshBuilder.End();

		unsigned char thirds[3][3] = {};
		g_pMaterialSystem->BeginFrame( 0 );
		{
			int width = 0, height = 0;
			g_pMaterialSystem->GetBackBufferDimensions( width, height );
			pRenderContext->Viewport( 0, 0, width, height );
			pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
			pRenderContext->ClearBuffers( true, true );
			for ( int mode = MATERIAL_VIEW; mode <= MATERIAL_PROJECTION; ++mode )
			{
				pRenderContext->MatrixMode( static_cast<MaterialMatrixMode_t>( mode ) );
				pRenderContext->LoadIdentity();
			}
			// studiorender's order (R_StudioDrawGroupHWSkin): bone count, bones,
			// then the draw. A rigid draw sees bone 0 only as the MODEL matrix.
			pRenderContext->SetNumBoneWeights( c.numBoneWeights );
			pRenderContext->LoadBoneMatrix( 0, SkinTranslation( c.bone0 ) );
			for ( int b = 0; b < 3; ++b )
				pRenderContext->LoadBoneMatrix( b + 1, SkinTranslation( c.bones[b] ) );
			pRenderContext->Bind( pMaterial );
			pMesh->Draw();
			pRenderContext->SetNumBoneWeights( 0 );
			for ( int t = 0; t < 3; ++t )
				ok = ReadPixel( ( 2 * t + 1 ) / 6.0f, 0.5f, thirds[t] ) && ok;
		}
		g_pMaterialSystem->EndFrame();
		g_pMaterialSystem->SwapBuffers();
		pRenderContext->DestroyStaticMesh( pMesh );
		fprintf( out,
		    "%s{\"name\":\"%s\",\"expected_third\":%d,"
		    "\"pixels\":[[%d,%d,%d],[%d,%d,%d],[%d,%d,%d]]}",
		    i ? "," : "", c.name, c.expectedThird, thirds[0][0], thirds[0][1], thirds[0][2],
		    thirds[1][0], thirds[1][1], thirds[1][2], thirds[2][0], thirds[2][1], thirds[2][2] );
	}
	fprintf( out, "]}\n" );
	pMaterial->DecrementReferenceCount();
	return ok;
}

// Draws one case in its own frame over a magenta clear and reads the pixels at
// the given fractions of the back buffer (x from the left, y from the top).
bool CMaterialPixelApp::RenderCase( IMaterial *pMaterial, int sortId, const int offset[2],
    int lightmapPageId, const float ( *points )[2], int pointCount, unsigned char ( *pixels )[3],
    float toneScale )
{
	int pageSize[2] = { 0, 0 };
	g_pMaterialSystem->GetLightmapPageSize( lightmapPageId, &pageSize[0], &pageSize[1] );
	bool ok = true;
	g_pMaterialSystem->BeginFrame( 0 );
	{
		CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
		int width = 0, height = 0;
		g_pMaterialSystem->GetBackBufferDimensions( width, height );
		pRenderContext->Viewport( 0, 0, width, height );
		pRenderContext->SetToneMappingScaleLinear( Vector( toneScale, toneScale, toneScale ) );
		// Magenta: a pixel the quad did not cover is unmistakable.
		pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
		pRenderContext->ClearBuffers( true, true );
		DrawLightmappedQuad( pMaterial, sortId, offset, pageSize );
		for ( int i = 0; i < pointCount; ++i )
			ok = ReadPixel( points[i][0], points[i][1], pixels[i] ) && ok;
	}
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();
	return ok;
}

} // namespace

DEFINE_CONSOLE_STEAM_APPLICATION_OBJECT( CMaterialPixelApp );
