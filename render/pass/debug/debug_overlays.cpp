//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.debug (RFC 0014); see debug_overlays.h.
//
//=============================================================================//

#include "render/pass/debug/debug_overlays.h"

#include "spv/debug_spv.h"

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

bool DebugOverlays::RecordHatch(
    IRenderDevice2 &device, CommandEncoder &encoder, const HatchTarget &target )
{
	if ( !target.color.IsValid() || target.width == 0 || target.height == 0 )
		return false;
	if ( m_Device && m_Device != &device )
		ReleaseDevice( *m_Device );
	m_Device = &device;
	const auto key = std::make_tuple( target.format, target.samples, target.encodeOutput );
	auto found = m_Pipelines.find( key );
	if ( found == m_Pipelines.end() )
	{
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
		        std::as_bytes( std::span( spirv::kFullscreenVertex ) ), "main", {}, 0 },
		    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
		        std::as_bytes( std::span( spirv::kHatchFragment ) ), "main", {}, 0 } };
		const SpecializationConstant constants[] = {
		    { ShaderStage::kFragment, 0, target.encodeOutput ? 1u : 0u } };
		const Format colors[] = { target.format };
		PipelineDesc desc;
		desc.kind = PipelineKind::kGraphics;
		desc.stages = stages;
		desc.topology = PrimitiveTopology::kTriangleList;
		desc.raster.cull = CullMode::kNone;
		desc.colorFormats = colors;
		desc.sampleCount = target.samples;
		desc.debugName = "render.pass.debug.hatch";
		desc.constants = constants;
		auto pipeline = device.CreatePipeline( desc );
		if ( !pipeline )
			return false;
		found = m_Pipelines.emplace( key, pipeline.Value() ).first;
	}
	encoder.BeginLabel( "debug hatch (RFC 0014)" );
	const ColorAttachment colors[] = {
	    { target.color, LoadOp::kLoad, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.width = target.width;
	rendering.height = target.height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport( { 0, 0, float( target.width ), float( target.height ), 0, 1 } );
	encoder.SetPipeline( found->second );
	encoder.Draw( 3, 1, 0, 0 );
	encoder.EndRendering();
	encoder.EndLabel();
	++m_Hatches;
	return true;
}

} // namespace render::pass::debug
