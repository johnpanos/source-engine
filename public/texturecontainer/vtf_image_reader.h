//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VTF adapter for the owned texture image contract.
//
//=============================================================================//

#ifndef TEXTURECONTAINER_VTF_IMAGE_READER_H
#define TEXTURECONTAINER_VTF_IMAGE_READER_H

#include "texturecontainer/texture_image.h"

namespace texturecontainer
{

// Reads the 2D, single-frame, single-face VTF cohort used by ordinary material
// base textures. Existing VTF animation, cubemap and volume callers stay on the
// legacy VTF path until the shared image contract represents those topologies.
[[nodiscard]] foundation::Expected<TextureImage, ReadError> ReadVtfImage(
    std::span<const std::byte> encoded );

} // namespace texturecontainer

#endif // TEXTURECONTAINER_VTF_IMAGE_READER_H
