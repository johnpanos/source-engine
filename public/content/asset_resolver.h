//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0015 read-only mounted asset lookup and reference closure.
//
//=============================================================================//

#ifndef CONTENT_ASSET_RESOLVER_H
#define CONTENT_ASSET_RESOLVER_H

#include "content/asset_index.h"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace content
{

enum class AssetLookupStatus
{
	Indexed,
	Unindexed,
	MissingFile,
	InvalidIndex,
};

struct AssetLookup
{
	AssetLookupStatus status = AssetLookupStatus::Unindexed;
	std::filesystem::path path;
	const AssetEntry *entry = nullptr; // borrowed; resolver outlives lookup
};

class AssetResolver
{
public:
	explicit AssetResolver( std::filesystem::path root );
	AssetLookup Find( const AssetRef &ref, std::span<const std::string_view> variants = {} ) const;
	std::vector<AssetEdge> References( const AssetRef &ref ) const;
	// Reports the first required reference absent from this package, naming its
	// referrer. Optional edges do not block a closure.
	std::optional<std::string> CheckClosure( std::span<const AssetRef> roots ) const;
	bool HasIndex() const noexcept { return m_Index.has_value(); }
	bool InvalidIndex() const noexcept { return m_InvalidIndex; }
	const std::string &Error() const noexcept { return m_Error; }

private:
	std::filesystem::path m_Root;
	std::optional<AssetIndex> m_Index;
	bool m_InvalidIndex = false;
	std::string m_Error;
};

} // namespace content

#endif // CONTENT_ASSET_RESOLVER_H
