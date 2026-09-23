//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "modellight" family of the material pixel conformance harness.
//
//          It lights VertexLitGeneric models the way studiorender does: an
//          ambient cube (SetAmbientLightCube), up to four local lights
//          (SetLight: directional, point and spot), and a static-prop color mesh
//          (IMesh::SetColorMesh) for baked per-vertex lighting. The material
//          system selects vertexlit_and_unlit_generic's DYNAMIC_LIGHT and
//          STATIC_LIGHT combos from that state, as in the game.
//
//          Each case draws a 3x3 grid of screen-facing quads with identity
//          transforms, so a vertex's world position is its clip-space position.
//          The quads' normal attributes point in nine directions, independent of
//          their geometry, so one frame samples the lighting over many normals.
//          Every frame is written to a raw RGB file next to the report; the
//          oracle (tools/quality/material_pixel_modellight.py) evaluates the
//          vertex lighting of common_vs_fxc.h independently and holds the frames
//          to it and to the D3D9 reference.
//
//=============================================================================//

#include "material_pixel_modellight.h"

#include "KeyValues.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/materialsystem_config.h"
#include "mathlib/vector4d.h"
#include "pixelwriter.h"
#include "tier1/strtools.h"
#include "tier2/tier2.h"
#include "vtf/vtf.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{

// The quads' depth in clip space (identity transforms).
const float kQuadZ = 0.5f;
// Each quad covers its grid cell minus this margin (in clip-space units).
const float kCellMargin = 0.06f;
const int kGrid = 3;
// The normal attribute of each quad, in grid order (row-major from the top left).
const float kQuadNormals[kGrid * kGrid][3] = {
    { 1, 0, 0 },
    { -1, 0, 0 },
    { 0, 1, 0 },
    { 0, -1, 0 },
    { 0, 0, 1 },
    { 0, 0, -1 },
    { 0.57735027f, 0.57735027f, 0.57735027f },
    { -0.87287156f, 0.43643578f, 0.21821789f },
    { 0.30151134f, -0.80403025f, 0.51256929f },
};

struct ModelLight
{
	LightType_t type;
	float color[3];
	float position[3];
	float direction[3];
	float attenuation[3]; // constant, linear, quadratic
	float theta;          // spot inner cone angle (radians)
	float phi;            // spot outer cone angle (radians)
	float falloff;        // spot exponent
};

// How a case places the model: identity, a rigid MODEL matrix, or hardware
// skinning through bone matrices (kTransform for both).
enum ModelPlacement
{
	kIdentity,
	kRigid,
	kSkinned
};

// The materials the cases draw with (kMaterials).
enum ModelMaterial
{
	kLambert,
	kHalfLambert,
	kPhong,
	kPhongLightWarp,
	kPhongSelfIllum,
	kPhongConstant,
	kPhongBaseAlphaMask,
	kSelfIllum,
	kMaterialCount
};

struct ModelLightCase
{
	const char *name;
	ModelMaterial material;
	float cube[6][3]; // +x, -x, +y, -y, +z, -z
	int numLights;
	ModelLight lights[4];
	bool staticColor;
	ModelPlacement placement;
};

// A half turn about z with z scaled by 0.8: it maps the quad grid onto itself
// (each quad onto its mirror cell, still clockwise on screen) while turning the
// normals, and the scale makes the shader's normalization matter.
const matrix3x4_t kTransform( -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0.8f, 0 );
// Skinned vertices blend three bones (weights w0, w1, 1 - w0 - w1) that all hold
// kTransform, so the placement is kTransform's whatever the weights.
const float kSkinWeights[2] = { 0.5f, 0.3f };

// A directional light's position, as LightDesc_t::RecalculateDerivedValues sets
// it for studiorender: far along its direction. The vertex shader's attenuation
// reads the position of every light type, so it is not left at the origin.
const float kDirectionalDistance = 2.0e6f;
// Linear light values, kept below saturation where the cases measure a sum.
const ModelLight kDirectional = { MATERIAL_LIGHT_DIRECTIONAL, { 0.7f, 0.55f, 0.4f }, { 0, 0, 0 },
    { -0.48f, -0.6f, 0.64f }, { 1, 0, 0 }, 0, 0, 0 };
