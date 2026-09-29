//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.viewport.null (RFC 0002
//			hammer.adapters.render; RFC 0016 "Editor viewports"; contract
//			render_adapter.viewport-geometry.v1): the editor's ViewportRenderer
//			on render.device.null, which validates every transition and
//			records the executed commands.
//
//			V1 the scene is staged once per key: the same key restages
//			   nothing, a new key restages;
//			V2 a 2D view with a grid runs two render passes (the grid clears,
//			   the scene loads) and draws the edges and the overlay; the 3D
//			   view runs two (the solids through render.pass.opaque and the
//			   preview program, ResolvePreview, then the edges) and resolves
//			   every batch;
//			V3 nothing blocks: Take returns nothing while the device has not
//			   completed the frame and the pixels (width x height RGBA) once
//			   it has;
//			V4 a view without its camera, a zero size and an unknown ticket
//			   are refused;
//			V5 the renderer releases everything it made: after it is gone the
//			   device holds what it held before;
//			V6 with a material source, a material with a texture gets its own
//			   batch and program (the source is asked once per material, not
//			   per restage), the camera view draws the untextured batch and
//			   the textured one, and a material without a texture stays
//			   untextured;
//			V7 the null device exports no images: a renderer cannot export,
//			   and an external frame is refused with kUnsupported;
//			V8 a $translucent material's batch is blended and drawn in a
//			   second pass after every opaque batch, $alphatest stays in
//			   the opaque pass; its texture is staged with its full mip
//			   chain;
//			V9 restaging is per chunk (id over 64): moving one solid
//			   uploads exactly its chunk's face and edge meshes (the bytes
//			   written equal that chunk's geometry, restated here, less
//			   than the scene's), the same content under a new key uploads
//			   nothing, and a chunk that empties is released;
//			V11 a material setting a variable the preview does not read
//			   ($envmap) still draws textured and is counted
//			   (approximatedMaterials, ignored["$envmap"]), and a material the
//			   source cannot import is a named failure drawn untextured.
//
//=============================================================================//

#include "hammer/adapters/render/scene_geometry.h"
#include "hammer/adapters/render/viewport_renderer.h"
#include "render/device/null/provider.h"
#include "testing/checks.h"
#include "viewport_fixture.h"

#include <algorithm>
#include <vector>

