//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.frame.v1 stages (RFC 0016): the legacy frame's order, as
//			CViewRender draws it. The values match RenderStageMarker in
//			render/legacy/stage_markers.h (the client's C++11 view of them).
//
//			Order rules (StageTracker enforces them; a violation is counted,
//			never fatal):
//			- a frame starts with kFrameBegin and ends with kFrameEnd;
//			- kViewBegin opens a view anywhere inside the frame, including
//			  inside another view (portals, mirrors, monitors), and kViewEnd
//			  closes the innermost one;
//			- inside a view, kSkybox through kPostProcess never go backwards;
//			- outside every view, only kViewBegin, kHud and kFrameEnd occur.
//
//=============================================================================//

#ifndef RENDER_FRAME_STAGES_H
#define RENDER_FRAME_STAGES_H

#include <cstdint>
#include <vector>

namespace render::frame
{

enum class Stage : std::uint8_t
{
	kFrameBegin = 0,
	kViewBegin = 1,
	kSkybox = 2,
	kOpaque = 3,
	kTranslucent = 4,
	kViewModel = 5,
	kPostProcess = 6,
	kViewEnd = 7,
	kHud = 8,
	kFrameEnd = 9,
	kCount
};

const char *StageName( Stage stage );

class StageTracker
{
public:
	void Reset();
	// False when stage breaks the order rules; the tracker stays usable.
	bool Mark( Stage stage );

	std::uint32_t Depth() const { return static_cast<std::uint32_t>( m_Views.size() ); }
	std::uint32_t Views() const { return m_ViewCount; }
	bool Ended() const { return m_Ended; }
	bool Balanced() const { return m_Views.empty(); }

private:
	std::vector<Stage> m_Views; // the current stage of each open view
	Stage m_Frame = Stage::kFrameBegin;
	std::uint32_t m_ViewCount = 0;
	bool m_Begun = false;
	bool m_Ended = false;
};

} // namespace render::frame

#endif // RENDER_FRAME_STAGES_H
