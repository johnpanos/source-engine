//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "portal" family of the material pixel conformance harness.
//
//          It renders Portal's stencil portals, including recursion, with the
//          real Portal materials (models/portals/portal_stencil_hole,
//          portalstaticoverlay_1, portal_refract_1 and engine/writez_model)
//          through the real material system and the selected render provider.
//          The draw and state sequence is the client's, step for step:
//          CPortalRender::DrawPortalsUsingStencils (game/client/portal/
//          PortalRender.cpp) and the CPortalRenderable_FlatBasic draw methods it
//          calls (portalrenderable_flatbasic.cpp): stencil increment through the
//          hole, depth cleared under the stencil (ClearBuffersObeyStencil), the
//          view behind the portal drawn under a custom clip plane, depth restored
//          with WriteZ, the stencil decremented over the whole screen
//          (PerformFullScreenStencilOperation), and the portal's overlay drawn
//          last. The recursion depth is the client's: r_portal_stencil_depth (2),
//          limited by the stencil bits the backend reports.
//
//          The scene is synthetic but uses the game's units: a 64x108 portal
//          100 units in front of the camera, a wall behind it whose color names
//          the recursion level that drew it, and a floor-like blocker in front of
//          the portal that straddles the plane of the exit portal, so the custom
//          clip plane cuts it. Each nested view moves the camera back and to the
//          side, so the portal appears smaller and off center at each level.
//
//          Every frame is written to a raw RGB file next to the report; the
//          oracle (tools/quality/material_pixel_portal.py) holds the frames to a
//          ray-traced model of the same scene and to the D3D9 reference.
//
//=============================================================================//

#include "material_pixel_portal.h"

#include "KeyValues.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/materialsystem_config.h"
#include "pixelwriter.h"
#include "tier1/strtools.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "vtf/vtf.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{

// CPortalRender's limits (PortalRender.h / r_portal_stencil_depth default).
const int kMaxPortalRecursiveViews = 11;
const int kPortalStencilDepth = 2;
// portal_shareddefs.h
const float kPortalHalfWidth = 32.0f;
const float kPortalHalfHeight = 54.0f;
// DrawSimplePortalMesh's default forward offset.
const float kPortalForwardOffset = 0.25f;

// Scene, in view space of the level-0 camera (x right, y up, looking down -z).
// The portal faces the camera; its "right" is the direction to the right when
// looking out of it, as Source's AngleVectors computes it (forward x up).
const float kPortalCenter[3] = { 0.0f, 0.0f, -100.0f };
const float kPortalForward[3] = { 0.0f, 0.0f, 1.0f };
const float kPortalUp[3] = { 0.0f, 1.0f, 0.0f };
const float kPortalRight[3] = { -1.0f, 0.0f, 0.0f };
// Each nested view's camera is the parent's moved by this (camera position).
const float kLevelCameraStep[3] = { 20.0f, 10.0f, 100.0f };
// The exit portal of a nested view sits this far in front of its camera, facing
// away from it: the custom clip plane keeps what lies beyond it.
const float kExitPortalDistance = 100.0f;
const float kWallZ = -400.0f;
const float kWallHalfSize = 2000.0f;
// Blocker: a horizontal quad at y = kBlockerY, x in [0, 40], z in [-40, 40].
const float kBlockerY = -5.0f;
const float kBlockerX[2] = { 0.0f, 40.0f };
const float kBlockerZ[2] = { -40.0f, 40.0f };
const float kFovX = 90.0f;
const float kZNear = 1.0f;
const float kZFar = 2000.0f;

// sRGB-encoded wall colors per recursion level, and the blocker's.
const unsigned char kLevelColors[3][3] = { { 200, 60, 40 }, { 40, 180, 70 }, { 50, 80, 200 } };
// The blocker drawn by each level, so the frame names the level that drew it.
const unsigned char kBlockerColors[3][3] = {
    { 230, 220, 40 }, { 220, 120, 220 }, { 40, 220, 220 } };
