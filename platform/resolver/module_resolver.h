//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The portable module resolver (platform.module-resolver.v1, R11).
//			It is the same on every platform; what differs is the injected file
//			probe and, optionally, the verifier.
//
//=============================================================================//

#ifndef PLATFORM_RESOLVER_MODULE_RESOLVER_H
#define PLATFORM_RESOLVER_MODULE_RESOLVER_H

#include "platform/contracts/module_resolver.h"

#include <memory>

namespace platform
{

// `probe` and `verifier` (may be null: no policy) are borrowed and must outlive
// the resolver.
[[nodiscard]] std::unique_ptr<IModuleResolver> CreateModuleResolver(
    const IFileProbe &probe, IModuleVerifier *verifier );

// The file name a module name becomes under `policy`'s extension rule
// ("engine" -> "engine.so"); exposed for the legacy bridge's diagnostics.
std::string ModuleFileName( std::string_view moduleName, const ModuleSearchPolicy &policy );

} // namespace platform

#endif // PLATFORM_RESOLVER_MODULE_RESOLVER_H
