//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Decoding of BC-compressed images for devices without the BC
//			formats (progressive enhancement: Adreno 730 and most mobile GPUs
//			sample BC1-BC7 only after this).
//
//=============================================================================//

#ifndef TEXTURECONTAINER_BLOCK_DECODE_H
#define TEXTURECONTAINER_BLOCK_DECODE_H

#include "texturecontainer/texture_image.h"

#include <optional>

namespace texturecontainer
{

// The uncompressed format a BC format decodes to: BC1-BC3 and BC7 to RGBA8
// (sRGB kept), BC4 to R8, BC5 to RG8, BC6H to RGBA16F (alpha 1). None for a
// format that is not BC.
std::optional<PixelFormat> DecodedBlockFormat( PixelFormat format ) noexcept;

// The image with every level decoded to DecodedBlockFormat. None when the
// format is not BC or a level's bytes do not hold its blocks.
std::optional<TextureImage> DecodeBlockImage( const TextureImage &image );

} // namespace texturecontainer

#endif // TEXTURECONTAINER_BLOCK_DECODE_H
