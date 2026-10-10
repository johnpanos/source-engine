//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): a batch's resolvers, neutral inputs and world stage textures.
//
//=============================================================================//

#include "world_batch.h"

namespace render::pass::world
{

// A new resolver (a map load): record the pipelines its program creates,
// and create the previous runs' ones now, before any draw needs them, so
// their driver compiles land in the load instead of mid-play.
void WorldPass::Batch::prewarmResolver( material::ProgramResolver &resolver, const char *which )
{
	material::SurfaceProgram &program = resolver.Program();
	// Each key is tagged with its resolver: the two make different points.
	const std::string tag = std::string( which ) + " ";
	program.SetCreatedSink(
	    [&s = s, tag]( const std::string &key )
	    {
		    std::lock_guard<std::mutex> guard( s.pipelineKeyLock );
		    s.createdKeys.insert( tag + key );
	    } );
	std::vector<std::string> keys;
	{
		std::lock_guard<std::mutex> guard( s.pipelineKeyLock );
		for ( const std::string &key : s.prewarmKeys )
			if ( key.starts_with( tag ) )
				keys.push_back( key.substr( tag.size() ) );
	}
	if ( keys.empty() )
		return;
	const auto started = std::chrono::steady_clock::now();
	const std::size_t created = program.Prewarm( keys );
	std::fprintf( stderr, "[render.pass.world] prewarmed %zu of %zu %s pipelines in %lld ms\n",
	    created, keys.size(), which,
	    static_cast<long long>( std::chrono::duration_cast<std::chrono::milliseconds>(
	        std::chrono::steady_clock::now() - started )
	            .count() ) );
}

// A world stage's textures: made at the first slot from the stage, then
// updated in place as its lighting changes (SetStageLightmap,
// SetStageChange), so the groups that bind them stay valid. Each upload
// has an initialized upload buffer of its own, released behind a later frame.
bool WorldPass::Batch::stageUpload( TextureId texture, const TextureDesc &desc,
    std::span<const std::byte> bytes, ResourceUsage from )
{
	// D25: one initialized host-visible copy-source buffer; no device-local
	// staging buffer and no pass through the upload ring.
	auto buffer = device.CreateUploadBuffer( bytes );
	if ( !buffer )
		return false;
	s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
	encoder.TransitionTexture( texture, from, ResourceUsage::kCopyDestination );
	TextureBufferCopy copy;
	copy.width = desc.width;
	copy.height = desc.height;
	encoder.CopyBufferToTexture( buffer.Value(), texture, copy );
	encoder.TransitionTexture( texture, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	return true;
}

// Rectangles of an RGBA16F stage texture from their packed texels.
bool WorldPass::Batch::stageRegions( TextureId texture, const TextureDesc &desc,
    const std::vector<StageRegion> &regions, std::span<const std::byte> texels )
{
	if ( regions.empty() )
		return true;
	if ( texels.empty() )
		return false;
	auto buffer = device.CreateUploadBuffer( texels );
	if ( !buffer )
		return false;
	s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
	encoder.TransitionTexture( texture, ResourceUsage::kSampled, ResourceUsage::kCopyDestination );
	std::uint64_t offset = 0;
	for ( const StageRegion &region : regions )
	{
		if ( region.x + region.width > desc.width || region.y + region.height > desc.height ||
		     offset + std::uint64_t( region.width ) * region.height * 8 > texels.size() )
			return false;
		TextureBufferCopy copy;
		copy.bufferOffset = offset;
		copy.x = region.x;
		copy.y = region.y;
		copy.width = region.width;
		copy.height = region.height;
		encoder.CopyBufferToTexture( buffer.Value(), texture, copy );
		offset += std::uint64_t( region.width ) * region.height * 8;
	}
	encoder.TransitionTexture( texture, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	return true;
}

bool WorldPass::Batch::stageMake( const char *name, Format format, std::uint32_t width,
    std::uint32_t height, std::span<const std::byte> bytes )
{
	if ( width == 0 || height == 0 || bytes.size() != device::RegionBytes( format, width, height ) )
		return false;
	TextureDesc desc;
	desc.format = format;
	desc.width = width;
	desc.height = height;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	desc.debugName = name;
	auto texture = device.CreateTexture( desc );
	if ( !texture )
		return false;
	r.stageTextures[name] = texture.Value();
	desc.debugName = {};
	r.stageDescs[name] = desc;
	return stageUpload( texture.Value(), desc, bytes, ResourceUsage::kUndefined );
}

// The map's reflection probes (RPRB v8): the BC6H radiance cube array,
// uploaded as the lump stores it (a copy per mip, probe and face, from one
// initialized upload buffer), and the probe buffer's bytes for the frame
// groups. A device without BC6H cube arrays refuses the stage by name.
bool WorldPass::Batch::stageMakeReflection( const StageReflectionProbes &probes )
{
	const CapabilitySet caps = device.Facts().capabilities;
	const bool blocks = probes.format == Format::kBC6HUfloat;
	if ( ( blocks && !caps.Has( Capability::kTextureCompressionBC ) ) ||
	     !caps.Has( Capability::kCubeArrays ) )
	{
		s.Fail( blocks ? "the map's reflection probes need BC6H cube arrays, which the device lacks"
		               : "the map's reflection probes need cube arrays, which the device lacks" );
		return false;
	}
	if ( probes.count == 0 || probes.count > 256 || probes.mips == 0 || probes.face == 0 ||
	     ( probes.face >> ( probes.mips - 1 ) ) < 4 || probes.buffer.empty() ||
	     probes.baseMip >= probes.mips )
		return false;
	// The array holds the mips from baseMip on (a texture setting drops the
	// top ones); the probe buffer's word 7 shifts the shader's lods.
	const std::uint32_t face = probes.face >> probes.baseMip;
	const std::uint32_t mips = probes.mips - probes.baseMip;
	std::uint64_t total = 0;
	for ( std::uint32_t mip = 0; mip < mips; ++mip )
		total += std::uint64_t( probes.count ) * 6 *
		         device::RegionBytes( probes.format, face >> mip, face >> mip );
	if ( probes.radiance.size() != total )
		return false;
	TextureDesc desc;
	desc.dimension = TextureDimension::kCube;
	desc.format = probes.format;
	desc.width = desc.height = face;
	// Two cubes at least: one probe's six layers would be a plain cube, not
	// the cube array the program reads (the second cube is never indexed).
	desc.depthOrLayers = 6 * std::max( probes.count, 2u );
	desc.mipLevels = mips;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	desc.debugName = "world reflection probes";
	auto texture = device.CreateTexture( desc );
	if ( !texture )
		return false;
	r.stageTextures[kStageReflection] = texture.Value();
	desc.debugName = {};
	r.stageDescs[kStageReflection] = desc;
	auto buffer = device.CreateUploadBuffer( probes.radiance );
	if ( !buffer )
		return false;
	s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
	encoder.TransitionTexture(
	    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	std::uint64_t offset = 0;
	for ( std::uint32_t mip = 0; mip < mips; ++mip )
	{
		const std::uint32_t size = face >> mip;
		const std::uint64_t faceBytes = device::RegionBytes( probes.format, size, size );
		for ( std::uint32_t layer = 0; layer < 6 * probes.count; ++layer )
		{
			TextureBufferCopy copy;
			copy.bufferOffset = offset;
			copy.mip = mip;
			copy.layer = layer;
			copy.width = copy.height = size;
			encoder.CopyBufferToTexture( buffer.Value(), texture.Value(), copy );
			offset += faceBytes;
		}
	}
	encoder.TransitionTexture(
	    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	const std::byte *words = reinterpret_cast<const std::byte *>( probes.buffer.data() );
	r.reflectionBuffer = std::make_shared<const std::vector<std::byte>>(
	    words, words + probes.buffer.size() * sizeof( std::uint32_t ) );
	return true;
}

// The grid table's rows, with room for the moving occluders' rows.
std::vector<float> WorldPass::Batch::paddedTable(
    const StageProbeVolume &table, std::uint32_t rows )
{
	std::vector<float> padded( std::size_t( table.tableTexels ) * 4 * rows, 0.0f );
	std::copy_n(
	    table.table.begin(), std::min( padded.size(), table.table.size() ), padded.begin() );
	return padded;
}

bool WorldPass::Batch::PrepareResources()
{
	if ( !r.resolver )
	{
		auto resolver = material::ProgramResolver::Create( device, target.colorFormat,
		    target.depthFormat, target.samples, material::VertexLayout::kSurface,
		    material::ProgramModules{ s.fragmentModule } );
		if ( !resolver )
		{
			s.Fail( resolver.Error() );
			return false;
		}
		r.resolver = std::move( resolver ).Value();
		prewarmResolver( *r.resolver, "world" );
		// The resolver's points draw with the scene terms the stage supports,
		// and none without one, as the model resolver's do: a plain map's
		// mesh handoff (VertexLitGeneric and Refract models) needs the mesh
		// points too. World pbr surfaces stay a stage's alone; their claim
		// (ClaimForDrawing's worldPbr) asks for the stage itself.
		r.resolver->SetWorldPbr(
		    true, WorldTerms( *world, target.runtimeDirect, target.ambientOcclusionTerm ) );
		r.resolver->SetSceneColorAvailable( true );
		r.materials.resize( world->materials.size() );
	}
	if ( ( !view.props.empty() || !view.posedModels.empty() ) && !r.modelResolver )
	{
		auto resolver = material::ProgramResolver::Create( device, target.colorFormat,
		    target.depthFormat, target.samples, material::VertexLayout::kModel,
		    material::ProgramModules{ s.fragmentModule } );
		if ( !resolver )
		{
			s.Fail( "the static model resolver: " + resolver.Error() );
			return false;
		}
		r.modelResolver = std::move( resolver ).Value();
		r.modelResolver->SetWorldPbr(
		    true, WorldTerms( *world, target.runtimeDirect, target.ambientOcclusionTerm ) );
		r.modelResolver->SetSceneColorAvailable( true );
		r.modelMaterials.resize( world->materials.size() );
		prewarmResolver( *r.modelResolver, "model" );
	}
	if ( !r.uploaded )
	{
		const auto vertexBytes = std::as_bytes( std::span( world->vertices ) );
		const auto indexBytes = std::as_bytes( std::span( world->indices ) );
		if ( vertexBytes.empty() && indexBytes.empty() )
		{
			r.uploaded = true; // a model-only world has no BSP vertex/index buffers
		}
		else
		{
			BufferDesc desc;
			desc.size = vertexBytes.size();
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
			desc.debugName = "world vertices";
			auto vertices = device.CreateBuffer( desc );
			desc.size = indexBytes.size();
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kIndex };
			desc.debugName = "world indices";
			auto indices = device.CreateBuffer( desc );
			if ( !vertices || !indices || vertexBytes.empty() || indexBytes.empty() )
			{
				if ( vertices )
					(void)device.Release( vertices.Value(), CompletionToken() );
				if ( indices )
					(void)device.Release( indices.Value(), CompletionToken() );
				s.Fail( "the world's buffers were refused" );
				return false;
			}
			r.vertices = vertices.Value();
			r.indices = indices.Value();
			encoder.TransitionBuffer(
			    r.vertices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( r.vertices, 0, vertexBytes );
			encoder.TransitionBuffer(
			    r.vertices, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
			encoder.TransitionBuffer(
			    r.indices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( r.indices, 0, indexBytes );
			encoder.TransitionBuffer(
			    r.indices, ResourceUsage::kCopyDestination, ResourceUsage::kIndex );
			r.uploaded = true;
		}
	}
	if ( world->stage && !r.stageMade )
	{
		const WorldStage &stage = *world->stage;
		const LightmapPages &pages = stage.lightmap;
		const LightmapPages &indirect = stage.indirect;
		bool made =
		    stageMake( kStageLightmap, pages.flatFormat, pages.width, pages.height, pages.flat );
		if ( made && pages.Directional() )
			made = stageMake(
			    kStageGradient, pages.gradientFormat, pages.width, pages.height, pages.gradient );
		if ( made && !indirect.flat.empty() )
			made = stageMake(
			    kStageIndirect, indirect.flatFormat, pages.width, pages.height, indirect.flat );
		if ( made && indirect.Directional() )
			made = stageMake( kStageIndirectGradient, indirect.gradientFormat, pages.width,
			    pages.height, indirect.gradient );
		if ( made && !stage.shadowMask.flat.empty() )
			made = stageMake( kStageShadowMask, stage.shadowMask.flatFormat, stage.shadowMask.width,
			    stage.shadowMask.height, stage.shadowMask.flat );
		if ( made && stage.probes )
		{
			const StageProbeVolume &probes = *stage.probes;
			const std::vector<std::byte> zero( probes.atlas.size(), std::byte( 0 ) );
			const std::vector<float> table =
			    paddedTable( probes, probes.rows + kStageOccluderRows );
			made = stageMake( kStageProbeAtlas, Format::kRGBA16Float, probes.atlasWidth,
			           probes.atlasHeight, probes.atlas ) &&
			       stageMake( kStageChange, Format::kRGBA16Float, probes.atlasWidth,
			           probes.atlasHeight, zero ) &&
			       stageMake( kStageProbeGrids, Format::kRGBA32Float, probes.tableTexels,
			           probes.rows + kStageOccluderRows, std::as_bytes( std::span( table ) ) );
		}
		for ( const auto &[name, table] : { std::pair{ kStageSplitSum, material::SplitSumTable() },
		          std::pair{ kStageLtc, material::LtcTable() } } )
		{
			if ( made )
				made = stageMake( name, table.format, table.width, table.height,
				    std::as_bytes( std::span( table.texels ) ) );
		}
		if ( !made )
		{
			s.Fail( "the world stage's textures were refused" );
			return false;
		}
		r.stageMade = true;
	}
	// The map's reflection probes: a stage's RPRB, or a plain map's cubemaps.
	// A plain map has no stage tables, so it makes the split-sum table the
	// probes' specular reads.
	if ( world->reflection && !r.reflectionMade )
	{
		bool made = stageMakeReflection( *world->reflection );
		if ( made && !world->stage )
		{
			const material::PbrSplitSumTable table = material::SplitSumTable();
			made = stageMake( kStageSplitSum, table.format, table.width, table.height,
			    std::as_bytes( std::span( table.texels ) ) );
		}
		if ( !made )
		{
			s.Fail( "the map's reflection probes were refused" );
			return false;
		}
		r.reflectionMade = true;
	}
	if ( world->stage )
	{
		// Under the lock: the page's base and parts are the state's (the
		// uploads copy what they read at once).
		std::unique_lock<std::mutex> guard( s.lock );
		if ( r.stageLightmapRevision != s.stageLightmapRevision )
		{
			const LightmapPages &own = world->stage->lightmap;
			const TextureDesc &desc = r.stageDescs[kStageLightmap];
			// A set behind the base takes it whole, then the parts after it.
			const bool whole = r.stageLightmapRevision < s.stageLightmapBaseRevision;
			const LightmapPages &base = s.stageLightmap ? *s.stageLightmap : own;
			bool fits = true;
			// Region patches are RGBA16F texels: block pages take none (the
			// composition restages decoded pages before sending any).
			if ( !s.stageLightmapPatches.empty() && base.Blocks() )
				fits = false;
			if ( whole && fits )
				fits = base.width == desc.width && base.height == desc.height &&
				       base.flatFormat == desc.format && base.Directional() == own.Directional() &&
				       stageUpload( r.stageTextures[kStageLightmap], desc, base.flat,
				           ResourceUsage::kSampled ) &&
				       ( !base.Directional() || stageUpload( r.stageTextures[kStageGradient], desc,
				                                    base.gradient, ResourceUsage::kSampled ) );
			for ( const State::StagePatch &patch : s.stageLightmapPatches )
			{
				if ( !fits || ( !whole && patch.revision <= r.stageLightmapRevision ) )
					continue;
				fits = stageRegions(
				    r.stageTextures[kStageLightmap], desc, patch.regions, patch.texels );
			}
			if ( !fits )
			{
				guard.unlock();
				s.Fail( "the world stage's lightmap update does not fit its pages" );
				return false;
			}
			r.stageLightmapRevision = s.stageLightmapRevision;
			// The parts every resource set holds fold into the base.
			std::uint64_t applied = r.stageLightmapRevision;
			for ( const Resources &set : s.variants )
				if ( set.stageMade )
					applied = std::min( applied, set.stageLightmapRevision );
			while ( !s.stageLightmapPatches.empty() &&
			        s.stageLightmapPatches.front().revision <= applied )
			{
				const State::StagePatch &patch = s.stageLightmapPatches.front();
				if ( !s.stageLightmap )
					s.stageLightmap = std::make_shared<LightmapPages>( own );
				LightmapPages &pages = *s.stageLightmap;
				std::size_t offset = 0;
				for ( const StageRegion &region : patch.regions )
				{
					const std::size_t row = std::size_t( region.width ) * 8;
					for ( std::uint32_t y = 0; y < region.height; ++y )
					{
						const std::size_t at =
						    ( std::size_t( region.y + y ) * pages.width + region.x ) * 8;
						if ( at + row <= pages.flat.size() && offset + row <= patch.texels.size() )
							std::memcpy(
							    pages.flat.data() + at, patch.texels.data() + offset, row );
						offset += row;
					}
				}
				s.stageLightmapBaseRevision = patch.revision;
				s.stageLightmapPatches.pop_front();
			}
		}
	}
	if ( world->stage && world->stage->probes )
	{
		std::unique_lock<std::mutex> guard( s.lock );
		if ( r.stageProbeRevision != s.stageProbeRevision )
		{
			const StageProbeVolume &probes = *world->stage->probes;
			const TextureDesc &desc = r.stageDescs[kStageProbeAtlas];
			const bool whole = r.stageProbeRevision < s.stageProbeBaseRevision;
			const std::vector<std::byte> &base =
			    s.stageProbeAtlas ? *s.stageProbeAtlas : probes.atlas;
			bool fits = true;
			if ( whole )
				fits = base.size() == probes.atlas.size() &&
				       stageUpload(
				           r.stageTextures[kStageProbeAtlas], desc, base, ResourceUsage::kSampled );
			for ( const State::StagePatch &patch : s.stageProbePatches )
			{
				if ( !fits || ( !whole && patch.revision <= r.stageProbeRevision ) )
					continue;
				fits = stageRegions(
				    r.stageTextures[kStageProbeAtlas], desc, patch.regions, patch.texels );
			}
			if ( !fits )
			{
				guard.unlock();
				s.Fail( "the world stage's probe atlas update does not fit its volume" );
				return false;
			}
			r.stageProbeRevision = s.stageProbeRevision;
			std::uint64_t applied = r.stageProbeRevision;
			for ( const Resources &set : s.variants )
				if ( set.stageMade )
					applied = std::min( applied, set.stageProbeRevision );
			while (
			    !s.stageProbePatches.empty() && s.stageProbePatches.front().revision <= applied )
			{
				const State::StagePatch &patch = s.stageProbePatches.front();
				if ( !s.stageProbeAtlas )
					s.stageProbeAtlas = std::make_shared<std::vector<std::byte>>( probes.atlas );
				std::size_t offset = 0;
				for ( const StageRegion &region : patch.regions )
				{
					const std::size_t row = std::size_t( region.width ) * 8;
					for ( std::uint32_t y = 0; y < region.height; ++y )
					{
						const std::size_t at =
						    ( std::size_t( region.y + y ) * probes.atlasWidth + region.x ) * 8;
						std::memcpy(
						    s.stageProbeAtlas->data() + at, patch.texels.data() + offset, row );
						offset += row;
					}
				}
				s.stageProbeBaseRevision = patch.revision;
				s.stageProbePatches.pop_front();
			}
		}
	}
	if ( world->stage && world->stage->probes )
	{
		// Under the lock: the change's base and parts are the state's (the
		// uploads copy what they read at once).
		std::unique_lock<std::mutex> guard( s.lock );
		if ( s.stageTable && r.stageChangeRevision != s.stageChangeRevision )
		{
			const StageProbeVolume &probes = *world->stage->probes;
			const std::uint32_t rows = probes.rows + kStageOccluderRows;
			const StageProbeVolume &stageTable = *s.stageTable;
			const bool whole = r.stageChangeRevision < s.stageBaseRevision;
			bool fits = stageTable.tableTexels == probes.tableTexels && stageTable.rows <= rows;
			if ( fits && whole )
			{
				const std::vector<std::byte> zero(
				    s.stageChangeBase.empty() ? probes.atlas.size() : 0, std::byte( 0 ) );
				const std::vector<std::byte> &change =
				    s.stageChangeBase.empty() ? zero : s.stageChangeBase;
				fits = change.size() == probes.atlas.size() &&
				       stageUpload( r.stageTextures[kStageChange], r.stageDescs[kStageChange],
				           change, ResourceUsage::kSampled );
			}
			for ( const State::StagePatch &patch : s.stagePatches )
			{
				if ( !fits || ( !whole && patch.revision <= r.stageChangeRevision ) )
					continue;
				fits = stageRegions( r.stageTextures[kStageChange], r.stageDescs[kStageChange],
				    patch.regions, patch.texels );
			}
			const std::vector<float> table = paddedTable( stageTable, rows );
			if ( !fits ||
			     !stageUpload( r.stageTextures[kStageProbeGrids], r.stageDescs[kStageProbeGrids],
			         std::as_bytes( std::span( table ) ), ResourceUsage::kSampled ) )
			{
				guard.unlock();
				s.Fail( "the world stage's probe change does not fit its volume" );
				return false;
			}
			r.stageChangeRevision = s.stageChangeRevision;
			// The parts every resource set holds fold into the base.
			std::uint64_t applied = r.stageChangeRevision;
			for ( const Resources &set : s.variants )
				if ( set.stageMade )
					applied = std::min( applied, set.stageChangeRevision );
			while ( !s.stagePatches.empty() && s.stagePatches.front().revision <= applied )
			{
				const State::StagePatch &patch = s.stagePatches.front();
				if ( s.stageChangeBase.empty() )
					s.stageChangeBase.assign( probes.atlas.size(), std::byte( 0 ) );
				std::size_t offset = 0;
				for ( const StageRegion &region : patch.regions )
				{
					for ( std::uint32_t y = 0; y < region.height; ++y )
					{
						const std::size_t row = std::size_t( region.width ) * 8;
						const std::size_t at =
						    ( std::size_t( region.y + y ) * probes.atlasWidth + region.x ) * 8;
						if ( at + row <= s.stageChangeBase.size() &&
						     offset + row <= patch.texels.size() )
							std::memcpy(
							    s.stageChangeBase.data() + at, patch.texels.data() + offset, row );
						offset += row;
					}
				}
				s.stageBaseRevision = patch.revision;
				s.stagePatches.pop_front();
			}
		}
	}
	return true;
}

} // namespace render::pass::world
