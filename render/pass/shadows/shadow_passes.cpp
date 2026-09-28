//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.shadows on the device (RFC 0016 K7); see
//			shadow_passes.h.
//
//=============================================================================//

#include "render/pass/shadows/shadow_passes.h"

#include "render/graph/executor.h"
#include "spv/shadow_spv.h"

#include <algorithm>
#include <utility>

namespace render::pass::shadows
{

namespace
{

using namespace render::device;

void Store( const math::float4x4 &m, float out[4][4] )
{
	for ( int r = 0; r < 4; ++r )
	{
		out[r][0] = m.rows[r].x;
		out[r][1] = m.rows[r].y;
		out[r][2] = m.rows[r].z;
		out[r][3] = m.rows[r].w;
	}
}

// a * b in double, rounded once: clip = viewProjection * world.
void Compose( const math::float4x4 &a, const math::float4x4 &b, float out[4][4] )
{
	for ( int r = 0; r < 4; ++r )
	{
		const float *row = &a.rows[r].x;
		for ( int c = 0; c < 4; ++c )
		{
			double sum = 0.0;
			for ( int k = 0; k < 4; ++k )
				sum += double( row[k] ) * double( ( &b.rows[k].x )[c] );
			out[r][c] = static_cast<float>( sum );
		}
	}
}

struct Matrix
{
	float m[4][4];
};

struct Draw
{
	resources::MeshEntry mesh;
	PipelineId pipeline;
	std::uint32_t instance = 0;
};

// Imports each mesh buffer once, in its residency usage.
class MeshImports
{
public:
	void Add( graph::GraphBuilder &builder, const resources::MeshEntry &mesh )
	{
		Import( builder, mesh.vertices, ResourceUsage::kVertex );
		if ( mesh.indices.IsValid() )
			Import( builder, mesh.indices, ResourceUsage::kIndex );
	}
	void Declare( graph::PassBuilder &pass ) const
	{
		for ( const auto &[key, entry] : m_Refs )
			pass.Read( entry.first, entry.second );
	}

private:
	void Import( graph::GraphBuilder &builder, BufferId buffer, ResourceUsage usage )
	{
		const auto key = std::make_pair( buffer.value, static_cast<int>( usage ) );
		if ( m_Refs.count( key ) )
			return;
		BufferDesc desc;
		desc.usages = { usage };
		m_Refs[key] = { builder.ImportBuffer( "mesh", buffer, desc, usage, usage ), usage };
	}
	std::map<std::pair<std::uint64_t, int>, std::pair<graph::ResourceRef, ResourceUsage>> m_Refs;
};

void RecordDraw( CommandEncoder &encoder, const Draw &draw )
{
	encoder.SetVertexBuffer( 0, draw.mesh.vertices );
	if ( draw.mesh.indices.IsValid() )
	{
		encoder.SetIndexBuffer( draw.mesh.indices, 0, draw.mesh.indexFormat );
		encoder.DrawIndexed( draw.mesh.indexCount, 1, 0, 0, draw.instance );
	}
	else
	{
		encoder.Draw( draw.mesh.vertexCount, 1, 0, draw.instance );
	}
}

} // namespace

ShadowTileGpu PackShadowTile(
    const ShadowTileProjection &projection, std::uint32_t atlasSize, float depthBias )
{
	ShadowTileGpu tile;
	Store( projection.viewProjection, tile.viewProjection );
	tile.transform[0] = projection.transform.scaleU;
	tile.transform[1] = projection.transform.biasU;
	tile.transform[2] = projection.transform.scaleV;
	tile.transform[3] = projection.transform.biasV;
	tile.bounds[0] = projection.u0;
	tile.bounds[1] = projection.v0;
	tile.bounds[2] = projection.u1;
	tile.bounds[3] = projection.v1;
	tile.params[0] = depthBias;
	tile.params[1] = float( atlasSize );
	return tile;
}

// ---------------------------------------------------------------------------
// Caster depth

foundation::Expected<std::unique_ptr<ShadowDepthRenderer>, ShadowPassStatus>
ShadowDepthRenderer::Create( IRenderDevice2 &device, Format depthFormat )
{
	if ( !IsDepthFormat( depthFormat ) )
		return foundation::MakeUnexpected( ShadowPassStatus::kInvalidTarget );
	std::unique_ptr<ShadowDepthRenderer> renderer( new ShadowDepthRenderer( device ) );
	renderer->m_DepthFormat = depthFormat;
	static const BindingDesc draw[] = {
	    { 0, BindingKind::kStorageBuffer, 1, { ShaderStage::kVertex } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( !layout )
		return foundation::MakeUnexpected( ShadowPassStatus::kDevice );
	renderer->m_DrawLayout = layout.Value();
	return renderer;
}

ShadowDepthRenderer::~ShadowDepthRenderer()
{
	for ( const auto &[stride, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, m_LastToken );
	if ( m_DrawLayout.IsValid() )
		(void)m_Device.Release( m_DrawLayout, m_LastToken );
}

foundation::Expected<PipelineId, ShadowPassStatus> ShadowDepthRenderer::PipelineFor(
    std::uint32_t stride )
{
	if ( auto found = m_Pipelines.find( stride ); found != m_Pipelines.end() )
		return found->second;
	static const ReflectedBinding bindings[] = {
	    { static_cast<std::uint32_t>( BindGroupRole::kDraw ), 0, BindingKind::kStorageBuffer } };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	    std::as_bytes( std::span( spirv::kShadowDepthVertex ) ), "main", bindings } };
	const BindGroupLayoutId layouts[kMaxBindGroups] = { {}, {}, {}, m_DrawLayout };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 } };
	const VertexBufferLayout buffers[] = { { stride, false } };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.vertex = { attributes, buffers };
	desc.raster.cull = CullMode::kNone;
	desc.depthStencil = { true, true, CompareOp::kLess };
	desc.depthFormat = m_DepthFormat;
	desc.debugName = "render.pass.shadows.depth";
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( ShadowPassStatus::kDevice );
	m_Pipelines.emplace( stride, pipeline.Value() );
	return pipeline.Value();
}

