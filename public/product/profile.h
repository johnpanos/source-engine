//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.profile (RFC 0027, contract product.profile.v2): product
//			profile files, the one `extends` merge, schema v2 validation and
//			the facts derived from a resolved profile.
//
//			Files reach this library only through an IProfileSource; it reads
//			no environment, current directory or repository root. It keeps no
//			global state, writes nothing to stdout or stderr and is safe to use
//			from several threads on distinct values.
//
//			The merge rule (the only one): `extends` names one parent file or a
//			list of them, relative to the child; parents merge left to right,
//			then the child over the result. Objects merge member by member and
//			any other value replaces. Existing members keep their position and
//			new ones append, as Python's dict update does, so a resolved v1
//			profile equals tools/quality/profile_extends.py's.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_PROFILE_H
#define PUBLIC_PRODUCT_PROFILE_H

#include "foundation/expected.h"
#include "foundation/json.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace product
{

inline constexpr std::string_view kProfileSchemaV2 = "source-product-profile/v2";
inline constexpr std::string_view kFragmentSchemaV2 = "source-product-fragment/v2";

// Every refusal names its code, the file and the profile key it is about.
enum class ProfileErrorCode
{
	kIo,
	kParse,
	kSchema,
	kExtendsCycle,
	kUnknownKey,
	kUnknownProvider,
	kMissingPin,
	kConflictingOption,
	kWorkspaceBuildFact,
	kNotBuildable,
	kUnknownProfile,
};

struct ProfileError
{
	ProfileErrorCode code = ProfileErrorCode::kSchema;
	std::string file;
	std::string key;
	std::string detail;

	// "unknown-key quality/product_profiles/x.json build.flavour: ..."
	std::string Describe() const;
};

std::string_view ProfileErrorCodeName( ProfileErrorCode code );

// Where profile files come from. Names are '/'-separated paths relative to
// the source's root (for example "fragments/linux-desktop.json").
class IProfileSource
{
public:
	virtual ~IProfileSource() = default;
	[[nodiscard]] virtual foundation::Expected<std::string, ProfileError> Read(
	    const std::string &name ) const = 0;
	// Every profile file, fragments included, sorted.
	[[nodiscard]] virtual std::vector<std::string> List() const = 0;
};

// A directory of profile files (quality/product_profiles), given explicitly.
class DirectoryProfileSource final : public IProfileSource
{
public:
	explicit DirectoryProfileSource( std::filesystem::path root ) : m_Root( std::move( root ) ) {}
	foundation::Expected<std::string, ProfileError> Read( const std::string &name ) const override;
	std::vector<std::string> List() const override;
	const std::filesystem::path &Root() const { return m_Root; }

private:
	std::filesystem::path m_Root;
};

// One leaf's origin: the file whose value won the merge.
struct Provenance
{
	std::string path; // "build.toolchain", "dependencies.pkg_config.sdl3"
	std::string file;
};

// The `extends` chain resolved, with no `extends` member left.
struct MergedDocument
{
	foundation::json::Value document;
	std::vector<std::string> files; // the chain, parents first, without repeats
	std::vector<Provenance> provenance;
};

[[nodiscard]] foundation::Expected<MergedDocument, ProfileError> ResolveExtends(
    const IProfileSource &source, const std::string &name );

// Provider names the catalog offers, per profile key. Validation refuses any
// name not listed (RFC 0027: no fallback provider).
struct ProviderNames
{
	std::set<std::string> toolchains;      // build.toolchain
	std::set<std::string> stages;          // pipeline.stages
	std::set<std::string> packagers;       // package.form
	std::set<std::string> transports;      // deploy.transport
	std::set<std::string> displaySessions; // launch.display_session
	std::set<std::string> runProviders;    // launch.run
};

struct ConfigureOption
{
	std::string key; // the profile key, e.g. "build_games"
	foundation::json::Value value;
};

struct Flavor
{
	std::string name;
	std::string description;
	std::vector<ConfigureOption> configureOptions; // added to the base options
};

struct Switch
{
	std::string name;
	std::string description;
	std::vector<std::string> arguments;
	std::vector<std::string> conflicts;
	// Launch variables the switch sets (launch.variables names them).
	std::map<std::string, std::vector<std::string>> sets;
};

// A validated schema v2 profile and its derived facts. Everything here is a
// value; it borrows nothing from the source.
struct ResolvedProfile
{
	std::string name; // the file stem, e.g. "portal2"
	std::string file; // "portal2.json"
	std::string id;
	std::string schema;
	bool fragment = false;
	foundation::json::Value document;
	std::vector<std::string> files;
	std::vector<Provenance> provenance;
	std::vector<std::string> aliases;

	// Build facts (absent for a profile with no `build`).
	std::string toolchain;
	std::vector<ConfigureOption> configureOptions;
	std::vector<Flavor> flavors;
	std::string defaultFlavor;
	std::vector<std::string> stages;
	std::vector<Switch> switches;
	// launch.variables: named argument lists the launch templates splice.
	std::map<std::string, std::vector<std::string>> launchVariables;

	bool Buildable() const { return !fragment && !toolchain.empty(); }
	const Flavor *FindFlavor( std::string_view flavor ) const;

	// The value's digest: FNV-1a 64 over the compact canonical document. It
	// changes with any fact of the profile, which is what tree identity needs.
	std::uint64_t Digest() const;

	// Waf configure arguments for a flavor: each option as --key-with-dashes
	// (true: the bare flag; false: omitted; else =value), base options first,
	// then the flavor's. The tree and prefix are the caller's (kiln owns them).
	[[nodiscard]] foundation::Expected<std::vector<std::string>, ProfileError> WafArguments(
	    std::string_view flavor ) const;

	// The tree a flavor builds into, relative to the output root.
	std::string TreeName( std::string_view flavor ) const;
	// The package's directory inside a tree: `package.directory`, else
	// "package". Packaging writes it and launches run from it.
	std::string PackageDirectoryName() const;
};

// Merge, then validate. A v1 file of any schema family resolves for
// inspection (`kiln profiles resolve`) but is not buildable; a v2 profile or
// fragment is validated strictly against schema v2 and `names`.
[[nodiscard]] foundation::Expected<ResolvedProfile, ProfileError> Resolve(
    const IProfileSource &source, const std::string &name, const ProviderNames &names );

// Find a profile by file stem, file name or alias. An alias may carry a host
// suffix ("portal2" on linux-x86_64 also matches an alias "portal2@linux-x86_64").
[[nodiscard]] foundation::Expected<std::string, ProfileError> FindProfile(
    const IProfileSource &source, std::string_view nameOrAlias, std::string_view hostTag );

// The workspace file (.kiln/local.json), which the caller reads and passes.
// It may set launch values and personal settings, never a build fact: any
// other per-profile member is refused by name (kWorkspaceBuildFact), so two
// checkouts building the same profile configure the same tree.
struct Workspace
{
	std::optional<std::string> defaultProfile;
	std::optional<std::string> defaultMap;
	foundation::json::Value document; // the whole file, for launch providers
};

[[nodiscard]] foundation::Expected<Workspace, ProfileError> ParseWorkspace(
    std::string_view text, std::string_view file );

// The profile with the workspace's launch overrides for it merged into
// `launch`. Build facts are unchanged by construction.
[[nodiscard]] foundation::Expected<ResolvedProfile, ProfileError> ApplyWorkspace(
    const ResolvedProfile &profile, const Workspace &workspace );

// The merge rule on two values (exposed for tests and tools).
void MergeInto( foundation::json::Value &base, const foundation::json::Value &derived );

} // namespace product

#endif // PUBLIC_PRODUCT_PROFILE_H