const ModelLight kDirectional2 = { MATERIAL_LIGHT_DIRECTIONAL, { 0.1f, 0.2f, 0.35f }, { 0, 0, 0 },
    { 0.8f, 0.0f, -0.6f }, { 1, 0, 0 }, 0, 0, 0 };
const ModelLight kPoint = { MATERIAL_LIGHT_POINT, { 1.6f, 1.2f, 0.9f }, { -0.3f, 0.4f, -0.9f },
    { 0, 0, 1 }, { 0.2f, 0.3f, 0.5f }, 0, 0, 0 };
const ModelLight kSpot = { MATERIAL_LIGHT_SPOT, { 1.1f, 0.9f, 1.3f }, { 0.25f, -0.2f, -1.2f },
    { -0.15f, 0.1f, 0.98f }, { 0.4f, 0.4f, 0.0f }, 0.7f, 1.6f, 2.0f };

const ModelLightCase kCases[] = {
    { "ambient_cube", kLambert,
        { { 0.6f, 0.08f, 0.05f }, { 0.07f, 0.5f, 0.06f }, { 0.05f, 0.08f, 0.55f },
            { 0.4f, 0.35f, 0.04f }, { 0.05f, 0.4f, 0.45f }, { 0.3f, 0.12f, 0.45f } },
        0, {}, false },
    { "directional", kLambert, {}, 1, { kDirectional }, false },
    { "point", kLambert, {}, 1, { kPoint }, false },
    { "spot", kLambert, {}, 1, { kSpot }, false },
    { "four_lights", kLambert,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        4, { kSpot, kDirectional, kPoint, kDirectional2 }, false },
    { "half_lambert", kHalfLambert, {}, 1, { kDirectional }, false },
    { "static_vertex", kLambert, {}, 0, {}, true },
    { "static_and_dynamic", kLambert,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        1, { kDirectional2 }, true },
    { "no_light", kLambert, {}, 0, {}, false },
    { "model_transform", kLambert,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        2, { kDirectional, kPoint }, false, kRigid },
    { "skinned", kLambert,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        2, { kDirectional, kPoint }, false, kSkinned },
    // VertexLitGeneric with $phong: skin_vs20 / skin_ps20b, lit per pixel.
    { "phong", kPhong,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        2, { kDirectional, kPoint }, false },
    { "phong_four_lights", kPhong, {}, 4, { kSpot, kDirectional, kPoint, kDirectional2 }, false },
    { "phong_lightwarp", kPhongLightWarp, {}, 2, { kDirectional, kPoint }, false },
    { "phong_selfillum", kPhongSelfIllum, {}, 2, { kDirectional, kPoint }, false },
    // No rim-light case: the shipped skin_ps20b.vcs has no RIMLIGHT combo for
    // ps_2_b (D3D9 draws those materials without a pixel shader), and Portal's
    // content has no $rimlight material.
    { "phong_constant", kPhongConstant,
        { { 0.2f, 0.1f, 0.05f }, { 0.05f, 0.2f, 0.1f }, { 0.1f, 0.05f, 0.2f },
            { 0.15f, 0.15f, 0.05f }, { 0.05f, 0.15f, 0.15f }, { 0.15f, 0.05f, 0.15f } },
        2, { kDirectional, kPoint }, false },
    { "phong_basealphamask", kPhongBaseAlphaMask, {}, 2, { kDirectional, kPoint }, false },
    { "phong_skinned", kPhong, {}, 2, { kDirectional, kPoint }, false, kSkinned },
    // VertexLitGeneric $selfillum without $phong: vertexlit_and_unlit_generic's
    // SELFILLUM combo, the vertex lighting blended toward the tint by base alpha.
    { "selfillum", kSelfIllum,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        2, { kDirectional, kPoint }, false },
};

