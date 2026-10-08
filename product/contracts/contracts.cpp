//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.contracts: shared vocabulary and the ProviderCatalog
//			(RFC 0027).
//
//=============================================================================//

#include "product/contracts.h"

#include <algorithm>
#include <cstdio>

namespace product
{

using foundation::json::Value;

std::uint64_t Hash64( std::string_view bytes )
{
	std::uint64_t hash = 1469598103934665603ull;
	for ( const char c : bytes )
	{
		hash ^= static_cast<unsigned char>( c );
		hash *= 1099511628211ull;
	}
	return hash;
}

namespace
{
std::string Hex64( std::uint64_t value )
{
	char text[17];
	std::snprintf( text, sizeof( text ), "%016llx", static_cast<unsigned long long>( value ) );
	return text;
}
} // namespace

std::string HashHex( std::string_view bytes )
{
	return Hex64( Hash64( bytes ) );
}

std::uint64_t Identity::Digest() const
{
	std::string text = "provider=" + provider + "\n";
	for ( const auto &fact : facts )
		text += fact.first + "=" + fact.second + "\n";
	return Hash64( text );
}

std::string Identity::Hex() const
{
	return Hex64( Digest() );
}

std::string RecipeKey( const Recipe &recipe, const Identity &toolchain )
{
	return HashHex( recipe.name + "\n" + recipe.pin + "\n" + recipe.definition.Write() + "\n" +
	                toolchain.Hex() );
}

std::string_view StageRoleName( StageRole role )
{
	switch ( role )
	{
	case StageRole::kDeps:
		return "deps";
	case StageRole::kEngine:
		return "engine";
	case StageRole::kContent:
		return "content";
	case StageRole::kPackage:
		return "package";
	case StageRole::kDeploy:
		return "deploy";
	case StageRole::kRun:
		return "run";
	case StageRole::kTest:
		return "test";
	case StageRole::kExtra:
		return "extra";
	}
	return "extra";
}

std::optional<StageRole> ParseStageRole( std::string_view name )
{
	for ( StageRole role :
	    { StageRole::kDeps, StageRole::kEngine, StageRole::kContent, StageRole::kPackage,
	        StageRole::kDeploy, StageRole::kRun, StageRole::kTest, StageRole::kExtra } )
	{
		if ( StageRoleName( role ) == name )
			return role;
	}
	return std::nullopt;
}

StageInputs::StageInputs( const ResolvedProfile &profile, std::string flavor,
    std::filesystem::path sourceRoot, std::filesystem::path treeRoot,
    std::vector<std::string> declared, const std::map<std::string, Artifact> &available,
    const ToolchainEnvironment *toolchain, platform::IToolProcessProvider *processes,
    IDiagnosticSink *diagnostics, const ICancellation *cancel )
    : m_Profile( profile ), m_Flavor( std::move( flavor ) ),
      m_SourceRoot( std::move( sourceRoot ) ), m_TreeRoot( std::move( treeRoot ) ),
      m_Declared( std::move( declared ) ), m_Available( available ), m_Toolchain( toolchain ),
      m_Processes( processes ), m_Diagnostics( diagnostics ), m_Cancel( cancel )
{
}

const Artifact *StageInputs::Get( std::string_view name )
{
	if ( std::find( m_Declared.begin(), m_Declared.end(), name ) == m_Declared.end() )
	{
		m_Undeclared.emplace_back( name );
		return nullptr;
	}
	const auto it = m_Available.find( std::string( name ) );
	return it == m_Available.end() ? nullptr : &it->second;
}

std::string PackageManifest::Digest() const
{
	return HashHex( ToJson().Write() );
}

Value PackageManifest::ToJson() const
{
	Value root = Value::Object();
	root.Set( "form", Value::String( form ) );
	Value &files = root.Set( "files", Value::Array() );
	for ( const ManifestEntry &entry : entries )
	{
		Value item = Value::Object();
		item.Set( "path", Value::String( entry.path ) );
		item.Set( "hash", Value::String( entry.hash ) );
		item.Set( "role", Value::String( entry.role ) );
		item.Set( "bytes", Value::Number( static_cast<long long>( entry.bytes ) ) );
		files.Push( std::move( item ) );
	}
	return root;
}

bool TransportHasCapability( IDeployTransport &transport, std::string_view capability )
{
	if ( capability == "install" )
		return transport.Install() != nullptr;
	if ( capability == "content-sync" )
		return transport.ContentSync() != nullptr;
	if ( capability == "launch" )
		return transport.Launch() != nullptr;
	if ( capability == "device-facts" )
		return transport.DeviceFacts() != nullptr;
	if ( capability == "log-stream" )
		return transport.LogStream() != nullptr;
	if ( capability == "crash-collect" )
		return transport.CrashCollect() != nullptr;
	if ( capability == "attach" )
		return transport.Attach() != nullptr;
	return false;
}

std::vector<std::string> MissingCapabilities(
    IDeployTransport &transport, const std::vector<std::string> &required )
{
	std::vector<std::string> missing;
	for ( const std::string &capability : required )
	{
		if ( !TransportHasCapability( transport, capability ) )
			missing.push_back( capability );
	}
	return missing;
}

std::string CatalogError::Describe() const
{
	return contract + " \"" + name + "\": " + detail;
}

namespace
{
template <typename T>
foundation::Expected<void, CatalogError> AddUnique(
    std::vector<std::unique_ptr<T>> &list, std::unique_ptr<T> provider, const char *contract )
{
	if ( !provider )
		return foundation::MakeUnexpected( CatalogError{ contract, "", "a null provider" } );
	const std::string_view name = provider->Name();
	if ( name.empty() )
		return foundation::MakeUnexpected(
		    CatalogError{ contract, "", "a provider needs a name" } );
	for ( const auto &existing : list )
	{
		if ( existing->Name() == name )
			return foundation::MakeUnexpected(
			    CatalogError{ contract, std::string( name ), "already in the catalog" } );
	}
	list.push_back( std::move( provider ) );
	return {};
}

template <typename T>
foundation::Expected<T *, CatalogError> FindNamed(
    const std::vector<std::unique_ptr<T>> &list, std::string_view name, const char *contract )
{
	for ( const auto &provider : list )
	{
		if ( provider->Name() == name )
			return provider.get();
	}
	return foundation::MakeUnexpected( CatalogError{
	    contract, std::string( name ), "not in the catalog (there is no fallback provider)" } );
}

template <typename T> std::set<std::string> NamesOf( const std::vector<std::unique_ptr<T>> &list )
{
	std::set<std::string> names;
	for ( const auto &provider : list )
		names.emplace( provider->Name() );
	return names;
}
} // namespace

foundation::Expected<void, CatalogError> ProviderCatalog::Add(
    std::unique_ptr<ITargetToolchain> provider )
{
	return AddUnique( m_Toolchains, std::move( provider ), "toolchain" );
}
foundation::Expected<void, CatalogError> ProviderCatalog::Add(
    std::unique_ptr<IRecipeBuilder> provider )
{
	return AddUnique( m_RecipeBuilders, std::move( provider ), "recipe builder" );
}
foundation::Expected<void, CatalogError> ProviderCatalog::Add(
    std::unique_ptr<IProductStage> provider )
{
	return AddUnique( m_Stages, std::move( provider ), "stage" );
}
foundation::Expected<void, CatalogError> ProviderCatalog::Add( std::unique_ptr<IPackager> provider )
{
	return AddUnique( m_Packagers, std::move( provider ), "packager" );
}
foundation::Expected<void, CatalogError> ProviderCatalog::Add(
    std::unique_ptr<IDeployTransport> provider )
{
	return AddUnique( m_Transports, std::move( provider ), "transport" );
}
foundation::Expected<void, CatalogError> ProviderCatalog::Add(
    std::unique_ptr<IDisplaySession> provider )
{
	return AddUnique( m_DisplaySessions, std::move( provider ), "display session" );
}
foundation::Expected<void, CatalogError> ProviderCatalog::AddRunProviderName( std::string name )
{
	if ( name.empty() ||
	     std::find( m_RunProviders.begin(), m_RunProviders.end(), name ) != m_RunProviders.end() )
		return foundation::MakeUnexpected(
		    CatalogError{ "run", name, "empty or already in the catalog" } );
	m_RunProviders.push_back( std::move( name ) );
	return {};
}

foundation::Expected<ITargetToolchain *, CatalogError> ProviderCatalog::Toolchain(
    std::string_view name ) const
{
	return FindNamed( m_Toolchains, name, "toolchain" );
}
foundation::Expected<IRecipeBuilder *, CatalogError> ProviderCatalog::RecipeBuilder(
    std::string_view name ) const
{
	return FindNamed( m_RecipeBuilders, name, "recipe builder" );
}
foundation::Expected<IProductStage *, CatalogError> ProviderCatalog::Stage(
    std::string_view name ) const
{
	return FindNamed( m_Stages, name, "stage" );
}
foundation::Expected<IPackager *, CatalogError> ProviderCatalog::Packager(
    std::string_view name ) const
{
	return FindNamed( m_Packagers, name, "packager" );
}
foundation::Expected<IDeployTransport *, CatalogError> ProviderCatalog::Transport(
    std::string_view name ) const
{
	return FindNamed( m_Transports, name, "transport" );
}
foundation::Expected<IDisplaySession *, CatalogError> ProviderCatalog::DisplaySession(
    std::string_view name ) const
{
	return FindNamed( m_DisplaySessions, name, "display session" );
}

ProviderNames ProviderCatalog::Names() const
{
	ProviderNames names;
	names.toolchains = NamesOf( m_Toolchains );
	names.stages = NamesOf( m_Stages );
	names.packagers = NamesOf( m_Packagers );
	names.transports = NamesOf( m_Transports );
	names.displaySessions = NamesOf( m_DisplaySessions );
	names.runProviders.insert( m_RunProviders.begin(), m_RunProviders.end() );
	return names;
}

} // namespace product
