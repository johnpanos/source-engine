//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.culling (RFC 0016 GPU-driven submission S4); see cull.h.
//
//=============================================================================//

#include "render/culling/cull.h"

#include "render/graph/executor.h"

#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <utility>

namespace render::culling
{

namespace
{

using namespace render::device;

// cull.comp's bindings, all in the draw group.
enum Binding : std::uint32_t
{
	kInstances = 0,
	kView,
	kVisibility,
	kBindingCount
};

static_assert( static_cast<int>( math::FrustumPlane::kCount ) == 6 );

// compact.comp's bindings, all in the draw group.
enum CompactBinding : std::uint32_t
{
	kCompactVisibility = 0,
	kCompactTemplates,
	kCompactView,
	kCompactBuckets,
	kCompactCommands,
	kCompactBindingCount
};

// A compute kernel of `bindingCount` storage buffers in the draw group, from
// the core artifact `source` or a suite's replacement SPIR-V.
foundation::Expected<std::pair<BindGroupLayoutId, PipelineId>, CullStatus> CreateKernel(
    IRenderDevice2 &device, const char *source, std::span<const BindingDesc> bindings,
    const char *debugName, std::span<const std::uint32_t> code )
{
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return foundation::MakeUnexpected( CullStatus::kNoCompute );
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( CullStatus::kDevice );
	auto fail = [&]
	{
		(void)device.Release( layout.Value(), CompletionToken() );
		return foundation::MakeUnexpected( CullStatus::kDevice );
	};
	// The kernel in the device's artifact format; a suite's seeded kernel
	// replaces the core one, on a SPIR-V device only.
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !code.empty() && !artifacts.ReplaceSpirv( source, code, device.Facts().artifactFormat ) )
		return fail();
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { source }, PipelineKind::kCompute );
	recipe.layouts = { {}, {}, {}, layout.Value() };
	recipe.debugName = debugName;
	auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
	if ( !resolved )
		return fail();
	auto pipeline = device.CreatePipeline( resolved.Value().Desc() );
	if ( !pipeline )
		return fail();
	return std::make_pair( layout.Value(), pipeline.Value() );
}

// `count` storage buffers at bindings 0..count-1.
std::vector<BindingDesc> StorageBindings( std::uint32_t count )
{
	std::vector<BindingDesc> bindings( count );
	for ( std::uint32_t b = 0; b < count; ++b )
		bindings[b] = { b, BindingKind::kStorageBuffer, 1, { ShaderStage::kCompute } };
	return bindings;
}

} // namespace

std::vector<CullInstance> PackInstances( const scene::SceneSnapshot &snapshot )
{
	std::vector<CullInstance> out( snapshot.instances.size() );
	for ( std::size_t i = 0; i < out.size(); ++i )
	{
		const scene::MeshInstance &instance = snapshot.instances[i];
		const math::Aabb &bounds = instance.worldBounds;
		out[i].min[0] = bounds.min.x;
		out[i].min[1] = bounds.min.y;
		out[i].min[2] = bounds.min.z;
		out[i].max[0] = bounds.max.x;
		out[i].max[1] = bounds.max.y;
		out[i].max[2] = bounds.max.z;
		out[i].viewMask = instance.desc.viewMask;
	}
	return out;
}

CullView PackView( const scene::SceneView &view, std::uint32_t count )
{
	return PackView( view.frustum, view.desc.viewBit, count );
}

CullView PackView( const math::Frustum &frustum, std::uint32_t viewBit, std::uint32_t count )
{
	CullView out;
	for ( int p = 0; p < 6; ++p )
	{
		const math::Plane &plane = frustum.planes[p];
		out.planes[p][0] = plane.normal.x;
		out.planes[p][1] = plane.normal.y;
		out.planes[p][2] = plane.normal.z;
		out.planes[p][3] = plane.d;
	}
	out.viewBit = viewBit;
	out.count = count;
	return out;
}

CullKernel::CullKernel( IRenderDevice2 &device ) : m_Device( device )
{
}

