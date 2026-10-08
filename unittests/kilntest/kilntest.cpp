//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0027 L0 conformance: foundation.json, product.profile's
//			schema v2 and its seeded faults, every product contract's shared
//			suite against the real providers, the fakes and the deliberately
//			bad providers, the ProviderCatalog, and the fixture platform run
//			end to end through an unchanged kiln.core.
//
//=============================================================================//

#include "bad_providers.h"
#include "fixture_platform/fixture_platform.h"
#include "suites.h"

#include "foundation/json.h"
#include "jobsystem/graph_executor.h"
#include "kiln/api.h"
#include "product/contracts.h"
#include "product/display_desktop.h"
#include "product/package_linux_dir.h"
#include "product/run_desktop.h"
#include "product/profile.h"
#include "product/stage_waf.h"
#include "product/toolchain_linux.h"
#include "testing/conformance_result.h"

#include "../../platform/posix/process_spawner.h"
#include "../../platform/posix/tool_process_provider.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

unsigned long g_Checks = 0;
unsigned long g_Failures = 0;

void Check( bool condition, const std::string &name )
{
	++g_Checks;
	if ( !condition )
	{
		++g_Failures;
		std::cerr << "FAIL: " << name << '\n';
	}
}

void CheckVerdict( const suites::Verdict &verdict, const std::string &name )
{
	for ( const std::string &violation : verdict.violations )
		std::cerr << "  " << name << ": " << violation << '\n';
	Check( verdict.Passed(), name + " passes its shared suite" );
}

// A bad provider must be rejected, by the clause it was written to break.
void CheckRejected(
    const suites::Verdict &verdict, const std::string &clause, const std::string &name )
{
	Check( verdict.Violates( clause ), name + " is rejected by " + clause );
}

class MemorySource final : public product::IProfileSource
{
public:
	std::map<std::string, std::string> files;
	foundation::Expected<std::string, product::ProfileError> Read(
	    const std::string &name ) const override
	{
		auto it = files.find( name );
		if ( it == files.end() )
			return foundation::MakeUnexpected(
			    product::ProfileError{ product::ProfileErrorCode::kIo, name, "", "absent" } );
		return it->second;
	}
	std::vector<std::string> List() const override
	{
		std::vector<std::string> names;
		for ( const auto &entry : files )
			names.push_back( entry.first );
		return names;
	}
};

class NullSink final : public product::IDiagnosticSink
{
public:
	void Report( product::Severity, std::string_view, std::string_view ) override {}
};

std::string HostCompilerVersion(
    platform::IToolProcessProvider &processes, const std::string &compiler )
{
	platform::ToolProcessRequest request;
	request.argv = { compiler, "--version" };
	request.workingDirectory = "/";
	request.executionTimeout = std::chrono::seconds( 30 );
	request.cancellationTimeout = std::chrono::seconds( 1 );
	const auto result = processes.Run( request );
	if ( !result.Succeeded() )
		return {};
	// The last whitespace-separated token that starts with a digit and has dots.
	const std::string line = result.stdoutData.substr( 0, result.stdoutData.find( '\n' ) );
	std::string version;
	size_t at = 0;
	while ( at < line.size() )
	{
		size_t end = line.find_first_of( " ()", at );
		if ( end == std::string::npos )
			end = line.size();
		const std::string token = line.substr( at, end - at );
		if ( !token.empty() && std::isdigit( static_cast<unsigned char>( token[0] ) ) &&
		     token.find( '.' ) != std::string::npos && version.empty() )
			version = token;
		at = end + 1;
	}
	return version;
}

product::ProviderNames FixtureNames()
{
	product::ProviderNames names;
	names.toolchains = { "fixture-host", "linux-gcc", "linux-clang" };
	names.stages = { "fixture-compile", "fixture-content", "fixture-symbols", "waf-engine" };
	names.packagers = { "fixture-dir" };
	names.transports = { "fixture-device" };
	names.displaySessions = { "fixture-headless" };
	return names;
}

//-----------------------------------------------------------------------------

void JsonChecks()
{
	auto parsed = foundation::json::Parse( R"({"b":1.50,"a":[true,null,"xé"],"c":{"d":-2e3}})" );
	Check( parsed.HasValue(), "json.parse" );
	if ( !parsed )
		return;
	const Value &value = parsed.Value();
	Check( value.Members().front().first == "b", "json.member-order-kept" );
	Check( value.Find( "b" )->Text() == "1.50", "json.number-text-kept" );
	auto again = foundation::json::Parse( value.WritePretty() );
	Check( again && again.Value() == value, "json.pretty-round-trip" );
	auto reordered = foundation::json::Parse( R"({"c":{"d":-2000},"a":[true,null,"xé"],"b":1.5})" );
	Check( reordered && reordered.Value() == value,
	    "json.equality-ignores-member-order-and-number-spelling" );
	auto different = foundation::json::Parse( R"({"c":{"d":-2000},"a":[null,true,"xé"],"b":1.5})" );
	Check( different && !( different.Value() == value ), "json.equality-keeps-array-order" );
	Check( !foundation::json::Parse( R"({"a":1,"a":2})" ), "json.reject-duplicate-member" );
	Value object = Value::Object();
	object.Set( "x", Value::Number( 1 ) );
	object.Set( "y", Value::Number( 2 ) );
	Check( object.Remove( "x" ) && !object.Find( "x" ) && !object.Remove( "x" ), "json.remove" );
	Check( Value::Object().WritePretty() == "{}" && Value::Array().WritePretty() == "[]",
	    "json.pretty-empty" );
}

//-----------------------------------------------------------------------------

constexpr const char *kFragment = R"({
  "schema": "source-product-fragment/v2",
  "id": "desktop",
  "description": "test fragment",
  "toolchain": {"family": "gcc", "version": "16.2.1", "cxx": "g++"},
  "dependencies": {"pkg_config": {"sdl3": "3.4.16"}},
  "configure_options": {"build_type": "release", "disable_warns": true},
  "build": {"toolchain": "linux-gcc", "flavors": {
    "dev": {"description": "dev", "configure_options": {}},
    "release": {"description": "release", "configure_options": {"product_flavor": "release", "enable_lto": true}}}},
  "pipeline": {"stages": ["waf-engine"]},
  "launch": {"switches": {"a": {"description": "A", "arguments": ["+a", "1"], "conflicts": ["b"]},
                          "b": {"description": "B", "arguments": ["+b", "1"]}}}
})";

std::string Game( const std::string &extra )
{
	return R"({"schema": "source-product-profile/v2", "id": "game", "description": "game",
	  "extends": ["fragments/desktop.json"], "aliases": ["game@linux-x86_64"],
	  "target": {"product": "game", "os": "linux", "architecture": "x86_64"},
	  "configure_options": {"build_games": "portal2"})" +
	       extra + "}";
}

bool FailsWith( const MemorySource &source, const std::string &name, product::ProfileErrorCode code,
    const std::string &key = {} )
{
	auto resolved = product::Resolve( source, name, FixtureNames() );
	if ( resolved )
		return false;
	if ( resolved.Error().code != code )
		std::cerr << "  got " << resolved.Error().Describe() << '\n';
	return resolved.Error().code == code && ( key.empty() || resolved.Error().key == key );
}

