//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the RFC 0001 process-environment
//			capability (platform::IProcessEnvironment). Every provider that claims
//			the contract runs THIS predicate against a fixture describing what
//			the process was started with: a native run launches itself with known
//			arguments and variables, a fake is constructed from them.
//
//=============================================================================//

#ifndef PLATFORMTEST_PROCESS_ENVIRONMENT_CONFORMANCE_H
#define PLATFORMTEST_PROCESS_ENVIRONMENT_CONFORMANCE_H

#include "platform/contracts/process_environment.h"

#include <cstdio>
#include <cstring>

namespace platformtest
{

struct ProcessEnvironmentReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define PE_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

// What the process was started with. `presentName` must have a non-empty
// value; `emptyName` is set to ""; `absentName` is not set, and neither is
// `caseVariantName`, which differs from `presentName` only in letter case.
struct ProcessEnvironmentFixture
{
	const char *const *arguments = nullptr;
	int argumentCount = 0;
	const char *presentName = nullptr;
	const char *presentValue = nullptr;
	const char *emptyName = nullptr;
	const char *absentName = nullptr;
	const char *caseVariantName = nullptr;
};

// Checks the Length/Get pair for one item against its expected value (null =
// absent): exact copy, no partial write when the buffer is one byte short.
template <typename LengthFn, typename GetFn>
void CheckItem( ProcessEnvironmentReport &r, const char *expected, LengthFn length, GetFn get )
{
	char buffer[512];
	std::memset( buffer, '#', sizeof( buffer ) );
	if ( expected == nullptr )
	{
		PE_CHECK( r, length() == -1 );
		PE_CHECK( r, get( buffer, static_cast<int>( sizeof( buffer ) ) ) == -1 );
		PE_CHECK( r, buffer[0] == '#' );
		return;
	}
	const int n = static_cast<int>( std::strlen( expected ) );
	PE_CHECK( r, length() == n );
	PE_CHECK( r, get( buffer, static_cast<int>( sizeof( buffer ) ) ) == n );
	PE_CHECK( r, std::strcmp( buffer, expected ) == 0 );
	PE_CHECK( r, get( buffer, static_cast<int>( sizeof( buffer ) ) ) == n ); // stable

	char exact[512];
	PE_CHECK( r, get( exact, n + 1 ) == n && std::strcmp( exact, expected ) == 0 );

	char tight[512];
	std::memset( tight, '#', sizeof( tight ) );
	PE_CHECK( r, get( tight, n ) == -1 );
	PE_CHECK( r, tight[0] == '#' );
	PE_CHECK( r, get( nullptr, 64 ) == -1 );
	PE_CHECK( r, get( tight, 0 ) == -1 );
}

inline ProcessEnvironmentReport RunProcessEnvironmentConformance(
    const platform::IProcessEnvironment &env, const ProcessEnvironmentFixture &fixture )
{
	ProcessEnvironmentReport r;

	PE_CHECK( r, env.ArgumentCount() == fixture.argumentCount );
	for ( int i = 0; i < fixture.argumentCount; ++i )
	{
		CheckItem(
		    r, fixture.arguments[i],
		    [&]
		    {
			    return env.ArgumentLength( i );
		    },
		    [&]( char *b, int s )
		    {
			    return env.GetArgument( i, b, s );
		    } );
	}
	const int outOfRange[] = { -1, fixture.argumentCount, fixture.argumentCount + 7 };
	for ( int index : outOfRange )
	{
		CheckItem(
		    r, nullptr,
		    [&]
		    {
			    return env.ArgumentLength( index );
		    },
		    [&]( char *b, int s )
		    {
			    return env.GetArgument( index, b, s );
		    } );
	}

	struct Variable
	{
		const char *name;
		const char *expected;
	};
	const Variable variables[] = {
	    { fixture.presentName, fixture.presentValue },
	    { fixture.emptyName, "" },
	    { fixture.absentName, nullptr },
	    { fixture.caseVariantName, nullptr },
	    { nullptr, nullptr },
	    { "", nullptr },
	    { "A=B", nullptr },
	};
	for ( const Variable &v : variables )
	{
		CheckItem(
		    r, v.expected,
		    [&]
		    {
			    return env.VariableLength( v.name );
		    },
		    [&]( char *b, int s )
		    {
			    return env.GetVariable( v.name, b, s );
		    } );
	}

	PE_CHECK( r, env.ProcessId() != 0 );
	PE_CHECK( r, env.ProcessId() == env.ProcessId() );
	const platform::DebuggerState d = env.GetDebuggerState();
	PE_CHECK( r, d == platform::DebuggerState::kUnknown ||
	                 d == platform::DebuggerState::kNotAttached ||
	                 d == platform::DebuggerState::kAttached );
	return r;
}

} // namespace platformtest

#endif // PLATFORMTEST_PROCESS_ENVIRONMENT_CONFORMANCE_H
