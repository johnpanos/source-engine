//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shared product contract suites (RFC 0027 L0).
//
//=============================================================================//

#include "suites.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <thread>
#include <utility>
#include <vector>

namespace suites
{

namespace fs = std::filesystem;
using foundation::json::Value;
using fixture::ReadBytes;
using fixture::WriteBytes;

bool Verdict::Violates( const std::string &clause ) const
{
	return std::any_of( violations.begin(), violations.end(),
	    [&]( const std::string &v )
	    {
		    return v.rfind( clause + " ", 0 ) == 0 || v == clause;
	    } );
}

void Verdict::Fail( std::string clause, std::string detail )
{
	violations.push_back( detail.empty() ? clause : clause + " " + detail );
}

platform::ToolProcessResult RecordingProcesses::Run( const platform::ToolProcessRequest &request )
{
	started.push_back( request.argv );
	return m_Inner.Run( request );
}

namespace
{

// Every file under a directory with its size and modification time.
std::map<std::string, std::string> Snapshot( const fs::path &root )
{
	std::map<std::string, std::string> files;
	std::error_code ec;
	for ( auto it = fs::recursive_directory_iterator( root, ec );
	    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
	{
		std::error_code inner;
		const std::string name = fs::relative( it->path(), root, inner ).generic_string();
		if ( it->is_regular_file( inner ) )
			files[name] = std::to_string( it->file_size( inner ) ) + "@" +
			              std::to_string( it->last_write_time( inner ).time_since_epoch().count() );
		else
			files[name] = "dir";
	}
	return files;
}

bool IsNetworkFetch( const std::vector<std::string> &argv )
{
	if ( argv.empty() )
		return false;
	const std::string tool = fs::path( argv[0] ).filename().string();
	return tool == "curl" || tool == "wget" || tool == "aria2c" ||
	       ( tool == "git" && argv.size() > 1 && ( argv[1] == "clone" || argv[1] == "fetch" ) );
}

std::string Environment( const product::ToolchainEnvironment &environment, const std::string &name )
{
	for ( const auto &entry : environment.environment )
	{
		if ( entry.name == name && entry.value )
			return *entry.value;
	}
	return {};
}

} // namespace

//-----------------------------------------------------------------------------
// ITargetToolchain
//-----------------------------------------------------------------------------

Verdict ToolchainSuite( product::ITargetToolchain &toolchain, RecordingProcesses &processes,
    const ToolchainCase &input )
{
	Verdict verdict;
	const auto sourceBefore = Snapshot( input.sourceRoot );
	const auto dependenciesBefore = Snapshot( input.dependencyRoot );
	processes.started.clear();
	product::ToolchainRequest request{
	    input.valid, input.sourceRoot, input.dependencyRoot, nullptr };
	auto first = toolchain.Prepare( request );
	if ( !first )
	{
		verdict.Fail(
		    "T1 prepares a valid profile", first.Error().code + ": " + first.Error().detail );
		return verdict;
	}
	const product::Identity &identity = first.Value().identity;
	if ( identity.provider != toolchain.Name() )
		verdict.Fail( "T2 identity names the provider" );
	const bool hasVersion = std::any_of( identity.facts.begin(), identity.facts.end(),
	    []( const auto &fact )
	    {
		    return fact.first.find( "version" ) != std::string::npos && !fact.second.empty();
	    } );
	if ( !hasVersion || !identity.facts.count( "sdk" ) || identity.facts.at( "sdk" ).empty() ||
	     !identity.facts.count( "target" ) )
		verdict.Fail( "T3 identity covers the compiler version, target and SDK" );
	auto second = toolchain.Prepare( request );
	if ( !second || second.Value().identity.Digest() != identity.Digest() ||
	     second.Value().environment != first.Value().environment )
		verdict.Fail( "T4 prepare is idempotent" );
	if ( Snapshot( input.sourceRoot ) != sourceBefore )
		verdict.Fail( "T5 writes nothing into the source tree" );
	for ( const auto &entry : Snapshot( input.dependencyRoot ) )
	{
		const bool own = entry.first.rfind( std::string( toolchain.Name() ) + "/", 0 ) == 0 ||
		                 entry.first == toolchain.Name();
		if ( !own && !dependenciesBefore.count( entry.first ) )
			verdict.Fail( "T5 writes only under its dependency directory", entry.first );
	}
	for ( const auto &argv : processes.started )
	{
		if ( IsNetworkFetch( argv ) )
			verdict.Fail( "T6 downloads only pinned, verified archives", argv[0] );
	}
	const std::string cxx = Environment( first.Value(), "CXX" );
	platform::ToolProcessRequest query;
	query.argv = !first.Value().compilerProbe.empty()
	                 ? first.Value().compilerProbe
	                 : std::vector<std::string>{
	                       cxx.empty() ? std::string( "/nonexistent" ) : cxx, "--version" };
	query.workingDirectory = input.sourceRoot.string();
	query.executionTimeout = std::chrono::seconds( 30 );
	query.cancellationTimeout = std::chrono::seconds( 1 );
	if ( cxx.empty() || !processes.Run( query ).Succeeded() )
		verdict.Fail( "T7 success implies a working compiler", cxx );
	product::CancellationFlag cancelled;
	cancelled.Cancel();
	auto cancelledRun =
	    toolchain.Prepare( { input.valid, input.sourceRoot, input.dependencyRoot, &cancelled } );
	if ( cancelledRun || cancelledRun.Error().code != product::kCancelled )
		verdict.Fail( "T8 honours cancellation" );
	if ( input.wrongPin )
	{
		auto mismatch = toolchain.Prepare(
		    { input.wrongPin, input.sourceRoot, input.dependencyRoot, nullptr } );
		if ( mismatch )
			verdict.Fail( "T9 verifies the profile's pin before use" );
	}
	product::DoctorReport report;
	toolchain.CheckHost( input.valid, report );
	if ( report.items.empty() )
		verdict.Fail( "T10 CheckHost reports its prerequisites" );
	return verdict;
}

//-----------------------------------------------------------------------------
// IRecipeBuilder
//-----------------------------------------------------------------------------

Verdict RecipeSuite( product::IRecipeBuilder &builder, const RecipeCase &input )
{
	Verdict verdict;
	product::Recipe recipe{ "zlib", "1.3.1", Value::Object() };
	product::RecipeRequest request{ recipe, &input.toolchainA, input.prefixRoot, nullptr };
	auto built = builder.Build( request );
	if ( !built )
	{
		verdict.Fail( "R1 builds a recipe", built.Error().detail );
		return verdict;
	}
	const std::string key = product::RecipeKey( recipe, input.toolchainA.identity );
	if ( built.Value().key != key ||
	     built.Value().prefix.filename().string().find( key ) == std::string::npos ||
	     !fs::exists( built.Value().prefix ) )
		verdict.Fail( "R2 the prefix is keyed by recipe, pin and toolchain" );
	auto again = builder.Build( request );
	if ( !again || again.Value().built || again.Value().prefix != built.Value().prefix )
		verdict.Fail( "R3 an existing key is reused without work" );
	auto other = builder.Build( { recipe, &input.toolchainB, input.prefixRoot, nullptr } );
	if ( !other || other.Value().prefix == built.Value().prefix || !other.Value().built )
		verdict.Fail( "R4 a different toolchain identity never reuses a prefix" );
	const auto before = Snapshot( input.prefixRoot );
	product::Recipe failing{ "broken", "1.0", Value::Object() };
	failing.definition.Set( "fail", Value::Bool( true ) );
	auto failed = builder.Build( { failing, &input.toolchainA, input.prefixRoot, nullptr } );
	if ( failed )
		verdict.Fail( "R5 a failing recipe fails" );
	if ( Snapshot( input.prefixRoot ) != before )
		verdict.Fail( "R6 a failure publishes nothing and keeps the previous prefixes" );
	product::CancellationFlag cancelled;
	cancelled.Cancel();
	product::Recipe fresh{ "fresh", "2.0", Value::Object() };
	auto cancelledRun = builder.Build( { fresh, &input.toolchainA, input.prefixRoot, &cancelled } );
	if ( cancelledRun || Snapshot( input.prefixRoot ) != before )
		verdict.Fail( "R7 cancellation publishes nothing" );
	return verdict;
}

//-----------------------------------------------------------------------------
// IProductStage
//-----------------------------------------------------------------------------

Verdict StageSuite( product::IProductStage &stage, const StageCase &input )
{
	Verdict verdict;
	const product::StageDescriptor descriptor = stage.Describe( *input.profile );
	if ( descriptor.produces.empty() && descriptor.role != product::StageRole::kRun &&
	     descriptor.role != product::StageRole::kTest )
		verdict.Fail( "S1 declares what it produces" );
	std::set<std::string> produced( descriptor.produces.begin(), descriptor.produces.end() );
	if ( produced.size() != descriptor.produces.size() )
		verdict.Fail( "S1 declares each artifact once" );
	const fs::path staging = input.treeRoot / ".suite-staging" / std::string( stage.Name() );
	std::error_code ec;

	// Cancellation publishes nothing.
	{
		fs::remove_all( staging, ec );
		fs::create_directories( staging, ec );
		product::CancellationFlag cancelled;
		cancelled.Cancel();
		product::StageInputs inputs( *input.profile, "dev", input.sourceRoot, input.treeRoot,
		    descriptor.consumes, input.available, input.toolchain, input.processes, nullptr,
		    &cancelled );
		product::StageOutputs outputs( staging );
		auto ran = stage.Run( inputs, outputs );
		if ( ran || ran.Error().code != product::kCancelled || !outputs.Published().empty() )
			verdict.Fail( "S5 cancellation publishes nothing" );
	}
	if ( !input.runSuccessPath )
		return verdict;

	fs::remove_all( staging, ec );
	fs::create_directories( staging, ec );
	product::StageInputs inputs( *input.profile, "dev", input.sourceRoot, input.treeRoot,
	    descriptor.consumes, input.available, input.toolchain, input.processes, nullptr, nullptr );
	product::StageOutputs outputs( staging );
	auto ran = stage.Run( inputs, outputs );
	if ( !ran )
	{
		verdict.Fail(
		    "S2 runs with its declared inputs", ran.Error().code + ": " + ran.Error().detail );
		return verdict;
	}
	if ( !inputs.UndeclaredReads().empty() )
		verdict.Fail( "S3 reads only declared artifacts", inputs.UndeclaredReads().front() );
	std::set<std::string> published;
	for ( const product::Artifact &artifact : outputs.Published() )
	{
		if ( !produced.count( artifact.name ) )
			verdict.Fail( "S4 produces only declared artifacts", artifact.name );
		if ( !published.insert( artifact.name ).second )
			verdict.Fail( "S4 publishes each artifact once", artifact.name );
		if ( artifact.digest.empty() || !fs::exists( artifact.path, ec ) )
			verdict.Fail( "S4 a published artifact exists and is hashed", artifact.name );
	}
	for ( const std::string &name : descriptor.produces )
	{
		if ( !published.count( name ) )
			verdict.Fail( "S6 publishes every declared output (no partial output)", name );
	}
	fs::remove_all( staging, ec );
	return verdict;
}

//-----------------------------------------------------------------------------
// IPackager
//-----------------------------------------------------------------------------

Verdict PackagerSuite( product::IPackager &packager, const PackagerCase &input )
{
	Verdict verdict;
	const fs::path sources = input.scratch / "sources";
	std::error_code ec;
	fs::remove_all( input.scratch, ec );
	WriteBytes( sources / "bin" / "app", "program" );
	WriteBytes( sources / "bin" / "libengine.so", "engine module" );
	WriteBytes( sources / "content" / "maps" / "a.bsp", "map" );
	product::PackageRequest request;
	request.profile = input.profile;
	request.inputs = { { sources / "bin" / "app", "bin/app", "executable" },
	    { sources / "bin" / "libengine.so", "bin/libengine.so", "module" },
	    { sources / "content" / "maps" / "a.bsp", "content/maps/a.bsp", "content" } };
	request.output = input.scratch / "package";
	request.credentials = { { "signing_key", "SECRET-KEY-7f3a" } };
	auto manifest = packager.Package( request );
	if ( !manifest )
	{
		verdict.Fail( "P1 packages its inputs", manifest.Error().detail );
		return verdict;
	}
	std::map<std::string, product::ManifestEntry> listed;
	for ( const auto &entry : manifest.Value().entries )
		listed[entry.path] = entry;
	for ( const auto &in : request.inputs )
	{
		auto it = listed.find( in.packagePath );
		if ( it == listed.end() || it->second.role != in.role ||
		     it->second.hash != product::HashHex( ReadBytes( in.source ) ) )
			verdict.Fail( "P1 the manifest lists every declared input with its hash and role",
			    in.packagePath );
	}
	if ( listed.size() != request.inputs.size() )
		verdict.Fail( "P2 the manifest lists nothing undeclared" );
	std::set<std::string> files;
	for ( const auto &entry : Snapshot( request.output ) )
	{
		if ( entry.second != "dir" )
			files.insert( entry.first );
	}
	std::set<std::string> expected;
	for ( const auto &in : request.inputs )
		expected.insert( in.packagePath );
	if ( files != expected )
		verdict.Fail( "P2 the package holds exactly the manifest's files" );
	for ( const auto &file : files )
	{
		if ( ReadBytes( request.output / file ).find( "SECRET-KEY-7f3a" ) != std::string::npos )
			verdict.Fail( "P4 no credential in the package", file );
	}
	if ( manifest.Value().ToJson().Write().find( "SECRET-KEY-7f3a" ) != std::string::npos )
		verdict.Fail( "P4 no credential in the manifest" );
	auto again = packager.Package( request );
	if ( !again || again.Value().Digest() != manifest.Value().Digest() )
		verdict.Fail( "P3 the same inputs give the same manifest" );
	const auto before = Snapshot( input.scratch );
	product::PackageRequest broken = request;
	broken.inputs.push_back( { sources / "missing", "bin/missing", "module" } );
	if ( packager.Package( broken ) )
		verdict.Fail( "P5 a missing input fails" );
	if ( Snapshot( input.scratch ) != before )
		verdict.Fail( "P5 a failure leaves no partial package and keeps the previous one" );
	return verdict;
}

//-----------------------------------------------------------------------------
// IDeployTransport
//-----------------------------------------------------------------------------

Verdict TransportSuite( product::IDeployTransport &transport, const TransportCase &input )
{
	Verdict verdict;
	fixture::FakeDevice &device = *input.device;
	device = fixture::FakeDevice{};
	const std::vector<std::string> claimed = {
	    "install", "content-sync", "launch", "device-facts" };
	std::vector<std::string> present;
	for ( const std::string &capability : claimed )
	{
		if ( product::TransportHasCapability( transport, capability ) )
			present.push_back( capability );
	}
	if ( !product::MissingCapabilities( transport, present ).empty() )
		verdict.Fail( "D1 capabilities are reported consistently" );
	product::DeviceAddress address{
	    "suite-device", "local", device.root, { { "password", "HUNTER2-device" } } };
	std::error_code ec;
	fs::remove_all( input.scratch, ec );
	std::vector<product::SyncEntry> entries;
	for ( const char *name : { "a.txt", "b.txt", "dir/c.txt" } )
	{
		const fs::path source = input.scratch / name;
		WriteBytes( source, std::string( "bytes of " ) + name );
		entries.push_back( { name, product::HashHex( ReadBytes( source ) ), source } );
	}
	if ( product::IContentSync *sync = transport.ContentSync() )
	{
		auto first = sync->Sync( address, entries, nullptr );
		if ( !first || first.Value().transferred != entries.size() ||
		     device.files.size() != entries.size() )
			verdict.Fail( "D2 a sync transfers every new entry" );
		for ( const auto &entry : entries )
		{
			auto it = device.files.find( entry.path );
			if ( it == device.files.end() || product::HashHex( it->second ) != entry.hash )
				verdict.Fail( "D2 a sync leaves the wanted bytes", entry.path );
		}
		const std::uint64_t writes = device.writes;
		auto second = sync->Sync( address, entries, nullptr );
		if ( !second || second.Value().transferred != 0 || device.writes != writes )
			verdict.Fail( "D3 unchanged entries are not resent" );
		WriteBytes( entries[0].source, "changed" );
		entries[0].hash = product::HashHex( "changed" );
		entries.pop_back(); // dir/c.txt goes away
		auto third = sync->Sync( address, entries, nullptr );
		if ( !third || third.Value().transferred != 1 || device.files.count( "dir/c.txt" ) ||
		     device.files["a.txt"] != "changed" )
			verdict.Fail( "D4 exactly the changed entries transfer and removed ones go" );
		auto escape = entries;
		WriteBytes( input.scratch / "evil", "evil" );
		escape.push_back(
		    { "../outside.txt", product::HashHex( "evil" ), input.scratch / "evil" } );
		const auto files = device.files;
		auto escaped = sync->Sync( address, escape, nullptr );
		if ( escaped || !device.outside.empty() || device.files != files )
			verdict.Fail( "D5 nothing is written or deleted outside the content root" );
		product::CancellationFlag cancelled;
		cancelled.Cancel();
		auto changed = entries;
		WriteBytes( changed[0].source, "changed again" );
		changed[0].hash = product::HashHex( "changed again" );
		auto stopped = sync->Sync( address, changed, &cancelled );
		if ( stopped || stopped.Error().code != product::kCancelled || device.files != files )
			verdict.Fail( "D6 cancellation keeps the previous state" );
		auto partial = entries;
		WriteBytes( input.scratch / "late", "late" );
		partial.push_back(
		    { "late.txt", product::HashHex( "different" ), input.scratch / "late" } );
		auto mismatched = sync->Sync( address, partial, nullptr );
		if ( mismatched || device.files != files )
			verdict.Fail( "D7 an interrupted or failed sync leaves the previous content (no "
			              "success after a partial transfer)" );
		device.reachable = false;
		auto unreachable = sync->Sync( address, entries, nullptr );
		if ( unreachable || unreachable.Error().code != product::kUnavailable )
			verdict.Fail( "D8 an unreachable device is unavailable, never a pass" );
		device.reachable = true;
	}
	if ( product::IInstall *install = transport.Install() )
	{
		product::PackageManifest manifest;
		manifest.form = "suite";
		manifest.entries.push_back( { "fixture_app", "0", "executable", 1 } );
		auto installed = install->Install( address, manifest, input.package, nullptr );
		if ( !installed || device.installs != 1 || device.installedPackage != input.package )
			verdict.Fail( "D9 a claimed install has an observable effect" );
		auto repeated = install->Install( address, manifest, input.package, nullptr );
		if ( !repeated || device.installs != 1 )
			verdict.Fail( "D10 install is idempotent" );
	}
	if ( product::ILaunch *launch = transport.Launch() )
	{
		product::LaunchRequest request;
		request.arguments = { "--exit=3" };
		auto launched = launch->Launch( address, request );
		if ( !launched || launched.Value().exitCode != 3 || device.launches != 1 )
			verdict.Fail( "D9 a claimed launch has an observable effect" );
		else
		{
			for ( const std::string &line : launched.Value().log )
			{
				if ( line.find( "HUNTER2-device" ) != std::string::npos )
					verdict.Fail( "D11 no credential reaches a log" );
			}
		}
		for ( const std::string &line : device.log )
		{
			if ( line.find( "HUNTER2-device" ) != std::string::npos )
				verdict.Fail( "D11 no credential reaches a log" );
		}
	}
	if ( product::IDeviceFacts *facts = transport.DeviceFacts() )
	{
		if ( !facts->Facts( address ) )
			verdict.Fail( "D12 a reachable device reports its facts" );
	}
	return verdict;
}

//-----------------------------------------------------------------------------
// IDisplaySession
//-----------------------------------------------------------------------------

Verdict DisplaySuite( product::IDisplaySession &session )
{
	Verdict verdict;
	// The caller's (the user's) display; an isolated session must not hand it out.
	setenv( "WAYLAND_DISPLAY", "suite-user-wayland-0", 1 );
	setenv( "DISPLAY", ":suite-user", 1 );
	product::CancellationFlag cancelled;
	cancelled.Cancel();
	product::DisplayRequest cancelledRequest;
	cancelledRequest.cancel = &cancelled;
	auto refused = session.Open( cancelledRequest );
	if ( refused || refused.Error().code != product::kCancelled || session.IsOpen() )
		verdict.Fail( "V1 cancellation opens nothing" );
	product::DisplayRequest request;
	request.scratch = std::filesystem::temp_directory_path() / "kilntest-display";
	auto opened = session.Open( request );
	if ( !opened )
	{
		verdict.Fail( "V2 opens", opened.Error().detail );
		return verdict;
	}
	if ( !session.IsOpen() )
		verdict.Fail( "V2 reports itself open" );
	if ( session.ClaimsIsolation() )
	{
		if ( !opened.Value().isolated )
			verdict.Fail( "V3 an isolating session returns an isolated environment" );
		for ( const auto &entry : opened.Value().environment )
		{
			if ( entry.value &&
			     ( *entry.value == "suite-user-wayland-0" || *entry.value == ":suite-user" ) )
				verdict.Fail(
				    "V3 an isolating session never hands out the user's display", entry.name );
		}
		bool clearsWayland = false, clearsX = false;
		for ( const auto &entry : opened.Value().environment )
		{
			clearsWayland |= entry.name == "WAYLAND_DISPLAY";
			clearsX |= entry.name == "DISPLAY";
		}
		if ( !clearsWayland || !clearsX )
			verdict.Fail( "V3 an isolating session overrides the inherited display variables" );
	}
	const char *wayland = std::getenv( "WAYLAND_DISPLAY" );
	const char *x = std::getenv( "DISPLAY" );
	if ( !wayland || std::string( wayland ) != "suite-user-wayland-0" || !x ||
	     std::string( x ) != ":suite-user" )
		verdict.Fail( "V4 the caller's environment is unchanged" );
	session.Close();
	session.Close();
	if ( session.IsOpen() )
		verdict.Fail( "V5 Close ends the session and is idempotent" );
	return verdict;
}

//-----------------------------------------------------------------------------
// IRunProvider (one launch)
//-----------------------------------------------------------------------------

Verdict RunSuite(
    product::IRunProvider &provider, platform::IProcessSpawner &spawner, const fs::path &scratch )
{
	Verdict verdict;
	std::error_code ec;
	fs::remove_all( scratch, ec );
	fs::create_directories( scratch, ec );
	const auto launch = [&]( const std::string &script )
	{
		product::RunRequest request;
		product::LaunchSpec spec;
		spec.name = "game";
		spec.argv = { "/bin/sh", "-c", script };
		spec.workingDirectory = scratch;
		spec.environment = { { "KILN_SUITE_VALUE", std::string( "from-launch" ) } };
		request.launches.push_back( spec );
		request.spawner = &spawner;
		return request;
	};
	auto status = provider.Run( launch( "exit 7" ) );
	if ( !status || status.Value() != 7 )
		verdict.Fail( "N1 the run's exit status is the program's" );
	auto waited = provider.Run( launch( "sleep 0.3; echo done > waited.txt" ) );
	if ( !waited || !fs::exists( scratch / "waited.txt", ec ) )
		verdict.Fail( "N2 the run waits for its program (no process is left running)" );
	product::RunRequest wrapped =
	    launch( "echo \"$KILN_SUITE_VALUE $KILN_SUITE_DISPLAY\" > environment.txt" );
	wrapped.display.commandPrefix = { "/usr/bin/env", "KILN_SUITE_DISPLAY=from-display" };
	auto prefixed = provider.Run( wrapped );
	if ( !prefixed ||
	     fixture::ReadBytes( scratch / "environment.txt" ) != "from-launch from-display\n" )
		verdict.Fail(
		    "N3 the display's prefix wraps the launch and the launch's environment applies" );
	product::CancellationFlag cancel;
	product::RunRequest slow = launch( "sleep 30" );
	slow.cancel = &cancel;
	std::thread canceller(
	    [&]
	    {
		    std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
		    cancel.Cancel();
	    } );
	const auto started = std::chrono::steady_clock::now();
	(void)provider.Run( slow );
	canceller.join();
	if ( std::chrono::steady_clock::now() - started > std::chrono::seconds( 10 ) )
		verdict.Fail( "N4 cancellation stops the run's programs" );
	product::RunRequest none;
	none.spawner = &spawner;
	if ( provider.Run( none ) )
		verdict.Fail( "N5 a run with no launch is refused" );
	product::RunRequest logged = launch( "echo to-stdout; echo to-stderr >&2" );
	logged.launches[0].outputFile = scratch / "game.log";
	auto loggedStatus = provider.Run( logged );
	if ( !loggedStatus || fixture::ReadBytes( scratch / "game.log" ) != "to-stdout\nto-stderr\n" )
		verdict.Fail( "N6 the program's output and errors go to the launch's log" );
	product::RunRequest owned = launch( "echo \"$KILN_SUITE_DRIVER\" > driver.txt" );
	owned.launches[0].environment.push_back( { "KILN_SUITE_DRIVER", std::string( "player" ) } );
	owned.display.environment = { { "KILN_SUITE_DRIVER", std::string( "offscreen" ) } };
	auto ownedStatus = provider.Run( owned );
	if ( !ownedStatus || fixture::ReadBytes( scratch / "driver.txt" ) != "offscreen\n" )
		verdict.Fail( "N7 the display session's environment wins over the launch's" );
	product::RunRequest reported = launch( "exit 0" );
	std::vector<std::pair<std::string, std::int64_t>> starts;
	reported.started = [&]( const std::string &name, platform::SpawnedProcess process )
	{
		starts.emplace_back( name, process.id );
	};
	auto reportedStatus = provider.Run( reported );
	if ( !reportedStatus || starts.size() != 1 || starts[0].first != "game" ||
	     starts[0].second <= 0 )
		verdict.Fail( "N8 each program's start is reported with its process" );
	return verdict;
}

} // namespace suites
