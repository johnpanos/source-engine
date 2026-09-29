//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.opaque (RFC 0016 K5, K4 families); see opaque.h.
//
//=============================================================================//

#include "render/pass/opaque/opaque.h"

#include "render/graph/executor.h"

#include <map>
#include <memory>
#include <span>
#include <vector>

namespace render::pass::opaque
{

namespace
{

using namespace render::device;

struct Draw
{
	resources::MeshEntry mesh;
	const material::DrawProgram *program = nullptr;
	const material::DrawGroup *drawGroup = nullptr;
	const material::DrawGroup *frameGroup = nullptr; // the frame group of its program's layout
	// The view group: the frame's of the program's layout, else the
	// program's neutral one.
	const material::ResidentGroup *viewGroup = nullptr;
	material::FamilyDrawConstants constants;
};

void Store( const math::float4x4 &m, float out[16] )
{
	for ( int r = 0; r < 4; ++r )
	{
		out[r * 4 + 0] = m.rows[r].x;
		out[r * 4 + 1] = m.rows[r].y;
		out[r * 4 + 2] = m.rows[r].z;
		out[r * 4 + 3] = m.rows[r].w;
	}
}

} // namespace

foundation::Expected<OpaqueStats, OpaqueStatus> AddOpaquePasses( graph::GraphBuilder &builder,
    const scene::SceneSnapshot &snapshot, const scene::DrawList &list, const scene::SceneView &view,
    const OpaqueSources &sources, const OpaqueTargets &targets )
{
	if ( !targets.color.IsValid() || !targets.depth.IsValid() || targets.width == 0 ||
	     targets.height == 0 )
		return foundation::MakeUnexpected( OpaqueStatus::kInvalidTargets );
	OpaqueStats stats;
	auto draws = std::make_shared<std::vector<Draw>>();
	for ( const scene::DrawItem &item : list.items )
	{
		const resources::MeshEntry *mesh = sources.meshes.Mesh( item.mesh );
		const material::DrawProgram *program = sources.programs.Program( item.material );
		if ( !mesh || !program || item.instance >= snapshot.instances.size() ||
		     mesh->vertexStride != program->vertexStride )
		{
			++stats.unresolved;
			continue;
		}
		const scene::MeshInstanceDesc &instance = snapshot.instances[item.instance].desc;
		const material::DrawGroup *drawGroup = nullptr;
		if ( program->drawLayout.IsValid() )
		{
			drawGroup =
			    sources.drawGroups ? sources.drawGroups->Group( instance.drawGroup ) : nullptr;
			if ( !drawGroup || drawGroup->layout != program->drawLayout )
			{
				++stats.unresolved;
				continue;
			}
		}
		auto matches = []( const material::DrawGroup *group, device::BindGroupLayoutId layout )
		{
			return !layout.IsValid() || ( group && group->layout == layout );
		};
		const material::DrawGroup *frameGroup = sources.frame;
		if ( program->frameLayout.IsValid() && !matches( frameGroup, program->frameLayout ) )
		{
			frameGroup = nullptr;
			for ( const material::DrawGroup *candidate : sources.frames )
			{
				if ( matches( candidate, program->frameLayout ) )
					frameGroup = candidate;
			}
		}
		const material::ResidentGroup *viewGroup = nullptr;
		if ( program->viewLayout.IsValid() )
		{
			if ( matches( sources.view, program->viewLayout ) )
				viewGroup = &sources.view->resident;
			else if ( program->hasNeutralView )
				viewGroup = &program->neutralView;
		}
		if ( !matches( frameGroup, program->frameLayout ) ||
		     ( program->viewLayout.IsValid() && !viewGroup ) )
		{
			++stats.unresolved;
			continue;
		}
		Draw draw;
		draw.mesh = *mesh;
		draw.program = program;
		draw.drawGroup = drawGroup;
		draw.frameGroup = program->frameLayout.IsValid() ? frameGroup : nullptr;
		draw.viewGroup = viewGroup;
		const math::float4x4 &world = instance.world;
		Store( math::Multiply( view.viewProjection, world ), draw.constants.toClip );
		Store( world, draw.constants.world );
		draws->push_back( draw );
	}
	stats.drawn = static_cast<std::uint32_t>( draws->size() );

	// Every buffer and texture the draws read, imported once each in its
	// residency usage.
	std::map<std::uint64_t, graph::ResourceRef> vertexRefs;
	std::map<std::uint64_t, graph::ResourceRef> indexRefs;
	std::map<std::uint64_t, graph::ResourceRef> uniformRefs;
	std::map<std::uint64_t, graph::ResourceRef> storageRefs;
	std::map<std::uint64_t, graph::ResourceRef> textureRefs;
	auto importBuffer = [&]( BufferId buffer, ResourceUsage usage,
	                        std::map<std::uint64_t, graph::ResourceRef> &refs )
	{
		if ( refs.count( buffer.value ) == 0 )
		{
			BufferDesc desc;
			desc.usages = { usage };
			refs[buffer.value] =
			    builder.ImportBuffer( "opaque-buffer", buffer, desc, usage, usage );
		}
	};
	for ( const Draw &draw : *draws )
	{
		importBuffer( draw.mesh.vertices, ResourceUsage::kVertex, vertexRefs );
		if ( draw.mesh.indices.IsValid() )
			importBuffer( draw.mesh.indices, ResourceUsage::kIndex, indexRefs );
		const material::ResidentGroup *frame =
		    draw.frameGroup ? &draw.frameGroup->resident : nullptr;
		const material::ResidentGroup *view = draw.viewGroup;
		for ( const material::ResidentGroup *group : { &draw.program->material,
		          draw.drawGroup ? &draw.drawGroup->resident : nullptr, frame, view } )
		{
			if ( !group )
				continue;
			for ( BufferId uniform : group->uniforms )
				importBuffer( uniform, ResourceUsage::kUniform, uniformRefs );
			for ( BufferId storage : group->storage )
				importBuffer( storage, ResourceUsage::kStorageRead, storageRefs );
			for ( const material::SampledTexture &texture : group->textures )
			{
				if ( textureRefs.count( texture.texture.value ) == 0 )
					textureRefs[texture.texture.value] =
					    builder.ImportTexture( "opaque-texture", texture.texture, texture.desc,
					        ResourceUsage::kSampled, ResourceUsage::kSampled );
			}
		}
	}

	graph::PassBuilder pass = builder.AddPass( "opaque", graph::PassKind::kRender );
	pass.Write( targets.color, ResourceUsage::kColorAttachment )
	    .Write( targets.depth, ResourceUsage::kDepthWrite );
	for ( const auto &[buffer, ref] : vertexRefs )
		pass.Read( ref, ResourceUsage::kVertex );
	for ( const auto &[buffer, ref] : indexRefs )
		pass.Read( ref, ResourceUsage::kIndex );
	for ( const auto &[buffer, ref] : uniformRefs )
		pass.Read( ref, ResourceUsage::kUniform );
	for ( const auto &[buffer, ref] : storageRefs )
		pass.Read( ref, ResourceUsage::kStorageRead );
	for ( const auto &[texture, ref] : textureRefs )
		pass.Read( ref, ResourceUsage::kSampled );
	pass.Execute(
	    [draws, targets]( graph::RecordContext &context )
	    {
		    CommandEncoder &encoder = context.Encoder();
		    ColorAttachment color;
		    color.texture = context.Texture( targets.color );
		    color.load = targets.clearColor ? LoadOp::kClear : LoadOp::kLoad;
		    color.clear = targets.clear;
		    DepthAttachment depth;
		    depth.texture = context.Texture( targets.depth );
		    depth.load = targets.clearDepth ? LoadOp::kClear : LoadOp::kLoad;
		    const ColorAttachment colorList[] = { color };
		    RenderingDesc rendering;
		    rendering.colors = colorList;
		    rendering.depth = depth;
		    rendering.width = targets.width;
		    rendering.height = targets.height;
		    encoder.BeginRendering( rendering );
		    encoder.SetViewport(
		        { 0.0f, 0.0f, float( targets.width ), float( targets.height ), 0.0f, 1.0f } );
		    const material::DrawProgram *bound = nullptr;
		    const material::DrawGroup *boundDraw = nullptr;
		    for ( const Draw &draw : *draws )
		    {
			    if ( draw.program != bound )
			    {
				    encoder.SetPipeline( draw.program->pipeline );
				    if ( draw.frameGroup )
					    encoder.SetBindGroup(
					        BindGroupRole::kFrame, draw.frameGroup->resident.group );
				    if ( draw.viewGroup )
					    encoder.SetBindGroup( BindGroupRole::kView, draw.viewGroup->group );
				    encoder.SetBindGroup( BindGroupRole::kMaterial, draw.program->material.group );
				    bound = draw.program;
				    boundDraw = nullptr;
			    }
			    if ( draw.drawGroup && draw.drawGroup != boundDraw )
			    {
				    encoder.SetBindGroup( BindGroupRole::kDraw, draw.drawGroup->resident.group );
				    boundDraw = draw.drawGroup;
			    }
			    if ( draw.program->drawConstantBytes > 0 )
				    encoder.SetDrawConstants( 0, std::as_bytes( std::span( &draw.constants, 1 ) )
				                                     .first( draw.program->drawConstantBytes ) );
			    encoder.SetVertexBuffer( 0, draw.mesh.vertices );
			    if ( draw.mesh.indices.IsValid() )
			    {
				    encoder.SetIndexBuffer( draw.mesh.indices, 0, draw.mesh.indexFormat );
				    encoder.DrawIndexed( draw.mesh.indexCount, 1, 0, 0, 0 );
			    }
			    else
			    {
				    encoder.Draw( draw.mesh.vertexCount, 1, 0, 0 );
			    }
		    }
		    encoder.EndRendering();
	    } );
	return stats;
}

} // namespace render::pass::opaque
