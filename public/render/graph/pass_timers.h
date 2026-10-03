//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GPU time of labeled sections (RFC 0014 D4, cl_render_debug_gpu_timers).
//			Attached to an encoder, the timers write a timestamp (clause D23)
//			after each label opens and before it closes, into a readback
//			buffer of the frame being recorded. Graph passes are labeled by
//			name, and hand-recorded sections label themselves, so every pass
//			is timed without naming it here.
//
//			A frame's times are read once a completion token covering its
//			submissions completes: the token given to EndFrame, or the
//			`submitted` of a later BeginFrame (no earlier than every
//			submission made before that frame). Buffers are reused only after
//			that; none is read or released before its token completes.
//
//			Called on the render sequence; encoders recorded on pool workers
//			may report their labels concurrently (the timers lock).
//
//=============================================================================//

#ifndef RENDER_GRAPH_PASS_TIMERS_H
#define RENDER_GRAPH_PASS_TIMERS_H

#include "render/device/device.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace render::graph
{

// One section's time over the frames a report covers.
struct PassTime
{
	std::string name;
	std::uint32_t depth = 0; // labels open around it
	double milliseconds = 0.0;
	std::uint32_t count = 0; // sections of that name and depth
};

struct PassTimerReport
{
	std::uint32_t frames = 0; // frames read into this report
	// In the order each name first recorded, summed over the frames.
	std::vector<PassTime> passes;
	std::uint32_t overflowed = 0; // timestamps dropped for want of room
};

class GpuPassTimers final : public device::ILabelObserver
{
public:
	// maxTimestamps per frame; a frame's later sections go untimed.
	GpuPassTimers( device::IRenderDevice2 &device, std::uint32_t maxTimestamps = 512 );
	~GpuPassTimers();
	GpuPassTimers( const GpuPassTimers & ) = delete;
	GpuPassTimers &operator=( const GpuPassTimers & ) = delete;

	// False when the device has no timestamps (D23): nothing is recorded.
	bool Supported() const { return m_Supported; }
	device::IRenderDevice2 &Device() const { return m_Device; }

	// Starts recording `frame` (a new value starts a new frame; the same one
	// continues it) and reads the frames `submitted` covers.
	void BeginFrame( std::uint64_t frame, device::CompletionToken submitted );
	// The frame being recorded is covered by `token` (a graph execution).
	void EndFrame( device::CompletionToken token );
	// Reads every frame whose token completed.
	void Collect();

	// The report since the last Take, and a new one begins.
	PassTimerReport Take();

	// Reports the encoder's labels until Detach (or the encoder's end).
	void Attach( device::CommandEncoder &encoder );
	void Detach( device::CommandEncoder &encoder );

	void OnBeginLabel( device::CommandEncoder &encoder, std::string_view label ) override;
	void OnEndLabel( device::CommandEncoder &encoder ) override;

private:
	// Timestamps are written in chunks of this many, one buffer each; an
	// encoder takes its own chunks, so no encoder's transition can discard
	// another's timestamps. Short product draw encoders use few labels; a
	// 16-slot chunk keeps their reserved storage within the timestamp budget.
	static constexpr std::uint32_t kChunk = 16;

	struct Section
	{
		std::string name;
		std::uint32_t depth = 0;
		std::uint32_t begin = 0; // timestamp indices (chunk * kChunk + slot)
		std::uint32_t end = 0;
		bool closed = false;
	};
	struct Frame
	{
		std::vector<device::BufferId> chunks; // kReadback, kCopyDestination
		std::uint32_t chunksUsed = 0;
		std::uint64_t frame = 0;
		bool waiting = false; // recorded; read once `done` completes
		bool hasDone = false;
		device::CompletionToken done;
		std::vector<Section> sections;
	};
	struct Attached
	{
		std::vector<std::uint32_t> open; // sections whose labels are open
		std::uint32_t chunk = 0;
		std::uint32_t slot = kChunk; // full: the next write takes a chunk
	};

	bool Write( device::CommandEncoder &encoder, Attached &attached, std::uint32_t *index );
	void Finish( device::CompletionToken done );
	void CollectLocked();
	void Read( Frame &frame );
	void Track( device::CompletionToken token );

	device::IRenderDevice2 &m_Device;
	std::uint32_t m_MaxChunks;
	bool m_Supported;
	std::mutex m_Lock;
	std::vector<std::unique_ptr<Frame>> m_Frames;
	Frame *m_Recording = nullptr;
	std::unordered_map<const device::CommandEncoder *, Attached> m_Attached;
	device::CompletionToken m_Latest; // the latest token seen, for release
	PassTimerReport m_Report;
};

} // namespace render::graph

#endif // RENDER_GRAPH_PASS_TIMERS_H