foundation::Expected<ShadowDepthStats, ShadowPassStatus> ShadowDepthRenderer::AddPasses(
    graph::GraphBuilder &builder, const ShadowAtlasTarget &target,
    std::span<const ShadowDepthView> views )
{
	if ( !target.atlas.IsValid() || target.atlasSize == 0 )
		return foundation::MakeUnexpected( ShadowPassStatus::kInvalidTarget );
	struct Frame
	{
		std::vector<Matrix> clips;
		std::vector<Draw> draws;
		std::vector<std::pair<ShadowViewport, std::pair<std::size_t, std::size_t>>> views;
	};
	auto frame = std::make_shared<Frame>();
	MeshImports imports;
	ShadowDepthStats stats;
	for ( const ShadowDepthView &view : views )
	{
		if ( view.tile.size <= 2 * target.guardTexels ||
		     std::uint64_t( view.tile.x ) + view.tile.size > target.atlasSize ||
		     std::uint64_t( view.tile.y ) + view.tile.size > target.atlasSize )
			return foundation::MakeUnexpected( ShadowPassStatus::kInvalidTarget );
		const std::size_t first = frame->draws.size();
		for ( const ShadowCaster &caster : view.casters )
		{
			auto pipeline = PipelineFor( caster.mesh.vertexStride );
			if ( !pipeline )
				return foundation::MakeUnexpected( pipeline.Error() );
			Matrix clip;
			Compose( view.viewProjection, caster.world, clip.m );
			frame->draws.push_back( { caster.mesh, pipeline.Value(),
			    static_cast<std::uint32_t>( frame->clips.size() ) } );
			frame->clips.push_back( clip );
			imports.Add( builder, caster.mesh );
		}
		frame->views.push_back(
		    { TileViewport( view.tile, target.guardTexels ), { first, frame->draws.size() } } );
		++stats.views;
	}
	stats.draws = static_cast<std::uint32_t>( frame->draws.size() );

	BufferDesc clipDesc;
	clipDesc.size = std::max<std::size_t>( frame->clips.size(), 1 ) * sizeof( Matrix );
	const graph::ResourceRef clipBuffer = builder.CreateBuffer( "shadow-casters", clipDesc );
	builder.AddPass( "shadow-depth-upload", graph::PassKind::kCopy )
	    .Write( clipBuffer, ResourceUsage::kCopyDestination )
	    .Execute(
	        [frame, clipBuffer]( graph::RecordContext &context )
	        {
		        if ( !frame->clips.empty() )
			        context.Encoder().WriteBuffer( context.Buffer( clipBuffer ), 0,
			            std::as_bytes( std::span<const Matrix>( frame->clips ) ) );
	        } );

	graph::PassBuilder pass = builder.AddPass( "shadow-depth", graph::PassKind::kRender );
	pass.Read( clipBuffer, ResourceUsage::kStorageRead )
	    .Write( target.atlas, ResourceUsage::kDepthWrite );
	imports.Declare( pass );
	pass.Execute(
	    [this, frame, clipBuffer, target]( graph::RecordContext &context )
	    {
		    const BindGroupEntry entry[] = { { 0, context.Buffer( clipBuffer ), 0, 0, {}, {} } };
		    auto group = m_Device.CreateBindGroup( { m_DrawLayout, entry } );
		    {
			    std::lock_guard<std::mutex> lock( m_PendingLock );
			    if ( !group )
			    {
				    ++m_RecordFailures;
				    return;
			    }
			    m_Pending.push_back( group.Value() );
		    }
		    CommandEncoder &encoder = context.Encoder();
		    DepthAttachment depth;
		    depth.texture = context.Texture( target.atlas );
		    depth.clearDepth = 1.0f;
		    RenderingDesc rendering;
		    rendering.depth = depth;
		    rendering.width = target.atlasSize;
		    rendering.height = target.atlasSize;
		    encoder.BeginRendering( rendering );
		    for ( const auto &[viewport, range] : frame->views )
		    {
			    encoder.SetViewport( { float( viewport.x ), float( viewport.y ),
			        float( viewport.size ), float( viewport.size ), 0.0f, 1.0f } );
			    PipelineId bound;
			    for ( std::size_t d = range.first; d < range.second; ++d )
			    {
				    const Draw &draw = frame->draws[d];
				    if ( draw.pipeline != bound )
				    {
					    encoder.SetPipeline( draw.pipeline );
					    encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
					    bound = draw.pipeline;
				    }
				    RecordDraw( encoder, draw );
			    }
		    }
		    encoder.EndRendering();
	    } );
	return stats;
}

