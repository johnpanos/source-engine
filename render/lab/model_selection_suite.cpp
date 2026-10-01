//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Body-group surface selection through the real world pass. Pixel
//          footprints are independent of the reader and selection mechanism.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "mdl/studio_model.h"
#include "render/pass/world/world_pass.h"

#include <atomic>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>

namespace render::lab
{

namespace
{

using namespace render::device;
using namespace render::pass::world;

constexpr std::uint32_t kSize = 64;

class SelectionTextures final : public IWorldTextures
{
public:
	TextureId Import( int, bool ) override { return {}; }
	SamplerDesc Sampler( int ) override { return {}; }
};

WorldData SelectionWorld()
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	WorldData::StaticMesh mesh;
	for ( std::uint32_t i = 0; i < 3; ++i )
	{
		WorldMaterial mat;
		mat.name = "body-alternative-" + std::to_string( i );
		mat.shader = "UnlitGeneric";
		mat.mesh = true;
		mat.variables.emplace_back( "$color", i == 0 ? "[1 0 0]" : "[0 1 0]" );
		if ( i == 2 )
			mat.variables.emplace_back( "$unsupported-body-effect", "1" );
		world.materials.push_back( std::move( mat ) );
		const float left = i == 0 ? -0.8f : 0.2f;
		const float right = left + 0.6f;
		for ( const auto &xy : { std::pair{ left, -0.5f }, std::pair{ right, -0.5f },
		          std::pair{ right, 0.5f }, std::pair{ left, 0.5f } } )
		{
			material::SurfaceModelVertex vertex;
			vertex.position[0] = xy.first;
			vertex.position[1] = xy.second;
			vertex.position[2] = 0.5f;
			vertex.normal[2] = 1.0f;
			vertex.tangent[0] = vertex.tangent[3] = 1.0f;
			mesh.vertices.push_back( vertex );
		}
		const std::uint32_t base = i * 4;
		mesh.indices.insert(
		    mesh.indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 } );
		mesh.surfaces.push_back( { i, 0, i * 6, 6 } );
	}
	world.staticMeshes.push_back( std::move( mesh ) );
	for ( std::uint32_t i = 0; i < 3; ++i )
	{
		WorldData::StaticInstance instance;
		instance.world[0] = instance.world[5] = instance.world[10] = instance.world[15] = 1.0f;
		instance.surfaceSelection.emplace();
		if ( i < 2 )
			instance.surfaceSelection->push_back( i );
		world.staticInstances.push_back( std::move( instance ) );
	}
	return world;
}

WorldView SelectionView( const WorldData &world, int body )
{
	// The fourth submodel is blank. This is authored geometry selection, not
	// a material-name rule; the content module owns body/base/count arithmetic.
	const mdl::BodyPart part{ 1, 4 };
	const std::uint32_t selected = part.SelectedModel( body );
	WorldView::PosedModel pose;
	pose.vertices = world.staticMeshes[0].vertices;
	pose.surfaceSelection.emplace();
	if ( selected < 3 )
		pose.surfaceSelection->push_back( selected );
	WorldView view;
	view.toClip[0] = view.toClip[5] = view.toClip[10] = view.toClip[15] = 1.0f;
	view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
	view.posedModels.push_back( std::move( pose ) );
	return view;
}

bool LeftFootprint( const CanvasImage &image )
{
	const float *left = image.At( 16, 32 );
	const float *right = image.At( 48, 32 );
	return left[0] > 0.9f && left[1] < 0.01f && right[0] < 0.01f && right[1] < 0.01f;
}

bool RightFootprint( const CanvasImage &image )
{
	const float *left = image.At( 16, 32 );
	const float *right = image.At( 48, 32 );
	return right[1] > 0.9f && right[0] < 0.01f && left[0] < 0.01f && left[1] < 0.01f;
}