CullKernel::~CullKernel()
{
	// Collect has released every recorded dispatch's bind group behind its
	// token; the pipeline and layout follow the last one.
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

foundation::Expected<std::unique_ptr<CullKernel>, CullStatus> CullKernel::Create(
    IRenderDevice2 &device, std::span<const std::uint32_t> code )
{
	auto made = CreateKernel( device, "render/culling/cull.comp", StorageBindings( kBindingCount ),
	    "render.culling", code );
	if ( !made )
		return foundation::MakeUnexpected( made.Error() );
	std::unique_ptr<CullKernel> kernel( new CullKernel( device ) );
	kernel->m_Layout = made.Value().first;
	kernel->m_Pipeline = made.Value().second;
	return kernel;
}

foundation::Expected<void, CullStatus> CullKernel::Record(
    CommandEncoder &encoder, const CullBuffers &buffers )
{
	if ( buffers.count == 0 )
		return {};
	const BindGroupEntry entries[kBindingCount] = {
	    { kInstances, buffers.instances, 0, std::uint64_t( buffers.count ) * sizeof( CullInstance ),
	        {}, {} },
	    { kView, buffers.view, 0, sizeof( CullView ), {}, {} },
	    { kVisibility, buffers.visibility, 0,
	        std::uint64_t( MaskWords( buffers.count ) ) * sizeof( std::uint32_t ), {}, {} },
	};
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( CullStatus::kDevice );
	}
	m_Pending.push_back( group.Value() );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Dispatch( ( MaskWords( buffers.count ) + 63 ) / 64 );
	return {};
}

void CullKernel::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

void AddCullPass(
    graph::GraphBuilder &builder, CullKernel &kernel, const CullPassResources &resources )
{
	graph::PassBuilder pass = builder.AddPass( "cull", graph::PassKind::kCompute );
	if ( resources.asyncCompute )
		pass.OnQueue( graph::Queue::kAsyncCompute );
	pass.Read( resources.instances, ResourceUsage::kStorageRead );
	pass.Read( resources.view, ResourceUsage::kStorageRead );
	pass.Write( resources.visibility, ResourceUsage::kStorageWrite );
	pass.Execute(
	    [&kernel, resources]( graph::RecordContext &context )
	    {
		    CullBuffers buffers;
		    buffers.instances = context.Buffer( resources.instances );
		    buffers.view = context.Buffer( resources.view );
		    buffers.visibility = context.Buffer( resources.visibility );
		    buffers.count = resources.count;
		    (void)kernel.Record( context.Encoder(), buffers );
	    } );
}

} // namespace render::culling

