//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The VMT mapping (RFC 0016 K4): which legacy shaders map onto a
//			core family and why, how each VMT key of a family is read, and the
//			families' schemas built from those rows.
//
//=============================================================================//

#include "render/device/bind_group.h"
#include "render/material/vmt_mapping.h"
#include "render/pbr_material_schema.h"

#include <cstdlib>
#include <string>

namespace render::material
{

namespace
{

// Shaders that map cleanly onto a core family. Every other shader the product
// registers (legacy_shaders.inc) runs in the legacy family.
constexpr VmtShaderRow kShaders[] = {
    { "lightmappedgeneric", "lightmapped",
        "world surfaces lit by lightmaps (flat, bumped and self-shadowed bump); the native "
        "port lightmapped.frag of lightmappedgeneric_ps2_3_x draws them" },
    { "worldvertextransition_dx9", "lightmapped",
        "two-layer displacement blends: LightmappedGeneric with $basetexture2 and the "
        "vertex-alpha blend, drawn by the same lightmapped port" },
    { "vertexlitgeneric", "vertexlit",
        "models lit by the ambient cube and vertex or per-pixel lights, including the $phong "
        "skin path (skin_vs20/skin_ps20b, vertexlit_and_unlit_generic ports)" },
    { "unlitgeneric", "unlit",
        "unlit surfaces: base texture times color, with detail and envmap; the "
        "vertexlit_and_unlit_generic port without lighting" },
    { "sprite_dx9", "unlit",
        "sprites: an unlit textured quad whose orientation and render mode are parameters "
        "(sprite_vs20/sprite_ps20b)" },
    { "pbrmetalrough", "pbr",
        "RFC 0007 metalness/roughness materials, drawn natively by world_pbr and model_pbr; "
        "the schema is render/pbr_material_schema.h" },
    { "subrect", kLegacyFamily,
        "not a shader: a sub-rectangle ($pos, $size) of another material ($material) that "
        "the material system resolves (CMaterialSubRect) and draws as that material" },
};

constexpr std::string_view kLegacyReason =
    "no core family yet: the legacy frontend runs the shader's stdshader_dx9 programs "
    "through the ports (RFC 0016 legacy family)";

// Keys every legacy-derived family reads (BaseShader's standard parameters and
// the material flags the draw state depends on); BuildKeyRows() gives each of
// kLegacyDerivedFamilies a copy.
constexpr std::string_view kLegacyDerivedFamilies[] = { "lightmapped", "vertexlit", "unlit" };

constexpr VmtKeyRow kCommonKeys[] = {
    { {}, "$basetexture", "basetexture", ValueKind::kTexture, "" },
    { {}, "$frame", "frame", ValueKind::kInt, "0" },
    { {}, "$color", "color", ValueKind::kFloat3, "[1 1 1]" },
    { {}, "$alpha", "alpha", ValueKind::kFloat, "1" },
    { {}, "$detail", "detail", ValueKind::kTexture, "" },
    { {}, "$detailscale", "detailscale", ValueKind::kFloat, "4" },
    { {}, "$detailblendmode", "detailblendmode", ValueKind::kInt, "0" },
    { {}, "$detailblendfactor", "detailblendfactor", ValueKind::kFloat, "1" },
    { {}, "$detailtint", "detailtint", ValueKind::kFloat3, "[1 1 1]" },
    { {}, "$envmap", "envmap", ValueKind::kTexture, "" },
    { {}, "$envmapmask", "envmapmask", ValueKind::kTexture, "" },
    { {}, "$envmaptint", "envmaptint", ValueKind::kFloat3, "[1 1 1]" },
    { {}, "$envmapcontrast", "envmapcontrast", ValueKind::kFloat, "0" },
    { {}, "$envmapsaturation", "envmapsaturation", ValueKind::kFloat, "1" },
    { {}, "$alphatestreference", "alphatestreference", ValueKind::kFloat, "0" },
    { {}, "$translucent", "translucent", ValueKind::kBool, "0" },
    { {}, "$alphatest", "alphatest", ValueKind::kBool, "0" },
    { {}, "$additive", "additive", ValueKind::kBool, "0" },
    { {}, "$nocull", "nocull", ValueKind::kBool, "0" },
    { {}, "$nofog", "nofog", ValueKind::kBool, "0" },
    { {}, "$ignorez", "ignorez", ValueKind::kBool, "0" },
    { {}, "$decal", "decal", ValueKind::kBool, "0" },
    { {}, "$selfillum", "selfillum", ValueKind::kBool, "0" },
    { {}, "$vertexcolor", "vertexcolor", ValueKind::kBool, "0" },
    { {}, "$vertexalpha", "vertexalpha", ValueKind::kBool, "0" },
    { {}, "$basealphaenvmapmask", "basealphaenvmapmask", ValueKind::kBool, "0" },
    { {}, "$normalmapalphaenvmapmask", "normalmapalphaenvmapmask", ValueKind::kBool, "0" },
    { {}, "$allowalphatocoverage", "allowalphatocoverage", ValueKind::kBool, "0" },
    { {}, "$znearer", "znearer", ValueKind::kBool, "0" },
    { {}, "$no_draw", "no_draw", ValueKind::kBool, "0" },
    { {}, "$model", "model", ValueKind::kBool, "0" },
    { {}, "$basetexturetransform", "basetexturetransform", ValueKind::kTransform, "" },
    { {}, "$detailtexturetransform", "detailtexturetransform", ValueKind::kTransform, "" },
    { {}, "$envmapmasktransform", "envmapmasktransform", ValueKind::kTransform, "" },
};

constexpr VmtKeyRow kLegacyDerivedKeys[] = {
    { "lightmapped", "$basetexture2", "basetexture2", ValueKind::kTexture, "" },
    { "lightmapped", "$frame2", "frame2", ValueKind::kInt, "0" },
    { "lightmapped", "$bumpmap", "bumpmap", ValueKind::kTexture, "" },
    { "lightmapped", "$bumpframe", "bumpframe", ValueKind::kInt, "0" },
    { "lightmapped", "$bumpmap2", "bumpmap2", ValueKind::kTexture, "" },
    { "lightmapped", "$ssbump", "ssbump", ValueKind::kBool, "0" },
    // Portal 2's LightmappedGeneric parameters this SDK's shader does not
    // declare (the backend's port reads them from the material).
    { "lightmapped", "$ssbumpmathfix", "ssbumpmathfix", ValueKind::kBool, "0" },
    { "lightmapped", "$envmaplightscale", "envmaplightscale", ValueKind::kFloat, "0" },
    { "lightmapped", "$envmaplightscaleminmax", "envmaplightscaleminmax", ValueKind::kFloat2,
        "[0 1]" },
    { "lightmapped", "$blendmodulatetexture", "blendmodulatetexture", ValueKind::kTexture, "" },
    { "lightmapped", "$seamless_scale", "seamless_scale", ValueKind::kFloat, "0" },
    { "lightmapped", "$fresnelreflection", "fresnelreflection", ValueKind::kFloat, "1" },
    { "lightmapped", "$nodiffusebumplighting", "nodiffusebumplighting", ValueKind::kBool, "0" },
    { "lightmapped", "$basetexturenoenvmap", "basetexturenoenvmap", ValueKind::kBool, "0" },
    { "lightmapped", "$basetexture2noenvmap", "basetexture2noenvmap", ValueKind::kBool, "0" },
    { "lightmapped", "$selfillumtint", "selfillumtint", ValueKind::kFloat3, "[1 1 1]" },
    { "lightmapped", "$masked", "masked", ValueKind::kBool, "0" },
    { "lightmapped", "$bumptransform", "bumptransform", ValueKind::kTransform, "" },
    { "lightmapped", "$basetexturetransform2", "basetexturetransform2", ValueKind::kTransform, "" },
    { "lightmapped", "$blendmasktransform", "blendmasktransform", ValueKind::kTransform, "" },

    { "vertexlit", "$bumpmap", "bumpmap", ValueKind::kTexture, "" },
    { "vertexlit", "$bumpframe", "bumpframe", ValueKind::kInt, "0" },
    { "vertexlit", "$selfillumtint", "selfillumtint", ValueKind::kFloat3, "[1 1 1]" },
    { "vertexlit", "$selfillummask", "selfillummask", ValueKind::kTexture, "" },
    { "vertexlit", "$color2", "color2", ValueKind::kFloat3, "[1 1 1]" },
    { "vertexlit", "$halflambert", "halflambert", ValueKind::kBool, "0" },
    { "vertexlit", "$phong", "phong", ValueKind::kBool, "0" },
    { "vertexlit", "$phongexponent", "phongexponent", ValueKind::kFloat, "5" },
    { "vertexlit", "$phongboost", "phongboost", ValueKind::kFloat, "1" },
    { "vertexlit", "$phongtint", "phongtint", ValueKind::kFloat3, "[1 1 1]" },
    { "vertexlit", "$phongfresnelranges", "phongfresnelranges", ValueKind::kFloat3, "[0 0.5 1]" },
    { "vertexlit", "$phongexponenttexture", "phongexponenttexture", ValueKind::kTexture, "" },
    { "vertexlit", "$phongalbedotint", "phongalbedotint", ValueKind::kBool, "0" },
    { "vertexlit", "$basemapalphaphongmask", "basemapalphaphongmask", ValueKind::kBool, "0" },
    { "vertexlit", "$invertphongmask", "invertphongmask", ValueKind::kBool, "0" },
    { "vertexlit", "$lightwarptexture", "lightwarptexture", ValueKind::kTexture, "" },
    { "vertexlit", "$envmapfresnel", "envmapfresnel", ValueKind::kFloat, "0" },
    { "vertexlit", "$rimlight", "rimlight", ValueKind::kBool, "0" },
    { "vertexlit", "$rimlightexponent", "rimlightexponent", ValueKind::kFloat, "4" },
    { "vertexlit", "$rimlightboost", "rimlightboost", ValueKind::kFloat, "1" },
    { "vertexlit", "$rimmask", "rimmask", ValueKind::kBool, "0" },
    { "vertexlit", "$ambientonly", "ambientonly", ValueKind::kBool, "0" },
    { "vertexlit", "$bumptransform", "bumptransform", ValueKind::kTransform, "" },

    { "unlit", "$vertexalphatest", "vertexalphatest", ValueKind::kBool, "0" },
    { "unlit", "$hdrcolorscale", "hdrcolorscale", ValueKind::kFloat, "1" },
    { "unlit", "$depthblend", "depthblend", ValueKind::kBool, "0" },
    { "unlit", "$depthblendscale", "depthblendscale", ValueKind::kFloat, "50" },
    { "unlit", "$spriteorigin", "spriteorigin", ValueKind::kFloat3, "[0 0 0]" },
    { "unlit", "$spriteorientation", "spriteorientation", ValueKind::kEnum,
        "parallel_upright|facing_upright|vp_parallel|oriented|vp_parallel_oriented" },
    { "unlit", "$spriterendermode", "spriterendermode", ValueKind::kInt, "0" },
    { "unlit", "$ignorevertexcolors", "ignorevertexcolors", ValueKind::kBool, "1" },
};

constexpr VmtMetadataRow kMetadata[] = {
    { "$surfaceprop", "physics surface properties (the physics and sound systems)" },
    { "$surfaceprop2", "physics surface properties of a blend's second layer" },
    { "$decalscale", "decal projection size (the decal system)" },
    { "$reflectivity", "radiosity reflectivity (vrad)" },
    { "$fallbackmaterial", "the material a profile without the shader draws instead" },
    { "$envmapsphere", "a DirectX 6 envmap mode no dx9 shader reads" },
    { "$modelmaterial", "the material a model draws when the world material is used on a model" },
    { "$bottommaterial", "water: the material seen from below" },
    { "$underwateroverlay", "water: the screen overlay drawn under water" },
    { "$nodecal", "decals are not applied (the decal system)" },
    { "$nofullbright", "mat_fullbright ignores the material" },
    { "$no_fullbright", "mat_fullbright ignores the material" },
    { "$surfaceprop_override", "physics surface properties" },
    { "$detailtype", "the detail-sprite set vbsp places on the surface" },
    { "$minsize", "particle size limits read by the particle system" },
    { "$maxsize", "particle size limits read by the particle system" },
    { "$maxdistance", "particle distance limit read by the particle system" },
    { "$farfadeinterval", "particle fade read by the particle system" },
};

#include "legacy_shaders.inc"

constexpr std::string_view kPbrFamily = "pbr";

ValueKind PbrKind( pbr::ParameterKind kind )
{
	switch ( kind )
	{
	case pbr::ParameterKind::kTexture:
		return ValueKind::kTexture;
	case pbr::ParameterKind::kFloat:
		return ValueKind::kFloat;
	case pbr::ParameterKind::kBoolean:
		return ValueKind::kBool;
	case pbr::ParameterKind::kMaterialReference:
		return ValueKind::kMaterial;
	}
	return ValueKind::kFloat;
}

// Each legacy-derived family's common rows and then its own; the PBR rows come
// from RFC 0007's schema, their parameter names dropping the '$'.
std::vector<VmtKeyRow> BuildKeyRows()
{
	std::vector<VmtKeyRow> rows;
	for ( const std::string_view family : kLegacyDerivedFamilies )
	{
		for ( VmtKeyRow row : kCommonKeys )
		{
			row.family = family;
			rows.push_back( row );
		}
		for ( const VmtKeyRow &row : kLegacyDerivedKeys )
		{
			if ( row.family == family )
				rows.push_back( row );
		}
	}
	for ( const pbr::MaterialParameterSpec &spec : pbr::kMaterialParameters )
	{
		const std::string_view key = spec.name;
		rows.push_back( { kPbrFamily, key, key.substr( 1 ), PbrKind( spec.kind ),
		    spec.defaultValue ? std::string_view( spec.defaultValue ) : std::string_view() } );
	}
	return rows;
}

ParameterType TypeOf( ValueKind kind )
{
	switch ( kind )
	{
	case ValueKind::kTransform:
		return ParameterType::kTransform;
	case ValueKind::kTexture:
		return ParameterType::kTexture;
	case ValueKind::kFloat:
		return ParameterType::kFloat;
	case ValueKind::kFloat2:
		return ParameterType::kFloat2;
	case ValueKind::kFloat3:
		return ParameterType::kFloat3;
	case ValueKind::kFloat4:
		return ParameterType::kFloat4;
	case ValueKind::kInt:
	case ValueKind::kBool:
	case ValueKind::kEnum:
	case ValueKind::kMaterial:
		return ParameterType::kInt;
	}
	return ParameterType::kFloat;
}

// A row's default as numbers: the leading numbers of its text ("[1 1 1]",
// "0.5"); an enum's default is its first name, index 0.
void DefaultOf( const VmtKeyRow &row, float ( &out )[8] )
{
	if ( row.kind == ValueKind::kEnum || row.kind == ValueKind::kTexture )
		return;
	if ( row.kind == ValueKind::kTransform )
	{
		out[0] = out[5] = 1.0f; // the identity's rows 0 and 1
		return;
	}
	const std::string text( row.fallback );
	const char *cursor = text.c_str();
	int count = 0;
	while ( count < 4 && *cursor )
	{
		while ( *cursor == ' ' || *cursor == '[' || *cursor == ']' )
			++cursor;
		if ( !*cursor )
			break;
		char *end = nullptr;
		const float value = std::strtof( cursor, &end );
		if ( end == cursor )
			break;
		out[count++] = value;
		cursor = end;
	}
	for ( int i = count; count > 0 && i < 4; ++i )
		out[i] = out[count - 1];
}

} // namespace

const VmtMappingTable &BuiltinVmtMapping()
{
	static const std::vector<VmtKeyRow> s_Keys = BuildKeyRows();
	static const VmtMappingTable s_Table{
	    kShaders, s_Keys, kMetadata, kLegacyShaders, kLegacyReason };
	return s_Table;
}

std::vector<FamilyDesc> FamiliesFromMapping( const VmtMappingTable &mapping )
{
	std::vector<FamilyDesc> families;
	for ( const VmtShaderRow &shader : mapping.shaders )
	{
		bool known = shader.family == kLegacyFamily; // defined by the legacy frontend
		for ( const FamilyDesc &family : families )
			known = known || family.name == shader.family;
		if ( known )
			continue;
		FamilyDesc family;
		family.name = std::string( shader.family );
		family.bindGroups = device::kMaxBindGroups;
		for ( const VmtKeyRow &row : mapping.keys )
		{
			if ( row.family != shader.family || row.kind == ValueKind::kMaterial )
				continue;
			ParameterDesc parameter;
			parameter.name = std::string( row.parameter );
			parameter.type = TypeOf( row.kind );
			DefaultOf( row, parameter.defaults );
			family.parameters.push_back( std::move( parameter ) );
		}
		families.push_back( std::move( family ) );
	}
	return families;
}

} // namespace render::material