// Procedural textures (sRGB-encoded where the material reads them as sRGB).
struct ProceduralTexture
{
	const char *name;
	int width;
	int height;
	unsigned char rgba[4]; // a solid texel, or with ramp: the value at x = 0
	bool ramp;             // a lightwarp ramp across x (LightWarpTexel)
};
const ProceduralTexture kTextures[] = {
    { "conformance/modellight_base", 4, 4, { 190, 150, 110, 255 }, false },
    // Alpha is the $selfillum mask.
    { "conformance/modellight_base_masked", 4, 4, { 190, 150, 110, 160 }, false },
    // Tangent-space normal ( 0.24, -0.144, 0.96 ), specular mask in alpha.
    { "conformance/modellight_normal", 4, 4, { 158, 109, 250, 200 }, false },
    // $phongexponenttexture: r exponent (1 + 149 r), g albedo tint, a rim mask.
    { "conformance/modellight_exponent", 4, 4, { 60, 100, 0, 180 }, false },
    { "conformance/modellight_lightwarp", 32, 1, { 0, 0, 0, 255 }, true },
};
const int kTextureCount = sizeof( kTextures ) / sizeof( kTextures[0] );

// The lightwarp texel at x of 32: distinct curves per channel, so a sample at
// the wrong coordinate or channel shows.
void LightWarpTexel( int x, unsigned char rgba[4] )
{
	const float t = x / 31.0f;
	rgba[0] = static_cast<unsigned char>( 255.0f * powf( t, 0.7f ) + 0.5f );
	rgba[1] = static_cast<unsigned char>( 255.0f * t + 0.5f );
	rgba[2] = static_cast<unsigned char>( 255.0f * t * t + 0.5f );
	rgba[3] = 255;
}

// The materials, as "$key" "value" pairs after VertexLitGeneric's $model 1.
struct MaterialVariant
{
	const char *name;
	const char *params[12][2];
};
const MaterialVariant kMaterials[kMaterialCount] = {
    { "conformance/modellight", { { "$basetexture", "conformance/modellight_base" } } },
    { "conformance/modellight_halflambert",
        { { "$basetexture", "conformance/modellight_base" }, { "$halflambert", "1" } } },
    { "conformance/modellight_phong",
        { { "$basetexture", "conformance/modellight_base" },
            { "$bumpmap", "conformance/modellight_normal" }, { "$phong", "1" },
            { "$phongexponenttexture", "conformance/modellight_exponent" }, { "$phongboost", "2" },
            { "$phongfresnelranges", "[0.2 0.6 1]" } } },
    { "conformance/modellight_phong_lightwarp",
        { { "$basetexture", "conformance/modellight_base" },
            { "$bumpmap", "conformance/modellight_normal" }, { "$phong", "1" },
            { "$phongexponenttexture", "conformance/modellight_exponent" }, { "$phongboost", "2" },
            { "$phongfresnelranges", "[0.2 0.6 1]" },
            { "$lightwarptexture", "conformance/modellight_lightwarp" } } },
    { "conformance/modellight_phong_selfillum",
        { { "$basetexture", "conformance/modellight_base_masked" },
            { "$bumpmap", "conformance/modellight_normal" }, { "$phong", "1" },
            { "$phongexponenttexture", "conformance/modellight_exponent" }, { "$phongboost", "2" },
            { "$selfillum", "1" }, { "$selfillumtint", "[0.9 0.6 0.3]" } } },
    // A constant exponent and tint override the exponent map's.
    { "conformance/modellight_phong_constant",
        { { "$basetexture", "conformance/modellight_base" },
            { "$bumpmap", "conformance/modellight_normal" }, { "$phong", "1" },
            { "$phongexponenttexture", "conformance/modellight_exponent" },
            { "$phongexponent", "24" }, { "$phongtint", "[1 0.5 0.25]" }, { "$phongboost", "1.5" },
            { "$phongfresnelranges", "[0.5 0.75 1]" } } },
    // The phong mask in the base alpha, the normal map's normal ignored. (Without
    // a bump map this is FASTPATH_NOBUMP, which the shipped skin_ps20b.vcs lacks.)
    { "conformance/modellight_phong_basealphamask",
        { { "$basetexture", "conformance/modellight_base_masked" },
            { "$bumpmap", "conformance/modellight_normal" }, { "$phong", "1" },
            { "$phongexponenttexture", "conformance/modellight_exponent" },
            { "$basemapalphaphongmask", "1" }, { "$phongboost", "2" },
            { "$phongfresnelranges", "[0.2 0.6 1]" } } },
    // Alpha 160/255 is the mask; the tint differs per channel.
    { "conformance/modellight_selfillum",
        { { "$basetexture", "conformance/modellight_base_masked" }, { "$selfillum", "1" },
            { "$selfillumtint", "[0.9 0.6 0.3]" } } },
};

