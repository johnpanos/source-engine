//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared fixtures of the render.pass.lines suites: render a line
//			list (and resident batches) into a fresh color and depth target on
//			any render.device.v2 device and read the color back.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_PASS_LINES_FIXTURES_H
#define RENDERTEST_CORE_PASS_LINES_FIXTURES_H

#include "render/graph/executor.h"
#include "render/pass/lines/lines.h"

#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <thread>
#include <vector>

namespace rendertest::lines
{

using namespace render;
using render::pass::lines::LineList;
using render::pass::lines::LinesRenderer;
using render::pass::lines::LinesStats;
using render::pass::lines::LinesStatus;
using render::pass::lines::LinesTargets;
using render::pass::lines::LinesView;
using render::pass::lines::MeshBatch;
using render::pass::lines::Rgba8;
using render::pass::lines::Space;
using render::pass::lines::Style;
using render::pass::lines::Topology;

constexpr std::uint32_t kSize = 64;

inline bool Wait( device::IRenderDevice2 &device, device::CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !device.IsComplete( token ) )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		(void)device.Poll();
		std::this_thread::yield();
	}
	(void)device.Poll();
	return true;
}

struct FrameResult
{
	bool ok = false;
	std::optional<LinesStatus> refused;
	LinesStats stats;
	std::uint32_t passes = 0;
	std::vector<std::uint8_t> rgba; // kSize x kSize, row 0 at the top
};

// A view whose world-to-clip maps world x and y in [-1, 1] onto the target
// (x right, y up) and world z in [0, 1] onto depth: an orthographic box.
inline LinesView BoxView( float depthBias = 0.0f )
{
	LinesView view;
	view.worldToClip = math::float4x4::Identity();
	view.width = kSize;
	view.height = kSize;
	view.depthBias = depthBias;
	return view;
}

// Draws into fresh targets and reads the color back.
inline FrameResult Draw( device::IRenderDevice2 &device, LinesRenderer &renderer,
    const LineList &list, std::span<const MeshBatch> batches = {},
    const LinesView &view = BoxView(), bool withDepth = true )
{
	FrameResult result;
	device::TextureDesc colorDesc;
	colorDesc.format = device::Format::kRGBA8Unorm;
	colorDesc.width = colorDesc.height = kSize;
	colorDesc.usages = {
	    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kCopySource };
	device::TextureDesc depthDesc = colorDesc;
	depthDesc.format = device::Format::kD32Float;
	depthDesc.usages = { device::ResourceUsage::kDepthWrite };
	device::BufferDesc readbackDesc;
	readbackDesc.size = kSize * kSize * 4;
	readbackDesc.usages = { device::ResourceUsage::kCopyDestination };
	readbackDesc.memory = device::MemoryKind::kReadback;
	auto color = device.CreateTexture( colorDesc );
	auto readback = device.CreateBuffer( readbackDesc );
	if ( !color || !readback )
		return result;

	graph::GraphBuilder builder;
	const graph::ResourceRef colorRef = builder.ImportTexture( "color", color.Value(), colorDesc,
	    device::ResourceUsage::kUndefined, device::ResourceUsage::kCopySource );
	graph::ResourceRef depthRef;
	if ( withDepth )
		depthRef = builder.CreateTexture( "depth", depthDesc );
	const graph::ResourceRef readbackRef = builder.ImportBuffer( "readback", readback.Value(),
	    readbackDesc, device::ResourceUsage::kUndefined, device::ResourceUsage::kCopyDestination );
	LinesTargets targets;
	targets.color = colorRef;
	targets.depth = depthRef;
	targets.width = targets.height = kSize;
	targets.clear = { 0.0f, 0.0f, 0.0f, 1.0f };
	auto stats = renderer.AddPasses( builder, list, batches, view, targets );
	if ( !stats )
	{
		result.refused = stats.Error();
	}
	else
	{
		result.stats = stats.Value();
		builder.AddPass( "readback", graph::PassKind::kCopy )
		    .Read( colorRef, device::ResourceUsage::kCopySource )
		    .Write( readbackRef, device::ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute(
		        [colorRef, readbackRef]( graph::RecordContext &context )
		        {
			        context.Encoder().CopyTextureToBuffer( context.Texture( colorRef ),
			            context.Buffer( readbackRef ), { 0, 0, 0, kSize, kSize } );
		        } );
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( compiled )
		{
			graph::SerialGraphExecutor executor;
			auto executed = executor.Execute( compiled.Value(), device );
			if ( executed && Wait( device, executed.Value().token ) )
			{
				renderer.Collect( executed.Value().token );
				result.passes = executed.Value().passes;
				result.rgba.resize( kSize * kSize * 4 );
				result.ok =
				    device
				        .ReadBuffer( readback.Value(), 0,
				            std::as_writable_bytes( std::span<std::uint8_t>( result.rgba ) ) )
				        .HasValue();
			}
		}
	}
	(void)device.Release( color.Value(), device::CompletionToken() );
	(void)device.Release( readback.Value(), device::CompletionToken() );
	(void)device.Poll();
	return result;
}

// Stages vertices in the LineVertex layout as a resident mesh and waits.
inline std::optional<resources::MeshEntry> StageMesh( device::IRenderDevice2 &device,
    resources::MeshCache &cache, std::string_view name,
    const std::vector<render::pass::lines::LineVertex> &vertices )
{
	resources::MeshData data;
	data.vertices = std::as_bytes( std::span( vertices ) );
	data.vertexStride = sizeof( render::pass::lines::LineVertex );
	auto entry = cache.Stage( name, data );
	auto encoder = device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !entry || !encoder )
		return std::nullopt;
	cache.RecordUploads( encoder.Value() );
	device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto token = device.Submit( device::QueueKind::kGraphics, encoders, {} );
	if ( !token || !Wait( device, token.Value() ) )
		return std::nullopt;
	cache.Retire( token.Value() );
	return entry.Value();
}

} // namespace rendertest::lines

#endif // RENDERTEST_CORE_PASS_LINES_FIXTURES_H
