//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Canonical PBRMetalRough material binding for native Vulkan worlds.
//          The shader API owns the WMSH PBR pipelines; this material owns its
//          Source parameters, texture loading and snapshot sampler contract.
//          A material with $transmission > 0 is glass: translucent, drawn
//          without blending or depth writes by the WMSH glass pipeline, which
//          reads its $transmission, $ior and $thickness from pixel constant c0.
//          On other meshes (models and props) the shader API draws the model
//          pipeline (shaders/model_pbr.frag), lit by the ambient cube (c4..c9)
//          and the local lights (c20..c25) this dynamic state commits.
//          c2.x is $emissionscale, c2.y $clearcoat and c2.z
//          $clearcoatroughness; c3.x the feature flags the native pipelines
//          test (render::pbr::NativeFeature).
//
//===========================================================================//

#include "BaseVSShader.h"
#include "render/pbr_material_schema.h"
#include "cpp_shader_constant_register_map.h"

#include <cstdlib>

namespace
{

float SchemaDefault( render::pbr::MaterialParameter parameter )
{
	return static_cast<float>( std::atof( render::pbr::Parameter( parameter ).defaultValue ) );
}

} // namespace

BEGIN_VS_SHADER( PBRMetalRough, "Metalness/roughness world material" )
BEGIN_SHADER_PARAMS
SHADER_PARAM( MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Linear metal/rough/AO map" )
SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Linear tangent-space normal map" )
SHADER_PARAM( EMISSIONTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Emissive color map" )
SHADER_PARAM( EMISSIONSCALE, SHADER_PARAM_TYPE_FLOAT, "1", "Linear emission multiplier" )
SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_ENVMAP, "", "Reflection probe" )
SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.7", "Alpha cutoff" )
SHADER_PARAM( TRANSMISSION, SHADER_PARAM_TYPE_FLOAT, "0", "Refracted fraction (glass)" )
SHADER_PARAM( IOR, SHADER_PARAM_TYPE_FLOAT, "1.5", "Index of refraction" )
SHADER_PARAM( THICKNESS, SHADER_PARAM_TYPE_FLOAT, "0", "World units through; 0 = thin sheet" )
SHADER_PARAM( CLEARCOAT, SHADER_PARAM_TYPE_FLOAT, "0", "Clear coat weight (IOR 1.5 layer)" )
SHADER_PARAM( CLEARCOATROUGHNESS, SHADER_PARAM_TYPE_FLOAT, "0.03", "Clear coat roughness" )
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	using render::pbr::MaterialParameter;
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
	// Vertex lit too: the engine sets up a studio model's lighting (ambient
	// cube, local lights) only when one of its materials is vertex lit
	// (modelloader.cpp MODELFLAG_VERTEXLIT); without it model_pbr.frag read a
	// black cube and no lights. Only studio-model loading reads this flag.
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
	if ( !params[TRANSMISSION]->IsDefined() )
		params[TRANSMISSION]->SetFloatValue( SchemaDefault( MaterialParameter::kTransmission ) );
	if ( !params[IOR]->IsDefined() )
		params[IOR]->SetFloatValue( SchemaDefault( MaterialParameter::kIndexOfRefraction ) );
	if ( !params[THICKNESS]->IsDefined() )
		params[THICKNESS]->SetFloatValue( SchemaDefault( MaterialParameter::kThickness ) );
	if ( !params[EMISSIONSCALE]->IsDefined() )
		params[EMISSIONSCALE]->SetFloatValue( SchemaDefault( MaterialParameter::kEmissionScale ) );
	if ( !params[CLEARCOAT]->IsDefined() )
		params[CLEARCOAT]->SetFloatValue( SchemaDefault( MaterialParameter::kClearCoat ) );
	if ( !params[CLEARCOATROUGHNESS]->IsDefined() )
		params[CLEARCOATROUGHNESS]->SetFloatValue(
		    SchemaDefault( MaterialParameter::kClearCoatRoughness ) );
	// Glass is translucent by definition: it sorts with translucent surfaces
	// and draws after the opaque scene it refracts.
	if ( params[TRANSMISSION]->GetFloatValue() > 0.0f )
		SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );
}

SHADER_FALLBACK
{
	return 0;
}

SHADER_INIT
{
	if ( !params[BASETEXTURE]->IsDefined() || !params[MRAOTEXTURE]->IsDefined() )
		Warning(
		    "PBRMetalRough material %s requires $basetexture and $mraotexture\n", pMaterialName );
	const float transmission = params[TRANSMISSION]->GetFloatValue();
	if ( !render::pbr::IsValidTransmission(
	         transmission, params[IOR]->GetFloatValue(), params[THICKNESS]->GetFloatValue() ) )
		Warning( "PBRMetalRough material %s has $transmission, $ior or $thickness out of range\n",
		    pMaterialName );
	if ( !render::pbr::IsValidClearCoat(
	         params[CLEARCOAT]->GetFloatValue(), params[CLEARCOATROUGHNESS]->GetFloatValue() ) )
		Warning( "PBRMetalRough material %s has $clearcoat or $clearcoatroughness out of range\n",
		    pMaterialName );
	if ( !( params[EMISSIONSCALE]->GetFloatValue() >= 0.0f ) )
		Warning( "PBRMetalRough material %s has a negative or invalid $emissionscale\n",
		    pMaterialName );
	if ( params[BASETEXTURE]->IsDefined() )
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
	if ( params[MRAOTEXTURE]->IsDefined() )
		LoadTexture( MRAOTEXTURE, 0 );
	if ( params[BUMPMAP]->IsDefined() )
		LoadBumpMap( BUMPMAP );
	if ( params[EMISSIONTEXTURE]->IsDefined() )
		LoadTexture( EMISSIONTEXTURE, TEXTUREFLAGS_SRGB );
	if ( params[ENVMAP]->IsDefined() )
		LoadCubeMap( ENVMAP, 0 );
}

