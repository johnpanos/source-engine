//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VMT material parsing and a material catalog for the editor (RFC 0002,
//			hammer.formats). A Source .vmt is a keyvalues document naming a shader
//			and its parameters; the parameter that matters for a 2D editor preview
//			is $basetexture, which names a .vtf under materials/. This module:
//
//			 - ParseMaterial: reads a .vmt's shader name and $basetexture using the
//			   strict hammer::formats keyvalues parser, case-insensitively (VMT keys
//			   are case-insensitive), including the one-level "patch" shader form
//			   (a patch names an include and may override $basetexture in an
//			   insert/replace block).
//			 - MaterialCatalog: enumerates every materials/**.vmt in an IAssetSource,
//			   resolves a VMF-authored material name (any case, either slash) to its
//			   shader/$basetexture, and decodes that base texture to an RGBA image,
//			   caching both. This is the clean-core replacement for the legacy
//			   Hammer path, which drove everything through the engine IMaterial /
//			   IMaterialSystem globals.
//
//			Strict-core: C++ standard library, the keyvalues codec, the VTF decoder,
//			and the IAssetSource port only. No tier0, no engine material system.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_MATERIAL_H
#define HAMMER_FORMATS_MATERIAL_H

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

// The editor-relevant facts about a material. 'name' is the canonical catalog name
// (lower-case, forward slash, no "materials/" prefix, no ".vmt"). 'baseTexture' is
// the canonical .vtf name (same convention, no ".vtf"), empty when the material
// declares none.
struct MaterialInfo
{
	std::string name;
	std::string shader;      // lower-cased shader/block name (e.g. "lightmappedgeneric")
	std::string baseTexture; // canonical, no extension; empty when absent
};

// Parses a single .vmt's text. Fills 'out' (shader + baseTexture) and, when the
// shader is "patch", 'includeOut' with the referenced material path (canonical,
// no extension, no "materials/" prefix). 'out.name' is left empty (the caller
// knows the name). Returns false with 'error' set when the text is not parseable
// keyvalues or has no top block.
bool ParseMaterial( const std::string &vmtText, MaterialInfo &out, std::string &includeOut, std::string &error );

// Normalizes a VMF-authored material name (mixed case, back or forward slashes, an
// optional leading slash or ".vmt"/".vtf" suffix) to the catalog's canonical form.
std::string CanonicalizeMaterialName( const std::string &name );

class MaterialCatalog
{
public:
	// Reads through 'source' (which must outlive the catalog). Nothing is loaded
	// until queried.
	explicit MaterialCatalog( const hammer::ports::IAssetSource &source );

	// Every material name found under materials/*.vmt, sorted and de-duplicated.
	// Built lazily on first call and cached.
	const std::vector<std::string> &MaterialNames();

	// Resolves a material (any authored spelling) to its info, following one level
	// of "patch" include. Returns nullptr when the .vmt is absent or unparseable.
	// The returned pointer is stable for the catalog's lifetime.
	const MaterialInfo *Material( const std::string &name );

	// Decodes the material's $basetexture to RGBA, cached. Returns nullptr when the
	// material or its base texture is missing or cannot be decoded. The pointer is
	// stable for the catalog's lifetime.
	const VtfImage *BaseTextureImage( const std::string &name );

private:
	const hammer::ports::IAssetSource &m_source;
	std::optional<std::vector<std::string>> m_names;
	std::map<std::string, MaterialInfo> m_materials; // canonical name -> info (hits)
	std::set<std::string> m_materialMisses;
	std::map<std::string, std::unique_ptr<VtfImage>> m_images; // canonical name -> image (null = miss)
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_MATERIAL_H