void ProfileChecks()
{
	MemorySource source;
	source.files["fragments/desktop.json"] = kFragment;
	source.files["game.json"] = Game( "" );
	auto resolved = product::Resolve( source, "game.json", FixtureNames() );
	Check( resolved.HasValue(), "profile.valid-v2-resolves" );
	if ( resolved )
	{
		const product::ResolvedProfile &game = resolved.Value();
		Check( game.Buildable() && game.toolchain == "linux-gcc" && game.defaultFlavor == "dev",
		    "profile.build-facts" );
		Check( !game.document.Find( "extends" ), "profile.extends-removed" );
		Check( game.document.Members().front().first == "schema",
		    "profile.merge-keeps-parent-member-order" );
		auto dev = game.WafArguments( "dev" );
		const std::vector<std::string> expected = {
		    "--build-type=release", "--disable-warns", "--build-games=portal2" };
		Check( dev && dev.Value() == expected, "profile.waf-arguments" );
		auto release = game.WafArguments( "release" );
		Check( release && release.Value().size() == 5 &&
		           release.Value()[3] == "--product-flavor=release" &&
		           release.Value()[4] == "--enable-lto",
		    "profile.flavor-adds-options" );
		Check( !game.WafArguments( "nope" ), "profile.unknown-flavor-refused" );
		Check( game.TreeName( "dev" ) == "game/dev", "profile.tree-name" );
		Check( game.switches.size() == 2, "profile.switches-from-fragment" );
		bool provenance = false;
		for ( const auto &entry : game.provenance )
			provenance |= entry.path == "build.toolchain" && entry.file == "fragments/desktop.json";
		Check( provenance, "profile.provenance-names-the-fragment" );
		auto found = product::FindProfile( source, "game", "linux-x86_64" );
		Check( found && found.Value() == "game.json", "profile.alias-resolves" );
		Check( !product::FindProfile( source, "game@x", "windows-x86_64" ) &&
		           !product::FindProfile( source, "nothing", "linux-x86_64" ),
		    "profile.unknown-alias-refused" );
		MemorySource changed = source;
		changed.files["game.json"] = Game( R"(, "launch": {"default_map": "x"})" );
		auto other = product::Resolve( changed, "game.json", FixtureNames() );
		Check( other && other.Value().Digest() != game.Digest(),
		    "profile.digest-follows-the-document" );
	}

	// Seeded faults (RFC 0027 L0 gate): each is refused by name.
	{
		MemorySource cyclic;
		cyclic.files["a.json"] = R"({"schema": "source-product-profile/v2", "extends": "b.json"})";
		cyclic.files["b.json"] = R"({"schema": "source-product-fragment/v2", "extends": "a.json"})";
		Check( FailsWith( cyclic, "a.json", product::ProfileErrorCode::kExtendsCycle, "extends" ),
		    "fault.extends-cycle" );
		MemorySource self;
		self.files["a.json"] = R"({"extends": ["a.json"]})";
		Check( FailsWith( self, "a.json", product::ProfileErrorCode::kExtendsCycle ),
		    "fault.extends-self" );
	}
	{
		MemorySource bad = source;
		bad.files["game.json"] = Game( R"(, "build": {"toolchian": "linux-gcc"})" );
		Check( FailsWith(
		           bad, "game.json", product::ProfileErrorCode::kUnknownKey, "build.toolchian" ),
		    "fault.unknown-key-in-section" );
		bad.files["game.json"] = Game( R"(, "lauch": {})" );
		Check( FailsWith( bad, "game.json", product::ProfileErrorCode::kUnknownKey, "lauch" ),
		    "fault.unknown-top-level-key" );
	}
	{
		MemorySource bad = source;
		bad.files["game.json"] = Game( R"(, "build": {"toolchain": "linux-icc"})" );
		Check( FailsWith( bad, "game.json", product::ProfileErrorCode::kUnknownProvider,
		           "build.toolchain" ),
		    "fault.unknown-toolchain-provider" );
		bad.files["game.json"] =
		    Game( R"(, "pipeline": {"stages": ["waf-engine", "shader-cache"]})" );
		Check( FailsWith( bad, "game.json", product::ProfileErrorCode::kUnknownProvider,
		           "pipeline.stages" ),
		    "fault.unknown-stage-provider" );
		bad.files["game.json"] = Game( R"(, "package": {"form": "linux-dir"})" );
		Check( FailsWith(
		           bad, "game.json", product::ProfileErrorCode::kUnknownProvider, "package.form" ),
		    "fault.unknown-packager" );
	}
	{
		MemorySource bad = source;
		bad.files["game.json"] = Game( R"(, "toolchain": {"version": ""})" );
		Check( FailsWith(
		           bad, "game.json", product::ProfileErrorCode::kMissingPin, "toolchain.version" ),
		    "fault.missing-toolchain-pin" );
		bad.files["game.json"] = Game( R"(, "dependencies": {"pkg_config": {"vulkan": ""}})" );
		Check( FailsWith( bad, "game.json", product::ProfileErrorCode::kMissingPin,
		           "dependencies.pkg_config.vulkan" ),
		    "fault.missing-dependency-pin" );
	}
	{
		MemorySource bad = source;
		bad.files["game.json"] =
		    R"({"schema": "source-product-profile/v2", "extends": ["fragments/desktop.json"],
		  "configure_options": {"build_games": "portal2", "out": "elsewhere"}})";
		Check( FailsWith( bad, "game.json", product::ProfileErrorCode::kConflictingOption,
		           "configure_options.out" ),
		    "fault.option-kiln-owns" );
		bad.files["game.json"] =
		    R"({"schema": "source-product-profile/v2", "extends": ["fragments/desktop.json"],
		  "configure_options": {"tools": true, "dedicated": true}})";
		Check( FailsWith( bad, "game.json", product::ProfileErrorCode::kConflictingOption,
		           "configure_options" ),
		    "fault.two-product-selectors" );
		bad.files["game.json"] =
		    R"({"schema": "source-product-profile/v2", "extends": ["fragments/desktop.json"],
		  "build": {"flavors": {"release": {"configure_options": {"build_type": "debug"}}}}})";
		Check( FailsWith( bad, "game.json", product::ProfileErrorCode::kConflictingOption,
		           "build.flavors.release.configure_options.build_type" ),
		    "fault.flavor-changes-a-base-option" );
	}
	{
		auto workspace = product::ParseWorkspace(
		    R"({"profiles": {"game": {"configure_options": {"build_games": "hl2"}}}})",
		    ".kiln/local.json" );
		Check( !workspace &&
		           workspace.Error().code == product::ProfileErrorCode::kWorkspaceBuildFact &&
		           workspace.Error().key == "profiles.game.configure_options",
		    "fault.workspace-changes-a-build-fact" );
		auto topLevel = product::ParseWorkspace(
		    R"({"build": {"toolchain": "linux-clang"}})", ".kiln/local.json" );
		Check( !topLevel && topLevel.Error().code == product::ProfileErrorCode::kWorkspaceBuildFact,
		    "fault.workspace-top-level-build-fact" );
		auto fine = product::ParseWorkspace(
		    R"({"default_profile": "game", "windowed": true, "profiles": {"game": {"launch": {"default_map": "sp_x"}}}})",
		    ".kiln/local.json" );
		Check( fine.HasValue() && fine.Value().defaultProfile == std::string( "game" ),
		    "profile.workspace-accepted" );
		if ( fine && resolved )
		{
			auto applied = product::ApplyWorkspace( resolved.Value(), fine.Value() );
			Check( applied &&
			           *applied.Value().document.Find( "launch" )->FindString( "default_map" ) ==
			               "sp_x" &&
			           applied.Value().WafArguments( "dev" ).Value() ==
			               resolved.Value().WafArguments( "dev" ).Value(),
			    "profile.workspace-overrides-launch-only" );
		}
		auto switches = product::ParseWorkspace(
		    R"({"profiles": {"game": {"launch": {"switches": {}}}}})", ".kiln/local.json" );
		if ( switches && resolved )
			Check( !product::ApplyWorkspace( resolved.Value(), switches.Value() ),
			    "fault.workspace-replaces-switches" );
	}
	{
		MemorySource bad = source;
		bad.files["game.json"] =
		    Game( R"(, "launch": {"switches": {"c": {"description": "C", "conflicts": ["zz"]}}})" );
		Check( FailsWith( bad, "game.json", product::ProfileErrorCode::kSchema ),
		    "fault.switch-conflicts-unknown-switch" );
	}
	{
		// v1 families resolve for inspection but are never buildable.
		MemorySource v1;
		v1.files["old.json"] =
		    R"({"schema": "source-android-product-profile/v1", "anything": {"goes": 1}})";
		auto old = product::Resolve( v1, "old.json", FixtureNames() );
		Check( old && !old.Value().Buildable(), "profile.v1-inspection-only" );
	}
	{
		// The merge rule equals profile_extends.py's: objects merge, the
		// rest replaces, existing members keep their position.
		Value base = foundation::json::Parse( R"({"a":1,"o":{"x":1,"y":[1]},"z":0})" ).Value();
		const Value derived =
		    foundation::json::Parse( R"({"o":{"y":[2],"w":3},"a":{"n":1},"new":true})" ).Value();
		product::MergeInto( base, derived );
		Check( base.Write() == R"({"a":{"n":1},"o":{"x":1,"y":[2],"w":3},"z":0,"new":true})",
		    "profile.merge-rule" );
	}
}

