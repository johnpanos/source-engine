//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): a world stage's lighting pages, probes and reflections on the device.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

namespace
{

// Decoded pages: the sun's mask is any gradient alpha below one (half 0.999
// is 0x3BFE; positive halves order as integers).
bool GradientHasSunMask( const pass::world::LightmapPages &pages )
{
	if ( pages.Blocks() )
		return false;
	for ( std::size_t t = 0; t + 8 <= pages.gradient.size(); t += 8 )
	{
		std::uint16_t alpha;
		std::memcpy( &alpha, pages.gradient.data() + t + 6, sizeof( alpha ) );
		if ( ( alpha & 0x8000u ) == 0 && alpha < 0x3BFEu )
			return true;
	}
	return false;
}

} // namespace

bool CoreWorld::StageCapture::UploadLightmap(
    const world_mesh_gpu::WorldLightmapUploadRequest &request )
{
	if ( request.regions )
	{
		// A partial update of the total layer: rectangles of its flat page
		// (a directional layer's left half), patched into this capture's
		// copy and sent to the pass as they are. One in the gradient half
		// sends the whole page. Block pages take no texels: they become
		// RGBA16F pages first, and the stage is set again with them.
		if ( lightmap.Blocks() )
		{
			if ( !DecodePages() )
				return false;
			if ( m_Owner.m_StageSet )
				m_Owner.SetStage();
		}
		if ( lightmap.flat.empty() || request.height != lightmap.height ||
		     request.width != ( lightmap.Directional() ? 2 : 1 ) * lightmap.width )
			return false;
		std::vector<pass::world::WorldPass::StageRegion> regions;
		bool flatOnly = true;
		std::size_t offset = 0;
		for ( std::uint32_t i = 0; i < request.regionCount; ++i )
		{
			const world_mesh_gpu::ProbeAtlasRegion &region = request.regions[i];
			if ( region.x + region.width > lightmap.width ||
			     region.y + region.height > lightmap.height )
				flatOnly = false;
			regions.push_back( { region.x, region.y, region.width, region.height } );
			offset += std::size_t( region.width ) * region.height * 8;
		}
		const std::byte *texels = static_cast<const std::byte *>( request.regionTotal );
		if ( offset && !texels )
			return false;
		// The capture's copy, as the pass's page will be.
		std::size_t at = 0;
		for ( const auto &region : regions )
		{
			const std::size_t row = std::size_t( region.width ) * 8;
			for ( std::uint32_t y = 0; y < region.height; ++y, at += row )
			{
				const std::uint32_t py = region.y + y;
				for ( std::uint32_t x = 0; x < region.width; ++x )
				{
					const std::uint32_t px = region.x + x;
					const std::byte *texel = texels + at + std::size_t( x ) * 8;
					if ( px < lightmap.width )
					{
						std::memcpy(
						    lightmap.flat.data() + ( std::size_t( py ) * lightmap.width + px ) * 8,
						    texel, 8 );
						continue;
					}
					// A gradient texel arrives as signed beta: stored as
					// beta / 4 + 0.5, the sun's alpha kept.
					std::byte *page =
					    lightmap.gradient.data() +
					    ( std::size_t( py ) * lightmap.width + px - lightmap.width ) * 8;
					for ( int c = 0; c < 3; ++c )
					{
						std::uint16_t half;
						std::memcpy( &half, texel + 2 * c, 2 );
						half = mapcontainer::FloatToHalf( std::clamp(
						    mapcontainer::HalfToFloat( half ) * 0.25f + 0.5f, 0.0f, 1.0f ) );
						std::memcpy( page + 2 * c, &half, 2 );
					}
				}
			}
		}
		if ( m_Owner.m_StageSet )
		{
			if ( flatOnly )
				m_Owner.m_Pass.SetStageLightmapRegions(
				    std::move( regions ), std::vector<std::byte>( texels, texels + offset ) );
			else
				m_Owner.m_Pass.SetStageLightmap( lightmap );
		}
		return true;
	}
	// The total layer's pages (the baked diffuse light, as render_lab draws
	// it without runtime direct light) and the indirect layer's pages (its
	// gradient page when the bake wrote its own): the LMAP v3 lump's blocks
	// when the request carries it, else the decoded layers.
	pass::world::LightmapPages total;
	pass::world::LightmapPages indirect;
	std::vector<std::byte> lump;
	bool lumpSun = false;
	mapcontainer::WorldLightmapBlocks blocks;
	if ( request.lmap && mapcontainer::ValidateWorldLightmap( request.lmap,
	                         std::size_t( request.lmapBytes ), mapcontainer::kWorldLightmapVersion,
	                         &blocks ) == mapcontainer::WorldLightmapError::Ok )
	{
		const std::byte *bytes = static_cast<const std::byte *>( request.lmap );
		lump.assign( bytes, bytes + request.lmapBytes );
		lumpSun = blocks.sun;
		for ( std::uint32_t i = 0; i < blocks.layerCount; ++i )
		{
			pass::world::LightmapPages pages =
			    pass::world::BlockLightmapLayer( blocks.width, blocks.height,
			        std::span( bytes + blocks.irradianceOffset[i], blocks.irradianceBytes ),
			        std::span( bytes + blocks.gradientOffset[i], blocks.gradientBytes ) );
			if ( blocks.roles[i] == mapcontainer::WorldLightmapLayer::Total )
				total = std::move( pages );
			else if ( blocks.roles[i] == mapcontainer::WorldLightmapLayer::Indirect )
				indirect = std::move( pages );
		}
	}
	else
	{
		const std::size_t layerBytes = std::size_t( request.width ) * request.height * 8;
		for ( std::uint32_t i = 0;
		    i < request.layerCount && i < world_mesh_gpu::kWorldLightmapMaxUploadLayers; ++i )
		{
			if ( !request.layers[i] )
				continue;
			const std::span<const std::byte> layer(
			    static_cast<const std::byte *>( request.layers[i] ), layerBytes );
			if ( request.roles[i] == world_mesh_gpu::WorldLightmapRole::Total )
				total = pass::world::SplitLightmapLayer( layer, request.width, request.height );
			else if ( request.roles[i] == world_mesh_gpu::WorldLightmapRole::Indirect )
				indirect = pass::world::SplitLightmapLayer( layer, request.width, request.height );
		}
		lumpSun = GradientHasSunMask( total );
	}
	if ( total.flat.empty() )
		return false;
	// The static lights' baked shadow masks at the total page's size.
	pass::world::LightmapPages masks;
	std::shared_ptr<std::vector<mapcontainer::LightShadowMaskRecord>> maskLights;
	mapcontainer::LightShadowMasks maskLayout;
	if ( request.lsmk &&
	     mapcontainer::ValidateLightShadowMasks(
	         request.lsmk, std::size_t( request.lsmkBytes ), &maskLayout ) &&
	     maskLayout.width == total.width && maskLayout.height == total.height )
	{
		const std::byte *bytes = static_cast<const std::byte *>( request.lsmk );
		masks.width = maskLayout.width;
		masks.height = maskLayout.height;
		masks.flatFormat = device::Format::kRGBA16Unorm;
		masks.flat.assign(
		    bytes + maskLayout.pageOffset, bytes + maskLayout.pageOffset + maskLayout.pageBytes );
		maskLights = std::make_shared<std::vector<mapcontainer::LightShadowMaskRecord>>();
		for ( std::uint32_t i = 0; i < maskLayout.lightCount; ++i )
			maskLights->push_back( mapcontainer::LightShadowMaskRecordAt( request.lsmk, i ) );
	}
	// A region update (no lump) keeps the stage's masks.
	if ( request.lsmk || request.lmap )
	{
		shadowMask = std::move( masks );
		this->maskLights = std::move( maskLights );
	}
	// Recomposed while a stage draws: the pass updates its pages in place.
	if ( m_Owner.m_StageSet && !lightmap.flat.empty() && lightmap.flatFormat == total.flatFormat )
	{
		lightmap = total;
		lmap = std::move( lump );
		sunMask = lumpSun;
		m_Owner.m_Pass.SetStageLightmap( std::move( total ) );
		return true;
	}
	// A stage drawing pages of the other kind is set again with these.
	const bool restage = m_Owner.m_StageSet && !lightmap.flat.empty();
	lightmap = std::move( total );
	this->indirect = std::move( indirect );
	lmap = std::move( lump );
	sunMask = lumpSun;
	if ( restage )
		m_Owner.SetStage();
	return true;
}

