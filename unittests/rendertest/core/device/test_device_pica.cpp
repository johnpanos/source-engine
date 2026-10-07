//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2.pica (RFC 0026 P1-P2): the shared device suite
//			against render.device.pica, on a 3DS or in Azahar.
//
//			The suite's fixtures are SPIR-V; the driver maps each to its kPica
//			equivalent: the vertex fixtures are picasso programs
//			(shaders/pica/*.v.pica, assembled by tools/n3ds/build_device_suite.sh
//			and wrapped in PVS1 here), the fragment fixtures PFP1 combiner
//			programs written here. comparison.frag has none (the PICA200
//			samples no depth) and double.comp none (no compute).
//
//			The program writes its output to sdmc:/pica_device.txt and an
//			exit marker to sdmc:/pica_device.done.
//
//=============================================================================//

#include <3ds.h>

#include "device_conformance.h"
#include "test_shaders.h"

#include "render/device/pica_format.h"
#include "render/device/pica_codes.h"
#include "render/device/pica/provider.h"
#include "testing/checks.h"

#include <cstdio>
#include <array>
#include <map>
#include <string>

// libctru's main-thread stack (32 KiB by default) is too small for the
// shared suite's clauses.
extern "C" u32 __stacksize__;
u32 __stacksize__ = 1024 * 1024;

extern "C" const unsigned char fullscreen_shbin[];
extern "C" const unsigned fullscreen_shbin_size;
extern "C" const unsigned char tophalf_shbin[];
extern "C" const unsigned tophalf_shbin_size;
extern "C" const unsigned char position_shbin[];
extern "C" const unsigned position_shbin_size;
extern "C" const unsigned char shifted_shbin[];
extern "C" const unsigned shifted_shbin_size;
extern "C" const unsigned char colored_shbin[];
extern "C" const unsigned colored_shbin_size;

namespace
{

using namespace render::device;
using namespace render::device::pica;
using namespace render::device::pica_format;

std::vector<std::byte> Vertex( const unsigned char *shbin, unsigned size, bool vertexIndex )
{
	VertexProgram program;
	program.vertexIndexRegister = vertexIndex ? 0 : kNone;
	return WriteVertexProgram( program, { reinterpret_cast<const std::byte *>( shbin ), size } );
}

// One combiner stage writing the constant (or texture 0) to colour and alpha.
CombinerStage Replace( std::uint8_t from )
{
	CombinerStage stage;
	stage.rgbSources = { from, from, from };
	stage.alphaSources = { from, from, from };
	return stage;
}

std::vector<std::byte> ColorFragment()
{
	FragmentProgram program;
	CombinerStage stage = Replace( source::kConstant );
	stage.constantKind = ConstantKind::kUniformBuffer;
	stage.constantGroup = 2;
	stage.constantBinding = 0;
	program.stages.push_back( stage );
	return WriteFragmentProgram( program );
}

std::vector<std::byte> ConstantFragment()
{
	FragmentProgram program;
	CombinerStage stage = Replace( source::kConstant );
	stage.constantKind = ConstantKind::kDrawConstants;
	program.stages.push_back( stage );
	return WriteFragmentProgram( program );
}

std::vector<std::byte> SpecializedFragment()
{
	FragmentProgram program;
	CombinerStage stage = Replace( source::kConstant );
	stage.constantColor = 0xFF0000FFu; // red, R in the low byte
	program.stages.push_back( stage );
	program.specializations.push_back( { 7, 1, 0, 0xFF00FF00u } ); // green
	return WriteFragmentProgram( program );
}

std::vector<std::byte> SampledFragment()
{
	FragmentProgram program;
	program.textures.push_back( { 2, 0, 2, 1 } );
	program.stages.push_back( Replace( source::kTexture0 ) );
	return WriteFragmentProgram( program );
}

const std::map<const std::uint32_t *, std::vector<std::byte>> &Artifacts()
{
	static const std::map<const std::uint32_t *, std::vector<std::byte>> artifacts = {
	    { rendertest::shaders::kFullScreenVertex,
	        Vertex( fullscreen_shbin, fullscreen_shbin_size, true ) },
	    { rendertest::shaders::kTopHalfVertex, Vertex( tophalf_shbin, tophalf_shbin_size, true ) },
	    { rendertest::shaders::kPositionVertex,
	        Vertex( position_shbin, position_shbin_size, false ) },
	    { rendertest::shaders::kColorFragment, ColorFragment() },
	    { rendertest::shaders::kConstantFragment, ConstantFragment() },
	    { rendertest::shaders::kSpecializedFragment, SpecializedFragment() },
	    { rendertest::shaders::kSampledFragment, SampledFragment() },
	};
	return artifacts;
}

// A fixture without a kPica form: one byte no reader accepts.
const std::byte kNoArtifact[1] = {};

// -- render.device.v2.pica.raster ------------------------------------------
// What the PICA200's fixed function does that the shared suite reaches only
// through its multisample clause (skipped: no 4x): indexed draws from vertex
// and index buffers, culling and winding, viewport offsets, depth ordering,
// vertexOffset, strips, a vertex-stage uniform buffer, the combiner's alpha
// test. 8x8 targets; each check names the pixels it expects.

constexpr std::uint32_t kSize = 8;

struct Raster
{
	testing::Checks &checks;
	IRenderDevice2 &device;
	BindGroupLayoutId view;     // binding 0: a uniform buffer for the vertex stage
	BindGroupLayoutId material; // binding 0: a uniform buffer for the fragment stage

	BufferId Upload( std::span<const std::byte> bytes, UsageSet usages )
	{
		BufferDesc desc;
		desc.size = bytes.size();
		desc.usages = usages;
		desc.usages.Add( ResourceUsage::kCopyDestination );
		auto buffer = device.CreateBuffer( desc );
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		CommandEncoder &e = encoder.Value();
		e.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.WriteBuffer( buffer.Value(), 0, bytes );
		ResourceUsage final = ResourceUsage::kVertex;
		for ( ResourceUsage u :
		    { ResourceUsage::kVertex, ResourceUsage::kIndex, ResourceUsage::kUniform } )
			if ( usages.Has( u ) )
				final = u;
		e.TransitionBuffer( buffer.Value(), ResourceUsage::kCopyDestination, final );
		CommandEncoder list[] = { std::move( e ) };
		auto token = device.Submit( QueueKind::kGraphics, list, {} );
		(void)device.WaitIdle();
		(void)token;
		return buffer.Value();
	}

