//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "softparticle" family of the material pixel conformance harness.
//
//          Soft particles fade where they meet the scene: SpriteCard's
//          DEPTHBLEND (spritecard_ps2x.fxc, common_ps_fxc.h DepthFeathering)
//          scales a card's alpha by how far in front of the opaque scene it is.
//          The scene's depth comes from the frame copy the engine takes after
//          the opaque passes (view_scene.cpp UpdateFullScreenDepthTexture:
//          CopyRenderTargetToTextureEx into _rt_FullFrameDepth, an alias of
//          _rt_PowerOfTwoFB), whose alpha holds the projected z over the
//          dest-alpha depth range on D3D9 PC.
//
//          Each case draws an opaque wall, takes that copy exactly as the
//          engine does, then draws one sprite card in front of the wall, built
//          vertex for vertex as C_OP_RenderSprites::RenderSpriteCard builds it,
//          with a white texture and a red vertex color. The card's center is
//          read. A control card without DEPTHBLEND gives the card's unfeathered
//          color; the oracle (tools/quality/material_pixel_conformance.py)
//          recovers each case's alpha from the two and holds it to D3D9's
//          formula, including the copy's 8-bit alpha.
//
//=============================================================================//

#include "material_pixel_softparticle.h"

#include "KeyValues.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "pixelwriter.h"
#include "tier1/strtools.h"
#include "tier2/tier2.h"
#include "vtf/vtf.h"

#include <cstdio>
#include <vector>

namespace
{

// View space (x right, y up, looking down -z), in the game's units.
const float kFovX = 90.0f;
const float kZNear = 1.0f;
const float kZFar = 2000.0f;
const float kWallHalfSize = 4000.0f;
const float kCardRadius = 8.0f;
const float kDepthBlendScale = 50.0f; // SpriteCard's $depthblendscale default
const unsigned char kWallColor[3] = { 30, 60, 200 };
const unsigned char kCardColor[4] = { 255, 40, 0, 255 };

struct SoftParticleCase
{
	const char *name;
	float wallDistance; // from the camera
	float gap;          // how far in front of the wall the card is
	bool depthBlend;
};
// Gaps of 0.1, 0.5 and over 1 of the blend scale; a wall beyond 0.75 of the
// dest-alpha depth range, where DepthFeathering turns feathering off; and the
// control.
const SoftParticleCase kCases[] = {
    { "gap_5", 100.0f, 5.0f, true },
    { "gap_25", 100.0f, 25.0f, true },
    { "gap_80", 100.0f, 80.0f, true },
    { "far_wall", 300.0f, 5.0f, true },
    { "control", 100.0f, 25.0f, false },
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

IMaterial *SolidMaterial( const char *textureName, const char *materialName, const char *shader,
    const unsigned char color[4], CSolidRegenerator *pRegenerator, bool depthBlend )
{
	ITexture *pTexture = g_pMaterialSystem->CreateProceduralTexture( textureName,
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	if ( !pTexture )
		return nullptr;
	for ( int k = 0; k < 4; ++k )
		pRegenerator->m_Color[k] = color[k];
	pTexture->SetTextureRegenerator( pRegenerator );
	pTexture->Download();
	KeyValues *pKeys = new KeyValues( shader );
	pKeys->SetString( "$basetexture", textureName );
	if ( !V_stricmp( shader, "SpriteCard" ) )
	{
		pKeys->SetInt( "$depthblend", depthBlend ? 1 : 0 );
		pKeys->SetFloat( "$depthblendscale", kDepthBlendScale );
	}
	IMaterial *pMaterial = g_pMaterialSystem->CreateMaterial( materialName, pKeys );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
		return nullptr;
	pMaterial->IncrementReferenceCount();
	return pMaterial;
}

void DrawWall( IMaterial *pWall, float distance )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->Bind( pWall );
	IMesh *pMesh = pRenderContext->GetDynamicMesh();
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
	const float corners[4][2] = { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };
	for ( int i = 0; i < 4; ++i )
	{
		meshBuilder.Position3f(
		    corners[i][0] * kWallHalfSize, corners[i][1] * kWallHalfSize, -distance );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0, 0.5f, 0.5f );
		meshBuilder.AdvanceVertex();
	}
	meshBuilder.End();
	pMesh->Draw();
}

// C_OP_RenderSprites::RenderSpriteCard for one particle, not instanced: four
// vertices at the particle's position, the corner id in texcoord 3.
void DrawCard( IMaterial *pCard, float distance )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->Bind( pCard );
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true );
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, 4, 6 );
	const float cornerIds[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for ( int i = 0; i < 4; ++i )
	{
		meshBuilder.Position3f( 0.0f, 0.0f, -distance );
		meshBuilder.Color4ub( kCardColor[0], kCardColor[1], kCardColor[2], kCardColor[3] );
		meshBuilder.TexCoord4f( 0, 0.0f, 0.0f, 1.0f, 1.0f );
		meshBuilder.TexCoord4f( 1, 0.0f, 0.0f, 1.0f, 1.0f );
		meshBuilder.TexCoord4f( 2, 0.0f, 0.0f, kCardRadius, 0.0f ); // blend, rot, radius, yaw
		meshBuilder.TexCoord2f( 3, cornerIds[i][0], cornerIds[i][1] );
		meshBuilder.TexCoord4f( 4, 0.0f, 0.0f, 1.0f, 1.0f );
		meshBuilder.AdvanceVertex();
	}
	const unsigned short indices[6] = { 0, 1, 2, 0, 2, 3 };
	for ( unsigned short index : indices )
		meshBuilder.FastIndex( index );
	meshBuilder.End();
	pMesh->Draw();
}

} // namespace

