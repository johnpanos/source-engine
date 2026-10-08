//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: kiln.core: sessions, the stage graph runner and evidence
//			(RFC 0027 L0). Names no platform; see public/kiln/api.h.
//
//=============================================================================//

#include "kiln/api.h"

#include "jobsystem/graph_executor.h"
#include "jobsystem/job_graph.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sstream>

namespace kiln
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;
using product::Artifact;
using product::StageRole;

Error Fail( std::string code, std::string detail )
{
	return Error{ std::move( code ), std::move( detail ) };
}

bool WriteFileAtomic( const fs::path &path, const std::string &text )
{
	std::error_code ec;
	fs::create_directories( path.parent_path(), ec );
	const fs::path staging = path.string() + ".staging";
	{
		std::ofstream stream( staging, std::ios::binary | std::ios::trunc );
		if ( !stream.write( text.data(), static_cast<std::streamsize>( text.size() ) ) )
			return false;
	}
	fs::rename( staging, path, ec );
	return !ec;
}

int RoleRank( StageRole role )
{
	return static_cast<int>( role );
}

// One stage of a run: a catalog provider or one of the contract steps below.
struct Step
{
	product::IProductStage *stage = nullptr;
	product::StageDescriptor descriptor;
	StageRole effectiveRole = StageRole::kExtra;
};

// The shared state of one Run, borrowed by the contract steps.
struct RunState
{
	const product::ResolvedProfile *profile = nullptr;
	std::string flavor;
	fs::path tree;
	std::map<std::string, Artifact> artifacts;
	std::optional<product::DeviceAddress> device;
	product::IDeployTransport *transport = nullptr;
	std::vector<std::string> requiredCapabilities;
	product::IPackager *packager = nullptr;
	product::IDisplaySession *display = nullptr;
	std::vector<std::string> extraArguments;
	std::optional<product::PackageManifest> manifest;
	std::optional<product::LaunchResult> launch;
	std::map<std::string, fs::path> locations;
	std::vector<std::string> mountSets;
	fs::path sourceRoot;
};

std::vector<std::string> StringList( const Value *value )
{
	std::vector<std::string> list;
	if ( value && value->IsArray() )
	{
		for ( const Value &item : value->Items() )
		{
			if ( item.IsString() )
				list.push_back( item.Text() );
		}
	}
	return list;
}

