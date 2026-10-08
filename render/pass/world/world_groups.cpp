//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): a batch's bind groups, materials and frame terms.
//
//=============================================================================//

#include "world_batch.h"

namespace render::pass::world
{

// Neutral inputs, made on first use: white color and far depth. The cube
// term is off wherever its neutral texture is bound; filling it keeps it defined.
TextureId WorldPass::Batch::neutral( TextureDimension dimension, bool array, bool depth )
{
	TextureId &slot = depth ? r.neutralDepth
	                  : dimension == TextureDimension::kCube
	                      ? ( array ? r.neutralCubeArray : r.neutralCube )
	                  : array ? r.neutralArray
	                          : r.neutralWhite;
	if ( slot.IsValid() )
		return slot;
	if ( !r.neutralStaging.IsValid() )
	{
		const std::uint32_t whiteAndDepth[] = { 0xFFFFFFFFu, 0x3F800000u };
		auto buffer = device.CreateUploadBuffer( std::as_bytes( std::span( whiteAndDepth ) ) );
		if ( !buffer )
			return {};
		r.neutralStaging = buffer.Value();
	}
	TextureDesc desc;
	desc.dimension = dimension;
	desc.format = depth ? Format::kD32Float : Format::kRGBA8Unorm;
	desc.width = desc.height = 1;
	desc.depthOrLayers = dimension == TextureDimension::kCube ? ( array ? 12 : 6 ) : array ? 2 : 1;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	desc.debugName = dimension == TextureDimension::kCube
	                     ? ( array ? "world neutral cube array" : "world neutral cube" )
	                 : array ? "world neutral array"
	                         : "world neutral white";
	auto texture = device.CreateTexture( desc );
	if ( !texture )
		return {};
	encoder.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	for ( std::uint32_t layer = 0; layer < desc.depthOrLayers; ++layer )
		encoder.CopyBufferToTexture(
		    r.neutralStaging, texture.Value(), { depth ? 4u : 0u, 0, layer, 1, 1 } );
	encoder.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	slot = texture.Value();
	return slot;
}

bool WorldPass::Batch::buildGroup( const material::GroupRequest &request,
    const std::map<std::string, int> &handles, Group &out, std::string *why )
{
	std::vector<BindGroupEntry> entries;
	if ( !request.constants.empty() )
	{
		auto constants =
		    s.groupResources.Acquire( device, request.constants.size(), ResourceUsage::kUniform );
		if ( !constants )
		{
			*why = "a constants buffer was refused";
			return false;
		}
		out.constants = constants.Value();
		entries.push_back( { request.constantsBinding, out.constants.id, 0, 0, {}, {} } );
	}
	for ( const material::GroupBuffer &storage : request.storage )
	{
		if ( storage.external.IsValid() )
		{
			out.storage.push_back( { storage.external } );
			entries.push_back( { storage.binding, storage.external, 0, 0, {}, {} } );
			continue;
		}
		auto buffer = s.groupResources.Acquire( device,
		    std::max<std::uint64_t>( storage.bytes.size(), 4 ), ResourceUsage::kStorageRead );
		if ( !buffer )
		{
			*why = "a storage buffer was refused";
			return false;
		}
		out.storage.push_back( buffer.Value() );
		entries.push_back( { storage.binding, buffer.Value().id, 0, 0, {}, {} } );
	}
	for ( const material::ProgramTexture &texture : request.textures )
	{
		// A texture the group's owner made (the view's shadow atlas) is
		// bound as it is, with the request's own sampler.
		const bool external = texture.external.IsValid();
		// A world stage's own texture (its lightmap pages, probes and
		// tables), bound as it is.
		const auto stageTexture = r.stageTextures.find( texture.name );
		const bool staged =
		    !external && !texture.name.empty() && stageTexture != r.stageTextures.end();
		const auto handle = handles.find( texture.name );
		const bool absent =
		    !external && !staged &&
		    ( texture.name.empty() || ( handle != handles.end() && handle->second == 0 ) );
		const TextureId id = external ? texture.external
		                     : staged ? stageTexture->second
		                     : absent ? neutral( texture.dimension, texture.array, texture.depth )
		                     : handle == handles.end()
		                         ? TextureId()
		                         : textures.Import( handle->second, texture.srgb );
		if ( !id.IsValid() )
		{
			texturePending = handle != handles.end() && textures.Pending( handle->second );
			*why = handle == handles.end()
			           ? "texture " + texture.name + " has no material system handle"
			           : "texture " + texture.name + " did not import" +
			                 ( texture.srgb ? " through an sRGB view" : "" );
			return false;
		}
		entries.push_back( { texture.binding, {}, 0, 0, id, {} } );
		if ( texture.samplerBinding == material::kNoSamplerBinding )
			continue; // only fetched
		auto sampler = s.groupResources.AcquireSampler(
		    device, external || staged ? texture.sampler
		            : absent           ? texture.sampler
		                               : textures.Sampler( handle->second ) );
		if ( !sampler )
		{
			*why = "a sampler was refused";
			return false;
		}
		if ( sampler.Value().owned )
			out.samplers.push_back( sampler.Value().id );
		entries.push_back( { texture.samplerBinding, {}, 0, 0, {}, sampler.Value().id } );
	}
	for ( const auto &[binding, desc] : request.samplers )
	{
		auto sampler = s.groupResources.AcquireSampler( device, desc );
		if ( !sampler )
		{
			*why = "an additional sampler was refused";
			return false;
		}
		if ( sampler.Value().owned )
			out.samplers.push_back( sampler.Value().id );
		entries.push_back( { binding, {}, 0, 0, {}, sampler.Value().id } );
	}
	auto group = device.CreateBindGroup( { request.layout, entries } );
	if ( !group )
	{
		const DeviceError &error = group.Error();
		*why = "a bind group was refused: " + std::string( DescribeStatus( error.status ) ) +
		       " (native " + std::to_string( error.nativeCode ) + ")";
		return false;
	}
	out.group = group.Value();
	if ( out.constants.id.IsValid() )
	{
		encoder.TransitionBuffer(
		    out.constants.id, out.constants.before, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( out.constants.id, 0, request.constants );
		encoder.TransitionBuffer(
		    out.constants.id, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
		out.constants.before = ResourceUsage::kUniform;
	}
	for ( std::size_t i = 0; i < out.storage.size(); ++i )
	{
		if ( out.storage[i].size == 0 )
			continue;
		encoder.TransitionBuffer(
		    out.storage[i].id, out.storage[i].before, ResourceUsage::kCopyDestination );
		if ( !request.storage[i].bytes.empty() )
			encoder.WriteBuffer( out.storage[i].id, 0, request.storage[i].bytes );
		encoder.TransitionBuffer(
		    out.storage[i].id, ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead );
		out.storage[i].before = ResourceUsage::kStorageRead;
	}
	return true;
}

void WorldPass::Batch::note( std::string why )
{
	if ( failure.empty() )
		failure = std::move( why );
}

Resources::Material *WorldPass::Batch::prepareMaterial( material::ProgramResolver &resolver,
    Resources::Material &m, const Claimed &claimed, const WorldMaterial &source )
{
	if ( m.failed )
		note( m.failure );
	if ( m.ready || m.failed )
		return m.ready ? &m : nullptr;
	const std::string &name = source.name;
	const bool model =
	    &resolver == r.modelResolver.get() || &resolver == r.prepassModelResolver.get();
	auto program =
	    source.mesh ? resolver.ResolveMesh( claimed.desc ) : resolver.Resolve( claimed.desc );
	std::string why;
	if ( !program )
		why = program.Error();
	else if ( program.Value().request.vertexStride !=
	          ( model ? sizeof( material::SurfaceModelVertex ) : sizeof( WorldVertex ) ) )
		why = "its program reads another vertex than its mesh";
	else if ( !buildGroup( program.Value().request.material, claimed.handles, m.group, &why ) )
		s.ReleaseGroup( m.group, CompletionToken() );
	if ( texturePending )
	{
		// Not a failure: a texture is still being filled. The material stays
		// unready and is built again at its next use.
		texturePending = false;
		lastMaterialPending = viewMaterialPending = true;
		{
			std::lock_guard<std::mutex> guard( s.lock );
			++s.stats.pendingMaterials;
		}
		return nullptr;
	}
	if ( !why.empty() )
	{
		m.failed = true;
		m.failure = "material " + name + ": " + why;
		note( m.failure );
		{
			std::lock_guard<std::mutex> guard( s.lock );
			auto gap = std::find_if( s.stats.gaps.begin(), s.stats.gaps.end(),
			    [&]( const auto &entry )
			    {
				    return entry.first == m.failure;
			    } );
			if ( gap == s.stats.gaps.end() )
				s.stats.gaps.emplace_back( m.failure, 1 );
			else
				++gap->second;
		}
		return nullptr;
	}
	m.program = std::move( program ).Value();
	m.resolver = &resolver;
	m.ready = true;
	m.viewInput = 0;
	for ( const std::string &input : m.program.viewInputs )
	{
		const auto handle = claimed.handles.find( input );
		if ( handle == claimed.handles.end() || handle->second == 0 )
		{
			m.ready = false;
			m.failed = true;
			m.failure =
			    "material " + name + ": its view input " + input + " has no material system handle";
			note( m.failure );
			return nullptr;
		}
		m.viewInput = handle->second;
	}
	m.refractInput = 0;
	if ( !m.program.refractInput.empty() )
	{
		const auto handle = claimed.handles.find( m.program.refractInput );
		if ( handle == claimed.handles.end() || handle->second == 0 )
		{
			m.ready = false;
			m.failed = true;
			m.failure = "material " + name + ": its refraction target " + m.program.refractInput +
			            " has no material system handle";
			note( m.failure );
			return nullptr;
		}
		m.refractInput = handle->second;
	}
	return &m;
}

Resources::Material *WorldPass::Batch::materialReadyIn( material::ProgramResolver &resolver,
    std::vector<Resources::Material> &materials, std::uint32_t index )
{
	lastMaterialPending = false;
	return prepareMaterial(
	    resolver, materials[index], ( *claims )[index], world->materials[index] );
}

Resources::Material *WorldPass::Batch::materialReady( std::uint32_t index )
{
	return materialReadyIn( *r.resolver, r.materials, index );
}

std::tuple<std::uint64_t, int, std::string> WorldPass::Batch::drawKey(
    const Resources::Material &m, int page, bool captured )
{
	std::string inputs = captured ? "captured;" : "";
	for ( const std::string &input : m.program.drawInputs )
		inputs += input + ";";
	return std::make_tuple( m.program.request.drawLayout.value, page, std::move( inputs ) );
}

const Group *WorldPass::Batch::drawGroupReady(
    const Resources::Material &m, int page, bool captured )
{
	const auto key = drawKey( m, page, captured );
	if ( auto found = r.drawGroups.find( key ); found != r.drawGroups.end() )
		return found->second.group.IsValid() ? &found->second : nullptr;
	Group &group = r.drawGroups[key];
	std::vector<std::string> inputs;
	std::map<std::string, int> handles;
	for ( const std::string &input : m.program.drawInputs )
	{
		if ( world->stage && !captured )
		{
			// A world stage's pages; an input the stage lacks is off.
			const WorldStage &stage = *world->stage;
			if ( input == "lightmap" )
				inputs.push_back( kStageLightmap );
			else if ( input == "lightmap-gradient" )
				inputs.push_back( stage.lightmap.Directional() ? kStageGradient : "" );
			else if ( input == "lightmap-indirect" )
				inputs.push_back( stage.indirect.flat.empty() ? "" : kStageIndirect );
			else if ( input == "lightmap-indirect-gradient" )
				inputs.push_back( stage.indirect.Directional() ? kStageIndirectGradient : "" );
			else if ( input == "lightmap-shadow-mask" )
				inputs.push_back( stage.shadowMask.flat.empty() ? "" : kStageShadowMask );
			else
			{
				note( "a program reads draw input " + input + ", which the world stage lacks" );
				return nullptr;
			}
			continue;
		}
		if ( input != "lightmap" )
		{
			note( "a program reads draw input " + input + ", which the world lacks" );
			return nullptr;
		}
		inputs.push_back( PageName( page ) );
		handles[PageName( page )] = page;
	}
	const std::optional<material::GroupRequest> request =
	    m.resolver->DrawGroup( m.program, inputs );
	std::string why;
	if ( !request || !buildGroup( *request, handles, group, &why ) )
	{
		s.ReleaseGroup( group, CompletionToken() );
		// Only successful bindings enter the cache. Retrying a missing
		// image must retain its named error and may succeed after upload.
		r.drawGroups.erase( key );
		note( "a draw group: " + ( why.empty() ? std::string( "not resolved" ) : why ) );
		return nullptr;
	}
	return &group;
}

const Group *WorldPass::Batch::frameGroupReady( const Resources::Material &m )
{
	if ( auto error = material::FrameInputError( m.program, terms ) )
	{
		note( *error );
		return nullptr;
	}
	const std::uint64_t layout = m.program.request.frameLayout.value;
	if ( framesWritten[layout] )
		return &r.frameGroups[layout];
	Group &group = r.frameGroups[layout];
	if ( !group.group.IsValid() )
	{
		const std::optional<material::GroupRequest> request =
		    m.resolver->FrameGroup( m.program, terms );
		if ( !request )
		{
			note( "a frame group was not resolved" );
			return nullptr;
		}
		std::string why;
		if ( !buildGroup( *request, {}, group, &why ) )
		{
			s.ReleaseGroup( group, CompletionToken() );
			note( "a frame group: " + why );
			return nullptr;
		}
		framesWritten[layout] = true; // buildGroup wrote this slot's terms
	}
	else if ( !framesWritten[layout] )
	{
		// The built group keeps its textures and storage; only the terms change.
		const std::optional<std::vector<std::byte>> constants =
		    m.resolver->FrameConstants( m.program, terms );
		if ( !constants )
		{
			note( "a frame group was not resolved" );
			return nullptr;
		}
		encoder.TransitionBuffer(
		    group.constants.id, ResourceUsage::kUniform, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( group.constants.id, 0, *constants );
		encoder.TransitionBuffer(
		    group.constants.id, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
		framesWritten[layout] = true;
	}
	return &group;
}

const Group *WorldPass::Batch::viewGroupReady( const Resources::Material &m )
{
	const std::uint64_t layout = m.program.request.viewLayout.value;
	if ( m.program.depthBlend &&
	     ( !viewDepthAlpha.IsValid() || !std::isfinite( view.depthAlphaRange ) ||
	         view.depthAlphaRange <= 0.0f ) )
	{
		note( "$depthblend has no imported depth-alpha snapshot with a positive range" );
		return nullptr;
	}
	if ( m.program.sceneColor && !viewSceneColor.IsValid() )
	{
		note( "a transmitting program has no scene-color snapshot" );
		return nullptr;
	}
	if ( m.viewInput != 0 && !viewReflection.IsValid() )
	{
		note( "a program reads the view's planar reflection, which did not import" );
		return nullptr;
	}
	if ( m.refractInput != 0 && !viewRefraction.IsValid() )
	{
		note( "a program reads the view's water refraction, which did not import" );
		return nullptr;
	}
	if ( view.lights )
	{
		const bool model =
		    m.resolver == r.modelResolver.get() || m.resolver == r.prepassModelResolver.get();
		Group *&slot = model ? modelLitViews[layout] : litViews[layout];
		Group *lit = m.program.depthBlend   ? &depthViews[layout]
		             : m.program.sceneColor ? &sceneViews[layout]
		             : m.refractInput != 0  ? &refractViews[layout]
		                                    : slot;
		if ( lit && lit->group.IsValid() )
			return lit;
		if ( !m.program.sceneColor && !m.program.depthBlend && m.refractInput == 0 )
		{
			const auto cached = std::find_if( r.litViews.begin(), r.litViews.end(),
			    [&]( const Resources::LitView &entry )
			    {
				    return entry.layout == layout && entry.lights == view.lights &&
				           entry.shadowAtlas == target.shadowAtlas &&
				           entry.occlusion == viewOcclusion && entry.reflection == viewReflection &&
				           entry.group.group.IsValid();
			    } );
			if ( cached != r.litViews.end() )
			{
				slot = &cached->group;
				return slot;
			}
			// A bound on retained bindings, never a draw limit. Unknown
			// frames and overflow keep the existing transient lifetime.
			constexpr std::size_t kMaxLitViews = 256;
			if ( target.frame != 0 && r.litViews.size() < kMaxLitViews )
			{
				r.litViews.push_back( { layout, view.lights, target.shadowAtlas, viewOcclusion,
				    viewReflection, {} } );
				lit = &r.litViews.back().group;
			}
			else
			{
				transientLitViews.emplace_back();
				lit = &transientLitViews.back();
			}
			slot = lit;
		}
		const StageViewLights &lights = *view.lights;
		// The lights' shadow tiles index the slot's atlas: tiles without
		// one would index nothing, which fails the view by name.
		material::SurfaceShadows shadows;
		if ( !lights.shadowTiles.empty() )
		{
			if ( !target.shadowAtlas.IsValid() )
			{
				note( "the view's lights have shadow tiles and the slot no atlas" );
				return nullptr;
			}
			shadows.atlas = target.shadowAtlas;
			shadows.atlasDesc = target.shadowAtlasDesc;
			shadows.tiles = lights.shadowTiles;
		}
		material::SurfaceScreenInputs screen;
		screen.depthAlpha = m.program.depthBlend ? viewDepthAlpha : TextureId();
		screen.depthAlphaRange = m.program.depthBlend ? view.depthAlphaRange : 0.0f;
		screen.depthAlphaSourceWidth = target.width;
		screen.depthAlphaSourceHeight = target.height;
		// The surface program fetches AO at the screen pixel. Models need
		// the same full-size view input as world surfaces.
		if ( viewOcclusion.IsValid() )
		{
			screen.ambientOcclusion = viewOcclusion;
			screen.ambientOcclusionDesc = target.ambientOcclusionDesc;
		}
		screen.planarReflection = viewReflection;
		if ( m.program.sceneColor )
		{
			screen.sceneColor = viewSceneColor;
			screen.sceneColorDesc = viewSceneColorDesc;
		}
		else if ( m.refractInput != 0 )
			screen.sceneColor = viewRefraction;
		material::SurfaceProjectors projectors;
		projectors.lights = lights.projectors;
		projectors.cookies = lights.cookies;
		projectors.cookiesDesc = lights.cookiesDesc;
		material::GroupRequest request = m.resolver->Program().ViewGroup( lights.view,
		    lights.froxels, lights.indices, lights.lights, shadows, projectors, screen );
		for ( auto &buffer : request.storage )
		{
			if ( buffer.binding == 1 && lights.gpuFroxels.IsValid() )
			{
				buffer.external = lights.gpuFroxels;
				buffer.bytes.clear();
			}
			if ( buffer.binding == 2 && lights.gpuIndices.IsValid() )
			{
				buffer.external = lights.gpuIndices;
				buffer.bytes.clear();
			}
		}
		std::string why;
		if ( request.layout != m.program.request.viewLayout ||
		     !buildGroup( request, {}, *lit, &why ) )
		{
			s.ReleaseGroup( *lit, CompletionToken() );
			note( "the view's lights: " +
			      ( why.empty() ? std::string( "another view layout" ) : why ) );
			return nullptr;
		}
		return lit;
	}
	if ( m.viewInput != 0 || m.refractInput != 0 || m.program.sceneColor || m.program.depthBlend )
	{
		Group &reflect = m.program.depthBlend   ? depthViews[layout]
		                 : m.program.sceneColor ? sceneViews[layout]
		                 : m.refractInput != 0  ? refractViews[layout]
		                                        : reflectViews[layout];
		if ( reflect.group.IsValid() )
			return &reflect;
		material::SurfaceScreenInputs screen;
		screen.depthAlpha = m.program.depthBlend ? viewDepthAlpha : TextureId();
		screen.depthAlphaRange = m.program.depthBlend ? view.depthAlphaRange : 0.0f;
		screen.depthAlphaSourceWidth = target.width;
		screen.depthAlphaSourceHeight = target.height;
		screen.planarReflection = viewReflection;
		if ( m.program.sceneColor )
		{
			screen.sceneColor = viewSceneColor;
			screen.sceneColorDesc = viewSceneColorDesc;
		}
		else if ( m.refractInput != 0 )
			screen.sceneColor = viewRefraction;
		const material::GroupRequest request = m.resolver->Program().NeutralViewGroup( screen );
		std::string why;
		if ( request.layout != m.program.request.viewLayout ||
		     !buildGroup( request, {}, reflect, &why ) )
		{
			s.ReleaseGroup( reflect, CompletionToken() );
			note( "the view's planar reflection: " +
			      ( why.empty() ? std::string( "another view layout" ) : why ) );
			return nullptr;
		}
		return &reflect;
	}
	Group &group = r.viewGroups[layout];
	if ( group.group.IsValid() )
		return &group;
	if ( !m.program.request.neutralView )
	{
		note( "a program reads a view group and names no neutral one" );
		return nullptr;
	}
	std::string why;
	if ( !buildGroup( *m.program.request.neutralView, {}, group, &why ) )
	{
		s.ReleaseGroup( group, CompletionToken() );
		note( "a view group: " + why );
		return nullptr;
	}
	return &group;
}

bool WorldPass::Batch::PrepareFrameTerms()
{
	std::copy_n( view.motionToClip, 16, terms.motionCurrentToClip );
	std::copy_n( view.previousToClip, 16, terms.motionPreviousToClip );
	terms.motionExtent[0] = view.viewport.width;
	terms.motionExtent[1] = view.viewport.height;
	terms.motionExtent[2] = view.previousViewValid ? 1.0f : 0.0f;
	std::memcpy( terms.clipPlanes, target.clipPlanes, sizeof( terms.clipPlanes ) );
	terms.lightmapScale = target.lightmapScale;
	terms.outputScale = target.outputScale;
	terms.encodeOutput = target.encodeOutput;
	terms.fogType = target.fogType;
	std::copy( target.fogColor, target.fogColor + 3, terms.fogColor );
	std::copy( target.fogParams, target.fogParams + 4, terms.fogParams );
	terms.fogEyeZ = target.fogEyeZ;
	std::copy( target.eye, target.eye + 3, terms.eye );
	terms.envmapScale = target.envmapScale;
	terms.specular = target.specular;
	terms.ssbumpNormalized = target.ssbumpNormalized;
	// The view's area lights (every point reads them).
	if ( view.lights )
		terms.areas = view.lights->areas;
	terms.time = target.time;
	std::memcpy( terms.foliage, target.foliage, sizeof( terms.foliage ) );
	terms.foliageAvailable = target.foliageAvailable;
	if ( !view.previousViewValid )
		std::copy_n( terms.foliage[0], 4, terms.foliage[1] );
	terms.waterReflectTintScale = target.waterReflectTintScale;
	std::copy( view.viewRight, view.viewRight + 2, terms.viewRight );
	if ( view.viewport.width > 0.0f && view.viewport.height > 0.0f )
	{
		terms.viewport[0] = view.viewport.x;
		terms.viewport[1] = view.viewport.y;
		terms.viewport[2] = 1.0f / view.viewport.width;
		terms.viewport[3] = 1.0f / view.viewport.height;
	}
	if ( world->stage )
	{
		// The stage's pages hold linear light (LMAP); the pbr point's tables
		// and the map's probes, with the host's change volume as the second
		// probe atlas (kSurfaceProbeBounce).
		terms.lightmapScale = 1.0f;
		terms.splitSumTable = kStageSplitSum;
		terms.ltcTable = kStageLtc;
		if ( world->stage->probes )
		{
			terms.map.probeAtlas = kStageProbeAtlas;
			terms.map.probeGrids = kStageProbeGrids;
			terms.map.probeBounce = r.stageTextures[kStageChange];
			terms.map.probeBounceDesc = r.stageDescs[kStageChange];
		}

		// The view's sun.
		if ( view.lights )
		{
			std::copy(
			    view.lights->sunDirection, view.lights->sunDirection + 4, terms.sunDirection );
			std::copy( view.lights->sunColor, view.lights->sunColor + 4, terms.sunColor );
			std::copy( view.lights->sunShadow, view.lights->sunShadow + 4, terms.sunShadow );
		}
	}
	if ( world->reflection )
	{
		terms.map.reflectionProbes = kStageReflection;
		terms.map.reflectionBuffer = r.reflectionBuffer;
		terms.splitSumTable = kStageSplitSum;
	}
	// Presence comes from this slot's actual frame/view inputs, never a quality
	// setting. Uniform light data and all shadow filters remain unchanged.
	viewFeatures = terms.areas.empty() ? 0u : material::kSurfaceViewAreas;
	if ( terms.sunColor[0] != 0.0f || terms.sunColor[1] != 0.0f || terms.sunColor[2] != 0.0f )
		viewFeatures |= material::kSurfaceViewSun;
	if ( view.lights && view.lights->view.counts[0] > 0.0f )
		viewFeatures |= material::kSurfaceViewProjectors;
	// All-zero planes never clip; any other plane keeps the clip test.
	if ( std::any_of( &target.clipPlanes[0][0], &target.clipPlanes[0][0] + 24,
	         []( float value )
	         {
		         return value != 0.0f;
	         } ) )
		viewFeatures |= material::kSurfaceViewClipPlanes;
	if ( target.softShadows )
		viewFeatures |= material::kSurfaceViewSoftShadows;
	if ( target.probeBounce )
		viewFeatures |= material::kSurfaceViewProbeBounce;
	return true;
}

bool WorldPass::Batch::PrepareViewGroups()
{
	viewDepthAlpha =
	    view.depthAlphaHandle > 0 ? textures.Import( view.depthAlphaHandle, false ) : TextureId();
	preparation.Select( "prepare world materials" );
	return true;
}

} // namespace render::pass::world
