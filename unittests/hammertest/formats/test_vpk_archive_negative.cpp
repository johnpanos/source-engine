//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.vpk_archive.v1. Proves the reader
//			REJECTS malformed archives (bad signature, unsupported version, short
//			header, over-long tree, unterminated tree, bad record terminator) and
//			fails cleanly on absent files and missing data archives -- never a
//			partial or fabricated read. Build/run via the conformance manifest.
//
//=============================================================================//

#include "formats/fake_byte_store.h"

#include "hammer/formats/vpk_archive.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

namespace
{
int g_checks = 0;
int g_failures = 0;

void PutU16( std::string &b, std::uint16_t v )
{
	b.push_back( char( v & 0xff ) );
	b.push_back( char( ( v >> 8 ) & 0xff ) );
}
void PutU32( std::string &b, std::uint32_t v )
{
	b.push_back( char( v & 0xff ) );
	b.push_back( char( ( v >> 8 ) & 0xff ) );
	b.push_back( char( ( v >> 16 ) & 0xff ) );
	b.push_back( char( ( v >> 24 ) & 0xff ) );
}
} // namespace
#define CHECK( cond, msg )                                                                         \
	do                                                                                             \
	{                                                                                              \
		++g_checks;                                                                                \
		if ( !( cond ) )                                                                           \
		{                                                                                          \
			std::printf( "FAIL: %s\n", ( msg ) );                                                  \
			++g_failures;                                                                          \
		}                                                                                          \
	} while ( 0 )

using hammer::formats::VpkArchive;

// Opens a single-file store and asserts the open is rejected.
static void ExpectRejected( const std::string &blob, const char *what )
{
	++g_checks;
	hammertest::InMemoryByteStore store;
	store.files["x_dir.vpk"] = blob;
	std::string error;
	auto vpk = VpkArchive::Open( store, "x_dir.vpk", error );
	if ( vpk != nullptr )
	{
		std::printf( "FAIL: %s should be rejected but Open succeeded\n", what );
		++g_failures;
	}
	else if ( error.empty() )
	{
		std::printf( "FAIL: %s rejected without a diagnostic\n", what );
		++g_failures;
	}
}

int main()
{
	// A valid single-inline-entry v2 blob to mutate.
	std::vector<hammertest::VpkBuildFile> files = { { "a/b.txt", "payload-bytes", 0, 0x7fff } };
	std::map<int, std::string> external;
	const std::string good = hammertest::BuildVpk( files, external, 2 );

	// Bad signature.
	{
		std::string b = good;
		b[0] = 'X';
		ExpectRejected( b, "bad signature" );
	}
	// Unsupported version (3).
	{
		std::string b = good;
		b[4] = 3;
		ExpectRejected( b, "unsupported version" );
	}
	// Too small to hold a header.
	ExpectRejected( good.substr( 0, 8 ), "short header" );
	// Tree size claims more than the file holds.
	{
		std::string b = good;
		b[8] = char( 0xff );
		b[9] = char( 0xff );
		b[10] = char( 0xff );
		ExpectRejected( b, "tree longer than file" );
	}
	// Unterminated extension list: header claims a 4-byte tree of non-NUL bytes.
	{
		std::string b;
		PutU32( b, 0x55aa1234u ); // signature
		PutU32( b, 2 );           // version 2
		PutU32( b, 4 );           // treeSize = 4
		PutU32( b, 0 );           // file data section
		PutU32( b, 0 );           // archive md5
		PutU32( b, 0 );           // other md5
		PutU32( b, 0 );           // signature section
		b += "abcd";              // no NUL terminator anywhere
		ExpectRejected( b, "unterminated tree" );
	}
	// Bad file-record terminator (not 0xffff).
	{
		std::string tree;
		tree += "t";
		tree.push_back( '\0' ); // extension
		tree += "d";
		tree.push_back( '\0' ); // dir
		tree += "f";
		tree.push_back( '\0' ); // name
		PutU32( tree, 0 );      // crc
		PutU16( tree, 0 );      // preload
		PutU16( tree, 0x7fff ); // archive index
		PutU32( tree, 0 );      // chunk offset
		PutU32( tree, 0 );      // chunk length
		PutU16( tree, 0x1234 ); // BAD terminator
		tree.push_back( '\0' ); // end files
		tree.push_back( '\0' ); // end dirs
		tree.push_back( '\0' ); // end tree

		std::string b;
		PutU32( b, 0x55aa1234u );
		PutU32( b, 2 );
		PutU32( b, std::uint32_t( tree.size() ) );
		PutU32( b, 0 );
		PutU32( b, 0 );
		PutU32( b, 0 );
		PutU32( b, 0 );
		b += tree;
		ExpectRejected( b, "bad record terminator" );
	}

	// Absent directory file.
	{
		hammertest::InMemoryByteStore store;
		std::string error;
		auto vpk = VpkArchive::Open( store, "missing_dir.vpk", error );
		CHECK( vpk == nullptr, "absent directory file rejected" );
	}

	// A good archive: ReadAsset on an unknown path fails; a valid path succeeds.
	{
		hammertest::InMemoryByteStore store;
		store.files["ok_dir.vpk"] = good;
		std::string error;
		auto vpk = VpkArchive::Open( store, "ok_dir.vpk", error );
		CHECK( vpk != nullptr, "good archive opens" );
		std::string out;
		CHECK( vpk && !vpk->ReadAsset( "does/not/exist.txt", out ), "unknown path read fails" );
		CHECK(
		    vpk && vpk->ReadAsset( "a/b.txt", out ) && out == "payload-bytes", "known path reads" );
	}

	// External-archive entry whose data file is missing must fail the read.
	{
		std::vector<hammertest::VpkBuildFile> ext = { { "s/x.dat", std::string( 64, 'Z' ), 0, 0 } };
		std::map<int, std::string> externalBlobs;
		std::string dirBlob = hammertest::BuildVpk( ext, externalBlobs, 2 );
		hammertest::InMemoryByteStore store;
		store.files["m_dir.vpk"] = dirBlob; // deliberately DO NOT add m_000.vpk
		std::string error;
		auto vpk = VpkArchive::Open( store, "m_dir.vpk", error );
		CHECK( vpk != nullptr, "archive with external entry opens" );
		std::string out;
		CHECK( vpk && !vpk->ReadAsset( "s/x.dat", out ), "missing data archive fails read" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.vpk_archive.sensitivity: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.vpk_archive.sensitivity: all malformed inputs rejected\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
