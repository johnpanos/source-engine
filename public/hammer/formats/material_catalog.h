//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Material catalog for the editor (RFC 0002, hammer.formats). Builds on
//			the VMT-definition parser (hammer::formats::ParseMaterial / Material)
//			and the VTF decoder to answer the two questions an editor asks of game
//			assets: "what materials exist?" and "give me this material's base
//			texture as an image". It enumerates materials/**.vmt in an IAssetSource,
//			resolves a VMF-authored material name (any case, either slash) to its
//			$basetexture -- following one level of the `patch` shader's `include`,
//			which the material parser deliberately leaves unresolved -- and decodes
//			that texture to RGBA, caching both. This is the clean-core replacement
//			for the legacy Hammer path that drove everything through the engine
//			IMaterial / IMaterialSystem globals.
//
//			Ownership is layered and DRY: VMT syntax + parameters are owned by
//			hammer::formats::Material (material.h); VTF bytes -> pixels by
//			VtfImage (vtf_image.h); asset lookup by the IAssetSource port. This
//			catalog only composes them and caches results. Strict-core: C++ stdlib,
//			those three, and the keyvalues codec they use.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_MATERIAL_CATALOG_H
#define HAMMER_FORMATS_MATERIAL_CATALOG_H

#include "hammer/formats/material.h"
#include "hammer/formats/vtf_image.h"
#include "hammer/ports/asset_source.h"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace hammer::formats
{

// Normalizes a VMF-authored material or texture name (mixed case, back or forward
// slashes, an optional leading slash, an optional "materials/" prefix, and an
// optional ".vmt"/".vtf" suffix) to the catalog's canonical form: lower-case,
// forward slashes, no prefix, no extension.
std::string CanonicalizeMaterialName( const std::string &name );

class MaterialCatalog
{
public:
	// Reads through 'source' (which must outlive the catalog). Nothing is loaded
	// until queried.
	explicit MaterialCatalog( const hammer::ports::IAssetSource &source );

	// Every material name found under materials/*.vmt, canonical, sorted, and
	// de-duplicated. Built lazily on first call and cached.
	const std::vector<std::string> &MaterialNames();

	// The canonical $basetexture name a material resolves to (following one level
	// of `patch` include), or empty when the material is absent/unparseable or
	// names no base texture. Cached.
	std::string ResolveBaseTexture( const std::string &name );

	// Decodes a material's base texture to RGBA, cached. Returns nullptr when the
	// material, its base texture, or the decode is missing/invalid; the result
	// (hit or miss) is cached and the pointer is stable for the catalog's lifetime.
	const VtfImage *BaseTextureImage( const std::string &name );

private:
	const hammer::ports::IAssetSource &m_source;
	std::optional<std::vector<std::string>> m_names;
	std::map<std::string, std::string> m_baseTextures; // canonical material -> canonical vtf name
	std::set<std::string> m_baseTextureMisses;
	std::map<std::string, std::unique_ptr<VtfImage>>
	    m_images; // canonical material -> image (null = miss)
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_MATERIAL_CATALOG_H