// The static overlay's noise scroll is driven by $time; a fixed time makes the
// frame reproducible (the shader falls back to the device clock at time 0).
const float kPortalTime = 12.5f;

struct PortalCase
{
	const char *name;
	float openAmount;   // $PortalOpenAmount (the client's m_fOpenAmount)
	float staticAmount; // $PortalStatic (m_fStaticAmount); 1 draws no view
};
const PortalCase kPortalCases[] = {
    { "recursion", 1.0f, 0.0f },
    { "opening", 0.5f, 0.0f },
    { "static", 1.0f, 1.0f },
};

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

void Add( const float *a, const float *b, float scale, float *out )
{
	for ( int i = 0; i < 3; ++i )
		out[i] = a[i] + b[i] * scale;
}

float Dot( const float *a, const float *b )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

class CPortalScene
{
public:
	bool Init();
	void Shutdown();
	// Renders one case in its own frame and writes its report entry and frame.
	bool RenderCase( const PortalCase &c, FILE *out, const char *framePath, bool first );

	int m_MaxDepth = 0;

private:
	IMaterial *SolidMaterial( const char *name, const unsigned char color[3] );
	void SetLevelView( int level );
	void DrawScene( int level );
	void DrawQuad( IMaterial *pMaterial, const float corners[4][3] );
	void DrawSimplePortalMesh( IMaterial *pMaterial, float forwardOffset );
	void UpdateFrontBufferTextures( IMaterial *pMaterial );
	void DrawPortalsUsingStencils( int level );
	void DrawPortal( int level );

	std::vector<ITexture *> m_Textures;
	std::vector<CSolidRegenerator *> m_Regenerators;
	IMaterial *m_pLevelWalls[3] = {};
	IMaterial *m_pBlockers[3] = {};
	IMaterial *m_pStencilHole = nullptr;
	IMaterial *m_pStaticOverlay = nullptr;
	IMaterial *m_pRefract = nullptr;
	IMaterial *m_pWriteZ = nullptr;
	ITexture *m_pPowerOfTwoFB = nullptr;
	const PortalCase *m_pCase = nullptr;
	int m_Width = 0;
	int m_Height = 0;
};