SHADER_DRAW
{
	const float transmission = params[TRANSMISSION]->GetFloatValue();
	const float ior = params[IOR]->GetFloatValue();
	const float thickness = params[THICKNESS]->GetFloatValue();
	const bool glass = transmission > 0.0f;
	const float emissionScale = params[EMISSIONSCALE]->GetFloatValue();
	const bool emission = params[EMISSIONTEXTURE]->IsTexture();
	const bool envmap = params[ENVMAP]->IsTexture();
	const bool normalMap = params[BUMPMAP]->IsTexture();
	const float clearCoat = params[CLEARCOAT]->GetFloatValue();
	const float clearCoatRoughness = params[CLEARCOATROUGHNESS]->GetFloatValue();
	// Missing required textures cannot borrow a previous material's GPU
	// binding. The shader API also validates formats before it queues a draw,
	// and declines features a mesh's native pipeline lacks by name.
	const bool supported = params[BASETEXTURE]->IsTexture() && params[MRAOTEXTURE]->IsTexture() &&
	                       render::pbr::IsValidTransmission( transmission, ior, thickness ) &&
	                       render::pbr::IsValidClearCoat( clearCoat, clearCoatRoughness ) &&
	                       emissionScale >= 0.0f;
	int features = 0;
	if ( normalMap )
		features |= render::pbr::kNativeNormalMap;
	if ( emission )
		features |= render::pbr::kNativeEmission;
	if ( envmap )
		features |= render::pbr::kNativeEnvMap;
	if ( IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ) && !glass )
		features |= render::pbr::kNativeTranslucent;
	if ( clearCoat > 0.0f )
		features |= render::pbr::kNativeClearCoat;
	SHADOW_STATE
	{
		if ( glass )
		{
			// The glass shader composites the refracted scene itself.
			pShaderShadow->EnableBlending( false );
			pShaderShadow->EnableDepthWrites( false );
		}
		else
			SetDefaultBlendingShadowState( BASETEXTURE, true );
		const bool alphaTest = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST );
		pShaderShadow->EnableAlphaTest( alphaTest );
		if ( alphaTest )
			pShaderShadow->AlphaFunc(
			    SHADER_ALPHAFUNC_GEQUAL, params[ALPHATESTREFERENCE]->GetFloatValue() );
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER10, true );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER10, false );
		if ( normalMap )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false );
		}
		if ( emission )
		{
			// Decoded by the shader, so the sampler reads the stored bytes.
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, false );
		}
		if ( envmap )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, false );
		}
		pShaderShadow->EnableSRGBWrite( true );
		// Models need the tangent (user data) for normal maps; WMSH batches
		// carry their own vertex layout.
		pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_NORMAL, 3, 0, 4 );
		pShaderShadow->SetVertexShader( "pbr_metalrough_world_vs", 0 );
		pShaderShadow->SetPixelShader( "pbr_metalrough_world_ps", 0 );
	}
	// Source needs a snapshot even for rejected materials; without Draw() in
	// that phase it reports "No render states in shader" and loses the map.
	if ( !supported && !pShaderShadow )
		return;
	DYNAMIC_STATE
	{
		BindTexture( SHADER_SAMPLER0, BASETEXTURE, FRAME );
		BindTexture( SHADER_SAMPLER10, MRAOTEXTURE, 0 );
		if ( normalMap )
			BindTexture( SHADER_SAMPLER1, BUMPMAP, 0 );
		if ( emission )
			BindTexture( SHADER_SAMPLER2, EMISSIONTEXTURE, 0 );
		if ( envmap )
			BindTexture( SHADER_SAMPLER3, ENVMAP, 0 );
		// c0: transmission, IOR, thickness, and 1 for glass.
		const float glassConstants[4] = { transmission, ior, thickness, glass ? 1.0f : 0.0f };
		pShaderAPI->SetPixelShaderConstant( 0, glassConstants, 1 );
		// c2: $emissionscale, $clearcoat, $clearcoatroughness; c3.x: the native
		// feature flags.
		const float emissionConstants[4] = { emissionScale, clearCoat, clearCoatRoughness, 0.0f };
		pShaderAPI->SetPixelShaderConstant( 2, emissionConstants, 1 );
		const float featureConstants[4] = { static_cast<float>( features ), 0.0f, 0.0f, 0.0f };
		pShaderAPI->SetPixelShaderConstant( 3, featureConstants, 1 );
		// Model lighting: the ambient cube and the sorted local lights, as
		// skin_dx9_helper.cpp commits them. WMSH batches ignore both.
		LightState_t lightState = { 0, false, false };
		pShaderAPI->GetDX9LightState( &lightState );
		pShaderAPI->SetPixelShaderStateAmbientLightCube(
		    PSREG_AMBIENT_CUBE, !lightState.m_bAmbientLight );
		pShaderAPI->CommitPixelShaderLighting( PSREG_LIGHT_INFO_ARRAY );
	}
	Draw();
}
END_SHADER
