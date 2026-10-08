//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.visibility (RFC 0016 layer 6 feature, K8 sprites
//			cohort): the client's pixel visibility (c_pixel_visibility.cpp:
//			glow sprites, lens flares, light coronas) on the core. A query is
//			the client's proxy, a camera-facing pyramid of five clip-space
//			points (the apex and four base corners, as PixelVisibility_DrawProxy
//			builds it), drawn twice at its slot into the frame's target with
//			no color or depth writes, each under an occlusion query (device
//			clause D43): once depth-tested against what the stream drew there
//			(LessEqual, the legacy occlusionproxy material) and once untested
//			(occlusionproxy_countdraw). The two counts are the samples visible
//			and possible, so their ratio is the legacy fraction.
//
//			A query is queued on the main thread in frame order and read on
//			the main thread once the frame that recorded it completes on the
//			GPU (pending until then, as an occlusion query in flight is).
//
//			Threads: Queue and Result on the main thread; Record,
//			FrameSubmitted and ReleaseDevice on the render sequence.
//
//=============================================================================//

#ifndef RENDER_PASS_VISIBILITY_VISIBILITY_H
#define RENDER_PASS_VISIBILITY_VISIBILITY_H

#include "render/device/device.h"
#include "render/device/encoder.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace render::pass::visibility
{

struct Query
{
	// The apex then the four base corners, in clip space with D3D9 pixel
	// centers (the pass shifts them half a pixel, as every view is shifted).
	float points[5][4] = {};
	device::Viewport viewport; // in the target's pixels
};

struct Counts
{
	std::int64_t visible = 0;  // samples passing the depth test
	std::int64_t possible = 0; // samples the proxy covers
};

struct VisibilityTarget
{
	device::IRenderDevice2 *device = nullptr;
	// The frame's target at the slot: color (written by nothing here) in
	// kColorAttachment and depth in kDepthWrite, in their home usages.
	device::TextureId color;
	device::Format colorFormat = device::Format::kUnknown;
	device::TextureId depth;
	device::Format depthFormat = device::Format::kUnknown;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t samples = 1;
	device::CompletionToken submitted;
	std::uint64_t frame = 0;
};

struct VisibilityStats
{
	std::uint64_t queued = 0;
	std::uint64_t refused = 0;
	std::uint64_t recorded = 0;
	std::uint64_t resolved = 0;
	std::uint64_t failed = 0;
	std::string lastFailure;
};

// A visibility tag: the forwarded bit with high byte 0x8f, then the query's
// serial in the low 24 bits (clear of the world's, the panels' 0x90,
// temporal's 0x88, post's 0x8c and luminance's 0x8e tags, and bits 30, 29).
inline constexpr std::uint32_t kVisibilityTag = 0x8f000000u;
inline constexpr std::uint32_t kVisibilitySerialMask = 0x00ffffffu;
inline bool IsVisibilityTag( std::uint32_t tag )
{
	return ( tag & ~kVisibilitySerialMask ) == kVisibilityTag;
}

class VisibilityCounter
{
public:
	VisibilityCounter();
	~VisibilityCounter();
	VisibilityCounter( const VisibilityCounter & ) = delete;
	VisibilityCounter &operator=( const VisibilityCounter & ) = delete;

	// Main thread: the tag of the slot that draws the proxy; 0 when malformed.
	std::uint32_t Queue( const Query &query );
	// Main thread: the counts once read back (returned once); nullopt while
	// pending; counts of -1 when the query failed.
	std::optional<Counts> Result( std::uint32_t tag );
	VisibilityStats Stats() const;

	void Record(
	    std::uint32_t tag, device::CommandEncoder &encoder, const VisibilityTarget &target );
	void FrameSubmitted( device::IRenderDevice2 &device, device::CompletionToken token );
	void ReleaseDevice( device::IRenderDevice2 &device );

private:
	struct State;
	std::unique_ptr<State> m_State;
};

} // namespace render::pass::visibility

#endif // RENDER_PASS_VISIBILITY_VISIBILITY_H
