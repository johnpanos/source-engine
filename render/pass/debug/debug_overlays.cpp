//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.debug (RFC 0014); see debug_overlays.h.
//
//=============================================================================//

#include "render/pass/debug/debug_overlays.h"

#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <span>

namespace render::pass::debug
{

using namespace render::device;

DebugOverlays::~DebugOverlays()
{
	if ( m_Device )
		ReleaseDevice( *m_Device );
}

void DebugOverlays::ReleaseDevice( IRenderDevice2 &device )
{
	for ( const auto &[key, pipeline] : m_Pipelines )
		(void)device.Release( pipeline, CompletionToken() );
	m_Pipelines.clear();
	m_Device = nullptr;
}

PipelineId DebugOverlays::PipelineFor(
    IRenderDevice2 &device, int program, const HatchTarget &target )
{
	if ( m_Device && m_Device != &device )
		ReleaseDevice( *m_Device );
	m_Device = &device;
	const auto key = std::make_tuple( program, target.format, target.samples, target.encodeOutput );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
		return found->second;
	const bool tint = program == 1;
	// The program in the device's artifact format (RFC 0016 K10).
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { "render/pass/debug/fullscreen.vert",
	    tint ? "render/pass/debug/tint.frag" : "render/pass/debug/hatch.frag" } );
	recipe.topology = PrimitiveTopology::kTriangleList;
	recipe.raster.cull = CullMode::kNone;
	// assign(1, ...): GCC 13 misreports a one-element initializer list (-Warray-bounds).
	recipe.colorFormats.assign( 1, target.format );
	recipe.blends.assign( 1, tint ? BlendMode::kAlpha : BlendMode::kOpaque );
	recipe.sampleCount = target.samples;
	recipe.debugName = tint ? "render.pass.debug.tint" : "render.pass.debug.hatch";
	auto resolved =
	    shaderlib::Resolve( recipe, shaderlib::CoreArtifacts(), device.Facts().artifactFormat );
	if ( !resolved )
		return PipelineId();
	const SpecializationConstant constants[] = {
	    { ShaderStage::kFragment, 0, target.encodeOutput ? 1u : 0u } };
	PipelineDesc desc = resolved.Value().Desc();
	desc.drawConstantBytes = tint ? 16u : 0u;
	desc.constants = constants;
	auto pipeline = device.CreatePipeline( desc );
	if ( !pipeline )
		return PipelineId();
	m_Pipelines.emplace( key, pipeline.Value() );
	return pipeline.Value();
}

void DebugOverlays::RecordFullTarget( CommandEncoder &encoder, const HatchTarget &target,
    PipelineId pipeline, std::span<const std::byte> constants, const char *label )
{
	encoder.BeginLabel( label );
	const ColorAttachment colors[] = {
	    { target.color, LoadOp::kLoad, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.width = target.width;
	rendering.height = target.height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport( { 0, 0, float( target.width ), float( target.height ), 0, 1 } );
	encoder.SetPipeline( pipeline );
	if ( !constants.empty() )
		encoder.SetDrawConstants( 0, constants );
	encoder.Draw( 3, 1, 0, 0 );
	encoder.EndRendering();
	encoder.EndLabel();
}

bool DebugOverlays::RecordHatch(
    IRenderDevice2 &device, CommandEncoder &encoder, const HatchTarget &target )
{
	if ( !target.color.IsValid() || target.width == 0 || target.height == 0 )
		return false;
	const PipelineId pipeline = PipelineFor( device, 0, target );
	if ( !pipeline.IsValid() )
		return false;
	RecordFullTarget( encoder, target, pipeline, {}, "debug hatch (RFC 0014)" );
	++m_Hatches;
	return true;
}

bool DebugOverlays::RecordTint( IRenderDevice2 &device, CommandEncoder &encoder,
    const HatchTarget &target, const float rgba[4] )
{
	if ( !target.color.IsValid() || target.width == 0 || target.height == 0 )
		return false;
	const PipelineId pipeline = PipelineFor( device, 1, target );
	if ( !pipeline.IsValid() )
		return false;
	float color[4];
	std::copy( rgba, rgba + 4, color );
	RecordFullTarget(
	    encoder, target, pipeline, std::as_bytes( std::span( color ) ), "debug tint (RFC 0014)" );
	++m_Tints;
	return true;
}

} // namespace render::pass::debug
