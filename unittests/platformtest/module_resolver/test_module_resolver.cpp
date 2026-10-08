//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive suite for platform.module-resolver.v1 and the path types
//			(R11, Q-FOUNDATION): the shared resolver suite against the portable
//			resolver, then VirtualPath parsing, NativePath joins and lossy
//			display on both flavors.
//
//			Build/run: tools/quality/conformance.py check --suite platform.module_resolver
//
//=============================================================================//

#include "module_resolver_conformance.h"
#include "platform/resolver/module_resolver.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

int g_checks = 0;
int g_failures = 0;

void Check( bool ok, const char *what, int line )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::printf( "FAIL line %d: %s\n", line, what );
	}
}

#define PT_CHECK( cond ) Check( ( cond ), #cond, __LINE__ )

void PathTypes()
{
	using platform::PathError;
	using platform::VirtualPath;

	struct Case
	{
		const char *text;
		PathError error;
	};
	const Case refused[] = {
	    { "", PathError::kEmpty },
	    { "/abs", PathError::kAbsolute },
	    { "C:/x", PathError::kAbsolute },
	    { "a\\b", PathError::kBackslash },
	    { "a//b", PathError::kEmptySegment },
	    { "a/", PathError::kEmptySegment },
	    { "a/./b", PathError::kDotSegment },
	    { "..", PathError::kDotSegment },
	    { "caf\xc3", PathError::kInvalidUtf8 },
	    { "\xc0\xaf", PathError::kInvalidUtf8 },     // overlong '/'
	    { "\xed\xa0\x80", PathError::kInvalidUtf8 }, // surrogate
	    { "\xf4\x90\x80\x80", PathError::kInvalidUtf8 },
	};
	for ( const Case &c : refused )
	{
		auto parsed = VirtualPath::Parse( c.text );
		Check( !parsed.HasValue() && parsed.Error() == c.error, c.text, __LINE__ );
	}
	{
		auto withNul = VirtualPath::Parse( std::string_view( "a\0b", 3 ) );
		PT_CHECK( !withNul.HasValue() && withNul.Error() == PathError::kEmbeddedNul );
	}
	for ( const char *ok :
	    { "a", "bin/engine.so", "caf\xc3\xa9/\xf0\x9f\x98\x80", ".hidden", "a..b" } )
	{
		auto parsed = VirtualPath::Parse( ok );
		Check( parsed.HasValue() && parsed.Value().String() == ok, ok, __LINE__ );
	}
	auto ab = VirtualPath::Parse( "a/b" ).Value();
	auto c = VirtualPath::Parse( "c" ).Value();
	PT_CHECK( ab.Join( c ).String() == "a/b/c" );
	PT_CHECK( ab.Segments().size() == 2 && ab.Segments()[1] == "b" );

	// Joins.
	using platformtest::Posix;
	using platformtest::Windows;
	PT_CHECK( Posix( "/x" ).Join( ab ).Value() == Posix( "/x/a/b" ) );
	PT_CHECK( Posix( "/" ).Join( ab ).Value() == Posix( "/a/b" ) );
	PT_CHECK( Windows( u"C:\\x" ).Join( ab ).Value() == Windows( u"C:\\x\\a\\b" ) );
	PT_CHECK( Windows( u"C:/x/" ).Join( ab ).Value() == Windows( u"C:/x/a\\b" ) );
	PT_CHECK( !platform::NativePath().Join( ab ).HasValue() );

	// Native bytes that are not UTF-8 survive untouched and display lossily.
	const platform::NativePath raw = Posix( "/data/\xff\xfe-mod" );
	PT_CHECK( platform::NativePathAccess::PosixBytes( raw ) == "/data/\xff\xfe-mod" );
	PT_CHECK( raw.ToDisplayString() == "/data/\xef\xbf\xbd\xef\xbf\xbd-mod" );
	PT_CHECK( raw.Join( c ).Value() == Posix( "/data/\xff\xfe-mod/c" ) );
	// An unpaired surrogate likewise.
	const platform::NativePath lone = Windows( std::u16string( u"C:\\a" ) + char16_t( 0xd800 ) );
	PT_CHECK( lone.ToDisplayString() == "C:\\a\xef\xbf\xbd" );
	PT_CHECK( !( raw == lone ) );
	PT_CHECK( platform::NativePath().ToDisplayString().empty() );
}

} // namespace

int main()
{
	const platformtest::ResolverReport r = platformtest::RunModuleResolverConformance(
	    []( const platform::IFileProbe &probe, platform::IModuleVerifier *verifier )
	    {
		    return platform::CreateModuleResolver( probe, verifier );
	    } );
	g_checks += r.checks;
	g_failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL module resolver: %d/%d; first: %s (line %d)\n", r.failures, r.checks,
		    r.firstFailure, r.firstFailureLine );
	}
	else
	{
		std::printf( "ok module resolver: %d checks passed\n", r.checks );
	}
	PathTypes();
	return testing::ReportConformance( g_checks, g_failures );
}