IMaterial *CPortalScene::SolidMaterial( const char *name, const unsigned char color[3] )
{
	char textureName[128];
	V_snprintf( textureName, sizeof( textureName ), "conformance/portal_%s", name );
	ITexture *pTexture = g_pMaterialSystem->CreateProceduralTexture( textureName,
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	if ( !pTexture )
		return nullptr;
	CSolidRegenerator *pRegenerator = new CSolidRegenerator;
	for ( int k = 0; k < 3; ++k )
		pRegenerator->m_Color[k] = color[k];
	pTexture->SetTextureRegenerator( pRegenerator );
	pTexture->Download();
	m_Textures.push_back( pTexture );
	m_Regenerators.push_back( pRegenerator );

	KeyValues *pKeys = new KeyValues( "UnlitGeneric" );
	pKeys->SetString( "$basetexture", textureName );
	char materialName[128];
	V_snprintf( materialName, sizeof( materialName ), "conformance/portal_%s", name );
	IMaterial *pMaterial = g_pMaterialSystem->CreateMaterial( materialName, pKeys );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
		return nullptr;
	pMaterial->IncrementReferenceCount();
	return pMaterial;
}

bool CPortalScene::Init()
{
	// The engine creates _rt_PowerOfTwoFB at startup (matsys_interface.cpp); the
	// opening portal's refraction samples the frame through it.
	g_pMaterialSystem->BeginRenderTargetAllocation();
	m_pPowerOfTwoFB = g_pMaterialSystem->CreateNamedRenderTargetTextureEx2( "_rt_PowerOfTwoFB",
	    1024, 1024, RT_SIZE_DEFAULT, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SHARED,
	    TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, CREATERENDERTARGETFLAGS_HDR );
	g_pMaterialSystem->EndRenderTargetAllocation();
	if ( !m_pPowerOfTwoFB || m_pPowerOfTwoFB->IsError() )
	{
		Warning( "portal conformance: cannot create _rt_PowerOfTwoFB\n" );
		return false;
	}
	m_pPowerOfTwoFB->IncrementReferenceCount();

	static const char *const kLevelNames[3] = { "wall0", "wall1", "wall2" };
	for ( int level = 0; level < 3; ++level )
	{
		m_pLevelWalls[level] = SolidMaterial( kLevelNames[level], kLevelColors[level] );
		if ( !m_pLevelWalls[level] )
			return false;
	}
	static const char *const kBlockerNames[3] = { "blocker0", "blocker1", "blocker2" };
	for ( int level = 0; level < 3; ++level )
	{
		m_pBlockers[level] = SolidMaterial( kBlockerNames[level], kBlockerColors[level] );
		if ( !m_pBlockers[level] )
			return false;
	}

	struct
	{
		IMaterial **ppMaterial;
		const char *name;
	} content[] = {
	    { &m_pStencilHole, "models/portals/portal_stencil_hole" },
	    { &m_pStaticOverlay, "models/portals/portalstaticoverlay_1" },
	    { &m_pRefract, "models/portals/portal_refract_1" },
	    { &m_pWriteZ, "engine/writez_model" },
	};
	for ( auto &entry : content )
	{
		*entry.ppMaterial =
		    g_pMaterialSystem->FindMaterial( entry.name, TEXTURE_GROUP_CLIENT_EFFECTS );
		if ( !*entry.ppMaterial || ( *entry.ppMaterial )->IsErrorMaterial() )
		{
			Warning( "portal conformance: %s is not in the runtime content\n", entry.name );
			return false;
		}
		( *entry.ppMaterial )->IncrementReferenceCount();
	}
	g_pMaterialSystem->CacheUsedMaterials();

	// CPortalRender::DrawPortalsUsingStencils' depth limit.
	const int stencilBits = g_pMaterialSystemHardwareConfig->StencilBufferBits();
	const int stencilLevels = stencilBits >= 31 ? kMaxPortalRecursiveViews : ( 1 << stencilBits );
	m_MaxDepth = kPortalStencilDepth;
	if ( m_MaxDepth > kMaxPortalRecursiveViews - 1 )
		m_MaxDepth = kMaxPortalRecursiveViews - 1;
	if ( m_MaxDepth > stencilLevels - 1 )
		m_MaxDepth = stencilLevels - 1;
	g_pMaterialSystem->GetBackBufferDimensions( m_Width, m_Height );
	return true;
}

void CPortalScene::Shutdown()
{
	IMaterial *materials[] = { m_pLevelWalls[0], m_pLevelWalls[1], m_pLevelWalls[2], m_pBlockers[0],
	    m_pBlockers[1], m_pBlockers[2], m_pStencilHole, m_pStaticOverlay, m_pRefract, m_pWriteZ };
	for ( IMaterial *pMaterial : materials )
	{
		if ( pMaterial )
			pMaterial->DecrementReferenceCount();
	}
	for ( ITexture *pTexture : m_Textures )
		pTexture->SetTextureRegenerator( nullptr );
	for ( CSolidRegenerator *pRegenerator : m_Regenerators )
		delete pRegenerator;
	if ( m_pPowerOfTwoFB )
		m_pPowerOfTwoFB->DecrementReferenceCount();
}

// The camera of recursion level `level`: projection and a view that places the
// level's camera, as Push3DView does for each portal view.
void CPortalScene::SetLevelView( int level )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->LoadIdentity();
	pRenderContext->PerspectiveX( kFovX, static_cast<double>( m_Width ) / m_Height, kZNear, kZFar );
	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->LoadIdentity();
	pRenderContext->Translate(
	    -kLevelCameraStep[0] * level, -kLevelCameraStep[1] * level, -kLevelCameraStep[2] * level );
	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->LoadIdentity();
}

