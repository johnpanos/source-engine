//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shared suites of the product contracts (RFC 0027 L0): one
//			suite per contract, run against every claiming provider, fakes
//			included. A suite returns the clauses a provider violated; a
//			conforming provider violates none, and each deliberately bad
//			provider must violate the clause it was written to break.
//
//=============================================================================//

#ifndef UNITTESTS_KILNTEST_SUITES_H
#define UNITTESTS_KILNTEST_SUITES_H

#include "fixture_platform/fixture_platform.h"
#include "product/contracts.h"

#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace suites
{

// The violated clauses ("T5 writes outside its dependency directory", ...).
struct Verdict
{
	std::vector<std::string> violations;
	bool Passed() const { return violations.empty(); }
	bool Violates( const std::string &clause ) const;
	void Fail( std::string clause, std::string detail = {} );
};

// Records every process a provider starts (to catch unpinned downloads).
class RecordingProcesses final : public platform::IToolProcessProvider
{
public:
	explicit RecordingProcesses( platform::IToolProcessProvider &inner ) : m_Inner( inner ) {}
	platform::ToolProcessResult Run( const platform::ToolProcessRequest &request ) override;
	int LiveProcessCount() const noexcept override { return m_Inner.LiveProcessCount(); }
	std::vector<std::vector<std::string>> started;

private:
	platform::IToolProcessProvider &m_Inner;
};

struct ToolchainCase
{
	const product::ResolvedProfile *valid = nullptr;    // pins the host compiler
	const product::ResolvedProfile *wrongPin = nullptr; // pins a version the host lacks
	std::filesystem::path sourceRoot;
	std::filesystem::path dependencyRoot;
};
Verdict ToolchainSuite( product::ITargetToolchain &toolchain, RecordingProcesses &processes,
    const ToolchainCase &input );

struct RecipeCase
{
	std::filesystem::path prefixRoot;
	product::ToolchainEnvironment toolchainA;
	product::ToolchainEnvironment toolchainB; // a different identity
};
Verdict RecipeSuite( product::IRecipeBuilder &builder, const RecipeCase &input );

struct StageCase
{
	const product::ResolvedProfile *profile = nullptr;
	std::filesystem::path sourceRoot;
	std::filesystem::path treeRoot;
	std::map<std::string, product::Artifact> available; // the declared inputs
	const product::ToolchainEnvironment *toolchain = nullptr;
	platform::IToolProcessProvider *processes = nullptr;
	bool runSuccessPath = true; // false: descriptor and cancellation clauses only
};
Verdict StageSuite( product::IProductStage &stage, const StageCase &input );

struct PackagerCase
{
	const product::ResolvedProfile *profile = nullptr;
	std::filesystem::path scratch;
};
Verdict PackagerSuite( product::IPackager &packager, const PackagerCase &input );

struct TransportCase
{
	fixture::FakeDevice *device = nullptr; // what the transport acts on
	std::filesystem::path scratch;
	std::filesystem::path package; // an installed package for launch clauses
};
Verdict TransportSuite( product::IDeployTransport &transport, const TransportCase &input );

Verdict DisplaySuite( product::IDisplaySession &session );

} // namespace suites

#endif // UNITTESTS_KILNTEST_SUITES_H