WorldData ImportedLodWorld( const mdl::Model &model )
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	for ( const auto &slots : model.lodTextures )
	{
		WorldMaterial material;
		material.name = slots[0];
		material.shader = "UnlitGeneric";
		material.mesh = true;
		material.variables.emplace_back( "$color", slots[0] == "base" ? "[1 0 0]" : "[0 1 0]" );
		world.materials.push_back( std::move( material ) );
	}
	WorldData::StaticMesh mesh;
	for ( const mdl::Mesh &part : model.meshes )
	{
		const std::uint32_t firstVertex = std::uint32_t( mesh.vertices.size() );
		const std::uint32_t firstIndex = std::uint32_t( mesh.indices.size() );
		for ( const mdl::Vertex &vertex : part.vertices )
		{
			material::SurfaceModelVertex out;
			out.position[0] = vertex.position.x;
			out.position[1] = vertex.position.y;
			out.position[2] = vertex.position.z;
			out.normal[2] = out.tangent[0] = out.tangent[3] = 1.0f;
			mesh.vertices.push_back( out );
		}
		for ( std::uint32_t index : part.indices )
			mesh.indices.push_back( firstVertex + index );
		mesh.surfaces.push_back(
		    { part.lod, 0, firstIndex, std::uint32_t( part.indices.size() ) } );
	}
	world.staticMeshes.push_back( std::move( mesh ) );
	return world;
}

WorldView ImportedLodView(
    const mdl::Model &model, const WorldData &world, unsigned int lod, int body = 0 )
{
	WorldView view;
	view.toClip[0] = view.toClip[5] = view.toClip[10] = view.toClip[15] = 1.0f;
	view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
	WorldView::PosedModel pose;
	pose.vertices = world.staticMeshes[0].vertices;
	pose.surfaceSelection.emplace();
	for ( std::uint32_t i = 0; i < model.meshes.size(); ++i )
	{
		const mdl::Mesh &mesh = model.meshes[i];
		if ( mesh.lod == lod &&
		     model.bodyParts[mesh.bodyPart].SelectedModel( body ) == mesh.bodyModel )
			pose.surfaceSelection->push_back( i );
	}
	view.posedModels.push_back( std::move( pose ) );
	return view;
}