// Every file under an artifact directory as a package input.
void AddFiles(
    const Artifact &artifact, const std::string &role, std::vector<product::PackageInput> &inputs )
{
	std::error_code ec;
	if ( fs::is_regular_file( artifact.path, ec ) )
	{
		inputs.push_back( { artifact.path, artifact.path.filename().generic_string(), role } );
		return;
	}
	for ( auto it = fs::recursive_directory_iterator( artifact.path, ec );
	    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
	{
		std::error_code inner;
		if ( it->is_regular_file( inner ) )
			inputs.push_back( { it->path(),
			    fs::relative( it->path(), artifact.path, inner ).generic_string(), role } );
	}
}

// The package role: the profile's package.form packager over every install
// and directory artifact of the earlier stages.
class PackageStep final : public product::IProductStage
{
public:
	PackageStep( RunState &state, std::vector<std::string> consumes )
	    : m_State( state ), m_Consumes( std::move( consumes ) )
	{
	}
	std::string_view Name() const noexcept override { return "package"; }
	product::StageDescriptor Describe( const product::ResolvedProfile & ) const override
	{
		return {
		    StageRole::kPackage, m_Consumes, { "platform-package" }, product::Determinism::kExact };
	}
	foundation::Expected<product::StageResult, product::ProviderError> Run(
	    product::StageInputs &inputs, product::StageOutputs &outputs ) override
	{
		product::PackageRequest request;
		request.profile = m_State.profile;
		request.cancel = inputs.Cancel();
		const Value *packageSection = m_State.profile->document.Find( "package" );
		const std::string *directory =
		    packageSection ? packageSection->FindString( "directory" ) : nullptr;
		request.output = m_State.tree / ( directory ? *directory : std::string( "package" ) );
		request.locations = m_State.locations;
		request.mountSets = m_State.mountSets;
		request.sourceRoot = m_State.sourceRoot;
		for ( const std::string &name : m_Consumes )
		{
			if ( const Artifact *artifact = inputs.Get( name ) )
			{
				AddFiles( *artifact, artifact->type == "install" ? "install" : "content",
				    request.inputs );
				request.artifacts[name] = *artifact;
			}
		}
		std::sort( request.inputs.begin(), request.inputs.end(),
		    []( const auto &a, const auto &b )
		    {
			    return a.packagePath < b.packagePath;
		    } );
		auto manifest = m_State.packager->Package( request );
		if ( !manifest )
			return foundation::MakeUnexpected( manifest.Error() );
		Artifact package;
		package.name = "platform-package";
		package.type = "package";
		package.path = request.output;
		package.digest = manifest.Value().Digest();
		outputs.Publish( std::move( package ) );
		product::StageResult result;
		result.workItems = manifest.Value().entries.size();
		result.summary = std::to_string( manifest.Value().entries.size() ) + " files, form " +
		                 manifest.Value().form;
		result.evidence.Set( "manifest", manifest.Value().ToJson() );
		m_State.manifest = std::move( manifest ).Value();
		return result;
	}

private:
	RunState &m_State;
	std::vector<std::string> m_Consumes;
};

// The deploy role: install the package and sync its content through the
// device's transport.
class DeployStep final : public product::IProductStage
{
public:
	explicit DeployStep( RunState &state ) : m_State( state ) {}
	std::string_view Name() const noexcept override { return "deploy"; }
	product::StageDescriptor Describe( const product::ResolvedProfile & ) const override
	{
		return { StageRole::kDeploy, { "platform-package" }, { "deployed-install" },
		    product::Determinism::kExact };
	}
	foundation::Expected<product::StageResult, product::ProviderError> Run(
	    product::StageInputs &inputs, product::StageOutputs &outputs ) override
	{
		const Artifact *package = inputs.Get( "platform-package" );
		if ( !package || !m_State.manifest )
			return foundation::MakeUnexpected(
			    product::ProviderError{ "invalid-request", "no package" } );
		product::StageResult result;
		if ( product::IInstall *install = m_State.transport->Install() )
		{
			auto installed = install->Install(
			    *m_State.device, *m_State.manifest, package->path, inputs.Cancel() );
			if ( !installed )
				return foundation::MakeUnexpected( installed.Error() );
			++result.workItems;
		}
		if ( product::IContentSync *sync = m_State.transport->ContentSync() )
		{
			std::vector<product::SyncEntry> entries;
			for ( const product::ManifestEntry &entry : m_State.manifest->entries )
			{
				if ( entry.role == "content" )
					entries.push_back( { entry.path, entry.hash, package->path / entry.path } );
			}
			auto synced = sync->Sync( *m_State.device, entries, inputs.Cancel() );
			if ( !synced )
				return foundation::MakeUnexpected( synced.Error() );
			result.workItems += synced.Value().transferred + synced.Value().removed;
			result.evidence.Set( "transferred",
			    Value::Number( static_cast<long long>( synced.Value().transferred ) ) );
			result.evidence.Set(
			    "unchanged", Value::Number( static_cast<long long>( synced.Value().unchanged ) ) );
		}
		Artifact deployed;
		deployed.name = "deployed-install";
		deployed.type = "deployed-install";
		deployed.path = m_State.device->contentRoot;
		deployed.digest = package->digest;
		deployed.facts.Set( "device", Value::String( m_State.device->name ) );
		outputs.Publish( std::move( deployed ) );
		result.upToDate = result.workItems == 0;
		result.summary = "deployed to " + m_State.device->name;
		return result;
	}

private:
	RunState &m_State;
};

// The run role: open the profile's display session and launch on the device.
class RunStep final : public product::IProductStage
{
public:
	explicit RunStep( RunState &state ) : m_State( state ) {}
	std::string_view Name() const noexcept override { return "run"; }
	product::StageDescriptor Describe( const product::ResolvedProfile & ) const override
	{
		return { StageRole::kRun, { "deployed-install" }, {}, product::Determinism::kStatistical };
	}
	foundation::Expected<product::StageResult, product::ProviderError> Run(
	    product::StageInputs &inputs, product::StageOutputs & ) override
	{
		if ( !inputs.Get( "deployed-install" ) )
			return foundation::MakeUnexpected(
			    product::ProviderError{ "invalid-request", "nothing deployed" } );
		product::LaunchRequest request;
		request.cancel = inputs.Cancel();
		const Value *launch = m_State.profile->document.Find( "launch" );
		request.arguments = StringList( launch ? launch->Find( "arguments" ) : nullptr );
		request.arguments.insert(
		    request.arguments.end(), m_State.extraArguments.begin(), m_State.extraArguments.end() );
		if ( m_State.display )
		{
			product::DisplayRequest displayRequest;
			displayRequest.scratch = m_State.tree / "display-session";
			displayRequest.cancel = inputs.Cancel();
			auto display = m_State.display->Open( displayRequest );
			if ( !display )
				return foundation::MakeUnexpected( display.Error() );
			request.environment = display.Value().environment;
		}
		if ( const Value *environment = launch ? launch->Find( "environment" ) : nullptr )
		{
			for ( const auto &member : environment->Members() )
			{
				if ( member.second.IsString() )
					request.environment.push_back( { member.first, member.second.Text() } );
			}
		}
		auto launched = m_State.transport->Launch()->Launch( *m_State.device, request );
		if ( m_State.display )
			m_State.display->Close();
		if ( !launched )
			return foundation::MakeUnexpected( launched.Error() );
		product::StageResult result;
		result.workItems = 1;
		result.summary = "exit " + std::to_string( launched.Value().exitCode );
		result.evidence.Set( "exit_code", Value::Number( launched.Value().exitCode ) );
		result.evidence.Set(
		    "log_lines", Value::Number( static_cast<long long>( launched.Value().log.size() ) ) );
		m_State.launch = std::move( launched ).Value();
		return result;
	}

private:
	RunState &m_State;
};

std::string Trim( std::string text )
{
	while ( !text.empty() && ( text.back() == '\n' || text.back() == '\r' ) )
		text.pop_back();
	return text;
}

} // namespace