// The static-prop color of a quad's corner (sRGB-like bytes, as vrad bakes them
// at half intensity: the shader doubles them, cOverbright).
void StaticColor( int quad, int corner, unsigned char rgb[3] )
{
	rgb[0] = static_cast<unsigned char>( 20 + 23 * quad + 11 * corner );
	rgb[1] = static_cast<unsigned char>( 120 - 9 * quad + 17 * corner );
	rgb[2] = static_cast<unsigned char>( 60 + 13 * ( ( quad + corner ) % 5 ) );
}

// Clip-space corners of a quad, clockwise on screen from the top left.
void QuadCorners( int quad, float corners[4][2] )
{
	const float cell = 2.0f / kGrid;
	const int col = quad % kGrid;
	const int row = quad / kGrid;
	const float x0 = -1.0f + col * cell + kCellMargin;
	const float x1 = -1.0f + ( col + 1 ) * cell - kCellMargin;
	const float y1 = 1.0f - row * cell - kCellMargin;
	const float y0 = 1.0f - ( row + 1 ) * cell + kCellMargin;
	const float c[4][2] = { { x0, y1 }, { x1, y1 }, { x1, y0 }, { x0, y0 } };
	memcpy( corners, c, sizeof( c ) );
}

class CProceduralRegenerator : public ITextureRegenerator
{
public:
	const ProceduralTexture *m_pSpec = nullptr;

	void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTF, Rect_t *pRect ) override
	{
		// The backend picks the storage format, so write through the pixel writer.
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
				{
					unsigned char rgba[4];
					if ( m_pSpec->ramp )
						LightWarpTexel( x, rgba );
					else
						memcpy( rgba, m_pSpec->rgba, sizeof( rgba ) );
					writer.WritePixel( rgba[0], rgba[1], rgba[2], rgba[3] );
				}
			}
		}
	}
	void Release() override {}
};

// A quad's tangent S, perpendicular to its normal; the binormal sign alternates
// between quads (it is the vertex's TANGENT w).
void QuadTangent( int quad, float tangent[4] )
{
	const float *n = kQuadNormals[quad];
	const float helper[3] = {
	    0.0f, fabsf( n[1] ) > 0.9f ? 0.0f : 1.0f, fabsf( n[1] ) > 0.9f ? 1.0f : 0.0f };
	// cross( helper, n )
	float t[3] = { helper[1] * n[2] - helper[2] * n[1], helper[2] * n[0] - helper[0] * n[2],
	    helper[0] * n[1] - helper[1] * n[0] };
	const float length = sqrtf( t[0] * t[0] + t[1] * t[1] + t[2] * t[2] );
	for ( int k = 0; k < 3; ++k )
		tangent[k] = t[k] / length;
	tangent[3] = ( quad % 2 ) ? -1.0f : 1.0f;
}

class CModelLightScene
{
public:
	bool Init();
	void Shutdown();
	bool RenderCase( const ModelLightCase &c, FILE *out, const char *framePath, bool first );

	int m_Width = 0;
	int m_Height = 0;
	float m_ToneScale = 1.0f;

private:
	void ApplyLighting( const ModelLightCase &c );

	ITexture *m_pTextures[kTextureCount] = {};
	IMaterial *m_pMaterials[kMaterialCount] = {};
	IMesh *m_pMesh[kMaterialCount] = {};
	IMesh *m_pColorMesh = nullptr;
};

