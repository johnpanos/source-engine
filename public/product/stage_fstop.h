//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.stage.fstop (RFC 0027 L1): the F-Stop content stage
//			`fstop-content`, role `content`. It assembles the `fstop` game
//			directory that the retired tools/quality/stage_fstop_runtime.py staged: the
//			gameinfo derived from Portal's, Valve's F-Stop-era content from
//			Steam2 depot 852 version 0 mirrored with lower-case names, the
//			merged sound-script, particle, HUD-layout and localization files,
//			the authored weapon scripts and the Portal 2 shop-door assets.
//
//			Locators it reads (content.locators): `portal-base` (a Portal
//			runtime), `fstop-depot` (the extracted depot: portal2/ and
//			portal2_tempcontent/), and optionally `portal2-runtime` (a staged
//			Portal 2 runtime for the shop-door models).
//
//			It produces the artifact `fstop-content`: fstop/... files and the
//			fstop_valve*/ link mirrors, which the linux-dir packager overlays.
//			It always computes the result of a fresh staging; the Python
//			stager appended its F-Stop lines again on every re-staging.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_STAGE_FSTOP_H
#define PUBLIC_PRODUCT_STAGE_FSTOP_H

#include "product/contracts.h"

#include <memory>

namespace product
{

inline constexpr std::string_view kFstopContentStage = "fstop-content";

std::unique_ptr<IProductStage> CreateFstopContentStage();

} // namespace product

#endif // PUBLIC_PRODUCT_STAGE_FSTOP_H
