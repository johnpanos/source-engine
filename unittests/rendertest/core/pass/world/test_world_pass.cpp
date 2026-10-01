//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5) on render.device.null, which
//			validates every transition and binding:
//
//			W1 SetWorld claims each material through the one resolver: a
//			   LightmappedGeneric and an UnlitGeneric (the lightmapped term
//			   with lighting one) are drawn; a material with $envmap stays
//			   legacy with the gap named, and a blended one too.
//			W2 A queued view records its surfaces at its slot, with its lightmap
//			   pages (a surface with no page samples the neutral white).
//			W3 The same slot recorded again (a capture re-records the stream)
//			   draws the same view.
//			W4 A slot whose view was queued against an earlier world draws
//			   nothing and is counted; a view naming a surface the pass does not
//			   draw is counted.
//			W5 Stats name the claimed materials.
//			W6 A claimed material the render sequence fails (a texture that
//			   does not import) is a counted, named failure; the pass still
//			   claims it (nothing claimed goes back to legacy).
//			W7 A slot of an earlier world recorded again (a capture across a
//			   level change) fails alone: the next world's queued views stay.
//			W9 A variable the model does not read keeps a material out unless
//			   it holds its shader's neutral value (declared defaults), and a
//			   variable with no neutral value keeps it out.
//			W10 Views of a host frame the backend never recorded are skipped,
//			   not failed; a view whose slot never recorded while another slot
//			   of its frame did is a failure.
//			P1 The resolver's editor preview (ProgramResolver::ResolvePreview)
//			   draws a material strict Resolve refuses, with its blend, and names
//			   every variable it ignores.
//			W8 Objects a frame used outlive it: a level change, or a second
//			   target format, between two slots of one submission leaves the
//			   first slot's objects alive, and they are released at a later
//			   frame's slot.
//			W13 Sparse probe-atlas updates reach a staged world; malformed
//			    rectangles fail the view by name.
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/material/program_resolver.h"
#include "render/pass/world/world_pass.h"
#include "testing/checks.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;
using namespace render::pass::world;

// The backend's textures, stood in by null textures made on import.
class FakeTextures final : public IWorldTextures
{
public:
	explicit FakeTextures( IRenderDevice2 &device ) : m_Device( device ) {}
	TextureId Import( int handle, bool srgb ) override
	{
		++imports;
		if ( handle <= 0 )
			return {};
		TextureDesc desc;
		desc.format = srgb ? Format::kRGBA8Srgb : Format::kRGBA8Unorm;
		desc.width = desc.height = 4;
		desc.usages = { ResourceUsage::kSampled };
		auto texture = m_Device.CreateTexture( desc );
		return texture ? texture.Value() : TextureId();
	}
	SamplerDesc Sampler( int ) override { return {}; }
	int imports = 0;

private:
	IRenderDevice2 &m_Device;
};

WorldData TestWorld()
{
	WorldData world;
	// Four quads, one per surface.
	for ( int s = 0; s < 4; ++s )
	{
		const float x = float( s ) * 2.0f;
		const float corners[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
		const std::uint32_t first = std::uint32_t( world.vertices.size() );
		for ( const auto &c : corners )
		{
			WorldVertex v;
			v.position[0] = x + c[0];
			v.position[1] = c[1];
			v.uv[0] = c[0];
			v.uv[1] = c[1];
			v.lightmapUv[0] = c[0];
			v.lightmapUv[1] = c[1];
			world.vertices.push_back( v );
		}
		WorldSurface surface;
		surface.material = std::uint32_t( s );
		surface.lightmapPage = s == 1 ? 0 : 7; // the unlit surface has no page
		surface.firstIndex = std::uint32_t( world.indices.size() );
		for ( std::uint32_t i : { 0u, 1u, 2u, 0u, 2u, 3u } )
			world.indices.push_back( first + i );
		surface.indexCount = 6;
		world.surfaces.push_back( surface );
	}
	auto material = []( const char *name, const char *shader,
	                    std::vector<std::pair<std::string, std::string>> variables )
	{
		WorldMaterial m;
		m.name = name;
		m.shader = shader;
		m.variables = std::move( variables );
		m.textures = { { "$basetexture", 3 } };
		return m;
	};
	world.materials.push_back(
	    material( "lit", "LightmappedGeneric", { { "$basetexture", "concrete/wall" } } ) );
	world.materials.push_back(
	    material( "unlit", "UnlitGeneric", { { "$basetexture", "lights/white" } } ) );
	world.materials.push_back( material( "shiny", "LightmappedGeneric",
	    { { "$basetexture", "metal/plate" }, { "$envmap", "env_cubemap" } } ) );
	world.materials.push_back( material( "glass", "LightmappedGeneric",
	    { { "$basetexture", "glass/pane" }, { "$translucent", "1" } } ) );
	return world;
}

WorldView View( std::vector<std::uint32_t> surfaces, std::uint64_t hostFrame = 0 )
{
	WorldView view;
	view.surfaces = std::move( surfaces );
	for ( int i = 0; i < 4; ++i )
		view.toClip[i * 5] = 1.0f;
	view.viewport = { 0, 0, 64, 64, 0, 1 };
	view.hostFrame = hostFrame;
	return view;
}

// Records slot `tag` into one submission, as the scene stage records a
// section; true when the device accepted it.
bool RecordSlot(
    IRenderDevice2 &device, WorldPass &pass, std::uint32_t tag, const WorldTarget &target )
{
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return false;
	pass.Record( tag, encoder.Value(), target );
	return device.Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} ).HasValue();
}

} // namespace

