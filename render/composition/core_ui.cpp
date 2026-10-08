//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's screen UI (RFC 0016 K8 UI cohort); see
//			core_ui.h.
//
//=============================================================================//

#include "core_ui.h"

#include <cstdio>
#include <optional>
#include <utility>

namespace render::composition
{

bool CoreUi::Refuse( std::string why )
{
	std::lock_guard<std::mutex> guard( m_Lock );
	++m_Stats.refused;
	m_LastRefusal = std::move( why );
	return false;
}

bool CoreUi::ClaimsMaterial( const RenderCoreWorldMaterial &material, char *why, int whySize )
{
	const std::optional<std::string> refused = m_World.ClaimUiMaterial( material );
	if ( refused && why && whySize > 0 )
		std::snprintf( why, std::size_t( whySize ), "%s", refused->c_str() );
	return !refused;
}

bool CoreUi::DrawList(
    const ui_draw_list::ListView &list, const RenderCoreWorldMaterial *materials )
{
	legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots();
	if ( !slots )
		return Refuse( "the backend has no core-pass slots" );
	std::string why;
	const std::uint32_t tag = m_World.QueueUiList( list, materials, why );
	if ( !tag )
		return Refuse( why );
	slots->MarkSlot( tag );
	std::lock_guard<std::mutex> guard( m_Lock );
	++m_Stats.submitted;
	m_Stats.commands += list.commandCount;
	return true;
}

void CoreUi::GetStats( RenderCoreUiStats *out ) const
{
	if ( !out )
		return;
	std::lock_guard<std::mutex> guard( m_Lock );
	*out = m_Stats;
	std::snprintf( out->lastRefusal, sizeof( out->lastRefusal ), "%s", m_LastRefusal.c_str() );
}

} // namespace render::composition
