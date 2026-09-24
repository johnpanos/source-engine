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
	kTransmission,
	kIndexOfRefraction,
	kThickness,
	kClearCoat,
	kClearCoatRoughness,
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
    // Glass: the fraction of light refracted through the surface rather than
    // diffusely reflected, the dielectric's index of refraction, and the
    // distance light travels through it in world units (0 = a thin sheet,
    // which passes light straight through). $basetexture tints what passes.
    { "$transmission", ParameterKind::kFloat, ColorEncoding::kNotApplicable, false, "0" },
    { "$ior", ParameterKind::kFloat, ColorEncoding::kNotApplicable, false, "1.5" },
    { "$thickness", ParameterKind::kFloat, ColorEncoding::kNotApplicable, false, "0" },
    // Clear coat: a thin dielectric layer (IOR 1.5) over the base, its weight
    // and its own perceptual roughness. 0 is no coat.
    { "$clearcoat", ParameterKind::kFloat, ColorEncoding::kNotApplicable, false, "0" },
    { "$clearcoatroughness", ParameterKind::kFloat, ColorEncoding::kNotApplicable, false, "0.03" },
};

static_assert( sizeof( kMaterialParameters ) / sizeof( kMaterialParameters[0] ) ==
                   static_cast<unsigned>( MaterialParameter::kCount ),
    "PBR material parameter table must match its enum" );

inline const MaterialParameterSpec &Parameter( MaterialParameter id )
{
	return kMaterialParameters[static_cast<unsigned>( id )];
}

// The accepted glass values: transmission in [0, 1], an index of refraction in
// [1, 3] (air to beyond diamond), and a finite, non-negative thickness. NaN
// fails every comparison and so every range.
static constexpr float kMaxIndexOfRefraction = 3.0f;

inline bool IsValidTransmission( float transmission, float ior, float thickness )
{
	return transmission >= 0.0f && transmission <= 1.0f && ior >= 1.0f &&
	       ior <= kMaxIndexOfRefraction && thickness >= 0.0f && thickness <= 1.0e6f;
}

// The features a PBRMetalRough material asks of the native Vulkan pipelines,
// as its dynamic state reports them in pixel constant c3.x. A pipeline that
// lacks one declines the draw by name rather than drawing without it.
enum NativeFeature : int
{
	kNativeNormalMap = 1,
	kNativeEmission = 2,
	kNativeEnvMap = 4,
	kNativeTranslucent = 16, // alpha-blended, not glass
	kNativeClearCoat = 32
};

// The accepted clear coat values: both in [0, 1]. NaN fails both ranges.
inline bool IsValidClearCoat( float weight, float roughness )
{
	return weight >= 0.0f && weight <= 1.0f && roughness >= 0.0f && roughness <= 1.0f;
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

// The legacy material loader constructs materials/<reference>.vmt. PBR
// fallbacks therefore use a relative, extensionless virtual material name.
inline bool IsValidFallbackReference( const char *name )
{
	if ( !name || !name[0] )
		return false;
	std::size_t segmentStart = 0;
	std::size_t length = 0;
	for ( ; name[length]; ++length )
	{
		const char c = name[length];
		if ( c == '\\' || c == ':' || static_cast<unsigned char>( c ) < 32 )
			return false;
		if ( c != '/' )
			continue;
		const std::size_t segmentLength = length - segmentStart;
		if ( segmentLength == 0 || ( segmentLength == 1 && name[segmentStart] == '.' ) ||
		     ( segmentLength == 2 && name[segmentStart] == '.' && name[segmentStart + 1] == '.' ) )
			return false;
		segmentStart = length + 1;
	}
	const std::size_t segmentLength = length - segmentStart;
	if ( segmentLength == 0 || ( segmentLength == 1 && name[segmentStart] == '.' ) ||
	     ( segmentLength == 2 && name[segmentStart] == '.' && name[segmentStart + 1] == '.' ) )
		return false;
	const char prefix[] = "materials/";
	const char suffix[] = ".vmt";
	auto same = []( char a, char b )
	{
		const char lower = a >= 'A' && a <= 'Z' ? static_cast<char>( a - 'A' + 'a' ) : a;
		return lower == b;
	};
	if ( length >= sizeof( prefix ) - 1 )
	{
		bool hasPrefix = true;
		for ( std::size_t i = 0; i < sizeof( prefix ) - 1; ++i )
			hasPrefix &= same( name[i], prefix[i] );
		if ( hasPrefix )
			return false;
	}
	if ( length >= sizeof( suffix ) - 1 )
	{
		bool hasSuffix = true;
		for ( std::size_t i = 0; i < sizeof( suffix ) - 1; ++i )
			hasSuffix &= same( name[length - sizeof( suffix ) + 1 + i], suffix[i] );
		if ( hasSuffix )
			return false;
	}
	return true;
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