//-----------------------------------------------------------------------------

struct Workbench
{
	fs::path root;
	fs::path source;
	fs::path dependencies;
	fs::path tree;
	fs::path scratch;
};

product::ResolvedProfile MakeProfile(
    const std::string &json, const std::string &name = "fixture.json" )
{
	MemorySource source;
	source.files[name] = json;
	auto resolved = product::Resolve( source, name, FixtureNames() );
	if ( !resolved )
	{
		std::cerr << "profile: " << resolved.Error().Describe() << '\n';
		std::exit( 2 );
	}
	return resolved.Value();
}

std::string ToolchainProfile( const std::string &toolchain, const std::string &family,
    const std::string &cxx, const std::string &cc, const std::string &version )
{
	return R"({"schema": "source-product-profile/v2", "id": "t", "description": "toolchain case",
	  "toolchain": {"family": ")" +
	       family + R"(", "version": ")" + version + R"(", "cxx": ")" + cxx + R"(", "cc": ")" + cc +
	       R"("},
	  "build": {"toolchain": ")" +
	       toolchain + R"(", "flavors": {"dev": {"description": "d"}}}})";
}

void ToolchainChecks( const Workbench &bench, platform::IToolProcessProvider &posix )
{
	suites::RecordingProcesses processes( posix );
	const struct
	{
		std::string provider, family, cxx, cc;
	} hosts[] = { { "linux-gcc", "gcc", "g++", "gcc" },
	    { "linux-clang", "clang", "clang++", "clang" }, { "fixture-host", "host", "c++", "cc" } };
	for ( const auto &host : hosts )
	{
		const std::string version = HostCompilerVersion( posix, host.cxx );
		if ( version.empty() )
		{
			// An unavailable host compiler is recorded, never a pass.
			std::cerr << "unavailable: " << host.cxx << " for " << host.provider << '\n';
			Check( false, "toolchain." + host.provider + " host compiler available" );
			continue;
		}
		const auto valid = MakeProfile(
		    ToolchainProfile( host.provider, host.family, host.cxx, host.cc, version ) );
		const auto wrong = MakeProfile(
		    ToolchainProfile( host.provider, host.family, host.cxx, host.cc, "0.0.1" ) );
		std::unique_ptr<product::ITargetToolchain> toolchain;
		if ( host.provider == "linux-gcc" )
			toolchain = product::CreateLinuxGccToolchain( processes );
		else if ( host.provider == "linux-clang" )
			toolchain = product::CreateLinuxClangToolchain( processes );
		else
			toolchain = fixture::CreateHostToolchain( processes );
		CheckVerdict( suites::ToolchainSuite( *toolchain, processes,
		                  { &valid, &wrong, bench.source, bench.dependencies } ),
		    "toolchain " + host.provider );
		if ( host.provider == "linux-gcc" )
		{
			const auto mismatched =
			    MakeProfile( ToolchainProfile( "linux-gcc", "clang", "g++", "gcc", version ) );
			Check(
			    !toolchain->Prepare( { &mismatched, bench.source, bench.dependencies, nullptr } ),
			    "toolchain.linux-gcc refuses a clang profile" );
		}
	}
	const std::string version = HostCompilerVersion( posix, "c++" );
	const auto valid =
	    MakeProfile( ToolchainProfile( "fixture-host", "host", "c++", "cc", version ) );
	const auto wrong =
	    MakeProfile( ToolchainProfile( "fixture-host", "host", "c++", "cc", "0.0.1" ) );
	const struct
	{
		bad::ToolchainFault fault;
		const char *clause;
		const char *name;
	} faults[] = {
	    { bad::ToolchainFault::kIdentityWithoutSdk, "T3", "identity without the SDK version" },
	    { bad::ToolchainFault::kUnpinnedDownload, "T6", "unpinned download" },
	    { bad::ToolchainFault::kWritesSourceTree, "T5", "write into the source tree" },
	    { bad::ToolchainFault::kMissingCompiler, "T7", "success with a missing compiler" } };
	for ( const auto &fault : faults )
	{
		auto toolchain = bad::Toolchain( fault.fault, processes );
		CheckRejected( suites::ToolchainSuite( *toolchain, processes,
		                   { &valid, &wrong, bench.source, bench.dependencies } ),
		    fault.clause, std::string( "bad toolchain: " ) + fault.name );
		std::error_code ec;
		fs::remove( bench.source / "toolchain.stamp", ec );
	}
}

