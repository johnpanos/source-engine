//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The textured preview's source of materials (RFC 0002
//			hammer.adapters.render; RFC 0016 K5 ProgramResolver::ResolvePreview).
//			ViewportRenderer asks it for a material the first time the scene
//			names the material, on the render sequence only, and keeps the
//			answer (a miss included) for its lifetime. The answer is the
//			material's VMT as render::material::ImportVmt imports it (the one
//			reader of VMTs: patches, includes, conditions and fallback blocks)
//			and the decoded images of the textures it names, keyed by the
//			importer's normalized names ("materials/a/b"), which are the names
//			the resolved program samples. The composition root implements it
//			over the game's assets (ImportSourceMaterial over its own asset
//			source, with hammer::formats::MaterialCatalog decoding), so nothing
//			here is shared with the host's sequence.
//
//			BuildMipChain is the renderer's mip generator, a pure function so
//			its filter is testable without a device: each level is a 2x2 box
//			of the one above in linear light (the sRGB-encoded texels decoded,
//			averaged, encoded again), because the preview samples the texture
//			as sRGB; averaging the encoded bytes would darken every mixed
//			texel. Alpha is stored linearly and averaged as stored.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_RENDER_MATERIAL_TEXTURES_H
#define HAMMER_ADAPTERS_RENDER_MATERIAL_TEXTURES_H

#include "foundation/expected.h"
#include "render/material/vmt_import.h"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::render_adapter
{

struct MaterialImage
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> rgba; // width * height, row 0 at the top, sRGB-encoded RGBA8
};

// A material as the preview draws it: its imported description and the
// decoded images of the textures it names, by the importer's normalized
// names (render::material::VmtTextureReference). A texture that is missing
// or does not decode is absent.
struct SourceMaterial
{
	::render::material::MaterialDesc desc;
	std::map<std::string, MaterialImage> textures;
};

class IMaterialTextures
{
public:
	virtual ~IMaterialTextures() = default;
	// 'material' as the VMF names it, or why the source has none (missing,
	// or the VMT does not import).
	virtual foundation::Expected<SourceMaterial, std::string> Material(
	    const std::string &material ) = 0;
};

// The files a source reads materials from.
struct MaterialFiles
{
	// A file of the game's search path ("materials/a/b.vmt", lower case), for
	// the material and its patch includes.
	::render::material::VmtResolver read;
	// A texture by the importer's normalized name ("materials/a/b"), or
	// nothing when it is missing or does not decode.
	std::function<std::optional<MaterialImage>( const std::string &texture )> decode;
};

// Imports the VMT at 'vmtPath' ("materials/a/b.vmt", as the source's own
// name rule makes it) with render::material::ImportVmt (default profile,
// 'files.read' resolving includes) and decodes the textures the preview
// samples: the base texture ($basetexture). Fails with the reason when the
// file is missing or does not import.
foundation::Expected<SourceMaterial, std::string> ImportSourceMaterial(
    std::string_view vmtPath, const MaterialFiles &files );

// A material from a shader and its variables as a VMT sets them
// (render::material::MapVariables), for sources without VMT text (generated
// materials, tests), with its textures by normalized name.
foundation::Expected<SourceMaterial, std::string> SourceMaterialFromVariables(
    std::string_view shader, std::vector<::render::material::VmtPair> variables,
    std::map<std::string, MaterialImage> textures = {} );

struct MipLevel
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> rgba; // as MaterialImage::rgba
};

// The image's full mip chain, level 0 (the image itself) down to 1x1:
// floor(log2(max(width, height))) + 1 levels, level m max(1, size >> m) on
// each axis. Texel (x, y) of level m + 1 is the box of texels (2x, 2y) to
// (2x + 1, 2y + 1) of level m, equally weighted: 2x2, except that the last
// box of an axis with an odd size also takes the odd row or column (3 wide)
// and an axis of one texel stays one wide. Color is averaged in linear light
// (IEC 61966-2-1 sRGB decode and encode, rounded to the nearest byte; the
// sums are kept in 1/65535 steps, which changes no byte away from 1e-5 of a
// byte boundary), alpha as stored, rounded. Empty when the image is empty or
// its bytes do not match its size.
std::vector<MipLevel> BuildMipChain( const MaterialImage &image );

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_MATERIAL_TEXTURES_H
