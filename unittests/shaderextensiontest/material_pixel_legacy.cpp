//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "legacy" family of the material pixel conformance harness.
//
//          Each case of a KeyValues case file creates a material of one
//          stdshader_dx9 shader from its parameters, draws it through the real
//          material system over a cleared frame, and reads pixels back. The
//          textures it names are procedural and written into the report texel
//          for texel, and the backend records every pass's inputs
//          (-vklegacycapture <file>: shader pair, combos, registers, bound
//          textures, geometry and fixed-function state). The oracle,
//          tools/quality/legacy_shader_oracle.py, runs the shipped D3D9 bytecode
//          of that pass on those inputs and holds the pixels to its result.
//
//          Case file:
//            "LegacyShaderCases"
//            {
//              "texture" { "name" "conformance/legacy/x" "width" "4" "height" "4"
//                          [ "color" "r g b a" | "texels" "r g b a ..." ]
//                          [ "clamp" "1" ] [ "point" "1" ] [ "cube" "1" ]
//                          [ "alpha" "1" ] (TEXTUREFLAGS_EIGHTBITALPHA: a
//                          translucent texture, as shaders test with
//                          IsTranslucent before using its alpha) }
//              "case"    { "name" "..." "shader" "Modulate" "clear" "r g b a"
//                          "material" { "$basetexture" "conformance/legacy/x" ... }
//                          [ "framebuffer" "r g b a" ]  (_rt_FullFrameFB's color)
//                          [ "framebuffer1" "r g b a" ] (_rt_FullFrameFB1's color)
//                          [ "lightmap" "<texture name>" ] (BindLightmapTexture)
//                          [ "vertex" { "pos" "x y z" "color" "r g b a" "uv0" "u v"
//                                       "uv1" "u v" "uv2" "u v" "normal" "x y z"
//                                       "tangents" "x y z" "tangentt" "x y z"
//                                       "userdata" "x y z w" } x4 ]
//                          [ "model" | "view" | "projection" "16 floats" ]
//                          [ "ambient" "18 floats" ] (+x -x +y -y +z -z RGB)
//                          [ "light" { "type" "point|spot|directional"
//                                      "color" "r g b" "position" "x y z"
//                                      "direction" "x y z" "attenuation" "c l q"
//                                      "theta" "rad" "phi" "rad" "falloff" "f" } ]
//                          [ "tonescale" "s" ] (SetToneMappingScaleLinear)
//                          [ "fog" { "mode" "linear|below" "start" "s" "end" "e"
//                                    "color" "r g b" "z" "water z"
//                                    "maxdensity" "d" } ]
//                                    (the scene fog; none without the key)
//                          [ "texturevars" { "$var" "<texture name>" } ] (set as
//                            the engine does, SetTextureValue before the draw)
//                          [ "convars" { "<name>" "<value>" } ] (for the draw)
//                          [ "vectorparms" { "<RenderParamVector_t>" "x y z" } ]
//                          [ "intparms" { "<RenderParamInt_t>" "v" } ]
//                          [ "cclookup" { "matrix" "12 floats" } ] (up to four
//                            color correction lookups, bound as
//                            TEXTURE_COLOR_CORRECTION_VOLUME_0.. in order: each
//                            32^3 entry is saturate( M * ( r g b 1 ) ), M 3x4
//                            row by row; the case reports their texels)
//                          "points" "x y x y ..." (0..1 of the frame) }
//            }
//          A case without vertices draws a quad over the whole frame in clip
//          space (identity transforms) with uv0 from 0 to 1.
//
//=============================================================================//

#include "material_pixel_legacy.h"

#include "filesystem.h"
#include "KeyValues.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/IColorCorrection.h"
#include "mathlib/vector4d.h"
#include "renderparm.h"
#include "tier1/convar.h"
#include "pixelwriter.h"
#include "tier1/strtools.h"
#include "tier2/tier2.h"
#include "vtf/vtf.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace
{

// A procedural RGBA8888 texture's texels, face by face (one face unless cube),
// top row first. The material system keeps a regenerator pointer beyond the
// harness's cases, so these live until exit.
class CTexelRegenerator : public ITextureRegenerator
{
public:
	int m_Width = 1;
	int m_Height = 1;
	int m_Faces = 1;
	std::vector<unsigned char> m_Texels; // m_Faces * m_Height * m_Width * 4

	void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTF, Rect_t *pRect ) override
	{
		for ( int face = 0; face < pVTF->FaceCount() && face < m_Faces; ++face )
		{
			int width = 0, height = 0, depth = 0;
			pVTF->ComputeMipLevelDimensions( 0, &width, &height, &depth );
			CPixelWriter writer;
			writer.SetPixelMemory(
			    pVTF->Format(), pVTF->ImageData( 0, face, 0 ), pVTF->RowSizeInBytes( 0 ) );
			for ( int y = 0; y < height && y < m_Height; ++y )
			{
				writer.Seek( 0, y );
				for ( int x = 0; x < width && x < m_Width; ++x )
				{
					const unsigned char *c =
					    &m_Texels[( ( static_cast<size_t>( face ) * m_Height + y ) * m_Width + x ) *
					              4];
					writer.WritePixel( c[0], c[1], c[2], c[3] );
				}
			}
		}
	}

	void Release() override {}
};

