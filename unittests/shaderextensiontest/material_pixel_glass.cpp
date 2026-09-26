//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "glass" family of the material pixel conformance harness.
//
//          Glass must show the scene behind it. Each case draws a wall of one
//          color and then a pane of real Portal glass in front of it, through a
//          perspective camera, and reads the pane's center and a point on the
//          wall beside it. Every case is drawn twice, over two walls whose
//          colors differ strongly in every channel. The oracle
//          (tools/quality/material_pixel_conformance.py) takes the pane's
//          transmission per channel from the two frames, (pane A - pane B) /
//          (wall A - wall B): what the glass adds of its own (base texture,
//          environment map) cancels, and an opaque pane transmits nothing.
//
//          The panes are the materials testchmb_a_01 draws:
//          glass/glasswindow_frosted (LightmappedGeneric, $translucent
//          $additive with an environment map; window glass),
//          glass/glasswindow_refract01 (Refract; the frame is copied to
//          _rt_PowerOfTwoFB first, as UpdateRefractTexture does) and
//          models/props/box_dropper_tube (VertexLitGeneric, $translucent
//          $additive with an environment map; model glass). A control pane,
//          the frosted base texture as an opaque LightmappedGeneric material,
//          must transmit nothing, so the measurement can tell opaque from clear.
//
//          World panes carry a lightmap of a dim uniform value, allocated,
//          packed and bound as map load and brush drawing do (the white page
//          saturates the frosted pane in integer HDR); every case binds
//          engine/defaultcubemap as the local cubemap, as the engine binds a
//          leaf's cubemap for env_cubemap.
//
//=============================================================================//

#include "material_pixel_glass.h"

#include "KeyValues.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "mathlib/vector4d.h"
#include "pixelwriter.h"
#include "tier2/tier2.h"
#include "vtf/vtf.h"

#include <cstdio>
#include <vector>

namespace
{

// Scene, in view space (x right, y up, looking down -z), in the game's units.
const float kWallZ = -400.0f;
const float kWallHalfSize = 2000.0f;
const float kPaneZ = -100.0f;
const float kPaneHalfSize = 40.0f;
const float kFovX = 90.0f;
const float kZNear = 1.0f;
const float kZFar = 2000.0f;
// Where the frame is read, as fractions of its width and height: the pane's
// center, and a point on the wall well outside the pane.
const float kPaneProbe[2] = { 0.5f, 0.5f };
const float kWallProbe[2] = { 0.1f, 0.1f };

// World panes' lightmaps: this size, one linear value in every texel.
const int kLightmapSize = 4;
const float kLightmapValue = 0.25f;

// The two walls (sRGB-encoded), at least 150 apart in every channel.
const unsigned char kWalls[2][3] = { { 210, 40, 200 }, { 30, 200, 20 } };

enum GlassExpect
{
	kAdditive, // passes the scene through and adds its own light
	kTinted,   // passes the scene through a tint (Refract)
	kOpaque,   // the control: passes nothing
};

enum GlassDraw
{
	kWorld, // a brush face: lightmap coordinates, the white lightmap page
	kModel, // a model: the ambient cube, no local lights
};

struct GlassCase
{
	const char *name;
	const char *material; // a runtime-content material, or a conformance/ one below
	GlassDraw draw;
	GlassExpect expect;
};

const GlassCase kGlassCases[] = {
    { "frosted_window", "glass/glasswindow_frosted", kWorld, kAdditive },
    { "refract_window", "glass/glasswindow_refract01", kWorld, kTinted },
    { "model_tube", "models/props/box_dropper_tube", kModel, kAdditive },
    { "opaque_control", "conformance/glass_opaque", kWorld, kOpaque },
};
const int kGlassCaseCount = sizeof( kGlassCases ) / sizeof( kGlassCases[0] );

const char *ExpectName( GlassExpect expect )
{
	return expect == kAdditive ? "additive" : expect == kTinted ? "tinted" : "opaque";
}

class CSolidRegenerator : public ITextureRegenerator
{
public:
	unsigned char m_Color[4] = { 255, 255, 255, 255 };
	void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTF, Rect_t *pRect ) override
	{
		for ( int mip = 0; mip < pVTF->MipCount(); ++mip )
		{
			int width = 0, height = 0, depth = 0;
			pVTF->ComputeMipLevelDimensions( mip, &width, &height, &depth );
			CPixelWriter writer;
			writer.SetPixelMemory(
			    pVTF->Format(), pVTF->ImageData( 0, 0, mip ), pVTF->RowSizeInBytes( mip ) );
			for ( int y = 0; y < height * depth; ++y )
			{
				writer.Seek( 0, y );
				for ( int x = 0; x < width; ++x )
					writer.WritePixel( m_Color[0], m_Color[1], m_Color[2], m_Color[3] );
			}
		}
	}
	void Release() override {}
};