	BindGroupId Uniform( BindGroupLayoutId layout, const float ( &value )[4] )
	{
		const BufferId buffer =
		    Upload( std::as_bytes( std::span( value ) ), { ResourceUsage::kUniform } );
		const BindGroupEntry entry[] = { { 0, buffer, 0, 16, {}, {} } };
		return device.CreateBindGroup( { layout, entry } ).Value();
	}

	struct Draw
	{
		PipelineId pipeline;
		BindGroupId material;
		BindGroupId view;
		BufferId vertices;
		BufferId indices;
		std::uint32_t count = 3;
		std::int32_t vertexOffset = 0;
		Viewport viewport{ 0, 0, float( kSize ), float( kSize ), 0, 1 };
	};

	// Draws into a cleared colour target (and depth, cleared to 1, when
	// depth is set) and returns its RGBA8 texels.
	std::vector<std::byte> Render(
	    std::span<const Draw> draws, bool depth, std::uint32_t area = kSize )
	{
		TextureDesc desc;
		desc.format = Format::kRGBA8Unorm;
		desc.width = desc.height = kSize;
		desc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
		auto color = device.CreateTexture( desc );
		desc.format = Format::kD24UnormS8;
		desc.usages = { ResourceUsage::kDepthWrite };
		auto depthTarget = device.CreateTexture( desc );
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		CommandEncoder &e = encoder.Value();
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
		e.TransitionTexture(
		    depthTarget.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
		const ColorAttachment attachment[] = {
		    { color.Value(), LoadOp::kClear, StoreOp::kStore, {}, {} } };
		RenderingDesc rendering;
		rendering.colors = attachment;
		if ( depth )
			rendering.depth =
			    DepthAttachment{ depthTarget.Value(), LoadOp::kClear, StoreOp::kStore, 1.0f };
		rendering.width = rendering.height = area;
		e.BeginRendering( rendering );
		for ( const Draw &draw : draws )
		{
			e.SetPipeline( draw.pipeline );
			if ( draw.view.IsValid() )
				e.SetBindGroup( BindGroupRole::kView, draw.view );
			if ( draw.material.IsValid() )
				e.SetBindGroup( BindGroupRole::kMaterial, draw.material );
			e.SetViewport( draw.viewport );
			if ( draw.vertices.IsValid() )
				e.SetVertexBuffer( 0, draw.vertices, 0 );
			if ( draw.indices.IsValid() )
			{
				e.SetIndexBuffer( draw.indices, 0, IndexFormat::kUint16 );
				e.DrawIndexed( draw.count, 1, 0, draw.vertexOffset );
			}
			else
				e.Draw( draw.count );
		}
		e.EndRendering();
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
		BufferDesc out;
		out.size = kSize * kSize * 4;
		out.usages = { ResourceUsage::kCopyDestination };
		out.memory = MemoryKind::kReadback;
		auto readback = device.CreateBuffer( out );
		e.TransitionBuffer(
		    readback.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyTextureToBuffer( color.Value(), readback.Value(), { 0, 0, 0, kSize, kSize } );
		CommandEncoder list[] = { std::move( e ) };
		auto token = device.Submit( QueueKind::kGraphics, list, {} );
		if ( !token )
		{
			std::printf(
			    "INFO raster submission refused: %s\n", DescribeStatus( token.Error().status ) );
			return {};
		}
		(void)device.WaitIdle();
		std::vector<std::byte> pixels( out.size );
		if ( !device.ReadBuffer( readback.Value(), 0, pixels ) )
			return {};
		for ( TextureId t : { color.Value(), depthTarget.Value() } )
			(void)device.Release( t, token.Value() );
		(void)device.Release( readback.Value(), token.Value() );
		(void)device.Poll();
		return pixels;
	}

	PipelineId Pipeline( std::span<const std::byte> vertex,
	    std::span<const ReflectedBinding> vertexUses, std::span<const std::byte> fragment,
	    bool attributes, PrimitiveTopology topology, render::device::CullMode cull,
	    bool frontCounterClockwise, bool depth )
	{
		static const ReflectedBinding materialUse[] = { { 2, 0, BindingKind::kUniformBuffer } };
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, ArtifactFormat::kPica, vertex, "main", vertexUses },
		    { ShaderStage::kFragment, ArtifactFormat::kPica, fragment, "main", materialUse } };
		// The view layout only where the vertex stage reads it: a layout with
		// bindings must be bound for every draw.
		const BindGroupLayoutId layouts[] = {
		    {}, vertexUses.empty() ? BindGroupLayoutId{} : view, material };
		static const VertexAttribute attribute[] = { { 0, VertexFormat::kFloat2, 0, 0 } };
		static const VertexBufferLayout buffer[] = { { 8, false } };
		const Format colors[] = { Format::kRGBA8Unorm };
		PipelineDesc desc;
		desc.stages = stages;
		desc.layouts = layouts;
		if ( attributes )
			desc.vertex = { attribute, buffer };
		desc.topology = topology;
		desc.raster.cull = cull;
		desc.raster.frontCounterClockwise = frontCounterClockwise;
		desc.colorFormats = colors;
		if ( depth )
		{
			desc.depthFormat = Format::kD24UnormS8;
			desc.depthStencil = { true, true, CompareOp::kLess };
		}
		auto pipeline = device.CreatePipeline( desc );
		checks.That( pipeline.HasValue(), "render.device.v2.pica.raster a pipeline is created" );
		return pipeline ? pipeline.Value() : PipelineId{};
	}
};

// 'r' red, 'g' green, '.' the clear colour (0, 0, 0, 0), per texel.
char Classify( const std::vector<std::byte> &pixels, std::uint32_t x, std::uint32_t y )
{
	const std::byte *p = &pixels[( y * kSize + x ) * 4];
	const int r = int( p[0] ), g = int( p[1] ), b = int( p[2] ), a = int( p[3] );
	if ( r == 255 && g == 0 && b == 0 && a == 255 )
		return 'r';
	if ( r == 0 && g == 255 && b == 0 && a == 255 )
		return 'g';
	if ( r == 0 && g == 0 && b == 0 && a == 0 )
		return '.';
	return '?';
}

// True when every texel is what `expect(x, y)` names.
template <typename Expect> bool Matches( const std::vector<std::byte> &pixels, Expect expect )
{
	if ( pixels.size() != kSize * kSize * 4 )
		return false;
	bool all = true;
	for ( std::uint32_t y = 0; y < kSize; ++y )
		for ( std::uint32_t x = 0; x < kSize; ++x )
			all = all && Classify( pixels, x, y ) == expect( x, y );
	if ( !all )
		for ( std::uint32_t y = 0; y < kSize; ++y )
		{
			std::printf( "INFO   " );
			for ( std::uint32_t x = 0; x < kSize; ++x )
				std::printf( "%c", Classify( pixels, x, y ) );
			std::printf( "\n" );
		}
	return all;
}

void RasterChecks( testing::Checks &checks, IRenderDevice2 &device )
{
	static const BindingDesc vertexUniform[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex } } };
	static const BindingDesc fragmentUniform[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
	auto view = device.CreateBindGroupLayout( { BindGroupRole::kView, vertexUniform } );
	auto material = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, fragmentUniform } );
	if ( !checks.That( view && material, "render.device.v2.pica.raster layouts are created" ) )
		return;
	Raster r{ checks, device, view.Value(), material.Value() };
	const float redColor[4] = { 1, 0, 0, 1 };
	const float greenColor[4] = { 0, 1, 0, 1 };
	const BindGroupId red = r.Uniform( r.material, redColor );
	const BindGroupId green = r.Uniform( r.material, greenColor );
	const auto &fullscreen = Artifacts().at( rendertest::shaders::kFullScreenVertex );
	const auto &position = Artifacts().at( rendertest::shaders::kPositionVertex );
	const auto &colorFragment = Artifacts().at( rendertest::shaders::kColorFragment );

	// Two quads: the left counter-clockwise, the right clockwise (clip Y up),
	// after four vertices the offset draw skips.
	const float quads[] = { 9, 9, 9, 9, 9, 9, 9, 9, // four unused vertices
	    -1, -1, 0, -1, 0, 1, -1, 1,                 // left, counter-clockwise
	    0, -1, 0, 1, 1, 1, 1, -1 };                 // right, clockwise
	const BufferId vertices =
	    r.Upload( std::as_bytes( std::span( quads ) ), { ResourceUsage::kVertex } );
	const std::uint16_t both[] = { 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11 };
	const BufferId indices =
	    r.Upload( std::as_bytes( std::span( both ) ), { ResourceUsage::kIndex } );
	auto left = []( std::uint32_t x, std::uint32_t )
	{
		return x < kSize / 2 ? 'r' : '.';
	};
	auto right = []( std::uint32_t x, std::uint32_t )
	{
		return x >= kSize / 2 ? 'r' : '.';
	};
	auto all = []( std::uint32_t, std::uint32_t )
	{
		return 'r';
	};

	const PipelineId backCcw = r.Pipeline( position, {}, colorFragment, true,
	    PrimitiveTopology::kTriangleList, render::device::CullMode::kBack, true, false );
	const PipelineId backCw = r.Pipeline( position, {}, colorFragment, true,
	    PrimitiveTopology::kTriangleList, render::device::CullMode::kBack, false, false );
	const PipelineId none = r.Pipeline( position, {}, colorFragment, true,
	    PrimitiveTopology::kTriangleList, render::device::CullMode::kNone, true, false );
	const PipelineId front = r.Pipeline( position, {}, colorFragment, true,
	    PrimitiveTopology::kTriangleList, render::device::CullMode::kFront, true, false );
	auto indexed = [&]( PipelineId pipeline )
	{
		Raster::Draw d;
		d.pipeline = pipeline;
		d.material = red;
		d.vertices = vertices;
		d.indices = indices;
		d.count = 12;
		const Raster::Draw list[] = { d };
		return r.Render( list, false );
	};
	checks.That( Matches( indexed( none ), all ),
	    "render.device.v2.pica.raster an indexed draw from vertex "
	    "and index buffers covers both quads" );
	checks.That( Matches( indexed( backCcw ), left ),
	    "render.device.v2.pica.raster counter-clockwise "
	    "front, back culled: the clockwise quad goes" );
	checks.That( Matches( indexed( backCw ), right ),
	    "render.device.v2.pica.raster clockwise front, back "
	    "culled: the counter-clockwise quad goes" );
	checks.That( Matches( indexed( front ), right ),
	    "render.device.v2.pica.raster counter-clockwise front, front culled: the front quad goes" );

	// vertexOffset: indices 0-5 read the left quad four vertices on.
	{
		const std::uint16_t first[] = { 0, 1, 2, 0, 2, 3 };
		Raster::Draw d;
		d.pipeline = none;
		d.material = red;
		d.vertices = vertices;
		d.indices = r.Upload( std::as_bytes( std::span( first ) ), { ResourceUsage::kIndex } );
		d.count = 6;
		d.vertexOffset = 4;
		const Raster::Draw list[] = { d };
		checks.That( Matches( r.Render( list, false ), left ),
		    "render.device.v2.pica.raster vertexOffset moves the indexed vertices" );
	}

	// A strip: the left quad as four vertices.
	{
		const float strip[] = { -1, -1, 0, -1, -1, 1, 0, 1 };
		const PipelineId stripped = r.Pipeline( position, {}, colorFragment, true,
		    PrimitiveTopology::kTriangleStrip, render::device::CullMode::kNone, true, false );
		Raster::Draw d;
		d.pipeline = stripped;
		d.material = red;
		d.vertices = r.Upload( std::as_bytes( std::span( strip ) ), { ResourceUsage::kVertex } );
		d.count = 4;
		const Raster::Draw list[] = { d };
		checks.That( Matches( r.Render( list, false ), left ),
		    "render.device.v2.pica.raster a triangle strip draws its quad" );
	}

	// Viewports: the right half, then the bottom half (row 0 at the top).
	{
		const PipelineId full = r.Pipeline( fullscreen, {}, colorFragment, false,
		    PrimitiveTopology::kTriangleList, render::device::CullMode::kNone, true, false );
		Raster::Draw d;
		d.pipeline = full;
		d.material = red;
		d.viewport = { 4, 0, 4, 8, 0, 1 };
		const Raster::Draw rightHalf[] = { d };
		checks.That( Matches( r.Render( rightHalf, false ), right ),
		    "render.device.v2.pica.raster a viewport at x 4 draws the right half" );
		d.viewport = { 0, 4, 8, 4, 0, 1 };
		const Raster::Draw bottomHalf[] = { d };
		checks.That( Matches( r.Render( bottomHalf, false ),
		                 []( std::uint32_t, std::uint32_t y )
		                 {
			                 return y >= kSize / 2 ? 'r' : '.';
		                 } ),
		    "render.device.v2.pica.raster a viewport at y 4 draws the bottom half" );
	}

	// Ordering within one submission: a draw from a vertex buffer, a write to
	// that buffer, and a second draw from it; each draw shows the vertices it
	// was recorded after (the left quad, then the right one in green).
	{
		const float leftQuad[] = { -1, -1, 0, -1, 0, 1, -1, 1 };
		const float rightQuad[] = { 0, -1, 1, -1, 1, 1, 0, 1 };
		const BufferId moving =
		    r.Upload( std::as_bytes( std::span( leftQuad ) ), { ResourceUsage::kVertex } );
		const std::uint16_t quad[] = { 0, 1, 2, 0, 2, 3 };
		const BufferId quadIndices =
		    r.Upload( std::as_bytes( std::span( quad ) ), { ResourceUsage::kIndex } );
		TextureDesc desc;
		desc.format = Format::kRGBA8Unorm;
		desc.width = desc.height = kSize;
		desc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource };
		auto color = device.CreateTexture( desc );
		BufferDesc out;
		out.size = kSize * kSize * 4;
		out.usages = { ResourceUsage::kCopyDestination };
		out.memory = MemoryKind::kReadback;
		auto readback = device.CreateBuffer( out );
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		CommandEncoder &e = encoder.Value();
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
		const ColorAttachment attachment[] = {
		    { color.Value(), LoadOp::kClear, StoreOp::kStore, {}, {} } };
		RenderingDesc rendering;
		rendering.colors = attachment;
		rendering.width = rendering.height = kSize;
		auto pass = [&]( BindGroupId material, LoadOp load )
		{
			const ColorAttachment loaded[] = { { color.Value(), load, StoreOp::kStore, {}, {} } };
			rendering.colors = loaded;
			e.BeginRendering( rendering );
			e.SetPipeline( none );
			e.SetBindGroup( BindGroupRole::kMaterial, material );
			e.SetVertexBuffer( 0, moving, 0 );
			e.SetIndexBuffer( quadIndices, 0, IndexFormat::kUint16 );
			e.DrawIndexed( 6 );
			e.EndRendering();
		};
		pass( red, LoadOp::kClear );
		e.TransitionBuffer( moving, ResourceUsage::kVertex, ResourceUsage::kCopyDestination );
		e.WriteBuffer( moving, 0, std::as_bytes( std::span( rightQuad ) ) );
		e.TransitionBuffer( moving, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
		pass( green, LoadOp::kLoad );
		e.TransitionTexture(
		    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
		e.TransitionBuffer(
		    readback.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyTextureToBuffer( color.Value(), readback.Value(), { 0, 0, 0, kSize, kSize } );
		CommandEncoder list[] = { std::move( e ) };
		auto token = device.Submit( QueueKind::kGraphics, list, {} );
		std::vector<std::byte> pixels( out.size );
		const bool read = token && device.ReadBuffer( readback.Value(), 0, pixels );
		checks.That( read && Matches( pixels,
		                         []( std::uint32_t x, std::uint32_t )
		                         {
			                         return x < kSize / 2 ? 'r' : 'g';
		                         } ),
		    "render.device.v2.pica.raster a draw keeps the vertices written before it, not after" );
	}

	// A render area smaller than its attachment: the full-screen draw stays
	// inside the top-left 4x4.
	{
		const PipelineId full = r.Pipeline( fullscreen, {}, colorFragment, false,
		    PrimitiveTopology::kTriangleList, render::device::CullMode::kNone, true, false );
		Raster::Draw d;
		d.pipeline = full;
		d.material = red;
		d.viewport = { 0, 0, 8, 8, 0, 1 };
		const Raster::Draw list[] = { d };
		checks.That( Matches( r.Render( list, false, 4 ),
		                 []( std::uint32_t x, std::uint32_t y )
		                 {
			                 return x < 4 && y < 4 ? 'r' : '.';
		                 } ),
		    "render.device.v2.pica.raster nothing is drawn outside the render area" );
	}

	// Depth: red at 0.25 and green at 0.5 in both orders; less keeps red.
	{
		const PipelineId nearer = r.Pipeline( fullscreen, {}, colorFragment, false,
		    PrimitiveTopology::kTriangleList, render::device::CullMode::kNone, true, true );
		const PipelineId farther = r.Pipeline( position, {}, colorFragment, true,
		    PrimitiveTopology::kTriangleList, render::device::CullMode::kNone, true, true );
		Raster::Draw a;
		a.pipeline = nearer;
		a.material = red;
		Raster::Draw b;
		b.pipeline = farther;
		b.material = green;
		b.vertices = vertices;
		b.indices = indices;
		b.count = 12;
		const Raster::Draw nearFirst[] = { a, b };
		const Raster::Draw farFirst[] = { b, a };
		checks.That( Matches( r.Render( nearFirst, true ), all ),
		    "render.device.v2.pica.raster depth less: a farther draw after a nearer one is "
		    "hidden" );
		checks.That( Matches( r.Render( farFirst, true ), all ),
		    "render.device.v2.pica.raster depth less: a nearer draw after a farther one covers "
		    "it" );
	}

	// A vertex-stage uniform buffer: the left quad moved right by 1.
	{
		VertexProgram shifted;
		shifted.uniforms.push_back( { 1, 0, 0, 1 } );
		const auto artifact = WriteVertexProgram(
		    shifted, { reinterpret_cast<const std::byte *>( shifted_shbin ), shifted_shbin_size } );
		static const ReflectedBinding viewUse[] = { { 1, 0, BindingKind::kUniformBuffer } };
		const PipelineId moved = r.Pipeline( artifact, viewUse, colorFragment, true,
		    PrimitiveTopology::kTriangleList, render::device::CullMode::kNone, true, false );
		const float shift[4] = { 1, 0, 0, 0 };
		const std::uint16_t first[] = { 4, 5, 6, 4, 6, 7 };
		Raster::Draw d;
		d.pipeline = moved;
		d.material = red;
		d.view = r.Uniform( r.view, shift );
		d.vertices = vertices;
		d.indices = r.Upload( std::as_bytes( std::span( first ) ), { ResourceUsage::kIndex } );
		d.count = 6;
		const Raster::Draw list[] = { d };
		checks.That( Matches( r.Render( list, false ), right ),
		    "render.device.v2.pica.raster a vertex-stage uniform buffer reaches its registers" );
	}

	// Vertex colour: position and a kUnorm8x4 colour interleaved (12-byte
	// vertices); the left quad red, the right green, by the vertices' bytes.
	{
		struct Vertex
		{
			float x, y;
			std::uint8_t rgba[4];
		};
		static_assert( sizeof( Vertex ) == 12 );
		const Vertex coloured[] = { { -1, -1, { 255, 0, 0, 255 } }, { 0, -1, { 255, 0, 0, 255 } },
		    { 0, 1, { 255, 0, 0, 255 } }, { -1, 1, { 255, 0, 0, 255 } },
		    { 0, -1, { 0, 255, 0, 255 } }, { 1, -1, { 0, 255, 0, 255 } },
		    { 1, 1, { 0, 255, 0, 255 } }, { 0, 1, { 0, 255, 0, 255 } } };
		const std::uint16_t two[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 };
		VertexProgram colored;
		const auto vertex = WriteVertexProgram(
		    colored, { reinterpret_cast<const std::byte *>( colored_shbin ), colored_shbin_size } );
		FragmentProgram primary;
		primary.stages.push_back( Replace( source::kPrimaryColor ) );
		const auto fragment = WriteFragmentProgram( primary );
		static const VertexAttribute attributes[] = {
		    { 0, VertexFormat::kFloat2, 0, 0 }, { 1, VertexFormat::kUnorm8x4, 8, 0 } };
		static const VertexBufferLayout buffer[] = { { 12, false } };
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, ArtifactFormat::kPica, vertex, "main", {} },
		    { ShaderStage::kFragment, ArtifactFormat::kPica, fragment, "main", {} } };
		const Format colors[] = { Format::kRGBA8Unorm };
		PipelineDesc desc;
		desc.stages = stages;
		desc.vertex = { attributes, buffer };
		desc.raster.cull = render::device::CullMode::kNone;
		desc.colorFormats = colors;
		auto pipeline = device.CreatePipeline( desc );
		checks.That( pipeline.HasValue(),
		    "render.device.v2.pica.raster a vertex-colour pipeline is created" );
		if ( pipeline )
		{
			Raster::Draw d;
			d.pipeline = pipeline.Value();
			d.vertices =
			    r.Upload( std::as_bytes( std::span( coloured ) ), { ResourceUsage::kVertex } );
			d.indices = r.Upload( std::as_bytes( std::span( two ) ), { ResourceUsage::kIndex } );
			d.count = 12;
			const Raster::Draw list[] = { d };
			checks.That( Matches( r.Render( list, false ),
			                 []( std::uint32_t x, std::uint32_t )
			                 {
				                 return x < kSize / 2 ? 'r' : 'g';
			                 } ),
			    "render.device.v2.pica.raster kUnorm8x4 vertex colours reach the combiner in RGBA "
			    "order" );
		}
	}

	// Two texture units modulated (a lightmapped surface's shape): yellow times
	// cyan is green, each unit with its own texture and texcoord.
	{
		static const BindingDesc twoTextures[] = {
		    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
		    { 1, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
		    { 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
		auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, twoTextures } );
		FragmentProgram program;
		program.textures = { { 2, 0, 2, 2 }, { 2, 1, 2, 2 } };
		CombinerStage stage;
		stage.rgbSources = { source::kTexture0, source::kTexture1, source::kTexture0 };
		stage.alphaSources = stage.rgbSources;
		stage.rgbCombine = stage.alphaCombine = 1; // GPU_MODULATE
		program.stages.push_back( stage );
		const auto fragment = WriteFragmentProgram( program );
		static const ReflectedBinding used[] = { { 2, 0, BindingKind::kSampledTexture },
		    { 2, 1, BindingKind::kSampledTexture }, { 2, 2, BindingKind::kSampler } };
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, ArtifactFormat::kPica, fullscreen, "main", {} },
		    { ShaderStage::kFragment, ArtifactFormat::kPica, fragment, "main", used } };
		const BindGroupLayoutId layouts[] = { {}, {}, layout.Value() };
		const Format colors[] = { Format::kRGBA8Unorm };
		PipelineDesc desc;
		desc.stages = stages;
		desc.layouts = layouts;
		desc.raster.cull = render::device::CullMode::kNone;
		desc.colorFormats = colors;
		auto pipeline = device.CreatePipeline( desc );
		auto texture = [&]( std::uint8_t red, std::uint8_t green, std::uint8_t blue )
		{
			TextureDesc t;
			t.format = Format::kRGBA8Unorm;
			t.width = t.height = kSize;
			t.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
			auto made = device.CreateTexture( t );
			std::vector<std::byte> texels;
			for ( std::uint32_t i = 0; i < kSize * kSize; ++i )
				for ( std::uint8_t c : { red, green, blue, std::uint8_t( 255 ) } )
					texels.push_back( std::byte( c ) );
			auto staging = device.CreateUploadBuffer( texels );
			auto encoder = device.BeginEncoder( QueueKind::kGraphics );
			CommandEncoder &e = encoder.Value();
			e.TransitionTexture(
			    made.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			e.CopyBufferToTexture( staging.Value(), made.Value(), { 0, 0, 0, kSize, kSize } );
			e.TransitionTexture(
			    made.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
			CommandEncoder list[] = { std::move( e ) };
			(void)device.Submit( QueueKind::kGraphics, list, {} );
			return made.Value();
		};
		SamplerDesc point;
		point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
		auto sampler = device.CreateSampler( point );
		const BindGroupEntry entries[] = { { 0, {}, 0, 0, texture( 255, 255, 0 ), {} },
		    { 1, {}, 0, 0, texture( 0, 255, 255 ), {} }, { 2, {}, 0, 0, {}, sampler.Value() } };
		auto group = device.CreateBindGroup( { layout.Value(), entries } );
		checks.That( pipeline && group,
		    "render.device.v2.pica.raster a two-texture pipeline and group are created" );
		if ( pipeline && group )
		{
			Raster::Draw d;
			d.pipeline = pipeline.Value();
			d.material = group.Value();
			const Raster::Draw list[] = { d };
			checks.That( Matches( r.Render( list, false ),
			                 []( std::uint32_t, std::uint32_t )
			                 {
				                 return 'g';
			                 } ),
			    "render.device.v2.pica.raster two units modulate: yellow times cyan is green" );
		}
	}

	// Mipmaps: a 16x16 texture (level 0 red, level 1 green) over the 8x8
	// target is minified 2:1, so it samples level 1. Azahar's software
	// renderer samples level 0 only (no texture LOD), so run_device_suite.py
	// reports this check unverified there, never passed.
	{
		static const BindingDesc sampled[] = {
		    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
		    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
		auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, sampled } );
		static const ReflectedBinding used[] = {
		    { 2, 0, BindingKind::kSampledTexture }, { 2, 1, BindingKind::kSampler } };
		const auto &fragment = Artifacts().at( rendertest::shaders::kSampledFragment );
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, ArtifactFormat::kPica, fullscreen, "main", {} },
		    { ShaderStage::kFragment, ArtifactFormat::kPica, fragment, "main", used } };
		const BindGroupLayoutId layouts[] = { {}, {}, layout.Value() };
		const Format colors[] = { Format::kRGBA8Unorm };
		PipelineDesc desc;
		desc.stages = stages;
		desc.layouts = layouts;
		desc.raster.cull = render::device::CullMode::kNone;
		desc.colorFormats = colors;
		auto pipeline = device.CreatePipeline( desc );
		TextureDesc t;
		t.format = Format::kRGBA8Unorm;
		t.width = t.height = 16;
		t.mipLevels = 2;
		t.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		auto texture = device.CreateTexture( t );
		std::vector<std::byte> level0, level1;
		for ( int i = 0; i < 16 * 16; ++i )
			for ( int c : { 255, 0, 0, 255 } )
				level0.push_back( std::byte( c ) );
		for ( int i = 0; i < 8 * 8; ++i )
			for ( int c : { 0, 255, 0, 255 } )
				level1.push_back( std::byte( c ) );
		auto staging0 = device.CreateUploadBuffer( level0 );
		auto staging1 = device.CreateUploadBuffer( level1 );
		SamplerDesc point;
		point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
		auto sampler = device.CreateSampler( point );
		const BindGroupEntry entries[] = {
		    { 0, {}, 0, 0, texture.Value(), {} }, { 1, {}, 0, 0, {}, sampler.Value() } };
		auto group = device.CreateBindGroup( { layout.Value(), entries } );
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		CommandEncoder &e = encoder.Value();
		e.TransitionTexture(
		    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyBufferToTexture( staging0.Value(), texture.Value(), { 0, 0, 0, 16, 16 } );
		e.CopyBufferToTexture( staging1.Value(), texture.Value(), { 0, 1, 0, 8, 8 } );
		e.TransitionTexture(
		    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		CommandEncoder list[] = { std::move( e ) };
		(void)device.Submit( QueueKind::kGraphics, list, {} );
		checks.That(
		    pipeline && group, "render.device.v2.pica.raster a mipmapped texture is bound" );
		if ( pipeline && group )
		{
			Raster::Draw d;
			d.pipeline = pipeline.Value();
			d.material = group.Value();
			const Raster::Draw draws[] = { d };
			checks.That( Matches( r.Render( draws, false ),
			                 []( std::uint32_t, std::uint32_t )
			                 {
				                 return 'g';
			                 } ),
			    "render.device.v2.pica.raster [needs texture LOD] 2:1 minification samples level "
			    "1" );
		}
	}

	// ETC1 and ETC1A4 mipmaps (clause D40 beyond level 0): the same 2:1
	// minification of a 16x16 texture whose level 0 decodes red and level 1
	// green samples level 1. Each block is individual mode, table 0, every
	// index 0: its base colour plus 2, clamped (255 or 2 per channel).
	for ( const bool alpha : { false, true } )
	{
		static const BindingDesc sampled[] = {
		    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
		    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
		auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, sampled } );
		static const ReflectedBinding used[] = {
		    { 2, 0, BindingKind::kSampledTexture }, { 2, 1, BindingKind::kSampler } };
		const auto &fragment = Artifacts().at( rendertest::shaders::kSampledFragment );
		const ShaderArtifactView stages[] = {
		    { ShaderStage::kVertex, ArtifactFormat::kPica, fullscreen, "main", {} },
		    { ShaderStage::kFragment, ArtifactFormat::kPica, fragment, "main", used } };
		const BindGroupLayoutId layouts[] = { {}, {}, layout.Value() };
		const Format colors[] = { Format::kRGBA8Unorm };
		PipelineDesc desc;
		desc.stages = stages;
		desc.layouts = layouts;
		desc.raster.cull = render::device::CullMode::kNone;
		desc.colorFormats = colors;
		auto pipeline = device.CreatePipeline( desc );
		TextureDesc t;
		t.format = alpha ? Format::kETC1A4 : Format::kETC1Rgb;
		t.width = t.height = 16;
		t.mipLevels = 2;
		t.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		auto texture = device.CreateTexture( t );
		auto blocks = [alpha]( int count, std::uint64_t r, std::uint64_t g, std::uint64_t b )
		{
			const std::uint64_t word =
			    r << 60 | r << 56 | g << 52 | g << 48 | b << 44 | b << 40; // indices 0
			std::vector<std::byte> bytes;
			for ( int i = 0; i < count; ++i )
			{
				if ( alpha )
				{
					for ( int k = 0; k < 8; ++k )
						bytes.push_back( std::byte( 0xFF ) ); // alpha 15 everywhere
					for ( int k = 0; k < 8; ++k )
						bytes.push_back( std::byte( word >> ( 8 * k ) ) );
				}
				else
					for ( int k = 0; k < 8; ++k )
						bytes.push_back( std::byte( word >> ( 56 - 8 * k ) ) );
			}
			return bytes;
		};
		auto staging0 = device.CreateUploadBuffer( blocks( 16, 15, 0, 0 ) );
		auto staging1 = device.CreateUploadBuffer( blocks( 4, 0, 15, 0 ) );
		SamplerDesc point;
		point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
		auto sampler = device.CreateSampler( point );
		const BindGroupEntry entries[] = {
		    { 0, {}, 0, 0, texture.Value(), {} }, { 1, {}, 0, 0, {}, sampler.Value() } };
		auto group = device.CreateBindGroup( { layout.Value(), entries } );
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		CommandEncoder &e = encoder.Value();
		e.TransitionTexture(
		    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		e.CopyBufferToTexture( staging0.Value(), texture.Value(), { 0, 0, 0, 16, 16 } );
		e.CopyBufferToTexture( staging1.Value(), texture.Value(), { 0, 1, 0, 8, 8 } );
		e.TransitionTexture(
		    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		CommandEncoder list[] = { std::move( e ) };
		(void)device.Submit( QueueKind::kGraphics, list, {} );
		const std::string name = alpha ? "ETC1A4" : "ETC1";
		checks.That( pipeline && group,
		    "render.device.v2.pica.raster a mipmapped " + name + " texture is bound" );
		if ( pipeline && group )
		{
			Raster::Draw d;
			d.pipeline = pipeline.Value();
			d.material = group.Value();
			const Raster::Draw draws[] = { d };
			const std::vector<std::byte> pixels = r.Render( draws, false );
			bool green = pixels.size() == kSize * kSize * 4;
			for ( std::size_t i = 0; green && i < kSize * kSize; ++i )
				green = int( pixels[i * 4] ) < 16 && int( pixels[i * 4 + 1] ) > 240 &&
				        int( pixels[i * 4 + 2] ) < 16;
			if ( !green && !pixels.empty() )
				std::printf( "INFO %s mip texel 0: %d %d %d %d\n", name.c_str(), int( pixels[0] ),
				    int( pixels[1] ), int( pixels[2] ), int( pixels[3] ) );
			checks.That( green, "render.device.v2.pica.raster [needs texture LOD] 2:1 "
			                    "minification of " + name + " samples level 1" );
		}
	}

	// The combiner's alpha test: constant alpha 0.5 against greater-than 200.
	{
		FragmentProgram program;
		CombinerStage stage = Replace( source::kConstant );
		stage.constantKind = ConstantKind::kUniformBuffer;
		stage.constantGroup = 2;
		program.stages.push_back( stage );
		program.alphaTest = test::kGreater;
		program.alphaReference = 200;
		const auto fragment = WriteFragmentProgram( program );
		const PipelineId tested = r.Pipeline( fullscreen, {}, fragment, false,
		    PrimitiveTopology::kTriangleList, render::device::CullMode::kNone, true, false );
		const float halfRed[4] = { 1, 0, 0, 0.5f };
		Raster::Draw d;
		d.pipeline = tested;
		d.material = r.Uniform( r.material, halfRed );
		const Raster::Draw failing[] = { d };
		checks.That( Matches( r.Render( failing, false ),
		                 []( std::uint32_t, std::uint32_t )
		                 {
			                 return '.';
		                 } ),
		    "render.device.v2.pica.raster alpha 128 fails an alpha test greater than 200" );
		d.material = red;
		const Raster::Draw passing[] = { d };
		checks.That( Matches( r.Render( passing, false ), all ),
		    "render.device.v2.pica.raster alpha 255 passes it" );
	}
}

// -- render.device.v2.pica.present (P4) ------------------------------------
// Four quadrants (red, green / blue, white) in the 400x240 region of a
// 512x256 texture, presented, then read from the top screen's scanout
// buffer: column-major, 240 texels per screen column, bottom row first
// (single-buffered, BGR8 after gfxInitDefault).

void PresentChecks( testing::Checks &checks, IRenderDevice2 &device )
{
	constexpr std::uint32_t kWidth = 512, kHeight = 256, kShownWidth = 400, kShownHeight = 240;
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = kWidth;
	desc.height = kHeight;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	auto texture = device.CreateTexture( desc );
	std::vector<std::byte> texels( std::size_t( kWidth ) * kHeight * 4 );
	for ( std::uint32_t y = 0; y < kHeight; ++y )
		for ( std::uint32_t x = 0; x < kWidth; ++x )
		{
			const bool right = x >= kShownWidth / 2, bottom = y >= kShownHeight / 2;
			const std::uint8_t rgb[4][3] = {
			    { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 }, { 255, 255, 255 } };
			const std::uint8_t *c = rgb[( bottom ? 2 : 0 ) + ( right ? 1 : 0 )];
			std::byte *t = &texels[( std::size_t( y ) * kWidth + x ) * 4];
			t[0] = std::byte( c[0] );
			t[1] = std::byte( c[1] );
			t[2] = std::byte( c[2] );
			t[3] = std::byte( 255 );
		}
	auto staging = device.CreateUploadBuffer( texels );
	auto encoder = device.BeginEncoder( QueueKind::kGraphics );
	CommandEncoder &e = encoder.Value();
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBufferToTexture( staging.Value(), texture.Value(), { 0, 0, 0, kWidth, kHeight } );
	e.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	CommandEncoder list[] = { std::move( e ) };
	(void)device.Submit( QueueKind::kGraphics, list, {} );

	auto refused =
	    PresentTopScreen( device, staging.Value() == BufferId{} ? TextureId{} : TextureId{ 999999 },
	        kShownWidth, kShownHeight );
	checks.That( !refused && refused.Error().status == DeviceStatus::kInvalidHandle,
	    "render.device.v2.pica.present an unknown texture is refused" );
	auto presented = PresentTopScreen( device, texture.Value(), kShownWidth, kShownHeight );
	checks.That(
	    presented.HasValue(), "render.device.v2.pica.present a sampled texture is presented" );
	gspWaitForVBlank();

	u16 columnTexels = 0, columns = 0;
	const u8 *scanout = gfxGetFramebuffer( GFX_TOP, GFX_LEFT, &columnTexels, &columns );
	const int bytes = gspGetBytesPerPixel( gfxGetScreenFormat( GFX_TOP ) );
	auto rgb = [&]( int x, int y )
	{
		const u8 *p =
		    scanout + ( std::size_t( x ) * columnTexels + ( columnTexels - 1 - y ) ) * bytes;
		return std::array<int, 3>{ p[2], p[1], p[0] }; // BGR8
	};
	struct Probe
	{
		int x, y;
		std::array<int, 3> want;
		const char *what;
	};
	const Probe probes[] = { { 100, 60, { 255, 0, 0 }, "top-left is red" },
	    { 300, 60, { 0, 255, 0 }, "top-right is green" },
	    { 100, 180, { 0, 0, 255 }, "bottom-left is blue" },
	    { 300, 180, { 255, 255, 255 }, "bottom-right is white" },
	    { 1, 1, { 255, 0, 0 }, "the top-left corner pixel is red" },
	    { 398, 238, { 255, 255, 255 }, "the bottom-right corner pixel is white" } };
	checks.That( scanout && columnTexels == 240 && columns == 400 && bytes == 3,
	    "render.device.v2.pica.present the top screen is 400 columns of 240 BGR8 texels" );
	if ( !scanout || columnTexels != 240 || columns != 400 || bytes != 3 )
		return;
	for ( const Probe &probe : probes )
	{
		const std::array<int, 3> got = rgb( probe.x, probe.y );
		if ( got != probe.want )
			std::printf(
			    "INFO present (%d, %d): %d %d %d\n", probe.x, probe.y, got[0], got[1], got[2] );
		checks.That(
		    got == probe.want, std::string( "render.device.v2.pica.present " ) + probe.what );
	}
}

} // namespace