bool PipelineResult::UpToDate() const
{
	return std::all_of( stages.begin(), stages.end(),
	    []( const StageRecord &stage )
	    {
		    return stage.upToDate;
	    } );
}

Session::Session( const product::ProviderCatalog &catalog,
    platform::IToolProcessProvider &processes, jobsystem::IGraphExecutor &executor,
    product::IDiagnosticSink &diagnostics, SessionConfig config )
    : m_Catalog( catalog ), m_Processes( processes ), m_Executor( executor ),
      m_Diagnostics( diagnostics ), m_Config( std::move( config ) ),
      m_Source( m_Config.profileRoot )
{
}

foundation::Expected<std::string, Error> Session::Locate( const std::string &nameOrAlias ) const
{
	auto found = product::FindProfile( m_Source, nameOrAlias, m_Config.hostTag );
	if ( !found )
		return foundation::MakeUnexpected( Fail( "profile", found.Error().Describe() ) );
	return found.Value();
}

foundation::Expected<product::Workspace, Error> Session::LoadWorkspace() const
{
	if ( !m_Config.workspaceText )
		return product::Workspace{ std::nullopt, std::nullopt, Value::Object() };
	auto workspace = product::ParseWorkspace( *m_Config.workspaceText, m_Config.workspaceFile );
	if ( !workspace )
		return foundation::MakeUnexpected( Fail( "workspace", workspace.Error().Describe() ) );
	return workspace.Value();
}

std::vector<ProfileSummary> Session::ListProfiles() const
{
	std::vector<ProfileSummary> list;
	const product::ProviderNames names = m_Catalog.Names();
	for ( const std::string &file : m_Source.List() )
	{
		ProfileSummary summary;
		summary.file = file;
		auto resolved = product::Resolve( m_Source, file, names );
		if ( resolved )
		{
			summary.name = resolved.Value().name;
			summary.schema = resolved.Value().schema;
			summary.aliases = resolved.Value().aliases;
			summary.buildable = resolved.Value().Buildable();
			summary.fragment = resolved.Value().fragment;
		}
		else
		{
			summary.name = file;
			summary.error = resolved.Error().Describe();
		}
		list.push_back( std::move( summary ) );
	}
	return list;
}

foundation::Expected<product::ResolvedProfile, Error> Session::ResolveProfile(
    const std::string &nameOrAlias ) const
{
	auto file = Locate( nameOrAlias );
	if ( !file )
		return foundation::MakeUnexpected( file.Error() );
	auto resolved = product::Resolve( m_Source, file.Value(), m_Catalog.Names() );
	if ( !resolved )
		return foundation::MakeUnexpected( Fail( "profile", resolved.Error().Describe() ) );
	return resolved.Value();
}

foundation::Expected<product::ResolvedProfile, Error> Session::ResolveForLaunch(
    const std::string &nameOrAlias ) const
{
	auto resolved = ResolveProfile( nameOrAlias );
	if ( !resolved )
		return resolved;
	auto workspace = LoadWorkspace();
	if ( !workspace )
		return foundation::MakeUnexpected( workspace.Error() );
	auto applied = product::ApplyWorkspace( resolved.Value(), workspace.Value() );
	if ( !applied )
		return foundation::MakeUnexpected( Fail( "workspace", applied.Error().Describe() ) );
	return applied.Value();
}

