//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A frame-owned scene-color snapshot for later graph passes.
//
//=============================================================================//

#ifndef RENDER_GRAPH_SCENE_COLOR_H
#define RENDER_GRAPH_SCENE_COLOR_H

#include "render/graph/graph_builder.h"

#include <optional>

namespace render::graph
{

// Resolves a multisampled 2D color image when needed, then copies it into a
// sampled transient. The source must already contain scene color and support
// kColorAttachment for an MSAA resolve or kCopySource for a single-sample copy.
// A later pass must read the returned image to keep the capture alive.
std::optional<ResourceRef> CaptureSceneColor( GraphBuilder &builder, ResourceRef source );

} // namespace render::graph

#endif // RENDER_GRAPH_SCENE_COLOR_H
