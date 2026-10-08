//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.package.linux-dir: the `linux-dir` packager (RFC 0027 L1).
//
//=============================================================================//

#include "product/package_linux_dir.h"

#include "content/vpk_archive.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
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
	// Entries a mount set's steps added: path -> set. When a package runs
	// without the set, they are removed, so the runtime matches the request.
	std::map<std::string, std::string> sets;
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
			const std::string *set = step.FindString( "mount_set" );
			if ( set && std::find( m_Request.mountSets.begin(), m_Request.mountSets.end(), *set ) ==
			                m_Request.mountSets.end() )
			{
				// A mount set's step runs only when the set is selected; what
				// it placed on an earlier run goes.
				RemoveSet( *set );
				continue;
			}
			m_CurrentSet = set ? *set : std::string();
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
			else if ( *op == "extract-packs" )
				done = ExtractPacks( step );
			else
				return foundation::MakeUnexpected(
				    Fail( "invalid-step", "unknown package op \"" + *op + "\"" ) );
			if ( !done )
			{
				// What earlier steps placed stays owned (a later run skips a
				// finished seed), so the record is kept even on failure.
				(void)SaveRecord();
				return done;
			}
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

	// Every placed entry is owned through here: its hash or link, its role,
	// and the mount set of the step placing it (none for an unconditional
	// step, which also clears an earlier set's claim).
	void Own( const std::string &relative, std::string value, std::string role )
	{
		m_Owned.entries[relative] = std::move( value );
		m_Owned.roles[relative] = std::move( role );
		if ( m_CurrentSet.empty() )
			m_Owned.sets.erase( relative );
		else
			m_Owned.sets[relative] = m_CurrentSet;
	}

	void RemoveSet( const std::string &set )
	{
		std::error_code ec;
		for ( auto it = m_Owned.sets.begin(); it != m_Owned.sets.end(); )
		{
			if ( it->second != set )
			{
				++it;
				continue;
			}
			fs::remove( m_Runtime / it->first, ec );
			// Directories the set's entries emptied go too (never the runtime).
			for ( fs::path parent = ( m_Runtime / it->first ).parent_path();
			    parent != m_Runtime && parent.string().size() > m_Runtime.string().size();
			    parent = parent.parent_path() )
			{
				if ( !fs::is_directory( parent, ec ) || fs::is_symlink( parent, ec ) ||
				     !fs::is_empty( parent, ec ) || !fs::remove( parent, ec ) )
					break;
			}
			m_Owned.entries.erase( it->first );
			m_Owned.roles.erase( it->first );
			it = m_Owned.sets.erase( it );
		}
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
					if ( const std::string *set = member.second.FindString( "mount_set" ) )
						m_Owned.sets[member.first] = *set;
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
			auto set = m_Owned.sets.find( entry.first );
			if ( set != m_Owned.sets.end() )
				item.Set( "mount_set", Value::String( set->second ) );
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
		Own( relative, "file:" + HashHex( ReadBytes( target ) ), role );
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
			Own( relative, "link:" + target.string(), role );
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
		Own( relative, "link:" + target.string(), role );
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
			const bool link = it->is_symlink( inner );
			if ( !link && !it->is_regular_file( inner ) )
				continue;
			// Lexical: a link's own path, not its target's.
			const std::string relative = it->path().lexically_relative( root ).generic_string();
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
			// The artifact's links stay links (a content stage's mirrors).
			if ( fs::is_symlink( root / relative, ec ) )
			{
				auto linked =
				    LinkEntry( fs::read_symlink( root / relative, ec ), relative, "content" );
				if ( !linked )
					return linked;
				continue;
			}
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
			if ( const std::string *set = line.FindString( "mount_set" ) )
			{
				if ( std::find( m_Request.mountSets.begin(), m_Request.mountSets.end(), *set ) ==
				     m_Request.mountSets.end() )
					continue;
			}
			if ( const std::string *record = line.FindString( "each_record" ) )
			{
				// One line per pack of an extraction record, in its order.
				auto parsed = foundation::json::Parse( ReadBytes( m_Runtime / *record ) );
				const Value *packs =
				    parsed && parsed.Value().IsObject() ? parsed.Value().Find( "packs" ) : nullptr;
				for ( const Value &pack :
				    packs && packs->IsArray() ? packs->Items() : std::vector<Value>{} )
				{
					if ( const std::string *id = pack.FindString( "id" ) )
						body += "\n" + indent + "\t" + Replace( *text, "{item}", *id );
				}
				continue;
			}
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
		Own( *file, "file:" + HashHex( staged ), "config" );
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
		Own( *path, "file:" + HashHex( bytes ), "content" );
		return {};
	}

	// A model is taken whole from one pack: its files share a unit.
	static std::optional<std::string> ModelUnit( const std::string &path )
	{
		if ( path.rfind( "models/", 0 ) != 0 )
			return std::nullopt;
		for ( const char *suffix :
		    { ".dx90.vtx", ".dx80.vtx", ".sw.vtx", ".vtx", ".mdl", ".vvd", ".phy", ".ani" } )
		{
			const std::string ending( suffix );
			if ( path.size() >= ending.size() &&
			     path.compare( path.size() - ending.size(), ending.size(), ending ) == 0 )
				return path.substr( 0, path.size() - ending.size() );
		}
		return std::nullopt;
	}

	// The VPHY version of each solid of a .phy file (-1 for a headerless one).
	static std::optional<std::vector<int>> CollisionVersions( const std::string &data )
	{
		const auto i32 = [&]( size_t at )
		{
			std::uint32_t v = 0;
			std::memcpy( &v, data.data() + at, 4 );
			return static_cast<std::int32_t>( v );
		};
		if ( data.size() < 16 )
			return std::nullopt;
		const std::int32_t headerSize = i32( 0 ), solids = i32( 8 );
		size_t position = static_cast<size_t>( headerSize );
		std::vector<int> versions;
		for ( std::int32_t i = 0; i < solids; ++i )
		{
			if ( position + 12 > data.size() )
				return std::nullopt;
			const std::int32_t size = i32( position );
			const bool vphy = data.compare( position + 4, 4, "VPHY" ) == 0;
			std::uint16_t version = 0;
			std::memcpy( &version, data.data() + position + 8, 2 );
			versions.push_back( vphy ? static_cast<std::int16_t>( version ) : -1 );
			position += 4 + static_cast<size_t>( size );
		}
		return versions;
	}

	static bool LowerEquals( const std::string &text, size_t at, const std::string &lowerWord )
	{
		if ( at + lowerWord.size() > text.size() )
			return false;
		for ( size_t i = 0; i < lowerWord.size(); ++i )
		{
			if ( std::tolower( static_cast<unsigned char>( text[at + i] ) ) !=
			     static_cast<unsigned char>( lowerWord[i] ) )
				return false;
		}
		return true;
	}

	// drop_vmt_keys: remove the lines ^[ \t]*"?KEY"?[ \t][^\n]*\n (any case).
	static foundation::Expected<std::string, ProviderError> DropKeys(
	    const std::string &path, std::string data, const Value *drops )
	{
		for ( const Value &drop :
		    drops && drops->IsArray() ? drops->Items() : std::vector<Value>{} )
		{
			const std::string *dropPath = drop.FindString( "path" );
			const std::string *key = drop.FindString( "key" );
			if ( !dropPath || !key || Lower( *dropPath ) != path )
				continue;
			const std::string lowerKey = Lower( *key );
			std::string out;
			int removed = 0;
			size_t start = 0;
			while ( start < data.size() )
			{
				const size_t end = data.find( '\n', start );
				if ( end == std::string::npos )
				{
					out += data.substr( start );
					break;
				}
				size_t i = start;
				while ( i < end && ( data[i] == ' ' || data[i] == '\t' ) )
					++i;
				if ( i < end && data[i] == '"' && !LowerEquals( data, i, lowerKey ) )
					++i;
				bool match = LowerEquals( data, i, lowerKey );
				if ( match )
				{
					size_t j = i + lowerKey.size();
					if ( j < end && data[j] == '"' )
						++j;
					match = j < end && ( data[j] == ' ' || data[j] == '\t' );
				}
				if ( match )
					++removed;
				else
					out += data.substr( start, end + 1 - start );
				start = end + 1;
			}
			if ( !removed )
				return foundation::MakeUnexpected(
				    Fail( "invalid-input", path + " has no " + *key + " to drop" ) );
			data = std::move( out );
		}
		return data;
	}

	// Replace every case-insensitive occurrence of `from` followed by '"' or
	// white space (Python's (?=["\s]) lookahead).
	static std::string ReplaceBeforeQuoteOrSpace(
	    const std::string &text, const std::string &from, const std::string &to )
	{
		const std::string lowerFrom = Lower( from );
		std::string out;
		size_t i = 0;
		while ( i < text.size() )
		{
			if ( LowerEquals( text, i, lowerFrom ) && i + from.size() < text.size() )
			{
				const char next = text[i + from.size()];
				if ( next == '"' || next == ' ' || next == '\t' || next == '\n' || next == '\r' ||
				     next == '\f' || next == '\v' )
				{
					out += to;
					i += from.size();
					continue;
				}
			}
			out += text[i++];
		}
		return out;
	}

	static std::pair<std::string, int> ReplaceAllNoCase(
	    const std::string &text, const std::string &from, const std::string &to )
	{
		const std::string lowerFrom = Lower( from );
		std::string out;
		int count = 0;
		size_t i = 0;
		while ( i < text.size() )
		{
			if ( !lowerFrom.empty() && LowerEquals( text, i, lowerFrom ) )
			{
				out += to;
				i += from.size();
				++count;
				continue;
			}
			out += text[i++];
		}
		return { out, count };
	}

	// namespaced: move a pack's materials from namespace.from to namespace.to.
	static foundation::Expected<std::pair<std::string, std::string>, ProviderError> Namespaced(
	    std::string path, std::string data, const Value &space,
	    const std::vector<std::string> &files )
	{
		const std::string *from = space.FindString( "from" );
		const std::string *to = space.FindString( "to" );
		if ( !from || !to || from->size() != to->size() )
			return foundation::MakeUnexpected(
			    Fail( "invalid-input", "a namespace move keeps the name length" ) );
		const std::string materials = "materials/" + *from;
		if ( path.rfind( materials, 0 ) == 0 )
		{
			path = "materials/" + *to + path.substr( materials.size() );
			if ( path.size() >= 4 && path.compare( path.size() - 4, 4, ".vmt" ) == 0 )
			{
				std::vector<std::string> refs;
				for ( const std::string &file : files )
				{
					if ( file.rfind( materials, 0 ) == 0 && file.size() > 4 &&
					     file.compare( file.size() - 4, 4, ".vtf" ) == 0 )
					{
						const std::string ref = file.substr( 10, file.size() - 14 );
						if ( std::find( refs.begin(), refs.end(), ref ) == refs.end() )
							refs.push_back( ref );
					}
				}
				std::stable_sort( refs.begin(), refs.end(),
				    []( const std::string &a, const std::string &b )
				    {
					    return a.size() > b.size();
				    } );
				for ( const std::string &ref : refs )
				{
					const std::string moved = *to + ref.substr( from->size() );
					data = ReplaceBeforeQuoteOrSpace( data, ref, moved );
					data = ReplaceBeforeQuoteOrSpace( data, Replace( ref, "/", "\\" ), moved );
				}
			}
		}
		else if ( path.size() >= 4 && path.compare( path.size() - 4, 4, ".mdl" ) == 0 )
		{
			int replaced = 0;
			for ( const auto &[a, b] : { std::pair{ *from, *to },
			          std::pair{ Replace( *from, "/", "\\" ), Replace( *to, "/", "\\" ) } } )
			{
				auto [text, count] = ReplaceAllNoCase( data, a, b );
				data = std::move( text );
				replaced += count;
			}
			if ( !replaced )
				return foundation::MakeUnexpected(
				    Fail( "invalid-input", path + " has no $cdmaterials " + *from + " to move" ) );
		}
		return std::pair{ path, data };
	}

	static long long MtimeNs( const fs::path &path )
	{
		std::error_code ec;
		const auto time = fs::last_write_time( path, ec );
		const auto system = std::chrono::file_clock::to_sys( time );
		return static_cast<long long>(
		    std::chrono::duration_cast<std::chrono::nanoseconds>( system.time_since_epoch() )
		        .count() );
	}

	// extract-packs: the manifest's VPK packs, filtered and rewritten, into
	// <into>/<id>, with a stamp per pack and <into>/mounts.json, the record
	// of what each pack supplies (the retired stage_portal2_runtime.stage_workshop's).
	foundation::Expected<void, ProviderError> ExtractPacks( const Value &step )
	{
		const std::string *from = step.FindString( "from" );
		const std::string *manifestName = step.FindString( "manifest" );
		const std::string *into = step.FindString( "into" );
		if ( !from || !manifestName || !into )
			return foundation::MakeUnexpected(
			    Fail( "invalid-step", "extract-packs needs from, manifest and into" ) );
		const fs::path manifestPath = m_Request.sourceRoot / *manifestName;
		auto parsed = foundation::json::Parse( ReadBytes( manifestPath ) );
		if ( !parsed || !parsed.Value().IsObject() )
			return foundation::MakeUnexpected(
			    Fail( "invalid-input", "cannot read " + manifestPath.string() ) );
		const Value &manifest = parsed.Value();
		const std::string *format = step.FindString( "format" );
		const std::string *actual = manifest.FindString( "format" );
		if ( format && ( !actual || *actual != *format ) )
			return foundation::MakeUnexpected( Fail(
			    "invalid-input", manifestPath.string() + " is not a " + *format + " manifest" ) );
		const Value *packs = manifest.Find( "packs" );
		const fs::path root = Location( *from );
		const fs::path base = m_Runtime / *into;
		content::FileByteSource source;
		std::map<std::string, std::unique_ptr<content::VpkArchive>> archives;
		std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>
		    chosen;
		Value missing = Value::Object();
		std::map<std::string, std::string> owner;
		std::error_code ec;
		for ( const Value &pack :
		    packs && packs->IsArray() ? packs->Items() : std::vector<Value>{} )
		{
			const std::string id =
			    pack.FindString( "id" ) ? *pack.FindString( "id" ) : std::string();
			const fs::path directory = root / id;
			std::string error;
			if ( !root.empty() && fs::is_regular_file( directory / "pak01_dir.vpk", ec ) )
				archives[id] = content::VpkArchive::Open(
				    source, ( directory / "pak01_dir.vpk" ).string(), error );
			if ( !archives[id] )
			{
				missing.Set( id, Value::String( "not installed" ) );
				continue;
			}
			std::vector<std::string> include, exclude;
			for ( const Value &item :
			    pack.Find( "include" ) ? pack.Find( "include" )->Items() : std::vector<Value>{} )
				include.push_back( item.Text() );
			for ( const Value &item :
			    pack.Find( "exclude" ) ? pack.Find( "exclude" )->Items() : std::vector<Value>{} )
				exclude.push_back( item.Text() );
			std::vector<std::pair<std::string, std::string>> files;
			for ( const content::VpkEntry &entry : archives[id]->Entries() )
			{
				const std::string path = Replace( Lower( entry.path ), "\\", "/" );
				const auto starts = [&]( const std::vector<std::string> &prefixes )
				{
					return std::any_of( prefixes.begin(), prefixes.end(),
					    [&]( const std::string &p )
					    {
						    return path.rfind( p, 0 ) == 0;
					    } );
				};
				if ( !starts( include ) || starts( exclude ) ||
				     std::find( exclude.begin(), exclude.end(), path ) != exclude.end() )
					continue;
				if ( auto unit = ModelUnit( path ) )
				{
					auto [it, inserted] = owner.emplace( *unit, id );
					if ( it->second != id )
						continue;
				}
				auto existing = std::find_if( files.begin(), files.end(),
				    [&]( const auto &f )
				    {
					    return f.first == path;
				    } );
				if ( existing != files.end() )
					existing->second = entry.path;
				else
					files.emplace_back( path, entry.path );
			}
			chosen.emplace_back( id, std::move( files ) );
		}
		for ( const auto &[id, files] : chosen )
		{
			for ( const auto &[path, name] : files )
			{
				if ( path.size() < 4 || path.compare( path.size() - 4, 4, ".phy" ) != 0 )
					continue;
				std::string data;
				archives[id]->Read( name, data );
				auto versions = CollisionVersions( data );
				if ( !versions )
					return foundation::MakeUnexpected( Fail( "invalid-input",
					    "workshop " + id + " " + path + ": truncated collision data" ) );
				for ( int version : *versions )
				{
					if ( version != -1 && version != 0x100 )
						return foundation::MakeUnexpected( Fail( "invalid-input",
						    "workshop " + id + " " + path +
						        ": collision version is not loadable; exclude the model in " +
						        manifestPath.filename().string() ) );
				}
			}
		}
		fs::create_directories( base, ec );
		Value report = Value::Object();
		report.Set( "format", Value::String( "p2ce-workshop-mount-record/v1" ) );
		report.Set( "manifest", Value::String( manifestPath.string() ) );
		// Filled below and set last, in the record's member order.
		Value reportPacks = Value::Array();
		std::vector<std::string> mounted;
		for ( const Value &pack : packs->Items() )
		{
			const std::string id = *pack.FindString( "id" );
			auto picked = std::find_if( chosen.begin(), chosen.end(),
			    [&]( const auto &c )
			    {
				    return c.first == id;
			    } );
			if ( picked == chosen.end() )
				continue;
			const auto &files = picked->second;
			const fs::path sourceDirectory = root / id;
			Value stamp = Value::Object();
			stamp.Set(
			    "namespace", pack.Find( "namespace" ) ? *pack.Find( "namespace" ) : Value() );
			stamp.Set(
			    "drop_keys", pack.Find( "drop_keys" ) ? *pack.Find( "drop_keys" ) : Value() );
			Value &stampArchives = stamp.Set( "archives", Value::Object() );
			std::vector<std::string> archiveNames;
			for ( auto it = fs::directory_iterator( sourceDirectory, ec );
			    !ec && it != fs::directory_iterator(); it.increment( ec ) )
			{
				const std::string name = it->path().filename().string();
				if ( name.rfind( "pak01_", 0 ) == 0 && it->path().extension() == ".vpk" )
					archiveNames.push_back( name );
			}
			std::sort( archiveNames.begin(), archiveNames.end() );
			for ( const std::string &name : archiveNames )
			{
				Value pair = Value::Array();
				pair.Push( Value::Number(
				    static_cast<long long>( fs::file_size( sourceDirectory / name, ec ) ) ) );
				pair.Push( Value::Number( MtimeNs( sourceDirectory / name ) ) );
				stampArchives.Set( name, std::move( pair ) );
			}
			std::vector<std::string> paths;
			for ( const auto &file : files )
				paths.push_back( file.first );
			std::sort( paths.begin(), paths.end() );
			Value &stampFiles = stamp.Set( "files", Value::Array() );
			for ( const std::string &path : paths )
				stampFiles.Push( Value::String( path ) );
			const fs::path target = base / id;
			const fs::path stampFile = target / ".workshop-source.json";
			auto previous = foundation::json::Parse( ReadBytes( stampFile ) );
			if ( !previous || !( previous.Value() == stamp ) )
			{
				fs::remove_all( target, ec );
				const std::string prefix = *into + "/" + id + "/";
				for ( auto it = m_Owned.entries.begin(); it != m_Owned.entries.end(); )
				{
					if ( it->first.rfind( prefix, 0 ) == 0 )
					{
						m_Owned.roles.erase( it->first );
						it = m_Owned.entries.erase( it );
					}
					else
						++it;
				}
				const Value *space = pack.Find( "namespace" );
				for ( const auto &[path, name] : files )
				{
					std::string data;
					archives[id]->Read( name, data );
					auto dropped = DropKeys( path, data, pack.Find( "drop_keys" ) );
					if ( !dropped )
						return foundation::MakeUnexpected( dropped.Error() );
					std::string outPath = path;
					std::string outData = std::move( dropped ).Value();
					if ( space && space->IsObject() )
					{
						auto moved = Namespaced( outPath, outData, *space, paths );
						if ( !moved )
							return foundation::MakeUnexpected( moved.Error() );
						outPath = moved.Value().first;
						outData = moved.Value().second;
					}
					auto written = WriteAtomic( target / outPath, outData );
					if ( !written )
						return written;
					Own(
					    *into + "/" + id + "/" + outPath, "file:" + HashHex( outData ), "content" );
				}
				fs::create_directories( target, ec );
				const std::string stampText = stamp.WritePretty( 1 ) + "\n";
				auto written = WriteAtomic( stampFile, stampText );
				if ( !written )
					return written;
				Own( prefix + ".workshop-source.json", "file:" + HashHex( stampText ), "config" );
			}
			else
			{
				// Unchanged: its entries stay, re-owned by this step.
				const std::string prefix = *into + "/" + id + "/";
				for ( const auto &entry : std::map<std::string, std::string>( m_Owned.entries ) )
				{
					if ( entry.first.rfind( prefix, 0 ) == 0 )
					{
						auto role = m_Owned.roles.find( entry.first );
						Own( entry.first, entry.second,
						    role == m_Owned.roles.end() ? std::string( "content" ) : role->second );
					}
				}
			}
			Value record = Value::Object();
			record.Set( "id", Value::String( id ) );
			record.Set( "title",
			    Value::String( pack.FindString( "title" ) ? *pack.FindString( "title" ) : "" ) );
			record.Set( "source", Value::String( ( sourceDirectory / "pak01_dir.vpk" ).string() ) );
			record.Set( "files", Value::Number( static_cast<long long>( files.size() ) ) );
			long long materials = 0, models = 0;
			for ( const auto &file : files )
			{
				const std::string &p = file.first;
				materials += p.size() >= 4 && p.compare( p.size() - 4, 4, ".vmt" ) == 0;
				models += p.size() >= 4 && p.compare( p.size() - 4, 4, ".mdl" ) == 0;
			}
			record.Set( "materials", Value::Number( materials ) );
			record.Set( "models", Value::Number( models ) );
			reportPacks.Push( std::move( record ) );
			mounted.push_back( id );
		}
		report.Set( "packs", std::move( reportPacks ) );
		report.Set( "unavailable", missing );
		for ( auto it = fs::directory_iterator( base, ec ); !ec && it != fs::directory_iterator();
		    it.increment( ec ) )
		{
			std::error_code inner;
			const std::string name = it->path().filename().string();
			if ( it->is_directory( inner ) &&
			     std::find( mounted.begin(), mounted.end(), name ) == mounted.end() )
				fs::remove_all( it->path(), inner );
		}
		const std::string text = report.WritePretty( 1 ) + "\n";
		auto written = WriteAtomic( base / "mounts.json", text );
		if ( !written )
			return written;
		Own( *into + "/mounts.json", "file:" + HashHex( text ), "config" );
		return {};
	}

	const PackageRequest &m_Request;
	fs::path m_Runtime;
	Owned m_Owned;
	std::string m_CurrentSet; // the running step's mount set, if any
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