foundation::Expected<DoctorResult, Error> Session::Doctor( const std::string &nameOrAlias )
{
	auto resolved = ResolveForLaunch( nameOrAlias );
	if ( !resolved )
		return foundation::MakeUnexpected( resolved.Error() );
	const product::ResolvedProfile &profile = resolved.Value();
	DoctorResult result;
	result.profile = profile.name;
	product::DoctorReport &report = result.report;
	report.Add( "profile " + profile.name, profile.Buildable(),
	    profile.Buildable() ? profile.schema : "not buildable (" + profile.schema + ")" );
	if ( !profile.toolchain.empty() )
	{
		auto toolchain = m_Catalog.Toolchain( profile.toolchain );
		if ( toolchain )
			toolchain.Value()->CheckHost( &profile, report );
		else
			report.Add( "toolchain " + profile.toolchain, false, toolchain.Error().Describe() );
	}
	for ( const std::string &stage : profile.stages )
		report.Add( "stage " + stage, m_Catalog.Stage( stage ).HasValue(), "catalog provider" );
	report.Add( "source root", fs::exists( m_Config.sourceRoot / "wscript" ),
	    m_Config.sourceRoot.string() );
	auto workspace = LoadWorkspace();
	report.Add( "workspace " + m_Config.workspaceFile, workspace.HasValue(),
	    !m_Config.workspaceText ? "absent (defaults apply)"
	    : workspace             ? "valid"
	                            : workspace.Error().detail );
	return result;
}

foundation::Expected<std::map<std::string, fs::path>, Error> Session::ResolveLocations(
    const product::ResolvedProfile &profile ) const
{
	std::map<std::string, fs::path> locations;
	auto workspace = LoadWorkspace();
	if ( !workspace )
		return foundation::MakeUnexpected( workspace.Error() );
	const Value *personal = workspace.Value().document.Find( "content_locations" );
	const Value *content = profile.document.Find( "content" );
	const Value *locators = content ? content->Find( "locators" ) : nullptr;
	if ( !locators )
		return locations;
	for ( const auto &member : locators->Members() )
	{
		std::string path;
		if ( const std::string *chosen = personal ? personal->FindString( member.first ) : nullptr )
			path = *chosen;
		else if ( const std::string *fallback = member.second.FindString( "default" ) )
			path = *fallback;
		for ( const auto &[token, value] :
		    { std::pair<std::string, std::string>{ "{root}", m_Config.sourceRoot.string() },
		        std::pair<std::string, std::string>{ "{home}", m_Config.homeDirectory.string() } } )
		{
			for ( size_t at = path.find( token ); at != std::string::npos;
			    at = path.find( token, at + value.size() ) )
				path.replace( at, token.size(), value );
		}
		std::error_code ec;
		// A writable location (a cache a stage fills) may not exist yet.
		const Value *create = member.second.Find( "create" );
		const bool writable = create && create->IsBool() && create->AsBool();
		if ( !path.empty() && ( writable || fs::exists( path, ec ) ) )
			locations[member.first] = path;
	}
	return locations;
}

