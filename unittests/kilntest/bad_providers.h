//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deliberately bad providers of each product contract (RFC 0027
//			"Contracts and their obligations"), and the fake recipe builder.
//			Each bad provider breaks exactly the obligation its name says; the
//			shared suite must reject it by that clause.
//
//=============================================================================//

#ifndef UNITTESTS_KILNTEST_BAD_PROVIDERS_H
#define UNITTESTS_KILNTEST_BAD_PROVIDERS_H

#include "fixture_platform/fixture_platform.h"
#include "product/contracts.h"

#include <memory>

namespace bad
{

enum class ToolchainFault
{
	kIdentityWithoutSdk,
	kUnpinnedDownload,
	kWritesSourceTree,
	kMissingCompiler,
};
std::unique_ptr<product::ITargetToolchain> Toolchain(
    ToolchainFault fault, platform::IToolProcessProvider &processes );

// The conforming fake recipe builder and its bad variants.
enum class RecipeFault
{
	kNone,
	kIgnoresToolchain,
	kPartialPublish,
};
std::unique_ptr<product::IRecipeBuilder> RecipeBuilder( RecipeFault fault );

enum class StageFault
{
	kReadsUndeclared,
	kProducesUndeclared,
	kPartialPublish,
	kIgnoresCancel,
};
std::unique_ptr<product::IProductStage> Stage( StageFault fault );

enum class PackagerFault
{
	kUndeclaredFile,
	kOmitsModule,
	kPartialPublish,
	kEmbedsCredential,
	kUnstableManifest,
};
std::unique_ptr<product::IPackager> Packager( PackagerFault fault );

enum class TransportFault
{
	kSuccessAfterPartial,
	kDeleteOutsideRoot,
	kCredentialInLog,
	kClaimedNoEffect,
	kIgnoresCancel,
	kResendsUnchanged,
};
std::unique_ptr<product::IDeployTransport> Transport(
    TransportFault fault, fixture::FakeDevice &device, platform::IToolProcessProvider &processes );

enum class RunFault
{
	kReturnsBeforeExit,
	kIgnoresCancel,
	kDropsLog,               // starts the program with inherited output, not the launch's log
	kLaunchOverridesDisplay, // a launch variable beats the display session's
	kSilentStart,            // never reports the programs it starts
};
std::unique_ptr<product::IRunProvider> RunProvider( RunFault fault );

enum class DisplayFault
{
	kLeaksUserDisplay,
	kMutatesCaller,
	kStaysOpen,
	kIgnoresCancel,
};
std::unique_ptr<product::IDisplaySession> Display( DisplayFault fault );

} // namespace bad

#endif // UNITTESTS_KILNTEST_BAD_PROVIDERS_H
