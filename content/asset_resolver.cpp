//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Index-first resolution; unindexed legacy search stays with hosts.
//
//=============================================================================//

#include "content/asset_resolver.h"

#include <set>

namespace content
{

AssetResolver::AssetResolver( std::filesystem::path root ) : m_Root( std::move( root ) )
{
	const std::filesystem::path path = m_Root / "assets.index";
	if ( std::filesystem::exists( path ) )
	{
		m_Index = AssetIndex::Read( path, &m_Error );
		m_InvalidIndex = !m_Index.has_value();
	}
}

AssetLookup AssetResolver::Find(
    const AssetRef &ref, std::span<const std::string_view> variants ) const
{
	if ( m_InvalidIndex )
		return { AssetLookupStatus::InvalidIndex, {}, nullptr };
	if ( !m_Index )
		return { AssetLookupStatus::Unindexed, {}, nullptr };
	const AssetEntry *entry = nullptr;
	for ( const std::string_view variant : variants )
	{
		entry = m_Index->Find( ref, variant );
		if ( entry )
			break;
	}
	if ( !entry && variants.empty() )
		entry = m_Index->Find( ref );
	if ( !entry )
		return { AssetLookupStatus::Unindexed, {}, nullptr };
	const std::filesystem::path path = m_Root / entry->location;
	if ( !std::filesystem::is_regular_file( path ) )
		return { AssetLookupStatus::MissingFile, path, entry };
	return { AssetLookupStatus::Indexed, path, entry };
}

std::vector<AssetEdge> AssetResolver::References( const AssetRef &ref ) const
{
	return m_Index ? m_Index->References( ref ) : std::vector<AssetEdge>{};
}

std::optional<std::string> AssetResolver::CheckClosure( std::span<const AssetRef> roots ) const
{
	if ( m_InvalidIndex )
		return "invalid asset index: " + m_Error;
	if ( !m_Index )
		return "package has no asset index";
	std::set<AssetRef> visited;
	std::vector<AssetRef> pending( roots.begin(), roots.end() );
	while ( !pending.empty() )
	{
		const AssetRef ref = pending.back();
		pending.pop_back();
		if ( !visited.insert( ref ).second )
			continue;
		if ( Find( ref ).status != AssetLookupStatus::Indexed )
			return "missing root asset " + std::string( AssetKindName( ref.kind ) ) + ':' +
			       ref.name;
		for ( const AssetEdge &edge : References( ref ) )
		{
			if ( edge.optional )
				continue;
			if ( Find( edge.target ).status != AssetLookupStatus::Indexed )
				return "missing " + std::string( AssetKindName( edge.target.kind ) ) + ':' +
				       edge.target.name + " referred by " +
				       std::string( AssetKindName( ref.kind ) ) + ':' + ref.name;
			pending.push_back( edge.target );
		}
	}
	return std::nullopt;
}

} // namespace content
