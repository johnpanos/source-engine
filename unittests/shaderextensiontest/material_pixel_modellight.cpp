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

// The base texture's color (sRGB bytes), decoded by the material's sRGB read.
const unsigned char kBaseColor[3] = { 190, 150, 110 };
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

struct ModelLightCase
{
	const char *name;
	bool halfLambert;
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
const ModelLight kDirectional2 = { MATERIAL_LIGHT_DIRECTIONAL, { 0.1f, 0.2f, 0.35f },
    { 0, 0, 0 }, { 0.8f, 0.0f, -0.6f }, { 1, 0, 0 }, 0, 0, 0 };
const ModelLight kPoint = { MATERIAL_LIGHT_POINT, { 1.6f, 1.2f, 0.9f }, { -0.3f, 0.4f, -0.9f },
    { 0, 0, 1 }, { 0.2f, 0.3f, 0.5f }, 0, 0, 0 };
const ModelLight kSpot = { MATERIAL_LIGHT_SPOT, { 1.1f, 0.9f, 1.3f }, { 0.25f, -0.2f, -1.2f },
    { -0.15f, 0.1f, 0.98f }, { 0.4f, 0.4f, 0.0f }, 0.7f, 1.6f, 2.0f };

const ModelLightCase kCases[] = {
    { "ambient_cube",
        false,
        { { 0.6f, 0.08f, 0.05f }, { 0.07f, 0.5f, 0.06f }, { 0.05f, 0.08f, 0.55f },
            { 0.4f, 0.35f, 0.04f }, { 0.05f, 0.4f, 0.45f }, { 0.3f, 0.12f, 0.45f } },
        0,
        {},
        false },
    { "directional", false, {}, 1, { kDirectional }, false },
    { "point", false, {}, 1, { kPoint }, false },
    { "spot", false, {}, 1, { kSpot }, false },
    { "four_lights",
        false,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        4,
        { kSpot, kDirectional, kPoint, kDirectional2 },
        false },
    { "half_lambert", true, {}, 1, { kDirectional }, false },
    { "static_vertex", false, {}, 0, {}, true },
    { "static_and_dynamic",
        false,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        1,
        { kDirectional2 },
        true },
    { "no_light", false, {}, 0, {}, false },
    { "model_transform",
        false,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        2,
        { kDirectional, kPoint },
        false,
        kRigid },
    { "skinned",
        false,
        { { 0.05f, 0.04f, 0.03f }, { 0.02f, 0.05f, 0.03f }, { 0.04f, 0.04f, 0.06f },
            { 0.03f, 0.02f, 0.02f }, { 0.02f, 0.03f, 0.05f }, { 0.04f, 0.02f, 0.03f } },
        2,
        { kDirectional, kPoint },
        false,
        kSkinned },
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

class CBaseColorRegenerator : public ITextureRegenerator
{
public:
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
					writer.WritePixel( kBaseColor[0], kBaseColor[1], kBaseColor[2], 255 );
			}
		}
	}
	void Release() override {}
};

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

	ITexture *m_pBase = nullptr;
	IMaterial *m_pMaterials[2] = { nullptr, nullptr }; // Lambert, half-Lambert
	IMesh *m_pMesh[2] = { nullptr, nullptr };
	IMesh *m_pColorMesh = nullptr;
};

