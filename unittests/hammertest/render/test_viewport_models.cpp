//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.models (RFC 0002 R17 follow-up, props and
//			instances; contract render_adapter.viewport-geometry.v1, G12, G13
//			and V10): the pure geometry rules for model entities and instance
//			content, and the renderer's model staging on render.device.null.
//
//			G12 with GeometryOptions::modelBox an entity drawn as a model has
//			    no marker faces; its box edges go to edges2D in its color when
//			    unselected and to edges in the selection color when selected;
//			    an entity without a model box keeps its marker; instance
//			    content (tint) multiplies every face color by the tint and
//			    draws unselected edges in the edge color;
//			G13 BuildModelBatches: one batch per material, untextured first; a
//			    resolved material with a texture size is textured with the
//			    model's uv, anything else untextured (uv 0) in the fill; colors
//			    are the shading times the tint; the skin family picks the
//			    texture; indices name the batch's vertices. ModelWorld rotates by
//			    Source angles, scales and translates; ModelWorldBox is the box of
//			    the transformed corners; ModelTint is the selection fill, else
//			    the render color, times the instance tint for instance content;
//			G14 posed models: ModelSequence takes the labelled sequence when
//			    the model has it, else the index when in range, else 0 (-1
//			    without sequences); PosedModel is the model at that sequence's
//			    first frame (a Y-up reference turned Z-up by its root, as the
//			    turbine elevator and the cube dropper are); ModelWorldBox of the
//			    posed model bounds the drawn model, not the bind pose;
//			V11 staging is per (sequence, skin, tint): one model file, two
//			    sequences, two variants; the moved entity reuses its variant;
//			V10 the renderer reads each model file once, stages a model once
//			    per (skin, tint) and draws it as one indexed draw per batch and
//			    entity; a missing model draws its marker and is counted; moving
//			    a model entity restages its chunk without reading or staging the
//			    model again; everything is released when the renderer goes.
//
//=============================================================================//

#include "hammer/adapters/render/scene_geometry.h"
#include "hammer/adapters/render/viewport_renderer.h"
#include "render/device/null/provider.h"
#include "testing/checks.h"
#include "unittests/mdltest/synthetic_model.h"
#include "viewport_fixture.h"

#include <cmath>
#include <map>

