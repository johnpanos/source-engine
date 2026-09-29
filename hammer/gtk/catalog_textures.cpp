//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/gtk/catalog_textures.h.
//
//=============================================================================//

#include "catalog_textures.h"

#ifdef HAMMER_KTX_PREVIEW
#include "hammer/adapters/source/ktx2_preview.h"
#endif

#include <cstddef>
#include <string_view>

namespace hammer::gtk
{

std::unique_ptr<CatalogTextures> CatalogTextures::Open(
    const std::string &vpkList, std::string &errors )
{
	std::unique_ptr<CatalogTextures> textures( new CatalogTextures() );
	std::size_t begin = 0;
	while ( begin <= vpkList.size() )
	{
		const std::size_t comma = vpkList.find( ',', begin );
		const std::string path =
		    vpkList.substr( begin, comma == std::string::npos ? std::string::npos : comma - begin );
		if ( !path.empty() )
		{
			std::string error;
			auto vpk = formats::VpkArchive::Open( textures->m_Store, path, error );
			if ( vpk )
			{
				textures->m_Assets.AddProvider( vpk.get() );
				textures->m_Archives.push_back( std::move( vpk ) );
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
	if ( textures->m_Archives.empty() )
	{
		return nullptr;
	}
#ifdef HAMMER_KTX_PREVIEW
	textures->m_Catalog = std::make_unique<formats::MaterialCatalog>(
	    textures->m_Assets, &adapters::source::DecodeKtx2Preview );
#else
	textures->m_Catalog = std::make_unique<formats::MaterialCatalog>( textures->m_Assets );
#endif
	return textures;
}

foundation::Expected<render_adapter::SourceMaterial, std::string> CatalogTextures::Material(
    const std::string &material )
{
	render_adapter::MaterialFiles files;
	files.read = [this]( std::string_view path ) -> std::optional<std::string>
	{
		std::string bytes;
		if ( !m_Assets.ReadAsset( std::string( path ), bytes ) )
			return std::nullopt;
		return bytes;
	};
	files.decode = [this](
	                   const std::string &texture ) -> std::optional<render_adapter::MaterialImage>
	{
		const formats::VtfImage *image = m_Catalog->TextureImage( texture );
		if ( !image || image->width <= 0 || image->height <= 0 ||
		     image->rgba.size() != std::size_t( image->width ) * std::size_t( image->height ) * 4 )
		{
			return std::nullopt;
		}
		render_adapter::MaterialImage out;
		out.width = static_cast<std::uint32_t>( image->width );
		out.height = static_cast<std::uint32_t>( image->height );
		out.rgba = image->rgba;
		return out;
	};
	return render_adapter::ImportSourceMaterial(
	    "materials/" + formats::CanonicalizeMaterialName( material ) + ".vmt", files );
}

} // namespace hammer::gtk
