//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0015 asset identity. Legacy paths remain their own formats.
//
//=============================================================================//

#ifndef CONTENT_ASSET_IDENTITY_H
#define CONTENT_ASSET_IDENTITY_H

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace content
{

enum class AssetKind : std::uint8_t
{
	Model = 1,
	Material,
	Texture,
	ParticleFile,
	Soundscript,
	Sound,
	Scene,
	Caption,
	Nav,
	Map,
};

std::string_view AssetKindName( AssetKind kind ) noexcept;
std::optional<AssetKind> AssetKindFromName( std::string_view name ) noexcept;

struct AssetRef
{
	AssetKind kind;
	std::string name;

	static std::optional<AssetRef> Create(
	    AssetKind kind, std::string_view name, bool newSource = false );
	static std::optional<AssetRef> FromRuntimePath( std::string_view path );

	std::string RuntimePath() const;
	std::uint64_t NameHash() const noexcept;
	bool operator==( const AssetRef &other ) const noexcept = default;
	bool operator<( const AssetRef &other ) const noexcept;
};

// ASCII case fold and slash normalization, with no absolute, empty, dot or
// parent components. newSource also enforces RFC 0015's portable set.
std::optional<std::string> NormalizeAssetName( std::string_view name, bool newSource = false );

// NormalizeAssetName's character rule alone (ASCII case fold, '\\' to '/'),
// with no component checks: for comparing name fragments and search patterns
// under the identity rule. Not an identity on its own.
std::string FoldAssetName( std::string_view text );

} // namespace content

#endif // CONTENT_ASSET_IDENTITY_H