// Engine and client convars stdshaders read, which the harness (no engine or
// client) registers with their defaults: the color blindness projection
// (engine/view.cpp; color_projection) and the Pyro vignette heat haze
// (client viewpostprocess.cpp; pyro_vision). Cases set them with "convars".
ConVar mat_color_projection(
    "mat_color_projection", "0", 0, "The color_projection shader's projection." );
ConVar pyro_vignette_distortion( "pyro_vignette_distortion", "1", 0,
    "Whether pyro_vision's vignette effect draws its heat haze." );

struct LegacyTexture
{
	std::string name;
	int flags = 0;
	CTexelRegenerator *regenerator = nullptr;
	ITexture *texture = nullptr;
};

// Parses up to `count` numbers from a space-separated string.
int ParseFloats( const char *text, float *out, int count )
{
	int parsed = 0;
	while ( text && *text && parsed < count )
	{
		char *end = nullptr;
		const float value = strtof( text, &end );
		if ( end == text )
			break;
		out[parsed++] = value;
		text = end;
	}
	return parsed;
}

void ParseColor( const char *text, unsigned char rgba[4] )
{
	float values[4] = { 255.0f, 255.0f, 255.0f, 255.0f };
	ParseFloats( text, values, 4 );
	for ( int i = 0; i < 4; ++i )
		rgba[i] = static_cast<unsigned char>( clamp( values[i], 0.0f, 255.0f ) );
}

bool CreateLegacyTexture( KeyValues *pKey, LegacyTexture *out )
{
	auto *regenerator = new CTexelRegenerator(); // lives until exit (see the class)
	regenerator->m_Width = pKey->GetInt( "width", 1 );
	regenerator->m_Height = pKey->GetInt( "height", 1 );
	const bool cube = pKey->GetInt( "cube", 0 ) != 0;
	regenerator->m_Faces = cube ? 6 : 1;
	const size_t texelCount =
	    static_cast<size_t>( regenerator->m_Faces ) * regenerator->m_Width * regenerator->m_Height;
	regenerator->m_Texels.resize( texelCount * 4 );
	unsigned char solid[4];
	ParseColor( pKey->GetString( "color", "255 255 255 255" ), solid );
	for ( size_t i = 0; i < texelCount; ++i )
		memcpy( &regenerator->m_Texels[i * 4], solid, 4 );
	const char *texels = pKey->GetString( "texels", "" );
	if ( texels[0] )
	{
		std::vector<float> values( texelCount * 4 );
		const int parsed = ParseFloats( texels, values.data(), static_cast<int>( values.size() ) );
		if ( parsed != static_cast<int>( values.size() ) )
		{
			Warning( "legacy cases: texture %s has %d of %d texel values\n",
			    pKey->GetString( "name" ), parsed, static_cast<int>( values.size() ) );
			return false;
		}
		for ( size_t i = 0; i < values.size(); ++i )
			regenerator->m_Texels[i] =
			    static_cast<unsigned char>( clamp( values[i], 0.0f, 255.0f ) );
	}
	out->name = pKey->GetString( "name" );
	out->flags =
	    TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL | TEXTUREFLAGS_SINGLECOPY;
	if ( pKey->GetInt( "clamp", 0 ) )
		out->flags |= TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_CLAMPU;
	if ( pKey->GetInt( "point", 0 ) )
		out->flags |= TEXTUREFLAGS_POINTSAMPLE;
	if ( cube )
		out->flags |= TEXTUREFLAGS_ENVMAP;
	if ( pKey->GetInt( "alpha", 0 ) )
		out->flags |= TEXTUREFLAGS_EIGHTBITALPHA;
	out->regenerator = regenerator;
	out->texture =
	    g_pMaterialSystem->CreateProceduralTexture( out->name.c_str(), TEXTURE_GROUP_OTHER,
	        regenerator->m_Width, regenerator->m_Height, IMAGE_FORMAT_RGBA8888, out->flags );
	if ( !out->texture || out->texture->IsError() )
	{
		Warning( "legacy cases: cannot create texture %s\n", out->name.c_str() );
		return false;
	}
	out->texture->SetTextureRegenerator( regenerator );
	out->texture->Download();
	return true;
}

