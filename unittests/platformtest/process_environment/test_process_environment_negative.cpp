//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the RFC 0001 process-environment conformance
//			suite (PLAT-ENV-001, Q-FOUNDATION). Feeds the SAME shared predicate
//			broken providers, each violating one clause, and asserts every one is
//			caught while the conforming backend passes.
//
//			Build/run: tools/quality/conformance.py check --suite platform.process_environment.sensitivity
//
//=============================================================================//

#include "fake_process_environment.h"
#include "process_environment_conformance.h"
#include "testing/conformance_result.h"

#include <cctype>
#include <cstdio>

namespace
{

using platformtest::CFakeProcessEnvironment;

const char *const kArgs[] = { "/opt/game/portal", "-game", "portal" };
const std::map<std::string, std::string> kVars = {
    { "SOURCE_TEST_VAR", "value" },
    { "SOURCE_TEST_EMPTY", "" },
};

platformtest::ProcessEnvironmentFixture Fixture()
{
	platformtest::ProcessEnvironmentFixture f;
	f.arguments = kArgs;
	f.argumentCount = 3;
	f.presentName = "SOURCE_TEST_VAR";
	f.presentValue = "value";
	f.emptyName = "SOURCE_TEST_EMPTY";
	f.absentName = "SOURCE_TEST_ABSENT";
	f.caseVariantName = "source_test_var";
	return f;
}

class CBase : public CFakeProcessEnvironment
{
public:
	CBase() : CFakeProcessEnvironment( { kArgs, kArgs + 3 }, kVars ) {}
};

// DEFECT: drops the program name.
class CDropsProgramName : public CFakeProcessEnvironment
{
public:
	CDropsProgramName() : CFakeProcessEnvironment( { kArgs + 1, kArgs + 3 }, kVars ) {}
};

// DEFECT: truncates into a short buffer instead of refusing.
class CPartialWrite : public CBase
{
public:
	int GetVariable( const char *name, char *buffer, int size ) const override
	{
		char full[256];
		const int n = CBase::GetVariable( name, full, sizeof( full ) );
		if ( n < 0 || buffer == nullptr || size <= 0 )
		{
			return n < 0 ? n : -1;
		}
		const int w = n < size - 1 ? n : size - 1;
		std::memcpy( buffer, full, w );
		buffer[w] = '\0';
		return w;
	}
};

// DEFECT: case-insensitive lookup (Windows-style).
class CCaseInsensitive : public CBase
{
public:
	int VariableLength( const char *name ) const override
	{
		return CBase::VariableLength( Upper( name ).c_str() );
	}
	int GetVariable( const char *name, char *b, int s ) const override
	{
		return CBase::GetVariable( Upper( name ).c_str(), b, s );
	}

private:
	static std::string Upper( const char *name )
	{
		std::string s = name != nullptr ? name : "";
		for ( char &c : s )
		{
			c = static_cast<char>( std::toupper( static_cast<unsigned char>( c ) ) );
		}
		return s;
	}
};

// DEFECT: an empty value is reported as absent.
class CEmptyIsAbsent : public CBase
{
public:
	int VariableLength( const char *name ) const override
	{
		const int n = CBase::VariableLength( name );
		return n == 0 ? -1 : n;
	}
	int GetVariable( const char *name, char *b, int s ) const override
	{
		return VariableLength( name ) < 0 ? -1 : CBase::GetVariable( name, b, s );
	}
};

// DEFECT: a name containing '=' matches the part before it.
class CSplitsOnEquals : public CBase
{
public:
	int VariableLength( const char *name ) const override
	{
		return name != nullptr && std::strcmp( name, "A=B" ) == 0 ? 1
		                                                          : CBase::VariableLength( name );
	}
};

// DEFECT: Length disagrees with Get (counts the NUL).
class CLengthCountsNul : public CBase
{
public:
	int ArgumentLength( int index ) const override
	{
		const int n = CBase::ArgumentLength( index );
		return n < 0 ? n : n + 1;
	}
};

// DEFECT: an out-of-range index returns the last argument.
class CClampsIndex : public CBase
{
public:
	int GetArgument( int index, char *b, int s ) const override
	{
		if ( index >= ArgumentCount() )
		{
			index = ArgumentCount() - 1;
		}
		return CBase::GetArgument( index, b, s );
	}
};

// DEFECT: process id 0.
class CZeroPid : public CBase
{
public:
	std::uint64_t ProcessId() const override { return 0; }
};

template <typename T> bool Caught()
{
	T env;
	return platformtest::RunProcessEnvironmentConformance( env, Fixture() ).failures > 0;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	{
		CBase good;
		const platformtest::ProcessEnvironmentReport r =
		    platformtest::RunProcessEnvironmentConformance( good, Fixture() );
		++checks;
		if ( r.failures != 0 )
		{
			std::printf( "FAIL: conforming provider rejected (%d/%d); first: %s (line %d)\n",
			    r.failures, r.checks, r.firstFailure, r.firstFailureLine );
			++failures;
		}
	}
	struct Case
	{
		bool ( *caught )();
		const char *name;
	};
	const Case cases[] = {
	    { Caught<CDropsProgramName>, "drops-program-name" },
	    { Caught<CPartialWrite>, "partial-write" },
	    { Caught<CCaseInsensitive>, "case-insensitive-names" },
	    { Caught<CEmptyIsAbsent>, "empty-value-absent" },
	    { Caught<CSplitsOnEquals>, "accepts-name-with-equals" },
	    { Caught<CLengthCountsNul>, "length-counts-nul" },
	    { Caught<CClampsIndex>, "clamps-out-of-range-index" },
	    { Caught<CZeroPid>, "zero-process-id" },
	};
	for ( const Case &c : cases )
	{
		++checks;
		if ( !c.caught() )
		{
			std::printf( "FAIL: broken provider '%s' was NOT caught\n", c.name );
			++failures;
		}
	}
	if ( failures == 0 )
	{
		std::printf( "ok test_process_environment_negative: all %zu broken providers caught\n",
		    sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
