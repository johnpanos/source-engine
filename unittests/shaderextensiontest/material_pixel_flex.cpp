//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "flex" family of the material pixel conformance harness.
//
//          Faces flex through a second vertex stream: studiorender writes each
//          vertex's position and normal deltas and its wrinkle weight into the
//          flex mesh (R_StudioFlexMeshGroup) and binds it to the model's static
//          mesh (IMesh::SetFlexMesh). The vertex shader adds the deltas
//          (common_vs_fxc.h ApplyMorph) and skin_vs20 hands the wrinkle weight
//          to skin_ps20b, whose WRINKLEMAP combo blends the base texture toward
//          $compress (negative weights) or $stretch (positive weights):
//            base.rgb = (1 - |w|) base + saturate(-w) compress + saturate(w) stretch
//
//          Each case draws one quad of a VertexLitGeneric $phong material with
//          $compress and $stretch (solid base, compress and stretch colors, flat
//          normal maps) from a static model mesh, the way studiorender draws a
//          delta-flexed group: build the flex stream, bind it, draw, unbind. The
//          quad's center and the point a position delta moves it to are read.
//          Lighting is the same in every case, so the oracle
//          (tools/quality/material_pixel_conformance.py) holds the pixels to
//          the blend's linearity rather than to a lighting model.
//
//=============================================================================//

#include "material_pixel_flex.h"

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

// Clip space (identity transforms): a quad about the center.
const float kQuadHalf = 0.25f;
const float kQuadZ = 0.5f;
const float kMoveX = 0.4f; // the position delta of the "moved" case

struct SolidTexture
{
	const char *name;
	unsigned char rgba[4];
};
const SolidTexture kTextures[] = {
    { "conformance/flex_base", { 128, 128, 128, 255 } },
    { "conformance/flex_compress", { 40, 70, 200, 255 } },
    { "conformance/flex_stretch", { 200, 70, 40, 255 } },
    { "conformance/flex_normal", { 128, 128, 255, 255 } },
};
const int kTextureCount = sizeof( kTextures ) / sizeof( kTextures[0] );

struct FlexCase
{
	const char *name;
	bool bound;    // a flex stream is bound for the draw
	float wrinkle; // every vertex's wrinkle weight
	float moveX;   // every vertex's position delta in x
};
const FlexCase kCases[] = {
    { "rest", true, 0.0f, 0.0f },
    { "stretch", true, 1.0f, 0.0f },
    { "compress", true, -1.0f, 0.0f },
    { "half_stretch", true, 0.5f, 0.0f },
    { "quarter_compress", true, -0.25f, 0.0f },
    { "moved", true, 0.0f, kMoveX },
    { "unbound", false, 0.0f, 0.0f },
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

void ReadAt( const std::vector<unsigned char> &rgba, int width, int height, float fx, float fy,
    unsigned char rgb[3] )
{
	const int x = static_cast<int>( fx * width );
	const int y = static_cast<int>( fy * height );
	const unsigned char *texel = &rgba[( static_cast<size_t>( y ) * width + x ) * 4];
	for ( int k = 0; k < 3; ++k )
		rgb[k] = texel[k];
}

} // namespace

