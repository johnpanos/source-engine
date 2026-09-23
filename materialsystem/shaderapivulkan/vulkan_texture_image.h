//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Upload an owned texture-container image through native Vulkan.
//
//=============================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_TEXTURE_IMAGE_H
#define SHADERAPIVULKAN_VULKAN_TEXTURE_IMAGE_H

#include "texturecontainer/texture_image.h"

#include <string>

namespace render_vulkan
{

class CVulkanContext;

enum class TextureImageUploadError
{
	InvalidImage,
	UnsupportedFormat,
	CreateFailed,
	UploadFailed,
};

struct TextureImageUploadFailure
{
	TextureImageUploadError code;
	std::string detail;
};

// Validates every level before creating a GPU image. A required format that
// the selected device cannot sample/upload fails; no silent format fallback.
// If an upload fails after creation, the managed image is destroyed and all
// pending uploads for its handle are cancelled before returning an error.
[[nodiscard]] foundation::Expected<int, TextureImageUploadFailure> CreateManagedTextureImage(
    CVulkanContext &context, const texturecontainer::TextureImage &image );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_TEXTURE_IMAGE_H
