//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.opaque.null (RFC 0016 K5, K4 families, headless):
//			render.pass.opaque on render.device.null. The null device validates
//			every transition and binding against its own state, so a frame it
//			accepts has its accesses declared (mesh buffers, the programs'
//			uniform buffers and textures); pixels are the Vulkan suite's job.
//
//			- the draw and readback passes run, and frames create no device
//			  objects (pipelines and groups are the family's and
//			  MaterialPrograms');
//			- draws whose mesh or program does not resolve, whose material's
//			  texture is not resident, whose mesh stride is not the program's,
//			  or whose program reads a draw group the instance lacks or has of
//			  another layout, or a view group the frame lacks, are counted, not
//			  drawn;
//			- invalid targets fail with kInvalidTargets before any pass is
//			  added;
//			- scene C's lightmapped draw (its page as a draw group) and pbr draw
//			  (the frame's and view's groups) resolve and draw.
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
	std::unique_ptr<Materials> materials =
	    StageMaterials( *device, device::Format::kRGBA8Unorm, device::Format::kD32Float );
	if ( !checks.That( meshes && materials, "setup.meshes-and-materials" ) )
		return checks.Report();
	checks.Equal( materials->programs.ReadyCount(), std::size_t( 8 ),
	    "setup.eight-programs-ready-and-the-untextured-one-not" );
	(void)device->Poll(); // staging released behind completed tokens goes at the next poll
	const std::size_t baseline = device->LiveResourceCount();

	auto a = SceneA( true );
	const FrameResult first = DrawScene( *device, *materials, *meshes, *a->Snapshot() );
	checks.That( first.ok && first.passes == 2, "N1.draw-and-readback-passes-run" );
	checks.That( first.stats.drawn == 2, "N2.resolved-draws-are-drawn" );
	checks.Equal( first.stats.unresolved, 7u,
	    "N2.missing-mesh-unknown-material-absent-texture-wrong-stride-and-groups-counted" );
	const FrameResult second = DrawScene( *device, *materials, *meshes, *a->Snapshot() );
	checks.That( second.ok, "N1.a-second-frame-runs" );
	checks.Equal( device->LiveResourceCount(), baseline, "N1.frames-create-no-device-objects" );

	graph::GraphBuilder builder;
	const scene::SceneView view = View();
	const scene::DrawList list = scene::BuildDrawList( *a->Snapshot(), view );
	OpaqueTargets invalid;
	auto refused = AddOpaquePasses( builder, *a->Snapshot(), list, view,
	    { *meshes, materials->programs, &materials->drawGroups, nullptr, nullptr, {} }, invalid );
	checks.That(
	    !refused && refused.Error() == OpaqueStatus::kInvalidTargets && builder.Passes().empty(),
	    "N3.invalid-targets-fail-before-adding-passes" );
	auto c = SceneC();
	// The lightmapped family's frame terms are another frame layout.
	const material::DrawGroup *const lightmappedFrame[] = {
	    materials->drawGroups.Group( kLightmappedFrameGroup ) };
	const FrameResult families = DrawScene( *device, *materials, *meshes, *c->Snapshot(),
	    materials->drawGroups.Group( kPbrFrameGroup ), materials->drawGroups.Group( kPbrViewGroup ),
	    lightmappedFrame );
	checks.That( families.ok && families.stats.drawn == 3 && families.stats.unresolved == 0,
	    "N4.lightmapped-and-pbr-draw-with-their-groups" );
	checks.Equal( materials->programs.GroupFailures(), 0u, "N1.groups-without-failure" );
	return checks.Report();
}
