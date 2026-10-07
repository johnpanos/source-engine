//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's posed-model fixture: the world pass draws one shared
//			mesh at two captured poses through the mesh PBR program, with a
//			live area light. Pixel coverage and light response are independent
//			checks on the pass that the game will use for animated doors.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/pass/world/world_pass.h"
#include "render/graph/scene_color.h"
#include "render/material/program_resolver.h"
#include "render/material/lightmapped_family.h"
#include "render/material/registry.h"
#include "render/material/vmt_import.h"
#include "render/material/vmt_matrix.h"

#include <algorithm>
#include <cstring>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

namespace render::lab
{

namespace
{

using namespace render::device;
using namespace render::pass::world;

constexpr std::uint32_t kSize = 64;

class EmptyTextures final : public IWorldTextures
{
public:
	TextureId normalFixture;
	TextureId phongWarpFixture;
	TextureId refractNormalFixture;
	TextureId refractWarpFixture;
	TextureId twoTextureBase;
	TextureId twoTextureOverlay;
	TextureId cutoutFixture;
	TextureId environmentFixture;
	TextureId decalFixture;
	TextureId localRefractBase; // 2x2: red left column, green right (nearest, clamped)
	TextureId splitNormal;      // 2x1: flat left, +x warp right (nearest, clamped)
	TextureId Import( int handle, bool ) override
	{
		return handle == 1    ? normalFixture
		       : handle == 2  ? phongWarpFixture
		       : handle == 3  ? refractNormalFixture
		       : handle == 4  ? refractWarpFixture
		       : handle == 5  ? twoTextureBase
		       : handle == 6  ? twoTextureOverlay
		       : handle == 7  ? cutoutFixture
		       : handle == 9  ? decalFixture
		       : handle == 8  ? environmentFixture
		       : handle == 10 ? localRefractBase
		       : handle == 11 ? splitNormal
		                      : TextureId{};
	}
	SamplerDesc Sampler( int handle ) override
	{
		SamplerDesc sampler;
		if ( handle == 6 || handle == 7 || handle == 10 || handle == 11 )
		{
			sampler.address = AddressMode::kClampToEdge;
			sampler.minFilter = sampler.magFilter = Filter::kNearest;
		}
		return sampler;
	}
};

WorldData MeshWorld( bool blended = false )
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	WorldMaterial material;
	material.name = "posed-door-fixture";
	material.shader = "VertexLitGeneric";
	material.mesh = true;
	if ( blended )
	{
		material.variables.push_back( { "$translucent", "1" } );
		material.variables.push_back( { "$alpha", "0.5" } );
	}
	world.materials.push_back( std::move( material ) );
	std::vector<material::SurfaceModelVertex> quad;
	for ( const auto &xy : { std::pair{ -0.5f, -0.5f }, std::pair{ 0.5f, -0.5f },
	          std::pair{ 0.5f, 0.5f }, std::pair{ -0.5f, 0.5f } } )
	{
		material::SurfaceModelVertex vertex;
		vertex.position[0] = xy.first;
		vertex.position[1] = xy.second;
		vertex.position[2] = 0.5f;
		vertex.normal[2] = 1.0f;
		vertex.tangent[0] = vertex.tangent[3] = 1.0f;
		quad.push_back( vertex );
	}
	WorldData::StaticMesh mesh;
	mesh.posed = true; // a posed fixture: its level is pinned and never released
	mesh.AddLevel(
	    WorldData::StaticMeshLod::MakeLevel( quad, { 0, 1, 2, 0, 2, 3 } ), { { 0, 0, 0, 6 } } );
	world.staticMeshes.push_back( std::move( mesh ) );
	return world;
}

WorldData MixedWorld()
{
	WorldData world = MeshWorld();
	WorldMaterial glass = world.materials.front();
	glass.name = "posed-glass-fixture";
	glass.translucent = true;
	glass.variables.push_back( { "$translucent", "1" } );
	glass.variables.push_back( { "$alpha", "0.5" } );
	world.materials.push_back( std::move( glass ) );
	// One level, two surfaces over the same index range: the same geometry
	// drawn opaque (material zero) and blended (material one), which is what
	// puts a studio two-pass surface in both phases.
	WorldData::StaticMesh mixed;
	mixed.posed = true;
	mixed.AddLevel( WorldData::StaticMeshLod::MakeLevel( *world.staticMeshes[0].lods[0].vertices,
	                    *world.staticMeshes[0].lods[0].indices ),
	    { { 0, 0, 0, 6 }, { 1, 0, 0, 6 } } );
	world.staticMeshes[0] = std::move( mixed );
	return world;
}

float Sum( const CanvasImage &image, std::uint32_t low, std::uint32_t high )
{
	float sum = 0.0f;
	for ( std::uint32_t y = 0; y < image.height; ++y )
		for ( std::uint32_t x = low; x < high; ++x )
			sum += image.At( x, y )[0];
	return sum;
}

std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( std::optional<std::string> why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	LabSceneColorCapture sceneColorCapture( *device );
	WorldData world = MeshWorld();
	material::FamilyRegistry coverageRegistry;
	for ( const auto &family : material::FamiliesFromMapping( material::BuiltinVmtMapping() ) )
		(void)coverageRegistry.Register( family );
	const auto *coverageSchema = coverageRegistry.Find( "lightmapped" );
	if ( !coverageSchema )
		return "lightmapped coverage schema unavailable";
	for ( int control = 0; control < 3; ++control )
	{
		material::ParameterBlock block( *coverageSchema );
		(void)block.SetInt( "allowalphatocoverage", 1 );
		(void)block.SetInt( "alphatest", control == 1 ? 0 : 1 );
		(void)block.SetInt( "translucent", control == 2 ? 1 : 0 );
		const auto claim = material::ClaimLightmapped( block );
		results.That( claim.claimed &&
		                  claim.Variant( material::SurfaceVertexLayout::kWorld ).alphaToCoverage ==
		                      ( control == 0 ),
		    "posed-model.world-cutout-coverage-policy-" + std::to_string( control ) );
	}
	const auto tubeGlass = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_backstage/vacum_pipe_glass" },
	        { "$bumpmap", "models/props_backstage/vacum_pipe_glass_normal" },
	        { "$translucent", "1" }, { "$envmap", "env_cubemap" },
	        { "$envmaptint", "[.85 .85 .85]" }, { "$basemapalphaenvmapmask", "1" },
	        { "$envmapfresnel", "1" }, { "$invertphongmask", "1" }, { "$halflambert", "1" },
	        { "$phong", "1" }, { "$phongexponent", "50" }, { "$phongboost", "1" },
	        { "$phongfresnelranges", "[.3 .6 4]" } },
	    {} );
	results.That(
	    tubeGlass && material::ClaimForMesh( tubeGlass.Value(), true ) &&
	        material::ClaimForMesh( tubeGlass.Value(), true ).Value() == BlendMode::kAlpha,
	    "posed-model.tube-glass-claims-native-probes" );
	results.That( tubeGlass && !material::ClaimForMesh( tubeGlass.Value(), false ),
	    "posed-model.tube-glass-requires-native-probes" );
	const auto fracturedGlass = material::MapVariables( "Refract_DX90",
	    { { "$model", "1" }, { "$normalmap", "models/props_destruction/glass_fracture_B_normal" },
	        { "$dudvmap", "models/props_destruction/glass_fracture_B_normal" },
	        { "$scale", "[1 1]" }, { "$refractamount", ".07" }, { "$bluramount", ".3" },
	        { "$refracttint", "{235 247 247}" }, { "$translucent", "1" },
	        { "$envmap", "env_cubemap" }, { "$envmapcontrast", "1" },
	        { "$envmapsaturation", "[1 1 1]" }, { "$envmaptint", "[.71 .79 .85]" } },
	    {} );
	results.That( fracturedGlass && !material::ClaimForMesh( fracturedGlass.Value(), true ),
	    "posed-model.refract-refuses-missing-scene-color" );
	results.That( fracturedGlass && !material::ClaimForMesh( fracturedGlass.Value(), false, true ),
	    "posed-model.refract-refuses-missing-native-probes" );
	results.That( fracturedGlass && material::ClaimForMesh( fracturedGlass.Value(), true, true ),
	    "posed-model.refract-authored-model-claims-with-scene-color" );
	auto refractResolver = material::ProgramResolver::Create(
	    *device, kCanvasColor, kCanvasDepth, 1, material::VertexLayout::kModel );
	bool refractPipeline = bool( refractResolver ) && bool( fracturedGlass );
	if ( refractPipeline )
	{
		refractResolver.Value()->SetWorldPbr( true, material::kSurfaceReflectionProbes );
		refractResolver.Value()->SetSceneColorAvailable( true );
		refractPipeline = bool( refractResolver.Value()->ResolveMesh( fracturedGlass.Value() ) );
	}
	results.That( refractPipeline, "posed-model.refract-builds-shared-surface-pipeline" );
	// $refracttinttexture (refract_ps2x.fxc): the tint texture rides the
	// emission binding, sRGB, with the point's tint-texture mode on.
	const auto tintedRefract = material::MapVariables( "Refract_DX90",
	    { { "$model", "1" }, { "$normalmap", "glass/normal" },
	        { "$refracttinttexture", "glass/tint" }, { "$refracttint", "[1 .5 .25]" } },
	    {} );
	results.That( tintedRefract && material::ClaimForMesh( tintedRefract.Value(), true, true ),
	    "posed-model.refract-tint-texture-claims" );
	bool tintBound = false;
	if ( refractResolver && tintedRefract )
	{
		const auto resolved = refractResolver.Value()->ResolveMesh( tintedRefract.Value() );
		if ( resolved )
		{
			material::SurfaceConstants constants;
			std::memcpy( &constants, resolved.Value().request.material.constants.data(),
			    sizeof( constants ) );
			for ( const auto &texture : resolved.Value().request.material.textures )
				tintBound = tintBound || ( texture.binding == 13 && texture.name == "materials/glass/tint" &&
				                             texture.srgb && constants.meshModes[2] == 1.0f );
		}
	}
	results.That( tintBound, "posed-model.refract-tint-texture-binds-srgb-with-mode" );
	const auto unsupportedRefract = material::MapVariables( "Refract_DX90",
	    { { "$normalmap", "glass/normal" }, { "$normalmap2", "glass/other_normal" } }, {} );
	results.That(
	    unsupportedRefract && !material::ClaimForMesh( unsupportedRefract.Value(), true, true ),
	    "posed-model.refract-secondary-normal-stays-legacy" );
	const auto tubeBlades = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/elevator/elevator_blades" }, { "$phong", "1" },
	        { "$phongboost", "2" }, { "$phongdisablehalflambert", "1" }, { "$phongexponent", "12" },
	        { "$phongfresnelranges", "[.6 1 2]" }, { "$basemapalphaphongmask", "1" },
	        { "$rimlight", "1" }, { "$rimlightexponent", "0.2" }, { "$rimlightboost", "3" },
	        { "$phongalbedotint", "1" } },
	    {} );
	results.That(
	    tubeBlades && material::ClaimForMesh( tubeBlades.Value(), true ) &&
	        material::ClaimForMesh( tubeBlades.Value(), true ).Value() == BlendMode::kOpaque,
	    "posed-model.tube-blades-claim-shared-pbr-point" );
	const auto mappedExponent = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/door_02" }, { "$phong", "1" }, { "$phongexponent", "0" },
	        { "$phongexponenttexture", "models/props/door_02_exponent" },
	        { "$phongalbedotint", "1" } },
	    {} );
	results.That( mappedExponent && material::ClaimForMesh( mappedExponent.Value(), true ),
	    "posed-model.phong-exponent-map-claims-shared-pbr-point" );
	const auto doubleSidedProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/saucepan/saucepan" }, { "$envmap", "env_cubemap" },
	        { "$basealphaenvmapmask", "1" }, { "$normalmapalphaenvmapmask", "1" },
	        { "$nocull", "1" }, { "$bumpmap", "models/props/saucepan/saucepan_normal" } },
	    {} );
	results.That(
	    doubleSidedProbeMask && material::ClaimForMesh( doubleSidedProbeMask.Value(), true ),
	    "posed-model.double-sided-probe-mask-claims-native-probes" );
	const auto textureProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" }, { "$envmap", "env_cubemap" },
	        { "$envmapmask", "models/props_office/door_glass_mask" } },
	    {} );
	results.That( textureProbeMask && material::ClaimForMesh( textureProbeMask.Value(), true ) &&
	                  !material::ClaimForMesh( textureProbeMask.Value(), false ),
	    "posed-model.authored-rgb-mask-claims-native-rprb" );
	const auto conflictingProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" },
	        { "$envmapmask", "models/props_office/door_glass_mask" }, { "$phong", "1" },
	        { "$phongexponenttexture", "models/props/door_02_exponent" } },
	    {} );
	results.That(
	    conflictingProbeMask && !material::ClaimForMesh( conflictingProbeMask.Value(), true ),
	    "posed-model.probe-mask-and-phong-exponent-need-distinct-slots" );
	const auto bumpedProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" }, { "$envmap", "env_cubemap" },
	        { "$envmapmask", "models/props_office/door_glass_mask" },
	        { "$bumpmap", "models/props_office/door_glass_normal" } },
	    {} );
	results.That( bumpedProbeMask && material::ClaimForMesh( bumpedProbeMask.Value(), false ),
	    "posed-model.bumped-envmap-mask-clears-legacy-probe" );
	const auto normalAlphaProbeMask = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" }, { "$envmap", "env_cubemap" },
	        { "$envmapmask", "models/props_office/door_glass_mask" },
	        { "$bumpmap", "models/props_office/door_glass_normal" },
	        { "$normalmapalphaenvmapmask", "1" } },
	    {} );
	results.That( normalAlphaProbeMask &&
	                  !material::ClaimForMesh( normalAlphaProbeMask.Value(), false ) &&
	                  material::ClaimForMesh( normalAlphaProbeMask.Value(), true ),
	    "posed-model.normal-alpha-mask-preserves-legacy-probe" );
	const auto bumpedBaseAlphaProbe = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_office/door_glass" }, { "$envmap", "env_cubemap" },
	        { "$bumpmap", "models/props_office/door_glass_normal" },
	        { "$basealphaenvmapmask", "1" } },
	    {} );
	results.That(
	    bumpedBaseAlphaProbe && material::ClaimForMesh( bumpedBaseAlphaProbe.Value(), false ),
	    "posed-model.bumped-base-alpha-mask-clears-legacy-probe" );
	const auto paintedTurret = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/npcs/turret/turret_01" },
	        { "$detail", "models/npcs/turret/turret_paint_blue" }, { "$detailscale", "1" },
	        { "$detailblendmode", "2" }, { "$detailblendfactor", "0.5" } },
	    {} );
	results.That( paintedTurret && material::ClaimForMesh( paintedTurret.Value(), true ),
	    "posed-model.detail-paint-uses-shared-surface-combine" );
	const auto alphaTint = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_bts/aluminium_extrusion" }, { "$color", "[.5 .7 1]" },
	        { "$blendtintbybasealpha", "1" } },
	    {} );
	results.That( alphaTint && material::ClaimForMesh( alphaTint.Value(), true ),
	    "posed-model.base-alpha-tint-claims-shared-pbr-point" );
	const auto color2Tint = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/player/chell/chell_head_diffuse" },
	        { "$color2", "[.25 .5 1]" } },
	    {} );
	results.That( color2Tint && material::ClaimForMesh( color2Tint.Value(), true ),
	    "posed-model.color2-factor-claims-shared-pbr-point" );
	const auto editorArm = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_map_editor/arm_64x64" }, { "$glowcolor", "1" } }, {} );
	results.That( editorArm && material::ClaimForMesh( editorArm.Value(), true ),
	    "posed-model.vertexlit-ignores-unlit-only-glow-key" );
	const auto authoredProbeFresnel = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/player/coop_bots/assembly_machine_glass" },
	        { "$envmap", "env_cubemap" }, { "$envmapfresnel", "1" },
	        { "$envmapfresnelminmaxexp", "[.1 1 .7]" } },
	    {} );
	results.That(
	    authoredProbeFresnel && material::ClaimForMesh( authoredProbeFresnel.Value(), true ),
	    "posed-model.authored-probe-fresnel-uses-native-rprb" );
	const auto lightScaledProbe = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/a4_destruction/backpanels_color01b_vtx" },
	        { "$envmap", "env_cubemap" }, { "$envmaplightscale", ".6" } },
	    {} );
	results.That( lightScaledProbe && material::ClaimForMesh( lightScaledProbe.Value(), true ),
	    "posed-model.probe-light-scale-uses-native-irradiance" );
	const auto additiveGlass = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/reflecto_cube_glow" }, { "$additive", "1" } }, {} );
	results.That(
	    additiveGlass && material::ClaimForMesh( additiveGlass.Value(), true ) &&
	        material::ClaimForMesh( additiveGlass.Value(), true ).Value() == BlendMode::kAdditive,
	    "posed-model.additive-mesh-uses-shared-surface-blend" );
	const auto warpedLight = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/npcs/turret/turret_01" }, { "$phong", "1" },
	        { "$lightwarptexture", "models/npcs/turret/turret_lightwarp" } },
	    {} );
	results.That( warpedLight && material::ClaimForMesh( warpedLight.Value(), true ),
	    "posed-model.light-warp-uses-shared-direct-diffuse" );
	const auto warpedSpecular = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/reflecto_cube_glass" }, { "$phong", "1" },
	        { "$phongexponent", "2" },
	        { "$phongwarptexture", "models/props/reflecto_cube_iridescence" } },
	    {} );
	results.That( warpedSpecular && material::ClaimForMesh( warpedSpecular.Value(), true ),
	    "posed-model.phong-warp-claims-shared-pbr-specular" );
	const auto conflictingWarps = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props/reflecto_cube_glass" }, { "$phong", "1" },
	        { "$phongwarptexture", "models/props/reflecto_cube_iridescence" },
	        { "$lightwarptexture", "models/npcs/turret/turret_lightwarp" } },
	    {} );
	results.That( conflictingWarps && !material::ClaimForMesh( conflictingWarps.Value(), true ),
	    "posed-model.two-warp-textures-require-distinct-slots" );
	const auto ssbump = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "anim_wp/framework/backpanels_color01" },
	        { "$bumpmap", "anim_wp/framework/backpanels_color01_height-ssbump" },
	        { "$ssbump", "1" }, { "$ssbumpmathfix", "1" } },
	    {} );
	results.That( ssbump && material::ClaimForMesh( ssbump.Value(), true ),
	    "posed-model.ssbump-basis-claims-shared-pbr-point" );
	const auto toxinNormal = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "models/props_destruction/toxintank" },
	        { "$bumpmap", "models/props_destruction/toxintank_normal" }, { "$bumpscale", "30" },
	        { "$phong", "1" } },
	    {} );
	results.That( toxinNormal && material::ClaimForMesh( toxinNormal.Value(), true ),
	    "posed-model.vertexlit-bumpscale-is-undeclared-metadata" );
	const auto unquotedTint = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "metal/black_wall_metal_002a" },
	        { "$envmap", "metal/black_wall_envmap_002a_hdr" }, { "$envmaptint", ".2" },
	        { ".2", ".2" } },
	    {} );
	results.That( unquotedTint && unquotedTint.Value().metadata.size() == 1 &&
	                  unquotedTint.Value().metadata[0].key == ".2" &&
	                  material::ClaimForMesh( unquotedTint.Value(), true ),
	    "posed-model.numeric-keyvalues-residue-is-audited-metadata" );
	const auto unknownControl = material::MapVariables( "VertexLitGeneric",
	    { { "$basetexture", "metal/black_wall_metal_002a" },
	        { "$unrecognized_render_control", "1" } },
	    {} );
	results.That( unknownControl && !material::ClaimForMesh( unknownControl.Value(), true ),
	    "posed-model.unknown-render-control-remains-a-gap" );
	const auto inactiveCloak = material::MapVariables(
	    "VertexLitGeneric", { { "$cloakfactor", "1" }, { "$cloakpassenabled", "0" } }, {} );
	const auto activeCloak = material::MapVariables(
	    "VertexLitGeneric", { { "$cloakfactor", "1" }, { "$cloakpassenabled", "1" } }, {} );
	results.That( inactiveCloak && material::ClaimForMesh( inactiveCloak.Value(), true ),
	    "posed-model.disabled-cloak-factor-is-inert" );
	results.That( activeCloak && !material::ClaimForMesh( activeCloak.Value(), true ),
	    "posed-model.enabled-cloak-still-requires-its-point" );
	const auto emissiveProbe = material::MapVariables( "UnlitGeneric",
	    { { "$basetexture", "models/props_map_editor/black_white_unlit" },
	        { "$envmap", "env_cubemap" } },
	    {} );
	results.That( emissiveProbe && material::ClaimForMesh( emissiveProbe.Value(), true ) &&
	                  !material::ClaimForMesh( emissiveProbe.Value(), false ),
	    "posed-model.unlit-reflection-requires-native-rprb" );
	const auto bindPose = *world.staticMeshes[0].lods[0].vertices;
	WorldPass pass;
	pass.SetWorld( std::move( world ) );
	results.That( pass.DrawsPosedModel( 0, 0 ), "posed-model.claims-mesh-program" );
	EmptyTextures empty;
	TextureDesc normalDesc;
	normalDesc.format = Format::kRGBA8Unorm;
	normalDesc.width = normalDesc.height = 1;
	normalDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	const std::array<std::byte, 4> normalPixel = {
	    std::byte{ 180 }, std::byte{ 80 }, std::byte{ 170 }, std::byte{ 255 } };
	auto normalTexture = textures.Stage( "ssbump-fixture", normalDesc, normalPixel );
	if ( !normalTexture )
		return "the model normal fixture could not be staged";
	empty.normalFixture = normalTexture.Value().texture;
	const std::array<std::byte, 4> warpPixel = {
	    std::byte{ 255 }, std::byte{ 10 }, std::byte{ 10 }, std::byte{ 255 } };
	auto warpTexture = textures.Stage( "phong-warp-fixture", normalDesc, warpPixel );
	if ( !warpTexture )
		return "the model specular warp fixture could not be staged";
	empty.phongWarpFixture = warpTexture.Value().texture;
	const std::array<std::byte, 4> refractNormalPixel = {
	    std::byte{ 128 }, std::byte{ 128 }, std::byte{ 255 }, std::byte{ 128 } };
	auto refractNormal = textures.Stage( "refract-normal-fixture", normalDesc, refractNormalPixel );
	if ( !refractNormal )
		return "the model Refract normal fixture could not be staged";
	empty.refractNormalFixture = refractNormal.Value().texture;
	const std::array<std::byte, 4> refractWarpPixel = {
	    std::byte{ 255 }, std::byte{ 128 }, std::byte{ 255 }, std::byte{ 255 } };
	auto refractWarp = textures.Stage( "refract-warp-fixture", normalDesc, refractWarpPixel );
	if ( !refractWarp )
		return "the model Refract warp fixture could not be staged";
	empty.refractWarpFixture = refractWarp.Value().texture;
	TextureDesc localBaseDesc = normalDesc;
	localBaseDesc.format = Format::kRGBA8Srgb;
	localBaseDesc.width = localBaseDesc.height = 2;
	const std::array<std::byte, 16> localBasePixels = { std::byte{ 255 }, std::byte{ 0 },
	    std::byte{ 0 }, std::byte{ 255 }, std::byte{ 0 }, std::byte{ 255 }, std::byte{ 0 },
	    std::byte{ 255 }, std::byte{ 255 }, std::byte{ 0 }, std::byte{ 0 }, std::byte{ 255 },
	    std::byte{ 0 }, std::byte{ 255 }, std::byte{ 0 }, std::byte{ 255 } };
	auto localBase = textures.Stage( "local-refract-base", localBaseDesc, localBasePixels );
	if ( !localBase )
		return "the local Refract base fixture could not be staged";
	empty.localRefractBase = localBase.Value().texture;
	TextureDesc splitDesc = normalDesc;
	splitDesc.width = 2;
	const std::array<std::byte, 8> splitPixels = { std::byte{ 128 }, std::byte{ 128 },
	    std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 128 }, std::byte{ 255 },
	    std::byte{ 255 } };
	auto split = textures.Stage( "split-normal-fixture", splitDesc, splitPixels );
	if ( !split )
		return "the split normal fixture could not be staged";
	empty.splitNormal = split.Value().texture;
	TextureDesc colorDesc = normalDesc;
	colorDesc.format = Format::kRGBA8Srgb;
	const std::array<std::byte, 4> basePixelFixture = {
	    std::byte{ 128 }, std::byte{ 192 }, std::byte{ 255 }, std::byte{ 64 } };
	const std::array<std::byte, 8> overlayPixels = { std::byte{ 255 }, std::byte{ 128 },
	    std::byte{ 64 }, std::byte{ 128 }, std::byte{ 64 }, std::byte{ 255 }, std::byte{ 128 },
	    std::byte{ 64 } };
	auto baseTexture = textures.Stage( "two-texture-base", colorDesc, basePixelFixture );
	colorDesc.width = 2;
	auto overlayTexture = textures.Stage( "two-texture-overlay", colorDesc, overlayPixels );
	if ( !baseTexture || !overlayTexture )
		return "the two-texture color fixtures could not be staged";
	empty.twoTextureBase = baseTexture.Value().texture;
	empty.twoTextureOverlay = overlayTexture.Value().texture;
	const std::array<std::byte, 8> cutoutPixels = { std::byte{ 255 }, std::byte{ 255 },
	    std::byte{ 255 }, std::byte{ 0 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 },
	    std::byte{ 255 } };
	auto cutoutTexture = textures.Stage( "model-depth-cutout", colorDesc, cutoutPixels );
	if ( !cutoutTexture )
		return "the model depth cutout fixture could not be staged";
	empty.cutoutFixture = cutoutTexture.Value().texture;
	// -1 poses the fixture's normals away from the eye (a pane seen from behind).
	float poseNormalSign = 1.0f;
	auto render = [&]( WorldPass &active, float offset, bool lit, std::uint64_t frame,
	                  const ClearColor &clear, CanvasImage &image, bool twoLayers = false,
	                  RenderCoreDrawPhase phase = RenderCoreDrawPhase::kAll, bool copySource = true,
	                  WorldPass *under = nullptr, const WorldMaterial *dynamicMaterial = nullptr,
	                  bool invalidDynamicIndex = false, float underOffset = 0.1f,
	                  bool depthPrepass = false, bool staticModels = false, bool clipLeft = false,
	                  bool splitLayers = false, bool opaqueBatch = false, bool worldSurface = false,
	                  bool equalDepthSkins = false, std::optional<Viewport> viewport = std::nullopt,
	                  float outputScale = 1.0f, bool fog = false ) -> std::optional<std::string>
	{
		WorldView view;
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 1.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		if ( viewport )
			view.viewport = *viewport;
		view.hostFrame = frame;
		WorldView::PosedModel pose;
		pose.phase = phase;
		pose.vertices = bindPose;
		for ( auto &vertex : pose.vertices )
		{
			vertex.position[0] += offset;
			vertex.normal[2] *= poseNormalSign;
		}
		if ( dynamicMaterial )
		{
			WorldView::DynamicDraw draw;
			draw.material = *dynamicMaterial;
			for ( const auto &vertex : pose.vertices )
			{
				WorldVertex out;
				std::copy_n( vertex.position, 3, out.position );
				std::copy_n( vertex.normal, 3, out.normal );
				std::copy_n( vertex.tangent, 3, out.tangentS );
				std::copy_n( vertex.uv, 2, out.uv );
				if ( dynamicMaterial->shader == "Cable_DX9" )
				{
					out.uv[0] = out.uv[1] = 0.75f;
					out.lightmapUv[0] = out.lightmapUv[1] = 0.25f;
					out.color[0] = out.color[3] = 128;
					out.color[1] = 64;
					out.color[2] = 255;
				}
				draw.vertices.push_back( out );
			}
			draw.indices = { 0, 1, 2, 0, 2, 3 };
			if ( invalidDynamicIndex )
				draw.indices.back() = 999;
			view.dynamicDraws.push_back( std::move( draw ) );
		}
		else if ( staticModels )
			view.staticInstances = { 0, 1 };
		else
			view.posedModels.push_back( std::move( pose ) );
		if ( twoLayers )
		{
			WorldView::PosedModel behind = view.posedModels.front();
			for ( auto &vertex : behind.vertices )
				vertex.position[2] += equalDepthSkins ? 0.0f : 0.1f;
			if ( equalDepthSkins )
				behind.skin = 1;
			view.posedModels.push_back( std::move( behind ) );
		}
		if ( lit )
		{
			auto lights = std::make_shared<StageViewLights>();
			material::SurfaceAreaLight area;
			area.center[2] = 0.8f;
			area.center[3] = 1.0f;
			area.halfU[0] = 0.5f;
			area.halfU[3] = 10.0f;
			area.halfV[1] = -0.5f;
			area.radiance[0] = 8.0f;
			area.radiance[1] = 7.0f;
			area.radiance[2] = 6.0f;
			area.radiance[3] = -1.0f;
			lights->areas.push_back( area );
			view.lights = std::move( lights );
		}
		std::uint32_t underTag = 0;
		if ( under )
		{
			WorldView underView = view;
			for ( auto &pose : underView.posedModels )
				for ( auto &vertex : pose.vertices )
					vertex.position[2] += underOffset;
			underTag = under->QueueView( std::move( underView ) );
			if ( !underTag )
				return "the background model view queued nothing";
		}
		if ( worldSurface )
			view.surfaces = { 0 };
		std::optional<WorldView> later;
		if ( splitLayers )
		{
			later = view;
			if ( worldSurface )
			{
				later->surfaces.clear();
				view.posedModels.clear();
			}
			else
			{
				later->posedModels = { view.posedModels.back() };
				view.posedModels.resize( 1 );
			}
		}
		const std::uint32_t tag = active.QueueView( std::move( view ) );
		const std::uint32_t laterTag = later ? active.QueueView( std::move( *later ) ) : 0;
		if ( !tag )
			return "the posed view queued nothing: " + active.Stats().lastRefusal;
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId depth ) -> std::optional<std::string>
		{
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.colorCopySource = copySource;
			target.sceneColorCapture = &sceneColorCapture;
			target.depth = depth;
			target.depthFormat = kCanvasDepth;
			target.width = target.height = kSize;
			target.textures = &empty;
			target.frame = frame;
			target.outputScale = outputScale;
			if ( fog )
			{
				target.fogType = 0;
				target.fogColor[0] = 0.1f;
				target.fogColor[1] = 0.2f;
				target.fogColor[2] = 0.3f;
				target.fogParams[1] = 2.0f;
				target.fogParams[3] = 0.5f;
			}
			target.depthPrepass = depthPrepass;
			if ( clipLeft )
				target.clipPlanes[0][0] = 1.0f;
			target.eye[2] = 2.0f;
			if ( under )
				under->Record( underTag, encoder, target );
			if ( opaqueBatch )
			{
				const std::uint32_t tags[] = { tag, laterTag };
				if ( active.OpaqueBatchSize( tags, target.streamEpoch ) != 2 )
					return "the opaque fixture did not batch both cohorts";
				active.RecordBatch( tags, encoder, target );
			}
			else
			{
				active.Record( tag, encoder, target );
				if ( laterTag )
					active.Record( laterTag, encoder, target );
			}
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {}, clear, &image, post );
	};
	CanvasImage center, shifted, dark;
	const ClearColor black{ 0, 0, 0, 1 };
	if ( std::optional<std::string> why = render( pass, 0.0f, true, 1, black, center ) )
		return why;
	if ( std::optional<std::string> why = render( pass, 0.5f, true, 2, black, shifted ) )
		return why;
	if ( std::optional<std::string> why = render( pass, 0.0f, false, 3, black, dark ) )
		return why;
	if ( const char *directory = std::getenv( "RENDER_LAB_IMAGES" ) )
	{
		std::filesystem::create_directories( directory );
		for ( const auto &[name, image] : { std::pair{ "posed-lit", &center },
		          std::pair{ "posed-shifted", &shifted }, std::pair{ "posed-unlit", &dark } } )
		{
			if ( !WritePfm( std::filesystem::path( directory ) / ( std::string( name ) + ".pfm" ),
			         image->width, image->height, image->rgba ) )
				return "could not write posed-model image";
		}
	}
	results.That( pass.Stats().viewsFailed == 0 && pass.Stats().posedDrawsDrawn == 3,
	    "posed-model.three-views-recorded", pass.Stats().lastFailure );
	CanvasImage depthLit;
	if ( auto why = render( pass, 0.0f, true, 4, black, depthLit, false, RenderCoreDrawPhase::kAll,
	         true, nullptr, nullptr, false, 0.1f, true ) )
		return why;
	results.That( depthLit.rgba == center.rgba, "posed-model.depth-prepass-exact",
	    "the complete opaque depth prepass preserves the lit posed image" );
	CanvasImage layersControl, layersDepth;
	if ( auto why = render( pass, 0.0f, true, 4, black, layersControl, true ) )
		return why;
	if ( auto why = render( pass, 0.0f, true, 4, black, layersDepth, true,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true ) )
		return why;
	results.That( layersDepth.rgba == layersControl.rgba,
	    "posed-model.overlapping-depth-prepass-exact",
	    "hidden posed surfaces preserve every lit pixel" );
	CanvasImage sharedCohorts;
	if ( auto why =
	         render( pass, 0.0f, true, 4, black, sharedCohorts, true, RenderCoreDrawPhase::kAll,
	             true, nullptr, nullptr, false, 0.1f, true, false, false, true ) )
		return why;
	results.That( sharedCohorts.rgba == layersDepth.rgba,
	    "posed-model.shared-lighting-across-cohorts-preserves-every-pixel",
	    "separately recorded opaque cohorts consume the same immutable lighting bindings" );
	CanvasImage batchedModels;
	if ( auto why =
	         render( pass, 0.0f, true, 4, black, batchedModels, true, RenderCoreDrawPhase::kAll,
	             true, nullptr, nullptr, false, 0.1f, true, false, false, true, true ) )
		return why;
	results.That( batchedModels.rgba == sharedCohorts.rgba, "posed-model.batched-cohorts-exact",
	    "shared depth preserves every lit pixel" );
	// Reversing material order across tickets would change the equal-depth
	// winner. Skin zero uses material one, the later skin uses material zero.
	WorldData tieWorld = MeshWorld();
	tieWorld.materials[0].variables = { { "$color", "[1 0 0]" } };
	tieWorld.materials.push_back( tieWorld.materials[0] );
	tieWorld.materials[1].variables = { { "$color", "[0 0 1]" } };
	tieWorld.staticMeshes[0].skinMaterials = { { 1 }, { 0 } };
	WorldPass ties;
	ties.SetWorld( std::move( tieWorld ) );
	CanvasImage separateTies, batchedTies;
	for ( bool batch : { false, true } )
	{
		if ( auto why = render( ties, 0.0f, true, 4, black, batch ? batchedTies : separateTies,
		         true, RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true, false,
		         false, true, batch, false, true ) )
			return why;
	}
	results.That( ties.Failures() == 0 && separateTies.rgba == batchedTies.rgba,
	    "posed-model.batch-preserves-equal-depth-material-order", ties.Stats().lastFailure );
	results.That(
	    batchedTies.At( kSize / 2, kSize / 2 )[0] > batchedTies.At( kSize / 2, kSize / 2 )[2],
	    "posed-model.later-equal-depth-ticket-wins" );
	ties.ReleaseDevice( *device );
	WorldData combinedWorld = MeshWorld();
	WorldMaterial wall;
	wall.name = "opaque-batch-wall";
	wall.shader = "PBRMetalRough";
	wall.variables = { { "$basetexture", "batch-base" }, { "$mraotexture", "batch-mrao" } };
	wall.textures = { { "$basetexture", 1 }, { "$mraotexture", 1 } };
	combinedWorld.materials.push_back( wall );
	for ( const auto &vertex : bindPose )
	{
		WorldVertex out;
		std::copy_n( vertex.position, 3, out.position );
		out.position[2] += 0.1f;
		std::copy_n( vertex.normal, 3, out.normal );
		std::copy_n( vertex.tangent, 3, out.tangentS );
		combinedWorld.vertices.push_back( out );
	}
	combinedWorld.indices = { 0, 1, 2, 0, 2, 3 };
	combinedWorld.surfaces.push_back( { 1, 0, 0, 6 } );
	WorldData cutoutBatchWorld = combinedWorld;
	cutoutBatchWorld.materials[0].variables = {
	    { "$basetexture", "batch-cutout" }, { "$alphatest", "1" } };
	cutoutBatchWorld.materials[0].textures = { { "$basetexture", 7 } };
	WorldPass combined;
	combined.SetWorld( std::move( combinedWorld ) );
	CanvasImage separateWorld, batchedWorld;
	for ( bool batch : { false, true } )
	{
		if ( auto why = render( combined, 0.25f, true, 4, black,
		         batch ? batchedWorld : separateWorld, true, RenderCoreDrawPhase::kAll, true,
		         nullptr, nullptr, false, 0.1f, true, false, false, true, batch, true ) )
			return why;
	}
	results.That( combined.Failures() == 0 && batchedWorld.rgba == separateWorld.rgba,
	    "posed-model.world-and-model-batch-exact", combined.Stats().lastFailure );
	combined.ReleaseDevice( *device );
	WorldPass cutoutBatch;
	cutoutBatch.SetWorld( std::move( cutoutBatchWorld ) );
	CanvasImage separateCutout, batchedCutout;
	for ( bool batch : { false, true } )
	{
		if ( auto why = render( cutoutBatch, 0.25f, true, 4, black,
		         batch ? batchedCutout : separateCutout, true, RenderCoreDrawPhase::kAll, true,
		         nullptr, nullptr, false, 0.1f, true, false, false, true, batch, true ) )
			return why;
	}
	results.That( cutoutBatch.Failures() == 0 && batchedCutout.rgba == separateCutout.rgba,
	    "posed-model.world-batch-preserves-cutout-coverage", cutoutBatch.Stats().lastFailure );
	cutoutBatch.ReleaseDevice( *device );
	WorldData staticWorld = MeshWorld();
	WorldData::StaticInstance frontStatic;
	for ( int i = 0; i < 4; ++i )
		frontStatic.world[i * 5] = 1.0f;
	staticWorld.staticInstances.push_back( frontStatic );
	frontStatic.world[11] = 0.1f;
	staticWorld.staticInstances.push_back( frontStatic );
	WorldPass staticPass;
	staticPass.SetWorld( std::move( staticWorld ) );
	CanvasImage staticControl, staticDepth;
	if ( auto why = render( staticPass, 0.0f, true, 4, black, staticControl, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, false, true ) )
		return why;
	if ( auto why = render( staticPass, 0.0f, true, 4, black, staticDepth, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true, true ) )
		return why;
	results.That( staticDepth.rgba == staticControl.rgba && Sum( staticDepth, 16, 32 ) > 0.1f,
	    "posed-model.static-overlapping-depth-prepass-exact",
	    "shared static geometry and instance transforms preserve every lit pixel" );
	staticPass.ReleaseDevice( *device );
	CanvasImage clippedControl, clippedDepth;
	if ( auto why = render( pass, 0.0f, true, 4, black, clippedControl, true,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, false, false, true ) )
		return why;
	if ( auto why = render( pass, 0.0f, true, 4, black, clippedDepth, true,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true, false, true ) )
		return why;
	results.That( clippedDepth.rgba == clippedControl.rgba &&
	                  clippedDepth.rgba != layersControl.rgba &&
	                  Sum( clippedDepth, 32, 48 ) > 0.1f && Sum( clippedDepth, 16, 32 ) == 0.0f,
	    "posed-model.clipped-depth-prepass-exact",
	    "fragment clip planes preserve covered and discarded samples with overlapping models" );
	WorldData cutoutWorld = MeshWorld();
	cutoutWorld.materials.push_back( cutoutWorld.materials[0] );
	cutoutWorld.materials[0].variables.push_back( { "$alphatest", "1" } );
	cutoutWorld.materials[0].variables.push_back( { "$basetexture", "model-depth-cutout" } );
	cutoutWorld.materials[0].textures.push_back( { "$basetexture", 7 } );
	// The cutout level's own staging, its uv from object position. The second
	// mesh shares it (the same geometry with the other material).
	WorldData::StaticMeshLod &cutoutLevel = cutoutWorld.staticMeshes[0].lods[0];
	std::vector<material::SurfaceModelVertex> cutout = *cutoutLevel.vertices;
	for ( auto &vertex : cutout )
	{
		vertex.uv[0] = vertex.position[0] + 0.5f;
		vertex.uv[1] = vertex.position[1] + 0.5f;
	}
	cutoutLevel.vertexCount = std::uint32_t( cutout.size() );
	cutoutLevel.vertices =
	    std::make_shared<const std::vector<material::SurfaceModelVertex>>( std::move( cutout ) );
	cutoutWorld.staticMeshes.push_back( cutoutWorld.staticMeshes[0] );
	cutoutWorld.staticMeshes[1].surfaces[0].material = 1;
	frontStatic.world[11] = 0.0f;
	cutoutWorld.staticInstances.push_back( frontStatic );
	frontStatic.world[11] = 0.1f;
	frontStatic.mesh = 1;
	cutoutWorld.staticInstances.push_back( frontStatic );
	WorldPass cutoutPass;
	cutoutPass.SetWorld( std::move( cutoutWorld ) );
	CanvasImage cutoutControl, cutoutPrepassed;
	if ( auto why = render( cutoutPass, 0.0f, true, 4, black, cutoutControl, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, false, true ) )
		return why;
	if ( auto why = render( cutoutPass, 0.0f, true, 4, black, cutoutPrepassed, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, true, true ) )
		return why;
	results.That( cutoutPrepassed.rgba == cutoutControl.rgba &&
	                  cutoutPrepassed.rgba != staticControl.rgba &&
	                  Sum( cutoutPrepassed, 16, 32 ) > 0.1f,
	    "posed-model.cutout-depth-prepass-exact",
	    "alpha-tested front geometry reveals the opaque model behind it without changing any "
	    "pixel" );
	cutoutPass.ReleaseDevice( *device );
	const float centralLight = Sum( center, 16, 32 );
	results.That( centralLight > 0.1f && centralLight > Sum( dark, 16, 32 ) * 1.5f,
	    "posed-model.lit-by-frame-light",
	    "lit " + std::to_string( centralLight ) + ", dark " +
	        std::to_string( Sum( dark, 16, 32 ) ) );
	results.That( Sum( shifted, 48, 64 ) > Sum( center, 48, 64 ) + 10.0f &&
	                  Sum( shifted, 16, 32 ) < Sum( center, 16, 32 ) * 0.5f,
	    "posed-model.current-pose-moves-pixels",
	    "right " + std::to_string( Sum( center, 48, 64 ) ) + " -> " +
	        std::to_string( Sum( shifted, 48, 64 ) ) + ", left " +
	        std::to_string( Sum( center, 16, 32 ) ) + " -> " +
	        std::to_string( Sum( shifted, 16, 32 ) ) );
	CanvasImage reused;
	if ( std::optional<std::string> why = render( pass, 0.0f, true, 4, black, reused ) )
		return why;
	results.That( reused.rgba == center.rgba && pass.Stats().viewsFailed == 0,
	    "posed-model.reused-view-resources-preserve-every-pixel" );
	WorldData backwardsModel = MeshWorld();
	backwardsModel.materials[0].shader = "UnlitGeneric";
	// The reversed winding is its own level's index staging.
	WorldData::StaticMeshLod &backwardsLevel = backwardsModel.staticMeshes[0].lods[0];
	std::vector<std::uint32_t> reversed = *backwardsLevel.indices;
	for ( unsigned i = 0; i < reversed.size(); i += 3 )
		std::swap( reversed[i + 1], reversed[i + 2] );
	backwardsLevel.indices =
	    std::make_shared<const std::vector<std::uint32_t>>( std::move( reversed ) );
	WorldData twoSidedModel = backwardsModel;
	twoSidedModel.materials[0].variables.emplace_back( "$nocull", "1" );
	WorldPass backwards, twoSided;
	backwards.SetWorld( std::move( backwardsModel ) );
	twoSided.SetWorld( std::move( twoSidedModel ) );
	CanvasImage backface, authoredBothFaces;
	if ( auto why = render( backwards, 0.0f, false, 25, black, backface ) )
		return why;
	if ( auto why = render( twoSided, 0.0f, false, 26, black, authoredBothFaces ) )
		return why;
	results.That( Sum( backface, 16, 48 ) == 0.0f && Sum( authoredBothFaces, 16, 48 ) > 100.0f &&
	                  backwards.Stats().viewsFailed == 0 && twoSided.Stats().viewsFailed == 0,
	    "posed-model.backfaces-culled-unless-authored-two-sided" );
	backwards.ReleaseDevice( *device );
	twoSided.ReleaseDevice( *device );
	WorldData color2World = MeshWorld();
	color2World.materials[0].variables.push_back( { "$color2", "[.25 .5 1]" } );
	WorldPass color2Pass;
	color2Pass.SetWorld( std::move( color2World ) );
	CanvasImage color2Image;
	if ( std::optional<std::string> why = render( color2Pass, 0.0f, true, 18, black, color2Image ) )
		return why;
	const float *basePixel = center.At( 32, 32 );
	const float *tintPixel = color2Image.At( 32, 32 );
	results.That( color2Pass.Stats().viewsFailed == 0 && color2Pass.Stats().posedDrawsDrawn == 1 &&
	                  tintPixel[0] < basePixel[0] * 0.55f && tintPixel[1] < basePixel[1] * 0.75f &&
	                  std::abs( tintPixel[2] - basePixel[2] ) < 0.005f,
	    "posed-model.color2-weights-pbr-albedo-before-lighting", color2Pass.Stats().lastFailure );
	WorldPass glass;
	glass.SetWorld( MeshWorld( true ) );
	results.That( glass.DrawsPosedModel( 0, 0 ), "posed-model.claims-alpha-blended-mesh" );
	const ClearColor background{ 0.12f, 0.24f, 0.36f, 1.0f };
	const float backgroundRgb[3] = { background.r, background.g, background.b };
	CanvasImage opaqueOverBackground, blended;
	if ( std::optional<std::string> why =
	         render( pass, 0.0f, true, 4, background, opaqueOverBackground ) )
		return why;
	if ( std::optional<std::string> why = render( glass, 0.0f, true, 5, background, blended ) )
		return why;
	bool halfOverBackground = true;
	for ( int channel = 0; channel < 3; ++channel )
	{
		const float expected =
		    0.5f * opaqueOverBackground.At( 32, 32 )[channel] + 0.5f * backgroundRgb[channel];
		halfOverBackground &= std::abs( blended.At( 32, 32 )[channel] - expected ) < 0.01f;
	}
	results.That( halfOverBackground && std::abs( blended.At( 32, 32 )[3] - 1.0f ) < 0.001f,
	    "posed-model.alpha-blends-without-alpha-write" );
	results.That( glass.Stats().viewsFailed == 0 && glass.Stats().posedDrawsDrawn == 1,
	    "posed-model.blended-view-recorded", glass.Stats().lastFailure );
	WorldData refractWorld = MeshWorld();
	refractWorld.materials[0].shader = "Refract_DX90";
	refractWorld.materials[0].variables = { { "$model", "1" },
	    { "$normalmap", "refract-normal-fixture" }, { "$refractamount", "0.07" },
	    { "$refracttint", "[1 .25 .5]" } };
	refractWorld.materials[0].textures.push_back( { "$normalmap", 3 } );
	WorldPass refract;
	refract.SetWorld( std::move( refractWorld ) );
	results.That( refract.DrawsPosedModel( 0, 0 ), "posed-model.refract-claims-scene-color-model" );
	CanvasImage refracted;
	if ( std::optional<std::string> why =
	         render( refract, 0.0f, false, 19, background, refracted ) )
		return why;
	results.That( refract.Stats().viewsFailed == 0 && refract.Stats().posedDrawsDrawn == 1,
	    "posed-model.refract-captures-scene-color", refract.Stats().lastFailure );
	results.That( std::abs( refracted.At( 32, 32 )[0] - background.r ) < 0.01f &&
	                  refracted.At( 32, 32 )[1] > background.g * 0.48f &&
	                  refracted.At( 32, 32 )[1] < background.g * 0.6f &&
	                  refracted.At( 32, 32 )[2] > background.b * 0.56f &&
	                  refracted.At( 32, 32 )[2] < background.b * 0.68f,
	    "posed-model.refract-tints-and-alpha-blends-captured-background",
	    "pixel " + std::to_string( refracted.At( 32, 32 )[0] ) + ", " +
	        std::to_string( refracted.At( 32, 32 )[1] ) + ", " +
	        std::to_string( refracted.At( 32, 32 )[2] ) );
	// Shattered glass ($nocull): the reversed pane draws only when two-sided,
	// and from behind it shades like the front (its normal faces the viewer,
	// so the silhouette fade keeps the tint instead of dropping it).
	{
		const auto paneWorld = [&]( bool reversed, bool twoSided )
		{
			WorldData world = MeshWorld();
			world.materials[0].shader = "Refract_DX90";
			world.materials[0].variables = { { "$model", "1" },
			    { "$normalmap", "refract-normal-fixture" }, { "$refractamount", "0" },
			    { "$refracttint", "[1 .25 .5]" } };
			// The fade makes the back face's normal matter; without it a drawn
			// pane is tinted however it faces, so the culled pane shows culling.
			if ( twoSided )
				world.materials[0].variables.push_back( { "$fadeoutonsilhouette", "1" } );
			if ( twoSided )
				world.materials[0].variables.push_back( { "$nocull", "1" } );
			world.materials[0].textures.push_back( { "$normalmap", 3 } );
			if ( reversed )
			{
				WorldData::StaticMeshLod &level = world.staticMeshes[0].lods[0];
				std::vector<std::uint32_t> flipped = *level.indices;
				for ( unsigned i = 0; i < flipped.size(); i += 3 )
					std::swap( flipped[i + 1], flipped[i + 2] );
				level.indices =
				    std::make_shared<const std::vector<std::uint32_t>>( std::move( flipped ) );
			}
			return world;
		};
		CanvasImage front, back, culled;
		const struct
		{
			bool reversed, twoSided;
			CanvasImage *image;
		} panes[] = { { false, true, &front }, { true, true, &back }, { true, false, &culled } };
		std::uint64_t frame = 30;
		std::string paneNotes;
		for ( const auto &pane : panes )
		{
			WorldPass glassPane;
			glassPane.SetWorld( paneWorld( pane.reversed, pane.twoSided ) );
			// Seen from behind: reversed winding and the normal facing away.
			poseNormalSign = pane.reversed ? -1.0f : 1.0f;
			auto why = render( glassPane, 0, false, frame++, background, *pane.image );
			poseNormalSign = 1.0f;
			if ( why )
				return why;
			paneNotes += std::to_string( glassPane.Stats().posedDrawsDrawn ) + "/" +
			             std::to_string( glassPane.Failures() ) + " " +
			             glassPane.Stats().lastFailure + "; ";
			glassPane.ReleaseDevice( *device );
		}
		bool same = true, tinted = front.At( 32, 32 )[1] < background.g * 0.8f;
		for ( int c = 0; c < 3; ++c )
			same &= std::abs( front.At( 32, 32 )[c] - back.At( 32, 32 )[c] ) < 0.01f;
		bool culledAway = true;
		for ( int c = 0; c < 3; ++c )
			culledAway &= std::abs( culled.At( 32, 32 )[c] - backgroundRgb[c] ) < 0.01f;
		results.That( tinted && same, "posed-model.refract-nocull-back-face-shades-like-front",
		    "front g " + std::to_string( front.At( 32, 32 )[1] ) + ", back g " +
		        std::to_string( back.At( 32, 32 )[1] ) + "; drawn/failures " + paneNotes );
		results.That( culledAway, "posed-model.refract-back-face-culled-without-nocull" );
	}
	CanvasImage missingCapture;
	if ( std::optional<std::string> why = render( refract, 0.0f, false, 20, background,
	         missingCapture, false, RenderCoreDrawPhase::kAll, false ) )
		return why;
	results.That( refract.Stats().viewsFailed == 1 &&
	                  std::abs( missingCapture.At( 32, 32 )[0] - background.r ) < 0.01f,
	    "posed-model.refract-refuses-view-without-capture-source" );
	// Native glass must preserve a uniform radiance field, and reflected HDR
	// radiance must remain linear even with the authored contrast control.
	WorldData coatingWorld = MeshWorld();
	coatingWorld.materials[0].shader = "Refract_DX90";
	coatingWorld.materials[0].variables = { { "$model", "1" },
	    { "$normalmap", "refract-normal-fixture" }, { "$envmap", "glass-environment" },
	    { "$envmaptint", "[1 1 1]" }, { "$envmapcontrast", "1" } };
	coatingWorld.materials[0].textures = { { "$normalmap", 3 }, { "$envmap", 8 } };
	for ( float radiance : { 1.0f, 8.0f } )
	{
		TextureDesc cubeDesc = normalDesc;
		cubeDesc.format = Format::kRGBA16Float;
		cubeDesc.dimension = TextureDimension::kCube;
		cubeDesc.depthOrLayers = 6;
		std::array<std::uint16_t, 24> cubePixels;
		for ( std::size_t i = 0; i < cubePixels.size(); ++i )
			cubePixels[i] = FloatToHalf( i % 4 == 3 ? 1.0f : radiance );
		auto cube = textures.Stage( "glass-environment-" + std::to_string( radiance ), cubeDesc,
		    std::as_bytes( std::span( cubePixels ) ) );
		if ( !cube )
			return "the glass HDR environment could not be staged";
		empty.environmentFixture = cube.Value().texture;
		WorldPass coating;
		coating.SetWorld( coatingWorld );
		CanvasImage coated;
		if ( auto why = render( coating, 0, false, 21, { 1, 1, 1, 1 }, coated ) )
			return why;
		// The fixture's alpha mask is 128/255; contrast squares its coating.
		// At normal incidence the shared dielectric Fresnel is 0.04.
		const float mask = 128.0f / 255.0f;
		const float expected = 1.0f + ( radiance - 1.0f ) * mask * mask * 0.04f;
		bool bounded = coating.Failures() == 0;
		for ( int c = 0; c < 3; ++c )
			bounded &= std::abs( coated.At( 32, 32 )[c] - expected ) < 0.004f;
		results.That( bounded,
		    radiance == 1.0f ? "posed-model.refract-preserves-white-furnace"
		                     : "posed-model.refract-HDR-reflection-is-linear",
		    "pixel " + std::to_string( coated.At( 32, 32 )[0] ) + ", expected " +
		        std::to_string( expected ) );
		coating.ReleaseDevice( *device );
	}
	WorldData underWorld = MeshWorld();
	underWorld.materials[0].shader = "UnlitGeneric";
	WorldPass under;
	under.SetWorld( std::move( underWorld ) );
	WorldData warpWorld = MeshWorld();
	warpWorld.materials[0].shader = "Refract_DX90";
	warpWorld.materials[0].variables = {
	    { "$model", "1" }, { "$normalmap", "refract-warp-fixture" }, { "$refractamount", "0.3" } };
	warpWorld.materials[0].textures.push_back( { "$normalmap", 4 } );
	WorldData zeroWorld = warpWorld;
	zeroWorld.materials[0].variables.back().second = "0";
	WorldPass zeroWarp;
	WorldPass displaced;
	zeroWarp.SetWorld( std::move( zeroWorld ) );
	displaced.SetWorld( std::move( warpWorld ) );
	CanvasImage noWarp, withWarp;
	if ( std::optional<std::string> why = render( zeroWarp, 0.0f, false, 21, black, noWarp, false,
	         RenderCoreDrawPhase::kAll, true, &under ) )
		return why;
	if ( std::optional<std::string> why = render( displaced, 0.0f, false, 22, black, withWarp,
	         false, RenderCoreDrawPhase::kAll, true, &under ) )
		return why;
	results.That( zeroWarp.Stats().viewsFailed == 0 && displaced.Stats().viewsFailed == 0 &&
	                  noWarp.At( 40, 32 )[0] > withWarp.At( 40, 32 )[0] + 0.15f,
	    "posed-model.refract-displaces-authored-scene-edge",
	    "no warp " + std::to_string( noWarp.At( 40, 32 )[0] ) + ", warp " +
	        std::to_string( withWarp.At( 40, 32 )[0] ) );
	// Portal 2's $localrefract (refract_ps2x LOCALREFRACT): the base texture
	// refracted in texture space, never the scene behind the surface. The
	// fixture's eye looks straight down the quad's axis at the centre pixel,
	// so the eye term of the offset is zero there.
	{
		const auto localMaterial = []( const char *normal, const char *depth )
		{
			WorldData world = MeshWorld();
			world.materials[0].shader = "Refract_DX90";
			world.materials[0].variables = { { "$model", "1" }, { "$normalmap", normal },
			    { "$basetexture", "local-base" }, { "$localrefract", "1" },
			    { "$localrefractdepth", depth }, { "$refractamount", ".025" },
			    { "$bluramount", "1" } };
			return world;
		};
		const auto srgb = []( float v )
		{
			return v <= 0.04045f ? v / 12.92f : std::pow( ( v + 0.055f ) / 1.055f, 2.4f );
		};
		// Uniform base (two-texture-base, sRGB 128 192 255 alpha 64), flat
		// normal: the base, 2.5 % towards its own alpha, times normal z cubed.
		const float baseAlpha = 64.0f / 255.0f;
		const float baseLinear[3] = {
		    srgb( 128.0f / 255.0f ), srgb( 192.0f / 255.0f ), srgb( 255.0f / 255.0f ) };
		struct Case
		{
			const char *name;
			int normalHandle;
			float zCubed;
		};
		const float tiltedZ = 170.0f / 255.0f * 2.0f - 1.0f;
		for ( const Case &c : { Case{ "posed-model.local-refract-flat-normal-draws-base", 3, 1.0f },
		          Case{ "posed-model.local-refract-normal-z-cubed-darkens", 1,
		              tiltedZ * tiltedZ * tiltedZ } } )
		{
			WorldData world = localMaterial( "fixture-normal", "0.05" );
			world.materials[0].textures = {
			    { "$normalmap", c.normalHandle }, { "$basetexture", 5 } };
			WorldPass local;
			local.SetWorld( std::move( world ) );
			CanvasImage image;
			if ( auto why = render( local, 0, false, 24, background, image, false,
			         RenderCoreDrawPhase::kAll, false ) )
				return why;
			bool exact = local.Failures() == 0 && local.Stats().posedDrawsDrawn == 1;
			std::string detail;
			for ( int ch = 0; ch < 3; ++ch )
			{
				const float expected = ( baseLinear[ch] * 0.975f + baseAlpha * 0.025f ) * c.zCubed;
				exact &= std::abs( image.At( 32, 32 )[ch] - expected ) < 0.004f;
				detail += std::to_string( image.At( 32, 32 )[ch] ) + "/" +
				          std::to_string( expected ) + " ";
			}
			results.That( exact, c.name, detail + local.Stats().lastFailure );
			local.ReleaseDevice( *device );
		}
		// The local point reads no scene color: it claims without a capture,
		// draws opaque, and its pixel does not change with the background.
		WorldData sceneFree = localMaterial( "fixture-normal", "0.05" );
		sceneFree.materials[0].textures = { { "$normalmap", 3 }, { "$basetexture", 5 } };
		const auto localDesc = material::MapVariables( "Refract_DX90",
		    { { "$model", "1" }, { "$normalmap", "n" }, { "$basetexture", "b" },
		        { "$localrefract", "1" }, { "$localrefractdepth", "0.05" } },
		    {} );
		const auto claimed = localDesc
		                         ? material::ClaimForMesh( localDesc.Value(), false, false )
		                         : foundation::Expected<BlendMode, std::string>(
		                               foundation::MakeUnexpected( std::string( "unmapped" ) ) );
		results.That( claimed && claimed.Value() == BlendMode::kOpaque,
		    "posed-model.local-refract-claims-opaque-without-scene-color",
		    claimed ? "" : claimed.Error() );
		WorldPass sceneFreePass;
		sceneFreePass.SetWorld( std::move( sceneFree ) );
		CanvasImage overDark, overBright;
		if ( auto why = render( sceneFreePass, 0, false, 25, black, overDark, false,
		         RenderCoreDrawPhase::kAll, false ) )
			return why;
		if ( auto why = render( sceneFreePass, 0, false, 26, { 1, 1, 1, 1 }, overBright, false,
		         RenderCoreDrawPhase::kAll, false ) )
			return why;
		bool same = sceneFreePass.Failures() == 0;
		for ( int ch = 0; ch < 3; ++ch )
			same &= std::abs( overDark.At( 32, 32 )[ch] - overBright.At( 32, 32 )[ch] ) < 1e-4f;
		results.That( same, "posed-model.local-refract-ignores-scene-behind" );
		sceneFreePass.ReleaseDevice( *device );
		// $localrefractdepth scales the normal's texture-space offset: depth 0
		// keeps the lookup at the red column, depth 0.8 with the +x warp normal
		// moves it into the green column (2x2 fixture, aspect 1).
		CanvasImage atDepth[2];
		for ( int i = 0; i < 2; ++i )
		{
			WorldData world = localMaterial( "fixture-warp", i == 0 ? "0" : "0.8" );
			world.materials[0].textures = { { "$normalmap", 4 }, { "$basetexture", 10 } };
			WorldPass pass2;
			pass2.SetWorld( std::move( world ) );
			if ( auto why = render( pass2, 0, false, 27 + i, background, atDepth[i], false,
			         RenderCoreDrawPhase::kAll, false ) )
				return why;
			pass2.ReleaseDevice( *device );
		}
		results.That( atDepth[0].At( 32, 32 )[0] > 0.9f && atDepth[0].At( 32, 32 )[1] < 0.05f &&
		                  atDepth[1].At( 32, 32 )[1] > 0.9f && atDepth[1].At( 32, 32 )[0] < 0.05f,
		    "posed-model.local-refract-depth-moves-base-lookup",
		    "depth 0 rg " + std::to_string( atDepth[0].At( 32, 32 )[0] ) + " " +
		        std::to_string( atDepth[0].At( 32, 32 )[1] ) + ", depth .8 rg " +
		        std::to_string( atDepth[1].At( 32, 32 )[0] ) + " " +
		        std::to_string( atDepth[1].At( 32, 32 )[1] ) );
		// Named refusals: the local point has no silhouette fade or tint
		// texture, and refracts nothing without a base texture.
		const auto refused = [&]( const std::vector<material::VmtPair> &extra, bool withBase )
		{
			std::vector<material::VmtPair> vars = {
			    { "$model", "1" }, { "$normalmap", "n" }, { "$localrefract", "1" } };
			if ( withBase )
				vars.push_back( { "$basetexture", "b" } );
			for ( auto &e : extra )
				vars.push_back( e );
			const auto desc = material::MapVariables( "Refract_DX90", vars, {} );
			return desc && !material::ClaimForMesh( desc.Value(), true, true );
		};
		const std::vector<material::VmtPair> fade = { { "$fadeoutonsilhouette", "1" } };
		const std::vector<material::VmtPair> tintTexture = { { "$refracttinttexture", "t" } };
		results.That( refused( fade, true ) && refused( tintTexture, true ) && refused( {}, false ),
		    "posed-model.local-refract-refuses-unported-controls" );
		// A base transform moves the local point's texture-space lookup; the
		// screen-space point has no base coordinates and keeps refusing it.
		const auto transformed = []( bool local )
		{
			std::vector<material::VmtPair> vars = { { "$model", "1" }, { "$normalmap", "n" },
			    { "$basetexture", "b" },
			    { "$basetexturetransform", "center .5 .5 scale 2 2 rotate 0 translate 0 0" } };
			if ( local )
				vars.push_back( { "$localrefract", "1" } );
			const auto desc = material::MapVariables( "Refract_DX90", vars, {} );
			return desc && bool( material::ClaimForMesh( desc.Value(), true, true ) );
		};
		results.That( transformed( true ) && !transformed( false ),
		    "posed-model.refract-base-transform-only-on-local-point" );
	}
	// $bumptransform (refract_vs20's normal map coordinates; a TextureScroll
	// proxy animates it): translated by .75 the lookup leaves the flat texel
	// for the warp texel and displaces the scene edge as the warp fixture does.
	{
		CanvasImage scrolled[2];
		for ( int i = 0; i < 2; ++i )
		{
			WorldData world = MeshWorld();
			world.materials[0].shader = "Refract_DX90";
			world.materials[0].variables = { { "$model", "1" },
			    { "$normalmap", "split-normal-fixture" }, { "$refractamount", "0.3" },
			    { "$bumptransform", i == 0 ? "center .5 .5 scale 1 1 rotate 0 translate 0 0"
			                               : "center .5 .5 scale 1 1 rotate 0 translate .75 0" } };
			world.materials[0].textures.push_back( { "$normalmap", 11 } );
			WorldPass scrollPass;
			scrollPass.SetWorld( std::move( world ) );
			if ( auto why = render( scrollPass, 0.0f, false, 50 + i, black, scrolled[i], false,
			         RenderCoreDrawPhase::kAll, true, &under ) )
				return why;
			scrollPass.ReleaseDevice( *device );
		}
		results.That( scrolled[0].At( 40, 32 )[0] > scrolled[1].At( 40, 32 )[0] + 0.15f &&
		                  std::abs( scrolled[0].At( 40, 32 )[0] - noWarp.At( 40, 32 )[0] ) < 0.01f,
		    "posed-model.refract-bump-transform-moves-normal-lookup",
		    "identity " + std::to_string( scrolled[0].At( 40, 32 )[0] ) + ", scrolled " +
		        std::to_string( scrolled[1].At( 40, 32 )[0] ) );
	}
	// The neurotoxin tube's live values (AnimatedTexture on $normalmap and
	// $dudvmap, TextureScroll on $bumptransform) claim: the handoff binds the
	// $bumpframe's handle, $dudvframe names a map Refract never samples. A
	// scrolled base texture on the screen-space point stays refused.
	{
		const auto animated = material::MapVariables( "Refract_DX90",
		    { { "$model", "1" }, { "$normalmap", "dev/water_normal" },
		        { "$dudvmap", "dev/water_dudv" }, { "$bumpframe", "17" }, { "$dudvframe", "17" },
		        { "$bumptransform", "center .5 .5 scale 1 1 rotate 0 translate .31 .31" },
		        { "$refractamount", ".2" }, { "$bluramount", "1" },
		        { "$refracttint", "{255 200 110}" },
		        { "$refracttinttexture", "models/props_lab/glass_tint001" } },
		    {} );
		const auto scrolledBase = material::MapVariables( "Refract_DX90",
		    { { "$model", "1" }, { "$normalmap", "n" }, { "$basetexture", "b" },
		        { "$basetexturetransform", "center .5 .5 scale 1 1 rotate 0 translate .5 0" } },
		    {} );
		results.That( animated && material::ClaimForMesh( animated.Value(), true, true ) &&
		                  scrolledBase &&
		                  !material::ClaimForMesh( scrolledBase.Value(), true, true ),
		    "posed-model.refract-animated-normal-frames-claim" );
	}
	// A smaller or offset viewport shares a full-size scene-color attachment
	// in the game. It must sample the scene beneath that pixel, not clear
	// pixels beyond the viewport (the core-only hatch in the game).
	for ( const Viewport viewport :
	    { Viewport{ 0, 0, 32, 32, 0, 1 }, Viewport{ 16, 8, 32, 32, 0, 1 } } )
	{
		CanvasImage smallView;
		if ( auto why = render( zeroWarp, 0, false, 23, black, smallView, false,
		         RenderCoreDrawPhase::kAll, true, &under, nullptr, false, 0.1f, false, false, false,
		         false, false, false, false, viewport ) )
			return why;
		const auto pixel =
		    smallView.At( std::uint32_t( viewport.x ) + 20, std::uint32_t( viewport.y ) + 16 );
		results.That( zeroWarp.Stats().viewsFailed == 0 && pixel[0] > 0.9f,
		    viewport.x == 0 ? "posed-model.refract-reduced-viewport-samples-attachment-pixels"
		                    : "posed-model.refract-offset-viewport-samples-attachment-pixels",
		    "transmitted red " + std::to_string( pixel[0] ) );
	}
	// The captured background already includes exposure and fog. A clear
	// transmitting pane must preserve that attachment through either change.
	for ( float exposure : { 0.25f, 4.0f } )
		for ( bool fog : { false, true } )
		{
			CanvasImage preserved;
			if ( auto why = render( zeroWarp, 0, false, 23, background, preserved, false,
			         RenderCoreDrawPhase::kAll, true, nullptr, nullptr, false, 0.1f, false, false,
			         false, false, false, false, false, std::nullopt, exposure, fog ) )
				return why;
			const auto pixel = preserved.At( 32, 32 );
			results.That( zeroWarp.Failures() == 0 && std::abs( pixel[0] - background.r ) < .002f &&
			                  std::abs( pixel[1] - background.g ) < .002f &&
			                  std::abs( pixel[2] - background.b ) < .002f,
			    "posed-model.refract-captured-scene-output-once-" + std::to_string( exposure ) +
			        ( fog ? "-fog" : "-clear" ),
			    "transmitted red " + std::to_string( pixel[0] ) );
		}
	CanvasImage singlePane, twoPanes;
	if ( std::optional<std::string> why = render( glass, 0.0f, true, 6, black, singlePane ) )
		return why;
	if ( std::optional<std::string> why = render( glass, 0.0f, true, 7, black, twoPanes, true ) )
		return why;
	results.That( twoPanes.At( 32, 32 )[0] > singlePane.At( 32, 32 )[0] * 1.1f,
	    "posed-model.blended-front-pane-does-not-write-depth" );
	WorldPass mixed;
	mixed.SetWorld( MixedWorld() );
	results.That( mixed.DrawsPosedModel( 0, 0, RenderCoreDrawPhase::kOpaque ) &&
	                  mixed.DrawsPosedModel( 0, 0, RenderCoreDrawPhase::kBlended ),
	    "posed-model.claims-both-studio-phases" );
	CanvasImage opaquePhase, blendedPhase;
	if ( std::optional<std::string> why = render(
	         mixed, 0.0f, true, 8, background, opaquePhase, false, RenderCoreDrawPhase::kOpaque ) )
		return why;
	if ( std::optional<std::string> why = render( mixed, 0.0f, true, 9, background, blendedPhase,
	         false, RenderCoreDrawPhase::kBlended ) )
		return why;
	bool phasesMatch = true;
	for ( int channel = 0; channel < 3; ++channel )
	{
		phasesMatch &= std::abs( opaquePhase.At( 32, 32 )[channel] -
		                         opaqueOverBackground.At( 32, 32 )[channel] ) < 0.01f;
		phasesMatch &=
		    std::abs( blendedPhase.At( 32, 32 )[channel] - blended.At( 32, 32 )[channel] ) < 0.01f;
	}
	results.That( phasesMatch && mixed.Stats().viewsFailed == 0,
	    "posed-model.two-pass-surfaces-stay-in-their-phases", mixed.Stats().lastFailure );
	WorldData floorWorld = MeshWorld();
	floorWorld.materials[0].variables.push_back( { "$selfillum", "1" } );
	floorWorld.materials[0].variables.push_back( { "$color", "[.5 .5 .5]" } );
	floorWorld.materials[0].variables.push_back( { "$selfillumtint", "[.7 .85 1]" } );
	WorldPass floor;
	floor.SetWorld( std::move( floorWorld ) );
	results.That( floor.DrawsPosedModel( 0, 0 ), "posed-model.claims-self-lit-elevator-floor" );
	CanvasImage selfLit, selfLitWithLights;
	if ( std::optional<std::string> why = render( floor, 0.0f, false, 10, black, selfLit ) )
		return why;
	if ( std::optional<std::string> why =
	         render( floor, 0.0f, true, 11, black, selfLitWithLights ) )
		return why;
	bool maskedReplacement = true;
	for ( int channel = 0; channel < 3; ++channel )
		maskedReplacement &= std::abs( selfLit.At( 32, 32 )[channel] -
		                               selfLitWithLights.At( 32, 32 )[channel] ) < 0.005f;
	results.That( maskedReplacement && selfLit.At( 32, 32 )[0] > 0.05f &&
	                  std::abs( selfLit.At( 32, 32 )[3] - 1.0f ) < 0.001f,
	    "posed-model.self-lit-mask-replaces-lighting-without-alpha-cutout" );
	results.That( selfLit.At( 32, 32 )[0] < selfLit.At( 32, 32 )[1] &&
	                  selfLit.At( 32, 32 )[1] < selfLit.At( 32, 32 )[2],
	    "posed-model.self-illumination-tint-is-authored" );
	WorldData bumpWorld = MeshWorld();
	bumpWorld.materials[0].variables.push_back( { "$bumpmap", "ssbump-fixture" } );
	bumpWorld.materials[0].textures.push_back( { "$bumpmap", 1 } );
	WorldData ssbumpWorld = bumpWorld;
	ssbumpWorld.materials[0].variables.push_back( { "$ssbump", "1" } );
	WorldPass bump;
	WorldPass ssbumpPass;
	bump.SetWorld( std::move( bumpWorld ) );
	ssbumpPass.SetWorld( std::move( ssbumpWorld ) );
	CanvasImage regularNormal, ssbumpNormal;
	if ( std::optional<std::string> why = render( bump, 0.0f, true, 12, black, regularNormal ) )
		return why;
	if ( std::optional<std::string> why =
	         render( ssbumpPass, 0.0f, true, 13, black, ssbumpNormal ) )
		return why;
	results.That(
	    bump.Stats().viewsFailed == 0 && ssbumpPass.Stats().viewsFailed == 0 &&
	        bump.Stats().posedDrawsDrawn == 1 && ssbumpPass.Stats().posedDrawsDrawn == 1 &&
	        std::abs( regularNormal.At( 32, 32 )[0] - ssbumpNormal.At( 32, 32 )[0] ) > 0.005f,
	    "posed-model.ssbump-basis-changes-pbr-lighting", ssbumpPass.Stats().lastFailure );
	WorldData plainSpecWorld = MeshWorld();
	plainSpecWorld.materials[0].variables.push_back( { "$phong", "1" } );
	plainSpecWorld.materials[0].variables.push_back( { "$phongexponent", "2" } );
	plainSpecWorld.materials[0].variables.push_back( { "$phongboost", "6" } );
	WorldData warpedSpecWorld = plainSpecWorld;
	warpedSpecWorld.materials[0].variables.push_back(
	    { "$phongwarptexture", "phong-warp-fixture" } );
	warpedSpecWorld.materials[0].textures.push_back( { "$phongwarptexture", 2 } );
	WorldPass plainSpec;
	WorldPass warpedSpec;
	plainSpec.SetWorld( std::move( plainSpecWorld ) );
	warpedSpec.SetWorld( std::move( warpedSpecWorld ) );
	CanvasImage plainSpecImage, warpedSpecImage;
	if ( std::optional<std::string> why =
	         render( plainSpec, 0.0f, true, 14, black, plainSpecImage ) )
		return why;
	if ( std::optional<std::string> why =
	         render( warpedSpec, 0.0f, true, 15, black, warpedSpecImage ) )
		return why;
	results.That( plainSpec.Stats().viewsFailed == 0 && warpedSpec.Stats().viewsFailed == 0 &&
	                  warpedSpec.Stats().posedDrawsDrawn == 1 &&
	                  warpedSpecImage.At( 32, 32 )[1] < plainSpecImage.At( 32, 32 )[1] * 0.9f,
	    "posed-model.phong-warp-colors-direct-pbr-specular", warpedSpec.Stats().lastFailure );
	WorldData unlitWorld = MeshWorld();
	unlitWorld.materials[0].shader = "UnlitGeneric";
	WorldPass unlit;
	unlit.SetWorld( std::move( unlitWorld ) );
	results.That( unlit.DrawsPosedModel( 0, 0 ), "posed-model.claims-emissive-unlit-mesh" );
	CanvasImage unlitDark, unlitBright;
	if ( std::optional<std::string> why = render( unlit, 0.0f, false, 16, black, unlitDark ) )
		return why;
	if ( std::optional<std::string> why = render( unlit, 0.0f, true, 17, black, unlitBright ) )
		return why;
	bool independentOfLight = true;
	for ( int channel = 0; channel < 3; ++channel )
		independentOfLight &= std::abs( unlitDark.At( 32, 32 )[channel] -
		                                unlitBright.At( 32, 32 )[channel] ) < 0.005f;
	results.That( independentOfLight && unlitDark.At( 32, 32 )[0] > 0.05f,
	    "posed-model.emissive-mesh-ignores-direct-light" );
	WorldData depthWorld = MeshWorld();
	depthWorld.materials[0].shader = "WriteZ_DX9";
	WorldPass depthOnly;
	depthOnly.SetWorld( std::move( depthWorld ) );
	CanvasImage occluded, depthBehind;
	if ( auto why = render( unlit, 0.0f, false, 30, background, occluded, false,
	         RenderCoreDrawPhase::kAll, true, &depthOnly, nullptr, false, -0.1f ) )
		return why;
	if ( auto why = render( unlit, 0.0f, false, 31, background, depthBehind, false,
	         RenderCoreDrawPhase::kAll, true, &depthOnly ) )
		return why;
	bool untouchedColor = true;
	for ( int channel = 0; channel < 3; ++channel )
		untouchedColor &=
		    std::abs( occluded.At( 32, 32 )[channel] - backgroundRgb[channel] ) < 0.005f;
	results.That( depthOnly.Failures() == 0 && depthOnly.Stats().posedDrawsDrawn == 2 &&
	                  untouchedColor && depthBehind.At( 32, 32 )[0] > background.r + 0.1f,
	    "posed-model.writez-occludes-only-behind-it-without-writing-color",
	    depthOnly.Stats().lastFailure );
	// The legacy WriteZ point has no texture sampling or alpha-test shader.
	const auto cutoutDepth = material::MapVariables( "WriteZ_DX9", { { "$alphatest", "1" } }, {} );
	results.That( cutoutDepth && !material::ClaimForMesh( cutoutDepth.Value(), false ),
	    "posed-model.writez-does-not-claim-an-unimplemented-alpha-mask" );
	depthOnly.ReleaseDevice( *device );
	// The decal texture is dimensionless data, not sRGB color. Its 0.5
	// neutral factor must survive exposure, fog and destination alpha.
	TextureDesc decalDesc;
	decalDesc.format = Format::kRGBA8Unorm;
	decalDesc.width = 2;
	decalDesc.height = 1;
	decalDesc.usages = { ResourceUsage::kSampled };
	const std::array<std::byte, 8> decalPixels = { std::byte{ 128 }, std::byte{ 64 },
	    std::byte{ 192 }, std::byte{ 128 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 },
	    std::byte{ 0 } };
	auto decalTexture = textures.Stage( "modulate-decal-data", decalDesc, decalPixels );
	if ( !decalTexture )
		return "the decal data fixture could not be staged";
	empty.decalFixture = decalTexture.Value().texture;
	WorldMaterial decalMaterial;
	decalMaterial.name = "modulate-decal-fixture";
	decalMaterial.shader = "DecalModulate_DX9";
	decalMaterial.variables = { { "$basetexture", "modulate-decal-data" }, { "$decal", "1" },
	    { "$vertexcolor", "1" }, { "$vertexalpha", "1" } };
	decalMaterial.textures = { { "$basetexture", 9 } };
	WorldPass decalPass;
	decalPass.SetWorld( MeshWorld() );
	auto renderDecal = [&]( float uv, float fog, float outputScale, CanvasImage &image,
	                       const WorldMaterial *overlay = nullptr ) -> std::optional<std::string>
	{
		WorldView view;
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 1.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		WorldView::DynamicDraw draw;
		draw.material = overlay ? *overlay : decalMaterial;
		if ( overlay )
		{
			draw.lightmapPage = 9;
			draw.capturedLightmap = true;
		}
		for ( const auto &vertex : bindPose )
		{
			WorldVertex v;
			std::copy_n( vertex.position, 3, v.position );
			v.uv[0] = uv;
			v.uv[1] = 0.5f;
			v.lightmapUv[0] = 0.25f;
			v.lightmapUv[1] = 0.5f;
			// DecalModulate ignores even authored vertex color/alpha flags.
			std::fill_n( v.color, 4, 0 );
			draw.vertices.push_back( v );
		}
		draw.indices = { 0, 1, 2, 0, 2, 3 };
		view.dynamicDraws.push_back( std::move( draw ) );
		const auto tag = decalPass.QueueView( std::move( view ) );
		if ( !tag )
			return "decal queue refused: " + decalPass.Stats().lastRefusal;
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId depth ) -> std::optional<std::string>
		{
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.depth = depth;
			target.depthFormat = kCanvasDepth;
			target.width = target.height = kSize;
			target.textures = &empty;
			target.frame = 25;
			target.outputScale = outputScale;
			if ( fog >= 0.0f )
			{
				target.fogType = 0.0f;
				target.fogParams[0] = -fog;
				target.fogParams[2] = 1.0f;
			}
			decalPass.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {}, { 0.8f, 0.6f, 0.4f, 0.25f }, &image, post );
	};
	CanvasImage decal, exposedDecal, foggedDecal, emptyDecal;
	for ( auto [uv, fog, scale, image] :
	    { std::tuple{ 0.25f, -1.0f, 1.0f, &decal }, std::tuple{ 0.25f, -1.0f, 3.0f, &exposedDecal },
	        std::tuple{ 0.25f, 0.5f, 3.0f, &foggedDecal },
	        std::tuple{ 0.75f, -1.0f, 1.0f, &emptyDecal } } )
	{
		if ( auto why = renderDecal( uv, fog, scale, *image ) )
			return why;
	}
	const auto *d = decal.At( kSize / 2, kSize / 2 );
	const auto *f = foggedDecal.At( kSize / 2, kSize / 2 );
	const auto *e = emptyDecal.At( kSize / 2, kSize / 2 );
	const float destination[] = { 0.8f, 0.6f, 0.4f };
	const float factor[] = { 128.0f / 255.0f, 64.0f / 255.0f, 192.0f / 255.0f };
	bool modulation = true, neutralFog = true, discarded = true;
	for ( int c = 0; c < 3; ++c )
	{
		modulation &= std::fabs( d[c] - 2.0f * factor[c] * destination[c] ) < 0.002f;
		const float fogWeight = std::pow( 0.5f, 0.8f );
		neutralFog &=
		    std::fabs( f[c] - 2.0f * ( factor[c] * ( 1.0f - fogWeight ) + 0.5f * fogWeight ) *
		                          destination[c] ) < 0.002f;
		discarded &= std::fabs( e[c] - destination[c] ) < 0.002f;
	}
	results.That( modulation && decalPass.Failures() == 0,
	    "posed-model.decal-modulates-lit-destination", decalPass.Stats().lastFailure );
	results.That( decal.rgba == exposedDecal.rgba,
	    "posed-model.decal-neutral-factor-independent-of-exposure" );
	results.That( neutralFog, "posed-model.decal-fogs-toward-neutral-factor" );
	results.That( std::fabs( d[3] - 0.25f ) < 0.001f && std::fabs( f[3] - 0.25f ) < 0.001f,
	    "posed-model.decal-preserves-destination-alpha" );
	results.That( discarded, "posed-model.decal-zero-alpha-discards" );
	// Modulate (modulate_ps2x) on the same point: saturate( base x $color ),
	// its color lerped from 0.5 by its alpha; 2x with $mod2x, else the
	// destination times the factor (drawn as the 2x blend of half of it). A
	// zero-alpha texel is the neutral 0.5, not discarded.
	{
		WorldMaterial modulate;
		modulate.name = "modulate-fixture";
		modulate.shader = "Modulate";
		modulate.variables = { { "$basetexture", "modulate-decal-data" }, { "$mod2x", "1" },
		    { "$color", "[0.5 1 1]" } };
		modulate.textures = { { "$basetexture", 9 } };
		WorldMaterial plain = modulate;
		plain.variables = { { "$basetexture", "modulate-decal-data" } };
		CanvasImage doubled, single;
		if ( auto why = renderDecal( 0.25f, -1.0f, 1.0f, doubled, &modulate ) )
			return why;
		if ( auto why = renderDecal( 0.75f, -1.0f, 1.0f, single, &plain ) )
			return why;
		const auto *m = doubled.At( kSize / 2, kSize / 2 );
		const auto *p = single.At( kSize / 2, kSize / 2 );
		const float tint[] = { 0.5f, 1.0f, 1.0f };
		const float alpha = 128.0f / 255.0f;
		bool mod2x = true, half = true;
		for ( int c = 0; c < 3; ++c )
		{
			const float weight = 0.5f + ( factor[c] * tint[c] - 0.5f ) * alpha;
			mod2x &= std::fabs( m[c] - 2.0f * weight * destination[c] ) < 0.003f;
			half &= std::fabs( p[c] - 0.5f * destination[c] ) < 0.003f;
		}
		results.That( mod2x && decalPass.Failures() == 0,
		    "posed-model.modulate-mod2x-lerps-tinted-base-from-neutral",
		    std::to_string( m[0] ) + " " + std::to_string( m[1] ) + " " + std::to_string( m[2] ) );
		results.That( half, "posed-model.modulate-plain-multiplies-by-neutral-half",
		    std::to_string( p[0] ) + " " + std::to_string( p[1] ) + " " + std::to_string( p[2] ) );
		const auto cloaked = material::MapVariables( "Modulate",
		    { { "$basetexture", "decal" }, { "$cloakpassenabled", "1" } }, {} );
		results.That( cloaked && !material::ClaimForDrawing( cloaked.Value() ),
		    "posed-model.modulate-refuses-cloak-pass" );
	}
	// Wireframe (and Eyeball, which falls back to it) draws UnlitGeneric's
	// quad as its triangles' edges: where UnlitGeneric fills the center the
	// wireframe leaves the clear color (device line fill, clause D38).
	if ( device->Facts().capabilities.Has( Capability::kFillModeLines ) )
	{
		WorldMaterial filled;
		filled.name = "wireframe-fixture-filled";
		filled.shader = "UnlitGeneric";
		filled.variables = { { "$basetexture", "modulate-decal-data" } };
		filled.textures = { { "$basetexture", 9 } };
		WorldMaterial edges = filled;
		edges.name = "wireframe-fixture";
		edges.shader = "Wireframe";
		WorldMaterial eyeball = edges;
		eyeball.name = "eyeball-fixture";
		eyeball.shader = "Eyeball";
		CanvasImage solid, wire, dead;
		if ( auto why = renderDecal( 0.25f, -1.0f, 1.0f, solid, &filled ) )
			return why;
		if ( auto why = renderDecal( 0.25f, -1.0f, 1.0f, wire, &edges ) )
			return why;
		if ( auto why = renderDecal( 0.25f, -1.0f, 1.0f, dead, &eyeball ) )
			return why;
		// The quad's two triangles share a diagonal, so the wireframe covers
		// a thin cross of lines: some pixels, far fewer than the filled quad.
		const float clear[] = { 0.8f, 0.6f, 0.4f };
		auto covered = [&]( const CanvasImage &image )
		{
			int count = 0;
			for ( std::uint32_t y = 0; y < kSize; ++y )
				for ( std::uint32_t x = 0; x < kSize; ++x )
				{
					const float *pixel = image.At( x, y );
					count += std::fabs( pixel[0] - clear[0] ) > 0.002f ||
					         std::fabs( pixel[1] - clear[1] ) > 0.002f ||
					         std::fabs( pixel[2] - clear[2] ) > 0.002f;
				}
			return count;
		};
		const int filledCount = covered( solid ), wireCount = covered( wire ),
		          eyeballCount = covered( dead );
		results.That( filledCount > 0 && wireCount > 0 && 4 * wireCount < filledCount &&
		                  eyeballCount == wireCount && decalPass.Failures() == 0,
		    "posed-model.wireframe-draws-edges-not-the-interior",
		    "covered pixels: filled " + std::to_string( filledCount ) + ", wireframe " +
		        std::to_string( wireCount ) + ", eyeball " + std::to_string( eyeballCount ) + " " +
		        decalPass.Stats().lastFailure );
	}
	// $ignorez on the mesh points (VertexLitGeneric, Refract) is claimed: the
	// variant's ignoreDepth, as UnlitGeneric's.
	{
		const auto lit = material::MapVariables(
		    "VertexLitGeneric", { { "$basetexture", "a/b" }, { "$ignorez", "1" } }, {} );
		const auto refract = material::MapVariables( "Refract",
		    { { "$normalmap", "a/n" }, { "$ignorez", "1" }, { "$translucent", "1" } }, {} );
		results.That( lit && material::ClaimForMesh( lit.Value(), false ).HasValue() && refract &&
		                  material::ClaimForMesh( refract.Value(), false, true ).HasValue(),
		    "posed-model.mesh-points-claim-ignorez" );
	}
	// UnlitGeneric's $color2 multiplies $color (ApplyColor2Factor); a negative
	// product is refused by name.
	{
		const auto tinted = material::MapVariables(
		    "UnlitGeneric", { { "$basetexture", "a/b" }, { "$color2", "[.5 .5 .5]" } }, {} );
		const auto negative = material::MapVariables(
		    "UnlitGeneric", { { "$basetexture", "a/b" }, { "$color2", "[-1 1 1]" } }, {} );
		results.That( tinted && material::ClaimForDrawing( tinted.Value() ).HasValue(),
		    "posed-model.unlit-color2-is-claimed" );
		const auto refused = negative ? material::ClaimForDrawing( negative.Value() )
		                              : foundation::MakeUnexpected( std::string() );
		results.That( !refused && refused.Error().find( "$color2" ) != std::string::npos,
		    "posed-model.unlit-negative-color2-is-refused-by-name" );
	}
	// $linearwrite after $gammacolorread passes the bytes through (claimed);
	// with color modulation in between, or without the gamma read, refused.
	{
		auto claim = []( const std::vector<material::VmtPair> &extra )
		{
			std::vector<material::VmtPair> pairs = { { "$basetexture", "a/b" },
			    { "$linearwrite", "1" } };
			pairs.insert( pairs.end(), extra.begin(), extra.end() );
			const auto mapped = material::MapVariables( "UnlitGeneric", pairs, {} );
			return mapped ? material::ClaimForDrawing( mapped.Value() )
			              : foundation::MakeUnexpected( std::string( "unmapped" ) );
		};
		results.That( claim( { { "$gammacolorread", "1" }, { "$vertexalpha", "1" } } ).HasValue(),
		    "posed-model.unlit-linearwrite-after-gamma-read-is-claimed" );
		const auto modulated = claim( { { "$gammacolorread", "1" }, { "$vertexcolor", "1" } } );
		const auto decoded = claim( {} );
		results.That( !modulated && modulated.Error().find( "$linearwrite" ) != std::string::npos &&
		                  !decoded,
		    "posed-model.unlit-linearwrite-refuses-modulation-and-decoded-reads" );
	}
	const auto unsupportedDecal = material::MapVariables(
	    "DecalModulate", { { "$basetexture", "decal" }, { "$envmap", "cube" } }, {} );
	results.That( unsupportedDecal && !material::ClaimForDrawing( unsupportedDecal.Value() ),
	    "posed-model.decal-refuses-unhandled-reflection" );
	WorldMaterial litOverlay;
	litOverlay.name = "captured-lightmap-decal";
	litOverlay.shader = "LightmappedGeneric";
	litOverlay.variables = {
	    { "$basetexture", "opaque-white-half" }, { "$translucent", "1" }, { "$decal", "1" } };
	litOverlay.textures = { { "$basetexture", 7 } };
	CanvasImage litDecal;
	if ( auto why = renderDecal( 0.75f, -1.0f, 1.0f, litDecal, &litOverlay ) )
		return why;
	const auto *litPixel = litDecal.At( kSize / 2, kSize / 2 );
	bool capturedLighting = true;
	for ( int c = 0; c < 3; ++c )
		capturedLighting &= std::fabs( litPixel[c] - factor[c] ) < 0.002f;
	results.That( capturedLighting && decalPass.Failures() == 0,
	    "posed-model.decal-preserves-captured-lightmap-page-and-coordinates",
	    decalPass.Stats().lastFailure );
	// Gameplay fades an authored opaque area-portal cover through $alpha.
	// Zero preserves the room and its destination alpha without an authored
	// translucent flag; returning to one must restore the visible cover.
	for ( const char *shader : { "UnlitGeneric", "LightmappedGeneric" } )
	{
		WorldMaterial cover;
		cover.name = "gameplay-faded-area-portal-cover";
		cover.shader = shader;
		cover.variables = { { "$basetexture", "opaque-white-half" }, { "$alpha", "0" } };
		cover.textures = { { "$basetexture", 7 } };
		CanvasImage invisible;
		if ( auto why = renderDecal( 0.75f, -1.0f, 1.0f, invisible, &cover ) )
			return why;
		const auto *pixel = invisible.At( kSize / 2, kSize / 2 );
		bool preserved = std::fabs( pixel[3] - 0.25f ) < 0.001f;
		for ( int c = 0; c < 3; ++c )
			preserved &= std::fabs( pixel[c] - destination[c] ) < 0.002f;
		results.That( preserved && decalPass.Failures() == 0,
		    std::string( "posed-model.gameplay-alpha-zero-preserves-room-" ) + shader,
		    decalPass.Stats().lastFailure );
		cover.variables.back().second = "1";
		CanvasImage opaque;
		if ( auto why = renderDecal( 0.75f, -1.0f, 1.0f, opaque, &cover ) )
			return why;
		results.That( opaque.rgba != invisible.rgba && decalPass.Failures() == 0,
		    std::string( "posed-model.gameplay-alpha-one-restores-cover-" ) + shader );
	}
	// The frontend already selected the frame image. Verify both selectors
	// reach the emissive panel, including a return to the original frame.
	for ( int frame : { 0, 1, 0 } )
	{
		WorldMaterial panel;
		panel.name = "live-door-state-panel";
		panel.shader = "LightmappedGeneric";
		panel.variables = { { "$basetexture", "selected-frame" }, { "$selfillum", "1" },
		    { "$frame", std::to_string( frame ) } };
		panel.textures = { { "$basetexture", frame ? 5 : 7 } };
		CanvasImage image;
		if ( auto why = renderDecal( 0.75f, -1.0f, 1.0f, image, &panel ) )
			return why;
		const auto *pixel = image.At( kSize / 2, kSize / 2 );
		const float encoded[3] = { 128.0f / 255.0f, 192.0f / 255.0f, 1.0f };
		bool matches = true;
		for ( int c = 0; c < 3; ++c )
		{
			const float base = encoded[c] <= 0.04045f
			                       ? encoded[c] / 12.92f
			                       : std::pow( ( encoded[c] + 0.055f ) / 1.055f, 2.4f );
			const float mask = 64.0f / 255.0f;
			const float expected = frame ? base * ( factor[c] * ( 1.0f - mask ) + mask ) : 1.0f;
			matches &= std::fabs( pixel[c] - expected ) < 0.003f;
		}
		results.That( decalPass.Failures() == 0 && matches,
		    "posed-model.lightmapped-selected-door-state-frame-" + std::to_string( frame ),
		    decalPass.Stats().lastFailure + " red " + std::to_string( pixel[0] ) );
	}
	decalPass.ReleaseDevice( *device );

	WorldMaterial cableMaterial;
	cableMaterial.shader = "Cable_DX9";
	cableMaterial.name = "expanded-rope-fixture";
	cableMaterial.variables = {
	    { "$basetexture", "rope-color" }, { "$bumpmap", "rope-normal" }, { "$nofog", "1" } };
	cableMaterial.textures = { { "$basetexture", 6 }, { "$bumpmap", 1 } };
	WorldPass cable;
	cable.SetWorld( MeshWorld() );
	CanvasImage ribbon;
	if ( auto why = render( cable, 0.0f, false, 24, black, ribbon, false, RenderCoreDrawPhase::kAll,
	         true, nullptr, &cableMaterial ) )
		return why;
	const float *ropePixel = ribbon.At( 32, 32 );
	// Independent sRGB decode; normal-map half-Lambert is blue squared.
	auto linear = []( float byte )
	{
		const float value = byte / 255.0f;
		return value <= 0.04045f ? value / 12.92f : std::pow( ( value + 0.055f ) / 1.055f, 2.4f );
	};
	const float normalTerm = std::pow( 170.0f / 255.0f, 2.0f );
	results.That(
	    cable.Stats().dynamicDrawsDrawn == 1 && cable.Stats().viewsFailed == 0 &&
	        std::abs( ropePixel[0] - normalTerm * 128.0f / 255.0f ) < 0.002f &&
	        std::abs( ropePixel[1] - normalTerm * linear( 128 ) * 64.0f / 255.0f ) < 0.002f &&
	        std::abs( ropePixel[2] - normalTerm * linear( 64 ) ) < 0.002f &&
	        std::abs( ropePixel[3] - std::pow( 128.0f / 255.0f, 2.0f ) ) < 0.002f,
	    "posed-model.cable-uses-both-uv-sets-and-linear-vertex-light", cable.Stats().lastFailure );
	const auto missingRopeNormal =
	    material::MapVariables( "SplineRope", { { "$basetexture", "rope-color" } }, {} );
	results.That(
	    missingRopeNormal && !material::ClaimForDrawing( missingRopeNormal.Value(), true ),
	    "posed-model.cable-refuses-missing-required-normal-texture" );
	cable.ReleaseDevice( *device );
	WorldData twoTextureWorld = MeshWorld();
	twoTextureWorld.materials[0].shader = "UnlitTwoTexture_DX9";
	twoTextureWorld.materials[0].variables = {
	    { "$basetexture", "two-texture-base" }, { "$texture2", "two-texture-overlay" } };
	twoTextureWorld.materials[0].textures = { { "$basetexture", 5 }, { "$texture2", 6 } };
	WorldData transformedWorld = twoTextureWorld;
	transformedWorld.materials[0].variables.push_back(
	    { "$texture2transform", "center .5 .5 scale 1 1 rotate 0 translate .75 0" } );
	WorldPass twoTexture, transformed;
	twoTexture.SetWorld( std::move( twoTextureWorld ) );
	transformed.SetWorld( std::move( transformedWorld ) );
	CanvasImage multiplied, shiftedOverlay;
	if ( auto why = render( twoTexture, 0.0f, false, 23, black, multiplied ) )
		return why;
	if ( auto why = render( transformed, 0.0f, false, 24, black, shiftedOverlay ) )
		return why;
	// IEC sRGB decode of the two authored byte images, multiplied in linear
	// light. Their alpha values must not change UnlitTwoTexture's alpha of 1.
	results.That( twoTexture.Stats().viewsFailed == 0 &&
	                  std::abs( multiplied.At( 32, 32 )[0] - 0.21586f ) < 0.003f &&
	                  std::abs( multiplied.At( 32, 32 )[1] - 0.11378f ) < 0.003f &&
	                  std::abs( multiplied.At( 32, 32 )[2] - 0.05127f ) < 0.003f &&
	                  std::abs( multiplied.At( 32, 32 )[3] - 1.0f ) < 0.003f,
	    "posed-model.two-texture-linear-product-and-opaque-alpha",
	    twoTexture.Stats().lastFailure + " claims " +
	        std::to_string( twoTexture.Stats().claimedMaterials ) + " rgb " +
	        std::to_string( multiplied.At( 32, 32 )[0] ) + " " +
	        std::to_string( multiplied.At( 32, 32 )[1] ) + " " +
	        std::to_string( multiplied.At( 32, 32 )[2] ) );
	results.That( transformed.Stats().viewsFailed == 0 &&
	                  shiftedOverlay.At( 32, 32 )[0] < multiplied.At( 32, 32 )[0] * 0.1f &&
	                  shiftedOverlay.At( 32, 32 )[1] > multiplied.At( 32, 32 )[1] * 3.0f,
	    "posed-model.two-texture-independent-transform-negative-control",
	    std::to_string( shiftedOverlay.At( 32, 32 )[0] ) + " " +
	        std::to_string( shiftedOverlay.At( 32, 32 )[1] ) );
	WorldPass dynamic;
	dynamic.SetWorld( MeshWorld() );
	WorldMaterial dynamicMaterial;
	dynamicMaterial.name = "dynamic-two-texture-fixture";
	dynamicMaterial.shader = "UnlitTwoTexture_DX9";
	dynamicMaterial.variables = {
	    { "$basetexture", "two-texture-base" }, { "$texture2", "two-texture-overlay" } };
	dynamicMaterial.textures = { { "$basetexture", 5 }, { "$texture2", 6 } };
	CanvasImage dynamicImage, proxyImage, invalidImage;
	if ( auto why = render( dynamic, 0.0f, false, 25, black, dynamicImage, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, &dynamicMaterial ) )
		return why;
	dynamicMaterial.variables.push_back( { "$texture2transform",
	    RenderMaterialVmt::MatrixValue(
	        std::array<float, 16>{ 1, 0, 0, .75f, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }.data() ) } );
	if ( auto why = render( dynamic, 0.0f, false, 26, black, proxyImage, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, &dynamicMaterial ) )
		return why;
	results.That(
	    dynamic.Stats().dynamicDrawsDrawn == 2 && dynamic.Failures() == 0 &&
	        std::abs( dynamicImage.At( 32, 32 )[0] - multiplied.At( 32, 32 )[0] ) < .003f &&
	        std::abs( proxyImage.At( 32, 32 )[0] - shiftedOverlay.At( 32, 32 )[0] ) < .003f,
	    "posed-model.dynamic-material-proxy-uses-current-snapshot", dynamic.Stats().lastFailure );
	if ( auto why = render( dynamic, 0.0f, false, 27, black, invalidImage, false,
	         RenderCoreDrawPhase::kAll, true, nullptr, &dynamicMaterial, true ) )
		return why;
	results.That( dynamic.Failures() == 1 && dynamic.Stats().dynamicDrawsDrawn == 2 &&
	                  invalidImage.At( 32, 32 )[0] == 0,
	    "posed-model.dynamic-invalid-index-fails-before-draw", dynamic.Stats().lastFailure );
	dynamic.ReleaseDevice( *device );
	// The frontend binds the selected native frame before handing off an
	// animated indicator. Keep the current handle across material snapshots.
	WorldPass indicator;
	indicator.SetWorld( MeshWorld() );
	WorldMaterial indicatorMaterial;
	indicatorMaterial.name = "animated-indicator";
	indicatorMaterial.shader = "UnlitGeneric";
	indicatorMaterial.variables = { { "$basetexture", "indicator" }, { "$decal", "1" } };
	for ( int frame : { 0, 1, 0 } )
	{
		indicatorMaterial.variables.resize( 2 );
		indicatorMaterial.variables.push_back( { "$frame", std::to_string( frame ) } );
		indicatorMaterial.textures = { { "$basetexture", frame == 0 ? 5 : 3 } };
		CanvasImage selected;
		if ( auto why = render( indicator, 0, false, 28, black, selected, false,
		         RenderCoreDrawPhase::kAll, true, nullptr, &indicatorMaterial ) )
			return why;
		const auto pixel = selected.At( 32, 32 );
		results.That(
		    indicator.Failures() == 0 &&
		        std::abs( pixel[0] - ( frame == 0 ? 0.21586f : 128.0f / 255.0f ) ) < 0.003f &&
		        std::abs( pixel[1] - ( frame == 0 ? 0.52712f : 128.0f / 255.0f ) ) < 0.003f,
		    "posed-model.indicator-selected-frame-" + std::to_string( frame ),
		    indicator.Stats().lastFailure + " red " + std::to_string( pixel[0] ) + " green " +
		        std::to_string( pixel[1] ) );
	}
	indicator.ReleaseDevice( *device );
	WorldPass refused;
	refused.SetWorld( MeshWorld() );
	WorldView unsupported;
	WorldView::DynamicDraw unsupportedDraw;
	unsupportedDraw.material.name = "post-material-negative-control";
	unsupportedDraw.material.shader = "MotionBlur_dx9";
	unsupported.dynamicDraws.push_back( std::move( unsupportedDraw ) );
	results.That( refused.QueueView( std::move( unsupported ) ) == 0 && refused.Failures() == 0 &&
	                  refused.Stats().viewsQueued == 0 &&
	                  refused.Stats().dynamicDrawsRefused == 1 &&
	                  refused.Stats().lastRefusal.find( "post-material-negative-control" ) !=
	                      std::string::npos,
	    "posed-model.unsupported-dynamic-material-is-refused-before-claiming-a-slot",
	    refused.Stats().lastRefusal );
	WorldData forcedWorld = MeshWorld();
	forcedWorld.materials[0].variables = { { "$phong", "0" }, { "$forcephong", "1" } };
	WorldData phongWorld = MeshWorld();
	phongWorld.materials[0].variables = { { "$phong", "1" } };
	WorldPass forced, phong;
	forced.SetWorld( std::move( forcedWorld ) );
	phong.SetWorld( std::move( phongWorld ) );
	CanvasImage forcedImage, phongImage;
	if ( auto why = render( forced, 0.0f, true, 28, black, forcedImage ) )
		return why;
	if ( auto why = render( phong, 0.0f, true, 29, black, phongImage ) )
		return why;
	results.That( forced.Failures() == 0 && forced.Stats().posedDrawsDrawn == 1 &&
	                  forcedImage.rgba == phongImage.rgba,
	    "posed-model.forcephong-selects-the-authored-phong-point" );
	results.That( forcedImage.rgba != center.rgba,
	    "posed-model.forcephong-response-differs-from-the-neutral-mesh-point" );
	forced.ReleaseDevice( *device );
	phong.ReleaseDevice( *device );

	twoTexture.ReleaseDevice( *device );
	transformed.ReleaseDevice( *device );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	glass.ReleaseDevice( *device );
	mixed.ReleaseDevice( *device );
	floor.ReleaseDevice( *device );
	bump.ReleaseDevice( *device );
	ssbumpPass.ReleaseDevice( *device );
	plainSpec.ReleaseDevice( *device );
	warpedSpec.ReleaseDevice( *device );
	unlit.ReleaseDevice( *device );
	color2Pass.ReleaseDevice( *device );
	messages = counter.load();
	return std::nullopt;
}

} // namespace

int RunPosedModelSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "posed-model", std::span<const Seeded>(), RunChecks );
}

} // namespace render::lab
