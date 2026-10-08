//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.toolchain.linux (RFC 0027): the host Linux toolchains
//			`linux-gcc` and `linux-clang`, the first ITargetToolchain
//			providers. They verify the profile's toolchain pins against the
//			host compiler, report its identity, and give Waf CC/CXX.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_TOOLCHAIN_LINUX_H
#define PUBLIC_PRODUCT_TOOLCHAIN_LINUX_H

#include "product/contracts.h"

#include <memory>

namespace product
{

// The process provider is borrowed and must outlive the toolchain.
std::unique_ptr<ITargetToolchain> CreateLinuxGccToolchain(
    platform::IToolProcessProvider &processes );
std::unique_ptr<ITargetToolchain> CreateLinuxClangToolchain(
    platform::IToolProcessProvider &processes );

} // namespace product

#endif // PUBLIC_PRODUCT_TOOLCHAIN_LINUX_H