foundation::Expected<PipelineResult, Error> Session::Run( const PipelineRequest &request )
{
	auto resolved = ResolveForLaunch( request.profile );
	if ( !resolved )
		return foundation::MakeUnexpected( resolved.Error() );
	const product::ResolvedProfile &profile = resolved.Value();
	if ( !profile.Buildable() )
		return foundation::MakeUnexpected( Fail(
		    "profile", profile.file + " is " +
		                   ( profile.fragment ? std::string( "a fragment" ) : profile.schema ) +
		                   "; kiln builds " + std::string( product::kProfileSchemaV2 ) +
		                   " profiles with build.toolchain" ) );
	const std::string flavor = request.flavor.value_or( profile.defaultFlavor );
	if ( !profile.FindFlavor( flavor ) )
		return foundation::MakeUnexpected(
		    Fail( "profile", "\"" + flavor + "\" is not a flavor of " + profile.name ) );

	RunState state;
	state.profile = &profile;
	state.flavor = flavor;
	state.tree = m_Config.outRoot / profile.TreeName( flavor );
	state.extraArguments = request.arguments;
	state.sourceRoot = m_Config.sourceRoot;
	// Selected mount sets must be declared (content.mount_sets).
	{
		const Value *content = profile.document.Find( "content" );
		const Value *sets = content ? content->Find( "mount_sets" ) : nullptr;
		for ( const std::string &name : request.mountSets )
		{
			if ( !sets || !sets->Find( name ) )
				return foundation::MakeUnexpected(
				    Fail( "profile", "\"" + name + "\" is not a mount set of " + profile.name +
				                         " (content.mount_sets)" ) );
		}
		state.mountSets = request.mountSets;
	}

	// Resolve every provider the request needs before touching anything.
	auto toolchain = m_Catalog.Toolchain( profile.toolchain );
	if ( !toolchain )
		return foundation::MakeUnexpected( Fail( "catalog", toolchain.Error().Describe() ) );
	std::vector<Step> steps;
	for ( const std::string &name : profile.stages )
	{
		auto stage = m_Catalog.Stage( name );
		if ( !stage )
			return foundation::MakeUnexpected( Fail( "catalog", stage.Error().Describe() ) );
		Step step;
		step.stage = stage.Value();
		step.descriptor = step.stage->Describe( profile );
		step.effectiveRole = step.descriptor.role;
		steps.push_back( step );
	}
	// An extra stage runs with the latest role among its inputs' producers.
	for ( Step &step : steps )
	{
		if ( step.descriptor.role != StageRole::kExtra )
			continue;
		StageRole role = StageRole::kEngine;
		for ( const std::string &input : step.descriptor.consumes )
		{
			for ( const Step &producer : steps )
			{
				const auto &produced = producer.descriptor.produces;
				if ( producer.descriptor.role != StageRole::kExtra &&
				     std::find( produced.begin(), produced.end(), input ) != produced.end() &&
				     RoleRank( producer.descriptor.role ) > RoleRank( role ) )
					role = producer.descriptor.role;
			}
		}
		step.effectiveRole = role;
	}
	auto locations = ResolveLocations( profile );
	if ( !locations )
		return foundation::MakeUnexpected( locations.Error() );
	state.locations = std::move( locations ).Value();
	std::vector<std::unique_ptr<product::IProductStage>> contractSteps;
	const Value *package = profile.document.Find( "package" );
	const std::string *form = package ? package->FindString( "form" ) : nullptr;
	if ( RoleRank( request.upTo ) >= RoleRank( StageRole::kPackage ) )
	{
		if ( !form )
			return foundation::MakeUnexpected(
			    Fail( "profile", profile.name + " declares no package.form" ) );
		auto packager = m_Catalog.Packager( *form );
		if ( !packager )
			return foundation::MakeUnexpected( Fail( "catalog", packager.Error().Describe() ) );
		state.packager = packager.Value();
		std::vector<std::string> consumes;
		for ( const Step &step : steps )
		{
			if ( RoleRank( step.effectiveRole ) <= RoleRank( request.upTo ) )
				consumes.insert( consumes.end(), step.descriptor.produces.begin(),
				    step.descriptor.produces.end() );
		}
		contractSteps.push_back( std::make_unique<PackageStep>( state, consumes ) );
	}
	if ( RoleRank( request.upTo ) >= RoleRank( StageRole::kDeploy ) )
	{
		if ( !request.device )
			return foundation::MakeUnexpected(
			    Fail( "device", "deploy and run need --device <name>" ) );
		auto workspace = LoadWorkspace();
		if ( !workspace )
			return foundation::MakeUnexpected( workspace.Error() );
		const Value *devices = workspace.Value().document.Find( "devices" );
		const Value *device = devices ? devices->Find( *request.device ) : nullptr;
		if ( !device || !device->IsObject() )
			return foundation::MakeUnexpected( Fail(
			    "device", "\"" + *request.device + "\" is not in " + m_Config.workspaceFile ) );
		const Value *deploy = profile.document.Find( "deploy" );
		const std::string *transportName = device->FindString( "transport" );
		if ( !transportName && deploy )
			transportName = deploy->FindString( "transport" );
		if ( !transportName )
			return foundation::MakeUnexpected(
			    Fail( "device", "no transport for " + *request.device ) );
		auto transport = m_Catalog.Transport( *transportName );
		if ( !transport )
			return foundation::MakeUnexpected( Fail( "catalog", transport.Error().Describe() ) );
		state.transport = transport.Value();
		state.requiredCapabilities =
		    StringList( deploy ? deploy->Find( "capabilities" ) : nullptr );
		if ( RoleRank( request.upTo ) >= RoleRank( StageRole::kRun ) &&
		     std::find( state.requiredCapabilities.begin(), state.requiredCapabilities.end(),
		         "launch" ) == state.requiredCapabilities.end() )
			state.requiredCapabilities.push_back( "launch" );
		const std::vector<std::string> missing =
		    product::MissingCapabilities( *state.transport, state.requiredCapabilities );
		if ( !missing.empty() )
		{
			std::string names;
			for ( const std::string &name : missing )
				names += ( names.empty() ? "" : ", " ) + name;
			return foundation::MakeUnexpected( Fail( "capability",
			    "transport \"" + *transportName + "\" lacks required capabilities: " + names +
			        " (nothing was touched)" ) );
		}
		product::DeviceAddress address;
		address.name = *request.device;
		if ( const std::string *text = device->FindString( "address" ) )
			address.address = *text;
		if ( const std::string *root = device->FindString( "content_root" ) )
			address.contentRoot = *root;
		else if ( const std::string *root =
		              deploy ? deploy->FindString( "content_root" ) : nullptr )
			address.contentRoot = *root;
		state.device = address;
		contractSteps.push_back( std::make_unique<DeployStep>( state ) );
	}
	if ( RoleRank( request.upTo ) >= RoleRank( StageRole::kRun ) )
	{
		const Value *launch = profile.document.Find( "launch" );
		if ( const std::string *display =
		         launch ? launch->FindString( "display_session" ) : nullptr )
		{
			auto session = m_Catalog.DisplaySession( *display );
			if ( !session )
				return foundation::MakeUnexpected( Fail( "catalog", session.Error().Describe() ) );
			state.display = session.Value();
		}
		contractSteps.push_back( std::make_unique<RunStep>( state ) );
	}
	for ( const auto &contractStep : contractSteps )
	{
		Step step;
		step.stage = contractStep.get();
		step.descriptor = step.stage->Describe( profile );
		step.effectiveRole = step.descriptor.role;
		steps.push_back( step );
	}
	steps.erase( std::remove_if( steps.begin(), steps.end(),
	                 [&]( const Step &step )
	                 {
		                 return RoleRank( step.effectiveRole ) > RoleRank( request.upTo );
	                 } ),
	    steps.end() );

	// Every consumed artifact has exactly one selected producer.
	std::map<std::string, size_t> producerOf;
	for ( size_t i = 0; i < steps.size(); ++i )
	{
		for ( const std::string &artifact : steps[i].descriptor.produces )
		{
			if ( !producerOf.emplace( artifact, i ).second )
				return foundation::MakeUnexpected(
				    Fail( "stage", "artifact \"" + artifact + "\" has two producers" ) );
		}
	}
	for ( const Step &step : steps )
	{
		for ( const std::string &artifact : step.descriptor.consumes )
		{
			if ( !producerOf.count( artifact ) )
				return foundation::MakeUnexpected( Fail( "stage",
				    std::string( step.stage->Name() ) + " consumes \"" + artifact +
				        "\", which no stage up to " +
				        std::string( product::StageRoleName( request.upTo ) ) + " produces" ) );
		}
	}

	auto prepared = toolchain.Value()->Prepare(
	    { &profile, m_Config.sourceRoot, m_Config.dependencyRoot, request.cancel } );
	if ( !prepared )
		return foundation::MakeUnexpected(
		    Fail( "toolchain", prepared.Error().code + ": " + prepared.Error().detail ) );
	const product::ToolchainEnvironment environment = std::move( prepared ).Value();

	// The stage graph: artifact edges, plus role order so a run is a
	// deterministic sequence of roles.
	PipelineResult result;
	result.profile = profile.name;
	result.flavor = flavor;
	result.tree = state.tree;
	std::vector<StageRecord> records( steps.size() );
	std::vector<size_t> executed; // stage indices in the order they ran
	std::optional<Error> failure;
	jobsystem::JobGraphBuilder builder;
	std::vector<jobsystem::JobHandle> handles;
	for ( size_t i = 0; i < steps.size(); ++i )
	{
		jobsystem::JobDesc desc;
		desc.name = "kiln.stage";
		desc.executor = jobsystem::Executor::BlockingIO();
		desc.function = [&, i]( jobsystem::JobRunContext &context )
		{
			Step &step = steps[i];
			StageRecord &record = records[i];
			record.name = std::string( step.stage->Name() );
			record.role = std::string( product::StageRoleName( step.effectiveRole ) );
			const fs::path staging = state.tree / ".kiln-staging" / record.name;
			std::error_code ec;
			fs::remove_all( staging, ec );
			fs::create_directories( staging, ec );
			product::StageInputs inputs( profile, flavor, m_Config.sourceRoot, state.tree,
			    step.descriptor.consumes, state.artifacts, &environment, &m_Processes,
			    &m_Diagnostics, request.cancel );
			inputs.SetLocations( state.locations );
			product::StageOutputs outputs( staging );
			auto ran = step.stage->Run( inputs, outputs );
			std::string problem;
			if ( !ran )
				problem = ran.Error().code + ": " + ran.Error().detail;
			else if ( !inputs.UndeclaredReads().empty() )
				problem = "read undeclared artifact \"" + inputs.UndeclaredReads().front() + "\"";
			else
			{
				std::set<std::string> published;
				for ( const Artifact &artifact : outputs.Published() )
				{
					const auto &produces = step.descriptor.produces;
					if ( std::find( produces.begin(), produces.end(), artifact.name ) ==
					     produces.end() )
						problem = "produced undeclared artifact \"" + artifact.name + "\"";
					published.insert( artifact.name );
				}
				for ( const std::string &name : step.descriptor.produces )
				{
					if ( problem.empty() && !published.count( name ) )
						problem = "did not publish declared artifact \"" + name + "\"";
				}
			}
			if ( problem.empty() && request.cancel && request.cancel->IsCancelled() )
				problem = std::string( product::kCancelled );
			if ( !problem.empty() )
			{
				fs::remove_all( staging, ec );
				failure = Fail( "stage", record.name + ": " + problem );
				context.Fail();
				return;
			}
			// Publish: staged artifacts move into the tree only now.
			for ( Artifact artifact : outputs.Published() )
			{
				const fs::path relative = fs::relative( artifact.path, staging, ec );
				if ( !ec && !relative.empty() && *relative.begin() != ".." )
				{
					const fs::path target = state.tree / "artifacts" / artifact.name;
					fs::remove_all( target, ec );
					fs::create_directories( target.parent_path(), ec );
					fs::rename( artifact.path, target, ec );
					if ( ec )
					{
						failure =
						    Fail( "stage", record.name + ": cannot publish " + artifact.name );
						context.Fail();
						return;
					}
					artifact.path = target;
				}
				state.artifacts[artifact.name] = artifact;
			}
			fs::remove_all( staging, ec );
			record.upToDate = ran.Value().upToDate;
			record.workItems = ran.Value().workItems;
			record.summary = ran.Value().summary;
			record.evidence = ran.Value().evidence;
			executed.push_back( i );
			m_Diagnostics.Report( product::Severity::kInfo, record.name, record.summary );
		};
		handles.push_back( builder.AddJob( desc ) );
	}
	for ( size_t i = 0; i < steps.size(); ++i )
	{
		for ( const std::string &artifact : steps[i].descriptor.consumes )
			builder.AddDependency( handles[producerOf[artifact]], handles[i] );
		for ( size_t j = 0; j < steps.size(); ++j )
		{
			if ( RoleRank( steps[j].effectiveRole ) < RoleRank( steps[i].effectiveRole ) )
				builder.AddDependency( handles[j], handles[i] );
		}
	}
	auto graph = builder.Seal();
	if ( !graph )
		return foundation::MakeUnexpected(
		    Fail( "stage", "the stage graph is invalid (a cycle among artifacts)" ) );
	jobsystem::RunOptions options;
	options.pumpMainThread = true;
	const jobsystem::RunResult run = m_Executor.Execute( graph.Value(), options );
	if ( failure )
		return foundation::MakeUnexpected( *failure );
	if ( !run.AllSucceeded() )
		return foundation::MakeUnexpected( Fail( "stage", "the stage graph did not complete" ) );

	for ( const size_t index : executed )
		result.stages.push_back( std::move( records[index] ) );
	for ( const auto &entry : state.artifacts )
		result.artifacts.push_back( entry.second );
	result.package = state.manifest;
	result.launch = state.launch;

	// Evidence (RFC 0005): revision, dirty digest, profile, toolchain, stages.
	const auto git = [&]( std::vector<std::string> argv ) -> std::string
	{
		platform::ToolProcessRequest query;
		query.argv = std::move( argv );
		query.workingDirectory = m_Config.sourceRoot.string();
		query.executionTimeout = std::chrono::seconds( 60 );
		query.cancellationTimeout = std::chrono::seconds( 2 );
		platform::ToolProcessResult answer = m_Processes.Run( query );
		return answer.Succeeded() ? answer.stdoutData : std::string( product::kUnavailable );
	};
	Value evidence = ToJson( result );
	evidence.Set( "schema", Value::String( "kiln-evidence/v1" ) );
	evidence.Set( "revision", Value::String( Trim( git( { "git", "rev-parse", "HEAD" } ) ) ) );
	evidence.Set( "dirty_digest",
	    Value::String( product::HashHex( git( { "git", "status", "--porcelain=v1", "-uno" } ) ) ) );
	evidence.Set( "profile_digest", Value::String( std::to_string( profile.Digest() ) ) );
	evidence.Set( "profile_files", Value::Array() );
	for ( const std::string &file : profile.files )
		evidence.Find( "profile_files" )->Push( Value::String( file ) );
	Value &identity = evidence.Set( "toolchain", Value::Object() );
	identity.Set( "provider", Value::String( environment.identity.provider ) );
	identity.Set( "digest", Value::String( environment.identity.Hex() ) );
	for ( const auto &fact : environment.identity.facts )
		identity.Set( fact.first, Value::String( fact.second ) );
	evidence.Set(
	    "reproduction", Value::String( "./kiln " +
	                                   std::string( request.upTo == StageRole::kEngine
	                                                    ? "build"
	                                                    : product::StageRoleName( request.upTo ) ) +
	                                   " " + profile.name + " --flavor " + flavor ) );
	result.evidenceFile = state.tree / "kiln-evidence" /
	                      ( std::string( product::StageRoleName( request.upTo ) ) + ".json" );
	if ( !WriteFileAtomic( result.evidenceFile, evidence.WritePretty() + "\n" ) )
		return foundation::MakeUnexpected(
		    Fail( "io", "cannot write " + result.evidenceFile.string() ) );
	result.evidence = std::move( evidence );
	return result;
}