void CPortalScene::DrawQuad( IMaterial *pMaterial, const float corners[4][3] )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->Bind( pMaterial );
	IMesh *pMesh = pRenderContext->GetDynamicMesh();
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
	const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for ( int i = 0; i < 4; ++i )
	{
		meshBuilder.Position3fv( corners[i] );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0, uv[i][0], uv[i][1] );
		meshBuilder.AdvanceVertex();
	}
	meshBuilder.End();
	pMesh->Draw();
}

// The world a view of level `level` draws before its portals: the wall that
// names the level and the blocker.
void CPortalScene::DrawScene( int level )
{
	const float s = kWallHalfSize;
	const float wall[4][3] = {
	    { -s, s, kWallZ }, { s, s, kWallZ }, { s, -s, kWallZ }, { -s, -s, kWallZ } };
	DrawQuad( m_pLevelWalls[level], wall );
	const float blocker[4][3] = { { kBlockerX[0], kBlockerY, kBlockerZ[0] },
	    { kBlockerX[1], kBlockerY, kBlockerZ[0] }, { kBlockerX[1], kBlockerY, kBlockerZ[1] },
	    { kBlockerX[0], kBlockerY, kBlockerZ[1] } };
	DrawQuad( m_pBlockers[level], blocker );
}

// CPortalRenderable_FlatBasic::DrawSimplePortalMesh, vertex for vertex.
void CPortalScene::DrawSimplePortalMesh( IMaterial *pMaterial, float forwardOffset )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->Bind( pMaterial );
	UpdateFrontBufferTextures( pMaterial );

	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	float center[3];
	Add( kPortalCenter, kPortalForward, forwardOffset, center );
	float verts[4][3];
	for ( int i = 0; i < 4; ++i )
	{
		const float right = ( i < 2 ) ? kPortalHalfWidth : -kPortalHalfWidth;
		const float up = ( i % 2 ) ? kPortalHalfHeight : -kPortalHalfHeight;
		float t[3];
		Add( center, kPortalRight, right, t );
		Add( t, kPortalUp, up, verts[i] );
	}
	const float texcoords[4][2] = { { 0, 1 }, { 0, 0 }, { 1, 1 }, { 1, 0 } };
	float tangent[4] = { -kPortalRight[0], -kPortalRight[1], -kPortalRight[2], 1.0f };

	CMeshBuilder meshBuilder;
	IMesh *pMesh = pRenderContext->GetDynamicMesh( false );
	meshBuilder.Begin( pMesh, MATERIAL_TRIANGLE_STRIP, 2 );
	for ( int i = 0; i < 4; ++i )
	{
		meshBuilder.Position3fv( verts[i] );
		meshBuilder.TexCoord2f( 0, texcoords[i][0], texcoords[i][1] );
		meshBuilder.TexCoord2f( 1, texcoords[i][0], texcoords[i][1] );
		meshBuilder.Normal3f( kPortalForward[0], kPortalForward[1], kPortalForward[2] );
		meshBuilder.UserData( tangent );
		meshBuilder.AdvanceVertex();
	}
	meshBuilder.End();
	pMesh->Draw();

	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->PopMatrix();
}

// UpdateFrontBufferTexturesForMaterial -> UpdateRefractTexture (view_scene.h):
// copy the current viewport into _rt_PowerOfTwoFB and make it the frame-buffer
// copy texture the material samples.
void CPortalScene::UpdateFrontBufferTextures( IMaterial *pMaterial )
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

// CPortalRenderable_FlatBasic::DrawPortal, stencil branch. The harness has one
// linked portal and never renders through its exit, so the main view draws the
// overlay (and would draw the render fix mesh, which only exists when the
// camera is within a portal height of the portal), and nested views draw the
// overlay (the depth doubler needs a remembered previous-frame view).
void CPortalScene::DrawPortal( int /* level */ )
{
	DrawSimplePortalMesh( m_pStaticOverlay, kPortalForwardOffset );
}