void RecipeChecks( const Workbench &bench )
{
	suites::RecipeCase input;
	input.prefixRoot = bench.scratch / "prefixes";
	input.toolchainA.identity = { "a", { { "cxx.version", "1" }, { "sdk", "x" } } };
	input.toolchainB.identity = { "b", { { "cxx.version", "2" }, { "sdk", "y" } } };
	std::error_code ec;
	fs::remove_all( input.prefixRoot, ec );
	CheckVerdict( suites::RecipeSuite( *bad::RecipeBuilder( bad::RecipeFault::kNone ), input ),
	    "recipe builder fake" );
	fs::remove_all( input.prefixRoot, ec );
	CheckRejected(
	    suites::RecipeSuite( *bad::RecipeBuilder( bad::RecipeFault::kIgnoresToolchain ), input ),
	    "R4", "bad recipe builder: ignores the toolchain identity" );
	fs::remove_all( input.prefixRoot, ec );
	CheckRejected(
	    suites::RecipeSuite( *bad::RecipeBuilder( bad::RecipeFault::kPartialPublish ), input ),
	    "R6", "bad recipe builder: publishes a partial prefix" );
}

void StageChecks( const Workbench &bench, platform::IToolProcessProvider &posix,
    const product::ResolvedProfile &profile, const product::ToolchainEnvironment &toolchain )
{
	suites::StageCase input;
	input.profile = &profile;
	input.sourceRoot = bench.source;
	input.treeRoot = bench.scratch / "stage-tree";
	input.toolchain = &toolchain;
	input.processes = &posix;
	std::error_code ec;
	fs::remove_all( input.treeRoot, ec );
	product::Artifact install{
	    "engine-install", "install", bench.scratch / "fake-install", "abc", Value::Object() };
	fixture::WriteBytes( install.path / "fixture_app", "binary" );
	input.available["engine-install"] = install;
	CheckVerdict(
	    suites::StageSuite( *fixture::CreateCompileStage(), input ), "stage fixture-compile" );
	CheckVerdict(
	    suites::StageSuite( *fixture::CreateContentStage(), input ), "stage fixture-content" );
	CheckVerdict(
	    suites::StageSuite( *fixture::CreateSymbolStage(), input ), "stage fixture-symbols" );
	// waf-engine's success path is a real engine build: `kiln build` evidence
	// (tools/kiln/kiln_gate.py) covers it; here its descriptor and cancellation.
	suites::StageCase waf = input;
	waf.runSuccessPath = false;
	CheckVerdict( suites::StageSuite( *product::CreateWafEngineStage(), waf ),
	    "stage waf-engine (descriptor, cancellation)" );
	auto wafDescriptor = product::CreateWafEngineStage()->Describe( profile );
	Check( wafDescriptor.role == product::StageRole::kEngine &&
	           wafDescriptor.produces ==
	               std::vector<std::string>{ std::string( product::kEngineInstallArtifact ) },
	    "stage waf-engine produces engine-install in the engine role" );
	input.available.clear();
	CheckRejected( suites::StageSuite( *bad::Stage( bad::StageFault::kReadsUndeclared ), input ),
	    "S3", "bad stage: reads an undeclared artifact" );
	CheckRejected( suites::StageSuite( *bad::Stage( bad::StageFault::kProducesUndeclared ), input ),
	    "S4", "bad stage: produces an undeclared artifact" );
	CheckRejected( suites::StageSuite( *bad::Stage( bad::StageFault::kPartialPublish ), input ),
	    "S6", "bad stage: publishes a partial output" );
	CheckRejected( suites::StageSuite( *bad::Stage( bad::StageFault::kIgnoresCancel ), input ),
	    "S5", "bad stage: ignores cancellation" );
}

void PackagerChecks( const Workbench &bench, const product::ResolvedProfile &profile )
{
	suites::PackagerCase input{ &profile, bench.scratch / "packager" };
	CheckVerdict( suites::PackagerSuite( *fixture::CreateDirectoryPackager(), input ),
	    "packager fixture-dir" );
	const struct
	{
		bad::PackagerFault fault;
		const char *clause;
		const char *name;
	} faults[] = { { bad::PackagerFault::kUndeclaredFile, "P2", "an undeclared file" },
	    { bad::PackagerFault::kOmitsModule, "P1", "an omitted declared module" },
	    { bad::PackagerFault::kPartialPublish, "P5", "a partial publish" },
	    { bad::PackagerFault::kEmbedsCredential, "P4", "an embedded credential" },
	    { bad::PackagerFault::kUnstableManifest, "P3", "an unstable manifest" } };
	for ( const auto &fault : faults )
		CheckRejected( suites::PackagerSuite( *bad::Packager( fault.fault ), input ), fault.clause,
		    std::string( "bad packager: " ) + fault.name );
}

void TransportChecks( const Workbench &bench, platform::IToolProcessProvider &posix )
{
	fixture::FakeDevice device;
	suites::TransportCase input{
	    &device, bench.scratch / "transport", bench.scratch / "transport-package" };
	fixture::WriteBytes( input.package / "fixture_app",
	    "#!/bin/sh\nstatus=0\nfor a in \"$@\"; do echo \"arg $a\"; case $a in --exit=*) "
	    "status=${a#--exit=};; esac; "
	    "done\nexit $status\n" );
	fs::permissions( input.package / "fixture_app", fs::perms::owner_all );
	CheckVerdict( suites::TransportSuite( *fixture::CreateFakeTransport( device, posix ), input ),
	    "transport fixture-device" );
	const struct
	{
		bad::TransportFault fault;
		const char *clause;
		const char *name;
	} faults[] = {
	    { bad::TransportFault::kSuccessAfterPartial, "D7", "success after a partial transfer" },
	    { bad::TransportFault::kDeleteOutsideRoot, "D5", "a delete outside the root" },
	    { bad::TransportFault::kCredentialInLog, "D11", "a credential in the log" },
	    { bad::TransportFault::kClaimedNoEffect, "D9", "a claimed capability with no effect" },
	    { bad::TransportFault::kIgnoresCancel, "D6", "ignored cancellation" },
	    { bad::TransportFault::kResendsUnchanged, "D3", "resending unchanged entries" } };
	for ( const auto &fault : faults )
	{
		fixture::FakeDevice badDevice;
		suites::TransportCase badInput{ &badDevice, input.scratch, input.package };
		CheckRejected(
		    suites::TransportSuite( *bad::Transport( fault.fault, badDevice, posix ), badInput ),
		    fault.clause, std::string( "bad transport: " ) + fault.name );
	}
}

