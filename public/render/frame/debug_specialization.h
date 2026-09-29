//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A program's debug specialization under the frame's debug
//			controls (RFC 0014): the one mapping from render.frame's
//			DebugControls to render.shader-library's specialization constants.
//
//=============================================================================//

#ifndef RENDER_FRAME_DEBUG_SPECIALIZATION_H
#define RENDER_FRAME_DEBUG_SPECIALIZATION_H

#include "render/frame/debug_controls.h"
#include "render/shaderlib/debug_view.h"

#include <string_view>

namespace render::frame
{

// Under valid controls: the view, the BRDF mode and the overrides apply to
// the program the filter names (every program when it is empty); a program
// outside the filter draws flat grey under a pixel view. The terms and the
// furnace apply to every program.
shaderlib::DebugSpecialization DebugSpecializationFor(
    const DebugControls &controls, std::string_view program );

} // namespace render::frame

#endif // RENDER_FRAME_DEBUG_SPECIALIZATION_H
