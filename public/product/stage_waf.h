//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.stage.waf (RFC 0027 L0): the Waf engine build as an
//			ordinary IProductStage named `waf-engine`, role `engine`.
//
//			Waf stays the authority for code: the stage passes the profile's
//			configure options and reads Waf's status and installed outputs. It
//			configures only when the configure digest (arguments, toolchain
//			identity) changed or a configuration input is newer than the tree,
//			never compiles itself, and keeps the Waf lock inside the tree
//			(out/<profile>/<flavor>/build/, installing to .../install/), so
//			trees never share a lock.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_STAGE_WAF_H
#define PUBLIC_PRODUCT_STAGE_WAF_H

#include "product/contracts.h"

#include <memory>

namespace product
{

inline constexpr std::string_view kWafEngineStage = "waf-engine";
inline constexpr std::string_view kEngineInstallArtifact = "engine-install";

// `python` is the interpreter that runs ./waf (an explicit path or a name on
// PATH). Processes come from the stage inputs at run time.
std::unique_ptr<IProductStage> CreateWafEngineStage( std::string python = "python3" );

} // namespace product

#endif // PUBLIC_PRODUCT_STAGE_WAF_H
