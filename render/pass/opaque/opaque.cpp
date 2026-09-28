//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.opaque (RFC 0016 K5); see opaque.h.
//
//=============================================================================//

#include "render/pass/opaque/opaque.h"

#include "spv/opaque_spv.h"
#include "render/graph/executor.h"

#include <cstring>
#include <span>
#include <utility>

namespace render::pass::opaque
{

namespace
{

using namespace render::device;

struct Draw
{
	resources::MeshEntry mesh;
	PipelineId pipeline;
	std::uint32_t instance = 0;
};

struct Frame
{
	float viewProjection[4][4] = {};
	std::vector<InstanceRecord> instances;
	std::vector<Draw> draws;
};

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

} // namespace

foundation::Expected<std::unique_ptr<OpaqueRenderer>, OpaqueStatus> OpaqueRenderer::Create(
    IRenderDevice2 &device, Format colorFormat, Format depthFormat )
{
	std::unique_ptr<OpaqueRenderer> renderer( new OpaqueRenderer( device ) );
	renderer->m_ColorFormat = colorFormat;
	renderer->m_DepthFormat = depthFormat;
	static const BindingDesc view[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex } } };
	static const BindingDesc draw[] = {
	    { 0, BindingKind::kStorageBuffer, 1, { ShaderStage::kVertex } } };
	auto viewLayout = device.CreateBindGroupLayout( { BindGroupRole::kView, view } );
	auto drawLayout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( !viewLayout || !drawLayout )
		return foundation::MakeUnexpected( OpaqueStatus::kDevice );
	renderer->m_ViewLayout = viewLayout.Value();
	renderer->m_DrawLayout = drawLayout.Value();
	return renderer;
}

OpaqueRenderer::~OpaqueRenderer()
{
	for ( const auto &[stride, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, m_LastToken );
	if ( m_ViewLayout.IsValid() )
		(void)m_Device.Release( m_ViewLayout, m_LastToken );
	if ( m_DrawLayout.IsValid() )
		(void)m_Device.Release( m_DrawLayout, m_LastToken );
}

foundation::Expected<PipelineId, OpaqueStatus> OpaqueRenderer::PipelineFor( std::uint32_t stride )
{
	if ( auto found = m_Pipelines.find( stride ); found != m_Pipelines.end() )
		return found->second;
	static const ReflectedBinding vertexBindings[] = {
	    { static_cast<std::uint32_t>( BindGroupRole::kView ), 0, BindingKind::kUniformBuffer },
	    { static_cast<std::uint32_t>( BindGroupRole::kDraw ), 0, BindingKind::kStorageBuffer } };
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kOpaqueVertex ) ), "main", vertexBindings },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kOpaqueFragment ) ), "main", {} } };
	const BindGroupLayoutId layouts[kMaxBindGroups] = { {}, m_ViewLayout, {}, m_DrawLayout };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 } };
	const VertexBufferLayout buffers[] = { { stride, false } };
	const Format colors[] = { m_ColorFormat };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.vertex = { attributes, buffers };
	desc.raster.cull = CullMode::kNone;
	desc.depthStencil = { true, true, CompareOp::kLessEqual };
	desc.colorFormats = colors;
	desc.depthFormat = m_DepthFormat;
	desc.debugName = "render.pass.opaque";
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( OpaqueStatus::kDevice );
	m_Pipelines.emplace( stride, pipeline.Value() );
	return pipeline.Value();
}