Value ToJson( const ProfileSummary &summary )
{
	Value value = Value::Object();
	value.Set( "name", Value::String( summary.name ) );
	value.Set( "file", Value::String( summary.file ) );
	value.Set( "schema", Value::String( summary.schema ) );
	Value &aliases = value.Set( "aliases", Value::Array() );
	for ( const std::string &alias : summary.aliases )
		aliases.Push( Value::String( alias ) );
	value.Set( "buildable", Value::Bool( summary.buildable ) );
	value.Set( "fragment", Value::Bool( summary.fragment ) );
	if ( summary.error )
		value.Set( "error", Value::String( *summary.error ) );
	return value;
}

Value ToJson( const PipelineResult &result )
{
	Value value = Value::Object();
	value.Set( "schema", Value::String( std::string( kJsonSchema ) ) );
	value.Set( "profile", Value::String( result.profile ) );
	value.Set( "flavor", Value::String( result.flavor ) );
	value.Set( "tree", Value::String( result.tree.string() ) );
	value.Set( "up_to_date", Value::Bool( result.UpToDate() ) );
	Value &stages = value.Set( "stages", Value::Array() );
	for ( const StageRecord &record : result.stages )
	{
		Value stage = Value::Object();
		stage.Set( "name", Value::String( record.name ) );
		stage.Set( "role", Value::String( record.role ) );
		stage.Set( "up_to_date", Value::Bool( record.upToDate ) );
		stage.Set( "work_items", Value::Number( static_cast<long long>( record.workItems ) ) );
		stage.Set( "summary", Value::String( record.summary ) );
		stage.Set( "evidence", record.evidence.IsNull() ? Value::Object() : record.evidence );
		stages.Push( std::move( stage ) );
	}
	Value &artifacts = value.Set( "artifacts", Value::Array() );
	for ( const Artifact &artifact : result.artifacts )
	{
		Value item = Value::Object();
		item.Set( "name", Value::String( artifact.name ) );
		item.Set( "type", Value::String( artifact.type ) );
		item.Set( "path", Value::String( artifact.path.string() ) );
		item.Set( "digest", Value::String( artifact.digest ) );
		artifacts.Push( std::move( item ) );
	}
	if ( result.package )
		value.Set( "package_digest", Value::String( result.package->Digest() ) );
	if ( result.launch )
		value.Set( "exit_code", Value::Number( result.launch->exitCode ) );
	if ( !result.evidenceFile.empty() )
		value.Set( "evidence_file", Value::String( result.evidenceFile.string() ) );
	return value;
}

