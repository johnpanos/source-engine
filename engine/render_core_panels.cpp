//========= Copyright Valve Corporation, All rights reserved. ============//
//
// In-world panels drawn by the render core (RFC 0016 render.pass.panels):
// VEngineWorldPanels001 (public/engine/iworldpanels.h). The core takes a
// panel in the views it draws the world in (r_core_world, the outermost view
// into the back buffer); other views (portal, monitor, reflection) draw it
// through the legacy 2D path until they are the core's (RFC 0016 K8), as the
// world does. A panel the core took and failed to draw is fatal under
// r_core_world_strict, as a world view is: legacy never draws it instead.
//
// Main thread only.
//
//=============================================================================//

#include "render_pch.h"
#include "engine/iworldpanels.h"
#include "gl_rmain.h"
#include "render_core_host.h"
#include "render_core_world.h"
#include "mathlib/vmatrix.h"
#include "tier1/convar.h"
#include "tier1/interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int host_framecount;

static ConVar r_core_panels( "r_core_panels", "1", 0,
    "RFC 0016 render.pass.panels: in-world VGUI screens draw as "
    "emissive surfaces on the render core in the views it draws (r_core_world); 0 draws them "
    "through the legacy 2D path" );

namespace
{

unsigned long long s_FailuresSeen = 0;
char s_LastRefusal[256] = {};

// The core's failures since the last check, under r_core_world_strict (one
// policy for everything the core claims).
void CheckFailures( IRenderCorePanels *pPanels )
{
	const unsigned long long failures = pPanels->Failures();
	if ( failures == s_FailuresSeen )
		return;
	RenderCorePanelStats stats;
	pPanels->GetStats( &stats );
	const unsigned long long newFailures = failures - s_FailuresSeen;
	s_FailuresSeen = failures;
	static ConVarRef r_core_world_strict( "r_core_world_strict" );
	if ( !r_core_world_strict.IsValid() || r_core_world_strict.GetBool() )
	{
		Sys_Error( "r_core_panels: the render core failed %llu claimed panel view(s): %s "
		           "(r_core_world_strict 0 reports failures instead; legacy never draws what the "
		           "core claimed)\n",
		    newFailures, stats.lastFailure );
	}
	Warning( "r_core_panels: the render core failed %llu claimed panel view(s), panels left "
	         "undrawn: %s\n",
	    newFailures, stats.lastFailure );
}

class CEngineWorldPanels final : public IEngineWorldPanels
{
public:
	bool CoreDrawsPanels() override
	{
		IRenderCorePanels *pPanels = RenderCoreHost_Panels();
		if ( !pPanels || !r_core_panels.GetBool() )
			return false;
		CheckFailures( pPanels );
		static ConVarRef r_core_world( "r_core_world" );
		if ( !r_core_world.IsValid() || r_core_world.GetInt() != 1 ||
		     RenderCoreWorld_ViewDepth() != 1 )
			return false;
		VMatrix view, projection;
		int viewport[4];
		return R_CurrentSceneView( view, projection, viewport ) && viewport[2] > 0 &&
		       viewport[3] > 0;
	}

	bool DrawPanel( const RenderCorePanel &panel ) override
	{
		IRenderCorePanels *pPanels = RenderCoreHost_Panels();
		float toClip[16], viewport[6];
		if ( !pPanels || !CurrentView( toClip, viewport ) )
			return false;
		// The world's views name host_framecount + 1 (the frame being built).
		if ( pPanels->DrawPanel(
		         panel, toClip, viewport, static_cast<unsigned long long>( host_framecount ) + 1 ) )
			return true;
		RenderCorePanelStats stats;
		pPanels->GetStats( &stats );
		if ( V_strcmp( stats.lastRefusal, s_LastRefusal ) )
		{
			V_strncpy( s_LastRefusal, stats.lastRefusal, sizeof( s_LastRefusal ) );
			DevMsg(
			    "r_core_panels: the core did not take panel %llu (%s); the legacy path draws it\n",
			    panel.id, stats.lastRefusal );
		}
		return false;
	}

	void RemovePanel( unsigned long long id ) override
	{
		if ( IRenderCorePanels *pPanels = RenderCoreHost_Panels() )
			pPanels->RemovePanel( id );
	}

private:
	// The view as pushed (the legacy context holds the same matrices and
	// viewport): world to clip, row-major, and x, y, width, height, depth range.
	static bool CurrentView( float toClip[16], float viewport[6] )
	{
		VMatrix view, projection;
		int rect[4];
		if ( !R_CurrentSceneView( view, projection, rect ) )
			return false;
		const VMatrix worldToClip = projection * view;
		for ( int r = 0; r < 4; ++r )
			for ( int c = 0; c < 4; ++c )
				toClip[r * 4 + c] = worldToClip.m[r][c];
		viewport[0] = float( rect[0] );
		viewport[1] = float( rect[1] );
		viewport[2] = float( rect[2] );
		viewport[3] = float( rect[3] );
		viewport[4] = 0.0f;
		viewport[5] = 1.0f;
		return rect[2] > 0 && rect[3] > 0;
	}
};

CEngineWorldPanels g_EngineWorldPanels;

} // namespace

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CEngineWorldPanels, IEngineWorldPanels,
    ENGINE_WORLD_PANELS_INTERFACE_VERSION, g_EngineWorldPanels );

CON_COMMAND( r_core_panels_stats, "RFC 0016 render.pass.panels: the core's in-world panels" )
{
	IRenderCorePanels *pPanels = RenderCoreHost_Panels();
	if ( !pPanels )
	{
		Msg( "r_core_panels_stats: no render core\n" );
		return;
	}
	RenderCorePanelStats stats;
	pPanels->GetStats( &stats );
	Msg(
	    "r_core_panels_stats: lists %llu refused %llu rasterized %llu views queued %llu drawn %llu "
	    "failed %llu panels drawn %llu image bytes %llu last image %u x %u (%.3f texels per unit) "
	    "last failure '%s' last refusal '%s'\n",
	    stats.submitted, stats.refused, stats.rasterized, stats.viewsQueued, stats.viewsDrawn,
	    stats.viewsFailed, stats.panelsDrawn, stats.textureBytes, stats.lastResolution.width,
	    stats.lastResolution.height, stats.lastResolution.texelsPerUnit, stats.lastFailure,
	    stats.lastRefusal );
}