namespace
{

using namespace hammertest::viewport_render;
using hammer::render_adapter::IMaterialTextures;
using hammer::render_adapter::MaterialImage;
using hammer::render_adapter::SourceMaterial;
using hammer::render_adapter::ViewportRenderer;
using hammer::render_adapter::ViewportStatus;
using hammer::render_adapter::ViewRequest;
using hammer::viewport::ViewKind;
namespace nulldev = render::device::null;

struct Trace
{
	std::uint32_t passes = 0;
	std::vector<std::uint64_t> draws;
};

Trace Recorded( render::device::IRenderDevice2 &device )
{
	Trace trace;
	for ( const nulldev::RecordedCommand &command : nulldev::Control( device )->Recorded() )
	{
		if ( command.op == nulldev::RecordedOp::kBeginRendering )
			++trace.passes;
		if ( command.op == nulldev::RecordedOp::kDraw ||
		     command.op == nulldev::RecordedOp::kDrawIndexed )
			trace.draws.push_back( command.count );
	}
	return trace;
}

// A 4x4 gray texture for DEV/DEV_MEASUREGENERIC01B, GLASS ($translucent) and
// GRATE ($alphatest), each named by its own material; nothing for other
// names. UNMAPPED sets a variable the preview does not read ($envmap).
class FakeTextures final : public IMaterialTextures
{
public:
	foundation::Expected<SourceMaterial, std::string> Material(
	    const std::string &material ) override
	{
		asked.push_back( material );
		if ( material != "DEV/DEV_MEASUREGENERIC01B" && material != "GLASS" &&
		     material != "GRATE" && material != "UNMAPPED" )
			return foundation::MakeUnexpected( material + " is missing" );
		MaterialImage image;
		image.width = image.height = 4;
		image.rgba.assign( 4 * 4 * 4, 128 );
		std::vector<render::material::VmtPair> variables = { { "$basetexture", material } };
		if ( material == "GLASS" )
			variables.push_back( { "$translucent", "1" } );
		if ( material == "GRATE" )
			variables.push_back( { "$alphatest", "1" } );
		if ( material == "UNMAPPED" )
			variables.push_back( { "$envmap", "env_cubemap" } );
		return hammer::render_adapter::SourceMaterialFromVariables( "LightmappedGeneric",
		    std::move( variables ),
		    { { render::material::VmtTextureReference( material ), std::move( image ) } } );
	}
	std::vector<std::string> asked;
};

// A one-face solid (a 128 x 128 quad at height z) of 'material'.
hammer::viewport::SolidDraw Pane( std::uint64_t id, const std::string &material, double z )
{
	hammer::viewport::SolidDraw solid;
	solid.id = ObjectId{ id };
	hammer::viewport::FaceDraw face;
	face.material = material;
	face.vertices = {
	    Vec3d( 64, -64, z ), Vec3d( 192, -64, z ), Vec3d( 192, 64, z ), Vec3d( 64, 64, z ) };
	face.normal = Vec3d( 0, 0, 1 );
	face.uAxis.axis = Vec3d( 1, 0, 0 );
	face.vAxis.axis = Vec3d( 0, -1, 0 );
	solid.faces.push_back( face );
	solid.bounds = { Vec3d( 64, -64, z ), Vec3d( 192, 64, z ) };
	return solid;
}

// The bytes of the face and edge meshes of the objects of 'chunk' (id over
// 64, restated), as the renderer stages them.
std::uint64_t ChunkBytes( const hammer::viewport::RenderSnapshot &snapshot, std::uint64_t chunk )
{
	hammer::viewport::RenderSnapshot part;
	for ( const auto &solid : snapshot.solids )
		if ( solid.id.value / 64 == chunk )
			part.solids.push_back( solid );
	for ( const auto &entity : snapshot.entities )
		if ( entity.id.value / 64 == chunk )
			part.entities.push_back( entity );
	const hammer::render_adapter::SceneGeometry g =
	    hammer::render_adapter::BuildSceneGeometry( part );
	std::uint64_t bytes = g.edges.size() * sizeof( hammer::render_adapter::LineVertex );
	for ( const auto &batch : g.faces )
		bytes += batch.vertices.size() * sizeof( hammer::render_adapter::PreviewVertex );
	return bytes;
}

struct Writes
{
	std::size_t count = 0;
	std::uint64_t bytes = 0;
};

Writes BufferWrites( render::device::IRenderDevice2 &device )
{
	Writes writes;
	for ( const nulldev::RecordedCommand &command : nulldev::Control( device )->Recorded() )
	{
		if ( command.op == nulldev::RecordedOp::kWriteBuffer )
		{
			++writes.count;
			writes.bytes += command.count;
		}
	}
	return writes;
}

} // namespace

