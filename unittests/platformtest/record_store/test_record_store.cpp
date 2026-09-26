//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for platform.record-store.v1: the
//			shared suite against the in-memory fake and the file store
//			(platform/records), plus the file store's own clauses: the record
//			is the file <directory>/<key> (gamestate.txt stays where the file
//			system wrote it), a leftover staging file changes nothing, and a
//			missing directory fails cleanly.
//
//			On Linux and, through tools/quality/ios_conformance.py, on iOS and
//			tvOS devices, where the file store runs in the app's temporary
//			directory.
//
//			Build/run: tools/quality/conformance.py check --suite platform.record_store
//
//=============================================================================//

#include "fake_record_store.h"
#include "record_store_conformance.h"
#include "../../../platform/records/file_record_store.h"
#include "testing/conformance_result.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{

using platform::RecordStoreErrorCode;
using platformtest::Bytes;

std::filesystem::path FreshDirectory()
{
	const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
	std::filesystem::path dir = std::filesystem::temp_directory_path() /
	                            ( "record_store_" + std::to_string( stamp ) );
	std::filesystem::create_directories( dir );
	return dir;
}

void WriteFile( const std::filesystem::path &path, const std::string &text )
{
	std::ofstream out( path, std::ios::binary | std::ios::trunc );
	out << text;
}

std::string ReadFile( const std::filesystem::path &path )
{
	std::ifstream in( path, std::ios::binary );
	return std::string( std::istreambuf_iterator<char>( in ), std::istreambuf_iterator<char>() );
}

platformtest::RecordStoreReport RunFileStoreClauses( const std::filesystem::path &dir )
{
	platformtest::RecordStoreReport r;
	platform::FileRecordStore store( dir.string() + "/" );

	// The record is the file <directory>/<key>: a gamestate.txt written by the
	// file system before is read, and a commit is what a later reader finds.
	WriteFile( dir / "gamestate.txt", "\"GameState\"\n{\n}\n" );
	RS_CHECK( r, platformtest::Holds( store, "gamestate.txt", Bytes( "\"GameState\"\n{\n}\n" ) ) );
	RS_CHECK( r, store.Commit( "gamestate.txt", Bytes( "new state" ) ).HasValue() );
	RS_CHECK( r, ReadFile( dir / "gamestate.txt" ) == "new state" );
	RS_CHECK( r, store.PathFor( "gamestate.txt" ) == ( dir / "gamestate.txt" ).string() );

	// Commit leaves no staging file, and a stale one (a crash mid-commit)
	// neither shadows the record nor blocks the next commit.
	RS_CHECK( r, !std::filesystem::exists( dir / "gamestate.txt~" ) );
	WriteFile( dir / "gamestate.txt~", "half" );
	RS_CHECK( r, platformtest::Holds( store, "gamestate.txt", Bytes( "new state" ) ) );
	RS_CHECK( r, store.Commit( "gamestate.txt", Bytes( "after crash" ) ).HasValue() );
	RS_CHECK( r, ReadFile( dir / "gamestate.txt" ) == "after crash" );
	RS_CHECK( r, !std::filesystem::exists( dir / "gamestate.txt~" ) );

	// A missing directory: nothing to load, and a commit fails cleanly.
	platform::FileRecordStore missing( ( dir / "missing" / "deeper" ).string() );
	RS_CHECK( r, platformtest::IsError( missing.Load( "gamestate.txt" ), RecordStoreErrorCode::kNotFound ) );
	RS_CHECK( r, platformtest::IsError( missing.Commit( "gamestate.txt", Bytes( "x" ) ),
	                 RecordStoreErrorCode::kIoFailure ) );
	RS_CHECK( r, !std::filesystem::exists( dir / "missing" ) );

	// A record larger than the limit on disk is refused on Load.
	platform::FileRecordStore small( dir.string(), 8 );
	WriteFile( dir / "big", "123456789" );
	RS_CHECK( r, platformtest::IsError( small.Load( "big" ), RecordStoreErrorCode::kTooLarge ) );
	WriteFile( dir / "fits", "12345678" );
	RS_CHECK( r, platformtest::Holds( small, "fits", Bytes( "12345678" ) ) );
	return r;
}

} // namespace

int main()
{
	int rc = 0;
	int checks = 0;
	int failures = 0;

	{
		auto backing = std::make_shared<platformtest::FakeRecordBacking>();
		platformtest::RecordStoreFixture fixture;
		fixture.open = [backing] { return std::make_unique<platformtest::FakeRecordStore>( backing ); };
		rc |= platformtest::ReportRecordStoreVariant( "record_store[fake]",
		    platformtest::RunRecordStoreConformance( fixture ), checks, failures );
	}

	const std::filesystem::path root = FreshDirectory();
	{
		const std::filesystem::path dir = root / "shared";
		std::filesystem::create_directories( dir );
		platformtest::RecordStoreFixture fixture;
		fixture.open = [dir] { return std::make_unique<platform::FileRecordStore>( dir.string() ); };
		rc |= platformtest::ReportRecordStoreVariant( "record_store[file]",
		    platformtest::RunRecordStoreConformance( fixture ), checks, failures );
	}
	{
		const std::filesystem::path dir = root / "file";
		std::filesystem::create_directories( dir );
		rc |= platformtest::ReportRecordStoreVariant( "record_store[file clauses]",
		    RunFileStoreClauses( dir ), checks, failures );
	}
	std::error_code ignored;
	std::filesystem::remove_all( root, ignored );

	(void)rc;
	return testing::ReportConformance( checks, failures );
}
