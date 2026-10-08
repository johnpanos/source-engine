//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.toolchain.n3ds (RFC 0027, RFC 0026): the Nintendo 3DS
//			cross toolchain `n3ds-devkitarm`. It prepares devkitARM and the
//			target's sysroot libraries under dependencies/n3ds-devkitarm/:
//			  - devkitPro, extracted once from the container image the
//			    profile pins by digest (dependencies.container_image);
//			  - the profile's pinned source archives cross-built with that
//			    toolchain (dependencies.archives; today SDL3), each published
//			    atomically under a key covering its pin and the toolchain.
//			It verifies toolchain.version against arm-none-eabi-gcc and gives
//			Waf the cross option and the pkg-config environment.
//
//			Until kiln.core runs recipe builders, the archives are this
//			provider's sysroot, as an SDK's libraries are an SDK provider's.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_TOOLCHAIN_N3DS_H
#define PUBLIC_PRODUCT_TOOLCHAIN_N3DS_H

#include "product/contracts.h"

#include <memory>

namespace product
{

inline constexpr std::string_view kN3dsToolchain = "n3ds-devkitarm";

// The process provider is borrowed and must outlive the toolchain.
std::unique_ptr<ITargetToolchain> CreateN3dsToolchain( platform::IToolProcessProvider &processes );

} // namespace product

#endif // PUBLIC_PRODUCT_TOOLCHAIN_N3DS_H
