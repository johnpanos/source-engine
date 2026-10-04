//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.v2 (RFC 0016 K4 "Material suite"): schema
//			layout, registration rules, parameter blocks and revisions, derived
//			copies, capability requirements, the built-in families from the VMT
//			mapping, the key-mapping oracle and VMT import. Seeded bad mappings
//			and stale copies are test_material_sensitivity.cpp. The proxy corpus
//			and the family ports are later K4 work.
//
//=============================================================================//

#include "material_conformance.h"
#include "render/device/bind_group.h"
#include "render/material/material.h"
#include "render/material/registry.h"
#include "render/material/scene_terms.h"
#include "render/pbr_material_schema.h"
#include "testing/checks.h"

#include <cmath>
#include <cstring>
#include <map>

namespace
{

using namespace render::material;

std::string Joined( const std::vector<std::string> &items )
{
	std::string out;
	for ( const std::string &item : items )
		out += ( out.empty() ? "" : "; " ) + item;
	return out;
}

const MaterialValue *ValueOf( const MaterialDesc &material, std::string_view parameter )
{
	for ( const MaterialValue &value : material.values )
	{
		if ( value.parameter == parameter )
			return &value;
	}
	return nullptr;
}

const VmtPair *VariableOf( const MaterialDesc &material, std::string_view key )
{
	for ( const VmtPair &variable : material.variables )
	{
		if ( variable.key == key )
			return &variable;
	}
	return nullptr;
}

void TestSceneTerms( testing::Checks &checks )
{
	SceneTermInputs inputs;
	inputs.runtimeDirect = true;
	inputs.totalDirectionalLightmap = true;
	inputs.probeBounce = true;
	checks.That( SceneTerms( inputs ) == ( kSurfaceClustered | kSurfaceDirectionalLightmap ),
	    "S1.runtime-direct-needs-indirect-and-bounce-needs-probes" );
	inputs.indirectLightmap = true;
	checks.That( SceneTerms( inputs ) == ( kSurfaceClustered | kSurfaceRuntimeDirect ),
	    "S1.runtime-direct-does-not-use-the-total-gradient" );
	inputs.indirectDirectionalLightmap = true;
	inputs.probeVolume = true;
	inputs.reflectionProbes = true;
	inputs.ambientOcclusion = true;
	checks.That( SceneTerms( inputs ) ==
	                 ( kSurfaceClustered | kSurfaceRuntimeDirect | kSurfaceDirectionalLightmap |
	                     kSurfaceProbeVolume | kSurfaceProbeBounce | kSurfaceReflectionProbes |
	                     kSurfaceAmbientOcclusion ),
	    "S1.lab-and-game-select-the-same-complete-scene-variant" );
}

// A resolver over a fixed set of files.
VmtResolver Files( std::map<std::string, std::string, std::less<>> files )
{
	return [files = std::move( files )]( std::string_view path ) -> std::optional<std::string>
	{
		const auto found = files.find( path );
		if ( found == files.end() )
			return std::nullopt;
		return found->second;
	};
}

void TestFamiliesFromMapping( testing::Checks &checks )
{
	const VmtMappingTable &mapping = BuiltinVmtMapping();
	const std::vector<FamilyDesc> families = FamiliesFromMapping( mapping );
	std::vector<std::string> names;
	for ( const FamilyDesc &family : families )
		names.push_back( family.name );
	checks.That( names == std::vector<std::string>{ "lightmapped", "vertexlit", "unlit", "cable",
	                          "pbr", "water", "refract", "depth", "portal-mask" },
	    "F1.the-mapping-defines-the-core-families-in-order" );
	FamilyRegistry registry;
	bool registered = true;
	bool fourGroups = true;
	for ( const FamilyDesc &family : families )
	{
		registered = registered && registry.Register( family ).HasValue();
		fourGroups = fourGroups && family.bindGroups == render::device::kMaxBindGroups;
	}
	checks.That( registered, "F1.every-family-registers" );
	checks.That( fourGroups, "F1.every-family-binds-the-four-groups" );
	// Every key row is a parameter of its family with the row's type, and every
	// parameter has a row: the schema is the rows.
	bool rowsAreParameters = true;
	std::size_t gpuRows = 0;
	for ( const VmtKeyRow &row : mapping.keys )
	{
		if ( row.kind == ValueKind::kMaterial )
			continue;
		++gpuRows;
		const FamilySchema *schema = registry.Find( row.family );
		const auto index = schema ? schema->IndexOf( row.parameter ) : std::nullopt;
		rowsAreParameters = rowsAreParameters && index.has_value();
	}
	std::size_t parameters = 0;
	for ( const FamilyDesc &family : families )
		parameters += family.parameters.size();
	checks.That( rowsAreParameters && parameters == gpuRows, "F2.the-schema-is-the-key-rows" );
	const FamilySchema *pbr = registry.Find( "pbr" );
	std::size_t pbrGpu = 0;
	for ( const render::pbr::MaterialParameterSpec &spec : render::pbr::kMaterialParameters )
	{
		if ( spec.kind != render::pbr::ParameterKind::kMaterialReference )
			++pbrGpu;
	}
	checks.That( pbr && pbr->desc.parameters.size() == pbrGpu && pbr->IndexOf( "mraotexture" ) &&
	                 pbr->IndexOf( "clearcoatroughness" ),
	    "F2.pbr-parameters-are-rfc-0007-s-schema" );
	const FamilySchema *lit = registry.Find( "lightmapped" );
	ParameterBlock defaults( *lit );
	float color[3] = {};
	float alpha = 0.0f;
	std::memcpy( color, defaults.Bytes().data() + lit->layout[*lit->IndexOf( "color" )].offset,
	    sizeof( color ) );
	std::memcpy( &alpha, defaults.Bytes().data() + lit->layout[*lit->IndexOf( "alpha" )].offset,
	    sizeof( alpha ) );
	checks.That( color[0] == 1.0f && color[1] == 1.0f && color[2] == 1.0f && alpha == 1.0f,
	    "F3.defaults-come-from-the-rows" );
	float ior = 0.0f;
	std::memcpy( &ior,
	    ParameterBlock( *pbr ).Bytes().data() + pbr->layout[*pbr->IndexOf( "ior" )].offset,
	    sizeof( ior ) );
	checks.Equal( ior, 1.5f, "F3.pbr-defaults-come-from-its-schema" );
	// Legacy shader names are known and resolve their fallback aliases.
	bool aliasesResolve = true;
	for ( const LegacyShaderName &shader : mapping.legacyShaders )
	{
		if ( shader.target.empty() )
			continue;
		bool found = false;
		for ( const LegacyShaderName &other : mapping.legacyShaders )
			found = found || other.name == shader.target;
		aliasesResolve = aliasesResolve && ( found || shader.name == "water" );
	}
	checks.That( mapping.legacyShaders.size() > 100 && aliasesResolve,
	    "F4.the-legacy-shader-table-is-populated-and-its-aliases-resolve" );
}

void TestKeyMapping( testing::Checks &checks )
{
	const std::vector<std::string> violations =
	    rendertest::material::KeyMappingViolations( BuiltinVmtMapping() );
	checks.That( violations.empty(), "K1.every-key-lands-in-the-parameter-it-names" );
	if ( !violations.empty() )
		std::printf( "  %s\n", Joined( violations ).c_str() );
}

void TestDerivedCopy( testing::Checks &checks )
{
	const std::vector<std::string> violations =
	    rendertest::material::DerivedCopyViolations<ParameterBlockCopy>();
	checks.That( violations.empty(), "R1.a-derived-copy-follows-its-block" );
	if ( !violations.empty() )
		std::printf( "  %s\n", Joined( violations ).c_str() );
}

void TestImportReading( testing::Checks &checks )
{
	VmtImportContext context;
	auto lit = ImportVmt( "\"LightmappedGeneric\"\n{\n"
	                      "\t\"$baseTexture\" \"Brick\\BrickWall001A.vtf\"\n"
	                      "\t\"$envmap\" \"env_cubemap\" [$X360]\n"
	                      "\t\"$envmaptint\" \"[.5 .5 .5]\" [$WIN32]\n"
	                      "\t\"$color\" \"{255 128 0}\"\n"
	                      "\t\"$alpha\" \"0.75\"\n"
	                      "\t\"$alpha\" \"0.25\"\n"
	                      "\t\"ldr?$detailscale\" \"2\"\n"
	                      "\t\"hdr?$detailtint\" \"[0 1 0]\"\n"
	                      "\t\"GPU>=2?$fresnelreflection\" \"0.5\"\n"
	                      "\t\"$surfaceprop\" \"concrete\"\n"
	                      "\t\"%keywords\" \"portal\"\n"
	                      "\t\"$notakey\" \"1\"\n"
	                      "\t\">=dx90\" { \"$selfillumtint\" \"[0 0 1]\" }\n"
	                      "\t\"<dx90\" { \"$selfillumtint\" \"[1 0 0]\" }\n"
	                      "\t\"Proxies\" { \"AnimatedTexture\" { \"animatedtexturevar\" "
	                      "\"$basetexture\" \"animatedtextureframerate\" \"12\" } }\n}\n",
	    context );
	checks.That( lit.HasValue(), "V1.a-lightmappedgeneric-vmt-imports" );
	if ( !lit )
		return;
	const MaterialDesc &m = lit.Value();
	checks.That(
	    m.family == "lightmapped" && m.legacyShader == "lightmappedgeneric" && !m.reason.empty(),
	    "V1.lightmappedgeneric-maps-to-lightmapped-with-its-reason" );
	const MaterialValue *base = ValueOf( m, "basetexture" );
	checks.That( base && base->text == "materials/brick/brickwall001a",
	    "V1.textures-take-the-materials-prefix-lower-case-and-no-extension" );
	checks.That( !ValueOf( m, "envmap" ), "V2.a-false-tag-drops-its-key" );
	const MaterialValue *tint = ValueOf( m, "envmaptint" );
	checks.That( tint && tint->numbers[0] == 0.5f && tint->numbers[2] == 0.5f,
	    "V2.a-true-tag-keeps-its-key" );
	const MaterialValue *color = ValueOf( m, "color" );
	checks.That( color && color->numbers[0] == 1.0f && color->numbers[1] == 128.0f / 255.0f &&
	                 color->numbers[2] == 0.0f,
	    "V3.braced-colors-are-0-to-255" );
	const MaterialValue *alpha = ValueOf( m, "alpha" );
	checks.That( alpha && alpha->numbers[0] == 0.75f, "V4.the-first-definition-wins" );
	checks.That( !ValueOf( m, "detailscale" ), "V2.ldr-variables-are-skipped-with-hdr" );
	checks.That( ValueOf( m, "detailtint" ), "V2.hdr-variables-apply" );
	checks.That( ValueOf( m, "fresnelreflection" ), "V2.gpu-level-variables-hold-at-level-3" );
	const MaterialValue *selfillum = ValueOf( m, "selfillumtint" );
	checks.That( m.fallbackBlock == ">=dx90" && selfillum && selfillum->numbers[2] == 1.0f &&
	                 selfillum->numbers[0] == 0.0f,
	    "V5.the-profile-s-fallback-block-overrides" );
	checks.That( m.metadata.size() == 1 && m.metadata[0].key == "$surfaceprop",
	    "V6.metadata-keys-are-kept-apart" );
	checks.That( m.editorKeys.size() == 1 && m.editorKeys[0].key == "%keywords",
	    "V6.editor-keys-are-preserved" );
	// The sky casts no shadow: %compilesky (any case) or %compile2Dsky marks it.
	checks.That( !IsSkySurface( m ), "V6.a-surface-without-a-sky-key-is-not-sky" );
	auto sky = ImportVmt( "UnlitGeneric { $basetexture sky \"%CompileSky\" 1 }", context );
	auto skybox = ImportVmt( "UnlitGeneric { $basetexture sky %compile2Dsky 1 }", context );
	auto notSky = ImportVmt( "UnlitGeneric { $basetexture sky %compilesky 0 }", context );
	checks.That( sky && IsSkySurface( sky.Value() ) && skybox && IsSkySurface( skybox.Value() ),
	    "V6.compilesky-and-compile2dsky-mark-a-sky-surface" );
	checks.That( notSky && !IsSkySurface( notSky.Value() ), "V6.compilesky-0-is-not-sky" );
	checks.That( m.unmapped == std::vector<std::string>{ "$notakey" },
	    "V6.keys-the-family-does-not-map-are-reported" );
	checks.That( m.proxies.size() == 1 && m.proxies[0].name == "AnimatedTexture" &&
	                 m.proxies[0].parameters.size() == 2 &&
	                 m.proxies[0].parameters[1].value == "12",
	    "V7.the-proxies-block-is-preserved" );

	// Level 1 skips GPU>=2 variables; dx 80 selects the <dx90 block.
	VmtImportContext old;
	old.profile.gpuLevel = 1;
	old.profile.dxLevel = 80;
	auto low =
	    ImportVmt( "LightmappedGeneric { \"GPU>=2?$alpha\" \"0.5\" \"!GPU>=2?$alpha\" \"0.4\" "
	               "\">=dx90\" { $color \"[0 0 1]\" } \"<dx90\" { $color \"[1 0 0]\" } }",
	        old );
	checks.That( low && ValueOf( low.Value(), "alpha" ) &&
	                 ValueOf( low.Value(), "alpha" )->numbers[0] == 0.4f &&
	                 low.Value().fallbackBlock == "<dx90" &&
	                 ValueOf( low.Value(), "color" )->numbers[0] == 1.0f,
	    "V5.the-profile-selects-variables-and-blocks" );

	// A conditional definition replaces an earlier one; an unconditional one does not.
	auto overridden =
	    ImportVmt( "UnlitGeneric { $alpha 0.1 \"srgb?$alpha\" 0.2 $alpha 0.3 }", context );
	checks.That( overridden && ValueOf( overridden.Value(), "alpha" )->numbers[0] == 0.2f,
	    "V4.a-conditional-definition-replaces-an-earlier-one" );

	// Aliases, legacy shaders and material kinds.
	auto wvt = ImportVmt( "WorldVertexTransition { $basetexture2 a }", context );
	checks.That( wvt && wvt.Value().family == "lightmapped" &&
	                 wvt.Value().legacyShader == "worldvertextransition_dx9",
	    "V8.fallback-aliases-resolve-before-mapping" );
	auto legacy = ImportVmt( "MotionBlur { $motionblurinternal .2 $surfaceprop glass }", context );
	checks.That( legacy && legacy.Value().family == kLegacyFamily &&
	                 legacy.Value().unmapped.empty() &&
	                 VariableOf( legacy.Value(), "$motionblurinternal" ) &&
	                 legacy.Value().metadata.size() == 1,
	    "V8.a-legacy-shader-keeps-every-variable" );
	auto subrect = ImportVmt( "Subrect { $material a/b $pos \"0 0\" $size \"8 8\" }", context );
	checks.That( subrect && subrect.Value().family == kLegacyFamily, "V8.subrect-is-legacy" );
	auto unknown = ImportVmt( "DebugLuxels { $basetexture a }", context );
	checks.That( !unknown && unknown.Error().status == ImportStatus::kUnknownShader &&
	                 unknown.Error().detail == "DebugLuxels",
	    "V8.an-unknown-shader-is-reported-by-name" );
	auto sprite =
	    ImportVmt( "Sprite { $spriteorientation vp_parallel $spriterendermode 5 }", context );
	checks.That( sprite && ValueOf( sprite.Value(), "spriteorientation" ) &&
	                 ValueOf( sprite.Value(), "spriteorientation" )->numbers[0] == 2.0f,
	    "V9.enumerations-read-their-names" );
	auto scalar = ImportVmt( "UnlitGeneric { $color 0.5 }", context );
	checks.That( scalar && ValueOf( scalar.Value(), "color" )->numbers[2] == 0.5f,
	    "V9.a-scalar-fills-every-component" );
	auto invalid = ImportVmt( "UnlitGeneric { $alpha none }", context );
	checks.That(
	    invalid && !ValueOf( invalid.Value(), "alpha" ) && invalid.Value().diagnostics.size() == 1,
	    "V9.an-unreadable-value-keeps-the-default-and-is-reported" );

	// KeyValues text the loader rejects, and the end of input it accepts.
	auto broken = ImportVmt( "UnlitGeneric { \"$alpha\" [$WIN32] 1 }", context );
	checks.That( !broken && broken.Error().status == ImportStatus::kMalformed,
	    "V10.malformed-text-is-reported" );
	auto empty = ImportVmt( "// nothing\n", context );
	checks.That( !empty && empty.Error().status == ImportStatus::kMalformed,
	    "V10.no-material-block-is-reported" );
	auto unclosed = ImportVmt( "UnlitGeneric\n{\n $alpha 0.5\n Proxies\n {\n", context );
	checks.That(
	    unclosed && ValueOf( unclosed.Value(), "alpha" ), "V10.the-end-closes-open-blocks" );
}

void TestPatches( testing::Checks &checks )
{
	VmtImportContext context;
	context.resolve = Files( {
	    { "materials/base/wall.vmt",
	        "LightmappedGeneric { $basetexture base/wall $alpha 1 $envmaptint \"[1 1 1]\" "
	        "Proxies { Sine { resultvar $alpha } } }" },
	    { "materials/base/middle.vmt", "patch { include \"materials/base/wall.vmt\" insert { "
	                                   "$detail middle/detail $alpha 0.5 } }" },
	    { "materials/base/loop.vmt", "patch { include materials/base/loop.vmt }" },
	} );
	auto patched = ImportVmt( "patch\n{\n include \"materials\\Base\\Middle.vmt\"\n"
	                          " replace { $envmaptint \"[0 0 0]\" $color \"[0 1 0]\" }\n"
	                          " insert { $bumpmap base/bump }\n}\n",
	    context );
	checks.That( patched.HasValue(), "P1.a-patch-chain-imports" );
	if ( patched )
	{
		const MaterialDesc &m = patched.Value();
		checks.That( m.includes == std::vector<std::string>{ "materials/base/middle.vmt",
		                               "materials/base/wall.vmt" },
		    "P1.includes-resolve-in-order" );
		checks.That( m.family == "lightmapped" && ValueOf( m, "bumpmap" ) && ValueOf( m, "detail" ),
		    "P2.insert-adds-keys" );
		checks.That( ValueOf( m, "envmaptint" ) && ValueOf( m, "envmaptint" )->numbers[0] == 0.0f,
		    "P2.replace-overwrites-an-existing-key" );
		checks.That( !ValueOf( m, "color" ), "P2.replace-adds-no-key" );
		checks.That( ValueOf( m, "alpha" ) && ValueOf( m, "alpha" )->numbers[0] == 0.5f,
		    "P2.insert-overwrites-the-base" );
		checks.That(
		    m.proxies.size() == 1 && m.proxies[0].name == "Sine", "P2.the-base-proxies-survive" );
	}
	auto missing = ImportVmt( "patch { include materials/none.vmt }", context );
	checks.That( !missing && missing.Error().status == ImportStatus::kMissingInclude &&
	                 missing.Error().detail == "materials/none.vmt",
	    "P3.a-missing-include-is-named" );
	auto without = ImportVmt( "patch { insert { $alpha 1 } }", context );
	checks.That( !without && without.Error().status == ImportStatus::kPatchWithoutInclude,
	    "P3.a-patch-without-include-fails" );
	auto loop = ImportVmt( "patch { include materials/base/loop.vmt }", context );
	checks.That( !loop && loop.Error().status == ImportStatus::kIncludeDepth,
	    "P3.patches-nest-at-most-ten-deep" );
	VmtImportContext unresolved;
	auto noResolver = ImportVmt( "patch { include materials/base/wall.vmt }", unresolved );
	checks.That( !noResolver && noResolver.Error().status == ImportStatus::kMissingInclude,
	    "P3.without-a-resolver-every-include-is-missing" );
}

void TestPbr( testing::Checks &checks )
{
	VmtImportContext context;
	context.path = "materials/metal/plate.vmt";
	context.resolve = Files( { { "materials/metal/plate_fallback.vmt", "LightmappedGeneric {}" },
	    { "materials/metal/plate.vmt", "PBRMetalRough {}" } } );
	const std::string required =
	    "$basetexture metal/plate $mraotexture metal/plate_mrao $ior 1.4 $clearcoat 1 ";
	auto good = ImportVmt(
	    "PBRMetalRough { " + required + "$fallbackmaterial metal/plate_fallback }", context );
	checks.That( good && good.Value().family == "pbr" && ValueOf( good.Value(), "ior" ) &&
	                 ValueOf( good.Value(), "ior" )->numbers[0] == 1.4f,
	    "B1.a-pbr-material-imports" );
	auto missing = ImportVmt(
	    "PBRMetalRough { $basetexture a $fallbackmaterial metal/plate_fallback }", context );
	checks.That( !missing && missing.Error().status == ImportStatus::kMissingRequired &&
	                 missing.Error().detail == "$mraotexture",
	    "B2.a-missing-required-parameter-is-named" );
	auto absent =
	    ImportVmt( "PBRMetalRough { " + required + "$fallbackmaterial metal/none }", context );
	checks.That( !absent && absent.Error().status == ImportStatus::kInvalidReference,
	    "B3.a-missing-fallback-is-invalid" );
	auto self =
	    ImportVmt( "PBRMetalRough { " + required + "$fallbackmaterial metal/plate }", context );
	checks.That( !self && self.Error().status == ImportStatus::kInvalidReference,
	    "B3.a-self-fallback-is-invalid" );
	auto escape = ImportVmt( "PBRMetalRough { " + required + "$fallbackmaterial ../x }", context );
	checks.That( !escape && escape.Error().status == ImportStatus::kInvalidReference,
	    "B3.a-malformed-fallback-is-invalid" );
}

void TestApply( testing::Checks &checks )
{
	FamilyRegistry registry;
	for ( const FamilyDesc &family : FamiliesFromMapping( BuiltinVmtMapping() ) )
		(void)registry.Register( family );
	VmtImportContext context;
	auto m = ImportVmt( "VertexLitGeneric { $color \"[.25 .5 1]\" $phong 1 $phongexponent 20 "
	                    "$basetexture a }",
	    context );
	const FamilySchema *schema = registry.Find( "vertexlit" );
	ParameterBlock block( *schema );
	const std::uint64_t start = block.Revision();
	checks.That( m && ApplyValues( m.Value(), block ).HasValue() && block.Revision() > start,
	    "A1.values-apply-and-raise-the-revision" );
	float color[3] = {};
	std::memcpy( color, block.Bytes().data() + schema->layout[*schema->IndexOf( "color" )].offset,
	    sizeof( color ) );
	std::int32_t phong = 0;
	std::memcpy( &phong, block.Bytes().data() + schema->layout[*schema->IndexOf( "phong" )].offset,
	    sizeof( phong ) );
	checks.That( color[0] == 0.25f && color[1] == 0.5f && color[2] == 1.0f && phong == 1,
	    "A1.values-land-at-their-offsets" );
	const FamilySchema *unlit = registry.Find( "unlit" );
	ParameterBlock other( *unlit );
	checks.That( m && !ApplyValues( m.Value(), other ).HasValue(),
	    "A2.a-block-of-another-family-refuses-its-parameters" );
}

} // namespace