void ShadowDepthRenderer::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

std::uint32_t ShadowDepthRenderer::RecordFailures() const
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	return m_RecordFailures;
}

// ---------------------------------------------------------------------------
// Receivers

foundation::Expected<std::unique_ptr<ShadowReceiverRenderer>, ShadowPassStatus>
ShadowReceiverRenderer::Create(
    IRenderDevice2 &device, Format colorFormat, std::span<const std::uint32_t> fragmentCode )
{
	std::unique_ptr<ShadowReceiverRenderer> renderer( new ShadowReceiverRenderer( device ) );
	renderer->m_ColorFormat = colorFormat;
	if ( fragmentCode.empty() )
		fragmentCode = spirv::kShadowReceiverFragment;
	renderer->m_Fragment.assign( fragmentCode.begin(), fragmentCode.end() );
	SamplerDesc point;
	point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
	point.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( point );
	static const BindingDesc frame[] = {
	    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	static const BindingDesc view[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } } };
	static const BindingDesc draw[] = {
	    { 0, BindingKind::kStorageBuffer, 1, { ShaderStage::kVertex } } };
	auto frameLayout = device.CreateBindGroupLayout( { BindGroupRole::kFrame, frame } );
	auto viewLayout = device.CreateBindGroupLayout( { BindGroupRole::kView, view } );
	auto drawLayout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( sampler )
		renderer->m_Sampler = sampler.Value();
	if ( frameLayout )
		renderer->m_FrameLayout = frameLayout.Value();
	if ( viewLayout )
		renderer->m_ViewLayout = viewLayout.Value();
	if ( drawLayout )
		renderer->m_DrawLayout = drawLayout.Value();
	if ( !sampler || !frameLayout || !viewLayout || !drawLayout )
		return foundation::MakeUnexpected( ShadowPassStatus::kDevice );
	return renderer;
}