bool CoreWorld::StageCapture::DecodePages()
{
	mapcontainer::WorldLightmapBlocks blocks;
	std::vector<std::byte> decoded;
	mapcontainer::WorldLightmapLayout layout;
	if ( mapcontainer::ValidateWorldLightmap( lmap.data(), lmap.size(),
	         mapcontainer::kWorldLightmapVersion,
	         &blocks ) != mapcontainer::WorldLightmapError::Ok ||
	     !mapcontainer::DecodeWorldLightmap( lmap.data(), blocks, &decoded, &layout ) )
		return false;
	for ( std::uint32_t i = 0; i < layout.layerCount; ++i )
	{
		pass::world::LightmapPages pages = pass::world::SplitLightmapLayer(
		    std::span( decoded.data() + layout.layerOffset[i], std::size_t( layout.layerBytes ) ),
		    layout.width, layout.height );
		if ( layout.roles[i] == mapcontainer::WorldLightmapLayer::Total )
			lightmap = std::move( pages );
		else if ( layout.roles[i] == mapcontainer::WorldLightmapLayer::Indirect )
			indirect = std::move( pages );
	}
	// The decoded form carries the sun on the total layer only; the block
	// pages carry it on every layer's gradient, which runtime direct light
	// reads from the indirect layer's.
	if ( indirect.Directional() && indirect.gradient.size() == lightmap.gradient.size() )
		for ( std::size_t t = 0; t + 8 <= indirect.gradient.size(); t += 8 )
			std::memcpy( indirect.gradient.data() + t + 6, lightmap.gradient.data() + t + 6, 2 );
	lmap.clear();
	return !lightmap.Blocks();
}

