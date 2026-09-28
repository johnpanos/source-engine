//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.frame.v1 and render.renderer (RFC 0016, first slice) on
//			render.device.null:
//
//			F1 the stage tracker accepts the legacy frame's order, including
//			   nested views, and counts every kind of violation;
//			F2 a frame runs every feature's passes on the device, in feature
//			   order, and reports them;
//			F3 BeginFrame/EndFrame misuse fails without breaking the next frame;
//			F4 stage hooks see each stage with its view depth;
//			F5 a feature that needs a missing capability fails composition,
//			   naming the feature and the capability.
//
//=============================================================================//

#include "render/device/null/provider.h"
#include "render/pass/present/feature.h"
#include "render/renderer/renderer_factory.h"
#include "testing/checks.h"

#include <cstring>
#include <string>
#include <vector>

namespace
{

using namespace render;
using frame::Stage;

class Probe final : public frame::IRenderFeature
{
public:
	Probe( const char *name, std::vector<std::string> &log, device::CapabilitySet required = {} )
	    : m_Name( name ), m_Log( log ), m_Required( required )
	{
	}
	const char *Name() const override { return m_Name; }
	frame::FeatureRequirements Requirements() const override { return { m_Required }; }
	void AddPasses( frame::FeatureContext &context ) override
	{
		std::vector<std::string> *log = &m_Log;
		const char *name = m_Name;
		context.graph.AddPass( name, graph::PassKind::kRender )
		    .Write( context.target, device::ResourceUsage::kColorAttachment )
		    .Execute(
		        [log, name]( graph::RecordContext & )
		        {
			        log->push_back( name );
		        } );
	}

private:
	const char *m_Name;
	std::vector<std::string> &m_Log;
	device::CapabilitySet m_Required;
};

class Hooks final : public frame::IRenderStageHooks
{
public:
	void OnStage( Stage stage, std::uint32_t depth ) override
	{
		seen.push_back( { stage, depth } );
	}
	std::vector<std::pair<Stage, std::uint32_t>> seen;
};

bool Sequence( std::initializer_list<Stage> stages )
{
	frame::StageTracker tracker;
	for ( Stage stage : stages )
	{
		if ( !tracker.Mark( stage ) )
			return false;
	}
	return true;
}

} // namespace