ShadowReceiverRenderer::~ShadowReceiverRenderer()
{
	for ( const auto &[stride, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, m_LastToken );
	for ( BindGroupLayoutId layout : { m_FrameLayout, m_ViewLayout, m_DrawLayout } )
	{
		if ( layout.IsValid() )
			(void)m_Device.Release( layout, m_LastToken );
	}
	if ( m_Sampler.IsValid() )
		(void)m_Device.Release( m_Sampler, m_LastToken );
}

foundation::Expected<PipelineId, ShadowPassStatus> ShadowReceiverRenderer::PipelineFor(
    std::uint32_t stride )
{
	if ( auto found = m_Pipelines.find( stride ); found != m_Pipelines.end() )
		return found->second;
	constexpr std::uint32_t kFrame = static_cast<std::uint32_t>( BindGroupRole::kFrame );
	constexpr std::uint32_t kView = static_cast<std::uint32_t>( BindGroupRole::kView );
	constexpr std::uint32_t kDraw = static_cast<std::uint32_t>( BindGroupRole::kDraw );
	static const ReflectedBinding vertexBindings[] = {
	    { kView, 0, BindingKind::kUniformBuffer }, { kDraw, 0, BindingKind::kStorageBuffer } };
	static const ReflectedBinding fragmentBindings[] = {
	    { kFrame, 0, BindingKind::kSampledTexture }, { kFrame, 1, BindingKind::kSampler },
	    { kView, 0, BindingKind::kUniformBuffer } };
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kShadowReceiverVertex ) ), "main", vertexBindings },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span<const std::uint32_t>( m_Fragment ) ), "main",
	        fragmentBindings } };
	const BindGroupLayoutId layouts[kMaxBindGroups] = {
	    m_FrameLayout, m_ViewLayout, {}, m_DrawLayout };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 } };
	const VertexBufferLayout buffers[] = { { stride, false } };
	const Format colors[] = { m_ColorFormat };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.vertex = { attributes, buffers };
	desc.raster.cull = CullMode::kNone;
	desc.colorFormats = colors;
	desc.debugName = "render.pass.shadows.receiver";
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( ShadowPassStatus::kDevice );
	m_Pipelines.emplace( stride, pipeline.Value() );
	return pipeline.Value();
}

