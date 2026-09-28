//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer::ports::IMaterialInfo over the material catalog (RFC 0002,
//			hammer.formats). Names and existence come from
//			MaterialCatalog::MaterialNames (materials/**.vmt); a name is
//			normalized with CanonicalizeMaterialName, so case, '/' versus '\', a
//			leading "materials/" and a ".vmt" suffix do not matter.
//
//			The mapping size is the width and height in the VTF header of the
//			material's $basetexture (MaterialCatalog::ResolveBaseTexture, which
//			follows one level of a `patch` include), read with ReadVtfInfo
//			without decoding pixels. A material that is missing, names no base
//			texture, or whose base texture has no readable VTF (for example a
//			KTX2-only package; no header-level KTX2 reader exists in the strict
//			core yet) has no size, never a default one.
//
//			Borrows the catalog and the asset source, which must outlive the
//			adapter. Sizes are cached per material. Not internally synchronized:
//			the catalog caches on first use too, so use it from one thread.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_MATERIAL_INFO_ADAPTER_H
#define HAMMER_FORMATS_MATERIAL_INFO_ADAPTER_H

#include "hammer/formats/material_catalog.h"
#include "hammer/ports/asset_source.h"
#include "hammer/ports/material_info.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::formats
{

class MaterialInfoAdapter final : public hammer::ports::IMaterialInfo
{
public:
	// 'source' is the asset source 'catalog' reads from.
	MaterialInfoAdapter( MaterialCatalog &catalog, const hammer::ports::IAssetSource &source );

	bool Exists( std::string_view material ) const override;
	std::optional<hammer::ports::MaterialSize> Size( std::string_view material ) const override;
	std::vector<std::string> Names() const override;

private:
	MaterialCatalog &m_catalog;
	const hammer::ports::IAssetSource &m_source;
	mutable std::map<std::string, std::optional<hammer::ports::MaterialSize>> m_sizes;
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_MATERIAL_INFO_ADAPTER_H