bool CModelLightScene::Init()
{
	g_pMaterialSystem->GetBackBufferDimensions( m_Width, m_Height );
	m_pBase = g_pMaterialSystem->CreateProceduralTexture( "conformance/modellight_base",
	    TEXTURE_GROUP_OTHER, 4, 4, IMAGE_FORMAT_RGBA8888,
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL |
	        TEXTUREFLAGS_SINGLECOPY );
	if ( !m_pBase )
		return false;
	// The texture lives until the material system shuts down, and calls its
	// regenerator then, so the regenerator outlives this scene.
	static CBaseColorRegenerator s_Regenerator;
	m_pBase->SetTextureRegenerator( &s_Regenerator );
	m_pBase->Download();

	for ( int m = 0; m < 2; ++m )
	{
		KeyValues *pKeys = new KeyValues( "VertexLitGeneric" );
		pKeys->SetString( "$basetexture", "conformance/modellight_base" );
		pKeys->SetInt( "$model", 1 );
		if ( m == 1 )
			pKeys->SetInt( "$halflambert", 1 );
		m_pMaterials[m] = g_pMaterialSystem->CreateMaterial(
		    m ? "conformance/modellight_halflambert" : "conformance/modellight", pKeys );
		if ( !m_pMaterials[m] || m_pMaterials[m]->IsErrorMaterial() )
			return false;
		m_pMaterials[m]->IncrementReferenceCount();
	}
	g_pMaterialSystem->CacheUsedMaterials();

	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	for ( int m = 0; m < 2; ++m )
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
			const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
			for ( int v = 0; v < 4; ++v )
			{
				meshBuilder.Position3f( corners[v][0], corners[v][1], kQuadZ );
				meshBuilder.Normal3fv( kQuadNormals[q] );
				meshBuilder.Color4ub( 255, 255, 255, 255 );
				meshBuilder.TexCoord2f( 0, uv[v][0], uv[v][1] );
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
	for ( int m = 0; m < 2; ++m )
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
	const int m = c.halfLambert ? 1 : 0;
	std::vector<unsigned char> rgba( static_cast<size_t>( m_Width ) * m_Height * 4 );
	g_pMaterialSystem->BeginFrame( 0 );
	{
		CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
		pRenderContext->Viewport( 0, 0, m_Width, m_Height );
		pRenderContext->SetToneMappingScaleLinear( Vector( m_ToneScale, m_ToneScale, m_ToneScale ) );
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

	fprintf( out, "%s{\"name\":\"%s\",\"half_lambert\":%s,\"static_color\":%s,",
	    first ? "" : ",", c.name, c.halfLambert ? "true" : "false",
	    c.staticColor ? "true" : "false" );
	// The model-to-world transform, rows of a 3x4 column-vector matrix.
	const matrix3x4_t &placement = c.placement == kIdentity
	                                   ? matrix3x4_t( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 )
	                                   : kTransform;
	fprintf( out, "\"placement\":\"%s\",\"model_matrix\":[",
	    c.placement == kSkinned ? "skinned" : c.placement == kRigid ? "rigid" : "identity" );
	for ( int r = 0; r < 3; ++r )
		fprintf( out, "%s[%.9g,%.9g,%.9g,%.9g]", r ? "," : "", placement[r][0], placement[r][1],
		    placement[r][2], placement[r][3] );
	fprintf( out, "],\"cube\":[" );
	for ( int f = 0; f < 6; ++f )
		fprintf( out, "%s[%.9g,%.9g,%.9g]", f ? "," : "", c.cube[f][0], c.cube[f][1],
		    c.cube[f][2] );
	fprintf( out, "],\"lights\":[" );
	for ( int i = 0; i < c.numLights; ++i )
	{
		const ModelLight &l = c.lights[i];
		const bool directional = l.type == MATERIAL_LIGHT_DIRECTIONAL;
		float position[3];
		for ( int k = 0; k < 3; ++k )
			position[k] = directional ? l.direction[k] * kDirectionalDistance : l.position[k];
		const char *type = l.type == MATERIAL_LIGHT_DIRECTIONAL ? "directional"
		                   : l.type == MATERIAL_LIGHT_SPOT     ? "spot"
		                                                       : "point";
		fprintf( out,
		    "%s{\"type\":\"%s\",\"color\":[%.9g,%.9g,%.9g],\"position\":[%.9g,%.9g,%.9g],"
		    "\"direction\":[%.9g,%.9g,%.9g],\"attenuation\":[%.9g,%.9g,%.9g],"
		    "\"theta\":%.9g,\"phi\":%.9g,\"falloff\":%.9g}",
		    i ? "," : "", type, l.color[0], l.color[1], l.color[2], position[0], position[1],
		    position[2], l.direction[0], l.direction[1], l.direction[2],
		    l.attenuation[0], l.attenuation[1], l.attenuation[2], l.theta, l.phi, l.falloff );
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
	    "\"static_control_flow\":%s,\"base_color\":[%d,%d,%d],\"quad_z\":%.9g,\"quads\":[",
	    scene.m_Width, scene.m_Height, toneScale,
	    g_pMaterialSystem->GetCurrentConfigForVideoCard().m_nAASamples,
	    g_pMaterialSystemHardwareConfig->MaxNumLights(),
	    g_pMaterialSystemHardwareConfig->SupportsStaticControlFlow() ? "true" : "false",
	    kBaseColor[0], kBaseColor[1], kBaseColor[2], kQuadZ );
	for ( int q = 0; q < kGrid * kGrid; ++q )
	{
		float corners[4][2];
		QuadCorners( q, corners );
		fprintf( out, "%s{\"corners\":[", q ? "," : "" );
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
