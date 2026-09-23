//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Version 1 VMT parameter contract for RFC 0007 PBRMetalRough.
//          Material loading, editor inspection and content validation use
//          these names and color encodings as one authority.
//
//===========================================================================//

#ifndef RENDER_PBR_MATERIAL_SCHEMA_H
#define RENDER_PBR_MATERIAL_SCHEMA_H

#include <cstddef>

namespace render
{
namespace pbr
{

enum class MaterialParameter : unsigned
{
	kBaseTexture,
	kMraoTexture,
	kBumpMap,
	kEmissionTexture,
	kEmissionScale,
	kAlphaTest,
	kAlphaTestReference,
	kTranslucent,
	kEnvMap,
	kFallbackMaterial,
	kCount
};

enum class ParameterKind
{
	kTexture,
	kFloat,
	kBoolean,
	kMaterialReference
};

enum class ColorEncoding
{
	kNotApplicable,
	kSrgb,
	kLinear
};

struct MaterialParameterSpec
{
	const char *name;
	ParameterKind kind;
	ColorEncoding encoding;
	bool required;
	const char *defaultValue;
};

// The order is MaterialParameter. A missing optional texture has no default;
// scalar defaults match the legacy material flags where applicable.
static constexpr MaterialParameterSpec kMaterialParameters[] = {
    { "$basetexture", ParameterKind::kTexture, ColorEncoding::kSrgb, true, nullptr },
    { "$mraotexture", ParameterKind::kTexture, ColorEncoding::kLinear, true, nullptr },
    { "$bumpmap", ParameterKind::kTexture, ColorEncoding::kLinear, false, nullptr },
    { "$emissiontexture", ParameterKind::kTexture, ColorEncoding::kSrgb, false, nullptr },
    { "$emissionscale", ParameterKind::kFloat, ColorEncoding::kNotApplicable, false, "1" },
    { "$alphatest", ParameterKind::kBoolean, ColorEncoding::kNotApplicable, false, "0" },
    { "$alphatestreference", ParameterKind::kFloat, ColorEncoding::kNotApplicable, false, nullptr },
    { "$translucent", ParameterKind::kBoolean, ColorEncoding::kNotApplicable, false, "0" },
    { "$envmap", ParameterKind::kTexture, ColorEncoding::kLinear, false, nullptr },
    { "$fallbackmaterial", ParameterKind::kMaterialReference, ColorEncoding::kNotApplicable, true,
        nullptr },
};

static_assert( sizeof( kMaterialParameters ) / sizeof( kMaterialParameters[0] ) ==
                   static_cast<unsigned>( MaterialParameter::kCount ),
    "PBR material parameter table must match its enum" );

inline const MaterialParameterSpec &Parameter( MaterialParameter id )
{
	return kMaterialParameters[static_cast<unsigned>( id )];
}

inline bool IsMetalRoughShader( const char *name )
{
	const char expected[] = "PBRMetalRough";
	if ( !name )
		return false;
	for ( std::size_t i = 0; i < sizeof( expected ); ++i )
	{
		const char a = name[i];
		const char b = expected[i];
		const char lower = a >= 'A' && a <= 'Z' ? static_cast<char>( a - 'A' + 'a' ) : a;
		const char expectedLower = b >= 'A' && b <= 'Z' ? static_cast<char>( b - 'A' + 'a' ) : b;
		if ( lower != expectedLower )
			return false;
		if ( a == '\0' )
			return true;
	}
	return false;
}

enum class DefinitionStatus
{
	kValid,
	kNotPbr,
	kMissingRequiredParameter
};

struct DefinitionResult
{
	DefinitionStatus status;
	const char *parameter;
};

using ParameterLookup = const char *(*)( const char *name, void *context );

inline DefinitionResult ValidateDefinition(
    const char *shader, ParameterLookup lookup, void *context )
{
	if ( !IsMetalRoughShader( shader ) )
		return { DefinitionStatus::kNotPbr, nullptr };
	for ( const MaterialParameterSpec &spec : kMaterialParameters )
	{
		if ( !spec.required )
			continue;
		const char *value = lookup ? lookup( spec.name, context ) : nullptr;
		if ( !value || !value[0] )
			return { DefinitionStatus::kMissingRequiredParameter, spec.name };
	}
	return { DefinitionStatus::kValid, nullptr };
}

} // namespace pbr
} // namespace render

#endif // RENDER_PBR_MATERIAL_SCHEMA_H
