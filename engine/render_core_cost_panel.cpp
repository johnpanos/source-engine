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
ConVar cl_render_debug_cost_page( "cl_render_debug_cost_page", "0", FCVAR_CHEAT,
    "Page of the render-core cost overlay (0 is hottest)." );

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
		const int width = std::min( GetWide(), 1280 );
		if ( width < 320 || GetTall() < line * 6 )
			return;
		const int historyHeight = m_Report.resources.supported ? 64 + 3 * line : 0;
		const unsigned int perPage = std::max( 1, ( GetTall() - historyHeight ) / line - 6 );
		const unsigned int page =
		    std::min( unsigned( std::max( 0, cl_render_debug_cost_page.GetInt() ) ),
		        m_Report.count ? ( m_Report.count - 1 ) / perPage : 0 );
		const unsigned int first = page * perPage;
		const unsigned int shown = std::min( m_Report.count - first, perPage );
		const int height = ( 6 + std::max( 1u, shown ) ) * line + historyHeight;
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
		if ( m_Report.resources.supported )
		{
			const auto &r = m_Report.resources;
			V_snprintf( label, sizeof( label ),
			    "Resources: +%llu created (%llu buf / %llu tex / %llu groups / %llu other), "
			    "%llu released, -%llu destroyed; %.1f KiB buffers",
			    r.created, r.buffers, r.textures, r.groups, r.other, r.released, r.destroyed,
			    double( r.bufferBytes ) / 1024.0 );
			text( label );
			V_snprintf( label, sizeof( label ),
			    "Live %llu | awaiting retirement %llu | logical handles incl. imports; not heap or "
			    "VRAM",
			    r.live, r.pending );
			text( label );
			History( 8, y, width - 16, 64, label, sizeof( label ) );
			y += 64;
			text( label );
		}
		else
			text( "Resource allocation counters unavailable on this device/sample" );
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
				const RenderCoreCostRow &row = m_Report.rows[first + i];
				char allocations[80];
				if ( row.resourcesSupported )
					V_snprintf( allocations, sizeof( allocations ), "+%llu -%llu %.1f KiB",
					    row.created, row.destroyed, double( row.bufferBytes ) / 1024.0 );
				else
					V_strncpy( allocations, "resources n/a", sizeof( allocations ) );
				V_snprintf( label, sizeof( label ), "D%u %-42.42s CPU %7.3f GPU %7.3f | %s",
				    row.depth, row.name, row.cpuMilliseconds, row.gpuMilliseconds, allocations );
				text( label );
				Bar( 8, y - 8, half, row.cpuMilliseconds );
				Bar( 16 + half, y - 8, half, row.gpuMilliseconds );
			}
		}
		V_snprintf( label, sizeof( label ),
		    "Page %u (cl_render_debug_cost_page) | %u rows hidden | %u timestamps dropped | "
		    "refresh 4 Hz",
		    page, m_Report.omitted + m_Report.count - shown, m_Report.dropped );
		text( label );
		text( "Core labels only: excludes game CPU, legacy passes, present and this overlay" );
	}

private:
	void History( int x, int y, int width, int height, char *label, std::size_t labelSize )
	{
		auto *surface = vgui::surface();
		const int half = ( width - 16 ) / 2;
		unsigned long long churnPeak = 1, livePeak = 1;
		for ( unsigned int i = 0; i < m_Report.historyCount; ++i )
		{
			const auto &sample = m_Report.history[i];
			churnPeak = std::max( { churnPeak, sample.created, sample.destroyed } );
			livePeak = std::max( { livePeak, sample.live, sample.pending } );
		}
		surface->DrawSetColor( 25, 30, 38, 255 );
		surface->DrawFilledRect( x, y, x + width, y + height );
		const unsigned int count = m_Report.historyCount;
		for ( unsigned int i = 0; i < count; ++i )
		{
			const auto &sample = m_Report.history[i];
			if ( !sample.supported )
				continue;
			const int left = int( i * half / count );
			const int right = int( ( i + 1 ) * half / count );
			const int middle = ( left + right ) / 2;
			auto bar = [&]( int a, int b, unsigned long long value, unsigned long long peak,
			               int red, int green, int blue )
			{
				surface->DrawSetColor( red, green, blue, 255 );
				const int h = int( double( value ) / double( peak ) * ( height - 2 ) );
				surface->DrawFilledRect( a, y + height - h, std::max( a + 1, b ), y + height );
			};
			bar( x + left, x + middle, sample.created, churnPeak, 40, 210, 100 );
			bar( x + middle, x + right, sample.destroyed, churnPeak, 255, 160, 40 );
			bar(
			    x + half + 16 + left, x + half + 16 + middle, sample.live, livePeak, 70, 180, 255 );
			bar( x + half + 16 + middle, x + half + 16 + right, sample.pending, livePeak, 235, 90,
			    210 );
		}
		V_snprintf( label, labelSize,
		    "%u recorded frames: create green / destroy orange (0-%llu) | live blue / pending pink "
		    "(0-%llu)",
		    count, churnPeak, livePeak );
	}

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