bool CModelLightScene::Init()
{
	g_pMaterialSystem->GetBackBufferDimensions( m_Width, m_Height );
	// The textures live until the material system shuts down, and call their
	// regenerators then, so the regenerators outlive this scene.
	static CProceduralRegenerator s_Regenerators[kTextureCount];
	for ( int t = 0; t < kTextureCount; ++t )
	{
		const ProceduralTexture &spec = kTextures[t];
		m_pTextures[t] = g_pMaterialSystem->CreateProceduralTexture( spec.name, TEXTURE_GROUP_OTHER,
		    spec.width, spec.height, IMAGE_FORMAT_RGBA8888,
		    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
		        TEXTUREFLAGS_SINGLECOPY | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT );
		if ( !m_pTextures[t] )
			return false;
		s_Regenerators[t].m_pSpec = &spec;
		m_pTextures[t]->SetTextureRegenerator( &s_Regenerators[t] );
		m_pTextures[t]->Download();
	}

	for ( int m = 0; m < kMaterialCount; ++m )
	{
		KeyValues *pKeys = new KeyValues( "VertexLitGeneric" );
		pKeys->SetInt( "$model", 1 );
		for ( const auto &param : kMaterials[m].params )
		{
			if ( param[0] )
				pKeys->SetString( param[0], param[1] );
		}
		m_pMaterials[m] = g_pMaterialSystem->CreateMaterial( kMaterials[m].name, pKeys );
		if ( !m_pMaterials[m] || m_pMaterials[m]->IsErrorMaterial() )
			return false;
		m_pMaterials[m]->IncrementReferenceCount();
	}
	g_pMaterialSystem->CacheUsedMaterials();
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	for ( int m = 0; m < kMaterialCount; ++m )
	{
		if ( m_pMaterials[m]->GetVertexFormat() == 0 )
		{
			Warning( "modellight conformance: %s has no vertex format after precache\n",
			    m_pMaterials[m]->GetName() );
			return false;
		}
		// The model mesh format: the material's, uncompressed, plus two bone
		// weights and the bone indices, as studiorender builds static model meshes
		// (studiorendercontext.cpp R_StudioCreateStaticMeshes).
		const VertexFormat_t format =
		    ( m_pMaterials[m]->GetVertexFormat() & ~VERTEX_FORMAT_COMPRESSED ) |
		    VERTEX_BONEWEIGHT( 2 ) | VERTEX_BONE_INDEX;
		m_pMesh[m] = pRenderContext->CreateStaticMesh(
		    format, TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS, m_pMaterials[m] );
		CMeshBuilder meshBuilder;
		meshBuilder.Begin( m_pMesh[m], MATERIAL_TRIANGLES, 4 * kGrid * kGrid, 6 * kGrid * kGrid );
		for ( int q = 0; q < kGrid * kGrid; ++q )
		{
			float corners[4][2];
			QuadCorners( q, corners );
			float tangent[4];
			QuadTangent( q, tangent );
			const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
			for ( int v = 0; v < 4; ++v )
			{
				meshBuilder.Position3f( corners[v][0], corners[v][1], kQuadZ );
				meshBuilder.Normal3fv( kQuadNormals[q] );
				meshBuilder.Color4ub( 255, 255, 255, 255 );
				meshBuilder.TexCoord2f( 0, uv[v][0], uv[v][1] );
				meshBuilder.UserData( tangent );
				meshBuilder.BoneWeight( 0, kSkinWeights[0] );
				meshBuilder.BoneWeight( 1, kSkinWeights[1] );
				for ( int b = 0; b < 3; ++b )
					meshBuilder.BoneMatrix( b, b );
				meshBuilder.BoneMatrix( 3, 0 );
				meshBuilder.AdvanceVertex();
			}
		}
		// Clockwise on screen, the front face.
		for ( int q = 0; q < kGrid * kGrid; ++q )
		{
			const unsigned short base = static_cast<unsigned short>( 4 * q );
			const unsigned short indices[6] = { 0, 1, 2, 0, 2, 3 };
			for ( unsigned short index : indices )
				meshBuilder.FastIndex( static_cast<unsigned short>( base + index ) );
		}
		meshBuilder.End();
	}

	// The static-prop color stream (engine/l_studio.cpp CColorMeshData): one
	// specular color per vertex of the model mesh.
	m_pColorMesh = pRenderContext->CreateStaticMesh(
	    VERTEX_SPECULAR, TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR );
	CMeshBuilder colorBuilder;
	colorBuilder.Begin( m_pColorMesh, MATERIAL_HETEROGENOUS, 4 * kGrid * kGrid, 0 );
	for ( int q = 0; q < kGrid * kGrid; ++q )
	{
		for ( int v = 0; v < 4; ++v )
		{
			unsigned char rgb[3];
			StaticColor( q, v, rgb );
			colorBuilder.Specular3ub( rgb[0], rgb[1], rgb[2] );
			colorBuilder.AdvanceVertex();
		}
	}
	colorBuilder.End();
	return true;
}

