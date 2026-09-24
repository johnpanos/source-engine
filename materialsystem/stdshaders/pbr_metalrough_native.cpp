//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Canonical PBRMetalRough material binding for native Vulkan worlds.
//          The shader API owns the WMSH PBR pipelines; this material owns its
//          Source parameters, texture loading and snapshot sampler contract.
//          A material with $transmission > 0 is glass: translucent, drawn
//          without blending or depth writes by the WMSH glass pipeline, which
//          reads its $transmission, $ior and $thickness from pixel constant c0.
//
//===========================================================================//

#include "BaseVSShader.h"
#include "render/pbr_material_schema.h"

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
SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_ENVMAP, "", "Reflection probe" )
SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.7", "Alpha cutoff" )
SHADER_PARAM( TRANSMISSION, SHADER_PARAM_TYPE_FLOAT, "0", "Refracted fraction (glass)" )
SHADER_PARAM( IOR, SHADER_PARAM_TYPE_FLOAT, "1.5", "Index of refraction" )
SHADER_PARAM( THICKNESS, SHADER_PARAM_TYPE_FLOAT, "0", "World units through; 0 = thin sheet" )
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	using render::pbr::MaterialParameter;
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
	if ( !params[TRANSMISSION]->IsDefined() )
		params[TRANSMISSION]->SetFloatValue( SchemaDefault( MaterialParameter::kTransmission ) );
	if ( !params[IOR]->IsDefined() )
		params[IOR]->SetFloatValue( SchemaDefault( MaterialParameter::kIndexOfRefraction ) );
	if ( !params[THICKNESS]->IsDefined() )
		params[THICKNESS]->SetFloatValue( SchemaDefault( MaterialParameter::kThickness ) );
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
	if ( params[EMISSIONTEXTURE]->IsDefined() || params[ENVMAP]->IsDefined() ||
	     ( IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ) && !( transmission > 0.0f ) ) )
		Warning( "PBRMetalRough material %s requests an unsupported native PBR feature\n",
		    pMaterialName );
	if ( params[BASETEXTURE]->IsDefined() )
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
	if ( params[MRAOTEXTURE]->IsDefined() )
		LoadTexture( MRAOTEXTURE, 0 );
	if ( params[BUMPMAP]->IsDefined() )
		LoadBumpMap( BUMPMAP );
}

SHADER_DRAW
{
	const float transmission = params[TRANSMISSION]->GetFloatValue();
	const float ior = params[IOR]->GetFloatValue();
	const float thickness = params[THICKNESS]->GetFloatValue();
	const bool glass = transmission > 0.0f;
	// Missing required textures cannot borrow a previous material's GPU
	// binding. The shader API also validates formats before it queues a draw.
	// Alpha-blended (non-glass) translucency has no native pipeline yet.
	const bool supported = params[BASETEXTURE]->IsTexture() && params[MRAOTEXTURE]->IsTexture() &&
	                       !params[EMISSIONTEXTURE]->IsDefined() && !params[ENVMAP]->IsDefined() &&
	                       render::pbr::IsValidTransmission( transmission, ior, thickness ) &&
	                       ( glass || !IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ) );
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
		if ( params[BUMPMAP]->IsTexture() )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false );
		}
		pShaderShadow->EnableSRGBWrite( true );
		pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_NORMAL, 3, 0, 0 );
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
		if ( params[BUMPMAP]->IsTexture() )
			BindTexture( SHADER_SAMPLER1, BUMPMAP, 0 );
		// c0: transmission, IOR, thickness, and 1 for glass.
		const float glassConstants[4] = { transmission, ior, thickness, glass ? 1.0f : 0.0f };
		pShaderAPI->SetPixelShaderConstant( 0, glassConstants, 1 );
	}
	Draw();
}
END_SHADER
