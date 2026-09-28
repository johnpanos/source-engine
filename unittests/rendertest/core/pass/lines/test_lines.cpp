//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lines.null (RFC 0016, headless): render.pass.lines on
//			render.device.null, which validates every transition and binding
//			and records the executed commands, so the command stream is
//			checked exactly; pixels are the Vulkan suite's job.
//
//			L1 an upload, a render and the readback pass run; the list's
//			   batches draw in the documented order (resident batches first,
//			   then world tested filled/lines, world untested filled/lines,
//			   screen lines, screen filled), one draw each, with the
//			   pipeline's 80-byte draw constants written before every draw;
//			L2 pipelines are made once per topology and depth mode and reused
//			   across frames;
//			L3 invalid targets, depth-tested items without a depth format and
//			   a batch mesh in another layout are refused before any pass is
//			   added;
//			L4 an empty list still clears the target (a render pass, no draw).
//
//=============================================================================//

#include "lines_fixtures.h"
#include "render/device/null/provider.h"
#include "testing/checks.h"

#include <vector>

namespace
{

using namespace rendertest::lines;

// The draws the null device ran, as vertex (or index) counts in order, and
// whether every draw followed a draw-constant write of 80 bytes.
struct DrawTrace
{
	std::vector<std::uint64_t> counts;
	bool constantsBeforeEveryDraw = true;
	std::uint32_t renderPasses = 0;
};

DrawTrace Trace( device::IRenderDevice2 &device )
{
	DrawTrace trace;
	bool constants = false;
	for ( const device::null::RecordedCommand &command :
	    device::null::Control( device )->Recorded() )
	{
		switch ( command.op )
		{
		case device::null::RecordedOp::kBeginRendering:
			++trace.renderPasses;
			break;
		case device::null::RecordedOp::kSetPipeline:
			constants = false;
			break;
		case device::null::RecordedOp::kSetDrawConstants:
			constants = command.count == 80;
			break;
		case device::null::RecordedOp::kDraw:
		case device::null::RecordedOp::kDrawIndexed:
			trace.counts.push_back( command.count );
			trace.constantsBeforeEveryDraw = trace.constantsBeforeEveryDraw && constants;
			break;
		default:
			break;
		}
	}
	return trace;
}

} // namespace

int main()
{
	testing::Checks checks;
	auto device = device::null::Create( {} ).Value();
	auto renderer =
	    LinesRenderer::Create( *device, device::Format::kRGBA8Unorm, device::Format::kD32Float );
	if ( !checks.That( renderer.HasValue(), "setup.renderer" ) )
		return checks.Report();
	resources::MeshCache cache( *device );
	std::vector<render::pass::lines::LineVertex> meshVertices( 6 );
	auto mesh = StageMesh( *device, cache, "edges", meshVertices );
	if ( !checks.That( mesh.has_value(), "setup.resident-mesh" ) )
		return checks.Report();

	// One item in each slot, added out of draw order, with distinct counts.
	LineList list;
	const Rgba8 white;
	list.Quad( { Space::kScreen, false }, 1, 1, 4, 4, white );               // screen filled: 6
	list.Line( { Space::kScreen, false }, { 0, 0, 0 }, { 5, 5, 0 }, white ); // screen lines: 2
	list.Box( { Space::kWorld, false }, { -0.5f, -0.5f, 0 }, { 0.5f, 0.5f, 0 }, white ); // 24
	list.Disc( { Space::kWorld, false }, 0, 0, 0.5f, white, 3 );                  // world filled: 9
	list.Line( { Space::kWorld, true }, { -1, 0, 0.5f }, { 1, 0, 0.5f }, white ); // tested lines: 2
	const math::float3 a{ -1, -1, 0.5f }, b{ 1, -1, 0.5f }, c{ 0, 1, 0.5f };
	list.Triangle( { Space::kWorld, true }, a, b, c, white, white, white ); // tested filled: 3
	const MeshBatch batches[] = { { *mesh, Topology::kLines, { Space::kWorld, true } } };

	const std::size_t baseline = device->LiveResourceCount();
	device::null::Control( *device )->ClearRecorded();
	const FrameResult first = Draw( *device, *renderer.Value(), list, batches );
	const DrawTrace trace = Trace( *device );
	checks.That( first.ok && first.passes == 3, "L1.upload-render-and-readback-passes-run" );
	checks.Equal( trace.counts, std::vector<std::uint64_t>{ 6, 3, 2, 9, 24, 2, 6 },
	    "L1.batches-draw-in-the-documented-order" );
	checks.That( trace.constantsBeforeEveryDraw, "L1.draw-constants-before-every-draw" );
	checks.That(
	    first.stats.draws == 7 && first.stats.meshBatches == 1 && first.stats.listVertices == 46,
	    "L1.stats-count-draws-batches-and-vertices" );
	const std::size_t afterFirst = device->LiveResourceCount();
	checks.Equal( afterFirst, baseline + 4, "L2.one-pipeline-per-topology-and-depth-mode" );
	const FrameResult second = Draw( *device, *renderer.Value(), list, batches );
	checks.That(
	    second.ok && device->LiveResourceCount() == afterFirst, "L2.frames-reuse-the-pipelines" );

	// L3: refusals before any pass.
	{
		graph::GraphBuilder builder;
		LinesTargets invalid;
		auto refused = renderer.Value()->AddPasses( builder, list, {}, BoxView(), invalid );
		checks.That(
		    !refused && refused.Error() == LinesStatus::kInvalidTargets && builder.Passes().empty(),
		    "L3.invalid-targets-are-refused-before-any-pass" );
	}
	{
		auto flat =
		    LinesRenderer::Create( *device, device::Format::kRGBA8Unorm, device::Format::kUnknown );
		LineList tested;
		tested.Line( { Space::kWorld, true }, { 0, 0, 0 }, { 1, 1, 0 }, white );
		const FrameResult refused = Draw( *device, *flat.Value(), tested, {}, BoxView(), false );
		checks.That( refused.refused == LinesStatus::kNeedsDepth,
		    "L3.depth-tested-items-need-a-depth-format" );
		LineList untested;
		untested.Line( { Space::kWorld, false }, { 0, 0, 0 }, { 1, 1, 0 }, white );
		const FrameResult drawn = Draw( *device, *flat.Value(), untested, {}, BoxView(), false );
		checks.That( drawn.ok, "L3.a-renderer-without-depth-draws-untested-items" );
	}
	{
		resources::MeshEntry wrong = *mesh;
		wrong.vertexStride = 12;
		const MeshBatch bad[] = { { wrong, Topology::kLines, { Space::kWorld, false } } };
		const FrameResult refused = Draw( *device, *renderer.Value(), LineList(), bad );
		checks.That(
		    refused.refused == LinesStatus::kBadMesh, "L3.a-mesh-in-another-layout-is-refused" );
	}

	// L4: an empty list clears.
	device::null::Control( *device )->ClearRecorded();
	const FrameResult empty = Draw( *device, *renderer.Value(), LineList() );
	const DrawTrace emptyTrace = Trace( *device );
	checks.That( empty.ok && emptyTrace.renderPasses == 1 && emptyTrace.counts.empty(),
	    "L4.an-empty-list-clears-without-drawing" );
	return checks.Report();
}
