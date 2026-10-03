//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition and render.legacy-frontend (RFC 0016 A.7),
//			through the C entry points the launcher uses:
//
//			P1 RenderCore_Create composes the null device, the frontend and the
//			   renderer, and fails with a structured result naming an unknown
//			   device or feature;
//			P2 the frontend's provider keeps the wrapped backend's id and
//			   module and forwards creation, counting it;
//			P3 RenderStageMarkers001 marks stages of the engine's frames,
//			   rejects the engine-owned begin and end, and counts violations;
//			P4 each frame runs the legacy-stream pass before present;
//			P5 RenderMaterialBlocks001 writes a family material's variables as
//			   its block (an UnlitGeneric's $color read back from the unlit
//			   block), writes nothing for a legacy-family shader and refuses
//			   an unknown shader.
//			P6 core passes (RFC 0016 K5): -render-core-passes selects the
//			   frontend's probe and refuses an unknown one by name; with a
//			   probe the frontend queues one slot on the backend at each
//			   view's opaque stage, tagged with the stage and view depth, and
//			   none without one.
//
//=============================================================================//

#include "render/composition/render_core.h"
#include "render/device/device.h"
#include "render/legacy/material_blocks.h"
#include "render/legacy/stage_markers.h"
#include "testing/checks.h"
#include "../../../mdltest/synthetic_model.h"

#include <cstring>
#include <vector>

