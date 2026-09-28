//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.present (RFC 0016): the frame's last pass, which
//			leaves the frame target ready for presentation. In this first
//			slice it clears the target when no earlier pass wrote it; the
//			present blit and gamma move here from CVulkanContext at K2-K3.
//
//=============================================================================//

#ifndef RENDER_PASS_PRESENT_FEATURE_H
#define RENDER_PASS_PRESENT_FEATURE_H

#include "render/device/encoder.h"
#include "render/frame/feature.h"

#include <memory>

namespace render::pass::present
{

struct PresentOptions
{
	device::ClearColor clear;
};

std::unique_ptr<frame::IRenderFeature> CreatePresentFeature( const PresentOptions &options );

} // namespace render::pass::present

#endif // RENDER_PASS_PRESENT_FEATURE_H