foundation::Expected<OpaqueStats, OpaqueStatus> OpaqueRenderer::AddPasses(
    graph::GraphBuilder &builder, const scene::SceneSnapshot &snapshot, const scene::DrawList &list,
    const scene::SceneView &view, const IMeshResolver &meshes, const IMaterialColors &colors,
    const OpaqueTargets &targets )
{
	if ( !targets.color.IsValid() || !targets.depth.IsValid() || targets.width == 0 ||
	     targets.height == 0 )
		return foundation::MakeUnexpected( OpaqueStatus::kInvalidTargets );
	OpaqueStats stats;
	auto frame = std::make_shared<Frame>();
	Store( view.viewProjection, frame->viewProjection );
	for ( const scene::DrawItem &item : list.items )
	{
		const resources::MeshEntry *mesh = meshes.Mesh( item.mesh );
		InstanceRecord record;
		if ( !mesh || item.instance >= snapshot.instances.size() ||
		     !colors.Color( item.material, record.color ) )
		{
			++stats.unresolved;
			continue;
		}
		auto pipeline = PipelineFor( mesh->vertexStride );
		if ( !pipeline )
			return foundation::MakeUnexpected( pipeline.Error() );
		Store( snapshot.instances[item.instance].desc.world, record.world );
		frame->draws.push_back(
		    { *mesh, pipeline.Value(), static_cast<std::uint32_t>( frame->instances.size() ) } );
		frame->instances.push_back( record );
	}
	stats.drawn = static_cast<std::uint32_t>( frame->draws.size() );

	BufferDesc viewDesc;
	viewDesc.size = sizeof( frame->viewProjection );
	const graph::ResourceRef viewBuffer = builder.CreateBuffer( "opaque-view", viewDesc );
	BufferDesc instanceDesc;
	instanceDesc.size =
	    std::max<std::size_t>( frame->instances.size(), 1 ) * sizeof( InstanceRecord );
	const graph::ResourceRef instanceBuffer =
	    builder.CreateBuffer( "opaque-instances", instanceDesc );

	builder.AddPass( "opaque-upload", graph::PassKind::kCopy )
	    .Write( viewBuffer, ResourceUsage::kCopyDestination )
	    .Write( instanceBuffer, ResourceUsage::kCopyDestination )
	    .Execute(
	        [frame, viewBuffer, instanceBuffer]( graph::RecordContext &context )
	        {
		        context.Encoder().WriteBuffer( context.Buffer( viewBuffer ), 0,
		            std::as_bytes( std::span( &frame->viewProjection[0][0], 16 ) ) );
		        if ( !frame->instances.empty() )
			        context.Encoder().WriteBuffer( context.Buffer( instanceBuffer ), 0,
			            std::as_bytes( std::span<const InstanceRecord>( frame->instances ) ) );
	        } );

	// The meshes' buffers, imported once each in their residency usages.
	std::map<std::uint64_t, graph::ResourceRef> vertexRefs;
	std::map<std::uint64_t, graph::ResourceRef> indexRefs;
	auto import = [&]( BufferId buffer, ResourceUsage usage,
	                  std::map<std::uint64_t, graph::ResourceRef> &refs )
	{
		if ( refs.count( buffer.value ) == 0 )
		{
			BufferDesc desc;
			desc.usages = { usage };
			refs[buffer.value] = builder.ImportBuffer( "mesh", buffer, desc, usage, usage );
		}
	};
	for ( const Draw &draw : frame->draws )
	{
		import( draw.mesh.vertices, ResourceUsage::kVertex, vertexRefs );
		if ( draw.mesh.indices.IsValid() )
			import( draw.mesh.indices, ResourceUsage::kIndex, indexRefs );
	}

	graph::PassBuilder pass = builder.AddPass( "opaque", graph::PassKind::kRender );
	pass.Read( viewBuffer, ResourceUsage::kUniform )
	    .Read( instanceBuffer, ResourceUsage::kStorageRead )
	    .Write( targets.color, ResourceUsage::kColorAttachment )
	    .Write( targets.depth, ResourceUsage::kDepthWrite );
	for ( const auto &[buffer, ref] : vertexRefs )
		pass.Read( ref, ResourceUsage::kVertex );
	for ( const auto &[buffer, ref] : indexRefs )
		pass.Read( ref, ResourceUsage::kIndex );
	pass.Execute(
	    [this, frame, viewBuffer, instanceBuffer, targets]( graph::RecordContext &context )
	    {
		    const BindGroupEntry viewEntry[] = {
		        { 0, context.Buffer( viewBuffer ), 0, 0, {}, {} } };
		    const BindGroupEntry drawEntry[] = {
		        { 0, context.Buffer( instanceBuffer ), 0, 0, {}, {} } };
		    auto viewGroup = m_Device.CreateBindGroup( { m_ViewLayout, viewEntry } );
		    auto drawGroup = m_Device.CreateBindGroup( { m_DrawLayout, drawEntry } );
		    {
			    std::lock_guard<std::mutex> lock( m_PendingLock );
			    if ( viewGroup )
				    m_Pending.push_back( viewGroup.Value() );
			    if ( drawGroup )
				    m_Pending.push_back( drawGroup.Value() );
			    if ( !viewGroup || !drawGroup )
			    {
				    ++m_RecordFailures;
				    return;
			    }
		    }
		    CommandEncoder &encoder = context.Encoder();
		    ColorAttachment color;
		    color.texture = context.Texture( targets.color );
		    color.clear = targets.clear;
		    DepthAttachment depth;
		    depth.texture = context.Texture( targets.depth );
		    const ColorAttachment colorList[] = { color };
		    RenderingDesc rendering;
		    rendering.colors = colorList;
		    rendering.depth = depth;
		    rendering.width = targets.width;
		    rendering.height = targets.height;
		    encoder.BeginRendering( rendering );
		    encoder.SetViewport(
		        { 0.0f, 0.0f, float( targets.width ), float( targets.height ), 0.0f, 1.0f } );
		    PipelineId bound;
		    for ( const Draw &draw : frame->draws )
		    {
			    if ( draw.pipeline != bound )
			    {
				    encoder.SetPipeline( draw.pipeline );
				    encoder.SetBindGroup( BindGroupRole::kView, viewGroup.Value() );
				    encoder.SetBindGroup( BindGroupRole::kDraw, drawGroup.Value() );
				    bound = draw.pipeline;
			    }
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
		    encoder.EndRendering();
	    } );
	return stats;
}

void OpaqueRenderer::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

std::uint32_t OpaqueRenderer::RecordFailures() const
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	return m_RecordFailures;
}

} // namespace render::pass::opaque
