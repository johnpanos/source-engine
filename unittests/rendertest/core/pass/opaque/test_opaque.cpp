//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.opaque.null (RFC 0016 K5, headless): render.pass.opaque on
//			render.device.null. The null device validates every transition and
//			binding against its own state, so a frame it accepts has its
//			accesses declared; pixels are the Vulkan suite's job.
//
//			- the upload and draw passes run, with one pipeline per vertex
//			  stride shared across frames;
//			- unresolved meshes and materials are counted, not drawn;
//			- invalid targets fail with kInvalidTargets before any pass is
//			  added.
//
//=============================================================================//

#include "opaque_fixtures.h"
#include "render/device/null/provider.h"
#include "testing/checks.h"

int main()
{
	using namespace rendertest::opaque;
	testing::Checks checks;
	auto device = device::null::Create( {} ).Value();
	resources::MeshCache cache( *device );
	std::optional<Meshes> meshes = StageCube( *device, cache );
	auto renderer =
	    OpaqueRenderer::Create( *device, device::Format::kRGBA8Unorm, device::Format::kD32Float );
	if ( !checks.That( meshes && renderer, "setup.mesh-and-renderer" ) )
		return checks.Report();
	const std::size_t baseline = device->LiveResourceCount();

	auto a = SceneA( true );
	const FrameResult first = DrawScene( *device, *renderer.Value(), *meshes, *a->Snapshot() );
	checks.That( first.ok && first.passes == 3, "N1.upload-draw-and-readback-passes-run" );
	checks.That(
	    first.stats.drawn == 2 && first.stats.unresolved == 2, "N2.unresolved-draws-are-counted" );
	const std::size_t afterFirst = device->LiveResourceCount();
	const FrameResult second = DrawScene( *device, *renderer.Value(), *meshes, *a->Snapshot() );
	checks.That( second.ok && device->LiveResourceCount() == afterFirst,
	    "N1.frames-reuse-the-pipeline-and-release-their-groups" );
	checks.That( afterFirst == baseline + 1, "N1.one-pipeline-for-one-stride" );

	graph::GraphBuilder builder;
	const scene::SceneView view = View();
	const scene::DrawList list = scene::BuildDrawList( *a->Snapshot(), view );
	OpaqueTargets invalid;
	auto refused = renderer.Value()->AddPasses(
	    builder, *a->Snapshot(), list, view, *meshes, Colors(), invalid );
	checks.That(
	    !refused && refused.Error() == OpaqueStatus::kInvalidTargets && builder.Passes().empty(),
	    "N3.invalid-targets-fail-before-adding-passes" );
	checks.Equal( renderer.Value()->RecordFailures(), 0u, "N1.records-without-failure" );
	return checks.Report();
}
