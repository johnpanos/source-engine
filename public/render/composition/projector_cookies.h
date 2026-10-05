//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Projector cookies (render.projected-light.v1) as the one RGBA 2D
//			array the surface program and render.pass.volumetric sample
//			(material::SurfaceProjectors, VolumetricTargets::cookies). The
//			one owner shared by the product's world stage (CoreWorld) and
//			render_lab, so both read the same texels.
//
//			A cookie is materials/<name>.vtf, decoded by the shared VTF
//			container reader to 8-bit RGBA; its texels are linear (pixel /
//			255), as the flashlight shader reads them. Layer i is name i;
//			the array has at least two layers, the rest white. Every cookie
//			must have the first one's size: a different size, a missing file
//			or one that does not decode is refused by name, never resampled.
//
//			Decoding is CPU work (DecodeCookies, any thread); the array is
//			made and uploaded on the device's sequence (CookieArray).
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_PROJECTOR_COOKIES_H
#define RENDER_COMPOSITION_PROJECTOR_COOKIES_H

#include "foundation/expected.h"
#include "mdl/studio_model.h"
#include "render/device/device.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace render::composition
{

struct CookieImages
{
	std::vector<std::string> names; // layer i is names[i]
	std::uint32_t width = 1;
	std::uint32_t height = 1;
	std::uint32_t layers = 2;
	std::uint64_t layerBytes = 4;
	std::vector<std::byte> bytes; // layers x layerBytes, RGBA8
};

// The cookies' texels, or the reason one is refused.
foundation::Expected<CookieImages, std::string> DecodeCookies(
    const mdl::IModelFiles &files, const std::vector<std::string> &names );

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
	    const mdl::IModelFiles &files, const std::vector<std::string> &names );
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

} // namespace render::composition

#endif // RENDER_COMPOSITION_PROJECTOR_COOKIES_H
