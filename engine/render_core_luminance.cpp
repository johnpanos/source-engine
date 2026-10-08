//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The luminance counts behind the client's auto exposure on the render core
// (RFC 0016 render.pass.luminance, K8 post cohort): VEngineLuminanceCount001
// (public/engine/iluminancecount.h) over the core's counts, in place of the
// client's luminance_compare draw under an occlusion query.
//
// Main thread only.
//
//=============================================================================//

#include "render_pch.h"
#include "engine/iluminancecount.h"
#include "render_core_host.h"
#include "render/composition/render_core_luminance.h"
#include "tier1/convar.h"
#include "tier1/interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar r_core_luminance( "r_core_luminance", "1", 0,
    "RFC 0016 render.pass.luminance: the tone-mapping histogram's luminance counts run on the "
    "render core; 0 counts them with occlusion queries" );

namespace
{

class CEngineLuminanceCount final : public IEngineLuminanceCount
{
public:
	bool CoreCounts() override { return RenderCoreHost_Luminance() && r_core_luminance.GetBool(); }

	unsigned Queue( ITexture *pTexture, int x0, int y0, int x1, int y1, float minimum,
	    float maximum, float scale ) override
	{
		IRenderCoreLuminance *pLuminance = RenderCoreHost_Luminance();
		return pLuminance ? pLuminance->Queue( pTexture, x0, y0, x1, y1, minimum, maximum, scale )
		                  : 0u;
	}

	int Result( unsigned id ) override
	{
		IRenderCoreLuminance *pLuminance = RenderCoreHost_Luminance();
		return pLuminance ? pLuminance->Result( id ) : -2;
	}
};

CEngineLuminanceCount g_EngineLuminanceCount;

} // namespace

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CEngineLuminanceCount, IEngineLuminanceCount,
    ENGINE_LUMINANCE_COUNT_INTERFACE_VERSION, g_EngineLuminanceCount );

CON_COMMAND( r_core_luminance_stats, "RFC 0016 render.pass.luminance: the core's luminance counts" )
{
	IRenderCoreLuminance *pLuminance = RenderCoreHost_Luminance();
	if ( !pLuminance )
	{
		Msg( "r_core_luminance_stats: no render core\n" );
		return;
	}
	RenderCoreLuminanceStats stats;
	pLuminance->GetStats( &stats );
	Msg( "r_core_luminance_stats: queued %llu refused %llu recorded %llu resolved %llu failed %llu "
	     "last failure '%s'\n",
	    stats.queued, stats.refused, stats.recorded, stats.resolved, stats.failed,
	    stats.lastFailure );
}
