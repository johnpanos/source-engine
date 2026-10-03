//========= Copyright Valve Corporation, All rights reserved. ============//
// Render-core cost diagnostics. VGUI owns presentation; the core owns samples.
#include "client_pch.h"
#include "render_core_host.h"
#include "render/composition/render_core_world.h"
#include "vgui_basepanel.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "tier2/tier2.h"

#include <algorithm>
#include <cmath>

#include "tier0/memdbgon.h"

namespace
{
class RenderCoreCostPanel final : public CBasePanel
{
public:
	explicit RenderCoreCostPanel( vgui::Panel *parent ) : CBasePanel( parent, "RenderCoreCosts" )
	{
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
		SetPaintBackgroundEnabled( false );
		SetPaintBorderEnabled( false );
		SetVisible( false );
	}

	bool ShouldDraw() override { return RenderCoreHost_ReadCosts( nullptr ); }

	void ApplySchemeSettings( vgui::IScheme *scheme ) override
	{
		CBasePanel::ApplySchemeSettings( scheme );
		m_Font = scheme->GetFont( "DefaultFixedOutline", IsProportional() );
	}

	void OnTick() override
	{
		CBasePanel::OnTick();
		if ( !IsVisible() )
		{
			m_Report = {};
			m_NextSample = 0;
			return;
		}
		const double now = Plat_FloatTime();
		if ( now >= m_NextSample )
		{
			(void)RenderCoreHost_ReadCosts( &m_Report );
			m_NextSample = now + 0.25;
		}
		int width, height;
		GetParent()->GetSize( width, height );
		SetBounds( 12, 12, std::max( 0, width - 24 ), std::max( 0, height - 24 ) );
	}

	void Paint() override
	{
		if ( !ShouldDraw() || !m_Font )
			return;
		auto *surface = vgui::surface();
		const int line = surface->GetFontTall( m_Font ) + 10;
		const int width = std::min( GetWide(), 1000 );
		if ( width < 320 || GetTall() < line * 6 )
			return;
		const unsigned int shown = std::min(
		    m_Report.count, static_cast<unsigned int>( std::max( 0, GetTall() / line - 5 ) ) );
		const int height = ( 5 + std::max( 1u, shown ) ) * line;
		surface->DrawSetColor( 8, 12, 20, 235 );
		surface->DrawFilledRect( 0, 0, width, std::min( height, GetTall() ) );
		int y = 2;
		auto text = [&]( const char *value )
		{
			g_pMatSystemSurface->DrawColoredText( m_Font, 8, y, 235, 240, 250, 255, "%s", value );
			y += line;
		};
		char label[256];
		V_snprintf( label, sizeof( label ), "Render core costs | sample %llu | age %llu frames",
		    m_Report.frame,
		    m_Report.currentFrame >= m_Report.frame ? m_Report.currentFrame - m_Report.frame : 0 );
		text( label );
		text( "CPU recording / GPU pass ms: inclusive scopes; do not sum rows or columns" );
		text( "Bars: CPU left, GPU right. Blue -> green -> yellow -> red = 0 -> 16.67 ms" );
		if ( !m_Report.available )
			text( "Waiting for a core view (render-core path required)" );
		else if ( !m_Report.supported )
			text( "GPU timestamps unsupported on this device; timing unavailable" );
		else if ( m_Report.count == 0 )
			text( "Waiting for GPU completion; no completed sample yet" );
		else
		{
			const int half = ( width - 24 ) / 2;
			for ( unsigned int i = 0; i < shown; ++i )
			{
				const RenderCoreCostRow &row = m_Report.rows[i];
				V_snprintf( label, sizeof( label ), "D%u  %-48.48s CPU %8.3f  GPU %8.3f", row.depth,
				    row.name, row.cpuMilliseconds, row.gpuMilliseconds );
				text( label );
				Bar( 8, y - 8, half, row.cpuMilliseconds );
				Bar( 16 + half, y - 8, half, row.gpuMilliseconds );
			}
		}
		V_snprintf( label, sizeof( label ), "%u rows hidden | %u timestamps dropped | refresh 4 Hz",
		    m_Report.omitted + m_Report.count - shown, m_Report.dropped );
		text( label );
		text( "Core labels only: excludes game CPU, legacy passes, present and this overlay" );
	}

private:
	static void Bar( int x, int y, int width, double ms )
	{
		auto *surface = vgui::surface();
		surface->DrawSetColor( 45, 48, 56, 255 );
		surface->DrawFilledRect( x, y, x + width, y + 5 );
		if ( !std::isfinite( ms ) || ms < 0 )
			return;
		const double fraction = std::clamp( ms / ( 1000.0 / 60.0 ), 0.0, 1.0 );
		const int colors[4][3] = {
		    { 40, 100, 255 }, { 30, 210, 100 }, { 255, 210, 35 }, { 255, 45, 30 } };
		const int segment = std::min( 2, int( fraction * 3 ) );
		const double blend = fraction * 3 - segment;
		int color[3];
		for ( int c = 0; c < 3; ++c )
			color[c] = int( colors[segment][c] * ( 1 - blend ) + colors[segment + 1][c] * blend );
		surface->DrawSetColor( color[0], color[1], color[2], 255 );
		surface->DrawFilledRect( x, y, x + int( width * fraction ), y + 5 );
	}

	vgui::HFont m_Font = 0;
	RenderCoreCostReport m_Report;
	double m_NextSample = 0;
};
} // namespace

vgui::Panel *RenderCoreCostPanel_Create( vgui::Panel *parent )
{
	return new RenderCoreCostPanel( parent );
}
