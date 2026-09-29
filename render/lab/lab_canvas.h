//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's canvas (RFC 0016 K11): a linear RGBA16F target with
//			a D32 depth target, drawn by a list of draws on one encoder and
//			read back as linear floats. The suites' fixtures draw through it;
//			texture and group uploads of the caller's caches are recorded
//			first, in the same submission. Private to render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_CANVAS_H
#define RENDER_LAB_LAB_CANVAS_H

#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/resources/texture_cache.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace render::lab
{

inline constexpr device::Format kCanvasColor = device::Format::kRGBA16Float;
inline constexpr device::Format kCanvasDepth = device::Format::kD32Float;

struct CanvasDraw
{
	device::PipelineId pipeline;
	std::array<device::BindGroupId, 4> groups; // by BindGroupRole; invalid ones stay unbound
	device::BufferId vertices;
	std::uint32_t vertexCount = 0;
	std::vector<std::byte> constants; // the draw constants the pipeline reads
};

// Work recorded after the draws, before the read back: the color target is
// in kColorAttachment and the depth target in kDepthWrite, and the work leaves
// them so (a pass that composites over the frame, render.pass.volumetric).
using CanvasPost = std::function<std::optional<std::string>(
    device::CommandEncoder &encoder, device::TextureId color, device::TextureId depth )>;

// A linear image read back from a canvas: RGBA per texel, row 0 at the top.
struct CanvasImage
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<float> rgba;

	const float *At( std::uint32_t x, std::uint32_t y ) const
	{
		return &rgba[( std::size_t( y ) * width + x ) * 4];
	}
};

class Canvas
{
public:
	static std::optional<std::string> Create( device::IRenderDevice2 &device, std::uint32_t width,
	    std::uint32_t height, std::unique_ptr<Canvas> &out );
	~Canvas();
	Canvas( const Canvas & ) = delete;
	Canvas &operator=( const Canvas & ) = delete;

	std::uint32_t Width() const { return m_Width; }
	std::uint32_t Height() const { return m_Height; }

	// A vertex buffer holding `bytes`, written at the next Render.
	device::BufferId Vertices( std::span<const std::byte> bytes );

	// The color target (kCanvasColor, left in kCopySource after a Render),
	// for a caller that consumes the frame on the device (an output pass).
	device::TextureId ColorTarget() const { return m_Color; }
	const device::TextureDesc &ColorDesc() const { return m_ColorDesc; }

	// Records the caches' pending uploads and the pending vertex writes,
	// clears the target to `clear`, draws the draws in order, records `post`
	// and waits; with `out`, reads the target back into it. The reason when
	// it cannot.
	std::optional<std::string> Render( resources::TextureCache &textures,
	    material::GroupResidency &groups, std::span<const CanvasDraw> draws,
	    const device::ClearColor &clear, CanvasImage *out, const CanvasPost &post = {} );

private:
	explicit Canvas( device::IRenderDevice2 &device ) : m_Device( device ) {}

	struct PendingWrite
	{
		device::BufferId buffer;
		std::vector<std::byte> bytes;
	};

	device::IRenderDevice2 &m_Device;
	std::uint32_t m_Width = 0;
	std::uint32_t m_Height = 0;
	device::TextureId m_Color;
	device::TextureDesc m_ColorDesc;
	device::TextureId m_Depth;
	device::BufferId m_Readback;
	std::vector<device::BufferId> m_Buffers;
	std::vector<PendingWrite> m_Writes;
};

} // namespace render::lab

#endif // RENDER_LAB_LAB_CANVAS_H
