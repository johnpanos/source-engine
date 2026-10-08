//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.toolchain.msvc-wine (RFC 0027): `windows-msvc-wine`,
//			the Windows target toolchain on a Linux host. MSVC runs under
//			Wine from the install tools/windows/msvc_wine.py provisions in
//			<dependencyRoot>/windows-msvc-wine/<toolset>-<sdk>/ from the
//			profile's toolchain.msvc_wine pins. Prepare() verifies that
//			install (its stamp equals the pins, `cl` reports the pinned
//			version) and gives Waf --msvc-wine; it downloads nothing, and
//			fails naming the provisioning command when the install is
//			missing or stale.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_TOOLCHAIN_MSVC_WINE_H
#define PUBLIC_PRODUCT_TOOLCHAIN_MSVC_WINE_H

#include "product/contracts.h"

#include <memory>
#include <string_view>

namespace product
{

inline constexpr std::string_view kMsvcWineToolchain = "windows-msvc-wine";

// The process provider is borrowed and must outlive the toolchain.
std::unique_ptr<ITargetToolchain> CreateMsvcWineToolchain(
    platform::IToolProcessProvider &processes );

} // namespace product

#endif // PUBLIC_PRODUCT_TOOLCHAIN_MSVC_WINE_H