bool CoreWorld::StageCapture::UploadProbeVolume(
    const world_mesh_gpu::ProbeVolumeUploadRequest &request )
{
	if ( !request.gridTable || request.tableFloats % 4 != 0 )
		return false;
	pass::world::StageProbeVolume volume;
	volume.atlasWidth = request.atlasWidth;
	volume.atlasHeight = request.atlasHeight;
	volume.tableTexels = request.tableFloats / 4;
	volume.rows = request.gridCount + request.occluderCount;
	volume.table.assign(
	    request.gridTable, request.gridTable + std::size_t( volume.rows ) * request.tableFloats );
	const std::size_t atlasBytes = std::size_t( request.atlasWidth ) * request.atlasHeight * 8;
	if ( request.regions )
	{
		// A part of the current atlas and optional change: applied to their
		// kept copies, then passed on as regions. Before the first whole
		// volume there is nothing to apply it to.
		if ( !probes )
			return true;
		if ( probes->atlas.size() != atlasBytes || probes->atlasWidth != request.atlasWidth ||
		     probes->atlasHeight != request.atlasHeight ||
		     ( request.regionCount && ( !request.regions || !request.regionAtlas ) ) )
			return false;
		std::vector<pass::world::WorldPass::StageRegion> regions;
		std::vector<std::byte> atlasTexels;
		std::vector<std::byte> texels;
		std::size_t packedBytes = 0;
		for ( std::uint32_t i = 0; i < request.regionCount; ++i )
		{
			const world_mesh_gpu::ProbeAtlasRegion &region = request.regions[i];
			if ( region.x > request.atlasWidth || region.width > request.atlasWidth - region.x ||
			     region.y > request.atlasHeight || region.height > request.atlasHeight - region.y )
				return false;
			regions.push_back( { region.x, region.y, region.width, region.height } );
			packedBytes += std::size_t( region.width ) * region.height * 8;
		}
		const std::byte *atlas = static_cast<const std::byte *>( request.regionAtlas );
		std::size_t atlasOffset = 0;
		for ( const auto &region : regions )
		{
			const std::size_t row = std::size_t( region.width ) * 8;
			for ( std::uint32_t y = 0; y < region.height; ++y )
			{
				std::memcpy(
				    probes->atlas.data() +
				        ( std::size_t( region.y + y ) * request.atlasWidth + region.x ) * 8,
				    atlas + atlasOffset, row );
				atlasOffset += row;
			}
		}
		if ( packedBytes )
			atlasTexels.assign( atlas, atlas + packedBytes );
		if ( request.regionDelta )
		{
			const std::byte *packed = static_cast<const std::byte *>( request.regionDelta );
			if ( change.size() != atlasBytes )
				change.assign( atlasBytes, std::byte( 0 ) );
			std::size_t offset = 0;
			for ( std::uint32_t i = 0; i < request.regionCount; ++i )
			{
				const world_mesh_gpu::ProbeAtlasRegion &region = request.regions[i];
				const std::size_t row = std::size_t( region.width ) * 8;
				for ( std::uint32_t y = 0; y < region.height; ++y )
				{
					std::memcpy(
					    change.data() +
					        ( std::size_t( region.y + y ) * request.atlasWidth + region.x ) * 8,
					    packed + offset, row );
					offset += row;
				}
			}
			texels.assign( packed, packed + offset );
		}
		table = volume;
		if ( m_Owner.m_StageSet )
			m_Owner.m_Pass.SetStageProbeRegions( std::move( regions ), std::move( atlasTexels ),
			    std::move( texels ), std::move( volume ) );
		return true;
	}
	if ( !request.atlas )
		return false;
	// The stage's probe atlas is the first volume published (a traced
	// producer publishes a change from its first frame, so the bake alone may
	// never come): a world surface reads the lightmap and adds the change
	// (kSurfaceProbeBounce); the atlas itself lights only surfaces without a
	// lightmap.
	change.clear();
	if ( request.deltaAtlas )
	{
		const std::byte *delta = static_cast<const std::byte *>( request.deltaAtlas );
		change.assign( delta, delta + atlasBytes );
	}
	table = volume;
	if ( !probes || probes->atlas.size() != atlasBytes ||
	     probes->atlasWidth != request.atlasWidth || probes->atlasHeight != request.atlasHeight )
	{
		const bool late = m_Owner.m_StageSet;
		const std::byte *atlas = static_cast<const std::byte *>( request.atlas );
		// The stage's own table has the grids' rows alone.
		pass::world::StageProbeVolume first = volume;
		first.atlas.assign( atlas, atlas + atlasBytes );
		first.rows = request.gridCount;
		first.table.resize( std::size_t( first.rows ) * request.tableFloats );
		probes = std::move( first );
		// The stage was set before its first volume arrived: set it again
		// with it (SetStage passes the change on).
		if ( late )
		{
			m_Owner.SetStage();
			return true;
		}
	}
	const std::byte *atlas = static_cast<const std::byte *>( request.atlas );
	probes->atlas.assign( atlas, atlas + atlasBytes );
	if ( m_Owner.m_StageSet )
	{
		m_Owner.m_Pass.SetStageProbeVolume( probes->atlas, change, std::move( volume ) );
	}
	return true;
}