namespace render::culling
{

std::vector<std::vector<device::DrawIndexedIndirectCommand>> CompactReference(
    std::span<const std::uint32_t> mask, std::span<const DrawTemplate> templates,
    std::span<const DrawBucket> buckets, std::uint32_t count )
{
	std::vector<std::vector<DrawIndexedIndirectCommand>> out( buckets.size() );
	for ( std::size_t b = 0; b < buckets.size(); ++b )
	{
		const std::uint32_t end = std::min( buckets[b].first + buckets[b].count, count );
		for ( std::uint32_t i = buckets[b].first; i < end && i < templates.size(); ++i )
		{
			if ( i / 32 < mask.size() && ( ( mask[i / 32] >> ( i % 32 ) ) & 1u ) )
			{
				const DrawTemplate &t = templates[i];
				out[b].push_back( { t.indexCount, 1, t.firstIndex, t.vertexOffset, i } );
			}
		}
	}
	return out;
}

CompactKernel::CompactKernel( IRenderDevice2 &device ) : m_Device( device )
{
}

CompactKernel::~CompactKernel()
{
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

foundation::Expected<std::unique_ptr<CompactKernel>, CullStatus> CompactKernel::Create(
    IRenderDevice2 &device, std::span<const std::uint32_t> code )
{
	auto made = CreateKernel( device, "render/culling/compact.comp",
	    StorageBindings( kCompactBindingCount ), "render.culling.compact", code );
	if ( !made )
		return foundation::MakeUnexpected( made.Error() );
	std::unique_ptr<CompactKernel> kernel( new CompactKernel( device ) );
	kernel->m_Layout = made.Value().first;
	kernel->m_Pipeline = made.Value().second;
	return kernel;
}

foundation::Expected<void, CullStatus> CompactKernel::Record(
    CommandEncoder &encoder, const CompactBuffers &buffers )
{
	if ( buffers.count > kMaxCompactInstances )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( CullStatus::kDevice );
	}
	// Zero instances still write zero draw counts.
	const std::uint32_t words = std::max( MaskWords( buffers.count ), 1u );
	const std::uint32_t templates = std::max( buffers.count, 1u );
	const std::uint32_t bucketCount = std::max( buffers.bucketCount, 1u );
	const BindGroupEntry entries[kCompactBindingCount] = {
	    { kCompactVisibility, buffers.visibility, 0, std::uint64_t( words ) * 4, {}, {} },
	    { kCompactTemplates, buffers.templates, 0,
	        std::uint64_t( templates ) * sizeof( DrawTemplate ), {}, {} },
	    { kCompactView, buffers.view, 0, sizeof( CullView ), {}, {} },
	    { kCompactBuckets, buffers.buckets, 0, std::uint64_t( bucketCount ) * sizeof( DrawBucket ),
	        {}, {} },
	    { kCompactCommands, buffers.commands, 0, CommandBufferBytes( buffers.count, bucketCount ),
	        {}, {} },
	};
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( CullStatus::kDevice );
	}
	m_Pending.push_back( group.Value() );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	const std::uint32_t constants[2] = {
	    buffers.bucketCount, static_cast<std::uint32_t>( CommandsOffset( bucketCount ) / 4 ) };
	encoder.SetDrawConstants( 0, std::as_bytes( std::span( constants ) ) );
	encoder.Dispatch( 1 );
	return {};
}

void CompactKernel::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

void AddCompactPass(
    graph::GraphBuilder &builder, CompactKernel &kernel, const CompactPassResources &resources )
{
	graph::PassBuilder pass = builder.AddPass( "cull-compact", graph::PassKind::kCompute );
	if ( resources.asyncCompute )
		pass.OnQueue( graph::Queue::kAsyncCompute );
	pass.Read( resources.visibility, ResourceUsage::kStorageRead );
	pass.Read( resources.templates, ResourceUsage::kStorageRead );
	pass.Read( resources.view, ResourceUsage::kStorageRead );
	pass.Read( resources.buckets, ResourceUsage::kStorageRead );
	pass.Write( resources.commands, ResourceUsage::kStorageWrite );
	pass.Execute(
	    [&kernel, resources]( graph::RecordContext &context )
	    {
		    CompactBuffers buffers;
		    buffers.visibility = context.Buffer( resources.visibility );
		    buffers.templates = context.Buffer( resources.templates );
		    buffers.view = context.Buffer( resources.view );
		    buffers.buckets = context.Buffer( resources.buckets );
		    buffers.commands = context.Buffer( resources.commands );
		    buffers.count = resources.count;
		    buffers.bucketCount = resources.bucketCount;
		    (void)kernel.Record( context.Encoder(), buffers );
	    } );
}

} // namespace render::culling

