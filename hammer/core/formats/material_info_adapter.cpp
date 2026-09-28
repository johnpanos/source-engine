//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the material-info adapter. See
//			public/hammer/formats/material_info_adapter.h. Name rules belong to
//			CanonicalizeMaterialName, base-texture resolution to the catalog and
//			VTF header parsing to vtf_image.h; this file only composes them.
//
//=============================================================================//

#include "hammer/formats/material_info_adapter.h"
#include "hammer/formats/vtf_image.h"

#include <algorithm>

namespace hammer::formats
{

MaterialInfoAdapter::MaterialInfoAdapter(
    MaterialCatalog &catalog, const hammer::ports::IAssetSource &source )
    : m_catalog( catalog ), m_source( source )
{
}

bool MaterialInfoAdapter::Exists( std::string_view material ) const
{
	const std::string canonical = CanonicalizeMaterialName( std::string( material ) );
	const std::vector<std::string> &names = m_catalog.MaterialNames();
	return !canonical.empty() && std::binary_search( names.begin(), names.end(), canonical );
}

std::optional<hammer::ports::MaterialSize> MaterialInfoAdapter::Size(
    std::string_view material ) const
{
	const std::string canonical = CanonicalizeMaterialName( std::string( material ) );
	const auto cached = m_sizes.find( canonical );
	if ( cached != m_sizes.end() )
	{
		return cached->second;
	}

	std::optional<hammer::ports::MaterialSize> size;
	if ( Exists( canonical ) )
	{
		const std::string base = m_catalog.ResolveBaseTexture( canonical );
		std::string bytes;
		if ( !base.empty() && m_source.ReadAsset( "materials/" + base + ".vtf", bytes ) )
		{
			std::string error;
			if ( const std::optional<VtfInfo> info = ReadVtfInfo( bytes, error ) )
			{
				if ( info->width > 0 && info->height > 0 )
				{
					size = hammer::ports::MaterialSize{ info->width, info->height };
				}
			}
		}
	}
	m_sizes.emplace( canonical, size );
	return size;
}

std::vector<std::string> MaterialInfoAdapter::Names() const
{
	return m_catalog.MaterialNames();
}

} // namespace hammer::formats