void CModelLightScene::Shutdown()
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	for ( int m = 0; m < kMaterialCount; ++m )
	{
		if ( m_pMesh[m] )
			pRenderContext->DestroyStaticMesh( m_pMesh[m] );
		if ( m_pMaterials[m] )
			m_pMaterials[m]->DecrementReferenceCount();
	}
	if ( m_pColorMesh )
		pRenderContext->DestroyStaticMesh( m_pColorMesh );
}

// studiorender's order (CStudioRender::R_StudioSetupLighting / SetLightingState):
// the ambient cube, then the local lights, the rest disabled.
void CModelLightScene::ApplyLighting( const ModelLightCase &c )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	Vector4D cube[6];
	for ( int f = 0; f < 6; ++f )
		cube[f].Init( c.cube[f][0], c.cube[f][1], c.cube[f][2], 0.0f );
	pRenderContext->SetAmbientLightCube( cube );
	// studiorender lights a model about its origin; the per-pixel shaders place
	// directional lights far from it along their direction.
	pRenderContext->SetLightingOrigin( Vector( 0.0f, 0.0f, kQuadZ ) );
	pRenderContext->DisableAllLocalLights();
	for ( int i = 0; i < c.numLights; ++i )
	{
		const ModelLight &l = c.lights[i];
		LightDesc_t desc;
		desc.m_Type = l.type;
		desc.m_Color.Init( l.color[0], l.color[1], l.color[2] );
		desc.m_Direction.Init( l.direction[0], l.direction[1], l.direction[2] );
		if ( l.type == MATERIAL_LIGHT_DIRECTIONAL )
			desc.m_Position = desc.m_Direction * kDirectionalDistance;
		else
			desc.m_Position.Init( l.position[0], l.position[1], l.position[2] );
		desc.m_Range = 0.0f;
		desc.m_Falloff = l.falloff;
		desc.m_Attenuation0 = l.attenuation[0];
		desc.m_Attenuation1 = l.attenuation[1];
		desc.m_Attenuation2 = l.attenuation[2];
		desc.m_Theta = l.theta;
		desc.m_Phi = l.phi;
		// The derived fields (m_ThetaDot, ...) serve software lighting only;
		// the shader API reads the fields above.
		desc.m_Flags = 0;
		pRenderContext->SetLight( i, desc );
	}
}

