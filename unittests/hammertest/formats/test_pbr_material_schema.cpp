//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0007 PBR VMT schema and fallback-reference conformance.
//
//===========================================================================//

#include "hammer/formats/material_catalog.h"
#include "hammer/ports/asset_source.h"
#include "render/pbr_material_schema.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace
{
int g_checks = 0;
int g_failures = 0;

class MemoryAssets : public hammer::ports::IAssetSource
{
public:
	std::map<std::string, std::string> assets;

	bool HasAsset( const std::string &path ) const override { return assets.count( path ) != 0; }
	bool ReadAsset( const std::string &path, std::string &out ) const override
	{
		const auto found = assets.find( path );
		if ( found == assets.end() )
			return false;
		out = found->second;
		return true;
	}
	void ListAssets( const std::string &prefix, const std::string &extension,
	    std::vector<std::string> &out ) const override
	{
		for ( const auto &entry : assets )
		{
			if ( entry.first.compare( 0, prefix.size(), prefix ) == 0 &&
			     entry.first.size() >= extension.size() &&
			     entry.first.compare(
			         entry.first.size() - extension.size(), extension.size(), extension ) == 0 )
				out.push_back( entry.first );
		}
	}
};

void Check( bool condition, const char *message )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL: %s\n", message );
	}
}

bool SupportsLegacyWorldShader( const std::string &shader, void * )
{
	return shader == "LightmappedGeneric";
}

bool RejectEveryShader( const std::string &, void * )
{
	return false;
}

std::string PbrVmt( const char *fallback )
{
	return std::string( "\"pbrmetalrough\" { \"$basetexture\" \"brick/red\" "
	                    "\"$mraotexture\" \"brick/red_mrao\" \"$fallbackmaterial\" \"" ) +
	       fallback + "\" }";
}

void CheckStatus( hammer::formats::MaterialCatalog &catalog, const char *material,
    hammer::formats::PbrMaterialStatus expected, const char *message )
{
	const auto result = catalog.ValidatePbrMaterial( material, SupportsLegacyWorldShader, nullptr );
	Check( result.status == expected, message );
}
} // namespace

