//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Asset index writer and reader using BSP2's 64/64 block layout.
//
//=============================================================================//

#include "content/asset_index.h"
#include "content/hash.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <tuple>

namespace content
{
namespace
{
constexpr std::uint8_t kMagic[8] = { 'S', 'R', 'C', 'A', 'S', 'I', 'X', 0x1A };
constexpr std::size_t kHeaderSize = 64;
constexpr std::size_t kDirectorySize = 64;
constexpr std::size_t kEntrySize = 46;
constexpr std::size_t kEdgeSize = 16;
constexpr std::uint32_t kFourCC = 0x58444E49; // INDX

void Put16( std::vector<std::uint8_t> &out, std::uint16_t value )
{
	out.push_back( std::uint8_t( value ) );
	out.push_back( std::uint8_t( value >> 8 ) );
}
void Put32( std::vector<std::uint8_t> &out, std::uint32_t value )
{
	for ( int shift = 0; shift < 32; shift += 8 )
		out.push_back( std::uint8_t( value >> shift ) );
}
void Put64( std::vector<std::uint8_t> &out, std::uint64_t value )
{
	for ( int shift = 0; shift < 64; shift += 8 )
		out.push_back( std::uint8_t( value >> shift ) );
}
std::uint16_t Get16( const std::uint8_t *p )
{
	return std::uint16_t( p[0] ) | ( std::uint16_t( p[1] ) << 8 );
}
std::uint32_t Get32( const std::uint8_t *p )
{
	return std::uint32_t( Get16( p ) ) | ( std::uint32_t( Get16( p + 2 ) ) << 16 );
}
std::uint64_t Get64( const std::uint8_t *p )
{
	return std::uint64_t( Get32( p ) ) | ( std::uint64_t( Get32( p + 4 ) ) << 32 );
}
std::array<std::uint8_t, 16> Hash( std::span<const std::uint8_t> bytes )
{
	std::array<std::uint8_t, 16> digest{};
	Blake2b hash( digest.size() );
	hash.Update( bytes.data(), bytes.size() );
	hash.Final( digest.data() );
	return digest;
}
void PutString( std::vector<std::uint8_t> &out, std::string_view text )
{
	out.insert( out.end(), text.begin(), text.end() );
}
bool Fail( std::string *error, const char *message )
{
	if ( error )
		*error = message;
	return false;
}
bool Fits( std::uint64_t offset, std::uint64_t size, std::uint64_t limit )
{
	return offset <= limit && size <= limit - offset;
}
bool TakeString(
    std::span<const std::uint8_t> bytes, std::size_t &cursor, std::size_t size, std::string &out )
{
	if ( !Fits( cursor, size, bytes.size() ) )
		return false;
	out.assign( reinterpret_cast<const char *>( bytes.data() + cursor ), size );
	cursor += size;
	return true;
}

bool ValidEntry( const AssetEntry &entry )
{
	return AssetRef::Create( entry.ref.kind, entry.ref.name ) == entry.ref &&
	       NormalizeAssetName( entry.location ) == entry.location && !entry.variant.empty() &&
	       !entry.compiler.empty() && !entry.key.empty() && entry.ref.name.size() <= 65535 &&
	       entry.variant.size() <= 65535 && entry.location.size() <= 65535 &&
	       entry.compiler.size() <= 65535 && entry.key.size() <= 65535;
}
} // namespace

const AssetEntry *AssetIndex::Find( const AssetRef &ref, std::string_view variant ) const noexcept
{
	for ( const AssetEntry &entry : m_Entries )
		if ( entry.ref == ref && entry.variant == variant )
			return &entry;
	return nullptr;
}

std::vector<AssetEdge> AssetIndex::References( const AssetRef &ref ) const
{
	std::vector<AssetEdge> result;
	for ( const AssetEdge &edge : m_Edges )
		if ( edge.source == ref )
			result.push_back( edge );
	return result;
}

std::optional<std::vector<std::uint8_t>> AssetIndex::Write(
    std::vector<AssetEntry> entries, std::vector<AssetEdge> edges, std::string *error )
{
	std::sort( entries.begin(), entries.end(),
	    []( const auto &a, const auto &b )
	    {
		    return std::tie( a.ref, a.variant ) < std::tie( b.ref, b.variant );
	    } );
	if ( entries.size() > 1000000 || edges.size() > 4000000 )
	{
		Fail( error, "asset index count limit" );
		return std::nullopt;
	}
	std::map<AssetRef, std::uint32_t> positions;
	std::map<std::uint64_t, AssetRef> hashes;
	std::set<std::pair<AssetRef, std::string>> variants;
	for ( std::uint32_t i = 0; i < entries.size(); ++i )
	{
		const AssetEntry &entry = entries[i];
		if ( !ValidEntry( entry ) || !variants.emplace( entry.ref, entry.variant ).second )
		{
			Fail( error, "invalid or duplicate asset entry" );
			return std::nullopt;
		}
		if ( auto found = hashes.find( entry.ref.NameHash() );
		    found != hashes.end() && !( found->second == entry.ref ) )
		{
			Fail( error, "asset name hash collision" );
			return std::nullopt;
		}
		hashes.insert_or_assign( entry.ref.NameHash(), entry.ref );
		positions.try_emplace( entry.ref, i );
	}
	for ( const AssetEdge &edge : edges )
	{
		if ( !positions.contains( edge.source ) ||
		     !AssetRef::Create( edge.target.kind, edge.target.name ) ||
		     edge.target.name.size() > 65535 )
		{
			Fail( error, "invalid asset reference" );
			return std::nullopt;
		}
	}
	std::sort( edges.begin(), edges.end(),
	    [&positions]( const auto &a, const auto &b )
	    {
		    return std::tie( positions.at( a.source ), a.target, a.optional ) <
		           std::tie( positions.at( b.source ), b.target, b.optional );
	    } );
	std::vector<std::uint8_t> block;
	PutString( block, "AIX1" );
	Put32( block, std::uint32_t( entries.size() ) );
	Put32( block, std::uint32_t( edges.size() ) );
	Put32( block, 0 );
	for ( const AssetEntry &entry : entries )
	{
		block.push_back( std::uint8_t( entry.ref.kind ) );
		block.push_back( 0 );
		for ( const std::string *value :
		    { &entry.ref.name, &entry.variant, &entry.location, &entry.compiler, &entry.key } )
			Put16( block, std::uint16_t( value->size() ) );
		Put16( block, 0 );
		Put64( block, entry.ref.NameHash() );
		block.insert( block.end(), entry.hash.begin(), entry.hash.end() );
		Put64( block, entry.size );
		PutString( block, entry.ref.name );
		PutString( block, entry.variant );
		PutString( block, entry.location );
		PutString( block, entry.compiler );
		PutString( block, entry.key );
	}
	for ( const AssetEdge &edge : edges )
	{
		Put32( block, positions.at( edge.source ) );
		block.push_back( std::uint8_t( edge.target.kind ) );
		block.push_back( edge.optional ? 1 : 0 );
		Put16( block, std::uint16_t( edge.target.name.size() ) );
		Put64( block, edge.target.NameHash() );
		PutString( block, edge.target.name );
	}
	std::vector<std::uint8_t> output( kHeaderSize, 0 );
	output.insert( output.end(), block.begin(), block.end() );
	while ( output.size() % 16 )
		output.push_back( 0 );
	const std::uint64_t directoryOffset = output.size();
	std::vector<std::uint8_t> directory;
	Put32( directory, kFourCC );
	Put32( directory, 1 );
	Put32( directory, 0 );
	Put32( directory, 16 );
	Put64( directory, 64 );
	Put64( directory, block.size() );
	Put64( directory, block.size() );
	Put64( directory, 0 );
	const auto blockHash = Hash( block );
	directory.insert( directory.end(), blockHash.begin(), blockHash.end() );
	output.insert( output.end(), directory.begin(), directory.end() );
	std::vector<std::uint8_t> header;
	header.insert( header.end(), std::begin( kMagic ), std::end( kMagic ) );
	Put32( header, 1 );
	Put32( header, 0 );
	Put32( header, kHeaderSize );
	Put32( header, kDirectorySize );
	Put64( header, directoryOffset );
	Put32( header, 1 );
	Put32( header, 1 ); // BLAKE2b-128
	Put32( header, 0 );
	Put32( header, 0 );
	const auto directoryHash = Hash( directory );
	header.insert( header.end(), directoryHash.begin(), directoryHash.end() );
	std::copy( header.begin(), header.end(), output.begin() );
	return output;
}

std::optional<AssetIndex> AssetIndex::Open(
    std::span<const std::uint8_t> bytes, std::string *error )
{
	if ( bytes.size() < kHeaderSize + kDirectorySize ||
	     std::memcmp( bytes.data(), kMagic, sizeof( kMagic ) ) != 0 ||
	     Get32( bytes.data() + 8 ) != 1 || Get32( bytes.data() + 12 ) != 0 ||
	     Get32( bytes.data() + 16 ) != 64 || Get32( bytes.data() + 20 ) != 64 ||
	     Get32( bytes.data() + 32 ) != 1 || Get32( bytes.data() + 36 ) != 1 ||
	     Get32( bytes.data() + 40 ) != 0 || Get32( bytes.data() + 44 ) != 0 )
	{
		Fail( error, "invalid asset index header" );
		return std::nullopt;
	}
	const std::uint64_t directoryOffset = Get64( bytes.data() + 24 );
	if ( directoryOffset < kHeaderSize || !Fits( directoryOffset, 64, bytes.size() ) ||
	     directoryOffset + 64 != bytes.size() || directoryOffset % 16 ||
	     Hash( bytes.subspan( directoryOffset, 64 ) ) !=
	         std::array<std::uint8_t, 16>{ bytes[48], bytes[49], bytes[50], bytes[51], bytes[52],
	             bytes[53], bytes[54], bytes[55], bytes[56], bytes[57], bytes[58], bytes[59],
	             bytes[60], bytes[61], bytes[62], bytes[63] } )
	{
		Fail( error, "asset index directory hash or bounds" );
		return std::nullopt;
	}
	const std::uint8_t *dir = bytes.data() + directoryOffset;
	const std::uint64_t offset = Get64( dir + 16 );
	const std::uint64_t size = Get64( dir + 24 );
	if ( Get32( dir ) != kFourCC || Get32( dir + 4 ) != 1 || Get32( dir + 8 ) ||
	     Get32( dir + 12 ) != 16 || offset != 64 || Get64( dir + 32 ) != size ||
	     Get64( dir + 40 ) || !Fits( offset, size, directoryOffset ) ||
	     Hash( bytes.subspan( offset, size ) ) != std::array<std::uint8_t, 16>{ dir[48], dir[49],
	                                                  dir[50], dir[51], dir[52], dir[53], dir[54],
	                                                  dir[55], dir[56], dir[57], dir[58], dir[59],
	                                                  dir[60], dir[61], dir[62], dir[63] } )
	{
		Fail( error, "asset index block hash or bounds" );
		return std::nullopt;
	}
	const auto block = bytes.subspan( offset, size );
	if ( block.size() < 16 || std::memcmp( block.data(), "AIX1", 4 ) != 0 ||
	     Get32( block.data() + 4 ) > 1000000 || Get32( block.data() + 8 ) > 4000000 ||
	     Get32( block.data() + 12 ) != 0 )
	{
		Fail( error, "invalid asset index payload" );
		return std::nullopt;
	}
	const auto entryCount = Get32( block.data() + 4 );
	const auto edgeCount = Get32( block.data() + 8 );
	std::size_t cursor = 16;
	AssetIndex result;
	std::set<std::pair<AssetRef, std::string>> variants;
	std::map<std::uint64_t, AssetRef> hashes;
	for ( std::uint32_t i = 0; i < entryCount; ++i )
	{
		if ( !Fits( cursor, kEntrySize, block.size() ) )
		{
			Fail( error, "truncated asset entry" );
			return std::nullopt;
		}
		const std::uint8_t *p = block.data() + cursor;
		const auto kind = AssetKind( p[0] );
		const std::uint16_t lengths[5] = {
		    Get16( p + 2 ), Get16( p + 4 ), Get16( p + 6 ), Get16( p + 8 ), Get16( p + 10 ) };
		cursor += kEntrySize;
		std::string fields[5];
		bool valid = p[1] == 0 && Get16( p + 12 ) == 0;
		for ( int j = 0; j < 5; ++j )
			valid = TakeString( block, cursor, lengths[j], fields[j] ) && valid;
		auto ref = AssetRef::Create( kind, fields[0] );
		AssetEntry entry{ ref.value_or( AssetRef{ kind, "" } ), fields[1], fields[2], fields[3],
		    fields[4], {}, Get64( p + 38 ) };
		std::copy_n( p + 22, 16, entry.hash.begin() );
		if ( !valid || !ref || !ValidEntry( entry ) || Get64( p + 14 ) != ref->NameHash() ||
		     !variants.emplace( entry.ref, entry.variant ).second )
		{
			Fail( error, "invalid or duplicate asset entry" );
			return std::nullopt;
		}
		if ( auto found = hashes.find( ref->NameHash() );
		    found != hashes.end() && !( found->second == *ref ) )
		{
			Fail( error, "asset name hash collision" );
			return std::nullopt;
		}
		hashes.insert_or_assign( ref->NameHash(), *ref );
		result.m_Entries.push_back( std::move( entry ) );
	}
	for ( std::uint32_t i = 0; i < edgeCount; ++i )
	{
		if ( !Fits( cursor, kEdgeSize, block.size() ) )
		{
			Fail( error, "truncated asset edge" );
			return std::nullopt;
		}
		const std::uint8_t *p = block.data() + cursor;
		cursor += kEdgeSize;
		std::string name;
		const bool hasName = TakeString( block, cursor, Get16( p + 6 ), name );
		auto target = AssetRef::Create( AssetKind( p[4] ), name );
		if ( Get32( p ) >= result.m_Entries.size() || p[5] > 1 || !hasName || !target ||
		     target->NameHash() != Get64( p + 8 ) )
		{
			Fail( error, "invalid asset reference" );
			return std::nullopt;
		}
		result.m_Edges.push_back(
		    AssetEdge{ result.m_Entries[Get32( p )].ref, std::move( *target ), p[5] != 0 } );
	}
	if ( cursor != block.size() )
	{
		Fail( error, "trailing asset index data" );
		return std::nullopt;
	}
	return result;
}

std::optional<AssetIndex> AssetIndex::Read( const std::filesystem::path &path, std::string *error )
{
	std::ifstream file( path, std::ios::binary );
	if ( !file )
	{
		Fail( error, "asset index open failed" );
		return std::nullopt;
	}
	const std::vector<std::uint8_t> bytes( std::istreambuf_iterator<char>{ file }, {} );
	return Open( bytes, error );
}

} // namespace content