namespace
{

using namespace hammertest::viewport_render;
using hammer::render_adapter::BuildModelBatches;
using hammer::render_adapter::BuildSceneGeometry;
using hammer::render_adapter::GeometryOptions;
using hammer::render_adapter::IMaterialTextures;
using hammer::render_adapter::IModelSource;
using hammer::render_adapter::MaterialImage;
using hammer::render_adapter::ModelAsset;
using hammer::render_adapter::ModelBatch;
using hammer::render_adapter::SceneGeometry;
using hammer::render_adapter::TextureSize;
using hammer::render_adapter::ViewportRenderer;
using hammer::render_adapter::ViewRequest;
using hammer::viewport::EntityDraw;
using hammer::viewport::ViewKind;
namespace nulldev = render::device::null;

// Two meshes: a box of texture 0 ("crate", found) and a small box of
// texture 1 ("lid", not found); skin 1 swaps the box to texture 2 ("rust").
ModelAsset TwoMeshes()
{
	mdltest::SyntheticModel model;
	model.textures = { "crate", "lid", "rust" };
	model.cdMaterials = { "m/" };
	model.skins = { { 0, 1 }, { 2, 1 } };
	model.bodyParts = { { { mdltest::BoxMesh( { 0, 0, 0 }, { 32, 16, 8 }, 0 ),
	    mdltest::BoxMesh( { 0, 0, 12 }, { 4, 4, 4 }, 1 ) } } };
	const mdltest::SyntheticFiles files = mdltest::WriteModel( model );
	ModelAsset asset;
	asset.model = mdl::ParseModel( { files.mdl, files.vvd, files.vtx, files.ani } ).Value();
	asset.materials = { { "m/crate", true }, { "m/lid", false }, { "m/rust", true } };
	return asset;
}

std::optional<TextureSize> Sized( const std::string &material )
{
	if ( material == "m/crate" || material == "m/rust" )
		return TextureSize{ 64, 64 };
	return std::nullopt;
}

bool Near( const Vec3d &a, const Vec3d &b, double tolerance = 1e-4 )
{
	return std::fabs( a.x - b.x ) <= tolerance && std::fabs( a.y - b.y ) <= tolerance &&
	       std::fabs( a.z - b.z ) <= tolerance;
}

EntityDraw ModelEntity( std::uint64_t id, Vec3d origin, Vec3d angles, bool selected = false )
{
	EntityDraw entity;
	entity.id = ObjectId{ id };
	entity.classname = "prop_static";
	entity.model = "models/test/crate.mdl";
	entity.origin = origin;
	entity.angles = angles;
	entity.mins = origin - Vec3d( 8, 8, 8 );
	entity.maxs = origin + Vec3d( 8, 8, 8 );
	entity.color = { 220, 30, 220 };
	entity.selected = selected;
	return entity;
}

// A tall box authored Y-up, (-4 0 -2)-(4 40 2), with two sequences: "BindPose"
// (the root unrotated) and "stand" (the root's raw Quaternion64 (0.5 0.5 0.5
// 0.5), which maps x y z to z x y: the box stands along z, (-2 -4 0)-(2 4 40)).
ModelAsset Standing()
{
	mdltest::SyntheticModel model = mdltest::BoxModel( { 4, 20, 2 }, "m/", "crate" );
	model.bodyParts[0][0][0] = mdltest::BoxMesh( { 0, 20, 0 }, { 4, 20, 2 } );
	for ( const char *label : { "BindPose", "stand" } )
	{
		mdltest::SyntheticAnimation anim;
		const bool stand = std::string( label ) == "stand";
		anim.data = mdltest::Records( { { 0, 0x20,
		    mdltest::Quaternion64(
		        stand ? mdl::Quaternion{ 0.5f, 0.5f, 0.5f, 0.5f } : mdl::Quaternion{} ) } } );
		model.animations.push_back( anim );
		mdltest::SyntheticSequence seq;
		seq.label = label;
		seq.blends = { static_cast<std::int16_t>( model.animations.size() - 1 ) };
		model.sequences.push_back( seq );
	}
	const mdltest::SyntheticFiles files = mdltest::WriteModel( model );
	ModelAsset asset;
	asset.model = mdl::ParseModel( { files.mdl, files.vvd, files.vtx, files.ani } ).Value();
	asset.materials = { { "m/crate", true } };
	return asset;
}

class FakeModels final : public IModelSource
{
public:
	std::map<std::string, int> reads;
	foundation::Expected<ModelAsset, mdl::ModelError> Model( const std::string &path ) override
	{
		++reads[path];
		if ( path == "models/test/stand.mdl" )
			return Standing();
		if ( path != "models/test/crate.mdl" )
			return foundation::MakeUnexpected(
			    mdl::ModelError{ mdl::ModelStatus::MissingFile, mdl::ModelFile::Mdl, 0 } );
		return TwoMeshes();
	}
};

class FakeTextures final : public IMaterialTextures
{
public:
	foundation::Expected<hammer::render_adapter::SourceMaterial, std::string> Material(
	    const std::string &material ) override
	{
		if ( material != "m/crate" && material != "m/rust" )
			return foundation::MakeUnexpected( material + " is missing" );
		MaterialImage image;
		image.width = image.height = 4;
		image.rgba.assign( 4 * 4 * 4, 200 );
		return hammer::render_adapter::SourceMaterialFromVariables( "VertexLitGeneric",
		    { { "$basetexture", material } }, { { "materials/" + material, std::move( image ) } } );
	}
};

std::size_t IndexedDraws( render::device::IRenderDevice2 &device, std::uint32_t count )
{
	std::size_t draws = 0;
	for ( const nulldev::RecordedCommand &command : nulldev::Control( device )->Recorded() )
		if ( command.op == nulldev::RecordedOp::kDrawIndexed && command.count == count )
			++draws;
	return draws;
}

} // namespace

