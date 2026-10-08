//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The screen UI drawn by the render core (RFC 0016 K8 UI cohort; RFC 0010's
// UI draw-list consumer): the material system surface records what VGUI
// paints to the screen (vgui/IScreenUiRecorder.h) and hands each segment
// here; the core draws its commands as dynamic draws of their materials,
// captured as the world's are and claimed by the core, at that point of the
// frame's stream. A material the core refuses, or a segment it does not take,
// the surface draws through the material system as before. A segment the core
// took and failed to draw is a failed world view (r_core_world_strict).
//
// Main thread only.
//
//=============================================================================//

#include "render_pch.h"
#include "render_core_host.h"
#include "render_core_ui.h"
#include "render_core_world.h"
#include "render/composition/render_core_ui.h"
#include "tier1/convar.h"
#include "tier3/tier3.h"
#include "vgui/IScreenUiRecorder.h"
#include "VGuiMatSurface/IMatSystemSurface.h"

#include <memory>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar r_core_ui( "r_core_ui", "1", 0,
    "RFC 0016 K8: the screen UI (VGUI) draws on the render core; 0 draws it through the "
    "material system" );

namespace
{

char s_LastRefusal[256] = {};

// The paint's UI materials: each captured once per top-level paint (proxies,
// texture frames and, across a level load's loading-screen paints, the
// textures themselves change between paints) with the core's claim of it. A
// key is its index plus one.
struct UiMaterial
{
	IMaterial *material = NULL;
	std::unique_ptr<CRenderCoreMaterialCapture> capture;
	bool claimed = false;
};

class CEngineScreenUi final : public IScreenUiConsumer
{
public:
	bool TakesScreenUi() override
	{
		if ( !RenderCoreHost_Ui() || !r_core_ui.GetBool() )
			return false;
		m_Materials.clear();
		return true;
	}

	unsigned ScreenUiMaterial( IMaterial *pMaterial, char *pszWhy, int nWhySize ) override
	{
		IRenderCoreUi *pUi = RenderCoreHost_Ui();
		if ( !pUi || !pMaterial )
		{
			V_strncpy( pszWhy, "no render core", nWhySize );
			return 0;
		}
		for ( size_t i = 0; i < m_Materials.size(); ++i )
		{
			if ( m_Materials[i].material != pMaterial )
				continue;
			if ( !m_Materials[i].claimed )
				V_strncpy( pszWhy, "the core refuses it", nWhySize );
			return m_Materials[i].claimed ? unsigned( i + 1 ) : 0u;
		}
		UiMaterial entry;
		entry.material = pMaterial;
		entry.capture = std::make_unique<CRenderCoreMaterialCapture>();
		entry.capture->Capture( pMaterial );
		entry.claimed = pUi->ClaimsMaterial( entry.capture->Desc(), pszWhy, nWhySize );
		m_Materials.push_back( std::move( entry ) );
		return m_Materials.back().claimed ? unsigned( m_Materials.size() ) : 0u;
	}

	bool DrawScreenUi( const ui_draw_list::ListView &list, const unsigned *materialKeys ) override
	{
		IRenderCoreUi *pUi = RenderCoreHost_Ui();
		if ( !pUi )
			return false;
		std::vector<RenderCoreWorldMaterial> materials;
		materials.reserve( list.materialCount );
		for ( unsigned i = 0; i < list.materialCount; ++i )
		{
			const unsigned key = materialKeys ? materialKeys[i] : 0;
			if ( key == 0 || key > m_Materials.size() || !m_Materials[key - 1].claimed )
				return false;
			materials.push_back( m_Materials[key - 1].capture->Desc() );
		}
		if ( pUi->DrawList( list, materials.data() ) )
			return true;
		RenderCoreUiStats stats;
		pUi->GetStats( &stats );
		if ( V_strcmp( stats.lastRefusal, s_LastRefusal ) )
		{
			V_strncpy( s_LastRefusal, stats.lastRefusal, sizeof( s_LastRefusal ) );
			DevMsg( "r_core_ui: the core did not take a UI segment (%s); the material system "
			        "draws it\n",
			    stats.lastRefusal );
		}
		return false;
	}

private:
	std::vector<UiMaterial> m_Materials;
};

CEngineScreenUi g_EngineScreenUi;
IScreenUiRecorder *g_pScreenUiRecorder = NULL;

} // namespace

void EngineScreenUi_Install()
{
	if ( !g_pMatSystemSurface )
		return;
	g_pScreenUiRecorder = static_cast<IScreenUiRecorder *>(
	    g_pMatSystemSurface->QueryInterface( VGUI_SCREEN_UI_RECORDER_INTERFACE_VERSION ) );
	if ( g_pScreenUiRecorder )
		g_pScreenUiRecorder->SetConsumer( &g_EngineScreenUi );
}

void EngineScreenUi_Remove()
{
	if ( g_pScreenUiRecorder )
		g_pScreenUiRecorder->SetConsumer( NULL );
	g_pScreenUiRecorder = NULL;
}

CON_COMMAND( r_core_ui_stats, "RFC 0016 K8: the screen UI on the render core" )
{
	if ( g_pScreenUiRecorder )
	{
		ScreenUiStats surface;
		g_pScreenUiRecorder->GetStats( &surface );
		Msg( "r_core_ui_stats: surface segments %llu declined %llu commands %llu material draws "
		     "%llu last reason '%s'\n",
		    surface.segments, surface.declined, surface.commands, surface.materialDraws,
		    surface.lastReason );
	}
	else
	{
		Msg( "r_core_ui_stats: the surface records no screen UI\n" );
	}
	IRenderCoreUi *pUi = RenderCoreHost_Ui();
	if ( !pUi )
	{
		Msg( "r_core_ui_stats: no render core\n" );
		return;
	}
	RenderCoreUiStats stats;
	pUi->GetStats( &stats );
	Msg( "r_core_ui_stats: core lists %llu refused %llu commands %llu last refusal '%s'\n",
	    stats.submitted, stats.refused, stats.commands, stats.lastRefusal );
}