void DisplayChecks()
{
	CheckVerdict(
	    suites::DisplaySuite( *fixture::CreateHeadlessSession() ), "display fixture-headless" );
	CheckVerdict( suites::DisplaySuite( *product::CreateUserDisplaySession() ), "display user" );
	CheckVerdict(
	    suites::DisplaySuite( *product::CreateHeadlessDisplaySession() ), "display none" );
	{
		const fs::path config = fs::temp_directory_path() / "kilntest-session.conf";
		fixture::WriteBytes( config, "<busconfig/>\n" );
		auto privateSession =
		    product::CreatePrivateDisplaySession( { "/nonexistent/session.conf", config } );
		CheckVerdict( suites::DisplaySuite( *privateSession ), "display private" );
		product::DisplayRequest request;
		request.scratch = fs::temp_directory_path() / "kilntest-private";
		auto opened = privateSession->Open( request );
		Check( opened && opened.Value().commandPrefix.size() > 4 &&
		           opened.Value().commandPrefix[0] == "dbus-run-session" &&
		           fixture::ReadBytes(
		               request.scratch / "services" / "org.freedesktop.portal.Documents.service" )
		                   .find( "Exec=/bin/false" ) != std::string::npos &&
		           fixture::ReadBytes( request.scratch / "session.conf" ).find( config.string() ) !=
		               std::string::npos,
		    "display private: a private bus that blocks the document portal, then mutter" );
		auto missing = product::CreatePrivateDisplaySession( { "/nonexistent/session.conf" } )
		                   ->Open( request );
		Check( !missing && missing.Error().code == product::kUnavailable,
		    "display private: no bus configuration is unavailable" );
	}
	const struct
	{
		bad::DisplayFault fault;
		const char *clause;
		const char *name;
	} faults[] = { { bad::DisplayFault::kLeaksUserDisplay, "V3", "hands out the user's display" },
	    { bad::DisplayFault::kMutatesCaller, "V4", "changes the caller's environment" },
	    { bad::DisplayFault::kStaysOpen, "V5", "stays open after Close" },
	    { bad::DisplayFault::kIgnoresCancel, "V1", "ignores cancellation" } };
	for ( const auto &fault : faults )
		CheckRejected( suites::DisplaySuite( *bad::Display( fault.fault ) ), fault.clause,
		    std::string( "bad display session: " ) + fault.name );
}

void CatalogChecks( platform::IToolProcessProvider &posix )
{
	product::ProviderCatalog catalog;
	Check( catalog.Add( fixture::CreateCompileStage() ).HasValue(), "catalog.add" );
	Check( !catalog.Add( fixture::CreateCompileStage() ), "catalog.duplicate-name-refused" );
	Check( !catalog.Add( std::unique_ptr<product::IPackager>() ), "catalog.null-refused" );
	auto missing = catalog.Stage( "nope" );
	Check( !missing && missing.Error().contract == "stage" && missing.Error().name == "nope",
	    "catalog.unknown-name-refused-by-name" );
	Check( catalog.Names().stages == std::set<std::string>{ "fixture-compile" }, "catalog.names" );
	(void)posix;
}

//-----------------------------------------------------------------------------
// The fixture platform end to end through kiln.core (RFC 0027 "The gate").

std::string FixtureProfile( const std::string &version, const std::string &capabilities )
{
	return R"({"schema": "source-product-profile/v2", "id": "fixture-v2",
	  "description": "The fixture platform: unittests/kilntest/fixture_platform",
	  "aliases": ["fixture"],
	  "target": {"product": "fixture-app", "os": "fixture", "architecture": "any"},
	  "toolchain": {"family": "host", "version": ")" +
	       version + R"(", "cxx": "c++"},
	  "build": {"toolchain": "fixture-host", "flavors": {"dev": {"description": "the only flavor"}}},
	  "pipeline": {"stages": ["fixture-compile", "fixture-content", "fixture-symbols"]},
	  "package": {"form": "fixture-dir"},
	  "deploy": {"transport": "fixture-device", "content_root": "content", "capabilities": )" +
	       capabilities + R"(},
	  "launch": {"display_session": "fixture-headless", "arguments": ["--hello"]}})";
}