int main()
{
	testing::Checks checks;
	nulldev::NullOptions options;
	options.completion = nulldev::CompletionMode::kManual;
	auto device = nulldev::Create( options ).Value();
	nulldev::INullDeviceControl *control = nulldev::Control( *device );
	const std::size_t baseline = device->LiveResourceCount();
	Document d;
	Build( d );
	{
		auto made = ViewportRenderer::Create( *device );
		if ( !checks.That( made.HasValue(), "setup.renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();

		// V1.
		checks.That( renderer.SetScene( Snapshot( d ), 1 ).HasValue() &&
		                 renderer.Scene().stagings == 1 && renderer.Scene().triangles == 24,
		    "V1.the-scene-is-staged" );
		checks.That( renderer.SetScene( Snapshot( d ), 1 ).HasValue() &&
		                 renderer.Scene().stagings == 1,
		    "V1.the-same-key-restages-nothing" );
		checks.That( renderer.SetScene( Snapshot( d, false ), 2 ).HasValue() &&
		                 renderer.Scene().stagings == 2,
		    "V1.a-new-key-restages" );
		control->CompleteAll();

		// V2, V3.
		const hammer::viewport::Camera2D top = TopCamera();
		ViewRequest request2D;
		request2D.kind = ViewKind::Top;
		request2D.camera2D = &top;
		request2D.grid = { { hammer::viewport::GridLineKind::Minor,
		    hammer::viewport::GridLineOrientation::Vertical, 0.0, 10.0 } };
		request2D.overlay.Box(
		    Vec3d( 0, 0, 0 ), Vec3d( 16, 16, 16 ), hammer::tools::OverlayRole::Pending );
		request2D.pixelWidth = 256;
		request2D.pixelHeight = 192;
		control->ClearRecorded();
		auto ticket = renderer.Render( request2D );
		checks.That( ticket.HasValue(), "V2.the-2d-view-renders" );
		if ( ticket )
		{
			auto early = renderer.Take( ticket.Value() );
			checks.That( early.HasValue() && !early.Value(), "V3.nothing-before-completion" );
			control->CompleteAll();
			const Trace trace = Recorded( *device );
			checks.Equal( trace.passes, 2u, "V2.the-grid-and-the-scene-are-two-passes" );
			checks.Equal( trace.draws,
			    std::vector<std::uint64_t>{ 2, renderer.Scene().edgeVertices, 24 },
			    "V2.the-2d-view-draws-grid-edges-and-overlay" );
			auto taken = renderer.Take( ticket.Value() );
			checks.That( taken.HasValue() && taken.Value() && taken.Value()->width == 256 &&
			                 taken.Value()->rgba.size() == std::size_t( 256 * 192 * 4 ),
			    "V3.the-pixels-arrive-after-completion" );
		}
		const hammer::viewport::Camera3D eye = EyeCamera();
		ViewRequest request3D;
		request3D.kind = ViewKind::Camera3D;
		request3D.camera3D = &eye;
		request3D.pixelWidth = 128;
		request3D.pixelHeight = 96;
		control->ClearRecorded();
		auto ticket3D = renderer.Render( request3D );
		control->CompleteAll();
		const Trace trace3D = Recorded( *device );
		checks.That( ticket3D.HasValue() && trace3D.passes == 2,
		    "V2.the-3d-view-is-the-opaque-pass-then-the-edges" );
		checks.That( renderer.LastView().drawn == 1 && renderer.LastView().unresolved == 0,
		    "V2.the-untextured-batch-resolves" );
		checks.Equal( trace3D.draws,
		    std::vector<std::uint64_t>{ renderer.Scene().faceVertices, renderer.Scene().edgeVertices },
		    "V2.the-3d-view-draws-faces-then-edges" );
		if ( ticket3D )
		{
			auto taken = renderer.Take( ticket3D.Value() );
			checks.That( taken.HasValue() && taken.Value(), "V3.the-3d-pixels-arrive" );
		}

		// V4.
		ViewRequest noCamera;
		noCamera.kind = ViewKind::Front;
		noCamera.pixelWidth = noCamera.pixelHeight = 8;
		auto refused = renderer.Render( noCamera );
		checks.That( !refused && refused.Error() == ViewportStatus::kInvalidView,
		    "V4.a-view-without-its-camera-is-refused" );
		ViewRequest empty = request3D;
		empty.pixelWidth = 0;
		auto zero = renderer.Render( empty );
		checks.That( !zero && zero.Error() == ViewportStatus::kInvalidView,
		    "V4.a-zero-size-is-refused" );
		auto unknown = renderer.Take( 999 );
		checks.That( !unknown && unknown.Error() == ViewportStatus::kUnknownTicket,
		    "V4.an-unknown-ticket-is-refused" );

		// A pending frame at destruction (V5).
		auto left = renderer.Render( request2D );
		checks.That( left.HasValue() && renderer.PendingCount() == 1, "V5.a-frame-is-pending" );
		control->CompleteAll();
	}
	(void)device->Poll();
	control->CompleteAll();
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline, "V5.everything-is-released" );

	// V6.
	{
		FakeTextures textures;
		auto made = ViewportRenderer::Create( *device, &textures );
		if ( !checks.That( made.HasValue(), "V6.a-textured-renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();
		hammer::scene::FaceTexture other;
		other.material = "DEV/MISSING";
		{
			hammer::scene::DocumentEdit edit( d.doc );
			(void)edit.Add( hammer::scene::MakeBoxSolid(
			    hammer::scene::Box{ Vec3d( 256, 0, 0 ), Vec3d( 320, 64, 64 ) }, other ) );
			hammer::scene::CommitEdit( d.doc, edit );
		}
		checks.That( renderer.SetScene( Snapshot( d ), 1 ).HasValue() &&
		                 renderer.SetScene( Snapshot( d, false ), 2 ).HasValue(),
		    "V6.staged-twice" );
		checks.Equal(
		    textures.asked.size(), std::size_t( 2 ), "V6.the-source-is-asked-once-per-material" );
		checks.That( renderer.Scene().textures == 1 && renderer.Scene().missingTextures == 1 &&
		                 renderer.Scene().batches == 2 && renderer.Scene().texturedBatches == 1,
		    "V6.one-textured-batch-and-a-missing-texture-stays-untextured" );
		control->CompleteAll();
		const hammer::viewport::Camera3D eye = EyeCamera();
		ViewRequest request;
		request.kind = ViewKind::Camera3D;
		request.camera3D = &eye;
		request.pixelWidth = 64;
		request.pixelHeight = 48;
		control->ClearRecorded();
		auto ticket = renderer.Render( request );
		control->CompleteAll();
		const Trace trace = Recorded( *device );
		checks.That( ticket.HasValue() && renderer.LastView().drawn == 2 &&
		                 renderer.LastView().unresolved == 0 && trace.draws.size() == 3 &&
		                 trace.draws[0] + trace.draws[1] == renderer.Scene().faceVertices,
		    "V6.the-camera-view-draws-both-batches-then-the-edges" );
		if ( ticket )
			(void)renderer.Take( ticket.Value() );
	}
	(void)device->Poll();
	control->CompleteAll();
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline, "V6.everything-is-released" );

	// V7.
	{
		auto made = ViewportRenderer::Create( *device );
		const hammer::viewport::Camera3D eye = EyeCamera();
		ViewRequest request;
		request.kind = ViewKind::Camera3D;
		request.camera3D = &eye;
		request.pixelWidth = 32;
		request.pixelHeight = 24;
		request.external = true;
		auto refused = made ? made.Value()->Render( request )
		                    : foundation::Expected<ViewportRenderer::Ticket, ViewportStatus>(
		                          foundation::MakeUnexpected( ViewportStatus::kDevice ) );
		checks.That( made && !made.Value()->CanExport() && !refused &&
		                 refused.Error() == ViewportStatus::kUnsupported,
		    "V7.an-external-frame-without-an-exporter-is-refused" );
	}

	// V8.
	{
		FakeTextures textures;
		auto made = ViewportRenderer::Create( *device, &textures );
		if ( !checks.That( made.HasValue(), "V8.a-textured-renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();
		hammer::viewport::RenderSnapshot scene = Snapshot( d, false );
		scene.solids.push_back( Pane( 500, "GLASS", 68 ) );
		scene.solids.push_back( Pane( 501, "GRATE", 80 ) );
		control->ClearRecorded();
		checks.That( renderer.SetScene( scene, 1 ).HasValue(), "V8.staged" );
		control->CompleteAll();
		std::size_t copies = 0;
		for ( const nulldev::RecordedCommand &command : control->Recorded() )
			copies += command.op == nulldev::RecordedOp::kCopyBufferToTexture ? 1 : 0;
		// Three 4x4 textures (4x4, 2x2, 1x1), the renderer's 1x1 white, the
		// programs' neutral textures for the terms that are off (a 1x1 2D and
		// a 1x1 cube's six faces; MaterialPrograms makes them) and the frame
		// group's neutral 2D for the surface program's unread split-sum slot
		// (the group residency that holds it makes its own).
		checks.Equal( copies, std::size_t( 3 * 3 + 1 + 1 + 6 + 1 ),
		    "V8.each-texture-uploads-its-three-levels" );
		checks.That( renderer.Scene().blendedBatches == 1 && renderer.Scene().texturedBatches == 3,
		    "V8.the-translucent-batch-alone-is-blended" );
		const hammer::viewport::Camera3D eye = EyeCamera();
		ViewRequest request;
		request.kind = ViewKind::Camera3D;
		request.camera3D = &eye;
		request.pixelWidth = 64;
		request.pixelHeight = 48;
		control->ClearRecorded();
		auto ticket = renderer.Render( request );
		control->CompleteAll();
		const Trace trace = Recorded( *device );
		checks.Equal( trace.passes, 3u, "V8.opaque-then-blended-then-edges" );
		// The opaque pass draws three batches (untextured, DEV and the grate's
		// 6 vertices, in the draw list's order), the blended pass the glass
		// pane (6), then the edges of the two chunks.
		const bool opaqueFirst =
		    trace.draws.size() == 6 &&
		    trace.draws[0] + trace.draws[1] + trace.draws[2] + 6 == renderer.Scene().faceVertices &&
		    std::count( trace.draws.begin(), trace.draws.begin() + 3, 6 ) == 1;
		checks.That( ticket.HasValue() && renderer.LastView().blended == 1 &&
		                 renderer.LastView().drawn == 4 && renderer.LastView().unresolved == 0 &&
		                 opaqueFirst && trace.draws[3] == 6,
		    "V8.the-translucent-batch-draws-after-every-opaque-one" );
		if ( ticket )
			(void)renderer.Take( ticket.Value() );
	}

	// V11.
	{
		FakeTextures textures;
		auto made = ViewportRenderer::Create( *device, &textures );
		if ( !checks.That( made.HasValue(), "V11.a-textured-renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();
		hammer::viewport::RenderSnapshot scene;
		scene.solids.push_back( Pane( 600, "UNMAPPED", 68 ) );
		scene.solids.push_back( Pane( 601, "BROKEN", 80 ) );
		checks.That( renderer.SetScene( scene, 1 ).HasValue(), "V11.staged" );
		const hammer::render_adapter::SceneStats &stats = renderer.Scene();
		auto ignored = stats.ignored.find( "$envmap" );
		checks.That( stats.textures == 1 && stats.texturedBatches == 1 &&
		                 stats.approximatedMaterials == 1 && ignored != stats.ignored.end() &&
		                 ignored->second == 1 && stats.ignored.size() == 1,
		    "V11.an-unmapped-variable-draws-textured-and-is-counted" );
		auto failure = stats.failures.find( "BROKEN" );
		checks.That( stats.failedMaterials == 1 && stats.missingTextures == 1 &&
		                 failure != stats.failures.end() &&
		                 failure->second.find( "missing" ) != std::string::npos,
		    "V11.a-material-that-does-not-import-is-a-named-failure" );
		control->CompleteAll();
		const hammer::viewport::Camera3D eye = EyeCamera();
		ViewRequest request;
		request.kind = ViewKind::Camera3D;
		request.camera3D = &eye;
		request.pixelWidth = 64;
		request.pixelHeight = 48;
		auto ticket = renderer.Render( request );
		control->CompleteAll();
		checks.That( ticket.HasValue() && renderer.LastView().drawn == 2 &&
		                 renderer.LastView().unresolved == 0,
		    "V11.both-draw" );
		if ( ticket )
			(void)renderer.Take( ticket.Value() );
	}

	// V9.
	{
		auto made = ViewportRenderer::Create( *device );
		if ( !checks.That( made.HasValue(), "V9.a-renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();
		const hammer::viewport::RenderSnapshot spread = Spread( d, 70 ); // ids 128 to 197
		checks.That( renderer.SetScene( spread, 1 ).HasValue() && renderer.Scene().chunks == 3 &&
		                 renderer.Scene().stagedChunks == 3,
		    "V9.a-scene-of-three-chunks-stages-each" );
		control->CompleteAll();
		const hammer::viewport::RenderSnapshot moved =
		    Moved( spread, ObjectId{ 130 }, Vec3d( 16, 0, 0 ) );
		control->ClearRecorded();
		checks.That( renderer.SetScene( moved, 2 ).HasValue(), "V9.restaged" );
		control->CompleteAll();
		const Writes writes = BufferWrites( *device );
		const std::uint64_t chunkBytes = ChunkBytes( moved, 2 );
		std::uint64_t sceneBytes = 0;
		for ( std::uint64_t chunk : { d.left.value / 64, std::uint64_t( 2 ), std::uint64_t( 3 ) } )
			sceneBytes += ChunkBytes( moved, chunk );
		checks.That( renderer.Scene().stagedChunks == 1 && renderer.Scene().stagedMeshes == 2 &&
		                 writes.count == 2,
		    "V9.moving-one-solid-uploads-its-chunks-face-and-edge-meshes" );
		checks.That( writes.bytes == chunkBytes && chunkBytes < sceneBytes,
		    "V9.the-bytes-written-are-that-chunks-geometry" );
		const hammer::render_adapter::SceneGeometry whole =
		    hammer::render_adapter::BuildSceneGeometry( moved );
		std::uint32_t faceVertices = 0;
		for ( const auto &batch : whole.faces )
			faceVertices += std::uint32_t( batch.vertices.size() );
		checks.That( renderer.Scene().faceVertices == faceVertices &&
		                 renderer.Scene().edgeVertices == whole.edges.size() &&
		                 renderer.Scene().triangles == whole.triangles,
		    "V9.the-resident-totals-are-the-whole-scenes" );
		control->ClearRecorded();
		checks.That( renderer.SetScene( moved, 3 ).HasValue() &&
		                 renderer.Scene().stagedChunks == 0 && BufferWrites( *device ).count == 0,
		    "V9.the-same-content-under-a-new-key-uploads-nothing" );
		control->CompleteAll();
		(void)device->Poll();
		const std::size_t live = device->LiveResourceCount();
		hammer::viewport::RenderSnapshot fewer = moved;
		std::erase_if( fewer.solids,
		    []( const hammer::viewport::SolidDraw &solid )
		    {
			    return solid.id.value >= 192 && solid.id.value < 256;
		    } );
		checks.That( renderer.SetScene( fewer, 4 ).HasValue() && renderer.Scene().chunks == 2 &&
		                 renderer.Scene().stagedMeshes == 0,
		    "V9.a-chunk-that-empties-is-dropped" );
		control->CompleteAll();
		(void)device->Poll();
		checks.That( device->LiveResourceCount() < live, "V9.its-meshes-are-released" );
	}
	(void)device->Poll();
	control->CompleteAll();
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline, "V9.everything-is-released" );
	return checks.Report();
}