int main()
{
	using namespace render::pbr;
	using hammer::formats::MaterialCatalog;
	using hammer::formats::PbrMaterialStatus;

	Check( IsMetalRoughShader( "PBRMetalRough" ) && IsMetalRoughShader( "pbrmetalrough" ),
	    "shader family lookup ignores VMT case" );
	Check( !IsMetalRoughShader( "PBR" ) && !IsMetalRoughShader( "PBRMetalRoughExtra" ),
	    "old PBR and prefixed names remain distinct" );
	Check( Parameter( MaterialParameter::kBaseTexture ).encoding == ColorEncoding::kSrgb &&
	           Parameter( MaterialParameter::kMraoTexture ).encoding == ColorEncoding::kLinear &&
	           Parameter( MaterialParameter::kEmissionTexture ).encoding == ColorEncoding::kSrgb &&
	           Parameter( MaterialParameter::kBumpMap ).encoding == ColorEncoding::kLinear,
	    "PBR texture slots have one declared color encoding" );
	Check( Parameter( MaterialParameter::kFallbackMaterial ).required &&
	           Parameter( MaterialParameter::kFallbackMaterial ).kind ==
	               ParameterKind::kMaterialReference,
	    "fallback is a required VMT reference" );
	Check( IsValidFallbackReference( "compat/red" ) && !IsValidFallbackReference( "../outside" ) &&
	           !IsValidFallbackReference( "compat//red" ) &&
	           !IsValidFallbackReference( "compat\\red" ) &&
	           !IsValidFallbackReference( "materials/compat/red" ) &&
	           !IsValidFallbackReference( "compat/red.vmt" ),
	    "PBR fallback references use relative virtual material paths" );

	MemoryAssets source;
	source.assets["materials/pbr/red.vmt"] = PbrVmt( "compat/red" );
	source.assets["materials/pbr/blue.vmt"] =
	    "\"patch\" { \"include\" \"pbr/red\" \"replace\" { \"$mraotexture\" "
	    "\"brick/blue_mrao\" } }";
	source.assets["materials/pbr/nested_outer.vmt"] =
	    "\"patch\" { \"include\" \"pbr/nested_inner\" \"replace\" { "
	    "\"$fallbackmaterial\" \"compat/unsupported\" } }";
	source.assets["materials/pbr/nested_inner.vmt"] =
	    "\"patch\" { \"include\" \"pbr/red\" \"replace\" { "
	    "\"$fallbackmaterial\" \"compat/red\" } }";
	source.assets["materials/pbr/replace_missing.vmt"] =
	    "\"patch\" { \"include\" \"pbr/no_mrao\" \"replace\" { "
	    "\"$mraotexture\" \"brick/replace_mrao\" } }";
	source.assets["materials/pbr/insert_missing.vmt"] =
	    "\"patch\" { \"include\" \"pbr/no_mrao\" \"insert\" { "
	    "\"$mraotexture\" \"brick/insert_mrao\" } }";
	source.assets["materials/pbr/patch_cycle_a.vmt"] =
	    "\"patch\" { \"include\" \"pbr/patch_cycle_b\" }";
	source.assets["materials/pbr/patch_cycle_b.vmt"] =
	    "\"patch\" { \"include\" \"pbr/patch_cycle_a\" }";
	for ( int i = 0; i < 10; ++i )
	{
		source.assets["materials/pbr/deep" + std::to_string( i ) + ".vmt"] =
		    "\"patch\" { \"include\" \"pbr/deep" + std::to_string( i + 1 ) + "\" }";
	}
	source.assets["materials/pbr/deep10.vmt"] = PbrVmt( "compat/red" );
	source.assets["materials/compat/red.vmt"] =
	    "\"patch\" { \"include\" \"materials/compat/red_base.vmt\" }";
	source.assets["materials/compat/red_base.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"brick/red\" }";
	source.assets["materials/compat/unsupported.vmt"] =
	    "\"UnlitGeneric\" { \"$basetexture\" \"brick/red\" }";
	source.assets["materials/pbr/no_mrao.vmt"] =
	    "\"PBRMetalRough\" { \"$basetexture\" \"brick/red\" "
	    "\"$fallbackmaterial\" \"compat/red\" }";
	source.assets["materials/pbr/no_fallback.vmt"] =
	    "\"PBRMetalRough\" { \"$basetexture\" \"brick/red\" "
	    "\"$mraotexture\" \"brick/red_mrao\" }";
	source.assets["materials/pbr/self.vmt"] = PbrVmt( "pbr/self" );
	source.assets["materials/pbr/missing.vmt"] = PbrVmt( "compat/missing" );
	source.assets["materials/pbr/traversal.vmt"] = PbrVmt( "../outside" );
	source.assets["materials/pbr/unsupported.vmt"] = PbrVmt( "compat/unsupported" );
	source.assets["materials/pbr/cycle.vmt"] = PbrVmt( "compat/cycle_a" );
	source.assets["materials/compat/cycle_a.vmt"] = "\"patch\" { \"include\" \"compat/cycle_b\" }";
	source.assets["materials/compat/cycle_b.vmt"] = "\"patch\" { \"include\" \"compat/cycle_a\" }";
	source.assets["materials/compat/broken.vmt"] = "\"LightmappedGeneric\" {";
	source.assets["materials/pbr/broken.vmt"] = PbrVmt( "compat/broken" );
	source.assets["materials/ordinary.vmt"] = "\"UnlitGeneric\" {}";
	MaterialCatalog catalog( source );

	const auto valid =
	    catalog.ValidatePbrMaterial( "PBR\\RED.vmt", SupportsLegacyWorldShader, nullptr );
	Check(
	    valid.status == PbrMaterialStatus::kValid && valid.fallbackShader == "LightmappedGeneric",
	    "patch include resolves to a provider-supported legacy shader" );
	CheckStatus( catalog, "pbr/blue", PbrMaterialStatus::kValid,
	    "PBR patch inherits schema fields and overrides its MRAO texture" );
	CheckStatus( catalog, "pbr/nested_outer", PbrMaterialStatus::kValid,
	    "nested patch values follow the legacy accumulation order" );
	CheckStatus( catalog, "pbr/replace_missing", PbrMaterialStatus::kMissingParameter,
	    "replace does not invent a missing PBR field" );
	CheckStatus( catalog, "pbr/insert_missing", PbrMaterialStatus::kValid,
	    "insert supplies a missing PBR field" );
	CheckStatus( catalog, "pbr/patch_cycle_a", PbrMaterialStatus::kMaterialCycle,
	    "primary PBR patch cycle is rejected" );
	CheckStatus( catalog, "pbr/deep0", PbrMaterialStatus::kMaterialChainTooDeep,
	    "PBR patch chain beyond the legacy loader's bound is rejected" );
	CheckStatus( catalog, "ordinary", PbrMaterialStatus::kNotPbr,
	    "ordinary material is outside the PBR schema" );
	CheckStatus( catalog, "pbr/no_mrao", PbrMaterialStatus::kMissingParameter,
	    "missing linear MRAO texture is rejected" );
	CheckStatus( catalog, "pbr/no_fallback", PbrMaterialStatus::kMissingParameter,
	    "missing fallback declaration is rejected" );
	CheckStatus(
	    catalog, "pbr/self", PbrMaterialStatus::kSelfFallback, "self fallback is rejected" );
	CheckStatus( catalog, "pbr/missing", PbrMaterialStatus::kFallbackMissing,
	    "missing fallback VMT is rejected" );
	CheckStatus( catalog, "pbr/traversal", PbrMaterialStatus::kInvalidFallbackPath,
	    "fallback path traversal is rejected" );
	CheckStatus( catalog, "pbr/unsupported", PbrMaterialStatus::kUnsupportedFallbackShader,
	    "unsupported fallback shader is rejected" );
	CheckStatus( catalog, "pbr/cycle", PbrMaterialStatus::kFallbackCycle,
	    "patch include cycle is rejected" );
	CheckStatus( catalog, "pbr/broken", PbrMaterialStatus::kFallbackMalformed,
	    "malformed fallback VMT is rejected" );
	CheckStatus( catalog, "pbr/absent", PbrMaterialStatus::kMaterialMissing,
	    "missing primary VMT is rejected" );
	CheckStatus( catalog, "../outside", PbrMaterialStatus::kInvalidMaterialPath,
	    "primary material path traversal is rejected" );
	const auto missingMrao =
	    catalog.ValidatePbrMaterial( "pbr/no_mrao", SupportsLegacyWorldShader, nullptr );
	Check( missingMrao.parameter && std::string( missingMrao.parameter ) == "$mraotexture",
	    "missing field is diagnosed with the schema-owned parameter name" );
	Check( catalog.ValidatePbrMaterial( "pbr/red", RejectEveryShader, nullptr ).status ==
	           PbrMaterialStatus::kUnsupportedFallbackShader,
	    "provider rejection is authoritative" );
	Check( catalog.ValidatePbrMaterial( "pbr/red", nullptr, nullptr ).status ==
	           PbrMaterialStatus::kUnsupportedFallbackShader,
	    "missing provider claim cannot certify fallback" );

	return testing::ReportConformance( g_checks, g_failures );
}