void FixturePlatformChecks( const Workbench &bench, platform::IToolProcessProvider &posix )
{
	const std::string version = HostCompilerVersion( posix, "c++" );
	const fs::path profiles = bench.root / "profiles";
	fixture::WriteBytes( profiles / "fixture.json",
	    FixtureProfile( version, R"(["install", "content-sync", "launch"])" ) );
	fixture::WriteBytes( profiles / "fixture-crash.json",
	    FixtureProfile( version, R"(["install", "content-sync", "launch", "crash-collect"])" ) );
	fixture::FakeDevice device;
	product::ProviderCatalog catalog = fixture::ComposeFixtureCatalog( device, posix );
	jobsystem::DeterministicExecutor executor;
	NullSink sink;
	kiln::SessionConfig config;
	config.sourceRoot = bench.source;
	config.profileRoot = profiles;
	config.outRoot = bench.root / "out";
	config.dependencyRoot = bench.dependencies;
	config.hostTag = "fixture-any";
	config.workspaceText =
	    R"({"devices": {"fake": {"transport": "fixture-device", "content_root": "content"}}})";
	kiln::Session session( catalog, posix, executor, sink, config );

	kiln::PipelineRequest request;
	request.profile = "fixture";
	request.upTo = product::StageRole::kRun;
	request.device = "fake";
	request.arguments = { "--exit=0" };
	auto first = session.Run( request );
	Check( first.HasValue(), "fixture.end-to-end runs build, content, package, deploy and run" );
	if ( !first )
	{
		std::cerr << "  " << first.Error().Describe() << '\n';
		return;
	}
	std::vector<std::string> order;
	for ( const auto &stage : first.Value().stages )
		order.push_back( stage.name + ":" + stage.role );
	const std::vector<std::string> expected = { "fixture-compile:engine", "fixture-symbols:engine",
	    "fixture-content:content", "package:package", "deploy:deploy", "run:run" };
	Check( order == expected, "fixture.stages-ordered-by-role-and-artifact" );
	if ( order != expected )
	{
		for ( const auto &name : order )
			std::cerr << "  stage " << name << '\n';
	}
	Check( first.Value().launch && first.Value().launch->exitCode == 0,
	    "fixture.program-ran-on-the-device" );
	if ( first.Value().launch )
	{
		const auto &log = first.Value().launch->log;
		Check( std::find( log.begin(), log.end(), "arg --hello" ) != log.end() &&
		           std::find( log.begin(), log.end(), "arg --exit=0" ) != log.end(),
		    "fixture.launch-passes-profile-and-request-arguments" );
		Check( std::find( log.begin(), log.end(), "display headless" ) != log.end(),
		    "fixture.display-session-environment" );
	}
	Check(
	    device.files.count( "texture.rgb332" ) == 1 && device.installs == 1 && device.launches == 1,
	    "fixture.content-synced-and-installed" );
	Check( first.Value().package && first.Value().package->entries.size() >= 4,
	    "fixture.package-manifest" );
	{
		// A caller-chosen runtime (a test's private one) gets the same package.
		kiln::PipelineRequest privateRuntime = request;
		privateRuntime.upTo = product::StageRole::kPackage;
		privateRuntime.runtime = bench.root / "private-runtime";
		auto packaged = session.Run( privateRuntime );
		const auto artifact = packaged ? std::find_if( packaged.Value().artifacts.begin(),
		                                     packaged.Value().artifacts.end(),
		                                     []( const product::Artifact &item )
		                                     {
			                                     return item.name == "platform-package";
		                                     } )
		                               : decltype( packaged.Value().artifacts.end() ){};
		Check(
		    packaged && packaged.Value().package && artifact != packaged.Value().artifacts.end() &&
		        artifact->path == bench.root / "private-runtime" &&
		        fs::is_directory( bench.root / "private-runtime" ) &&
		        packaged.Value().package->entries.size() == first.Value().package->entries.size(),
		    "fixture.package-into-a-chosen-runtime" );
	}
	Check( fs::exists( first.Value().evidenceFile ), "fixture.evidence-written" );
	const Value &evidence = first.Value().evidence;
	Check( evidence.FindString( "revision" ) && evidence.FindString( "dirty_digest" ) &&
	           evidence.Find( "toolchain" ) && evidence.FindString( "reproduction" ),
	    "fixture.evidence-records-revision-toolchain-and-reproduction" );
	Check( !fs::exists( first.Value().tree / ".kiln-staging" / "fixture-compile" ),
	    "fixture.staging-cleared" );

	const std::uint64_t writesAfterFirst = device.writes;
	auto second = session.Run( request );
	Check( second.HasValue(), "fixture.second-run" );
	if ( second )
	{
		bool built = true;
		for ( const auto &stage : second.Value().stages )
		{
			if ( stage.role == "engine" || stage.role == "content" )
				built &= stage.upToDate;
		}
		Check( built, "fixture.unchanged-second-run-rebuilds-nothing" );
		Check( device.writes == writesAfterFirst && device.installs == 1 && device.launches == 2,
		    "fixture.unchanged-second-run-resends-nothing" );
	}

	// A missing required capability is refused by name before the device is touched.
	{
		fixture::FakeDevice untouched;
		product::ProviderCatalog other = fixture::ComposeFixtureCatalog( untouched, posix );
		kiln::Session crashSession( other, posix, executor, sink, config );
		kiln::PipelineRequest crash = request;
		crash.profile = "fixture-crash";
		auto refused = crashSession.Run( crash );
		Check( !refused && refused.Error().code == "capability" &&
		           refused.Error().detail.find( "crash-collect" ) != std::string::npos &&
		           untouched.writes == 0 && untouched.installs == 0 && untouched.launches == 0,
		    "core.missing-capability-refused-before-touching-the-device" );
	}
	// Up to `engine`, nothing is packaged or deployed (`kiln build`).
	{
		kiln::PipelineRequest build = request;
		build.upTo = product::StageRole::kEngine;
		build.device.reset();
		auto built = session.Run( build );
		Check( built && built.Value().stages.size() == 2 && !built.Value().package,
		    "core.build-stops-at-engine" );
	}
	// Deploy needs a device from the workspace.
	{
		kiln::PipelineRequest noDevice = request;
		noDevice.device.reset();
		auto refused = session.Run( noDevice );
		Check( !refused && refused.Error().code == "device", "core.deploy-without-device-refused" );
	}
	// A stage in the graph that reads an undeclared artifact fails the run and publishes nothing.
	{
		fixture::FakeDevice other;
		product::ProviderCatalog badCatalog = fixture::ComposeFixtureCatalog( other, posix );
		(void)badCatalog.Add( bad::Stage( bad::StageFault::kReadsUndeclared ) );
		fixture::WriteBytes( profiles / "fixture-bad.json",
		    R"({"schema": "source-product-profile/v2", "id": "b", "description": "b",
		      "toolchain": {"family": "host", "version": ")" +
		        version + R"(", "cxx": "c++"},
		      "build": {"toolchain": "fixture-host", "flavors": {"dev": {"description": "d"}}},
		      "pipeline": {"stages": ["fixture-compile", "bad-stage"]}})" );
		kiln::Session badSession( badCatalog, posix, executor, sink, config );
		kiln::PipelineRequest badRequest;
		badRequest.profile = "fixture-bad";
		badRequest.upTo = product::StageRole::kContent;
		auto failed = badSession.Run( badRequest );
		Check( !failed && failed.Error().code == "stage" &&
		           failed.Error().detail.find( "undeclared" ) != std::string::npos &&
		           !fs::exists(
		               bench.root / "out" / "fixture-bad" / "dev" / "artifacts" / "bad-output" ),
		    "core.undeclared-read-fails-the-run-and-publishes-nothing" );
	}
	// An unknown provider fails composition by name; a fragment is not buildable.
	{
		fixture::WriteBytes( profiles / "fixture-unknown.json",
		    R"({"schema": "source-product-profile/v2", "id": "u", "description": "u",
		      "toolchain": {"family": "host", "version": "1", "cxx": "c++"},
		      "build": {"toolchain": "fixture-host", "flavors": {"dev": {"description": "d"}}},
		      "pipeline": {"stages": ["shader-cache"]}})" );
		kiln::PipelineRequest unknown;
		unknown.profile = "fixture-unknown";
		auto refused = session.Run( unknown );
		Check( !refused && refused.Error().detail.find( "unknown-provider" ) != std::string::npos &&
		           refused.Error().detail.find( "shader-cache" ) != std::string::npos,
		    "core.unknown-provider-refused-by-name" );
		fixture::WriteBytes( profiles / "frag.json",
		    R"({"schema": "source-product-fragment/v2", "id": "f", "description": "f"})" );
		kiln::PipelineRequest fragment;
		fragment.profile = "frag";
		Check( !session.Run( fragment ), "core.fragment-not-buildable" );
	}
	// Cancellation before the run publishes nothing new.
	{
		product::CancellationFlag cancelled;
		cancelled.Cancel();
		kiln::PipelineRequest stop = request;
		stop.cancel = &cancelled;
		const auto launches = device.launches;
		Check( !session.Run( stop ) && device.launches == launches,
		    "core.cancellation-stops-the-run" );
	}
	// The workspace may not change a build fact.
	{
		kiln::SessionConfig badConfig = config;
		badConfig.workspaceText =
		    R"({"profiles": {"fixture": {"build": {"toolchain": "linux-gcc"}}}})";
		kiln::Session badSession( catalog, posix, executor, sink, badConfig );
		auto refused = badSession.Run( request );
		Check(
		    !refused && refused.Error().detail.find( "workspace-build-fact" ) != std::string::npos,
		    "core.workspace-build-fact-refused" );
	}
}

//-----------------------------------------------------------------------------
// The launch plan (RFC 0027 L1): templates over variables, switches, workspace.

kiln::PlayRequest Play( const std::string &profile )
{
	kiln::PlayRequest request;
	request.profile = profile;
	return request;
}

std::vector<std::string> PlanArgv(
    kiln::Session &session, kiln::PlayRequest request, kiln::Error *error = nullptr )
{
	auto plan = session.PlanLaunch( request );
	if ( !plan )
	{
		if ( error )
			*error = plan.Error();
		return { "<refused>" };
	}
	return plan.Value().argv;
}