int main()
{
	testing::Checks checks;
	checks.That( Sequence( { Stage::kFrameBegin, Stage::kViewBegin, Stage::kSkybox, Stage::kOpaque,
	                 Stage::kTranslucent, Stage::kViewModel, Stage::kPostProcess, Stage::kViewEnd,
	                 Stage::kHud, Stage::kFrameEnd } ),
	    "F1.the-legacy-order-is-accepted" );
	checks.That(
	    Sequence( { Stage::kFrameBegin, Stage::kViewBegin, Stage::kOpaque, Stage::kViewBegin,
	        Stage::kSkybox, Stage::kOpaque, Stage::kViewEnd, Stage::kTranslucent, Stage::kViewEnd,
	        Stage::kHud, Stage::kViewBegin, Stage::kOpaque, Stage::kViewEnd, Stage::kFrameEnd } ),
	    "F1.nested-views-and-views-after-the-hud-are-accepted" );
	checks.That(
	    !Sequence( { Stage::kFrameBegin, Stage::kViewBegin, Stage::kTranslucent, Stage::kOpaque } ),
	    "F1.a-view-stage-going-backwards-is-a-violation" );
	checks.That( !Sequence( { Stage::kFrameBegin, Stage::kOpaque } ),
	    "F1.a-view-stage-outside-a-view-is-a-violation" );
	checks.That( !Sequence( { Stage::kFrameBegin, Stage::kViewBegin, Stage::kHud } ),
	    "F1.the-hud-inside-a-view-is-a-violation" );
	checks.That(
	    !Sequence( { Stage::kFrameBegin, Stage::kViewEnd } ), "F1.closing-no-view-is-a-violation" );
	checks.That( !Sequence( { Stage::kFrameBegin, Stage::kViewBegin, Stage::kFrameEnd } ),
	    "F1.ending-with-an-open-view-is-a-violation" );

	auto device = device::null::Create( {} ).Value();
	std::vector<std::string> log;
	renderer::RendererDeps deps;
	deps.device = device.get();
	deps.features.push_back( std::make_unique<Probe>( "first", log ) );
	deps.features.push_back( std::make_unique<Probe>( "second", log ) );
	deps.features.push_back( pass::present::CreatePresentFeature( {} ) );
	auto created = renderer::CreateRenderer( std::move( deps ) );
	if ( !checks.That( created.HasValue(), "F2.the-renderer-is-created" ) )
		return checks.Report();
	std::unique_ptr<frame::IRenderer> renderer = std::move( created ).Value();
	Hooks hooks;
	renderer->AddStageHooks( &hooks );

	const std::size_t baseline = device->LiveResourceCount();
	frame::FrameDesc desc;
	desc.width = 64;
	desc.height = 32;
	for ( int frame = 0; frame < 3; ++frame )
	{
		desc.frame = frame;
		checks.That( renderer->BeginFrame( desc ).HasValue(), "F2.a-frame-begins" );
		renderer->MarkStage( Stage::kViewBegin );
		renderer->MarkStage( Stage::kOpaque );
		renderer->MarkStage( Stage::kViewEnd );
		renderer->MarkStage( Stage::kHud );
		auto stats = renderer->EndFrame();
		checks.That( stats && stats.Value().passes == 3 && stats.Value().orderViolations == 0 &&
		                 stats.Value().views == 1 && stats.Value().stagesMarked == 6,
		    "F2.a-frame-runs-each-feature-once-in-order" );
		if ( frame > 0 )
			checks.That( stats && stats.Value().transientsCreated == 0 &&
			                 stats.Value().transientsReused == 1,
			    "F2.later-frames-reuse-the-frame-target" );
	}
	checks.That(
	    log == std::vector<std::string>{ "first", "second", "first", "second", "first", "second" },
	    "F2.features-record-in-order" );
	(void)device->Poll();
	checks.Equal( device->LiveResourceCount(), baseline + 1, "F2.the-pool-keeps-one-frame-target" );
	checks.That(
	    renderer->Totals().frames == 3 && renderer->Totals().passes == 9, "F2.totals-accumulate" );

	checks.That( !renderer->EndFrame(), "F3.ending-without-a-frame-fails" );
	checks.That( !renderer->MarkStage( Stage::kOpaque ), "F3.marking-outside-a-frame-fails" );
	checks.That( renderer->BeginFrame( desc ).HasValue() && !renderer->BeginFrame( desc ),
	    "F3.beginning-twice-fails" );
	renderer->MarkStage( Stage::kOpaque ); // outside a view
	auto violated = renderer->EndFrame();
	checks.That( violated && violated.Value().orderViolations == 1, "F3.violations-are-counted" );
	frame::FrameDesc empty;
	checks.That( !renderer->BeginFrame( empty ), "F3.a-frame-without-a-size-or-target-fails" );
	checks.That( renderer->BeginFrame( desc ).HasValue() && renderer->EndFrame().HasValue(),
	    "F3.the-next-frame-still-runs" );

	hooks.seen.clear();
	(void)renderer->BeginFrame( desc );
	renderer->MarkStage( Stage::kViewBegin );
	renderer->MarkStage( Stage::kViewBegin );
	renderer->MarkStage( Stage::kViewEnd );
	renderer->MarkStage( Stage::kViewEnd );
	(void)renderer->EndFrame();
	const std::vector<std::pair<Stage, std::uint32_t>> expected = { { Stage::kFrameBegin, 0 },
	    { Stage::kViewBegin, 1 }, { Stage::kViewBegin, 2 }, { Stage::kViewEnd, 1 },
	    { Stage::kViewEnd, 0 }, { Stage::kFrameEnd, 0 } };
	checks.That( hooks.seen == expected, "F4.hooks-see-each-stage-with-its-depth" );
	renderer->RemoveStageHooks( &hooks );
	renderer.reset();
	(void)device->Poll();
	checks.Equal(
	    device->LiveResourceCount(), baseline, "F2.destroying-the-renderer-releases-its-pool" );

	device::null::NullOptions noCompute;
	noCompute.capabilities.Remove( device::Capability::kCompute );
	auto limited = device::null::Create( noCompute ).Value();
	renderer::RendererDeps needs;
	needs.device = limited.get();
	needs.features.push_back( std::make_unique<Probe>(
	    "skinning", log, device::CapabilitySet{ device::Capability::kCompute } ) );
	auto refused = renderer::CreateRenderer( std::move( needs ) );
	checks.That( !refused &&
	                 refused.Error().status == renderer::RendererStatus::kMissingCapability &&
	                 std::strcmp( refused.Error().feature, "skinning" ) == 0 &&
	                 refused.Error().capability == device::Capability::kCompute,
	    "F5.a-missing-capability-fails-naming-feature-and-capability" );
	return checks.Report();
}