bool RunFlexCases( FILE *out, void ( *writeClearProbe )( FILE * ) )
{
	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"flex\"," );
	writeClearProbe( out );

	static CSolidRegenerator s_Regenerators[kTextureCount];
	for ( int t = 0; t < kTextureCount; ++t )
	{
		ITexture *pTexture = g_pMaterialSystem->CreateProceduralTexture( kTextures[t].name,
		    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
		    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
		        TEXTUREFLAGS_SINGLECOPY );
		if ( !pTexture )
			return false;
		for ( int k = 0; k < 4; ++k )
			s_Regenerators[t].m_Color[k] = kTextures[t].rgba[k];
		pTexture->SetTextureRegenerator( &s_Regenerators[t] );
		pTexture->Download();
	}
	KeyValues *pKeys = new KeyValues( "VertexLitGeneric" );
	pKeys->SetInt( "$model", 1 );
	pKeys->SetInt( "$phong", 1 );
	pKeys->SetFloat( "$phongboost", 0.0f );
	pKeys->SetString( "$basetexture", "conformance/flex_base" );
	pKeys->SetString( "$compress", "conformance/flex_compress" );
	pKeys->SetString( "$stretch", "conformance/flex_stretch" );
	pKeys->SetString( "$bumpmap", "conformance/flex_normal" );
	pKeys->SetString( "$bumpcompress", "conformance/flex_normal" );
	pKeys->SetString( "$bumpstretch", "conformance/flex_normal" );
	IMaterial *pMaterial = g_pMaterialSystem->CreateMaterial( "conformance/flex_face", pKeys );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
	{
		Warning( "flex conformance: the material did not build\n" );
		return false;
	}
	pMaterial->IncrementReferenceCount();
	g_pMaterialSystem->CacheUsedMaterials();
	if ( pMaterial->GetVertexFormat() == 0 )
	{
		Warning( "flex conformance: the material has no vertex format after precache\n" );
		return false;
	}

	// The model's static mesh, as studiorender builds one (studiorendercontext.cpp
	// R_StudioCreateStaticMeshes): the material's format, uncompressed, with two
	// bone weights and indices.
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	const VertexFormat_t format = ( pMaterial->GetVertexFormat() & ~VERTEX_FORMAT_COMPRESSED ) |
	                              VERTEX_BONEWEIGHT( 2 ) | VERTEX_BONE_INDEX;
	IMesh *pStatic = pRenderContext->CreateStaticMesh(
	    format, TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS, pMaterial );
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pStatic, MATERIAL_TRIANGLES, 4, 6 );
	const float corners[4][2] = {
	    { -kQuadHalf, kQuadHalf }, { kQuadHalf, kQuadHalf }, { kQuadHalf, -kQuadHalf },
	    { -kQuadHalf, -kQuadHalf } };
	const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	const float tangent[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	for ( int i = 0; i < 4; ++i )
	{
		meshBuilder.Position3f( corners[i][0], corners[i][1], kQuadZ );
		meshBuilder.Normal3f( 0.0f, 0.0f, -1.0f );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.TexCoord2f( 0, uv[i][0], uv[i][1] );
		meshBuilder.UserData( tangent );
		meshBuilder.BoneWeight( 0, 1.0f );
		meshBuilder.BoneWeight( 1, 0.0f );
		for ( int b = 0; b < 4; ++b )
			meshBuilder.BoneMatrix( b, 0 );
		meshBuilder.AdvanceVertex();
	}
	const unsigned short indices[6] = { 0, 1, 2, 0, 2, 3 };
	for ( unsigned short index : indices )
		meshBuilder.FastIndex( index );
	meshBuilder.End();

	int width = 0, height = 0;
	g_pMaterialSystem->GetBackBufferDimensions( width, height );
	fprintf( out,
	    "\"frame\":[%d,%d],\"move_x\":%g,\"base\":[%d,%d,%d],\"compress\":[%d,%d,%d],"
	    "\"stretch\":[%d,%d,%d],\"cases\":[",
	    width, height, kMoveX, kTextures[0].rgba[0], kTextures[0].rgba[1], kTextures[0].rgba[2],
	    kTextures[1].rgba[0], kTextures[1].rgba[1], kTextures[1].rgba[2], kTextures[2].rgba[0],
	    kTextures[2].rgba[1], kTextures[2].rgba[2] );
	std::vector<unsigned char> rgba( static_cast<size_t>( width ) * height * 4 );
	bool first = true;
	for ( const FlexCase &c : kCases )
	{
		g_pMaterialSystem->BeginFrame( 0 );
		{
			CMatRenderContextPtr pContext( g_pMaterialSystem );
			pContext->Viewport( 0, 0, width, height );
			pContext->SetToneMappingScaleLinear( Vector( 1.0f, 1.0f, 1.0f ) );
			pContext->ClearColor4ub( 255, 0, 255, 255 );
			pContext->ClearBuffers( true, true );
			for ( int mode = MATERIAL_VIEW; mode <= MATERIAL_PROJECTION; ++mode )
			{
				pContext->MatrixMode( static_cast<MaterialMatrixMode_t>( mode ) );
				pContext->LoadIdentity();
			}
			pContext->MatrixMode( MATERIAL_MODEL );
			pContext->LoadIdentity();
			// studiorender's lighting state: a neutral ambient cube, no local lights.
			Vector4D cube[6];
			for ( int f = 0; f < 6; ++f )
				cube[f].Init( 0.6f, 0.6f, 0.6f, 0.0f );
			pContext->SetAmbientLightCube( cube );
			pContext->SetLightingOrigin( Vector( 0.0f, 0.0f, kQuadZ ) );
			pContext->DisableAllLocalLights();
			// R_StudioDrawStaticMesh: bones, then (delta flexed) the flex stream.
			pContext->SetNumBoneWeights( 2 );
			const matrix3x4_t identity( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 );
			pContext->LoadBoneMatrix( 0, identity );
			if ( c.bound )
			{
				IMesh *pFlex = pContext->GetFlexMesh();
				int flexOffset = 0;
				CMeshBuilder flexBuilder;
				flexBuilder.Begin( pFlex, MATERIAL_HETEROGENOUS, 4, 0, &flexOffset );
				for ( int i = 0; i < 4; ++i )
				{
					flexBuilder.Position3f( c.moveX, 0.0f, 0.0f );
					flexBuilder.NormalDelta3f( 0.0f, 0.0f, 0.0f );
					flexBuilder.Wrinkle1f( c.wrinkle );
					flexBuilder.AdvanceVertex();
				}
				flexBuilder.End( false, false );
				pStatic->SetFlexMesh( pFlex, flexOffset );
			}
			pContext->Bind( pMaterial );
			pStatic->Draw();
			if ( c.bound )
				pStatic->DisableFlexMesh();
			pContext->SetNumBoneWeights( 0 );
			pContext->ReadPixels( 0, 0, width, height, rgba.data(), IMAGE_FORMAT_RGBA8888 );
		}
		g_pMaterialSystem->EndFrame();
		g_pMaterialSystem->SwapBuffers();
		unsigned char center[3], moved[3];
		ReadAt( rgba, width, height, 0.5f, 0.5f, center );
		ReadAt( rgba, width, height, 0.5f + 0.5f * kMoveX, 0.5f, moved );
		fprintf( out,
		    "%s{\"name\":\"%s\",\"bound\":%s,\"wrinkle\":%g,\"move_x\":%g,"
		    "\"center\":[%d,%d,%d],\"moved\":[%d,%d,%d]}",
		    first ? "" : ",", c.name, c.bound ? "true" : "false", c.wrinkle, c.moveX, center[0],
		    center[1], center[2], moved[0], moved[1], moved[2] );
		first = false;
	}
	fprintf( out, "]}\n" );
	pRenderContext->DestroyStaticMesh( pStatic );
	pMaterial->DecrementReferenceCount();
	return true;
}
