//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's pixel visibility counts (RFC 0016
//			render.pass.visibility); see core_visibility.h.
//
//=============================================================================//

#include "core_visibility.h"

#include <cstdio>
#include <cstring>
#include <optional>

namespace render::composition
{

static_assert(
    ( pass::visibility::kVisibilityTag & legacy::kCorePassForwarded ) != 0 &&
        ( ( pass::visibility::kVisibilityTag | pass::visibility::kVisibilitySerialMask ) &
            ( legacy::kCorePassLegacyOff | legacy::kCorePassFrameEnd ) ) == 0,
    "visibility tags are forwarded tags, clear of the frontend's slot bits" );

unsigned CoreVisibility::Queue( const float points[5][4], const float viewport[6] )
{
	legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots();
	if ( !slots )
		return 0;
	pass::visibility::Query query;
	std::memcpy( query.points, points, sizeof( query.points ) );
	query.viewport = {
	    viewport[0], viewport[1], viewport[2], viewport[3], viewport[4], viewport[5] };
	const std::uint32_t tag = m_Counter.Queue( query );
	if ( tag )
		slots->MarkSlot( tag );
	return tag;
}

int CoreVisibility::Result( unsigned id, long long *visible, long long *possible )
{
	const std::optional<pass::visibility::Counts> counts = m_Counter.Result( id );
	if ( !counts )
		return 0;
	if ( counts->visible < 0 || counts->possible < 0 )
		return -1;
	if ( visible )
		*visible = counts->visible;
	if ( possible )
		*possible = counts->possible;
	return 1;
}

void CoreVisibility::GetStats( RenderCoreVisibilityStats *out ) const
{
	if ( !out )
		return;
	const pass::visibility::VisibilityStats stats = m_Counter.Stats();
	*out = RenderCoreVisibilityStats{};
	out->queued = stats.queued;
	out->refused = stats.refused;
	out->recorded = stats.recorded;
	out->resolved = stats.resolved;
	out->failed = stats.failed;
	std::snprintf( out->lastFailure, sizeof( out->lastFailure ), "%s", stats.lastFailure.c_str() );
}

void CoreVisibility::RecordSlot(
    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target )
{
	pass::visibility::VisibilityTarget visibility;
	visibility.device = target.device;
	visibility.color = target.color;
	visibility.colorFormat = target.colorFormat;
	visibility.depth = target.depth;
	visibility.depthFormat = target.depthFormat;
	visibility.width = target.width;
	visibility.height = target.height;
	visibility.samples = target.samples;
	visibility.submitted = target.submitted;
	visibility.frame = target.frame;
	m_Device = target.device;
	m_Counter.Record( tag, encoder, visibility );
}

void CoreVisibility::FrameSubmitted( device::CompletionToken token, bool submitted )
{
	if ( submitted && m_Device )
		m_Counter.FrameSubmitted( *m_Device, token );
}

void CoreVisibility::ReleaseDevice( device::IRenderDevice2 &device )
{
	m_Counter.ReleaseDevice( device );
	if ( m_Device == &device )
		m_Device = nullptr;
}

} // namespace render::composition
