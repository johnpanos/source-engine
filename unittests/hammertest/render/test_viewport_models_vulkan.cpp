//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.viewport.models (RFC 0002 R17 follow-up,
//			props and instances; linux-native-vulkan-gpu): the viewports draw
//			studio models and func_instance content with real pixels, judged
//			by relations. The scene comes from the editor's own path: a
//			document through the extraction with an InstancePreview over real
//			VMF text (VmfMapCodec, in-memory store); the model is a synthetic
//			MDL/VVD/VTX box (unittests/mdltest/synthetic_model.h) read by
//			content.studio-model, textured red through a fake material source.
//
//			M1 a prop (yaw 90) shows its textured top face (the texel times
//			   the face's shading, within two levels) where its rotated
//			   footprint projects, and the background where only the unrotated
//			   footprint would lie (the transform, not just the position);
//			M2 a prop whose model is missing draws its marker box and is
//			   counted; the model file is read once for two entities;
//			M3 the instance's box (placed by yaw 90 and a translation) shows
//			   its fill times the instance tint where the placement puts its
//			   top, which differs from the untinted fill (the tint shows);
//			   the instance's own prop shows the texel times the tinted
//			   shading;
//			M4 2D: the prop's rotated world box is drawn in its entity color
//			   (and not the unrotated one); the instance's edges are drawn in
//			   the instance edge color;
//			M5 selecting the prop tints its model with the selection fill;
//			the Khronos validation layer reports no message.
//
//=============================================================================//

#include "hammer/adapters/render/viewport_renderer.h"
#include "hammer/app/instance_preview.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/viewport/extraction.h"
#include "render/device/vulkan/provider.h"
#include "testing/checks.h"
#include "unittests/hammertest/app/fake_file_store.h"
#include "unittests/mdltest/synthetic_model.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>