std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	SelectionTextures imports;
	WorldData world = SelectionWorld();
	WorldPass pass;
	pass.SetWorld( world );
	WorldView left = SelectionView( world, 0 );
	WorldView right = SelectionView( world, 1 );
	WorldView unsupported = SelectionView( world, 2 );
	WorldView blank = SelectionView( world, 3 );
	results.That( pass.DrawsPosedModel(
	                  0, 0, RenderCoreDrawPhase::kAll, left.posedModels[0].surfaceSelection ) &&
	                  pass.DrawsPosedModel(
	                      0, 0, RenderCoreDrawPhase::kAll, right.posedModels[0].surfaceSelection ),
	    "model-selection.inactive-unsupported-material-does-not-reject-selected-body" );
	results.That( !pass.DrawsPosedModel( 0, 0, RenderCoreDrawPhase::kAll,
	                  unsupported.posedModels[0].surfaceSelection ),
	    "model-selection.active-unsupported-material-is-refused" );
	results.That( pass.DrawsPosedModel(
	                  0, 0, RenderCoreDrawPhase::kAll, blank.posedModels[0].surfaceSelection ),
	    "model-selection.blank-body-is-a-valid-empty-draw" );
	WorldView invalid = left;
	invalid.posedModels[0].surfaceSelection = { { 3 } };
	results.That( pass.QueueView( std::move( invalid ) ) == 0,
	    "model-selection.invalid-surface-refused-before-claiming-slot" );
	invalid = left;
	invalid.posedModels[0].surfaceSelection = { { 1, 0 } };
	results.That( pass.QueueView( std::move( invalid ) ) == 0,
	    "model-selection.unordered-selection-refused" );
	invalid = left;
	invalid.posedModels[0].surfaceSelection = { { 0, 0 } };
	results.That( pass.QueueView( std::move( invalid ) ) == 0,
	    "model-selection.duplicate-selection-refused" );

	const std::uint32_t leftTag = pass.QueueView( left );
	// Change the authored selection after queueing. The first draw owns its copy.
	left.posedModels[0].surfaceSelection = right.posedModels[0].surfaceSelection;
	const std::uint32_t rightTag = pass.QueueView( left );
	const std::uint32_t blankTag = pass.QueueView( blank );
	results.That( leftTag && rightTag && blankTag,
	    "model-selection.body-changes-queue-independent-snapshots" );
	std::uint64_t frame = 0;
	auto render = [&]( std::uint32_t tag, CanvasImage &image )
	{
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
			target.textures = &imports;
			target.frame = ++frame;
			pass.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {}, { 0, 0, 0, 1 }, &image, post );
	};
	CanvasImage leftImage, rightImage, blankImage;
	if ( auto why = render( leftTag, leftImage ) )
		return why;
	if ( auto why = render( rightTag, rightImage ) )
		return why;
	if ( auto why = render( blankTag, blankImage ) )
		return why;
	results.That( LeftFootprint( leftImage ), "model-selection.first-body-pixel-footprint" );
	results.That( RightFootprint( rightImage ), "model-selection.second-body-pixel-footprint" );
	results.That( !LeftFootprint( rightImage ) && !RightFootprint( leftImage ),
	    "model-selection.swapped-body-negative-control-is-detected" );
	results.That( blankImage.At( 16, 32 )[0] == 0.0f && blankImage.At( 48, 32 )[1] == 0.0f,
	    "model-selection.blank-body-draws-neither-alternative" );
	results.That( pass.Stats().viewsFailed == 0 && pass.Stats().viewsDrawn == 3 &&
	                  pass.Stats().posedDrawsDrawn == 2,
	    "model-selection.only-selected-surfaces-record", pass.Stats().lastFailure );
	results.That( pass.DrawsStaticInstance( 0 ) && pass.DrawsStaticInstance( 1 ) &&
	                  pass.DrawsStaticInstance( 2 ),
	    "model-selection.static-instances-claim-only-their-selected-surfaces" );
	for ( std::uint32_t i = 0; i < 3; ++i )
	{
		WorldView view = SelectionView( world, 0 );
		view.posedModels.clear();
		view.staticInstances = { i };
		const std::uint32_t tag = pass.QueueView( std::move( view ) );
		CanvasImage image;
		if ( auto why = render( tag, image ) )
			return why;
		results.That( i == 0   ? LeftFootprint( image )
		              : i == 1 ? RightFootprint( image )
		                       : image.At( 16, 32 )[0] == 0.0f && image.At( 48, 32 )[1] == 0.0f,
		    "model-selection.static-body-pixel-footprint-" + std::to_string( i ) );
	}
	results.That( pass.Stats().viewsFailed == 0 && pass.Stats().viewsDrawn == 6 &&
	                  pass.Stats().staticDrawsDrawn == 2,
	    "model-selection.static-blank-body-records-no-geometry", pass.Stats().lastFailure );
	CanvasImage replay;
	if ( auto why = render( leftTag, replay ) )
		return why;
	results.That( LeftFootprint( replay ) && pass.Stats().viewsFailed == 0,
	    "model-selection.capture-replay-keeps-the-earlier-body-selection" );
	WorldData emptyWorld = world;
	emptyWorld.staticMeshes[0] = {};
	emptyWorld.materials.clear();
	pass.SetWorld( emptyWorld );
	const std::uint64_t beforeBlank = pass.Stats().viewsDrawn;
	WorldView emptyView = SelectionView( emptyWorld, 3 );
	results.That( pass.DrawsPosedModel( 0, 0, RenderCoreDrawPhase::kAll,
	                  emptyView.posedModels[0].surfaceSelection ) &&
	                  pass.DrawsStaticInstance( 2 ),
	    "model-selection.wholly-blank-model-needs-no-geometry-or-material" );
	const std::uint32_t emptyTag = pass.QueueView( std::move( emptyView ) );
	CanvasImage emptyImage;
	if ( auto why = render( emptyTag, emptyImage ) )
		return why;
	results.That( emptyTag && pass.Stats().viewsFailed == 0 &&
	                  pass.Stats().viewsDrawn == beforeBlank + 1 &&
	                  emptyImage.At( 16, 32 )[0] == 0.0f && emptyImage.At( 48, 32 )[1] == 0.0f,
	    "model-selection.wholly-blank-model-records-an-empty-view", pass.Stats().lastFailure );
	const auto read = []( const char *suffix )
	{
		std::ifstream file(
		    std::string( "render/lab/fixtures/model-lods/selection." ) + suffix, std::ios::binary );
		return std::string( std::istreambuf_iterator<char>( file ), {} );
	};
	const std::string mdlBytes = read( "mdl" ), vvdBytes = read( "vvd" ), vtxBytes = read( "vtx" );
	const auto geometry = mdl::ParseModelGeometryVariants( { mdlBytes, vvdBytes, vtxBytes, {} } );
	const bool expectedGeometry = geometry && geometry.Value().lodTextures.size() == 3 &&
	                              geometry.Value().meshes.size() == 2 &&
	                              geometry.Value().bodyParts.size() == 1 &&
	                              geometry.Value().bodyParts[0].modelCount == 2;
	results.That( expectedGeometry, "model-selection.LOD-fixture-imports-all-levels" );
	if ( !expectedGeometry )
		return geometry ? "LOD fixture has unexpected topology" : mdl::Describe( geometry.Error() );
	const mdl::Model &model = geometry.Value();
	world = ImportedLodWorld( model );
	pass.SetWorld( world );
	WorldView high = ImportedLodView( model, world, 0 );
	WorldView low = ImportedLodView( model, world, 1 );
	WorldView noGeometry = ImportedLodView( model, world, 2 );
	results.That( model.lodTextures[0][0] == "base" && model.lodTextures[1][0] == "lower" &&
	                  model.lodTextures[2][0] == "base",
	    "model-selection.LOD-material-replacement-is-slot-specific" );
	const std::uint32_t highTag = pass.QueueView( high );
	high.posedModels[0].surfaceSelection = low.posedModels[0].surfaceSelection;
	const std::uint32_t lowTag = pass.QueueView( high );
	const std::uint32_t noGeometryTag = pass.QueueView( noGeometry );
	results.That( highTag && lowTag && noGeometryTag,
	    "model-selection.LOD-changes-own-their-queued-topology" );
	CanvasImage highImage, lowImage, noGeometryImage;
	if ( auto why = render( highTag, highImage ) )
		return why;
	if ( auto why = render( lowTag, lowImage ) )
		return why;
	if ( auto why = render( noGeometryTag, noGeometryImage ) )
		return why;
	results.That( LeftFootprint( highImage ), "model-selection.LOD-zero-reference-footprint" );
	results.That(
	    RightFootprint( lowImage ), "model-selection.lower-LOD-topology-and-material-footprint" );
	results.That( !LeftFootprint( lowImage ) && !RightFootprint( highImage ),
	    "model-selection.wrong-LOD-negative-control-is-detected" );
	results.That(
	    noGeometryImage.At( 16, 32 )[0] == 0.0f && noGeometryImage.At( 48, 32 )[1] == 0.0f,
	    "model-selection.blank-LOD-has-no-geometry" );
	const std::uint32_t blankBodyTag = pass.QueueView( ImportedLodView( model, world, 1, 1 ) );
	CanvasImage blankBodyImage;
	if ( auto why = render( blankBodyTag, blankBodyImage ) )
		return why;
	results.That( blankBodyTag && blankBodyImage.At( 16, 32 )[0] == 0.0f &&
	                  blankBodyImage.At( 48, 32 )[1] == 0.0f,
	    "model-selection.body-and-LOD-select-independently" );
	if ( auto why = render( highTag, replay ) )
		return why;
	results.That( LeftFootprint( replay ) && pass.Stats().viewsFailed == 0,
	    "model-selection.LOD-capture-replay-keeps-the-earlier-level", pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "model-selection.validation-silent" );
	return std::nullopt;
}

} // namespace

int RunModelSelectionSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "model-selection", std::span<const Seeded>(), RunChecks );
}

} // namespace render::lab
