//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.debug (RFC 0014); see debug_overlays.h.
//
//=============================================================================//

#include "render/pass/debug/debug_overlays.h"

#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <span>
#include <array>
#include <cmath>
#include <cstdio>
#include <vector>
#include <string>

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
	    program == 2 ? "render/pass/debug/cost.frag" :
	    tint ? "render/pass/debug/tint.frag" : "render/pass/debug/hatch.frag" } );
	recipe.topology = PrimitiveTopology::kTriangleList;
	recipe.raster.cull = CullMode::kNone;
	recipe.colorFormats = { target.format };
	recipe.blends = { program != 0 ? BlendMode::kAlpha : BlendMode::kOpaque };
	recipe.sampleCount = target.samples;
	recipe.debugName = program == 2 ? "render.pass.debug.cost" : tint ? "render.pass.debug.tint" : "render.pass.debug.hatch";
	auto resolved =
	    shaderlib::Resolve( recipe, shaderlib::CoreArtifacts(), device.Facts().artifactFormat );
	if ( !resolved )
		return PipelineId();
	const SpecializationConstant constants[] = {
	    { ShaderStage::kFragment, 0, target.encodeOutput ? 1u : 0u } };
	PipelineDesc desc = resolved.Value().Desc();
	desc.drawConstantBytes = program == 2 ? 112u : tint ? 16u : 0u;
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

bool DebugOverlays::RecordCosts( IRenderDevice2 &device, CommandEncoder &encoder,
    const HatchTarget &target, const CostOverlay &costs )
{
	if ( !target.color.IsValid() || target.width < 32 || target.height < 32 )
		return false;
	const PipelineId pipeline = PipelineFor( device, 2, target );
	if ( !pipeline.IsValid() )
		return false;
	// One draw per row. Text is packed into push constants, avoiding a font
	// texture, transient upload allocation or per-glyph draw calls.
	struct Constants
	{
		float rect[4];
		float metrics[4];
		std::uint32_t text[20];
	};
	static_assert( sizeof( Constants ) == 112 );
	const float scale = std::min( 1.0f, float( target.width - 24 ) / 640.0f );
	const float height = 20.0f * scale;
	const std::size_t capacity = std::min<std::size_t>( 32,
	    std::size_t( float( target.height - 24 ) / height ) );
	if ( capacity < 5 )
		return false;
	const ColorAttachment color[] = {
	    { target.color, LoadOp::kLoad, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
	RenderingDesc rendering;
	rendering.colors = color;
	rendering.width = target.width;
	rendering.height = target.height;
	encoder.BeginLabel( "core cost overlay (not in sample)" );
	encoder.BeginRendering( rendering );
	encoder.SetPipeline( pipeline );
	std::size_t line = 0;
	auto draw = [&]( const char *text, double cpu, double gpu, bool measured )
	{
		Constants constants{};
		constants.rect[0] = 12;
		constants.rect[1] = 12 + float( line++ ) * height;
		constants.rect[2] = scale;
		constants.metrics[0] = float( std::clamp( cpu / ( 1000.0 / 60.0 ), 0.0, 1.0 ) );
		constants.metrics[1] = float( std::clamp( gpu / ( 1000.0 / 60.0 ), 0.0, 1.0 ) );
		constants.metrics[2] = measured ? 1 : 0;
		for ( std::size_t i = 0; i < 80 && text[i]; ++i )
		{
			unsigned char c = static_cast<unsigned char>( text[i] );
			if ( c >= 'a' && c <= 'z' )
				c -= 'a' - 'A';
			constants.text[i / 4] |= std::uint32_t( c ) << ( 8 * ( i % 4 ) );
		}
		encoder.SetViewport( { 12, constants.rect[1], 640 * scale, height, 0, 1 } );
		encoder.SetDrawConstants( 0, std::as_bytes( std::span( &constants, 1 ) ) );
		encoder.Draw( 3, 1, 0, 0 );
	};
	char text[160];
	std::snprintf( text, sizeof( text ), "CORE COSTS - FRAME %llu - AGE %llu",
	    static_cast<unsigned long long>( costs.frame ),
	    static_cast<unsigned long long>( costs.currentFrame >= costs.frame ?
	        costs.currentFrame - costs.frame : 0 ) );
	draw( text, 0, 0, false );
	draw( "CPU RECORD / GPU PASS MS - INCLUSIVE: DO NOT SUM ROWS", 0, 0, false );
	draw( "BARS: CPU LEFT / GPU RIGHT. BLUE-GREEN-YELLOW-RED: 0-16.67 MS", 0, 0, false );
	if ( !costs.supported )
		draw( "GPU TIMESTAMPS UNAVAILABLE - NO MEASURED SAMPLE", 0, 0, false );
	else if ( costs.rows.empty() )
		draw( "WAITING FOR A COMPLETED CORE FRAME", 0, 0, false );
	else
	{
		std::vector<const CostRow *> sorted;
		for ( const CostRow &row : costs.rows )
			sorted.push_back( &row );
		std::stable_sort( sorted.begin(), sorted.end(), []( const CostRow *a, const CostRow *b )
		    { return std::max( a->cpuMilliseconds, a->gpuMilliseconds ) >
		             std::max( b->cpuMilliseconds, b->gpuMilliseconds ); } );
		const std::size_t shown = std::min( sorted.size(), capacity - 5 );
		for ( std::size_t i = 0; i < shown; ++i )
		{
			const CostRow &row = *sorted[i];
			const bool valid = std::isfinite( row.cpuMilliseconds ) && row.cpuMilliseconds >= 0 &&
			    std::isfinite( row.gpuMilliseconds ) && row.gpuMilliseconds >= 0;
			if ( valid )
				std::snprintf( text, sizeof( text ), "D%u %-38.38s CPU %7.3f GPU %7.3f",
				    row.depth, std::string( row.name ).c_str(), row.cpuMilliseconds, row.gpuMilliseconds );
			else
				std::snprintf( text, sizeof( text ), "D%u %.*s - INVALID SAMPLE", row.depth,
				    int( std::min<std::size_t>( row.name.size(), 38 ) ), row.name.data() );
			draw( text, valid ? row.cpuMilliseconds : 0, valid ? row.gpuMilliseconds : 0, valid );
		}
		std::snprintf( text, sizeof( text ), "%zu ROWS HIDDEN / %u TIMESTAMPS DROPPED",
		    sorted.size() - shown, costs.dropped );
		draw( text, 0, 0, false );
	}
	draw( "CORE LABELS ONLY: EXCLUDES GAME CPU, LEGACY, PRESENT AND OVERLAY", 0, 0, false );
	encoder.EndRendering();
	encoder.EndLabel();
	return true;
}

} // namespace render::pass::debug