bool CModelLightScene::RenderCase(
    const ModelLightCase &c, FILE *out, const char *framePath, bool first )
{
	const int m = c.material;
	std::vector<unsigned char> rgba( static_cast<size_t>( m_Width ) * m_Height * 4 );
	g_pMaterialSystem->BeginFrame( 0 );
	{
		CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
		pRenderContext->Viewport( 0, 0, m_Width, m_Height );
		pRenderContext->SetToneMappingScaleLinear(
		    Vector( m_ToneScale, m_ToneScale, m_ToneScale ) );
		pRenderContext->ClearColor4ub( 255, 0, 255, 255 );
		pRenderContext->ClearBuffers( true, true );
		for ( int mode = MATERIAL_VIEW; mode <= MATERIAL_PROJECTION; ++mode )
		{
			pRenderContext->MatrixMode( static_cast<MaterialMatrixMode_t>( mode ) );
			pRenderContext->LoadIdentity();
		}
		pRenderContext->MatrixMode( MATERIAL_MODEL );
		if ( c.placement == kRigid )
			pRenderContext->LoadMatrix( kTransform );
		else
			pRenderContext->LoadIdentity();
		ApplyLighting( c );
		// studiorender's order (R_StudioDrawGroupHWSkin): bone count, bones, draw.
		if ( c.placement == kSkinned )
		{
			pRenderContext->SetNumBoneWeights( 2 );
			for ( int b = 0; b < 3; ++b )
				pRenderContext->LoadBoneMatrix( b, kTransform );
		}
		pRenderContext->Bind( m_pMaterials[m] );
		m_pMesh[m]->SetColorMesh( c.staticColor ? m_pColorMesh : nullptr, 0 );
		m_pMesh[m]->Draw();
		m_pMesh[m]->SetColorMesh( nullptr, 0 );
		pRenderContext->SetNumBoneWeights( 0 );
		pRenderContext->MatrixMode( MATERIAL_MODEL );
		pRenderContext->LoadIdentity();
		pRenderContext->ReadPixels( 0, 0, m_Width, m_Height, rgba.data(), IMAGE_FORMAT_RGBA8888 );
	}
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();

	FILE *frame = fopen( framePath, "wb" );
	if ( !frame )
	{
		Warning( "modellight conformance: cannot write %s\n", framePath );
		return false;
	}
	for ( size_t i = 0; i < rgba.size(); i += 4 )
		fwrite( &rgba[i], 1, 3, frame );
	fclose( frame );

	fprintf( out,
	    "%s{\"name\":\"%s\",\"material\":\"%s\",\"half_lambert\":%s,"
	    "\"static_color\":%s,",
	    first ? "" : ",", c.name, kMaterials[c.material].name,
	    c.material == kHalfLambert ? "true" : "false", c.staticColor ? "true" : "false" );
	// The model-to-world transform, rows of a 3x4 column-vector matrix.
	const matrix3x4_t &placement =
	    c.placement == kIdentity ? matrix3x4_t( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 ) : kTransform;
	fprintf( out, "\"placement\":\"%s\",\"model_matrix\":[",
	    c.placement == kSkinned ? "skinned"
	    : c.placement == kRigid ? "rigid"
	                            : "identity" );
	for ( int r = 0; r < 3; ++r )
		fprintf( out, "%s[%.9g,%.9g,%.9g,%.9g]", r ? "," : "", placement[r][0], placement[r][1],
		    placement[r][2], placement[r][3] );
	fprintf( out, "],\"cube\":[" );
	for ( int f = 0; f < 6; ++f )
		fprintf(
		    out, "%s[%.9g,%.9g,%.9g]", f ? "," : "", c.cube[f][0], c.cube[f][1], c.cube[f][2] );
	fprintf( out, "],\"lights\":[" );
	for ( int i = 0; i < c.numLights; ++i )
	{
		const ModelLight &l = c.lights[i];
		const bool directional = l.type == MATERIAL_LIGHT_DIRECTIONAL;
		float position[3];
		for ( int k = 0; k < 3; ++k )
			position[k] = directional ? l.direction[k] * kDirectionalDistance : l.position[k];
		const char *type = l.type == MATERIAL_LIGHT_DIRECTIONAL ? "directional"
		                   : l.type == MATERIAL_LIGHT_SPOT      ? "spot"
		                                                        : "point";
		fprintf( out,
		    "%s{\"type\":\"%s\",\"color\":[%.9g,%.9g,%.9g],\"position\":[%.9g,%.9g,%.9g],"
		    "\"direction\":[%.9g,%.9g,%.9g],\"attenuation\":[%.9g,%.9g,%.9g],"
		    "\"theta\":%.9g,\"phi\":%.9g,\"falloff\":%.9g}",
		    i ? "," : "", type, l.color[0], l.color[1], l.color[2], position[0], position[1],
		    position[2], l.direction[0], l.direction[1], l.direction[2], l.attenuation[0],
		    l.attenuation[1], l.attenuation[2], l.theta, l.phi, l.falloff );
	}
	fprintf( out, "],\"frame_file\":\"%s\"}", V_UnqualifiedFileName( framePath ) );
	return true;
}

} // namespace

