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

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <filesystem>
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

class EmptyTextures final : public IWorldTextures
{
public:
	TextureId Import( int, bool ) override { return {}; }
	SamplerDesc Sampler( int ) override { return {}; }
};

WorldData MeshWorld()
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
	world.materials.push_back( std::move( material ) );
	WorldData::StaticMesh mesh;
	for ( const auto &xy : { std::pair{ -0.5f, -0.5f }, std::pair{ 0.5f, -0.5f },
	          std::pair{ 0.5f, 0.5f }, std::pair{ -0.5f, 0.5f } } )
	{
		material::SurfaceModelVertex vertex;
		vertex.position[0] = xy.first;
		vertex.position[1] = xy.second;
		vertex.position[2] = 0.5f;
		vertex.normal[2] = 1.0f;
		vertex.tangent[0] = vertex.tangent[3] = 1.0f;
		mesh.vertices.push_back( vertex );
	}
	mesh.indices = { 0, 1, 2, 0, 2, 3 };
	mesh.surfaces.push_back( { 0, 0, 0, 6 } );
	world.staticMeshes.push_back( std::move( mesh ) );
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
	WorldData world = MeshWorld();
	const auto bindPose = world.staticMeshes[0].vertices;
	WorldPass pass;
	pass.SetWorld( std::move( world ) );
	results.That( pass.DrawsPosedModel( 0, 0 ), "posed-model.claims-mesh-program" );
	EmptyTextures empty;
	auto render = [&]( float offset, bool lit, std::uint64_t frame,
	                  CanvasImage &image ) -> std::optional<std::string>
	{
		WorldView view;
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 1.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		view.hostFrame = frame;
		WorldView::PosedModel pose;
		pose.vertices = bindPose;
		for ( auto &vertex : pose.vertices )
			vertex.position[0] += offset;
		view.posedModels.push_back( std::move( pose ) );
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
		const std::uint32_t tag = pass.QueueView( std::move( view ) );
		if ( !tag )
			return "the posed view queued nothing";
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
			target.frame = frame;
			target.eye[2] = 2.0f;
			pass.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {}, { 0, 0, 0, 1 }, &image, post );
	};
	CanvasImage center, shifted, dark;
	if ( std::optional<std::string> why = render( 0.0f, true, 1, center ) )
		return why;
	if ( std::optional<std::string> why = render( 0.5f, true, 2, shifted ) )
		return why;
	if ( std::optional<std::string> why = render( 0.0f, false, 3, dark ) )
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
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	messages = counter.load();
	return std::nullopt;
}

} // namespace

int RunPosedModelSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "posed-model", std::span<const Seeded>(), RunChecks );
}

} // namespace render::lab
