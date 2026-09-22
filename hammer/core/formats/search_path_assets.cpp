//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the ordered asset search path. See
//			public/hammer/formats/search_path_assets.h for the contract.
//
//=============================================================================//

#include "hammer/formats/search_path_assets.h"

#include <unordered_set>

namespace hammer::formats
{

void SearchPathAssets::AddProvider( const hammer::ports::IAssetSource *provider )
{
	if ( provider )
		m_providers.push_back( provider );
}

bool SearchPathAssets::HasAsset( const std::string &path ) const
{
	for ( const hammer::ports::IAssetSource *provider : m_providers )
	{
		if ( provider->HasAsset( path ) )
			return true;
	}
	return false;
}

bool SearchPathAssets::ReadAsset( const std::string &path, std::string &out ) const
{
	for ( const hammer::ports::IAssetSource *provider : m_providers )
	{
		if ( provider->ReadAsset( path, out ) )
			return true;
	}
	return false;
}

void SearchPathAssets::ListAssets(
    const std::string &prefix, const std::string &extensionWithDot, std::vector<std::string> &out ) const
{
	std::unordered_set<std::string> seen;
	// Preserve search order, keep the first occurrence of each path.
	for ( const hammer::ports::IAssetSource *provider : m_providers )
	{
		std::vector<std::string> local;
		provider->ListAssets( prefix, extensionWithDot, local );
		for ( std::string &path : local )
		{
			if ( seen.insert( path ).second )
				out.push_back( std::move( path ) );
		}
	}
}

} // namespace hammer::formats
