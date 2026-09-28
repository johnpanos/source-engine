//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The textured preview's source of material images (RFC 0002
//			hammer.adapters.render; RFC 0016 K4). ViewportRenderer asks it for
//			a material's base texture the first time the scene names the
//			material, on the render sequence only, and keeps the answer (a
//			miss included) for its lifetime. The composition root implements it
//			over the game's assets (hammer::formats::MaterialCatalog on its
//			own asset source), so nothing here is shared with the host's
//			sequence.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_RENDER_MATERIAL_TEXTURES_H
#define HAMMER_ADAPTERS_RENDER_MATERIAL_TEXTURES_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hammer::render_adapter
{

struct MaterialImage
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> rgba; // width * height, row 0 at the top, sRGB-encoded RGBA8
};

class IMaterialTextures
{
public:
	virtual ~IMaterialTextures() = default;
	// The base texture of 'material' (as the VMF names it), or nothing when the
	// material, its base texture or its decode is missing.
	virtual std::optional<MaterialImage> BaseTexture( const std::string &material ) = 0;
};

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_MATERIAL_TEXTURES_H
