//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless VMT (Valve Material Type) definition parser for the Hammer
//			editor (RFC 0002, hammer.formats). A .vmt names a shader and its
//			parameters ($basetexture, $surfaceprop, ...) plus optional proxies; it
//			is the material *definition* the editor and renderer consume to know a
//			material's base texture, surface property, and flags. This strict,
//			MFC-free, GPU-free core parses the definition (it does not load VTF
//			pixels or render anything). VMT is keyvalues syntax, so this builds on
//			the shared keyvalues codec and adds material semantics on top:
//			case-insensitive parameter lookup and the common accessors.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_MATERIAL_H
#define HAMMER_FORMATS_MATERIAL_H

#include "hammer/formats/keyvalues.h"

#include <optional>
#include <string>
#include <vector>

namespace hammer::formats
{

// A parsed VMT material definition.
struct Material
{
	std::string shader;                 // the top-level shader block name (e.g. LightmappedGeneric)
	std::vector<KeyValue> parameters;   // top-level $-parameters
	bool hasProxies = false;            // a "Proxies" sub-block is present
	std::vector<KeyValue> patchReplace; // a patch's "replace" block params (else empty)
	std::vector<KeyValue> patchInsert;  // a patch's "insert" block params (else empty)

	// Case-insensitive lookup of a TOP-LEVEL parameter (VMT names ignore case).
	const std::string *Param( const std::string &name ) const;
	bool HasParam( const std::string &name ) const;

	// Case-insensitive lookup that, for a patch material, resolves in the order
	// replace -> insert -> top-level, so a patched value wins. For a non-patch
	// material this is identical to Param.
	const std::string *ResolvedParam( const std::string &name ) const;

	// Common accessors (empty string when the parameter is absent). These resolve
	// through ResolvedParam, so a patch's replaced $basetexture is returned.
	std::string BaseTexture() const;
	std::string SurfaceProp() const;

	// True for a "patch" material (shader == "patch", case-insensitive), which
	// includes and overrides another VMT.
	bool IsPatch() const;
};

// Parses VMT text. Returns nullopt when the text has no top-level shader block or
// fails keyvalues parsing.
std::optional<Material> ParseMaterial( const std::string &vmtText );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_MATERIAL_H