void WriteTexture( FILE *out, const LegacyTexture &t, bool first )
{
	const CTexelRegenerator &r = *t.regenerator;
	fprintf( out, "%s\"%s\":{\"width\":%d,\"height\":%d,\"faces\":%d,\"flags\":%d,\"texels\":[",
	    first ? "" : ",", t.name.c_str(), r.m_Width, r.m_Height, r.m_Faces, t.flags );
	for ( size_t i = 0; i < r.m_Texels.size(); ++i )
		fprintf( out, "%s%d", i ? "," : "", r.m_Texels[i] );
	fprintf( out, "]}" );
}

struct LegacyVertex
{
	float pos[3] = { 0, 0, 0.5f };
	unsigned char color[4] = { 255, 255, 255, 255 };
	float uv[3][2] = {};
	float normal[3] = { 0, 0, 1 };
	float tangentS[3] = { 1, 0, 0 };
	float tangentT[3] = { 0, 1, 0 };
	float userData[4] = { 1, 0, 0, 1 };
};

// The default quad: the whole frame in clip space, uv0 0..1 from the top left,
// wound clockwise on screen as D3D9 draws front faces.
void DefaultQuad( LegacyVertex quad[4] )
{
	const float corners[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for ( int i = 0; i < 4; ++i )
	{
		quad[i] = LegacyVertex();
		quad[i].pos[0] = corners[i][0] * 2.0f - 1.0f;
		quad[i].pos[1] = 1.0f - corners[i][1] * 2.0f;
		for ( int set = 0; set < 3; ++set )
		{
			quad[i].uv[set][0] = corners[i][0];
			quad[i].uv[set][1] = corners[i][1];
		}
	}
}

bool ParseVertices( KeyValues *pCase, LegacyVertex quad[4] )
{
	DefaultQuad( quad );
	int index = 0;
	for ( KeyValues *pKey = pCase->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	{
		if ( Q_stricmp( pKey->GetName(), "vertex" ) )
			continue;
		if ( index >= 4 )
		{
			Warning( "legacy cases: %s has more than four vertices\n", pCase->GetString( "name" ) );
			return false;
		}
		LegacyVertex &v = quad[index++];
		ParseFloats( pKey->GetString( "pos", "" ), v.pos, 3 );
		if ( pKey->GetString( "color", "" )[0] )
			ParseColor( pKey->GetString( "color" ), v.color );
		ParseFloats( pKey->GetString( "uv0", "" ), v.uv[0], 2 );
		ParseFloats( pKey->GetString( "uv1", "" ), v.uv[1], 2 );
		ParseFloats( pKey->GetString( "uv2", "" ), v.uv[2], 2 );
		ParseFloats( pKey->GetString( "normal", "" ), v.normal, 3 );
		ParseFloats( pKey->GetString( "tangents", "" ), v.tangentS, 3 );
		ParseFloats( pKey->GetString( "tangentt", "" ), v.tangentT, 3 );
		ParseFloats( pKey->GetString( "userdata", "" ), v.userData, 4 );
	}
	return index == 0 || index == 4;
}

void LoadCaseMatrix(
    IMatRenderContext *pContext, KeyValues *pCase, const char *key, MaterialMatrixMode_t mode )
{
	// Loaded as a general matrix even when it is the identity: like the
	// engine's per-view loads, it makes the shader API commit the transforms
	// again, so a case never sees registers a previous case's shader wrote.
	pContext->MatrixMode( mode );
	float m[16];
	VMatrix matrix;
	matrix.Identity();
	if ( ParseFloats( pCase->GetString( key, "" ), m, 16 ) == 16 )
		memcpy( matrix.Base(), m, sizeof( m ) );
	pContext->LoadMatrix( matrix );
}

// The case's ambient cube and local lights (studiorender's order: the cube,
// then the lights, the rest disabled), and its tone-mapping scale.
void ApplyCaseLighting( IMatRenderContext *pContext, KeyValues *pCase )
{
	float ambient[18] = {};
	ParseFloats( pCase->GetString( "ambient", "" ), ambient, 18 );
	Vector4D cube[6];
	for ( int f = 0; f < 6; ++f )
		cube[f].Init( ambient[f * 3], ambient[f * 3 + 1], ambient[f * 3 + 2], 0.0f );
	pContext->SetAmbientLightCube( cube );
	pContext->DisableAllLocalLights();
	int index = 0;
	for ( KeyValues *pKey = pCase->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	{
		if ( Q_stricmp( pKey->GetName(), "light" ) )
			continue;
		LightDesc_t desc;
		const char *type = pKey->GetString( "type", "point" );
		desc.m_Type = !Q_stricmp( type, "spot" )          ? MATERIAL_LIGHT_SPOT
		              : !Q_stricmp( type, "directional" ) ? MATERIAL_LIGHT_DIRECTIONAL
		                                                  : MATERIAL_LIGHT_POINT;
		float v[3] = { 0, 0, 0 };
		ParseFloats( pKey->GetString( "color", "1 1 1" ), v, 3 );
		desc.m_Color.Init( v[0], v[1], v[2] );
		v[0] = v[1] = v[2] = 0.0f;
		ParseFloats( pKey->GetString( "position", "0 0 0" ), v, 3 );
		desc.m_Position.Init( v[0], v[1], v[2] );
		v[0] = v[1] = 0.0f;
		v[2] = -1.0f;
		ParseFloats( pKey->GetString( "direction", "0 0 -1" ), v, 3 );
		desc.m_Direction.Init( v[0], v[1], v[2] );
		float atten[3] = { 1.0f, 0.0f, 0.0f };
		ParseFloats( pKey->GetString( "attenuation", "1 0 0" ), atten, 3 );
		desc.m_Attenuation0 = atten[0];
		desc.m_Attenuation1 = atten[1];
		desc.m_Attenuation2 = atten[2];
		desc.m_Range = 0.0f;
		desc.m_Theta = pKey->GetFloat( "theta", 0.5f );
		desc.m_Phi = pKey->GetFloat( "phi", 1.0f );
		desc.m_Falloff = pKey->GetFloat( "falloff", 1.0f );
		desc.m_Flags = 0;
		pContext->SetLight( index++, desc );
	}
	const float scale = pCase->GetFloat( "tonescale", 1.0f );
	pContext->SetToneMappingScaleLinear( Vector( scale, scale, scale ) );
}

// The case's "texturevars": texture-valued material vars set before the draw,
// as the engine sets render targets on materials that never load them.
void ApplyCaseTextureVars( IMaterial *pMaterial, KeyValues *pCase )
{
	KeyValues *pVars = pCase->FindKey( "texturevars" );
	if ( !pVars )
		return;
	for ( KeyValues *pVar = pVars->GetFirstValue(); pVar; pVar = pVar->GetNextValue() )
	{
		bool found = false;
		IMaterialVar *pMaterialVar = pMaterial->FindVar( pVar->GetName(), &found, false );
		if ( found )
			pMaterialVar->SetTextureValue(
			    g_pMaterialSystem->FindTexture( pVar->GetString(), TEXTURE_GROUP_OTHER ) );
		else
			Warning( "legacy cases: %s has no var %s\n", pMaterial->GetName(), pVar->GetName() );
	}
}

// The case's "convars", "vectorparms" and "intparms"; `restore` puts the convars
// back and zeroes the rendering parameters after the draw.
void ApplyCaseParameters( IMatRenderContext *pContext, KeyValues *pCase, bool restore )
{
	if ( KeyValues *pConVars = pCase->FindKey( "convars" ) )
	{
		for ( KeyValues *pVar = pConVars->GetFirstValue(); pVar; pVar = pVar->GetNextValue() )
		{
			ConVar *pConVar = g_pCVar->FindVar( pVar->GetName() );
			if ( !pConVar )
				Warning( "legacy cases: no convar %s\n", pVar->GetName() );
			else if ( restore )
				pConVar->Revert();
			else
				pConVar->SetValue( pVar->GetString() );
		}
		// Shader DLL convars (FCVAR_MATERIAL_SYSTEM_THREAD) queue their sets
		// until the material system thread applies them, as the engine does
		// each frame.
		g_pCVar->ProcessQueuedMaterialThreadConVarSets();
	}
	if ( KeyValues *pParms = pCase->FindKey( "vectorparms" ) )
	{
		for ( KeyValues *pParm = pParms->GetFirstValue(); pParm; pParm = pParm->GetNextValue() )
		{
			float v[3] = { 0, 0, 0 };
			if ( !restore )
				ParseFloats( pParm->GetString(), v, 3 );
			pContext->SetVectorRenderingParameter(
			    atoi( pParm->GetName() ), Vector( v[0], v[1], v[2] ) );
		}
	}
	if ( KeyValues *pParms = pCase->FindKey( "intparms" ) )
	{
		for ( KeyValues *pParm = pParms->GetFirstValue(); pParm; pParm = pParm->GetNextValue() )
			pContext->SetIntRenderingParameter(
			    atoi( pParm->GetName() ), restore ? 0 : pParm->GetInt() );
	}
}

// The case's color correction lookups ("cclookup"), created in order with
// decreasing weights so the color correction system binds them as
// TEXTURE_COLOR_CORRECTION_VOLUME_0.. in that order. Writes their textures
// (named as CColorCorrection.cpp names them) to the case's report.
std::vector<ColorCorrectionHandle_t> CreateCaseLookups( FILE *out, KeyValues *pCase )
{
	std::vector<ColorCorrectionHandle_t> handles;
	for ( KeyValues *pKey = pCase->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	{
		if ( Q_stricmp( pKey->GetName(), "cclookup" ) )
			continue;
		if ( !colorcorrection || handles.size() == 4 )
		{
			Warning( "legacy cases: %s: no color correction system or over four lookups\n",
			    pCase->GetString( "name" ) );
			break;
		}
		float m[12] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 };
		ParseFloats( pKey->GetString( "matrix", "" ), m, 12 );
		char name[256];
		V_snprintf( name, sizeof( name ), "conformance/legacy/%s/cclookup%d",
		    pCase->GetString( "name" ), static_cast<int>( handles.size() ) );
		const ColorCorrectionHandle_t handle = colorcorrection->AddLookup( name );
		colorcorrection->LockLookup( handle );
		for ( int b = 0; b < 32; ++b )
		{
			for ( int g = 0; g < 32; ++g )
			{
				for ( int r = 0; r < 32; ++r )
				{
					const float in[4] = { r / 31.0f, g / 31.0f, b / 31.0f, 1.0f };
					unsigned char rgb[3];
					for ( int row = 0; row < 3; ++row )
					{
						const float v = m[row * 4] * in[0] + m[row * 4 + 1] * in[1] +
						                m[row * 4 + 2] * in[2] + m[row * 4 + 3] * in[3];
						rgb[row] =
						    static_cast<unsigned char>( clamp( v, 0.0f, 1.0f ) * 255.0f + 0.5f );
					}
					RGBX5551_t inColor;
					inColor.r = r;
					inColor.g = g;
					inColor.b = b;
					inColor.x = 0;
					color24 outColor;
					outColor.r = rgb[0];
					outColor.g = rgb[1];
					outColor.b = rgb[2];
					colorcorrection->SetLookup( handle, inColor, outColor );
				}
			}
		}
		colorcorrection->UnlockLookup( handle );
		handles.push_back( handle );
	}
	if ( handles.empty() )
		return handles;
	colorcorrection->ResetLookupWeights();
	for ( size_t i = 0; i < handles.size(); ++i )
		colorcorrection->SetLookupWeight( handles[i], 1.0f - 0.1f * static_cast<float>( i ) );

	// Each lookup's volume as ColorCorrectionTexture regenerates it: x red, y
	// green, z blue, BGRX8888 (alpha 255), clamped.
	fprintf( out, ",\"volumes\":{" );
	for ( size_t i = 0; i < handles.size(); ++i )
	{
		char textureName[64];
		V_snprintf( textureName, sizeof( textureName ), "ColorCorrection - %p",
		    reinterpret_cast<void *>( handles[i] ) );
		fprintf( out, "%s\"%s\":{\"width\":32,\"height\":32,\"depth\":32,\"flags\":%d,\"texels\":[",
		    i ? "," : "", textureName,
		    TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_CLAMPU );
		for ( int b = 0; b < 32; ++b )
		{
			for ( int g = 0; g < 32; ++g )
			{
				for ( int r = 0; r < 32; ++r )
				{
					RGBX5551_t inColor;
					inColor.r = r;
					inColor.g = g;
					inColor.b = b;
					inColor.x = 0;
					const color24 c = colorcorrection->GetLookup( handles[i], inColor );
					fprintf( out, "%s%d,%d,%d,255", ( b | g | r ) ? "," : "", c.r, c.g, c.b );
				}
			}
		}
		fprintf( out, "]}" );
	}
	fprintf( out, "}" );
	return handles;
}

// The case's scene fog (FogMode none without a "fog" block): linear range
// fog, or linear below the water plane at "z".
void ApplyCaseFog( IMatRenderContext *pContext, KeyValues *pCase )
{
	KeyValues *pFog = pCase->FindKey( "fog" );
	if ( !pFog )
	{
		pContext->FogMode( MATERIAL_FOG_NONE );
		return;
	}
	const char *mode = pFog->GetString( "mode", "linear" );
	pContext->FogMode(
	    !Q_stricmp( mode, "below" ) ? MATERIAL_FOG_LINEAR_BELOW_FOG_Z : MATERIAL_FOG_LINEAR );
	pContext->FogStart( pFog->GetFloat( "start", 0.0f ) );
	pContext->FogEnd( pFog->GetFloat( "end", 1.0f ) );
	pContext->SetFogZ( pFog->GetFloat( "z", 0.0f ) );
	pContext->FogMaxDensity( pFog->GetFloat( "maxdensity", 1.0f ) );
	unsigned char color[4];
	ParseColor( pFog->GetString( "color", "0 0 0" ), color );
	pContext->FogColor3ub( color[0], color[1], color[2] );
}

} // namespace

bool RunLegacyCases( FILE *out, const char *casesPath, void ( *writeClearProbe )( FILE * ) )
{
	KeyValues *pCases = new KeyValues( "LegacyShaderCases" );
	if ( !pCases->LoadFromFile( g_pFullFileSystem, casesPath, NULL ) )
	{
		Warning( "legacy cases: cannot read %s\n", casesPath );
		pCases->deleteThis();
		return false;
	}

	// The engine's frame copies (matsys_interface.cpp CreateFullFrameFBTexture),
	// which screen-space shaders read as their $basetexture or as the frame
	// buffer copy textures (BindStandardTexture TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0
	// and _1, the engine's SetFrameBufferCopyTexture).
	g_pMaterialSystem->BeginRenderTargetAllocation();
	ITexture *pFrameBuffer = g_pMaterialSystem->CreateNamedRenderTargetTextureEx2(
	    "_rt_FullFrameFB", 1, 1, RT_SIZE_FULL_FRAME_BUFFER,
	    g_pMaterialSystem->GetBackBufferFormat(), MATERIAL_RT_DEPTH_SHARED,
	    TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, CREATERENDERTARGETFLAGS_HDR );
	ITexture *pFrameBuffer1 = g_pMaterialSystem->CreateNamedRenderTargetTextureEx2(
	    "_rt_FullFrameFB1", 1, 1, RT_SIZE_FULL_FRAME_BUFFER,
	    g_pMaterialSystem->GetBackBufferFormat(), MATERIAL_RT_DEPTH_SHARED,
	    TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, CREATERENDERTARGETFLAGS_HDR );
	g_pMaterialSystem->EndRenderTargetAllocation();
	if ( !pFrameBuffer || pFrameBuffer->IsError() || !pFrameBuffer1 || pFrameBuffer1->IsError() )
		return false;
	pFrameBuffer->IncrementReferenceCount();
	pFrameBuffer1->IncrementReferenceCount();

	std::vector<LegacyTexture> textures;
	for ( KeyValues *pKey = pCases->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	{
		if ( Q_stricmp( pKey->GetName(), "texture" ) )
			continue;
		LegacyTexture texture;
		if ( !CreateLegacyTexture( pKey, &texture ) )
			return false;
		textures.push_back( texture );
	}

	// Every case's material first, then one CacheUsedMaterials (as the harness's
	// other families do: D3D9's dynamic mesh needs the vertex formats cached).
	struct Case
	{
		KeyValues *keys;
		IMaterial *material;
	};
	std::vector<Case> cases;
	for ( KeyValues *pKey = pCases->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	{
		if ( Q_stricmp( pKey->GetName(), "case" ) )
			continue;
		KeyValues *pMaterialKeys = new KeyValues( pKey->GetString( "shader", "UnlitGeneric" ) );
		if ( KeyValues *pParams = pKey->FindKey( "material" ) )
		{
			for ( KeyValues *pParam = pParams->GetFirstValue(); pParam;
			    pParam = pParam->GetNextValue() )
				pMaterialKeys->SetString( pParam->GetName(), pParam->GetString() );
		}
		char name[256];
		V_snprintf( name, sizeof( name ), "conformance/legacy/%s", pKey->GetString( "name" ) );
		IMaterial *pMaterial = g_pMaterialSystem->CreateMaterial( name, pMaterialKeys );
		if ( !pMaterial || pMaterial->IsErrorMaterial() )
		{
			Warning( "legacy cases: material %s did not load\n", name );
			return false;
		}
		pMaterial->IncrementReferenceCount();
		cases.push_back( { pKey, pMaterial } );
	}
	g_pMaterialSystem->CacheUsedMaterials();

	// A texture parameter the material's shader does not load itself (DepthWrite
	// binds $basetexture without LoadTexture: the engine hands it a loaded
	// texture) gets the case's texture, as the engine would set it.
	for ( const Case &c : cases )
	{
		KeyValues *pParams = c.keys->FindKey( "material" );
		for ( KeyValues *pParam = pParams ? pParams->GetFirstValue() : NULL; pParam;
		    pParam = pParam->GetNextValue() )
		{
			for ( const LegacyTexture &t : textures )
			{
				if ( Q_stricmp( t.name.c_str(), pParam->GetString() ) )
					continue;
				bool found = false;
				IMaterialVar *pVar = c.material->FindVar( pParam->GetName(), &found, false );
				if ( found && pVar && !pVar->IsTexture() )
					pVar->SetTextureValue( t.texture );
			}
		}
	}

	fprintf( out, "{\"schema\":\"source-material-pixels/v1\",\"family\":\"legacy\"," );
	writeClearProbe( out );
	int width = 0, height = 0;
	g_pMaterialSystem->GetBackBufferDimensions( width, height );
	fprintf( out, "\"frame\":[%d,%d],\"textures\":{", width, height );
	for ( size_t i = 0; i < textures.size(); ++i )
		WriteTexture( out, textures[i], i == 0 );
	fprintf( out, "},\"cases\":[" );

	bool ok = true;
	for ( size_t c = 0; c < cases.size(); ++c )
	{
		KeyValues *pCase = cases[c].keys;
		LegacyVertex quad[4];
		if ( !ParseVertices( pCase, quad ) )
		{
			ok = false;
			continue;
		}
		unsigned char clearColor[4], fbColor[4], fb1Color[4];
		ParseColor( pCase->GetString( "clear", "0 0 0 255" ), clearColor );
		const bool framebuffer = pCase->GetString( "framebuffer", "" )[0] != 0;
		ParseColor( pCase->GetString( "framebuffer", "0 0 0 255" ), fbColor );
		const bool framebuffer1 = pCase->GetString( "framebuffer1", "" )[0] != 0;
		ParseColor( pCase->GetString( "framebuffer1", "0 0 0 255" ), fb1Color );
		float points[64];
		const int pointCount =
		    ParseFloats( pCase->GetString( "points", "0.5 0.5" ), points, 64 ) / 2;

		fprintf( out,
		    "%s{\"name\":\"%s\",\"shader\":\"%s\",\"material\":\"%s\",\"clear\":[%d,%d,%d,%d]",
		    c ? "," : "", pCase->GetString( "name" ), pCase->GetString( "shader" ),
		    cases[c].material->GetName(), clearColor[0], clearColor[1], clearColor[2],
		    clearColor[3] );
		if ( pCase->FindKey( "tolerance" ) )
			fprintf( out, ",\"tolerance\":%d", pCase->GetInt( "tolerance" ) );
		if ( framebuffer )
			fprintf( out, ",\"framebuffer\":[%d,%d,%d,%d]", fbColor[0], fbColor[1], fbColor[2],
			    fbColor[3] );
		if ( framebuffer1 )
			fprintf( out, ",\"framebuffer1\":[%d,%d,%d,%d]", fb1Color[0], fb1Color[1], fb1Color[2],
			    fb1Color[3] );
		const std::vector<ColorCorrectionHandle_t> lookups = CreateCaseLookups( out, pCase );
		ApplyCaseTextureVars( cases[c].material, pCase );
		fprintf( out, ",\"pixels\":[" );

		g_pMaterialSystem->BeginFrame( 0 );
		{
			CMatRenderContextPtr pContext( g_pMaterialSystem );
			pContext->Viewport( 0, 0, width, height );
			pContext->SetFrameBufferCopyTexture( pFrameBuffer, 0 );
			pContext->SetFrameBufferCopyTexture( pFrameBuffer1, 1 );
			if ( framebuffer )
			{
				pContext->ClearColor4ub( fbColor[0], fbColor[1], fbColor[2], fbColor[3] );
				pContext->ClearBuffers( true, true );
				Rect_t whole = { 0, 0, width, height };
				pContext->CopyRenderTargetToTextureEx( pFrameBuffer, 0, &whole, NULL );
			}
			if ( framebuffer1 )
			{
				pContext->ClearColor4ub( fb1Color[0], fb1Color[1], fb1Color[2], fb1Color[3] );
				pContext->ClearBuffers( true, true );
				Rect_t whole = { 0, 0, width, height };
				pContext->CopyRenderTargetToTextureEx( pFrameBuffer1, 0, &whole, NULL );
			}
			pContext->ClearColor4ub( clearColor[0], clearColor[1], clearColor[2], clearColor[3] );
			pContext->ClearBuffers( true, true );
			LoadCaseMatrix( pContext, pCase, "view", MATERIAL_VIEW );
			LoadCaseMatrix( pContext, pCase, "projection", MATERIAL_PROJECTION );
			LoadCaseMatrix( pContext, pCase, "model", MATERIAL_MODEL );
			ApplyCaseLighting( pContext, pCase );
			ApplyCaseFog( pContext, pCase );
			ApplyCaseParameters( pContext, pCase, false );
			const char *lightmap = pCase->GetString( "lightmap", "" );
			if ( lightmap[0] )
			{
				for ( const LegacyTexture &t : textures )
				{
					if ( t.name == lightmap )
						pContext->BindLightmapTexture( t.texture );
				}
			}
			pContext->Bind( cases[c].material );
			// A model's vertex data carries tangents and TANGENT user data whatever
			// format the material declares (aftershock_vs20 and volume_clouds_vs20
			// read TANGENT under a position-and-normal format), and the backend
			// keeps only the components a mesh's format has. Declare every
			// component the quad writes.
			const VertexFormat_t quadFormat =
			    VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_TANGENT_S |
			    VERTEX_TANGENT_T | VERTEX_USERDATA_SIZE( 4 ) | VERTEX_TEXCOORD_SIZE( 0, 2 ) |
			    VERTEX_TEXCOORD_SIZE( 1, 2 ) | VERTEX_TEXCOORD_SIZE( 2, 2 );
			IMesh *pMesh = pContext->GetDynamicMeshEx( quadFormat );
			CMeshBuilder meshBuilder;
			meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
			for ( const LegacyVertex &v : quad )
			{
				meshBuilder.Position3fv( v.pos );
				meshBuilder.Normal3fv( v.normal );
				meshBuilder.Color4ubv( v.color );
				for ( int set = 0; set < 3; ++set )
					meshBuilder.TexCoord2fv( set, v.uv[set] );
				meshBuilder.TangentS3fv( v.tangentS );
				meshBuilder.TangentT3fv( v.tangentT );
				meshBuilder.UserData( v.userData );
				meshBuilder.AdvanceVertex();
			}
			meshBuilder.End();
			pMesh->Draw();
			for ( int p = 0; p < pointCount; ++p )
			{
				const int x = clamp( static_cast<int>( points[p * 2] * width ), 0, width - 1 );
				const int y =
				    clamp( static_cast<int>( points[p * 2 + 1] * height ), 0, height - 1 );
				unsigned char rgba[4] = { 0, 0, 0, 0 };
				pContext->ReadPixels( x, y, 1, 1, rgba, IMAGE_FORMAT_RGBA8888 );
				fprintf( out, "%s{\"x\":%d,\"y\":%d,\"rgba\":[%d,%d,%d,%d]}", p ? "," : "", x, y,
				    rgba[0], rgba[1], rgba[2], rgba[3] );
			}
			pContext->MatrixMode( MATERIAL_MODEL );
			pContext->LoadIdentity();
			ApplyCaseParameters( pContext, pCase, true );
		}
		g_pMaterialSystem->EndFrame();
		g_pMaterialSystem->SwapBuffers();
		for ( ColorCorrectionHandle_t handle : lookups )
			colorcorrection->RemoveLookup( handle );
		fprintf( out, "]}" );
	}
	fprintf( out, "]}\n" );
	for ( const Case &c : cases )
		c.material->DecrementReferenceCount();
	pFrameBuffer->DecrementReferenceCount();
	pFrameBuffer1->DecrementReferenceCount();
	pCases->deleteThis();
	return ok;
}
