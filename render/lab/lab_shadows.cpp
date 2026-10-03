//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shadows (RFC 0016 K11); see lab_shadows.h.
//
//=============================================================================//

#include "lab_shadows.h"

#include "render/graph/graph_builder.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"

namespace render::lab
{

namespace
{

using namespace render::device;
namespace shadows = render::pass::shadows;

constexpr std::uint32_t kAtlasSize = 8192;
constexpr std::uint32_t kGuard = 4;

} // namespace

std::optional<std::string> DrawShadows( IRenderDevice2 &device,
    shadows::ShadowDepthRenderer &renderer, const LabLights &lights,
    std::span<const shadows::ShadowCaster> casters, const LabShadowCamera &camera, LabShadows &out )
{
	out = LabShadows();
	shadows::ShadowPlanInput input;
	input.lights = lights.lights;
	input.areas = lights.areas;
	if ( lights.sun )
		input.toSun = lights.sun->toSun;
	input.projectors = lights.projectors;
	input.camera = camera;
	input.atlasSize = kAtlasSize;
	input.guardTexels = kGuard;
	shadows::ShadowPlan plan;
	if ( std::optional<std::string> why = shadows::PlanShadows( input, plan ) )
		return why;
	out.tiles = std::move( plan.tiles );
	out.lightTiles = std::move( plan.lightTiles );
	out.lightLayouts = std::move( plan.lightLayouts );
	out.areaTiles = std::move( plan.areaTiles );
	out.projectorTiles = std::move( plan.projectorTiles );
	out.sunFirst = plan.sunFirst;
	out.sunCount = plan.sunCount;
	if ( plan.views.empty() )
		return std::nullopt;
	std::vector<shadows::ShadowDepthView> views;
	for ( const shadows::ShadowPlanView &view : plan.views )
		views.push_back( { view.viewProjection, view.tile, casters } );

	out.atlasDesc.format = Format::kD32Float;
	out.atlasDesc.width = out.atlasDesc.height = kAtlasSize;
	out.atlasDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
	auto atlas = device.CreateTexture( out.atlasDesc );
	if ( !atlas )
		return std::string( "the shadow atlas was refused" );
	out.atlas = atlas.Value();
	graph::GraphBuilder builder;
	const graph::ResourceRef atlasRef = builder.ImportTexture(
	    "atlas", out.atlas, out.atlasDesc, ResourceUsage::kUndefined, ResourceUsage::kSampled );
	auto stats = renderer.AddPasses( builder, { atlasRef, kAtlasSize, kGuard }, views );
	if ( !stats )
		return std::string( "the shadow depth passes were refused" );
	out.views = stats.Value().views;
	out.draws = stats.Value().draws;
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
		return std::string( "the shadow graph did not compile" );
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), device );
	if ( !executed )
		return std::string( "the shadow graph did not execute" );
	(void)device.WaitIdle();
	renderer.Collect( executed.Value().token );
	return std::nullopt;
}

void ReleaseShadows( IRenderDevice2 &device, LabShadows &shadows, CompletionToken token )
{
	if ( shadows.atlas.IsValid() )
		(void)device.Release( shadows.atlas, token );
	shadows.atlas = TextureId();
}

} // namespace render::lab