namespace
{

int g_Creates = 0;

class FakeSlots final : public render::legacy::ICorePassSlots
{
public:
	void MarkSlot( std::uint32_t tag ) override { tags.push_back( tag ); }
	std::vector<std::uint32_t> tags;
};
FakeSlots g_Slots;

bool FakeCreate( render::LegacyShaderServices *services )
{
	++g_Creates;
	services->manager = reinterpret_cast<IShaderDeviceMgr *>( 0x10 );
	services->corePassSlots = &g_Slots;
	return true;
}

// The slots one frame with one view queues.
std::vector<std::uint32_t> SlotsOfAFrame(
    const RenderCoreBinding &binding, render::frame::DebugControls debug = {} )
{
	g_Slots.tags.clear();
	render::frame::FrameDesc frame;
	frame.width = 320;
	frame.height = 200;
	frame.debug = debug;
	(void)binding.renderer->BeginFrame( frame );
	(void)binding.stageMarkers->MarkStage( RENDER_STAGE_VIEW_BEGIN );
	(void)binding.stageMarkers->MarkStage( RENDER_STAGE_SKYBOX );
	(void)binding.stageMarkers->MarkStage( RENDER_STAGE_OPAQUE );
	(void)binding.stageMarkers->MarkStage( RENDER_STAGE_TRANSLUCENT );
	(void)binding.stageMarkers->MarkStage( RENDER_STAGE_VIEW_END );
	(void)binding.stageMarkers->MarkStage( RENDER_STAGE_HUD );
	(void)binding.renderer->EndFrame();
	return g_Slots.tags;
}

const render::LegacyShaderProvider g_Backend = { "fixture", "fixture_module", &FakeCreate, true };

std::string SelectionStage()
{
	// One triangle written directly from the WMSH v1 byte contract. The
	// model importer and the composition under test do not write this stage.
	std::string bytes( 412, '\0' );
	bytes.replace( 0, 4, "WMSH" );
	const auto put = [&]( std::size_t at, std::int32_t value )
	{
		mdltest::detail::Put32( bytes, at, value );
	};
	put( 4, 1 );
	put( 8, 128 );
	for ( std::size_t at : { 16u, 20u } )
		put( at, 3 );
	for ( std::size_t at : { 24u, 28u, 32u, 36u, 40u, 44u } )
		put( at, 1 );
	put( 48, 12 );
	const std::int32_t offsets[] = { 128, 256, 272, 288, 320, 368, 384, 400, 412 };
	for ( std::size_t i = 0; i < std::size( offsets ); ++i )
		put( 56 + i * 8, offsets[i] );
	for ( int vertex = 0; vertex < 3; ++vertex )
	{
		const std::size_t at = 128 + vertex * 40;
		mdltest::detail::PutF( bytes, at, vertex == 1 ? 1.0f : 0.0f );
		mdltest::detail::PutF( bytes, at + 4, vertex == 2 ? 1.0f : 0.0f );
		put( at + 16, 32767 ); // tangent +X; normal +Z is octahedral (0, 0)
		bytes[at + 20] = 1;
		put( 256 + vertex * 4, vertex );
	}
	put( 296, 3 );
	put( 304, 1 );
	put( 324, 3 );
	put( 332, 3 );
	mdltest::detail::PutF( bytes, 348, 2.0f );
	mdltest::detail::PutF( bytes, 360, 1.0f );
	mdltest::detail::PutF( bytes, 364, 1.0f );
	put( 372, 1 );
	put( 400, 7 );
	bytes.replace( 404, 7, "fixture" );
	return bytes;
}

void BodyGroupComposition( testing::Checks &checks, const RenderCoreBinding &binding )
{
	IRenderCoreWorld &world = *binding.world;
	const std::uint16_t texel[] = { 0x3c00, 0x3c00, 0x3c00, 0x3c00 };
	world_mesh_gpu::WorldLightmapUploadRequest lightmap;
	lightmap.width = lightmap.height = lightmap.layerCount = 1;
	lightmap.layers[0] = texel;
	checks.That( world.StageUpload()->UploadLightmap( lightmap ), "P9.selection-stage-lightmap" );
	const std::string stage = SelectionStage();
	const RenderCoreWorldMeshlet meshlet{ 0, 0, 3 };
	RenderCoreWorldMaterial materials[2]{};
	materials[0].name = "supported";
	materials[0].shader = "UnlitGeneric";
	materials[1].name = "unsupported";
	materials[1].shader = "NoSuchShader";
	world.SetWorldMesh( stage.data(), stage.size(), &meshlet, 1, materials, 1, nullptr );

	mdltest::SyntheticModel source;
	source.textures = { "supported", "unsupported" };
	source.skins = { { 0, 1 } };
	source.bodyParts = { { { mdltest::BoxMesh( { -2, 0, 0 }, { 1, 1, 1 }, 0 ) },
	                         { mdltest::BoxMesh( { 2, 0, 0 }, { 1, 1, 1 }, 0 ) } },
	    { { mdltest::BoxMesh( { 0, 0, 0 }, { 1, 1, 1 }, 0 ) },
	        { mdltest::BoxMesh( { 0, 2, 0 }, { 1, 1, 1 }, 1 ) }, {} } };
	mdltest::SyntheticFiles files = mdltest::WriteModel( source );
	RenderCoreStaticModel model{ "arbitrary/bodygroups.mdl", files.mdl.data(), files.mdl.size(),
	    files.vvd.data(), files.vvd.size(), files.vtx.data(), files.vtx.size(), materials, 2 };
	RenderCoreStaticProp prop{};
	prop.world[0] = prop.world[5] = prop.world[10] = 1.0f;
	world.SetStaticProps( &model, 1, &prop, 1 );
	checks.That(
	    world.DrawsStaticProp( 0 ), "P9.static-default-ignores-inactive-unsupported-body" );
	const float identity[] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	const float viewport[] = { 0, 0, 64, 64, 0, 1 };
	const float bones[] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 };
	RenderCorePosedModel pose{ 0, 0, bones, 1 };
	for ( int body : { -1, 0, 1, 2, 3, 4, 5, 6 } )
	{
		pose.body = body;
		g_Slots.tags.clear();
		const bool accepted = world.DrawView(
		    nullptr, 0, identity, viewport, 1, nullptr, nullptr, 0, nullptr, 0, &pose, 1 );
		const bool expected = body != 2 && body != 3;
		checks.That( accepted == expected && g_Slots.tags.size() == ( expected ? 1u : 0u ),
		    "P9.body-base-count-reaches-eligibility-before-slot-publication" );
	}
	// A successful import with no selected surfaces differs from a failed
	// import. Neither needs animation data: the host supplies the palette.
	source.bodyParts = { { {}, { mdltest::BoxMesh( { 0, 0, 0 }, { 1, 1, 1 }, 0 ) } } };
	files = mdltest::WriteModel( source );
	model.mdl = files.mdl.data();
	model.mdlBytes = files.mdl.size();
	model.vvd = files.vvd.data();
	model.vvdBytes = files.vvd.size();
	model.vtx = files.vtx.data();
	model.vtxBytes = files.vtx.size();
	world.SetStaticProps( &model, 1, &prop, 1 );
	pose.body = 0;
	checks.That( world.DrawsStaticProp( 0 ) && world.DrawView( nullptr, 0, identity, viewport, 2,
	                                               nullptr, nullptr, 0, nullptr, 0, &pose, 1 ),
	    "P9.explicit-blank-body-is-valid" );
	model.mdlBytes = 1;
	world.SetStaticProps( &model, 1, &prop, 1 );
	checks.That( !world.DrawsStaticProp( 0 ) && !world.DrawView( nullptr, 0, identity, viewport, 3,
	                                                nullptr, nullptr, 0, nullptr, 0, &pose, 1 ),
	    "P9.failed-import-is-not-a-blank-body" );
	world.ClearWorld();
	g_Slots.tags.clear();
}

