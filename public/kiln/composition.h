//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: kiln.composition (RFC 0027): the standard providers in one typed
//			ProviderCatalog, so any tool can embed exactly what the `kiln` CLI
//			uses, or compose its own subset and pass it to kiln::Session.
//			No global registry and no discovery: the provider list is here.
//
//=============================================================================//

#ifndef PUBLIC_KILN_COMPOSITION_H
#define PUBLIC_KILN_COMPOSITION_H

#include "kiln/api.h"

#include <memory>

namespace kiln
{

// Owns the process environment, the process provider, the executor and the
// catalog that borrows the process provider; members are destroyed catalog
// first.
struct DefaultComposition
{
	std::unique_ptr<platform::IProcessEnvironment> environment;
	std::unique_ptr<platform::IToolProcessProvider> processes;
	std::unique_ptr<platform::IProcessSpawner> spawner;
	std::unique_ptr<jobsystem::IGraphExecutor> executor;
	product::ProviderCatalog catalog;
	std::string hostTag; // "<os>-<architecture>" of this host, e.g. "linux-x86_64"

	DefaultComposition();
	DefaultComposition( DefaultComposition && ) noexcept;
	DefaultComposition &operator=( DefaultComposition && ) noexcept;
	~DefaultComposition();
};

// The standard catalog over a borrowed process provider: toolchains
// linux-gcc and linux-clang, the waf-engine stage.
[[nodiscard]] foundation::Expected<product::ProviderCatalog, Error> ComposeDefaultCatalog(
    platform::IToolProcessProvider &processes );

// `argc`/`argv` are the process's arguments for its environment snapshot; an
// embedding host (sepipe inside Python) that does not own them passes none.
[[nodiscard]] foundation::Expected<DefaultComposition, Error> ComposeDefault(
    int argc = 0, const char *const *argv = nullptr );

} // namespace kiln

#endif // PUBLIC_KILN_COMPOSITION_H