bool RunModelLightCases(
    FILE *out, const char *outPath, void ( *writeClearProbe )( FILE * ), float toneScale )
{
	// The clear probe draws the first frame, which applies the pinned video
	// config: on D3D9 that resets the device, which fails while static meshes
	// (losable resources) are alive. The scene's meshes are created after it.
	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"modellight\"," );
	writeClearProbe( out );
	CModelLightScene scene;
	scene.m_ToneScale = toneScale;
	if ( !scene.Init() )
	{
		scene.Shutdown();
		return false;
	}
	fprintf( out,
	    "\"frame\":[%d,%d],\"tone_scale\":%.9g,\"aa_samples\":%d,\"max_lights\":%d,"
	    "\"static_control_flow\":%s,\"base_color\":[%d,%d,%d],\"quad_z\":%.9g,"
	    "\"lighting_origin\":[0,0,%.9g],",
	    scene.m_Width, scene.m_Height, toneScale,
	    g_pMaterialSystem->GetCurrentConfigForVideoCard().m_nAASamples,
	    g_pMaterialSystemHardwareConfig->MaxNumLights(),
	    g_pMaterialSystemHardwareConfig->SupportsStaticControlFlow() ? "true" : "false",
	    kTextures[0].rgba[0], kTextures[0].rgba[1], kTextures[0].rgba[2], kQuadZ, kQuadZ );
	// The procedural textures' texels, and the materials' parameters.
	fprintf( out, "\"textures\":{" );
	for ( int t = 0; t < kTextureCount; ++t )
	{
		const ProceduralTexture &spec = kTextures[t];
		fprintf( out, "%s\"%s\":{\"size\":[%d,%d],\"row\":[", t ? "," : "", spec.name, spec.width,
		    spec.height );
		for ( int x = 0; x < spec.width; ++x )
		{
			unsigned char rgba[4];
			if ( spec.ramp )
				LightWarpTexel( x, rgba );
			else
				memcpy( rgba, spec.rgba, sizeof( rgba ) );
			fprintf( out, "%s[%d,%d,%d,%d]", x ? "," : "", rgba[0], rgba[1], rgba[2], rgba[3] );
		}
		fprintf( out, "]}" );
	}
	fprintf( out, "},\"materials\":{" );
	for ( int m = 0; m < kMaterialCount; ++m )
	{
		fprintf( out, "%s\"%s\":{", m ? "," : "", kMaterials[m].name );
		bool firstParam = true;
		for ( const auto &param : kMaterials[m].params )
		{
			if ( !param[0] )
				continue;
			fprintf( out, "%s\"%s\":\"%s\"", firstParam ? "" : ",", param[0], param[1] );
			firstParam = false;
		}
		fprintf( out, "}" );
	}
	fprintf( out, "},\"quads\":[" );
	for ( int q = 0; q < kGrid * kGrid; ++q )
	{
		float corners[4][2];
		QuadCorners( q, corners );
		float tangent[4];
		QuadTangent( q, tangent );
		fprintf( out, "%s{\"tangent\":[%.9g,%.9g,%.9g,%.9g],\"corners\":[", q ? "," : "",
		    tangent[0], tangent[1], tangent[2], tangent[3] );
		for ( int v = 0; v < 4; ++v )
			fprintf( out, "%s[%.9g,%.9g]", v ? "," : "", corners[v][0], corners[v][1] );
		fprintf( out, "],\"normal\":[%.9g,%.9g,%.9g],\"static_colors\":[", kQuadNormals[q][0],
		    kQuadNormals[q][1], kQuadNormals[q][2] );
		for ( int v = 0; v < 4; ++v )
		{
			unsigned char rgb[3];
			StaticColor( q, v, rgb );
			fprintf( out, "%s[%d,%d,%d]", v ? "," : "", rgb[0], rgb[1], rgb[2] );
		}
		fprintf( out, "]}" );
	}
	fprintf( out, "],\"cases\":[" );

	bool ok = true;
	bool first = true;
	for ( const ModelLightCase &c : kCases )
	{
		const std::string framePath = std::string( outPath ) + "." + c.name + ".rgb";
		ok = scene.RenderCase( c, out, framePath.c_str(), first ) && ok;
		first = false;
	}
	fprintf( out, "]}\n" );
	scene.Shutdown();
	return ok;
}