namespace render::culling
{

namespace
{

constexpr float kMinW = 1e-5f;

std::pair<std::uint32_t, std::uint32_t> LevelSize( const OcclusionView &view, std::uint32_t level )
{
	std::uint32_t w = view.width;
	std::uint32_t h = view.height;
	for ( std::uint32_t l = 0; l < level; ++l )
	{
		w = std::max( ( w + 1 ) / 2, 1u );
		h = std::max( ( h + 1 ) / 2, 1u );
	}
	return { w, h };
}

} // namespace

OcclusionView MakeOcclusionView( const math::float4x4 &viewProjection, std::uint32_t width,
    std::uint32_t height, std::uint32_t count )
{
	OcclusionView view;
	for ( int r = 0; r < 4; ++r )
	{
		view.viewProjection[r][0] = viewProjection.rows[r].x;
		view.viewProjection[r][1] = viewProjection.rows[r].y;
		view.viewProjection[r][2] = viewProjection.rows[r].z;
		view.viewProjection[r][3] = viewProjection.rows[r].w;
	}
	view.width = std::max( width, 1u );
	view.height = std::max( height, 1u );
	view.count = count;
	std::uint32_t offset = 0;
	std::uint32_t w = view.width;
	std::uint32_t h = view.height;
	for ( view.levels = 0; view.levels < kMaxPyramidLevels; )
	{
		view.levelOffset[view.levels++] = offset;
		offset += w * h;
		if ( w == 1 && h == 1 )
			break;
		w = std::max( ( w + 1 ) / 2, 1u );
		h = std::max( ( h + 1 ) / 2, 1u );
	}
	return view;
}

std::uint64_t PyramidFloats( const OcclusionView &view )
{
	const auto [w, h] = LevelSize( view, view.levels - 1 );
	return std::uint64_t( view.levelOffset[view.levels - 1] ) + std::uint64_t( w ) * h;
}

std::vector<float> DepthPyramidReference( std::span<const float> depth, const OcclusionView &view )
{
	std::vector<float> out( PyramidFloats( view ), 0.0f );
	for ( std::uint32_t i = 0; i < view.width * view.height && i < depth.size(); ++i )
		out[i] = depth[i];
	for ( std::uint32_t level = 1; level < view.levels; ++level )
	{
		const auto [bw, bh] = LevelSize( view, level - 1 );
		const auto [w, h] = LevelSize( view, level );
		const float *below = out.data() + view.levelOffset[level - 1];
		float *here = out.data() + view.levelOffset[level];
		for ( std::uint32_t y = 0; y < h; ++y )
		{
			for ( std::uint32_t x = 0; x < w; ++x )
			{
				float d = 0.0f;
				for ( std::uint32_t dy = 0; dy < 2; ++dy )
					for ( std::uint32_t dx = 0; dx < 2; ++dx )
					{
						const std::uint32_t ax = x * 2 + dx;
						const std::uint32_t ay = y * 2 + dy;
						if ( ax < bw && ay < bh )
							d = std::max( d, below[ay * bw + ax] );
					}
				here[y * w + x] = d;
			}
		}
	}
	return out;
}

std::vector<std::uint32_t> OcclusionReference( std::span<const CullInstance> instances,
    std::span<const std::uint32_t> frustumMask, std::span<const float> pyramid,
    const OcclusionView &view )
{
	const auto &m = view.viewProjection;
	auto occluded = [&]( const CullInstance &instance )
	{
		if ( instance.flags & kCullNeverOcclude )
			return false;
		float minU = 1e30f, minV = 1e30f, maxU = -1e30f, maxV = -1e30f, nearZ = 1e30f;
		for ( std::uint32_t c = 0; c < 8; ++c )
		{
			const float p[4] = { ( c & 1 ) ? instance.max[0] : instance.min[0],
			    ( c & 2 ) ? instance.max[1] : instance.min[1],
			    ( c & 4 ) ? instance.max[2] : instance.min[2], 1.0f };
			float clip[4];
			for ( int r = 0; r < 4; ++r )
				clip[r] = m[r][0] * p[0] + m[r][1] * p[1] + m[r][2] * p[2] + m[r][3] * p[3];
			if ( clip[3] <= kMinW )
				return false;
			const float u = ( clip[0] / clip[3] ) * 0.5f + 0.5f;
			const float v = 0.5f - ( clip[1] / clip[3] ) * 0.5f;
			minU = std::min( minU, u );
			maxU = std::max( maxU, u );
			minV = std::min( minV, v );
			maxV = std::max( maxV, v );
			nearZ = std::min( nearZ, clip[2] / clip[3] );
		}
		if ( nearZ <= 0.0f )
			return false;
		auto texel = [&]( float t, std::uint32_t size )
		{
			return std::min(
			    static_cast<std::uint32_t>( std::clamp( t, 0.0f, 1.0f ) * float( size ) ),
			    size - 1 );
		};
		std::uint32_t x0 = texel( minU, view.width ), x1 = texel( maxU, view.width );
		std::uint32_t y0 = texel( minV, view.height ), y1 = texel( maxV, view.height );
		std::uint32_t level = 0;
		while ( ( x1 - x0 > 1 || y1 - y0 > 1 ) && level + 1 < view.levels )
		{
			x0 >>= 1;
			x1 >>= 1;
			y0 >>= 1;
			y1 >>= 1;
			++level;
		}
		const auto [w, h] = LevelSize( view, level );
		(void)h;
		float farthest = 0.0f;
		for ( std::uint32_t y = y0; y <= y1; ++y )
			for ( std::uint32_t x = x0; x <= x1; ++x )
				farthest = std::max( farthest, pyramid[view.levelOffset[level] + y * w + x] );
		return nearZ > farthest;
	};
	std::vector<std::uint32_t> out( MaskWords( view.count ), 0 );
	for ( std::uint32_t i = 0; i < view.count && i < instances.size(); ++i )
	{
		const bool kept =
		    i / 32 < frustumMask.size() && ( ( frustumMask[i / 32] >> ( i % 32 ) ) & 1u );
		if ( kept && !occluded( instances[i] ) )
			out[i / 32] |= 1u << ( i % 32 );
	}
	return out;
}

OcclusionKernels::OcclusionKernels( IRenderDevice2 &device ) : m_Device( device )
{
}

OcclusionKernels::~OcclusionKernels()
{
	for ( PipelineId pipeline : { m_PyramidPipeline, m_OcclusionPipeline } )
		if ( pipeline.IsValid() )
			(void)m_Device.Release( pipeline, m_LastToken );
	for ( BindGroupLayoutId layout : { m_PyramidLayout, m_OcclusionLayout } )
		if ( layout.IsValid() )
			(void)m_Device.Release( layout, m_LastToken );
}

foundation::Expected<std::unique_ptr<OcclusionKernels>, CullStatus> OcclusionKernels::Create(
    IRenderDevice2 &device, std::span<const std::uint32_t> occlusionCode )
{
	const ShaderStageSet compute = { ShaderStage::kCompute };
	const BindingDesc pyramid[] = { { 0, BindingKind::kSampledTexture, 1, compute },
	    { 1, BindingKind::kSampler, 1, compute }, { 2, BindingKind::kStorageBuffer, 1, compute },
	    { 3, BindingKind::kStorageBuffer, 1, compute } };
	auto pyramidKernel =
	    CreateKernel( device, "render/culling/hiz.comp", pyramid, "render.culling.hiz", {} );
	if ( !pyramidKernel )
		return foundation::MakeUnexpected( pyramidKernel.Error() );
	std::unique_ptr<OcclusionKernels> kernels( new OcclusionKernels( device ) );
	kernels->m_PyramidLayout = pyramidKernel.Value().first;
	kernels->m_PyramidPipeline = pyramidKernel.Value().second;
	auto occlusion = CreateKernel( device, "render/culling/occlusion.comp", StorageBindings( 5 ),
	    "render.culling.occlusion", occlusionCode );
	if ( !occlusion )
		return foundation::MakeUnexpected( occlusion.Error() );
	kernels->m_OcclusionLayout = occlusion.Value().first;
	kernels->m_OcclusionPipeline = occlusion.Value().second;
	return kernels;
}

foundation::Expected<void, CullStatus> OcclusionKernels::RecordPyramid(
    CommandEncoder &encoder, const HiZBuffers &buffers, const OcclusionView &view )
{
	const BindGroupEntry entries[] = { { 0, {}, 0, 0, buffers.depth, {} },
	    { 1, {}, 0, 0, {}, buffers.sampler },
	    { 2, buffers.view, 0, sizeof( OcclusionView ), {}, {} },
	    { 3, buffers.pyramid, 0, PyramidFloats( view ) * sizeof( float ), {}, {} } };
	auto group = m_Device.CreateBindGroup( { m_PyramidLayout, entries } );
	if ( !group )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( CullStatus::kDevice );
	}
	m_Pending.push_back( group.Value() );
	encoder.SetPipeline( m_PyramidPipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	for ( std::uint32_t level = 0; level < view.levels; ++level )
	{
		const auto [w, h] = LevelSize( view, level );
		encoder.SetDrawConstants( 0, std::as_bytes( std::span( &level, 1 ) ) );
		encoder.Dispatch( ( w + 7 ) / 8, ( h + 7 ) / 8 );
	}
	return {};
}

foundation::Expected<void, CullStatus> OcclusionKernels::RecordOcclusion(
    CommandEncoder &encoder, const OcclusionBuffers &buffers )
{
	if ( buffers.count == 0 )
		return {};
	const std::uint64_t maskBytes = std::uint64_t( MaskWords( buffers.count ) ) * 4;
	const BindGroupEntry entries[] = {
	    { 0, buffers.instances, 0, std::uint64_t( buffers.count ) * sizeof( CullInstance ), {},
	        {} },
	    { 1, buffers.view, 0, sizeof( OcclusionView ), {}, {} },
	    { 2, buffers.pyramid, 0, 0, {}, {} }, { 3, buffers.frustum, 0, maskBytes, {}, {} },
	    { 4, buffers.visibility, 0, maskBytes, {}, {} } };
	auto group = m_Device.CreateBindGroup( { m_OcclusionLayout, entries } );
	if ( !group )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( CullStatus::kDevice );
	}
	m_Pending.push_back( group.Value() );
	encoder.SetPipeline( m_OcclusionPipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Dispatch( ( MaskWords( buffers.count ) + 63 ) / 64 );
	return {};
}

void OcclusionKernels::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

void AddOcclusionPass(
    graph::GraphBuilder &builder, OcclusionKernels &kernels, const OcclusionPassResources &r )
{
	graph::PassBuilder pass = builder.AddPass( "cull-occlusion", graph::PassKind::kCompute );
	if ( r.asyncCompute )
		pass.OnQueue( graph::Queue::kAsyncCompute );
	pass.Read( r.depth, ResourceUsage::kSampled );
	pass.Read( r.instances, ResourceUsage::kStorageRead );
	pass.Read( r.view, ResourceUsage::kStorageRead );
	pass.Read( r.frustum, ResourceUsage::kStorageRead );
	// Written by the pyramid dispatches and read by the test, in one usage:
	// the adapter orders dispatches on one resource (render.device.v2).
	pass.Write( r.pyramid, ResourceUsage::kStorageWrite );
	pass.Write( r.visibility, ResourceUsage::kStorageWrite );
	pass.Execute(
	    [&kernels, r]( graph::RecordContext &context )
	    {
		    HiZBuffers hiz;
		    hiz.depth = context.Texture( r.depth );
		    hiz.sampler = r.sampler;
		    hiz.view = context.Buffer( r.view );
		    hiz.pyramid = context.Buffer( r.pyramid );
		    (void)kernels.RecordPyramid( context.Encoder(), hiz, r.occlusionView );
		    OcclusionBuffers buffers;
		    buffers.instances = context.Buffer( r.instances );
		    buffers.view = context.Buffer( r.view );
		    buffers.pyramid = context.Buffer( r.pyramid );
		    buffers.frustum = context.Buffer( r.frustum );
		    buffers.visibility = context.Buffer( r.visibility );
		    buffers.count = r.occlusionView.count;
		    (void)kernels.RecordOcclusion( context.Encoder(), buffers );
	    } );
}

} // namespace render::culling
