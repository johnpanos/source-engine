//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: kiln.api (RFC 0027): the product pipeline as a library.
//
//			A Session borrows a ProviderCatalog, a process provider, a graph
//			executor and a diagnostic sink, all owned by the caller and
//			outliving the session. Every `kiln` command is one call here, so
//			any tool can do in-process what the CLI does.
//
//			kiln.core knows verbs, stage roles, artifacts, the catalog and
//			evidence. It names no OS, SDK, package form, transport, pixel
//			format or asset kind, and never compares a profile's target to
//			choose behaviour: it asks the provider the profile names.
//
//			Threading: one request at a time per Session. Stages run on the
//			injected jobs.graph executor in artifact order; the deterministic
//			executor is the oracle.
//
//=============================================================================//

#ifndef PUBLIC_KILN_API_H
#define PUBLIC_KILN_API_H

#include "foundation/expected.h"
#include "foundation/json.h"
#include "product/contracts.h"
#include "product/profile.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace jobsystem
{
class IGraphExecutor;
}

namespace kiln
{

inline constexpr std::string_view kContractName = "kiln.api.v1";
inline constexpr std::string_view kJsonSchema = "kiln-output/v1";

struct Error
{
	std::string code; // "profile", "catalog", "toolchain", "stage", "capability", ...
	std::string detail;
	std::string Describe() const { return code + ": " + detail; }
};

// Explicit locations; the library reads no environment or current directory.
struct SessionConfig
{
	std::filesystem::path sourceRoot;         // the repository (read-only to kiln)
	std::filesystem::path profileRoot;        // quality/product_profiles
	std::filesystem::path outRoot;            // out/
	std::filesystem::path dependencyRoot;     // dependencies/
	std::string hostTag;                      // "linux-x86_64", for aliases
	std::optional<std::string> workspaceText; // .kiln/local.json, read by the caller
	std::string workspaceFile = ".kiln/local.json";
};

struct ProfileSummary
{
	std::string name;
	std::string file;
	std::string schema;
	std::vector<std::string> aliases;
	bool buildable = false;
	bool fragment = false;
	std::optional<std::string> error; // a v2 profile that fails validation
};

struct StageRecord
{
	std::string name;
	std::string role;
	bool upToDate = false;
	std::uint64_t workItems = 0;
	std::string summary;
	foundation::json::Value evidence;
};

struct PipelineRequest
{
	std::string profile; // a name, file or alias
	std::optional<std::string> flavor;
	product::StageRole upTo = product::StageRole::kEngine;
	std::optional<std::string> device;  // a workspace device, for deploy and run
	std::vector<std::string> arguments; // appended to the launch arguments
	const product::ICancellation *cancel = nullptr;
};

struct PipelineResult
{
	std::string profile;
	std::string flavor;
	std::filesystem::path tree;
	std::vector<StageRecord> stages;
	std::vector<product::Artifact> artifacts;
	std::optional<product::PackageManifest> package;
	std::optional<product::LaunchResult> launch;
	std::filesystem::path evidenceFile;
	foundation::json::Value evidence;
	bool UpToDate() const;
};

struct DoctorResult
{
	std::string profile;
	product::DoctorReport report;
};

class Session
{
public:
	Session( const product::ProviderCatalog &catalog, platform::IToolProcessProvider &processes,
	    jobsystem::IGraphExecutor &executor, product::IDiagnosticSink &diagnostics,
	    SessionConfig config );

	// Every profile file, fragments included, with validation status.
	[[nodiscard]] std::vector<ProfileSummary> ListProfiles() const;
	// The merged document of any profile (v1 families too), validated when v2.
	[[nodiscard]] foundation::Expected<product::ResolvedProfile, Error> ResolveProfile(
	    const std::string &nameOrAlias ) const;
	// The workspace overlay applied, for launch consumers.
	[[nodiscard]] foundation::Expected<product::ResolvedProfile, Error> ResolveForLaunch(
	    const std::string &nameOrAlias ) const;
	[[nodiscard]] foundation::Expected<DoctorResult, Error> Doctor(
	    const std::string &nameOrAlias );
	// Runs the profile's stage graph up to request.upTo.
	[[nodiscard]] foundation::Expected<PipelineResult, Error> Run( const PipelineRequest &request );

	const SessionConfig &Config() const { return m_Config; }

private:
	foundation::Expected<std::string, Error> Locate( const std::string &nameOrAlias ) const;
	foundation::Expected<product::Workspace, Error> LoadWorkspace() const;

	const product::ProviderCatalog &m_Catalog;
	platform::IToolProcessProvider &m_Processes;
	jobsystem::IGraphExecutor &m_Executor;
	product::IDiagnosticSink &m_Diagnostics;
	SessionConfig m_Config;
	product::DirectoryProfileSource m_Source;
};

// Results as versioned JSON (`kiln --json`, sepipe).
foundation::json::Value ToJson( const ProfileSummary &summary );
foundation::json::Value ToJson( const PipelineResult &result );
foundation::json::Value ToJson( const DoctorResult &result );
foundation::json::Value ExplainJson( const product::ResolvedProfile &profile );

} // namespace kiln

#endif // PUBLIC_KILN_API_H