// CPortalRender::DrawPortalsUsingStencils for one portal, `level` being
// m_iViewRecursionLevel.
void CPortalScene::DrawPortalsUsingStencils( int level )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	if ( level >= m_MaxDepth )
	{
		DrawPortal( level );
		return;
	}
	pRenderContext->Flush( true );

	const int parentRef = level;
	const int childRef = parentRef + 1;
	if ( level == 0 )
	{
		pRenderContext->SetStencilEnable( true );
		pRenderContext->SetStencilCompareFunction( STENCILCOMPARISONFUNCTION_ALWAYS );
		pRenderContext->SetStencilPassOperation( STENCILOPERATION_REPLACE );
		pRenderContext->SetStencilFailOperation( STENCILOPERATION_KEEP );
		pRenderContext->SetStencilZFailOperation( STENCILOPERATION_KEEP );
		pRenderContext->SetStencilTestMask( 0xFF );
		pRenderContext->SetStencilWriteMask( 0xFF );
		pRenderContext->SetStencilReferenceValue( 0 );
	}

	// Step 0: effects before cutting the hole (the opening refraction).
	pRenderContext->SetStencilCompareFunction( STENCILCOMPARISONFUNCTION_EQUAL );
	pRenderContext->SetStencilPassOperation( STENCILOPERATION_KEEP );
	pRenderContext->SetStencilFailOperation( STENCILOPERATION_KEEP );
	pRenderContext->SetStencilZFailOperation( STENCILOPERATION_KEEP );
	pRenderContext->SetStencilReferenceValue( parentRef );
	if ( m_pCase->openAmount > 0.0f && m_pCase->openAmount < 1.0f )
		DrawSimplePortalMesh( m_pRefract, kPortalForwardOffset );

	// Step 1: write the stencil values through the hole.
	pRenderContext->SetStencilPassOperation( STENCILOPERATION_INCR );
	DrawSimplePortalMesh( m_pStencilHole, kPortalForwardOffset );

	// Step 2: clear depth where the stencil holds the child level.
	pRenderContext->SetStencilPassOperation( STENCILOPERATION_KEEP );
	pRenderContext->SetStencilReferenceValue( childRef );
	pRenderContext->ClearBuffersObeyStencil( false, true );

	// Step 3: the view behind the portal (RenderPortalViewToBackBuffer), unless
	// the portal is all static.
	if ( m_pCase->staticAmount < 1.0f )
	{
		float exitCenter[3];
		const float camera[3] = { kLevelCameraStep[0] * childRef, kLevelCameraStep[1] * childRef,
		    kLevelCameraStep[2] * childRef };
		const float exitForward[3] = { 0.0f, 0.0f, -1.0f };
		Add( camera, exitForward, kExitPortalDistance, exitCenter );
		float halfBack[3];
		Add( exitCenter, exitForward, -0.5f, halfBack );
		float plane[4] = {
		    exitForward[0], exitForward[1], exitForward[2], Dot( exitForward, halfBack ) };
		pRenderContext->PushCustomClipPlane( plane );
		pRenderContext->MatrixMode( MATERIAL_VIEW );
		pRenderContext->PushMatrix();
		pRenderContext->MatrixMode( MATERIAL_PROJECTION );
		pRenderContext->PushMatrix();
		SetLevelView( childRef );

		DrawScene( childRef );
		DrawPortalsUsingStencils( childRef );

		pRenderContext->MatrixMode( MATERIAL_PROJECTION );
		pRenderContext->PopMatrix();
		pRenderContext->MatrixMode( MATERIAL_VIEW );
		pRenderContext->PopMatrix();
		pRenderContext->MatrixMode( MATERIAL_MODEL );
		pRenderContext->PopCustomClipPlane();
	}

	pRenderContext->SetStencilCompareFunction( STENCILCOMPARISONFUNCTION_EQUAL );
	pRenderContext->SetStencilPassOperation( STENCILOPERATION_KEEP );
	pRenderContext->SetStencilFailOperation( STENCILOPERATION_KEEP );
	pRenderContext->SetStencilZFailOperation( STENCILOPERATION_KEEP );
	pRenderContext->SetStencilTestMask( 0xFF );
	pRenderContext->SetStencilWriteMask( 0xFF );
	pRenderContext->SetStencilReferenceValue( childRef );

	// Step 4: DrawPostStencilFixes (no fog in this scene, so no fog quad).
	pRenderContext->ClearBuffersObeyStencil( false, true );
	DrawSimplePortalMesh( m_pWriteZ, 0.0f );

	// Step 5: restore the stencil to the parent level.
	pRenderContext->SetStencilReferenceValue( childRef );
	pRenderContext->SetStencilCompareFunction( STENCILCOMPARISONFUNCTION_EQUAL );
	pRenderContext->SetStencilPassOperation( STENCILOPERATION_DECR );
	pRenderContext->SetStencilFailOperation( STENCILOPERATION_KEEP );
	pRenderContext->SetStencilZFailOperation( STENCILOPERATION_KEEP );
	pRenderContext->PerformFullScreenStencilOperation();

	// Step 6.
	if ( level == 0 )
	{
		pRenderContext->SetStencilEnable( false );
		pRenderContext->SetStencilCompareFunction( STENCILCOMPARISONFUNCTION_NEVER );
		pRenderContext->SetStencilPassOperation( STENCILOPERATION_KEEP );
		pRenderContext->SetStencilFailOperation( STENCILOPERATION_KEEP );
		pRenderContext->SetStencilZFailOperation( STENCILOPERATION_KEEP );
		pRenderContext->SetStencilTestMask( 0xFF );
		pRenderContext->SetStencilWriteMask( 0xFF );
		pRenderContext->SetStencilReferenceValue( 0 );
	}
	else
	{
		pRenderContext->SetStencilReferenceValue( parentRef );
		pRenderContext->SetStencilCompareFunction( STENCILCOMPARISONFUNCTION_EQUAL );
		pRenderContext->SetStencilPassOperation( STENCILOPERATION_KEEP );
	}
	pRenderContext->Flush( true );
	DrawPortal( level );
}

