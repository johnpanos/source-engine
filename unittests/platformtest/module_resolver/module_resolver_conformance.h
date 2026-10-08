//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for platform.module-resolver.v1 (R11).
//			Every resolver claiming the contract runs THIS predicate, built by a
//			factory over the suite's own fake probe and verifier, so search order,
//			the attempted list, the extension rule, file kinds and verifier
//			policy are judged on known inputs. Includes POSIX-byte and UTF-16
//			roots (the encoding cases).
//
//=============================================================================//

#ifndef PLATFORMTEST_MODULE_RESOLVER_CONFORMANCE_H
#define PLATFORMTEST_MODULE_RESOLVER_CONFORMANCE_H

#include "platform/contracts/module_resolver.h"
#include "../../../platform/native_path/native_path_access.h"

#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace platformtest
{

struct ResolverReport
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

#define MR_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

inline platform::NativePath Posix( const std::string &bytes )
{
	return platform::NativePathAccess::FromPosixBytes( bytes );
}

inline platform::NativePath Windows( const std::u16string &units )
{
	return platform::NativePathAccess::FromWindowsUnits( units );
}

// A probe over an in-memory set of paths; counts its calls.
class CFakeProbe final : public platform::IFileProbe
{
public:
	void Add( const platform::NativePath &path, platform::FileKind kind )
	{
		m_entries.push_back( { path, kind } );
	}

	platform::FileKind Probe( const platform::NativePath &path ) const override
	{
		++calls;
		for ( const auto &entry : m_entries )
		{
			if ( entry.first == path )
			{
				return entry.second;
			}
		}
		return platform::FileKind::kMissing;
	}

	mutable int calls = 0;

private:
	std::vector<std::pair<platform::NativePath, platform::FileKind>> m_entries;
};

class CFakeVerifier final : public platform::IModuleVerifier
{
public:
	explicit CFakeVerifier( bool accept ) : m_accept( accept ) {}

	platform::VerifyResult Verify( const platform::NativePath &candidate ) override
	{
		seen.push_back( candidate );
		return m_accept ? platform::VerifyResult::kAccepted : platform::VerifyResult::kRejected;
	}

	std::vector<platform::NativePath> seen;

private:
	bool m_accept;
};

using ResolverFactory = std::function<std::unique_ptr<platform::IModuleResolver>(
    const platform::IFileProbe &, platform::IModuleVerifier * )>;

// The legacy POSIX bridge's patterns: bin/lib, bin/, lib, plain.
inline platform::ModuleSearchPolicy LegacyPolicy( std::vector<platform::NativePath> roots )
{
	platform::ModuleSearchPolicy policy;
	policy.roots = std::move( roots );
	policy.patterns = { { "bin", "lib" }, { "bin", "" }, { "", "lib" }, { "", "" } };
	policy.extension = ".so";
	return policy;
}

inline ResolverReport RunModuleResolverConformance( const ResolverFactory &make )
{
	using platform::FileKind;
	using platform::ModuleSearchPolicy;
	using platform::ResolveStatus;

	ResolverReport r;
	const platform::NativePath rootA = Posix( "/opt/game" );
	const platform::NativePath rootB = Posix( "/opt/mod/" ); // trailing '/' joins once

	// Root-major order; the first qualifying candidate wins and nothing after
	// it is probed.
	{
		CFakeProbe probe;
		probe.Add( Posix( "/opt/game/libengine.so" ), FileKind::kRegularFile );
		probe.Add( Posix( "/opt/mod/bin/libengine.so" ), FileKind::kRegularFile );
		auto resolver = make( probe, nullptr );
		auto result = resolver->Resolve( "engine", LegacyPolicy( { rootB, rootA } ) );
		MR_CHECK( r, result.HasValue() );
		MR_CHECK( r, result && result.Value().path == Posix( "/opt/mod/bin/libengine.so" ) );
		MR_CHECK( r, result && result.Value().root == 0 && result.Value().pattern == 0 );
		MR_CHECK( r, probe.calls == 1 );
	}
	{
		CFakeProbe probe;
		probe.Add( Posix( "/opt/game/engine.so" ), FileKind::kRegularFile );
		probe.Add( Posix( "/opt/mod/engine.so" ), FileKind::kRegularFile );
		auto resolver = make( probe, nullptr );
		auto result = resolver->Resolve( "engine", LegacyPolicy( { rootA, rootB } ) );
		MR_CHECK( r, result && result.Value().path == Posix( "/opt/game/engine.so" ) );
		MR_CHECK( r, result && result.Value().root == 0 && result.Value().pattern == 3 );
		MR_CHECK( r, probe.calls == 4 );
	}

	// Not found: every candidate, in order, exactly once.
	{
		CFakeProbe probe;
		auto resolver = make( probe, nullptr );
		auto result = resolver->Resolve( "server", LegacyPolicy( { rootA, rootB } ) );
		MR_CHECK( r, !result.HasValue() );
		if ( !result.HasValue() )
		{
			const auto &failure = result.Error();
			const platform::NativePath expected[] = { Posix( "/opt/game/bin/libserver.so" ),
			    Posix( "/opt/game/bin/server.so" ), Posix( "/opt/game/libserver.so" ),
			    Posix( "/opt/game/server.so" ), Posix( "/opt/mod/bin/libserver.so" ),
			    Posix( "/opt/mod/bin/server.so" ), Posix( "/opt/mod/libserver.so" ),
			    Posix( "/opt/mod/server.so" ) };
			MR_CHECK( r, failure.status == ResolveStatus::kNotFound );
			MR_CHECK( r, failure.attempted.size() == 8 );
			for ( std::size_t i = 0; i < 8 && i < failure.attempted.size(); ++i )
			{
				r.Record( failure.attempted[i] == expected[i], "attempted order", __LINE__ );
			}
		}
		MR_CHECK( r, probe.calls == 8 );
	}

	// The extension rule.
	struct ExtensionCase
	{
		const char *name;
		const char *extension;
		bool replace;
		const char *file;
	};
	const ExtensionCase extensions[] = {
	    { "engine", ".so", true, "engine.so" },
	    { "engine.dll", ".so", true, "engine.so" },
	    { "engine.dll", ".so", false, "engine.dll.so" },
	    { "engine.", ".so", true, "engine.so" },
	    { ".so", ".so", true, ".so.so" },
	    { "a.b/c", ".so", true, "a.b/c.so" },
	    { "a/b.c", "dylib", true, "a/b.dylib" },
	};
	for ( const ExtensionCase &e : extensions )
	{
		CFakeProbe probe;
		probe.Add( Posix( std::string( "/opt/game/" ) + e.file ), FileKind::kRegularFile );
		auto resolver = make( probe, nullptr );
		ModuleSearchPolicy policy;
		policy.roots = { rootA };
		policy.patterns = { { "", "" } };
		policy.extension = e.extension;
		policy.replaceExtension = e.replace;
		auto result = resolver->Resolve( e.name, policy );
		r.Record( result && result.Value().path == Posix( std::string( "/opt/game/" ) + e.file ),
		    "extension rule", __LINE__ );
	}

	// File kinds: only regular files by default; anything existing when asked.
	for ( const FileKind kind : { FileKind::kDirectory, FileKind::kOther } )
	{
		CFakeProbe probe;
		probe.Add( Posix( "/opt/game/libengine.so" ), kind );
		probe.Add( Posix( "/opt/game/engine.so" ), FileKind::kRegularFile );
		auto resolver = make( probe, nullptr );
		ModuleSearchPolicy policy = LegacyPolicy( { rootA } );
		auto strict = resolver->Resolve( "engine", policy );
		MR_CHECK( r, strict && strict.Value().path == Posix( "/opt/game/engine.so" ) );
		policy.acceptAnyExisting = true;
		auto loose = resolver->Resolve( "engine", policy );
		MR_CHECK( r, loose && loose.Value().path == Posix( "/opt/game/libengine.so" ) );
	}

	// Verifier: sees only the first qualifying candidate; a rejection ends the
	// search and names the refused candidate last.
	{
		CFakeProbe probe;
		probe.Add( Posix( "/opt/game/bin/engine.so" ), FileKind::kRegularFile );
		probe.Add( Posix( "/opt/game/engine.so" ), FileKind::kRegularFile );
		CFakeVerifier refuse( false );
		auto resolver = make( probe, &refuse );
		auto result = resolver->Resolve( "engine", LegacyPolicy( { rootA } ) );
		MR_CHECK( r, !result.HasValue() );
		MR_CHECK( r, !result.HasValue() && result.Error().status == ResolveStatus::kRejected );
		MR_CHECK( r, !result.HasValue() && result.Error().attempted.size() == 2 &&
		                 result.Error().attempted.back() == Posix( "/opt/game/bin/engine.so" ) );
		MR_CHECK( r, refuse.seen.size() == 1 );
		MR_CHECK( r, probe.calls == 2 );

		CFakeVerifier accept( true );
		auto accepting = make( probe, &accept );
		auto ok = accepting->Resolve( "engine", LegacyPolicy( { rootA } ) );
		MR_CHECK( r, ok && ok.Value().path == Posix( "/opt/game/bin/engine.so" ) );
		MR_CHECK( r, accept.seen.size() == 1 );
	}

	// Invalid names never reach the probe.
	for ( const char *name : { "", "/abs", "a/../b", "a\\b", "a//b", "./a", "bad\xff" } )
	{
		CFakeProbe probe;
		auto resolver = make( probe, nullptr );
		auto result = resolver->Resolve( name, LegacyPolicy( { rootA } ) );
		r.Record( !result.HasValue() && result.Error().status == ResolveStatus::kInvalidName &&
		              probe.calls == 0,
		    "invalid name refused before probing", __LINE__ );
	}

	// Invalid policies.
	{
		ModuleSearchPolicy noRoots = LegacyPolicy( {} );
		ModuleSearchPolicy noPatterns = LegacyPolicy( { rootA } );
		noPatterns.patterns.clear();
		ModuleSearchPolicy emptyRoot = LegacyPolicy( { rootA, platform::NativePath() } );
		ModuleSearchPolicy badDirectory = LegacyPolicy( { rootA } );
		badDirectory.patterns = { { "../bin", "" } };
		ModuleSearchPolicy badPrefix = LegacyPolicy( { rootA } );
		badPrefix.patterns = { { "", "lib/" } };
		for ( const ModuleSearchPolicy *policy :
		    { &noRoots, &noPatterns, &emptyRoot, &badDirectory, &badPrefix } )
		{
			CFakeProbe probe;
			auto resolver = make( probe, nullptr );
			auto result = resolver->Resolve( "engine", *policy );
			r.Record( !result.HasValue() &&
			              result.Error().status == ResolveStatus::kInvalidPolicy &&
			              probe.calls == 0,
			    "invalid policy refused before probing", __LINE__ );
		}
	}

	// UTF-16 roots: '\' separators and a non-BMP name joined as a surrogate pair.
	{
		const platform::NativePath winRoot = Windows( u"C:\\Games\\Portal" );
		const std::u16string expected = u"C:\\Games\\Portal\\bin\\\U0001F600engine.dll";
		CFakeProbe probe;
		probe.Add( Windows( expected ), FileKind::kRegularFile );
		auto resolver = make( probe, nullptr );
		ModuleSearchPolicy policy;
		policy.roots = { winRoot };
		policy.patterns = { { "bin", "\xf0\x9f\x98\x80" } };
		policy.extension = ".dll";
		auto result = resolver->Resolve( "engine", policy );
		MR_CHECK( r, result && result.Value().path == Windows( expected ) );
		MR_CHECK( r, result && result.Value().path.ToDisplayString() ==
		                           "C:\\Games\\Portal\\bin\\\xf0\x9f\x98\x80"
		                           "engine.dll" );
	}

	return r;
}

} // namespace platformtest

#endif // PLATFORMTEST_MODULE_RESOLVER_CONFORMANCE_H
