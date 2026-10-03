//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0015 loose-package index, read by runtime without compilers.
//
//=============================================================================//

#ifndef CONTENT_ASSET_INDEX_H
#define CONTENT_ASSET_INDEX_H

#include "content/asset_identity.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace content
{

struct AssetEntry
{
	AssetRef ref;
	std::string variant;
	std::string location; // relative path inside the loose package
	std::string compiler;
	std::string key;
	std::array<std::uint8_t, 16> hash{};
	std::uint64_t size = 0;
};

struct AssetEdge
{
	AssetRef source;
	AssetRef target;
	bool optional = false;
};

class AssetIndex
{
public:
	static std::optional<AssetIndex> Open(
	    std::span<const std::uint8_t> bytes, std::string *error = nullptr );
	static std::optional<AssetIndex> Read(
	    const std::filesystem::path &path, std::string *error = nullptr );
	static std::optional<std::vector<std::uint8_t>> Write( std::vector<AssetEntry> entries,
	    std::vector<AssetEdge> edges, std::string *error = nullptr );

	const AssetEntry *Find(
	    const AssetRef &ref, std::string_view variant = "legacy" ) const noexcept;
	std::vector<AssetEdge> References( const AssetRef &ref ) const;
	const std::vector<AssetEntry> &Entries() const noexcept { return m_Entries; }

private:
	std::vector<AssetEntry> m_Entries;
	std::vector<AssetEdge> m_Edges;
};

} // namespace content

#endif // CONTENT_ASSET_INDEX_H
