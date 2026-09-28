//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.frame.v1 stage order (RFC 0016); see stages.h.
//
//=============================================================================//

#include "render/frame/stages.h"

namespace render::frame
{

const char *StageName( Stage stage )
{
	switch ( stage )
	{
	case Stage::kFrameBegin:
		return "frame-begin";
	case Stage::kViewBegin:
		return "view-begin";
	case Stage::kSkybox:
		return "skybox";
	case Stage::kOpaque:
		return "opaque";
	case Stage::kTranslucent:
		return "translucent";
	case Stage::kViewModel:
		return "view-model";
	case Stage::kPostProcess:
		return "post-process";
	case Stage::kViewEnd:
		return "view-end";
	case Stage::kHud:
		return "hud";
	case Stage::kFrameEnd:
		return "frame-end";
	case Stage::kCount:
		break;
	}
	return "unknown";
}

void StageTracker::Reset()
{
	m_Views.clear();
	m_Frame = Stage::kFrameBegin;
	m_ViewCount = 0;
	m_Begun = false;
	m_Ended = false;
}

bool StageTracker::Mark( Stage stage )
{
	if ( stage >= Stage::kCount )
		return false;
	if ( stage == Stage::kFrameBegin )
	{
		const bool fresh = !m_Begun;
		Reset();
		m_Begun = true;
		return fresh;
	}
	if ( !m_Begun || m_Ended )
		return false;
	switch ( stage )
	{
	case Stage::kViewBegin:
		m_Views.push_back( Stage::kViewBegin );
		++m_ViewCount;
		return true;
	case Stage::kViewEnd:
		if ( m_Views.empty() )
			return false;
		m_Views.pop_back();
		return true;
	case Stage::kHud:
		if ( !m_Views.empty() )
			return false;
		m_Frame = Stage::kHud;
		return true;
	case Stage::kFrameEnd:
		m_Ended = true;
		return m_Views.empty();
	default:
		break;
	}
	// kSkybox through kPostProcess: inside a view, never backwards.
	if ( m_Views.empty() || stage < m_Views.back() )
		return false;
	m_Views.back() = stage;
	return true;
}

} // namespace render::frame
