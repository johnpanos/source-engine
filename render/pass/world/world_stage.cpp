//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): the world stage's published inputs.
//
//=============================================================================//

#include "world_pass_internal.h"

namespace render::pass::world
{

void WorldPass::SetStageLightmap( LightmapPages pages )
{
	State &s = *m_State;
	auto shared = std::make_shared<LightmapPages>( std::move( pages ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageLightmap = std::move( shared );
	s.stageLightmapPatches.clear();
	s.stageLightmapBaseRevision = ++s.stageLightmapRevision;
}

void WorldPass::SetStageProbeVolume(
    std::vector<std::byte> atlas, std::vector<std::byte> change, StageProbeVolume table )
{
	State &s = *m_State;
	auto sharedAtlas = std::make_shared<std::vector<std::byte>>( std::move( atlas ) );
	auto sharedTable = std::make_shared<const StageProbeVolume>( std::move( table ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageProbeAtlas = std::move( sharedAtlas );
	s.stageProbePatches.clear();
	s.stageProbeBaseRevision = ++s.stageProbeRevision;
	s.stageChangeBase = std::move( change );
	s.stagePatches.clear();
	s.stageTable = std::move( sharedTable );
	s.stageBaseRevision = ++s.stageChangeRevision;
}

void WorldPass::SetStageProbeRegions( std::vector<StageRegion> regions,
    std::vector<std::byte> atlasTexels, std::vector<std::byte> changeTexels,
    StageProbeVolume table )
{
	State &s = *m_State;
	auto sharedTable = std::make_shared<const StageProbeVolume>( std::move( table ) );
	std::lock_guard<std::mutex> guard( s.lock );
	if ( !atlasTexels.empty() )
		s.stageProbePatches.push_back(
		    { ++s.stageProbeRevision, regions, std::move( atlasTexels ) } );
	s.stageTable = std::move( sharedTable );
	if ( changeTexels.empty() )
		regions.clear();
	s.stagePatches.push_back(
	    { ++s.stageChangeRevision, std::move( regions ), std::move( changeTexels ) } );
}

void WorldPass::SetStageLightmapRegions(
    std::vector<StageRegion> regions, std::vector<std::byte> texels )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageLightmapPatches.push_back(
	    { ++s.stageLightmapRevision, std::move( regions ), std::move( texels ) } );
}

void WorldPass::SetStageChange( std::vector<std::byte> change, StageProbeVolume table )
{
	State &s = *m_State;
	auto sharedTable = std::make_shared<const StageProbeVolume>( std::move( table ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageChangeBase = std::move( change );
	s.stagePatches.clear();
	s.stageTable = std::move( sharedTable );
	s.stageBaseRevision = ++s.stageChangeRevision;
}

} // namespace render::pass::world
