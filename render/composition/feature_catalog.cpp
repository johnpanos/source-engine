//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The features this product links, by name (RFC 0016 A.8: adding a
//			feature pass adds one line here and nothing else outside its own
//			module).
//
//=============================================================================//

#include "render/legacy/core_backend.h"
#include "render/pass/present/feature.h"
#include "render/pass/skinning/feature.h"

#include <memory>
#include <string_view>

std::unique_ptr<render::frame::IRenderFeature> RenderCore_CreateFeature(
    std::string_view name, render::legacy::ILegacyFrontend &frontend )
{
	if ( name == "legacy-stream" )
		return frontend.CreateStreamFeature();
	if ( name == "present" )
		return render::pass::present::CreatePresentFeature( {} );
	if ( name == render::pass::skinning::kSkinningFeature )
		return render::pass::skinning::CreateSkinningFeature();
	if ( name == render::pass::skinning::kCpuSkinningFeature )
		return render::pass::skinning::CreateCpuSkinningFeature();
	return nullptr;
}
