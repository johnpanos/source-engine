//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): the legacy frontend's mesh and UI handoff, and scene-color capture.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

std::optional<pass::world::WorldSceneColor> CoreWorld::Capture( device::IRenderDevice2 &device,
    device::CommandEncoder &encoder, device::TextureId source,
    const device::TextureDesc &sourceDesc, std::uint64_t frame )
{
	auto captured = graph::RecordSceneColor( device, encoder, source, sourceDesc );
	if ( !captured )
		return std::nullopt;
	graph::RecordedSceneColor value = std::move( captured ).Value();
	pass::world::WorldSceneColor result{ value.texture, value.desc };
	m_SceneCaptures.emplace_back( frame, std::move( value.resources ) );
	return result;
}

std::uint32_t CoreWorld::QueueMesh( const legacy::CoreMeshDraw &draw )
{
	// The engine's bloom chain (RFC 0016 K8): claimed by name, drawn once by
	// the output stage.
	if ( draw.kind == legacy::CoreMeshKind::kScreenEffect )
		return m_Output.QueuePost( draw );
	if ( !AcceptsMeshes() && draw.kind == legacy::CoreMeshKind::kSurface )
		return 0;
	const bool cards = draw.kind == legacy::CoreMeshKind::kParticle && draw.cards;
	if ( !draw.name || !draw.shader || !( draw.indices || draw.indices16 ) || !draw.indexCount ||
	     ( cards ? !draw.cardCount : !draw.vertices || !draw.vertexCount ) ||
	     ( draw.variableCount && !draw.variables ) )
		return 0;
	// A static prop's baked vertex lighting reaches the core in its
	// vertices' color, for the mesh point alone (VertexLitGeneric).
	if ( draw.staticVertexLighting && !draw.mesh )
	{
		m_Pass.NoteRefusal( std::string( "material " ) + draw.name +
		                    ": baked vertex lighting on a shader without the mesh point" );
		return 0;
	}
	pass::world::WorldView view;
	std::copy_n( draw.toClip, 16, view.toClip );
	// The unjittered camera transform the shaders project world points with
	// (PortalRefract's refraction; the current motion position).
	std::copy_n( draw.toClip, 16, view.motionToClip );
	view.viewport = draw.viewport;
	view.depthAlphaHandle = draw.depthAlphaHandle;
	view.depthAlphaRange = draw.depthAlphaRange;
	view.debug = m_Renderer.AppliedDebug();
	pass::world::WorldView::DynamicDraw geometry;
	std::copy_n( draw.modelToWorld, 16, geometry.modelToWorld );
	// A revisioned material (the frontend promises equal content for equal
	// revisions) is built once and shared by its draws: per draw it was a
	// copy of every variable and a default lookup each, then (copying the
	// cached one) of every string pair.
	bool reused = false;
	const bool revisioned =
	    draw.materialRevision && draw.kind != legacy::CoreMeshKind::kStencilClear;
	if ( revisioned )
	{
		std::lock_guard<std::mutex> guard( m_MaterialLock );
		auto known = m_RevisionMaterials.find( draw.materialRevision );
		if ( known != m_RevisionMaterials.end() && known->second->mesh == draw.mesh )
		{
			geometry.sharedMaterial = known->second;
			reused = true;
		}
	}
	if ( !reused )
	{
		geometry.material.name = draw.name;
		geometry.material.shader =
		    draw.kind == legacy::CoreMeshKind::kStencilClear ? "UnlitGeneric" : draw.shader;
		if ( draw.kind == legacy::CoreMeshKind::kStencilClear )
		{
			geometry.material.variables.emplace_back( "$vertexcolor", "1" );
			geometry.material.variables.emplace_back( "$vertexalpha", "1" );
			geometry.material.variables.emplace_back( "$nofog", "1" );
		}
		geometry.material.mesh = draw.mesh;

		for ( std::uint32_t i = 0;
		    draw.kind != legacy::CoreMeshKind::kStencilClear && i < draw.variableCount; ++i )
		{
			const legacy::CoreMeshVariable &variable = draw.variables[i];
			if ( !variable.key || !variable.value )
				return 0;
			geometry.material.variables.emplace_back( variable.key, variable.value );
			const char *declared = MaterialDefault( draw.shader, variable.key );
			if ( !declared )
				declared = variable.defaultValue;
			if ( declared )
				geometry.material.defaults.emplace_back( variable.key, declared );
			if ( variable.textureHandle )
				geometry.material.textures.emplace_back( variable.key, variable.textureHandle );
		}
		geometry.material.revision = revisioned ? draw.materialRevision : 0;
		if ( revisioned )
		{
			std::lock_guard<std::mutex> guard( m_MaterialLock );
			if ( m_RevisionMaterials.size() >= 256 )
				m_RevisionMaterials.clear();
			geometry.sharedMaterial = std::make_shared<const pass::world::WorldMaterial>(
			    std::move( geometry.material ) );
			geometry.material = {};
			m_RevisionMaterials[draw.materialRevision] = geometry.sharedMaterial;
		}
	}
	geometry.staticVertexLight = draw.staticVertexLighting;
	if ( cards )
	{
		geometry.cards.assign( draw.cards, draw.cards + draw.cardCount );
		std::copy_n( draw.cardModel, 16, geometry.cardModel );
		std::copy_n( draw.cardView, 16, geometry.cardView );
		geometry.cardSplineRange = draw.cardSplineRange;
		geometry.cardSplineNormals = draw.cardSplineNormals;
	}
	// The frontend's own arrays are taken when offered (CoreMeshDraw::take*):
	// a copy per model draw was a frame's largest memcpy caller.
	else if ( draw.takeVertices && draw.takeVertices->data() == draw.vertices &&
	          draw.takeVertices->size() == draw.vertexCount )
		geometry.vertices = std::move( *draw.takeVertices );
	else
		geometry.vertices.assign( draw.vertices, draw.vertices + draw.vertexCount );
	if ( draw.indices16 )
	{
		if ( draw.takeIndices16 && draw.takeIndices16->data() == draw.indices16 &&
		     draw.takeIndices16->size() == draw.indexCount )
			geometry.indices16 = std::move( *draw.takeIndices16 );
		else
			geometry.indices16.assign( draw.indices16, draw.indices16 + draw.indexCount );
	}
	else if ( draw.takeIndices && draw.takeIndices->data() == draw.indices &&
	          draw.takeIndices->size() == draw.indexCount )
		geometry.indices = std::move( *draw.takeIndices );
	else
		geometry.indices.assign( draw.indices, draw.indices + draw.indexCount );
	geometry.lightmapPage = draw.lightmapPage;
	geometry.capturedLightmap = draw.capturedLightmap;
	if ( draw.modelLighting && draw.lightCount <= material::kMaxModelLights )
	{
		// The eye (legacy cEyePos) from the row-major world-to-view
		// transform: -R^T t.
		float eye[3];
		for ( int c = 0; c < 3; ++c )
			eye[c] = -( draw.worldToView[0 + c] * draw.worldToView[3] +
			            draw.worldToView[4 + c] * draw.worldToView[7] +
			            draw.worldToView[8 + c] * draw.worldToView[11] );
		geometry.lighting = material::PackSourceModelLighting(
		    eye, draw.ambientCube, std::span( draw.lights, draw.lightCount ) );
	}
	view.dynamicDraws.push_back( std::move( geometry ) );
	// Without a map (menus, loading screens) the pass draws on an empty world.
	if ( !m_Pass.HasWorld() )
		m_Pass.SetWorld( pass::world::WorldData() );
	const std::uint32_t tag = m_Pass.QueueView( std::move( view ) );
	if ( tag )
	{
		StreamView stream;
		std::copy_n( draw.worldToView, 16, stream.view.begin() );
		std::copy_n( draw.viewToClip, 16, stream.projection.begin() );
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		m_StreamViews.emplace( tag, stream );
	}
	return tag;
}

