//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: In-memory hammer::ports::IAssetSource fake for the material catalog
//			suites (RFC 0002, hammer.formats). A plain path->bytes map that honors
//			the canonical path convention; no engine, no disk.
//
//=============================================================================//

#ifndef HAMMERTEST_FAKE_ASSET_SOURCE_H
#define HAMMERTEST_FAKE_ASSET_SOURCE_H

#include "hammer/ports/asset_source.h"

#include <map>
#include <string>
#include <vector>

namespace hammertest
{

class InMemoryAssetSource : public hammer::ports::IAssetSource
{
public:
	std::map<std::string, std::string> assets;

	bool HasAsset( const std::string &path ) const override { return assets.count( path ) != 0; }

	bool ReadAsset( const std::string &path, std::string &out ) const override
	{
		auto it = assets.find( path );
		if ( it == assets.end() )
			return false;
		out = it->second;
		return true;
	}

	void ListAssets( const std::string &prefix, const std::string &extensionWithDot,
	    std::vector<std::string> &out ) const override
	{
		for ( const auto &kv : assets )
		{
			const std::string &path = kv.first;
			if ( !prefix.empty() &&
			     ( path.size() < prefix.size() || path.compare( 0, prefix.size(), prefix ) != 0 ) )
				continue;
			if ( !extensionWithDot.empty() &&
			     ( path.size() < extensionWithDot.size() ||
			         path.compare( path.size() - extensionWithDot.size(), extensionWithDot.size(),
			             extensionWithDot ) != 0 ) )
				continue;
			out.push_back( path );
		}
	}
};

} // namespace hammertest

#endif // HAMMERTEST_FAKE_ASSET_SOURCE_H