class CGlassScene
{
public:
	bool Init();
	void Shutdown();
	// Draws case `c` over wall `wall` and reads the pane and the wall.
	bool RenderCase( const GlassCase &c, int wall, unsigned char pane[3], unsigned char beside[3] );

	IMaterial *m_pPanes[kGlassCaseCount] = {};
	int m_Width = 0;
	int m_Height = 0;

private:
	void SetView();
	bool AllocateLightmaps();
	void DrawQuad( IMaterial *pMaterial, float z, float halfSize, const float lightmapUv[2] );
	void UpdateFrontBufferTexture( IMaterial *pMaterial );

	CSolidRegenerator m_WallRegenerator;
	ITexture *m_pWallTexture = nullptr;
	IMaterial *m_pWall = nullptr;
	ITexture *m_pPowerOfTwoFB = nullptr;
	ITexture *m_pCubemap = nullptr;
	// Per case: the lightmap page and the lightmap center's coordinates
	// (world panes only).
	int m_LightmapPage[kGlassCaseCount] = {};
	float m_LightmapUv[kGlassCaseCount][2] = {};
};

bool CGlassScene::Init()
{
	g_pMaterialSystem->GetBackBufferDimensions( m_Width, m_Height );
	// The engine creates _rt_PowerOfTwoFB at startup (matsys_interface.cpp);
	// Refract samples the frame through it.
	g_pMaterialSystem->BeginRenderTargetAllocation();
	m_pPowerOfTwoFB = g_pMaterialSystem->CreateNamedRenderTargetTextureEx2( "_rt_PowerOfTwoFB",
	    1024, 1024, RT_SIZE_DEFAULT, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SHARED,
	    TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, CREATERENDERTARGETFLAGS_HDR );
	g_pMaterialSystem->EndRenderTargetAllocation();
	if ( !m_pPowerOfTwoFB || m_pPowerOfTwoFB->IsError() )
	{
		Warning( "glass conformance: cannot create _rt_PowerOfTwoFB\n" );
		return false;
	}
	m_pPowerOfTwoFB->IncrementReferenceCount();

	m_pCubemap = g_pMaterialSystem->FindTexture( "engine/defaultcubemap", TEXTURE_GROUP_CUBE_MAP );
	if ( !m_pCubemap || m_pCubemap->IsError() )
	{
		Warning( "glass conformance: engine/defaultcubemap is not in the runtime content\n" );
		return false;
	}
	m_pCubemap->IncrementReferenceCount();

	m_pWallTexture = g_pMaterialSystem->CreateProceduralTexture( "conformance/glass_wall",
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	if ( !m_pWallTexture )
		return false;
	m_pWallTexture->SetTextureRegenerator( &m_WallRegenerator );
	KeyValues *pKeys = new KeyValues( "UnlitGeneric" );
	pKeys->SetString( "$basetexture", "conformance/glass_wall" );
	m_pWall = g_pMaterialSystem->CreateMaterial( "conformance/glass_wall", pKeys );
	if ( !m_pWall || m_pWall->IsErrorMaterial() )
		return false;
	m_pWall->IncrementReferenceCount();

	// The control: the frosted pane's base texture, drawn opaque.
	pKeys = new KeyValues( "LightmappedGeneric" );
	pKeys->SetString( "$basetexture", "Glass/glasswindow_frosted" );
	IMaterial *pControl = g_pMaterialSystem->CreateMaterial( "conformance/glass_opaque", pKeys );
	if ( !pControl || pControl->IsErrorMaterial() )
		return false;

	for ( int i = 0; i < kGlassCaseCount; ++i )
	{
		const GlassCase &c = kGlassCases[i];
		m_pPanes[i] = c.expect == kOpaque
		                  ? pControl
		                  : g_pMaterialSystem->FindMaterial( c.material,
		                        c.draw == kModel ? TEXTURE_GROUP_MODEL : TEXTURE_GROUP_WORLD );
		if ( !m_pPanes[i] || m_pPanes[i]->IsErrorMaterial() )
		{
			Warning( "glass conformance: %s is not in the runtime content\n", c.material );
			return false;
		}
		m_pPanes[i]->IncrementReferenceCount();
	}
	// Precache as map load does, which builds the render state lightmap
	// allocation sorts by.
	g_pMaterialSystem->CacheUsedMaterials();
	return AllocateLightmaps();
}

// One lightmap per world pane, allocated, packed and filled as map load does
// (Begin/EndLightmapAllocation, UpdateLightmap with linear float texels).
bool CGlassScene::AllocateLightmaps()
{
	int sortIds[kGlassCaseCount] = {};
	int offsets[kGlassCaseCount][2] = {};
	g_pMaterialSystem->BeginLightmapAllocation();
	for ( int i = 0; i < kGlassCaseCount; ++i )
	{
		if ( kGlassCases[i].draw == kWorld )
			sortIds[i] = g_pMaterialSystem->AllocateLightmap(
			    kLightmapSize, kLightmapSize, offsets[i], m_pPanes[i] );
	}
	g_pMaterialSystem->EndLightmapAllocation();

	std::vector<MaterialSystem_SortInfo_t> sortInfo( g_pMaterialSystem->GetNumSortIDs() );
	g_pMaterialSystem->GetSortInfo( sortInfo.data() );
	float texels[kLightmapSize * kLightmapSize * 4];
	for ( int t = 0; t < kLightmapSize * kLightmapSize; ++t )
	{
		for ( int k = 0; k < 3; ++k )
			texels[t * 4 + k] = kLightmapValue;
		texels[t * 4 + 3] = 1.0f;
	}
	for ( int i = 0; i < kGlassCaseCount; ++i )
	{
		if ( kGlassCases[i].draw != kWorld )
			continue;
		const int page = sortInfo[sortIds[i]].lightmapPageID;
		int size[2] = { kLightmapSize, kLightmapSize };
		g_pMaterialSystem->UpdateLightmap( page, size, offsets[i], texels, NULL, NULL, NULL );
		int pageWidth = 0, pageHeight = 0;
		g_pMaterialSystem->GetLightmapPageSize( page, &pageWidth, &pageHeight );
		if ( pageWidth <= 0 || pageHeight <= 0 )
		{
			Warning( "glass conformance: lightmap page %d has no size\n", page );
			return false;
		}
		m_LightmapPage[i] = page;
		m_LightmapUv[i][0] = ( offsets[i][0] + 0.5f * kLightmapSize ) / pageWidth;
		m_LightmapUv[i][1] = ( offsets[i][1] + 0.5f * kLightmapSize ) / pageHeight;
	}
	return true;
}

void CGlassScene::Shutdown()
{
	for ( IMaterial *pMaterial : m_pPanes )
	{
		if ( pMaterial )
			pMaterial->DecrementReferenceCount();
	}
	if ( m_pWall )
		m_pWall->DecrementReferenceCount();
	if ( m_pWallTexture )
		m_pWallTexture->SetTextureRegenerator( nullptr );
	if ( m_pCubemap )
		m_pCubemap->DecrementReferenceCount();
	if ( m_pPowerOfTwoFB )
		m_pPowerOfTwoFB->DecrementReferenceCount();
}

void CGlassScene::SetView()
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->LoadIdentity();
	pRenderContext->PerspectiveX( kFovX, static_cast<double>( m_Width ) / m_Height, kZNear, kZFar );
	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->LoadIdentity();
	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->LoadIdentity();
}

