//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.package.linux-dir: the `linux-dir` packager (RFC 0027 L1).
//
//=============================================================================//

#include "product/package_linux_dir.h"

#include "content/vpk_archive.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kRecordName = ".kiln-package.json";

ProviderError Fail( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

std::string Lower( std::string text )
{
	for ( char &c : text )
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	return text;
}

std::string ReadBytes( const fs::path &path )
{
	std::ifstream stream( path, std::ios::binary );
	std::ostringstream text;
	text << stream.rdbuf();
	return text.str();
}

std::vector<std::string> Strings( const Value *value )
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

std::string Replace( std::string text, const std::string &from, const std::string &to )
{
	for ( size_t at = text.find( from ); at != std::string::npos;
	    at = text.find( from, at + to.size() ) )
		text.replace( at, from.size(), to );
	return text;
}

bool IsElf32( const fs::path &path )
{
	std::ifstream stream( path, std::ios::binary );
	char header[5] = {};
	stream.read( header, 5 );
	return stream.gcount() == 5 && header[0] == 0x7f && header[1] == 'E' && header[2] == 'L' &&
	       header[3] == 'F' && header[4] == 1;
}

// The entries the packager placed, persisted in the runtime so a later
// package knows what it owns: path -> "file:<hash>" or "link:<target>".
struct Owned
{
	std::map<std::string, std::string> entries;
	std::map<std::string, std::string> roles;
};

class Builder
{
public:
	Builder( const PackageRequest &request, const fs::path &runtime )
	    : m_Request( request ), m_Runtime( runtime )
	{
	}

	foundation::Expected<void, ProviderError> Run( const Value &steps )
	{
		LoadRecord();
		for ( const Value &step : steps.Items() )
		{
			if ( m_Request.cancel && m_Request.cancel->IsCancelled() )
				return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "" ) );
			const std::string *op = step.FindString( "op" );
			if ( !op )
				return foundation::MakeUnexpected(
				    Fail( "invalid-step", "a package step names its op" ) );
			foundation::Expected<void, ProviderError> done;
			if ( *op == "seed" )
				done = Seed( step );
			else if ( *op == "overlay" )
				done = Overlay( step );
			else if ( *op == "remove-elf32" )
				done = RemoveElf32( step );
			else if ( *op == "mount-each" )
				done = MountEach( step );
			else if ( *op == "link" )
				done = Link( step );
			else if ( *op == "search-paths" )
				done = SearchPaths( step );
			else if ( *op == "extract" )
				done = Extract( step );
			else
				return foundation::MakeUnexpected(
				    Fail( "invalid-step", "unknown package op \"" + *op + "\"" ) );
			if ( !done )
				return done;
		}
		return SaveRecord();
	}

	PackageManifest Manifest() const
	{
		PackageManifest manifest;
		manifest.form = std::string( kLinuxDirPackager );
		for ( const auto &entry : m_Owned.entries )
		{
			auto role = m_Owned.roles.find( entry.first );
			manifest.entries.push_back( { entry.first, entry.second,
			    role == m_Owned.roles.end() ? std::string( "content" ) : role->second, 0 } );
		}
		return manifest;
	}

