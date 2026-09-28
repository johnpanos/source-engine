//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: In-memory hammer::ports::IMaterialInfo for suites of modules that
//			consume the port (texture alignment, the material browser, the map
//			check). A map from normalized name (lower case, '\' -> '/') to an
//			optional mapping size.
//
//			Usage:
//				hammertest::FakeMaterialInfo materials;
//				materials.Add( "dev/dev_measuregeneric01", 128, 128 ).AddUnsized( "tools/x" );
//
//=============================================================================//

#ifndef HAMMERTEST_FAKE_MATERIAL_INFO_H
#define HAMMERTEST_FAKE_MATERIAL_INFO_H

#include "hammer/ports/material_info.h"

#include <cctype>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammertest
{

class FakeMaterialInfo final : public hammer::ports::IMaterialInfo
{
public:
	// A material with a known mapping size.
	FakeMaterialInfo &Add( std::string_view name, int width, int height )
	{
		m_materials[Normalize( name )] = hammer::ports::MaterialSize{ width, height };
		return *this;
	}

	// A material that exists but whose size is unknown.
	FakeMaterialInfo &AddUnsized( std::string_view name )
	{
		m_materials[Normalize( name )] = std::nullopt;
		return *this;
	}

	bool Exists( std::string_view material ) const override
	{
		return m_materials.count( Normalize( material ) ) != 0;
	}

	std::optional<hammer::ports::MaterialSize> Size( std::string_view material ) const override
	{
		const auto it = m_materials.find( Normalize( material ) );
		return it == m_materials.end() ? std::nullopt : it->second;
	}

	std::vector<std::string> Names() const override
	{
		std::vector<std::string> names;
		for ( const auto &entry : m_materials )
		{
			names.push_back( entry.first );
		}
		return names;
	}

private:
	static std::string Normalize( std::string_view name )
	{
		std::string out( name );
		for ( char &c : out )
		{
			c = c == '\\' ? '/'
			              : static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
		}
		return out;
	}

	std::map<std::string, std::optional<hammer::ports::MaterialSize>> m_materials;
};

} // namespace hammertest

#endif // HAMMERTEST_FAKE_MATERIAL_INFO_H
