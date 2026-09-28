//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.skinning (RFC 0016 K6); see skinning.h.
//
//=============================================================================//

#include "render/pass/skinning/skinning.h"

#include "render/graph/executor.h"

#include "skin_spv.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

namespace render::pass::skinning
{

namespace
{

using namespace render::device;

// skin.comp's bindings, all in the draw group.
enum Binding : std::uint32_t
{
	kVertices = 0,
	kBones,
	kFlexOffsets,
	kFlexDeltas,
	kFlexWeights,
	kOutput,
	kBindingCount
};

void Transform( const BoneMatrix &bone, const float in[3], float w, float out[3] )
{
	for ( int r = 0; r < 3; ++r )
		out[r] = bone.rows[r][0] * in[0] + bone.rows[r][1] * in[1] + bone.rows[r][2] * in[2] +
		         bone.rows[r][3] * w;
}

} // namespace

void SkinReference( const SkinInputs &inputs, std::span<SkinnedVertex> out )
{
	const std::size_t boneCount = inputs.bones.size();
	for ( std::size_t v = 0; v < inputs.vertices.size() && v < out.size(); ++v )
	{
		const SkinVertex &vertex = inputs.vertices[v];
		float position[3] = { vertex.position[0], vertex.position[1], vertex.position[2] };
		float normal[3] = { vertex.normal[0], vertex.normal[1], vertex.normal[2] };
		float tangent[3] = { vertex.tangent[0], vertex.tangent[1], vertex.tangent[2] };
		float wrinkle = 0.0f;
		if ( !inputs.flexOffsets.empty() )
		{
			for ( std::uint32_t d = inputs.flexOffsets[v]; d < inputs.flexOffsets[v + 1]; ++d )
			{
				const FlexDelta &delta = inputs.flexDeltas[d];
				const FlexWeights &w = inputs.flexWeights[delta.flex];
				const float weight = w.weight[0] + ( w.weight[1] - w.weight[0] ) * delta.side;
				for ( int k = 0; k < 3; ++k )
				{
					position[k] += weight * delta.position[k];
					normal[k] += weight * delta.normal[k];
					tangent[k] += weight * delta.normal[k];
				}
				wrinkle += weight * delta.wrinkle;
			}
		}
		const float weights[3] = {
		    vertex.weight0, vertex.weight1, 1.0f - ( vertex.weight0 + vertex.weight1 ) };
		SkinnedVertex &result = out[v];
		result = SkinnedVertex();
		for ( int b = 0; b < 3; ++b )
		{
			std::size_t bone = ( vertex.bones >> ( 8 * b ) ) & 0xffu;
			if ( bone >= boneCount )
				bone = 0;
			float p[3], n[3], t[3];
			Transform( inputs.bones[bone], position, 1.0f, p );
			Transform( inputs.bones[bone], normal, 0.0f, n );
			Transform( inputs.bones[bone], tangent, 0.0f, t );
			for ( int k = 0; k < 3; ++k )
			{
				result.position[k] += weights[b] * p[k];
				result.normal[k] += weights[b] * n[k];
				result.tangent[k] += weights[b] * t[k];
			}
		}
		result.wrinkle = wrinkle;
		result.tangent[3] = vertex.tangent[3];
	}
}

SkinError CompareSkinned( std::span<const SkinnedVertex> a, std::span<const SkinnedVertex> b )
{
	SkinError error;
	if ( a.size() != b.size() )
	{
		error.position = error.normal = error.tangent = error.wrinkle = INFINITY;
		return error;
	}
	auto worst = []( float &into, float x, float y )
	{
		const float d = std::fabs( x - y );
		into = std::isnan( d ) ? INFINITY : std::max( into, d );
	};
	for ( std::size_t v = 0; v < a.size(); ++v )
	{
		for ( int k = 0; k < 3; ++k )
		{
			worst( error.position, a[v].position[k], b[v].position[k] );
			worst( error.normal, a[v].normal[k], b[v].normal[k] );
		}
		for ( int k = 0; k < 4; ++k )
			worst( error.tangent, a[v].tangent[k], b[v].tangent[k] );
		worst( error.normal, a[v].normal[3], b[v].normal[3] );
		worst( error.wrinkle, a[v].wrinkle, b[v].wrinkle );
	}
	return error;
}

SkinningKernel::SkinningKernel( IRenderDevice2 &device ) : m_Device( device )
{
}

SkinningKernel::~SkinningKernel()
{
	// Collect has released every recorded dispatch's bind group behind its
	// token; the pipeline and layout follow the last one.
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

foundation::Expected<std::unique_ptr<SkinningKernel>, SkinningStatus> SkinningKernel::Create(
    IRenderDevice2 &device, std::span<const std::uint32_t> code )
{
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return foundation::MakeUnexpected( SkinningStatus::kNoCompute );
	if ( code.empty() )
		code = spirv::kSkinCompute;
	std::unique_ptr<SkinningKernel> kernel( new SkinningKernel( device ) );

	BindingDesc bindings[kBindingCount];
	ReflectedBinding reflected[kBindingCount];
	for ( std::uint32_t b = 0; b < kBindingCount; ++b )
	{
		bindings[b] = { b, BindingKind::kStorageBuffer, 1, { ShaderStage::kCompute } };
		reflected[b] = {
		    static_cast<std::uint32_t>( BindGroupRole::kDraw ), b, BindingKind::kStorageBuffer };
	}
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( SkinningStatus::kDevice );
	kernel->m_Layout = layout.Value();

	const BindGroupLayoutId layouts[kMaxBindGroups] = { {}, {}, {}, kernel->m_Layout };
	const ShaderArtifactView stage[] = { { ShaderStage::kCompute, ArtifactFormat::kSpirv,
	    std::as_bytes( code ), "main", reflected } };
	PipelineDesc desc;
	desc.kind = PipelineKind::kCompute;
	desc.stages = stage;
	desc.layouts = layouts;
	desc.debugName = "render.pass.skinning";
	auto pipeline = device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( SkinningStatus::kDevice );
	kernel->m_Pipeline = pipeline.Value();
	return kernel;
}

foundation::Expected<void, SkinningStatus> SkinningKernel::Record(
    CommandEncoder &encoder, const SkinningBuffers &buffers )
{
	if ( buffers.vertexCount == 0 )
		return {};
	const std::uint32_t deltas = std::max( buffers.deltaCount, 1u );
	const std::uint32_t flexes = std::max( buffers.flexCount, 1u );
	const BindGroupEntry entries[kBindingCount] = {
	    { kVertices, buffers.vertices, 0,
	        std::uint64_t( buffers.vertexCount ) * sizeof( SkinVertex ), {}, {} },
	    { kBones, buffers.bones, 0, std::uint64_t( buffers.boneCount ) * sizeof( BoneMatrix ), {},
	        {} },
	    { kFlexOffsets, buffers.flexOffsets, 0,
	        std::uint64_t( buffers.vertexCount + 1 ) * sizeof( std::uint32_t ), {}, {} },
	    { kFlexDeltas, buffers.flexDeltas, 0, std::uint64_t( deltas ) * sizeof( FlexDelta ), {},
	        {} },
	    { kFlexWeights, buffers.flexWeights, 0, std::uint64_t( flexes ) * sizeof( FlexWeights ), {},
	        {} },
	    { kOutput, buffers.output, 0,
	        std::uint64_t( buffers.vertexCount ) * sizeof( SkinnedVertex ), {}, {} },
	};
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( SkinningStatus::kDevice );
	}
	m_Pending.push_back( group.Value() );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Dispatch( ( buffers.vertexCount + 63 ) / 64 );
	return {};
}

void SkinningKernel::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

void AddSkinningPass(
    graph::GraphBuilder &builder, SkinningKernel &kernel, const SkinningPassResources &resources )
{
	graph::PassBuilder pass = builder.AddPass( "skinning", graph::PassKind::kCompute );
	for ( graph::ResourceRef input : { resources.vertices, resources.bones, resources.flexOffsets,
	          resources.flexDeltas, resources.flexWeights } )
		pass.Read( input, ResourceUsage::kStorageRead );
	pass.Write( resources.output, ResourceUsage::kStorageWrite );
	pass.Execute(
	    [&kernel, resources]( graph::RecordContext &context )
	    {
		    SkinningBuffers buffers;
		    buffers.vertices = context.Buffer( resources.vertices );
		    buffers.bones = context.Buffer( resources.bones );
		    buffers.flexOffsets = context.Buffer( resources.flexOffsets );
		    buffers.flexDeltas = context.Buffer( resources.flexDeltas );
		    buffers.flexWeights = context.Buffer( resources.flexWeights );
		    buffers.output = context.Buffer( resources.output );
		    buffers.vertexCount = resources.vertexCount;
		    buffers.boneCount = resources.boneCount;
		    buffers.deltaCount = resources.deltaCount;
		    buffers.flexCount = resources.flexCount;
		    (void)kernel.Record( context.Encoder(), buffers );
	    } );
}

} // namespace render::pass::skinning
