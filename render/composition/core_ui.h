//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The composition's screen UI (RFC 0016 K8 UI cohort): IRenderCoreUi
//			over the world's dynamic draws (CoreWorld::QueueUiList), each list
//			at its slot of the frame's stream.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_CORE_UI_H
#define RENDER_COMPOSITION_CORE_UI_H

#include "core_world.h"
#include "render/composition/render_core_ui.h"
#include "render/legacy/core_backend.h"

#include <mutex>
#include <string>

namespace render::composition
{

class CoreUi final : public IRenderCoreUi
{
public:
	CoreUi( legacy::ILegacyFrontend &frontend, CoreWorld &world )
	    : m_Frontend( frontend ), m_World( world )
	{
	}

	bool ClaimsMaterial( const RenderCoreWorldMaterial &material, char *why, int whySize ) override;
	bool DrawList(
	    const ui_draw_list::ListView &list, const RenderCoreWorldMaterial *materials ) override;
	void GetStats( RenderCoreUiStats *out ) const override;

private:
	bool Refuse( std::string why );

	legacy::ILegacyFrontend &m_Frontend;
	CoreWorld &m_World;
	mutable std::mutex m_Lock; // the stats
	RenderCoreUiStats m_Stats = {};
	std::string m_LastRefusal;
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_UI_H
