//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Projector cookies (render.projected-light.v1) as the one RGBA 2D
//			array the surface program and render.pass.volumetric sample
//			(material::SurfaceProjectors, VolumetricTargets::cookies). The
//			one owner shared by the product's world stage (CoreWorld) and
//			render_lab, so both read the same texels.
//
//			A cookie is materials/<name>.vtf's top level, read by the shared
//			VTF container reader in its own format: 8-bit RGBA or BGRA, or
//			BC1 to BC5 blocks as shipped (Portal 2's flashlight cookies are
//			block-compressed), sampled through a linear (unorm) view: the
//			texels are linear, pixel / 255, as the flashlight shader reads
//			them. Layer i is name i; the array has at least two layers, the
//			rest white. Every cookie must have the first one's size and
//			format: a different one, a missing file or one that does not
//			decode is refused by name, never resampled or converted.
//
//			Decoding is CPU work (DecodeCookies, any thread); the array is
//			made and uploaded on the device's sequence (CookieArray).
//
//=============================================================================//

#ifndef RENDER_MAP_MEDIA_PROJECTOR_COOKIES_H
#define RENDER_MAP_MEDIA_PROJECTOR_COOKIES_H

#include "foundation/expected.h"
#include "mdl/studio_model.h"
#include "render/device/device.h"
#include "texturecontainer/vtf_container.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace render::map_media
{

struct CookieImages
{
	std::vector<std::string> names; // layer i is names[i]
	std::uint32_t width = 1;
	std::uint32_t height = 1;
	std::uint32_t layers = 2;
	std::uint64_t layerBytes = 4;
	device::Format format = device::Format::kRGBA8Unorm; // a linear view
	std::vector<std::byte> bytes; // layers x layerBytes
};

// The cookies' texels, or the reason one is refused. `decompressor` reads VTF
// 7.6 (P2:CE) compressed runs; the composition root passes it, and without
// one a compressed cookie is refused.
foundation::Expected<CookieImages, std::string> DecodeCookies( const mdl::IModelFiles &files,
    const std::vector<std::string> &names,
    texturecontainer::vtf::Decompressor decompressor = nullptr );

class CookieArray
{
public:
	CookieArray() = default;
	// Waits for the device to idle before releasing (a program's end);
	// a renderer that keeps drawing calls Release with its token instead.
	~CookieArray();
	CookieArray( const CookieArray & ) = delete;
	CookieArray &operator=( const CookieArray & ) = delete;

	// The reason when the array was refused.
	std::optional<std::string> Create( device::IRenderDevice2 &device, const CookieImages &images );
	// DecodeCookies, then Create.
	std::optional<std::string> Create( device::IRenderDevice2 &device,
	    const mdl::IModelFiles &files, const std::vector<std::string> &names,
	    texturecontainer::vtf::Decompressor decompressor = nullptr );
	// Copies the texels into the array, which ends in kSampled.
	void RecordUpload( device::CommandEncoder &encoder );
	// Releases the array and its staging behind `token`.
	void Release( device::CompletionToken token );
	device::TextureId Texture() const { return m_Texture; }
	device::TextureDesc Desc() const;

private:
	device::IRenderDevice2 *m_Device = nullptr;
	device::TextureId m_Texture;
	device::BufferId m_Staging;
	CookieImages m_Images;
};

} // namespace render::map_media

#endif // RENDER_MAP_MEDIA_PROJECTOR_COOKIES_H