bool CPortalScene::RenderCase( const PortalCase &c, FILE *out, const char *framePath, bool first )
{
	m_pCase = &c;
	IMaterial *portalMaterials[] = { m_pStencilHole, m_pStaticOverlay, m_pRefract };
	for ( IMaterial *pMaterial : portalMaterials )
	{
		// The proxies the client binds (PortalOpenAmount, PortalStatic,
		// CurrentTime) are replaced by fixed values.
		bool found = false;
		if ( IMaterialVar *pVar = pMaterial->FindVar( "$PortalOpenAmount", &found, false ) )
			pVar->SetFloatValue( c.openAmount );
		if ( IMaterialVar *pVar = pMaterial->FindVar( "$PortalStatic", &found, false ) )
			pVar->SetFloatValue( c.staticAmount );
		if ( IMaterialVar *pVar = pMaterial->FindVar( "$time", &found, false ) )
			pVar->SetFloatValue( kPortalTime );
	}

	std::vector<unsigned char> rgba( static_cast<size_t>( m_Width ) * m_Height * 4 );
	g_pMaterialSystem->BeginFrame( 0 );
	{
		CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
		pRenderContext->Viewport( 0, 0, m_Width, m_Height );
		pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
		pRenderContext->ClearBuffers( true, true, true );
		SetLevelView( 0 );
		DrawScene( 0 );
		DrawPortalsUsingStencils( 0 );
		pRenderContext->ReadPixels( 0, 0, m_Width, m_Height, rgba.data(), IMAGE_FORMAT_RGBA8888 );
	}
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();

	FILE *frame = fopen( framePath, "wb" );
	if ( !frame )
	{
		Warning( "portal conformance: cannot write %s\n", framePath );
		return false;
	}
	for ( size_t i = 0; i < rgba.size(); i += 4 )
		fwrite( &rgba[i], 1, 3, frame );
	fclose( frame );

	const char *base = V_UnqualifiedFileName( framePath );
	fprintf( out,
	    "%s{\"name\":\"%s\",\"open_amount\":%g,\"static_amount\":%g,\"frame_file\":\"%s\"}",
	    first ? "" : ",", c.name, c.openAmount, c.staticAmount, base );
	return true;
}

} // namespace