void LaunchPlanChecks( const Workbench &bench, platform::IToolProcessProvider &posix )
{
	const fs::path profiles = bench.root / "launch-profiles";
	fixture::WriteBytes( profiles / "game.json",
	    R"({"schema": "source-product-profile/v2", "id": "g", "description": "g",
	  "aliases": ["game"],
	  "launch": {"executable": "run_me", "game": "demo", "default_map": "start",
	    "map_arguments": ["+map", "{map}"],
	    "variables": {"width": ["800"], "height": ["600"], "windowed": ["-windowed"], "producer": ["baked"],
	                  "core": ["+core", "1"], "assets": ["{root}/assets"]},
	    "arguments": ["-game", "{game}", "-w", "{width}", "-h", "{height}", "{windowed}", "{switches}", "{core}",
	                  "+producer", "{producer}", "-assets", "{assets}", "{map_arguments}", "{args}"],
	    "environment": {"LD_LIBRARY_PATH": "{runtime}/bin:{inherit}", "GAME": "{game}"},
	    "switches": {
	      "validate": {"description": "v", "arguments": ["-validate"]},
	      "probe": {"description": "p", "arguments": ["-probe", "x"]},
	      "sdf": {"description": "s", "set": {"producer": ["sdf"]}},
	      "no-core": {"description": "n", "arguments": ["-nocore"], "set": {"core": []}, "conflicts": ["core"]},
	      "core": {"description": "c", "set": {"core": ["+core", "1"]}, "conflicts": ["no-core"]}}}})" );
	fixture::WriteBytes( profiles / "bad-set.json",
	    R"({"schema": "source-product-profile/v2", "id": "b", "description": "b",
	  "launch": {"executable": "x", "arguments": [], "variables": {},
	    "switches": {"s": {"description": "s", "set": {"undeclared": ["1"]}}}}})" );
	fixture::WriteBytes( profiles / "bad-var.json",
	    R"({"schema": "source-product-profile/v2", "id": "c", "description": "c",
	  "launch": {"executable": "x", "arguments": ["{nowhere}"]}})" );
	fixture::FakeDevice device;
	product::ProviderCatalog catalog = fixture::ComposeFixtureCatalog( device, posix );
	jobsystem::DeterministicExecutor executor;
	NullSink sink;
	kiln::SessionConfig config;
	config.sourceRoot = bench.source;
	config.profileRoot = profiles;
	config.outRoot = bench.root / "out";
	config.hostTag = "fixture-any";
	kiln::Session session( catalog, posix, executor, sink, config );

	kiln::PlayRequest request;
	request.profile = "game";
	const std::string root = bench.source.string();
	const std::vector<std::string> plain = { "./run_me", "-game", "demo", "-w", "800", "-h", "600",
	    "-windowed", "+core", "1", "+producer", "baked", "-assets", root + "/assets", "+map",
	    "start" };
	Check( PlanArgv( session, request ) == plain, "launch.template-splices-variables" );
	request.switches = { "probe", "validate", "sdf", "no-core" };
	request.map = "other";
	request.arguments = { "-console", "+x 1" };
	const std::vector<std::string> switched = { "./run_me", "-game", "demo", "-w", "800", "-h",
	    "600", "-windowed", "-probe", "x", "-validate", "-nocore", "+producer", "sdf", "-assets",
	    root + "/assets", "+map", "other", "-console", "+x 1" };
	Check( PlanArgv( session, request ) == switched,
	    "launch.switches-append-in-order-and-set-variables" );
	request.switches = { "validate", "probe" };
	auto reordered = PlanArgv( session, request );
	Check( std::find( reordered.begin(), reordered.end(), "-validate" ) <
	           std::find( reordered.begin(), reordered.end(), "-probe" ),
	    "launch.switch-order-is-the-request-order" );
	kiln::Error error;
	request.switches = { "core", "no-core" };
	Check( PlanArgv( session, request, &error ).front() == "<refused>" && error.code == "switch" &&
	           error.detail.find( "conflicts" ) != std::string::npos,
	    "launch.conflicting-switches-refused" );
	request.switches = { "nope" };
	Check( PlanArgv( session, request, &error ).front() == "<refused>" &&
	           error.detail.find( "nope" ) != std::string::npos,
	    "launch.unknown-switch-refused-by-name" );
	request.switches = { "validate", "validate" };
	Check( PlanArgv( session, request, &error ).front() == "<refused>",
	    "launch.repeated-switch-refused" );

	auto plan = session.PlanLaunch( Play( "game" ) );
	Check( plan && plan.Value().environment.size() == 2 &&
	           plan.Value().environment[0].value ==
	               ( bench.root / "out" / "game" / "runtime" ).string() + "/bin:{inherit}" &&
	           plan.Value().environment[1].value == std::string( "demo" ) &&
	           plan.Value().workingDirectory == bench.root / "out" / "game" / "runtime",
	    "launch.environment-and-working-directory" );
	kiln::PlayRequest chosen = Play( "game" );
	chosen.runtime = bench.root / "elsewhere";
	auto moved = session.PlanLaunch( chosen );
	Check( moved && moved.Value().runtime == bench.root / "elsewhere" &&
	           moved.Value().workingDirectory == bench.root / "elsewhere" &&
	           moved.Value().environment[0].value ==
	               ( bench.root / "elsewhere" ).string() + "/bin:{inherit}",
	    "launch.chosen-runtime-is-the-working-directory-and-{runtime}" );
	kiln::PlayRequest exact = Play( "game" );
	exact.exactArguments = std::vector<std::string>{ "-test", "+map", "x" };
	exact.environment = { { "HOME", std::string( "/sandbox" ) }, { "GONE", std::nullopt } };
	exact.wrapper = { "capture", "--" };
	auto exactPlan = session.PlanLaunch( exact );
	Check( exactPlan &&
	           exactPlan.Value().argv ==
	               std::vector<std::string>{ "capture", "--", "./run_me", "-test", "+map", "x" } &&
	           exactPlan.Value().environment.size() == 4 &&
	           exactPlan.Value().environment[2].name == "HOME" &&
	           !exactPlan.Value().environment[3].value,
	    "launch.exact-arguments-wrapper-and-environment-follow-the-profile" );
	exact.switches = { "validate" };
	auto exactSwitches = session.PlanLaunch( exact );
	Check( !exactSwitches && exactSwitches.Error().code == "request",
	    "launch.exact-arguments-with-switches-refused" );

	kiln::SessionConfig personal = config;
	personal.workspaceText =
	    R"({"resolution": [2560, 1440], "windowed": false, "default_map": "mine",
	  "profiles": {"game": {"launch": {"variables": {"producer": ["probe"]}}}}})";
	kiln::Session workspace( catalog, posix, executor, sink, personal );
	const std::vector<std::string> bound = { "./run_me", "-game", "demo", "-w", "2560", "-h",
	    "1440", "+core", "1", "+producer", "probe", "-assets", root + "/assets", "+map", "mine" };
	Check( PlanArgv( workspace, Play( "game" ) ) == bound,
	    "launch.workspace-binds-resolution-windowed-map-and-variables" );

	auto badSet =
	    kiln::Session( catalog, posix, executor, sink, config ).PlanLaunch( Play( "bad-set" ) );
	Check( !badSet && badSet.Error().detail.find( "undeclared" ) != std::string::npos,
	    "launch.switch-setting-an-undeclared-variable-refused" );
	auto badVar = session.PlanLaunch( Play( "bad-var" ) );
	Check( !badVar && badVar.Error().detail.find( "nowhere" ) != std::string::npos,
	    "launch.unknown-template-variable-refused" );
	auto switches = session.Switches( "game" );
	Check( switches && switches.Value().size() == 5, "launch.switches-listed" );
}

