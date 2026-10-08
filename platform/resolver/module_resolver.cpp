//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The portable module resolver (platform.module-resolver.v1, R11).
//
//=============================================================================//

#include "module_resolver.h"

namespace platform
{
namespace
{

bool ValidPolicy( const ModuleSearchPolicy &policy )
{
	if ( policy.roots.empty() || policy.patterns.empty() )
	{
		return false;
	}
	for ( const NativePath &root : policy.roots )
	{
		if ( root.IsEmpty() )
		{
			return false;
		}
	}
	for ( const ModulePattern &pattern : policy.patterns )
	{
		if ( pattern.prefix.find( '/' ) != std::string::npos ||
		     pattern.prefix.find( '\\' ) != std::string::npos ||
		     pattern.prefix.find( '\0' ) != std::string::npos )
		{
			return false;
		}
		if ( !pattern.directory.empty() && !VirtualPath::Parse( pattern.directory ) )
		{
			return false;
		}
	}
	return true;
}

class CModuleResolver final : public IModuleResolver
{
public:
	CModuleResolver( const IFileProbe &probe, IModuleVerifier *verifier )
	    : m_probe( probe ), m_verifier( verifier )
	{
	}

	foundation::Expected<ResolvedModule, ResolveFailure> Resolve(
	    std::string_view moduleName, const ModuleSearchPolicy &policy ) const override
	{
		ResolveFailure failure;
		if ( !ValidPolicy( policy ) )
		{
			failure.status = ResolveStatus::kInvalidPolicy;
			return foundation::MakeUnexpected( std::move( failure ) );
		}
		if ( !VirtualPath::Parse( moduleName ) )
		{
			failure.status = ResolveStatus::kInvalidName;
			return foundation::MakeUnexpected( std::move( failure ) );
		}
		const std::string fileName = ModuleFileName( moduleName, policy );

		for ( std::size_t r = 0; r < policy.roots.size(); ++r )
		{
			for ( std::size_t p = 0; p < policy.patterns.size(); ++p )
			{
				const ModulePattern &pattern = policy.patterns[p];
				std::string relative;
				if ( !pattern.directory.empty() )
				{
					relative = pattern.directory + "/";
				}
				relative += pattern.prefix + fileName;
				auto child = VirtualPath::Parse( relative );
				if ( !child )
				{
					failure.status = ResolveStatus::kInvalidName;
					return foundation::MakeUnexpected( std::move( failure ) );
				}
				auto candidate = policy.roots[r].Join( child.Value() );
				if ( !candidate )
				{
					failure.status = ResolveStatus::kInvalidPolicy;
					return foundation::MakeUnexpected( std::move( failure ) );
				}
				failure.attempted.push_back( candidate.Value() );
				const FileKind kind = m_probe.Probe( candidate.Value() );
				const bool qualifies = policy.acceptAnyExisting ? kind != FileKind::kMissing
				                                                : kind == FileKind::kRegularFile;
				if ( !qualifies )
				{
					continue;
				}
				if ( m_verifier != nullptr &&
				     m_verifier->VerifyCandidate( candidate.Value() ) == VerifyResult::kRejected )
				{
					failure.status = ResolveStatus::kRejected;
					return foundation::MakeUnexpected( std::move( failure ) );
				}
				ResolvedModule resolved;
				resolved.path = std::move( candidate ).Value();
				resolved.root = r;
				resolved.pattern = p;
				return resolved;
			}
		}
		failure.status = ResolveStatus::kNotFound;
		return foundation::MakeUnexpected( std::move( failure ) );
	}

private:
	const IFileProbe &m_probe;
	IModuleVerifier *m_verifier;
};

} // namespace

std::string ModuleFileName( std::string_view moduleName, const ModuleSearchPolicy &policy )
{
	std::string name( moduleName );
	if ( policy.replaceExtension )
	{
		// The last '.' after the last '/', unless it is the first character.
		const std::size_t slash = name.rfind( '/' );
		const std::size_t dot = name.rfind( '.' );
		if ( dot != std::string::npos && dot > 0 && ( slash == std::string::npos || dot > slash ) )
		{
			name.erase( dot );
		}
	}
	if ( !policy.extension.empty() && policy.extension[0] != '.' )
	{
		name += '.';
	}
	name += policy.extension;
	return name;
}

std::unique_ptr<IModuleResolver> CreateModuleResolver(
    const IFileProbe &probe, IModuleVerifier *verifier )
{
	return std::make_unique<CModuleResolver>( probe, verifier );
}

} // namespace platform