private:
	fs::path Location( const std::string &name ) const
	{
		auto it = m_Request.locations.find( name );
		return it == m_Request.locations.end() ? fs::path() : it->second;
	}

	void LoadRecord()
	{
		auto parsed = foundation::json::Parse( ReadBytes( m_Runtime / kRecordName ) );
		if ( !parsed || !parsed.Value().IsObject() )
			return;
		if ( const Value *entries = parsed.Value().Find( "entries" ) )
		{
			for ( const auto &member : entries->Members() )
			{
				if ( member.second.IsObject() )
				{
					if ( const std::string *hash = member.second.FindString( "hash" ) )
						m_Owned.entries[member.first] = *hash;
					if ( const std::string *role = member.second.FindString( "role" ) )
						m_Owned.roles[member.first] = *role;
				}
			}
		}
	}

	foundation::Expected<void, ProviderError> SaveRecord()
	{
		Value record = Value::Object();
		record.Set( "schema", Value::String( "kiln-linux-dir/v1" ) );
		Value &entries = record.Set( "entries", Value::Object() );
		for ( const auto &entry : m_Owned.entries )
		{
			Value item = Value::Object();
			item.Set( "hash", Value::String( entry.second ) );
			auto role = m_Owned.roles.find( entry.first );
			item.Set(
			    "role", Value::String( role == m_Owned.roles.end() ? "content" : role->second ) );
			entries.Set( entry.first, std::move( item ) );
		}
		return WriteAtomic( m_Runtime / kRecordName, record.WritePretty() + "\n" );
	}

	static foundation::Expected<void, ProviderError> WriteAtomic(
	    const fs::path &path, const std::string &bytes )
	{
		std::error_code ec;
		fs::create_directories( path.parent_path(), ec );
		const fs::path staging = path.string() + ".kiln-staging";
		{
			std::ofstream stream( staging, std::ios::binary | std::ios::trunc );
			if ( !stream.write( bytes.data(), static_cast<std::streamsize>( bytes.size() ) ) )
				return foundation::MakeUnexpected(
				    Fail( "io", "cannot write " + staging.string() ) );
		}
		fs::rename( staging, path, ec );
		if ( ec )
			return foundation::MakeUnexpected( Fail( "io", "cannot publish " + path.string() ) );
		return {};
	}

	// Place a private copy of `source` at `relative`, keeping its times and
	// permissions; a temporary name and a rename make the replacement atomic.
	foundation::Expected<void, ProviderError> CopyEntry(
	    const fs::path &source, const std::string &relative, const std::string &role )
	{
		const fs::path target = m_Runtime / relative;
		std::error_code ec;
		fs::create_directories( target.parent_path(), ec );
		const fs::path staging = target.string() + ".kiln-staging";
		fs::remove( staging, ec );
		if ( !fs::copy_file( source, staging, fs::copy_options::overwrite_existing, ec ) )
			return foundation::MakeUnexpected(
			    Fail( "io", "cannot copy " + source.string() + ": " + ec.message() ) );
		fs::last_write_time( staging, fs::last_write_time( source, ec ), ec );
		fs::remove( target, ec ); // a link or file of the same name
		fs::rename( staging, target, ec );
		if ( ec )
			return foundation::MakeUnexpected( Fail( "io", "cannot place " + target.string() ) );
		m_Owned.entries[relative] = "file:" + HashHex( ReadBytes( target ) );
		m_Owned.roles[relative] = role;
		return {};
	}

	foundation::Expected<void, ProviderError> LinkEntry(
	    const fs::path &target, const std::string &relative, const std::string &role )
	{
		const fs::path link = m_Runtime / relative;
		std::error_code ec;
		fs::create_directories( link.parent_path(), ec );
		if ( fs::is_symlink( link, ec ) && fs::read_symlink( link, ec ) == target )
		{
			m_Owned.entries[relative] = "link:" + target.string();
			m_Owned.roles[relative] = role;
			return {};
		}
		const fs::path staging = link.string() + ".kiln-staging";
		fs::remove( staging, ec );
		fs::create_symlink( target, staging, ec );
		if ( ec )
			return foundation::MakeUnexpected(
			    Fail( "io", "cannot link " + link.string() + ": " + ec.message() ) );
		if ( fs::is_directory( link, ec ) && !fs::is_symlink( link, ec ) )
			return foundation::MakeUnexpected(
			    Fail( "conflict", link.string() + " is a directory the packager does not own" ) );
		fs::rename( staging, link, ec );
		if ( ec )
			return foundation::MakeUnexpected( Fail( "io", "cannot place " + link.string() ) );
		m_Owned.entries[relative] = "link:" + target.string();
		m_Owned.roles[relative] = role;
		return {};
	}

	foundation::Expected<void, ProviderError> Seed( const Value &step )
	{
		const std::string *from = step.FindString( "from" );
		const std::string *marker = step.FindString( "marker" );
		if ( !from || !marker )
			return foundation::MakeUnexpected(
			    Fail( "invalid-step", "seed needs from and marker" ) );
		std::error_code ec;
		if ( fs::exists( m_Runtime / *marker, ec ) )
			return {}; // seeded before; later packages only overlay
		const fs::path base = Location( *from );
		if ( base.empty() || !fs::exists( base / *marker, ec ) )
			return foundation::MakeUnexpected( Fail( std::string( kUnavailable ),
			    "content locator \"" + *from + "\" has no " + *marker + " (set content_locations." +
			        *from + " in .kiln/local.json)" ) );
		const fs::path source = fs::weakly_canonical( base, ec );
		std::set<std::string> shared, skipDirectories, skipSuffixes;
		for ( const std::string &suffix : Strings( step.Find( "shared_suffixes" ) ) )
			shared.insert( Lower( suffix ) );
		for ( const std::string &name : Strings( step.Find( "skip_directories" ) ) )
			skipDirectories.insert( Lower( name ) );
		for ( const std::string &suffix : Strings( step.Find( "skip_suffixes" ) ) )
			skipSuffixes.insert( Lower( suffix ) );
		const std::string game =
		    step.FindString( "game" ) ? *step.FindString( "game" ) : std::string();
		const Value *contentOnly = step.Find( "content_only" );
		const bool onlyContent = contentOnly && contentOnly->IsBool() && contentOnly->AsBool();
		return SeedDirectory(
		    source, "", shared, skipDirectories, skipSuffixes, game, onlyContent );
	}

	foundation::Expected<void, ProviderError> SeedDirectory( const fs::path &source,
	    const std::string &relative, const std::set<std::string> &shared,
	    const std::set<std::string> &skipDirectories, const std::set<std::string> &skipSuffixes,
	    const std::string &game, bool contentOnly )
	{
		const fs::path directory = relative.empty() ? source : source / relative;
		std::vector<fs::directory_entry> entries;
		std::error_code ec;
		for ( auto it = fs::directory_iterator( directory, ec );
		    !ec && it != fs::directory_iterator(); it.increment( ec ) )
			entries.push_back( *it );
		std::sort( entries.begin(), entries.end(),
		    []( const auto &a, const auto &b )
		    {
			    return a.path().filename() < b.path().filename();
		    } );
		fs::create_directories( m_Runtime / relative, ec );
		for ( const fs::directory_entry &entry : entries )
		{
			const std::string name = entry.path().filename().string();
			const std::string child = relative.empty() ? name : relative + "/" + name;
			std::error_code inner;
			const bool isLink = entry.is_symlink( inner );
			const bool isDirectory = entry.is_directory( inner );
			if ( isDirectory )
			{
				if ( contentOnly && relative.empty() && name != game && name != "platform" )
					continue;
				if ( contentOnly && relative == game && name == "bin" )
					continue;
				if ( skipDirectories.count( Lower( name ) ) )
					continue;
				if ( isLink )
				{
					// A linked directory stays one link (to its resolved target).
					auto linked =
					    LinkEntry( fs::canonical( entry.path(), inner ), child, "content" );
					if ( !linked )
						return linked;
					continue;
				}
				auto seeded = SeedDirectory(
				    source, child, shared, skipDirectories, skipSuffixes, game, contentOnly );
				if ( !seeded )
					return seeded;
				continue;
			}
			if ( contentOnly && relative.empty() )
				continue;
			const std::string suffix = Lower( entry.path().extension().string() );
			if ( skipSuffixes.count( suffix ) )
				continue;
			if ( shared.count( suffix ) )
			{
				auto linked = LinkEntry( fs::canonical( entry.path(), inner ), child, "content" );
				if ( !linked )
					return linked;
				continue;
			}
			auto copied = CopyEntry( entry.path(), child, "content" );
			if ( !copied )
				return copied;
		}
		return {};
	}

	foundation::Expected<void, ProviderError> Overlay( const Value &step )
	{
		const std::string *name = step.FindString( "artifact" );
		auto artifact = name ? m_Request.artifacts.find( *name ) : m_Request.artifacts.end();
		if ( artifact == m_Request.artifacts.end() )
			return foundation::MakeUnexpected( Fail( "missing-input",
			    "overlay needs the artifact \"" + ( name ? *name : std::string() ) + "\"" ) );
		const std::vector<std::string> include = Strings( step.Find( "include" ) );
		const fs::path root = artifact->second.path;
		std::vector<std::string> files;
		std::error_code ec;
		for ( auto it = fs::recursive_directory_iterator( root, ec );
		    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
		{
			std::error_code inner;
			if ( !it->is_regular_file( inner ) )
				continue;
			const std::string relative = fs::relative( it->path(), root, inner ).generic_string();
			if ( std::any_of( include.begin(), include.end(),
			         [&]( const std::string &pattern )
			         {
				         return GlobMatch( pattern, relative );
			         } ) )
				files.push_back( relative );
		}
		std::sort( files.begin(), files.end() );
		if ( files.empty() )
			return foundation::MakeUnexpected(
			    Fail( "missing-input", "overlay of " + *name + " matched no file" ) );
		for ( const std::string &relative : files )
		{
			auto copied = CopyEntry( root / relative, relative,
			    relative.find( ".so" ) != std::string::npos ? "module" : "executable" );
			if ( !copied )
				return copied;
		}
		return {};
	}

	foundation::Expected<void, ProviderError> RemoveElf32( const Value &step )
	{
		std::error_code ec;
		for ( const std::string &directory : Strings( step.Find( "directories" ) ) )
		{
			for ( auto it = fs::directory_iterator( m_Runtime / directory, ec );
			    !ec && it != fs::directory_iterator(); it.increment( ec ) )
			{
				const std::string name = it->path().filename().string();
				std::error_code inner;
				if ( name.find( ".so" ) == std::string::npos || !it->is_regular_file( inner ) ||
				     !IsElf32( it->path() ) )
					continue;
				fs::remove( it->path(), inner );
				m_Owned.entries.erase( directory + "/" + name );
				m_Owned.roles.erase( directory + "/" + name );
			}
			ec.clear();
		}
		return {};
	}

	foundation::Expected<void, ProviderError> MountEach( const Value &step )
	{
		const std::string *from = step.FindString( "from" );
		const std::string *into = step.FindString( "into" );
		const std::string *prefix = step.FindString( "prefix" );
		if ( !from || !into || !prefix || prefix->empty() )
			return foundation::MakeUnexpected(
			    Fail( "invalid-step", "mount-each needs from, into and prefix" ) );
		const std::string record =
		    step.FindString( "record" ) ? *step.FindString( "record" ) : std::string();
		const std::string require =
		    step.FindString( "require" ) ? *step.FindString( "require" ) : std::string();
		const std::string skip = step.FindString( "skip_if_exists" )
		                             ? *step.FindString( "skip_if_exists" )
		                             : std::string();
		const fs::path store = Location( *from );
		std::error_code ec;
		std::set<std::string> wanted;
		if ( !store.empty() && fs::is_directory( store, ec ) )
		{
			const fs::path storeCanonical = fs::canonical( store, ec );
			std::vector<std::string> names;
			for ( auto it = fs::directory_iterator( store, ec );
			    !ec && it != fs::directory_iterator(); it.increment( ec ) )
			{
				std::error_code inner;
				const std::string name = it->path().filename().string();
				if ( it->is_directory( inner ) && name.front() != '.' )
					names.push_back( name );
			}
			std::sort( names.begin(), names.end() );
			for ( const std::string &name : names )
			{
				if ( !record.empty() &&
				     !fs::is_regular_file( store / name / Replace( record, "{name}", name ), ec ) )
					continue;
				if ( !require.empty() &&
				     !fs::exists( store / name / Replace( require, "{name}", name ), ec ) )
					continue;
				if ( !skip.empty() &&
				     fs::exists( m_Runtime / Replace( skip, "{name}", name ), ec ) )
					continue;
				const std::string relative = *into + "/" + *prefix + name;
				const fs::path target = fs::relative(
				    storeCanonical / name, fs::weakly_canonical( m_Runtime / *into, ec ), ec );
				auto linked = LinkEntry( target, relative, "content" );
				if ( !linked )
					return linked;
				wanted.insert( relative );
			}
		}
		else
		{
			const Value *optional = step.Find( "optional" );
			if ( !( optional && optional->IsBool() && optional->AsBool() ) )
				return foundation::MakeUnexpected( Fail( std::string( kUnavailable ),
				    "content locator \"" + *from + "\" is not a directory" ) );
		}
		// Stale managed mounts go.
		const std::string managed = *into + "/" + *prefix;
		for ( auto it = m_Owned.entries.begin(); it != m_Owned.entries.end(); )
		{
			if ( it->first.rfind( managed, 0 ) == 0 && !wanted.count( it->first ) )
			{
				fs::remove( m_Runtime / it->first, ec );
				m_Owned.roles.erase( it->first );
				it = m_Owned.entries.erase( it );
			}
			else
				++it;
		}
		return {};
	}

	bool Optional( const Value &step ) const
	{
		const Value *optional = step.Find( "optional" );
		return optional && optional->IsBool() && optional->AsBool();
	}

	foundation::Expected<void, ProviderError> Link( const Value &step )
	{
		const std::string *path = step.FindString( "path" );
		const std::string *from = step.FindString( "from" );
		if ( !path || !from )
			return foundation::MakeUnexpected( Fail( "invalid-step", "link needs path and from" ) );
		const std::string source =
		    step.FindString( "source" ) ? *step.FindString( "source" ) : std::string();
		const std::string check =
		    step.FindString( "check" ) ? *step.FindString( "check" ) : std::string();
		const fs::path base = Location( *from );
		std::error_code ec;
		const fs::path target =
		    base.empty() ? fs::path() : ( source.empty() ? base : base / source );
		if ( target.empty() || !fs::exists( target, ec ) ||
		     ( !check.empty() && !fs::exists( target / check, ec ) ) )
		{
			if ( !Optional( step ) )
				return foundation::MakeUnexpected( Fail( std::string( kUnavailable ),
				    "content locator \"" + *from + "\" lacks " +
				        ( source.empty() ? check : source + "/" + check ) ) );
			// An optional link whose content is absent is removed, so a runtime
			// never mounts stale content.
			if ( fs::is_symlink( m_Runtime / *path, ec ) )
				fs::remove( m_Runtime / *path, ec );
			m_Owned.entries.erase( *path );
			m_Owned.roles.erase( *path );
			return {};
		}
		return LinkEntry( fs::canonical( target, ec ), *path, "content" );
	}

	foundation::Expected<void, ProviderError> SearchPaths( const Value &step )
	{
		const std::string *file = step.FindString( "file" );
		const Value *lines = step.Find( "lines" );
		if ( !file || !lines || !lines->IsArray() )
			return foundation::MakeUnexpected(
			    Fail( "invalid-step", "search-paths needs file and lines" ) );
		const std::string block =
		    step.FindString( "block" ) ? *step.FindString( "block" ) : "SearchPaths";
		const std::string indent =
		    step.FindString( "indent" ) ? *step.FindString( "indent" ) : "\t\t";
		std::string body = indent + block + "\n" + indent + "{";
		std::error_code ec;
		for ( const Value &line : lines->Items() )
		{
			if ( line.IsString() )
			{
				body += "\n" + indent + "\t" + line.Text();
				continue;
			}
			const std::string *text = line.FindString( "line" );
			if ( !text )
				return foundation::MakeUnexpected(
				    Fail( "invalid-step", "a search-paths entry has a line" ) );
			if ( const std::string *when = line.FindString( "when_exists" ) )
			{
				if ( !fs::exists( m_Runtime / *when, ec ) )
					continue;
			}
			if ( const std::string *each = line.FindString( "each_directory" ) )
			{
				// One line per entry of a runtime directory's record (in order),
				// or per subdirectory when there is no record.
				for ( const std::string &item : Directories( m_Runtime / *each ) )
					body += "\n" + indent + "\t" + Replace( *text, "{item}", item );
				continue;
			}
			body += "\n" + indent + "\t" + *text;
		}
		body += "\n" + indent + "}";
		const fs::path path = m_Runtime / *file;
		// The file is read as text with universal newlines and written with LF,
		// as the Python staging this replaces did (retail gameinfo is CRLF).
		const std::string contents =
		    Replace( Replace( ReadBytes( path ), "\r\n", "\n" ), "\r", "\n" );
		const size_t start = contents.find( indent + block );
		const size_t open = start == std::string::npos ? start : contents.find( '{', start );
		const size_t close = open == std::string::npos ? open : contents.find( '}', open );
		if ( close == std::string::npos )
			return foundation::MakeUnexpected(
			    Fail( "invalid-input", *file + " has no " + block + " block" ) );
		const std::string staged =
		    contents.substr( 0, start ) + body + contents.substr( close + 1 );
		if ( staged == ReadBytes( path ) )
			return {}; // keep the modification time a device sync compares
		auto written = WriteAtomic( path, staged );
		if ( !written )
			return written;
		m_Owned.entries[*file] = "file:" + HashHex( staged );
		m_Owned.roles[*file] = "config";
		return {};
	}

	static std::vector<std::string> Directories( const fs::path &directory )
	{
		std::vector<std::string> names;
		std::error_code ec;
		for ( auto it = fs::directory_iterator( directory, ec );
		    !ec && it != fs::directory_iterator(); it.increment( ec ) )
		{
			std::error_code inner;
			if ( it->is_directory( inner ) )
				names.push_back( it->path().filename().string() );
		}
		std::sort( names.begin(), names.end() );
		return names;
	}

	foundation::Expected<void, ProviderError> Extract( const Value &step )
	{
		const std::string *from = step.FindString( "from" );
		const std::string *archive = step.FindString( "archive" );
		const std::string *entry = step.FindString( "entry" );
		const std::string *path = step.FindString( "path" );
		if ( !from || !archive || !entry || !path )
			return foundation::MakeUnexpected(
			    Fail( "invalid-step", "extract needs from, archive, entry and path" ) );
		std::error_code ec;
		if ( fs::is_regular_file( m_Runtime / *path, ec ) )
			return {}; // extracted before
		const fs::path base = Location( *from );
		if ( base.empty() )
			return foundation::MakeUnexpected( Fail(
			    std::string( kUnavailable ), "content locator \"" + *from + "\" is missing" ) );
		content::FileByteSource source;
		std::string error;
		auto vpk = content::VpkArchive::Open( source, ( base / *archive ).string(), error );
		std::string bytes;
		if ( !vpk || !vpk->Read( *entry, bytes ) || bytes.size() < 4 )
			return foundation::MakeUnexpected( Fail( "invalid-input",
			    "cannot extract " + *entry + " from " + ( base / *archive ).string() +
			        ( error.empty() ? "" : ": " + error ) ) );
		auto written = WriteAtomic( m_Runtime / *path, bytes );
		if ( !written )
			return written;
		m_Owned.entries[*path] = "file:" + HashHex( bytes );
		m_Owned.roles[*path] = "content";
		return {};
	}

	const PackageRequest &m_Request;
	fs::path m_Runtime;
	Owned m_Owned;
};

