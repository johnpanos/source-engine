//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's in-world panels (RFC 0016
//			render.pass.panels); see core_panels.h.
//
//=============================================================================//

#include "core_panels.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace render::composition
{

static_assert( ( pass::panels::kPanelTag & legacy::kCorePassForwarded ) != 0 &&
                   ( pass::panels::kPanelTag &
                       ( legacy::kCorePassLegacyOff | legacy::kCorePassFrameEnd ) ) == 0,
    "panel tags are forwarded tags, clear of the frontend's slot bits" );

namespace
{

// The backend's textures as the pass reads a list's (gamma values, as
// stored: the legacy 2D path samples them so).
class Textures final : public pass::panels::IPanelTextures
{
public:
	explicit Textures( legacy::ICoreTextures &textures ) : m_Textures( textures ) {}
	device::TextureId Import( int key ) override { return m_Textures.Import( key, false ); }
	device::SamplerDesc Sampler( int key ) override { return m_Textures.Sampler( key ); }

private:
	legacy::ICoreTextures &m_Textures;
};

} // namespace

bool CorePanels::DrawPanel( const RenderCorePanel &panel, const float worldToClip[16],
    const float viewport[6], unsigned long long hostFrame )
{
	auto refuse = [&]( std::string why )
	{
		std::lock_guard<std::mutex> guard( m_Lock );
		m_LastRefusal = std::move( why );
		return false;
	};
	legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots();
	if ( !slots )
		return refuse( "the backend has no core-pass slots" );
	if ( !m_Host || !m_Host->textureHandle )
		return refuse( "no render call queue host names textures" );
	if ( !( viewport[2] > 0.0f ) || !( viewport[3] > 0.0f ) )
		return refuse( "the view has no viewport" );

	// The frame's list, once: later calls of the frame draw it again.
	if ( !m_Pass.Submitted( hostFrame, panel.id ) )
	{
		pass::panels::Panel list;
		list.id = panel.id;
		list.placement = panel.placement;
		list.unitsWide = panel.unitsWide;
		list.unitsTall = panel.unitsTall;
		list.quads.assign( panel.quads, panel.quads + panel.quadCount );
		list.textures.reserve( panel.textureCount );
		for ( unsigned int i = 0; i < panel.textureCount; ++i )
		{
			const int handle = panel.textures[i] ? m_Host->textureHandle( panel.textures[i] ) : 0;
			if ( handle == 0 )
				return refuse( "texture " + std::to_string( i ) + " of panel " +
				               std::to_string( panel.id ) + " has not reached the renderer" );
			list.textures.push_back( handle );
		}
		list.resolution = panel.resolution;
		list.emissionScale = panel.emissionScale;
		list.transparent = panel.transparent;
		std::copy(
		    &panel.ambientCube[0][0], &panel.ambientCube[0][0] + 18, &list.ambientCube[0][0] );
		if ( !m_Pass.Submit( hostFrame, std::move( list ) ) )
			return refuse( m_Pass.Stats().lastFailure );
	}

	pass::panels::PanelView view;
	view.hostFrame = hostFrame;
	view.panels = { panel.id };
	// The engine's transform puts pixel centers on integer coordinates
	// (D3D9); the port's are half a pixel right and down of them (the same
	// correction as the world's views).
	std::memcpy( view.toClip, worldToClip, sizeof( view.toClip ) );
	for ( int c = 0; c < 4; ++c )
	{
		view.toClip[0 * 4 + c] += worldToClip[3 * 4 + c] / viewport[2];
		view.toClip[1 * 4 + c] -= worldToClip[3 * 4 + c] / viewport[3];
	}
	view.viewport = {
	    viewport[0], viewport[1], viewport[2], viewport[3], viewport[4], viewport[5] };
	view.debug = m_Renderer.AppliedDebug();
	const std::uint32_t tag = m_Pass.QueueView( std::move( view ) );
	if ( tag == 0 )
		return refuse( "the view queued no panel" );
	slots->MarkSlot( tag );
	return true;
}

void CorePanels::GetStats( RenderCorePanelStats *out ) const
{
	if ( !out )
		return;
	const pass::panels::PanelStats stats = m_Pass.Stats();
	*out = RenderCorePanelStats{};
	out->submitted = stats.submitted;
	out->refused = stats.refused;
	out->rasterized = stats.rasterized;
	out->viewsQueued = stats.viewsQueued;
	out->viewsDrawn = stats.viewsDrawn;
	out->viewsFailed = stats.viewsFailed;
	out->panelsDrawn = stats.panelsDrawn;
	out->textureBytes = stats.textureBytes;
	out->lastResolution = stats.lastResolution;
	std::snprintf( out->lastFailure, sizeof( out->lastFailure ), "%s", stats.lastFailure.c_str() );
	std::lock_guard<std::mutex> guard( m_Lock );
	std::snprintf( out->lastRefusal, sizeof( out->lastRefusal ), "%s", m_LastRefusal.c_str() );
}

void CorePanels::RecordSlot(
    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target )
{
	std::optional<Textures> textures;
	if ( target.textures )
		textures.emplace( *target.textures );
	pass::panels::PanelTarget panels;
	panels.device = target.device;
	// The sRGB view when the target has one; else the unorm view, and the
	// shader encodes (the output encoding frame term), as the world does.
	panels.terms.encodeOutput = !target.colorSrgb.IsValid();
	panels.color = panels.terms.encodeOutput ? target.color : target.colorSrgb;
	panels.colorFormat = panels.terms.encodeOutput ? target.colorFormat : target.colorSrgbFormat;
	panels.depth = target.depth;
	panels.depthFormat = target.depthFormat;
	panels.width = target.width;
	panels.height = target.height;
	panels.samples = target.samples;
	panels.textures = textures ? &*textures : nullptr;
	panels.submitted = target.submitted;
	panels.frame = target.frame;
	panels.terms.lightmapScale = target.lightmapScale;
	panels.terms.outputScale = target.outputScale;
	std::copy( target.eye, target.eye + 3, panels.terms.eye );
	panels.terms.envmapScale = target.envmapScale;
	panels.terms.specular = target.specular;
	panels.terms.ssbumpNormalized = target.ssbumpNormalized;
	panels.terms.fogType = target.fog.type;
	std::copy( target.fog.color, target.fog.color + 3, panels.terms.fogColor );
	std::copy( target.fog.params, target.fog.params + 4, panels.terms.fogParams );
	panels.terms.fogEyeZ = target.fog.eyeZ;
	m_Pass.Record( tag, encoder, panels );
}

} // namespace render::composition