void PackagerUnitChecks( const Workbench &bench, platform::IToolProcessProvider &posix )
{
	Check( product::GlobMatch( "bin/*.so", "bin/libengine.so" ) &&
	           !product::GlobMatch( "bin/*.so", "bin/sub/x.so" ) &&
	           product::GlobMatch( "**", "a/b/c" ) &&
	           product::GlobMatch( "*/bin/*.so", "portal/bin/libclient.so" ) &&
	           !product::GlobMatch( "hl2_launcher", "bin/hl2_launcher" ) &&
	           product::GlobMatch( "a/**/z", "a/b/c/z" ),
	    "package.glob-segments" );
	// A mount set must be declared by the profile.
	const fs::path profiles = bench.root / "mount-profiles";
	fixture::WriteBytes( profiles / "fixture.json",
	    R"({"schema": "source-product-profile/v2", "id": "m", "description": "m",
	      "toolchain": {"family": "host", "version": ")" +
	        HostCompilerVersion( posix, "c++" ) + R"(", "cxx": "c++"},
	      "build": {"toolchain": "fixture-host", "flavors": {"dev": {"description": "d"}}},
	      "pipeline": {"stages": ["fixture-compile"]},
	      "content": {"mount_sets": {"extra": {"description": "e"}}}})" );
	fixture::FakeDevice device;
	product::ProviderCatalog catalog = fixture::ComposeFixtureCatalog( device, posix );
	jobsystem::DeterministicExecutor executor;
	NullSink sink;
	kiln::SessionConfig config;
	config.sourceRoot = bench.source;
	config.profileRoot = profiles;
	config.outRoot = bench.root / "mount-out";
	config.hostTag = "fixture-any";
	kiln::Session session( catalog, posix, executor, sink, config );
	kiln::PipelineRequest request;
	request.profile = "fixture";
	request.mountSets = { "nope" };
	auto refused = session.Run( request );
	Check( !refused && refused.Error().detail.find( "nope" ) != std::string::npos,
	    "package.undeclared-mount-set-refused" );
	request.mountSets = { "extra" };
	Check( session.Run( request ).HasValue(), "package.declared-mount-set-accepted" );

	// linux-dir: a mount set's entries leave the runtime when a later package
	// runs without the set.
	{
		const fs::path store = bench.root / "set-store";
		fs::create_directories( store / "pack" );
		fixture::WriteBytes( store / "pack" / "a.txt", "a" );
		auto parsed = foundation::json::Parse(
		    R"({"package": {"steps": [{"op": "link", "path": "game/extra", "from": "store",
		      "source": "pack", "mount_set": "extra"}]}})" );
		product::ResolvedProfile profile;
		profile.name = "set-fixture";
		profile.document = parsed.Value();
		auto packager = product::CreateLinuxDirPackager();
		product::PackageRequest package;
		package.profile = &profile;
		package.output = bench.root / "set-runtime";
		package.locations = { { "store", store } };
		package.sourceRoot = bench.source;
		package.mountSets = { "extra" };
		auto with = packager->Package( package );
		const bool linked = fs::is_symlink( package.output / "game/extra" );
		package.mountSets = {};
		auto without = packager->Package( package );
		Check( with && without && linked && !fs::exists( package.output / "game/extra" ) &&
		           !fs::exists( package.output / "game" ) &&
		           !fs::is_symlink( package.output / "game/extra" ) &&
		           without.Value().entries.empty(),
		    "package.linux-dir-unselected-mount-set-entries-removed" );
	}
}
} // namespace

int main()
{
	const fs::path root =
	    fs::temp_directory_path() /
	    ( "kilntest-" +
	        std::to_string( std::chrono::steady_clock::now().time_since_epoch().count() ) );
	std::error_code ec;
	fs::remove_all( root, ec );
	Workbench bench{
	    root, root / "source", root / "dependencies", root / "tree", root / "scratch" };
	fs::create_directories( bench.source );
	fs::create_directories( bench.dependencies );
	fixture::WriteBytes( bench.source / "wscript", "# fixture source root\n" );

	std::unique_ptr<platform::IToolProcessProvider> posix =
	    platform::CreatePosixToolProcessProvider();

	JsonChecks();
	ProfileChecks();
	ToolchainChecks( bench, *posix );
	RecipeChecks( bench );
	const std::string version = HostCompilerVersion( *posix, "c++" );
	const auto profile =
	    MakeProfile( ToolchainProfile( "fixture-host", "host", "c++", "cc", version ) );
	auto toolchain = fixture::CreateHostToolchain( *posix )->Prepare(
	    { &profile, bench.source, bench.dependencies, nullptr } );
	Check( toolchain.HasValue(), "fixture toolchain prepares" );
	if ( toolchain )
		StageChecks( bench, *posix, profile, toolchain.Value() );
	PackagerChecks( bench, profile );
	TransportChecks( bench, *posix );
	DisplayChecks();
	CatalogChecks( *posix );
	{
		auto spawner = platform::CreatePosixProcessSpawner();
		CheckVerdict( suites::RunSuite( *product::CreateSingleRunProvider(), *spawner,
		                  bench.scratch / "run-single" ),
		    "run single" );
		CheckVerdict( suites::RunSuite( *product::CreateExternalInstallRunProvider(), *spawner,
		                  bench.scratch / "run-external" ),
		    "run external-install" );
		CheckRejected( suites::RunSuite( *bad::RunProvider( bad::RunFault::kReturnsBeforeExit ),
		                   *spawner, bench.scratch / "run-bad1" ),
		    "N2", "bad run provider: returns before its program ends" );
		CheckRejected( suites::RunSuite( *bad::RunProvider( bad::RunFault::kIgnoresCancel ),
		                   *spawner, bench.scratch / "run-bad2" ),
		    "N4", "bad run provider: ignores cancellation" );
		CheckRejected( suites::RunSuite( *bad::RunProvider( bad::RunFault::kDropsLog ), *spawner,
		                   bench.scratch / "run-bad3" ),
		    "N6", "bad run provider: drops the launch's log" );
		CheckRejected(
		    suites::RunSuite( *bad::RunProvider( bad::RunFault::kLaunchOverridesDisplay ), *spawner,
		        bench.scratch / "run-bad4" ),
		    "N7", "bad run provider: lets the launch override the display session" );
		CheckRejected( suites::RunSuite( *bad::RunProvider( bad::RunFault::kSilentStart ), *spawner,
		                   bench.scratch / "run-bad5" ),
		    "N8", "bad run provider: does not report its programs' starts" );
		std::string error;
		auto missing = spawner->Spawn( { { "/nonexistent/program" }, "/", {}, {} }, error );
		Check( missing.id < 0 && error.find( "/nonexistent/program" ) != std::string::npos,
		    "spawn: a missing program fails by name" );
	}
	FixturePlatformChecks( bench, *posix );
	LaunchPlanChecks( bench, *posix );
	PackagerUnitChecks( bench, *posix );
	Check( posix->LiveProcessCount() == 0, "no process outlives its request" );

	fs::remove_all( root, ec );
	return testing::ReportConformance( g_Checks, g_Failures );
}
