//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's frame output (RFC 0016 "Output",
//			render.output.v1, K12 "Game output"): the legacy backend's present
//			stage asks the frontend's recorder for the frame's output, which
//			the composition records with render.pass.output, one renderer per
//			swapchain format on the backend's device. Private to the
//			composition.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_CORE_OUTPUT_H
#define RENDER_COMPOSITION_CORE_OUTPUT_H

#include "render/legacy/core_passes.h"
#include "render/pass/output/output.h"
#include "render/pass/post/post.h"

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace render::composition
{

// The tags of claimed post draws (RFC 0016 K8 "Post and screen effects"):
// forwarded, then bits 27 and 26, then a serial. World tags never set bit 27,
// temporal tags are 0x88 in the top byte and panel tags set bit 28.
inline constexpr std::uint32_t kPostTag = 0x8c000000u;
inline bool IsPostTag( std::uint32_t tag )
{
	return ( tag & 0xff000000u ) == kPostTag;
}

class CoreOutput
{
public:
	// A legacy bloom-chain draw (CoreMeshKind::kScreenEffect): claimed by
	// name, it returns a post tag; refused, 0 and the reason is kept.
	std::uint32_t QueuePost( const legacy::CoreMeshDraw &draw );
	// A post tag's slot, at its stream position: its claim joins the frame's
	// bloom. Records nothing (the output stage draws the bloom once).
	void RecordPost( std::uint32_t tag );
	// Claimed post draws, refused ones and the last refusal.
	std::uint64_t PostClaimed() const;
	std::uint64_t PostRefused() const;
	std::string PostLastRefusal() const;
	// Frames whose output added a bloom.
	std::uint64_t BloomFrames() const { return m_BloomFrames; }

	// Records the output as a section; false when nothing was recorded (an
	// invalid target, a format without an encoding, parameters the pass
	// refuses), so the backend presents as before.
	bool Record( device::CommandEncoder &encoder, const legacy::CoreOutputTargets &targets );
	void ReleaseDevice( device::IRenderDevice2 &device );
	// Frames whose output the pass refused.
	std::uint64_t Failures() const { return m_Failures; }

private:
	struct FramePost
	{
		bool downsample = false, blurX = false, blurY = false, add = false, enabled = true;
		pass::post::BloomParams params;
	};
	device::IRenderDevice2 *m_Device = nullptr;
	std::unique_ptr<pass::post::BloomRenderer> m_Bloom;
	mutable std::mutex m_PostLock;
	std::map<std::uint32_t, pass::post::PostClaim> m_PostQueued;
	FramePost m_FramePost;
	std::uint32_t m_PostSerial = 0;
	std::uint64_t m_PostClaimed = 0;
	std::uint64_t m_PostRefused = 0;
	std::string m_PostLastRefusal;
	std::uint64_t m_BloomFrames = 0;
	std::map<device::Format, std::unique_ptr<pass::output::OutputRenderer>> m_Renderers;
	std::uint64_t m_Failures = 0;
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_OUTPUT_H