// A camera-facing quad with every attribute the panes' shaders read: the
// normal toward the camera, the tangent along +x, base and lightmap
// coordinates. Components the bound material's format lacks are not stored.
void CGlassScene::DrawQuad(
    IMaterial *pMaterial, float z, float halfSize, const float lightmapUv[2] )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->Bind( pMaterial );
	IMesh *pMesh = pRenderContext->GetDynamicMesh();
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
	const float corners[4][2] = { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };
	const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	const float tangent[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	for ( int i = 0; i < 4; ++i )
	{
		meshBuilder.Position3f( corners[i][0] * halfSize, corners[i][1] * halfSize, z );
		meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0, uv[i][0], uv[i][1] );
		meshBuilder.TexCoord2f( 1, lightmapUv[0], lightmapUv[1] );
		meshBuilder.TangentS3f( 1.0f, 0.0f, 0.0f );
		meshBuilder.TangentT3f( 0.0f, 1.0f, 0.0f );
		meshBuilder.UserData( tangent );
		meshBuilder.AdvanceVertex();
	}
	meshBuilder.End();
	pMesh->Draw();
}

// UpdateRefractTexture (view_scene.h): copy the viewport into _rt_PowerOfTwoFB
// and make it the frame-buffer copy texture the material samples.
void CGlassScene::UpdateFrontBufferTexture( IMaterial *pMaterial )
{
	if ( !pMaterial->NeedsPowerOfTwoFrameBufferTexture() )
		return;
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	int x, y, w, h;
	pRenderContext->GetViewport( x, y, w, h );
	Rect_t rect = { x, y, w, h };
	pRenderContext->CopyRenderTargetToTextureEx( m_pPowerOfTwoFB, 0, &rect, NULL );
	pRenderContext->SetFrameBufferCopyTexture( m_pPowerOfTwoFB );
}

