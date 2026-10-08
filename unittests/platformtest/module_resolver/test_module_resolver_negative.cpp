//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for platform.module-resolver.v1 (R11): the SAME
//			shared predicate must reject resolvers that each break one clause,
//			while the portable resolver passes.
//
//			Build/run: tools/quality/conformance.py check --suite platform.module_resolver.sensitivity
//
//=============================================================================//

#include "module_resolver_conformance.h"
#include "platform/resolver/module_resolver.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

using platform::FileKind;
using platform::IFileProbe;
using platform::IModuleResolver;
using platform::IModuleVerifier;
using platform::ModuleSearchPolicy;
using platform::NativePath;
using platform::ResolvedModule;
using platform::ResolveFailure;
using platform::ResolveStatus;
using Result = foundation::Expected<ResolvedModule, ResolveFailure>;

// Searches pattern-major instead of root-major.
class CPatternMajor final : public IModuleResolver
{
public:
	CPatternMajor( const IFileProbe &probe, IModuleVerifier *v )
	    : m_inner( platform::CreateModuleResolver( probe, v ) )
	{
	}
	Result Resolve( std::string_view name, const ModuleSearchPolicy &policy ) const override
	{
		ResolveFailure all;
		for ( std::size_t p = 0; p < policy.patterns.size(); ++p )
		{
			for ( std::size_t root = 0; root < policy.roots.size(); ++root )
			{
				ModuleSearchPolicy one = policy;
				one.roots = { policy.roots[root] };
				one.patterns = { policy.patterns[p] };
				auto result = m_inner->Resolve( name, one );
				if ( result )
				{
					result.Value().root = root;
					result.Value().pattern = p;
					return result;
				}
				if ( result.Error().status != ResolveStatus::kNotFound )
				{
					return result;
				}
				all.attempted.insert( all.attempted.end(), result.Error().attempted.begin(),
				    result.Error().attempted.end() );
			}
		}
		return foundation::MakeUnexpected( all );
	}

private:
	std::unique_ptr<IModuleResolver> m_inner;
};

// Wraps the real resolver and corrupts one aspect of its result.
template <typename Corrupt> class CCorrupting final : public IModuleResolver
{
public:
	CCorrupting( const IFileProbe &probe, IModuleVerifier *v, bool keepVerifier = true )
	    : m_inner( platform::CreateModuleResolver( probe, keepVerifier ? v : nullptr ) )
	{
	}
	Result Resolve( std::string_view name, const ModuleSearchPolicy &policy ) const override
	{
		return Corrupt()( *m_inner, name, policy );
	}

private:
	std::unique_ptr<IModuleResolver> m_inner;
};

struct AlwaysAppend // never replaces an existing extension
{
	Result operator()( const IModuleResolver &inner, std::string_view name,
	    const ModuleSearchPolicy &policy ) const
	{
		ModuleSearchPolicy p = policy;
		p.replaceExtension = false;
		return inner.Resolve( name, p );
	}
};

struct AcceptsDirectories
{
	Result operator()( const IModuleResolver &inner, std::string_view name,
	    const ModuleSearchPolicy &policy ) const
	{
		ModuleSearchPolicy p = policy;
		p.acceptAnyExisting = true;
		return inner.Resolve( name, p );
	}
};

struct DropsAttempts
{
	Result operator()( const IModuleResolver &inner, std::string_view name,
	    const ModuleSearchPolicy &policy ) const
	{
		auto result = inner.Resolve( name, policy );
		if ( !result && result.Error().attempted.size() > 1 )
		{
			ResolveFailure f = result.Error();
			f.attempted.pop_back();
			return foundation::MakeUnexpected( f );
		}
		return result;
	}
};

struct NotFoundForEverything // reports kNotFound for invalid names and policies
{
	Result operator()( const IModuleResolver &inner, std::string_view name,
	    const ModuleSearchPolicy &policy ) const
	{
		auto result = inner.Resolve( name, policy );
		if ( !result )
		{
			ResolveFailure f = result.Error();
			f.status = ResolveStatus::kNotFound;
			return foundation::MakeUnexpected( f );
		}
		return result;
	}
};

