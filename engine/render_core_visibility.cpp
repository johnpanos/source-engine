//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The client's pixel visibility on the render core (RFC 0016
// render.pass.visibility, K8 sprites cohort): VEngineVisibilityCount001
// (public/engine/ivisibilitycount.h) over the core's occlusion counts.
//
// Main thread only.
//
//=============================================================================//

#include "render_pch.h"
#include "engine/ivisibilitycount.h"
#include "render_core_host.h"
#include "render/composition/render_core_visibility.h"
#include "tier1/convar.h"
#include "tier1/interface.h"

#include <algorithm>
#include <climits>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar r_core_visibility( "r_core_visibility", "1", 0,
    "RFC 0016 render.pass.visibility: the client's pixel visibility proxies are counted on the "
    "render core; 0 counts them with the material system's occlusion queries" );

namespace
{

class CEngineVisibilityCount final : public IEngineVisibilityCount
{
public:
	bool CoreCounts() override
	{
		return RenderCoreHost_Visibility() && r_core_visibility.GetBool();
	}

	unsigned Queue( const float points[5][4], const float viewport[6] ) override
	{
		IRenderCoreVisibility *pVisibility = RenderCoreHost_Visibility();
		return pVisibility ? pVisibility->Queue( points, viewport ) : 0u;
	}

	int Result( unsigned id, int *pVisible, int *pPossible ) override
	{
		IRenderCoreVisibility *pVisibility = RenderCoreHost_Visibility();
		if ( !pVisibility )
			return -1;
		long long visible = 0, possible = 0;
		const int status = pVisibility->Result( id, &visible, &possible );
		if ( status == 1 )
		{
			if ( pVisible )
				*pVisible = int( std::min<long long>( visible, INT_MAX ) );
			if ( pPossible )
				*pPossible = int( std::min<long long>( possible, INT_MAX ) );
		}
		return status;
	}
};

CEngineVisibilityCount g_EngineVisibilityCount;

} // namespace

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CEngineVisibilityCount, IEngineVisibilityCount,
    ENGINE_VISIBILITY_COUNT_INTERFACE_VERSION, g_EngineVisibilityCount );

CON_COMMAND(
    r_core_visibility_stats, "RFC 0016 render.pass.visibility: the core's pixel visibility" )
{
	IRenderCoreVisibility *pVisibility = RenderCoreHost_Visibility();
	if ( !pVisibility )
	{
		Msg( "r_core_visibility_stats: no render core\n" );
		return;
	}
	RenderCoreVisibilityStats stats;
	pVisibility->GetStats( &stats );
	Msg(
	    "r_core_visibility_stats: queued %llu refused %llu recorded %llu resolved %llu failed %llu "
	    "last failure '%s'\n",
	    stats.queued, stats.refused, stats.recorded, stats.resolved, stats.failed,
	    stats.lastFailure );
}
