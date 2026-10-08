//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.toolchain.emscripten (RFC 0027, RFC 0029): the
//			WebAssembly cross toolchain `emscripten`. It prepares the pinned
//			Emscripten SDK (quality/toolchain/emscripten.json) and the
//			pinned Dawn release's emdawnwebgpu package (quality/toolchain/
//			webgpu.json) through their one owner, tools/render/
//			webgpu_lane.py fetch, which verifies every archive by sha256. It
//			checks toolchain.version against em++ and gives Waf --emscripten
//			and the SDK directory.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_TOOLCHAIN_EMSCRIPTEN_H
#define PUBLIC_PRODUCT_TOOLCHAIN_EMSCRIPTEN_H

#include "product/contracts.h"

#include <memory>

namespace product
{

inline constexpr std::string_view kEmscriptenToolchain = "emscripten";

// The process provider is borrowed and must outlive the toolchain.
std::unique_ptr<ITargetToolchain> CreateEmscriptenToolchain(
    platform::IToolProcessProvider &processes );

} // namespace product

#endif // PUBLIC_PRODUCT_TOOLCHAIN_EMSCRIPTEN_H