// Skips the verifier entirely.
class CSkipsVerifier final : public IModuleResolver
{
public:
	CSkipsVerifier( const IFileProbe &probe, IModuleVerifier * )
	    : m_inner( platform::CreateModuleResolver( probe, nullptr ) )
	{
	}
	Result Resolve( std::string_view name, const ModuleSearchPolicy &policy ) const override
	{
		return m_inner->Resolve( name, policy );
	}

private:
	std::unique_ptr<IModuleResolver> m_inner;
};

// After a rejection, falls through to the next candidate.
class CFallsThroughRejection final : public IModuleResolver
{
public:
	CFallsThroughRejection( const IFileProbe &probe, IModuleVerifier *v )
	    : m_probe( probe ), m_verifier( v ),
	      m_plain( platform::CreateModuleResolver( probe, nullptr ) )
	{
	}
	Result Resolve( std::string_view name, const ModuleSearchPolicy &policy ) const override
	{
		auto first = platform::CreateModuleResolver( m_probe, m_verifier )->Resolve( name, policy );
		if ( first || first.Error().status != ResolveStatus::kRejected )
		{
			return first;
		}
		// Retry with the refused candidate's pattern removed.
		ModuleSearchPolicy p = policy;
		auto plain = m_plain->Resolve( name, policy );
		if ( plain && p.patterns.size() > 1 )
		{
			p.patterns.erase( p.patterns.begin() + static_cast<long>( plain.Value().pattern ) );
			return m_plain->Resolve( name, p );
		}
		return first;
	}

private:
	const IFileProbe &m_probe;
	IModuleVerifier *m_verifier;
	std::unique_ptr<IModuleResolver> m_plain;
};

// Keeps probing after the hit (wasted work and a wrong attempted list).
class CProbesEverything final : public IModuleResolver
{
public:
	CProbesEverything( const IFileProbe &probe, IModuleVerifier *v )
	    : m_probe( probe ), m_inner( platform::CreateModuleResolver( probe, v ) )
	{
	}
	Result Resolve( std::string_view name, const ModuleSearchPolicy &policy ) const override
	{
		auto result = m_inner->Resolve( name, policy );
		if ( result )
		{
			for ( const NativePath &root : policy.roots )
			{
				m_probe.Probe( root );
			}
		}
		return result;
	}

private:
	const IFileProbe &m_probe;
	std::unique_ptr<IModuleResolver> m_inner;
};

template <typename T> bool Caught()
{
	return platformtest::RunModuleResolverConformance(
	           []( const IFileProbe &p, IModuleVerifier *v )
	           {
		           return std::unique_ptr<IModuleResolver>( new T( p, v ) );
	           } )
	           .failures > 0;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	{
		const auto r = platformtest::RunModuleResolverConformance(
		    []( const IFileProbe &p, IModuleVerifier *v )
		    {
			    return platform::CreateModuleResolver( p, v );
		    } );
		++checks;
		if ( r.failures != 0 )
		{
			std::printf( "FAIL: the portable resolver was rejected (%d/%d); first: %s (line %d)\n",
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
	    { Caught<CPatternMajor>, "pattern-major-order" },
	    { Caught<CCorrupting<AlwaysAppend>>, "extension-never-replaced" },
	    { Caught<CCorrupting<AcceptsDirectories>>, "accepts-directories" },
	    { Caught<CCorrupting<DropsAttempts>>, "drops-attempted-candidates" },
	    { Caught<CCorrupting<NotFoundForEverything>>, "invalid-input-reported-not-found" },
	    { Caught<CSkipsVerifier>, "skips-verifier" },
	    { Caught<CFallsThroughRejection>, "falls-through-a-rejection" },
	    { Caught<CProbesEverything>, "probes-after-the-hit" },
	};
	for ( const Case &c : cases )
	{
		++checks;
		if ( !c.caught() )
		{
			std::printf( "FAIL: broken resolver '%s' was NOT caught\n", c.name );
			++failures;
		}
	}
	if ( failures == 0 )
	{
		std::printf( "ok test_module_resolver_negative: all %zu broken resolvers caught\n",
		    sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