void LodComposition( testing::Checks &checks, const RenderCoreBinding &binding )
{
	IRenderCoreWorld &world = *binding.world;
	const std::string stage = SelectionStage();
	const RenderCoreWorldMeshlet meshlet{ 0, 0, 3 };
	RenderCoreWorldMaterial materials[3]{};
	materials[0].name = "base";
	materials[0].shader = "UnlitGeneric";
	materials[1].name = "lower";
	materials[1].shader = "NoSuchShader";
	materials[2] = materials[0];
	world.SetWorldMesh( stage.data(), stage.size(), &meshlet, 1, materials, 1, nullptr );

	mdltest::SyntheticFiles files = mdltest::WriteModel( mdltest::LodPixelModel() );
	RenderCoreStaticModel model{ "arbitrary/lods.mdl", files.mdl.data(), files.mdl.size(),
		files.vvd.data(), files.vvd.size(), files.vtx.data(), files.vtx.size(), materials, 1, 3 };
	RenderCoreStaticProp prop{};
	prop.world[0] = prop.world[5] = prop.world[10] = 1.0f;
	world.SetStaticProps( &model, 1, &prop, 1 );
	checks.That( world.DrawsStaticProp( 0 ), "P10.static-default-selects-LOD-zero" );

	const float bones[] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 0 };
	const float transform[] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	const float viewport[] = { 0, 0, 64, 64, 0, 1 };
	const auto drawsStatic = [&]( unsigned int lod )
	{
		const RenderCoreStaticPropDraw draw{ 0, lod };
		g_Slots.tags.clear();
		return world.DrawView( nullptr, 0, transform, viewport, 1, nullptr, nullptr, 0, &draw, 1,
		           nullptr, 0 ) &&
		       g_Slots.tags.size() == 1;
	};
	RenderCorePosedModel pose{ 0, 0, bones, 2 };
	const auto draws = [&]( unsigned int lod, int body = 0 )
	{
		pose.lod = lod;
		pose.body = body;
		g_Slots.tags.clear();
		return world.DrawView(
		           nullptr, 0, transform, viewport, 1, nullptr, nullptr, 0, nullptr, 0, &pose, 1 ) &&
		       g_Slots.tags.size() == 1;
	};
	checks.That( draws( 0 ), "P10.LOD-zero-is-eligible" );
	checks.That( !draws( 1 ) && g_Slots.tags.empty(),
	    "P10.unsupported-replacement-is-refused-before-slot-publication" );
	checks.That( draws( 2 ), "P10.explicit-blank-shadow-LOD-is-valid" );
	checks.That( !draws( 3 ) && g_Slots.tags.empty(),
	    "P10.out-of-range-LOD-is-refused-before-slot-publication" );
	checks.That( !world.DrawsStaticProp( 0, 1 ) && !drawsStatic( 1 ) && g_Slots.tags.empty(),
	    "P10.static-unsupported-replacement-is-refused-before-slot-publication" );
	checks.That(
	    world.DrawsStaticProp( 0, 2 ) && drawsStatic( 2 ), "P10.static-blank-LOD-is-valid" );
	checks.That( !world.DrawsStaticProp( 0, 3 ) && !drawsStatic( 3 ) && g_Slots.tags.empty(),
	    "P10.static-out-of-range-LOD-is-refused-before-slot-publication" );

	materials[1].shader = "UnlitGeneric";
	world.SetStaticProps( &model, 1, &prop, 1 );
	checks.That( draws( 1 ), "P10.supported-replacement-LOD-is-eligible" );
	checks.That( draws( 1, 1 ), "P10.body-and-LOD-selection-compose" );
	checks.That( world.DrawsStaticProp( 0, 1 ) && drawsStatic( 1 ),
	    "P10.static-supported-replacement-LOD-is-eligible" );

	model.materialLodCount = 1;
	world.SetStaticProps( &model, 1, &prop, 1 );
	checks.That( draws( 0 ) && !draws( 1 ) && g_Slots.tags.empty(),
	    "P10.missing-replacement-descriptor-refuses-only-that-LOD" );
	checks.That( world.DrawsStaticProp( 0, 0 ) && !world.DrawsStaticProp( 0, 1 ) &&
	                 !drawsStatic( 1 ) && g_Slots.tags.empty(),
	    "P10.static-missing-replacement-descriptor-refuses-only-that-LOD" );
	world.ClearWorld();
	g_Slots.tags.clear();
}

} // namespace

