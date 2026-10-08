//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.contracts (RFC 0027, product.contracts.v1): the extension
//			contracts of the product pipeline and the typed ProviderCatalog.
//
//			A platform, package form, transport or pipeline step is added by
//			writing a provider of these contracts and listing it in a catalog;
//			kiln.core never changes. Each contract states what substitution
//			requires, and one shared suite per contract (unittests/kilntest)
//			runs against every claiming provider, fakes included, and rejects
//			the deliberately bad providers listed with it.
//
//			Common rules for every provider:
//			  - Inputs come from the request; providers read no environment,
//			    current directory or repository root of their own.
//			  - Writes go only where the request says (a tree, a prefix root,
//			    a staging directory, a device root) and publish atomically.
//			  - Recoverable errors are values (foundation::Expected); nothing
//			    is printed. Diagnostics go to the injected sink.
//			  - Cancellation is a borrowed token; a cancelled call publishes
//			    nothing and returns kCancelled.
//			  - Providers are owned by the catalog, which outlives every
//			    request that borrows them. A provider may be called from any
//			    one thread at a time unless it says otherwise.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_CONTRACTS_H
#define PUBLIC_PRODUCT_CONTRACTS_H

#include "foundation/expected.h"
#include "platform/contracts/process_spawn.h"
#include "platform/contracts/tool_process.h"
#include "product/profile.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace product
{

//-----------------------------------------------------------------------------
// Shared vocabulary
//-----------------------------------------------------------------------------

class ICancellation
{
public:
	virtual ~ICancellation() = default;
	virtual bool IsCancelled() const noexcept = 0;
};

// A cancellation token a caller owns and may set from any thread.
class CancellationFlag final : public ICancellation, public platform::IToolProcessCancellation
{
public:
	void Cancel() noexcept { m_Cancelled.store( true, std::memory_order_release ); }
	bool IsCancelled() const noexcept override
	{
		return m_Cancelled.load( std::memory_order_acquire );
	}
	bool IsCancellationRequested() const noexcept override { return IsCancelled(); }

private:
	std::atomic<bool> m_Cancelled{ false };
};

enum class Severity
{
	kInfo,
	kWarning,
	kError,
};

// Diagnostics and progress. Implementations must accept calls from the
// thread running the request.
class IDiagnosticSink
{
public:
	virtual ~IDiagnosticSink() = default;
	virtual void Report( Severity severity, std::string_view source, std::string_view message ) = 0;
};

// One provider failure. `code` is a stable kebab-case name.
struct ProviderError
{
	std::string code;
	std::string detail;
};

inline constexpr std::string_view kCancelled = "cancelled";
inline constexpr std::string_view kUnavailable = "unavailable";

// What `kiln doctor` reports: each prerequisite present or unavailable.
struct DoctorItem
{
	std::string name;
	bool present = false;
	std::string detail;
};

struct DoctorReport
{
	std::vector<DoctorItem> items;
	void Add( std::string name, bool present, std::string detail )
	{
		items.push_back( DoctorItem{ std::move( name ), present, std::move( detail ) } );
	}
};

// An identity: named facts, each an input that can change an output. The
// digest (FNV-1a 64 over "key=value\n" in key order) keys caches and trees.
struct Identity
{
	std::string provider;
	std::map<std::string, std::string> facts;
	std::uint64_t Digest() const;
	std::string Hex() const;
};

// FNV-1a 64 of bytes, as lowercase hex; the pipeline's change detector for
// local trees and manifests (not a security hash).
std::string HashHex( std::string_view bytes );
std::uint64_t Hash64( std::string_view bytes );

//-----------------------------------------------------------------------------
// ITargetToolchain: one target's compilers, SDK and sysroot.
//
// - Identity() covers every input that can change an output (compiler and
//   linker identity, SDK version, sysroot, deployment target). The real
//   facts come from the host; an identity omitting the SDK version (for an
//   SDK-based target) is a bad provider.
// - Prepare() verifies the profile's pins before use, writes only under
//   `dependencyRoot/<provider>/`, is idempotent, downloads only pinned and
//   verified archives, and returns the environment Waf and recipe builders
//   consume. It fails naming the missing prerequisite; succeeding with a
//   missing compiler is a bad provider.
// - CheckHost() reports each prerequisite as present or unavailable.
//-----------------------------------------------------------------------------

struct ToolchainRequest
{
	const ResolvedProfile *profile = nullptr;
	std::filesystem::path sourceRoot;     // read-only
	std::filesystem::path dependencyRoot; // the only writable place
	const ICancellation *cancel = nullptr;
};

struct ToolchainEnvironment
{
	Identity identity;
	std::vector<platform::ToolProcessEnvironmentOverride> environment;
	std::vector<std::string> wafOptions; // cross options; empty for the host
	std::optional<std::filesystem::path> cmakeToolchainFile;
};

class ITargetToolchain
{
public:
	virtual ~ITargetToolchain() = default;
	virtual std::string_view Name() const noexcept = 0;
	[[nodiscard]] virtual foundation::Expected<ToolchainEnvironment, ProviderError> Prepare(
	    const ToolchainRequest &request ) = 0;
	virtual void CheckHost( const ResolvedProfile *profile, DoctorReport &report ) = 0;
};

//-----------------------------------------------------------------------------
// IRecipeBuilder: one declared third-party recipe built for a toolchain into
// a prefix.
//
// - The prefix is `prefixRoot/<recipe>-<key>` where key covers the recipe,
//   its pin and the toolchain identity; a stale prefix is never reused
//   across toolchains.
// - Publication is atomic (staging, then rename); a failure leaves the
//   previous prefix in place and no partial one.
// - Building an existing key again does no work (idempotent).
//-----------------------------------------------------------------------------

struct Recipe
{
	std::string name;
	std::string pin; // exact version or archive digest
	foundation::json::Value definition;
};

struct RecipeRequest
{
	Recipe recipe;
	const ToolchainEnvironment *toolchain = nullptr;
	std::filesystem::path prefixRoot;
	const ICancellation *cancel = nullptr;
};

struct RecipeResult
{
	std::filesystem::path prefix;
	std::string key;
	bool built = false; // false when an existing prefix was reused
};

class IRecipeBuilder
{
public:
	virtual ~IRecipeBuilder() = default;
	virtual std::string_view Name() const noexcept = 0;
	[[nodiscard]] virtual foundation::Expected<RecipeResult, ProviderError> Build(
	    const RecipeRequest &request ) = 0;
};

// The key a recipe's prefix is published under; every builder uses it.
std::string RecipeKey( const Recipe &recipe, const Identity &toolchain );

//-----------------------------------------------------------------------------
// IProductStage: one step of a product pipeline.
//
// - Describe() declares the role, the artifacts consumed and produced, and
//   the determinism. Roles order the pipeline; artifacts order stages
//   within it. The built-in steps (the Waf engine build, the content build)
//   are providers like any other.
// - Run() reads only declared inputs, writes only declared outputs (through
//   staging, published atomically), and on cancellation publishes nothing.
// - Bad providers: reads an undeclared artifact, produces an undeclared one,
//   publishes a partial output, ignores cancellation.
//-----------------------------------------------------------------------------

enum class StageRole
{
	kDeps,
	kEngine,
	kContent,
	kPackage,
	kDeploy,
	kRun,
	kTest,
	kExtra,
};

std::string_view StageRoleName( StageRole role );
std::optional<StageRole> ParseStageRole( std::string_view name );

enum class Determinism
{
	kExact,
	kStatistical,
};

struct StageDescriptor
{
	StageRole role = StageRole::kExtra;
	std::vector<std::string> consumes;
	std::vector<std::string> produces;
	Determinism determinism = Determinism::kExact;
};

// A named, typed, hashed product of a stage.
struct Artifact
{
	std::string name;
	std::string type; // "tree", "directory", "package", "install", ...
	std::filesystem::path path;
	std::string digest;
	foundation::json::Value facts = foundation::json::Value::Object();
};

// What a stage may read. Lookups of undeclared artifacts are refused and
// recorded, so the runner fails the stage (the contract's first obligation).
class StageInputs
{
public:
	StageInputs( const ResolvedProfile &profile, std::string flavor,
	    std::filesystem::path sourceRoot, std::filesystem::path treeRoot,
	    std::vector<std::string> declared, const std::map<std::string, Artifact> &available,
	    const ToolchainEnvironment *toolchain, platform::IToolProcessProvider *processes,
	    IDiagnosticSink *diagnostics, const ICancellation *cancel );

	const ResolvedProfile &Profile() const { return m_Profile; }
	const std::string &Flavor() const { return m_Flavor; }
	const std::filesystem::path &SourceRoot() const { return m_SourceRoot; }
	const std::filesystem::path &TreeRoot() const { return m_TreeRoot; }
	// Resolved content locators (content.locators), name -> directory.
	const std::map<std::string, std::filesystem::path> &Locations() const { return m_Locations; }
	void SetLocations( std::map<std::string, std::filesystem::path> locations )
	{
		m_Locations = std::move( locations );
	}
	const ToolchainEnvironment *Toolchain() const { return m_Toolchain; }
	platform::IToolProcessProvider *Processes() const { return m_Processes; }
	IDiagnosticSink *Diagnostics() const { return m_Diagnostics; }
	const ICancellation *Cancel() const { return m_Cancel; }

	// nullptr when absent or undeclared; an undeclared read is recorded.
	const Artifact *Get( std::string_view name );
	const std::vector<std::string> &UndeclaredReads() const { return m_Undeclared; }

private:
	const ResolvedProfile &m_Profile;
	std::string m_Flavor;
	std::filesystem::path m_SourceRoot;
	std::filesystem::path m_TreeRoot;
	std::vector<std::string> m_Declared;
	const std::map<std::string, Artifact> &m_Available;
	const ToolchainEnvironment *m_Toolchain;
	platform::IToolProcessProvider *m_Processes;
	IDiagnosticSink *m_Diagnostics;
	const ICancellation *m_Cancel;
	std::vector<std::string> m_Undeclared;
	std::map<std::string, std::filesystem::path> m_Locations;
};

// What a stage produces. A stage stages its files under StagingDirectory()
// and calls Publish(); the runner moves each declared artifact into place
// only after Run succeeds, so a failed or cancelled stage publishes nothing.
class StageOutputs
{
public:
	explicit StageOutputs( std::filesystem::path stagingDirectory )
	    : m_Staging( std::move( stagingDirectory ) )
	{
	}
	const std::filesystem::path &StagingDirectory() const { return m_Staging; }
	void Publish( Artifact artifact ) { m_Published.push_back( std::move( artifact ) ); }
	const std::vector<Artifact> &Published() const { return m_Published; }

private:
	std::filesystem::path m_Staging;
	std::vector<Artifact> m_Published;
};

struct StageResult
{
	bool upToDate = false;       // no work was needed
	std::uint64_t workItems = 0; // compile/link/copy actions performed
	std::string summary;
	foundation::json::Value evidence = foundation::json::Value::Object();
};

class IProductStage
{
public:
	virtual ~IProductStage() = default;
	virtual std::string_view Name() const noexcept = 0;
	virtual StageDescriptor Describe( const ResolvedProfile &profile ) const = 0;
	[[nodiscard]] virtual foundation::Expected<StageResult, ProviderError> Run(
	    StageInputs &inputs, StageOutputs &outputs ) = 0;
};

//-----------------------------------------------------------------------------
// IPackager: build artifacts and content in, one platform package and its
// manifest out.
//
// - The manifest lists every file with its hash and role; the package holds
//   exactly those files, none from outside the declared inputs.
// - The same inputs give the same manifest.
// - Credentials (from the request, never from files) are never written into
//   the package, its manifest, the log or evidence.
// - A failure leaves no partial package (staging, then rename).
// - Bad providers: an undeclared file, an omitted declared module, a partial
//   publish, an embedded credential, an unstable manifest.
//-----------------------------------------------------------------------------

struct PackageInput
{
	std::filesystem::path source;
	std::string packagePath; // '/'-separated, relative
	std::string role;        // "module", "executable", "content", "config"
};

struct PackageRequest
{
	const ResolvedProfile *profile = nullptr;
	std::vector<PackageInput> inputs;
	std::filesystem::path output; // the package to publish (must not exist yet or is replaced)
	std::map<std::string, std::string> credentials; // never persisted
	const ICancellation *cancel = nullptr;
	// The build artifacts by name, for forms that lay out a whole tree.
	std::map<std::string, Artifact> artifacts;
	// Resolved content locators (content.locators, overridden by the
	// workspace's content_locations): name -> directory. A locator the host
	// lacks is absent; the form decides whether that is fatal.
	std::map<std::string, std::filesystem::path> locations;
	// The mount sets selected for this package (content.mount_sets).
	std::vector<std::string> mountSets;
	// The repository, for profile data that names its files (read-only).
	std::filesystem::path sourceRoot;
};

struct ManifestEntry
{
	std::string path;
	std::string hash;
	std::string role;
	std::uint64_t bytes = 0;
	bool operator==( const ManifestEntry &other ) const = default;
};

struct PackageManifest
{
	std::string form;
	std::vector<ManifestEntry> entries; // sorted by path
	std::string Digest() const;
	foundation::json::Value ToJson() const;
};

class IPackager
{
public:
	virtual ~IPackager() = default;
	virtual std::string_view Name() const noexcept = 0;
	[[nodiscard]] virtual foundation::Expected<PackageManifest, ProviderError> Package(
	    const PackageRequest &request ) = 0;
};

//-----------------------------------------------------------------------------
// IDeployTransport: one kind of device link, split into capabilities so no
// transport carries no-op methods. A transport returns only the capability
// interfaces it implements; a profile's deploy.capabilities are checked
// before the device is touched.
//
// Obligations: content sync transfers exactly the entries whose hashes
// differ and verifies them; nothing is written or deleted outside the
// device's content root; an interrupted install or sync leaves the previous
// install runnable; an unreachable device returns kUnavailable (never a
// pass); every operation is idempotent; a launch handle bounds its log;
// cancellation keeps the previous state; no credential reaches a log.
//-----------------------------------------------------------------------------

struct DeviceAddress
{
	std::string name;
	std::string address;
	std::filesystem::path contentRoot;              // the device's declared content root
	std::map<std::string, std::string> credentials; // never logged
};

struct SyncEntry
{
	std::string path; // relative to the content root
	std::string hash;
	std::filesystem::path source;
};

struct SyncResult
{
	std::uint64_t transferred = 0;
	std::uint64_t unchanged = 0;
	std::uint64_t removed = 0;
};

struct LaunchRequest
{
	std::vector<std::string> arguments;
	std::vector<platform::ToolProcessEnvironmentOverride> environment;
	const ICancellation *cancel = nullptr;
};

struct LaunchResult
{
	int exitCode = 0;
	std::vector<std::string> log; // bounded by the launch
};

class IInstall
{
public:
	virtual ~IInstall() = default;
	[[nodiscard]] virtual foundation::Expected<void, ProviderError> Install(
	    const DeviceAddress &device, const PackageManifest &manifest,
	    const std::filesystem::path &package, const ICancellation *cancel ) = 0;
};

class IContentSync
{
public:
	virtual ~IContentSync() = default;
	// `entries` is the complete wanted state of the content root.
	[[nodiscard]] virtual foundation::Expected<SyncResult, ProviderError> Sync(
	    const DeviceAddress &device, const std::vector<SyncEntry> &entries,
	    const ICancellation *cancel ) = 0;
};

class ILaunch
{
public:
	virtual ~ILaunch() = default;
	[[nodiscard]] virtual foundation::Expected<LaunchResult, ProviderError> Launch(
	    const DeviceAddress &device, const LaunchRequest &request ) = 0;
};

class IDeviceFacts
{
public:
	virtual ~IDeviceFacts() = default;
	[[nodiscard]] virtual foundation::Expected<foundation::json::Value, ProviderError> Facts(
	    const DeviceAddress &device ) = 0;
};

class ILogStream;
class ICrashCollect;
class IAttach;

class IDeployTransport
{
public:
	virtual ~IDeployTransport() = default;
	virtual std::string_view Name() const noexcept = 0;
	virtual IInstall *Install() noexcept { return nullptr; }
	virtual IContentSync *ContentSync() noexcept { return nullptr; }
	virtual ILaunch *Launch() noexcept { return nullptr; }
	virtual IDeviceFacts *DeviceFacts() noexcept { return nullptr; }
	virtual ILogStream *LogStream() noexcept { return nullptr; }
	virtual ICrashCollect *CrashCollect() noexcept { return nullptr; }
	virtual IAttach *Attach() noexcept { return nullptr; }
};

// The capability names profiles use ("install", "content-sync", "launch",
// "device-facts", "log-stream", "crash-collect", "attach").
bool TransportHasCapability( IDeployTransport &transport, std::string_view capability );
// Every required capability the transport lacks, by name; checked before a
// device is touched.
std::vector<std::string> MissingCapabilities(
    IDeployTransport &transport, const std::vector<std::string> &required );

//-----------------------------------------------------------------------------
// IDisplaySession: where a launched program draws (RFC 0027 L1 names
// `user`, `private` and `none`).
//
// - Open() returns the environment a launch adds; it must not change the
//   caller's environment.
// - A session that claims isolation never hands out the user's display.
// - Close() is idempotent and ends everything Open() started; a session
//   left open at destruction is closed.
//-----------------------------------------------------------------------------

// What a session needs: a scratch directory it owns for its private files
// (bus configuration, sockets) and the virtual monitor of an isolated one.
struct DisplayRequest
{
	std::filesystem::path scratch;
	int width = 1920;
	int height = 1080;
	double refreshHz = 60.0;
	const ICancellation *cancel = nullptr;
};

struct DisplayEnvironment
{
	// The session's variables; a run provider applies them after the
	// launch's, so they win (run-suite clause N7).
	std::vector<platform::ToolProcessEnvironmentOverride> environment;
	// Programs that wrap the launch (an isolated compositor runs it as its
	// child and ends with it); empty for the user's own session.
	std::vector<std::string> commandPrefix;
	bool isolated = false; // no window can reach the user's desktop
	bool headless = false; // no display at all
};

class IDisplaySession
{
public:
	virtual ~IDisplaySession() = default;
	virtual std::string_view Name() const noexcept = 0;
	virtual bool ClaimsIsolation() const noexcept = 0;
	[[nodiscard]] virtual foundation::Expected<DisplayEnvironment, ProviderError> Open(
	    const DisplayRequest &request ) = 0;
	virtual void Close() noexcept = 0;
	virtual bool IsOpen() const noexcept = 0;
};

//-----------------------------------------------------------------------------
// IRunProvider: how a launch plan runs on this host: one program (`single`),
// a co-op pair that connects (`coop-pair`), an installed game run as is
// (`external-install`). The plan's programs, arguments and environment are
// profile data; a provider adds only the mechanics (start order, readiness,
// waiting). It starts processes only through the spawner it is given, never
// starts a process it does not wait for or stop, and returns the exit status
// of the run (the first nonzero one). Cancellation stops every process it
// started.
//-----------------------------------------------------------------------------

struct LaunchSpec
{
	std::string name; // "game", "host", "client"
	std::vector<std::string> argv;
	std::vector<platform::ToolProcessEnvironmentOverride>
	    environment; // "{inherit}" filled by the spawner
	std::filesystem::path workingDirectory;
	// The program's output file (standard output and error); empty inherits.
	std::filesystem::path outputFile;
};

struct RunRequest
{
	std::vector<LaunchSpec> launches; // in start order
	std::map<std::string, std::string>
	    facts; // profile launch values a provider reads (ports, logs)
	DisplayEnvironment display;
	platform::IProcessSpawner *spawner = nullptr;
	IDiagnosticSink *diagnostics = nullptr;
	const ICancellation *cancel = nullptr;
	// Called once for each program as it starts, with the launch's name and
	// the spawner's process (run-suite clause N8); may be empty.
	std::function<void( const std::string &, platform::SpawnedProcess )> started;
};

class IRunProvider
{
public:
	virtual ~IRunProvider() = default;
	virtual std::string_view Name() const noexcept = 0;
	[[nodiscard]] virtual foundation::Expected<int, ProviderError> Run(
	    const RunRequest &request ) = 0;
};

//-----------------------------------------------------------------------------
// ProviderCatalog: the typed value a root composes. No global registry, no
// self-registration, no discovery by file name. Adding a provider whose name
// is already taken for its contract is refused.
//-----------------------------------------------------------------------------

struct CatalogError
{
	std::string contract; // "toolchain", "stage", ...
	std::string name;
	std::string detail;
	std::string Describe() const;
};

class ProviderCatalog
{
public:
	ProviderCatalog() = default;
	ProviderCatalog( ProviderCatalog && ) = default;
	ProviderCatalog &operator=( ProviderCatalog && ) = default;
	ProviderCatalog( const ProviderCatalog & ) = delete;
	ProviderCatalog &operator=( const ProviderCatalog & ) = delete;

	foundation::Expected<void, CatalogError> Add( std::unique_ptr<ITargetToolchain> provider );
	foundation::Expected<void, CatalogError> Add( std::unique_ptr<IRecipeBuilder> provider );
	foundation::Expected<void, CatalogError> Add( std::unique_ptr<IProductStage> provider );
	foundation::Expected<void, CatalogError> Add( std::unique_ptr<IPackager> provider );
	foundation::Expected<void, CatalogError> Add( std::unique_ptr<IDeployTransport> provider );
	foundation::Expected<void, CatalogError> Add( std::unique_ptr<IDisplaySession> provider );
	foundation::Expected<void, CatalogError> Add( std::unique_ptr<IRunProvider> provider );

	foundation::Expected<ITargetToolchain *, CatalogError> Toolchain( std::string_view name ) const;
	foundation::Expected<IRecipeBuilder *, CatalogError> RecipeBuilder(
	    std::string_view name ) const;
	foundation::Expected<IProductStage *, CatalogError> Stage( std::string_view name ) const;
	foundation::Expected<IPackager *, CatalogError> Packager( std::string_view name ) const;
	foundation::Expected<IDeployTransport *, CatalogError> Transport( std::string_view name ) const;
	foundation::Expected<IDisplaySession *, CatalogError> DisplaySession(
	    std::string_view name ) const;
	foundation::Expected<IRunProvider *, CatalogError> RunProvider( std::string_view name ) const;

	// The names profiles may select, for product::Resolve.
	ProviderNames Names() const;

private:
	template <typename T> using List = std::vector<std::unique_ptr<T>>;
	List<ITargetToolchain> m_Toolchains;
	List<IRecipeBuilder> m_RecipeBuilders;
	List<IProductStage> m_Stages;
	List<IPackager> m_Packagers;
	List<IDeployTransport> m_Transports;
	List<IDisplaySession> m_DisplaySessions;
	List<IRunProvider> m_RunProviders;
};

} // namespace product

#endif // PUBLIC_PRODUCT_CONTRACTS_H
