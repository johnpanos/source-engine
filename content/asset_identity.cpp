//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Canonical RFC 0015 kind table, names and lookup hash.
//
//=============================================================================//

#include "content/asset_identity.h"
#include "content/hash.h"

#include <algorithm>
#include <array>

namespace content
{
namespace
{
struct KindRule
{
	AssetKind kind;
	std::string_view name;
	std::string_view prefix;
	std::string_view extension;
};

constexpr std::array<KindRule, 10> kKinds = { {
    { AssetKind::Model, "model", "models/", ".mdl" },
    { AssetKind::Material, "material", "materials/", ".vmt" },
    { AssetKind::Texture, "texture", "materials/", ".vtf" },
    { AssetKind::ParticleFile, "particle-file", "particles/", ".pcf" },
    { AssetKind::Soundscript, "soundscript", "scripts/", "" },
    { AssetKind::Sound, "sound", "sound/", "" },
    { AssetKind::Scene, "scene", "scenes/", ".vcd" },
    { AssetKind::Caption, "caption", "resource/", ".dat" },
    { AssetKind::Nav, "nav", "maps/", ".nav" },
    { AssetKind::Map, "map", "maps/", ".bsp" },
} };

const KindRule *Rule( AssetKind kind ) noexcept
{
	for ( const KindRule &rule : kKinds )
		if ( rule.kind == kind )
			return &rule;
	return nullptr;
}

bool Portable( std::string_view part ) noexcept
{
	if ( part.empty() || part.back() == '.' )
		return false;
	for ( const char c : part )
		if ( !( c >= 'a' && c <= 'z' ) && !( c >= '0' && c <= '9' ) && c != '_' && c != '.' &&
		     c != '-' )
			return false;
	const std::string_view stem = part.substr( 0, part.find( '.' ) );
	if ( stem == "con" || stem == "prn" || stem == "aux" || stem == "nul" )
		return false;
	if ( stem.size() == 4 && ( stem.starts_with( "com" ) || stem.starts_with( "lpt" ) ) &&
	     stem.back() >= '1' && stem.back() <= '9' )
		return false;
	return true;
}

bool ValidUtf8( std::string_view text ) noexcept
{
	for ( std::size_t i = 0; i < text.size(); )
	{
		const auto c = static_cast<unsigned char>( text[i] );
		if ( c == 0 )
			return false;
		if ( c < 0x80 )
		{
			++i;
			continue;
		}
		const std::size_t count = c >= 0xF0 && c <= 0xF4   ? 4
		                          : c >= 0xE0 && c <= 0xEF ? 3
		                          : c >= 0xC2 && c <= 0xDF ? 2
		                                                   : 0;
		if ( count == 0 || i + count > text.size() )
			return false;
		for ( std::size_t j = 1; j < count; ++j )
			if ( ( static_cast<unsigned char>( text[i + j] ) & 0xC0 ) != 0x80 )
				return false;
		const auto second = static_cast<unsigned char>( text[i + 1] );
		if ( ( c == 0xE0 && second < 0xA0 ) || ( c == 0xED && second >= 0xA0 ) ||
		     ( c == 0xF0 && second < 0x90 ) || ( c == 0xF4 && second >= 0x90 ) )
			return false;
		i += count;
	}
	return true;
}
} // namespace

std::string_view AssetKindName( AssetKind kind ) noexcept
{
	const KindRule *rule = Rule( kind );
	return rule ? rule->name : std::string_view{};
}

std::optional<AssetKind> AssetKindFromName( std::string_view name ) noexcept
{
	for ( const KindRule &rule : kKinds )
		if ( rule.name == name )
			return rule.kind;
	return std::nullopt;
}

std::optional<std::string> NormalizeAssetName( std::string_view name, bool newSource )
{
	if ( name.empty() || name.front() == '/' || !ValidUtf8( name ) ||
	     ( name.size() >= 2 && name[1] == ':' ) )
		return std::nullopt;
	std::string result;
	result.reserve( name.size() );
	for ( char c : name )
	{
		if ( c == '\\' )
			c = '/';
		if ( c >= 'A' && c <= 'Z' )
			c = char( c + ( 'a' - 'A' ) );
		result += c;
	}
	for ( std::size_t start = 0; start <= result.size(); )
	{
		const std::size_t end = result.find( '/', start );
		const std::string_view part(
		    result.data() + start, ( end == std::string::npos ? result.size() : end ) - start );
		if ( part.empty() || part == "." || part == ".." || ( newSource && !Portable( part ) ) )
			return std::nullopt;
		if ( end == std::string::npos )
			break;
		start = end + 1;
	}
	return result;
}

std::optional<AssetRef> AssetRef::Create( AssetKind kind, std::string_view name, bool newSource )
{
	const KindRule *rule = Rule( kind );
	auto normalized = NormalizeAssetName( name, newSource );
	if ( !rule || !normalized || !normalized->starts_with( rule->prefix ) )
		return std::nullopt;
	if ( kind == AssetKind::Material || kind == AssetKind::Texture )
	{
		if ( normalized->ends_with( ".vmt" ) || normalized->ends_with( ".vtf" ) )
			return std::nullopt;
	}
	else if ( !rule->extension.empty() && !normalized->ends_with( rule->extension ) )
		return std::nullopt;
	return AssetRef{ kind, std::move( *normalized ) };
}

std::optional<AssetRef> AssetRef::FromRuntimePath( std::string_view path )
{
	auto normalized = NormalizeAssetName( path );
	if ( !normalized )
		return std::nullopt;
	for ( const KindRule &rule : kKinds )
	{
		if ( !normalized->starts_with( rule.prefix ) ||
		     ( !rule.extension.empty() && !normalized->ends_with( rule.extension ) ) )
			continue;
		std::string_view name = *normalized;
		if ( rule.kind == AssetKind::Material || rule.kind == AssetKind::Texture )
			name.remove_suffix( rule.extension.size() );
		return Create( rule.kind, name );
	}
	return std::nullopt;
}

std::string AssetRef::RuntimePath() const
{
	const KindRule *rule = Rule( kind );
	return rule ? name + std::string( rule->extension ) : std::string{};
}

std::uint64_t AssetRef::NameHash() const noexcept
{
	Blake2b hash( 8 );
	const std::string_view kindName = AssetKindName( kind );
	hash.Update( kindName.data(), kindName.size() );
	const char separator = '\0';
	hash.Update( &separator, 1 );
	hash.Update( name.data(), name.size() );
	std::uint8_t bytes[8];
	hash.Final( bytes );
	std::uint64_t value = 0;
	for ( int i = 7; i >= 0; --i )
		value = ( value << 8 ) | bytes[i];
	return value;
}

bool AssetRef::operator<( const AssetRef &other ) const noexcept
{
	return kind == other.kind ? name < other.name : kind < other.kind;
}

} // namespace content
