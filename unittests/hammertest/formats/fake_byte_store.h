//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Test fakes and fixture builders for the asset codecs (RFC 0002,
//			hammer.formats). Provides an in-memory hammer::ports::IByteStore and a
//			VPK-blob builder so the VPK reader is exercised against a fixture built
//			by an INDEPENDENT serializer (not the reader itself) -- the Q-CONTENT
//			"independent readers/tools" discipline. The builder emits real VPK v1
//			and v2 byte layouts: header, extension/dir/file tree, inline preload
//			runs, an inline data section, and separate _NNN.vpk data archives.
//
//=============================================================================//

#ifndef HAMMERTEST_FAKE_BYTE_STORE_H
#define HAMMERTEST_FAKE_BYTE_STORE_H

#include "hammer/ports/byte_store.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace hammertest
{

// In-memory ranged byte store. Maps a path to its full bytes; ReadRange enforces
// the strict "short read is a failure" clause of the contract.
class InMemoryByteStore : public hammer::ports::IByteStore
{
public:
	std::map<std::string, std::string> files;

	bool Size( const std::string &path, std::uint64_t &outSize ) const override
	{
		auto it = files.find( path );
		if ( it == files.end() )
			return false;
		outSize = it->second.size();
		return true;
	}

	bool ReadRange( const std::string &path, std::uint64_t offset, std::size_t length,
	    std::string &out ) const override
	{
		auto it = files.find( path );
		if ( it == files.end() )
			return false;
		const std::string &data = it->second;
		if ( offset > data.size() || offset + length > data.size() )
			return false;
		out.assign( data, static_cast<std::size_t>( offset ), length );
		return true;
	}
};

// One file to place in a built VPK.
struct VpkBuildFile
{
	std::string path;                    // canonical: lower, forward-slash, ext included
	std::string data;                    // full file contents
	std::uint16_t preload = 0;           // first N bytes stored inline in the directory
	std::uint16_t archiveIndex = 0x7fff; // 0x7fff = inline dir data section; else _NNN.vpk
};

namespace detail
{

inline void PutU16( std::string &b, std::uint16_t v )
{
	b.push_back( char( v & 0xff ) );
	b.push_back( char( ( v >> 8 ) & 0xff ) );
}

inline void PutU32( std::string &b, std::uint32_t v )
{
	b.push_back( char( v & 0xff ) );
	b.push_back( char( ( v >> 8 ) & 0xff ) );
	b.push_back( char( ( v >> 16 ) & 0xff ) );
	b.push_back( char( ( v >> 24 ) & 0xff ) );
}

// Splits "materials/foo/bar.vmt" into ext="vmt", dir="materials/foo", name="bar".
// A root file "bar.vmt" yields dir=" " (the VPK root marker).
inline void SplitPath(
    const std::string &path, std::string &ext, std::string &dir, std::string &name )
{
	std::size_t dot = path.rfind( '.' );
	std::string stem = ( dot == std::string::npos ) ? path : path.substr( 0, dot );
	ext = ( dot == std::string::npos ) ? "" : path.substr( dot + 1 );
	std::size_t slash = stem.rfind( '/' );
	if ( slash == std::string::npos )
	{
		dir = " ";
		name = stem;
	}
	else
	{
		dir = stem.substr( 0, slash );
		name = stem.substr( slash + 1 );
	}
}

} // namespace detail

// Builds a VPK directory blob (version 1 or 2) plus its external data archives.
// 'externalArchives' is filled with archiveIndex -> blob for any file whose
// archiveIndex != 0x7fff. Inline chunks (0x7fff) are placed in the directory
// blob's data section (v1: right after the tree; v2: the file-data section).
inline std::string BuildVpk( const std::vector<VpkBuildFile> &files,
    std::map<int, std::string> &externalArchives, std::uint32_t version = 2 )
{
	using namespace detail;

	// Group by ext -> dir -> [(name, index-into-files)].
	std::map<std::string, std::map<std::string, std::vector<std::size_t>>> tree;
	for ( std::size_t i = 0; i < files.size(); ++i )
	{
		std::string ext, dir, name;
		SplitPath( files[i].path, ext, dir, name );
		tree[ext][dir].push_back( i );
	}

	// Assign chunk offsets. Inline chunks accumulate in 'inlineData'; external ones
	// accumulate per archive.
	std::string inlineData;
	std::map<int, std::uint32_t> externalCursor;
	struct Placed
	{
		std::uint32_t chunkOffset;
		std::uint32_t chunkLength;
	};
	std::vector<Placed> placed( files.size() );
	for ( std::size_t i = 0; i < files.size(); ++i )
	{
		const VpkBuildFile &f = files[i];
		std::uint32_t chunkLen = std::uint32_t( f.data.size() - f.preload );
		std::string chunk = f.data.substr( f.preload );
		if ( f.archiveIndex == 0x7fff )
		{
			placed[i] = { std::uint32_t( inlineData.size() ), chunkLen };
			inlineData += chunk;
		}
		else
		{
			std::string &arc = externalArchives[f.archiveIndex];
			placed[i] = { externalCursor[f.archiveIndex], chunkLen };
			arc += chunk;
			externalCursor[f.archiveIndex] += chunkLen;
		}
	}

	// Serialize the tree.
	std::string body;
	for ( const auto &extPair : tree )
	{
		body += extPair.first;
		body.push_back( '\0' );
		for ( const auto &dirPair : extPair.second )
		{
			body += dirPair.first;
			body.push_back( '\0' );
			for ( std::size_t idx : dirPair.second )
			{
				std::string ext, dir, name;
				SplitPath( files[idx].path, ext, dir, name );
				body += name;
				body.push_back( '\0' );
				PutU32( body, 0 );                       // crc (unchecked by reader)
				PutU16( body, files[idx].preload );      // preload byte count
				PutU16( body, files[idx].archiveIndex ); // archive index
				PutU32( body, placed[idx].chunkOffset ); // chunk offset
				PutU32( body, placed[idx].chunkLength ); // chunk length
				PutU16( body, 0xffff );                  // terminator
				body += files[idx].data.substr( 0, files[idx].preload ); // inline preload
			}
			body.push_back( '\0' ); // end of files in this dir
		}
		body.push_back( '\0' ); // end of dirs for this ext
	}
	body.push_back( '\0' ); // end of tree

	std::string blob;
	PutU32( blob, 0x55aa1234u );                  // signature
	PutU32( blob, version );                      // version
	PutU32( blob, std::uint32_t( body.size() ) ); // tree size
	if ( version == 2 )
	{
		PutU32( blob, std::uint32_t( inlineData.size() ) ); // file data section size
		PutU32( blob, 0 );                                  // archive MD5 section
		PutU32( blob, 0 );                                  // other MD5 section
		PutU32( blob, 0 );                                  // signature section
	}
	blob += body;
	blob += inlineData;
	return blob;
}

} // namespace hammertest

#endif // HAMMERTEST_FAKE_BYTE_STORE_H