bool RunSoftParticleCases( FILE *out, void ( *writeClearProbe )( FILE * ) )
{
	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"softparticle\"," );
	writeClearProbe( out );

	// The engine's full-frame depth copy (matsys_interface.cpp: _rt_FullFrameDepth
	// is an alias of _rt_PowerOfTwoFB), cached as the standard depth texture when
	// the allocation ends.
	g_pMaterialSystem->AddTextureAlias( "_rt_FullFrameDepth", "_rt_PowerOfTwoFB" );
	g_pMaterialSystem->BeginRenderTargetAllocation();
	ITexture *pDepthCopy = g_pMaterialSystem->CreateNamedRenderTargetTextureEx2( "_rt_PowerOfTwoFB",
	    1024, 1024, RT_SIZE_DEFAULT, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SHARED,
	    TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, CREATERENDERTARGETFLAGS_HDR );
	g_pMaterialSystem->EndRenderTargetAllocation();
	if ( !pDepthCopy || pDepthCopy->IsError() )
	{
		Warning( "softparticle conformance: cannot create _rt_PowerOfTwoFB\n" );
		return false;
	}
	pDepthCopy->IncrementReferenceCount();

	static CSolidRegenerator s_Regenerators[3];
	const unsigned char white[4] = { 255, 255, 255, 255 };
	const unsigned char wall[4] = { kWallColor[0], kWallColor[1], kWallColor[2], 255 };
	IMaterial *pWall = SolidMaterial( "conformance/softparticle_wall", "conformance/softparticle_wall",
	    "UnlitGeneric", wall, &s_Regenerators[0], false );
	IMaterial *pCard = SolidMaterial( "conformance/softparticle_card", "conformance/softparticle_card",
	    "SpriteCard", white, &s_Regenerators[1], true );
	IMaterial *pControl = SolidMaterial( "conformance/softparticle_card_hard",
	    "conformance/softparticle_card_hard", "SpriteCard", white, &s_Regenerators[2], false );
	if ( !pWall || !pCard || !pControl )
	{
		Warning( "softparticle conformance: a material did not build\n" );
		return false;
	}
	g_pMaterialSystem->CacheUsedMaterials();

	int width = 0, height = 0;
	g_pMaterialSystem->GetBackBufferDimensions( width, height );
	fprintf( out,
	    "\"frame\":[%d,%d],\"z_near\":%g,\"z_far\":%g,\"depth_blend_scale\":%g,"
	    "\"wall\":[%d,%d,%d],\"card_color\":[%d,%d,%d,%d],\"cases\":[",
	    width, height, kZNear, kZFar, kDepthBlendScale, kWallColor[0], kWallColor[1],
	    kWallColor[2], kCardColor[0], kCardColor[1], kCardColor[2], kCardColor[3] );
	std::vector<unsigned char> rgba( static_cast<size_t>( width ) * height * 4 );
	bool first = true;
	for ( const SoftParticleCase &c : kCases )
	{
		g_pMaterialSystem->BeginFrame( 0 );
		{
			CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
			pRenderContext->Viewport( 0, 0, width, height );
			pRenderContext->SetToneMappingScaleLinear( Vector( 1.0f, 1.0f, 1.0f ) );
			pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
			pRenderContext->ClearBuffers( true, true );
			pRenderContext->MatrixMode( MATERIAL_PROJECTION );
			pRenderContext->LoadIdentity();
			pRenderContext->PerspectiveX(
			    kFovX, static_cast<double>( width ) / height, kZNear, kZFar );
			pRenderContext->MatrixMode( MATERIAL_VIEW );
			pRenderContext->LoadIdentity();
			pRenderContext->MatrixMode( MATERIAL_MODEL );
			pRenderContext->LoadIdentity();
			DrawWall( pWall, c.wallDistance );
			// UpdateFullScreenDepthTexture, after the opaque passes.
			pRenderContext->CopyRenderTargetToTextureEx( pDepthCopy, 0, NULL, NULL );
			pRenderContext->SetFullScreenDepthTextureValidityFlag( true );
			DrawCard( c.depthBlend ? pCard : pControl, c.wallDistance - c.gap );
			pRenderContext->ReadPixels( 0, 0, width, height, rgba.data(), IMAGE_FORMAT_RGBA8888 );
			pRenderContext->SetFullScreenDepthTextureValidityFlag( false );
		}
		g_pMaterialSystem->EndFrame();
		g_pMaterialSystem->SwapBuffers();
		const unsigned char *center =
		    &rgba[( static_cast<size_t>( height / 2 ) * width + width / 2 ) * 4];
		const unsigned char *corner = &rgba[( static_cast<size_t>( height / 8 ) * width + 8 ) * 4];
		fprintf( out,
		    "%s{\"name\":\"%s\",\"wall_distance\":%g,\"gap\":%g,\"depth_blend\":%s,"
		    "\"pixel\":[%d,%d,%d],\"beside\":[%d,%d,%d]}",
		    first ? "" : ",", c.name, c.wallDistance, c.gap, c.depthBlend ? "true" : "false",
		    center[0], center[1], center[2], corner[0], corner[1], corner[2] );
		first = false;
	}
	fprintf( out, "]}\n" );
	pWall->DecrementReferenceCount();
	pCard->DecrementReferenceCount();
	pControl->DecrementReferenceCount();
	pDepthCopy->DecrementReferenceCount();
	return true;
}
