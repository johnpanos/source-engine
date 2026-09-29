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

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hammer::render_adapter
{

// How a material's surface covers what is behind it, from its VMT
// parameters. The renderer claims it through the unlit family (ClaimUnlit
// owns the blend and depth-write rules).
struct MaterialSurface
{
	bool translucent = false;        // $translucent: blended, no depth write
	bool additive = false;           // $additive: added to what is behind
	bool alphaTest = false;          // $alphatest: texels under the reference are cut
	float alphaTestReference = 0.0f; // $alphatestreference; 0 when not authored
	float alpha = 1.0f;              // $alpha

	friend bool operator==( const MaterialSurface &, const MaterialSurface & ) = default;
};

struct MaterialImage
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> rgba; // width * height, row 0 at the top, sRGB-encoded RGBA8
	MaterialSurface surface;
};

class IMaterialTextures
{
public:
	virtual ~IMaterialTextures() = default;
	// The base texture of 'material' (as the VMF names it) with its surface
	// parameters, or nothing when the material, its base texture or its
	// decode is missing.
	virtual std::optional<MaterialImage> BaseTexture( const std::string &material ) = 0;
};

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
