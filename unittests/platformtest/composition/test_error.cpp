//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: foundation.error conformance (RFC 0006 M1 / roadmap R05).
//
//=============================================================================//

#include "foundation/error.h"
#include "foundation/expected.h"
#include "testing/checks.h"

#include <cstdint>
#include <cstdio>
#include <type_traits>

namespace
{
enum class FileStatus : std::uint8_t
{
	NotFound,
	Denied,
};
enum class FileOperation : std::uint8_t
{
	Open,
	Read,
};
enum class OtherStatus : std::uint8_t
{
	NotFound,
};
using FileError = foundation::Error<FileStatus, FileOperation>;

// Compact, plain-value clauses.
static_assert( std::is_trivially_copyable_v<FileError> );
static_assert( std::is_standard_layout_v<FileError> );
static_assert( std::is_aggregate_v<FileError> );
static_assert( sizeof( FileError ) <= 2 * sizeof( std::int32_t ) );
static_assert( !std::is_same_v<FileError, foundation::Error<OtherStatus, FileOperation>> );
static_assert( !std::is_convertible_v<FileError, foundation::Error<OtherStatus, FileOperation>> );

foundation::Expected<int, FileError> OpenFile( bool exists, int native )
{
	if ( !exists )
		return foundation::MakeUnexpected(
		    FileError{ FileStatus::NotFound, FileOperation::Open, native } );
	return 3;
}
} // namespace

testing::TestResult RunErrorConformance()
{
	testing::Checks checks;
	auto check = [&]( bool value, std::source_location where = std::source_location::current() )
	{
		checks.That( value, "check", where );
	};

	const FileError unset{};
	check( unset.status == FileStatus::NotFound && unset.operation == FileOperation::Open );
	check( unset.nativeCode == 0 && !unset.HasNativeCode() );

	const FileError denied{ FileStatus::Denied, FileOperation::Read, 13 };
	check( denied.HasNativeCode() && denied.nativeCode == 13 );
	check( denied == FileError{ FileStatus::Denied, FileOperation::Read, 13 } );
	check( denied != FileError{ FileStatus::Denied, FileOperation::Read, 0 } );
	check( denied != FileError{ FileStatus::Denied, FileOperation::Open, 13 } );
	check( denied != FileError{ FileStatus::NotFound, FileOperation::Read, 13 } );

	const auto missing = OpenFile( false, 2 );
	check(
	    !missing && missing.Error() == FileError{ FileStatus::NotFound, FileOperation::Open, 2 } );
	const auto opened = OpenFile( true, 0 );
	check( opened && opened.Value() == 3 );

	FileError copied = denied;
	check( copied == denied );

	return checks.Result();
}

#ifndef SOURCE_CONFORMANCE_LINKED
int main()
{
	const auto result = RunErrorConformance();
	std::printf( "CONFORMANCE %zu %zu\n", result.checks, result.failures );
	return result.Passed() ? 0 : 1;
}
#endif