class LinuxDirPackager final : public IPackager
{
public:
	std::string_view Name() const noexcept override { return kLinuxDirPackager; }

	foundation::Expected<PackageManifest, ProviderError> Package(
	    const PackageRequest &request ) override
	{
		if ( request.cancel && request.cancel->IsCancelled() )
			return foundation::MakeUnexpected( Fail( std::string( kCancelled ), "" ) );
		if ( !request.profile )
			return foundation::MakeUnexpected( Fail( "invalid-request", "no profile" ) );
		const Value *package = request.profile->document.Find( "package" );
		const Value *steps = package ? package->Find( "steps" ) : nullptr;
		if ( !steps || !steps->IsArray() || steps->Items().empty() )
			return foundation::MakeUnexpected(
			    Fail( "invalid-request", "package.steps lists the linux-dir steps" ) );
		Builder builder( request, request.output );
		auto built = builder.Run( *steps );
		if ( !built )
			return foundation::MakeUnexpected( built.Error() );
		return builder.Manifest();
	}
};

bool GlobFrom( std::string_view pattern, std::string_view path )
{
	while ( !pattern.empty() )
	{
		if ( pattern.substr( 0, 2 ) == "**" )
		{
			pattern.remove_prefix( 2 );
			for ( size_t i = 0; i <= path.size(); ++i )
			{
				if ( GlobFrom( pattern, path.substr( i ) ) )
					return true;
			}
			return false;
		}
		if ( pattern.front() == '*' )
		{
			pattern.remove_prefix( 1 );
			for ( size_t i = 0; i <= path.size(); ++i )
			{
				if ( GlobFrom( pattern, path.substr( i ) ) )
					return true;
				if ( i < path.size() && path[i] == '/' )
					return false;
			}
			return false;
		}
		if ( path.empty() || pattern.front() != path.front() )
			return false;
		pattern.remove_prefix( 1 );
		path.remove_prefix( 1 );
	}
	return path.empty();
}

} // namespace

bool GlobMatch( std::string_view pattern, std::string_view path )
{
	return GlobFrom( pattern, path );
}

std::unique_ptr<IPackager> CreateLinuxDirPackager()
{
	return std::make_unique<LinuxDirPackager>();
}

} // namespace product