bool CGlassScene::RenderCase(
    const GlassCase &c, int wall, unsigned char pane[3], unsigned char beside[3] )
{
	for ( int k = 0; k < 3; ++k )
		m_WallRegenerator.m_Color[k] = kWalls[wall][k];
	m_pWallTexture->Download();
	const int index = static_cast<int>( &c - kGlassCases );
	std::vector<unsigned char> rgba( static_cast<size_t>( m_Width ) * m_Height * 4 );
	g_pMaterialSystem->BeginFrame( 0 );
	{
		CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
		pRenderContext->Viewport( 0, 0, m_Width, m_Height );
		pRenderContext->SetToneMappingScaleLinear( Vector( 1.0f, 1.0f, 1.0f ) );
		pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
		pRenderContext->ClearBuffers( true, true );
		SetView();
		pRenderContext->BindLocalCubemap( m_pCubemap );
		const float noLightmap[2] = { 0.0f, 0.0f };
		DrawQuad( m_pWall, kWallZ, kWallHalfSize, noLightmap );
		if ( c.draw == kWorld )
		{
			pRenderContext->BindLightmapPage( m_LightmapPage[index] );
		}
		else
		{
			// studiorender's lighting state: a neutral ambient cube, no local lights.
			Vector4D cube[6];
			for ( int f = 0; f < 6; ++f )
				cube[f].Init( 0.5f, 0.5f, 0.5f, 0.0f );
			pRenderContext->SetAmbientLightCube( cube );
			pRenderContext->DisableAllLocalLights();
			pRenderContext->SetNumBoneWeights( 0 );
		}
		UpdateFrontBufferTexture( m_pPanes[index] );
		DrawQuad( m_pPanes[index], kPaneZ, kPaneHalfSize, m_LightmapUv[index] );
		pRenderContext->ReadPixels( 0, 0, m_Width, m_Height, rgba.data(), IMAGE_FORMAT_RGBA8888 );
	}
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();

	const auto sample = [&]( const float at[2], unsigned char rgb[3] )
	{
		const int x = static_cast<int>( at[0] * m_Width );
		const int y = static_cast<int>( at[1] * m_Height );
		const unsigned char *texel = &rgba[( static_cast<size_t>( y ) * m_Width + x ) * 4];
		for ( int k = 0; k < 3; ++k )
			rgb[k] = texel[k];
	};
	sample( kPaneProbe, pane );
	sample( kWallProbe, beside );
	return true;
}

} // namespace

bool RunGlassCases( FILE *out, void ( *writeClearProbe )( FILE * ) )
{
	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"glass\"," );
	writeClearProbe( out );
	CGlassScene scene;
	if ( !scene.Init() )
	{
		scene.Shutdown();
		return false;
	}
	fprintf( out, "\"frame\":[%d,%d],\"walls\":[[%d,%d,%d],[%d,%d,%d]],\"cases\":[", scene.m_Width,
	    scene.m_Height, kWalls[0][0], kWalls[0][1], kWalls[0][2], kWalls[1][0], kWalls[1][1],
	    kWalls[1][2] );
	bool ok = true;
	for ( int i = 0; i < kGlassCaseCount; ++i )
	{
		const GlassCase &c = kGlassCases[i];
		unsigned char pane[2][3] = {}, beside[2][3] = {};
		for ( int wall = 0; wall < 2; ++wall )
			ok = scene.RenderCase( c, wall, pane[wall], beside[wall] ) && ok;
		fprintf( out,
		    "%s{\"name\":\"%s\",\"material\":\"%s\",\"shader\":\"%s\",\"expect\":\"%s\","
		    "\"pane\":[[%d,%d,%d],[%d,%d,%d]],\"beside\":[[%d,%d,%d],[%d,%d,%d]]}",
		    i ? "," : "", c.name, c.material, scene.m_pPanes[i]->GetShaderName(),
		    ExpectName( c.expect ), pane[0][0], pane[0][1], pane[0][2], pane[1][0], pane[1][1],
		    pane[1][2], beside[0][0], beside[0][1], beside[0][2], beside[1][0], beside[1][1],
		    beside[1][2] );
	}
	fprintf( out, "]}\n" );
	scene.Shutdown();
	return ok;
}
