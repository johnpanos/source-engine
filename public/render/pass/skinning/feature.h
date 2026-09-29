//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.skinning's frame features (RFC 0016 K6, K10):
//			"skinning" skins on the device (SkinningKernel; it requires compute
//			and storage buffers) and names "skinning-cpu", the CPU path
//			(SkinReference), as its fallback, which composition takes on a
//			device without them when the product profile declares the
//			substitution (FeatureRequirements::fallback).
//
//			Neither adds passes yet: the frame carries no skinned meshes until
//			K6's product skinning feeds them. Composition's choice between them
//			is what exists.
//
//=============================================================================//

#ifndef RENDER_PASS_SKINNING_FEATURE_H
#define RENDER_PASS_SKINNING_FEATURE_H

#include "render/frame/feature.h"

#include <memory>

namespace render::pass::skinning
{

inline constexpr const char *kSkinningFeature = "skinning";
inline constexpr const char *kCpuSkinningFeature = "skinning-cpu";

std::unique_ptr<frame::IRenderFeature> CreateSkinningFeature();
std::unique_ptr<frame::IRenderFeature> CreateCpuSkinningFeature();

} // namespace render::pass::skinning

#endif // RENDER_PASS_SKINNING_FEATURE_H