int main()
{
	gfxInitDefault();
	// One scanout buffer: what the presenter wrote is what the check reads.
	gfxSetDoubleBuffering( GFX_TOP, false );
	std::FILE *log = std::freopen( "sdmc:/pica_device.txt", "w", stdout );
	// Unbuffered: a run that stops shows how far it got.
	std::setvbuf( stdout, nullptr, _IONBF, 0 );

	// PICA_SENSITIVITY (build_device_suite.sh <variant>): one seeded defect,
	// which the suite must fail on (render.device.v2.pica.sensitivity).
	PicaAdapterOptions options;
#if defined( PICA_SENSITIVITY )
	PicaAdapterOptions::Sensitivity &broken = options.sensitivity;
	broken.depthNotNegated = PICA_SENSITIVITY == 1;
	broken.dropDrawConstants = PICA_SENSITIVITY == 2;
	broken.ignoreColorWriteMasks = PICA_SENSITIVITY == 3;
	broken.blendAsOpaque = PICA_SENSITIVITY == 4;
	broken.texturesUpsideDown = PICA_SENSITIVITY == 5;
	broken.windingReversed = PICA_SENSITIVITY == 6;
	broken.cpuRacesGpu = PICA_SENSITIVITY == 7;
	broken.etc1BytesUnswapped = PICA_SENSITIVITY == 8;
#endif
	rendertest::DeviceDriver driver;
	driver.name = "render.device.v2.pica";
	driver.create = [options]() -> std::unique_ptr<IRenderDevice2>
	{
		auto created = render::device::pica::Create( options );
		if ( !created )
		{
			std::printf( "INFO create failed: status %u\n", unsigned( created.Error().status ) );
			return nullptr;
		}
		return std::move( created ).Value();
	};
	driver.complete = []( IRenderDevice2 &device, CompletionToken token )
	{
		if ( device.IsComplete( token ) )
			return true;
		(void)device.WaitIdle();
		return device.IsComplete( token );
	};
	driver.lose = []( IRenderDevice2 &device )
	{
		(void)SimulateDeviceLoss( device );
	};
	driver.rasterizes = true;
	driver.artifact = []( std::span<const std::uint32_t> spirv ) -> std::span<const std::byte>
	{
		const auto found = Artifacts().find( spirv.data() );
		if ( found == Artifacts().end() )
			return kNoArtifact;
		return found->second;
	};

	testing::Checks checks;
	rendertest::RunDeviceConformance( checks, driver );
	rendertest::RunRasterConformance( checks, driver );
	if ( auto device = driver.create() )
		RasterChecks( checks, *device );
	if ( auto device = driver.create() )
		PresentChecks( checks, *device );
	const int result = checks.Report();
	std::fflush( stdout );
	if ( log )
		std::fclose( log );
	if ( std::FILE *done = std::fopen( "sdmc:/pica_device.done", "w" ) )
	{
		std::fprintf( done, "%d\n", result );
		std::fclose( done );
	}
	gfxExit();
	return result;
}