int main()
{
	testing::Checks checks;

	// --- G12 ---------------------------------------------------------------------------
	{
		hammer::viewport::RenderSnapshot snapshot;
		snapshot.entities = { ModelEntity( 1, Vec3d( 0, 0, 0 ), Vec3d() ),
		    ModelEntity( 2, Vec3d( 100, 0, 0 ), Vec3d(), true ) };
		EntityDraw marker = ModelEntity( 3, Vec3d( 200, 0, 0 ), Vec3d() );
		marker.model.clear();
		snapshot.entities.push_back( marker );
		GeometryOptions options;
		options.modelBox = []( const EntityDraw &entity ) -> std::optional<hammer::scene::Box>
		{
			if ( entity.model.empty() )
				return std::nullopt;
			return hammer::scene::Box{
			    entity.origin - Vec3d( 4, 4, 4 ), entity.origin + Vec3d( 4, 4, 4 ) };
		};
		const SceneGeometry g = BuildSceneGeometry( snapshot, options );
		std::size_t faceVertices = 0;
		for ( const auto &batch : g.faces )
			faceVertices += batch.vertices.size();
		checks.Equal( faceVertices, std::size_t( 36 ), "G12.only-the-marker-entity-has-faces" );
		checks.Equal(
		    g.edges2D.size(), std::size_t( 24 ), "G12.the-unselected-model-box-is-2d-only" );
		checks.Equal(
		    g.edges.size(), std::size_t( 48 ), "G12.selected-model-box-and-marker-in-every-view" );
		bool model2D = !g.edges2D.empty();
		for ( const auto &v : g.edges2D )
			model2D = model2D && std::fabs( v.position[0] ) <= 4.0f &&
			          v.color == render::pass::lines::Rgba8{ 220, 30, 220, 255 }.Packed();
		checks.That( model2D, "G12.the-2d-box-is-the-model-box-in-the-entity-color" );
		bool selectedEdges = false;
		for ( const auto &v : g.edges )
			selectedEdges =
			    selectedEdges ||
			    ( std::fabs( v.position[0] - 100.0f ) <= 4.0f &&
			        v.color == render::pass::lines::Rgba8{ 255, 148, 38, 255 }.Packed() );
		checks.That( selectedEdges, "G12.the-selected-model-box-is-in-the-selection-color" );

		// Instance content: every face color times the tint, edges in the edge color.
		Document d;
		Build( d );
		const hammer::viewport::RenderSnapshot plainSnapshot = Snapshot( d, false );
		GeometryOptions tinted;
		tinted.tint = hammer::viewport::kInstanceTint;
		tinted.edgeColor = hammer::viewport::kInstanceEdgeColor;
		const SceneGeometry plain = BuildSceneGeometry( plainSnapshot );
		const SceneGeometry tint = BuildSceneGeometry( plainSnapshot, tinted );
		bool multiplied = plain.faces.size() == tint.faces.size();
		for ( std::size_t b = 0; multiplied && b < plain.faces.size(); ++b )
		{
			const auto &p = plain.faces[b].vertices;
			const auto &t = tint.faces[b].vertices;
			multiplied = p.size() == t.size();
			for ( std::size_t i = 0; multiplied && i < p.size(); ++i )
				multiplied =
				    std::abs( int( t[i].color[0] ) - int( p[i].color[0] ) ) <= 1 &&
				    std::abs( int( t[i].color[1] ) - int( p[i].color[1] ) ) <= 1 &&
				    std::abs( int( t[i].color[2] ) - int( p[i].color[2] ) * 128 / 255 ) <= 1;
		}
		checks.That( multiplied, "G12.instance-faces-are-multiplied-by-the-tint" );
		bool edgeColor = !tint.edges.empty();
		for ( const auto &v : tint.edges )
			edgeColor =
			    edgeColor && v.color == render::pass::lines::Rgba8{ 128, 128, 0, 255 }.Packed();
		checks.That( edgeColor, "G12.instance-edges-are-in-the-instance-edge-color" );
	}

	// --- G13 ---------------------------------------------------------------------------
	{
		const ModelAsset asset = TwoMeshes();
		const std::vector<ModelBatch> skin0 =
		    BuildModelBatches( asset, 0, &Sized, { 255, 255, 255 }, { 200, 200, 200 } );
		checks.That(
		    skin0.size() == 2 && skin0[0].material.empty() && skin0[1].material == "m/crate",
		    "G13.untextured-batch-first-then-by-material" );
		if ( skin0.size() == 2 )
		{
			checks.That( skin0[0].indices.size() == 36 && skin0[1].indices.size() == 36,
			    "G13.each-batch-holds-its-mesh-triangles" );
			bool inRange = true;
			for ( const ModelBatch &batch : skin0 )
				for ( std::uint32_t i : batch.indices )
					inRange = inRange && i < batch.vertices.size();
			checks.That( inRange, "G13.indices-name-the-batch-vertices" );
			const mdl::Mesh &box = asset.model.meshes[0];
			bool uv = skin0[1].vertices.size() == box.vertices.size();
			for ( std::size_t i = 0; uv && i < box.vertices.size(); ++i )
				uv = skin0[1].vertices[i].uv[0] == box.vertices[i].u &&
				     skin0[1].vertices[i].uv[1] == box.vertices[i].v;
			checks.That( uv, "G13.textured-vertices-keep-the-model-uv" );
			bool untexturedUv = true;
			for ( const auto &v : skin0[0].vertices )
				untexturedUv = untexturedUv && v.uv[0] == 0.0f && v.uv[1] == 0.0f;
			checks.That( untexturedUv, "G13.untextured-vertices-have-no-uv" );
			// The top face (+z) of the textured box, white tint vs a tint.
			const std::vector<ModelBatch> red =
			    BuildModelBatches( asset, 0, &Sized, { 255, 0, 0 }, { 200, 200, 200 } );
			bool times = red.size() == 2;
			for ( std::size_t i = 0; times && i < skin0[1].vertices.size(); ++i )
				times = red[1].vertices[i].color[0] == skin0[1].vertices[i].color[0] &&
				        red[1].vertices[i].color[1] == 0 && red[1].vertices[i].color[2] == 0;
			checks.That( times, "G13.colors-are-the-shading-times-the-tint" );
		}
		const std::vector<ModelBatch> skin1 =
		    BuildModelBatches( asset, 1, &Sized, { 255, 255, 255 }, { 200, 200, 200 } );
		checks.That( skin1.size() == 2 && skin1[1].material == "m/rust",
		    "G13.the-skin-family-picks-the-texture" );
		const std::vector<ModelBatch> flat =
		    BuildModelBatches( asset, 0, {}, { 255, 255, 255 }, { 200, 200, 200 } );
		checks.That( flat.size() == 1 && flat[0].material.empty() && flat[0].indices.size() == 72,
		    "G13.without-sizes-everything-is-untextured" );

		EntityDraw e = ModelEntity( 1, Vec3d( 10, 20, 30 ), Vec3d( 0, 90, 0 ) );
		e.modelKeys.scale = 2.0;
		const render::math::float4x4 world = hammer::render_adapter::ModelWorld( e );
		const auto apply = [&]( Vec3d p )
		{
			const float *r0 = &world.rows[0].x;
			const float *r1 = &world.rows[1].x;
			const float *r2 = &world.rows[2].x;
			return Vec3d( r0[0] * p.x + r0[1] * p.y + r0[2] * p.z + r0[3],
			    r1[0] * p.x + r1[1] * p.y + r1[2] * p.z + r1[3],
			    r2[0] * p.x + r2[1] * p.y + r2[2] * p.z + r2[3] );
		};
		checks.That( Near( apply( Vec3d( 1, 0, 0 ) ), Vec3d( 10, 22, 30 ) ) &&
		                 Near( apply( Vec3d( 0, 0, 1 ) ), Vec3d( 10, 20, 32 ) ),
		    "G13.model-world-rotates-scales-and-translates" );
		EntityDraw unit = ModelEntity( 1, Vec3d( 10, 20, 30 ), Vec3d( 0, 90, 0 ) );
		const hammer::scene::Box box = hammer::render_adapter::ModelWorldBox( asset.model, unit );
		checks.That(
		    Near( box.mins, Vec3d( -6, -12, 22 ) ) && Near( box.maxs, Vec3d( 26, 52, 46 ) ),
		    "G13.model-world-box-bounds-the-rotated-model" );
		EntityDraw colored = unit;
		colored.modelKeys.renderColor = hammer::scene::Rgb{ 100, 200, 250 };
		checks.That( hammer::render_adapter::ModelTint( colored, false ) ==
		                     hammer::scene::Rgb{ 100, 200, 250 } &&
		                 hammer::render_adapter::ModelTint( colored, true ) ==
		                     hammer::scene::Rgb{ 100, 200, 125 } &&
		                 hammer::render_adapter::ModelTint( unit, true ) ==
		                     hammer::scene::Rgb{ 255, 255, 128 },
		    "G13.model-tint-is-the-render-color-times-the-instance-tint" );
		colored.selected = true;
		checks.That( hammer::render_adapter::ModelTint( colored, true ) ==
		                 hammer::scene::Rgb{ 255, 158, 71 },
		    "G13.a-selected-model-is-tinted-with-the-selection-fill" );
	}

	// --- G14 ---------------------------------------------------------------------------
	{
		using hammer::render_adapter::ModelSequence;
		using hammer::render_adapter::ModelWorldBox;
		using hammer::render_adapter::PosedModel;
		const ModelAsset asset = Standing();
		hammer::viewport::ModelKeys keys;
		keys.sequence = "STAND";
		checks.That( ModelSequence( asset.model, keys ) == 1, "G14.the-label-names-the-sequence" );
		keys.sequence = "missing";
		keys.sequenceIndex = 1;
		checks.That( ModelSequence( asset.model, keys ) == 1,
		    "G14.a-label-the-model-lacks-falls-back-to-the-index" );
		keys.sequence.clear();
		keys.sequenceIndex = 9;
		checks.That( ModelSequence( asset.model, keys ) == 0 &&
		                 ModelSequence( TwoMeshes().model, keys ) == -1,
		    "G14.an-index-out-of-range-is-0-and-no-sequences-is--1" );
		const ModelAsset posed = PosedModel( asset, 1 );
		const Vec3d lo( posed.model.mins.x, posed.model.mins.y, posed.model.mins.z );
		const Vec3d hi( posed.model.maxs.x, posed.model.maxs.y, posed.model.maxs.z );
		checks.That( Near( lo, Vec3d( -2, -4, 0 ), 1e-3 ) && Near( hi, Vec3d( 2, 4, 40 ), 1e-3 ) &&
		                 posed.materials.size() == 1,
		    "G14.the-posed-model-stands-along-z" );
		checks.That( PosedModel( asset, 0 ).model.maxs.y > 39.0f,
		    "G14.the-bind-pose-sequence-keeps-the-authored-y-up-model" );
		EntityDraw e = ModelEntity( 1, Vec3d( 100, 0, 0 ), Vec3d( 0, 90, 0 ) );
		const hammer::scene::Box box = ModelWorldBox( posed.model, e );
		// Yaw 90 turns (x y z) to (-y x z): (-4 -2 0)-(4 2 40), then + (100 0 0).
		checks.That( Near( box.mins, Vec3d( 96, -2, 0 ), 1e-3 ) &&
		                 Near( box.maxs, Vec3d( 104, 2, 40 ), 1e-3 ),
		    "G14.the-world-box-bounds-the-posed-model" );
	}

	// --- V11 ---------------------------------------------------------------------------
	{
		nulldev::NullOptions nullOptions;
		auto device = nulldev::Create( nullOptions ).Value();
		const std::size_t baseline = device->LiveResourceCount();
		{
			FakeModels models;
			FakeTextures textures;
			auto made = ViewportRenderer::Create( *device, &textures, &models );
			if ( !checks.That( made.HasValue(), "V11.setup" ) )
				return checks.Report();
			ViewportRenderer &renderer = *made.Value();
			hammer::viewport::RenderSnapshot snapshot;
			for ( int i = 0; i < 3; ++i )
			{
				EntityDraw e =
				    ModelEntity( std::uint64_t( 10 + i ), Vec3d( 64.0 * i, 0, 0 ), Vec3d() );
				e.classname = "prop_dynamic";
				e.model = "models/test/stand.mdl";
				e.modelKeys.sequence = i == 0 ? "" : "stand";
				snapshot.entities.push_back( e );
			}
			snapshot.bounds = hammer::scene::Box{ Vec3d( -64, -64, -64 ), Vec3d( 256, 64, 64 ) };
			checks.That( renderer.SetScene( snapshot, 1 ).HasValue(), "V11.the-scene-stages" );
			const hammer::render_adapter::SceneStats first = renderer.Scene();
			checks.That( models.reads["models/test/stand.mdl"] == 1 && first.modelVariants == 2 &&
			                 first.modelEntities == 3,
			    "V11.one-file-two-sequences-two-variants" );
			snapshot.entities[2].origin = Vec3d( 200, 0, 0 );
			checks.That( renderer.SetScene( snapshot, 2 ).HasValue() &&
			                 renderer.Scene().modelVariants == 2 &&
			                 models.reads["models/test/stand.mdl"] == 1,
			    "V11.a-moved-posed-model-reuses-its-variant" );
		}
		(void)device->WaitIdle();
		(void)device->Poll();
		checks.Equal( device->LiveResourceCount(), baseline, "V11.everything-is-released" );
	}

	// --- V10 ---------------------------------------------------------------------------
	{
		nulldev::NullOptions nullOptions;
		auto device = nulldev::Create( nullOptions ).Value();
		const std::size_t baseline = device->LiveResourceCount();
		{
			FakeModels models;
			FakeTextures textures;
			auto made = ViewportRenderer::Create( *device, &textures, &models );
			if ( !checks.That( made.HasValue(), "V10.setup" ) )
				return checks.Report();
			ViewportRenderer &renderer = *made.Value();
			hammer::viewport::RenderSnapshot snapshot;
			snapshot.entities = { ModelEntity( 1, Vec3d( 0, 0, 0 ), Vec3d() ),
			    ModelEntity( 2, Vec3d( 100, 0, 0 ), Vec3d( 0, 45, 0 ) ),
			    ModelEntity( 3, Vec3d( 200, 0, 0 ), Vec3d(), true ) };
			EntityDraw missing = ModelEntity( 4, Vec3d( 300, 0, 0 ), Vec3d() );
			missing.model = "models/test/missing.mdl";
			snapshot.entities.push_back( missing );
			snapshot.bounds = hammer::scene::Box{ Vec3d( -64, -64, -64 ), Vec3d( 364, 64, 64 ) };
			checks.That( renderer.SetScene( snapshot, 1 ).HasValue(), "V10.the-scene-stages" );
			const hammer::render_adapter::SceneStats first = renderer.Scene();
			checks.That( models.reads["models/test/crate.mdl"] == 1 &&
			                 models.reads["models/test/missing.mdl"] == 1 && first.models == 2,
			    "V10.each-model-file-is-read-once" );
			checks.That(
			    first.modelVariants == 2 && first.modelEntities == 3 && first.missingModels == 1,
			    "V10.one-variant-per-skin-and-tint-three-drawn-one-missing" );

			hammer::viewport::Camera3D eye;
			eye.SetViewport( 64, 48 );
			eye.SetPosition( Vec3d( 150, -600, 200 ) );
			eye.LookAt( Vec3d( 150, 0, 0 ) );
			ViewRequest request;
			request.kind = ViewKind::Camera3D;
			request.camera3D = &eye;
			request.pixelWidth = 64;
			request.pixelHeight = 48;
			nulldev::Control( *device )->ClearRecorded();
			checks.That( renderer.RenderAndWait( request ).HasValue(), "V10.the-view-renders" );
			// Each drawn entity: one indexed draw per batch (36 indices each).
			checks.Equal( IndexedDraws( *device, 36 ), std::size_t( 6 ),
			    "V10.one-indexed-draw-per-batch-and-entity" );

			// Move one prop: its chunk restages, the model is not read or staged again.
			snapshot.entities[1].origin = Vec3d( 120, 0, 0 );
			checks.That(
			    renderer.SetScene( snapshot, 2 ).HasValue(), "V10.the-moved-scene-stages" );
			const hammer::render_adapter::SceneStats second = renderer.Scene();
			checks.That( models.reads["models/test/crate.mdl"] == 1 && second.modelVariants == 2,
			    "V10.a-move-reads-and-stages-no-model" );
			checks.Equal(
			    second.stagedChunks, std::uint32_t( 1 ), "V10.a-move-restages-its-chunk" );
		}
		(void)device->WaitIdle();
		(void)device->Poll();
		checks.Equal( device->LiveResourceCount(), baseline, "V10.everything-is-released" );
	}
	return checks.Report();
}
