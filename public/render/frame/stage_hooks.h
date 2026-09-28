//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.frame.v1 stage hooks (RFC 0016). Code that must act at a
//			stage of the legacy frame (the Portal renderers, at K8) registers a
//			hook with the renderer instead of reaching into the view code.
//
//=============================================================================//

#ifndef RENDER_FRAME_STAGE_HOOKS_H
#define RENDER_FRAME_STAGE_HOOKS_H

#include "render/frame/stages.h"

#include <cstdint>

namespace render::frame
{

class IRenderStageHooks
{
public:
	virtual ~IRenderStageHooks() = default;
	// Called on the marking thread, after the stage is recorded. depth is
	// the number of open views.
	virtual void OnStage( Stage stage, std::uint32_t depth ) = 0;
};

} // namespace render::frame

#endif // RENDER_FRAME_STAGE_HOOKS_H
