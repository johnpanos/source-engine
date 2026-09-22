//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: An ordered search path over several asset sources (RFC 0002,
//			hammer.formats). A Source game mounts many providers -- several VPK
//			archives plus loose files under the game directory -- and resolves an
//			asset from the first provider that has it. This composite implements
//			IAssetSource by delegating to an ordered list of other IAssetSources:
//			ReadAsset/HasAsset return the first hit, and ListAssets is the union
//			(de-duplicated) across all of them.
//
//			Strict-core: C++ standard library and the IAssetSource port only. It
//			borrows the providers (raw pointers); they must outlive the search path.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_SEARCH_PATH_ASSETS_H
#define HAMMER_FORMATS_SEARCH_PATH_ASSETS_H

#include "hammer/ports/asset_source.h"

#include <string>
#include <vector>

namespace hammer::formats
{

class SearchPathAssets final : public hammer::ports::IAssetSource
{
public:
	SearchPathAssets() = default;

	// Appends a provider to the end of the search order (lowest priority so far).
	// A null provider is ignored. The provider is borrowed, not owned.
	void AddProvider( const hammer::ports::IAssetSource *provider );

	std::size_t ProviderCount() const { return m_providers.size(); }

	// IAssetSource. First-hit-wins for Has/Read; ListAssets is the de-duplicated
	// union across providers in search order.
	bool HasAsset( const std::string &path ) const override;
	bool ReadAsset( const std::string &path, std::string &out ) const override;
	void ListAssets( const std::string &prefix, const std::string &extensionWithDot,
	    std::vector<std::string> &out ) const override;

private:
	std::vector<const hammer::ports::IAssetSource *> m_providers;
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_SEARCH_PATH_ASSETS_H
