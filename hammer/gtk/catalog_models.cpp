//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/gtk/catalog_models.h.
//
//=============================================================================//

#include "catalog_models.h"

#include <algorithm>
#include <filesystem>
#include <system_error>

namespace hammer::gtk
{

namespace
{

std::vector<std::string> SplitList( const std::string &list )
{
	std::vector<std::string> out;
	std::size_t begin = 0;
	while ( begin <= list.size() )
	{
		const std::size_t comma = list.find( ',', begin );
		std::string item =
		    list.substr( begin, comma == std::string::npos ? std::string::npos : comma - begin );
		if ( !item.empty() )
		{
			out.push_back( std::move( item ) );
		}
		if ( comma == std::string::npos )
		{
			break;
		}
		begin = comma + 1;
	}
	return out;
}

} // namespace

std::vector<std::string> GameInstanceRoots( const std::string &vpkList, const std::string &extra )
{
	std::vector<std::string> roots;
	const auto add = [&]( const std::string &root )
	{
		if ( std::find( roots.begin(), roots.end(), root ) == roots.end() )
		{
			roots.push_back( root );
		}
	};
	for ( const std::string &vpk : SplitList( vpkList ) )
	{
		const std::filesystem::path maps =
		    std::filesystem::path( vpk ).parent_path().parent_path() / "sdk_content" / "maps";
		std::error_code error;
		if ( std::filesystem::is_directory( maps, error ) )
		{
			add( maps.string() );
		}
	}
	for ( const std::string &root : SplitList( extra ) )
	{
		add( root );
	}
	return roots;
}

std::unique_ptr<CatalogModels> CatalogModels::Open(
    const std::string &vpkList, std::string &errors )
{
	std::unique_ptr<CatalogModels> models( new CatalogModels() );
	std::size_t begin = 0;
	while ( begin <= vpkList.size() )
	{
		const std::size_t comma = vpkList.find( ',', begin );
		const std::string path =
		    vpkList.substr( begin, comma == std::string::npos ? std::string::npos : comma - begin );
		if ( !path.empty() )
		{
			std::string error;
			auto vpk = formats::VpkArchive::Open( models->m_Store, path, error );
			if ( vpk )
			{
				models->m_Assets.AddProvider( vpk.get() );
				models->m_Archives.push_back( std::move( vpk ) );
			}
			else
			{
				errors += ( errors.empty() ? "" : "; " ) + path + ": " + error;
			}
		}
		if ( comma == std::string::npos )
		{
			break;
		}
		begin = comma + 1;
	}
	if ( models->m_Archives.empty() )
	{
		return nullptr;
	}
	return models;
}

foundation::Expected<render_adapter::ModelAsset, mdl::ModelError> CatalogModels::Model(
    const std::string &path )
{
	auto loaded = mdl::LoadModel( m_Files, path );
	if ( !loaded )
	{
		return foundation::MakeUnexpected( loaded.Error() );
	}
	render_adapter::ModelAsset asset;
	asset.materials = mdl::ResolveMaterials( loaded.Value(), m_Files );
	asset.model = std::move( loaded ).Value();
	return asset;
}

} // namespace hammer::gtk