namespace
{

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
using hammer::render_adapter::ViewPixels;
using hammer::render_adapter::ViewRequest;
using hammer::scene::ObjectId;
using hammer::viewport::ViewKind;
using mapgeometry::Vec3d;

struct Rgb
{
	int r = 0;
	int g = 0;
	int b = 0;
	friend bool operator==( const Rgb &, const Rgb & ) = default;
};

Rgb At( const ViewPixels &pixels, double x, double y )
{
	const int ix = int( std::floor( x ) );
	const int iy = int( std::floor( y ) );
	if ( ix < 0 || iy < 0 || ix >= int( pixels.width ) || iy >= int( pixels.height ) ||
	     pixels.rgba.size() != std::size_t( pixels.width ) * pixels.height * 4 )
		return { -1, -1, -1 };
	const std::size_t i = ( std::size_t( iy ) * pixels.width + std::size_t( ix ) ) * 4;
	return { pixels.rgba[i], pixels.rgba[i + 1], pixels.rgba[i + 2] };
}

bool Near( const ViewPixels &pixels, double x, double y, Rgb color, int levels = 2 )
{
	for ( int dy = -1; dy <= 1; ++dy )
		for ( int dx = -1; dx <= 1; ++dx )
		{
			const Rgb c = At( pixels, x + dx, y + dy );
			if ( std::abs( c.r - color.r ) <= levels && std::abs( c.g - color.g ) <= levels &&
			     std::abs( c.b - color.b ) <= levels )
				return true;
		}
	return false;
}

bool Within( Rgb a, Rgb b, int levels )
{
	return std::abs( a.r - b.r ) <= levels && std::abs( a.g - b.g ) <= levels &&
	       std::abs( a.b - b.b ) <= levels;
}

double ToLinear( double display )
{
	return display <= 0.04045 ? display / 12.92 : std::pow( ( display + 0.055 ) / 1.055, 2.4 );
}

int ToDisplay( double linear )
{
	const double v =
	    linear <= 0.0031308 ? linear * 12.92 : 1.055 * std::pow( linear, 1.0 / 2.4 ) - 0.055;
	return int( std::lround( std::clamp( v, 0.0, 1.0 ) * 255.0 ) );
}

// texel x shade in linear light, as the unlit family draws a textured face.
Rgb Modulated( Rgb texel, Rgb shade )
{
	auto channel = []( int t, int s )
	{
		return ToDisplay( ToLinear( t / 255.0 ) * ToLinear( s / 255.0 ) );
	};
	return {
	    channel( texel.r, shade.r ), channel( texel.g, shade.g ), channel( texel.b, shade.b ) };
}

constexpr Rgb kRed{ 220, 40, 30 };
constexpr const char *kCrate = "models/test/crate";
constexpr const char *kCratePath = "models/test/crate.mdl";

// The synthetic crate: a box of half-extents (32, 16, 8) about its origin.
ModelAsset Crate()
{
	const mdltest::SyntheticFiles files =
	    mdltest::WriteModel( mdltest::BoxModel( { 32, 16, 8 }, "models/test/", "crate" ) );
	ModelAsset asset;
	asset.model = mdl::ParseModel( { files.mdl, files.vvd, files.vtx } ).Value();
	asset.materials = { mdl::ResolvedMaterial{ kCrate, true } };
	return asset;
}

class FakeModels final : public IModelSource
{
public:
	std::map<std::string, int> reads;
	foundation::Expected<ModelAsset, mdl::ModelError> Model( const std::string &path ) override
	{
		++reads[path];
		if ( path != kCratePath )
			return foundation::MakeUnexpected(
			    mdl::ModelError{ mdl::ModelStatus::MissingFile, mdl::ModelFile::Mdl, 0 } );
		return Crate();
	}
};

// The crate's material is uniform red; nothing else has a texture.
class RedCrate final : public IMaterialTextures
{
public:
	std::optional<MaterialImage> BaseTexture( const std::string &material ) override
	{
		if ( material != kCrate )
			return std::nullopt;
		MaterialImage image;
		image.width = image.height = 64;
		for ( int i = 0; i < 64 * 64; ++i )
			image.rgba.insert( image.rgba.end(),
			    { std::uint8_t( kRed.r ), std::uint8_t( kRed.g ), std::uint8_t( kRed.b ), 255 } );
		return image;
	}
};

// The color of a vertex of the top face (normal +z) in the crate's one
// textured batch (the model's one mesh, vertices in model order).
std::optional<Rgb> TopColor( const std::vector<ModelBatch> &batches )
{
	const ModelAsset crate = Crate();
	if ( batches.size() != 1 || batches[0].material.empty() || crate.model.meshes.size() != 1 )
		return std::nullopt;
	const std::vector<mdl::Vertex> &vertices = crate.model.meshes[0].vertices;
	for ( std::size_t i = 0; i < vertices.size() && i < batches[0].vertices.size(); ++i )
		if ( vertices[i].normal.z > 0.9f )
			return Rgb{ batches[0].vertices[i].color[0], batches[0].vertices[i].color[1],
			    batches[0].vertices[i].color[2] };
	return std::nullopt;
}

std::optional<Rgb> TopFaceColor( const SceneGeometry &g, double z )
{
	for ( const auto &batch : g.faces )
		for ( std::size_t i = 0; i + 2 < batch.vertices.size(); i += 3 )
		{
			bool match = true;
			for ( std::size_t k = 0; k < 3; ++k )
				match = match && std::fabs( batch.vertices[i + k].position[2] - z ) < 1e-3;
			if ( match )
				return Rgb{ batch.vertices[i].color[0], batch.vertices[i].color[1],
				    batch.vertices[i].color[2] };
		}
	return std::nullopt;
}

hammer::scene::Entity Prop( const char *model, Vec3d origin, const char *angles )
{
	hammer::scene::Entity e;
	e.classname = "prop_static";
	e.SetKey( "model", model );
	e.SetOrigin( origin );
	e.SetKey( "angles", angles );
	return e;
}

std::optional<ViewPixels> Frame( ViewportRenderer &renderer,
    const hammer::viewport::RenderSnapshot &scene, std::uint64_t key, const ViewRequest &request )
{
	if ( !renderer.SetScene( scene, key ) )
		return std::nullopt;
	auto frame = renderer.RenderAndWait( request );
	if ( !frame )
		return std::nullopt;
	return std::move( frame ).Value();
}

} // namespace