pass::world::WorldMaterial CoreWorld::UiMaterial( const RenderCoreWorldMaterial &material ) const
{
	pass::world::WorldMaterial out = std::move( WorldMaterials( &material, 1 ).front() );
	// An animated texture's frame ($frame) is the handle of that frame.
	int frame = 0;
	ITexture *base = nullptr;
	for ( int v = 0; v < material.variableCount; ++v )
	{
		const char *key = material.keys[v] ? material.keys[v] : "";
		if ( !std::strcmp( key, "$frame" ) && material.values[v] )
			frame = std::atoi( material.values[v] );
		if ( !std::strcmp( key, "$basetexture" ) && material.textures )
			base = material.textures[v];
	}
	if ( frame > 0 && base && m_Host && m_Host->textureFrameHandle )
	{
		for ( auto &[key, handle] : out.textures )
		{
			if ( key == "$basetexture" )
				handle = m_Host->textureFrameHandle( base, frame );
		}
	}
	return out;
}

std::optional<std::string> CoreWorld::ClaimUiMaterial( const RenderCoreWorldMaterial &material )
{
	return m_Pass.DynamicClaim( UiMaterial( material ) );
}

std::uint32_t CoreWorld::QueueUiList(
    const ui_draw_list::ListView &list, const RenderCoreWorldMaterial *materials, std::string &why )
{
	if ( !materials && list.materialCount )
	{
		why = "the list's materials are missing";
		return 0;
	}
	std::vector<pass::world::WorldMaterial> converted;
	converted.reserve( list.materialCount );
	for ( std::uint32_t i = 0; i < list.materialCount; ++i )
		converted.push_back( UiMaterial( materials[i] ) );
	pass::world::WorldView view;
	if ( std::optional<std::string> malformed = pass::world::UiListView( list, converted, view ) )
	{
		why = *malformed;
		return 0;
	}
	view.debug = m_Renderer.AppliedDebug();
	if ( !m_Pass.HasWorld() )
		m_Pass.SetWorld( pass::world::WorldData() );
	const std::uint32_t tag = m_Pass.QueueView( std::move( view ) );
	if ( !tag )
		why = m_Pass.Stats().lastRefusal;
	return tag;
}

} // namespace render::composition