int main()
{
	testing::Checks checks;
	auto created = null::Create( {} );
	if ( !checks.That( created.HasValue(), "setup.null-device" ) )
		return checks.Report();
	IRenderDevice2 &device = *created.Value();

	WorldPass pass;
	pass.SetWorld( TestWorld() );
	const WorldStats loaded = pass.Stats();
	checks.That( loaded.materials == 4 && loaded.claimedMaterials == 2 && pass.Draws( 0 ) &&
	                 pass.Draws( 1 ) && !pass.Draws( 2 ) && !pass.Draws( 3 ),
	    "W1.lightmapped-and-unlit-claimed-envmap-and-blended-not" );
	bool envmapGap = false;
	bool blendedGap = false;
	for ( const auto &[reason, count] : loaded.gaps )
	{
		envmapGap = envmapGap || reason.find( "$envmap" ) != std::string::npos;
		blendedGap = blendedGap || reason.find( "blended" ) != std::string::npos;
	}
	checks.That( envmapGap && blendedGap, "W1.gaps-are-named" );
	// "program material": the program that draws it (cl_render_debug_claims).
	checks.That( loaded.claimed.size() == 2 && loaded.claimed[0].first == "lightmapped lit" &&
	                 loaded.claimed[1].first == "unlit unlit",
	    "W5.stats-name-the-claimed-materials-and-their-programs" );

	// The slot's target: color and depth in their home usages.
	TextureDesc colorDesc;
	colorDesc.format = Format::kRGBA8Srgb;
	colorDesc.width = colorDesc.height = 64;
	colorDesc.usages = { ResourceUsage::kColorAttachment };
	TextureDesc depthDesc = colorDesc;
	depthDesc.format = Format::kD32Float;
	depthDesc.usages = { ResourceUsage::kDepthWrite };
	auto color = device.CreateTexture( colorDesc );
	auto depth = device.CreateTexture( depthDesc );
	auto setup = device.BeginEncoder( QueueKind::kGraphics );
	if ( !color || !depth || !setup )
		return checks.Report();
	setup.Value().TransitionTexture(
	    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	setup.Value().TransitionTexture(
	    depth.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
	(void)device.Submit( QueueKind::kGraphics, { &setup.Value(), 1 }, {} );

	FakeTextures textures( device );
	WorldTarget target;
	target.device = &device;
	target.color = color.Value();
	target.colorFormat = Format::kRGBA8Srgb;
	target.depth = depth.Value();
	target.depthFormat = Format::kD32Float;
	target.width = target.height = 64;
	target.textures = &textures;
	target.lightmapScale = 16.0f;
	target.outputScale = 1.3f;

	const std::uint32_t tag = pass.QueueView( View( { 0, 1 } ) );
	checks.That( IsWorldTag( tag ), "W2.a-view-queues-a-world-tag" );
	checks.That( RecordSlot( device, pass, tag, target ), "W2.the-slot-records-and-submits" );
	WorldStats stats = pass.Stats();
	checks.That( stats.viewsDrawn == 1 && stats.surfacesDrawn == 2 && stats.viewsFailed == 0,
	    "W2.both-surfaces-draw" );

	checks.That( RecordSlot( device, pass, tag, target ), "W3.the-same-slot-records-again" );
	stats = pass.Stats();
	checks.That( stats.viewsDrawn == 2 && stats.surfacesDrawn == 4 && stats.viewsFailed == 0,
	    "W3.a-re-recorded-slot-draws-the-same-view" );
	{
		WorldPass interleaved;
		interleaved.SetWorld( TestWorld() );
		const std::uint32_t current = interleaved.QueueView( View( { 0 }, 200 ) );
		const std::uint32_t future = interleaved.QueueView( View( { 0 }, 201 ) );
		WorldView streamed = View( {}, 200 );
		WorldView::DynamicDraw geometry;
		const WorldData fixture = TestWorld();
		geometry.material = fixture.materials[1];
		geometry.vertices = fixture.vertices;
		geometry.indices = { 0, 1, 2 };
		streamed.dynamicDraws.push_back( std::move( geometry ) );
		const std::uint32_t dynamic = interleaved.QueueView( std::move( streamed ) );
		WorldTarget streamTarget = target;
		streamTarget.streamEpoch = 1000;
		checks.That( current && future && dynamic &&
		                 RecordSlot( device, interleaved, current, streamTarget ) &&
		                 RecordSlot( device, interleaved, dynamic, streamTarget ) &&
		                 interleaved.Failures() == 0,
		    "W14.render-thread-dynamic-ticket-does-not-discard-main-thread-future-view" );
		streamTarget.streamEpoch = 1001;
		checks.That( RecordSlot( device, interleaved, future, streamTarget ) &&
		                 interleaved.Failures() == 0 && interleaved.Stats().viewsDrawn == 3,
		    "W14.future-frame-still-records-its-promised-view" );
		interleaved.ReleaseDevice( device );
	}
	{
		WorldPass captured;
		captured.SetWorld( TestWorld() );
		std::vector<std::uint32_t> tags;
		for ( unsigned int i = 0; i < 97; ++i )
			tags.push_back( captured.QueueView( View( { 0 }, 300 ) ) );
		WorldTarget captureTarget = target;
		captureTarget.streamEpoch = 2000;
		bool accepted = true;
		for ( std::uint32_t slot : tags )
			accepted = RecordSlot( device, captured, slot, captureTarget ) && accepted;
		++captureTarget.frame; // readback may use a new GPU submission
		for ( std::uint32_t slot : tags )
			accepted = RecordSlot( device, captured, slot, captureTarget ) && accepted;
		checks.That( accepted && captured.Failures() == 0 && captured.Stats().viewsDrawn == 194,
		    "W15.capture-replays-every-slot-of-a-stream-larger-than-64-views" );
		++captureTarget.streamEpoch;
		(void)RecordSlot( device, captured, tags.front(), captureTarget );
		checks.That( captured.Failures() == 1,
		    "W15.discarded-stream-cannot-be-replayed-under-a-new-stream-identity" );
		captured.ReleaseDevice( device );
	}

	// One object-space mesh is shared by its static instances. The same
	// queued world view accepts only instances whose complete mesh is claimed.
	{
		WorldData props = TestWorld();
		WorldMaterial model;
		model.name = "crate";
		model.shader = "VertexLitGeneric";
		model.mesh = true;
		model.variables = { { "$basetexture", "models/crate" } };
		model.textures = { { "$basetexture", 3 } };
		props.materials.push_back( std::move( model ) );
		WorldMaterial alternate = props.materials.back();
		alternate.name = "crate-alternate-skin";
		props.materials.push_back( std::move( alternate ) );
		WorldData::StaticMesh mesh;
		for ( const auto &xy :
		    { std::pair{ 0.0f, 0.0f }, std::pair{ 1.0f, 0.0f }, std::pair{ 0.0f, 1.0f } } )
		{
			material::SurfaceModelVertex vertex;
			vertex.position[0] = xy.first;
			vertex.position[1] = xy.second;
			vertex.normal[2] = 1.0f;
			vertex.tangent[0] = 1.0f;
			vertex.tangent[3] = 1.0f;
			mesh.vertices.push_back( vertex );
		}
		mesh.indices = { 0, 1, 2 };
		mesh.surfaces.push_back( { 4, 0, 0, 3 } );
		mesh.skinMaterials = { { 4 }, { 5 } };
		props.staticMeshes.push_back( std::move( mesh ) );
		WorldData::StaticInstance instance;
		instance.world[0] = instance.world[5] = instance.world[10] = instance.world[15] = 1.0f;
		instance.world[3] = 2.0f;
		props.staticInstances.push_back( instance );
		instance.world[3] = 4.0f;
		instance.skin = 1;
		props.staticInstances.push_back( instance );
		instance.skin = 2;
		props.staticInstances.push_back( instance );
		WorldPass modelPass;
		modelPass.SetWorld( std::move( props ) );
		checks.That( modelPass.DrawsStaticInstance( 0 ) && modelPass.DrawsStaticInstance( 1 ) &&
		                 !modelPass.DrawsStaticInstance( 2 ) && !modelPass.DrawsStaticInstance( 3 ),
		    "W11.claims-shared-geometry-with-valid-skins" );
		WorldView modelView = View( {} );
		modelView.staticInstances = { 0, 1 };
		const std::uint32_t modelTag = modelPass.QueueView( std::move( modelView ) );
		checks.That( modelTag != 0 && modelPass.Stats().staticInstancesQueued == 2,
		    "W11.static-only-view-has-its-own-slot" );
		checks.That( RecordSlot( device, modelPass, modelTag, target ) &&
		                 modelPass.Stats().viewsFailed == 0 &&
		                 modelPass.Stats().staticInstancesQueued == 2 &&
		                 modelPass.Stats().staticDrawsDrawn == 2,
		    "W11.shared-static-mesh-records-through-the-model-program" );
		checks.That( modelPass.DrawsPosedModel( 0, 0 ) && modelPass.DrawsPosedModel( 0, 1 ) &&
		                 !modelPass.DrawsPosedModel( 0, 2 ),
		    "W12.posed-model-claims-only-complete-skins" );
		WorldView poseView = View( {} );
		WorldView::PosedModel pose;
		pose.mesh = 0;
		pose.skin = 1;
		pose.vertices.resize( 3 );
		pose.vertices[0].position[0] = 0.25f;
		pose.vertices[1].position[0] = 1.25f;
		pose.vertices[2].position[1] = 1.0f;
		for ( auto &vertex : pose.vertices )
		{
			vertex.normal[2] = 1.0f;
			vertex.tangent[0] = vertex.tangent[3] = 1.0f;
		}
		poseView.posedModels.push_back( std::move( pose ) );
		const std::uint32_t poseTag = modelPass.QueueView( std::move( poseView ) );
		checks.That( poseTag != 0 && RecordSlot( device, modelPass, poseTag, target ) &&
		                 modelPass.Stats().viewsFailed == 0 &&
		                 modelPass.Stats().posedModelsQueued == 1 &&
		                 modelPass.Stats().posedDrawsDrawn == 1,
		    "W12.posed-model-uses-the-shared-material-and-current-pose" );
		modelPass.ReleaseDevice( device );
	}

	// A frame that straddles a level change (queued mode records its slots
	// after the change): its view was the earlier world's, which is gone. It
	// draws nothing and is skipped, not failed.
	const std::uint32_t stale = pass.QueueView( View( { 0 } ) );
	const std::uint64_t skippedBefore = pass.Stats().viewsSkipped;
	pass.SetWorld( TestWorld() );
	(void)RecordSlot( device, pass, stale, target );
	stats = pass.Stats();
	checks.That(
	    stats.viewsDrawn == 2 && stats.viewsFailed == 0 && stats.viewsSkipped == skippedBefore + 1,
	    "W4.a-view-of-an-earlier-world-draws-nothing-and-is-skipped" );

	const std::uint32_t mixed = pass.QueueView( View( { 0, 2 } ) );
	(void)RecordSlot( device, pass, mixed, target );
	stats = pass.Stats();
	checks.That( stats.viewsDrawn == 3 && stats.surfacesDrawn == 5 && stats.viewsFailed == 1,
	    "W4.a-surface-the-pass-does-not-draw-is-counted" );

	// W9: unread variables against their neutral values.
	{
		WorldData world = TestWorld();
		world.materials[0].variables.push_back( { "$outline", "0" } );
		world.materials[0].defaults = { { "$outline", "0" } };
		world.materials[1].variables.push_back( { "$outline", "1" } );
		world.materials[1].defaults = { { "$outline", "0" } };
		world.materials[2] = world.materials[0];
		world.materials[2].variables.push_back( { "$mystery", "2" } );
		WorldPass unread;
		unread.SetWorld( std::move( world ) );
		bool named = false;
		bool unknown = false;
		for ( const auto &[reason, count] : unread.Stats().gaps )
		{
			named = named || reason.find( "does not read $outline 1" ) != std::string::npos;
			unknown = unknown || reason == "unlit: the model does not read $mystery" ||
			          reason.find( "does not read $mystery" ) != std::string::npos;
		}
		checks.That( unread.Draws( 0 ), "W9.an-unread-variable-at-neutral-is-claimed" );
		checks.That( !unread.Draws( 1 ) && named, "W9.an-unread-variable-set-is-a-named-gap" );
		checks.That(
		    !unread.Draws( 2 ) && unknown, "W9.an-unread-variable-without-neutral-is-a-gap" );
	}

	// P1: the editor preview against strict resolution.
	{
		auto resolver =
		    material::ProgramResolver::Create( device, Format::kRGBA8Srgb, Format::kD32Float, 1 );
		auto mapped = material::MapVariables( "LightmappedGeneric",
		    { { "$basetexture", "metal/plate" }, { "$envmap", "env_cubemap" }, { "$additive", "1" },
		        { "$surfaceprop", "metal" } },
		    {} );
		if ( checks.That( resolver.HasValue() && mapped.HasValue(), "P1.setup" ) )
		{
			material::ProgramResolver &r = *resolver.Value();
			checks.That( !r.Resolve( mapped.Value() ).HasValue(), "P1.strict-resolve-refuses" );
			auto preview = r.ResolvePreview( mapped.Value() );
			checks.That( preview.HasValue() && preview.Value().program.request.pipeline.IsValid() &&
			                 preview.Value().program.blend == BlendMode::kAdditive,
			    "P1.the-preview-draws-with-its-blend" );
			checks.That( preview.HasValue() && preview.Value().ignored.size() == 1 &&
			                 preview.Value().ignored[0] == "$envmap",
			    "P1.the-preview-names-what-it-ignores" );
		}
	}

	// W6: the lit material's texture does not import.
	{
		WorldData broken = TestWorld();
		broken.materials[0].textures = { { "$basetexture", -5 } };
		pass.SetWorld( std::move( broken ) );
		const std::uint64_t before = pass.Failures();
		const std::uint32_t failing = pass.QueueView( View( { 0, 1 } ) );
		target.frame = 10;
		(void)RecordSlot( device, pass, failing, target );
		stats = pass.Stats();
		checks.That( pass.Failures() == before + 1 && stats.viewsFailed == before + 1 &&
		                 stats.lastFailure.find( "lit" ) != std::string::npos &&
		                 stats.lastFailure.find( "did not import" ) != std::string::npos,
		    "W6.a-claimed-material-that-fails-is-a-named-failure" );
		checks.That( pass.Draws( 0 ), "W6.the-failed-material-stays-claimed" );
	}

	// W7: a capture re-records an earlier world's slot after the change.
	{
		pass.SetWorld( TestWorld() );
		const std::uint32_t old = pass.QueueView( View( { 0 } ) );
		target.frame = 11;
		(void)RecordSlot( device, pass, old, target );
		pass.SetWorld( TestWorld() );
		const std::uint32_t next = pass.QueueView( View( { 0, 1 } ) );
		const std::uint64_t before = pass.Failures();
		const std::uint64_t drawn = pass.Stats().viewsDrawn;
		const std::uint64_t skipped = pass.Stats().viewsSkipped;
		target.frame = 12;
		(void)RecordSlot( device, pass, old, target );
		checks.That( pass.Failures() == before && pass.Stats().viewsSkipped == skipped + 1 &&
		                 pass.Stats().viewsDrawn == drawn,
		    "W7.the-earlier-worlds-slot-is-skipped" );
		(void)RecordSlot( device, pass, next, target );
		checks.That( pass.Failures() == before && pass.Stats().viewsDrawn == drawn + 1,
		    "W7.the-next-worlds-view-is-still-queued-and-draws" );
	}

	// W8: two slots of one submission, a level change and a new format
	// between them.
	{
		TextureDesc unormDesc = colorDesc;
		unormDesc.format = Format::kRGBA8Unorm;
		auto unorm = device.CreateTexture( unormDesc );
		auto prepare = device.BeginEncoder( QueueKind::kGraphics );
		if ( unorm && prepare )
		{
			prepare.Value().TransitionTexture(
			    unorm.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
			(void)device.Submit( QueueKind::kGraphics, { &prepare.Value(), 1 }, {} );
		}
		WorldTarget unormTarget = target;
		unormTarget.color = unorm ? unorm.Value() : TextureId();
		unormTarget.colorFormat = Format::kRGBA8Unorm;
		unormTarget.encodeOutput = true;

		const std::uint64_t before = pass.Failures();
		const std::uint32_t first = pass.QueueView( View( { 0, 1 } ) );
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		bool accepted = false;
		if ( encoder )
		{
			target.frame = unormTarget.frame = 13;
			pass.Record( first, encoder.Value(), target );
			pass.SetWorld( TestWorld() );
			const std::uint32_t second = pass.QueueView( View( { 0, 1 } ) );
			pass.Record( second, encoder.Value(), target );
			const std::uint32_t third = pass.QueueView( View( { 0 } ) );
			pass.Record( third, encoder.Value(), unormTarget );
			accepted =
			    device.Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} ).HasValue();
		}
		checks.That( accepted && pass.Failures() == before,
		    "W8.a-submission-keeps-what-its-earlier-slots-used" );
		// A later frame's slot releases the retired world behind its token.
		const std::uint32_t later = pass.QueueView( View( { 0 } ) );
		target.frame = 14;
		checks.That( RecordSlot( device, pass, later, target ) && pass.Failures() == before,
		    "W8.a-later-frame-releases-the-retired-world" );
		if ( unorm )
			(void)device.Release( unorm.Value(), {} );
	}

	// W10: host frame 100 never records; frame 101 records one of its two.
	{
		const std::uint64_t failures = pass.Failures();
		const std::uint64_t skipped = pass.Stats().viewsSkipped;
		(void)pass.QueueView( View( { 0 }, 100 ) );
		(void)pass.QueueView( View( { 1 }, 100 ) );
		const std::uint32_t lost = pass.QueueView( View( { 0 }, 101 ) );
		const std::uint32_t drawn = pass.QueueView( View( { 1 }, 101 ) );
		(void)lost;
		target.frame = 15;
		(void)RecordSlot( device, pass, drawn, target );
		checks.That( pass.Stats().viewsSkipped == skipped + 2,
		    "W10.views-of-an-unrecorded-frame-are-skipped" );
		checks.That(
		    pass.Failures() == failures + 1 &&
		        pass.Stats().lastFailure.find( "frame that recorded" ) != std::string::npos,
		    "W10.a-lost-slot-of-a-recorded-frame-fails" );
	}

	// W13: sparse GI publication updates the probe atlas the mesh program
	// samples. A rectangle outside the volume fails the view by name.
	{
		WorldData world = TestWorld();
		WorldStage stage;
		stage.lightmap.width = stage.lightmap.height = 4;
		stage.lightmap.flat.resize( 4 * 4 * 8 );
		StageProbeVolume probes;
		probes.atlasWidth = probes.atlasHeight = 4;
		probes.atlas.resize( 4 * 4 * 8 );
		probes.tableTexels = 6;
		probes.rows = 1;
		probes.table.resize( 6 * 4 );
		stage.probes = std::move( probes );
		world.stage = std::make_shared<const WorldStage>( std::move( stage ) );
		WorldPass staged;
		staged.SetWorld( std::move( world ) );
		StageProbeVolume update;
		update.tableTexels = 6;
		update.rows = 1;
		update.table.resize( 6 * 4 );
		target.frame = 16;
		const std::uint32_t initial = staged.QueueView( View( { 0 } ) );
		const bool ready = RecordSlot( device, staged, initial, target ) && staged.Failures() == 0;
		staged.SetStageProbeRegions( { { 1, 1, 1, 1 } }, std::vector<std::byte>( 8 ), {}, update );
		target.frame = 17;
		const std::uint32_t changed = staged.QueueView( View( { 0 } ) );
		checks.That(
		    ready && RecordSlot( device, staged, changed, target ) && staged.Failures() == 0,
		    "W13.sparse-probe-atlas-update-records-with-the-world" );
		staged.SetStageProbeRegions( { { 4, 1, 1, 1 } }, std::vector<std::byte>( 8 ), {}, update );
		target.frame = 18;
		const std::uint32_t malformed = staged.QueueView( View( { 0 } ) );
		(void)RecordSlot( device, staged, malformed, target );
		checks.That( staged.Failures() == 1 && staged.Stats().lastFailure.find(
		                                           "probe atlas update" ) != std::string::npos,
		    "W13.out-of-volume-probe-patch-fails-by-name" );
		staged.ReleaseDevice( device );
	}

	pass.ReleaseDevice( device );
	(void)device.WaitIdle();
	return checks.Report();
}
