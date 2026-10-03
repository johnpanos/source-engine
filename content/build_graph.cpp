//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: One serial action key, store and atomic loose-package publication.
//
//=============================================================================//

#include "content/build_graph.h"
#include "content/hash.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>

namespace content
{
namespace
{
namespace fs = std::filesystem;

std::array<std::uint8_t, 16> Hash( std::span<const std::uint8_t> bytes )
{
	std::array<std::uint8_t, 16> result{};
	Blake2b hash( result.size() );
	hash.Update( bytes.data(), bytes.size() );
	hash.Final( result.data() );
	return result;
}
std::string Hex( std::span<const std::uint8_t> bytes )
{
	static constexpr char digits[] = "0123456789abcdef";
	std::string text;
	for ( std::uint8_t byte : bytes )
	{
		text += digits[byte >> 4];
		text += digits[byte & 15];
	}
	return text;
}
void HashText( Blake2b &hash, std::string_view text )
{
	const std::uint64_t length = text.size();
	std::uint8_t size[8];
	for ( int i = 0; i < 8; ++i )
		size[i] = std::uint8_t( length >> ( i * 8 ) );
	hash.Update( size, sizeof( size ) );
	hash.Update( text.data(), text.size() );
}
std::array<std::uint8_t, 16> ActionKey( const IAssetCompiler &compiler, const AssetRef &ref,
    const BuildInputs &inputs, std::string_view profile,
    const std::array<std::uint8_t, 16> &toolDigest )
{
	std::array<std::uint8_t, 16> result{};
	Blake2b hash( result.size() );
	HashText( hash, compiler.Id() );
	HashText( hash, std::to_string( compiler.Version() ) );
	hash.Update( toolDigest.data(), toolDigest.size() );
	HashText( hash, AssetKindName( ref.kind ) );
	HashText( hash, ref.name );
	for ( const auto &[name, value] : compiler.ProfileFacts( profile ) )
	{
		HashText( hash, name );
		HashText( hash, value );
	}
	for ( const auto &[path, data] : inputs.Files() )
	{
		HashText( hash, path );
		const auto digest = Hash( data );
		hash.Update( digest.data(), digest.size() );
	}
	hash.Final( result.data() );
	return result;
}
bool ReadAll( const fs::path &path, std::vector<std::uint8_t> &out )
{
	std::ifstream file( path, std::ios::binary );
	if ( !file )
		return false;
	out.assign( std::istreambuf_iterator<char>{ file }, {} );
	return file.eof() || file.good();
}
bool WriteFile( const fs::path &path, std::span<const std::uint8_t> data )
{
	std::ofstream file( path, std::ios::binary | std::ios::trunc );
	if ( !file )
		return false;
	file.write( reinterpret_cast<const char *>( data.data() ), data.size() );
	file.flush();
	return file.good();
}
std::string UniqueSuffix()
{
	static std::atomic<std::uint64_t> next{ 0 };
	return std::to_string( std::chrono::steady_clock::now().time_since_epoch().count() ) + "-" +
	       std::to_string( next.fetch_add( 1, std::memory_order_relaxed ) );
}
std::optional<fs::path> CreateStagingDirectory( const fs::path &parent, std::string_view prefix )
{
	for ( int attempt = 0; attempt < 16; ++attempt )
	{
		const fs::path candidate = parent / ( std::string( prefix ) + UniqueSuffix() );
		std::error_code ec;
		if ( fs::create_directory( candidate, ec ) )
			return candidate;
		if ( ec )
			return std::nullopt;
	}
	return std::nullopt;
}
bool WriteAtomic( const fs::path &path, std::span<const std::uint8_t> data )
{
	std::error_code ec;
	fs::create_directories( path.parent_path(), ec );
	if ( ec )
		return false;
	const auto staging = CreateStagingDirectory( path.parent_path(), ".partial-" );
	if ( !staging )
		return false;
	const fs::path partial = *staging / path.filename();
	if ( !WriteFile( partial, data ) )
	{
		fs::remove_all( *staging, ec );
		return false;
	}
	fs::rename( partial, path, ec );
	const bool committed = !ec;
	fs::remove_all( *staging, ec );
	return committed;
}
std::vector<std::uint8_t> Bytes( std::string_view text )
{
	return { text.begin(), text.end() };
}
} // namespace

BuildInputs::BuildInputs( fs::path root ) : m_Root( fs::weakly_canonical( root ) )
{
}

std::optional<std::span<const std::uint8_t>> BuildInputs::Read(
    std::string_view relative, std::string *error )
{
	auto name = NormalizeAssetName( relative );
	if ( !name )
	{
		if ( error )
			*error = "invalid input path";
		return std::nullopt;
	}
	if ( auto found = m_Files.find( *name ); found != m_Files.end() )
		return found->second;
	if ( m_Frozen )
	{
		if ( error )
			*error = "undeclared compiler input: " + *name;
		return std::nullopt;
	}
	const fs::path source = m_Root / *name;
	std::error_code ec;
	const fs::path canonical = fs::weakly_canonical( source, ec );
	const fs::path relativePath = ec ? fs::path{} : canonical.lexically_relative( m_Root );
	if ( ec || relativePath.empty() || *relativePath.begin() == ".." )
	{
		if ( error )
			*error = "input escapes content root: " + *name;
		return std::nullopt;
	}
	std::vector<std::uint8_t> data;
	if ( !ReadAll( canonical, data ) )
	{
		if ( error )
			*error = "missing compiler input: " + *name;
		return std::nullopt;
	}
	auto [it, inserted] = m_Files.emplace( *name, std::move( data ) );
	return it->second;
}

std::optional<std::vector<AssetEdge>> TexturePassthroughCompiler::Plan(
    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const
{
	if ( ref.kind != Kind() || !inputs.Read( ref.RuntimePath(), &error ) )
		return std::nullopt;
	return std::vector<AssetEdge>{};
}

std::optional<std::vector<std::uint8_t>> TexturePassthroughCompiler::Compile(
    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const
{
	auto source = inputs.Read( ref.RuntimePath(), &error );
	if ( !source )
		return std::nullopt;
	const auto data = *source;
	if ( data.size() < 16 || data[0] != 'V' || data[1] != 'T' || data[2] != 'F' || data[3] != 0 )
	{
		error = "invalid VTF: " + ref.name;
		return std::nullopt;
	}
	const std::uint32_t header = std::uint32_t( data[12] ) | ( std::uint32_t( data[13] ) << 8 ) |
	                             ( std::uint32_t( data[14] ) << 16 ) |
	                             ( std::uint32_t( data[15] ) << 24 );
	if ( header < 16 || header > data.size() )
	{
		error = "invalid VTF header size: " + ref.name;
		return std::nullopt;
	}
	return std::vector<std::uint8_t>( data.begin(), data.end() );
}

BuildGraph::BuildGraph(
    fs::path source, fs::path output, std::string profile, std::array<std::uint8_t, 16> toolDigest )
    : m_Source( std::move( source ) ), m_Output( std::move( output ) ),
      m_Profile( std::move( profile ) ), m_ToolDigest( toolDigest )
{
}

void BuildGraph::Register( const IAssetCompiler &compiler )
{
	m_Compilers.insert_or_assign( compiler.Kind(), &compiler );
}

std::optional<BuildResult> BuildGraph::Build(
    std::span<const AssetRef> roots, std::string_view packageName, std::string &error )
{
	const auto package = NormalizeAssetName( packageName, true );
	const auto profile = NormalizeAssetName( m_Profile, true );
	if ( !package || package->find( '/' ) != std::string::npos || !profile ||
	     profile->find( '/' ) != std::string::npos || roots.empty() )
	{
		error = "invalid package request";
		return std::nullopt;
	}
	const fs::path base = m_Output / m_Profile;
	std::vector<AssetRef> pending( roots.begin(), roots.end() );
	std::map<AssetRef, AssetEntry> entries;
	std::vector<AssetEdge> edges;
	BuildResult build;
	for ( std::size_t cursor = 0; cursor < pending.size(); ++cursor )
	{
		const AssetRef ref = pending[cursor];
		if ( entries.contains( ref ) )
			continue;
		auto compiler = m_Compilers.find( ref.kind );
		if ( compiler == m_Compilers.end() )
		{
			error = "no compiler for " + std::string( AssetKindName( ref.kind ) ) + ':' + ref.name;
			return std::nullopt;
		}
		BuildInputs inputs( m_Source );
		auto references = compiler->second->Plan( ref, inputs, error );
		if ( !references )
		{
			for ( const AssetEdge &edge : edges )
			{
				if ( edge.target == ref )
				{
					error += " required by " + std::string( AssetKindName( edge.source.kind ) ) +
					         ':' + edge.source.name;
					break;
				}
			}
			return std::nullopt;
		}
		inputs.Freeze();
		const auto key = ActionKey( *compiler->second, ref, inputs, m_Profile, m_ToolDigest );
		const std::string keyText = Hex( key );
		const fs::path action = base / "actions" / ( keyText + ".act" );
		std::vector<std::uint8_t> record;
		std::array<std::uint8_t, 16> outputHash{};
		std::uint64_t outputSize = 0;
		bool hit = false;
		if ( ReadAll( action, record ) && record.size() == 28 &&
		     std::equal( record.begin(), record.begin() + 4, "ACT1" ) )
		{
			std::copy_n( record.begin() + 4, 16, outputHash.begin() );
			for ( int i = 0; i < 8; ++i )
				outputSize |= std::uint64_t( record[20 + i] ) << ( i * 8 );
			std::vector<std::uint8_t> stored;
			hit = ReadAll( base / "store" / Hex( outputHash ), stored ) &&
			      stored.size() == outputSize && Hash( stored ) == outputHash;
		}
		if ( !hit )
		{
			auto bytes = compiler->second->Compile( ref, inputs, error );
			if ( !bytes )
				return std::nullopt;
			outputHash = Hash( *bytes );
			outputSize = bytes->size();
			const fs::path blob = base / "store" / Hex( outputHash );
			if ( !WriteAtomic( blob, *bytes ) )
			{
				error = "store write failed: " + blob.string();
				return std::nullopt;
			}
			record = Bytes( "ACT1" );
			record.insert( record.end(), outputHash.begin(), outputHash.end() );
			for ( int i = 0; i < 8; ++i )
				record.push_back( std::uint8_t( outputSize >> ( i * 8 ) ) );
			if ( !WriteAtomic( action, record ) )
			{
				error = "action write failed: " + action.string();
				return std::nullopt;
			}
		}
		entries.emplace(
		    ref, AssetEntry{ ref, "legacy", ref.RuntimePath(),
		             std::string( compiler->second->Id() ), keyText, outputHash, outputSize } );
		BuildTraceNode trace{ ref, keyText, hit, {} };
		for ( const auto &[path, data] : inputs.Files() )
			trace.inputs.push_back( path );
		build.trace.push_back( std::move( trace ) );
		for ( const AssetEdge &edge : *references )
		{
			if ( !( edge.source == ref ) )
			{
				error = "compiler reported an edge for another asset";
				return std::nullopt;
			}
			edges.push_back( edge );
			if ( !edge.optional && !entries.contains( edge.target ) )
				pending.push_back( edge.target );
		}
	}
	std::vector<AssetEntry> indexEntries;
	for ( const auto &[ref, entry] : entries )
		indexEntries.push_back( entry );
	const auto index = AssetIndex::Write( std::move( indexEntries ), edges, &error );
	if ( !index )
		return std::nullopt;
	Blake2b versionHash( 16 );
	HashText( versionHash, *package );
	HashText( versionHash, m_Profile );
	versionHash.Update( index->data(), index->size() );
	std::array<std::uint8_t, 16> version{};
	versionHash.Final( version.data() );
	build.version = Hex( version );
	const fs::path packageBase = base / "packages" / *package;
	build.package = packageBase / build.version;
	std::error_code ec;
	fs::create_directories( packageBase, ec );
	if ( ec )
	{
		error = "package root creation failed";
		return std::nullopt;
	}
	if ( !fs::exists( build.package ) )
	{
		const auto staging = CreateStagingDirectory( packageBase, ".package-" );
		if ( !staging )
		{
			error = "package staging failed";
			return std::nullopt;
		}
		bool ready = true;
		for ( const auto &[ref, entry] : entries )
		{
			const fs::path destination = *staging / entry.location;
			fs::create_directories( destination.parent_path(), ec );
			if ( ec )
			{
				ready = false;
				break;
			}
			fs::copy_file( base / "store" / Hex( entry.hash ), destination, ec );
			if ( ec )
			{
				ready = false;
				break;
			}
		}
		if ( ready )
			ready = WriteFile( *staging / "assets.index", *index );
		if ( ready )
			fs::rename( *staging, build.package, ec );
		if ( ec && fs::exists( build.package ) )
		{
			std::vector<std::uint8_t> existing;
			if ( ReadAll( build.package / "assets.index", existing ) && existing == *index )
				ec.clear();
		}
		if ( !ready || ec )
		{
			fs::remove_all( *staging, ec );
			error = "package staging or commit failed";
			return std::nullopt;
		}
		fs::remove_all( *staging, ec );
	}
	else
	{
		std::vector<std::uint8_t> existing;
		if ( !ReadAll( build.package / "assets.index", existing ) || existing != *index )
		{
			error = "existing package version differs from its index";
			return std::nullopt;
		}
	}
	const fs::path next = packageBase / ( ".current-" + UniqueSuffix() );
	fs::create_directory_symlink( build.version, next, ec );
	if ( !ec )
		fs::rename( next, packageBase / "current", ec );
	if ( ec )
	{
		fs::remove( next, ec );
		error = "package publication failed";
		return std::nullopt;
	}
	return build;
}

std::optional<std::array<std::uint8_t, 16>> FileDigest( const fs::path &path, std::string &error )
{
	std::vector<std::uint8_t> bytes;
	if ( !ReadAll( path, bytes ) )
	{
		error = "compiler identity binary is missing: " + path.string();
		return std::nullopt;
	}
	return Hash( bytes );
}

} // namespace content