Value ToJson( const DoctorResult &result )
{
	Value value = Value::Object();
	value.Set( "schema", Value::String( std::string( kJsonSchema ) ) );
	value.Set( "profile", Value::String( result.profile ) );
	Value &items = value.Set( "items", Value::Array() );
	for ( const product::DoctorItem &item : result.report.items )
	{
		Value entry = Value::Object();
		entry.Set( "name", Value::String( item.name ) );
		entry.Set( "present", Value::Bool( item.present ) );
		entry.Set( "detail", Value::String( item.detail ) );
		items.Push( std::move( entry ) );
	}
	return value;
}

Value ExplainJson( const product::ResolvedProfile &profile )
{
	Value value = Value::Object();
	value.Set( "schema", Value::String( std::string( kJsonSchema ) ) );
	value.Set( "profile", Value::String( profile.name ) );
	Value &files = value.Set( "files", Value::Array() );
	for ( const std::string &file : profile.files )
		files.Push( Value::String( file ) );
	Value &provenance = value.Set( "provenance", Value::Object() );
	for ( const product::Provenance &entry : profile.provenance )
		provenance.Set( entry.path, Value::String( entry.file ) );
	if ( profile.Buildable() )
	{
		Value &flavors = value.Set( "waf_arguments", Value::Object() );
		for ( const product::Flavor &flavor : profile.flavors )
		{
			Value &list = flavors.Set( flavor.name, Value::Array() );
			for ( const std::string &argument :
			    profile.WafArguments( flavor.name ).ValueOr( std::vector<std::string>{} ) )
				list.Push( Value::String( argument ) );
		}
		value.Set( "toolchain", Value::String( profile.toolchain ) );
		Value &stages = value.Set( "stages", Value::Array() );
		for ( const std::string &stage : profile.stages )
			stages.Push( Value::String( stage ) );
	}
	return value;
}

} // namespace kiln