int main()
{
	testing::Checks checks;
	namespace vulkan = render::device::vulkan;
	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<render::device::IRenderDevice2> device = std::move( created ).Value();

		// The document: a prop (yaw 90), a prop whose model is missing and a
		// func_instance (yaw 90) of a box with a prop on it.
		hammer::formats::VmfMapCodec codec;
		hammertest::InMemoryFileStore store;
		{
			hammer::scene::MapDocument room( 5 );
			hammer::scene::DocumentEdit edit( room );
			hammer::scene::FaceTexture texture;
			texture.material = "DEV/DEV_MEASUREGENERIC01B";
			edit.Add(
			    hammer::scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 32 ) }, texture ) );
			edit.Add( Prop( kCratePath, Vec3d( 32, 32, 64 ), "0 0 0" ) );
			hammer::scene::CommitEdit( room, edit );
			store.files["/maps/instances/room.vmf"] = codec.Encode( room ).Value();
		}
		hammer::app::InstancePreview preview( codec, store );
		preview.SetDocumentPath( "/maps/test.vmf" );
		hammer::scene::MapDocument doc( 1 );
		ObjectId prop;
		{
			hammer::scene::DocumentEdit edit( doc );
			prop = edit.Add( Prop( "Models\\Test\\Crate.MDL", Vec3d( 0, 0, 100 ), "0 90 0" ) );
			edit.Add( Prop( "models/test/missing.mdl", Vec3d( -256, 0, 0 ), "0 0 0" ) );
			hammer::scene::Entity instance;
			instance.classname = "func_instance";
			instance.SetKey( "file", "instances/room.vmf" );
			instance.SetOrigin( Vec3d( 512, 0, 0 ) );
			instance.SetKey( "angles", "0 90 0" );
			edit.Add( instance );
			hammer::scene::CommitEdit( doc, edit );
		}
		hammer::viewport::ExtractOptions extract;
		extract.instances = &preview;
		const hammer::viewport::RenderSnapshot scene =
		    hammer::viewport::Extract( doc, {}, extract );
		checks.That( scene.instances.size() == 1 && scene.instances[0].solids.size() == 1 &&
		                 scene.instances[0].entities.size() == 1,
		    "setup.the-instance-is-extracted" );

		RedCrate textures;
		FakeModels models;
		auto made = ViewportRenderer::Create( *device, &textures, &models );
		if ( !checks.That( made.HasValue(), "setup.renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();

		hammer::viewport::Camera3D eye;
		eye.SetViewport( 512, 384 );
		eye.SetPosition( Vec3d( 256, -700, 900 ) );
		eye.LookAt( Vec3d( 256, 0, 0 ) );
		ViewRequest request3D;
		request3D.kind = ViewKind::Camera3D;
		request3D.camera3D = &eye;
		request3D.pixelWidth = 512;
		request3D.pixelHeight = 384;

		const auto empty = Frame( renderer, hammer::viewport::RenderSnapshot(), 1, request3D );
		const auto frame = Frame( renderer, scene, 2, request3D );
		checks.That( empty && frame, "setup.the-views-render" );
		const hammer::render_adapter::SceneStats stats = renderer.Scene();
		if ( empty && frame )
		{
			const std::vector<ModelBatch> plain = BuildModelBatches( Crate(), 0,
			    []( const std::string & )
			    {
				    return std::optional<TextureSize>( TextureSize{ 64, 64 } );
			    },
			    { 255, 255, 255 }, { 200, 200, 200 } );
			const std::optional<Rgb> shade = TopColor( plain );
			const auto rotated = eye.WorldToScreen( Vec3d( 0, 26, 108 ) );
			const auto unrotated = eye.WorldToScreen( Vec3d( 26, 0, 108 ) );
			checks.That(
			    shade && rotated &&
			        Within( At( *frame, rotated->x, rotated->y ), Modulated( kRed, *shade ), 2 ),
			    "M1.the-prop-top-shows-the-texel-times-its-shading-where-it-is-rotated-to" );
			checks.That( unrotated && At( *frame, unrotated->x, unrotated->y ) ==
			                              At( *empty, unrotated->x, unrotated->y ),
			    "M1.the-unrotated-footprint-shows-the-background" );

			// M2.
			const auto marker = eye.WorldToScreen( Vec3d( -256, 0, 8 ) );
			checks.That( marker && !( At( *frame, marker->x, marker->y ) ==
			                           At( *empty, marker->x, marker->y ) ),
			    "M2.a-missing-model-draws-its-marker" );
			checks.That( stats.missingModels == 1 && stats.modelEntities == 2,
			    "M2.one-missing-model-counted-two-drawn" );
			checks.That( models.reads[kCratePath] == 1 && stats.models == 2,
			    "M2.each-model-file-is-read-once" );

			// M3.
			const hammer::viewport::InstanceDraw &instance = scene.instances[0];
			hammer::viewport::RenderSnapshot content;
			content.solids = instance.solids;
			GeometryOptions tinted;
			tinted.tint = hammer::viewport::kInstanceTint;
			const std::optional<Rgb> tintedTop =
			    TopFaceColor( BuildSceneGeometry( content, tinted ), 32 );
			const std::optional<Rgb> plainTop = TopFaceColor( BuildSceneGeometry( content ), 32 );
			// Local (x, y) -> (-y, x), then + (512, 0, 0): the box covers x
			// 448..512, y 0..64.
			const auto box = eye.WorldToScreen( Vec3d( 470, 4, 32 ) );
			checks.That( box && tintedTop && Within( At( *frame, box->x, box->y ), *tintedTop, 2 ),
			    "M3.the-instance-box-shows-its-tinted-fill-where-it-is-placed" );
			checks.That( tintedTop && plainTop && std::abs( tintedTop->b - plainTop->b ) > 10,
			    "M3.the-tint-changes-the-fill" );
			const std::vector<ModelBatch> tintedModel = BuildModelBatches( Crate(), 0,
			    []( const std::string & )
			    {
				    return std::optional<TextureSize>( TextureSize{ 64, 64 } );
			    },
			    hammer::viewport::kInstanceTint, { 200, 200, 200 } );
			const std::optional<Rgb> tintedShade = TopColor( tintedModel );
			// The instance's prop: local (32, 32, 64) -> (480, 32, 64), top at 72.
			const auto inner = eye.WorldToScreen( Vec3d( 480, 32, 72 ) );
			checks.That(
			    inner && tintedShade &&
			        Within( At( *frame, inner->x, inner->y ), Modulated( kRed, *tintedShade ), 2 ),
			    "M3.the-instance-prop-shows-the-texel-times-the-tinted-shading" );
			checks.That( stats.instanceChunks == 1, "M3.one-instance-chunk" );
		}

		// M4.
		hammer::viewport::Camera2D top;
		top.SetKind( ViewKind::Top );
		top.SetViewport( 512, 384 );
		top.SetZoom( 0.5 );
		top.SetCenter( { 128, 0 } );
		ViewRequest request2D;
		request2D.kind = ViewKind::Top;
		request2D.camera2D = &top;
		request2D.pixelWidth = 512;
		request2D.pixelHeight = 384;
		const auto frame2D = Frame( renderer, scene, 2, request2D );
		checks.That( frame2D.has_value(), "M4.the-top-view-renders" );
		if ( frame2D )
		{
			const Rgb entity{ 220, 30, 220 };
			const auto rotatedEdge = top.WorldToScreen( Vec3d( 16, 0, 0 ) );
			const auto unrotatedEdge = top.WorldToScreen( Vec3d( 32, 0, 0 ) );
			checks.That( Near( *frame2D, rotatedEdge.x, rotatedEdge.y, entity ),
			    "M4.the-rotated-model-box-is-drawn-in-the-entity-color" );
			checks.That( !Near( *frame2D, unrotatedEdge.x, unrotatedEdge.y, entity ),
			    "M4.the-unrotated-box-is-not" );
			const auto instanceEdge = top.WorldToScreen( Vec3d( 448, 32, 0 ) );
			checks.That( Near( *frame2D, instanceEdge.x, instanceEdge.y, { 128, 128, 0 } ),
			    "M4.instance-edges-are-drawn-in-the-instance-edge-color" );
		}

		// M5.
		const hammer::viewport::RenderSnapshot selected = hammer::viewport::Extract(
		    doc, hammer::viewport::SelectionInput{ { prop }, {} }, extract );
		const auto frameSelected = Frame( renderer, selected, 3, request3D );
		if ( checks.That( frameSelected.has_value(), "M5.the-selected-view-renders" ) )
		{
			hammer::viewport::EntityDraw chosen;
			for ( const auto &entity : selected.entities )
				if ( entity.id == prop )
					chosen = entity;
			const std::vector<ModelBatch> tintedModel = BuildModelBatches( Crate(), 0,
			    []( const std::string & )
			    {
				    return std::optional<TextureSize>( TextureSize{ 64, 64 } );
			    },
			    hammer::render_adapter::ModelTint( chosen, false ), { 200, 200, 200 } );
			const std::optional<Rgb> shade = TopColor( tintedModel );
			const auto rotated = eye.WorldToScreen( Vec3d( 0, 26, 108 ) );
			checks.That( chosen.selected && shade && rotated &&
			                 Within( At( *frameSelected, rotated->x, rotated->y ),
			                     Modulated( kRed, *shade ), 2 ),
			    "M5.a-selected-model-shows-the-selection-tint" );
		}
		made.Value().reset();
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