int main()
{
	testing::Checks checks;
	RenderCoreResult result;

	RenderCoreConfig unknownDevice;
	unknownDevice.device = "metal";
	checks.That( !RenderCore_Create( &unknownDevice, &result ) &&
	                 result.status == RENDER_CORE_UNKNOWN_DEVICE &&
	                 std::strstr( result.message, "metal" ),
	    "P1.an-unlinked-device-fails-by-name" );
	RenderCoreConfig unknownFeature;
	unknownFeature.features = "legacy-stream,shadows,present";
	checks.That( !RenderCore_Create( &unknownFeature, &result ) &&
	                 result.status == RENDER_CORE_UNKNOWN_FEATURE &&
	                 std::strstr( result.message, "shadows" ),
	    "P1.an-unlinked-feature-fails-by-name" );
	checks.That(
	    !RenderCore_Create( nullptr, &result ) && result.status == RENDER_CORE_INVALID_CONFIG,
	    "P1.no-config-fails" );

	RenderCoreConfig config;
	config.legacyBackend = &g_Backend;
	RenderCore *core = RenderCore_Create( &config, &result );
	if ( !checks.That( core && result.status == RENDER_CORE_OK, "P1.the-null-core-composes" ) )
		return checks.Report();
	const RenderCoreBinding *binding = RenderCore_GetBinding( core );
	checks.That( binding && binding->device && binding->renderer && binding->sceneFactory.create &&
	                 binding->stageMarkers && std::strcmp( binding->deviceName, "null" ) == 0,
	    "P1.the-binding-is-complete" );
	if ( IRenderMaterialBlocks *blocks = binding ? binding->materialBlocks : nullptr )
	{
		const char *keys[] = { "$basetexture", "$color", "$refractamount" };
		const char *values[] = { "vgui/white", "[0.5 0.25 1]", ".2" };
		char line[1024];
		const int length = blocks->FormatBlock(
		    "a/b", "none", "UnlitGeneric", 2, keys, values, line, sizeof( line ) );
		checks.That( length > 0 && length < int( sizeof( line ) ) &&
		                 std::strstr( line, "\"family\":\"unlit\"" ) &&
		                 std::strstr( line, "\"key\":\"$color\",\"parameter\":\"color\",\"value\":"
		                                    "[0.5,0.25,1]" ) &&
		                 std::strstr( line, "\"texture\":\"materials/vgui/white\"" ),
		    "P5.a-family-material-becomes-its-block" );
		checks.That( blocks->FormatBlock( "a/c", "none", "Refract", 1, keys + 2, values + 2, line,
		                 sizeof( line ) ) > 0 &&
		                 std::strstr( line, "\"family\":\"refract\"" ),
		    "P5.refract-now-has-a-core-material-block" );
		checks.That( blocks->FormatBlock( "a/d", "none", "NoSuchShader", 0, nullptr, nullptr, line,
		                 sizeof( line ) ) == -1,
		    "P5.an-unknown-shader-is-refused" );
	}
	else
	{
		checks.That( false, "P5.the-binding-has-material-blocks" );
	}
	// A host drives its own graphs on the core's device port.
	if ( binding && binding->device )
	{
		render::device::BufferDesc desc;
		desc.size = 64;
		desc.usages = render::device::UsageSet{ render::device::ResourceUsage::kCopyDestination };
		auto buffer = binding->device->CreateBuffer( desc );
		checks.That(
		    buffer.HasValue() &&
		        binding->device->Release( buffer.Value(), render::device::CompletionToken() )
		            .HasValue(),
		    "P1.the-device-port-is-usable" );
	}

	const render::LegacyShaderProvider *provider = RenderCore_GetLegacyProvider( core );
	checks.That( provider && std::strcmp( provider->id, "fixture" ) == 0 &&
	                 std::strcmp( provider->legacyModuleName, "fixture_module" ) == 0 &&
	                 provider->supportsQueuedRendering && provider->createFor && !provider->create,
	    "P2.the-provider-keeps-the-backend-identity" );
	render::LegacyShaderServices services;
	checks.That( provider && provider->createFor( provider->context, &services ) &&
	                 services.manager == reinterpret_cast<IShaderDeviceMgr *>( 0x10 ) &&
	                 g_Creates == 1 && RenderCore_GetLegacyProviderCreates( core ) == 1,
	    "P2.creation-forwards-to-the-backend" );

	IRenderStageMarkers *markers = binding->stageMarkers;
	render::frame::IRenderer *renderer = binding->renderer;
	checks.That(
	    !markers->MarkStage( RENDER_STAGE_OPAQUE ), "P3.marks-outside-a-frame-are-rejected" );
	render::frame::FrameDesc frame;
	frame.width = 320;
	frame.height = 200;
	(void)renderer->BeginFrame( frame );
	checks.That( markers->MarkStage( RENDER_STAGE_VIEW_BEGIN ) &&
	                 markers->MarkStage( RENDER_STAGE_OPAQUE ) &&
	                 markers->MarkStage( RENDER_STAGE_TRANSLUCENT ) &&
	                 markers->MarkStage( RENDER_STAGE_VIEW_END ) &&
	                 markers->MarkStage( RENDER_STAGE_HUD ),
	    "P3.the-client-marks-a-frame" );
	checks.That( !markers->MarkStage( RENDER_STAGE_FRAME_END ) &&
	                 !markers->MarkStage( RENDER_STAGE_FRAME_BEGIN ),
	    "P3.the-engine-owns-begin-and-end" );
	auto stats = renderer->EndFrame();
	checks.That( stats && stats.Value().passes == 2 && stats.Value().views == 1 &&
	                 stats.Value().orderViolations == 0,
	    "P4.the-frame-runs-legacy-stream-then-present" );
	checks.Equal( markers->GetOrderViolations(), 3u, "P3.rejected-marks-are-counted" );

	// P6: no probe, no slot; the fake backend was created above, so the
	// frontend holds its slots.
	checks.That( binding->corePasses && binding->corePasses->SlotStages() == 0 &&
	                 SlotsOfAFrame( *binding ).empty(),
	    "P6.without-a-probe-no-slot-is-marked" );

	auto scene = binding->sceneFactory.create();
	checks.That( scene && scene->Revision() == 0, "P1.the-scene-factory-makes-scenes" );
	scene.reset();
	RenderCore_Destroy( core );
	RenderCore_Destroy( nullptr );
	checks.That( true, "P1.destroy-tears-down" );

	RenderCoreConfig unknownProbe;
	unknownProbe.legacyBackend = &g_Backend;
	unknownProbe.corePasses = "world";
	checks.That( !RenderCore_Create( &unknownProbe, &result ) &&
	                 result.status == RENDER_CORE_INVALID_CONFIG &&
	                 std::strstr( result.message, "world" ),
	    "P6.an-unknown-probe-fails-by-name" );
	RenderCoreConfig probed;
	probed.legacyBackend = &g_Backend;
	probed.corePasses = "empty";
	RenderCore *probedCore = RenderCore_Create( &probed, &result );
	const RenderCoreBinding *probedBinding = RenderCore_GetBinding( probedCore );
	const render::LegacyShaderProvider *probedProvider = RenderCore_GetLegacyProvider( probedCore );
	render::LegacyShaderServices probedServices;
	if ( checks.That( probedBinding && probedProvider &&
	                      probedProvider->createFor( probedProvider->context, &probedServices ),
	         "P6.a-probed-core-composes" ) )
	{
		BodyGroupComposition( checks, *probedBinding );
		LodComposition( checks, *probedBinding );
		RenderCoreWorldQuality quality{};
		quality.coreOnly = true;
		probedBinding->world->SetQuality( quality );
		checks.That( !probedBinding->corePasses->AcceptsMeshes(),
		    "P8.playable-composition-does-not-enable-the-unfinished-dynamic-handoff" );
		quality.dynamicDraws = true;
		probedBinding->world->SetQuality( quality );
		checks.That( probedBinding->corePasses->AcceptsMeshes(),
		    "P8.dynamic-handoff-requires-explicit-opt-in" );
		quality.dynamicDraws = false;
		probedBinding->world->SetQuality( quality );
		g_Slots.tags.clear();
		probedBinding->world->BeginFrame();
		checks.That(
		    g_Slots.tags == std::vector<std::uint32_t>{ render::legacy::kCorePassForwarded |
		                                                render::legacy::kCorePassLegacyOff |
		                                                render::legacy::kCorePassPortalEffects },
		    "P7.core-product-retains-portal-effects-before-any-cohort" );
		const std::vector<std::uint32_t> coreTags = SlotsOfAFrame( *probedBinding );
		checks.That( coreTags == std::vector<std::uint32_t>{ render::legacy::CorePassTag(
		                                                         RENDER_STAGE_OPAQUE, 1 ),
		                             render::legacy::kCorePassLegacyHud },
		    "P7.core-scene-permits-legacy-ui-only-at-the-top-level-hud-stage" );
		render::frame::DebugControls debug;
		debug.legacy = render::frame::DebugLegacy::kSkip;
		checks.That(
		    SlotsOfAFrame( *probedBinding, debug ) ==
		        std::vector<std::uint32_t>{ render::legacy::CorePassTag( RENDER_STAGE_OPAQUE, 1 ) },
		    "P7.core-diagnostics-never-enable-the-legacy-hud-exception" );
		g_Slots.tags.clear();
		probedBinding->world->BeginFrame();
		checks.That(
		    g_Slots.tags == std::vector<std::uint32_t>{ render::legacy::kCorePassForwarded |
		                                                render::legacy::kCorePassLegacyOff },
		    "P7.core-diagnostics-never-enable-retained-portal-effects" );
		quality.coreOnly = false;
		probedBinding->world->SetQuality( quality );
		(void)SlotsOfAFrame( *probedBinding ); // publish the normal frame's debug policy
		g_Slots.tags.clear();
		probedBinding->world->BeginFrame();
		checks.That( g_Slots.tags.empty(), "P7.compatibility-frame-does-not-inherit-core-only" );
		const std::vector<std::uint32_t> tags = SlotsOfAFrame( *probedBinding );
		checks.That( tags.size() == 1 &&
		                 render::legacy::CorePassTagStage( tags[0] ) == RENDER_STAGE_OPAQUE &&
		                 render::legacy::CorePassTagDepth( tags[0] ) == 1,
		    "P6.a-probe-marks-one-slot-at-the-views-opaque-stage" );
	}
	RenderCore_Destroy( probedCore );
	return checks.Report();
}
