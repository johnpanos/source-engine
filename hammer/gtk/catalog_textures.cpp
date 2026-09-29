//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/gtk/catalog_textures.h.
//
//=============================================================================//

#include "catalog_textures.h"

#ifdef HAMMER_KTX_PREVIEW
#include "hammer/adapters/source/ktx2_preview.h"
#endif

#include <cstdlib>

namespace hammer::gtk
{

namespace
{

// A VMT number as the material system reads it (leading number, else the
// default).
float Number( const std::optional<std::string> &value, float fallback )
{
	if ( !value )
	{
		return fallback;
	}
	const char *begin = value->c_str();
	char *end = nullptr;
	const double parsed = std::strtod( begin, &end );
	return end == begin ? fallback : float( parsed );
}

} // namespace

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

std::optional<render_adapter::MaterialImage> CatalogTextures::BaseTexture(
    const std::string &material )
{
	const formats::VtfImage *image = m_Catalog->BaseTextureImage( material );
	if ( !image || image->width <= 0 || image->height <= 0 ||
	     image->rgba.size() != std::size_t( image->width ) * std::size_t( image->height ) * 4 )
	{
		return std::nullopt;
	}
	render_adapter::MaterialImage out;
	out.width = static_cast<std::uint32_t>( image->width );
	out.height = static_cast<std::uint32_t>( image->height );
	out.rgba = image->rgba;
	render_adapter::MaterialSurface &surface = out.surface;
	surface.translucent = Number( m_Catalog->ResolveParameter( material, "$translucent" ), 0 ) != 0;
	surface.additive = Number( m_Catalog->ResolveParameter( material, "$additive" ), 0 ) != 0;
	surface.alphaTest = Number( m_Catalog->ResolveParameter( material, "$alphatest" ), 0 ) != 0;
	surface.alphaTestReference =
	    Number( m_Catalog->ResolveParameter( material, "$alphatestreference" ), 0 );
	surface.alpha = Number( m_Catalog->ResolveParameter( material, "$alpha" ), 1 );
	return out;
}

} // namespace hammer::gtk
