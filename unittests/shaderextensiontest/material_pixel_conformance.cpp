//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Per-material pixel conformance. Renders real Source materials through
//          the real material system and the selected render provider, and
//          reports the pixels each backend produced for fixed inputs.
//
//          The first family is LightmappedGeneric's lightmap term. Each case
//          draws one full-viewport quad with a solid base texture and a lightmap
//          allocated, packed and bound exactly as a compiled map's lightmaps are
//          (Begin/EndLightmapAllocation, UpdateLightmap with linear float texels,
//          BindLightmapPage). The lightmap's left and right halves hold different
//          values and one pixel is read from each half, so the result also shows
//          that the lightmap is addressed by its own texture coordinates.
//
//          This program measures; it does not judge. It writes the inputs and
//          the measured pixels as source-material-pixels/v1 JSON, and
//          tools/quality/material_pixel_conformance.py applies the oracle:
//          backend-independent properties on every backend, and agreement with
//          the D3D9 reference capture within a stated tolerance.
//
//          Run inside a staged game runtime (the driver stages one):
//            material_pixel_conformance -game portal -renderer <id>
//                -hdr <none|integer> -out <file.json>
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
#include "pixelwriter.h"
#include "render/builtin_shader_provider.h"
#include "render/legacy_shader_provider.h"
#include "tier0/icommandline.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "vtf/vtf.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <string>
#include <vector>

namespace
{

const int kWindowSize = 256;
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

private:
	bool RunLightmapCases( FILE *out );
	bool RenderCase( IMaterial *pMaterial, int sortId, const int offset[2], int lightmapPageId,
	    const float ( *points )[2], int pointCount, unsigned char ( *pixels )[3] );
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

int CMaterialPixelApp::Main()
{
	const char *outPath = CommandLine()->ParmValue( "-out", "" );
	const char *hdr = CommandLine()->ParmValue( "-hdr", "none" );
	const bool integerHdr = !Q_stricmp( hdr, "integer" );
	if ( !outPath[0] || ( !integerHdr && Q_stricmp( hdr, "none" ) ) )
	{
		Warning( "material pixel conformance: need -out <file> and -hdr <none|integer>\n" );
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
	const bool ok = RunLightmapCases( out );
	fclose( out );
	pLauncher->DestroyGameWindow();
	return ok ? 0 : 1;
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
	fprintf( out, "\"renderer\":\"%s\",\"hdr_type\":%d,",
	    CommandLine()->ParmValue( "-renderer", "" ),
	    static_cast<int>( g_pMaterialSystemHardwareConfig->GetHDRType() ) );
	// Readback self-check: a cleared, undrawn frame must read back as the clear
	// color. Without it a backend whose readback returns zeros would "pass" every
	// case that expects black.
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
	    "\"pixels\":[[%d,%d,%d],[%d,%d,%d]]}}\n",
	    top[0], top[1], top[2], bottom[0], bottom[1], bottom[2], rows[0][0], rows[0][1], rows[0][2],
	    rows[1][0], rows[1][1], rows[1][2] );
	s_BaseRegenerator.m_Split = false;
	pMaterial->DecrementReferenceCount();
	return ok;
}

// Draws one case in its own frame over a magenta clear and reads the pixels at
// the given fractions of the back buffer (x from the left, y from the top).
bool CMaterialPixelApp::RenderCase( IMaterial *pMaterial, int sortId, const int offset[2],
    int lightmapPageId, const float ( *points )[2], int pointCount, unsigned char ( *pixels )[3] )
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
		pRenderContext->SetToneMappingScaleLinear( Vector( 1, 1, 1 ) );
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
