//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Per-frame cost counters for native Vulkan frame-pacing measurement.
//
//          A frame hitch is only fixable once it has a name. Each operation that
//          can block the frame being built (a pipeline compile, a submit that
//          waits for the queue, a device wait, a buffer reallocation, a texture
//          upload, a query wait) records its count and wall time into the
//          current FrameCost. The context emits one line per presented frame
//          when a stats sink is open (tools/quality/frame_pacing.py reads it),
//          and otherwise only accumulates, which costs two clock reads per
//          instrumented operation.
//
//          mesh_draw and emit run while the engine builds the frame (before
//          Present), so they are part of a frame's "engine" time, not its
//          "backend" time.
//
//          Kinds nest where the operations do: a texture creation that submits
//          a layout transition counts under both texture_create and
//          single_submit. Sums across kinds are therefore not meaningful; each
//          kind answers "how much of this frame did X take".
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_FRAME_STATS_H
#define SHADERAPIVULKAN_VULKAN_FRAME_STATS_H

#ifdef _WIN32
#pragma once
#endif

#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>

namespace render_vulkan
{

enum FrameCostKind
{
	kCostPipelineCreate, // vkCreateGraphicsPipelines for a state first seen
	kCostSingleSubmit,   // a one-off command buffer submitted and waited for
	kCostDeviceWaitIdle, // vkDeviceWaitIdle outside teardown
	kCostTextureCreate,  // image, memory, views and descriptor sets
	kCostTextureUpload,  // staging copy of caller texels into an image
	kCostBufferGrow,     // a per-frame buffer reallocated to a larger size
	kCostQueryWait,      // an occlusion query read that waited for the GPU
	kCostFenceWait,      // BeginFrame waiting for a frame slot or image
	kCostAcquire,        // vkAcquireNextImageKHR
	kCostRecord,         // replaying the frame's stream into the command buffer
	kCostSubmit,         // vkQueueSubmit of the frame
	kCostPresent,        // vkQueuePresentKHR
	kCostMeshDraw,       // IMesh::Draw: the material's shader passes and their emits
	kCostEmit,           // converting a pass's mesh into the frame's vertex stream
	kCostEmitReuse,      // a pass that reused geometry already in the stream (count only)
	kFrameCostKinds
};

// Stable names: the frame-stats line format (frame_pacing.py) keys on them.
inline const char *FrameCostName( int kind )
{
	static const char *const kNames[kFrameCostKinds] = { "pipeline_create", "single_submit",
	    "device_wait_idle", "texture_create", "texture_upload", "buffer_grow", "query_wait",
	    "fence_wait", "acquire", "record", "submit", "present", "mesh_draw", "emit", "emit_reuse" };
	return kind >= 0 && kind < kFrameCostKinds ? kNames[kind] : "unknown";
}

inline uint64_t FrameClockMicros()
{
	return static_cast<uint64_t>( std::chrono::duration_cast<std::chrono::microseconds>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	                                  .count() );
}

// CPU time consumed by the calling thread, in microseconds (0 where the
// platform has no per-thread clock). A frame's wall interval minus the render
// thread's CPU time is time it waited or was preempted: on a loaded host the
// wall interval alone cannot tell a slower build from a busier machine.
inline uint64_t ThreadCpuMicros()
{
#if defined( CLOCK_THREAD_CPUTIME_ID )
	timespec now;
	if ( clock_gettime( CLOCK_THREAD_CPUTIME_ID, &now ) != 0 )
		return 0;
	return static_cast<uint64_t>( now.tv_sec ) * 1000000u +
	       static_cast<uint64_t>( now.tv_nsec ) / 1000u;
#else
	return 0;
#endif
}

struct FrameCost
{
	uint32_t count[kFrameCostKinds];
	uint64_t us[kFrameCostKinds];
	uint64_t uploadBytes;

	FrameCost() { Reset(); }
	void Reset()
	{
		std::memset( count, 0, sizeof( count ) );
		std::memset( us, 0, sizeof( us ) );
		uploadBytes = 0;
	}
	void Add( FrameCostKind kind, uint64_t micros )
	{
		++count[kind];
		us[kind] += micros;
	}
};

// Adds the scope's wall time to one kind of the frame's cost.
class CFrameCostScope
{
public:
	CFrameCostScope( FrameCost &cost, FrameCostKind kind )
	    : m_cost( cost ), m_kind( kind ), m_start( FrameClockMicros() )
	{
	}
	~CFrameCostScope() { m_cost.Add( m_kind, FrameClockMicros() - m_start ); }

	CFrameCostScope( const CFrameCostScope & ) = delete;
	CFrameCostScope &operator=( const CFrameCostScope & ) = delete;

private:
	FrameCost &m_cost;
	FrameCostKind m_kind;
	uint64_t m_start;
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_FRAME_STATS_H