bool RunPortalCases( FILE *out, const char *outPath, void ( *writeClearProbe )( FILE * ) )
{
	CPortalScene scene;
	if ( !scene.Init() )
	{
		scene.Shutdown();
		return false;
	}
	int width = 0, height = 0;
	g_pMaterialSystem->GetBackBufferDimensions( width, height );

	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"portal\"," );
	writeClearProbe( out );
	fprintf( out,
	    "\"frame\":[%d,%d],\"stencil_bits\":%d,\"max_depth\":%d,\"aa_samples\":%d,"
	    "\"dx_level\":%d,\"force_trilinear\":%s,\"force_anisotropy\":%d,"
	    "\"max_anisotropy\":%d,\"max_user_clip_planes\":%d,\"fast_clip\":%s,\"scene\":{\"fov_x\":%"
	    "g,\"z_near\":%g,\"z_far\":%g,"
	    "\"portal_center\":[%g,%g,%g],\"portal_half_size\":[%g,%g],\"portal_forward_offset\":%g,"
	    "\"camera_step\":[%g,%g,%g],\"exit_portal_distance\":%g,\"wall_z\":%g,"
	    "\"blocker\":{\"y\":%g,\"x\":[%g,%g],\"z\":[%g,%g],"
	    "\"colors\":[[%d,%d,%d],[%d,%d,%d],[%d,%d,%d]]},"
	    "\"level_colors\":[[%d,%d,%d],[%d,%d,%d],[%d,%d,%d]],\"time\":%g},\"cases\":[",
	    width, height, g_pMaterialSystemHardwareConfig->StencilBufferBits(), scene.m_MaxDepth,
	    g_pMaterialSystem->GetCurrentConfigForVideoCard().m_nAASamples,
	    g_pMaterialSystemHardwareConfig->GetDXSupportLevel(),
	    g_pMaterialSystem->GetCurrentConfigForVideoCard().ForceTrilinear() ? "true" : "false",
	    g_pMaterialSystem->GetCurrentConfigForVideoCard().m_nForceAnisotropicLevel,
	    g_pMaterialSystemHardwareConfig->MaximumAnisotropicLevel(),
	    g_pMaterialSystemHardwareConfig->MaxUserClipPlanes(),
	    g_pMaterialSystemHardwareConfig->UseFastClipping() ? "true" : "false", kFovX, kZNear, kZFar,
	    kPortalCenter[0], kPortalCenter[1], kPortalCenter[2], kPortalHalfWidth, kPortalHalfHeight,
	    kPortalForwardOffset, kLevelCameraStep[0], kLevelCameraStep[1], kLevelCameraStep[2],
	    kExitPortalDistance, kWallZ, kBlockerY, kBlockerX[0], kBlockerX[1], kBlockerZ[0],
	    kBlockerZ[1], kBlockerColors[0][0], kBlockerColors[0][1], kBlockerColors[0][2],
	    kBlockerColors[1][0], kBlockerColors[1][1], kBlockerColors[1][2], kBlockerColors[2][0],
	    kBlockerColors[2][1], kBlockerColors[2][2], kLevelColors[0][0], kLevelColors[0][1],
	    kLevelColors[0][2], kLevelColors[1][0], kLevelColors[1][1], kLevelColors[1][2],
	    kLevelColors[2][0], kLevelColors[2][1], kLevelColors[2][2], kPortalTime );

	bool ok = true;
	bool first = true;
	for ( const PortalCase &c : kPortalCases )
	{
		std::string framePath = std::string( outPath ) + "." + c.name + ".rgb";
		ok = scene.RenderCase( c, out, framePath.c_str(), first ) && ok;
		first = false;
	}
	fprintf( out, "]}\n" );
	scene.Shutdown();
	return ok;
}
