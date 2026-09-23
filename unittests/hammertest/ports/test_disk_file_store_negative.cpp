//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for ports.file_store.v1. A broken provider that
//			"writes" without persisting (Read never returns what Write stored)
//			violates the round-trip clause. The shared contract suite must detect
//			it; otherwise the suite does not actually pin persistence.
//
//			Convention: exit 0 when the real fake conforms AND the broken provider
//			is caught; exit 1 otherwise.
//
//=============================================================================//

#include "ports/file_store_contract.h"

#include "app/fake_file_store.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

namespace
{

// Broken provider: reports success but never persists content, so Read after
// Write cannot round-trip. Exists is faked to true so the failure lands on the
// content clauses, not the existence clause.
class ForgetfulFileStore : public hammer::ports::IFileStore
{
public:
	bool Exists( const std::string & ) const override { return true; }
	bool Read( const std::string &, std::string & ) const override { return false; }
	bool Write( const std::string &, const std::string & ) override { return true; }
	bool Rename( const std::string &, const std::string & ) override { return true; }
	bool Remove( const std::string & ) override { return true; }
};

} // namespace

int main()
{
	hammertest::InMemoryFileStore good;
	ForgetfulFileStore broken;

	const int goodFailures = hammertest::RunFileStoreContract( good, "good/", "good" );
	const int brokenFailures = hammertest::RunFileStoreContract( broken, "broken/", "broken" );

	int checks = 0;
	int failures = 0;
	++checks;
	if ( goodFailures != 0 )
	{
		std::printf( "FAIL: real InMemoryFileStore did not conform (%d)\n", goodFailures );
		++failures;
	}
	++checks;
	if ( brokenFailures == 0 )
	{
		std::printf( "FAIL: oracle did not catch the forgetful provider\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "ports.file_store negative: ORACLE UNSOUND (%d)\n", failures );
		return testing::ReportConformance( checks, failures );
	}
	std::printf( "ports.file_store negative: oracle catches a non-persisting store "
	             "(real conforms, broken caught)\n" );
	return testing::ReportConformance( checks, failures );
}
