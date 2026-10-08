//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.profile: the extends merge, schema v2 validation and
//			derived facts (RFC 0027).
//
//=============================================================================//

#include "product/profile.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace product
{

using foundation::json::Value;

namespace
{

ProfileError Error( ProfileErrorCode code, std::string file, std::string key, std::string detail )
{
	return ProfileError{ code, std::move( file ), std::move( key ), std::move( detail ) };
}

// The parent directory of a '/'-separated name, with a trailing '/'.
std::string Directory( const std::string &name )
{
	const size_t slash = name.rfind( '/' );
	return slash == std::string::npos ? std::string() : name.substr( 0, slash + 1 );
}

// Join a relative reference to a directory and fold "." and ".." segments.
// A reference that climbs above the source root is refused by the caller.
std::optional<std::string> JoinName( const std::string &directory, const std::string &relative )
{
	std::vector<std::string> parts;
	std::string path = directory + relative;
	std::string segment;
	std::istringstream stream( path );
	while ( std::getline( stream, segment, '/' ) )
	{
		if ( segment.empty() || segment == "." )
			continue;
		if ( segment == ".." )
		{
			if ( parts.empty() )
				return std::nullopt;
			parts.pop_back();
			continue;
		}
		parts.push_back( segment );
	}
	std::string joined;
	for ( size_t i = 0; i < parts.size(); ++i )
		joined += ( i ? "/" : "" ) + parts[i];
	return joined;
}

void RecordLeaves( const Value &value, const std::string &path, const std::string &file,
    std::map<std::string, std::string> &origin )
{
	// Replacing a value replaces every leaf below its path.
	// '/' is the character after '.', so [path + ".", path + "/") is exactly
	// the set of keys below path.
	origin.erase( path );
	origin.erase( origin.lower_bound( path + "." ), origin.lower_bound( path + "/" ) );
	if ( value.IsObject() && !value.Members().empty() )
	{
		for ( const auto &member : value.Members() )
			RecordLeaves( member.second, path.empty() ? member.first : path + "." + member.first,
			    file, origin );
		return;
	}
	origin[path] = file;
}

void MergeTracked( Value &base, const Value &derived, const std::string &path,
    const std::string &file, std::map<std::string, std::string> &origin )
{
	for ( const auto &member : derived.Members() )
	{
		const std::string child = path.empty() ? member.first : path + "." + member.first;
		Value *existing = base.Find( member.first );
		if ( existing && existing->IsObject() && member.second.IsObject() )
		{
			MergeTracked( *existing, member.second, child, file, origin );
			continue;
		}
		base.Set( member.first, member.second );
		RecordLeaves( member.second, child, file, origin );
	}
}

struct Resolver
{
	const IProfileSource &source;
	std::vector<std::string> stack;
	std::vector<std::string> files;
	std::map<std::string, std::string> origin;

	foundation::Expected<Value, ProfileError> Load( const std::string &name )
	{
		if ( std::find( stack.begin(), stack.end(), name ) != stack.end() )
		{
			std::string chain;
			for ( const auto &entry : stack )
				chain += entry + " -> ";
			return foundation::MakeUnexpected( Error(
			    ProfileErrorCode::kExtendsCycle, name, "extends", "cycle: " + chain + name ) );
		}
		auto text = source.Read( name );
		if ( !text )
			return foundation::MakeUnexpected( text.Error() );
		auto parsed = foundation::json::Parse( text.Value() );
		if ( !parsed )
			return foundation::MakeUnexpected( Error( ProfileErrorCode::kParse, name, "",
			    "offset " + std::to_string( parsed.Error().offset ) + ": " +
			        parsed.Error().detail ) );
		Value document = std::move( parsed ).Value();
		if ( !document.IsObject() )
			return foundation::MakeUnexpected(
			    Error( ProfileErrorCode::kSchema, name, "", "a profile is a JSON object" ) );
		std::vector<std::string> parents;
		if ( const Value *extends = document.Find( "extends" ) )
		{
			if ( extends->IsString() )
				parents.push_back( extends->Text() );
			else if ( extends->IsArray() && !extends->Items().empty() )
			{
				for ( const Value &item : extends->Items() )
				{
					if ( !item.IsString() )
						return foundation::MakeUnexpected( Error( ProfileErrorCode::kSchema, name,
						    "extends", "extends names files: a string or a list of strings" ) );
					parents.push_back( item.Text() );
				}
			}
			else
				return foundation::MakeUnexpected( Error( ProfileErrorCode::kSchema, name,
				    "extends", "extends names files: a string or a list of strings" ) );
			document.Remove( "extends" );
		}
		stack.push_back( name );
		Value merged = Value::Object();
		for ( const std::string &parent : parents )
		{
			const auto joined = JoinName( Directory( name ), parent );
			if ( !joined || joined->empty() )
				return foundation::MakeUnexpected( Error( ProfileErrorCode::kIo, name, "extends",
				    "\"" + parent + "\" leaves the profile directory" ) );
			auto loaded = Load( *joined );
			if ( !loaded )
				return loaded;
			MergeTracked( merged, loaded.Value(), "", *joined, origin );
		}
		stack.pop_back();
		MergeTracked( merged, document, "", name, origin );
		if ( std::find( files.begin(), files.end(), name ) == files.end() )
			files.push_back( name );
		return merged;
	}
};

// Top-level members a resolved profile may carry: the v1 families' keys
// (inspection only) and schema v2's sections.
const std::set<std::string> &KnownTopLevel()
{
	static const std::set<std::string> keys = { // identity
	    "schema", "id", "description",
	    // v1 sections, kept with their meaning
	    "target", "intent", "toolchain", "host_toolchain", "dependencies", "runtime_environment",
	    "configure_options", "required_checks", "boot_check", "evidence", "indirect_light", "sdk",
	    "static_composition", "android", "ios", "tvos", "macos", "app_icon", "app_banner",
	    // schema v2
	    "aliases", "build", "pipeline", "content", "target_facts", "package", "deploy", "launch" };
	return keys;
}

const std::set<std::string> kBuildKeys = {
    "toolchain", "flavors", "default_flavor", "compiler_cache", "host_tool_profile" };
const std::set<std::string> kFlavorKeys = { "description", "configure_options" };
const std::set<std::string> kPipelineKeys = { "stages" };
const std::set<std::string> kPackageKeys = { "form", "directory", "steps" };
const std::set<std::string> kDeployKeys = { "transport", "content_root", "capabilities" };
const std::set<std::string> kLaunchKeys = { "game", "default_map", "executable", "arguments",
    "environment", "display_session", "run", "switches", "variables", "map_arguments" };
const std::set<std::string> kSwitchKeys = { "description", "arguments", "conflicts", "set" };
const std::set<std::string> kContentKeys = {
    "roots", "base_packages", "mount_order", "mount_sets", "locators", "lowering" };
const std::set<std::string> kTargetKeys = { "product", "os", "architecture" };
const std::set<std::string> kToolchainKeys = { "cxx", "cc", "family", "version", "standard_library",
    "dialect_policy", "glibcxx_cxx11_abi", "build_type" };

// Options kiln owns for every tree; a profile setting them conflicts.
const std::set<std::string> kReservedOptions = { "out", "o", "prefix", "top", "t" };
// Waf's product selectors; at most one may be set (wscript configure).
const std::vector<std::string> kProductSelectors = { "dedicated", "tests", "tools" };

class Validator
{
public:
	Validator( const ResolvedProfile &profile, const ProviderNames &names )
	    : m_Profile( profile ), m_Names( names )
	{
	}

	std::optional<ProfileError> Run( ResolvedProfile &out )
	{
		const Value &doc = m_Profile.document;
		for ( const auto &member : doc.Members() )
		{
			if ( !KnownTopLevel().count( member.first ) )
				return Fail(
				    ProfileErrorCode::kUnknownKey, member.first, "not a schema v2 section" );
		}
		if ( auto e = Keys( "target", kTargetKeys ) )
			return e;
		if ( auto e = Keys( "build", kBuildKeys ) )
			return e;
		if ( auto e = Keys( "pipeline", kPipelineKeys ) )
			return e;
		if ( auto e = Keys( "package", kPackageKeys ) )
			return e;
		if ( auto e = Keys( "deploy", kDeployKeys ) )
			return e;
		if ( auto e = Keys( "launch", kLaunchKeys ) )
			return e;
		if ( auto e = Keys( "content", kContentKeys ) )
			return e;
		if ( auto e = Keys( "toolchain", kToolchainKeys ) )
			return e;
		if ( auto e = Aliases( out ) )
			return e;
		if ( auto e =
		         Options( "configure_options", Find( "configure_options" ), out.configureOptions ) )
			return e;
		if ( auto e = Build( out ) )
			return e;
		if ( auto e = Providers( out ) )
			return e;
		if ( auto e = Switches( out ) )
			return e;
		if ( out.Buildable() )
		{
			if ( auto e = Pins() )
				return e;
		}
		return std::nullopt;
	}

private:
	ProfileError Fail( ProfileErrorCode code, std::string key, std::string detail ) const
	{
		return Error( code, m_Profile.file, std::move( key ), std::move( detail ) );
	}

	// The file that set a key, for messages about inherited values.
	std::string Origin( const std::string &key ) const
	{
		for ( const auto &entry : m_Profile.provenance )
		{
			if ( entry.path == key || entry.path.compare( 0, key.size() + 1, key + "." ) == 0 )
				return entry.file;
		}
		return m_Profile.file;
	}

	const Value *Find( std::string_view key ) const { return m_Profile.document.Find( key ); }

	std::optional<ProfileError> Keys( const char *section, const std::set<std::string> &keys ) const
	{
		const Value *value = Find( section );
		if ( !value )
			return std::nullopt;
		if ( !value->IsObject() )
			return Fail( ProfileErrorCode::kSchema, section, "must be an object" );
		for ( const auto &member : value->Members() )
		{
			if ( !keys.count( member.first ) )
			{
				ProfileError error = Fail( ProfileErrorCode::kUnknownKey,
				    std::string( section ) + "." + member.first,
				    "not a key of " + std::string( section ) );
				error.file = Origin( std::string( section ) + "." + member.first );
				return error;
			}
		}
		return std::nullopt;
	}

	std::optional<ProfileError> StringList(
	    const Value *value, const std::string &key, std::vector<std::string> &out ) const
	{
		if ( !value )
			return std::nullopt;
		if ( !value->IsArray() )
			return Fail( ProfileErrorCode::kSchema, key, "must be a list of strings" );
		for ( const Value &item : value->Items() )
		{
			if ( !item.IsString() || item.Text().empty() )
				return Fail(
				    ProfileErrorCode::kSchema, key, "must be a list of non-empty strings" );
			out.push_back( item.Text() );
		}
		return std::nullopt;
	}

	// A list of strings, possibly empty, whose strings may be empty.
	std::optional<ProfileError> AnyStringList(
	    const Value *value, const std::string &key, std::vector<std::string> &out ) const
	{
		if ( !value )
			return std::nullopt;
		if ( !value->IsArray() )
			return Fail( ProfileErrorCode::kSchema, key, "must be a list of strings" );
		for ( const Value &item : value->Items() )
		{
			if ( !item.IsString() )
				return Fail( ProfileErrorCode::kSchema, key, "must be a list of strings" );
			out.push_back( item.Text() );
		}
		return std::nullopt;
	}

	std::optional<ProfileError> Aliases( ResolvedProfile &out ) const
	{
		return StringList( Find( "aliases" ), "aliases", out.aliases );
	}

	std::optional<ProfileError> Options(
	    const std::string &key, const Value *value, std::vector<ConfigureOption> &out ) const
	{
		if ( !value )
			return std::nullopt;
		if ( !value->IsObject() )
			return Fail( ProfileErrorCode::kSchema, key, "must be an object of Waf options" );
		int selectors = 0;
		for ( const auto &member : value->Members() )
		{
			const std::string &name = member.first;
			const std::string where = key + "." + name;
			const bool wellFormed = !name.empty() && std::all_of( name.begin(), name.end(),
			                                             []( char c )
			                                             {
				                                             return ( c >= 'a' && c <= 'z' ) ||
				                                                    ( c >= '0' && c <= '9' ) ||
				                                                    c == '_';
			                                             } );
			if ( !wellFormed )
				return Fail(
				    ProfileErrorCode::kSchema, where, "option names are lower_snake_case" );
			if ( kReservedOptions.count( name ) )
				return Fail( ProfileErrorCode::kConflictingOption, where,
				    "kiln owns the tree and prefix (out/<profile>/<flavor>); a profile may not "
				    "set " +
				        name );
			const Value &v = member.second;
			if ( !( v.IsString() || v.IsBool() || v.IsNumber() ) )
				return Fail(
				    ProfileErrorCode::kSchema, where, "an option is a string, bool or number" );
			if ( std::find( kProductSelectors.begin(), kProductSelectors.end(), name ) !=
			         kProductSelectors.end() &&
			     v.IsBool() && v.AsBool() )
				++selectors;
			out.push_back( ConfigureOption{ name, v } );
		}
		if ( selectors > 1 )
			return Fail( ProfileErrorCode::kConflictingOption, key,
			    "at most one of dedicated, tests and tools selects the product" );
		return std::nullopt;
	}

	std::optional<ProfileError> Build( ResolvedProfile &out ) const
	{
		const Value *build = Find( "build" );
		if ( !build )
			return std::nullopt;
		if ( const std::string *toolchain = build->FindString( "toolchain" ) )
			out.toolchain = *toolchain;
		else if ( build->Find( "toolchain" ) )
			return Fail(
			    ProfileErrorCode::kSchema, "build.toolchain", "names a toolchain provider" );
		if ( const std::string *flavor = build->FindString( "default_flavor" ) )
			out.defaultFlavor = *flavor;
		if ( const Value *flavors = build->Find( "flavors" ) )
		{
			if ( !flavors->IsObject() || flavors->Members().empty() )
				return Fail(
				    ProfileErrorCode::kSchema, "build.flavors", "is an object of named flavors" );
			for ( const auto &member : flavors->Members() )
			{
				const std::string where = "build.flavors." + member.first;
				if ( !member.second.IsObject() )
					return Fail( ProfileErrorCode::kSchema, where, "must be an object" );
				for ( const auto &key : member.second.Members() )
				{
					if ( !kFlavorKeys.count( key.first ) )
						return Fail( ProfileErrorCode::kUnknownKey, where + "." + key.first,
						    "not a key of a flavor" );
				}
				Flavor flavor;
				flavor.name = member.first;
				if ( const std::string *description = member.second.FindString( "description" ) )
					flavor.description = *description;
				if ( auto e = Options( where + ".configure_options",
				         member.second.Find( "configure_options" ), flavor.configureOptions ) )
					return e;
				// A flavor adds options; changing a base option is a conflict
				// (two values for one Waf option in one tree's identity).
				for ( const ConfigureOption &option : flavor.configureOptions )
				{
					for ( const ConfigureOption &base : out.configureOptions )
					{
						if ( base.key == option.key && !( base.value == option.value ) )
							return Fail( ProfileErrorCode::kConflictingOption,
							    where + ".configure_options." + option.key,
							    "conflicts with configure_options." + option.key + " (" +
							        base.value.Write() + " against " + option.value.Write() + ")" );
					}
				}
				out.flavors.push_back( std::move( flavor ) );
			}
		}
		if ( !out.toolchain.empty() && out.flavors.empty() )
			return Fail( ProfileErrorCode::kSchema, "build.flavors",
			    "a buildable profile declares flavors" );
		if ( !out.toolchain.empty() )
		{
			if ( out.defaultFlavor.empty() )
				out.defaultFlavor = out.flavors.front().name;
			if ( !out.FindFlavor( out.defaultFlavor ) )
				return Fail( ProfileErrorCode::kSchema, "build.default_flavor",
				    "\"" + out.defaultFlavor + "\" is not a declared flavor" );
		}
		return std::nullopt;
	}

	std::optional<ProfileError> Named( const std::string &key, const std::string &value,
	    const std::set<std::string> &known, const char *contract ) const
	{
		if ( known.count( value ) )
			return std::nullopt;
		ProfileError error = Fail( ProfileErrorCode::kUnknownProvider, key,
		    "\"" + value + "\" is not a " + contract + " provider in the catalog" );
		error.file = Origin( key );
		return error;
	}

	std::optional<ProfileError> Providers( ResolvedProfile &out ) const
	{
		if ( !out.toolchain.empty() )
		{
			if ( auto e =
			         Named( "build.toolchain", out.toolchain, m_Names.toolchains, "toolchain" ) )
				return e;
		}
		if ( const Value *pipeline = Find( "pipeline" ) )
		{
			if ( auto e = StringList( pipeline->Find( "stages" ), "pipeline.stages", out.stages ) )
				return e;
			for ( const std::string &stage : out.stages )
			{
				if ( auto e = Named( "pipeline.stages", stage, m_Names.stages, "stage" ) )
					return e;
			}
		}
		const auto single = [&]( const char *section, const char *key,
		                        const std::set<std::string> &known,
		                        const char *contract ) -> std::optional<ProfileError>
		{
			const Value *value = Find( section );
			const Value *name = value ? value->Find( key ) : nullptr;
			if ( !name )
				return std::nullopt;
			const std::string where = std::string( section ) + "." + key;
			if ( !name->IsString() )
				return Fail( ProfileErrorCode::kSchema, where, "names a provider" );
			return Named( where, name->Text(), known, contract );
		};
		if ( auto e = single( "package", "form", m_Names.packagers, "packager" ) )
			return e;
		if ( auto e = single( "deploy", "transport", m_Names.transports, "transport" ) )
			return e;
		if ( auto e =
		         single( "launch", "display_session", m_Names.displaySessions, "display session" ) )
			return e;
		if ( auto e = single( "launch", "run", m_Names.runProviders, "run" ) )
			return e;
		return std::nullopt;
	}

	std::optional<ProfileError> Switches( ResolvedProfile &out ) const
	{
		const Value *launch = Find( "launch" );
		if ( const Value *variables = launch ? launch->Find( "variables" ) : nullptr )
		{
			if ( !variables->IsObject() )
				return Fail(
				    ProfileErrorCode::kSchema, "launch.variables", "maps names to argument lists" );
			for ( const auto &member : variables->Members() )
			{
				std::vector<std::string> &list = out.launchVariables[member.first];
				if ( auto e =
				         AnyStringList( &member.second, "launch.variables." + member.first, list ) )
					return e;
			}
		}
		for ( const char *key : { "arguments", "map_arguments" } )
		{
			std::vector<std::string> unused;
			if ( auto e = AnyStringList( launch ? launch->Find( key ) : nullptr,
			         std::string( "launch." ) + key, unused ) )
				return e;
		}
		if ( const Value *environment = launch ? launch->Find( "environment" ) : nullptr )
		{
			if ( !environment->IsObject() )
				return Fail(
				    ProfileErrorCode::kSchema, "launch.environment", "maps names to strings" );
			for ( const auto &member : environment->Members() )
			{
				if ( !member.second.IsString() )
					return Fail( ProfileErrorCode::kSchema, "launch.environment." + member.first,
					    "an environment value is a string" );
			}
		}
		const Value *switches = launch ? launch->Find( "switches" ) : nullptr;
		if ( !switches )
			return std::nullopt;
		if ( !switches->IsObject() )
			return Fail(
			    ProfileErrorCode::kSchema, "launch.switches", "is an object of named switches" );
		for ( const auto &member : switches->Members() )
		{
			const std::string where = "launch.switches." + member.first;
			if ( !member.second.IsObject() )
				return Fail( ProfileErrorCode::kSchema, where, "must be an object" );
			for ( const auto &key : member.second.Members() )
			{
				if ( !kSwitchKeys.count( key.first ) )
					return Fail( ProfileErrorCode::kUnknownKey, where + "." + key.first,
					    "not a key of a switch" );
			}
			Switch entry;
			entry.name = member.first;
			if ( const std::string *description = member.second.FindString( "description" ) )
				entry.description = *description;
			else
				return Fail( ProfileErrorCode::kSchema, where + ".description",
				    "every switch is described" );
			if ( auto e = StringList(
			         member.second.Find( "arguments" ), where + ".arguments", entry.arguments ) )
				return e;
			if ( auto e = StringList(
			         member.second.Find( "conflicts" ), where + ".conflicts", entry.conflicts ) )
				return e;
			if ( const Value *sets = member.second.Find( "set" ) )
			{
				if ( !sets->IsObject() )
					return Fail( ProfileErrorCode::kSchema, where + ".set",
					    "maps launch variables to argument lists" );
				for ( const auto &variable : sets->Members() )
				{
					if ( !out.launchVariables.count( variable.first ) )
						return Fail( ProfileErrorCode::kSchema, where + ".set." + variable.first,
						    "is not a launch variable (launch.variables)" );
					if ( auto e = AnyStringList( &variable.second, where + ".set." + variable.first,
					         entry.sets[variable.first] ) )
						return e;
				}
			}
			out.switches.push_back( std::move( entry ) );
		}
		for ( const Switch &entry : out.switches )
		{
			for ( const std::string &conflict : entry.conflicts )
			{
				const bool known = std::any_of( out.switches.begin(), out.switches.end(),
				    [&]( const Switch &other )
				    {
					    return other.name == conflict;
				    } );
				if ( !known || conflict == entry.name )
					return Fail( ProfileErrorCode::kSchema,
					    "launch.switches." + entry.name + ".conflicts",
					    "\"" + conflict + "\" is not another switch" );
			}
		}
		return std::nullopt;
	}

	// A buildable profile pins its toolchain and every declared dependency.
	std::optional<ProfileError> Pins() const
	{
		const Value *toolchain = Find( "toolchain" );
		if ( !toolchain || !toolchain->IsObject() )
			return Fail( ProfileErrorCode::kMissingPin, "toolchain",
			    "a buildable profile pins its toolchain" );
		for ( const char *key : { "family", "version", "cxx" } )
		{
			const std::string *value = toolchain->FindString( key );
			if ( !value || value->empty() )
				return Fail( ProfileErrorCode::kMissingPin, std::string( "toolchain." ) + key,
				    "a buildable profile pins its toolchain " + std::string( key ) );
		}
		const Value *dependencies = Find( "dependencies" );
		const Value *pkg = dependencies ? dependencies->Find( "pkg_config" ) : nullptr;
		if ( pkg )
		{
			if ( !pkg->IsObject() )
				return Fail( ProfileErrorCode::kSchema, "dependencies.pkg_config",
				    "maps names to versions" );
			for ( const auto &member : pkg->Members() )
			{
				if ( !member.second.IsString() || member.second.Text().empty() )
					return Fail( ProfileErrorCode::kMissingPin,
					    "dependencies.pkg_config." + member.first,
					    "every dependency has an exact version" );
			}
		}
		return std::nullopt;
	}

	const ResolvedProfile &m_Profile;
	const ProviderNames &m_Names;
};

std::string Stem( const std::string &name )
{
	std::string stem =
	    name.substr( name.rfind( '/' ) == std::string::npos ? 0 : name.rfind( '/' ) + 1 );
	if ( stem.size() > 5 && stem.compare( stem.size() - 5, 5, ".json" ) == 0 )
		stem.resize( stem.size() - 5 );
	return stem;
}

} // namespace

std::string_view ProfileErrorCodeName( ProfileErrorCode code )
{
	switch ( code )
	{
	case ProfileErrorCode::kIo:
		return "io";
	case ProfileErrorCode::kParse:
		return "parse";
	case ProfileErrorCode::kSchema:
		return "schema";
	case ProfileErrorCode::kExtendsCycle:
		return "extends-cycle";
	case ProfileErrorCode::kUnknownKey:
		return "unknown-key";
	case ProfileErrorCode::kUnknownProvider:
		return "unknown-provider";
	case ProfileErrorCode::kMissingPin:
		return "missing-pin";
	case ProfileErrorCode::kConflictingOption:
		return "conflicting-option";
	case ProfileErrorCode::kWorkspaceBuildFact:
		return "workspace-build-fact";
	case ProfileErrorCode::kNotBuildable:
		return "not-buildable";
	case ProfileErrorCode::kUnknownProfile:
		return "unknown-profile";
	}
	return "unknown";
}

std::string ProfileError::Describe() const
{
	std::string text( ProfileErrorCodeName( code ) );
	if ( !file.empty() )
		text += " " + file;
	if ( !key.empty() )
		text += ( file.empty() ? " " : " " ) + key;
	if ( !detail.empty() )
		text += ": " + detail;
	return text;
}

foundation::Expected<std::string, ProfileError> DirectoryProfileSource::Read(
    const std::string &name ) const
{
	std::ifstream stream( m_Root / name, std::ios::binary );
	if ( !stream )
		return foundation::MakeUnexpected(
		    Error( ProfileErrorCode::kIo, name, "", "cannot read " + ( m_Root / name ).string() ) );
	std::ostringstream text;
	text << stream.rdbuf();
	return text.str();
}

std::vector<std::string> DirectoryProfileSource::List() const
{
	std::vector<std::string> names;
	std::error_code ec;
	for ( auto it = std::filesystem::recursive_directory_iterator( m_Root, ec );
	    !ec && it != std::filesystem::recursive_directory_iterator(); it.increment( ec ) )
	{
		if ( it->is_regular_file() && it->path().extension() == ".json" )
			names.push_back( std::filesystem::relative( it->path(), m_Root ).generic_string() );
	}
	std::sort( names.begin(), names.end() );
	return names;
}

void MergeInto( Value &base, const Value &derived )
{
	std::map<std::string, std::string> unused;
	MergeTracked( base, derived, "", "", unused );
}

foundation::Expected<MergedDocument, ProfileError> ResolveExtends(
    const IProfileSource &source, const std::string &name )
{
	Resolver resolver{ source, {}, {}, {} };
	auto document = resolver.Load( name );
	if ( !document )
		return foundation::MakeUnexpected( document.Error() );
	MergedDocument merged;
	merged.document = std::move( document ).Value();
	merged.files = std::move( resolver.files );
	for ( const auto &entry : resolver.origin )
		merged.provenance.push_back( Provenance{ entry.first, entry.second } );
	return merged;
}

const Flavor *ResolvedProfile::FindFlavor( std::string_view flavor ) const
{
	for ( const Flavor &entry : flavors )
	{
		if ( entry.name == flavor )
			return &entry;
	}
	return nullptr;
}

std::uint64_t ResolvedProfile::Digest() const
{
	std::uint64_t hash = 1469598103934665603ull;
	for ( const char c : document.Write() )
	{
		hash ^= static_cast<unsigned char>( c );
		hash *= 1099511628211ull;
	}
	return hash;
}

foundation::Expected<std::vector<std::string>, ProfileError> ResolvedProfile::WafArguments(
    std::string_view flavorName ) const
{
	const Flavor *flavor = FindFlavor( flavorName );
	if ( !flavor )
		return foundation::MakeUnexpected( Error( ProfileErrorCode::kNotBuildable, file,
		    "build.flavors", "\"" + std::string( flavorName ) + "\" is not a flavor of " + name ) );
	std::vector<std::string> arguments;
	const auto add = [&]( const ConfigureOption &option )
	{
		std::string flag = "--" + option.key;
		std::replace( flag.begin(), flag.end(), '_', '-' );
		if ( option.value.IsBool() )
		{
			if ( option.value.AsBool() )
				arguments.push_back( flag );
			return;
		}
		arguments.push_back( flag + "=" + option.value.Text() );
	};
	for ( const ConfigureOption &option : configureOptions )
		add( option );
	for ( const ConfigureOption &option : flavor->configureOptions )
	{
		const bool repeated = std::any_of( configureOptions.begin(), configureOptions.end(),
		    [&]( const ConfigureOption &base )
		    {
			    return base.key == option.key;
		    } );
		if ( !repeated )
			add( option );
	}
	return arguments;
}

std::string ResolvedProfile::TreeName( std::string_view flavor ) const
{
	return name + "/" + std::string( flavor );
}

foundation::Expected<ResolvedProfile, ProfileError> Resolve(
    const IProfileSource &source, const std::string &name, const ProviderNames &names )
{
	auto merged = ResolveExtends( source, name );
	if ( !merged )
		return foundation::MakeUnexpected( merged.Error() );
	ResolvedProfile profile;
	profile.name = Stem( name );
	profile.file = name;
	profile.document = std::move( merged.Value().document );
	profile.files = std::move( merged.Value().files );
	profile.provenance = std::move( merged.Value().provenance );
	if ( const std::string *id = profile.document.FindString( "id" ) )
		profile.id = *id;
	if ( const std::string *schema = profile.document.FindString( "schema" ) )
		profile.schema = *schema;
	profile.fragment = profile.schema == kFragmentSchemaV2;
	if ( profile.schema != kProfileSchemaV2 && !profile.fragment )
		return profile; // a v1 family: inspection only, never buildable
	Validator validator( profile, names );
	if ( auto error = validator.Run( profile ) )
		return foundation::MakeUnexpected( *error );
	return profile;
}

foundation::Expected<std::string, ProfileError> FindProfile(
    const IProfileSource &source, std::string_view nameOrAlias, std::string_view hostTag )
{
	const std::string wanted( nameOrAlias );
	std::vector<std::string> matches;
	for ( const std::string &name : source.List() )
	{
		if ( name == wanted || Stem( name ) == wanted )
			return name;
	}
	for ( const std::string &name : source.List() )
	{
		auto merged = ResolveExtends( source, name );
		if ( !merged )
			continue;
		const Value *aliases = merged.Value().document.Find( "aliases" );
		if ( !aliases || !aliases->IsArray() )
			continue;
		for ( const Value &alias : aliases->Items() )
		{
			if ( !alias.IsString() )
				continue;
			const std::string &text = alias.Text();
			const size_t at = text.find( '@' );
			const std::string base = text.substr( 0, at );
			const bool hostMatches = at == std::string::npos || text.substr( at + 1 ) == hostTag;
			// An alias names the profile that declares it, not the ones that
			// inherit it from a fragment.
			bool declared = false;
			for ( const Provenance &entry : merged.Value().provenance )
				declared |= entry.path == "aliases" && entry.file == name;
			if ( base == wanted && hostMatches && declared )
				matches.push_back( name );
		}
	}
	if ( matches.size() == 1 )
		return matches.front();
	return foundation::MakeUnexpected( Error( ProfileErrorCode::kUnknownProfile, "", wanted,
	    matches.empty() ? "no profile or alias of that name for " + std::string( hostTag )
	                    : "the alias names " + std::to_string( matches.size() ) + " profiles" ) );
}

namespace
{
// Personal settings the workspace file may carry at its top level.
const std::set<std::string> kWorkspaceKeys = { "default_profile", "default_map", "resolution",
    "windowed", "frame_cap", "content_locations", "devices", "profiles" };
} // namespace

foundation::Expected<Workspace, ProfileError> ParseWorkspace(
    std::string_view text, std::string_view file )
{
	const std::string where( file );
	auto parsed = foundation::json::Parse( text );
	if ( !parsed )
		return foundation::MakeUnexpected( Error( ProfileErrorCode::kParse, where, "",
		    "offset " + std::to_string( parsed.Error().offset ) + ": " + parsed.Error().detail ) );
	Workspace workspace;
	workspace.document = std::move( parsed ).Value();
	if ( !workspace.document.IsObject() )
		return foundation::MakeUnexpected(
		    Error( ProfileErrorCode::kSchema, where, "", "the workspace file is a JSON object" ) );
	for ( const auto &member : workspace.document.Members() )
	{
		if ( !kWorkspaceKeys.count( member.first ) )
			return foundation::MakeUnexpected( Error( ProfileErrorCode::kWorkspaceBuildFact, where,
			    member.first, "not a workspace setting; build facts belong in profiles" ) );
	}
	if ( const std::string *profile = workspace.document.FindString( "default_profile" ) )
		workspace.defaultProfile = *profile;
	if ( const std::string *map = workspace.document.FindString( "default_map" ) )
		workspace.defaultMap = *map;
	if ( const Value *profiles = workspace.document.Find( "profiles" ) )
	{
		if ( !profiles->IsObject() )
			return foundation::MakeUnexpected( Error(
			    ProfileErrorCode::kSchema, where, "profiles", "maps profile names to overrides" ) );
		for ( const auto &entry : profiles->Members() )
		{
			if ( !entry.second.IsObject() )
				return foundation::MakeUnexpected( Error( ProfileErrorCode::kSchema, where,
				    "profiles." + entry.first, "an override is an object" ) );
			for ( const auto &member : entry.second.Members() )
			{
				if ( member.first != "launch" )
					return foundation::MakeUnexpected( Error( ProfileErrorCode::kWorkspaceBuildFact,
					    where, "profiles." + entry.first + "." + member.first,
					    "the workspace may override launch values only; this would change the "
					    "tree" ) );
			}
		}
	}
	return workspace;
}

foundation::Expected<ResolvedProfile, ProfileError> ApplyWorkspace(
    const ResolvedProfile &profile, const Workspace &workspace )
{
	ResolvedProfile result = profile;
	const Value *profiles = workspace.document.Find( "profiles" );
	const Value *overrides = profiles ? profiles->Find( profile.name ) : nullptr;
	const Value *launch = overrides ? overrides->Find( "launch" ) : nullptr;
	if ( !launch )
		return result;
	if ( !launch->IsObject() )
		return foundation::MakeUnexpected( Error( ProfileErrorCode::kSchema, "",
		    "profiles." + profile.name + ".launch", "must be an object" ) );
	for ( const auto &member : launch->Members() )
	{
		if ( !kLaunchKeys.count( member.first ) || member.first == "switches" )
			return foundation::MakeUnexpected( Error( ProfileErrorCode::kWorkspaceBuildFact, "",
			    "profiles." + profile.name + ".launch." + member.first,
			    "not an overridable launch value" ) );
	}
	Value *target = result.document.Find( "launch" );
	if ( !target )
		target = &result.document.Set( "launch", Value::Object() );
	MergeInto( *target, *launch );
	if ( const Value *variables = launch->Find( "variables" ) )
	{
		if ( !variables->IsObject() )
			return foundation::MakeUnexpected( Error( ProfileErrorCode::kSchema, "",
			    "profiles." + profile.name + ".launch.variables",
			    "maps names to argument lists" ) );
		for ( const auto &member : variables->Members() )
		{
			std::vector<std::string> list;
			for ( const Value &item :
			    member.second.IsArray() ? member.second.Items() : std::vector<Value>{} )
			{
				if ( !item.IsString() )
					return foundation::MakeUnexpected( Error( ProfileErrorCode::kSchema, "",
					    "profiles." + profile.name + ".launch.variables." + member.first,
					    "an argument list of strings" ) );
				list.push_back( item.Text() );
			}
			result.launchVariables[member.first] = std::move( list );
		}
	}
	return result;
}

} // namespace product