// Texture transforms (rows 0 and 1, as shaders read them) in both of the
// material system's text forms, and MapVariables mapping as ImportVmt does.
void TestTransforms( testing::Checks &checks )
{
	VmtImportContext context;
	auto near = []( const float *a, const float *b, int count )
	{
		for ( int i = 0; i < count; ++i )
		{
			if ( std::fabs( a[i] - b[i] ) > 1e-5f )
				return false;
		}
		return true;
	};
	// T(c + t) Rz(30) S(2, 3) T(-c) with c = (.5, .5), t = (.1, .2).
	const float composed[8] = {
	    1.7320508f, -1.5f, 0.0f, 0.4839746f, 1.0f, 2.5980762f, 0.0f, -1.0990381f };
	auto center = ImportVmt( "UnlitGeneric { $basetexturetransform \"center .5 .5 scale 2 3 "
	                         "rotate 30 translate .1 .2\" }",
	    context );
	const MaterialValue *value =
	    center ? ValueOf( center.Value(), "basetexturetransform" ) : nullptr;
	checks.That(
	    value && value->kind == ValueKind::kTransform && near( value->numbers, composed, 8 ),
	    "X1.center-scale-rotate-translate-composes-as-the-material-system" );
	auto matrix = ImportVmt( "LightmappedGeneric { $bumptransform \"[1 2 3 4 5 6 7 8 9 10 11 12 "
	                         "13 14 15 16]\" }",
	    context );
	const float rows[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	value = matrix ? ValueOf( matrix.Value(), "bumptransform" ) : nullptr;
	checks.That( value && near( value->numbers, rows, 8 ), "X1.a-matrix-keeps-rows-0-and-1" );
	auto bad = ImportVmt( "UnlitGeneric { $basetexturetransform \"spin 3\" }", context );
	checks.That(
	    bad && !ValueOf( bad.Value(), "basetexturetransform" ) && !bad.Value().diagnostics.empty(),
	    "X2.a-malformed-transform-keeps-its-default-with-a-diagnostic" );

	FamilyRegistry registry;
	for ( const FamilyDesc &desc : FamiliesFromMapping( BuiltinVmtMapping() ) )
		(void)registry.Register( desc );
	const FamilySchema *unlit = registry.Find( "unlit" );
	const std::optional<std::size_t> index =
	    unlit ? unlit->IndexOf( "basetexturetransform" ) : std::nullopt;
	bool identity = false;
	bool applied = false;
	if ( index )
	{
		const ParameterLayout &layout = unlit->layout[*index];
		ParameterBlock block( *unlit );
		float stored[8] = {};
		std::memcpy( stored, block.Bytes().data() + layout.offset, sizeof( stored ) );
		const float identityRows[8] = { 1, 0, 0, 0, 0, 1, 0, 0 };
		identity = layout.type == ParameterType::kTransform && layout.offset % 16 == 0 &&
		           near( stored, identityRows, 8 );
		if ( center && ApplyValues( center.Value(), block ) )
		{
			std::memcpy( stored, block.Bytes().data() + layout.offset, sizeof( stored ) );
			applied = near( stored, composed, 8 );
		}
	}
	checks.That( identity, "X3.a-transform-defaults-to-the-identity-on-16-bytes" );
	checks.That( applied, "X3.a-transform-applies-to-its-block" );

	// MapVariables: a bound material's variables map as its VMT would.
	auto imported = ImportVmt( "UnlitGeneric { $basetexture a/b $color \"[.5 .25 1]\" "
	                           "$basetexturetransform \"center .5 .5 scale 2 3 rotate 30 "
	                           "translate .1 .2\" $alpha .5 $unknownkey 3 }",
	    context );
	auto mapped = MapVariables( "UnlitGeneric",
	    { { "$basetexture", "a/b" }, { "$color", "[.5 .25 1]" },
	        { "$basetexturetransform", "center .5 .5 scale 2 3 rotate 30 translate .1 .2" },
	        { "$alpha", ".5" }, { "$unknownkey", "3" } },
	    context );
	bool same = imported && mapped && imported.Value().family == mapped.Value().family &&
	            imported.Value().values.size() == mapped.Value().values.size() &&
	            imported.Value().unmapped == mapped.Value().unmapped;
	for ( std::size_t i = 0; same && i < imported.Value().values.size(); ++i )
	{
		const MaterialValue &a = imported.Value().values[i];
		const MaterialValue &b = mapped.Value().values[i];
		same = a.parameter == b.parameter && a.text == b.text && near( a.numbers, b.numbers, 8 );
	}
	checks.That( same, "X4.map-variables-maps-as-import-does" );
	auto unknown = MapVariables( "NoSuchShader", {}, context );
	checks.That( !unknown && unknown.Error().status == ImportStatus::kUnknownShader,
	    "X4.map-variables-reports-an-unknown-shader" );
}

int main()
{
	using namespace render;
	testing::Checks checks;
	TestFamiliesFromMapping( checks );
	TestSceneTerms( checks );
	TestKeyMapping( checks );
	TestDerivedCopy( checks );
	TestImportReading( checks );
	TestPatches( checks );
	TestPbr( checks );
	TestApply( checks );
	TestTransforms( checks );

	FamilyRegistry registry;
	FamilyDesc lit;
	lit.name = "lightmapped";
	lit.parameters = { { "alpha", ParameterType::kFloat, { 1.0f } },
	    { "tint", ParameterType::kFloat4, { 1.0f, 1.0f, 1.0f, 1.0f } },
	    { "basetexture", ParameterType::kTexture, {} }, { "flags", ParameterType::kInt, {} },
	    { "scale", ParameterType::kFloat2, { 1.0f, 1.0f } } };
	lit.bindGroups = 4;
	auto id = registry.Register( lit );
	checks.That( id.HasValue(), "M1.a-family-registers" );
	const FamilySchema *schema = id ? registry.Find( id.Value() ) : nullptr;
	checks.That( schema && schema->layout[0].offset == 0 && schema->layout[1].offset == 16 &&
	                 schema->layout[2].offset == 0 && schema->layout[3].offset == 32 &&
	                 schema->layout[4].offset == 40 && schema->blockSize == 48 &&
	                 schema->textureSlots == 1,
	    "M1.parameters-take-std140-offsets" );
	checks.That( !registry.Register( lit ), "M2.a-duplicate-family-fails" );
	FamilyDesc five = lit;
	five.name = "five";
	five.bindGroups = 5;
	auto rejected = registry.Register( five );
	checks.That( !rejected && rejected.Error().status == MaterialStatus::kTooManyBindGroups,
	    "M2.a-five-group-family-fails" );
	FamilyDesc twice;
	twice.name = "twice";
	twice.parameters = { { "a", ParameterType::kFloat, {} }, { "a", ParameterType::kFloat, {} } };
	checks.That( !registry.Register( twice ), "M2.a-parameter-declared-twice-fails" );

	FamilyDesc compute;
	compute.name = "skinned";
	compute.required.Add( device::Capability::kCompute );
	const FamilySchema *needs = registry.Find( registry.Register( compute ).Value() );
	device::DeviceFacts facts;
	checks.That( MissingCapability( *needs, facts ) == device::Capability::kCompute,
	    "M3.a-missing-capability-is-named" );
	facts.capabilities.Add( device::Capability::kCompute );
	checks.That( !MissingCapability( *needs, facts ), "M3.a-present-capability-passes" );

	if ( !schema )
		return checks.Report();
	// Adding a family must not move this one: blocks point at their schema.
	for ( int i = 0; i < 64; ++i )
	{
		FamilyDesc filler;
		filler.name = "filler" + std::to_string( i );
		(void)registry.Register( filler );
	}
	ParameterBlock block( *registry.Find( id.Value() ) );
	float alpha = 0.0f;
	std::memcpy( &alpha, block.Bytes().data(), sizeof( alpha ) );
	checks.Equal( alpha, 1.0f, "M4.defaults-fill-the-block" );
	const std::uint64_t start = block.Revision();
	checks.That( block.SetFloat( "alpha", 1.0f ).HasValue() && block.Revision() == start,
	    "M4.setting-the-same-value-keeps-the-revision" );
	checks.That( block.SetFloat( "alpha", 0.5f ).HasValue() && block.Revision() == start + 1,
	    "M4.a-change-raises-the-revision" );
	auto unknown = block.SetFloat( "alhpa", 0.5f );
	checks.That( !unknown && unknown.Error().status == MaterialStatus::kUnknownParameter,
	    "M4.an-unknown-key-fails" );
	auto wrongType = block.SetFloat( "tint", 0.5f );
	checks.That( !wrongType && wrongType.Error().status == MaterialStatus::kTypeMismatch,
	    "M4.a-wrong-type-fails" );
	checks.That( block.SetTexture( "basetexture", device::TextureId{ 7 } ).HasValue() &&
	                 block.Textures()[0] == device::TextureId{ 7 },
	    "M4.textures-take-their-slot" );
	return checks.Report();
}