bool CoreWorld::StageCapture::UploadReflectionProbes(
    const world_mesh_gpu::ReflectionProbesUploadRequest &request )
{
	reflection.reset();
	if ( !request.data )
		return true; // the map's probes removed
	if ( !request.buffer || request.radianceBytes > request.dataBytes || !request.probeCount ||
	     !request.mipCount || !request.faceSize )
		return false;
	// Copies (the caller owns the request's bytes until the call returns);
	// the BC6H radiance goes to the pass as stored.
	pass::world::StageReflectionProbes probes;
	probes.count = request.probeCount;
	probes.mips = request.mipCount;
	probes.face = request.faceSize;
	probes.relight = request.relight;
	probes.baseMip = std::min( request.baseMip, request.mipCount - 1 );
	probes.buffer.assign( request.buffer, request.buffer + request.bufferWords );
	// A plain map's cubemaps arrive as RGBA16F texels, not BC6H blocks.
	if ( request.radianceHalf )
		probes.format = device::Format::kRGBA16Float;
	// Only the mips from the base on are kept: the data is stored mip-major.
	std::uint64_t skipped = 0;
	for ( std::uint32_t mip = 0; mip < probes.baseMip; ++mip )
		skipped +=
		    std::uint64_t( request.probeCount ) * 6 *
		    device::RegionBytes( probes.format, request.faceSize >> mip, request.faceSize >> mip );
	if ( skipped > request.radianceBytes )
		return false;
	const std::byte *data = static_cast<const std::byte *>( request.data );
	probes.radiance.assign( data + skipped, data + request.radianceBytes );
	// The core does not relight probes, so the relight cubes are not kept.
	reflection = std::move( probes );
	return true;
}

void CoreWorld::StageCapture::Release()
{
	lightmap = pass::world::LightmapPages();
	indirect = pass::world::LightmapPages();
	lmap.clear();
	sunMask = false;
	probes.reset();
	change.clear();
	table.reset();
	reflection.reset();
}

} // namespace render::composition