foundation::Expected<std::uint32_t, ShadowPassStatus> ShadowReceiverRenderer::AddPasses(
    graph::GraphBuilder &builder, graph::ResourceRef atlas, const ShadowReceiverView &view,
    const ShadowReceiverLight &light, std::span<const ShadowReceiver> receivers,
    const ShadowReceiverTargets &targets )
{
	if ( !atlas.IsValid() || !targets.color.IsValid() || targets.width == 0 || targets.height == 0 )
		return foundation::MakeUnexpected( ShadowPassStatus::kInvalidTarget );
	if ( light.tileCount == 0 || light.tileCount > 4 )
		return foundation::MakeUnexpected( ShadowPassStatus::kInvalidLight );
	struct Frame
	{
		ShadowReceiverViewGpu record;
		std::vector<Matrix> worlds;
		std::vector<Draw> draws;
	};
	auto frame = std::make_shared<Frame>();
	ShadowReceiverViewGpu &record = frame->record;
	Store( view.viewProjection, record.viewProjection );
	Store( view.view, record.view );
	const bool spot = light.kind == ShadowReceiverLight::Kind::kSpot;
	record.lightPositionKind[0] = light.position.x;
	record.lightPositionKind[1] = light.position.y;
	record.lightPositionKind[2] = light.position.z;
	record.lightPositionKind[3] = spot ? 0.0f : 1.0f;
	record.lightAxisCos[0] = light.axis.x;
	record.lightAxisCos[1] = light.axis.y;
	record.lightAxisCos[2] = light.axis.z;
	record.lightAxisCos[3] = light.outerCos;
	record.lightRange[0] = light.range;
	record.lightRange[1] = float( light.tileCount );
	record.lightRange[2] = light.ambient;
	for ( std::uint32_t i = 0; i < 4; ++i )
	{
		record.cascadeSplits[i] = light.splitFar[i];
		record.tiles[i] = light.tiles[i];
	}

	MeshImports imports;
	for ( const ShadowReceiver &receiver : receivers )
	{
		auto pipeline = PipelineFor( receiver.mesh.vertexStride );
		if ( !pipeline )
			return foundation::MakeUnexpected( pipeline.Error() );
		Matrix world;
		Store( receiver.world, world.m );
		frame->draws.push_back( { receiver.mesh, pipeline.Value(),
		    static_cast<std::uint32_t>( frame->worlds.size() ) } );
		frame->worlds.push_back( world );
		imports.Add( builder, receiver.mesh );
	}

	BufferDesc viewDesc;
	viewDesc.size = sizeof( ShadowReceiverViewGpu );
	const graph::ResourceRef viewBuffer = builder.CreateBuffer( "shadow-receiver-view", viewDesc );
	BufferDesc worldDesc;
	worldDesc.size = std::max<std::size_t>( frame->worlds.size(), 1 ) * sizeof( Matrix );
	const graph::ResourceRef worldBuffer =
	    builder.CreateBuffer( "shadow-receiver-worlds", worldDesc );
	builder.AddPass( "shadow-receiver-upload", graph::PassKind::kCopy )
	    .Write( viewBuffer, ResourceUsage::kCopyDestination )
	    .Write( worldBuffer, ResourceUsage::kCopyDestination )
	    .Execute(
	        [frame, viewBuffer, worldBuffer]( graph::RecordContext &context )
	        {
		        context.Encoder().WriteBuffer( context.Buffer( viewBuffer ), 0,
		            std::as_bytes( std::span( &frame->record, 1 ) ) );
		        if ( !frame->worlds.empty() )
			        context.Encoder().WriteBuffer( context.Buffer( worldBuffer ), 0,
			            std::as_bytes( std::span<const Matrix>( frame->worlds ) ) );
	        } );

	graph::PassBuilder pass = builder.AddPass( "shadow-receiver", graph::PassKind::kRender );
	pass.Read( atlas, ResourceUsage::kSampled )
	    .Read( viewBuffer, ResourceUsage::kUniform )
	    .Read( worldBuffer, ResourceUsage::kStorageRead )
	    .Write( targets.color, ResourceUsage::kColorAttachment );
	imports.Declare( pass );
	pass.Execute(
	    [this, frame, atlas, viewBuffer, worldBuffer, targets]( graph::RecordContext &context )
	    {
		    const BindGroupEntry frameEntries[] = {
		        { 0, {}, 0, 0, context.Texture( atlas ), {} }, { 1, {}, 0, 0, {}, m_Sampler } };
		    const BindGroupEntry viewEntry[] = {
		        { 0, context.Buffer( viewBuffer ), 0, 0, {}, {} } };
		    const BindGroupEntry drawEntry[] = {
		        { 0, context.Buffer( worldBuffer ), 0, 0, {}, {} } };
		    auto frameGroup = m_Device.CreateBindGroup( { m_FrameLayout, frameEntries } );
		    auto viewGroup = m_Device.CreateBindGroup( { m_ViewLayout, viewEntry } );
		    auto drawGroup = m_Device.CreateBindGroup( { m_DrawLayout, drawEntry } );
		    {
			    std::lock_guard<std::mutex> lock( m_PendingLock );
			    for ( const auto *group : { &frameGroup, &viewGroup, &drawGroup } )
			    {
				    if ( *group )
					    m_Pending.push_back( group->Value() );
			    }
			    if ( !frameGroup || !viewGroup || !drawGroup )
			    {
				    ++m_RecordFailures;
				    return;
			    }
		    }
		    CommandEncoder &encoder = context.Encoder();
		    ColorAttachment color;
		    color.texture = context.Texture( targets.color );
		    color.clear = targets.clear;
		    const ColorAttachment colors[] = { color };
		    RenderingDesc rendering;
		    rendering.colors = colors;
		    rendering.width = targets.width;
		    rendering.height = targets.height;
		    encoder.BeginRendering( rendering );
		    // Viewport state outlives a rendering scope on the encoder; a depth
		    // pass recorded earlier on it may have left a tile's.
		    encoder.SetViewport(
		        { 0.0f, 0.0f, float( targets.width ), float( targets.height ), 0.0f, 1.0f } );
		    PipelineId bound;
		    for ( const Draw &draw : frame->draws )
		    {
			    if ( draw.pipeline != bound )
			    {
				    encoder.SetPipeline( draw.pipeline );
				    encoder.SetBindGroup( BindGroupRole::kFrame, frameGroup.Value() );
				    encoder.SetBindGroup( BindGroupRole::kView, viewGroup.Value() );
				    encoder.SetBindGroup( BindGroupRole::kDraw, drawGroup.Value() );
				    bound = draw.pipeline;
			    }
			    RecordDraw( encoder, draw );
		    }
		    encoder.EndRendering();
	    } );
	return static_cast<std::uint32_t>( frame->draws.size() );
}

void ShadowReceiverRenderer::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

std::uint32_t ShadowReceiverRenderer::RecordFailures() const
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	return m_RecordFailures;
}

} // namespace render::pass::shadows
